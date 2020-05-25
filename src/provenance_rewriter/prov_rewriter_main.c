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
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/game_provenance/gp_main.h"
#include "provenance_rewriter/semiring_combiner/sc_main.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "provenance_rewriter/summarization_rewrites/summarize_main.h"
#include "provenance_rewriter/xml_rewrites/xml_prov_main.h"
#include "provenance_rewriter/unnest_rewrites/unnest_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"

#include "temporal_queries/temporal_rewriter.h"

#include "sql_serializer/sql_serializer.h"
#include "sql_serializer/sql_serializer_postgres.h"
#include "metadata_lookup/metadata_lookup.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/datalog/datalog_model.h"
#include "analysis_and_translate/analyze_dl.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"

#include "utility/string_utils.h"

/* function declarations */
static QueryOperator* findProvenanceComputations(QueryOperator *op,
		Set *haveSeen);
static QueryOperator* rewriteProvenanceComputation(ProvenanceComputation *op);

/* function definitions */
Node*
provRewriteQBModel(Node *qbModel) {
	if (isA(qbModel, List)) {
		DEBUG_LOG("THE LENGTH OF THE QBMODEL");
		DEBUG_LOG("%d\n", LIST_LENGTH(((List*)qbModel)));
		return (Node*) provRewriteQueryList((List*) qbModel);
	} else if (IS_OP(qbModel)) {
//		DEBUG_LOG("QBMODEL");
//		if(isA(qbModel, ProvenanceComputation))
//		{
//			DEBUG_LOG("qbMode is a ProvenanceComputation\n");
//			ProvenanceComputation *pc = (ProvenanceComputation *) qbModel;
//			if(pc->inputType == PROV_INPUT_UPDATEPS)
//			{
//				DEBUG_LOG("update qbModel\n");
////				return NULL; //TODO call your function to maintain provenance sketch
////				return (Node*) update_ps(qbModel);
//				Constant * result = createConstString("STOP HERE");
//				return (Node*) (result);
//			}
//		}
		return (Node*) provRewriteQuery((QueryOperator*) qbModel);
	} else if (IS_DL_NODE(qbModel)) {
		createRelToRuleMap(qbModel);
		return (Node*) rewriteForGP(qbModel);
	}
	FATAL_LOG("cannot rewrite node <%s>", nodeToString(qbModel));

	return NULL;
}

List*
provRewriteQueryList(List *list) {
	int index = 1;
	FOREACH(QueryOperator,q,list)
	{
		DEBUG_LOG("ITERATOR: %d\n", index++);
//    	DEBUG_LOG("Q->INPUT TYPE %s\n", );
		q_his_cell->data.ptr_value = provRewriteQuery(q);
	}

	return list;
}

QueryOperator*
provRewriteQuery(QueryOperator *input) {
	Set *seen = PSET();
	Node *inputProp = input->properties;

	QueryOperator *result = findProvenanceComputations(input, seen);
	result->properties = inputProp;

	return result;
}

static QueryOperator*
findProvenanceComputations(QueryOperator *op, Set *haveSeen) {
	// is provenance computation? then rewrite

	if (isA(op, ProvenanceComputation)) {

		//if it is a update, then return the updated ps

		DEBUG_LOG("qbMode is a ProvenanceComputation\n");
		ProvenanceComputation *pc = (ProvenanceComputation*) op;
		if (pc->inputType == PROV_INPUT_UPDATEPS) {
			DEBUG_LOG("update qbModel\n");

			Constant * result = createConstString(update_ps(pc));
			return (QueryOperator*) (result);
		}

		return rewriteProvenanceComputation((ProvenanceComputation*) op);
	}

	// else search for children with provenance
	FOREACH(QueryOperator,c,op->inputs)
	{
		if (!hasSetElem(haveSeen, c)) {
			addToSet(haveSeen, c);
			findProvenanceComputations(c, haveSeen);
		}
	}

	return op;
}

