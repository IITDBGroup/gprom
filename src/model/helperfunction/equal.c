/*************************************
 *         equal.c
 *    Author: Hao Guo
 *    One-line description
 *
 *
 *
 **************************************/

#include <string.h>

#include "common.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "log/logger.h"

/* equal functions for specific node types */
static boolean equalFunctionCall(FunctionCall *a, FunctionCall *b);
static boolean equalAttributeReference (AttributeReference *a,
        AttributeReference *b);
static boolean equalOperator (Operator *a, Operator *b);
static boolean equalConstant (Constant *a, Constant *b);
static boolean equalList(List *a, List *b);
static boolean equalStringList (List *a, List *b);
static boolean equalSet (Set *a, Set *b);

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
static boolean equalProvenanceComputation( ProvenanceComputation *a,  ProvenanceComputation *b);

// equal functions for query_block
static boolean equalQueryBlock(QueryBlock *a, QueryBlock *b);
static boolean equalSetQuery(SetQuery *a, SetQuery *b);
static boolean equalNestedSubquery (NestedSubquery *a, NestedSubquery *b);
static boolean equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b);
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

/*use these macros to compare fields */

/*compare one simple scalar field(int, boolean, float, etc)*/
#define COMPARE_SCALAR_FIELD(fldname)  \
		do{  \
			if (a->fldname != b->fldname)  \
			return FALSE;  \
		} while (0)

/*compare a field pointer to Node tree*/
#define COMPARE_NODE_FIELD(fldname)  \
		do{  \
			if(!equal(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/*compare a field pointer to a string list*/
#define COMPARE_STRING_LIST_FIELD(fldname)  \
        do{  \
            if(!equalStringList(a->fldname, b->fldname))  \
            return FALSE;  \
        } while (0)

/*compare a field that is a pointer to a C string or maybe NULL*/
#define COMPARE_STRING_FIELD(fldname)  \
		do{  \
			if(!equalstr(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/*compare a string field that maybe NULL*/
#define equalstr(a, b)  \
		(((a) != NULL && (b) != NULL) ? (strcmp(a, b) == 0) : (a) == (b))

/* */
static boolean
equalAttributeReference (AttributeReference *a,
        AttributeReference *b)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(fromClauseItem);
    COMPARE_SCALAR_FIELD(attrPosition);
    COMPARE_SCALAR_FIELD(outerLevelsUp);

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
    }

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
    assert(LIST_LENGTH(a) > 0 && LIST_LENGTH(b) > 0);

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

static boolean
equalStringList (List *a, List *b)
{
    if (a == NULL && b == NULL)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length
    assert(LIST_LENGTH(a) > 0 && LIST_LENGTH(b) > 0);

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
    //COMPARE_NODE_FIELD(parents);
    COMPARE_NODE_FIELD(provAttrs);

    return TRUE;
}

static boolean
equalTableAccessOperator(TableAccessOperator *a, TableAccessOperator *b)
{
    COMPARE_STRING_FIELD(tableName);
 
    return TRUE;
}

static boolean 
equalSelectionOperator(SelectionOperator *a, SelectionOperator *b)
{
    COMPARE_NODE_FIELD(cond);
    
    return TRUE;
}

static boolean 
equalProjectionOperator(ProjectionOperator *a, ProjectionOperator *b)
{
    COMPARE_NODE_FIELD(projExprs);
   
    return TRUE;
}

static boolean 
equalJoinOperator(JoinOperator *a, JoinOperator *b)
{
    COMPARE_SCALAR_FIELD(joinType);
    COMPARE_NODE_FIELD(cond);
   
    return TRUE;
}

static boolean 
equalAggregationOperator(AggregationOperator *a, AggregationOperator *b)
{
    COMPARE_NODE_FIELD(aggrs);
    COMPARE_NODE_FIELD(groupBy);
   
    return TRUE;
}

static boolean 
equalSetOperator(SetOperator *a, SetOperator *b)
{
    COMPARE_SCALAR_FIELD(setOpType);
  
    return TRUE;
}

static boolean 
equalDuplicateRemoval(DuplicateRemoval *a, DuplicateRemoval *b)
{
    COMPARE_NODE_FIELD(attrs);
  
    return TRUE;
}

static boolean 
equalProvenanceComputation( ProvenanceComputation *a,  ProvenanceComputation *b)
{
    COMPARE_SCALAR_FIELD(provType);
  
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
    COMPARE_STRING_LIST_FIELD(userProvAttrs);

    return TRUE;
}

static boolean 
equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b)
{
    COMPARE_NODE_FIELD(query);
    COMPARE_NODE_FIELD(selectClause);
    COMPARE_SCALAR_FIELD(provType);

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
    COMPARE_NODE_FIELD(cond);
  
    return TRUE;
}

static boolean 
equalDistinctClause(DistinctClause *a,  DistinctClause *b)
{
    COMPARE_NODE_FIELD(distinctExprs);
    
    return TRUE;
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
        case T_FunctionCall:
            retval = equalFunctionCall(a,b);
            break;
        case T_AttributeReference:
            retval = equalAttributeReference(a,b);
            break;
        case T_Operator:
            retval = equalOperator(a,b);
            break;
        case T_Constant:
            retval = equalConstant(a,b);
            break;
            /*something different cases this, and we have*/
            /*different types of T_Node       */
        case T_QueryOperator:
            retval = equalQueryOperator(a,b);
            break;
        case T_Schema:
            retval = equalSchema(a,b);
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
        default:
            retval = FALSE;
            break;
    }

    if (!retval)
        TRACE_LOG("not equals \n%s\n\n%s", nodeToString(a), nodeToString(b));

    return retval;
}
