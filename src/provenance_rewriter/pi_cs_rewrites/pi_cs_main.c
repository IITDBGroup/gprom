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
#include "model/query_operator/query_operator.h"

QueryOperator *
rewritePI_CS (QueryOperator *op)
{
    switch(op->type)
    {
        case T_SelectionOperator:
            rewritePI_CS(OP_LCHILD(op->inputs));
            return op;
            break;
        case T_ProjectionOperator:
            rewritePI_CS(OP_LCHILD(op->inputs));
            return op;
        case T_AggregationOperator:
            return rewritePI_CSAggregation (op);
        default:
            return NULL;
    }
}
