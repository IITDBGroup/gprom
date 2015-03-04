/*-----------------------------------------------------------------------------
 *
 * hash.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_block/query_block.h"
#include "model/datalog/datalog_model.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/list/list.h"
#include "log/logger.h"

// hash constants
#define FNV_OFFSET ((uint64_t) 14695981039346656037U)
#define FNV_PRIME ((uint64_t) 1099511628211U)

// hash macros
#define HASH_RETURN() return cur

#define HASH_STRING(a) cur = hashString(cur, node->a)
#define HASH_INT(a) cur = hashInt(cur, node->a)
#define HASH_BOOLEAN(a) cur = hashBool(cur, node->a)
#define HASH_NODE(a) cur = hashValueInternal(cur, node->a)

// hash functions for simple types
static inline uint64_t hashInt(uint64_t cur, int value);
static inline uint64_t hashLong(uint64_t cur, long value);
static inline uint64_t hashFloat(uint64_t cur, float value);
static inline uint64_t hashString(uint64_t cur, char *value);
static inline uint64_t hashBool(uint64_t cur, boolean value);
static inline uint64_t hashMemory(uint64_t cur, void *memP, size_t bytes);

// hash functions for collections
static inline uint64_t hashList(uint64_t cur, List *node);
static uint64_t hashSet (uint64_t cur, Set *node);
static uint64_t hashHashMap (uint64_t cur, HashMap *node);
static uint64_t hashVector (uint64_t cur, Vector *node);
static uint64_t hashKeyValue (uint64_t cur, KeyValue *node);

// hash for expression nodes
static uint64_t hashConstant (uint64_t cur, Constant *node);
static uint64_t hashAttributeReference (uint64_t cur, AttributeReference *node);
static uint64_t hashSQLParameter (uint64_t cur, SQLParameter *node);
static uint64_t hashCaseExpr (uint64_t cur, CaseExpr *node);
static uint64_t hashCaseWhen (uint64_t cur, CaseWhen *node);
static uint64_t hashIsNullExpr (uint64_t cur, IsNullExpr *node);
static uint64_t hashWindowBound (uint64_t cur, WindowBound *node);
static uint64_t hashWindowFrame (uint64_t cur, WindowFrame *node);
static uint64_t hashWindowDef (uint64_t cur, WindowDef *node);
static uint64_t hashWindowFunction (uint64_t cur, WindowFunction *node);

// hash functions for query block model
static uint64_t hashFunctionCall (uint64_t cur, FunctionCall *node);
static uint64_t hashOperator (uint64_t cur, Operator *node);
static uint64_t hashRowNumExpr (uint64_t cur, RowNumExpr *node);
static uint64_t hashOrderExpr (uint64_t cur, OrderExpr *node);
static uint64_t hashCastExpr (uint64_t cur, CastExpr *node);
static uint64_t hashSetQuery (uint64_t cur, SetQuery *node);
static uint64_t hashProvenanceStmt (uint64_t cur, ProvenanceStmt *node);
static uint64_t hashProvenanceTransactionInfo (uint64_t cur, ProvenanceTransactionInfo *node);
static uint64_t hashQueryBlock (uint64_t cur, QueryBlock *node);
static uint64_t hashSelectItem (uint64_t cur, SelectItem *node);
static uint64_t hashFromProvInfo (uint64_t cur, FromProvInfo *node);
static uint64_t hashFromTableRef (uint64_t cur, FromTableRef *node);
static uint64_t hashFromSubquery (uint64_t cur, FromSubquery *node);
static uint64_t hashFromJoinExpr (uint64_t cur, FromJoinExpr *node);
static uint64_t hashDistinctClause (uint64_t cur, DistinctClause *node);
static uint64_t hashNestedSubquery (uint64_t cur, NestedSubquery *node);
static uint64_t hashInsert (uint64_t cur, Insert *node);
static uint64_t hashDelete (uint64_t cur, Delete *node);
static uint64_t hashUpdate (uint64_t cur, Update *node);
static uint64_t hashTransactionStmt (uint64_t cur, TransactionStmt *node);
static uint64_t hashWithStmt (uint64_t cur, WithStmt *node);
static uint64_t hashUtilityStatement (uint64_t cur, UtilityStatement *node);

// hash functions for query operator model
static uint64_t hashSchema (uint64_t cur, Schema *node);
static uint64_t hashAttributeDef (uint64_t cur, AttributeDef *node);
static uint64_t hashQueryOperator (uint64_t cur, QueryOperator *node);
static uint64_t hashSelectionOperator (uint64_t cur, SelectionOperator *node);
static uint64_t hashProjectionOperator (uint64_t cur, ProjectionOperator *node);
static uint64_t hashJoinOperator (uint64_t cur, JoinOperator *node);
static uint64_t hashAggregationOperator (uint64_t cur, AggregationOperator *node);
static uint64_t hashProvenanceComputation (uint64_t cur, ProvenanceComputation *node);
static uint64_t hashTableAccessOperator (uint64_t cur, TableAccessOperator *node);
static uint64_t hashSetOperator (uint64_t cur, SetOperator *node);
static uint64_t hashDuplicateRemoval (uint64_t cur, DuplicateRemoval *node);
static uint64_t hashConstRelOperator (uint64_t cur, ConstRelOperator *node);
static uint64_t hashNestingOperator (uint64_t cur, NestingOperator *node);
static uint64_t hashWindowOperator (uint64_t cur, WindowOperator *node);
static uint64_t hashOrderOperator (uint64_t cur, OrderOperator *node);

// hash functions for datalog model
static uint64_t hashDLNode (uint64_t cur, DLNode *node);
static uint64_t hashDLAtom (uint64_t cur, DLAtom *node);
static uint64_t hashDLVar (uint64_t cur, DLVar *node);
static uint64_t hashDLRule (uint64_t cur, DLRule *node);
static uint64_t hashDLProgram (uint64_t cur, DLProgram *node);
static uint64_t hashDLComparison (uint64_t cur, DLComparison *node);

// hash function entry point
static uint64_t hashValueInternal(uint64_t h, void *a);

/* hash simple values */
static inline uint64_t
hashInt(uint64_t cur, int value)
{
    return hashMemory(cur, &value, sizeof(int));
}

