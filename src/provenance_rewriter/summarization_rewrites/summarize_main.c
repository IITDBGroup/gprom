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
#include "provenance_rewriter/prov_rewriter.h"
#include "sql_serializer/sql_serializer.h"
#include "operator_optimizer/operator_optimizer.h"
#include "provenance_rewriter/prov_utility.h"
#include "utility/string_utils.h"
#include "model/query_operator/operator_property.h"

#include "provenance_rewriter/summarization_rewrites/summarize_main.h"

static List *provAttrs = NIL;
static List *origAttrs = NIL;

static Node *rewriteSampleOutput (Node * input);
static Node *rewritePatternOutput (char *summaryType, Node * input);
static Node *rewriteScanSampleOutput (Node *sampleInput, Node * patternInput);
static Node *rewriteCandidateOutput (Node *scanSampleInput);
static Node *rewriteComputeFracOutput (Node *candidateInput, Node * sampleInput);
static Node *rewriteMostGenExplOutput (Node *computeFracInput);

Node *
rewriteSummaryOutput (char *summaryType, Node *rewrittenTree)
{
	char *sType = summaryType;

	Node *result;
	Node *provJoin;
	Node *samples;
	Node *patterns;
	Node *scanSamples;
	Node *candidates;
	Node *computeFrac;

	provJoin = rewriteProvJoinOutput(rewrittenTree);
	samples = rewriteSampleOutput(provJoin);
	patterns = rewritePatternOutput(sType, samples);
	scanSamples = rewriteScanSampleOutput(samples, patterns);
	candidates = rewriteCandidateOutput(scanSamples);
	computeFrac = rewriteComputeFracOutput(candidates, samples);
	result = rewriteMostGenExplOutput(computeFrac);

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

	samples->parents = singleton(so);
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
	OP_LCHILD(computeFrac)->parents = OP_RCHILD(computeFrac)->parents = singleton(computeFrac);

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	AttributeReference *totProv;
	AttributeReference *covProv;
	AttributeReference *numProv;

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

	// add attribute for accuracy
//	AttributeReference *numProv = createFullAttrReference(strdup("numInProv"), 0, 2, 0, DT_INT);
//	AttributeReference *covProv = createFullAttrReference(strdup("Covered"), 0, 1, 0, DT_INT);
	Node* accuRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,covProv));
	projExpr = appendToTailOfList(projExpr, accuRate);

	// add attribute for coverage
//	AttributeReference *totProv = createFullAttrReference(strdup("totalProv"), 0, 0, 0, DT_INT);
	Node* covRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,totProv));
	projExpr = appendToTailOfList(projExpr, covRate);

	attrNames = CONCAT_LISTS(attrNames, singleton("Accuracy"), singleton("Coverage"));
	op = createProjectionOp(projExpr, computeFrac, NIL, attrNames);
	computeFrac->parents = singleton(op);
	computeFrac = (QueryOperator *) op;

	// create ORDER BY
	// TODO: probably put another projection for order by operation
