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
extern void computeECPropBottomUp (QueryOperator *root);
extern List *GenerateCondECSetListUsedInBottomUp(Node *op);
extern List *GenerateCondECBasedOnCondOp(List *condList);
extern List *CombineDuplicateElemSetInECList(List *DupECList);
extern List *LSCHtoRSCH(List *setList, List *rECSetList, List *lSchemaList, List *rSchemaList);
extern void computeECPropTopDown (QueryOperator *root);

/* used in projection bottom up */
extern List *SCHAtoBUsedInBomUp(List *setList, List *childECSetList, List *attrA, List *attrB);
extern void SCHBtoAUsedInTopBom(List **setList, List *attrRefs, List *attrDefs);

extern void computeReqColProp (QueryOperator *root);
extern void computeSetProp (QueryOperator *root);

extern void initializeSetProp(QueryOperator *root);
extern void initializeIColProp(QueryOperator *root);
extern void printIcols(QueryOperator *root);

extern Set* AddAttrOfSelectCondToSet(Set *set, Operator *op);

/* help print function */
extern void printECPro(QueryOperator *root);
extern void printSingleECList(List *l);

#endif /* INCLUDE_OPERATOR_OPTIMIZER_OPTIMIZER_PROP_INFERENCE_H_ */