static inline uint64_t
hashString(uint64_t cur, char *value)
{
    if (value == NULL)
        return cur;
    size_t len = strlen(value);
    return hashMemory(cur, value, len);
}

static inline uint64_t
hashBool(uint64_t cur, boolean value)
{
    return hashMemory(cur, &value, sizeof(boolean));
}

static inline uint64_t
hashLong(uint64_t cur, long value)
{
    return hashMemory(cur, &value, sizeof(long));
}

static inline uint64_t
hashFloat(uint64_t cur, float value)
{
    return hashMemory(cur, &value, sizeof(float));
}

static inline uint64_t
hashMemory(uint64_t cur, void *memP, size_t bytes)
{
    // byte pointers
    char *curP = (char *) &cur;
    char *valueP = (char *) memP;

    for(int i = 0; i < bytes; i++, valueP++)
    {
        cur *= FNV_PRIME;
        *curP ^= *valueP;
    }

    return cur;
}

/* hash collections */
static inline uint64_t
hashList(uint64_t cur, List *node)
{
    if (isA(node, List))
    {
        FOREACH(Node,n,node)
            cur = hashValueInternal(cur, n);
    }
    if (isA(node, IntList))
    {
        FOREACH_INT(i,node)
            cur = hashInt(cur, i);
    }

    HASH_RETURN();
}

uint64_t
hashStringList(uint64_t cur, List *node)
{
    FOREACH(char,n,node)
        cur = hashString(cur, n);

    HASH_RETURN();
}

