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

static List *provAttrs = NIL;
static List *normAttrs = NIL;

static Node *rewriteUserQuestion (List *userQuestion, Node *rewrittenTree);
static Node *rewriteProvJoinOutput (List *userQuestion, Node *rewrittenTree);
static Node *rewriteSampleOutput (Node * input);
static Node *rewritePatternOutput (char *summaryType, Node * input);
static Node *rewriteScanSampleOutput (Node *sampleInput, Node * patternInput);
static Node *rewriteCandidateOutput (Node *scanSampleInput);
static Node *rewriteComputeFracOutput (Node *candidateInput, Node * sampleInput);
static Node *rewriteMostGenExplOutput (Node *computeFracInput);

Node *
rewriteSummaryOutput (char *summaryType, Node *rewrittenTree, List *userQuestion)
{
	List *uQuestion = userQuestion;
	char *sType = summaryType;

	Node *result;
	Node *provJoin;
	Node *samples;
	Node *patterns;
	Node *scanSamples;
	Node *candidates;
	Node *computeFrac;

	if (uQuestion != NIL)
		rewrittenTree = rewriteUserQuestion(uQuestion, rewrittenTree);

	provJoin = rewriteProvJoinOutput(uQuestion, rewrittenTree);
	samples = rewriteSampleOutput(provJoin);
	patterns = rewritePatternOutput(sType, samples); //TODO: different types of pattern generation
	scanSamples = rewriteScanSampleOutput(samples, patterns);
	candidates = rewriteCandidateOutput(scanSamples);
	computeFrac = rewriteComputeFracOutput(candidates, samples);
	result = rewriteMostGenExplOutput(computeFrac);

	 if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) result));

	return result;
}

static Node *
rewriteMostGenExplOutput (Node *computeFracInput)
{
	Node *result;

	QueryOperator *computeFrac = (QueryOperator *) computeFracInput;

	// create selection for returning top most general explanation
	Node *selCond = (Node *) createOpExpr("=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(1)));
	SelectionOperator *so = createSelectionOp(selCond, computeFrac, NIL, getAttrNames(computeFrac->schema));

	computeFrac->parents = singleton(so);
	computeFrac = (QueryOperator *) so;

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,computeFrac->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}

	op = createProjectionOp(projExpr, computeFrac, NIL, getAttrNames(computeFrac->schema));
	computeFrac->parents = singleton(op);
	computeFrac = (QueryOperator *) op;

	result = (Node *) computeFrac;

	DEBUG_NODE_BEATIFY_LOG("most general explanation from summarization:", result);
	INFO_OP_LOG("most general explanation from summarization as overview:", result);

	return result;
}

static Node *
rewriteComputeFracOutput (Node *candidateInput, Node *sampleInput)
{
	Node *result;

	QueryOperator *candidates = (QueryOperator *) candidateInput;
	QueryOperator *samples = (QueryOperator *) sampleInput;

	// get total count for prov from samples
	int aPos = LIST_LENGTH(samples->schema->attrDefs) - 1;
	AttributeReference *lC = createFullAttrReference(strdup("HAS_PROV"), 0, aPos, 0, DT_INT);

	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
	SelectionOperator *so = createSelectionOp(whereClause, samples, NIL, getAttrNames(samples->schema));

	samples->parents = appendToTailOfList(samples->parents,so);
	samples = (QueryOperator *) so;

	// create projection operator
	AttributeReference *countProv = createFullAttrReference(strdup("*"), 0, aPos, 0, DT_INT);
	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
	fcCount->isAgg = TRUE;
	countProv->name = strdup("totalProv");

	ProjectionOperator *op = createProjectionOp(singleton(fcCount), samples, NIL, singleton(countProv->name));
	samples->parents = singleton(op);
	samples = (QueryOperator *) op;

	// cross product with candidates to compute
	List *crossInput = LIST_MAKE(samples,candidates);
	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(candidates->schema));
	QueryOperator *computeFrac = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, crossInput, NIL, attrNames);

	// set the parent of the operator's children
