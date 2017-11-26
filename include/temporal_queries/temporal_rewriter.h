/*-----------------------------------------------------------------------------
 *
 * temporal_rewriter.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_TEMPORAL_QUERIES_TEMPORAL_REWRITER_H_
#define INCLUDE_TEMPORAL_QUERIES_TEMPORAL_REWRITER_H_

#include "model/query_operator/query_operator.h"

#define TBEGIN_NAME "T_B"
#define TEND_NAME "T_E"
#define TEMPORAL_DT DT_INT

extern QueryOperator *rewriteImplicitTemporal (QueryOperator *q);
extern void addCoalescingAndNormalization (QueryOperator *q);
extern QueryOperator *addCoalesce (QueryOperator *input);
extern QueryOperator *addTemporalNormalization (QueryOperator *input, QueryOperator *reference, List *attrs);
extern QueryOperator *addTemporalNormalizationUsingWindow (QueryOperator *input, QueryOperator *reference, List *attrs);
extern QueryOperator *addCoalesceForAllOp(QueryOperator *op);
extern QueryOperator *rewriteTemporalAggregationWithNormalization(AggregationOperator *agg);
extern QueryOperator *rewriteTemporalSetDiffWithNormalization(SetOperator *diff);

#endif /* INCLUDE_TEMPORAL_QUERIES_TEMPORAL_REWRITER_H_ */
