/*************************************
 *         copy.c
 *    Author: Hao Guo
 *    copy function for tree nodes
 *
 *
 *
 **************************************/

#include <string.h>

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"


/* functions to copy specific node types */
static AttributeReference *copyAttributeReference(AttributeReference *from);
static List *deepCopyList(List *from);
static FunctionCall *copyFunctionCall(FunctionCall *from);
static Operator *copyOperator(Operator *from);

/*schema helper functions*/
static AttributeDef *copyAttributeDef(AttributeDef *from);
static Schema *copySchema(Schema *from);
static Schema *copySchemaFromList(Schema *from);

/*functions to copy query_operator*/

//static QueryOperator *copyQueryOperator(QueryOperator *from);
static TableAccessOperator *copyTableAccessOp(TableAccessOperator *from);
static SelectionOperator *copySelectionOp(SelectionOperator *from);
static ProjectionOperator *copyProjectionOp(ProjectionOperator *from);
static JoinOperator *copyJoinOp(JoinOperator *from); 
static AggregationOperator *copyAggregationOp(AggregationOperator *from);
static SetOperator *copySetOperator(SetOperator *from);
static DuplicateRemoval *copyDuplicateRemovalOp(DuplicateRemoval *from);
static ProvenanceComputation *copyProvenanceComputOp(ProvenanceComputation *from);

/*functions to copy query_block*/
static SetQuery *copySetQuery(SetQuery *from);
//static SetOp *copySetOp(SetOp *from);
static QueryBlock *copyQueryBlock(QueryBlock *from);
static Constant *copyConstant(Constant *from);
static ProvenanceStmt *copyProvenanceStmt(ProvenanceStmt *from);
static SelectItem *copySelectItem(SelectItem  *from);
static FromTableRef *copyFromTableRef(FromTableRef *from);
static FromSubquery *copyFromSubquery(FromSubquery *from);
static FromJoinExpr *copyFromJoinExpr(FromJoinExpr *from);
static DistinctClause *copyDistinctClause(DistinctClause *from);
static Insert *copyInsert(Insert *from);
static Delete *copyDelete(Delete *from);
static Update *copyUpdate(Update *from);

/*use the Macros(the varibles are 'new' and 'from')*/

/* creates a new pointer to a node and allocated mem */
#define COPY_INIT(type) \
		type *new; \
		new = makeNode(type);

/*copy a simple scalar field(int, bool, float, etc)*/
#define COPY_SCALAR_FIELD(fldname)  \
		(new->fldname = from->fldname)

/*copy a field that is a pointer to Node or Node tree*/
#define COPY_NODE_FIELD(fldname)  \
		(new->fldname) = (copyObject(from->fldname))

/*copy a field that is a pointer to C string or NULL*/
#define COPY_STRING_FIELD(fldname) \
                 new->fldname = (from->fldname !=NULL ? strdup(from->fldname) : NULL)

/*deep copy for List operation*/
static List *
deepCopyList(List *from)
{
    COPY_INIT(List);

    assert(getListLength(from) >= 1);
    COPY_SCALAR_FIELD(length);
    COPY_SCALAR_FIELD(type); // if it is an Int_List

    if (from->type == T_List)
    {
        FOREACH_INT(i, from)
            new = appendToTailOfListInt(new, i);
    }
    else
    {
        FOREACH(Node,n,from)
            new = appendToTailOfList(new, copyObject(n));
    }

    return new;
}

static AttributeReference *
copyAttributeReference(AttributeReference *from)
{
    COPY_INIT(AttributeReference);

    COPY_STRING_FIELD(name);

    return new;
}

static FunctionCall *
copyFunctionCall(FunctionCall *from)
{ 
    COPY_INIT(FunctionCall);
   
    COPY_STRING_FIELD(functionname);
    COPY_NODE_FIELD(args);
    COPY_SCALAR_FIELD(isAgg);

    return new;
}

static Operator *
copyOperator(Operator *from)
{ 
    COPY_INIT(Operator);
    
    COPY_STRING_FIELD(name);

    return new;
}

static AttributeDef *
copyAttributeDef(AttributeDef *from)
{
    COPY_INIT(AttributeDef);
    COPY_SCALAR_FIELD(dataType);
    COPY_STRING_FIELD(attrName);
    COPY_SCALAR_FIELD(pos);
    
    return new;
}
static Schema *
copySchema(Schema *from)
{
    COPY_INIT(Schema);
    COPY_STRING_FIELD(name);
    COPY_NODE_FIELD(attrDefs);
    
    return new;
}
//static Schema *
//copySchemaFromList(Schema *from)
//{
//    COPY_INIT(Schema);
//    COPY_STRING_FIELD(name);
//    COPY_NODE_FIELD(attrNames);
//    COPY_NODE_FIELD(dataTypes);
//
//    return new;
//}

