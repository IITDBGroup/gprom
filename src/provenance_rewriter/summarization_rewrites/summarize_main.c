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
#define SAMP_NUM_L_ATTR SAMP_NUM_PREFIX "Left"
#define SAMP_NUM_R_ATTR SAMP_NUM_PREFIX "Right"

static List *provAttrs = NIL;
static List *normAttrs = NIL;
static List *userQattrs = NIL;
static Node *moveRels = NULL;

static Node *rewriteUserQuestion (List *userQuestion, Node *rewrittenTree);
static Node *rewriteProvJoinOutput (List *userQuestion, Node *rewrittenTree);
static Node *rewriteRandomProvTuples (Node *input);
static Node *rewriteRandomNonProvTuples (Node *input);
static Node *rewriteSampleOutput (Node *randProv, Node *randNonProv, int sampleSize);
static Node *rewritePatternOutput (char *summaryType, Node *input);
static Node *rewriteScanSampleOutput (Node *sampleInput, Node *patternInput);
static Node *rewriteCandidateOutput (Node *scanSampleInput);
static List *domAttrsOutput (List *userQattrs, Node *sampleInput);
static Node *scaleUpOutput (List *doms, Node *candInput, Node *provJoin, Node *randProv, Node *randNonProv);
static Node *rewriteComputeFracOutput (Node *scaledCandInput, Node *sampleInput);
static Node *rewritefMeasureOutput (Node *computeFracInput, List *userQuestion);
static Node *rewriteMostGenExplOutput (Node *fMeasureInput, int topK);

Node *
rewriteSummaryOutput (Node *rewrittenTree, List *summOpts)
{
	// options for summarization
	List *userQuestion = NIL;
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

//	// For dl, separate ruleFire with moveRules
//	if (isA(rewrittenTree, HashMap))
//	{
//		HashMap *summInputs = (HashMap *) rewrittenTree;
//		rewrittenTree = (Node *) MAP_GET_STRING(summInputs, "summAns");
//	}

	// store moveRules in separate
	//TODO: not safe to check whether input comes from dl or SQL
	List *separate = (List *) rewrittenTree;
	if (isA(getHeadOfListP(separate), DuplicateRemoval))
		moveRels = (Node *) getTailOfListP(separate);

	// summarization steps
	List *doms = NIL;
	Node *result, *provJoin, *randomProv, *randomNonProv, *samples, *patterns,
		*scanSamples, *candidates, *scaleUp, *computeFrac, *fMeasure;

	if (userQuestion != NIL)
		rewrittenTree = rewriteUserQuestion(userQuestion, rewrittenTree);

	provJoin = rewriteProvJoinOutput(userQuestion, rewrittenTree);
	randomProv = rewriteRandomProvTuples(provJoin);
	randomNonProv = rewriteRandomNonProvTuples(provJoin);
	samples = rewriteSampleOutput(randomProv, randomNonProv, sampleSize);
	patterns = rewritePatternOutput(summaryType, samples); //TODO: different types of pattern generation
	scanSamples = rewriteScanSampleOutput(samples, patterns);
	candidates = rewriteCandidateOutput(scanSamples);
	doms = domAttrsOutput(userQattrs, rewrittenTree);
	scaleUp = scaleUpOutput(doms, candidates, provJoin, randomProv, randomNonProv);
	computeFrac = rewriteComputeFracOutput(scaleUp, samples);
	fMeasure = rewritefMeasureOutput(computeFrac, userQuestion);
	result = rewriteMostGenExplOutput(fMeasure, topK);

	//TODO: integrate with edge rel for dl
//	if (moveRels != NULL)


	if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
		ASSERT(checkModel((QueryOperator *) result));

	return result;
}

