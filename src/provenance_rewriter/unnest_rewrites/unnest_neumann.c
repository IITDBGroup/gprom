/*
 * unnest_neumann.c
 *      Implements the rewrites in https://btw-2015.informatik.uni-hamburg.de/res/proceedings/Hauptband/Wiss/Neumann-Unnesting_Arbitrary_Querie.pdf
 *      TODO: Use https://doi.org/10.18420/BTW2025-01
 *
 *      Author: Felix
 */

#include "common.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "operator_optimizer/operator_optimizer.h"
#include "provenance_rewriter/unnest_rewrites/unnest_main.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"

#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"

#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "utility/string_utils.h"

// static boolean allFreeContained (QueryOperator *);
static QueryOperator *neumanningInternal (QueryOperator *op, List *correlated);
static QueryOperator *bindingTransform (NestingOperator *op);

// functions for pushdown of nesting operator into RHS
static QueryOperator *neumannPushdown(NestingOperator *op);
static QueryOperator *neumannPushdownSelection(NestingOperator *op, SelectionOperator *c);
static QueryOperator *neumannPushdownProjection(NestingOperator *op, ProjectionOperator *c);
static QueryOperator *neumannPushdownAggregation(NestingOperator *op, AggregationOperator *c);
static QueryOperator *neumannPushdownJoin(NestingOperator *op, JoinOperator *c);
static QueryOperator *neumannPushdownSet(NestingOperator *op, SetOperator *c);
static QueryOperator *neumannPushdownDupRem(NestingOperator *op, DuplicateRemoval *c);
static QueryOperator *neumannPushdownNesting(NestingOperator *op, NestingOperator *c);

static void pushOperatorThroughUnary(NestingOperator *n, QueryOperator *child);
static void pushOperatorThroughBinary(NestingOperator *n, QueryOperator *child, boolean pushLeft, boolean pushRight);
static Set *determineFreeAttributes(QueryOperator *op);

QueryOperator *
neumanning(QueryOperator *op)
{
    List *correlated = NIL;
    return neumanningInternal(op, correlated);
}

/**
 * @brief Method implementing the unnesting rewrites from Neumann et al.
 * First decorrelates and unnests the children of an operator and then the operator itself.
 *
 * @param op current operator to process
 * @param correlated ???
 * @return rewritten operator
 */

static QueryOperator *
neumanningInternal(QueryOperator *op, List *correlated)
{
    FOREACH(QueryOperator, child, op->inputs)
    {
        neumanningInternal(child, correlated);
    }

    // TODO
    if(isA(op, NestingOperator))
    {
        QueryOperator *newroot;
        NestingOperator *n = (NestingOperator *) op;

        // create correlation with unique bindings and join
        newroot = bindingTransform((NestingOperator*)op);

        // determine free attributes for each operator (correlated referenced
        // below the operator and in the operator's parameters
        determineFreeAttributes(OP_RCHILD(op));

        // pushdown nesting operator into RHS
        newroot = neumannPushdown(n);
        INFO_OP_LOG("After nesting operator pushdown", newroot);

        // adjust correlated attribute references, reducing outerlevels up by one
        List *corrAttrs = getCorrelatedAttrReferences((Node *) newroot, TRUE);

        FOREACH(AttributeReference,a,corrAttrs) // FIXME if we keep this a nesting operator we should not do that, also then the invariant will fail.
        {
            (a->outerLevelsUp)--;
        }

        INFO_OP_LOG("After reducing nesting level of correlated attribute references", newroot);

        return newroot;
    }

    return op;
}

