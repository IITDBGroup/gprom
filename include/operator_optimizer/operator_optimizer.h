/*-----------------------------------------------------------------------------
 *
 * operator_optimizer.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef OPERATOR_OPTIMIZER_H_
#define OPERATOR_OPTIMIZER_H_

#include "model/query_operator/query_operator.h"

/* apply all optimizations */
extern Node *optimizeOperatorModel (Node *root);

/* activate materialization for adjcent projections */
extern QueryOperator *materializeProjectionSequences (QueryOperator *root);

/* try to merge adajacent operators of the same type into one operator. */
extern QueryOperator *mergeAdjacentOperators (QueryOperator *root);

/* try to push down the selection operator for provenence. */
extern QueryOperator *pushDownSelectionOperatorOnProv(QueryOperator *root);

extern QueryOperator *factorAttrsInExpressions(QueryOperator *root);

extern QueryOperator *removeRedundantProjections(QueryOperator *root);

/* try to pull up provenance projections. */
extern QueryOperator *pullingUpProvenanceProjections(QueryOperator *root);

/* try to push down the selection operator through joins for provenence. */
extern QueryOperator *pushDownSelectionThroughJoinsOperatorOnProv(QueryOperator *root);

#endif /* OPERATOR_OPTIMIZER_H_ */
