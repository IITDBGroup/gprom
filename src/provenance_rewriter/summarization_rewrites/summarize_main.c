/*-----------------------------------------------------------------------------
 *
 * summarize_main.c
 *			  
 *		
 *		AUTHOR: seokki
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"

#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "sql_serializer/sql_serializer.h"
#include "operator_optimizer/operator_optimizer.h"
#include "provenance_rewriter/prov_utility.h"
#include "utility/string_utils.h"
#include "model/query_operator/operator_property.h"

#include "provenance_rewriter/summarization_rewrites/summarize_main.h"

#define NUM_PROV_ATTR "NumInProv"
#define NUM_NONPROV_ATTR "NumInNonProv"
#define HAS_PROV_ATTR "HAS_PROV"
#define TOTAL_PROV_ATTR "TotalProv"
#define TOTAL_PROV_SAMP_ATTR "TotalProvInSamp"
#define TOTAL_NONPROV_SAMP_ATTR "TotalNonProvInSamp"
#define ACCURACY_ATTR "Precision"
#define COVERAGE_ATTR "Recall"
#define FMEASURE_ATTR "Fmeasure"
#define COVERED_ATTR "Covered"
#define SAMP_NUM_PREFIX "SampNum"
#define NON_PROV_SIZE "nonProvSize"
#define SAMP_NUM_L_ATTR SAMP_NUM_PREFIX "Left"
#define SAMP_NUM_R_ATTR SAMP_NUM_PREFIX "Right"

static List *domAttrsOutput (List *userQ, Node *sampleInput, int sampleSize, char *qType);
//static int *computeSampleSize (int *samplePerc, Node *prov);

static Node *rewriteUserQuestion (List *userQ, Node *rewrittenTree);
static Node *rewriteProvJoinOutput (Node *rewrittenTree);
static Node *rewriteRandomProvTuples (Node *provExpl, char *qType, List *fPattern);
static Node *rewriteRandomNonProvTuples (Node *provExpl, char *qType, List *fPattern);
static Node *rewriteSampleOutput (Node *randProv, Node *randNonProv, int sampleSize, char *qType);
static Node *rewritePatternOutput (char *summaryType, Node *unionSample, Node *randProv);
static Node *rewriteScanSampleOutput (Node *sampleInput, Node *patternInput);
static Node *rewriteCandidateOutput (Node *scanSampleInput, char *qType);
static Node *scaleUpOutput (List *doms, Node *candInput, Node *provJoin, Node *randProv, Node *randNonProv);
static Node *joinOnSeqOutput (List *doms);
static Node *replaceDomWithSampleDom (Node *sampleDom, Node *input);
static Node *rewriteComputeFracOutput (Node *scaledCandInput, Node *sampleInput, char *qType);
static Node *rewritefMeasureOutput (Node *computeFracInput, List *userQuestion);
static Node *rewriteTopkExplOutput (Node *fMeasureInput, int topK);
static Node *integrateWithEdgeRel (Node *topkInput, Node *moveRels, List *fPattern);

static List *provAttrs = NIL;
static List *normAttrs = NIL;
static List *userQuestion = NIL;
static boolean isDl = FALSE;


Node *
rewriteSummaryOutput (Node *rewrittenTree, List *summOpts, char *qType)
{
	// options for summarization
	List *fPattern = NIL;
	char *summaryType = NULL;
	int sampleSize = 0;
	int topK = 0;

	if (summOpts != NIL)
	{
		FOREACH(Node,n,summOpts)
		{
			if(isA(n,KeyValue))
			{
				KeyValue *kv = (KeyValue *) n;

				if(streq(STRING_VALUE(kv->key),"topk"))
					topK = INT_VALUE(kv->value);

				// whynot only for PUG (not implemented for SQL yet)
				if(streq(qType,"WHYNOT") && streq(STRING_VALUE(kv->key),"fpattern"))
					fPattern = copyObject((List *) kv->value);

				if(streq(STRING_VALUE(kv->key),"sumtype"))
					summaryType = STRING_VALUE(kv->value);

				if(streq(STRING_VALUE(kv->key),"sumsamp"))
					sampleSize = INT_VALUE(kv->value);
			}

			if(isA(n,List))
			{
				List *explSamp = (List *) n;

				FOREACH(KeyValue,kv,explSamp)
				{
					if(streq(STRING_VALUE(kv->key),"sumtype"))
						summaryType = STRING_VALUE(kv->value);

					if(streq(STRING_VALUE(kv->key),"toexpl"))
						userQuestion = (List *) kv->value;

					if(streq(STRING_VALUE(kv->key),"sumsamp"))
						sampleSize = INT_VALUE(kv->value);
				}
			}
		}
	}

	// store moveRules in separate
	//TODO: not safe to check whether input comes from dl or SQL
	Node *moveRels = NULL;
	if (isA(getHeadOfListP((List *) rewrittenTree), DuplicateRemoval))
	{
		isDl = TRUE;
		moveRels = (Node *) getTailOfListP((List *) rewrittenTree);
	}

	// summarization steps
	List *doms = NIL;
//	int *sampleSize = computeSampleSize(samplePerc,rewrittenTree);

	Node *result, *provJoin, *randomProv, *randomNonProv, *samples, *patterns,
		*sampleDom, *whynotExpl, *scanSamples, *candidates, *scaleUp, *computeFrac, *fMeasure;

	if(streq(qType,"WHY"))
	{
		if (userQuestion != NIL)
			rewrittenTree = rewriteUserQuestion(userQuestion, rewrittenTree);

		provJoin = rewriteProvJoinOutput(rewrittenTree);
		randomProv = rewriteRandomProvTuples(provJoin, qType, fPattern);
		randomNonProv = rewriteRandomNonProvTuples(provJoin, qType, fPattern);
		samples = rewriteSampleOutput(randomProv, randomNonProv, sampleSize, qType);
		patterns = rewritePatternOutput(summaryType, samples, randomProv); //TODO: different types of pattern generation
		scanSamples = rewriteScanSampleOutput(samples, patterns);
		candidates = rewriteCandidateOutput(scanSamples, qType);
		doms = domAttrsOutput(userQuestion, rewrittenTree, sampleSize, qType);
		scaleUp = scaleUpOutput(doms, candidates, provJoin, randomProv, randomNonProv);
		computeFrac = rewriteComputeFracOutput(scaleUp, randomProv, qType);
		fMeasure = rewritefMeasureOutput(computeFrac, userQuestion);
		result = rewriteTopkExplOutput(fMeasure, topK);
	}
	else if(streq(qType,"WHYNOT"))
	{
		doms = domAttrsOutput(userQuestion, rewrittenTree, sampleSize, qType);
		sampleDom = joinOnSeqOutput(doms);
		whynotExpl = replaceDomWithSampleDom(sampleDom, rewrittenTree);
		randomProv = rewriteRandomProvTuples(whynotExpl,qType, fPattern);
		randomNonProv = rewriteRandomNonProvTuples(whynotExpl, qType, fPattern);
		samples = rewriteSampleOutput(randomProv, randomNonProv, sampleSize, qType);
		patterns = rewritePatternOutput(summaryType, samples, randomProv); //TODO: different types of pattern generation
		scanSamples = rewriteScanSampleOutput(samples, patterns);
		candidates = rewriteCandidateOutput(scanSamples, qType);
		computeFrac = rewriteComputeFracOutput(candidates, randomProv, qType);
		fMeasure = rewritefMeasureOutput(computeFrac, userQuestion);
		result = rewriteTopkExplOutput(fMeasure, topK);

//		INFO_OP_LOG("WHYNOT summary is currently on the implementation process!!");
	}

	// integrate with edge relation
	if (moveRels != NULL)
		result = integrateWithEdgeRel(result, moveRels, fPattern);

	if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
		ASSERT(checkModel((QueryOperator *) result));

	return result;
}



/*
 * integrate with edge rel for dl
 */
static Node *
integrateWithEdgeRel(Node * topkInput, Node *moveRels, List *fPattern)
{
	Node *result;
	QueryOperator *newEdgeBase = (QueryOperator *) topkInput;
	QueryOperator *edgeRels = (QueryOperator *) moveRels;

	// only prov attrs are needed (create projection operator)
	int pos = 0;
	List *projExpr = NIL;
	List *attrNames = NIL;

	FOREACH(AttributeDef,a,newEdgeBase->schema->attrDefs)
	{
		if(isPrefix(a->attrName,"PROV_"))
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

			attrNames = appendToTailOfList(attrNames, a->attrName);
		}
		pos++;
	}

	// bring the failure pattern back before merge with edge rel for WHYNOT
	if(fPattern != NIL)
	{
		FOREACH(Node,n,fPattern)
		{
			projExpr = appendToTailOfList(projExpr, n);
			attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("A",itoa(pos)));
			pos++;
		}
	}

	ProjectionOperator *op = createProjectionOp(projExpr, newEdgeBase, NIL, attrNames);
	newEdgeBase->parents = singleton(op);
	newEdgeBase = (QueryOperator *) op;

	// store table access operator for later use of dom attrs
	List *rels = NIL;
	findTableAccessVisitor((Node *) edgeRels,&rels);

	FOREACH(TableAccessOperator,t,rels)
	{
		switchSubtreeWithExisting((QueryOperator *) t,newEdgeBase);
		DEBUG_LOG("replaced table %s with\n:%s", t->tableName, operatorToOverviewString((Node *) newEdgeBase));
	}

	result = (Node *) edgeRels;

	DEBUG_NODE_BEATIFY_LOG("pug top-k summarized explanation from summarization:", result);
	INFO_OP_LOG("pug top-k summarized explanation from summarization as overview:", result);

	return result;
}


/*
 * return top-k explanations
 */