static Node *
rewriteMostGenExplOutput (Node *fMeasureInput, int topK)
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

	FOREACH(AttributeDef,p,fMeasure->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}

	op = createProjectionOp(projExpr, fMeasure, NIL, getAttrNames(fMeasure->schema));
	fMeasure->parents = singleton(op);
	fMeasure = (QueryOperator *) op;

	result = (Node *) fMeasure;

	DEBUG_NODE_BEATIFY_LOG("most general explanation from summarization:", result);
	INFO_OP_LOG("most general explanation from summarization as overview:", result);

	return result;
}


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

	FOREACH(AttributeDef,p,computeFrac->schema->attrDefs)
	{
		if(streq(p->attrName, ACCURACY_ATTR))
			prec = createFullAttrReference(strdup(ACCURACY_ATTR), 0, pos, 0, p->dataType);

		if(streq(p->attrName, COVERAGE_ATTR))
			rec = createFullAttrReference(strdup(COVERAGE_ATTR), 0, pos, 0, p->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
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


static Node *
rewriteComputeFracOutput (Node *scaledCandInput, Node *sampleInput)
{
	Node *result;

	QueryOperator *candidates = (QueryOperator *) scaledCandInput;
//	QueryOperator *samples = (QueryOperator *) sampleInput;

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

	FOREACH(AttributeDef,p,candidates->schema->attrDefs)
	{
		if (pos == 0)
			totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, pos, 0, p->dataType);

		if (pos == 1)
			covProv = createFullAttrReference(strdup(COVERED_ATTR), 0, pos, 0, p->dataType);

		if (pos == 2)
			numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, pos, 0, p->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
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
 * scale up the measure values to real one
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

	FOREACH(AttributeDef,p,crossPdom->schema->attrDefs)
	{
		if(streq(p->attrName,TOTAL_PROV_ATTR))
			totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, pos, 0, p->dataType);
		else if(streq(p->attrName,NUM_PROV_ATTR))
			numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, pos, 0, p->dataType);
		else if(streq(p->attrName,NUM_NONPROV_ATTR))
			numNonProv = createFullAttrReference(strdup(NUM_NONPROV_ATTR), 0, pos, 0, p->dataType);
		else if(streq(p->attrName,TOTAL_PROV_SAMP_ATTR))
			totProvInSamp = createFullAttrReference(strdup(TOTAL_PROV_SAMP_ATTR), 0, pos, 0, p->dataType);
		else if(streq(p->attrName,TOTAL_NONPROV_SAMP_ATTR))
			totNonProvInSamp = createFullAttrReference(strdup(TOTAL_NONPROV_SAMP_ATTR), 0, pos, 0, p->dataType);
		else if(isPrefix(p->attrName,"cnt"))
		{
			if(counter == 0)
			{
				domL = createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType);
				counter++;
			}
			else
				domR = createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType);

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
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));

			attrs = appendToTailOfList(attrs, p);
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

	DEBUG_NODE_BEATIFY_LOG("scale up numInProv and covered for summarization:", result);
	INFO_OP_LOG("scale up numInProv and covered for summarization as overview:", result);

	return result;
}


/*
 * generate domain attrs for later use of scale up of the measure values to the real values
 */
static List *
domAttrsOutput (List *userQattrs, Node *input)
{
	List *result = NIL;

	// replace the attr names in userQattrs with the orig names
	int attrPos = 0;
	FOREACH(AttributeReference, ar, userQattrs)
	{
		AttributeDef *aDef = (AttributeDef *) getNthOfListP(normAttrs, attrPos);
		ar->name = strdup(aDef->attrName);
		attrPos++;
	}

	// translated input algebra to use the table acess operators
	QueryOperator *prov = (QueryOperator *) input;
	QueryOperator *transInput = (QueryOperator *) prov->properties;

	// store table access operator for later use of dom attrs
	List *rels = NIL;
	findTableAccessVisitor((Node *) transInput,&rels);

	int attrCount = 0;
//	int relCount = 0;
	char *relName = NULL;
	AggregationOperator *aggCount;

	FOREACH(TableAccessOperator, t, rels)
	{
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

				if(!searchListNode(userQattrs, (Node *) ar))
				{
					// create count attr
					AttributeReference *countAr = createFullAttrReference(strdup(ar->name), 0, ar->attrPosition - attrCount, 0, DT_INT);
					FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countAr));
					fcCount->isAgg = TRUE;

					// create agg operator
					char *domAttr = CONCAT_STRINGS("cnt",ar->name);
					aggCount = createAggregationOp(singleton(fcCount), NIL, (QueryOperator *) t, NIL, singleton(strdup(domAttr)));
					SET_BOOL_STRING_PROP((Node *) aggCount, PROP_MATERIALIZE);

					DEBUG_NODE_BEATIFY_LOG("dom attrs for summarization:", (Node *) aggCount);
					INFO_OP_LOG("dom attrs for summarization as overview:", (Node *) aggCount);

					result = appendToTailOfList(result, (Node *) aggCount);
				}
				attrCount++;
			}
		}
	}

	return result;
}



