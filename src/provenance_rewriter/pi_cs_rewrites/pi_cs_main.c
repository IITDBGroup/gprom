/*-----------------------------------------------------------------------------
 *
 * pi_cs_main.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/query_operator/query_operator.h"

static void rewritePI_CSOperator (QueryOperator *op);
static void rewritePI_CSSelection (SelectionOperator *op);
static void rewritePI_CSProjection (ProjectionOperator *op);
static void rewritePI_CSJoin (JoinOperator *op);
static void rewritePI_CSAggregation (AggregationOperator *op);
//TODO add set operations

QueryOperator *
rewritePI_CS (ProvenanceComputation  *op)
{
    QueryOperator *rewRoot = OP_LCHILD(op);

    // rewrite subquery under provenance computation
    rewritePI_CSOperator(rewRoot);

    // update root of rewritten subquery
    rewRoot = OP_LCHILD(op);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, rewRoot);

    return rewRoot;
}

static void
rewritePI_CSOperator (QueryOperator *op)
{
    switch(op->type)
    {
        case T_SelectionOperator:
            rewritePI_CSSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            rewritePI_CSProjection((ProjectionOperator *) op);
            break;
        case T_AggregationOperator:
            rewritePI_CSAggregation ((AggregationOperator *) op);
            break;
        case T_JoinOperator:
            rewritePI_CSJoin((JoinOperator *) op);
        //TODO OTHER OPERATORS
            break;
        default:
            break;
    }
}

static void
rewritePI_CSSelection (SelectionOperator *op)
{
    // rewrite child first
    rewritePI_CSOperator(OP_LCHILD(op));


    // adapt schema addProvenanceAttrsToSchema(op, OP_LCHILD(op));
}

static void
rewritePI_CSProjection (ProjectionOperator *op)
{
    // rewrite child
    rewritePI_CSOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs

    // adapt schema
}

static void
rewritePI_CSJoin (JoinOperator *op)
{
    // rewrite children
    rewritePI_CSOperator(OP_LCHILD(op));
    rewritePI_CSOperator(OP_RCHILD(op));

    // adapt schema

}

/*
 * Rewrite an aggregation operator:
 *      - replace aggregation with projection over join between the aggregation
 *       and the aggregation rewritten input
 */
static void
rewritePI_CSAggregation (AggregationOperator *op)
{
    JoinOperator *joinProv;
    ProjectionOperator *proj;
    QueryOperator *aggInput;

    // create final projection and replace aggregation subtree with projection
    proj = createProjectionOp(NIL, NULL, NIL, NIL);
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);

    // create join operator
    joinProv = createJoinOp(JOIN_LEFT_OUTER, NULL, NIL, NIL,
            NIL);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) joinProv);

    // copy aggregation input
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));

    // add projection including group by expressions if necessary

    // add aggregation to join input
    addChildOperator((QueryOperator *) joinProv, (QueryOperator *) op);
    addChildOperator((QueryOperator *) joinProv, (QueryOperator *) aggInput);

    // rewrite aggregation input copy
    rewritePI_CSOperator(aggInput);

    // create join condition

    // create projection expressions for final projection
}
