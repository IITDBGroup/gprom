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

#include "provenance_rewriter/summarization_rewrites/summarize_main.h"

static Node *rewriteSampleOutput (Node * input);


Node *
rewriteSummaryOutput (char *summaryType, Node *rewrittenTree)
{
	char *sType = summaryType;

	Node *result;
	Node *provJoin;
	Node *samples;
	Node *patterns;
//	Node *scanSamples;

	provJoin = rewriteProvJoinOutput(rewrittenTree);
	samples = rewriteSampleOutput(provJoin);

	if (strcmp(sType,"LCA") == 0)
		patterns = samples;
//		patterns = rewritePatternOutput(samples);
//	scanSamples = rewriteScanSampleOutput(rewrittenTree);

	result = patterns;

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

	// create order by operator
	Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
	OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);

	OrderOperator *ord = createOrderOp(singleton(ordExpr), provJoin, NIL);
	provJoin->parents = singleton(ord);
	provJoin = (QueryOperator *) ord;

	// create selection for returning only 2 tuples from random order
	Node *selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(2)));
	so = createSelectionOp(selCond, provJoin, NIL, attrNames);

	provJoin->parents = singleton(so);
	provJoin = (QueryOperator *) so;

	op = createProjectionOp(projExpr, provJoin, NIL, attrNames);
	provJoin->parents = singleton(op);
	provJoin = (QueryOperator *) op;

	// random sampling from hasProv <> 1
	QueryOperator *nonProvJoin = (QueryOperator *) input;

	whereClause = (Node *) createOpExpr("<>",LIST_MAKE(lC,createConstInt(1)));
	so = createSelectionOp(whereClause, nonProvJoin, NIL, attrNames);

	nonProvJoin->parents = singleton(so);
	nonProvJoin = (QueryOperator *) so;

	// create projection for adding "HAS_PROV" attribute
	op = createProjectionOp(projExpr, nonProvJoin, NIL, attrNames);
	nonProvJoin->parents = singleton(op);
	nonProvJoin = (QueryOperator *) op;

	// create order by operator
	ord = createOrderOp(singleton(ordExpr), nonProvJoin, NIL);
	nonProvJoin->parents = singleton(ord);
	nonProvJoin = (QueryOperator *) ord;

	// create selection for returning only 3 tuples from random order
	selCond = (Node *) createOpExpr("<=",LIST_MAKE(singleton(makeNode(RowNumExpr)),createConstInt(3)));
	so = createSelectionOp(selCond, nonProvJoin, NIL, attrNames);

	nonProvJoin->parents = singleton(so);
	nonProvJoin = (QueryOperator *) so;

	op = createProjectionOp(projExpr, nonProvJoin, NIL, attrNames);
	nonProvJoin->parents = singleton(op);
	nonProvJoin = (QueryOperator *) op;

	// create UNION operator to get all sample
	List *allInput = LIST_MAKE(provJoin,nonProvJoin);
	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION, allInput, NIL, attrNames);
	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);

	result = (Node *) unionOp;

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

	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
	QueryOperator *origProv = prov;

	// create selection for user prov question
	// TODO: temporary where clause (apply from parse)
	AttributeReference *lC = createFullAttrReference(strdup("B"), 0, 1, 0, DT_INT);
	Node *whereClause = (Node *) createOpExpr("=",LIST_MAKE(lC,createConstInt(1)));
	SelectionOperator *so = createSelectionOp(whereClause, prov, NIL, getAttrNames(prov->schema));

	prov->parents = singleton(so);
	prov = (QueryOperator *) so;

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

	// create join condition
	Node *curCond = NULL;
	Node *joinCond;
	int aPos = 0;

	FOREACH(AttributeDef,attrs,transInput->schema->attrDefs)
	{
		char *a = attrs->attrName;

		Node *attrCond;
		AttributeReference *lA, *rA;

		lA = createFullAttrReference(strdup(a), 0, aPos, 0, attrs->dataType);

		int rPos = aPos + LIST_LENGTH(transInput->schema->attrDefs);
		char *a2 = STRING_VALUE(getNthOfListP(prov->schema->attrDefs,rPos));

		if(strcmp(a,a2) == 0)
			FATAL_LOG("USING join is using ambiguous attribute references <%s>", a);
		else
			rA = createFullAttrReference(strdup(a2), 1, rPos, 0, attrs->dataType);

		aPos++;

		// create equality condition and update global condition
		attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
		curCond = AND_EXPRS(attrCond,curCond);
	}
	joinCond = curCond;

	inputs = LIST_MAKE(transInput,prov);

	// create join operator
	attrNames = concatTwoLists(getAttrNames(transInput->schema), getAttrNames(prov->schema));
	QueryOperator *provJoin = (QueryOperator *) createJoinOp(JOIN_LEFT_OUTER, joinCond, inputs, NIL, attrNames);

	// set the parent of the operator's children
	OP_LCHILD(provJoin)->parents = OP_RCHILD(provJoin)->parents = singleton(provJoin);

	// create projection for join
	projExpr = NIL;
	pos = 0;

	FOREACH(AttributeDef,a,transInput->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

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

	Set *allNames = STRSET();
	List *uniqueAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(provJoin),singleton(hasProv->attrName));
	makeNamesUnique(uniqueAttrNames, allNames);

	op = createProjectionOp(projExpr, provJoin, NIL, uniqueAttrNames);
	provJoin->parents = singleton(op);
	provJoin = (QueryOperator *) op;

	// create duplicate removal
	QueryOperator *dr = (QueryOperator *) createDuplicateRemovalOp(projExpr, provJoin, NIL, getAttrNames(provJoin->schema));
	provJoin->parents = singleton(dr);
	provJoin = (QueryOperator *) dr;

	rewrittenTree = (Node *) provJoin;

	DEBUG_NODE_BEATIFY_LOG("rewritten query for summarization returned:", rewrittenTree);
	INFO_OP_LOG("rewritten query for summarization as overview:", rewrittenTree);

	return rewrittenTree;
}