/*
 * compute measure values, e.g., numInProv and coverage
 * numInProv: how many prov within whole prov
 * coverage: how many prov or non-prov are covered by the pattern
 */
static Node *
rewriteCandidateOutput (Node *scanSampleInput)
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

	Constant *sumHasNonProv = createConstInt(0);
	FunctionCall *fcShnp = createFunctionCall("SUM", singleton(sumHasNonProv));
	fcShnp->isAgg = TRUE;

	AttributeReference *sumHasProv = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
	FunctionCall *fc = createFunctionCall("SUM", singleton(sumHasProv));
	fc->isAgg = TRUE;

//	Constant *countProv = createConstInt(1);
//	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
//	fcCount->isAgg = TRUE;

	aggrs = appendToTailOfList(aggrs,fcShnp);
	aggrs = appendToTailOfList(aggrs,fc);

	// create new attribute names for aggregation output schema
	List *attrNames = concatTwoLists(singleton(NUM_NONPROV_ATTR), singleton(NUM_PROV_ATTR));

//	for (int i = 0; i < LIST_LENGTH(aggrs); i++)
//		attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS(NUM_PROV_ATTR, itoa(i)));

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
		FOREACH(AttributeDef,al,samples->schema->attrDefs)
		{
			AttributeReference *lA, *rA = NULL;

			if(isPrefix(ar->attrName,al->attrName))
			{
				int alPos = LIST_LENGTH(normAttrs) + aPos;

				lA = createFullAttrReference(strdup(al->attrName), 0, alPos, 0, al->dataType);
				rA = createFullAttrReference(strdup(ar->attrName), 1, aPos, 0, ar->dataType);

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
rewritePatternOutput (char *summaryType, Node *input)
{
	Node *result;

	// compute Lowest Common Ancestors (LCA)
	if (strcmp(summaryType,"LCA") == 0)
	{
		QueryOperator *allSample = (QueryOperator *) input;

		// return only sample tuples having provenance
		QueryOperator *provSample = (QueryOperator *) input;

		int aPos = LIST_LENGTH(provSample->schema->attrDefs) - 1;
		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

		Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
		SelectionOperator *so = createSelectionOp(whereClause, provSample, NIL, getAttrNames(provSample->schema));

		provSample->parents = singleton(so);
		provSample = (QueryOperator *) so;
		provSample->provAttrs = copyObject(provAttrs);

		// create projection operator
		List *projExpr = NIL;
		int pos = LIST_LENGTH(allSample->schema->attrDefs);

		FOREACH(AttributeDef,a,provSample->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
			pos++;
		}

		ProjectionOperator *op = createProjectionOp(projExpr, provSample, NIL, getAttrNames(provSample->schema));
		provSample->parents = singleton(op);
		provSample->provAttrs = copyObject(provAttrs);

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
		projExpr = NIL;
		List *lProjExpr = NIL;
		List *rProjExpr = NIL;
		pos = 0;
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

//		pos++;
//		FOREACH(AttributeDef,a,provSample->schema->attrDefs)
//		{
////			if(searchListNode(normAttrs,(Node *) a))
//			if(strcmp(a->attrName,strdup(HAS_PROV_ATTR)) != 0)
//			{
//				rProjExpr = appendToTailOfList(rProjExpr,
//						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//				pos++;
//			}
//		}

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

		op = createProjectionOp(projExpr, patternJoin, NIL, provAttrNames);
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
rewriteSampleOutput (Node *randProv, Node *randNonProv, int sampleSize)
{
	Node *result;
	List *attrNames = NIL;

	QueryOperator *randomProvL = (QueryOperator *) randProv;
	QueryOperator *randomProvR = (QueryOperator *) randProv;

	attrNames = getAttrNames(randomProvL->schema);

	// create projection for adding "ROWNUM" for the left input
	int pos = 0;
	List *projExpr = NIL;

	FOREACH(AttributeDef,p,randomProvL->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}
	projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));
	attrNames = appendToTailOfList(attrNames, strdup(SAMP_NUM_L_ATTR));

	ProjectionOperator *leftOp = createProjectionOp(projExpr, randomProvL, NIL, attrNames);
	randomProvL->parents = singleton(leftOp);
	randomProvL = (QueryOperator *) leftOp;
	randomProvL->provAttrs = copyObject(provAttrs);

	// create projection for computing sample size for the right input
//	AttributeReference *countProv = createFullAttrReference(strdup("1"), 0, 0, 0, DT_INT);
	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(createConstInt(1)));
	fcCount->isAgg = TRUE;

	float spSize = 0.0;

	if (sampleSize != 0)
		spSize = (float) sampleSize / 100;
	else
		spSize = 0.1; // TODO: Whole or still do sampling?

	Node* sampSize = (Node *) createOpExpr("*",LIST_MAKE(fcCount,createConstFloat(spSize)));
	FunctionCall *fcCeil = createFunctionCall("CEIL", singleton(sampSize));

	ProjectionOperator *rightOp = createProjectionOp(singleton(fcCeil), randomProvR, NIL, singleton(strdup(SAMP_NUM_R_ATTR)));
	randomProvR->parents = appendToTailOfList(randomProvR->parents, rightOp);
	randomProvR = (QueryOperator *) rightOp;

	// create JOIN operator
	QueryOperator *left = (QueryOperator *) leftOp;
	QueryOperator *right = (QueryOperator *) rightOp;
	Node *joinCond = NULL;

	FOREACH(AttributeDef,l,left->schema->attrDefs)
	{
		FOREACH(AttributeDef,r,right->schema->attrDefs)
		{
			if (streq(l->attrName,SAMP_NUM_L_ATTR) && streq(r->attrName,SAMP_NUM_R_ATTR))
			{
				AttributeReference *lA, *rA = NULL;
				lA = createFullAttrReference(strdup(l->attrName), 0, LIST_LENGTH(left->schema->attrDefs)-1, 0, l->dataType);
				rA = createFullAttrReference(strdup(r->attrName), 1, 0, 0, r->dataType);
				joinCond = (Node *) createOpExpr("<=",LIST_MAKE(lA,rA));
			}
		}
	}

	List *crossInput = LIST_MAKE(left, right);
	attrNames = concatTwoLists(getAttrNames(left->schema),getAttrNames(right->schema));
	QueryOperator *randProvJoin = (QueryOperator *) createJoinOp(JOIN_INNER, joinCond, crossInput, NIL, attrNames);

	// set the parent of the operator's children
	left->parents = appendToTailOfList(left->parents,randProvJoin);
	right->parents = appendToTailOfList(right->parents,randProvJoin);

	// create projection to remove sampNum attrs
	pos = 0;
	projExpr = NIL;

	FOREACH(AttributeDef,p,randProvJoin->schema->attrDefs)
	{
		if (!isPrefix(p->attrName,SAMP_NUM_PREFIX))
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		}
		pos++;
	}

	attrNames = getAttrNames(((QueryOperator *) randProv)->schema);
	ProjectionOperator *op = createProjectionOp(projExpr, randProvJoin, NIL, attrNames);
	randProvJoin->parents = singleton(op);
	randProvJoin = (QueryOperator *) op;
	randProvJoin->provAttrs = copyObject(provAttrs);


	// sampling from random ordered non-provenance tuples
	QueryOperator *randomNonProvL = (QueryOperator *) randNonProv;
	QueryOperator *randomNonProvR = (QueryOperator *) randNonProv;

	attrNames = getAttrNames(randomNonProvL->schema);

	// create projection for adding "ROWNUM" for the left input
	pos = 0;
	projExpr = NIL;

	FOREACH(AttributeDef,p,randomNonProvL->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}
	projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));
	attrNames = appendToTailOfList(attrNames, strdup(SAMP_NUM_L_ATTR));

	leftOp = createProjectionOp(projExpr, randomNonProvL, NIL, attrNames);
	randomNonProvL->parents = singleton(leftOp);
	randomNonProvL = (QueryOperator *) leftOp;
	randomNonProvL->provAttrs = copyObject(provAttrs);

	// create projection for computing sample size for the right input
	rightOp = createProjectionOp(singleton(fcCeil), randomNonProvR, NIL, singleton(strdup(SAMP_NUM_R_ATTR)));
	randomNonProvR->parents = appendToTailOfList(randomNonProvR->parents, rightOp);
	randomNonProvR = (QueryOperator *) rightOp;

	// create JOIN operator
	left = (QueryOperator *) leftOp;
	right = (QueryOperator *) rightOp;
	joinCond = NULL;

	FOREACH(AttributeDef,l,left->schema->attrDefs)
	{
		FOREACH(AttributeDef,r,right->schema->attrDefs)
		{
		    if (streq(l->attrName,SAMP_NUM_L_ATTR) && streq(r->attrName,SAMP_NUM_R_ATTR))
			{
				AttributeReference *lA, *rA = NULL;
				lA = createFullAttrReference(strdup(l->attrName), 0, LIST_LENGTH(left->schema->attrDefs)-1, 0, l->dataType);
				rA = createFullAttrReference(strdup(r->attrName), 1, 0, 0, r->dataType);
				joinCond = (Node *) createOpExpr("<=",LIST_MAKE(lA,rA));
			}
		}
	}

	crossInput = LIST_MAKE(left, right);
	attrNames = concatTwoLists(getAttrNames(left->schema),getAttrNames(right->schema));
	QueryOperator *randNonProvJoin = (QueryOperator *) createJoinOp(JOIN_INNER, joinCond, crossInput, NIL, attrNames);

	// set the parent of the operator's children
	left->parents = appendToTailOfList(left->parents,randNonProvJoin);
	right->parents = appendToTailOfList(right->parents,randNonProvJoin);

	// create projection to remove sampNum attrs
	pos = 0;
	projExpr = NIL;

	FOREACH(AttributeDef,p,randNonProvJoin->schema->attrDefs)
	{
		if (!isPrefix(p->attrName,SAMP_NUM_PREFIX))
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		}
		pos++;
	}

	attrNames = getAttrNames(((QueryOperator *) randNonProv)->schema);
	op = createProjectionOp(projExpr, randNonProvJoin, NIL, attrNames);
	randNonProvJoin->parents = singleton(op);
	randNonProvJoin = (QueryOperator *) op;
	randNonProvJoin->provAttrs = copyObject(provAttrs);

	// create UNION operator to get all sample
	List *allInput = LIST_MAKE(randProvJoin,randNonProvJoin);
	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION, allInput, NIL, attrNames);
	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);
	unionOp->provAttrs = copyObject(provAttrs);


