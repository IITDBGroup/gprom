/*
 * unnest_neumann.c
 *      Implements the rewrites in https://btw-2015.informatik.uni-hamburg.de/res/proceedings/Hauptband/Wiss/Neumann-Unnesting_Arbitrary_Querie.pdf
 *      TODO: Use https://doi.org/10.18420/BTW2025-01
 *
 *      Author: Felix
 */

#include "common.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "operator_optimizer/operator_optimizer.h"
#include "operator_optimizer/optimizer_prop_inference.h"
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
static void adjustCorrAttributes(QueryOperator *op, QueryOperator *D);

// functions for pushdown of nesting operator into RHS
static QueryOperator *neumannPushdown(NestingOperator *op);
static QueryOperator *neumannPushdownSelection(NestingOperator *op, SelectionOperator *c);
static QueryOperator *neumannPushdownProjection(NestingOperator *op, ProjectionOperator *c);
static QueryOperator *neumannPushdownAggregation(NestingOperator *op, AggregationOperator *c);
static QueryOperator *neumannPushdownJoin(NestingOperator *op, JoinOperator *c);
static QueryOperator *neumannPushdownSet(NestingOperator *op, SetOperator *c);
static QueryOperator *neumannPushdownDupRem(NestingOperator *op, DuplicateRemoval *c);
static QueryOperator *neumannPushdownNesting(NestingOperator *op, NestingOperator *c);

static List *rhsDattrNames(QueryOperator *D);
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
        QueryOperator *D;
        boolean isCorrelated;

        if(!IS_LATERAL(n))
        {
            THROW(SEVERITY_RECOVERABLE,
                  "Neumanning only supported for LATERAL subqueries, but got this non-lateral one as input:\n\n%s",
                  operatorToOverviewString(op));
        }

        // create correlation with unique bindings and join
        newroot = bindingTransform((NestingOperator*)op);

        // determine free attributes for each operator (correlated referenced
        // below the operator and in the operator's parameters
        removeProp(OP_RCHILD(op), PROP_FREE_ATTRS);
        determineFreeAttributes(OP_RCHILD(op));
        isCorrelated = HAS_STRING_PROP(OP_RCHILD(op), PROP_FREE_ATTRS);

        // pushdown nesting operator into RHS
        newroot = neumannPushdown(n);
        INFO_OP_LOG("After nesting operator pushdown", newroot);

        // adjust correlated attribute references, reducing outerlevels up by one and changing position based on ordering of attributes in D
        if(isCorrelated)
        {
            D = OP_LCHILD(op);
            adjustCorrAttributes(newroot, D);
        }

        INFO_OP_LOG("After reducing nesting level of correlated attribute references", newroot);

        return newroot;
    }

    return op;
}

static void
adjustCorrAttributes(QueryOperator *op, QueryOperator *D)
{
    List *corrAttrs = getCorrelatedAttrReferences((Node *) op, TRUE);
    HashMap *dattrPos = NEW_MAP(Constant, Constant);

    // map D attribute names to positions
    FOREACH(AttributeReference,a,getProjExprsForAllAttrs(D))
    {
        MAP_ADD_STRING_KEY(dattrPos, a->name, createConstInt(a->attrPosition));
    }

    FOREACH(AttributeReference,a,corrAttrs) // FIXME if we keep this a nesting operator we should not do that, also then the invariant will fail.
    {
        if(a->outerLevelsUp == 1)
        {
            a->attrPosition = INT_VALUE(MAP_GET_STRING(dattrPos, a->name));
        }
        (a->outerLevelsUp)--;
    }
}


