/*-----------------------------------------------------------------------------
 *
 * pi_cs_composable.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"

#include "model/query_operator/query_operator.h"

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/prov_schema.h"

static Node *asOf;
static RelCount *nameState;

// static methods
static boolean isTupleAtATimeSubtree(QueryOperator *op);

static QueryOperator *rewritePI_CSComposablOperator (QueryOperator *op);
static QueryOperator *rewritePI_CSComposablSelection (SelectionOperator *op);
static QueryOperator *rewritePI_CSComposablProjection (ProjectionOperator *op);
static QueryOperator *rewritePI_CSComposablJoin (JoinOperator *op);
static QueryOperator *rewritePI_CSComposablAggregation (AggregationOperator *op);
static QueryOperator *rewritePI_CSComposablSet (SetOperator *op);
static QueryOperator *rewritePI_CSComposablTableAccess(TableAccessOperator *op);
static QueryOperator *rewritePI_CSComposablConstRel(ConstRelOperator *op);
static QueryOperator *rewritePI_CSComposablDuplicateRemOp(DuplicateRemoval *op);


/*
 *
 */
QueryOperator *
rewritePI_CSComposable (ProvenanceComputation *op)
{
    QueryOperator *rewRoot;

    rewRoot = OP_LCHILD(op);
    rewRoot = rewritePI_CSComposablOperator(rewRoot);

    return (QueryOperator *) rewRoot;
}

/*
 *
 */
static boolean
isTupleAtATimeSubtree(QueryOperator *op)
{
    switch(op->type)
    {
        case T_SelectionOperator:
        case T_ProjectionOperator:
        case T_JoinOperator:
            FOREACH(QueryOperator,child,op->inputs)
                if (!isTupleAtATimeSubtree)
                    return FALSE;
            return TRUE;
        default:
            return FALSE;
    }
}

static QueryOperator *
rewritePI_CSComposablOperator (QueryOperator *op)
{
    switch(op->type)
    {
        case T_SelectionOperator:
            return rewritePI_CSComposablSelection((SelectionOperator *) op);
        case T_ProjectionOperator:
            return rewritePI_CSComposablProjection((ProjectionOperator *) op);
        case T_JoinOperator:
            return rewritePI_CSComposablJoin((JoinOperator *) op);
        case T_AggregationOperator:
            return rewritePI_CSComposablAggregation((AggregationOperator *) op);
        case T_Set:
            return rewritePI_CSComposablSet((SetOperator *) op);
        case T_TableAccessOperator:
            return rewritePI_CSComposablTableAccess((TableAccessOperator *) op);
        case T_ConstRelOperator:
            return rewritePI_CSComposablConstRel((ConstRelOperator *) op);
        case T_DuplicateRemoval:
            return rewritePI_CSComposablDuplicateRemOp((DuplicateRemoval *) op);
        default:
            FATAL_LOG("rewrite for %u not implemented", op->type);
            return NULL;
    }
}

static QueryOperator *
rewritePI_CSComposablSelection (SelectionOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablProjection (ProjectionOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablJoin (JoinOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablAggregation (AggregationOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablSet (SetOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablTableAccess(TableAccessOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablConstRel(ConstRelOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposablDuplicateRemOp(DuplicateRemoval *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}
