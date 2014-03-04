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

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"

static boolean isTupleAtATimeSubtree(QueryOperator *op);

QueryOperator *
rewritePI_CSComposable (ProvenanceComputation *op)
{
    QueryOperator *rewRoot;



    rewRoot = OP_LCHILD(op);

    return (QueryOperator *) rewRoot;
}


static boolean
isTupleAtATimeSubtree(QueryOperator *op)
{
    switch(op->type)
    {
        case SelectionOperator:
        case ProjectionOperator:
        case JoinOperator:
            FOREACH(QueryOperator,child,op->inputs)
                if (!isTupleAtATimeSubtree)
                    return FALSE;
            return TRUE;
        default:
            return FALSE;
    }
}
