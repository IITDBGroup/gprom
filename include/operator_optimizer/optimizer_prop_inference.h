/*-----------------------------------------------------------------------------
 *
 * optimizer_prop_inference.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_OPERATOR_OPTIMIZER_OPTIMIZER_PROP_INFERENCE_H_
#define INCLUDE_OPERATOR_OPTIMIZER_OPTIMIZER_PROP_INFERENCE_H_

#include "model/query_operator/query_operator.h"

extern void computeKeyProp (QueryOperator *root);
extern void computeECProp (QueryOperator *root);
extern void computeSetProp (QueryOperator *root);

#endif /* INCLUDE_OPERATOR_OPTIMIZER_OPTIMIZER_PROP_INFERENCE_H_ */
