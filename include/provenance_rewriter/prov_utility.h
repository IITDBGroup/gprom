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
#include "model/set/set.h"
#include "model/query_operator/query_operator.h"

#define REWR_NULLARY_SETUP(rewrite_method,optype)			\
	QueryOperator *rewr;									\
															\
    DEBUG_LOG("REWRITE-" #rewrite_method  " - " #optype);	\
    DEBUG_NODE_BEATIFY_LOG("Operator tree", op)

#define REWR_UNARY_SETUP(rewrite_method,optype)				\
	QueryOperator *rewr;									\
	QueryOperator *rewrInput;								\
															\
    ASSERT(OP_LCHILD(op));									\
															\
    DEBUG_LOG("REWRITE-" #rewrite_method  " - " #optype);	\
    DEBUG_NODE_BEATIFY_LOG("Operator tree", op)

#define REWR_BINARY_SETUP(rewrite_method,optype)			\
	QueryOperator *rewr;									\
	QueryOperator *rewrLeftInput;							\
	QueryOperator *rewrRightInput;							\
															\
    ASSERT(OP_LCHILD(op));									\
    ASSERT(OP_RCHILD(op));									\
															\
	DEBUG_LOG("REWRITE-" #rewrite_method  " - " #optype);	\
    DEBUG_NODE_BEATIFY_LOG("Operator tree", op)

#define REWR_NULLARY()											\
	do															\
	{															\
		rewr = shallowCopyQueryOperator((QueryOperator *) op);	\
	} while(0)

#define REWR_UNARY_CHILD(_method)								\
	do															\
	{															\
     	rewr = shallowCopyQueryOperator((QueryOperator *) op);	\
	    rewrInput = _method(OP_LCHILD(op), state);				\
		addChildOperator(rewr, rewrInput);						\
	} while(0)

#define REWR_BINARY_CHILDREN(_method)							\
	do															\
	{															\
     	rewr = shallowCopyQueryOperator((QueryOperator *) op);	\
	    rewrLeftInput = _method(OP_LCHILD(op), state);			\
	    rewrRightInput = _method(OP_RCHILD(op), state);			\
		addChildOperator(rewr, rewrLeftInput);					\
		addChildOperator(rewr, rewrRightInput);					\
	} while(0)

#define LOG_RESULT(mes,op)						\
    do {										\
        INFO_OP_LOG(mes,op);					\
        DEBUG_NODE_BEATIFY_LOG(mes,op);			\
    } while(0)

#define LOG_RESULT_AND_RETURN(optype)						\
	do														\
	{														\
		LOG_RESULT(#optype " - rewritten operator:", rewr);	\
		return (QueryOperator *) rewr;						\
	} while(0)

// schema manipulation
extern void clearAttrsFromSchema(QueryOperator *target);
extern void addNormalAttrsToSchema(QueryOperator *target, QueryOperator *source);
extern void addProvenanceAttrsToSchemaWithRename(QueryOperator *target, QueryOperator * source, char *suffix);
extern void addProvenanceAttrsToSchema(QueryOperator *target, QueryOperator *source);
extern void addProvenanceAttrsToSchemabasedOnList(QueryOperator *target, List *provList);
extern void makeNamesUnique (List *names, Set *allNames);

// bookkeeping rewritten version of subqueries and copies of subqueries
extern boolean isOpRewritten(HashMap *opToRewrittenOp, QueryOperator *op);
extern QueryOperator *getRewrittenOp(HashMap *opToRewrittenOp, QueryOperator *op);
extern QueryOperator *setRewrittenOp(HashMap *opToRewrittenOp, QueryOperator *op, QueryOperator *rewrittenOp);
extern QueryOperator *getOrSetOpCopy(HashMap *origOps, QueryOperator *op);

// create projection expressions
extern List *getProvAttrProjectionExprs(QueryOperator *op);
extern List *getNormalAttrProjectionExprs(QueryOperator *op);
extern List *getAllAttrProjectionExprs(QueryOperator *op);
extern QueryOperator *createProjOnAllAttrs(QueryOperator *op);
extern QueryOperator *createProjOnAttrs(QueryOperator *op, List *attrPos);
extern QueryOperator *createProjOnAttrsByName(QueryOperator *op, List *attrNames, List *newAttrNames);
//extern AttributeReference *createAttrsRefByName(QueryOperator *op, char *attrNames);
//extern AttributeReference *createAttrRefByPos(QueryOperator *op, int pos);
extern AttributeReference *createAttrsRefByName(QueryOperator *op, char *attrNames);

// graph manipulation
extern void switchSubtrees(QueryOperator *orig, QueryOperator *new);
extern void switchSubtreeWithExisting (QueryOperator *orig, QueryOperator *new);
extern QueryOperator *copyUnrootedSubtree(QueryOperator *op);
extern void removeParentFromOps (List *operators, QueryOperator *parent);
extern void substOpInParents (List *parents, QueryOperator *orig, QueryOperator *newOp);

// graph search
extern boolean findTableAccessVisitor (Node *node, List **result);
extern List *findOperatorAttrRefs (QueryOperator *op);
extern boolean hasProvComputation(Node *op);

#endif /* PROV_UTILITY_H_ */