static QueryOperator *
neumannPushdown(NestingOperator *op)
{
    QueryOperator *rightChild = OP_RCHILD(op);

    INFO_LOG("PUSHING AT %s", singleOperatorToOverview(rightChild));

    // no more correlations below? Then we are done
    if(!HAS_STRING_PROP(rightChild, PROP_FREE_ATTRS))
    {
        INFO_LOG("Done pushing nesting operator at %s", singleOperatorToOverview(rightChild));

        // LATERAL or scalar subquery, replace with regular join
        if(op->nestingType == NESTQ_LATERAL || op->nestingType == NESTQ_SCALAR)
        {
            QueryOperator *l = OP_LCHILD(op);
            QueryOperator *r = OP_RCHILD(op);
            JoinOperator *j = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(l,r), NIL, NIL);

            replaceParent(l, (QueryOperator *) op, (QueryOperator *) j);
            replaceParent(r, (QueryOperator *) op, (QueryOperator *) j);

            // replace op with j
            switchSubtreeWithExisting((QueryOperator *) op,
                                      (QueryOperator *) j);

            INFO_OP_LOG("Replace nesting operator with uncorrelated join", j);

            return (QueryOperator *) j;
        }

        INFO_OP_LOG("Pushed nesting operator below all correlations", op);
        return (QueryOperator *) op;
    }

    switch(rightChild->type)
    {
    case T_SelectionOperator:
        return neumannPushdownSelection(op, (SelectionOperator *) rightChild);
    case T_ProjectionOperator:
        return neumannPushdownProjection(op, (ProjectionOperator *) rightChild);
    case T_AggregationOperator:
        return neumannPushdownAggregation(op, (AggregationOperator *) rightChild);
    case T_JoinOperator:
        return neumannPushdownJoin(op, (JoinOperator *) rightChild);
    case T_SetOperator:
        return neumannPushdownSet(op, (SetOperator *) rightChild);
    case T_DuplicateRemoval:
        return neumannPushdownDupRem(op, (DuplicateRemoval *) rightChild);
    case T_NestingOperator:
        return neumannPushdownNesting(op, (NestingOperator *) rightChild);
    case T_TableAccessOperator:
        return rightChild;
    // TODO remaining operators
    default:
        THROW(SEVERITY_RECOVERABLE,
              "Neumann pushdown not supported yet for this operator type %s",
              singleOperatorToOverview(rightChild));
    }

    return (QueryOperator *) rightChild;
}

#define LOG_PRE_PUSH(_parent,_child) \
        INFO_LOG("****************************************\nPUSH[%s] operator %s\nthrough\n%s", NodeTagToString(_child->op.type), singleOperatorToOverview(_parent), singleOperatorToOverview(_child))
#define LOG_POST_PUSH(_op) \
INFO_LOG("****************************************\nPUSH-DONE[%s] Pushed through operator:\n%s", NodeTagToString(_op->op.type), operatorToOverviewString(_op))

/**
 * @brief Pushdown nesting operator through an unary operator.
 *
 * This method only changes the algebra graph structure and adjust the nesting
 * operators schema. It is the responsibility of the caller to fix anything
 * else.
 *
 * @param n nesting operator to push down
 * @param child child to push through
 */

static void
pushOperatorThroughUnary(NestingOperator *n, QueryOperator *child)
{
    QueryOperator *grandchild = OP_LCHILD(child);
    QueryOperator *nest = (QueryOperator *) n;

    removeParent(grandchild, child);
    removeChild(child,grandchild);
    removeParent(child, nest);
    removeChild(nest, child);
    addChildOperator(nest, grandchild);
    addChildOperator(child, nest);

    switchSubtreeWithExisting(nest, child);

    // adjust schema of nesting operator
    adaptSchemaFromChildren((QueryOperator *) n);

    INFO_OP_LOG("After pushing the nesting operator (tree structure only)", child);
}

static void
pushOperatorThroughBinary(NestingOperator *n, QueryOperator *child, boolean pushLeft, boolean pushRight)
{
    // TODO
    INFO_OP_LOG("After pushing the nesting operator (tree structure only)", child);
}


