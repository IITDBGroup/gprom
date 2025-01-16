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
#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/schema_utility.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "mem_manager/mem_mgr.h"
#include "operator_optimizer/operator_merge.h"


/* state */
typedef struct ReplaceRefState {
    Vector *projExpr;
    Vector *refCount;
    ProjectionOperator *op;
} ReplaceRefState;

/* declarations */
//static Node *replaceAttributeRefsMutator (Node *node, ReplaceRefState *state);
static boolean replaceAttributeRefsMutator (Node *node, ReplaceRefState *state, void **parentPointer);
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
        op->cond = (Node *) createOpExpr(OPNAME_AND, LIST_MAKE(op->cond, child->cond));
        op->op.inputs = child->op.inputs;

        FOREACH(QueryOperator, el, op->op.inputs)
             el->parents = child->op.parents;
        //el->parents = copyObject(child->op.parents);

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
    QueryOperator *cur = OP_LCHILD(op);
    ProjectionOperator *parent;
    ProjectionOperator *child;

    while(isA(cur,ProjectionOperator))
    {
        stack = appendToHeadOfList(stack, cur);
        cur = OP_LCHILD(cur);
    }

    // merge one child parent pair at a time
    if (!MY_LIST_EMPTY(stack))
        parent = (ProjectionOperator *) popHeadOfListP(stack);

    while(!MY_LIST_EMPTY(stack))
    {
        Vector *opRefCount;
        Vector *childRefCount;

        // get next parent
        child = parent;
        parent = (ProjectionOperator *) popHeadOfListP(stack);

        ASSERT(isA(parent,ProjectionOperator) && isA(child, ProjectionOperator));

        // calculate child reference count vectors needed to determine safety
        opRefCount = calculateChildAttrRefCnts(parent, (QueryOperator *) child);
        childRefCount = calculateChildAttrRefCnts(child, OP_LCHILD(child));
        DEBUG_NODE_BEATIFY_LOG("reference counts:", opRefCount, childRefCount);

		// if merging this projections would blow up expression size, then don't do it unless -Omerge_unsafe_proj has been set
        if (!isMergeSafe(opRefCount, childRefCount) && !opt_optimization_merge_unsafe_proj)
            break;

        // combine expressions and link child's children to root
        state->op = child;
        state->projExpr = makeVectorFromList(child->projExprs);
        state->refCount = makeVectorIntSeq(0,VEC_LENGTH(state->projExpr),0);

        // calculate new reference vector
        mergeReferenceVectors((QueryOperator *) parent, opRefCount, childRefCount);
        DEBUG_NODE_BEATIFY_LOG("merged reference count:",
                GET_STRING_PROP(parent,PROP_MERGE_ATTR_REF_CNTS));

        START_TIMER("OptimizeModel - replace attrs with expr");
        replaceAttributeRefsMutator((Node *) parent->projExprs, state, NULL);
        STOP_TIMER("OptimizeModel - replace attrs with expr");

        if(HAS_STRING_PROP(child, PROP_PROJ_PROV_ATTR_DUP))
        {
            if(GET_BOOL_STRING_PROP(child, PROP_PROJ_PROV_ATTR_DUP) == TRUE)
                SET_BOOL_STRING_PROP((QueryOperator *)parent, PROP_PROJ_PROV_ATTR_DUP);
        }
        parent->op.inputs = child->op.inputs;
        FOREACH(QueryOperator, el, parent->op.inputs)
            el->parents = replaceNode(el->parents, child, parent);

        // clean up child
        child->projExprs = NULL;
        child->op.inputs = NULL;
//        deepFree(child);

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
            ASSERT(checkModel((QueryOperator *) parent));
    }

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
        int *curA = VEC_TO_IA(curV);

        for(int i = 0; i < VEC_LENGTH(curV); i++)
        {
            Vector *childV;
            int refCount = curA[i];

            // more than one reference? then check number of attr refs in child
            if (refCount > 1)
            {
                childV = (Vector *) getVecNode(childRef, i);
                FOREACH_VEC_INT(childCount,childV)
                {
                    if (childCount > 1)
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
        vecAppendNode(result, (Node *) makeVectorIntSeq(0,numGChildAttrs,0));

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
            int refC = parentA[j];

            DEBUG_LOG("ref count: <%u> for child: %s", refC, nodeToString(getVecNode(childRef, j)));

            for(int k = 0; k < numGChildAttrs; k++)
                resultA[k] += refC * childA[k];
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
        state->projExpr = makeVectorFromList(child->projExprs);
        state->refCount = makeVectorIntSeq(0,VEC_LENGTH(state->projExpr),0);

        // change selection
//        op->cond =
        replaceAttributeRefsMutator(op->cond, state, (void **) &(op->cond));
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


//static Node *
//replaceAttributeRefsMutator (Node *node, ReplaceRefState *state)
//{
//    if (node == NULL)
//        return NULL;
//
//    if (isA(node, AttributeReference))
//    {
//        AttributeReference *a = (AttributeReference *) node;
//        int pos = getAttributeNum(a->name, (QueryOperator *) state->op);
//        deepFree(a);
//
//        return copyObject(getNthOfListP(state->projExpr, pos));
//    }
//
//    return mutate(node, replaceAttributeRefsMutator, state);
//}

static boolean
replaceAttributeRefsMutator (Node *node, ReplaceRefState *state, void **parentPointer)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        Node **parentP = (Node **) parentPointer;
        AttributeReference *a = (AttributeReference *) node;
        int pos = getAttributeNum(a->name, (QueryOperator *) state->op);

        // do not copy if not necessary
        if (VEC_TO_IA(state->refCount)[pos] == 0)
        {
            DEBUG_LOG("usage count is 0 for %s - do not copy", a->name ? a->name : "NULL");
            *parentP = VEC_TO_ARR(state->projExpr,Node)[pos];
            VEC_TO_IA(state->refCount)[pos] = 1;
        }
        else
        {
            DEBUG_LOG("%s has been used before - copy", a->name ? a->name : "NULL");
            *parentP = copyObject(VEC_TO_ARR(state->projExpr,Node)[pos]);
        }
        deepFree(a);

        return TRUE;
    }

    return visitWithPointers(node, replaceAttributeRefsMutator, parentPointer, state);
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
        DEBUG_NODE_BEATIFY_LOG("projection expression:", p);
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
