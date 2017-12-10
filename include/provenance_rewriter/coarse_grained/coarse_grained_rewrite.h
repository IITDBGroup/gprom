/*-----------------------------------------------------------------------------
 *
 * coarse_grained_rewrite.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_

#include "model/query_operator/query_operator.h"

extern QueryOperator *addTopAggForCoarse (QueryOperator *op);
extern void markTableAccessAndAggregation (QueryOperator *op, Node *coarsePara);
extern void markUseTableAccessAndAggregation (QueryOperator *op, Node *coarsePara);



#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_ */