//	OP_LCHILD(computeFrac)->parents = OP_RCHILD(computeFrac)->parents = singleton(computeFrac);
	samples->parents = appendToTailOfList(samples->parents,computeFrac);
	candidates->parents = singleton(computeFrac);

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	AttributeReference *totProv, *covProv, *numProv = NULL;
//	AttributeReference *covProv;
//	AttributeReference *numProv;

	FOREACH(AttributeDef,p,computeFrac->schema->attrDefs)
	{
		if (pos == 0)
			totProv = createFullAttrReference(strdup("totalProv"), 0, pos, 0, p->dataType);

		if (pos == 1)
			covProv = createFullAttrReference(strdup("Covered"), 0, pos, 0, p->dataType);

		if (pos == 2)
			numProv = createFullAttrReference(strdup("numInProv"), 0, pos, 0, p->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}

	// round up after second decimal number
	Node *rdup = (Node *) createConstInt(atoi("2"));

	// add attribute for accuracy
//	AttributeReference *numProv = createFullAttrReference(strdup("numInProv"), 0, 2, 0, DT_INT);
//	AttributeReference *covProv = createFullAttrReference(strdup("Covered"), 0, 1, 0, DT_INT);
	Node *accuRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,covProv));
	FunctionCall *rdupAr = createFunctionCall("ROUND", LIST_MAKE(accuRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupAr);

	// add attribute for coverage
//	AttributeReference *totProv = createFullAttrReference(strdup("totalProv"), 0, 0, 0, DT_INT);
	Node* covRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,totProv));
	FunctionCall *rdupCr = createFunctionCall("ROUND", LIST_MAKE(covRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupCr);

	attrNames = CONCAT_LISTS(attrNames, singleton("Accuracy"), singleton("Coverage"));
	op = createProjectionOp(projExpr, computeFrac, NIL, attrNames);
	computeFrac->parents = singleton(op);
	computeFrac = (QueryOperator *) op;

	// create ORDER BY
	// TODO: probably put another projection for order by operation
//	AttributeReference *accuR = createFullAttrReference(strdup("Accuracy"), 0,
//							LIST_LENGTH(computeFrac->schema->attrDefs) - 2, 0, DT_INT);

	OrderExpr *accExpr = createOrderExpr(accuRate, SORT_DESC, SORT_NULLS_LAST);
	OrderExpr *covExpr = createOrderExpr(covRate, SORT_DESC, SORT_NULLS_LAST);

	OrderOperator *ord = createOrderOp(LIST_MAKE(accExpr, covExpr), computeFrac, NIL);
	computeFrac->parents = singleton(ord);
	computeFrac = (QueryOperator *) ord;

	result = (Node *) computeFrac;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("compute fraction for summarization:", result);
	INFO_OP_LOG("compute fraction for summarization as overview:", result);

	return result;
}

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
		if (strcmp(a->attrName,"HAS_PROV") != 0)
		{
			groupBy = appendToTailOfList(groupBy,
					createFullAttrReference(strdup(a->attrName), 0, gPos, 0, a->dataType));

			gPos++;
		}
	}

	List *aggrs = NIL;
	AttributeReference *sumHasProv = createFullAttrReference(strdup("HAS_PROV"), 0, gPos, 0, DT_INT);
	FunctionCall *fc = createFunctionCall("SUM", singleton(sumHasProv));
	fc->isAgg = TRUE;

	AttributeReference *countProv = createFullAttrReference(strdup("*"), 0, gPos, 0, DT_INT);
	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
	fcCount->isAgg = TRUE;

	aggrs = appendToTailOfList(aggrs,fcCount);
	aggrs = appendToTailOfList(aggrs,fc);

	// create new attribute names for aggregation output schema
	List *attrNames = concatTwoLists(singleton("Covered"), singleton("NumInProv"));

//	for (int i = 0; i < LIST_LENGTH(aggrs); i++)
//		attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("NumInProv", itoa(i)));

	List *attrs = getAttrDefNames(removeFromTail(scanSamples->schema->attrDefs));
	attrNames = concatTwoLists(attrNames, attrs);

	AggregationOperator *gb = (AggregationOperator *) createAggregationOp(aggrs, groupBy, scanSamples, NIL, attrNames);
	scanSamples->parents = singleton(gb);
	scanSamples = (QueryOperator *) gb;
//	scanSamples->provAttrs = provAttrs;

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

	FOREACH(AttributeDef,attrs,samples->schema->attrDefs)
	{
		if (strcmp(attrs->attrName,"HAS_PROV") != 0)
		{
			char *a = attrs->attrName;
			AttributeReference *lA, *rA = NULL;

			lA = createFullAttrReference(strdup(a), 0, aPos, 0, attrs->dataType);

			char *a2 = STRING_VALUE(getNthOfListP(patterns->schema->attrDefs,aPos));
	//			int rPos = aPos + sLen + 1;

			if(strcmp(a,a2) == 0)
			{
				rA = createFullAttrReference(strdup(a2), 1, aPos, 0, attrs->dataType);

				// create equality condition and update global condition
				joinCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
				isNullCond = (Node *) createIsNullExpr((Node *) rA);
				attrCond = OR_EXPRS(joinCond,isNullCond);
				curCond = AND_EXPRS(attrCond,curCond);
			}
			aPos++;
		}
	}

	// create join operator
	List *inputs = LIST_MAKE(samples,patterns);
	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(patterns->schema));
	QueryOperator *scanSample = (QueryOperator *) createJoinOp(JOIN_INNER, curCond, inputs, NIL, attrNames);

	// set the parent of the operator's children