static uint64_t
hashSet (uint64_t cur, Set *node)
{
    HASH_INT(setType);

    switch (node->setType)
    {
        case SET_TYPE_INT:
            FOREACH_SET_INT(i,node)
                hashInt(cur, i);
            break;
        case SET_TYPE_NODE:
            FOREACH_SET(Node,n,node)
                hashValueInternal(cur,n);
            break;
        case SET_TYPE_STRING:
            FOREACH_SET(char,c,node)
                hashString(cur,c);
            break;
        case SET_TYPE_POINTER:
            FOREACH_SET(void,p,node)
                hashLong(cur, (long) p);
            break;
    }

    HASH_RETURN();
}


static uint64_t
hashHashMap (uint64_t cur, HashMap *node)
{
    HASH_INT(keyType);
    HASH_INT(valueType);

    FOREACH_HASH_ENTRY(e,node)
        hashKeyValue(cur, e);

    HASH_RETURN();
}


static uint64_t
hashVector (uint64_t cur, Vector *node)
{
    switch(node->elType)
    {
        case VECTOR_INT:
            FOREACH_VEC_INT(i,node)
                hashInt(cur,*i);
        break;
        case VECTOR_NODE:
            FOREACH_VEC(Node,n,node)
                hashValueInternal(cur, *n);
            break;
        case VECTOR_STRING:
            FOREACH_VEC(char,c,node)
                hashString(cur, *c);
            break;
    }

    HASH_RETURN();
}

/* expression node hash functions */
static uint64_t
hashConstant (uint64_t cur, Constant *node)
{
    HASH_INT(constType);

    switch(node->constType)
    {
        case DT_INT:
            cur = hashInt(cur, INT_VALUE(node));
            break;
        case DT_LONG:
            cur = hashLong(cur, LONG_VALUE(node));
            break;
        case DT_FLOAT:
            cur = hashFloat(cur, FLOAT_VALUE(node));
            break;
        case DT_BOOL:
            cur = hashBool(cur, BOOL_VALUE(node));
            break;
        case DT_STRING:
            cur = hashString(cur, STRING_VALUE(node));
            break;
        case DT_VARCHAR2:
	    cur = hashString(cur, STRING_VALUE(node));
	    break;
    }

    HASH_BOOLEAN(isNull);

    HASH_RETURN();
}

static uint64_t
hashAttributeReference (uint64_t cur, AttributeReference *node)
{
    HASH_STRING(name);
    HASH_INT(fromClauseItem);
    HASH_INT(attrPosition);
    HASH_INT(outerLevelsUp);

    HASH_RETURN();
}

static uint64_t
hashSQLParameter (uint64_t cur, SQLParameter *node)
{
    HASH_STRING(name);
    HASH_INT(position);
    HASH_INT(parType);

    HASH_RETURN();
}

static uint64_t
hashCaseExpr (uint64_t cur, CaseExpr *node)
{
    HASH_NODE(expr);
    HASH_NODE(whenClauses);
    HASH_NODE(elseRes);

    HASH_RETURN();
}

static uint64_t
hashCaseWhen (uint64_t cur, CaseWhen *node)
{
    HASH_NODE(when);
    HASH_NODE(then);

    HASH_RETURN();
}

static uint64_t
hashIsNullExpr (uint64_t cur, IsNullExpr *node)
{
    HASH_NODE(expr);

    HASH_RETURN();
}

static uint64_t
hashWindowBound (uint64_t cur, WindowBound *node)
{
    HASH_INT(bType);
    HASH_NODE(expr);

    HASH_RETURN();
}

static uint64_t
hashWindowFrame (uint64_t cur, WindowFrame *node)
{
    HASH_INT(frameType);
    HASH_NODE(lower);
    HASH_NODE(higher);

    HASH_RETURN();
}

static uint64_t
hashWindowDef (uint64_t cur, WindowDef *node)
{
    HASH_NODE(partitionBy);
    HASH_NODE(orderBy);
    HASH_NODE(frame);

    HASH_RETURN();
}

static uint64_t
hashWindowFunction (uint64_t cur, WindowFunction *node)
{
    HASH_NODE(f);
    HASH_NODE(win);

    HASH_RETURN();
}




static uint64_t
hashKeyValue (uint64_t cur, KeyValue *node)
{
    HASH_NODE(key);
    HASH_NODE(value);

    HASH_RETURN();
}