/*functions to copy query_operator*/
//static QueryOperator *
//copyQueryOperator(QueryOperator *from)
//{
//    COPY_INIT(QueryOperator);
//    COPY_NODE_FIELD(inputs);
//    COPY_NODE_FIELD(schema);
//    COPY_NODE_FIELD(parents);
//    COPY_NODE_FIELD(provAttrs);
//
//    return new;
//}

static TableAccessOperator *
copyTableAccessOp(TableAccessOperator *from)
{
    COPY_INIT(TableAccessOperator);
    COPY_STRING_FIELD(tableName);

    return new;
}

static SelectionOperator *
copySelectionOp(SelectionOperator *from)
{
    COPY_INIT(SelectionOperator);
    COPY_NODE_FIELD(cond);

    return new;
}

static ProjectionOperator *
copyProjectionOp(ProjectionOperator *from)
{
    COPY_INIT(ProjectionOperator);

    COPY_NODE_FIELD(projExprs);

    return new;
}
static JoinOperator *
copyJoinOp(JoinOperator *from)
{
    COPY_INIT(JoinOperator);
    COPY_SCALAR_FIELD(joinType);
    COPY_NODE_FIELD(cond);

    return new;
}
static AggregationOperator *
copyAggregationOp(AggregationOperator *from)
{
    COPY_INIT(AggregationOperator);
    COPY_NODE_FIELD(aggrs);
    COPY_NODE_FIELD(groupBy);

    return new;
}
static SetOperator *
copySetOperator(SetOperator *from)
{
    COPY_INIT(SetOperator);
    COPY_SCALAR_FIELD(setOpType);

    return new;
}
static DuplicateRemoval *
copyDuplicateRemovalOp(DuplicateRemoval *from)
{
    COPY_INIT(DuplicateRemoval);
    COPY_NODE_FIELD(attrs);

    return new;
}
static ProvenanceComputation *
copyProvenanceComputOp(ProvenanceComputation *from)
{
    COPY_INIT(ProvenanceComputation);
    COPY_SCALAR_FIELD(provType);

    return new;
}
/*functions to copy query_block*/
/*
static SetQuery *
copySetQuery(SetQuery *from)
{
    COPY_INIT(SetQuery);
    COPY_NODE_FIELD(selectClause);
    COPY_SCALAR_FIELD(rootSetOp);

    return new;
}*/

static SetQuery *
copySetQuery(SetQuery *from)
{
    COPY_INIT(SetQuery);
    COPY_SCALAR_FIELD(setOp);
    COPY_SCALAR_FIELD(all);
    COPY_NODE_FIELD(selectClause);
    COPY_NODE_FIELD(lChild);
    COPY_NODE_FIELD(rChild);

    return new;
}
static QueryBlock *
copyQueryBlock(QueryBlock *from)
{
    COPY_INIT(QueryBlock);
    COPY_NODE_FIELD(selectClause);
    COPY_NODE_FIELD(distinct);
    COPY_NODE_FIELD(fromClause);
    COPY_NODE_FIELD(whereClause);
    COPY_NODE_FIELD(groupByClause);
    COPY_NODE_FIELD(havingClause);
    COPY_NODE_FIELD(orderByClause);
    COPY_NODE_FIELD(limitClause);
    
    return new;
}
static Insert *
copyInsert(Insert *from)
{
    COPY_INIT(Insert);
    COPY_STRING_FIELD(nodeName);
    COPY_NODE_FIELD(query);

    return new;
}
static Delete *
copyDelete(Delete *from)
{
    COPY_INIT(Delete);
    COPY_STRING_FIELD(nodeName);
    COPY_NODE_FIELD(cond);

    return new;
}
static Update *
copyUpdate(Update *from)
{
    COPY_INIT(Update);
    COPY_STRING_FIELD(nodeName);
    COPY_NODE_FIELD(selectClause);
    COPY_NODE_FIELD(cond);

    return new;
}
static ProvenanceStmt *
copyProvenanceStmt(ProvenanceStmt *from)
{
    COPY_INIT(ProvenanceStmt);
    COPY_NODE_FIELD(query);

    return new;
}
static SelectItem *
copySelectItem(SelectItem  *from)
{
    COPY_INIT(SelectItem);
    COPY_STRING_FIELD(alias);
    COPY_NODE_FIELD(expr);

    return new;
}
static FromTableRef *
copyFromTableRef(FromTableRef *from)
{
    COPY_INIT(FromTableRef);
    COPY_STRING_FIELD(tableId);

    return new;
}

