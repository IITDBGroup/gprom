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

static void rewriteOperator (QueryOperator *op);
static void rewriteSelection (SelectionOperator *op);


QueryOperator *
rewritePI_CS (ProvenanceComputation  *op)
{
    QueryOperator *rewRoot = OP_LCHILD(op);

    // rewrite subquery under provenance computation
    rewriteOperator(rewRoot);
    // update root of rewritten subquery
    rewRoot = OP_LCHILD(op);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees(op, rewRoot);

    return rewRoot;
}

static void
rewriteOperator (QueryOperator *op)
{
    FOREACH(QueryOperator,child,op->inputs)
        rewriteOperator(child);

    switch(op->type)
    {
        case T_SelectionOperator:
            rewriteSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            rewriteOperator(OP_LCHILD(op));
//            addProvenanceAttrsToSchema(op, OP_LCHILD(op));
//            addProvenanceAttrsToProjection((ProjectionOperator *)op);
            break;
        case T_AggregationOperator:
//            return rewritePI_CSAggregation (op);
            break;
        case T_JoinOperator:
            rewriteOperator(OP_LCHILD(op));
            rewriteOperator(OP_RCHILD(op));
//            addProvenanceAttrsToSchema(op, OP_LCHILD(op));
//            addProvenanceAttrsToSchema(op, OP_RCHILD(op));
            //TODO OTHER OPERATOR
            break;
        default:
            break;
    }
}

static void
rewriteSelection (SelectionOperator *op)
{
    rewriteOperator(OP_LCHILD(op));
    // adapt schema addProvenanceAttrsToSchema(op, OP_LCHILD(op));
}
