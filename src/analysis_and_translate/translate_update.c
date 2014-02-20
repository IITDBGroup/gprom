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

#include "mem_manager/mem_mgr.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/translator.h"
#include "analysis_and_translate/translate_update.h"

#include "metadata_lookup/metadata_lookup.h"

static QueryOperator *translateUpdateInternal(Update *f);
static QueryOperator *translateInsert(Insert *f);
static QueryOperator *translateDelete(Delete *f);

QueryOperator *
translateUpdate(Node *update)
{
	switch (update->type) {
        case T_Insert:
            return translateInsert((Insert *) update);
        case T_Delete:
            return translateDelete((Delete *) update);
        case T_Update:
//            if (isRewriteOptionActivated("translate_update_with_case"))
//                return translateUpdateWithCase((Update *) update);
//            else
                return translateUpdateInternal((Update *) update);
        default:
            return NULL;
	}
}


static QueryOperator *
translateInsert(Insert *insert)
{
	List *attr = getAttributeNames(insert->tableName);
	QueryOperator *insertQuery;

	TableAccessOperator *to;
	to = createTableAccessOp(insert->tableName, NULL, NULL, NIL, deepCopyStringList(attr), NIL);

	if (isA(insert->query,  List))
	{
		ConstRelOperator *co;
		co = createConstRelOp((List *) insert->query,NIL, deepCopyStringList(attr),NIL);
		insertQuery= (QueryOperator *) co;
	}
	else
	{
	    insertQuery =  translateQuery((Node *) insert->query);
	}

	SetOperator *seto;
	seto = createSetOperator(SETOP_UNION, NIL, NIL, deepCopyStringList(attr));

	addChildOperator((QueryOperator *) seto, (QueryOperator *) to);
	addChildOperator((QueryOperator *) seto, insertQuery);

	return (QueryOperator *) seto;
}

static QueryOperator *
translateDelete(Delete *delete)
{
	List *attr = getAttributeNames(delete->nodeName);

	TableAccessOperator *to;
	to = createTableAccessOp(strdup(delete->nodeName), NULL, NULL, NIL, deepCopyStringList(attr), NIL);

	SelectionOperator *so;
	Node *negatedCond;
	negatedCond = (Node *) createOpExpr("NOT", singleton(copyObject(delete->cond)));
	so = createSelectionOp(negatedCond, NULL, NIL, deepCopyStringList(attr));

	addChildOperator((QueryOperator *) so, (QueryOperator *) to);

	return (QueryOperator *) so;
}

static QueryOperator *
translateUpdateInternal(Update *update)
{
    List *attrs = getAttributeNames(update->nodeName);

    // create table access operator
	TableAccessOperator *to;
	to = createTableAccessOp(strdup(update->nodeName), NULL, NULL, NIL, deepCopyStringList(attrs), NIL);

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
	        projExpr = (Node *) createFullAttrReference(getNthOfListP(attrs,i), 0, i, 0);
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

        return (QueryOperator *) seto;
	}

	// update without WHERE clause
    addChildOperator((QueryOperator *) po, (QueryOperator *) to);
    return (QueryOperator *) po;
}