static Node *
rewriteTopkExplOutput (Node *fMeasureInput, int topK)
{
	Node *result;
	QueryOperator *fMeasure = (QueryOperator *) fMeasureInput;

	// create selection for returning top most general explanation
	Node *selCond = NULL;

	if (topK != 0)
		selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(topK)));
	else
		selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(1))); // TODO: top1 or more?

	SelectionOperator *so = createSelectionOp(selCond, fMeasure, NIL, getAttrNames(fMeasure->schema));

	fMeasure->parents = singleton(so);
	fMeasure = (QueryOperator *) so;

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,a,fMeasure->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	op = createProjectionOp(projExpr, fMeasure, NIL, getAttrNames(fMeasure->schema));
	fMeasure->parents = singleton(op);
	fMeasure = (QueryOperator *) op;

	result = (Node *) fMeasure;

	DEBUG_NODE_BEATIFY_LOG("top-k summarized explanation from summarization:", result);
	INFO_OP_LOG("top-k summarized explanation from summarization as overview:", result);

	return result;
}



/*
 * compute f-measure based on precision and recall
 */
static Node *
rewritefMeasureOutput (Node *computeFracInput, List *userQuestion)
{
	Node *result;
	QueryOperator *computeFrac = (QueryOperator *) computeFracInput;

	// where clause to filter out the pattern that only contains user question info
	int aPos = 0;
	int count = 1;
	AttributeReference *lA, *rA = NULL;
	Node *whereCond = NULL;

	FOREACH(AttributeDef,a,computeFrac->schema->attrDefs)
	{
		if(isPrefix(a->attrName, "use"))
		{
			if(count % 2 != 0)
				lA = createFullAttrReference(strdup(a->attrName), 0, aPos, 0, a->dataType);
			else
				rA = createFullAttrReference(strdup(a->attrName), 0, aPos, 0, a->dataType);

			if(lA != NULL && rA != NULL)
			{
				Node *pairCond = (Node *) createOpExpr("+",LIST_MAKE(lA,rA));

				if(whereCond != NULL)
					whereCond = (Node *) createOpExpr("+",LIST_MAKE(whereCond,pairCond));
				else
					whereCond = copyObject(pairCond);

				lA = NULL;
				rA = NULL;
			}
			count++;
		}
		aPos++;
	}

	// add the last attr
	if(lA != NULL && rA == NULL)
		whereCond = (Node *) createOpExpr("+",LIST_MAKE(whereCond,lA));

	int maxNum = count - LIST_LENGTH(userQuestion) - 1;
	Node *filterCond = (Node *) createOpExpr("<",LIST_MAKE(whereCond,createConstInt(maxNum)));

	SelectionOperator *so = createSelectionOp(filterCond, computeFrac, NIL, getAttrNames(computeFrac->schema));
	computeFrac->parents = appendToTailOfList(computeFrac->parents,so);
	computeFrac = (QueryOperator *) so;

	// projection operator with a f-measure computation
	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
	AttributeReference *prec, *rec = NULL;

	FOREACH(AttributeDef,a,computeFrac->schema->attrDefs)
	{
		if(streq(a->attrName, ACCURACY_ATTR))
			prec = createFullAttrReference(strdup(ACCURACY_ATTR), 0, pos, 0, a->dataType);

		if(streq(a->attrName, COVERAGE_ATTR))
			rec = createFullAttrReference(strdup(COVERAGE_ATTR), 0, pos, 0, a->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	// add f-measure computation
	Node *times = (Node *) createOpExpr("*",LIST_MAKE(prec,rec));
	Node *plus = (Node *) createOpExpr("+",LIST_MAKE(prec,rec));
	Node *cal = (Node *) createOpExpr("/",LIST_MAKE(times,plus));
	Node *fmeasure = (Node *) createOpExpr("*",LIST_MAKE(createConstInt(2),cal));
	projExpr = appendToTailOfList(projExpr, fmeasure);

	List *attrNames = CONCAT_LISTS(getAttrNames(computeFrac->schema), singleton(FMEASURE_ATTR));
	op = createProjectionOp(projExpr, computeFrac, NIL, attrNames);
	computeFrac->parents = singleton(op);
	computeFrac = (QueryOperator *) op;

	// create ORDER BY
	OrderExpr *fmeasureExpr = createOrderExpr(fmeasure, SORT_DESC, SORT_NULLS_LAST);
	OrderOperator *ord = createOrderOp(singleton(fmeasureExpr), computeFrac, NIL);

	computeFrac->parents = singleton(ord);
	computeFrac = (QueryOperator *) ord;

	result = (Node *) computeFrac;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("compute f-measure for summarization:", result);
	INFO_OP_LOG("compute f-measure for summarization as overview:", result);

	return result;
}


/*
 * compute precision and recall
 */
static Node *
rewriteComputeFracOutput (Node *scaledCandInput, Node *sampleInput, char *qType)
{
	Node *result;
	QueryOperator *candidates = (QueryOperator *) scaledCandInput;

	if(streq(qType,"WHYNOT"))
	{
		QueryOperator *provSample = (QueryOperator *) sampleInput;

		// count hasProv
		Constant *countProv = createConstInt(1);
		FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
		fcCount->isAgg = TRUE;

		ProjectionOperator *projOp = createProjectionOp(singleton(fcCount), provSample, NIL, singleton(strdup(TOTAL_PROV_ATTR)));
		provSample->parents = singleton(projOp);
		provSample = (QueryOperator *) projOp;

		// create cross product
		List *inputs = LIST_MAKE(provSample, candidates);
		List *attrNames = CONCAT_LISTS(getAttrNames(provSample->schema), getAttrNames(candidates->schema));

		QueryOperator *cp = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
		OP_LCHILD(cp)->parents = OP_RCHILD(cp)->parents = singleton(cp);
		candidates = cp;
	}

//	// get total count for prov from samples
//	int aPos = LIST_LENGTH(samples->schema->attrDefs) - 1;
//	AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);
//
//	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
//	SelectionOperator *so = createSelectionOp(whereClause, samples, NIL, getAttrNames(samples->schema));
//
//	samples->parents = appendToTailOfList(samples->parents,so);
//	samples = (QueryOperator *) so;
//
//	// create projection operator
//	Constant *countProv = createConstInt(1);
//	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
//	fcCount->isAgg = TRUE;
//	//countProv->name = strdup(TOTAL_PROV_ATTR);
//
//	ProjectionOperator *op = createProjectionOp(singleton(fcCount), samples, NIL, singleton(strdup(TOTAL_PROV_ATTR)));
//	samples->parents = singleton(op);
//	samples = (QueryOperator *) op;
//
//	// cross product with candidates to compute
//	List *crossInput = LIST_MAKE(samples,candidates);
//	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(candidates->schema));
//	QueryOperator *computeFrac = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, crossInput, NIL, attrNames);
//
//	// set the parent of the operator's children
////	OP_LCHILD(computeFrac)->parents = OP_RCHILD(computeFrac)->parents = singleton(computeFrac);
//	samples->parents = appendToTailOfList(samples->parents,computeFrac);
//	candidates->parents = singleton(computeFrac);

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	AttributeReference *totProv, *covProv, *numProv = NULL;
//	AttributeReference *covProv;
//	AttributeReference *numProv;

	FOREACH(AttributeDef,a,candidates->schema->attrDefs)
	{
		if (pos == 0)
			totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, pos, 0, a->dataType);

		if (pos == 1)
			covProv = createFullAttrReference(strdup(COVERED_ATTR), 0, pos, 0, a->dataType);

		if (pos == 2)
			numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, pos, 0, a->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	// round up after second decimal number
	Node *rdup = (Node *) createConstInt(atoi("2"));

	// add attribute for accuracy
//	AttributeReference *numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, 2, 0, DT_INT);
//	AttributeReference *covProv = createFullAttrReference(strdup(COVERED_ATTR), 0, 1, 0, DT_INT);
	Node *precRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,covProv));
	FunctionCall *rdupAr = createFunctionCall("ROUND", LIST_MAKE(precRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupAr);

	// add attribute for coverage
//	AttributeReference *totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, 0, 0, DT_INT);
	Node* recRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,totProv));
	FunctionCall *rdupCr = createFunctionCall("ROUND", LIST_MAKE(recRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupCr);

	List *attrNames = NIL;
	attrNames = CONCAT_LISTS(getAttrNames(candidates->schema), singleton(ACCURACY_ATTR), singleton(COVERAGE_ATTR));
	ProjectionOperator *op = createProjectionOp(projExpr, candidates, NIL, attrNames);
	candidates->parents = singleton(op);
	candidates = (QueryOperator *) op;

//	// create ORDER BY
//	// TODO: probably put another projection for order by operation
////	AttributeReference *accuR = createFullAttrReference(strdup(ACCURACY_ATTR), 0,
////							LIST_LENGTH(computeFrac->schema->attrDefs) - 2, 0, DT_INT);
//
//	OrderExpr *accExpr = createOrderExpr(precRate, SORT_DESC, SORT_NULLS_LAST);
//	OrderExpr *covExpr = createOrderExpr(recRate, SORT_DESC, SORT_NULLS_LAST);
//
//	OrderOperator *ord = createOrderOp(LIST_MAKE(accExpr, covExpr), computeFrac, NIL);
//	computeFrac->parents = singleton(ord);
//	computeFrac = (QueryOperator *) ord;

	result = (Node *) candidates;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("compute fraction for summarization:", result);
	INFO_OP_LOG("compute fraction for summarization as overview:", result);

	return result;
}



/*
 * for WHY, scale up the measure values to real one
 */