static uint64_t
hashFunctionCall (uint64_t cur, FunctionCall *node)
{
    HASH_STRING(functionname);
    HASH_NODE(args);
    HASH_BOOLEAN(isAgg);

    HASH_RETURN();
}


static uint64_t
hashOperator (uint64_t cur, Operator *node)
{
    HASH_STRING(name);
    HASH_NODE(args);

    HASH_RETURN();
}


static uint64_t
hashRowNumExpr (uint64_t cur, RowNumExpr *node)
{
    HASH_RETURN();
}


static uint64_t
hashOrderExpr (uint64_t cur, OrderExpr *node)
{
    HASH_NODE(expr);
    HASH_INT(order);
    HASH_INT(nullOrder);

    HASH_RETURN();
}


static uint64_t
hashCastExpr (uint64_t cur, CastExpr *node)
{
    HASH_INT(resultDT);
    HASH_NODE(expr);

    HASH_RETURN();
}


static uint64_t
hashSetQuery (uint64_t cur, SetQuery *node)
{
    HASH_INT(setOp);
    HASH_BOOLEAN(all);
    HASH_NODE(selectClause);
    HASH_NODE(lChild);
    HASH_NODE(rChild);

    HASH_RETURN();
}


static uint64_t
hashProvenanceStmt (uint64_t cur, ProvenanceStmt *node)
{
    HASH_NODE(query);
    HASH_NODE(selectClause);
    HASH_INT(provType);
    HASH_INT(inputType);
    HASH_NODE(transInfo);
    HASH_NODE(asOf);
    HASH_NODE(options);

    HASH_RETURN();
}


static uint64_t
hashProvenanceTransactionInfo (uint64_t cur, ProvenanceTransactionInfo *node)
{
    HASH_INT(transIsolation);
    HASH_NODE(updateTableNames);
    HASH_NODE(originalUpdates);
    HASH_NODE(scns);
    HASH_NODE(commitSCN);

    HASH_RETURN();
}


static uint64_t
hashQueryBlock (uint64_t cur, QueryBlock *node)
{
    HASH_NODE(selectClause);
    HASH_NODE(distinct);
    HASH_NODE(fromClause);
    HASH_NODE(whereClause);
    HASH_NODE(groupByClause);
    HASH_NODE(havingClause);
    HASH_NODE(orderByClause);
    HASH_NODE(limitClause);

    HASH_RETURN();
}


static uint64_t
hashSelectItem (uint64_t cur, SelectItem *node)
{
    HASH_STRING(alias);
    HASH_NODE(expr);

    HASH_RETURN();
}


static uint64_t
hashFromProvInfo (uint64_t cur, FromProvInfo *node)
{
    HASH_BOOLEAN(baserel);
    HASH_BOOLEAN(intermediateProv);
    HASH_NODE(userProvAttrs);

    HASH_RETURN();
}

static uint64_t
hashFromItem (uint64_t cur, FromItem *node)
{
    HASH_STRING(name);
    HASH_NODE(attrNames);
    HASH_NODE(dataTypes);
    HASH_NODE(provInfo);

    HASH_RETURN();
}

#define HASH_FROM_ITEM() hashFromItem(cur, (FromItem *) node)

static uint64_t
hashFromTableRef (uint64_t cur, FromTableRef *node)
{
    HASH_FROM_ITEM();
    HASH_STRING(tableId);

    HASH_RETURN();
}


static uint64_t
hashFromSubquery (uint64_t cur, FromSubquery *node)
{
    HASH_FROM_ITEM();
    HASH_NODE(subquery);

    HASH_RETURN();
}


static uint64_t
hashFromJoinExpr (uint64_t cur, FromJoinExpr *node)
{
    HASH_FROM_ITEM();
    HASH_NODE(left);
    HASH_NODE(right);
    HASH_INT(joinType);
    HASH_INT(joinCond);
    HASH_NODE(cond);

    HASH_RETURN();
}


