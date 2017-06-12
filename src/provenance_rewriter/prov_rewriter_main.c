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
#include "provenance_rewriter/game_provenance/gp_main.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"
#include "provenance_rewriter/xml_rewrites/xml_prov_main.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/datalog/datalog_model.h"
#include "analysis_and_translate/analyze_dl.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"

/* function declarations */
static QueryOperator *findProvenanceComputations (QueryOperator *op, Set *haveSeen);
static QueryOperator *rewriteProvenanceComputation (ProvenanceComputation *op);

/* function definitions */
Node *
provRewriteQBModel (Node *qbModel)
{
    if (isA(qbModel, List))
        return (Node *) provRewriteQueryList((List *) qbModel);
    else if (IS_OP(qbModel))
        return (Node *) provRewriteQuery((QueryOperator *) qbModel);
    else if (IS_DL_NODE(qbModel))
    {
        createRelToRuleMap(qbModel);
        return (Node *) rewriteForGP(qbModel);
    }
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
    Set *seen = PSET();
    Node *inputProp = input->properties;

    QueryOperator *result = findProvenanceComputations(input, seen);
    result->properties = inputProp;

    return result;
}


static QueryOperator *
findProvenanceComputations (QueryOperator *op, Set *haveSeen)
{
    // is provenance computation? then rewrite
    if (isA(op, ProvenanceComputation))
        return rewriteProvenanceComputation((ProvenanceComputation *) op);

    // else search for children with provenance
    FOREACH(QueryOperator,c,op->inputs)
    {
        if (!hasSetElem(haveSeen, c))
        {
            addToSet(haveSeen, c);
            findProvenanceComputations(c, haveSeen);
        }
    }

    return op;
}

static QueryOperator *
rewriteProvenanceComputation (ProvenanceComputation *op)
{
    QueryOperator *result;
    boolean requiresPostFiltering = FALSE;

    // for a sequence of updates of a transaction merge the sequence into a single
    // query before rewrite.
    if (op->inputType == PROV_INPUT_UPDATE_SEQUENCE
            || op->inputType == PROV_INPUT_TRANSACTION
            || op->inputType == PROV_INPUT_REENACT
            || op->inputType == PROV_INPUT_REENACT_WITH_TIMES)
    {
        START_TIMER("rewrite - merge update reenactments");
        mergeUpdateSequence(op);
        STOP_TIMER("rewrite - merge update reenactments");

        // need to restrict to updated rows?
        if ((op->inputType == PROV_INPUT_TRANSACTION
                || op->inputType == PROV_INPUT_REENACT_WITH_TIMES
                || op->inputType == PROV_INPUT_REENACT)
                && HAS_STRING_PROP(op,PROP_PC_ONLY_UPDATED))
        {
            START_TIMER("rewrite - restrict to updated rows");
            restrictToUpdatedRows(op);
            requiresPostFiltering = HAS_STRING_PROP(op,PROP_PC_REQUIRES_POSTFILTERING);
            STOP_TIMER("rewrite - restrict to updated rows");
        }
    }

    // turn operator graph into a tree since provenance rewrites currently expect a tree
    if (isRewriteOptionActivated(OPTION_TREEIFY_OPERATOR_MODEL))
    {
        treeify((QueryOperator *) op);
        INFO_OP_LOG("treeified operator model:", op);
        DEBUG_NODE_BEATIFY_LOG("treeified operator model:", op);
        ASSERT(isTree((QueryOperator *) op));
    }

    // apply provenance rewriting if required
    switch(op->provType)
    {
        case PROV_PI_CS:
            if (isRewriteOptionActivated(OPTION_PI_CS_USE_COMPOSABLE))
                result =  rewritePI_CSComposable(op);
            else
                result = rewritePI_CS(op);
            removeParent(result, (QueryOperator *) op);
            break;
        case PROV_TRANSFORMATION:
            result =  rewriteTransformationProvenance((QueryOperator *) op);
            break;
        case PROV_XML:
            result = rewriteXML(op); //TODO
            break;
        case PROV_NONE:
            result = OP_LCHILD(op);
            break;
    }

    // for reenactment we may have to postfilter results if only rows affected by the transaction should be shown
    if (requiresPostFiltering)
    {
        START_TIMER("rewrite - restrict to updated rows by postfiltering");
        result = filterUpdatedInFinalResult(op, result);
        STOP_TIMER("rewrite - restrict to updated rows by postfiltering");
        INFO_OP_LOG("after adding selection for postfiltering", result);
    }

    return result;
}

