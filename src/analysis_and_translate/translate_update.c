/*-----------------------------------------------------------------------------
 *
 * translate_update.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "log/logger.h"

#include "mem_manager/mem_mgr.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "analysis_and_translate/translator.h"
#include "analysis_and_translate/translate_update.h"

#include "configuration/option.h"

#include "metadata_lookup/metadata_lookup.h"

static QueryOperator *translateUpdateUnion(Update *f);
static QueryOperator *translateUpdateWithCase(Update *f);
static QueryOperator *translateInsert(Insert *f);
static QueryOperator *translateDelete(Delete *f);


QueryOperator *
translateUpdate(Node *update) {
	switch (update->type) {
	case T_Insert:
		return translateInsert((Insert *) update);
	case T_Delete:
		return translateDelete((Delete *) update);
	case T_Update:
		if (isRewriteOptionActivated(OPTION_TRANSLATE_UPDATE_WITH_CASE))
			return translateUpdateWithCase((Update *) update);
		else
			return translateUpdateUnion((Update *) update);
	default:
		return NULL;
	}
}


static QueryOperator *
translateInsert(Insert *insert)
{
	List *attr = getAttributeNames(insert->tableName);
	List *dts = getAttributeDataTypes(insert->tableName);
	QueryOperator *insertQuery;

	TableAccessOperator *to;
	to = createTableAccessOp(insert->tableName, NULL, NULL, NIL, deepCopyStringList(attr), dts);
	SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	if (isA(insert->query,  List))
	{
		ConstRelOperator *co;
		co = createConstRelOp((List *) insert->query,NIL, deepCopyStringList(attr), dts);
		insertQuery= (QueryOperator *) co;
	}
	else
	    insertQuery =  translateQuery((Node *) insert->query);

	SetOperator *seto;
	seto = createSetOperator(SETOP_UNION, NIL, NIL, deepCopyStringList(attr));

	addChildOperator((QueryOperator *) seto, (QueryOperator *) to);
	addChildOperator((QueryOperator *) seto, insertQuery);

	INFO_LOG("translated insert:\n%s", operatorToOverviewString((Node *) seto));
	DEBUG_LOG("translated insert:\n%s", nodeToString((Node *) seto));

	return (QueryOperator *) seto;
}

static QueryOperator *
translateDelete(Delete *delete)
{
	List *attr = getAttributeNames(delete->nodeName);

	TableAccessOperator *to;
	to = createTableAccessOp(strdup(delete->nodeName), NULL, NULL, NIL, deepCopyStringList(attr), NIL);
	SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	SelectionOperator *so;
	Node *negatedCond;
	negatedCond = (Node *) createOpExpr("NOT", singleton(copyObject(delete->cond)));
	so = createSelectionOp(negatedCond, NULL, NIL, deepCopyStringList(attr));

	addChildOperator((QueryOperator *) so, (QueryOperator *) to);

	INFO_LOG("translated delete:\n%s", operatorToOverviewString((Node *) so));
	DEBUG_LOG("translated delete:\n%s", beatify(nodeToString((Node *) so)));

	return (QueryOperator *) so;
}

static QueryOperator *
translateUpdateUnion(Update *update)
{
    List *attrs = getAttributeNames(update->nodeName);

    // create table access operator
	TableAccessOperator *to;
	to = createTableAccessOp(strdup(update->nodeName), NULL, NULL, NIL, deepCopyStringList(attrs), NIL);
	SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	// CREATE PROJECTION EXPRESSIONS
	List *projExprs = NIL;
	for(int i = 0; i < LIST_LENGTH(attrs); i++)
	{
        Node *projExpr= NULL;
	    FOREACH(Operator,o,update->selectClause)
        {
	        AttributeReference *a = (AttributeReference *) getNthOfListP(o->args, 0);
	        if (a->attrPosition == i)
	            projExpr = (Node *) copyObject(getNthOfListP(o->args, 1));
        }

	    if (projExpr == NULL)
	        projExpr = (Node *) createFullAttrReference(getNthOfListP(attrs,i), 0, i, 0, DT_STRING); //TODO
	    projExprs = appendToTailOfList(projExprs, projExpr);
	}

	ProjectionOperator *po;
	po = createProjectionOp(projExprs, NULL, NIL, deepCopyStringList(attrs));

	// create selection operator, negated selection, and union if update has WHERE clause
	if (update->cond != NULL)
	{
        SelectionOperator *so;
        so = createSelectionOp(copyObject(update->cond), NULL, NIL, deepCopyStringList(attrs));
        addChildOperator((QueryOperator *) so, (QueryOperator *) to);
        addChildOperator((QueryOperator *) po, (QueryOperator *) so);

        SelectionOperator *nso;
        Node *negatedCond;
        negatedCond = (Node *) createOpExpr("NOT", singleton(copyObject(update->cond)));
        nso = createSelectionOp(negatedCond, NULL, NIL, deepCopyStringList(attrs));
        addChildOperator((QueryOperator *) nso, (QueryOperator *) to);

        SetOperator *seto;
        seto = createSetOperator(SETOP_UNION, NIL, NIL, deepCopyStringList(attrs));

        addChildOperator((QueryOperator *) seto, (QueryOperator *) po);
        addChildOperator((QueryOperator *) seto, (QueryOperator *) nso);

        INFO_LOG("translated update:\n%s", operatorToOverviewString((Node *) seto));
        DEBUG_LOG("translated update:\n%s", nodeToString((Node *) seto));

        return (QueryOperator *) seto;
	}

	// update without WHERE clause
    addChildOperator((QueryOperator *) po, (QueryOperator *) to);

    INFO_LOG("translated update:\n%s", operatorToOverviewString((Node *) po));
    DEBUG_LOG("translated update:\n%s", beatify(nodeToString((Node *) po)));

    return (QueryOperator *) po;
}

static QueryOperator *
translateUpdateWithCase(Update *update)
{
	List *attrs = getAttributeNames(update->nodeName);
	List *dts = getAttributeDataTypes(update->nodeName);
	boolean hasCond = (update->cond != NULL);

	// create table access operator
	TableAccessOperator *to;
	to = createTableAccessOp(strdup(update->nodeName), NULL, NULL, NIL,
			deepCopyStringList(attrs), dts);
    SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	// CREATE PROJECTION EXPRESSIONS
	List *projExprs = NIL;
	for (int i = 0; i < LIST_LENGTH(attrs); i++)
	{
		Node *projExpr = NULL;
		DataType aDT = getNthOfListInt(dts, i);

		FOREACH(Operator,o,update->selectClause)
		{
			AttributeReference *a = (AttributeReference *) getNthOfListP(
					o->args, 0);
			if (a->attrPosition == i)
			{
			    Node *cond = copyObject(update->cond);
			    Node *then = copyObject(getNthOfListP(o->args, 1));
			    Node *els = (Node *) createFullAttrReference(getNthOfListP(attrs, i),
			                0, i, 0, a->attrType); //TODO
                CaseExpr *caseExpr;
                CaseWhen *caseWhen;

                if (!hasCond)
                    cond = (Node *) createConstBool (TRUE);

                caseWhen = createCaseWhen(cond, then);
                caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

                projExpr = (Node *) caseExpr;
			}

		}

		if (projExpr == NULL)
			projExpr = (Node *) createFullAttrReference(getNthOfListP(attrs, i),
					0, i, 0, aDT);
		projExprs = appendToTailOfList(projExprs, projExpr);
	}

	ProjectionOperator *po;
	po = createProjectionOp(projExprs, NULL, NIL, deepCopyStringList(attrs));

	addChildOperator((QueryOperator *) po, (QueryOperator *) to);
	DEBUG_LOG("translated update:\n%s", beatify(nodeToString((Node *) po)));

	return (QueryOperator *) po;

}