static Node *
scaleUpOutput (List *doms, Node *candInput, Node *provJoin, Node *randSamp, Node *randNonSamp)
{
	Node *result;
	List *inputs = NIL;
	List *attrNames = NIL;

	// inputs for computing scale up
	QueryOperator *candidates = (QueryOperator *) candInput;
	QueryOperator *provQuery = (QueryOperator *) provJoin;
	QueryOperator *sampProv = (QueryOperator *) randSamp;
	QueryOperator *sampNonProv = (QueryOperator *) randNonSamp;

	// store candidates and doms as inputs for cross product later
	SET_BOOL_STRING_PROP((Node *) candidates, PROP_MATERIALIZE);
	inputs = appendToTailOfList(inputs, (Node *) candidates);
	attrNames = getAttrNames(candidates->schema);

	// generate sub-queries for 1) totalProv
	int aPos = LIST_LENGTH(provQuery->schema->attrDefs) - 1;
	AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

	Node *cond = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
	SelectionOperator *so = createSelectionOp(cond, provQuery, NIL, getAttrNames(provQuery->schema));

	provQuery->parents = singleton(so);
	provQuery = (QueryOperator *) so;

	Constant *countTProv = createConstInt(1);
	FunctionCall *fcTp = createFunctionCall("COUNT", singleton(countTProv));
	fcTp->isAgg = TRUE;

	AggregationOperator *totalProv = createAggregationOp(singleton(fcTp), NIL, provQuery, NIL, singleton(strdup(TOTAL_PROV_ATTR)));
	SET_BOOL_STRING_PROP((Node *) totalProv, PROP_MATERIALIZE);
	inputs = appendToTailOfList(inputs, (Node *) totalProv);
	attrNames = appendToTailOfList(attrNames, strdup(TOTAL_PROV_ATTR));

	// create cross product for provQuery and totalProv
	QueryOperator *provQtotalProv = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
	candidates->parents = singleton(provQtotalProv);
	((QueryOperator *) totalProv)->parents = singleton(provQtotalProv);

	// 2) totalProvInSamp
	int gPos = LIST_LENGTH(sampProv->schema->attrDefs) - 1;
	AttributeReference *TProvInSamp = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
	FunctionCall *fcTps = createFunctionCall("COUNT", singleton(TProvInSamp));
	fcTps->isAgg = TRUE;

	AggregationOperator *totalProvInSamp = createAggregationOp(singleton(fcTps), NIL, sampProv, NIL, singleton(strdup(TOTAL_PROV_SAMP_ATTR)));
	SET_BOOL_STRING_PROP((Node *) totalProvInSamp, PROP_MATERIALIZE);
	inputs = LIST_MAKE(provQtotalProv, totalProvInSamp);
	attrNames = appendToTailOfList(attrNames, strdup(TOTAL_PROV_SAMP_ATTR));

	// create cross product for provQtotalProv and totalProvInSamp
	QueryOperator *crossPtotalProvInSamp = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
	provQtotalProv->parents = singleton(crossPtotalProvInSamp);
	((QueryOperator *) totalProvInSamp)->parents = singleton(crossPtotalProvInSamp);

	// 3) totalNonProvInSamp
	gPos = LIST_LENGTH(sampNonProv->schema->attrDefs) - 1;
	AttributeReference *TNonProvInSamp = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
	FunctionCall *fcTnps = createFunctionCall("COUNT", singleton(TNonProvInSamp));
	fcTnps->isAgg = TRUE;

	AggregationOperator *totalNonProvInSamp = createAggregationOp(singleton(fcTnps), NIL, sampNonProv, NIL, singleton(strdup(TOTAL_NONPROV_SAMP_ATTR)));
	SET_BOOL_STRING_PROP((Node *) totalNonProvInSamp, PROP_MATERIALIZE);
	inputs = LIST_MAKE(crossPtotalProvInSamp, totalNonProvInSamp);
	attrNames = appendToTailOfList(attrNames, strdup(TOTAL_NONPROV_SAMP_ATTR));

	// create cross product for provQtotalProv and totalNonProvInSamp
	QueryOperator *crossPtotalNonProvInSamp = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
	crossPtotalProvInSamp->parents = singleton(crossPtotalNonProvInSamp);
	((QueryOperator *) totalNonProvInSamp)->parents = singleton(crossPtotalNonProvInSamp);

	// add cross product for doms
	QueryOperator *crossPdom;

	for(int i = 0; i < LIST_LENGTH(doms); i++)
	{
		Node *n = (Node *) getNthOfListP(doms,i);
		SET_BOOL_STRING_PROP(n, PROP_MATERIALIZE);

		if(i == 0)
			inputs = LIST_MAKE(crossPtotalNonProvInSamp, n);
		else
			inputs = LIST_MAKE(crossPdom, n);

		QueryOperator *oDom = (QueryOperator *) n;
		attrNames = concatTwoLists(attrNames, getAttrNames(oDom->schema));

		// create cross product for provQuery and doms
		crossPdom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);

		if(i == 0)
			crossPtotalNonProvInSamp->parents = singleton(crossPdom);
		else
			OP_LCHILD(crossPdom)->parents = singleton(crossPdom);

		oDom->parents = singleton(crossPdom);
	}

	/*
	 * create projection operator for computing
	 * p = numInProv * totalProv / totalProvInSamp
	 * np = numInNonProv * (domA * domB ... * domN - totalProv) / totalNonProvInSamp
	 * p + np = covered in real dataset
	 * p = numInProv in real dataset
	 */

	int pos = 0;
	int counter = 0;
	List *projExpr = NIL;
	List *attrs = NIL;
	Node *crossDoms = NULL;
	AttributeReference *totProv, *numProv, *numNonProv, *totProvInSamp, *totNonProvInSamp, *domL, *domR = NULL;

	FOREACH(AttributeDef,a,crossPdom->schema->attrDefs)
	{
		if(streq(a->attrName,TOTAL_PROV_ATTR))
			totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,NUM_PROV_ATTR))
			numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,NUM_NONPROV_ATTR))
			numNonProv = createFullAttrReference(strdup(NUM_NONPROV_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,TOTAL_PROV_SAMP_ATTR))
			totProvInSamp = createFullAttrReference(strdup(TOTAL_PROV_SAMP_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,TOTAL_NONPROV_SAMP_ATTR))
			totNonProvInSamp = createFullAttrReference(strdup(TOTAL_NONPROV_SAMP_ATTR), 0, pos, 0, a->dataType);
		else if(isPrefix(a->attrName,"cnt"))
		{
			if(counter == 0)
			{
				domL = createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType);
				counter++;
			}
			else
				domR = createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType);

			if(domL != NULL && domR != NULL)
			{
				crossDoms = (Node *) createOpExpr("*",LIST_MAKE(domL,domR));
				domL = (AttributeReference *) crossDoms;
				domR = NULL;
			}
		}
		else
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

			attrs = appendToTailOfList(attrs, a);
		}

		pos++;
	}

	// create numInProv, covered, and totalProv, and add to the head of the projExpr
	Node *subNumProv = (Node *) createOpExpr("*",LIST_MAKE(numProv,totProv));
	Node *numInProv = (Node *) createOpExpr("/",LIST_MAKE(subNumProv,totProvInSamp));
	projExpr = appendToHeadOfList(projExpr, numInProv);

	Node *nonProv = (Node *) createOpExpr("-",LIST_MAKE(crossDoms,totProv));
	Node *scaleNonProv = (Node *) createOpExpr("*",LIST_MAKE(numNonProv,nonProv));
	Node *numInNonProv = (Node *) createOpExpr("/",LIST_MAKE(scaleNonProv,totNonProvInSamp));
	Node *numCov = (Node *) createOpExpr("+",LIST_MAKE(numInProv,numInNonProv));
	projExpr = appendToHeadOfList(projExpr, numCov);
	projExpr = appendToHeadOfList(projExpr, totProv);

	// create projection for candidates with real measure values
	attrNames = CONCAT_LISTS(singleton(TOTAL_PROV_ATTR), singleton (COVERED_ATTR), singleton(NUM_PROV_ATTR), getAttrDefNames(attrs));
	ProjectionOperator *op = createProjectionOp(projExpr, crossPdom, NIL, attrNames);
	crossPdom->parents = singleton(op);

	result = (Node *) op;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("scale up numInProv and covered for summarization:", result);
	INFO_OP_LOG("scale up numInProv and covered for summarization as overview:", result);

	return result;
}



/*
 * generate domain attrs for later use of scale up of the measure values to the real values
 */
static List *
domAttrsOutput (List *userQ, Node *input, int sampleSize, char *qType)
{
	List *result = NIL;

	// translated input algebra to use the table acess operators
	QueryOperator *prov;
	if(isA(input, List))
		prov = (QueryOperator *) getHeadOfListP((List *) input);
	else
		prov = (QueryOperator *) input;

	QueryOperator *transInput = (QueryOperator *) prov->properties;

	if(isDl)
	{
		QueryOperator *dup = (QueryOperator *) transInput;
		ProjectionOperator *fromInputQ = (ProjectionOperator *) getHeadOfListP(dup->inputs);
		userQuestion = fromInputQ->projExprs;
	}
//	else
//	{
//
//		// replace the attr defs in userQuestion to attr Refs
//		int attrPos = 0;
//		FOREACH(AttributeDef, a, userQuestion)
//		{
//			AttributeDef *aDef = (AttributeDef *) getNthOfListP(normAttrs, attrPos);
//			ar->name = strdup(aDef->attrName);
//			attrPos++;
//		}
//	}

	// store table access operator for later use of dom attrs
	List *rels = NIL;
	findTableAccessVisitor((Node *) transInput,&rels);

	int attrCount = 0;
	int relCount = 0;
	char *relName = NULL;
	HashMap *existAttr = NEW_MAP(Constant,Constant);

	FOREACH(TableAccessOperator, t, rels)
	{
		// replace attr names in user question with full names
		if(isDl)
		{
			FOREACH(AttributeReference,ar,userQuestion)
			{
				if(ar->fromClauseItem == relCount)
				{
					ar->outerLevelsUp = 0; // TODO: force to be '0' or keep it
					int pos = ar->attrPosition;
					ar->name = STRING_VALUE(getNthOfListP(t->op.schema->attrDefs,pos));
				}
			}
		}

		// if the input query is not self-joined, then reset everything
		if(relName != NULL)
		{
			if(!streq(relName,t->tableName))
//				relCount++;
//			else
			{
				attrCount = 0;
//				relCount = 0;
				relName = NULL;
			}
		}

		// collect the attrs not in the prov question and create dom for those
		// TODO: condition is temporary (e.g., to filter out the case that for self-join, dom attrs are generated based on only left)
		if(relName == NULL)
		{
			relName = strdup(t->tableName);

			FOREACH(AttributeDef, a, t->op.schema->attrDefs)
			{
				AttributeReference *ar = createFullAttrReference(strdup(a->attrName), 0, attrCount, 0, a->dataType);

	//			if(relCount > 0)
	//				ar->attrPosition = ar->attrPosition + attrCount;

				if(!searchListNode(userQuestion, (Node *) ar))
				{
					// create attr domains only once
					int existAttrCnt = MAP_INCR_STRING_KEY(existAttr,a->attrName);

					if(existAttrCnt == 0)
					{
						// count for why
						if(streq(qType,"WHY"))
						{
							// create count attr
							AttributeReference *countAr = createFullAttrReference(strdup(ar->name), 0, ar->attrPosition - attrCount, 0, DT_INT);
							FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countAr));
							fcCount->isAgg = TRUE;

							// create agg operator
							char *domAttr = CONCAT_STRINGS("cnt",ar->name);
							AggregationOperator *aggCount = createAggregationOp(singleton(fcCount), NIL, (QueryOperator *) t,
																NIL, singleton(strdup(domAttr)));
							SET_BOOL_STRING_PROP((Node *) aggCount, PROP_MATERIALIZE);

							result = appendToTailOfList(result, (Node *) aggCount);
						}
						// random sample from attr domain for whynot
						else if(streq(qType,"WHYNOT"))
						{
							// TODO: compute percentile for random sample based on the sample size
							int perc = 0;
							if(sampleSize >= 100 && sampleSize < 1000)
								perc = 25;
							else if(sampleSize >= 1000)
								perc = 21;

							// TODO: SAMPLE clause must not be part of the table name
							t->tableName = CONCAT_STRINGS(relName," ","SAMPLE(",itoa(perc),")");

							// rownum as a sequence
							Node *rowNum = (Node *) makeNode(RowNumExpr);
							List *projExpr = LIST_MAKE(rowNum, ar);
							ProjectionOperator *projDom = createProjectionOp(projExpr, (QueryOperator *) t, NIL,
														LIST_MAKE(strdup("SEQ"),strdup(ar->name)));
							SET_BOOL_STRING_PROP((Node *) projDom, PROP_MATERIALIZE);
							result = appendToTailOfList(result, (Node *) projDom);
						}
					}
				}
				attrCount++;
			}
		}
		relCount++;
	}

	DEBUG_NODE_BEATIFY_LOG("dom attrs for summarization:", (Node *) result);
	INFO_OP_LOG("dom attrs for summarization as overview:", (Node *) result);

	return result;
}


