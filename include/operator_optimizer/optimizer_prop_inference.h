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
#include "include/model/set/set.h"

extern void computeKeyProp (QueryOperator *root);
extern void computeECProp (QueryOperator *root);
extern void computeReqColProp (QueryOperator *root);
extern void computeSetProp (QueryOperator *root);

extern void initializeSetProp(QueryOperator *root);
extern void initializeIColProp(QueryOperator *root, Set *seticols);

extern Set* AddAttrOfSelectCondToSet(Set *set, Operator *op);

#endif /* INCLUDE_OPERATOR_OPTIMIZER_OPTIMIZER_PROP_INFERENCE_H_ */
