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

#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COMMON_PROP_INFERENCE_H_ */
