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
#include "model/query_operator/operator_property.h"
#include "model/query_operator/schema_utility.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
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
static Vector *calculateChildAttrRefCnts(ProjectionOperator *op, QueryOperator *child);
static inline void getAttrRefCount(Node *expr, Vector *cnts);
static boolean isMergeSafe(Vector *opRef, Vector *childRef);
static void mergeReferenceVectors(QueryOperator *o, Vector *opRef, Vector *childRef);



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
    List *stack = singleton(op);
    ProjectionOperator *parent = op;
    ProjectionOperator *child;

    // build stack and then process bottom up
    while(isA((parent = OP_LCHILD(parent)),ProjectionOperator))
    {
        // only one parent of child is allowed in merging
        if (LIST_LENGTH(((QueryOperator *) parent)->parents) > 1)
            break;

        stack = appendToTailOfList(stack, parent);
    }

    // merge one child parent pair at a time
    child = (ProjectionOperator *) popTailOfListP(stack);
    parent = (ProjectionOperator *) popTailOfListP(stack);

    while(!LIST_EMPTY(stack))
    {
        Vector *opRefCount;
        Vector *childRefCount;

        // calculate child reference count vectors needed to determine safety
        opRefCount = calculateChildAttrRefCnts(parent, (QueryOperator *) child);
        childRefCount = calculateChildAttrRefCnts(child, OP_LCHILD(child));

        if (!isMergeSafe(opRefCount, childRefCount))
            break;

        // combine expressions and link child's children to root
        state->op = child;
        state->projExpr = child->projExprs;

        START_TIMER("OptimizeModel - replace attrs with expr");
        parent->projExprs = (List *) replaceAttributeRefsMutator((Node *) parent->projExprs, state);
        parent->op.inputs = child->op.inputs;
        STOP_TIMER("OptimizeModel - replace attrs with expr");

        FOREACH(QueryOperator, el, parent->op.inputs)
            el->parents = replaceNode(el->parents, child, parent);

        // calculate new reference vector
        mergeReferenceVectors((QueryOperator *) parent, opRefCount, childRefCount);

        // clean up child
        child->projExprs = NULL;
        child->op.inputs = NULL;
//        deepFree(child);

        // get next parent
        child = parent;
        parent = (ProjectionOperator *) popTailOfListP(stack);
    }

//    while(isA(OP_LCHILD(op),ProjectionOperator))
//    {
//        Vector *opRefCount;
//        Vector *childRefCount;
//        ProjectionOperator *child = (ProjectionOperator *) OP_LCHILD(op);
//
//        // only one parent of child is allowed
//        if (LIST_LENGTH(child->op.parents) > 1)
//        	break;
//
//        // calculate child reference count vectors needed to determine safety
//        opRefCount = calculateChildAttrRefCnts(op, (QueryOperator *) child);
//        childRefCount = calculateChildAttrRefCnts(child, OP_LCHILD(op));
//
//        if (!isMergeSafe(opRefCount, childRefCount))
//            break;
//
//        // combine expressions and link child's children to root
//        state->op = child;
//        state->projExpr = child->projExprs;
//
//        START_TIMER("OptimizeModel - replace attrs with expr");
//        op->projExprs = (List *) replaceAttributeRefsMutator((Node *) op->projExprs, state);
//        op->op.inputs = child->op.inputs;
//        STOP_TIMER("OptimizeModel - replace attrs with expr");
//
//        FOREACH(QueryOperator, el, op->op.inputs)
//        	el->parents = replaceNode(el->parents, child, op);
//
//        // calculate new reference vector
//        mergeReferenceVectors((QueryOperator *) op, opRefCount, childRefCount);
//
//        // clean up child
//        child->projExprs = NULL;
//        child->op.inputs = NULL;
////        deepFree(child);
//    }

    state->op = NULL;
    state->projExpr = NULL;
    FREE(state);

    return op;
}

static boolean
isMergeSafe(Vector *opRef, Vector *childRef)
{
    FOREACH_VEC(Vector,curV,opRef)
    {
        int *curA = VEC_TO_IA(*curV);

        for(int i = 0; i < VEC_LENGTH(*curV); i++)
        {
            Vector *childV;
            int refCount = curA[i];

            // more than one reference? then check number of attr refs in child
            if (refCount > 1)
            {
                childV = (Vector *) getVecNode(childRef, i);
                FOREACH_VEC_INT(childCount,childV)
                {
                    if (*childCount > 1)
                        return FALSE;
                }
            }
        }
    }

    return TRUE;
}

