/*-----------------------------------------------------------------------------
 *
 * gc_prop_inference.h
 *
 *
 *		AUTHOR: Xing Niu
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_GC_PROP_INFERENCE_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_GC_PROP_INFERENCE_H_

#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"

extern void gcBottomUp(QueryOperator *root,List *attrNames);
//extern void predBottomUp(QueryOperator *root);
//extern void printEXPRPro(QueryOperator *root);
//extern void printPREDPro(QueryOperator *root);

#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_GC_PROP_INFERENCE_H_ */