//	AttributeReference *accuR = createFullAttrReference(strdup("Accuracy"), 0,
//							LIST_LENGTH(computeFrac->schema->attrDefs) - 2, 0, DT_INT);

	OrderExpr *accExpr = createOrderExpr((Node *) accuRate, SORT_DESC, SORT_NULLS_LAST);
	OrderExpr *covExpr = createOrderExpr((Node *) covRate, SORT_DESC, SORT_NULLS_LAST);

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
	List *origExprs = NIL;
	List *caseExprs = NIL;
	int pos = 0;

	FOREACH(AttributeDef,a,scanSamples->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

		pos++;
	}

	pos = 2;
	FOREACH(AttributeDef,a,origAttrs)
	{
		origExprs = appendToTailOfList(origExprs,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

		pos++;
	}

	attrs = NIL;

	FOREACH(Node,n,origExprs)
	{
		AttributeDef *a = (AttributeDef *) n;

		Node *cond = (Node *) createIsNullExpr((Node *) a);
		Node *then = (Node *) createConstInt(1);
		Node *els = (Node *) createConstInt(0);

		CaseWhen *caseWhen = createCaseWhen(cond, then);
		CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

		caseExprs = appendToTailOfList(caseExprs, (List *) caseExpr);
		attrs = appendToTailOfList(attrs, CONCAT_STRINGS("use",a->attrName));
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
	Node *curCond = NULL;
	int aPos = 0;
//	int sLen = LIST_LENGTH(samples->schema->attrDefs) - 1;

	FOREACH(AttributeDef,attrs,samples->schema->attrDefs)
	{
		if (searchListNode(origAttrs, (Node *) attrs))
		{
			char *a = attrs->attrName;
			AttributeReference *lA, *rA;

			lA = createFullAttrReference(strdup(a), 0, aPos, 0, attrs->dataType);

			char *a2 = STRING_VALUE(getNthOfListP(patterns->schema->attrDefs,aPos));
//			int rPos = aPos + sLen + 1;

			if(strcmp(a,a2) == 0)
				rA = createFullAttrReference(strdup(a2), 1, aPos, 0, attrs->dataType);

			// create equality condition and update global condition
			Node *joinCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
			Node *isNullCond = (Node *) createIsNullExpr((Node *) rA);
			Node *attrCond = OR_EXPRS(joinCond,isNullCond);

			curCond = AND_EXPRS(attrCond,curCond);
			aPos++;
		}
	}

	// create join operator
	List *inputs = LIST_MAKE(samples,patterns);
	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(patterns->schema));
	QueryOperator *scanSample = (QueryOperator *) createJoinOp(JOIN_INNER, curCond, inputs, NIL, attrNames);

	// set the parent of the operator's children
	OP_LCHILD(scanSample)->parents = OP_RCHILD(scanSample)->parents = singleton(scanSample);
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
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));

		pos++;
	}

	AttributeReference *hp = (AttributeReference *) getHeadOfListP(hasExpr);
	projExpr = appendToTailOfList(projExpr, hp);

	attrNames = concatTwoLists(getAttrNames(patterns->schema),singleton(hp->name));
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
		OP_LCHILD(patternJoin)->parents = OP_RCHILD(patternJoin)->parents = singleton(patternJoin);
