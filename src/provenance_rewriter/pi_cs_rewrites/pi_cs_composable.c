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

QueryOperator *
rewritePI_CSComposable (ProvenanceComputation *op)
{
    QueryOperator *rewRoot;



    rewRoot = OP_LCHILD(op);

    return (QueryOperator *) rewRoot;
}
