/*-----------------------------------------------------------------------------
 *
 * common_prop_inference.h
 *
 *
 *		AUTHOR: Xing Niu
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COMMON_PROP_INFERENCE_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COMMON_PROP_INFERENCE_H_

#include "model/query_operator/query_operator.h"

extern void exprBottomUp(QueryOperator *root);
extern void predBottomUp(QueryOperator *root);
extern void printEXPRPro(QueryOperator *root);
extern void printPREDPro(QueryOperator *root);

extern Node *ListAttrRefsToNameSetForAgg(QueryOperator *op, List *l);
extern Node *ListAttrRefsToEqCondsForAgg(QueryOperator *op, List *l);
//extern boolean checkEqCompForListAttrRefsOfOp(QueryOperator *root, List *l, char *isGcOrGe);

/* used for gc and ge */
extern char *escapeUnderscore (char *str);
extern char *getRightAttrName (char *attr);
extern boolean addPrimeOnAttrsInOperator(Node *node, char *state);
extern Node *generateAttrAndPrimeEq(List *l);
extern List *generateAttrDefAndPrimeNonEq(List *l);

extern List *generateAttrDefAndPrimeEq(List *l);

extern boolean isStartAsAGG(char *name);



extern Node *getConds(QueryOperator *op);

#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COMMON_PROP_INFERENCE_H_ */