/*
 * compute measure values, e.g., numInProv and coverage
 * numInProv: how many prov within whole prov
 * coverage: how many prov or non-prov are covered by the pattern
 */
static Node *
rewriteCandidateOutput (Node *scanSampleInput, char *qType)
{
	Node *result;

	QueryOperator *scanSamples = (QueryOperator *) scanSampleInput;

	// create group by operator
	List *groupBy = NIL;
	int gPos = 0;

	FOREACH(AttributeDef,a,scanSamples->schema->attrDefs)
	{
		if (!streq(a->attrName,HAS_PROV_ATTR))
		{
			groupBy = appendToTailOfList(groupBy,
					createFullAttrReference(strdup(a->attrName), 0, gPos, 0, a->dataType));

			gPos++;
		}
	}

	List *aggrs = NIL;
	FunctionCall *fcShnp;
	List *attrNames = NIL;

	if(streq(qType,"WHY"))
	{
		Constant *sumHasNonProv = createConstInt(0);
		fcShnp = createFunctionCall("SUM", singleton(sumHasNonProv));
		attrNames = concatTwoLists(singleton(NUM_NONPROV_ATTR), singleton(NUM_PROV_ATTR));
	}
	else if(streq(qType,"WHYNOT"))
	{
		Constant *countProv = createConstInt(1);
		fcShnp = createFunctionCall("COUNT", singleton(countProv));
		attrNames = concatTwoLists(singleton(COVERED_ATTR), singleton(NUM_PROV_ATTR));
	}
	fcShnp->isAgg = TRUE;

	AttributeReference *sumHasProv = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
	FunctionCall *fc = createFunctionCall("SUM", singleton(sumHasProv));
	fc->isAgg = TRUE;

	aggrs = appendToTailOfList(aggrs,fcShnp);
	aggrs = appendToTailOfList(aggrs,fc);

	List *attrs = getAttrDefNames(removeFromTail(scanSamples->schema->attrDefs));
	attrNames = concatTwoLists(attrNames, attrs);

	AggregationOperator *gb = createAggregationOp(aggrs, groupBy, scanSamples, NIL, attrNames);
	scanSamples->parents = singleton(gb);
	scanSamples = (QueryOperator *) gb;
//	scanSamples->provAttrs = copyObject(provAttrs);

	// create projection operator
	List *projExpr = NIL;
//	List *origExprs = NIL;
	List *caseExprs = NIL;
	int pos = 0;

	FOREACH(AttributeDef,a,scanSamples->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

		pos++;
	}

//	pos = 2;
//	FOREACH(AttributeDef,a,normAttrs)
//	{
//		origExprs = appendToTailOfList(origExprs,
//				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//
//		pos++;
//	}

	attrs = NIL;

	FOREACH(AttributeDef,n,projExpr)
	{
//		AttributeDef *a = (AttributeDef *) n;

		if (isPrefix(n->attrName,"PROV_"))
		{
			Node *cond = (Node *) createIsNullExpr((Node *) n);
			Node *then = (Node *) createConstInt(1);
			Node *els = (Node *) createConstInt(0);

			CaseWhen *caseWhen = createCaseWhen(cond, then);
			CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

			caseExprs = appendToTailOfList(caseExprs, (List *) caseExpr);
			attrs = appendToTailOfList(attrs, CONCAT_STRINGS("use",n->attrName));
		}
	}

	attrNames = concatTwoLists(getAttrNames(scanSamples->schema), attrs);
	ProjectionOperator *op = createProjectionOp(concatTwoLists(projExpr,caseExprs), scanSamples, NIL, attrNames);
	scanSamples->parents = singleton(op);
	scanSamples = (QueryOperator *) op;

	result = (Node *) scanSamples;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("candidate patterns for summarization:", result);
	INFO_OP_LOG("candidate patterns for summarization as overview:", result);

	return result;
}


/*
 * match patterns generated with the full sample
 */
static Node *
rewriteScanSampleOutput (Node *sampleInput, Node *patternInput)
{
	Node *result;

	QueryOperator *samples = (QueryOperator *) sampleInput;
	QueryOperator *patterns = (QueryOperator *) patternInput;

	// create join condition
	int aPos = 0;
	Node *joinCond = NULL;
	Node *isNullCond = NULL;
	Node *attrCond = NULL;
	Node *curCond = NULL;
//	int sLen = LIST_LENGTH(samples->schema->attrDefs) - 1;

	FOREACH(AttributeDef,ar,patterns->schema->attrDefs)
	{
		AttributeReference *lA, *rA = NULL;
		rA = createFullAttrReference(strdup(ar->attrName), 1, aPos, 0, ar->dataType);

		FOREACH(AttributeDef,al,samples->schema->attrDefs)
		{
			if(isSubstr(ar->attrName,al->attrName))
			{
				int alPos = LIST_LENGTH(normAttrs) + aPos;
				lA = createFullAttrReference(strdup(al->attrName), 0, alPos, 0, al->dataType);

				// create equality condition and update global condition
				joinCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
				isNullCond = (Node *) createIsNullExpr((Node *) rA);
				attrCond = OR_EXPRS(joinCond,isNullCond);
				curCond = AND_EXPRS(attrCond,curCond);
			}
		}
		aPos++;
	}

	// create join operator
	List *inputs = LIST_MAKE(samples,patterns);
	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(patterns->schema));
	QueryOperator *scanSample = (QueryOperator *) createJoinOp(JOIN_INNER, curCond, inputs, NIL, attrNames);
	makeAttrNamesUnique(scanSample);

	// set the parent of the operator's children
//	OP_LCHILD(scanSample)->parents = OP_RCHILD(scanSample)->parents = singleton(scanSample);
	samples->parents = appendToTailOfList(samples->parents,scanSample);
	patterns->parents = singleton(scanSample);
