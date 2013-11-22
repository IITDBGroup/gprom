/*-----------------------------------------------------------------------------
 *
 * prov_utility.h
 *		General utility functions for provenance rewrite.
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PROV_UTILITY_H_
#define PROV_UTILITY_H_

#include "model/query_operator/query_operator.h"

extern void addProvenanceAttrsToSchema(QueryOperator *target, QueryOperator *source);
extern void switchSubtrees(QueryOperator *orig, QueryOperator *new);
extern QueryOperator *copyUnrootedSubtree(QueryOperator *op);

#endif /* PROV_UTILITY_H_ */
