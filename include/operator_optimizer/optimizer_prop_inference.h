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
#include "model/set/set.h"
#include "model/set/hashmap.h"

extern void computeKeyProp (QueryOperator *root);

#define MIN_KEY "MIN"
#define MAX_KEY "MAX"

#define SET_ICOLS(_op,_icols) setStringProperty((QueryOperator *) _op, PROP_STORE_SET_ICOLS, (Node *) _icols)
#define GET_ICOLS(_op) ((Set *) getStringProperty((QueryOperator *) _op, PROP_STORE_SET_ICOLS))
#define HAS_ICOLS(_op) HAS_STRING_PROP(_op, PROP_STORE_SET_ICOLS)
#define GET_OR_CREATE_ICOLS(_op,_store_icols)	\
	if(HAS_ICOLS(_op))							\
	{											\
		_store_icols = GET_ICOLS(_op);			\
	}											\
    else										\
	{											\
		_store_icols = STRSET();				\
		SET_ICOLS(_op,_store_icols);			\
	}
#define IS_ICOLS_DONE(_op) HAS_STRING_PROP(_op, PROP_STORE_SET_ICOLS_DONE)

extern void computeMinMaxPropForSubset(QueryOperator *root, Set *attrs);
extern void computeMinMaxProp(QueryOperator *root);
extern Set *getInputSchemaDependencies(QueryOperator *op, Set *attrs, boolean left);
extern void computeChildOperatorProp(QueryOperator *root);
extern void getConMap(Node *expr, HashMap *leftResult, HashMap *rightResult);

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

extern void computeReqColProp(QueryOperator *root);
extern boolean isAttrRequired(QueryOperator *q, char *attr);
extern void computeSetProp (QueryOperator *root);

extern void initializeSetProp(QueryOperator *root);
extern void initializeIColProp(QueryOperator *root);
extern void printIcols(QueryOperator *root);

extern Set *addAttrOfSelectCondToSet(Set *set, Node *expr);

/* not null property */
extern void computeNotNullProp(QueryOperator *op);

/* help print function */
extern char *ecToString(List *ec);
extern void printECPro(QueryOperator *root);
extern void printSingleECList(List *l);

/* empty property for each operator used in loop each optimization method*/
extern void emptyProperty(QueryOperator *root);
extern void removeProp(QueryOperator *op, char *prop);
extern void removeMinMaxProps(QueryOperator *op);

#endif /* INCLUDE_OPERATOR_OPTIMIZER_OPTIMIZER_PROP_INFERENCE_H_ */
