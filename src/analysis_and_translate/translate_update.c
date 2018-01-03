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
#include "model/query_operator/query_operator_model_checker.h"
#include "analysis_and_translate/translator.h"
#include "analysis_and_translate/translate_update.h"
#include "provenance_rewriter/prov_utility.h"

#include "configuration/option.h"

#include "metadata_lookup/metadata_lookup.h"

static QueryOperator *translateUpdateUnion(Update *f);
static QueryOperator *translateUpdateWithCase(Update *f);
static QueryOperator *translateInsert(Insert *f);
static QueryOperator *translateDelete(Delete *f);
static List*generateProjectionForUpdate(int hasCond, List* attrs, List* dts, Update* update);

QueryOperator *
translateUpdate(Node *update)
{
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

QueryOperator *
translateCreateTable(CreateTable *c)
{
    ConstRelOperator *op;
    SelectionOperator *sel;
    List *attrNames = getAttrDefNames(c->tableElems);
    List *dataTypes = getAttrDataTypes(c->tableElems);
    List *nulls = NIL;

    // translate a query
    if (c->query)
    {
        return translateQuery(c->query);
    }
    // create a new table without content for now use a selection on FALSE over a constant relation
    else
    {
        FOREACH_INT(dt,dataTypes)
            nulls = appendToTailOfList(nulls, createNullConst(dt));

        op = createConstRelOp(nulls, NIL, attrNames, dataTypes);
        sel = createSelectionOp((Node *) createConstBool(FALSE),
                (QueryOperator *) op, NIL, deepCopyStringList(attrNames));
    }

    return (QueryOperator *) sel;
}

QueryOperator *
translateAlterTable(AlterTable *a)
{
    TableAccessOperator *in;
    ProjectionOperator *p = NULL;
    List *attrNames = getAttrDefNames(a->beforeSchema);
    List *dataTypes = getAttrDataTypes(a->beforeSchema);
    List *newAttrs = getAttrDefNames(a->schema);

    //TODO how to deal with other alter tables
    in = createTableAccessOp(strdup(a->tableName), NULL, strdup(a->tableName), NIL,
            attrNames, dataTypes, NULL);
    switch(a->cmdType)
    {
        case ALTER_TABLE_REMOVE_COLUMN:
            p = (ProjectionOperator *) createProjOnAttrsByName((QueryOperator *) in, newAttrs);
        break;
        case ALTER_TABLE_ADD_COLUMN:
        {
            AttributeDef *newAttr = (AttributeDef *) getTailOfListP(a->schema);
            p = (ProjectionOperator *) createProjOnAllAttrs((QueryOperator *) in);
            p->projExprs = appendToTailOfList(p->projExprs, createNullConst(newAttr->dataType));
            addAttrToSchema((QueryOperator *) p, strdup(newAttr->attrName), newAttr->dataType);
        }
        break;
        default:
            FATAL_LOG("unknown alter table command %i", a->cmdType);
    }

    addChildOperator((QueryOperator *) p, (QueryOperator *) in);

    return (QueryOperator *) p;
}



static QueryOperator *
translateInsert(Insert *insert)
{
	List *attr = getAttrDefNames(insert->schema);//getAttributeNames(insert->insertTableName);
	List *dts = getAttrDataTypes(insert->schema);//getAttributeDataTypes(insert->insertTableName);
	QueryOperator *insertQuery;

	TableAccessOperator *to;
	to = createTableAccessOp(insert->insertTableName, NULL, NULL, NIL, deepCopyStringList(attr), dts, NULL);
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

	FORBOTH(AttributeDef,a,cA,GET_OPSCHEMA(seto)->attrDefs,GET_OPSCHEMA(to)->attrDefs)
	{
	    a->dataType = cA->dataType;
	}

	INFO_LOG("translated insert:\n%s", operatorToOverviewString((Node *) seto));
	DEBUG_LOG("translated insert:\n%s", nodeToString((Node *) seto));

	return (QueryOperator *) seto;
}

static QueryOperator *
translateDelete(Delete *delete)
{
    List *attr = getAttrDefNames(delete->schema);
    List *dts = getAttrDataTypes(delete->schema);

	TableAccessOperator *to;
	to = createTableAccessOp(strdup(delete->deleteTableName), NULL, NULL, NIL, deepCopyStringList(attr), dts, NULL);
	SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	SelectionOperator *so;
	Node *negatedCond;
	negatedCond = (Node *) createOpExpr("NOT", singleton(copyObject(delete->cond)));
	so = createSelectionOp(negatedCond, (QueryOperator *) to, NIL, deepCopyStringList(attr));

	addParent((QueryOperator *) to, (QueryOperator *) so);

	INFO_OP_LOG("translated delete:", so);
	DEBUG_NODE_BEATIFY_LOG("translated delete:", so);

	return (QueryOperator *) so;
}

static QueryOperator *
translateUpdateUnion(Update *update)
{
    List *attrs = getAttrDefNames(update->schema);//getAttributeNames(update->updateTableName);
    List *dts = getAttrDataTypes(update->schema);
    boolean hasCond = (update->cond != NULL);

    // create table access operator
	TableAccessOperator *to;
	to = createTableAccessOp(strdup(update->updateTableName), NULL, NULL, NIL, deepCopyStringList(attrs), dts, NULL);
	SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	// CREATE PROJECTION EXPRESSIONS
	List *projExprs = generateProjectionForUpdate(hasCond, attrs, dts, update);

	ProjectionOperator *po;
	po = createProjectionOp(projExprs, NULL, NIL, deepCopyStringList(attrs));

	// create selection operator, negated selection, and union if update has WHERE clause
	if (update->cond != NULL)
	{
        SelectionOperator *so;
        so = createSelectionOp(copyObject(update->cond), (QueryOperator *) to, NIL, deepCopyStringList(attrs));
        addParent((QueryOperator *) to, (QueryOperator *) so);
        addChildOperator((QueryOperator *) po, (QueryOperator *) so);

        SelectionOperator *nso;
        Node *negatedCond;
        negatedCond = (Node *) createOpExpr("NOT", singleton(copyObject(update->cond)));
        nso = createSelectionOp(negatedCond, (QueryOperator *) to, NIL, deepCopyStringList(attrs));
        addParent((QueryOperator *) to, (QueryOperator *) nso);

        SetOperator *seto;
        seto = createSetOperator(SETOP_UNION, LIST_MAKE(po,nso), NIL, deepCopyStringList(attrs));

        addParent((QueryOperator *) po, (QueryOperator *) seto);
        addParent((QueryOperator *) nso, (QueryOperator *) seto);

        DEBUG_NODE_BEATIFY_LOG("translated update:", po);

        return (QueryOperator *) seto;
	}

	// update without WHERE clause
    addChildOperator((QueryOperator *) po, (QueryOperator *) to);

    DEBUG_NODE_BEATIFY_LOG("translated update:", po);

    return (QueryOperator *) po;
}

static List*
generateProjectionForUpdate(int hasCond, List* attrs, List* dts, Update* update)
{
    List* projExprs = NIL;

    for (int i = 0; i < LIST_LENGTH(attrs); i++)
    {
        Node* projExpr = NULL;
        DataType aDT = getNthOfListInt(dts, i);
        FOREACH(Operator,o,update->selectClause)
        {
            AttributeReference *a = (AttributeReference *) getNthOfListP(
                    o->args, 0);
            if (a->attrPosition == i)
            {
                if (!hasCond)
                {
                    Node *upAttr = copyObject(getNthOfListP(o->args, 1));
                    projExpr = (Node *) upAttr;
                }
                else
                {
                    Node *cond = copyObject(update->cond);
                    Node *then = copyObject(getNthOfListP(o->args, 1));
                    Node *els = (Node *) createFullAttrReference(
                            getNthOfListP(attrs, i), 0, i, 0, a->attrType); //TODO
                    CaseExpr *caseExpr;
                    CaseWhen *caseWhen;

                    //                    if (!hasCond)
                    //                        cond = (Node *) createConstBool (TRUE);

                    caseWhen = createCaseWhen(cond, then);
                    caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

                    projExpr = (Node *) caseExpr;
                }
            }

        }
        if (projExpr == NULL)
            projExpr = (Node*) createFullAttrReference(getNthOfListP(attrs, i),
                    0, i, 0, aDT);

        projExprs = appendToTailOfList(projExprs, projExpr);
    }
    return projExprs;
}

static QueryOperator *
translateUpdateWithCase(Update *update)
{
	List *attrs = getAttrDefNames(update->schema);
	List *dts = getAttrDataTypes(update->schema);
	boolean hasCond = (update->cond != NULL);

	// create table access operator
	TableAccessOperator *to;
	to = createTableAccessOp(strdup(update->updateTableName), NULL, NULL, NIL,
			deepCopyStringList(attrs), dts, NULL);
    SET_BOOL_STRING_PROP(to,PROP_TABLE_IS_UPDATED);

	// CREATE PROJECTION EXPRESSIONS
	List *projExprs = NIL;
    projExprs = generateProjectionForUpdate(hasCond, attrs, dts, update);

	ProjectionOperator *po;
	po = createProjectionOp(projExprs, NULL, NIL, deepCopyStringList(attrs));

	addChildOperator((QueryOperator *) po, (QueryOperator *) to);
	DEBUG_NODE_BEATIFY_LOG("translated update:", po);
	ASSERT(checkModel((QueryOperator *)po));

	return (QueryOperator *) po;

}