//	OP_LCHILD(scanSample)->parents = OP_RCHILD(scanSample)->parents = singleton(scanSample);
	samples->parents = appendToTailOfList(samples->parents,scanSample);
	patterns->parents = singleton(scanSample);
//	scanSample->provAttrs = provAttrs;

	// create projection for adding "HAS_PROV" attribute
	int pos = 0;
	int hasPos = 0;
	List *projExpr = NIL;
	List *hasExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,scanSample->schema->attrDefs)
	{
		if (strcmp(p->attrName,"HAS_PROV") == 0)
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
//	scanSample->provAttrs = provAttrs;

	result = (Node *) scanSample;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("join patterns with samples for summarization:", result);
	INFO_OP_LOG("join patterns with samples for summarization as overview:", result);

	return result;
}

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
		AttributeReference *lC = createFullAttrReference(strdup("HAS_PROV"), 0, aPos, 0, DT_INT);

		Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
		SelectionOperator *so = createSelectionOp(whereClause, provSample, NIL, getAttrNames(provSample->schema));

		provSample->parents = singleton(so);
		provSample = (QueryOperator *) so;
		provSample->provAttrs = provAttrs;

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
		provSample->provAttrs = provAttrs;

		// create CROSS_JOIN operator
		List *crossInput = LIST_MAKE(allSample, provSample);
		List *attrNames = concatTwoLists(getAttrNames(allSample->schema),getAttrNames(provSample->schema));
		QueryOperator *patternJoin = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, crossInput, NIL, attrNames);

		// set the parent of the operator's children
//		OP_LCHILD(patternJoin)->parents = OP_RCHILD(patternJoin)->parents = singleton(patternJoin);
		allSample->parents = appendToTailOfList(allSample->parents,patternJoin);
		provSample->parents = singleton(patternJoin);
//		patternJoin->provAttrs = provAttrs;

		// create projection operator
		projExpr = NIL;
		List *lProjExpr = NIL;
		List *rProjExpr = NIL;
		pos = 0;

		FOREACH(AttributeDef,a,allSample->schema->attrDefs)
		{
//			if(searchListNode(normAttrs,(Node *) a))
			if(strcmp(a->attrName,strdup("HAS_PROV")) != 0)
			{
				lProjExpr = appendToTailOfList(lProjExpr,
						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
				pos++;
			}
		}

		pos++;
		FOREACH(AttributeDef,a,provSample->schema->attrDefs)
		{
//			if(searchListNode(normAttrs,(Node *) a))
			if(strcmp(a->attrName,strdup("HAS_PROV")) != 0)
			{
				rProjExpr = appendToTailOfList(rProjExpr,
						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
				pos++;
			}
		}

		FORBOTH(Node,l,r,lProjExpr,rProjExpr)
		{
			AttributeDef *a = (AttributeDef *) r;

//			if (strcmp(a->attrName,"HAS_PROV") != 0)
//			{
				DataType d = a->dataType;
				Node *cond = (Node *) createOpExpr("=",LIST_MAKE(l,r));

				Node *then = l;
				Node *els = (Node *) createNullConst(d);

				CaseWhen *caseWhen = createCaseWhen(cond, then);
				CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

				projExpr = appendToTailOfList(projExpr, (List *) caseExpr);
//			}
//			else
//				projExpr = appendToTailOfList(projExpr, r);
		}

		op = createProjectionOp(projExpr, patternJoin, NIL, getAttrNames(patternJoin->schema));
		patternJoin->parents = singleton(op);
		patternJoin = (QueryOperator *) op;
		patternJoin->provAttrs = provAttrs;

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
		patternJoin->provAttrs = provAttrs;

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

static Node *
rewriteSampleOutput (Node *input)
{
	Node *result;
	List *attrNames = NIL;

	// random sampling from hasProv = 1
	QueryOperator *provJoin = (QueryOperator *) input;
	attrNames = getAttrNames(provJoin->schema);

	// create selection for the prov instance
	int aPos = LIST_LENGTH(provJoin->schema->attrDefs) - 1;
	AttributeReference *lC = createFullAttrReference(strdup("HAS_PROV"), 0, aPos, 0, DT_INT);

	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
	SelectionOperator *so = createSelectionOp(whereClause, provJoin, NIL, attrNames);

	provJoin->parents = singleton(so);
	provJoin = (QueryOperator *) so;
	provJoin->provAttrs = provAttrs;

	// create projection for adding "HAS_PROV" attribute
	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,provJoin->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}

	op = createProjectionOp(projExpr, provJoin, NIL, attrNames);
	provJoin->parents = singleton(op);
	provJoin = (QueryOperator *) op;
	provJoin->provAttrs = provAttrs;

	// create order by operator
	Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
	OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);

	OrderOperator *ord = createOrderOp(singleton(ordExpr), provJoin, NIL);
	provJoin->parents = singleton(ord);
	provJoin = (QueryOperator *) ord;
	provJoin->provAttrs = provAttrs;

	// create selection for returning only 2 tuples from random order
	Node *selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(2)));
	so = createSelectionOp(selCond, provJoin, NIL, attrNames);

	provJoin->parents = singleton(so);
	provJoin = (QueryOperator *) so;
	provJoin->provAttrs = provAttrs;

	op = createProjectionOp(projExpr, provJoin, NIL, attrNames);
	provJoin->parents = singleton(op);
	provJoin = (QueryOperator *) op;
	provJoin->provAttrs = provAttrs;

	// random sampling from hasProv <> 1
	QueryOperator *nonProvJoin = (QueryOperator *) input;

	whereClause = (Node *) createOpExpr("<>",LIST_MAKE(lC,createConstInt(1)));
	so = createSelectionOp(whereClause, nonProvJoin, NIL, attrNames);

	nonProvJoin->parents = appendToTailOfList(nonProvJoin->parents, so);
	nonProvJoin = (QueryOperator *) so;
	nonProvJoin->provAttrs = provAttrs;

	// create projection for adding "HAS_PROV" attribute
	op = createProjectionOp(projExpr, nonProvJoin, NIL, attrNames);
	nonProvJoin->parents = singleton(op);
	nonProvJoin = (QueryOperator *) op;
	nonProvJoin->provAttrs = provAttrs;

	// create order by operator
	ord = createOrderOp(singleton(ordExpr), nonProvJoin, NIL);
	nonProvJoin->parents = singleton(ord);
	nonProvJoin = (QueryOperator *) ord;
	nonProvJoin->provAttrs = provAttrs;

	// create selection for returning only 3 tuples from random order
	selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(3)));
	so = createSelectionOp(selCond, nonProvJoin, NIL, attrNames);

	nonProvJoin->parents = singleton(so);
	nonProvJoin = (QueryOperator *) so;
	nonProvJoin->provAttrs = provAttrs;

	op = createProjectionOp(projExpr, nonProvJoin, NIL, attrNames);
	nonProvJoin->parents = singleton(op);
	nonProvJoin = (QueryOperator *) op;
	nonProvJoin->provAttrs = provAttrs;

	// create UNION operator to get all sample
	List *allInput = LIST_MAKE(provJoin,nonProvJoin);
	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION, allInput, NIL, attrNames);
	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);
	unionOp->provAttrs = provAttrs;

	result = (Node *) unionOp;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sampling for summarization:", result);
	INFO_OP_LOG("sampling for summarization as overview:", result);

	return result;
}