//	scanSample->provAttrs = provAttrs;

	// create projection for adding HAS_PROV_ATTR attribute
	int pos = 0;
	int hasPos = 0;
	List *projExpr = NIL;
	List *hasExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,scanSample->schema->attrDefs)
	{
		if (streq(p->attrName,HAS_PROV_ATTR))
		{
			hasPos = pos;
			hasExpr = appendToTailOfList(hasExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		}
		else if (pos > hasPos && hasPos != 0)
		{
			if (isPrefix(p->attrName,"PROV_"))
				projExpr = appendToTailOfList(projExpr,
						createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		}
		pos++;
	}

	AttributeReference *hp = (AttributeReference *) getHeadOfListP(hasExpr);
	projExpr = appendToTailOfList(projExpr, hp);

	List *subAttrs = NIL;
	FOREACH(char,a,getAttrNames(patterns->schema))
		if (isPrefix(a,"PROV_"))
			subAttrs = appendToTailOfList(subAttrs,a);

	attrNames = concatTwoLists(subAttrs,singleton(hp->name));
	op = createProjectionOp(projExpr, scanSample, NIL, attrNames);

	scanSample->parents = singleton(op);
	scanSample = (QueryOperator *) op;
//	scanSample->provAttrs = copyObject(provAttrs);

	result = (Node *) scanSample;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("join patterns with samples for summarization:", result);
	INFO_OP_LOG("join patterns with samples for summarization as overview:", result);

	return result;
}


/*
 * compute patterns (currently LCA is implemented)
 * TODO: more techniques to generate patterns
 */
static Node *
rewritePatternOutput (char *summaryType, Node *unionSample, Node *randProv)
{
	Node *result;

	// compute Lowest Common Ancestors (LCA)
	if (streq(summaryType,"LCA"))
	{
		QueryOperator *allSample = (QueryOperator *) unionSample;

		// return only sample tuples having provenance
		QueryOperator *provSample = (QueryOperator *) randProv;

//		int aPos = LIST_LENGTH(provSample->schema->attrDefs) - 1;
//		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);
//
//		Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
//		SelectionOperator *so = createSelectionOp(whereClause, provSample, NIL, getAttrNames(provSample->schema));
//
//		provSample->parents = singleton(so);
//		provSample = (QueryOperator *) so;
//		provSample->provAttrs = copyObject(provAttrs);
//
//		// create projection operator
//		List *projExpr = NIL;
//		int pos = LIST_LENGTH(allSample->schema->attrDefs);
//
//		FOREACH(AttributeDef,a,provSample->schema->attrDefs)
//		{
//			projExpr = appendToTailOfList(projExpr,
//					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//			pos++;
//		}
//
//		ProjectionOperator *op = createProjectionOp(projExpr, provSample, NIL, getAttrNames(provSample->schema));
//		provSample->parents = singleton(op);
//		provSample->provAttrs = copyObject(provAttrs);

		// create CROSS_JOIN operator
		List *crossInput = LIST_MAKE(allSample, provSample);
		List *attrNames = concatTwoLists(getAttrNames(allSample->schema),getAttrNames(provSample->schema));
		QueryOperator *patternJoin = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, crossInput, NIL, attrNames);
		makeAttrNamesUnique(patternJoin);

		// set the parent of the operator's children
//		OP_LCHILD(patternJoin)->parents = OP_RCHILD(patternJoin)->parents = singleton(patternJoin);
		allSample->parents = appendToTailOfList(allSample->parents,patternJoin);
		provSample->parents = singleton(patternJoin);
//		patternJoin->provAttrs = copyObject(provAttrs);

		// create projection operator
		List *projExpr = NIL;
		List *lProjExpr = NIL;
		List *rProjExpr = NIL;
		int pos = 0;
		int numAttr = getNumAttrs(allSample);

		FOREACH(AttributeDef,a,allSample->schema->attrDefs)
		{
//			if(searchListNode(normAttrs,(Node *) a))
			if(strcmp(a->attrName,strdup(HAS_PROV_ATTR)) != 0)
			{
				lProjExpr = appendToTailOfList(lProjExpr,
						createFullAttrReference(strdup(getAttrNameByPos(patternJoin, pos)),
						        0, pos, 0, a->dataType));
                rProjExpr = appendToTailOfList(rProjExpr,
                        createFullAttrReference(strdup(getAttrNameByPos(patternJoin, pos + numAttr)),
                                0, pos + numAttr, 0, a->dataType));
				pos++;
			}
		}

		List *provAttrNames = NIL;

		FORBOTH(Node,l,r,lProjExpr,rProjExpr)
		{
			AttributeDef *a = (AttributeDef *) r;

			if(isPrefix(a->attrName,"PROV_"))
			{
				provAttrNames = appendToTailOfList(provAttrNames,a->attrName);

				DataType d = a->dataType;
				Node *cond = (Node *) createOpExpr("=",LIST_MAKE(l,r));

				Node *then = l;
				Node *els = (Node *) createNullConst(d);

				CaseWhen *caseWhen = createCaseWhen(cond, then);
				CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

				projExpr = appendToTailOfList(projExpr, (List *) caseExpr);
			}
		}

		ProjectionOperator *op = createProjectionOp(projExpr, patternJoin, NIL, provAttrNames);
		patternJoin->parents = singleton(op);
		patternJoin = (QueryOperator *) op;
//		patternJoin->provAttrs = copyObject(provAttrs);

		// create duplicate removal
		projExpr = NIL;
		pos = 0;

		FOREACH(AttributeDef,a,patternJoin->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

			pos++;
		}

		QueryOperator *dr = (QueryOperator *) createDuplicateRemovalOp(projExpr, patternJoin, NIL, getAttrNames(patternJoin->schema));
		patternJoin->parents = singleton(dr);
		patternJoin = (QueryOperator *) dr;
//		patternJoin->provAttrs = copyObject(provAttrs);

		result = (Node *) patternJoin;
		SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

		DEBUG_NODE_BEATIFY_LOG("pattern generation for summarization:", result);
		INFO_OP_LOG("pattern generation for summarization as overview:", result);
	}
	else
	{
		result = NULL;
		INFO_OP_LOG("Other pattern generation techniques have not implemented yet!!");
	}

	return result;
}




/*
 * Full sample of prov and non-prov
 */
static Node *
rewriteSampleOutput (Node *randProv, Node *randNonProv, int sampleSize, char *qType)
{
	Node *result;
	List *allInput = NIL;
	QueryOperator *randomProv = (QueryOperator *) randProv;

	if(streq(qType,"WHY"))
	{
		/* sampling from prov */
		int provSize = sampleSize / 2;

		// create selection clause
		Node *selCond = NULL;
		selCond = (Node *) createOpExpr("<=",LIST_MAKE(makeNode(RowNumExpr),createConstInt(provSize)));
		SelectionOperator *so = createSelectionOp(selCond, randomProv, NIL, getAttrNames(randomProv->schema));

		randomProv->parents = singleton(so);
		randomProv = (QueryOperator *) so;

		// compute non-prov size
		int hasProvPos = LIST_LENGTH(getAttrNames(randomProv->schema)) - 1;
		AttributeReference *hasProv = createFullAttrReference(strdup(HAS_PROV_ATTR),0,hasProvPos,0,DT_INT);
		FunctionCall *countProv = createFunctionCall("COUNT", singleton(hasProv));
		countProv->isAgg = TRUE;

		Node *nonProvSize = (Node *) createOpExpr("-", LIST_MAKE(createConstInt(sampleSize),countProv));
		ProjectionOperator *nonProvOp = createProjectionOp(singleton(nonProvSize),randomProv,NIL,singleton(strdup(NON_PROV_SIZE)));

		/* sampling from non-prov */
		QueryOperator *randomNonProv = (QueryOperator *) randNonProv;

		// create CROSS_JOIN operator
		List *crossInput = LIST_MAKE(randomNonProv, (QueryOperator *) nonProvOp);
		List *attrNames = concatTwoLists(getAttrNames(randomNonProv->schema),singleton(strdup(NON_PROV_SIZE)));
		QueryOperator *randomNonProvJoin= (QueryOperator *) createJoinOp(JOIN_CROSS,NULL,crossInput,NIL,attrNames);
		OP_LCHILD(randomNonProvJoin)->parents = OP_RCHILD(randomNonProvJoin)->parents = singleton(randomNonProvJoin);

		int pos = 0;
		List *projExpr = NIL;
		attrNames = NIL;

		FOREACH(AttributeDef,a,randomNonProvJoin->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
							createFullAttrReference(strdup(a->attrName),0,pos,0,a->dataType));

			attrNames = appendToTailOfList(attrNames,strdup(a->attrName));
			pos++;
		}

		ProjectionOperator *intermedProj = createProjectionOp(projExpr,randomNonProvJoin,NIL,attrNames);
		randomNonProvJoin->parents = singleton(intermedProj);
		randomNonProvJoin = (QueryOperator *) intermedProj;

		// generate non-prov for the size of (sampleSize - sampleProvSize/2)
		AttributeReference *npSize = (AttributeReference *) getNthOfListP(intermedProj->projExprs,pos-1);
		selCond = (Node *) createOpExpr("<=",LIST_MAKE(makeNode(RowNumExpr), npSize));
		so = createSelectionOp(selCond,randomNonProvJoin,NIL,getAttrNames(randomNonProvJoin->schema));

		randomNonProvJoin->parents = singleton(so);
		randomNonProvJoin = (QueryOperator *) so;

		// remove the attribute of nonProvSize
		pos = 0;
		projExpr = NIL;
		attrNames = NIL;

		FOREACH(AttributeDef,a,randomNonProvJoin->schema->attrDefs)
		{
			if(!streq(a->attrName,NON_PROV_SIZE))
			{
				projExpr = appendToTailOfList(projExpr,
								createFullAttrReference(strdup(a->attrName),0,pos,0,a->dataType));

				attrNames = appendToTailOfList(attrNames,strdup(a->attrName));
			}
			pos++;
		}

		intermedProj = createProjectionOp(projExpr,randomNonProvJoin,NIL,attrNames);
		randomNonProvJoin->parents = singleton(intermedProj);
		randomNonProvJoin = (QueryOperator *) intermedProj;


	//
	//	/*
	//	 * create sample based on the previous method
	//	 */
	//	List *attrNames = NIL;
	//	QueryOperator *randomProvL = (QueryOperator *) randProv;
	//	QueryOperator *randomProvR = (QueryOperator *) randProv;
	//
	//	attrNames = getAttrNames(randomProvL->schema);
	//
	//	// create projection for adding "ROWNUM" for the left input
	//	int pos = 0;
	//	List *projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randomProvL->schema->attrDefs)
	//	{
	//		projExpr = appendToTailOfList(projExpr,
	//				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		pos++;
	//	}
	//	projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));
	//	attrNames = appendToTailOfList(attrNames, strdup(SAMP_NUM_L_ATTR));
	//
	//	ProjectionOperator *leftOp = createProjectionOp(projExpr, randomProvL, NIL, attrNames);
	//	randomProvL->parents = singleton(leftOp);
	//	randomProvL = (QueryOperator *) leftOp;
	//	randomProvL->provAttrs = copyObject(provAttrs);
	//
	//	// create projection for computing sample size for the right input
	////	AttributeReference *countProv = createFullAttrReference(strdup("1"), 0, 0, 0, DT_INT);
	//	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(createConstInt(1)));
	//	fcCount->isAgg = TRUE;
	//
	//	float spSize = 0.0;
	//
	//	if (samplePerc != 0)
	//		spSize = (float) samplePerc / 100;
	//	else
	//		spSize = 0.1; // TODO: Whole or still do sampling?
	//
	//	Node* sampSize = (Node *) createOpExpr("*",LIST_MAKE(fcCount,createConstFloat(spSize)));
	//	FunctionCall *fcCeil = createFunctionCall("CEIL", singleton(sampSize));
	//
	//	ProjectionOperator *rightOp = createProjectionOp(singleton(fcCeil), randomProvR, NIL, singleton(strdup(SAMP_NUM_R_ATTR)));
	//	randomProvR->parents = appendToTailOfList(randomProvR->parents, rightOp);
	//	randomProvR = (QueryOperator *) rightOp;
	//
	//	// create JOIN operator
	//	QueryOperator *left = (QueryOperator *) leftOp;
	//	QueryOperator *right = (QueryOperator *) rightOp;
	//	Node *joinCond = NULL;
	//
	//	FOREACH(AttributeDef,l,left->schema->attrDefs)
	//	{
	//		FOREACH(AttributeDef,r,right->schema->attrDefs)
	//		{
	//			if (streq(l->attrName,SAMP_NUM_L_ATTR) && streq(r->attrName,SAMP_NUM_R_ATTR))
	//			{
	//				AttributeReference *lA, *rA = NULL;
	//				lA = createFullAttrReference(strdup(l->attrName), 0, LIST_LENGTH(left->schema->attrDefs)-1, 0, l->dataType);
	//				rA = createFullAttrReference(strdup(r->attrName), 1, 0, 0, r->dataType);
	//				joinCond = (Node *) createOpExpr("<=",LIST_MAKE(lA,rA));
	//			}
	//		}
	//	}
	//
	//	List *crossInput = LIST_MAKE(left, right);
	//	attrNames = concatTwoLists(getAttrNames(left->schema),getAttrNames(right->schema));
	//	QueryOperator *randProvJoin = (QueryOperator *) createJoinOp(JOIN_INNER, joinCond, crossInput, NIL, attrNames);
	//
	//	// set the parent of the operator's children
	//	left->parents = appendToTailOfList(left->parents,randProvJoin);
	//	right->parents = appendToTailOfList(right->parents,randProvJoin);
	//
	//	// create projection to remove sampNum attrs
	//	pos = 0;
	//	projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randProvJoin->schema->attrDefs)
	//	{
	//		if (!isPrefix(p->attrName,SAMP_NUM_PREFIX))
	//		{
	//			projExpr = appendToTailOfList(projExpr,
	//					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		}
	//		pos++;
	//	}
	//
	//	attrNames = getAttrNames(((QueryOperator *) randProv)->schema);
	//	ProjectionOperator *op = createProjectionOp(projExpr, randProvJoin, NIL, attrNames);
	//	randProvJoin->parents = singleton(op);
	//	randProvJoin = (QueryOperator *) op;
	//	randProvJoin->provAttrs = copyObject(provAttrs);
	//
	//
	//	// sampling from random ordered non-provenance tuples
	//	QueryOperator *randomNonProvL = (QueryOperator *) randNonProv;
	//	QueryOperator *randomNonProvR = (QueryOperator *) randNonProv;
	//
	//	attrNames = getAttrNames(randomNonProvL->schema);
	//
	//	// create projection for adding "ROWNUM" for the left input
	//	pos = 0;
	//	projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randomNonProvL->schema->attrDefs)
	//	{
	//		projExpr = appendToTailOfList(projExpr,
	//				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		pos++;
	//	}
	//	projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));
	//	attrNames = appendToTailOfList(attrNames, strdup(SAMP_NUM_L_ATTR));
	//
	//	leftOp = createProjectionOp(projExpr, randomNonProvL, NIL, attrNames);
	//	randomNonProvL->parents = singleton(leftOp);
	//	randomNonProvL = (QueryOperator *) leftOp;
	//	randomNonProvL->provAttrs = copyObject(provAttrs);
	//
	//	// create projection for computing sample size for the right input
	//	rightOp = createProjectionOp(singleton(fcCeil), randomNonProvR, NIL, singleton(strdup(SAMP_NUM_R_ATTR)));
	//	randomNonProvR->parents = appendToTailOfList(randomNonProvR->parents, rightOp);
	//	randomNonProvR = (QueryOperator *) rightOp;
	//
	//	// create JOIN operator
	//	left = (QueryOperator *) leftOp;
	//	right = (QueryOperator *) rightOp;
	//	joinCond = NULL;
	//
	//	FOREACH(AttributeDef,l,left->schema->attrDefs)
	//	{
	//		FOREACH(AttributeDef,r,right->schema->attrDefs)
	//		{
	//		    if (streq(l->attrName,SAMP_NUM_L_ATTR) && streq(r->attrName,SAMP_NUM_R_ATTR))
	//			{
	//				AttributeReference *lA, *rA = NULL;
	//				lA = createFullAttrReference(strdup(l->attrName), 0, LIST_LENGTH(left->schema->attrDefs)-1, 0, l->dataType);
	//				rA = createFullAttrReference(strdup(r->attrName), 1, 0, 0, r->dataType);
	//				joinCond = (Node *) createOpExpr("<=",LIST_MAKE(lA,rA));
	//			}
	//		}
	//	}
	//
	//	crossInput = LIST_MAKE(left, right);
	//	attrNames = concatTwoLists(getAttrNames(left->schema),getAttrNames(right->schema));
	//	QueryOperator *randNonProvJoin = (QueryOperator *) createJoinOp(JOIN_INNER, joinCond, crossInput, NIL, attrNames);
	//
	//	// set the parent of the operator's children
	//	left->parents = appendToTailOfList(left->parents,randNonProvJoin);
	//	right->parents = appendToTailOfList(right->parents,randNonProvJoin);
	//
	//	// create projection to remove sampNum attrs
	//	pos = 0;
	//	projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randNonProvJoin->schema->attrDefs)
	//	{
	//		if (!isPrefix(p->attrName,SAMP_NUM_PREFIX))
	//		{
	//			projExpr = appendToTailOfList(projExpr,
	//					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		}
	//		pos++;
	//	}
	//
	//	attrNames = getAttrNames(((QueryOperator *) randNonProv)->schema);
	//	op = createProjectionOp(projExpr, randNonProvJoin, NIL, attrNames);
	//	randNonProvJoin->parents = singleton(op);
	//	randNonProvJoin = (QueryOperator *) op;
	//	randNonProvJoin->provAttrs = copyObject(provAttrs);

		allInput = LIST_MAKE(randomProv,randomNonProvJoin);
	}
	else if(streq(qType,"WHYNOT"))
	{
		// make attr name with "PROV_"
		FOREACH(AttributeDef,a,randomProv->schema->attrDefs)
			if(!streq(a->attrName,HAS_PROV_ATTR))
				a->attrName = CONCAT_STRINGS("PROV_",a->attrName);

		FOREACH(AttributeDef,a,((QueryOperator *) randNonProv)->schema->attrDefs)
			if(!streq(a->attrName,HAS_PROV_ATTR))
				a->attrName = CONCAT_STRINGS("PROV_",a->attrName);

		allInput = LIST_MAKE(randomProv,randNonProv);
	}

	// create UNION operator to get all sample
	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION,allInput,NIL,getAttrNames(randomProv->schema));
	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);
