/*
 * unnest_neumann.c
 *      Implements the rewrites in https://btw-2015.informatik.uni-hamburg.de/res/proceedings/Hauptband/Wiss/Neumann-Unnesting_Arbitrary_Querie.pdf
 *      TODO: Use https://doi.org/10.18420/BTW2025-01
 *
 *      Author: Felix
 */

#include "common.h"
#include "model/set/hashmap.h"
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
static Node *neumanningInternal (QueryOperator *, List *);
static QueryOperator *bindingTransform (NestingOperator *);

Node*
neumanning(QueryOperator *op)
{
    List *correlated = NIL;
    return neumanningInternal(op, correlated);
}

static Node *
neumanningInternal(QueryOperator *op, List *correlated)
{
    FOREACH(QueryOperator, child, op->inputs)
    {
        neumanning(child);
    }

    // TODO
    if(isA(op, NestingOperator))
    {
        bindingTransform((NestingOperator*)op);
    }

    return (Node*)op;
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
    Set *correlated = getNestingCorrelatedAttributes(op, FALSE); // get correlated attribute names
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