static void
mergeReferenceVectors(QueryOperator *o, Vector *opRef, Vector *childRef)
{
    int numGChildAttrs = getNumAttrs(OP_LCHILD(OP_LCHILD(o)));
    int numChildAttrs = getNumAttrs(OP_LCHILD(o));
    int numAttrs = getNumAttrs(o);
    Vector *result = makeVector(VECTOR_NODE,T_Vector);

    // create 0 initial new vector
    for(int i = 0; i < numAttrs; i++)
        vecAppendNode(result, (Node *) makeVectorIntSeq(0,numChildAttrs,0));

    // multiply child count vectors for referenced attributes by reference counts
    // for each parent attribute
    for(int i = 0; i < numAttrs; i++)
    {
        int *resultA = VEC_TO_IA(getVecNode(result, i));
        int *parentA = VEC_TO_IA(getVecNode(opRef, i));

        // substitute each child reference count with the according reference
        // count vector in the child. For example if the parent vector for one expression
        // is [3,0,1] and the child vectors are [[1,2],[1,0],[2,0]], then the result
        // vector for this expression would be [5,6].

        for(int j = 0; j < numChildAttrs; j++)
        {
            int *childA = VEC_TO_IA(getVecNode(childRef, j));
            int refC = resultA[j];

            for(int k = 0; k < numGChildAttrs; k++)
                resultA[k] = refC * childA[k];
        }
    }

    deepFreeVec(opRef);
    deepFreeVec(childRef);

    SET_STRING_PROP(o, PROP_MERGE_ATTR_REF_CNTS, result);
}


QueryOperator *
pushDownSelectionWithProjection(SelectionOperator *op)
{
	ReplaceRefState *state = NEW(ReplaceRefState);
	QueryOperator *newRoot = (QueryOperator *) op;
	QueryOperator *cur = op;
    ProjectionOperator *child = NULL, *parent = NULL;
	List *stack = singleton(op);

	// build stack and then process bottom up
	while(isA((cur = OP_LCHILD(cur)),ProjectionOperator))
	{
        // only one parent of child is allowed in merging
        if (LIST_LENGTH(cur->parents) > 1)
            break;

	    stack = appendToTailOfList(stack, cur);
	}

	// merge one child parent pair at a time
	child = (ProjectionOperator *) popTailOfListP(stack);
	parent = (ProjectionOperator *) popTailOfListP(stack);

	while(!LIST_EMPTY(stack))
    {
        QueryOperator *grandChild = OP_LCHILD(child);
        List *oldP = parent->op.parents;

//        // set newRoot
//        if (newRoot == (QueryOperator *) parent)
//            newRoot = (QueryOperator *) child;

        // combine expressions and link child's children to root
        state->op = child;
        state->projExpr = child->projExprs;

        // change selection
        parent->cond = replaceAttributeRefsMutator(op->cond, state);
        parent->op.schema = copyObject(grandChild->schema);
        parent->op.inputs = child->op.inputs;
        parent->op.parents = singleton(child);

        // push down selection adapt projection
        child->op.inputs = singleton(op);
        child->op.parents = oldP;

        // replace projection with selection in parents of grandchild
        grandChild->parents = replaceNode(grandChild->parents, child, op);

        // replace selection with projection in previous selection parents
        FOREACH(QueryOperator, el, oldP)
            el->inputs = replaceNode(el->inputs, op, child);
    }


//
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

	return op;
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

static Vector *
calculateChildAttrRefCnts(ProjectionOperator *op, QueryOperator *child)
{
    int numOpAttrs = getNumAttrs((QueryOperator *) op);
    int numChildAttrs = getNumAttrs((QueryOperator *) child);
    int i = 0;

    if (HAS_STRING_PROP(op, PROP_MERGE_ATTR_REF_CNTS))
        return (Vector *) GET_STRING_PROP(op, PROP_MERGE_ATTR_REF_CNTS);

    START_TIMER("OptimizeModel - check safety of projection merge");

    // referenceCount 2-dim vector. For each projection expression keep track how many times it references which child attr
    Vector *refCount = makeVectorOfSize(VECTOR_NODE,T_Vector,numOpAttrs);

    for(i = 0; i < numOpAttrs; i++)
        VEC_ADD_NODE(refCount,makeVectorIntSeq(0,numChildAttrs,0));

    i = 0;
    FOREACH(Node,p,op->projExprs)
    {
        DEBUG_LOG("projection expression: %s", beatify(nodeToString(p)));
        Vector *curCount = (Vector *) getVecNode(refCount,i++);

        getAttrRefCount(p, curCount);
    }

    SET_STRING_PROP(op, PROP_MERGE_ATTR_REF_CNTS, refCount);

    STOP_TIMER("OptimizeModel - check safety of projection merge");

    return refCount;
}

static inline void
getAttrRefCount(Node *expr, Vector *cnts)
{
    List *attrRef = getAttrReferences(expr);
    int *cntA = VEC_TO_IA(cnts);

    FOREACH(AttributeReference,a,attrRef)
    {
        cntA[a->attrPosition]++;
        DEBUG_LOG("attribute %s count is %u", a->name, cntA[a->attrPosition]);
    }
}