static uint64_t
hashDistinctClause (uint64_t cur, DistinctClause *node)
{
    HASH_NODE(distinctExprs);

    HASH_RETURN();
}


static uint64_t
hashNestedSubquery (uint64_t cur, NestedSubquery *node)
{
    HASH_INT(nestingType);
    HASH_NODE(expr);
    HASH_STRING(comparisonOp);
    HASH_NODE(query);

    HASH_RETURN();
}


static uint64_t
hashInsert (uint64_t cur, Insert *node)
{
    HASH_STRING(tableName);
    HASH_NODE(attrList);
    HASH_NODE(query);

    HASH_RETURN();
}


static uint64_t
hashDelete (uint64_t cur, Delete *node)
{
    HASH_STRING(nodeName);
    HASH_NODE(cond);

    HASH_RETURN();
}


static uint64_t
hashUpdate (uint64_t cur, Update *node)
{
    HASH_STRING(nodeName);
    HASH_NODE(selectClause);
    HASH_NODE(cond);

    HASH_RETURN();
}


static uint64_t
hashTransactionStmt (uint64_t cur, TransactionStmt *node)
{
    HASH_INT(stmtType);

    HASH_RETURN();
}


static uint64_t
hashWithStmt (uint64_t cur, WithStmt *node)
{
    HASH_NODE(withViews);
    HASH_NODE(query);

    HASH_RETURN();
}


static uint64_t
hashUtilityStatement (uint64_t cur, UtilityStatement *node)
{
    HASH_INT(stmtType);
    HASH_RETURN();
}


static uint64_t
hashSchema (uint64_t cur, Schema *node)
{
    HASH_STRING(name);
    HASH_NODE(attrDefs);

    HASH_RETURN();
}


static uint64_t
hashAttributeDef (uint64_t cur, AttributeDef *node)
{
    HASH_INT(dataType);
    HASH_STRING(attrName);
    HASH_INT(pos);

    HASH_RETURN();
}

static uint64_t
hashQueryOperator (uint64_t cur, QueryOperator *node)
{
    HASH_NODE(inputs);
    HASH_NODE(schema);
    HASH_NODE(provAttrs);
    HASH_NODE(properties);

    // want to hash parents, but cannot traverse because it may result infinite loops
    FOREACH(void,p,node->parents)
        hashLong(cur, (long) p);

    HASH_RETURN();
}

#define HASH_QO() hashQueryOperator (cur, (QueryOperator *) node)

static uint64_t
hashSelectionOperator (uint64_t cur, SelectionOperator *node)
{
    HASH_QO();
    HASH_NODE(cond);

    HASH_RETURN();
}


static uint64_t
hashProjectionOperator (uint64_t cur, ProjectionOperator *node)
{
    HASH_QO();
    HASH_NODE(projExprs);

    HASH_RETURN();
}


static uint64_t
hashJoinOperator (uint64_t cur, JoinOperator *node)
{
    HASH_QO();
    HASH_INT(joinType);
    HASH_NODE(cond);

    HASH_RETURN();
}


static uint64_t
hashAggregationOperator (uint64_t cur, AggregationOperator *node)
{
    HASH_QO();
    HASH_NODE(aggrs);
    HASH_NODE(groupBy);

    HASH_RETURN();
}


static uint64_t
hashProvenanceComputation (uint64_t cur, ProvenanceComputation *node)
{
    HASH_QO();
    HASH_INT(provType);
    HASH_INT(inputType);
    HASH_NODE(transactionInfo);
    HASH_NODE(asOf);

    HASH_RETURN();
}


static uint64_t
hashTableAccessOperator (uint64_t cur, TableAccessOperator *node)
{
    HASH_QO();
    HASH_STRING(tableName);
    HASH_NODE(asOf);

    HASH_RETURN();
}


static uint64_t
hashSetOperator (uint64_t cur, SetOperator *node)
{
    HASH_QO();
    HASH_INT(setOpType);

    HASH_RETURN();
}


static uint64_t
hashDuplicateRemoval (uint64_t cur, DuplicateRemoval *node)
{
    HASH_QO();
    HASH_NODE(attrs);

    HASH_RETURN();
}