//
//	// random sampling from hasProv = 1
//	QueryOperator *provJoin = (QueryOperator *) input;
//	attrNames = getAttrNames(provJoin->schema);
//
//	// create selection for the prov instance
//	int aPos = LIST_LENGTH(provJoin->schema->attrDefs) - 1;
//	AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);
//
//	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
//	SelectionOperator *so = createSelectionOp(whereClause, provJoin, NIL, attrNames);
//
//	provJoin->parents = singleton(so);
//	provJoin = (QueryOperator *) so;
//	provJoin->provAttrs = copyObject(provAttrs);
//
//	// create projection for adding HAS_PROV_ATTR attribute
//	int pos = 0;
//	List *projExpr = NIL;
//	ProjectionOperator *op;
//
//	FOREACH(AttributeDef,p,provJoin->schema->attrDefs)
//	{
//		projExpr = appendToTailOfList(projExpr,
//				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
//		pos++;
//	}
//
//	op = createProjectionOp(projExpr, provJoin, NIL, attrNames);
//	provJoin->parents = singleton(op);
//	provJoin = (QueryOperator *) op;
//	provJoin->provAttrs = copyObject(provAttrs);
//
//	// create order by operator
//	Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
//	OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);
//
//	OrderOperator *ord = createOrderOp(singleton(ordExpr), provJoin, NIL);
//	provJoin->parents = singleton(ord);
//	provJoin = (QueryOperator *) ord;
//	provJoin->provAttrs = copyObject(provAttrs);
//
//	// create selection for returning only 2 tuples from random order
//	Node *selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(2)));
//	so = createSelectionOp(selCond, provJoin, NIL, attrNames);
//
//	provJoin->parents = singleton(so);
//	provJoin = (QueryOperator *) so;
//	provJoin->provAttrs = copyObject(provAttrs);
//
//	op = createProjectionOp(projExpr, provJoin, NIL, attrNames);
//	provJoin->parents = singleton(op);
//	provJoin = (QueryOperator *) op;
//	provJoin->provAttrs = copyObject(provAttrs);
//
//	// random sampling from hasProv <> 1
//	QueryOperator *nonProvJoin = (QueryOperator *) input;
//
//	whereClause = (Node *) createOpExpr("<>",LIST_MAKE(lC,createConstInt(1)));
//	so = createSelectionOp(whereClause, nonProvJoin, NIL, attrNames);
//
//	nonProvJoin->parents = appendToTailOfList(nonProvJoin->parents, so);
//	nonProvJoin = (QueryOperator *) so;
//	nonProvJoin->provAttrs = copyObject(provAttrs);
//
//	// create projection for adding HAS_PROV_ATTR attribute
//	op = createProjectionOp(projExpr, nonProvJoin, NIL, attrNames);
//	nonProvJoin->parents = singleton(op);
//	nonProvJoin = (QueryOperator *) op;
//	nonProvJoin->provAttrs = copyObject(provAttrs);
//
//	// create order by operator
//	ord = createOrderOp(singleton(ordExpr), nonProvJoin, NIL);
//	nonProvJoin->parents = singleton(ord);
//	nonProvJoin = (QueryOperator *) ord;
//	nonProvJoin->provAttrs = copyObject(provAttrs);
//
//	// create selection for returning only 3 tuples from random order
//	selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(3)));
//	so = createSelectionOp(selCond, nonProvJoin, NIL, attrNames);
//
//	nonProvJoin->parents = singleton(so);
//	nonProvJoin = (QueryOperator *) so;
//	nonProvJoin->provAttrs = copyObject(provAttrs);
//
//	op = createProjectionOp(projExpr, nonProvJoin, NIL, attrNames);
//	nonProvJoin->parents = singleton(op);
//	nonProvJoin = (QueryOperator *) op;
//	nonProvJoin->provAttrs = copyObject(provAttrs);
//
//	// create UNION operator to get all sample
//	List *allInput = LIST_MAKE(provJoin,nonProvJoin);
//	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION, allInput, NIL, attrNames);
//	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);
//	unionOp->provAttrs = copyObject(provAttrs);


	result = (Node *) unionOp;