static QueryOperator *
neumannPushdownSelection(NestingOperator *op, SelectionOperator *c)
{
    LOG_PRE_PUSH(op,c);

    // just switch selection with nesting operator
    pushOperatorThroughUnary(op, (QueryOperator *) c);

    // adjust schema and attribute references
    adaptSchemaFromChildren((QueryOperator *) c);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c,
                                            (QueryOperator *) op,
                                            NULL);

    LOG_POST_PUSH(c);

    neumannPushdown(op);
    return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownProjection(NestingOperator *op, ProjectionOperator *c)
{
    QueryOperator *D = OP_LCHILD(op);
    List *newProjs = getProjExprsForAllAttrs(D);

    LOG_PRE_PUSH(op,c);

    // just switch selection with nesting operator
    pushOperatorThroughUnary(op, (QueryOperator *) c);

    // what we have to project on depends on the type of nesting operator
    // 1) for exists just project on all attributes from the nesting operator
    if(op->nestingType == NESTQ_EXISTS)
    {
        c->projExprs = getProjExprsForAllAttrs((QueryOperator *) op);
        c->op.schema->attrDefs = copyObject(op->op.schema->attrDefs);
    }
    // 2) for LATERAL AND SCALAR
    else if (op->nestingType == NESTQ_LATERAL || op->nestingType == NESTQ_SCALAR)
    {
        // add attributes of D to projection expressions
        c->projExprs = CONCAT_LISTS(newProjs,c->projExprs);
        c->op.schema->attrDefs = CONCAT_LISTS(copyObject(D->schema->attrDefs),
                                              c->op.schema->attrDefs);

        // adjust attribute references
        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c,
                                                (QueryOperator *) op,
                                                NULL);
    }
    else
    {
        THROW(SEVERITY_RECOVERABLE,
              "nesting type not supported yet by projection %s",
              singleOperatorToOverview(c));
    }

    LOG_POST_PUSH(c);
    neumannPushdown(op);

    return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownAggregation(NestingOperator *op, AggregationOperator *c)
{
    QueryOperator *D = OP_LCHILD(op);
    List *newGroupBy = getProjExprsForAllAttrs(D);
    List *newGroupNyNames = getQueryOperatorAttrNames((QueryOperator *) D);
    List *origAggAttrsNames = getQueryOperatorAttrNames((QueryOperator *) c);
    List *dAttrNames = getQueryOperatorAttrNames(D);
    LOG_PRE_PUSH(op,c);

    // just switch selection with nesting operator
    pushOperatorThroughUnary(op, (QueryOperator *) c);

    // add D's attributes as group-by attributes
    c->groupBy = CONCAT_LISTS(c->groupBy, newGroupBy);
    c->op.schema->attrDefs = CONCAT_LISTS(c->op.schema->attrDefs,
                                          copyObject(D->schema->attrDefs));
    adaptSchemaFromChildren((QueryOperator *) c);

    // add projection to reorder attributes and restore names of D attributes
    QueryOperator *proj;
    List *newNames = CONCAT_LISTS(deepCopyStringList(dAttrNames),
                                  deepCopyStringList(origAggAttrsNames));
    List *projAttrs = CONCAT_LISTS(deepCopyStringList(newGroupNyNames),
                                   deepCopyStringList(origAggAttrsNames));
    proj = createProjOnAttrsByName((QueryOperator *) c, projAttrs, newNames);
    proj->inputs = singleton(c);
    switchSubtreeWithExisting((QueryOperator *) c, proj);
    c->op.parents = singleton(proj);

    // pushdown into grandchild
    LOG_POST_PUSH(c);
    neumannPushdown(op);

    return (QueryOperator *) proj;
}

