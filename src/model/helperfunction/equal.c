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
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"


/* equal functions for specific node types */
static boolean equalFunctionCall(FunctionCall *a, FunctionCall *b);
static boolean equalAttributeReference (AttributeReference *a,
        AttributeReference *b);
static boolean equalList(List *a, List *b);
// equal functions for query_operator
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
static boolean equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b);
static boolean equalSelectItem(SelectItem *a, SelectItem *b);
static boolean equalFromItem(FromItem *a, FromItem *b);
static boolean equalFromTableRef(FromTableRef *a, FromTableRef *b);
static boolean equalFromSubquery(FromSubquery *a, FromSubquery *b);
static boolean equalFromJoinExpr(FromJoinExpr *a, FromJoinExpr *b);
static boolean equalDistinctClause(DistinctClause *a,  DistinctClause *b);


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

    return TRUE;
}

/* */
static boolean
equalFunctionCall(FunctionCall *a, FunctionCall *b)
{
    COMPARE_NODE_FIELD(functionname);
    COMPARE_NODE_FIELD(args);

    return TRUE;
}

/*equal list fun */
static boolean
equalList(List *a, List *b)
{
    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length

    switch(a->type)
    {
        case T_List:
        {
            FORBOTH(Node,l,r,a,b)
            {
                if (!equal(a,b))
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
equalQueryOperator(QueryOperator *a, QueryOperator *b)
{
    COMPARE_NODE_FIELD(inputs);
    COMPARE_NODE_FIELD(schema);
    COMPARE_NODE_FIELD(parents);
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
    COMPARE_NODE_FIELD(havingClause);
  
    return TRUE;
}

static boolean 
equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b)
{
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
    COMPARE_NODE_FIELD(attrNames);
    
    return TRUE;
}

static boolean 
equalFromTableRef(FromTableRef *a, FromTableRef *b)
{
    COMPARE_SCALAR_FIELD(from);
    COMPARE_STRING_FIELD(tableId);
    
    return TRUE;
}

static boolean 
equalFromSubquery(FromSubquery *a, FromSubquery *b)
{
    COMPARE_SCALAR_FIELD(from);
    COMPARE_NODE_FIELD(subquery);
   
    return TRUE;
}

static boolean 
equalFromJoinExpr(FromJoinExpr *a, FromJoinExpr *b)
{
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

    switch(nodeTag(a))
    {
        case T_List:
        case T_IntList:
            retval = equalList(a,b);
            break;
        case T_FunctionCall:
            retval = equalFunctionCall(a,b);
            break;
        case T_AttributeReference:
            retval = equalAttributeReference(a,b);
            break;
            /*something different cases this, and we have*/
            /*different types of T_Node       */
        case T_QueryOperator:
            retval = equalQueryOperator(a,b);
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
        case T_ProvenanceStmt:
            retval = ProvenanceStmt(a,b);
            break;
        case T_SelectItem:
            retval = equalSelectItem(a,b);
            break;
        case T_FromItem:
            retval = equalFromItem(a,b);
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
        default:
            retval = FALSE;
            break;
    }

    return retval;
}