//	unionOp->provAttrs = copyObject(provAttrs);

	result = (Node *) unionOp;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sampling for summarization:", result);
	INFO_OP_LOG("sampling for summarization as overview:", result);

	return result;
}


//
///*
// * compute sample size based on the sample percentile requested
// * TODO: how to use sample percentile for sample size
// * (e.g., what would the sample size be based on?)
// */
//
//static int *computeSampleSize (int *samplePerc, Node *prov)
//{
//	QueryOperator *provOp = (QueryOperator *) prov;
//
//	// create count operator with sample percentile
//}


/*
 * random sampling for non-prov tuples
 */
static Node *
rewriteRandomNonProvTuples (Node *provExpl, char *qType, List *fPattern)
{
	Node *result;
	QueryOperator *randomNonProv = (QueryOperator *) provExpl;

	if(streq(qType,"WHY"))
	{
		List *attrNames = NIL;
		// random sampling from hasProv = 0
		attrNames = getAttrNames(randomNonProv->schema);

		// create selection for the prov instance
		int aPos = LIST_LENGTH(randomNonProv->schema->attrDefs) - 1;
		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

		Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(0)));
		SelectionOperator *so = createSelectionOp(whereClause, randomNonProv, NIL, attrNames);

		randomNonProv->parents = appendToTailOfList(randomNonProv->parents, so);
		randomNonProv = (QueryOperator *) so;
		randomNonProv->provAttrs = copyObject(provAttrs);

		// create projection for adding HAS_PROV_ATTR attribute
		int pos = 0;
		List *projExpr = NIL;
		ProjectionOperator *op;

		FOREACH(AttributeDef,p,randomNonProv->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
			pos++;
		}

		op = createProjectionOp(projExpr, randomNonProv, NIL, attrNames);
		randomNonProv->parents = singleton(op);
		randomNonProv = (QueryOperator *) op;
		randomNonProv->provAttrs = copyObject(provAttrs);

		// create order by operator
		Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
		OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);

		OrderOperator *ord = createOrderOp(singleton(ordExpr), randomNonProv, NIL);
		randomNonProv->parents = singleton(ord);
		randomNonProv = (QueryOperator *) ord;
		randomNonProv->provAttrs = copyObject(provAttrs);
	}
	else if(streq(qType,"WHYNOT"))
	{
		int numAttrInput = LIST_LENGTH(randomNonProv->schema->attrDefs);
		int lenOfPattern = LIST_LENGTH(fPattern);
		int patternPos = numAttrInput - lenOfPattern;
		int pos = 0;

		// create selection condition
		Node *curCond = NULL;

		while(patternPos < numAttrInput)
		{
			Node *attrCond = NULL;
			AttributeDef *a = (AttributeDef *) getNthOfListP(randomNonProv->schema->attrDefs, patternPos);

			AttributeReference *lA = createFullAttrReference(strdup(a->attrName),0,patternPos,0,a->dataType);
			Node *rA = (Node *) createConstInt(INT_VALUE(getNthOfListP(fPattern,pos)));

			attrCond = (Node *) createOpExpr("<>",LIST_MAKE(lA,rA));

			if(pos == 0)
				curCond = attrCond;
			else if(pos > 0)
				curCond = OR_EXPRS(curCond,attrCond);

			patternPos++;
			pos++;
		}

//		// add NOT in where clause
//		Node *notOp = (Node *) createOpExpr("NOT ", singleton(curCond));
		SelectionOperator *selCond = createSelectionOp(curCond, randomNonProv, NIL, NIL);
		randomNonProv->parents = singleton(selCond);
		randomNonProv = (QueryOperator *) selCond;

		// create projection operator
		List *projExpr = NIL;
		List *attrNames = NIL;
		patternPos = numAttrInput - lenOfPattern;
		pos = 0;

		FOREACH(AttributeDef,a,randomNonProv->schema->attrDefs)
		{
			if(pos < patternPos)
			{
				attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
				projExpr = appendToTailOfList(projExpr,
								createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
			}
			pos++;
		}

		// add 0 as a hasProv
		Node *hasProv = (Node *) createConstInt(0);
		projExpr = appendToTailOfList(projExpr, hasProv);
		attrNames = appendToTailOfList(attrNames, strdup(HAS_PROV_ATTR));

		ProjectionOperator *projOp = createProjectionOp(projExpr, randomNonProv, NIL, attrNames);
		randomNonProv->parents = singleton(projOp);
		randomNonProv = (QueryOperator *) projOp;
	}

	result = (Node *) randomNonProv;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("random order for non-provenance tuples for summarization:", result);
	INFO_OP_LOG("random order for non-provenance tuples for summarization as overview:", result);

	return result;
}


