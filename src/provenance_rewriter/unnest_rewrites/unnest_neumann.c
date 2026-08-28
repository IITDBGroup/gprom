/*
 * unnest_neumann.c
 *      Implements the rewrites in https://btw-2015.informatik.uni-hamburg.de/res/proceedings/Hauptband/Wiss/Neumann-Unnesting_Arbitrary_Querie.pdf
 *      TODO: Use https://doi.org/10.18420/BTW2025-01
 *      
 *      Author: Felix
 */

#include "common.h"
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
neumanning (QueryOperator *op)
{
    List *correlated = NIL;
    return neumanningInternal(op, correlated);
}

static Node *
neumanningInternal(QueryOperator *op, List *correlated)
{
    FOREACH(QueryOperator, child, op->inputs) {
        neumanning(child);
    }

    if(isA(op, NestingOperator)) {
        bindingTransform((NestingOperator*)op);
    }

    return (Node*)op;
}

static QueryOperator *
bindingTransform (NestingOperator *op) 
{
    Set *correlated = getNestingCorrelatedAttributes(op, FALSE);
    // List *D_attrs = intersectList(correlatedAttrs, getAttrReferences((Node*)OP_LCHILD(op)));
    List *D_attrs = NIL;
    List *names = NIL;
    FOREACH(AttributeReference, attr, D_attrs) {
        names = appendToTailOfList(names, attr->name);
    }
    ProjectionOperator *proj = createProjectionOp(D_attrs, OP_LCHILD(op), NIL, makeNodeListFromSet(correlated));
    QueryOperator *orig = OP_LCHILD(op);
    // addParent(orig, (QueryOperator*)proj);
    orig->parents = singleton(proj);

    DuplicateRemoval *D = createDuplicateRemovalOp(D_attrs, (QueryOperator*)proj, singleton(op), makeNodeListFromSet(correlated));
    ((QueryOperator*)proj)->parents = singleton(D);

    INFO_LOG("d %s", beatify(nodeToString(D)));

    // addParent((QueryOperator*)op, (QueryOperator*)D);
    ((QueryOperator*)op)->inputs->head->data.ptr_value = (void*)D;
    // switchSubtrees(OP_LCHILD(op), (QueryOperator*)D);

    // TODO: add join
    // TODO: this should be the join
    return (QueryOperator*)op;
}
