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

static List *intersectList (List *, List *);
// static boolean allFreeContained (QueryOperator *);
static void bindingTransform (NestingOperator *);

void testHelloHi() 
{
    bindingTransform(NULL);
}

// Adapted from removeListElementsFromAnotherList, not sure if this exists elsewhere
static List *
intersectList (List *left, List *right)
{
    List *result = NIL;
    boolean flag = FALSE;

    FOREACH_LC(lc2, right)
    {
        flag = FALSE;
        void *value = LC_P_VAL(lc2);

        FOREACH_LC(lc1, left)
        {
            void *ptrVal = LC_P_VAL(lc1);
            if(equal(ptrVal, value))
            {
                flag = TRUE;
                break;
            }
        }
        if(flag == TRUE)
        {
            result = appendToTailOfList(result, value);
        }
    }

    return result;
}

static void
neumanning (QueryOperator *op)
{
    FOREACH(QueryOperator, child, op->inputs) {
        neumanning(child);
    }

    if(isA(op, NestingOperator)) {
        bindingTransform(op);
    }
}

static QueryOperator *
bindingTransform (NestingOperator *op) 
{
    List *correlatedAttrs = copyList(getNestingCorrelatedAttrReferences(op, FALSE));
    // TODO: check using levels up
    List *D_attrs = intersectList(correlatedAttrs, getAttrReferences((Node*)OP_LCHILD(op)));
    List *names = NIL;
    FOREACH(AttributeReference, attr, D_attrs) {
        names = appendToTailOfList(names, attr->name);
    }
    ProjectionOperator *D = createProjectionOp(D_attrs, OP_LCHILD(op), ((QueryOperator*)op)->parents, names);
    // TODO: remove duplicates from this ^

    // TODO: add join
    // TODO: move around in tree

    DEBUG_LOG("d %s", beatify(nodeToString(D)));


    // TODO: this should be the join
    return (QueryOperator*)D;
}
