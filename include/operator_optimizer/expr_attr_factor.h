/*-----------------------------------------------------------------------------
 *
 * expr_attr_factor.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef EXPR_ATTR_FACTOR_H_
#define EXPR_ATTR_FACTOR_H_

#include "model/query_operator/query_operator.h"

extern QueryOperator *projectionFactorAttrReferences(ProjectionOperator *op);
Node *factorAttrRefs (Node *node);

#endif /* EXPR_ATTR_FACTOR_H_ */