static QueryOperator*
rewriteProvenanceComputation(ProvenanceComputation *op) {
	QueryOperator *result = NULL;
	boolean requiresPostFiltering = FALSE;
	boolean applySummarization = HAS_STRING_PROP(op, PROP_SUMMARIZATION_DOSUM);
	HashMap *properties = (HashMap*) copyObject(op->op.properties);

	// for a sequence of updates of a transaction merge the sequence into a single
	// query before rewrite.
	if (op->inputType == PROV_INPUT_UPDATE_SEQUENCE
			|| op->inputType == PROV_INPUT_TRANSACTION
			|| op->inputType == PROV_INPUT_REENACT
			|| op->inputType == PROV_INPUT_REENACT_WITH_TIMES) {
		START_TIMER("rewrite - merge update reenactments");
		mergeUpdateSequence(op);
		STOP_TIMER("rewrite - merge update reenactments");

		// need to restrict to updated rows?
		if ((op->inputType == PROV_INPUT_TRANSACTION
				|| op->inputType == PROV_INPUT_REENACT_WITH_TIMES
				|| op->inputType == PROV_INPUT_REENACT)
				&& HAS_STRING_PROP(op, PROP_PC_ONLY_UPDATED)) {
			START_TIMER("rewrite - restrict to updated rows");
			restrictToUpdatedRows(op);
			requiresPostFiltering = HAS_STRING_PROP(op,
					PROP_PC_REQUIRES_POSTFILTERING);
			STOP_TIMER("rewrite - restrict to updated rows");
		}
	}
//    if (op->inputType == PROV_INPUT_UPDATEPS)
//    {
////    	if(isA(op, Update)) {
////    		DEBUG_LOG("UPDATE OPERATOR");
////    	} else{
////    		DEBUG_LOG("NOT UPDATE");
////    	}
//    	DEBUG_LOG("TEST XXXXXXXXXX");
////    	if(op, )
//    	return (QueryOperator*) op;
//    }
	if (op->inputType == PROV_INPUT_TEMPORAL_QUERY) {
		return rewriteImplicitTemporal((QueryOperator*) op);
	}

	if (op->inputType == PROV_INPUT_UNCERTAIN_QUERY) {
		return rewriteUncert((QueryOperator*) op);
	}

	// turn operator graph into a tree since provenance rewrites currently expect a tree
	if (isRewriteOptionActivated(OPTION_TREEIFY_OPERATOR_MODEL)) {
		treeify((QueryOperator*) op);
		INFO_OP_LOG("treeified operator model:", op);
		DEBUG_NODE_BEATIFY_LOG("treeified operator model:", op);
		ASSERT(isTree((QueryOperator* ) op));
	}

	//semiring comb operations
	boolean isCombinerActivated = isSemiringCombinerActivatedOp(
			(QueryOperator*) op);

	//used to get coarse grained parameter used in CASE PROV_COARSE_GRAINED
	Node *coarsePara = NULL;
	psInfo *psPara = NULL;

	ProvenanceComputation *originalOp = copyObject(op);
	if (isRewriteOptionActivated(OPTION_UNNEST_REWRITE))
		op = (ProvenanceComputation*) unnestRewriteQuery((QueryOperator*) op);

	// apply provenance rewriting if required
	switch (op->provType) {
	case PROV_PI_CS:
		if (isRewriteOptionActivated(OPTION_PI_CS_USE_COMPOSABLE))
			result = rewritePI_CSComposable(op);
		else
			result = rewritePI_CS(op);
		removeParent(result, (QueryOperator*) op);

		//semiring comb operations
		if (isCombinerActivated) {
			Node *addExpr;
			Node *multExpr;

			addExpr = getSemiringCombinerAddExpr((QueryOperator*) op);
			multExpr = getSemiringCombinerMultExpr((QueryOperator*) op);

			INFO_LOG("user has provied a semiring combiner: %s:\n\n%s",
					beatify(nodeToString(addExpr)),
					beatify(nodeToString(multExpr)));
			result = addSemiringCombiner(result, addExpr, multExpr);
			INFO_OP_LOG("Add semiring combiner:", result);
		}

		break;

	case CAP_USE_PROV_COARSE_GRAINED:
		coarsePara = (Node*) getStringProperty((QueryOperator*) op,
		PROP_PC_COARSE_GRAINED);
		psPara = createPSInfo(coarsePara);
		DEBUG_LOG("coarse grained fragment parameters: %s",
				nodeToString((Node* ) psPara));
		markTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

		//mark the number of table - used in provenance scratch
		markNumOfTableAccess((QueryOperator*) op);
		bottomUpPropagateLevelAggregation((QueryOperator*) op, psPara);

		/* copy op for use ps */
		ProvenanceComputation *useOp = (ProvenanceComputation*) copyObject(op);

		/* capture provenance sketch */
		QueryOperator *capOp = rewritePI_CS(op);
		capOp = addTopAggForCoarse(capOp);

		List *attrNames = getAttrNames(capOp->schema);
		DEBUG_LOG("PS Attr Names : %s", stringListToString(attrNames));

		//char *capSql = CONCAT_STRINGS(serializeOperatorModel((Node *)capOp), ";");
		//char *capSql = serializeQueryPostgres(capOp);
		char *capSql = serializeOperatorModel((Node*) capOp);
		DEBUG_LOG("Capture Provenance Sketch Sql : %s", capSql);

		/* run capture sql and return a hashmap: (attrName, ps bit vector) key: PROV_nation1  value: "11111111111111" */
		HashMap *psMap = getPS(capSql, attrNames);

		if (isRewriteOptionActivated(OPTION_PS_USE_NEST)) {
			useOp = originalOp;
			//mark the number of table - used in provenance scratch
			//markTableAccessAndAggregation((QueryOperator *) useOp,  (Node *) psPara);
			markNumOfTableAccess((QueryOperator*) useOp);
			bottomUpPropagateLevelAggregation((QueryOperator*) useOp, psPara);
		}

		/* use provenance sketch */
		markAutoUseTableAccess((QueryOperator*) useOp, psMap);
		markUseTableAccessAndAggregation((QueryOperator*) useOp,
				(Node*) psPara);

		result = rewritePI_CS(useOp);
		removeParent(result, (QueryOperator*) useOp);

		break;

	case PROV_COARSE_GRAINED:
		coarsePara = (Node*) getStringProperty((QueryOperator*) op,
		PROP_PC_COARSE_GRAINED);
		psPara = createPSInfo(coarsePara);
		DEBUG_LOG("coarse grained fragment parameters: %s",
				nodeToString((Node* ) psPara));
		markTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

		//mark the number of table - used in provenance scratch
		markNumOfTableAccess((QueryOperator*) op);
		DEBUG_LOG("finish markNumOfTableAccess!");
		bottomUpPropagateLevelAggregation((QueryOperator*) op, psPara);
		DEBUG_LOG("finish bottomUpPropagateLevelAggregation!");
		result = rewritePI_CS(op);
		result = addTopAggForCoarse(result);
		break;
	case USE_PROV_COARSE_GRAINED:
		if (isRewriteOptionActivated(OPTION_PS_USE_NEST))
			op = originalOp;

		coarsePara = (Node*) getStringProperty((QueryOperator*) op,
		PROP_PC_COARSE_GRAINED);
		psPara = createPSInfo(coarsePara);
		DEBUG_LOG("use coarse grained fragment parameters: %s",
				nodeToString((Node* ) psPara));
		markUseTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

		//mark the number of table - used in provenance scratch
		markNumOfTableAccess((QueryOperator*) op);

		result = rewritePI_CS(op);
		removeParent(result, (QueryOperator*) op);
		break;
	case PROV_TYPE_UPDATEPS:
		if (isRewriteOptionActivated(OPTION_PS_USE_NEST))
			op = originalOp;

		coarsePara = (Node*) getStringProperty((QueryOperator*) op,
		PROP_PC_COARSE_GRAINED);
		psPara = createPSInfo(coarsePara);
		DEBUG_LOG("use coarse grained fragment parameters: %s",
				nodeToString((Node* ) psPara));
		markUseTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

		//mark the number of table - used in provenance scratch
		markNumOfTableAccess((QueryOperator*) op);

//        	    //CODE ADDED
		QueryOperator *op1 = (QueryOperator*) op;
		QueryOperator *rChild = OP_RCHILD(op1);
		op1->inputs = singleton(rChild);
		//END ADDED

		//op->inputs(update, query)
		INFO_OP_LOG("\nMMMMMMMMMtreeified operator model:\n", op);
		DEBUG_NODE_BEATIFY_LOG(
				"\nQQQQQQQQQQQQQQQQQQQQQQQQQQQQQtreeified operator model:\n",
				op);
		//
		result = rewritePI_CS(op);
		removeParent(result, (QueryOperator*) op);
		break;
	case PROV_TRANSFORMATION:
		result = rewriteTransformationProvenance((QueryOperator*) op);
		break;
	case PROV_XML:
		result = rewriteXML(op); //TODO
		break;
	case PROV_NONE:
		result = OP_LCHILD(op);
		break;
	}

	// apply summarization
	if (applySummarization) {
		result = (QueryOperator*) rewriteSummaryOutput((Node*) result,
				properties, PROV_Q_WHY);
	}

	// for reenactment we may have to postfilter results if only rows affected by the transaction should be shown
	if (requiresPostFiltering) {
		START_TIMER("rewrite - restrict to updated rows by postfiltering");
		result = filterUpdatedInFinalResult(op, result);
		STOP_TIMER("rewrite - restrict to updated rows by postfiltering");
		INFO_OP_LOG("after adding selection for postfiltering", result);
	}

	return result;
}
