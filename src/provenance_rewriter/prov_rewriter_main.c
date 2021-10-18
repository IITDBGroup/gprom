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
#include "symbolic_eval/z3_solver.h"
#include "provenance_rewriter/coarse_grained/common_prop_inference.h"
#include "provenance_rewriter/coarse_grained/gc_prop_inference.h"
#include "provenance_rewriter/coarse_grained/ge_prop_inference.h"
#include "provenance_rewriter/coarse_grained/prop_inference.h"
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
#include "parameterized_query/parameterized_queries.h"

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

//static HashMap *
//bindsToHashMap(List *names, List *values)
//{
//	HashMap *map = NEW_MAP(Constant,Constant);
//	FORBOTH(Constant, n, v, names, values)
//	{
//		MAP_ADD_STRING_KEY(map, STRING_VALUE(n), v);
//	}
//	return map;
//}

static QueryOperator *
rewriteProvenanceComputation (ProvenanceComputation *op)
{
    QueryOperator *result = NULL;
    boolean requiresPostFiltering = FALSE;
    boolean applySummarization = HAS_STRING_PROP(op, PROP_SUMMARIZATION_DOSUM);
    HashMap *properties = (HashMap *) copyObject(op->op.properties);

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

    if (op->inputType == PROV_INPUT_TEMPORAL_QUERY)
    {
        return rewriteImplicitTemporal((QueryOperator *) op);
    }

    if (op->inputType == PROV_INPUT_UNCERTAIN_QUERY)
    {
        return rewriteUncert((QueryOperator *) op);
    }
	if (op->inputType == PROV_INPUT_UNCERTAIN_TUPLE_QUERY)
	{
		return rewriteUncertTuple((QueryOperator *) op);
	}
    if (op->inputType == PROV_INPUT_RANGE_QUERY)
    {
    	return rewriteRange((QueryOperator *) op);
    }

    // turn operator graph into a tree since provenance rewrites currently expect a tree
    if (isRewriteOptionActivated(OPTION_TREEIFY_OPERATOR_MODEL))
    {
        treeify((QueryOperator *) op);
        INFO_OP_LOG("treeified operator model:", op);
        DEBUG_NODE_BEATIFY_LOG("treeified operator model:", op);
        ASSERT(isTree((QueryOperator *) op));
    }

    //semiring comb operations
    boolean isCombinerActivated = isSemiringCombinerActivatedOp((QueryOperator *) op);

    //used to get coarse grained parameter used in CASE PROV_COARSE_GRAINED
	Node *coarsePara = NULL;
	psInfo* psPara = NULL;

	ProvenanceComputation *originalOp = copyObject(op);
	if(isRewriteOptionActivated(OPTION_UNNEST_REWRITE))
		op = (ProvenanceComputation *) unnestRewriteQuery((QueryOperator *)op);

    // apply provenance rewriting if required
    switch(op->provType)
    {
        case PROV_PI_CS:
		{
            if (isRewriteOptionActivated(OPTION_PI_CS_USE_COMPOSABLE))
                result =  rewritePI_CSComposable(op);
            else
                result = rewritePI_CS(op);
            removeParent(result, (QueryOperator *) op);

            //semiring comb operations
            if(isCombinerActivated)
            {
                Node *addExpr;
                Node *multExpr;

                addExpr = getSemiringCombinerAddExpr((QueryOperator *) op);
                multExpr = getSemiringCombinerMultExpr((QueryOperator *) op);

                INFO_LOG("user has provied a semiring combiner: %s:\n\n%s", beatify(nodeToString(addExpr)), beatify(nodeToString(multExpr)));
                result = addSemiringCombiner(result,addExpr,multExpr);
                INFO_OP_LOG("Add semiring combiner:", result);
            }
		}
		break;
        case CAP_USE_PROV_COARSE_GRAINED:
		{

			coarsePara = (Node *) getStringProperty((QueryOperator *)op, PROP_PC_COARSE_GRAINED);
			psPara = createPSInfo(coarsePara);
			DEBUG_LOG("coarse grained fragment parameters: %s",nodeToString((Node *) psPara));
			markTableAccessAndAggregation((QueryOperator *) op,  (Node *) psPara);

			//mark the number of table - used in provenance scratch
			markNumOfTableAccess((QueryOperator *) op);
			bottomUpPropagateLevelAggregation((QueryOperator *) op, psPara);

			/* copy op for use ps */
			ProvenanceComputation *useOp = (ProvenanceComputation *) copyObject(op);

			/* prepare capture provenance sketch */
			QueryOperator *capOp = rewritePI_CS(op);
			capOp = addTopAggForCoarse(capOp);

			List *attrNames = getAttrNames(capOp->schema);
			DEBUG_LOG("PS Attr Names : %s", stringListToString(attrNames));

			//char *capSql = CONCAT_STRINGS(serializeOperatorModel((Node *)capOp), ";");
			//char *capSql = serializeQueryPostgres(capOp);
			char *capSql = serializeOperatorModel((Node *)capOp);
			DEBUG_LOG("Capture Provenance Sketch Sql : %s", capSql);

			HashMap *psMap = NULL;

			/* if in self-turning mode
			 * 1) check whether need to capture ps: if no, load ps, if yes, capture
			 * 2) cache the ps
			 * if not in self-turning mode, we don't need to cache ps
			 *
			 * For 1) check strategy:
			 * if already exists ps in the cache (from load or just cached), then load directly and without recapture
			 * ps is decided by query template and parameters
			 * future: maybe also check ps created on different columns and different number of ranges?
			 * e.g., given query q: first time  q -> a 1,4,7,10
			 * 						second time q -> a 1,5,10
			 * 						third time  q -> b 1,30,60,90
			 */

			if(getStringOption(OPTION_PS_STORE_TABLE) != NULL)
			{
				DEBUG_LOG("Now in self-turning mode. ");
				QueryOperator *rootParaSql = OP_LCHILD(op);
				psMap = getPSFromCache(rootParaSql);
				if(psMap == NULL) //if not find ps from cache, then capture first
				{
					DEBUG_LOG("Not find ps. ");
					/* run capture sql and return a hashmap: (attrName, ps bit vector) key: PROV_nation1  value: "11111111111111" */
					psMap = getPS(capSql,attrNames);
				}
				else
					DEBUG_LOG("Find ps. ");

				DEBUG_NODE_BEATIFY_LOG("Current psMap: ", psMap);
				//cache ps information
				//ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);
				//memContext = getCurMemContext();
				//QueryOperator *rootParaSql = OP_LCHILD(op);
				cachePsInfo(rootParaSql,psPara,psMap);
				//RELEASE_MEM_CONTEXT();
			}
			else
			{
				/* run capture sql and return a hashmap: (attrName, ps bit vector) key: PROV_nation1  value: "11111111111111" */
				//HashMap *psMap = getPS(capSql,attrNames);
				psMap = getPS(capSql,attrNames);
				DEBUG_NODE_BEATIFY_LOG("print psMap: ", psMap);
			}

//			//cache ps information if in self-turning model
//			if(getStringOption(OPTION_PS_STORE_TABLE) != NULL)
//			{
//			    //ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);
//			    memContext = getCurMemContext();
//				QueryOperator *rootParaSql = OP_LCHILD(op);
//				cachePsInfo(rootParaSql,psPara,psMap);
//				//RELEASE_MEM_CONTEXT();
//			}

			if(isRewriteOptionActivated(OPTION_PS_USE_NEST))
			{
				useOp = originalOp;
				//mark the number of table - used in provenance scratch
				//markTableAccessAndAggregation((QueryOperator *) useOp,  (Node *) psPara);
				markNumOfTableAccess((QueryOperator *) useOp);
				bottomUpPropagateLevelAggregation((QueryOperator *) useOp, psPara);
			}

			/* use provenance sketch */
			markAutoUseTableAccess((QueryOperator *) useOp, psMap);
			markUseTableAccessAndAggregation((QueryOperator *) useOp, (Node *) psPara);

            result = rewritePI_CS(useOp);
            removeParent(result, (QueryOperator *) useOp);
		}
		break;
        case PROV_COARSE_GRAINED:
		{
// only check safety if we have compiled with Z3
#if HAVE_Z3
        	DEBUG_LOG("Start Capture PS: ");
        	DEBUG_LOG("Start exprBottomUp: ");
        	exprBottomUp(OP_LCHILD(op));
        	DEBUG_LOG("Start predBottomUp: ");
        	predBottomUp(OP_LCHILD(op));
        	DEBUG_LOG("print expr:");
        	printEXPRPro(OP_LCHILD(op));
        	DEBUG_LOG("print pred:");
        	printPREDPro(OP_LCHILD(op));
        	DEBUG_LOG("Start gcBottomUp: ");
        	gcBottomUp(OP_LCHILD(op),singleton("a"));
        	boolean isSafe = GET_BOOL_STRING_PROP(OP_LCHILD(op), PROP_STORE_SET_GC);
        	INFO_LOG("isSaft: %d", isSafe == 1 ? "safe" : "unsafe");
#endif

        	coarsePara = (Node *) getStringProperty((QueryOperator *)op, PROP_PC_COARSE_GRAINED);
        	psPara = createPSInfo(coarsePara);
        	DEBUG_LOG("coarse grained fragment parameters: %s",nodeToString((Node *) psPara));
        	markTableAccessAndAggregation((QueryOperator *) op,  (Node *) psPara);

        	//mark the number of table - used in provenance scratch
        	markNumOfTableAccess((QueryOperator *) op);
        	DEBUG_LOG("finish markNumOfTableAccess!");
        	bottomUpPropagateLevelAggregation((QueryOperator *) op, psPara);
        	DEBUG_LOG("finish bottomUpPropagateLevelAggregation!");
        	result = rewritePI_CS(op);
        	result = addTopAggForCoarse(result);
		}
		break;
        case USE_PROV_COARSE_GRAINED:
		{
			if(isRewriteOptionActivated(OPTION_PS_USE_NEST))
				op = originalOp;

			coarsePara = (Node *) getStringProperty((QueryOperator *)op, PROP_PC_COARSE_GRAINED);
			psPara = createPSInfo(coarsePara);
			DEBUG_LOG("use coarse grained fragment parameters: %s",nodeToString((Node *) psPara));
			markUseTableAccessAndAggregation((QueryOperator *) op, (Node *) psPara);

			//mark the number of table - used in provenance scratch
			markNumOfTableAccess((QueryOperator *) op);

            result = rewritePI_CS(op);
            removeParent(result, (QueryOperator *) op);
		}
		break;
        case USE_PROV_COARSE_GRAINED_BIND:
    			if(isRewriteOptionActivated(OPTION_PS_USE_NEST))
    				op = originalOp;

//    			List *binds = (List *) getStringProperty((QueryOperator *)op, PROP_PC_COARSE_GRAINED_BIND);
//    			List *b1 = (List *) getNthOfListP(binds,0);
//    			List *b2 = (List *) getNthOfListP(binds,1);
//    			List *b3 = (List *) getNthOfListP(binds,2);
//    			DEBUG_LOG("bind parameters 1:");
//    			FOREACH(Constant, c, b1)
//    			{
//    				DEBUG_LOG("%s",STRING_VALUE(c));
//    			}
//    			DEBUG_LOG("bind values 2:");
//    			FOREACH(Constant, c, b2)
//    			{
//    				DEBUG_LOG("%d",INT_VALUE(c));
//    			}
//    			DEBUG_LOG("bind values 3:");
//    			FOREACH(Constant, c, b3)
//    			{
//    				DEBUG_LOG("%d",INT_VALUE(c));
//    			}
//    			HashMap *lmap = bindsToHashMap(b1, b2);
//    			HashMap *rmap = bindsToHashMap(b1, b3);
//
//    			bottomUpInference(OP_LCHILD(op), lmap, rmap);
#if HAVE_Z3
    			DEBUG_LOG("Start Capture PS: ");
    			DEBUG_LOG("Start exprBottomUp: ");
    			exprBottomUp(OP_LCHILD(op));
    			DEBUG_LOG("Start predBottomUp: ");
    			predBottomUp(OP_LCHILD(op));
    			DEBUG_LOG("print expr:");
    			printEXPRPro(OP_LCHILD(op));
    			DEBUG_LOG("print pred:");
    			printPREDPro(OP_LCHILD(op));
    			DEBUG_LOG("Start gcBottomUp: ");
    			doGeBottomUp((QueryOperator *) op);
#endif
        		coarsePara = (Node *) getStringProperty((QueryOperator *)op, PROP_PC_COARSE_GRAINED);
        		psPara = createPSInfo(coarsePara);
        		DEBUG_LOG("use coarse grained fragment parameters: %s",nodeToString((Node *) psPara));
        		markUseTableAccessAndAggregation((QueryOperator *) op, (Node *) psPara);

        	    //mark the number of table - used in provenance scratch
        	    markNumOfTableAccess((QueryOperator *) op);

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


    // apply summarization
    if (applySummarization)
    {
        result = (QueryOperator *) rewriteSummaryOutput((Node *) result, properties, PROV_Q_WHY);
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
