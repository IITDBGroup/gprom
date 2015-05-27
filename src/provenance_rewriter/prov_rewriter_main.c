/*-----------------------------------------------------------------------------
 *
 * prov_rewriter_main.c
 *		Main entry point to the provenance rewriter.
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"
#include "configuration/option.h"

#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

/* function declarations */
static QueryOperator *findProvenanceComputations (QueryOperator *op);
static QueryOperator *rewriteProvenanceComputation (ProvenanceComputation *op);

/* function definitions */
Node *
provRewriteQBModel (Node *qbModel)
{
    if (isA(qbModel, List))
        return (Node *) provRewriteQueryList((List *) qbModel);
    else if (IS_OP(qbModel))
        return (Node *) provRewriteQuery((QueryOperator *) qbModel);

    FATAL_LOG("cannot rewrite node <%s>", nodeToString(qbModel));

    return NULL;
}

List *
provRewriteQueryList (List *list)
{
    FOREACH(QueryOperator,q,list)
        q_his_cell->data.ptr_value = provRewriteQuery(q);

    return list;
}

QueryOperator *
provRewriteQuery (QueryOperator *input)
{
    return findProvenanceComputations(input);
}


QueryOperator *
findProvenanceComputations (QueryOperator *op)
{
    // is provenance computation? then rewrite
    if (isA(op, ProvenanceComputation))
        return rewriteProvenanceComputation((ProvenanceComputation *) op);

    // else search for children with provenance
    FOREACH(QueryOperator, x, op->inputs)
        findProvenanceComputations(x);

    return op;
}

QueryOperator *
rewriteProvenanceComputation (ProvenanceComputation *op)
{
    // for a sequence of updates of a transaction merge the sequence into a single
    // query before rewrite.
    if (op->inputType == PROV_INPUT_UPDATE_SEQUENCE
            || op->inputType == PROV_INPUT_TRANSACTION)
    {
        START_TIMER("rewrite - merge update reenactments");
        mergeUpdateSequence(op);
        STOP_TIMER("rewrite - merge update reenactments");

        // need to restrict to updated rows?
        if (op->inputType == PROV_INPUT_TRANSACTION
                && HAS_STRING_PROP(op,PROP_PC_ONLY_UPDATED))
        {
            START_TIMER("rewrite - restrict to updated rows");
            restrictToUpdatedRows(op);
            STOP_TIMER("rewrite - restrict to updated rows");
        }
    }

    if (isRewriteOptionActivated(OPTION_TREEIFY_OPERATOR_MODEL))
    {
        treeify((QueryOperator *) op);
        INFO_LOG("treeifyed operator model:\n\n%s", operatorToOverviewString((Node *) op));
        DEBUG_LOG("treeifyed operator model:\n\n%s", beatify(nodeToString(op)));
        ASSERT(isTree((QueryOperator *) op));
    }

    switch(op->provType)
    {
        case PROV_PI_CS:
            if (isRewriteOptionActivated(OPTION_PI_CS_USE_COMPOSABLE))
                return rewritePI_CSComposable(op);
            else
                return rewritePI_CS(op);
        case PROV_TRANSFORMATION:
            return rewriteTransformationProvenance((QueryOperator *) op);
    }
    return NULL;
}

