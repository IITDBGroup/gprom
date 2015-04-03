/*************************************
 *         equal.c
 *    Author: Hao Guo
 *    One-line description
 *
 *
 *
 **************************************/

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/vector.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/datalog/datalog_model.h"
#include "log/logger.h"

/* equal function for collection types */
static boolean equalList(List *a, List *b);
static boolean equalSet (Set *a, Set *b);
static boolean equalKeyValue (KeyValue *a, KeyValue *b);
static boolean equalHashMap (HashMap *a, HashMap *b);
static boolean equalVector (Vector *a, Vector *b);

/* equal functions for expression types */
static boolean equalFunctionCall(FunctionCall *a, FunctionCall *b);
static boolean equalAttributeReference (AttributeReference *a,
        AttributeReference *b);
static boolean equalSQLParameter (SQLParameter *a, SQLParameter* b);
static boolean equalOperator (Operator *a, Operator *b);
static boolean equalConstant (Constant *a, Constant *b);
static boolean equalCaseExpr (CaseExpr *a, CaseExpr *b);
static boolean equalCaseWhen (CaseWhen *a, CaseWhen *b);
static boolean equalIsNullExpr (IsNullExpr *a, IsNullExpr *b);
static boolean equalWindowBound (WindowBound *a, WindowBound *b);
static boolean equalWindowFrame (WindowFrame *a, WindowFrame *b);
static boolean equalWindowDef (WindowDef *a, WindowDef *b);
static boolean equalWindowFunction (WindowFunction *a, WindowFunction *b);
static boolean equalRowNumExpr (RowNumExpr *a, RowNumExpr *b);
static boolean equalOrderExpr (OrderExpr *a, OrderExpr *b);

// equal functions for query_operator
static boolean equalSchema(Schema *a, Schema *b);
//static boolean equalSchemaFromLists(Schema *a, Schema *b);
static boolean equalAttributeDef(AttributeDef *a, AttributeDef *b);
static boolean equalQueryOperator(QueryOperator *a, QueryOperator *b);
static boolean equalTableAccessOperator(TableAccessOperator *a, TableAccessOperator *b);
static boolean equalSelectionOperator(SelectionOperator *a, SelectionOperator *b);
static boolean equalProjectionOperator(ProjectionOperator *a, ProjectionOperator *b);
static boolean equalJoinOperator(JoinOperator *a, JoinOperator *b);
static boolean equalAggregationOperator(AggregationOperator *a, AggregationOperator *b);
static boolean equalSetOperator(SetOperator *a, SetOperator *b);
static boolean equalDuplicateRemoval(DuplicateRemoval *a, DuplicateRemoval *b);
static boolean equalProvenanceComputation(ProvenanceComputation *a, ProvenanceComputation *b);
static boolean equalConstRelOperator(ConstRelOperator *a, ConstRelOperator *b);
static boolean equalNestingOperator(NestingOperator *a, NestingOperator *b);
static boolean equalWindowOperator(WindowOperator *a, WindowOperator *b);
static boolean equalOrderOperator(OrderOperator *a, OrderOperator *b);

// Json
static boolean equalFromJsonTable(FromJsonTable *a, FromJsonTable *b);
static boolean equalJsonColInfoItem(JsonColInfoItem *a, JsonColInfoItem *b);
static boolean equalJsonTableOperator(JsonTableOperator *a, JsonTableOperator *b);
static boolean equalJsonPath(JsonPath *a, JsonPath *b);

// equal functions for query_block
static boolean equalQueryBlock(QueryBlock *a, QueryBlock *b);
static boolean equalSetQuery(SetQuery *a, SetQuery *b);
static boolean equalNestedSubquery (NestedSubquery *a, NestedSubquery *b);
static boolean equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b);
static boolean equalProvenanceTransactionInfo(ProvenanceTransactionInfo *a,
        ProvenanceTransactionInfo *b);