static QueryOperator *
neumannPushdown(NestingOperator *op)
{
    QueryOperator *rightChild = OP_RCHILD(op);
    ASSERT(IS_LATERAL(op));
    INFO_LOG("PUSHING AT %s", singleOperatorToOverview(rightChild));

    // no more correlations below? Then we are done
    if(!HAS_STRING_PROP(rightChild, PROP_FREE_ATTRS))
    {
        INFO_LOG("Done pushing nesting operator at %s", singleOperatorToOverview(rightChild));
        JoinType jt = JOIN_CROSS; // (op->nestingType == NESTQ_LEFT_LATERAL) ? JOIN_LEFT_OUTER : JOIN_CROSS;
        Node *cond = (jt == JOIN_LEFT_OUTER) ? (Node *) createConstBool(TRUE) : NULL;

        QueryOperator *l = OP_LCHILD(op);
        QueryOperator *r = OP_RCHILD(op);
        JoinOperator *j = createJoinOp(jt, cond, LIST_MAKE(l,r), NIL, NIL);

        replaceParent(l, (QueryOperator *) op, (QueryOperator *) j);
        replaceParent(r, (QueryOperator *) op, (QueryOperator *) j);

        // replace op with j
        switchSubtreeWithExisting((QueryOperator *) op,
                                  (QueryOperator *) j);

        INFO_OP_LOG("Replace nesting operator with uncorrelated join", j);

        return (QueryOperator *) j;
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

    disconnectParentChild(child, grandchild);
    disconnectParentChild(nest, child);
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
    QueryOperator *leftGC = OP_LCHILD(child);
    QueryOperator *rightGC = OP_RCHILD(child);
    QueryOperator *nest = (QueryOperator *) n;

    disconnectParentChild(nest, child);

    if(pushLeft && pushRight)
    {
        QueryOperator *nCopy = shallowCopyQueryOperator(nest);
        QueryOperator *D = OP_LCHILD(n);

        // disconnect left and right input of join
        disconnectParentChild(child, leftGC);
        disconnectParentChild(child, rightGC);

        // connect left input to original nesting operator
        addChildOperator(nest, leftGC);
        addChildOperator(child, nest);

        // connect D and right input to copy of nesting operator
        addChildOperator(nCopy, D);
        addChildOperator(nCopy, rightGC);
        addChildOperator(child, nCopy);

        // adjust schema of nesting operator
        adaptSchemaFromChildren((QueryOperator *) nCopy);
    }
    else if(pushLeft)
    {
        disconnectParentChild(child, leftGC);
        addChildOperator(nest, leftGC);
        addChildOperator(child, nest);
    }
    else if(pushRight)
    {
        disconnectParentChild(child, rightGC);
        addChildOperator(nest, rightGC);
        addChildOperator(child, nest);
    }

    // original nesting operator is always pushed (sometimes left, sometimes right)
    switchSubtreeWithExisting(nest, child);

    // adjust schema of nesting operator
    adaptSchemaFromChildren((QueryOperator *) n);

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

    c->projExprs = CONCAT_LISTS(newProjs,c->projExprs);
    c->op.schema->attrDefs = CONCAT_LISTS(copyObject(D->schema->attrDefs),
                                          c->op.schema->attrDefs);

    // adjust attribute references
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c,
                                            (QueryOperator *) op,
                                            NULL);

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
    /* List *origGBAttrNames = aggOpGetGroupByAttrNames(c); */
    List *origAggAttrsNames = getQueryOperatorAttrNames((QueryOperator *) c);
    List *dAttrNames = getQueryOperatorAttrNames(D);
    QueryOperator *proj;
    boolean grpby = isGroupBy(c);
    boolean hasAgg = LIST_LENGTH(c->aggrs) > 0;
    LOG_PRE_PUSH(op,c);

    // just switch selection with nesting operator
    pushOperatorThroughUnary(op, (QueryOperator *) c);

    // add D's attributes as group-by attributes
    c->groupBy = CONCAT_LISTS(c->groupBy, newGroupBy);
    c->op.schema->attrDefs = CONCAT_LISTS(c->op.schema->attrDefs,
                                          copyObject(D->schema->attrDefs));
    adaptSchemaFromChildren((QueryOperator *) c);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c,
                                            (QueryOperator *) op,
                                            NULL);

    // if this is aggregation without group-by, we need to left-outer join with
    // D again to retain the semantics of aggregation without group-by even
    // though we group on D
    if(!grpby)
    {
        QueryOperator *j;
        List *dnames = getQueryOperatorAttrNames(D);
        List *aggNames = aggOpGetAggAttrNames(c);
        List *rhsDnames = rhsDattrNames(D);
        Node *newCond = createEqualityJoinCond((QueryOperator *) D,
                                               (QueryOperator *) c,
                                               dnames,
                                               dnames,
                                               TRUE); // TODO if not nullable use equality instead
        List *newAttrNames;

        newAttrNames = deepCopyStringList(dnames);
        if(hasAgg)
        {
            newAttrNames = CONCAT_LISTS(newAttrNames, aggNames);
        }
        newAttrNames = CONCAT_LISTS(newAttrNames, rhsDnames);

        // left join D with aggregation result
        j = (QueryOperator *) createJoinOp(JOIN_LEFT_OUTER, newCond, LIST_MAKE(D,c), NIL, newAttrNames);
        addParent(D, j);

        // projection to create aggregation function results
        List *projExprs;
        List *projAttrNames;
        /* List *gbproj = getProjExprsForAttrNames((QueryOperator *) c, */
        /*                                         origGBAttrNames); */
        List *aggproj = NIL;
        List *dproj = getProjExprsForAllAttrs(D);

        // if agg result is null then substitute with right val
        FORBOTH(void,
                ar,
                ad,
                getProjExprsForAttrNames((QueryOperator *) j,
                                         aggOpGetAggAttrNames(c)),
                c->aggrs)
        {
            AttributeReference *a = (AttributeReference *) ar;
            FunctionCall *agg = (FunctionCall *) ad;
            DataType aggResType = typeOf((Node *) agg);
            Node *defaultVal;
            Node *coalesceExpr;

            if(strieq(agg->functionname, COUNT_FUNC_NAME))
            {
                switch(aggResType)
                {
                case DT_LONG:
                    defaultVal = (Node *) createConstLong(0);
                break;
                case DT_INT:
                    defaultVal = (Node *) createConstInt(0);
                break;
                default:
                    defaultVal = NULL;
                    THROW(SEVERITY_RECOVERABLE,
                          "Do not support data type %s for count aggregate!",
                          DataTypeToString(aggResType));
                }
            }
            else
            {
                defaultVal = (Node *) createNullConst(aggResType);
            }

            coalesceExpr = (Node *) createFunctionCall(COALESCE_FUNC_NAME,
                                                       LIST_MAKE(a, defaultVal));
            aggproj = appendToTailOfList(aggproj, coalesceExpr);
        }

        projExprs = CONCAT_LISTS(dproj, aggproj);
        projAttrNames = CONCAT_LISTS(deepCopyStringList(dnames),
                                     aggOpGetAggAttrNames(c));

        proj = (QueryOperator *) createProjectionOp(projExprs, (QueryOperator *) c, NIL, projAttrNames);
        proj->inputs = singleton(j);
        j->parents = singleton(proj);
        switchSubtreeWithExisting((QueryOperator *) c, proj);
        c->op.parents = singleton(j);
        /* op->nestingType = NESTQ_LEFT_LATERAL; */
    }
    else
    {
        // add projection to reorder attributes and restore names of D attributes
        List *newNames = CONCAT_LISTS(deepCopyStringList(dAttrNames),
                                      deepCopyStringList(origAggAttrsNames));
        List *projAttrs = CONCAT_LISTS(deepCopyStringList(newGroupNyNames),
                                       deepCopyStringList(origAggAttrsNames));
        proj = createProjOnAttrsByName((QueryOperator *) c, projAttrs, newNames);
        proj->inputs = singleton(c);
        switchSubtreeWithExisting((QueryOperator *) c, proj);
        c->op.parents = singleton(proj);
    }

    // pushdown into grandchild
    LOG_POST_PUSH(c);
    neumannPushdown(op);

    return (QueryOperator *) proj;
}

