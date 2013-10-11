/*-----------------------------------------------------------------------------
 *
 * query_block.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "mem_manager/mem_mgr.h"
#include "model/query_block/query_block.h"
#include "model/node/nodetype.h"

SetQuery *
createSetQuery(char *setOp, boolean all, Node *lChild,
        Node *rChild)
{
    SetQuery *result = makeNode(SetQuery);

    if (!strcmp(setOp, "UNION"))
        result->setOp = SETOP_UNION;
    if (!strcmp(setOp, "INTERSECT"))
        result->setOp = SETOP_INTERSECTION;
    if (!strcmp(setOp, "MINUS"))
        result->setOp = SETOP_DIFFERENCE;

    result->all = all;
    result->selectClause = NIL;
    result->lChild = lChild;
    result->rChild = rChild;

    return result;
}

QueryBlock *
createQueryBlock(void)
{
    return makeNode(QueryBlock);
}

ProvenanceStmt *
createProvenanceStmt(Node *query)
{
    ProvenanceStmt *result = makeNode(ProvenanceStmt);

    result->query = query;

    return result;
}

SelectItem *
createSelectItem(char *alias, Node *expr)
{
    SelectItem *result = makeNode(SelectItem);

    result->alias = alias;
    result->expr = expr;

    return result;
}

FromItem *
createFromTableRef(char *alias, List *attrNames,
        char *tableId)
{
    FromTableRef *result = makeNode(FromTableRef);

    ((FromItem *) result)->name = alias;
    ((FromItem *) result)->attrNames = attrNames;

    result->tableId = tableId;

    return (FromItem *) result;
}

FromItem *
createFromSubquery(char *alias, List *attrNames, Node *query)
{
    FromSubquery *result = makeNode(FromSubquery);

    ((FromItem *) result)->name = alias;
    ((FromItem *) result)->attrNames = attrNames;

    result->subquery = query;

    return (FromItem *) result;
}

FromItem *
createFromJoin(char *alias, List *attrNames, FromItem *left,
        FromItem *right, JoinType joinType, JoinConditionType condType,
        Node *cond)
{
    FromJoinExpr *result = makeNode(FromJoinExpr);

    ((FromItem *) result)->name = alias;
    ((FromItem *) result)->attrNames = attrNames;

    result->left = left;
    result->right = right;
    result->cond = cond;
    result->joinType = joinType;
    result->joinCond = condType;

    return (FromItem *) result;
}

DistinctClause *
createDistinctClause (List *distinctExprs)
{
    DistinctClause *result = makeNode(DistinctClause);

    result->distinctExprs = distinctExprs;

    return result;
}

NestedSubquery *
createNestedSubquery (NestingExprType nType, Node *expr,
        char *comparisonOp, Node *query)
{
    NestedSubquery *result = makeNode(NestedSubquery);

    result->nestingType = nType;
    result->expr = expr;
    result->comparisonOp = strdup(comparisonOp);
    result->query = query;

    return result;
}