//	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sampling for summarization:", result);
	INFO_OP_LOG("sampling for summarization as overview:", result);

	return result;
}


/*
 * randomly sample prov tuples
 */
static Node *
rewriteRandomProvTuples (Node *input)
{
	Node *result;
	List *attrNames = NIL;

	// random sampling from hasProv = 1
	QueryOperator *randomProv = (QueryOperator *) input;
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

	result = (Node *) randomProv;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("random order for provenance tuples for summarization:", result);
	INFO_OP_LOG("random order for provenance tuples for summarization as overview:", result);

	return result;
}


/*
 * randomly sample non-prov tuples
 */
static Node *
rewriteRandomNonProvTuples (Node *input)
{
	Node *result;
	List *attrNames = NIL;

	// random sampling from hasProv = 1
	QueryOperator *randomNonProv = (QueryOperator *) input;
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

	result = (Node *) randomNonProv;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("random order for non-provenance tuples for summarization:", result);
	INFO_OP_LOG("random order for non-provenance tuples for summarization as overview:", result);

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
rewriteProvJoinOutput (List *userQuestion, Node *rewrittenTree)
{
	Node *result;
	List *inputs = NIL;
	List *attrNames = NIL;
	QueryOperator *prov;

//	if(userQuestion == NIL)
	if (isA(rewrittenTree, List))
		prov = (QueryOperator *) getHeadOfListP((List *) rewrittenTree);
	else
		prov = (QueryOperator *) rewrittenTree;

	// take the input query out for use with join operator later
	QueryOperator *transInput = (QueryOperator *) prov->properties;

	// store normal and provenance attributes for later use
	if (provAttrs == NIL || normAttrs == NIL)
	{
		provAttrs = getProvenanceAttrs(prov);
		normAttrs = getNormalAttrs(prov);
	}

	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
//	QueryOperator *origProv = prov;

	// create projection for adding HAS_PROV_ATTR attribute
	FOREACH(AttributeDef,p,prov->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}
	projExpr = appendToTailOfList(projExpr,createConstInt(1));

	// add an attribute for prov
	int attrPos = LIST_LENGTH(transInput->schema->attrDefs) + LIST_LENGTH(prov->schema->attrDefs);
	AttributeDef *hasProv = (AttributeDef *) createFullAttrReference(strdup(HAS_PROV_ATTR), 0, attrPos, 0, DT_INT);

	List *newAttrs = concatTwoLists(getAttrNames(prov->schema),singleton(hasProv->attrName));
	op = createProjectionOp(projExpr, prov, NIL, newAttrs);

	prov->parents = singleton(op);
	prov = (QueryOperator *) op;
	prov->provAttrs = copyObject(provAttrs);

	// create join condition
	// TODO: find the corresponding provenance attribute to join if the name of attrs repeats
//	boolean suffix = FALSE;
	Node *curCond = NULL;
	int aPos = 0;
	int chkPos = 0;
	int orgAttr = LIST_LENGTH(transInput->schema->attrDefs);

	FOREACH(AttributeDef,attrs,transInput->schema->attrDefs)
	{
		char *a = attrs->attrName;

		Node *attrCond = NULL;
		AttributeReference *lA, *rA = NULL;

		lA = createFullAttrReference(strdup(a), 0, aPos, 0, attrs->dataType);

		// check suffix upfront to recognize if attributes are renamed
		for(int provAttr = orgAttr; provAttr < LIST_LENGTH(prov->schema->attrDefs); provAttr++)
		{
			AttributeDef *r = getAttrDefByPos(prov,provAttr);

			if(isSuffix(r->attrName,a))
			{
				rA = createFullAttrReference(strdup(r->attrName), 1, provAttr, 0, r->dataType);
				attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
				chkPos++;

				if(chkPos == 1)
					curCond = attrCond;
				else if (chkPos > 1)
					curCond = AND_EXPRS(curCond,attrCond);
			}
			else if(strcmp(a,r->attrName) == 0)
				FATAL_LOG("USING join is using ambiguous attribute references <%s>", a);
		}
		aPos++;
	}

	// no matches exist on name, then match by position
	if(curCond == NULL || chkPos > orgAttr) // then there exist repeating attrs
	{
		List *orgRef = ((ProjectionOperator *) transInput)->projExprs;
		chkPos = 0;
		attrPos = 0;
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

	FOREACH(AttributeDef,a,transInput->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	FOREACH(AttributeDef,a,prov->schema->attrDefs)
	{
		if(isPrefix(a->attrName,"PROV_"))
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		}
		pos++;
	}

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

	op = createProjectionOp(projExpr, provJoin, NIL, getAttrNames(prov->schema));
	provJoin->parents = singleton(op);
	provJoin = (QueryOperator *) op;
	provJoin->provAttrs = copyObject(provAttrs);

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
rewriteUserQuestion (List *userQuestion, Node *rewrittenTree)
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
	Node *curCond = NULL;
	SelectionOperator *so;

	FOREACH(Constant,c,userQuestion)
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
			quest = (AttributeReference *) getNthOfListP(inputAttrRefs,attrPos);
			userQattrs = appendToTailOfList(userQattrs,quest);
		}

		attrPos++;
	}
	so = createSelectionOp(curCond, input, NIL, NIL);

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