/*
 * random sampling for prov tuples
 */
static Node *
rewriteRandomProvTuples (Node *provExpl, char *qType, List *fPattern)
{
	Node *result;
	QueryOperator *randomProv = (QueryOperator *) provExpl;

	if(streq(qType,"WHY"))
	{
		List *attrNames = NIL;
		// random sampling from hasProv = 1
//		randomProv = (QueryOperator *) provExpl;
		attrNames = getAttrNames(randomProv->schema);

		// create selection for the prov instance
		int aPos = LIST_LENGTH(randomProv->schema->attrDefs) - 1;
		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

		Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
		SelectionOperator *so = createSelectionOp(whereClause, randomProv, NIL, attrNames);

		randomProv->parents = singleton(so);
		randomProv = (QueryOperator *) so;
		randomProv->provAttrs = copyObject(provAttrs);

		// create projection for adding HAS_PROV_ATTR attribute
		int pos = 0;
		List *projExpr = NIL;
		ProjectionOperator *op;

		FOREACH(AttributeDef,p,randomProv->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
			pos++;
		}

		op = createProjectionOp(projExpr, randomProv, NIL, attrNames);
		randomProv->parents = singleton(op);
		randomProv = (QueryOperator *) op;
		randomProv->provAttrs = copyObject(provAttrs);

		// create order by operator
		Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
		OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);

		OrderOperator *ord = createOrderOp(singleton(ordExpr), randomProv, NIL);
		randomProv->parents = singleton(ord);
		randomProv = (QueryOperator *) ord;
		randomProv->provAttrs = copyObject(provAttrs);
	}
	else if(streq(qType,"WHYNOT"))
	{
		int numAttrInput = LIST_LENGTH(randomProv->schema->attrDefs);
		int lenOfPattern = LIST_LENGTH(fPattern);
		int patternPos = numAttrInput - lenOfPattern;
		int pos = 0;

		// create selection condition
		Node *curCond = NULL;

		while(patternPos < numAttrInput)
		{
			Node *attrCond = NULL;
			AttributeDef *a = (AttributeDef *) getNthOfListP(randomProv->schema->attrDefs, patternPos);

			AttributeReference *lA = createFullAttrReference(strdup(a->attrName),0,patternPos,0,a->dataType);
			Node *rA = (Node *) createConstInt(INT_VALUE(getNthOfListP(fPattern,pos)));

			attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));

			if(pos == 0)
				curCond = attrCond;
			else if(pos > 0)
				curCond = AND_EXPRS(curCond,attrCond);

			patternPos++;
			pos++;
		}

		SelectionOperator *selCond = createSelectionOp(curCond, randomProv, NIL, NIL);
		randomProv->parents = singleton(selCond);
		randomProv = (QueryOperator *) selCond;

		// create projection operator
		List *projExpr = NIL;
		List *attrNames = NIL;
		patternPos = numAttrInput - lenOfPattern;
		pos = 0;

		FOREACH(AttributeDef,a,randomProv->schema->attrDefs)
		{
			if(pos < patternPos)
			{
				attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
				projExpr = appendToTailOfList(projExpr,
								createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
			}
			pos++;
		}

		// add 1 as a hasProv
		Node *hasProv = (Node *) createConstInt(1);
		projExpr = appendToTailOfList(projExpr, hasProv);
		attrNames = appendToTailOfList(attrNames, strdup(HAS_PROV_ATTR));

		ProjectionOperator *projOp = createProjectionOp(projExpr, randomProv, NIL, attrNames);
		randomProv->parents = singleton(projOp);
		randomProv = (QueryOperator *) projOp;
	}

	result = (Node *) randomProv;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("random order for provenance tuples for summarization:", result);
	INFO_OP_LOG("random order for provenance tuples for summarization as overview:", result);

	return result;
}


/*
 * replace cross product with sample domain generated
 */
static Node *
replaceDomWithSampleDom (Node *sampleDom, Node *input)
{
	Node *result;
	Node *whyNotRuleFire = (Node *) getHeadOfListP((List *) input);

	// TODO: function for capturing sub-rooted RA for domain
	// count number of attrs in sample domain
	QueryOperator *sDom = (QueryOperator *) sampleDom;
	int numOfAttrs = LIST_LENGTH(sDom->schema->attrDefs);

	List *rels = NIL;
	findTableAccessVisitor((Node *) whyNotRuleFire,&rels);

	FOREACH(TableAccessOperator,t,rels)
	{
		if(streq(t->tableName,"DQ"))
		{
			// use the count to capture sub-rooted RA for domain
			QueryOperator *tBase = (QueryOperator *) t;

			while(numOfAttrs > 0)
			{
				tBase = (QueryOperator *) getHeadOfListP(tBase->parents);
				numOfAttrs--;
			}

			// replace the sub-rooted RA for domain with sample domain
			switchSubtreeWithExisting((QueryOperator *) tBase, (QueryOperator *) sampleDom);
			DEBUG_LOG("replaced domain %s with\n:%s", operatorToOverviewString((Node *) tBase),
					operatorToOverviewString((Node *) sampleDom));
		}
	}

	result = (Node *) whyNotRuleFire;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sample based why-not explanation:", result);
	INFO_OP_LOG("sample based why-not explanation:", result);

	return result;
}


/*
 * for WHYNOT, join on seq
 */
static Node *
joinOnSeqOutput (List *doms)
{
	Node *result;
	QueryOperator *sampDom;
	List *inputs = NIL;
	List *attrNames = NIL;
	int lApos = 0;
	AttributeReference *lA = NULL;

	inputs = appendToTailOfList(inputs, (Node *) getNthOfListP(doms,0));
	QueryOperator *firstOp = (QueryOperator *) getNthOfListP(doms,0);
	FOREACH(AttributeDef,a,firstOp->schema->attrDefs)
	{
		if(streq(a->attrName,"SEQ"))
			lA = createFullAttrReference(strdup(a->attrName),0,lApos,0,a->dataType);

		attrNames = appendToTailOfList(attrNames, a->attrName);
		lApos++;
	}

	for(int i = 1; i < LIST_LENGTH(doms); i++)
	{
		Node *n = (Node *) getNthOfListP(doms,i);

		if(i == 1)
			inputs = appendToTailOfList(inputs,n);
		else
			inputs = LIST_MAKE(sampDom, n);

		QueryOperator *oDom = (QueryOperator *) n;
		AttributeReference *rA = NULL;
		int rApos = 0;

		FOREACH(AttributeDef,oa,oDom->schema->attrDefs)
		{
			if(streq(oa->attrName,"SEQ"))
				rA = createFullAttrReference(strdup(oa->attrName),1,rApos,0,oa->dataType);

			attrNames = appendToTailOfList(attrNames, oa->attrName);
			rApos++;
		}

		// create join on "seq"
		if(lA != NULL && rA != NULL)
		{
			Node *cond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
			sampDom = (QueryOperator *) createJoinOp(JOIN_INNER, cond, inputs, NIL, attrNames);
		}

		if(i == 1)
			firstOp->parents = singleton(sampDom);
		else
			OP_LCHILD(sampDom)->parents = singleton(sampDom);

		oDom->parents = singleton(sampDom);
	}

	// add projection operator to remove seq attr
	List *projExpr = NIL;
	attrNames = NIL;
	int pos = 0;

	FOREACH(AttributeDef,a,sampDom->schema->attrDefs)
	{
		if(!streq(a->attrName,"SEQ"))
		{
			projExpr = appendToTailOfList(projExpr,createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
			attrNames = appendToTailOfList(attrNames, a->attrName);
		}

		pos++;
	}
	ProjectionOperator *p = createProjectionOp(projExpr,sampDom,NIL,attrNames);
	sampDom->parents = singleton(p);
	sampDom = (QueryOperator *) p;

	result = (Node *) sampDom;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sample domain based on the seq:", result);
	INFO_OP_LOG("sample domain based on the seq:", result);

	return result;
}


/*
 * For SQL, create base input for the summarization by
 * joining whole provenance and a user specific provenance
 * and mark user specific as 1 and 0 otherwise
 *
 * TODO: for Datalog, we need to have whole prov
 */
static Node *
rewriteProvJoinOutput (Node *rewrittenTree)
{
	Node *result;
	QueryOperator *prov;

//	if(userQuestion == NIL)
	if(isA(rewrittenTree, List))
		prov = (QueryOperator *) getHeadOfListP((List *) rewrittenTree);
	else
		prov = (QueryOperator *) rewrittenTree;

	// For dl, make attr names starting with "PROV"
	if(isDl)
	{
		// replace the attr names starting with "PROV_"
		FOREACH(QueryOperator,q,prov->inputs)
			FOREACH(AttributeDef,a,q->schema->attrDefs)
				a->attrName = CONCAT_STRINGS("PROV_",a->attrName);

		FOREACH(AttributeDef,a,prov->schema->attrDefs)
			a->attrName = CONCAT_STRINGS("PROV_",a->attrName);
	}

	// take the input query out for use with join operator later
	QueryOperator *transInput = (QueryOperator *) prov->properties;

	// store normal and provenance attributes for later use
	if(provAttrs == NIL || normAttrs == NIL)
	{
		if(isDl)
		{
//			FOREACH(AttributeDef,a,prov->schema->attrDefs)
//				provAttrs = appendToTailOfList(provAttrs, CONCAT_STRINGS("i",itoa(getAttrPos(prov, a->attrName))));
			ProjectionOperator *p = (ProjectionOperator *) getHeadOfListP((List *) transInput->inputs);
			normAttrs = copyObject(p->projExprs);

			// store user question attrs
			userQuestion = normAttrs;
		}
		else
		{
			provAttrs = getProvenanceAttrs(prov);
			normAttrs = getNormalAttrs(prov);
		}
	}

	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
	List *provAttrNames = NIL;
//	QueryOperator *origProv = prov;

	// create projection for adding HAS_PROV_ATTR attribute
	FOREACH(AttributeDef,p,prov->schema->attrDefs)
	{
		if(isPrefix(p->attrName,"PROV_"))
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));

			provAttrNames = appendToTailOfList(provAttrNames, strdup(p->attrName));
		}
		pos++;
	}
	projExpr = appendToTailOfList(projExpr,createConstInt(1));

	// add an attribute for prov