static boolean equalWithStmt(WithStmt *a, WithStmt *b);
static boolean equalSelectItem(SelectItem *a, SelectItem *b);
static boolean equalFromItem(FromItem *a, FromItem *b);
static boolean equalFromTableRef(FromTableRef *a, FromTableRef *b);
static boolean equalFromSubquery(FromSubquery *a, FromSubquery *b);
static boolean equalFromJoinExpr(FromJoinExpr *a, FromJoinExpr *b);
static boolean equalDistinctClause(DistinctClause *a,  DistinctClause *b);
static boolean equalInsert(Insert *a, Insert *b);
static boolean equalDelete(Delete *a, Delete *b);
static boolean equalUpdate(Update *a, Update *b);
static boolean equalTransactionStmt(TransactionStmt *a, TransactionStmt *b);
static boolean equalFromProvInfo (FromProvInfo *a, FromProvInfo *b);

// equal functions for datalog model
static boolean equalDLAtom (DLAtom *a, DLAtom *b);
static boolean equalDLVar (DLVar *a, DLVar *b);
static boolean equalDLRule (DLRule *a, DLRule *b);
static boolean equalDLProgram (DLProgram *a, DLProgram *b);
static boolean equalDLComparison (DLComparison *a, DLComparison *b);

/* use these macros to compare fields */

/* compare one simple scalar field(int, boolean, float, etc)*/
#define COMPARE_SCALAR_FIELD(fldname)  \
		do{  \
			if (a->fldname != b->fldname)  \
			return FALSE;  \
		} while (0)