static Constant *
copyConstant(Constant *from)
{
      COPY_INIT(Constant);
      COPY_SCALAR_FIELD(constType); 
	   
      switch (from->constType)	 
  {	 
      case DT_INT:	 
           new->value = NEW(int);	 
           *((int *) new->value) = *((int *) from->value);	 
           break;	 
      case DT_FLOAT: 
           new->value = NEW(double);	 
           *((double *) new->value) = *((double *) from->value);
           break;	 
      case DT_BOOL: 
           new->value = NEW(boolean); 
           *((boolean *) new->value) = *((boolean *) from->value); 
           break;	 
      case DT_STRING:	 
        new->value = strdup(from->value); 
        break; 
   } 	 
      return new;
}

static FromSubquery *
copyFromSubquery(FromSubquery *from)
{
    COPY_INIT(FromSubquery);
    COPY_NODE_FIELD(subquery);
    
    return new;
}

static FromJoinExpr *
copyFromJoinExpr(FromJoinExpr *from)
{
    COPY_INIT(FromJoinExpr);

    COPY_SCALAR_FIELD(left);
    COPY_SCALAR_FIELD(right);
    COPY_SCALAR_FIELD(joinType);
    COPY_SCALAR_FIELD(joinCond);
    COPY_NODE_FIELD(cond);

    return new;
}

static DistinctClause *
copyDistinctClause(DistinctClause *from)
{
    COPY_INIT(DistinctClause);

    COPY_NODE_FIELD(distinctExprs);

    return new;
}
/*copyObject copy of a Node tree or list and all substructure copied too */
/*this is a deep copy & with recursive*/

void *copyObject(void *from)
{
    void *retval;

    if(from == NULL)
        return NULL;

    switch(nodeTag(from))
    {
        /*different type nodes*/

        /*list nodes*/
        case T_List:
        case T_IntList:
            retval = deepCopyList(from);
            break;
        case T_AttributeReference:
            retval = copyAttributeReference(from);
            break;
        case T_FunctionCall:
            retval = copyFunctionCall(from);
            break;
        case T_Operator:
            retval = copyOperator(from);
            break;
        case T_Schema:
            retval = copySchema(from);
            break;
        case T_AttributeDef:
            retval = copyAttributeDef(from);
            break;
             /* query block model nodes */
//        case T_SetOp:
//            retval = copySetOp(from);
//            break;
        case T_SetQuery:
            retval = copySetQuery(from);
            break;
        case T_ProvenanceStmt:
            retval = copyProvenanceStmt(from);
            break;
        case T_QueryBlock:
            retval = copyQueryBlock(from);
            break;
        case T_SelectItem:
            retval = copySelectItem(from);
            break;
        case T_Constant:
            retval = copyConstant(from);
            break;
        case T_FromTableRef:
            retval = copyFromTableRef(from);
            break;
        case T_FromSubquery:
            retval = copyFromSubquery(from);
            break;
        case T_FromJoinExpr:
            retval = copyFromJoinExpr(from);
            break;
        case T_DistinctClause:
            retval = copyDistinctClause(from);
            break;
        case T_Insert:
            retval = copyInsert(from);
            break;
        case T_Delete:
            retval = copyDelete(from);
            break;
        case T_Update:
            retval = copyUpdate(from);
            break;

             /* query operator model nodes */
        case T_SelectionOperator:
            retval = copySelectionOp(from);
            break;
        case T_ProjectionOperator:
            retval = copyProjectionOp(from);
            break;
        case T_JoinOperator:
            retval = copyJoinOp(from);
            break;
        case T_AggregationOperator:
            retval = copyAggregationOp(from);
            break;
        case T_ProvenanceComputation:
            retval = copyProvenanceComputOp(from);
            break;
        case T_TableAccessOperator:
            retval = copyTableAccessOp(from);
            break;
        case T_SetOperator:
            retval = copySetOperator(from);
            break;
        case T_DuplicateRemoval:
            retval = copyDuplicateRemovalOp(from);
            break;

        default:
            retval = NULL;
            break;
    }

    return retval;
}
