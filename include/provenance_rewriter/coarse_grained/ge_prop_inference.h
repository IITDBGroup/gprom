/*-----------------------------------------------------------------------------
 *
 * gc_prop_inference.h
 *
 *
 *		AUTHOR: Xing Niu
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_GE_PROP_INFERENCE_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_GE_PROP_INFERENCE_H_

#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"

extern void geBottomUp(QueryOperator *root, HashMap *lmap, HashMap *rmap);
//extern void predBottomUp(QueryOperator *root);
//extern void printEXPRPro(QueryOperator *root);
//extern void printPREDPro(QueryOperator *root);
extern HashMap *bindsToHashMap(List *names, List *values);
extern void doGeBottomUp(QueryOperator *op);

#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_GE_PROP_INFERENCE_H_ */
