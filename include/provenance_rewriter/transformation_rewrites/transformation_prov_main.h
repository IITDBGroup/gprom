/*-----------------------------------------------------------------------------
 *
 * transformation_prov_main.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef TRANSFORMATION_PROV_MAIN_H_
#define TRANSFORMATION_PROV_MAIN_H_

#include "model/query_operator/query_operator.h"

extern QueryOperator *rewriteTransformationProvenance (QueryOperator *op);
extern QueryOperator *rewriteTransformationProvenanceImport (QueryOperator *op);
extern void findTableAccessOperator(List **drOp, QueryOperator *root);
extern TableAccessOperator *findTAOp(QueryOperator *root);

#endif /* TRANSFORMATION_PROV_MAIN_H_ */
