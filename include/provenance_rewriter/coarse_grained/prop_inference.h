/*-----------------------------------------------------------------------------
 *
 * prop_inference.h
 *
 *
 *		AUTHOR: Xing Niu
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PROP_INFERENCE_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PROP_INFERENCE_H_

#include "model/query_operator/query_operator.h"
//#include "provenance_rewriter/coarse_grained/z3_solver.h"
#include "model/set/hashmap.h"

extern void bottomUpInference(QueryOperator *root, HashMap *lmap, HashMap *rmap);

















#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PROP_INFERENCE_H_ */