//		patternJoin->provAttrs = provAttrs;

		// create projection operator
		projExpr = NIL;
		List *lProjExpr = NIL;
		List *rProjExpr = NIL;
		pos = 0;

		FOREACH(AttributeDef,a,allSample->schema->attrDefs)
		{
			if(searchListNode(origAttrs,(Node *) a))
			{
				lProjExpr = appendToTailOfList(lProjExpr,
						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
				pos++;
			}
		}

		FOREACH(AttributeDef,a,provSample->schema->attrDefs)
		{
			if(searchListNode(origAttrs,(Node *) a))
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

	nonProvJoin->parents = singleton(so);
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

Node *
rewriteProvJoinOutput (Node *rewrittenTree)
{
	List *inputs = NIL;
	List *attrNames = NIL;

//	QueryOperator *transInput = (QueryOperator *) getHeadOfListP(parents);

	QueryOperator *prov = (QueryOperator *) getHeadOfListP((List *) rewrittenTree);
	QueryOperator *transInput = (QueryOperator *) prov->properties;
	provAttrs = getProvenanceAttrs(prov);
	origAttrs = getNormalAttrs(prov);

	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
	QueryOperator *origProv = prov;

	// create selection for user prov question
	// TODO: temporary where clause (apply from parse)
//	AttributeReference *lC = createFullAttrReference(strdup("B"), 0, 1, 0, DT_INT);
//	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));

//	AttributeReference *lC = createFullAttrReference(strdup("I"), 0, 0, 0, DT_INT);
//	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstString("a")));
//
//	SelectionOperator *so = createSelectionOp(whereClause, prov, NIL, getAttrNames(prov->schema));

//	attrNames = getAttrDefNames(getNormalAttrs(prov));
//	SelectionOperator *so = createSelectionOp(whereClause, prov, NIL, attrNames);
//	addProvenanceAttrsToSchema((QueryOperator *) so, OP_LCHILD(so));

//	prov->parents = singleton(so);
//	prov = (QueryOperator *) so;
//	prov->provAttrs = provAttrs;

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
	// TODO: find the corresponding provenance attribute to join
	boolean suffix = FALSE;
	Node *curCond = NULL;
	int aPos = 0;


	FOREACH(AttributeDef,attrs,transInput->schema->attrDefs)
	{
		char *a = attrs->attrName;

		Node *attrCond;
		AttributeReference *lA, *rA;

		lA = createFullAttrReference(strdup(a), 0, aPos, 0, attrs->dataType);

		// check suffix upfront to recognize if attributes are renamed
		// TODO: how to recognize which renamed normal attributes correspond to provenance attributes
		for(int rPos = LIST_LENGTH(transInput->schema->attrDefs); rPos < LIST_LENGTH(prov->schema->attrDefs); rPos++)
		{
			char *attrProv = STRING_VALUE(getNthOfListP(prov->schema->attrDefs,rPos));

			if (isSuffix(attrProv,a))
				suffix = TRUE;
		}

		if (LIST_LENGTH(provAttrs) > LIST_LENGTH(origAttrs) && suffix)
		{
			char *a2 = NULL;

			for(int rPos = LIST_LENGTH(transInput->schema->attrDefs); rPos < LIST_LENGTH(prov->schema->attrDefs); rPos++)
			{
				char *attrProv = STRING_VALUE(getNthOfListP(prov->schema->attrDefs,rPos));

				if (isSuffix(attrProv,a))
				{
					a2 = attrProv;

					if(strcmp(a,a2) == 0)
						FATAL_LOG("USING join is using ambiguous attribute references <%s>", a);
					else
						rA = createFullAttrReference(strdup(a2), 1, rPos, 0, attrs->dataType);
				}
			}
		}
		else
		{
			int rPos = aPos + LIST_LENGTH(transInput->schema->attrDefs);
			char *a2 = STRING_VALUE(getNthOfListP(prov->schema->attrDefs,rPos));

			if(strcmp(a,a2) == 0)
				FATAL_LOG("USING join is using ambiguous attribute references <%s>", a);
			else
				rA = createFullAttrReference(strdup(a2), 1, rPos, 0, attrs->dataType);
		}

		aPos++;

		// create equality condition and update global condition
		attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
		curCond = AND_EXPRS(attrCond,curCond);
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

//	FOREACH(AttributeDef,a,transInput->schema->attrDefs)
//	{
//		projExpr = appendToTailOfList(projExpr,
//				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//		pos++;
//	}

	FOREACH(AttributeDef,a,origProv->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
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

//	// create projection for join
//	projExpr = NIL;
//	pos = 0;
//
//	FOREACH(AttributeDef,a,origProv->schema->attrDefs)
//	{
//		projExpr = appendToTailOfList(projExpr,
//				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//		pos++;
//	}
//	projExpr = appendToTailOfList(projExpr, hasProv);
//
//	uniqueAttrNames = CONCAT_LISTS(getAttrNames(origProv->schema),singleton(hasProv->attrName));
//	op = createProjectionOp(projExpr, provJoin, NIL, uniqueAttrNames);
//	provJoin->parents = singleton(op);
//	provJoin = (QueryOperator *) op;
//	provJoin->provAttrs = provAttrs;

	rewrittenTree = (Node *) provJoin;
	SET_BOOL_STRING_PROP(rewrittenTree, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("rewritten query for summarization returned:", rewrittenTree);
	INFO_OP_LOG("rewritten query for summarization as overview:", rewrittenTree);

	return rewrittenTree;
}
