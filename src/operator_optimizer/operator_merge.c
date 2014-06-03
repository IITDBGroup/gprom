/*-----------------------------------------------------------------------------
 *
 * operator_merge.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/schema_utility.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "mem_manager/mem_mgr.h"
#include "operator_optimizer/operator_merge.h"

/* state */
typedef struct ReplaceRefState {
    List *projExpr;
    ProjectionOperator *op;
} ReplaceRefState;

/* declarations */
static Node *replaceAttributeRefsMutator (Node *node, ReplaceRefState *state);
static boolean mergeProjectionIsSafe(ProjectionOperator *op, ProjectionOperator *child);
static HashMap *getAttrRefCount(Node *expr);


SelectionOperator *
mergeSelection(SelectionOperator *op)
{
    while(isA(OP_LCHILD(op),SelectionOperator))
    {
        SelectionOperator *child = (SelectionOperator *) OP_LCHILD(op);

        // only one parent of child is allowed
        if (LIST_LENGTH(child->op.parents) > 1)
        	break;

        // and condition and link child's children to root
        op->cond = (Node *) createOpExpr("AND", LIST_MAKE(op->cond, child->cond));
        op->op.inputs = child->op.inputs;

        FOREACH(QueryOperator, el, op->op.inputs)
        	el->parents = child->op.parents;

        // clean up child
        child->cond = NULL;
        child->op.inputs = NULL;
//        deepFree(child);
    }

    return op;
}

ProjectionOperator *
mergeProjection(ProjectionOperator *op)
{
    ReplaceRefState *state = NEW(ReplaceRefState);

    while(isA(OP_LCHILD(op),ProjectionOperator))
    {
        ProjectionOperator *child = (ProjectionOperator *) OP_LCHILD(op);

        // only one parent of child is allowed
        if (LIST_LENGTH(child->op.parents) > 1)
        	break;

        if (!mergeProjectionIsSafe(op, child))
            break;

        // combine expressions and link child's children to root
        state->op = child;
        state->projExpr = child->projExprs;

        START_TIMER("OptimizeModel - replace attrs with expr");
        op->projExprs = (List *) replaceAttributeRefsMutator((Node *) op->projExprs, state);
        op->op.inputs = child->op.inputs;
        STOP_TIMER("OptimizeModel - replace attrs with expr");

        FOREACH(QueryOperator, el, op->op.inputs)
        	el->parents = replaceNode(el->parents, child, op);

        // clean up child
        child->projExprs = NULL;
        child->op.inputs = NULL;
//        deepFree(child);
    }

    state->op = NULL;
    state->projExpr = NULL;
    FREE(state);
    return op;
}

QueryOperator *
pushDownSelectionWithProjection(SelectionOperator *op)
{
	ReplaceRefState *state = NEW(ReplaceRefState);
	QueryOperator *newRoot = (QueryOperator *) op;

	while(isA(OP_LCHILD(op),ProjectionOperator))
	{
		ProjectionOperator *child = (ProjectionOperator *) OP_LCHILD(op);
		QueryOperator *grandChild = OP_LCHILD(child);
		List *oldP = op->op.parents;

		// only one parent of child is allowed
		if (LIST_LENGTH(child->op.parents) > 1)
			break;

        // set newRoot
        if (newRoot == (QueryOperator *) op)
            newRoot = (QueryOperator *) child;

		// combine expressions and link child's children to root
		state->op = child;
		state->projExpr = child->projExprs;

		// change selection
		op->cond = replaceAttributeRefsMutator(op->cond, state);
		op->op.schema = copyObject(grandChild->schema);
		op->op.inputs = child->op.inputs;
        op->op.parents = singleton(child);

		// push down selection adapt projection
		child->op.inputs = singleton(op);
		child->op.parents = oldP;

		// replace projection with selection in parents of grandchild
		grandChild->parents = replaceNode(grandChild->parents, child, op);

		// replace selection with projection in previous selection parents
		FOREACH(QueryOperator, el, oldP)
			el->inputs = replaceNode(el->inputs, op, child);
	}

	state->op = NULL;
	state->projExpr = NULL;
	FREE(state);
	return newRoot;
}

static Node *
replaceAttributeRefsMutator (Node *node, ReplaceRefState *state)
{
    if (node == NULL)
        return NULL;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        int pos = getAttributeNum(a->name, (QueryOperator *) state->op);
        deepFree(a);

        return copyObject(getNthOfListP(state->projExpr, pos));
    }

    return mutate(node, replaceAttributeRefsMutator, state);
}

static boolean
mergeProjectionIsSafe(ProjectionOperator *op, ProjectionOperator *child)
{
    START_TIMER("OptimizeModel - check safety of projection merge");

    FOREACH(Node,p,op->projExprs)
    {
        DEBUG_LOG("projection expression: %s", beatify(nodeToString(p)));
        HashMap *cnt = getAttrRefCount(p);

        // do not merge if there is more than one
        FOREACH_HASH_KEY(Constant,key,cnt)
        {
            char *aName = STRING_VALUE(key);
            int count = INT_VALUE(getMap(cnt, (Node *) key));

            // mentioned more than once. This is unsafe, unless the child attribute is a safe
            if (count > 1)
            {
                Node *childProjExpr = getNthOfListP(child->projExprs, getAttrPos((QueryOperator *) child,aName));
                HashMap *childCnt = getAttrRefCount(childProjExpr);
                DEBUG_LOG("mentioned more than once: %s: %u", aName, count);

                FOREACH_HASH(Constant,c,childCnt)
                {
                    if (INT_VALUE(c) > 1)
                    {
                        DEBUG_LOG("child attribute is not safe to merge: %s: %u", aName, count);
                        STOP_TIMER("OptimizeModel - check safety of projection merge");
                        return FALSE;
                    }
                }
            }
        }
    }
    STOP_TIMER("OptimizeModel - check safety of projection merge");

    return TRUE;
}

static HashMap *
getAttrRefCount(Node *expr)
{
    HashMap *cnt = NEW_MAP(Constant,Constant);
    List *attrRef = getAttrReferences(expr);

    FOREACH(AttributeReference,a,attrRef)
    {
        Constant *aC;
        if (!MAP_HAS_STRING_KEY(cnt, a->name))
        {
            aC = createConstInt(0);
            MAP_ADD_STRING_KEY(cnt, a->name, aC);
        }
        else
            aC = (Constant *) MAP_GET_STRING(cnt, a->name);
        INT_VALUE(aC) += 1;
        DEBUG_LOG("attribute %s count is %s", a->name, exprToSQL((Node *) aC));
    }

    return cnt;
}