static QueryOperator *
neumannPushdownJoin(NestingOperator *op, JoinOperator *c)
{
    //QueryOperator *D = OP_LCHILD(op);
    QueryOperator *left = OP_LCHILD(c);
    QueryOperator *right = OP_RCHILD(c);
    boolean pushLeft, pushRight;

    LOG_PRE_PUSH(op,c);

    pushLeft = HAS_STRING_PROP(left, PROP_FREE_ATTRS) ||
        (c->joinType == JOIN_LEFT_OUTER
         || c->joinType == JOIN_FULL_OUTER);
    pushRight = HAS_STRING_PROP(right, PROP_FREE_ATTRS) ||
        (c->joinType == JOIN_RIGHT_OUTER
         || c->joinType == JOIN_FULL_OUTER);

    // structural change
    pushOperatorThroughBinary(op, (QueryOperator *) c, pushLeft, pushRight);

    // which sides to push into
    if(pushLeft && pushRight)
    {
        // TODO
    }
    else if (pushLeft)
    {

    }
    else if (pushRight)
    {

    }

    LOG_POST_PUSH(c);

    return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownSet(NestingOperator *op, SetOperator *c)
{
   LOG_PRE_PUSH(op,c);
   // TODO
   LOG_POST_PUSH(c);

   return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownDupRem(NestingOperator *op, DuplicateRemoval *c)
{
   LOG_PRE_PUSH(op,c);
   // TODO
   LOG_POST_PUSH(c);

   return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownNesting(NestingOperator *op, NestingOperator *c)
{
   LOG_PRE_PUSH(op,c);
   // TODO
   LOG_POST_PUSH(c);

   return (QueryOperator *) c;
}

static Set *
determineFreeAttributes(QueryOperator *op)
{
    Set *freeattrs = STRSET();

    FOREACH(QueryOperator,c,op->inputs)
    {
        freeattrs = unionSets(freeattrs,determineFreeAttributes(c));
    }

    List *corrA = getCorrelatedAttrRefsInOperator(op);
    FOREACH(AttributeReference,a,corrA)
    {
        if(a->outerLevelsUp == 1)
        {
            addToSet(freeattrs, strdup(a->name));
        }
    }

    if(!HAS_STRING_PROP(op, PROP_FREE_ATTRS) && !EMPTY_SET(freeattrs))
    {
        SET_STRING_PROP(op, PROP_FREE_ATTRS, freeattrs);
    }

    return freeattrs;
}


/**
 * @brief Rewrite the nesting operator into a join and a nesting operator with the unique bindings for correlated attribute references.
 *
 * Let D be the correlated attributes from Q1 referenced in Q2. The rewrite is:
 *
 * (Q1 NEST Q2) -> (Q1 NATURAL JOIN_D (DUPREM(PROJECTION[D](Q1)) NEST Q2))
 *
 * @param op the nesting operator to transform
 * @return the generated join operator
 */

static QueryOperator *
bindingTransform(NestingOperator *op)
{
    Set *correlated = getNestingCorrelatedAttributeNames(op, FALSE); // get correlated attribute names
    List *D_attrs = makeStrListFromSet(correlated);
    List *D_newnames = NIL; // new attribute names to avoid name clashes
    List *corr_arefs = getNestingCorrelatedAttrReferences(op, FALSE); // actual attribute references
    HashMap *oldToNewNames = NEW_MAP(Constant,Constant);
    QueryOperator *left = OP_LCHILD(op);
    QueryOperator *right = OP_RCHILD(op);
    QueryOperator *join;

    // sort correlated attribute names from the LHS
    D_attrs = sortList(D_attrs,
                       (int (*) (const void **, const void **)) strCompare);

    // add prefix to attribute names
    FOREACH(char,name,D_attrs)
    {
        char *newname = CONCAT_STRINGS("__corr_", strdup(name));
        D_newnames = appendToTailOfList(D_newnames,
                                        newname);
        MAP_ADD_STRING_KEY_AND_VAL(oldToNewNames, name, newname);
    }

    // adjust actual attribute references
    FOREACH(AttributeReference,a,corr_arefs)
    {
        char *newname = MAP_GET_STRING_VAL_FOR_STRING_KEY(oldToNewNames, a->name);
        DEBUG_LOG("Replace %s with %s in:\n%s",
                  a->name,
                  newname,
                  a);
        a->name = strdup(newname);
    }

    INFO_OP_LOG("adjusted nested subquery references: ", op);

    // project onto correlated attribute names
    ProjectionOperator *proj = (ProjectionOperator *) createProjOnAttrsByName(left,
                                                                              D_attrs,
                                                                              D_newnames);
    addChildOperator((QueryOperator *) proj, left);
    removeParent(left, (QueryOperator *) op);

    // add duplicate removal operator to create unique bindings for correlated attributes D
    DuplicateRemoval *D = createDuplicateRemovalOp(NULL,
                                                   (QueryOperator*)proj,
                                                   singleton(op),
                                                   NIL);
    ((QueryOperator*)proj)->parents = singleton(D);

    // nesting operator now should have D and the original right input of nesting operator op as children
    op->op.inputs = LIST_MAKE(D,right);
    // adjust nesting operator's schema from original LHS to D
    List *nestingAttrs = sublist(copyList(op->op.schema->attrDefs), getNumAttrs(left), -1);
    op->op.schema->attrDefs = CONCAT_LISTS(copyList(D->op.schema->attrDefs), nestingAttrs);
    INFO_OP_LOG("replaced left input with D: ", op);

    // create join between original left input of nesting operator and the nesting operator itself
    // join condition is D = renamed(D)
    Node *cond = createEqualityJoinCond(left,
                                        (QueryOperator *) op,
                                        D_attrs,
                                        D_newnames,
                                        TRUE); // TODO check whether all these attributes are not nullable and use equality if this is the case.
    join = (QueryOperator *) createJoinOp(JOIN_INNER, cond, LIST_MAKE(left, op), NIL, NIL);

    /* join->parents= op->op.parents; */
    addParent(left, join);

    // create projection to remove duplicated correlated attributes from join
    List *resattrs = CONCAT_LISTS(getQueryOperatorAttrNames(left),
                                  sublist(copyList(getQueryOperatorAttrNames((QueryOperator *) op)),
                                          getNumAttrs((QueryOperator *) D), -1));

    QueryOperator *joinproj = createProjOnAttrsByName(join, resattrs, NIL);
    addChildOperator(joinproj, join);
    switchSubtreeWithExisting((QueryOperator *) op, joinproj);
    op->op.parents = singleton(join);

    INFO_OP_LOG("Join between original LHS of nesting operator and new nesting operator.", join);

    return (QueryOperator*) join;
}