#define RHS_D_PREFIX backendifyIdentifier("rhs__")

static List *
rhsDattrNames(QueryOperator *D)
{
    List *newdnames = deepCopyStringList(getQueryOperatorAttrNames(D));

    FOREACH_LC(lc,newdnames)
    {
        char *name = lc->data.ptr_value;
        lc->data.ptr_value = CONCAT_STRINGS(RHS_D_PREFIX, name);
    }

    return newdnames;
}

static QueryOperator *
neumannPushdownJoin(NestingOperator *op, JoinOperator *c)
{
    QueryOperator *D = OP_LCHILD(op);
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

    // both sides: need natural join
    if(pushLeft && pushRight)
    {
        // TODO adapt attribute name of RHS and add join condition

        NestingOperator *rop = (NestingOperator *) OP_RCHILD(c);
        List *dnames = getQueryOperatorAttrNames(D);
        Node *newCond = createEqualityJoinCond((QueryOperator *) op,
                                               (QueryOperator *) rop,
                                               dnames,
                                               dnames,
                                               TRUE); // TODO if not nullable use equality instead
        List *newdnames = rhsDattrNames(D);
        List *oldjoinNames = getQueryOperatorAttrNames((QueryOperator *) c);
        int numOrigLeftAttrs = getNumAttrs(OP_RCHILD(op));
        List *oldrightNames = sublist(deepCopyStringList(oldjoinNames), numOrigLeftAttrs, -1);
        List *newleftnames = getQueryOperatorAttrNames((QueryOperator *) op);

        // if cross product, turn into inner
        if(c->joinType == JOIN_CROSS)
        {
            c->joinType = JOIN_INNER;
        }

        // conjunct with natural join condition on D
        c->cond = AND_EXPRS(c->cond, newCond);

        // fix result schema
        adaptSchemaFromChildren((QueryOperator *) c);
        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c, (QueryOperator *) op, NULL);
        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c, (QueryOperator *) rop, NULL);

        // rename join result attributes: D, original left attributes, D_renamed, original right attributes
        List *joinresultNames = CONCAT_LISTS(deepCopyStringList(newleftnames), newdnames, oldrightNames);
        FORBOTH(void,n,adef,joinresultNames,c->op.schema->attrDefs)
        {
            AttributeDef *a = (AttributeDef *) adef;
            a->attrName = strdup(n);
        }

        // add projection to remove copies of D attributes
        List *projectAttrs = CONCAT_LISTS(deepCopyStringList(newleftnames), deepCopyStringList(oldrightNames));
        QueryOperator *proj = createProjOnAttrsByName((QueryOperator *) c,
                                                      projectAttrs,
                                                      NIL);
        proj->inputs = singleton(c);
        switchSubtreeWithExisting((QueryOperator *) c, proj);
        c->op.parents = singleton(proj);

        LOG_POST_PUSH(c);
        neumannPushdown(op);
        neumannPushdown(rop);
    }
    else if (pushLeft) // TODO adapt attributes and references
    {
        // adapt schema
        adaptSchemaFromChildren((QueryOperator *) c);

        // adjust attribute references
        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c,
                                                (QueryOperator *) op,
                                                NULL);

        LOG_POST_PUSH(c);
        neumannPushdown(op);
    }
    // need to reorder attributes afterwards with projection such that D attributes come first
    else if (pushRight)
    {
        // adapt schema
        adaptSchemaFromChildren((QueryOperator *) c);

        // adjust attribute references
        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) c,
                                                (QueryOperator *) op,
                                                NULL);

        // add projection to remove copies of D attributes
        List *dnames = getQueryOperatorAttrNames(D);
        List *oldleftNames = getQueryOperatorAttrNames(OP_LCHILD(c));
        List *oldrightNames = getQueryOperatorAttrNames(OP_RCHILD(op));

        List *projectAttrs = CONCAT_LISTS(dnames, oldleftNames, oldrightNames);
        QueryOperator *proj = createProjOnAttrsByName((QueryOperator *) c,
                                                      projectAttrs,
                                                      NIL);
        proj->inputs = singleton(c);
        switchSubtreeWithExisting((QueryOperator *) c, proj);
        c->op.parents = singleton(proj);

        LOG_POST_PUSH(c);
        neumannPushdown(op);
    }

    return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownSet(NestingOperator *op, SetOperator *c)
{
    LOG_PRE_PUSH(op,c);

    // push into both inputs
    pushOperatorThroughBinary(op, (QueryOperator *) c, TRUE, TRUE);

    // fix schema to include D attributes
    adaptSchemaFromChildren((QueryOperator *) c);

    LOG_POST_PUSH(c);

    // push left and right nesting operator down
    NestingOperator *rop = (NestingOperator *) OP_RCHILD(c);
    neumannPushdown(op);
    neumannPushdown(rop);

    return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownDupRem(NestingOperator *op, DuplicateRemoval *c)
{
    LOG_PRE_PUSH(op,c);

    // just switch selection with nesting operator
    pushOperatorThroughUnary(op, (QueryOperator *) c);

    // fix schema to include D attributes
    adaptSchemaFromChildren((QueryOperator *) c);

    LOG_POST_PUSH(c);
    neumannPushdown(op);

    return (QueryOperator *) c;
}

static QueryOperator *
neumannPushdownNesting(NestingOperator *op, NestingOperator *c)
{
    LOG_PRE_PUSH(op,c);
    // TODO

    // push nesting operator into RHS
    pushOperatorThroughBinary(op, (QueryOperator *) c, FALSE, TRUE);

    // fix schema to include D attributes
    adaptSchemaFromChildren((QueryOperator *) c);

    LOG_POST_PUSH(c);
    neumannPushdown(op);

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


#define CORR_ATTR_PREFIX backendifyIdentifier("corr__")

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
    char *nestid = strdup(getNestingOperatorId(op));

    // if there are no correlations, then do not do binding transform
    if(EMPTY_SET(correlated))
    {
        return (QueryOperator *) op;
    }

    // sort correlated attribute names from the LHS
    D_attrs = sortList(D_attrs,
                       (int (*) (const void **, const void **)) strCompare);

    // add prefix to attribute names
    FOREACH(char,name,D_attrs)
    {
        char *newname = CONCAT_STRINGS(CORR_ATTR_PREFIX, nestid, "_", strdup(name));
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