static uint64_t
hashConstRelOperator (uint64_t cur, ConstRelOperator *node)
{
    HASH_QO();
    HASH_NODE(values);

    HASH_RETURN();
}


static uint64_t
hashNestingOperator (uint64_t cur, NestingOperator *node)
{
    HASH_QO();
    HASH_INT(nestingType);
    HASH_NODE(cond);

    HASH_RETURN();
}


static uint64_t
hashWindowOperator (uint64_t cur, WindowOperator *node)
{
    HASH_QO();
    HASH_NODE(partitionBy);
    HASH_NODE(orderBy);
    HASH_NODE(frameDef);
    HASH_STRING(attrName);
    HASH_NODE(f);

    HASH_RETURN();
}


static uint64_t
hashOrderOperator (uint64_t cur, OrderOperator *node)
{
    HASH_QO();
    HASH_NODE(orderExprs);

    HASH_RETURN();
}

static uint64_t
hashDLNode (uint64_t cur, DLNode *node)
{
    HASH_NODE(properties);
    HASH_RETURN();
}

#define HASH_DL() hashDLNode (cur, (DLNode *) node);

static uint64_t
hashDLAtom (uint64_t cur, DLAtom *node)
{
    HASH_DL();
    HASH_STRING(rel);
    HASH_NODE(args);
    HASH_BOOLEAN(negated);

    HASH_RETURN();
}


static uint64_t
hashDLVar (uint64_t cur, DLVar *node)
{
    HASH_DL();
    HASH_STRING(name);
    HASH_INT(dt);

    HASH_RETURN();
}


static uint64_t
hashDLRule (uint64_t cur, DLRule *node)
{
    HASH_DL();
    HASH_NODE(head);
    HASH_NODE(body);

    HASH_RETURN();
}


static uint64_t
hashDLProgram (uint64_t cur, DLProgram *node)
{
    HASH_DL();
    HASH_NODE(rules);
    HASH_NODE(facts);
    HASH_STRING(ans);

    HASH_RETURN();
}


static uint64_t
hashDLComparison (uint64_t cur, DLComparison *node)
{
    HASH_DL();
    HASH_NODE(opExpr);

    HASH_RETURN();
}




