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

#include "model/list/list.h"
#include "model/query_operator/query_operator.h"

// schema manipulation
extern void clearAttrsFromSchema(QueryOperator *target);
extern void addNormalAttrsToSchema(QueryOperator *target, QueryOperator *source);
extern void addProvenanceAttrsToSchema(QueryOperator *target, QueryOperator *source);
extern void addProvenanceAttrsToSchemabasedOnList(QueryOperator *target, List *provList);

// create projection expressions
extern List *getProvAttrProjectionExprs(QueryOperator *op);
extern List *getNormalAttrProjectionExprs(QueryOperator *op);
extern QueryOperator *createProjOnAllAttrs(QueryOperator *op);
extern QueryOperator *createProjOnAttrs(QueryOperator *op, List *attrPos);

// graph manipulation
extern void switchSubtrees(QueryOperator *orig, QueryOperator *new);
extern void switchSubtreeWithExisting (QueryOperator *orig, QueryOperator *new);
extern QueryOperator *copyUnrootedSubtree(QueryOperator *op);
extern void removeParentFromOps (List *operators, QueryOperator *parent);

// graph search
extern boolean findTableAccessVisitor (Node *node, List **result);
extern List *findOperatorAttrRefs (QueryOperator *op);

#endif /* PROV_UTILITY_H_ */
