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
#include "model/set/set.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "uthash.h"

/* helper macros */
#define FREE_NODE_FIELD(fldname) deepFree(node->fldname)
#define FREE_SCALAR_FIELD(fldname)
#define FREE_P_FIELD(fldname) \
    if (node->fldname != NULL) \
        FREE(node->fldname);
#define FREE_LIST_SHALLOW(fldname) freeList(node->fldname)
#define FINISH_FREE() FREE(node)

/* set */
static void freeSet (Set *node);

/* expression model */
static void freeConstant (Constant *node);
static void freeAttributeReference (AttributeReference *node);
static void freeFunctionCall (FunctionCall *node);
static void freeOperator (Operator *node);

/* query block model */
static void freeQueryBlock (QueryBlock * node);

/* query operator model */
static void freeAttributeDef (AttributeDef *node);
static void freeSchema (Schema *node);
static void freeAggregationOperator (AggregationOperator *node);
static void freeProjectionOperator (ProjectionOperator *node);
static void freeProvenanceComputation (ProvenanceComputation *node);
static void freeSelectionOperator (SelectionOperator *node);
static void freeJoinOperator (JoinOperator *node);
static void freeDuplicateRemoval (DuplicateRemoval *node);
static void freeSetOperator (SetOperator *node);
static void freeTableAccessOperator (TableAccessOperator *node);

/* definitions */
static void
freeSet (Set *node)
{
    SetElem *el, *tmp;

    HASH_ITER(hh,node->elem, el, tmp)
    {
        HASH_DEL(node->elem, el);
        switch(node->setType)
        {
            case SET_TYPE_NODE:
                deepFree(el->data);
            break;
            case SET_TYPE_POINTER:
            case SET_TYPE_STRING:
                FREE(el->data);
            break;
            default:
                break;
        }

    }

    FINISH_FREE();
}

void
freeStringInfo (StringInfo node)
{
    if (node == NULL)
        return;

    FREE_P_FIELD(data);

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

static void
freeOperator (Operator *node)
{
    FREE_P_FIELD(name);
    FREE_NODE_FIELD(args);

    FINISH_FREE();
}

static void
freeQueryBlock (QueryBlock * node)
{
    FREE_NODE_FIELD(selectClause);
    FREE_NODE_FIELD(distinct);
    FREE_NODE_FIELD(fromClause);
    FREE_NODE_FIELD(whereClause);

    FINISH_FREE();
}

static void
freeAttributeDef (AttributeDef *node)
{
    FREE_P_FIELD(attrName);

    FINISH_FREE();
}

static void
freeSchema (Schema *node)
{
    FREE_P_FIELD(name);
    FREE_NODE_FIELD(attrDefs);

    FINISH_FREE();
}

#define FREE_OPERATOR() \
    do { \
        FREE_NODE_FIELD(op.inputs); \
        FREE_NODE_FIELD(op.schema); \
        FREE_LIST_SHALLOW(op.parents); \
        FREE_NODE_FIELD(op.provAttrs); \
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
freeProjectionOperator (ProjectionOperator *node)
{
    FREE_OPERATOR();

    FREE_NODE_FIELD(projExprs);

    FINISH_FREE();
}

static void
freeProvenanceComputation (ProvenanceComputation *node)
{
    FREE_OPERATOR();

    FINISH_FREE();
}

static void
freeSelectionOperator (SelectionOperator *node)
{
    FREE_OPERATOR();

    FREE_NODE_FIELD(cond);

    FINISH_FREE();
}

static void
freeJoinOperator (JoinOperator *node)
{
    FREE_OPERATOR();

    FREE_NODE_FIELD(cond);

    FINISH_FREE();
}

static void
freeDuplicateRemoval (DuplicateRemoval *node)
{
    FREE_OPERATOR();

    FREE_NODE_FIELD(attrs);

    FINISH_FREE();
}

static void
freeSetOperator (SetOperator *node)
{
    FREE_OPERATOR();
    FINISH_FREE();
}

static void
freeTableAccessOperator (TableAccessOperator *node)
{
    FREE_OPERATOR();

    FREE_P_FIELD(tableName);

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
        case T_Set:
            freeSet((Set *) node);
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
            freeOperator((Operator *) node);
            break;
            /* query block model nodes */
        //case T_SetOp:
        case T_SetQuery:
        case T_ProvenanceStmt:
        case T_QueryBlock:
            freeQueryBlock((QueryBlock *) node);
            break;
        case T_SelectItem:
        case T_FromItem:
        case T_FromTableRef:
        case T_FromSubquery:
        case T_FromJoinExpr:
        case T_DistinctClause:

            /* query operator model nodes */
        case T_Schema:
            freeSchema ((Schema *) node);
            break;
        case T_AttributeDef:
            freeAttributeDef ((AttributeDef *) node);
            break;
//        case T_QueryOperator:
        case T_SelectionOperator:
            freeSelectionOperator ((SelectionOperator *) node);
            break;
        case T_ProjectionOperator:
            freeProjectionOperator((ProjectionOperator *) node);
            break;
        case T_JoinOperator:
            freeJoinOperator((JoinOperator *) node);
            break;
        case T_AggregationOperator:
            freeAggregationOperator((AggregationOperator *) node);
            break;
        case T_ProvenanceComputation:
            freeProvenanceComputation((ProvenanceComputation *) node);
            break;
        case T_TableAccessOperator:
            freeTableAccessOperator ((TableAccessOperator *) node);
            break;
        case T_SetOperator:
            freeSetOperator((SetOperator *) node);
            break;
        case T_DuplicateRemoval:
            freeDuplicateRemoval((DuplicateRemoval *) node);
            break;
        /* error case */
        case T_Invalid:
        default:
            ERROR_LOG("cannot free invalid node type");
            break;
    }
}