static Node *
rewriteProvJoinOutput (List *userQuestion, Node *rewrittenTree)
{
	Node *result;
	List *inputs = NIL;
	List *attrNames = NIL;
	QueryOperator *prov;

	if(userQuestion == NULL)
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

	// create projection for adding "HAS_PROV" attribute
	FOREACH(AttributeDef,p,prov->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		pos++;
	}
	projExpr = appendToTailOfList(projExpr,createConstInt(1));

	// add an attribute for prov
	int attrPos = LIST_LENGTH(transInput->schema->attrDefs) + LIST_LENGTH(prov->schema->attrDefs);
	AttributeDef *hasProv = (AttributeDef *) createFullAttrReference(strdup("HAS_PROV"), 0, attrPos, 0, DT_INT);

	List *newAttrs = concatTwoLists(getAttrNames(prov->schema),singleton(hasProv->attrName));
	op = createProjectionOp(projExpr, prov, NIL, newAttrs);

	prov->parents = singleton(op);
	prov = (QueryOperator *) op;
	prov->provAttrs = provAttrs;

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

	// set the parent of the operator's children
	OP_LCHILD(provJoin)->parents = OP_RCHILD(provJoin)->parents = singleton(provJoin);
//	provJoin->provAttrs = provAttrs;

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
	provJoin->provAttrs = provAttrs;

	result = (Node *) provJoin;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("rewritten query for summarization returned:", result);
	INFO_OP_LOG("rewritten query for summarization as overview:", rewrittenTree);

	return result;
}

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

	// check the list for constant value to create sel condition
	int chkPos = 0;
	int attrPos = 0;
	Node *curCond = NULL;
	SelectionOperator *so;

	FOREACH(Constant,c,userQuestion)
	{
		if (strcmp(strdup(c->value),"*") != 0)
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
		}

		attrPos++;
	}
	so = createSelectionOp(curCond, input, NIL, getAttrNames(input->schema));

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

	input->properties = prop;
	result = (Node *) input;
//	SET_BOOL_STRING_PROP(rewrittenTree, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("provenance question for summarization:", result);
	INFO_OP_LOG("provenance question for summarization as overview:", result);

	return result;
}