//	int attrPos = LIST_LENGTH(transInput->schema->attrDefs) + LIST_LENGTH(prov->schema->attrDefs);
//	int attrPos = LIST_LENGTH(prov->schema->attrDefs);
//	AttributeDef *hasProv = (AttributeDef *) createFullAttrReference(strdup(HAS_PROV_ATTR), 0, attrPos, 0, DT_INT);

//	List *newAttrs = concatTwoLists(getAttrNames(prov->schema),singleton(hasProv->attrName));
	List *newAttrs = concatTwoLists(provAttrNames,singleton(HAS_PROV_ATTR));
	op = createProjectionOp(projExpr, prov, NIL, newAttrs);

	prov->parents = singleton(op);
	prov = (QueryOperator *) op;
//	prov->provAttrs = copyObject(provAttrs);

	// create join condition
	// TODO: find the corresponding provenance attribute to join if the name of attrs repeats
//	boolean suffix = FALSE;
	Node *curCond = NULL;
	int aPos = 0;
	int chkPos = 0;
	List *inputs = NIL;
	List *attrNames = NIL;
	int orgAttr = LIST_LENGTH(transInput->schema->attrDefs);

	FOREACH(AttributeDef,ia,transInput->schema->attrDefs)
	{
		Node *attrCond = NULL;
		AttributeReference *lA, *rA = NULL;
		lA = createFullAttrReference(strdup(ia->attrName), 0, aPos, 0, ia->dataType);

//		// check suffix upfront to recognize if attributes are renamed
		for(int provAttr = orgAttr; provAttr < LIST_LENGTH(prov->schema->attrDefs); provAttr++)
		{
			AttributeDef *pa = getAttrDefByPos(prov,provAttr);

			if(isSuffix(pa->attrName,ia->attrName))
			{
				rA = createFullAttrReference(strdup(pa->attrName), 1, provAttr, 0, pa->dataType);
				attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
				chkPos++;

				if(chkPos == 1)
					curCond = attrCond;
				else if (chkPos > 1)
					curCond = AND_EXPRS(curCond,attrCond);
			}
			else if(streq(ia->attrName,pa->attrName))
				FATAL_LOG("USING join is using ambiguous attribute references <%s>", ia->attrName);
		}
		aPos++;
	}

	// no matches exist on name, then match by position
	if(curCond == NULL || chkPos > orgAttr) // then there exist repeating attrs
	{
		List *orgRef = ((ProjectionOperator *) transInput)->projExprs;
		chkPos = 0;
		int attrPos = 0;
		curCond = NULL;

		FOREACH(AttributeReference,a,orgRef)
		{
			Node *attrCond;
			AttributeReference *lA, *rA = NULL;

			int matPos = a->attrPosition + LIST_LENGTH(orgRef);
			lA = createFullAttrReference(strdup(a->name), 0, attrPos, 0, a->attrType);

//			for(int rPos = 0; rPos < LIST_LENGTH(prov->schema->attrDefs); rPos++)
			List *provRef = ((ProjectionOperator *) prov)->projExprs;

			FOREACH(AttributeReference,rPos,provRef)
			{
				if(rPos->attrPosition == matPos)
				{
//					AttributeDef *r = getAttrDefByPos(prov,rPos);
					rA = createFullAttrReference(strdup(rPos->name), 1, rPos->attrPosition, 0, rPos->attrType);
					attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
					chkPos++;

					if(chkPos == 1)
						curCond = attrCond;
					else if(chkPos > 1)
						curCond = AND_EXPRS(curCond,attrCond);
				}
			}
			attrPos++;
		}

		if(isDl)
		{
			// attrs from input query
			QueryOperator *dup = (QueryOperator *) transInput;
			int pos = 0;

			// create join condition based on the attr names
			FOREACH(AttributeDef,ia,dup->schema->attrDefs)
			{
				Node *attrCond;
				AttributeReference *lA = createFullAttrReference(strdup(ia->attrName), 0, pos, 0, ia->dataType);

				FOREACH(AttributeDef,ra,prov->schema->attrDefs)
				{
					if(isSuffix(ra->attrName,lA->name))
					{
						AttributeReference *rA = createFullAttrReference(strdup(ra->attrName), 1, pos, 0, ra->dataType);
						attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));

						if(curCond == NULL)
							curCond = attrCond;
						else
							curCond = AND_EXPRS(curCond,attrCond);
					}
				}
				pos++;
			}
		}
	}

	inputs = LIST_MAKE(transInput,prov);

	// create join operator
	attrNames = concatTwoLists(getAttrNames(transInput->schema), getAttrNames(prov->schema));
	QueryOperator *provJoin = (QueryOperator *) createJoinOp(JOIN_LEFT_OUTER, curCond, inputs, NIL, attrNames);
	makeAttrNamesUnique(provJoin);

	// set the parent of the operator's children
	OP_LCHILD(provJoin)->parents = OP_RCHILD(provJoin)->parents = singleton(provJoin);
//	provJoin->provAttrs = copyObject(provAttrs);

	// create projection for join
	projExpr = NIL;
	pos = 0;

	FOREACH(AttributeDef,a,provJoin->schema->attrDefs)
	{
		if(!streq(a->attrName,strdup(HAS_PROV_ATTR)))
		{
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
			pos++;
		}
	}

	AttributeReference *hasProv = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, pos, 0, DT_INT);
	Node *cond = (Node *) createIsNullExpr((Node *) hasProv);
	Node *then = (Node *) createConstInt(0);
	Node *els = (Node *) createConstInt(1);

	CaseWhen *caseWhen = createCaseWhen(cond, then);
	CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

	projExpr = appendToTailOfList(projExpr, (List *) caseExpr);
	DEBUG_LOG("projection expressions for join: %s", projExpr);

//	attrNames = concatTwoLists(getAttrNames(transInput->schema), getAttrNames(prov->schema));

//	Set *allNames = STRSET();
//	List *uniqueAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(provJoin),singleton(hasProv->attrName));
//	makeNamesUnique(uniqueAttrNames, allNames);

	op = createProjectionOp(projExpr, provJoin, NIL, getAttrNames(provJoin->schema));
	provJoin->parents = singleton(op);
	provJoin = (QueryOperator *) op;
	provJoin->provAttrs = copyObject(provAttrs);

	// create duplicate removal
	projExpr = NIL;
	pos = 0;

	FOREACH(AttributeDef,a,provJoin->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

		pos++;
	}

	QueryOperator *dr = (QueryOperator *) createDuplicateRemovalOp(projExpr, provJoin, NIL, getAttrNames(provJoin->schema));
	provJoin->parents = singleton(dr);
	provJoin = (QueryOperator *) dr;


	result = (Node *) provJoin;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("join input with provenance question for summarization returned:", result);
	INFO_OP_LOG("join input with provenance question for summarization as overview:", result);

	return result;
}


/*
 * For SQL input, integrate a particular user's interest into provenance computation
 * for Datalog, this step should be skipped since it is already part of the output of PUG
 */
static Node *
rewriteUserQuestion (List *userQ, Node *rewrittenTree)
{
	Node *result;

	QueryOperator *input = (QueryOperator *) getHeadOfListP((List *) rewrittenTree);
	Node *prop = input->properties;

	if (provAttrs == NIL || normAttrs == NIL)
	{
		provAttrs = getProvenanceAttrs(input);
		normAttrs = getNormalAttrs(input);
	}

	// get attrRefs for the input
	List *inputAttrRefs = ((ProjectionOperator *) input)->projExprs;

	// check the list for constant value to create sel condition
	int chkPos = 0;
	int attrPos = 0;
	List *origUserQattrs = NIL;
	Node *curCond = NULL;
	SelectionOperator *so;

	FOREACH(Constant,c,userQ)
	{
		if (!streq(strdup(c->value),"*"))
		{
			char *attr = getAttrNameByPos(input,attrPos);
			AttributeDef *aDef = getAttrDefByName(input,attr);

			AttributeReference *quest = createFullAttrReference(strdup(attr), 0, attrPos, 0, aDef->dataType);
			Node *selCond = (Node *) createOpExpr("=",LIST_MAKE(quest,c));

			if(chkPos == 0)
				curCond = selCond;
			else
				curCond = AND_EXPRS(curCond,selCond);

			chkPos++;

			// store user question attrRefs for later use of attrDom computation
			origUserQattrs = appendToTailOfList(origUserQattrs,
						(AttributeReference *) getNthOfListP(inputAttrRefs,attrPos));
		}

		attrPos++;
	}
	so = createSelectionOp(curCond, input, NIL, NIL);
	userQuestion = origUserQattrs;

	input->parents = singleton(so);
	input = (QueryOperator *) so;

	// create projection operator
	int pos = 0;
//	List *attrs = NIL;
	List *projExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,input->schema->attrDefs)
	{
//		if(isPrefix(p->attrName,"PROV_"))
//		{
//			attrs = appendToTailOfList(attrs,p->attrName);

			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
//		}
		pos++;
	}

	op = createProjectionOp(projExpr, input, NIL, getAttrNames(input->schema));
	input->parents = singleton(op);
	input = (QueryOperator *) op;
	input->provAttrs = copyObject(provAttrs);

	input->properties = prop;
	result = (Node *) input;
//	SET_BOOL_STRING_PROP(rewrittenTree, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("provenance question for summarization:", result);
	INFO_OP_LOG("provenance question for summarization as overview:", result);

	return result;
}