/* compare a field pointer to Node tree*/
#define COMPARE_NODE_FIELD(fldname)  \
		do{  \
			if(!equal(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/* compare a field pointer to a string list*/
#define COMPARE_STRING_LIST_FIELD(fldname)  \
        do{  \
            if(!equalStringList((List *) a->fldname, (List *) b->fldname))  \
            return FALSE;  \
        } while (0)

/* compare a field that is a pointer to a C string or maybe NULL*/
#define COMPARE_STRING_FIELD(fldname)  \
		do{  \
			if(!equalstr(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/* compare the common query operator fields */
#define COMPARE_QUERY_OP() \
		do { \
			if(!equalQueryOperator((QueryOperator *) &a->op, (QueryOperator *) &b->op)) \
			return FALSE; \
		} while (0)

/*compare a string field that maybe NULL*/
#define equalstr(a, b)  \
		(((a) != NULL && (b) != NULL) ? (strcmp(a, b) == 0) : (a) == (b))

/* datalog model comparisons */
static boolean
equalDLAtom (DLAtom *a, DLAtom *b)
{
    COMPARE_STRING_FIELD(rel);
    COMPARE_NODE_FIELD(args);
    COMPARE_SCALAR_FIELD(negated);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLVar (DLVar *a, DLVar *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(dt);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLRule (DLRule *a, DLRule *b)
{
    COMPARE_NODE_FIELD(head);
    COMPARE_NODE_FIELD(body);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLProgram (DLProgram *a, DLProgram *b)
{
    COMPARE_NODE_FIELD(rules);
    COMPARE_NODE_FIELD(facts);
    COMPARE_STRING_FIELD(ans);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLComparison (DLComparison *a, DLComparison *b)
{
    COMPARE_NODE_FIELD(opExpr);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}


/* */
static boolean
equalAttributeReference (AttributeReference *a,
        AttributeReference *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(fromClauseItem);
    COMPARE_SCALAR_FIELD(attrPosition);
    COMPARE_SCALAR_FIELD(outerLevelsUp);
    COMPARE_SCALAR_FIELD(attrType);

    return TRUE;
}

static boolean
equalSQLParameter (SQLParameter *a, SQLParameter *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(position);
    COMPARE_SCALAR_FIELD(parType);

    return TRUE;
}

static boolean
equalOperator (Operator *a, Operator *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_NODE_FIELD(args);

    return TRUE;
}

static boolean
equalConstant (Constant *a, Constant *b)
{
    COMPARE_SCALAR_FIELD(constType);

    // if both are NULL they are considered equal
    if (a->isNull && b->isNull)
        return TRUE;

    // only one of them is NULL return FALSE
    if (a->isNull || b->isNull)
        return FALSE;

    switch(a->constType)
    {
        case DT_INT:
            return INT_VALUE(a) == INT_VALUE(b);
        case DT_FLOAT:
            return FLOAT_VALUE(a) == FLOAT_VALUE(b);
        case DT_BOOL:
            return BOOL_VALUE(a) == BOOL_VALUE(b);
        case DT_STRING:
            return strcmp(STRING_VALUE(a), STRING_VALUE(b)) == 0;
        case DT_LONG:
            return LONG_VALUE(a) == LONG_VALUE(b);
        case DT_VARCHAR2:
	    return strcmp(STRING_VALUE(a), STRING_VALUE(b)) == 0;
    }

    COMPARE_SCALAR_FIELD(isNull);

    return TRUE;
}

static boolean
equalCaseExpr (CaseExpr *a, CaseExpr *b)
{
    COMPARE_NODE_FIELD(expr);
    COMPARE_NODE_FIELD(whenClauses);
    COMPARE_NODE_FIELD(elseRes);

    return TRUE;
}

static boolean
equalCaseWhen (CaseWhen *a, CaseWhen *b)
{
    COMPARE_NODE_FIELD(when);
    COMPARE_NODE_FIELD(then);

    return TRUE;
}

static boolean
equalIsNullExpr (IsNullExpr *a, IsNullExpr *b)
{
    COMPARE_NODE_FIELD(expr);

    return TRUE;
}

static boolean
equalWindowBound (WindowBound *a, WindowBound *b)
{
    COMPARE_SCALAR_FIELD(bType);
    COMPARE_NODE_FIELD(expr);

    return TRUE;
}

static boolean
equalWindowFrame (WindowFrame *a, WindowFrame *b)
{
    COMPARE_SCALAR_FIELD(frameType);
    COMPARE_NODE_FIELD(lower);
    COMPARE_NODE_FIELD(higher);

    return TRUE;
}

static boolean
equalWindowDef (WindowDef *a, WindowDef *b)
{
    COMPARE_NODE_FIELD(partitionBy);
    COMPARE_NODE_FIELD(orderBy);
    COMPARE_NODE_FIELD(frame);

    return TRUE;
}

static boolean
equalWindowFunction (WindowFunction *a, WindowFunction *b)
{
    COMPARE_NODE_FIELD(f);
    COMPARE_NODE_FIELD(win);

    return TRUE;
}

static boolean
equalRowNumExpr (RowNumExpr *a, RowNumExpr *b)
{
    return TRUE;
}

static boolean
equalOrderExpr (OrderExpr *a, OrderExpr *b)
{
    COMPARE_NODE_FIELD(expr);
    COMPARE_SCALAR_FIELD(order);
    COMPARE_SCALAR_FIELD(nullOrder);

    return TRUE;
}

/* */
static boolean
equalFunctionCall(FunctionCall *a, FunctionCall *b)
{
    COMPARE_STRING_FIELD(functionname);
    COMPARE_NODE_FIELD(args);
    COMPARE_SCALAR_FIELD(isAgg);

    return TRUE;
}

/*equal list fun */
static boolean
equalList(List *a, List *b)
{
    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length
    ASSERT(LIST_LENGTH(a) > 0 && LIST_LENGTH(b) > 0);

    switch(a->type)
    {
        case T_List:
        {
            FORBOTH(Node,l,r,a,b)
            {
                if (!equal(l,r))
                    return FALSE;
            }
        }
        break;
        case T_IntList:
        {
            FORBOTH_INT(i,j,a,b)
            {
                if (i != j)
                    return FALSE;
            }
        }
        break;
        default:
            return FALSE;
    }

    return TRUE;
}

boolean
equalStringList (List *a, List *b)
{
    if (a == NULL && b == NULL)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length
    ASSERT(LIST_LENGTH(a) > 0 && LIST_LENGTH(b) > 0);

    FORBOTH(char,s1,s2,a,b)
    {
        if (strcmp(s1,s2) != 0)
            return FALSE;
    }

    return TRUE;
}

static boolean
equalSet (Set *a, Set *b)
{
    if (a->setType != b->setType)
        return FALSE;
    if (setSize(a) != setSize(b))
        return FALSE;

    if (a->setType == SET_TYPE_INT)
    {
        FOREACH_SET(int,i,a)
        {
            if (!hasSetIntElem(b,*i))
                return FALSE;
        }

        FOREACH_SET(int,i,b)
        {
            if (!hasSetIntElem(a,*i))
                return FALSE;
        }

        return TRUE;
    }

    FOREACH_SET(void,el,a)
    {
        if (!hasSetElem(b,el))
            return FALSE;
    }
    FOREACH_SET(void,el,b)
    {
        if (!hasSetElem(a,el))
            return FALSE;
    }

    return TRUE;
}

static boolean
equalKeyValue (KeyValue *a, KeyValue *b)
{
    COMPARE_NODE_FIELD(key);
    COMPARE_NODE_FIELD(value);

    return TRUE;
}

static boolean
equalHashMap (HashMap *a, HashMap *b)
{
    COMPARE_SCALAR_FIELD(keyType);
    COMPARE_SCALAR_FIELD(valueType);

    FOREACH_HASH_KEY(Node,k,a)
        if(!hasMapKey(b,k) || !equal(getMap(a,k),getMap(b,k)))
            return FALSE;

    FOREACH_HASH_KEY(Node,k,b)
        if(!hasMapKey(a,k))
            return FALSE;

    return TRUE;
}

static boolean
equalVector (Vector *a, Vector *b)
{
    COMPARE_SCALAR_FIELD(elType);
    COMPARE_SCALAR_FIELD(length);
//    COMPARE_SCALAR_FIELD(elNodeType);

    switch(a->elType)
    {
        case VECTOR_INT:
        {
            int *aA, *bA;
            aA = VEC_TO_IA(a);
            bA = VEC_TO_IA(b);

            for(int i = 0; i < VEC_LENGTH(a); i++)
                if (aA[i] != bA[i])
                    return FALSE;
        }
        break;
        case VECTOR_STRING:
        {
            char **aA, **bA;
            aA = VEC_TO_ARR(a,char);
            bA = VEC_TO_ARR(b,char);

            for(int i = 0; i < VEC_LENGTH(a); i++)
                if (strcmp(aA[i],bA[i]) != 0)
                    return FALSE;
        }
        break;
        case VECTOR_NODE:
        {
            Node **aA, **bA;
            aA = VEC_TO_ARR(a,Node);
            bA = VEC_TO_ARR(b,Node);

            for(int i = 0; i < VEC_LENGTH(a); i++)
                if (equal(aA[i],bA[i]) != 0)
                    return FALSE;
        }
        break;
    }

    return TRUE;
}


static boolean
equalSchema(Schema *a, Schema *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_NODE_FIELD(attrDefs);
   
    return TRUE;
}

//static boolean
//equalSchemaFromLists(Schema *a, Schema *b)
//{
//    COMPARE_STRING_FIELD(name);
//    COMPARE_NODE_FIELD(attrNames);
//    COMPARE_NODE_FIELD(dataTypes);
//
//    return TRUE;
//}

static boolean
equalAttributeDef(AttributeDef *a, AttributeDef *b)
{
    COMPARE_STRING_FIELD(attrName);
    COMPARE_SCALAR_FIELD(pos);
   
    return TRUE;
}

static boolean
equalQueryOperator(QueryOperator *a, QueryOperator *b)
{
    COMPARE_NODE_FIELD(inputs);
    COMPARE_NODE_FIELD(schema);
    //COMPARE_NODE_FIELD(parents); //TODO implement compare one node
    COMPARE_NODE_FIELD(provAttrs);
    COMPARE_NODE_FIELD(properties);

    return TRUE;
}

static boolean
equalTableAccessOperator(TableAccessOperator *a, TableAccessOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_STRING_FIELD(tableName);
    COMPARE_NODE_FIELD(asOf);

    return TRUE;
}

static boolean 
equalSelectionOperator(SelectionOperator *a, SelectionOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(cond);
    
    return TRUE;
}

static boolean 
equalProjectionOperator(ProjectionOperator *a, ProjectionOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(projExprs);
   
    return TRUE;
}

static boolean 
equalJoinOperator(JoinOperator *a, JoinOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(joinType);
    COMPARE_NODE_FIELD(cond);
   
    return TRUE;
}

static boolean 
equalAggregationOperator(AggregationOperator *a, AggregationOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(aggrs);
    COMPARE_NODE_FIELD(groupBy);
   
    return TRUE;
}

static boolean 
equalSetOperator(SetOperator *a, SetOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(setOpType);
  
    return TRUE;
}

static boolean 
equalDuplicateRemoval(DuplicateRemoval *a, DuplicateRemoval *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(attrs);
  
    return TRUE;
}

static boolean 
equalProvenanceComputation(ProvenanceComputation *a,  ProvenanceComputation *b)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(provType);
    COMPARE_SCALAR_FIELD(inputType);
    COMPARE_NODE_FIELD(transactionInfo);
    COMPARE_NODE_FIELD(asOf);

    return TRUE;
}

static boolean
equalConstRelOperator(ConstRelOperator *a, ConstRelOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(values);

    return TRUE;
}

static boolean
equalNestingOperator(NestingOperator *a, NestingOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(nestingType);
    COMPARE_NODE_FIELD(cond);

    return TRUE;
}

static boolean
equalWindowOperator (WindowOperator *a, WindowOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(partitionBy);
    COMPARE_NODE_FIELD(orderBy);
    COMPARE_NODE_FIELD(frameDef);
    COMPARE_STRING_FIELD(attrName);
    COMPARE_NODE_FIELD(f);

    return TRUE;
}

static boolean
equalOrderOperator(OrderOperator *a, OrderOperator *b)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(orderExprs);

    return TRUE;
}


static boolean
equalFromJsonTable(FromJsonTable *a, FromJsonTable *b)
{
	COMPARE_NODE_FIELD(columns);
    COMPARE_STRING_FIELD(documentcontext);
    COMPARE_NODE_FIELD(jsonColumn);
    COMPARE_STRING_FIELD(jsonTableIdentifier);

    return TRUE;
}

static boolean
equalJsonColInfoItem(JsonColInfoItem *a, JsonColInfoItem *b)
{
    COMPARE_STRING_FIELD(attrName);
    COMPARE_STRING_FIELD(path);
    COMPARE_STRING_FIELD(attrType);

    COMPARE_STRING_FIELD(format);
    COMPARE_STRING_FIELD(wrapper);
    COMPARE_NODE_FIELD(nested);
    COMPARE_STRING_FIELD(forOrdinality);

    return TRUE;
}

static boolean
equalJsonTableOperator(JsonTableOperator *a, JsonTableOperator *b)
{
    COMPARE_QUERY_OP();

    COMPARE_NODE_FIELD(columns);
    COMPARE_STRING_FIELD(documentcontext);
    COMPARE_NODE_FIELD(jsonColumn);
    COMPARE_STRING_FIELD(jsonTableIdentifier);

    return TRUE;
}

static boolean
equalJsonPath(JsonPath *a, JsonPath *b)
{
    COMPARE_STRING_FIELD(path);

    return TRUE;
}

// equal functions for query_block
static boolean 
equalQueryBlock(QueryBlock *a, QueryBlock *b)
{
    COMPARE_NODE_FIELD(selectClause);
    COMPARE_NODE_FIELD(distinct);
    COMPARE_NODE_FIELD(fromClause);
    COMPARE_NODE_FIELD(whereClause);
    COMPARE_NODE_FIELD(groupByClause);
    COMPARE_NODE_FIELD(havingClause);
    COMPARE_NODE_FIELD(orderByClause);
    COMPARE_NODE_FIELD(limitClause);

    return TRUE;
}

static boolean
equalSetQuery(SetQuery *a, SetQuery *b)
{
    COMPARE_SCALAR_FIELD(setOp);
    COMPARE_SCALAR_FIELD(all);
    COMPARE_STRING_LIST_FIELD(selectClause);
    COMPARE_NODE_FIELD(lChild);
    COMPARE_NODE_FIELD(rChild);

    return TRUE;
}

static boolean
equalNestedSubquery (NestedSubquery *a, NestedSubquery *b)
{
    COMPARE_SCALAR_FIELD(nestingType);
    COMPARE_NODE_FIELD(expr);
    COMPARE_STRING_FIELD(comparisonOp);
    COMPARE_NODE_FIELD(query);

    return TRUE;
}


static boolean 
equalInsert(Insert *a, Insert *b)
{
    COMPARE_STRING_FIELD(tableName);
    COMPARE_NODE_FIELD(query);
   
    return TRUE;

}

static boolean 
equalDelete(Delete *a, Delete *b)
{
    COMPARE_STRING_FIELD(nodeName);
    COMPARE_NODE_FIELD(cond);
   
    return TRUE;

}

static boolean 
equalUpdate(Update *a, Update *b)
{
    COMPARE_STRING_FIELD(nodeName);
    COMPARE_NODE_FIELD(selectClause);
    COMPARE_NODE_FIELD(cond);
   
    return TRUE;

}

static boolean
equalTransactionStmt(TransactionStmt *a, TransactionStmt *b)
{
    COMPARE_SCALAR_FIELD(stmtType);

    return TRUE;
}

static boolean
equalFromProvInfo (FromProvInfo *a, FromProvInfo *b)
{
    COMPARE_SCALAR_FIELD(baserel);
    COMPARE_SCALAR_FIELD(intermediateProv);
    COMPARE_STRING_LIST_FIELD(userProvAttrs);

    return TRUE;
}

static boolean 
equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b)
{
    COMPARE_NODE_FIELD(query);
    COMPARE_NODE_FIELD(selectClause);
    COMPARE_SCALAR_FIELD(provType);
    COMPARE_SCALAR_FIELD(inputType);
    COMPARE_NODE_FIELD(transInfo);
    COMPARE_NODE_FIELD(asOf);
    COMPARE_NODE_FIELD(options);

    return TRUE;
}

static boolean 
equalProvenanceTransactionInfo(ProvenanceTransactionInfo *a,
        ProvenanceTransactionInfo *b)
{
    COMPARE_SCALAR_FIELD(transIsolation);
    COMPARE_STRING_LIST_FIELD(updateTableNames);
    COMPARE_NODE_FIELD(originalUpdates);
    COMPARE_NODE_FIELD(scns);
    COMPARE_NODE_FIELD(commitSCN);


    return TRUE;
}

static boolean
equalWithStmt(WithStmt *a, WithStmt *b)
{
    COMPARE_NODE_FIELD(withViews);
    COMPARE_NODE_FIELD(query);

    return TRUE;
}

static boolean
equalSelectItem(SelectItem *a, SelectItem *b)
{
    COMPARE_STRING_FIELD(alias);
    COMPARE_NODE_FIELD(expr);
   
    return TRUE;
}

static boolean 
equalFromItem(FromItem *a, FromItem *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_STRING_LIST_FIELD(attrNames);
    COMPARE_NODE_FIELD(provInfo);
    COMPARE_NODE_FIELD(dataTypes);
    
    return TRUE;
}

static boolean 
equalFromTableRef(FromTableRef *a, FromTableRef *b)
{
    if (!equalFromItem((FromItem *) a, (FromItem *) b))
        return FALSE;
    COMPARE_STRING_FIELD(tableId);
    
    return TRUE;
}

static boolean 
equalFromSubquery(FromSubquery *a, FromSubquery *b)
{
    if (!equalFromItem((FromItem *) a, (FromItem *) b))
        return FALSE;
    COMPARE_NODE_FIELD(subquery);
   
    return TRUE;
}

static boolean 
equalFromJoinExpr(FromJoinExpr *a, FromJoinExpr *b)
{
    if (!equalFromItem((FromItem *) a, (FromItem *) b))
        return FALSE;
    COMPARE_NODE_FIELD(left);
    COMPARE_NODE_FIELD(right);
    COMPARE_SCALAR_FIELD(joinType);
    COMPARE_SCALAR_FIELD(joinCond);
    if (a->joinCond == JOIN_COND_USING)
        COMPARE_STRING_LIST_FIELD(cond);
    else
        COMPARE_NODE_FIELD(cond);
  
    return TRUE;
}

static boolean 
equalDistinctClause(DistinctClause *a,  DistinctClause *b)
{
    COMPARE_NODE_FIELD(distinctExprs);
    
    return TRUE;
}

/* returns true if two pointers point to the same memory location */
boolean
ptrEqual(void *a, void *b)
{
    return a == b;
}

/*equalfun returns  whether two nodes are equal*/
boolean
equal(void *a, void *b)
{
    boolean retval;
    if (a == b)
        return TRUE;

    if (a == NULL || b == NULL)
        return FALSE;

    if (nodeTag(a) !=nodeTag(b))
        return FALSE;

    TRACE_LOG("same node types \n<%s>\nand\n<%s>", nodeToString(a), nodeToString(b));

    switch(nodeTag(a))
    {
        case T_List:
        case T_IntList:
            retval = equalList(a,b);
            break;
        case T_Set:
            retval = equalSet(a,b);
            break;
        case T_HashMap:
            retval = equalHashMap(a,b);
            break;
        case T_Vector:
            retval = equalVector(a,b);
            break;

        case T_FunctionCall:
            retval = equalFunctionCall(a,b);
            break;
        case T_AttributeReference:
            retval = equalAttributeReference(a,b);
            break;
        case T_SQLParameter:
            retval = equalSQLParameter(a,b);
            break;
        case T_Operator:
            retval = equalOperator(a,b);
            break;
        case T_Constant:
            retval = equalConstant(a,b);
            break;
        case T_CaseExpr:
            retval = equalCaseExpr(a,b);
            break;
        case T_CaseWhen:
            retval = equalCaseWhen(a,b);
            break;
        case T_IsNullExpr:
            retval = equalIsNullExpr(a,b);
            break;
        case T_WindowBound:
            retval = equalWindowBound(a,b);
            break;
        case T_WindowFrame:
            retval = equalWindowFrame(a,b);
            break;
        case T_WindowDef:
            retval = equalWindowDef(a,b);
            break;
        case T_WindowFunction:
            retval = equalWindowFunction(a,b);
            break;
        case T_RowNumExpr:
            retval = equalRowNumExpr(a,b);
            break;
        case T_OrderExpr:
            retval = equalOrderExpr(a,b);
            break;
            /*something different cases this, and we have*/
            /*different types of T_Node       */
        case T_QueryOperator:
            retval = equalQueryOperator(a,b);
            break;
        case T_Schema:
            retval = equalSchema(a,b);
            break;
        case T_KeyValue:
            retval = equalKeyValue(a,b);
            break;
        case T_AttributeDef:
            retval = equalAttributeDef(a,b);
            break;
        case T_TableAccessOperator:
            retval = equalTableAccessOperator(a,b);
            break;
        case T_SelectionOperator:
            retval = equalSelectionOperator(a,b);
            break;
        case T_ProjectionOperator:
            retval = equalProjectionOperator(a,b);
            break;
        case T_JoinOperator:
            retval = equalJoinOperator(a,b);
            break;
        case T_AggregationOperator:
            retval = equalAggregationOperator(a,b);
            break;
        case T_SetOperator:
            retval = equalSetOperator(a,b);
            break;
        case T_DuplicateRemoval:
            retval = equalDuplicateRemoval(a,b);
            break;
        case T_ProvenanceComputation:
            retval = equalProvenanceComputation(a,b);
            break;
        case T_ConstRelOperator:
            retval = equalConstRelOperator(a,b);
            break;
        case T_QueryBlock:
            retval = equalQueryBlock(a,b);
            break;
        case T_SetQuery:
            retval = equalSetQuery(a,b);
            break;
        case T_NestedSubquery:
            retval = equalNestedSubquery(a,b);
            break;
        case T_ProvenanceStmt:
            retval = equalProvenanceStmt(a,b);
            break;
        case T_ProvenanceTransactionInfo:
            retval = equalProvenanceTransactionInfo(a,b);
            break;
        case T_WithStmt:
            retval = equalWithStmt(a,b);
            break;
        case T_SelectItem:
            retval = equalSelectItem(a,b);
            break;
        case T_FromItem:
            retval = equalFromItem(a,b);
            break;
        case T_FromProvInfo:
            retval = equalFromProvInfo(a,b);
            break;
        case T_FromTableRef:
            retval = equalFromTableRef(a,b);
            break;
        case T_FromSubquery:
            retval = equalFromSubquery(a,b);
            break;
        case T_FromJoinExpr:
            retval = equalFromJoinExpr(a,b);
            break;
        case T_DistinctClause:
            retval = equalDistinctClause(a,b);
            break;
        case T_Insert:
            retval = equalInsert(a,b);
            break;
        case T_Delete:
            retval = equalDelete(a,b);
            break;
        case T_Update:
            retval = equalUpdate(a,b);
            break;
        case T_TransactionStmt:
            retval = equalTransactionStmt(a,b);
            break;
        case T_NestingOperator:
            retval = equalNestingOperator(a,b);
            break;
        case T_WindowOperator:
            retval = equalWindowOperator(a,b);
            break;
        case T_OrderOperator:
            retval = equalOrderOperator(a,b);
            break;
        case T_FromJsonTable:
            retval = equalFromJsonTable(a,b);
            break;
        case T_JsonColInfoItem:
        	retval = equalJsonColInfoItem(a,b);
        	break;
        case T_JsonTableOperator:
        	retval = equalJsonTableOperator(a,b);
        	break;
        case T_JsonPath:
        	retval = equalJsonPath(a,b);
        	break;
        /* datalog model */
        case T_DLAtom:
            retval = equalDLAtom(a,b);
            break;
        case T_DLVar:
            retval = equalDLVar(a,b);
            break;
        case T_DLRule:
            retval = equalDLRule(a,b);
            break;
        case T_DLProgram:
            retval = equalDLProgram(a,b);
            break;
        case T_DLComparison:
            retval = equalDLComparison(a,b);
            break;
        default:
            retval = FALSE;
            break;
    }

//    printf("equals: %p, %p, %s\n%s -- %s\n", a, b, (a == b) ? "TRUE" : "FALSE", nodeToString(a), nodeToString(b));

    if (!retval)
        TRACE_LOG("not equals \n%s\n\n%s", nodeToString(a), nodeToString(b));

    return retval;
}