/* generic hash function */
static uint64_t
hashValueInternal(uint64_t h, void *a)
{
    Node *n;

    if (a == NULL)
        return h;

    n = (Node *) a;

    // hash node type
    h = hashInt(h, n->type);

    switch(n->type)
    {
        case T_List:
            return hashList(h,(List *) n);
        /* expression nodes */
        case T_Set:
            return hashSet(h, (Set *) n);
        case T_HashMap:
            return hashHashMap(h, (HashMap *) n);
        case T_Vector:
            return hashVector(h, (Vector *) n);
        case T_KeyValue:
            return hashKeyValue(h, (KeyValue *) n);
        case T_Constant:
            return hashConstant(h,(Constant *) n);
        case T_AttributeReference:
            return hashAttributeReference (h, (AttributeReference *) n);
        case T_SQLParameter:
            return hashSQLParameter (h, (SQLParameter *) n);
        case T_CaseExpr:
            return hashCaseExpr (h, (CaseExpr *) n);
        case T_CaseWhen:
            return hashCaseWhen (h, (CaseWhen *) n);
        case T_IsNullExpr:
            return hashIsNullExpr (h, (IsNullExpr *) n);
        case T_WindowBound:
            return hashWindowBound (h, (WindowBound *) n);
        case T_WindowFrame:
            return hashWindowFrame (h, (WindowFrame *) n);
        case T_WindowDef:
            return hashWindowDef (h, (WindowDef *) n);
        case T_WindowFunction:
            return hashWindowFunction (h, (WindowFunction *) n);
        case T_Operator:
            return hashOperator(h, (Operator *) n);
        case T_FunctionCall:
            return hashFunctionCall(h, (FunctionCall *) n);
        case T_RowNumExpr:
            return hashRowNumExpr(h, (RowNumExpr *) n);
        case T_OrderExpr:
            return hashOrderExpr(h, (OrderExpr *) n);
        case T_CastExpr:
            return hashCastExpr(h, (CastExpr *) n);
            /* query block nodes */
        case T_SetQuery:
            return hashSetQuery(h, (SetQuery *) n);
        case T_ProvenanceStmt:
            return hashProvenanceStmt(h, (ProvenanceStmt *) n);
        case T_ProvenanceTransactionInfo:
            return hashProvenanceTransactionInfo(h, (ProvenanceTransactionInfo *) n);
        case T_QueryBlock:
            return hashQueryBlock(h, (QueryBlock *) n);
        case T_SelectItem:
            return hashSelectItem(h, (SelectItem *) n);
        case T_FromProvInfo:
            return hashFromProvInfo(h, (FromProvInfo *) n);
        case T_FromTableRef:
            return hashFromTableRef(h, (FromTableRef *) n);
        case T_FromSubquery:
            return hashFromSubquery(h, (FromSubquery *) n);
        case T_FromJoinExpr:
            return hashFromJoinExpr(h, (FromJoinExpr *) n);
        case T_DistinctClause:
            return hashDistinctClause(h, (DistinctClause *) n);
        case T_NestedSubquery:
            return hashNestedSubquery(h, (NestedSubquery *) n);
        case T_Insert:
            return hashInsert(h, (Insert *) n);
        case T_Delete:
            return hashDelete(h, (Delete *) n);
        case T_Update:
            return hashUpdate(h, (Update *) n);
        case T_TransactionStmt:
            return hashTransactionStmt(h, (TransactionStmt *) n);
        case T_WithStmt:
            return hashWithStmt(h, (WithStmt *) n);
        case T_UtilityStatement:
            return hashUtilityStatement(h, (UtilityStatement *) n);
            /* query operator nodes */
        case T_Schema:
            return hashSchema(h, (Schema *) n);
        case T_AttributeDef:
            return hashAttributeDef(h, (AttributeDef *) n);
        case T_SelectionOperator:
            return hashSelectionOperator(h, (SelectionOperator *) n);
        case T_ProjectionOperator:
            return hashProjectionOperator(h, (ProjectionOperator *) n);
        case T_JoinOperator:
            return hashJoinOperator(h, (JoinOperator *) n);
        case T_AggregationOperator:
            return hashAggregationOperator(h, (AggregationOperator *) n);
        case T_ProvenanceComputation:
            return hashProvenanceComputation(h, (ProvenanceComputation *) n);
        case T_TableAccessOperator:
            return hashTableAccessOperator(h, (TableAccessOperator *) n);
        case T_SetOperator:
            return hashSetOperator(h, (SetOperator *) n);
        case T_DuplicateRemoval:
            return hashDuplicateRemoval(h, (DuplicateRemoval *) n);
        case T_ConstRelOperator:
            return hashConstRelOperator(h, (ConstRelOperator *) n);
        case T_NestingOperator:
            return hashNestingOperator(h, (NestingOperator *) n);
        case T_WindowOperator:
            return hashWindowOperator(h, (WindowOperator *) n);
        case T_OrderOperator:
            return hashOrderOperator(h, (OrderOperator *) n);
            /* datalog model */
        case T_DLAtom:
            return hashDLAtom(h, (DLAtom *) n);
        case T_DLVar:
            return hashDLVar(h, (DLVar *) n);
        case T_DLRule:
            return hashDLRule(h, (DLRule *) n);
        case T_DLProgram:
            return hashDLProgram(h, (DLProgram *) n);
        case T_DLComparison:
            return hashDLComparison(h, (DLComparison *) n);
        default:
            FATAL_LOG("unknown node type");
            break;
    }

    return h;
}

uint64_t
hashValue(void *a)
{
    uint64_t h = FNV_OFFSET;
    h = hashValueInternal(h, a);
//    printf("hash value of %p is: %llu: short %u\n", a, h, (unsigned) h);

    return h;
}
