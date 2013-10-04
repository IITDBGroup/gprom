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

/* try to merge adajacent operators of the same type into one operator. */
extern QueryOperator *mergeAdjacentOperators (QueryOperator *root);

#endif /* OPERATOR_OPTIMIZER_H_ */
