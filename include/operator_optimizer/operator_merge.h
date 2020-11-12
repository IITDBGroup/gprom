/*-----------------------------------------------------------------------------
 *
 * operator_merge.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef OPERATOR_MERGE_H_
#define OPERATOR_MERGE_H_

#include "model/query_operator/query_operator.h"

extern SelectionOperator *mergeSelection(SelectionOperator *op);
extern ProjectionOperator *mergeProjection(ProjectionOperator *op);

extern QueryOperator *pushDownSelectionWithProjection(SelectionOperator *op);


#endif /* OPERATOR_MERGE_H_ */
