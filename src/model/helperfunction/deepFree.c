/*-----------------------------------------------------------------------------
 *
 * deepFree.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

/* helper macros */
#define FREE_NODE_FIELD(fldname) deepFree(node->fldname)
#define FREE_SCALAR_FIELD(fldname)
#define FREE_P_FIELD(fldname) \
    if (node->fldname != NULL) \
        FREE(node->fldname);
#define FREE_LIST_SHALLOW(fldname) freeList(node->fldname)
#define FINISH_FREE() FREE(node)



/* declarations */
static void freeAggregationOperator (AggregationOperator *node);
static void freeConstant (Constant *node);
static void freeAttributeReference (AttributeReference *node);
static void freeFunctionCall (FunctionCall *node);

/* definitions */

#define FREE_OPERATOR() \
    do { \
        QueryOperator *node = (QueryOperator *) node; \
        FREE_NODE_FIELD(inputs); \
        FREE_NODE_FIELD(schema); \
        FREE_LIST_SHALLOW(parents); \
        FREE_NODE_FIELD(provAttrs); \
    } while (0)

static void
freeAggregationOperator (AggregationOperator *node)
{
    FREE_OPERATOR();

    FREE_NODE_FIELD(aggrs);
    FREE_NODE_FIELD(groupBy);

    FINISH_FREE();
}

static void
freeConstant (Constant *node)
{
    FREE_P_FIELD(value);

    FINISH_FREE();
}

static void
freeAttributeReference (AttributeReference *node)
{
    FREE_P_FIELD(name);

    FINISH_FREE();
}

static void
freeFunctionCall (FunctionCall *node)
{
    FREE_P_FIELD(functionname);
    FREE_NODE_FIELD(args);

    FINISH_FREE();
}

/* frees a node and all of its children */
void
deepFree (void *a)
{
    Node *node;

    if (a == NULL)
        return;

    node = (Node *) a;
    switch(node->type)
    {
        case T_List:
        case T_IntList:
            deepFreeList ((List *) node);
            break;
            /* expression nodes */
        case T_Constant:
            freeConstant((Constant *) node);
            break;
        case T_AttributeReference:
            freeAttributeReference((AttributeReference *) node);
            break;
        case T_FunctionCall:
            freeFunctionCall((FunctionCall *) node);
            break;
        case T_Operator:

            /* query block model nodes */
        case T_SetOp:
        case T_SetQuery:
        case T_ProvenanceStmt:
        case T_QueryBlock:
        case T_SelectItem:
        case T_FromItem:
        case T_FromTableRef:
        case T_FromSubquery:
        case T_FromJoinExpr:
        case T_DistinctClause:

            /* query operator model nodes */
        case T_Schema:
        case T_AttributeDef:
        case T_QueryOperator:
        case T_SelectionOperator:
        case T_ProjectionOperator:
        case T_JoinOperator:
            break;
        case T_AggregationOperator:
            freeAggregationOperator((AggregationOperator *) node);
            break;
        case T_ProvenanceComputation:
        case T_TableAccessOperator:
        case T_SetOperator:
        case T_DuplicateRemoval:
            break;
        /* error case */
        case T_Invalid:
        default:
            ERROR_LOG("cannot free invalid node type");
            break;
    }
}
