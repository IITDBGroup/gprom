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

#include "common.h"

#include "mem_manager/mem_mgr.h"
#include "model/query_block/query_block.h"
#include "model/node/nodetype.h"
#include "log/logger.h"

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
createFromItem (char *alias, List *attrNames)
{
    FromItem *result = makeNode(FromItem);

    result->name = alias;
    result->attrNames = attrNames;
    result->provInfo = NULL;

    return result;
}

FromItem *
createFromTableRef(char *alias, List *attrNames,
        char *tableId, List *dataTypes)
{
    FromTableRef *result = makeNode(FromTableRef);

    ((FromItem *) result)->name = alias;
    ((FromItem *) result)->attrNames = attrNames;
    ((FromItem *) result)->provInfo = NULL;
    ((FromItem *) result)->dataTypes = dataTypes;

    result->tableId = tableId;

    return (FromItem *) result;
}

FromItem *
createFromSubquery(char *alias, List *attrNames, Node *query)
{
    FromSubquery *result = makeNode(FromSubquery);

    ((FromItem *) result)->name = alias;
    ((FromItem *) result)->attrNames = attrNames;
    ((FromItem *) result)->provInfo = NULL;

    result->subquery = query;

    return (FromItem *) result;
}

FromItem *
createFromJoin(char *alias, List *attrNames, FromItem *left,
        FromItem *right, char *joinType, char *condType,
        Node *cond)
{
    FromJoinExpr *result = makeNode(FromJoinExpr);

    ((FromItem *) result)->name = alias;
    ((FromItem *) result)->attrNames = attrNames;
    ((FromItem *) result)->provInfo = NULL;

    result->left = left;
    result->right = right;
    result->cond = cond;
    result->joinType = joinTypeFromString(joinType);
    result->joinCond = joinConditionTypeFromString(condType);

    return (FromItem *) result;
}

FromItem *
createFromJsonTable(AttributeReference *jsonColumn, char *documentcontext, List *columns, char *jsonTableIdentifier)
{
    FromJsonTable *result = makeNode(FromJsonTable);
    result->columns = columns;
    result->documentcontext = strdup(documentcontext);
    result->jsonColumn = jsonColumn;
    result->jsonTableIdentifier = strdup(jsonTableIdentifier);
    ((FromItem *)result)->name = jsonTableIdentifier;

    return (FromItem *)result;
}

JsonColInfoItem *
createJsonColInfoItem (char *attrName, char *attrType, char *path, char *format, char *wrapper, List *nested, char *forOrdinality)
{
    JsonColInfoItem *result = makeNode(JsonColInfoItem);

    result->attrName = attrName;
    result->path = path;
    result->attrType = attrType;

    result->format = format;
    result->wrapper = wrapper;
    result->nested = nested;
    result->forOrdinality = forOrdinality;

    return result;
}

JsonPath *
createJsonPath(char *path)
{
	JsonPath *result = makeNode(JsonPath);
	result->path = path;

	return result;
}

JoinConditionType
joinConditionTypeFromString (char *condType)
{
    if (strcmp(condType,"JOIN_COND_ON") == 0)
            return JOIN_COND_ON;
    if (strcmp(condType,"JOIN_COND_USING") == 0)
            return JOIN_COND_USING;
    if (strcmp(condType,"JOIN_COND_NATURAL") == 0)
            return JOIN_COND_NATURAL;

    return JOIN_COND_ON;
}

JoinType
joinTypeFromString (char *joinType)
{
    if (strcmp(joinType,"JOIN_INNER") == 0)
        return JOIN_INNER;
    if (strcmp(joinType,"JOIN_CROSS") == 0)
            return JOIN_CROSS;
    if (strcmp(joinType,"JOIN_LEFT_OUTER") == 0)
            return JOIN_LEFT_OUTER;
    if (strcmp(joinType,"JOIN_RIGHT_OUTER") == 0)
            return JOIN_RIGHT_OUTER;
    if (strcmp(joinType,"JOIN_FULL_OUTER") == 0)
            return JOIN_FULL_OUTER;
    FATAL_LOG("unkown JoinType <%s>", joinType);
    return JOIN_CROSS;
}

DistinctClause *
createDistinctClause (List *distinctExprs)
{
    DistinctClause *result = makeNode(DistinctClause);

    result->distinctExprs = distinctExprs;

    return result;
}

NestedSubquery *
createNestedSubquery (char *nType, Node *expr,
        char *comparisonOp, Node *query)
{
    NestedSubquery *result = makeNode(NestedSubquery);

    if (!strcmp(nType, "ANY"))
        result->nestingType = NESTQ_ANY;
    if (!strcmp(nType, "ALL"))
        result->nestingType = NESTQ_ALL;
    if (!strcmp(nType, "EXISTS"))
        result->nestingType = NESTQ_EXISTS;
    if (!strcmp(nType, "SCALAR"))
        result->nestingType = NESTQ_SCALAR;

    result->expr = expr;
    result->comparisonOp = strdup(comparisonOp);
    result->query = query;

    return result;
}

Insert *
createInsert(char *nodeName, Node *query, List *attrs)
{
    Insert *result = makeNode(Insert);
    result->tableName = nodeName;
    result->query = query;
    result->attrList = attrs;

    return result;
}


Delete *
createDelete(char *nodeName, Node *cond)
{
    Delete *result = makeNode(Delete);
    result->nodeName = nodeName;
    result->cond = cond;

    return result;
}


Update *
createUpdate(char *nodeName, List *selectClause, Node *cond)
{
    Update *result = makeNode(Update);
    result->nodeName = nodeName;
    ((Update *) result)->selectClause = selectClause;
    result->cond = cond;

    return result;
}

TransactionStmt *
createTransactionStmt (char *stmtType)
{
    TransactionStmt *result = makeNode(TransactionStmt);

    if (strcmp(stmtType, "TRANSACTION_BEGIN") == 0)
        result->stmtType = TRANSACTION_BEGIN;
    else if (strcmp(stmtType, "TRANSACTION_COMMIT") == 0)
        result->stmtType = TRANSACTION_COMMIT;
    else if (strcmp(stmtType, "TRANSACTION_ABORT") == 0)
        result->stmtType = TRANSACTION_ABORT;
    else
        FATAL_LOG("unkown transaction stmt type <%s>", stmtType);
    return result;
}

WithStmt *
createWithStmt (List *views, Node *query)
{
    WithStmt *result = makeNode(WithStmt);

    result->withViews = views;
    result->query = query;

    return result;
}
