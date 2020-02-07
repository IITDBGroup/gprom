#include "common.h"
#include "log/logger.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/query_operator/query_operator_model_checker.h"

#include "analysis_and_translate/translator_oracle.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "utility/enum_magic.h"
#include "utility/string_utils.h"

/* type of uncertainty annotations we produce */
NEW_ENUM_WITH_ONLY_TO_STRING(UncertaintyType,
						UNCERTAIN_TUPLE_LEVEL,
						UNCERTAIN_ATTR_LEVEL,
						UNCERTAIN_ATTR_RANGES
	);

#define LEAST_FUNC_NAME backendifyIdentifier("least")
#define GREATEST_FUNC_NAME backendifyIdentifier("greatest")
#define UNCERT_FUNC_NAME backendifyIdentifier("uncert")
#define MAX_FUNC_NAME backendifyIdentifier("max")
#define MIN_FUNC_NAME backendifyIdentifier("min")
#define SUM_FUNC_NAME backendifyIdentifier("sum")
#define COUNT_FUNC_NAME backendifyIdentifier("count")
#define ROW_NUMBER_FUNC_NAME backendifyIdentifier("row_number")

/* xtables attributes */
#define MAX_PROB_ATTR_NAME "MAX_PROB"
#define SUM_PROB_ATTR_NAME "SUM_PROB"
#define COUNT_ATTR_NAME "COUNT_"
#define ROW_NUM_BY_ID_ATTR_NAME "ROW_NUM_BY_ID"

/* bound based uncertainty */
#define ATTR_LOW_BOUND backendifyIdentifier("LB_")
#define ATTR_HIGH_BOUND backendifyIdentifier("UB_")
#define ATTR_UNCERT_PFX backendifyIdentifier("U_")
#define SELFJOIN_AFFIX backendifyIdentifier("1")//differentiate attr names when selfjoin

/* function declarations */
static Node *UncertOp(Operator *expr, HashMap *hmp);
static Node *UncertIf(CaseExpr *expr, HashMap *hmp);
static Node *UncertFun(FunctionCall *expr, HashMap *hmp);
static Node *createCaseOperator(Node *expr);
static Node *createReverseCaseOperator(Node *expr);
static Node *getOutputExprFromInput(Node *expr, int offset);
Node *getUncertaintyExpr(Node *expr, HashMap *hmp);

//Range expression rewriting
static Node *RangeUBOp(Operator *expr, HashMap *hmp);
static Node *RangeLBOp(Operator *expr, HashMap *hmp);
Node *getUBExpr(Node *expr, HashMap *hmp);
Node *getLBExpr(Node *expr, HashMap *hmp);
extern char *getAttrTwoString(char *in);
//extern char *getAttrOneString(char *in);

// uncertain data ingeation rewrites
static QueryOperator *rewrite_UncertTIP(QueryOperator *op, UncertaintyType typ);
static QueryOperator *rewrite_UncertIncompleteTable(QueryOperator *op);
static QueryOperator *rewrite_UncertXTable(QueryOperator *op, UncertaintyType typ);
static QueryOperator *rewrite_RangeTIP(QueryOperator *op);

//uncertain query rewriting
static QueryOperator *rewriteUncertProvComp(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertSelection(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertProjection(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertJoin(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertAggregation(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertDuplicateRemoval(QueryOperator *op, boolean attrLevel);
static QueryOperator *rewrite_UncertSet(QueryOperator *op, boolean attrLevel);

static void addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef);

//Range query rewriting
static QueryOperator *rewriteRangeProvComp(QueryOperator *op);
static QueryOperator *rewrite_RangeTableAccess(QueryOperator *op);
static QueryOperator *rewrite_RangeProjection(QueryOperator *op);
static QueryOperator *rewrite_RangeSelection(QueryOperator *op);
static QueryOperator *rewrite_RangeJoin(QueryOperator *op);
static QueryOperator *rewrite_RangeAggregation(QueryOperator *op);

//Range query rewriting combiners
static QueryOperator *combineRowByBG(QueryOperator *op);
static QueryOperator *combineRowMinBg(QueryOperator *op);
static QueryOperator *combinePosToOne(QueryOperator *op);

static void addRangeAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef);
static void addRangeRowToSchema(HashMap *hmp, QueryOperator *target);

static List *putMidListToEnd(List *in, int p1, int p2);


QueryOperator *
rewriteUncert(QueryOperator * op)
{
	QueryOperator *rewrittenOp;

	if(HAS_STRING_PROP(op,PROP_TIP_ATTR))
	{
		rewrittenOp = rewrite_UncertTIP(op, UNCERTAIN_ATTR_LEVEL);
		return rewrittenOp;
	}

	if(HAS_STRING_PROP(op,PROV_PROP_INCOMPLETE_TABLE))
	{
		rewrittenOp = rewrite_UncertIncompleteTable(op);
		return rewrittenOp;
	}

	if(HAS_STRING_PROP(op,PROP_XTABLE_GROUPID))
	{
		if(HAS_STRING_PROP(op,PROP_XTABLE_PROB))
		{
			rewrittenOp = rewrite_UncertXTable(op, UNCERTAIN_ATTR_LEVEL);
			return rewrittenOp;
		}
	}

	switch(op->type)
	{
	    case T_ProvenanceComputation:
	        rewrittenOp = rewriteUncertProvComp(op, TRUE);
	        break;
		case T_TableAccessOperator:
			rewrittenOp =rewrite_UncertTableAccess(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite TableAccess:", rewrittenOp);
			break;
		case T_SelectionOperator:
			rewrittenOp = rewrite_UncertSelection(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite Selection:", rewrittenOp);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_UncertProjection(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite Projection:", rewrittenOp);
			break;
		case T_JoinOperator:
			rewrittenOp = rewrite_UncertJoin(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite Join:", rewrittenOp);
			break;
		case T_AggregationOperator:
			rewrittenOp = rewrite_UncertAggregation(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite Aggregation:", rewrittenOp);
			break;
		case T_DuplicateRemoval:
			rewrittenOp = rewrite_UncertDuplicateRemoval(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite DuplicateRemoval:", rewrittenOp);
			break;
		case T_SetOperator:
			rewrittenOp = rewrite_UncertSet(op, TRUE);;
			INFO_OP_LOG("Uncertainty Rewrite Set:", rewrittenOp);
			break;
		default:
			FATAL_LOG("rewrite for %s not implemented", NodeTagToString(op->type));
			rewrittenOp = NULL;
			break;
	}

	return rewrittenOp;
}

QueryOperator *
rewriteUncertTuple(QueryOperator *op)
{
	QueryOperator *rewrittenOp;

	if(HAS_STRING_PROP(op,PROP_TIP_ATTR))
	{
		rewrittenOp = rewrite_UncertTIP(op, UNCERTAIN_TUPLE_LEVEL);
		return rewrittenOp;
	}

	if(HAS_STRING_PROP(op,PROV_PROP_INCOMPLETE_TABLE))
	{
		rewrittenOp = rewrite_UncertIncompleteTable(op);
		return rewrittenOp;
	}

	if(HAS_STRING_PROP(op,PROP_XTABLE_GROUPID))
	{
		if(HAS_STRING_PROP(op,PROP_XTABLE_PROB))
		{
			rewrittenOp = rewrite_UncertXTable(op, UNCERTAIN_TUPLE_LEVEL);
			return rewrittenOp;
		}
	}

	switch(op->type)
	{
	    case T_ProvenanceComputation:
	        rewrittenOp = rewriteUncertProvComp(op, FALSE);
	        break;
		case T_TableAccessOperator:
			rewrittenOp = rewrite_UncertTableAccess(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite TableAccess:", rewrittenOp);
			break;
		case T_SelectionOperator:
			rewrittenOp = rewrite_UncertSelection(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite Selection:", rewrittenOp);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_UncertProjection(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite Projection:", rewrittenOp);
			break;
		case T_JoinOperator:
			rewrittenOp = rewrite_UncertJoin(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite Join:", rewrittenOp);
			break;
		case T_AggregationOperator:
			rewrittenOp = rewrite_UncertAggregation(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite Aggregation:", rewrittenOp);
			break;
		case T_DuplicateRemoval:
			rewrittenOp = rewrite_UncertDuplicateRemoval(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite DuplicateRemoval:", rewrittenOp);
			break;
		case T_SetOperator:
			rewrittenOp = rewrite_UncertSet(op, FALSE);
			INFO_OP_LOG("Uncertainty Rewrite Set:", rewrittenOp);
			break;
		default:
			FATAL_LOG("rewrite for %s not implemented", NodeTagToString(op->type));
			rewrittenOp = NULL;
			break;
	}

	return rewrittenOp;
}

QueryOperator *
rewriteRange(QueryOperator * op)
{
	QueryOperator *rewrittenOp;
	if(HAS_STRING_PROP(op,PROP_TIP_ATTR))
	{
		rewrittenOp = rewrite_RangeTIP(op);
		INFO_OP_LOG("Range Rewrite TIP:", rewrittenOp);
		return rewrittenOp;
	}

	if(HAS_STRING_PROP(op,PROV_PROP_INCOMPLETE_TABLE))
	{
		rewrittenOp = rewrite_UncertIncompleteTable(op);
		return rewrittenOp;
	}

	if(HAS_STRING_PROP(op,PROP_XTABLE_GROUPID))
	{
		if(HAS_STRING_PROP(op,PROP_XTABLE_PROB))
		{
			rewrittenOp = rewrite_UncertXTable(op, UNCERTAIN_ATTR_RANGES);
			return rewrittenOp;
		}
	}

	switch(op->type)
	{
	    case T_ProvenanceComputation:
	        rewrittenOp = rewriteRangeProvComp(op);
	        break;
		case T_TableAccessOperator:
			rewrittenOp = rewrite_RangeTableAccess(op);
			if(0){
				rewrittenOp = combinePosToOne(rewrittenOp);
			}
			INFO_OP_LOG("Range Rewrite TableAccess:", rewrittenOp);
			break;
		case T_SelectionOperator:
			rewrittenOp = rewrite_RangeSelection(op);
			INFO_OP_LOG("Range Rewrite Selection:", rewrittenOp);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_RangeProjection(op);
			INFO_OP_LOG("Range Rewrite Projection:", rewrittenOp);
			break;
		case T_JoinOperator:
			rewrittenOp = rewrite_RangeJoin(op);
			INFO_OP_LOG("Range Rewrite Join:", rewrittenOp);
			break;
		case T_AggregationOperator:
			rewrittenOp = rewrite_RangeAggregation(op);
			INFO_OP_LOG("Range Rewrite Aggregation:", rewrittenOp);
			break;
		case T_DuplicateRemoval:
			rewrittenOp = rewrite_UncertDuplicateRemoval(op, TRUE);
			INFO_OP_LOG("Uncertainty Rewrite DuplicateRemoval:", rewrittenOp);
			break;
		case T_SetOperator:
			rewrittenOp = rewrite_UncertSet(op, TRUE);
			INFO_OP_LOG("Uncertainty Rewrite Set:", rewrittenOp);
			break;
		default:
			FATAL_LOG("rewrite for %s not implemented", NodeTagToString(op->type));
			rewrittenOp = NULL;
			break;
	}
	return rewrittenOp;
}

Node *
removeUncertOpFromExpr(Node *expr)
{
	if(!expr){
		return NULL;
	}
	switch(expr->type){
			case T_Operator: {
				if(streq(((Operator *)expr)->name,UNCERT_FUNC_NAME)) {
						return (Node *)getHeadOfListP(((Operator *)expr)->args);
					}
				FOREACH(Node, nd, ((Operator *)expr)->args){
					replaceNode(((Operator *)expr)->args, nd, removeUncertOpFromExpr(nd));
				}
				return expr;
				break;
			}
			case T_CaseExpr: {
				((CaseExpr *)expr)->elseRes = removeUncertOpFromExpr(((CaseExpr *)expr)->elseRes);
				((CaseExpr *)expr)->expr = removeUncertOpFromExpr(((CaseExpr *)expr)->expr);
				FOREACH(Node, nd, ((CaseExpr *)expr)->whenClauses){
					CaseWhen *tmp = (CaseWhen *)nd;
					tmp->when = removeUncertOpFromExpr(tmp->when);
					tmp->then = removeUncertOpFromExpr(tmp->then);
				}
				return expr;
				break;
			}
			case T_FunctionCall: {
				if(streq(((FunctionCall *)expr)->functionname,UNCERT_FUNC_NAME)) {
					return (Node *)getHeadOfListP(((FunctionCall *)expr)->args);
				}
				FOREACH(Node, nd, ((FunctionCall *)expr)->args){
					replaceNode(((Operator *)expr)->args, nd, removeUncertOpFromExpr(nd));
				}
				return expr;
				break;
			}
			default: {
				return expr;
				break;
			}
		}
		return expr;
}

Node *
getUBExpr(Node *expr, HashMap *hmp)
{
	switch(expr->type){
		case T_Constant: {
			return expr;
		}
		case T_AttributeReference: {
			if(((AttributeReference *)expr)->outerLevelsUp == -1){
				((AttributeReference *)expr)->outerLevelsUp = 0;
			}
//			INFO_LOG("AttrExprUB - %s", nodeToString(hmp));
//			INFO_LOG("AttrExprUB - %s", nodeToString(expr));

			Node * ret = getNthOfListP((List *)getMap(hmp, expr), 0);
			((AttributeReference *)ret)->outerLevelsUp = 0;
//			INFO_LOG("AttrExprUB - %s", ((AttributeReference *)ret)->name);
			return ret;
		}
		case T_Operator: {
			return RangeUBOp((Operator *) expr, hmp);
		}
		case T_CaseExpr: {
			return UncertIf((CaseExpr *) expr, hmp);
		}
		case T_FunctionCall: {
			return UncertFun((FunctionCall *) expr, hmp);
		}
		default: {
			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
		}
	}
	return NULL;
}

Node *
getLBExpr(Node *expr, HashMap *hmp)
{
	switch(expr->type){
		case T_Constant: {
			return expr;
		}
		case T_AttributeReference: {
			if(((AttributeReference *)expr)->outerLevelsUp == -1){
				((AttributeReference *)expr)->outerLevelsUp = 0;
			}
			Node * ret = getNthOfListP((List *)getMap(hmp, expr), 1);
			((AttributeReference *)ret)->outerLevelsUp = 0;
//			INFO_LOG("AttrExprLB - %s - %s", ((AttributeReference *)expr)->name, ((AttributeReference *)ret)->name);
			return ret;
		}
		case T_Operator: {
			return RangeLBOp((Operator *) expr, hmp);
		}
		case T_CaseExpr: {
			return UncertIf((CaseExpr *) expr, hmp);
		}
		case T_FunctionCall: {
			return UncertFun((FunctionCall *) expr, hmp);
		}
		default: {
			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
		}
	}
	return NULL;
}

Node *
getUncertaintyExpr(Node *expr, HashMap *hmp)
{
	INFO_LOG("expression: %s ,  %p", exprToSQL(expr, NULL), expr); //TODO deal with nested subqueries
	switch(expr->type){
		case T_Constant: {
			return (Node *)createConstInt(1);
		}
		case T_AttributeReference: {
			if(((AttributeReference *)expr)->outerLevelsUp == -1){
				((AttributeReference *)expr)->outerLevelsUp = 0;
			}
			Node * ret = getMap(hmp, expr);
			((AttributeReference *)ret)->outerLevelsUp = 0;
			return ret;
		}
		case T_Operator: {
			return UncertOp((Operator *) expr, hmp);
		}
		case T_CaseExpr: {
			return UncertIf((CaseExpr *) expr, hmp);
		}
		case T_FunctionCall: {
			return UncertFun((FunctionCall *) expr, hmp);
		}
		default: {
			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
		}
	}
	return NULL;
}

char *
getUncertString(char *in)
{
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", ATTR_UNCERT_PFX);
	appendStringInfo(str, "%s", in);
	return str->data;
}

char *
getUBString(char *in)
{
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", ATTR_HIGH_BOUND);
	appendStringInfo(str, "%s", in);
	return backendifyIdentifier(str->data);
}

char *
getLBString(char *in)
{
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", ATTR_LOW_BOUND);
	appendStringInfo(str, "%s", in);
	return backendifyIdentifier(str->data);
}

//Combine row annotations group by best guess on REWRITTEN operator.
static QueryOperator *combineRowByBG(QueryOperator *op){
	HashMap * hmp = NEW_MAP(Node, Node);
	HashMap * hmpIn = (HashMap *)getStringProperty(op, "UNCERT_MAPPING");

	List *attrExpr = getNormalAttrProjectionExprs(op);
	List *oldattrname = getNormalAttrNames(op);
	List *attrnames = NIL;
	List *aggrs = NIL;
	List *groupBy = NIL;

	FOREACH(Node, nd, attrExpr){
		Node *node = copyObject(nd);
		if(((AttributeReference *)node)->outerLevelsUp == -1){
			((AttributeReference *)node)->outerLevelsUp = 0;
		}
		if(hasMapKey(hmpIn,node)) {
//			INFO_LOG("Comb_row_by_bg_haskey: %s", nodeToString(node));
			groupBy = appendToTailOfList(groupBy, node);
			List *val = (List *)getMap(hmpIn, node);
			Node *max = (Node *)createFunctionCall(MAX_FUNC_NAME,singleton(copyObject(getHeadOfListP(val))));
//			INFO_LOG("Comb_row_by_bg_max: %s", nodeToString(max));
			aggrs = appendToTailOfList(aggrs, max);
			attrnames = appendToTailOfList(attrnames, ((AttributeReference *)getHeadOfListP(val))->name);
			Node *min = (Node *)createFunctionCall(MIN_FUNC_NAME,singleton(copyObject(getTailOfListP(val))));
//			INFO_LOG("Comb_row_by_bg_min: %s", nodeToString(max));
			aggrs = appendToTailOfList(aggrs, min);
			attrnames = appendToTailOfList(attrnames, ((AttributeReference *)getTailOfListP(val))->name);
		}
	}
	Node * node = getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN));
	Node *ct = (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(node)));
	aggrs = appendToTailOfList(aggrs, ct);
	attrnames = appendToTailOfList(attrnames, ((AttributeReference *)node)->name);

	node = getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS));
	ct = (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(node)));
	aggrs = appendToTailOfList(aggrs, ct);
	attrnames = appendToTailOfList(attrnames, ((AttributeReference *)node)->name);

	node = getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE));
	ct = (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(node)));
	aggrs = appendToTailOfList(aggrs, ct);
	attrnames = appendToTailOfList(attrnames, ((AttributeReference *)node)->name);

	FOREACH(Node, nd, groupBy){
		attrnames = appendToTailOfList(attrnames, ((AttributeReference *)nd)->name);
	}

//	INFO_LOG("Comb_row_by_bg_attrNames: %s", stringListToString(attrnames));
//	INFO_LOG("Comb_row_by_bg_aggrs: %s", nodeToString(aggrs));
//	INFO_LOG("Comb_row_by_bg_groupby: %s", nodeToString(groupBy));

	int normalattrlen = groupBy->length;

	QueryOperator *aggrop = (QueryOperator *)createAggregationOp(aggrs, groupBy, op, NIL, attrnames);
	switchSubtrees(op, aggrop);
	op->parents = singleton(aggrop);

	FOREACH(AttributeDef, nd, aggrop->schema->attrDefs){
		if(nd->dataType==DT_LONG){
			nd->dataType=DT_INT;
		}
	}

	attrExpr = getNormalAttrProjectionExprs(aggrop);
	List *projexpr1 = sublist(attrExpr, attrExpr->length-normalattrlen, attrExpr->length-1);
	INFO_LOG("projexpr1: %s", nodeToString(projexpr1));
	List *projexpr2 = sublist(attrExpr, 0, attrExpr->length-normalattrlen-1);
	INFO_LOG("projexpr2: %s", nodeToString(projexpr2));
	projexpr1 = concatTwoLists(projexpr1, projexpr2);

	oldattrname = sublist(oldattrname, 0, normalattrlen-1);

	QueryOperator *projop = (QueryOperator *)createProjectionOp(projexpr1, aggrop, NIL, oldattrname);
	switchSubtrees(aggrop, projop);
	aggrop->parents = singleton(projop);

	List *projExprs = getNormalAttrProjectionExprs(projop);
	List *plist = sublist(projExprs, 0, normalattrlen-1);

	FOREACH(Node, nd, plist){
		addRangeAttrToSchema(hmp, projop, nd);
	}
	addRangeRowToSchema(hmp, projop);
	setStringProperty(projop, "UNCERT_MAPPING", (Node *)hmp);

	return projop;
}

static QueryOperator *combineRowMinBg(QueryOperator *op) {

	HashMap * hmpIn = (HashMap *)getStringProperty(op, "UNCERT_MAPPING");

	List *attrExpr = getNormalAttrProjectionExprs(op);
//	List *oldattrname = getNormalAttrNames(op);
	List *attrnames = NIL;
	List *aggrs = NIL;
	List *norm = NIL;
	List *normname = NIL;

	FOREACH(Node, nd, attrExpr){
		Node *node = copyObject(nd);
		if(((AttributeReference *)node)->outerLevelsUp == -1){
			((AttributeReference *)node)->outerLevelsUp = 0;
		}
		if(hasMapKey(hmpIn,node)) {
//			INFO_LOG("Comb_row_by_bg_haskey: %s", nodeToString(node));
			norm = appendToTailOfList(norm, (Node *)createFunctionCall(MIN_FUNC_NAME,singleton(node)));
			normname = appendToTailOfList(normname, ((AttributeReference *)node)->name);
			List *val = (List *)getMap(hmpIn, node);
			Node *max = (Node *)createFunctionCall(MAX_FUNC_NAME,singleton(copyObject(getHeadOfListP(val))));
			aggrs = appendToTailOfList(aggrs, max);
			attrnames = appendToTailOfList(attrnames, ((AttributeReference *)getHeadOfListP(val))->name);
			Node *min = (Node *)createFunctionCall(MIN_FUNC_NAME,singleton(copyObject(getTailOfListP(val))));
//			INFO_LOG("Comb_row_by_bg_min: %s", nodeToString(max));
			aggrs = appendToTailOfList(aggrs, min);
			attrnames = appendToTailOfList(attrnames, ((AttributeReference *)getTailOfListP(val))->name);
		}
	}
	Node * node = getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN));
	Node *ct = (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(node)));
	aggrs = appendToTailOfList(aggrs, ct);
	attrnames = appendToTailOfList(attrnames, ((AttributeReference *)node)->name);

	node = getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS));
	ct = (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(node)));
	aggrs = appendToTailOfList(aggrs, ct);
	attrnames = appendToTailOfList(attrnames, ((AttributeReference *)node)->name);

	node = getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE));
	ct = (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(node)));
	aggrs = appendToTailOfList(aggrs, ct);
	attrnames = appendToTailOfList(attrnames, ((AttributeReference *)node)->name);

	normname = concatTwoLists(normname, attrnames);
	norm = concatTwoLists(norm, aggrs);

	QueryOperator *aggrop = (QueryOperator *)createAggregationOp(norm, NIL, op, NIL, normname);
	switchSubtrees(op, aggrop);
	op->parents = singleton(aggrop);

	Operator *notnull = createOpExpr(">", LIST_MAKE(getAttrRefByName(aggrop,ROW_POSSIBLE), createConstInt(0)));
	QueryOperator *selpos = (QueryOperator *)createSelectionOp((Node *)notnull, aggrop, NIL, normname);
	switchSubtrees(aggrop, selpos);
	aggrop->parents = singleton(selpos);

	setStringProperty(selpos, "UNCERT_MAPPING", (Node *)hmpIn);

//	INFO_OP_LOG("aggr min bg:", selpos);
	return selpos;
}

static QueryOperator *combinePosToOne(QueryOperator *op) {
	HashMap * hmpIn = (HashMap *)getStringProperty(op, "UNCERT_MAPPING");
	QueryOperator *opdup = copyObject(op);
	Operator *bgSel = createOpExpr(">", LIST_MAKE(getAttrRefByName(op,ROW_BESTGUESS), createConstInt(0)));
	Operator *posSel = createOpExpr("=", LIST_MAKE(getAttrRefByName(opdup,ROW_BESTGUESS), createConstInt(0)));
	List *attrnames = getNormalAttrNames(op);

	QueryOperator *bg = (QueryOperator *)createSelectionOp((Node *)bgSel, op, NIL, attrnames);
	switchSubtrees(op, bg);
	op->parents = singleton(bg);
	setStringProperty(bg, "UNCERT_MAPPING", (Node *)hmpIn);

	QueryOperator *pos = (QueryOperator *)createSelectionOp((Node *)posSel, opdup, NIL, attrnames);
	switchSubtrees(opdup, pos);
	opdup->parents = singleton(pos);
	setStringProperty(pos, "UNCERT_MAPPING", (Node *)hmpIn);

	INFO_OP_LOG("bg:", bg);
	INFO_OP_LOG("pos:", pos);
	//
	QueryOperator *onepos = combineRowMinBg(pos);

	QueryOperator *unionop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(bg, onepos), NIL, attrnames);
	switchSubtrees(bg, unionop);
	bg->parents = singleton(unionop);
	onepos->parents = singleton(unionop);

	setStringProperty(unionop, "UNCERT_MAPPING", (Node *)hmpIn);

	return unionop;
}

static QueryOperator *
rewrite_RangeTIP(QueryOperator *op)
{
	INFO_LOG("rewriteRangeTIP - %s\n", UncertaintyTypeToString(UNCERTAIN_ATTR_RANGES));

	char * TIPName = STRING_VALUE(GET_STRING_PROP(op,PROP_TIP_ATTR));

//	int pos = getAttrRefByName(op,TIPName)->attrPosition;

	Operator *bgcond = createOpExpr(">=", LIST_MAKE(getAttrRefByName(op,TIPName), createConstFloat(0.5)));
	Operator *certcond = createOpExpr(">=", LIST_MAKE(getAttrRefByName(op,TIPName), createConstFloat(1.0)));
	Operator *poscond = createOpExpr(">", LIST_MAKE(getAttrRefByName(op,TIPName), createConstFloat(0.0)));

	HashMap *hmp = NEW_MAP(Node, Node);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(((ProjectionOperator *)createProjOnAllAttrs(op))->projExprs, op, NIL, getAttrNames(op->schema));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

//	INFO_LOG("Range_TIP_proj: %s", nodeToString(((ProjectionOperator *)proj)->projExprs));

	List *attrExpr = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr){
		addRangeAttrToSchema(hmp, proj, nd);
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
	}
	addRangeRowToSchema(hmp, proj);
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator((Node *)certcond));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator((Node *)bgcond));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator((Node *)poscond));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);

//	INFO_LOG("Range_TIP_HMP: %s", nodeToString(((ProjectionOperator *)proj)->projExprs));
//	INFO_LOG("Range_TIP_HMP: %s", nodeToString(hmp));

	return proj;
}

static QueryOperator *
rewrite_UncertTIP(QueryOperator *op, UncertaintyType typ)
{
	if(typ == UNCERTAIN_ATTR_RANGES)
		return rewrite_RangeTIP(op);

	DEBUG_LOG("rewriteUncertTIP - %s\n", UncertaintyTypeToString(typ));
	//prints the op->provAttr = singletonint
	//get TIP attribute name using PROP_USER_TIP_ATTR as the key
	char * TIPName = STRING_VALUE(GET_STRING_PROP(op,PROP_TIP_ATTR));

	//get TIP attribute position
	//	int TIPPos = getAttrPos(op,TIPName);

	//Create operator expression
	//Create full attribute reference -> datatype -> cast? -> opschema?
	Operator *ltequal = createOpExpr("<=",LIST_MAKE(createConstFloat(0.5),getAttrRefByName(op,TIPName)));

	//create select op with the condition
	QueryOperator *selec = (QueryOperator *)createSelectionOp((Node *)ltequal, op, NIL, getAttrNames(op->schema));

	//Uncert attributes Hashmap
	HashMap * hmp = NEW_MAP(Node, Node);

	//create proj operator on the selection operator results
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(selec), selec, NIL, getNormalAttrNames(selec));

	//switching subtrees
	switchSubtrees(op, proj);

	//parent pointers for select operator
	selec->parents = singleton(proj);

	//parent pointer for op
	op->parents = singleton(selec);

	if (typ == UNCERTAIN_ATTR_LEVEL)
	{
		//Final projection? U_A.... U_R
		List *attrExpr = getNormalAttrProjectionExprs(op);
		FOREACH(Node, nd, attrExpr)
		{
			//Add U_nd->name to the schema, with data type int
			addUncertAttrToSchema(hmp, proj, nd);
			//Set the values of U_nd->name to 1
			appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
		}
	}

	//Create operator expression when P==1
	Node *TIPIsOne = (Node *)createOpExpr("=",LIST_MAKE(createConstFloat(1),getAttrRefByName(op,TIPName)));

	//Add U_R to the schema with data type int
	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	//Set the values of U_R using a CASE WHEN TIPisOne is true
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(TIPIsOne));

	//Update string property
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);

	DEBUG_NODE_BEATIFY_LOG("rewritten query root for TIP uncertainty is:", proj);

	return proj;
}

static QueryOperator *
rewrite_UncertIncompleteTable(QueryOperator *op)
{
	DEBUG_LOG("rewriteIncompleteTable\n");

	//Uncert attributes Hashmap
	HashMap * hmp = NEW_MAP(Node, Node);

	//create proj operator on the op
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));

	//switching subtrees
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

	//Create operator expression when an entry is NULL
	//Node *entryIsNull = (Node *)createOpExpr("is",LIST_MAKE(getNormalAttrNames(op), (Node *) createConstString("NULL")));

	//Final projection? U_A.... U_R
	List *attrExpr = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr){
		//Add U_nd->name to the schema, with data type int
		addUncertAttrToSchema(hmp, proj, nd);
		//Set the values of U_nd->name to CASE WHEN entryIsNull
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs,
				createCaseExpr(NULL,singleton(createCaseWhen((Node *)createIsNullExpr(nd),
				(Node *)createConstInt(0))),(Node *)createConstInt(1)));
	}

	//Add U_R to the schema with data type int
	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	//Set the values of U_R to 1
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));

	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);

	DEBUG_NODE_BEATIFY_LOG("rewritten query root for INCOMPLETE uncertainty is:", proj);

	return proj;
}

static  QueryOperator *
rewrite_UncertXTable(QueryOperator *op, UncertaintyType typ)
{
	DEBUG_LOG("rewriteXTable - %s\n", UncertaintyTypeToString(typ));

	//Uncert attributes Hashmap
	HashMap * hmp = NEW_MAP(Node, Node);

	//get Group Id attribute name using PROP_PROP_XTABLE_GROUPID as the key
	char *groupId = STRING_VALUE(GET_STRING_PROP(op,PROP_XTABLE_GROUPID));
	//get Probability attribute name using PROP_XTABLE_PROB as the key
	char *prob = STRING_VALUE(GET_STRING_PROP(op,PROP_XTABLE_PROB));

	//Get attribute reference for Group ID
	AttributeReference *groupIdRef = getAttrRefByName(op, groupId);
	//Get attribute reference for Probability
	AttributeReference *probRef = getAttrRefByName(op, prob);

	//Make partition by;
	List *partByGroupId = singleton(copyObject(groupIdRef));

	//WindowBound *winBoundCountOpen = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
	//WindowFrame *winFrameCountOpen = createWindowFrame(WINFRAME_ROWS,winBoundCountOpen,NULL);

	/* Window nnnfunction 1 - max */
	//Make max(prob) function call
	FunctionCall *maxProbFC = createFunctionCall(MAX_FUNC_NAME,singleton(copyObject(probRef)));
	char *maxProbName = MAX_PROB_ATTR_NAME;
	QueryOperator *maxProbWOp = (QueryOperator *) createWindowOp((Node *)maxProbFC, partByGroupId, NIL, NULL, maxProbName, op, NIL);

	/* Window function 2 - sum*/
	//Make sum(prob) function call
	FunctionCall *sumProbFC = createFunctionCall(SUM_FUNC_NAME,singleton(copyObject(probRef)));
	char *sumProbName = SUM_PROB_ATTR_NAME;
	QueryOperator *sumProbWOp = (QueryOperator *) createWindowOp((Node *)sumProbFC, partByGroupId, NIL, NULL, sumProbName, maxProbWOp, NIL);
	maxProbWOp->parents = singleton(sumProbWOp);

	/* Window function 3+ - count attr */
	//TODO count is not necessary (and window count distinct does not work in postgres, can just check min == max
	QueryOperator *prevWOp = sumProbWOp;
	/* List *attrExpr1 = getNormalAttrProjectionExprs(op); */
	/* FOREACH(Node, nd, attrExpr1) */
	/* { */
	/* 	char *countAttrName = CONCAT_STRINGS(COUNT_ATTR_NAME,((AttributeReference *)nd)->name); */
	/* 	//Make count(nd) function call */
	/* 	FunctionCall *countNdFC = createFunctionCall(COUNT_FUNC_NAME,singleton(nd)); */
	/* 	countNdFC->isDistinct = TRUE; */
	/* 	QueryOperator *countNdWOp = (QueryOperator *)createWindowOp((Node *)countNdFC, partByGroupId, NIL, NULL, countAttrName, prevWOp, NIL); */

	/* 	prevWOp->parents = singleton(countNdWOp); */
	/* 	prevWOp = countNdWOp; */
	/* } */

	/* Window function 4+ - min attr*/
	List *attrExpr2 = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr2)
	{
		char *lowAttrName = getLBString(((AttributeReference *) nd)->name);
		//Make the MIN(nd) function call
		FunctionCall *minNdFC = createFunctionCall(MIN_FUNC_NAME,singleton(nd));
		QueryOperator *minNdWOp = (QueryOperator *)createWindowOp((Node *)minNdFC, partByGroupId, NIL, NULL, lowAttrName, prevWOp, NIL);

		prevWOp->parents = singleton(minNdWOp);
		prevWOp = minNdWOp;
	}

	/* Window function 5+ - max attr*/
	List *attrExpr3 = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr3)
	{
		char *highAttrName = getUBString(((AttributeReference *)nd)->name);
		//Make the MAX(nd) function call
		FunctionCall *maxNdFC = createFunctionCall(MAX_FUNC_NAME,singleton(nd));
		QueryOperator *maxNdWOp = (QueryOperator *)createWindowOp((Node *)maxNdFC, partByGroupId, NIL, NULL, highAttrName, prevWOp, NIL);

		prevWOp->parents = singleton(maxNdWOp);
		prevWOp = maxNdWOp;
	}

	Operator *selec1Cond = NULL;

	// range semantics, only show most likely alternative, but return one alternative per x-tuple
	if (typ == UNCERTAIN_ATTR_RANGES)
	{
		selec1Cond = createOpExpr("=",LIST_MAKE(getAttrRefByName(prevWOp, maxProbName), copyObject(probRef)));
	}
	// UADB, only show rows that are best guess (maximal probability alternative unless not including an alternative has the highest probability)
	else if (typ == UNCERTAIN_TUPLE_LEVEL || typ == UNCERTAIN_ATTR_LEVEL)
	{
		Operator *oneMinusSum = createOpExpr("-",LIST_MAKE(createConstInt(1),getAttrRefByName(prevWOp, sumProbName)));
		Operator *firstParam = createOpExpr(">",LIST_MAKE(getAttrRefByName(prevWOp, maxProbName),oneMinusSum));

		Operator *secondParam = createOpExpr("=",LIST_MAKE(getAttrRefByName(prevWOp, maxProbName), copyObject(probRef)));
		selec1Cond = createOpExpr("AND", LIST_MAKE(firstParam,secondParam));
	}

	/* Selection - Select rows with the maximum probability  */
	QueryOperator *selecMaxProbRow = (QueryOperator *)createSelectionOp((Node *)selec1Cond, prevWOp, NIL, getAttrNames(prevWOp->schema));
	prevWOp->parents = singleton(selecMaxProbRow);

	/* Window function 6 - row number*/
	//Make sum(prob) function call
	FunctionCall *rowNumByIdFC = createFunctionCall(ROW_NUMBER_FUNC_NAME, NIL);
	char *rowNumByIdName = ROW_NUM_BY_ID_ATTR_NAME;
	List *orderBy = NIL;
	orderBy = appendToTailOfList(orderBy, copyObject(probRef));

	QueryOperator *rowNumberByIdWOp = (QueryOperator *) createWindowOp((Node *)rowNumByIdFC, partByGroupId, orderBy, NULL, rowNumByIdName, selecMaxProbRow, NIL);
	selecMaxProbRow->parents = singleton(rowNumberByIdWOp);

	/* Selection - Select rows with row number equal to 1 */
	Operator *countEqualsOne = createOpExpr("=",LIST_MAKE(createConstInt(1),getAttrRefByName(rowNumberByIdWOp, rowNumByIdName)));
	QueryOperator *selecRowNumberIsOne = (QueryOperator *)createSelectionOp((Node *)countEqualsOne, rowNumberByIdWOp, NIL, getAttrNames(rowNumberByIdWOp->schema));
	rowNumberByIdWOp->parents = singleton(selecRowNumberIsOne);

	/* Final Projection */
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(selecRowNumberIsOne), selecRowNumberIsOne, NIL, getNormalAttrNames(selecRowNumberIsOne));
	selecRowNumberIsOne->parents = singleton(proj);

	/* either add uncertain attributes or add range bounds */
	List *attrExpr4 = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr4)
	{
		// range uncertainty
		if(typ == UNCERTAIN_ATTR_RANGES)
		{
			addRangeAttrToSchema(hmp, proj, nd);
			appendToTailOfList(((ProjectionOperator *)proj)->projExprs,
							   getAttrRefByName(selecRowNumberIsOne, getUBString(((AttributeReference *) nd)->name)));
			appendToTailOfList(((ProjectionOperator *)proj)->projExprs,
							   getAttrRefByName(selecRowNumberIsOne, getLBString(((AttributeReference *) nd)->name)));
		}
		// attribute or tuple level cerainty
		else if (typ == UNCERTAIN_ATTR_LEVEL)
		{
			//Add U_nd->name to the schema, with data type int
			addUncertAttrToSchema(hmp, proj, nd);
			appendToTailOfList(((ProjectionOperator *)proj)->projExprs,
							   createCaseOperator((Node *)createOpExpr("=",LIST_MAKE(createConstFloat(1),getAttrRefByName(selecRowNumberIsOne,maxProbName)))));
		}
	}

	//Condition for U_R
	if (typ == UNCERTAIN_ATTR_RANGES)
	{
		addRangeRowToSchema(hmp, proj);

		// certain = sum probability is 1.0 (there is only one alternative with probability of one
		Node *sumProbIsOne = (Node *)createOpExpr("=",LIST_MAKE(createConstFloat(1),getAttrRefByName(selecRowNumberIsOne,sumProbName)));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(sumProbIsOne));

		// best guess (max probability of option is larger equals to 0.5
		Node *maxProbLargerOneMinusSum = (Node *)createOpExpr("<=",LIST_MAKE(
															   createOpExpr("-", LIST_MAKE(createConstFloat(1), getAttrRefByName(selecRowNumberIsOne, sumProbName))),
															   getAttrRefByName(selecRowNumberIsOne,maxProbName)));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(maxProbLargerOneMinusSum));

		// possible (always 1)
		appendToTailOfList(((ProjectionOperator *) proj)->projExprs, createConstInt(1));
	}
	else
	{
		Node *sumProbIsOne = (Node *)createOpExpr("=",LIST_MAKE(createConstFloat(1),getAttrRefByName(selecRowNumberIsOne,sumProbName)));
		//Add U_R to the schema with data type int
		addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(sumProbIsOne));
	}

	switchSubtrees(op, proj);
	op->parents = singleton(maxProbWOp);

	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);

	return proj;
}

static QueryOperator *
rewriteUncertProvComp(QueryOperator *op, boolean attrLevel)
{
    ASSERT(LIST_LENGTH(op->inputs) == 1);
    QueryOperator *top = getHeadOfListP(op->inputs);

	if (attrLevel)
	{
		top = rewriteUncert(top);
	}
	else
	{
		top = rewriteUncertTuple(top);
	}

    // make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    disambiguiteAttrNames((Node *) top, done);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root for uncertainty is:", top);

    return top;
}

static QueryOperator *
rewriteRangeProvComp(QueryOperator *op)
{
    ASSERT(LIST_LENGTH(op->inputs) == 1);
    QueryOperator *top = getHeadOfListP(op->inputs);

    top = rewriteRange(top);

    // make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    disambiguiteAttrNames((Node *) top, done);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root for range is:", top);

    return top;
}


static List *
putMidListToEnd(List *in, int p1, int p2)
{
	if(!in){
		return in;
	}
	List *uncert = sublist(in, p2, in->length-1);
	List *gbby = sublist(in, p1, p2-1);
	in = sublist(in, 0, p1-1);
	in = CONCAT_LISTS(in, uncert, gbby);
	return in;
}

static Node *
createCaseOperator(Node *expr)
{
	CaseWhen * cwhen = createCaseWhen(expr, (Node *)createConstInt(1));
	return (Node *)createCaseExpr(NULL, singleton(cwhen), (Node *)createConstInt(0));
}

static Node *
createReverseCaseOperator(Node *expr)
{
	CaseWhen * cwhen = createCaseWhen(expr, (Node *)createConstInt(0));
	return (Node *)createCaseExpr(NULL, singleton(cwhen), (Node *)createConstInt(1));
}

static Node *
getOutputExprFromInput(Node *expr, int offset)
{
    if(!expr){
        return NULL;
    }
    switch(expr->type){
        case T_AttributeReference: {
            if(((AttributeReference *)expr)->fromClauseItem==1){
                ((AttributeReference *)expr)->fromClauseItem = 0;
                ((AttributeReference *)expr)->attrPosition += offset;
            }
            return expr;
            break;
        }
        case T_Operator: {
            FOREACH(Node, nd, ((Operator *)expr)->args){
                getOutputExprFromInput(nd, offset);
            }
            return expr;
            break;
        }
        case T_CaseExpr: {
            ((CaseExpr *)expr)->elseRes = getOutputExprFromInput(((CaseExpr *)expr)->elseRes, offset);
            ((CaseExpr *)expr)->expr = getOutputExprFromInput(((CaseExpr *)expr)->expr, offset);
            FOREACH(Node, nd, ((CaseExpr *)expr)->whenClauses){
                CaseWhen *tmp = (CaseWhen *)nd;
                tmp->when = getOutputExprFromInput(tmp->when, offset);
                tmp->then = getOutputExprFromInput(tmp->then, offset);
            }
            return expr;
            break;
        }
        case T_FunctionCall: {
            FOREACH(Node, nd, ((FunctionCall *)expr)->args){
                getOutputExprFromInput(nd, offset);
            }
            return expr;
            break;
        }
        default: {
            return expr;
            break;
        }
    }
    return expr;
}

static Node *
UncertFun(FunctionCall *expr, HashMap *hmp)
{
	if(streq(expr->functionname,UNCERT_FUNC_NAME)) {
		return (Node *)createConstInt(0);
	}
	else
	{
	    Node *result = NULL;
	    FOREACH(Node,sub,expr->args)
        {
	        if (result == NULL)
	        {
	            result = getUncertaintyExpr(sub, hmp);
	        }
	        else
	        {
	            result = (Node *)createFunctionCall(LEAST_FUNC_NAME,
	                    LIST_MAKE(result,getUncertaintyExpr(sub, hmp)));
	        }
        }

	    if (result == NULL)
	        result = (Node *) createConstInt(1);

	    return result;
	}
}

static Node *
UncertIf(CaseExpr *expr, HashMap *hmp)
{
	Node *ret = NULL;
	Node *elseExpr = NULL;
	if(expr->elseRes){
		elseExpr = getUncertaintyExpr(expr->elseRes, hmp);
	}
	if(expr->expr){
		FOREACH(Node,nd,expr->whenClauses) {
			Node *exprtmp = (Node *)createOpExpr("=", appendToTailOfList(singleton(expr->expr),((CaseWhen *)nd)->when));
			Node * uncertwhen = getUncertaintyExpr(exprtmp, hmp);
			Node * uncertthen = getUncertaintyExpr(((CaseWhen *)nd)->then, hmp);
			Node * evalwhen = createCaseOperator(exprtmp);
			Node *temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(uncertwhen),uncertthen));
			temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(temp),evalwhen));
			if(!ret) {
				ret = temp;
			} else {
				ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(ret),temp));
			}
			if(elseExpr){
				Node *evalwhen = createReverseCaseOperator(exprtmp);
				temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(uncertwhen),evalwhen));
				elseExpr = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(elseExpr),temp));
			}
		}
	} else {
		FOREACH(Node,nd,expr->whenClauses) {
			Node * uncertwhen = getUncertaintyExpr(((CaseWhen *)nd)->when, hmp);
			Node * uncertthen = getUncertaintyExpr(((CaseWhen *)nd)->then, hmp);
			Node * evalwhen = createCaseOperator(((CaseWhen *)nd)->when);
			Node *temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(uncertwhen),uncertthen));
			temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(temp),evalwhen));
			if(!ret) {
				ret = temp;
			} else {
				ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(ret),temp));
			}
			if(elseExpr){
				Node *evalwhen = createReverseCaseOperator(((CaseWhen *)nd)->when);
				temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(uncertwhen),evalwhen));
				elseExpr = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(elseExpr),temp));
			}
		}
	}
	if(elseExpr){
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(ret),elseExpr));
	}
	return ret;
}

//uncertainty func
static Node *
UncertOp(Operator *expr, HashMap *hmp)
{
	if(!expr){
		return NULL;
	}
	if(strcmp(expr->name,UNCERT_FUNC_NAME)==0) {
		return (Node *)createConstInt(0);
	}
	if(strcmp(expr->name,"*")==0) {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		Node *c1 = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(getUncertaintyExpr(e1, hmp)),getUncertaintyExpr(e2, hmp)));
		Node *c2 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e1), (Node *)createConstInt(0)));
		Node *ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c1), createCaseOperator(c2)));
		Node *c3 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e2), (Node *)createConstInt(0)));
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(ret), createCaseOperator(c3)));
		return ret;
	}
	else if(strcmp(strToUpper(expr->name),"OR")==0){
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		List * argList = singleton(getUncertaintyExpr(e1, hmp));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		Node *ret = (Node *)createFunctionCall(LEAST_FUNC_NAME, argList);
		argList = singleton(createCaseOperator(e1));
		appendToTailOfList(argList, getUncertaintyExpr(e1, hmp));
		Node *temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, argList);
		argList = singleton(createCaseOperator(e2));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, argList);
		return ret;
	}
	else if(strcmp(strToUpper(expr->name),"AND")==0) {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		List * argList = singleton(getUncertaintyExpr(e1, hmp));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		Node *ret = (Node *)createFunctionCall(LEAST_FUNC_NAME, argList);
		argList = singleton(createReverseCaseOperator(e1));
		appendToTailOfList(argList, getUncertaintyExpr(e1, hmp));
		Node *temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, argList);
		argList = singleton(createReverseCaseOperator(e2));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		temp = (Node *)createFunctionCall(LEAST_FUNC_NAME, argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, argList);
		return ret;
	}
	else {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		return (Node *)createFunctionCall(LEAST_FUNC_NAME,
		        LIST_MAKE(getUncertaintyExpr(e1, hmp),getUncertaintyExpr(e2, hmp)));
	}
	return NULL;
}

static Node *RangeUBOp(Operator *expr, HashMap *hmp){
	if(!expr){
			return NULL;
		}
		if(strcmp(expr->name,UNCERT_FUNC_NAME)==0) {
			return (Node *)createConstInt(0);
		}
		if(strcmp(expr->name,"+")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			//Upper bound of addition is the sum of upper bounds
			Node *ret = (Node *)createOpExpr("+", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			INFO_LOG("REWRITE_RANGE_EXPR_PLUS: %s", nodeToString(ret));
			return ret;
		}
		if(strcmp(expr->name,"-")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			//Upper bound of subtraction is the ub-lb
			Node *ret = (Node *)createOpExpr("-", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"=")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *c1 = (Node *)createOpExpr("<=", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c2 = (Node *)createOpExpr(">=", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			Node *c3 = (Node *)createOpExpr("<=", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c4 = (Node *)createOpExpr(">=", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			Node *c5 = (Node *)createOpExpr(">=", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c6 = (Node *)createOpExpr("<=", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));

			Node *c12 = (Node *)createOpExpr("AND", appendToTailOfList(singleton(c1),c2));
			Node *c34 = (Node *)createOpExpr("AND", appendToTailOfList(singleton(c3),c4));
			Node *c56 = (Node *)createOpExpr("AND", appendToTailOfList(singleton(c5),c6));
			Node *c1234 = (Node *)createOpExpr("OR", appendToTailOfList(singleton(c12),c34));
			Node *ret = (Node *)createOpExpr("OR", appendToTailOfList(singleton(c1234),c56));
			return ret;
		}
		if(strcmp(expr->name,">")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(">", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"<")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("<", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"<=")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("<=", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"*")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *c1 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c2 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			Node *c3 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c4 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			Node *c12 = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c1), c2));
			Node *c34 = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c3), c4));
			Node *ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c12), c34));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),"OR")==0){
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("OR", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),"AND")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("AND", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		else {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			return (Node *)createFunctionCall(LEAST_FUNC_NAME,LIST_MAKE(getUncertaintyExpr(e1, hmp),getUncertaintyExpr(e2, hmp)));
		}
		return NULL;
}

static Node *RangeLBOp(Operator *expr, HashMap *hmp){
	if(!expr){
			return NULL;
		}
		if(strcmp(expr->name,UNCERT_FUNC_NAME)==0) {
			return (Node *)createConstInt(0);
		}
		if(strcmp(expr->name,"+")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			//Lower bound of addition is the sum of lower bounds
			Node *ret = (Node *)createOpExpr("+", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"-")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			//Lower bound of subtraction is the lb-ub
			Node *ret = (Node *)createOpExpr("-", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"*")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *c1 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c2 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			Node *c3 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node *c4 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			Node *c12 = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(c1), c2));
			Node *c34 = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(c3), c4));
			Node *ret = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(c12), c34));
			return ret;
		}
		if(strcmp(expr->name,"=")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *c1 = (Node *)createOpExpr("=", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e1, hmp)));
			Node *c2 = (Node *)createOpExpr("=", appendToTailOfList(singleton(getUBExpr(e2, hmp)),getLBExpr(e2, hmp)));
			Node *c12 = (Node *)createOpExpr("AND", appendToTailOfList(singleton(c1),c2));
			Node *ret = (Node *)createOpExpr("AND", appendToTailOfList(singleton(expr),c12));
			return ret;
		}
		if(strcmp(expr->name,">")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(">", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,">=")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(">=", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,"<")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("<", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),"OR")==0){
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("OR", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),"AND")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr("AND", appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		else {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			return (Node *)createFunctionCall(LEAST_FUNC_NAME,
			        LIST_MAKE(getUncertaintyExpr(e1, hmp),getUncertaintyExpr(e2, hmp)));
		}
		return NULL;
}

static QueryOperator *
rewrite_UncertSet(QueryOperator *op, boolean attrLevel)
{
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Set (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	// rewrite children first
	if (attrLevel)
	{
		rewriteUncert(OP_LCHILD(op));
		rewriteUncert(OP_RCHILD(op));
	}
	else
	{
		rewriteUncertTuple(OP_LCHILD(op));
		rewriteUncertTuple(OP_RCHILD(op));
	}

	HashMap * hmp = NEW_MAP(Node, Node);

	List *projExpr = getNormalAttrProjectionExprs(op);

	if (attrLevel)
	{
		FOREACH(Node, nd, projExpr){
			addUncertAttrToSchema(hmp, op, nd);
		}
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

	//TODO intersection is not handled correctly

	// set difference
	if(((SetOperator *)op)->setOpType == SETOP_DIFFERENCE)
	{
		List *projExpr = getNormalAttrProjectionExprs(op);
		projExpr = removeFromTail(projExpr);
		projExpr = appendToTailOfList(projExpr, createConstInt(0));

		QueryOperator *proj = (QueryOperator *)createProjectionOp(projExpr, op, NIL, getNormalAttrNames(op));
		switchSubtrees(op, proj);
		op->parents = singleton(proj);
		setStringProperty(proj, "UNCERT_MAPPING", (Node *) copyObject(hmp));

		return proj;
	}

	return op;
}

extern char *getAttrTwoString(char *in){
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", in);
	appendStringInfo(str, "%s", SELFJOIN_AFFIX);
	return backendifyIdentifier(str->data);
}

static QueryOperator *rewrite_RangeAggregation(QueryOperator *op){
	//TODO
	ASSERT(OP_LCHILD(op));

	//record original schema info
	List *proj_projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *pro_attrName = getNormalAttrNames(OP_LCHILD(op));
//	List *agg_attrName = getNormalAttrNames(OP_LCHILD(op));
	List *agg_projExpr = getNormalAttrProjectionExprs(op);

	List *aggr_groupby_list = ((AggregationOperator *)op)->groupBy;
	List *aggr_out_names = getNormalAttrNames(op);

	// rewrite child first
	QueryOperator *childop = rewriteRange(OP_LCHILD(op));

	if(0){
			childop = combineRowByBG(childop);
	}

	INFO_LOG("REWRITE-RANGE - Aggregation");
	HashMap * hmp = NEW_MAP(Node, Node);
	HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");

	//rewrite non-groupby case
	if(((AggregationOperator *)op)->groupBy == NIL){
		INFO_LOG("RANGE_Aggregation - No groupby");

		int ptr = 0;

		List *aggrl = copyList(((AggregationOperator *)op)->aggrs);

		//add projection
		FOREACH(Node, nd, aggrl){
			Node * funattr = getHeadOfListP(((FunctionCall *)nd)->args);
			ptr = ((AttributeReference *)funattr)->attrPosition;
			if(strcmp(((FunctionCall *)nd)->functionname, COUNT_FUNC_NAME)==0){
				getNthOfList(proj_projExpr,ptr)->data.ptr_value = getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS));
				proj_projExpr = appendToTailOfList(proj_projExpr, getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE)));
				proj_projExpr = appendToTailOfList(proj_projExpr, getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN)));
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(getNthOfListP(pro_attrName, ptr)));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(getNthOfListP(pro_attrName, ptr)));
			}
			//TODO upper bound can be optimized if there are certain tuples exist
			if(strcmp(((FunctionCall *)nd)->functionname, MIN_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				Node *funattrub = getHeadOfListP((List *)getMap(hmpIn, funattr));
				Node *funattrlb = getTailOfListP((List *)getMap(hmpIn, funattr));
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrub);
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrlb);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(getNthOfListP(pro_attrName, ptr)));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(getNthOfListP(pro_attrName, ptr)));
			}
			//TODO lower bound can be optimized if there are certain tuples exist
			if(strcmp(((FunctionCall *)nd)->functionname, MAX_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				Node *funattrub = getHeadOfListP((List *)getMap(hmpIn, funattr));
				Node *funattrlb = getTailOfListP((List *)getMap(hmpIn, funattr));
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrub);
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrlb);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(getNthOfListP(pro_attrName, ptr)));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(getNthOfListP(pro_attrName, ptr)));
			}
		}
		Node *cr = getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN));
		Node *br = getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS));
		Node *pr = getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE));
		proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(">",LIST_MAKE(cr, createConstInt(0)))));
		proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(">",LIST_MAKE(br, createConstInt(0)))));
		proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(">",LIST_MAKE(pr, createConstInt(0)))));
		pro_attrName = appendToTailOfList(pro_attrName,ROW_CERTAIN);
		pro_attrName = appendToTailOfList(pro_attrName,ROW_BESTGUESS);
		pro_attrName = appendToTailOfList(pro_attrName,ROW_POSSIBLE);
		QueryOperator *proj = (QueryOperator *)createProjectionOp(proj_projExpr, childop, singleton(op), pro_attrName);
		switchSubtrees(childop, proj);
		childop->parents = singleton(proj);
		op->inputs = singleton(proj);

		INFO_OP_LOG("Range Aggregation no groupby - add projection:", proj);

		//rewrite aggregation

		/* int pos = 0; */
		int aggpos = 0;

		FOREACH(Node, nd, aggrl){
			Node * funattr = getHeadOfListP(((FunctionCall *)nd)->args);
			INFO_LOG("%s", nodeToString(funattr));
			/* pos = ((AttributeReference *)funattr)->attrPosition; */
			if(strcmp(((FunctionCall *)nd)->functionname, COUNT_FUNC_NAME)==0){
				Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				getNthOfList(((AggregationOperator *)op)->aggrs,aggpos)->data.ptr_value = createFunctionCall(SUM_FUNC_NAME,singleton(funattr));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(SUM_FUNC_NAME,singleton(funattrub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(SUM_FUNC_NAME,singleton(funattrlb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, MIN_FUNC_NAME)==0){
				Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME,singleton(funattrub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MIN_FUNC_NAME,singleton(funattrlb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, MAX_FUNC_NAME)==0){
				Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME,singleton(funattrub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MIN_FUNC_NAME,singleton(funattrlb)));
			}
			aggpos++;
		}
		addRangeRowToSchema(hmp, op);
		Node* ct = (Node *)getAttrRefByName(proj,ROW_CERTAIN);
		Node* bg = (Node *)getAttrRefByName(proj,ROW_BESTGUESS);
		Node* ps = (Node *)getAttrRefByName(proj,ROW_POSSIBLE);
		((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(ct)));
		((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(bg)));
		((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(ps)));

		setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
//		INFO_LOG("%s", nodeToString(hmp));

		return op;
	}

	//rewrite groupby case

	INFO_LOG("RANGE_Aggregation - With groupby");

	//two branches for "selfjoin"

	QueryOperator *child = OP_LCHILD(op);
	QueryOperator *childdup = copyObject(child);

	//pre-aggregate group by attributes on left child

	List *aggrlist = NIL;
	List *gattrn = NIL;
	List *rattrn = NIL;

	FOREACH(Node, n, aggr_groupby_list){
		gattrn = appendToTailOfList(gattrn, ((AttributeReference *)n)->name);
		Node *aRef_ub = (Node *)getAttrRefByName(child, getUBString(((AttributeReference *)n)->name));
		Node *aRef_lb = (Node *)getAttrRefByName(child, getLBString(((AttributeReference *)n)->name));
		Node *ubaggr = (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(aRef_ub));
		Node *lbaggr = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(aRef_lb));
		aggrlist = appendToTailOfList(aggrlist, ubaggr);
		aggrlist = appendToTailOfList(aggrlist, lbaggr);
		rattrn = appendToTailOfList(rattrn, ((AttributeReference *)aRef_ub)->name);
		rattrn = appendToTailOfList(rattrn, ((AttributeReference *)aRef_lb)->name);
	}

	List *attrnames = concatTwoLists(rattrn, gattrn);

	QueryOperator *preaggr = (QueryOperator *)createAggregationOp(aggrlist, aggr_groupby_list, child, NIL, attrnames);
	switchSubtrees(child, preaggr);
	child->parents = singleton(preaggr);

	INFO_OP_LOG("Range Aggregation with groupby - left child aggregation:", preaggr);

	//do the join

	List *attrn1 = getNormalAttrNames(preaggr);
	List *attrn2 = NIL;

//	List *expr1 = getNormalAttrProjectionExprs(child);
//	List *expr2 = getNormalAttrProjectionExprs(childdup);

	//right attrs from join rename
	FOREACH(char, n, getNormalAttrNames(childdup)){
		attrn2 = appendToTailOfList(attrn2, getAttrTwoString(n));
	}
	List *attrJoin = concatTwoLists(attrn1,attrn2);
	INFO_LOG("%s", stringListToString(attrJoin));

	Node *joinExpr = NULL;

	//matching for join on all group by attributes
	FOREACH(Node, n, aggr_groupby_list){
//		Node *aRef1 = (Node *)getAttrRefByName(child, ((AttributeReference *)n)->name);
		Node *aRef1_ub = (Node *)getAttrRefByName(preaggr, getUBString(((AttributeReference *)n)->name));
		Node *aRef1_lb = (Node *)getAttrRefByName(preaggr, getLBString(((AttributeReference *)n)->name));
//		Node *aRef2 = (Node *)getAttrRefByName(childdup, ((AttributeReference *)n)->name);
//		((AttributeReference *)aRef2)->fromClauseItem =1;
		Node *aRef2_ub = (Node *)getAttrRefByName(childdup, getUBString(((AttributeReference *)n)->name));
		((AttributeReference *)aRef2_ub)->fromClauseItem = 1;
		Node *aRef2_lb = (Node *)getAttrRefByName(childdup, getLBString(((AttributeReference *)n)->name));
		((AttributeReference *)aRef2_lb)->fromClauseItem = 1;
		Node *refExprCase1 = (Node *)createOpExpr("AND" ,LIST_MAKE(createOpExpr("<=", LIST_MAKE(aRef1_lb,aRef2_lb)), createOpExpr(">=", LIST_MAKE(aRef1_ub,aRef2_lb))));
		Node *refExprCase2 = (Node *)createOpExpr("AND" ,LIST_MAKE(createOpExpr("<=", LIST_MAKE(aRef1_lb,aRef2_ub)), createOpExpr(">=", LIST_MAKE(aRef1_ub,aRef2_ub))));
		Node *refExprCase3 = (Node *)createOpExpr("AND" ,LIST_MAKE(createOpExpr(">=", LIST_MAKE(aRef1_lb,aRef2_lb)), createOpExpr("<=", LIST_MAKE(aRef1_ub,aRef2_ub))));
		Node *refExpr = (Node *)createOpExpr("OR", LIST_MAKE(refExprCase3, createOpExpr("OR", LIST_MAKE(refExprCase1,refExprCase2))));
		if(joinExpr == NULL){
			joinExpr = refExpr;
		}
		else {
			joinExpr = (Node *)createOpExpr("AND", LIST_MAKE(refExpr, joinExpr));
		}
	}

	QueryOperator *join = (QueryOperator *)createJoinOp(JOIN_INNER, joinExpr,LIST_MAKE(preaggr, childdup), NIL, attrJoin);
	switchSubtrees(preaggr, join);
	childdup->parents = singleton(join);
	preaggr->parents = singleton(join);

	INFO_OP_LOG("Range Aggregation with groupby - join:", join);

	//pre_projection
	List *projList = NIL;
	List *nameList = NIL;

	//a,ub_a,lb_a,b,ub_b,lb_b,...
	Node *cert_case = NULL;
	Node *bg_case = NULL;

	FOREACH(Node, n, aggr_groupby_list){
		char *fname = ((AttributeReference *)n)->name;
		char *fname_ub = getUBString(fname);
		char *fname_lb = getLBString(fname);
		projList = appendToTailOfList(projList, getAttrRefByName(join, fname));
		projList = appendToTailOfList(projList, getAttrRefByName(join, fname_ub));
		projList = appendToTailOfList(projList, getAttrRefByName(join, fname_lb));
		nameList = appendToTailOfList(nameList,fname);
		nameList = appendToTailOfList(nameList,fname_ub);
		nameList = appendToTailOfList(nameList,fname_lb);

		Node * cert_eq_1 = (Node *)createOpExpr("=", LIST_MAKE(getAttrRefByName(join, fname_ub), getAttrRefByName(join, fname_lb)));
		Node * cert_eq_2 = (Node *)createOpExpr("=", LIST_MAKE(getAttrRefByName(join, getAttrTwoString(fname_ub)), getAttrRefByName(join, getAttrTwoString(fname_lb))));
		Node * cert_eq_3 = (Node *)createOpExpr("=", LIST_MAKE(getAttrRefByName(join, fname_ub), getAttrRefByName(join, getAttrTwoString(fname_ub))));

		Node * bg_eq = (Node *)createOpExpr("=", LIST_MAKE(getAttrRefByName(join, fname), getAttrRefByName(join, getAttrTwoString(fname))));

		Node * cert_eq = (Node *)createOpExpr("AND", LIST_MAKE(cert_eq_3, createOpExpr("AND", LIST_MAKE(cert_eq_1, cert_eq_2))));
		if(cert_case == NULL){
			cert_case = cert_eq;
		} else {
			cert_case = (Node *)createOpExpr("AND", LIST_MAKE(cert_case, cert_eq));
		}
		if(bg_case == NULL){
			bg_case = bg_eq;
		} else {
			bg_case = (Node *)createOpExpr("AND", LIST_MAKE(bg_case, bg_eq));
		}
	}

	List *aggrl = copyList(((AggregationOperator *)op)->aggrs);

	//projection on aggregation attributes

	FOREACH(Node, n, aggrl){
		Node * funattr = getHeadOfListP(((FunctionCall *)n)->args);
		char * fname = getAttrTwoString(((AttributeReference *)funattr)->name);
		char * fname_ub = getUBString(fname);
		char * fname_lb = getLBString(fname);
		if(strcmp(((FunctionCall *)n)->functionname, COUNT_FUNC_NAME)==0){
			projList = appendToTailOfList(projList, getAttrRefByName(join, getAttrTwoString(ROW_BESTGUESS)));
			projList = appendToTailOfList(projList, getAttrRefByName(join, getAttrTwoString(ROW_POSSIBLE)));
			projList = appendToTailOfList(projList, getAttrRefByName(join, getAttrTwoString(ROW_CERTAIN)));
			nameList = appendToTailOfList(nameList,fname);
			nameList = appendToTailOfList(nameList,fname_ub);
			nameList = appendToTailOfList(nameList,fname_lb);
		}
		if(strcmp(((FunctionCall *)n)->functionname, MIN_FUNC_NAME)==0 || strcmp(((FunctionCall *)n)->functionname, MAX_FUNC_NAME)==0){
			projList = appendToTailOfList(projList, getAttrRefByName(join, fname));
			projList = appendToTailOfList(projList, getAttrRefByName(join, fname_ub));
			projList = appendToTailOfList(projList, getAttrRefByName(join, fname_lb));
			nameList = appendToTailOfList(nameList,fname);
			nameList = appendToTailOfList(nameList,fname_ub);
			nameList = appendToTailOfList(nameList,fname_lb);
		}
	}
	//TODO optimize annotations

	projList = appendToTailOfList(projList, createCaseOperator(cert_case));
	projList = appendToTailOfList(projList, createCaseOperator(bg_case));
	projList = appendToTailOfList(projList, getAttrRefByName(join, getAttrTwoString(ROW_POSSIBLE)));
	nameList = appendToTailOfList(nameList, ROW_CERTAIN);
	nameList = appendToTailOfList(nameList, ROW_BESTGUESS);
	nameList = appendToTailOfList(nameList, ROW_POSSIBLE);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(projList, join, NIL, nameList);
	switchSubtrees(join, proj);
	join->parents = singleton(proj);

	INFO_OP_LOG("Range Aggregation with groupby - projection:", proj);

	//	new aggregation groupby list
	List *new_groupby_list = NIL;
	List *new_aggr_List = NIL;
	List *namelist_gb = NIL;
	List *namelist_aggr = NIL;

	int pos = 0;

	FOREACH(Node, n, aggrl){
		Node * funattr = getHeadOfListP(((FunctionCall *)n)->args);
		char * origname = ((AttributeReference *)funattr)->name;
		char * fname = getAttrTwoString(origname);
		char * fname_ub = getUBString(fname);
		char * fname_lb = getLBString(fname);
		if(strcmp(((FunctionCall *)n)->functionname, COUNT_FUNC_NAME)==0){
			Node *bgfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname)));
			new_aggr_List = appendToTailOfList(new_aggr_List, bgfunc);
			Node *ubfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname_ub)));
			new_aggr_List = appendToTailOfList(new_aggr_List, ubfunc);
			Node *lbfunc = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, fname_lb)));
			new_aggr_List = appendToTailOfList(new_aggr_List, lbfunc);
			char *outname = (char *)getNthOfListP(aggr_out_names, pos);
			namelist_aggr = appendToTailOfList(namelist_aggr, outname);
			namelist_aggr = appendToTailOfList(namelist_aggr, getUBString(outname));
			namelist_aggr = appendToTailOfList(namelist_aggr, getLBString(outname));
		}
		if(strcmp(((FunctionCall *)n)->functionname, MIN_FUNC_NAME)==0){
			Node *bgfunc = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, fname)));
			new_aggr_List = appendToTailOfList(new_aggr_List, bgfunc);
			Node *ubfunc = (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(proj, fname_ub)));
			new_aggr_List = appendToTailOfList(new_aggr_List, ubfunc);
			Node *lbfunc = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, fname_lb)));
			new_aggr_List = appendToTailOfList(new_aggr_List, lbfunc);
			char *outname = (char *)getNthOfListP(aggr_out_names, pos);
			namelist_aggr = appendToTailOfList(namelist_aggr, outname);
			namelist_aggr = appendToTailOfList(namelist_aggr, getUBString(outname));
			namelist_aggr = appendToTailOfList(namelist_aggr, getLBString(outname));
		}
		if(strcmp(((FunctionCall *)n)->functionname, MAX_FUNC_NAME)==0){
			Node *bgfunc = (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(proj, fname)));
			new_aggr_List = appendToTailOfList(new_aggr_List, bgfunc);
			Node *ubfunc = (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(proj, fname_ub)));
			new_aggr_List = appendToTailOfList(new_aggr_List, ubfunc);
			Node *lbfunc = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, fname_lb)));
			new_aggr_List = appendToTailOfList(new_aggr_List, lbfunc);
			char *outname = (char *)getNthOfListP(aggr_out_names, pos);
			namelist_aggr = appendToTailOfList(namelist_aggr, outname);
			namelist_aggr = appendToTailOfList(namelist_aggr, getUBString(outname));
			namelist_aggr = appendToTailOfList(namelist_aggr, getLBString(outname));
		}
		pos++;
	}
	new_aggr_List = appendToTailOfList(new_aggr_List, (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, ROW_CERTAIN))));
	new_aggr_List = appendToTailOfList(new_aggr_List, (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, ROW_BESTGUESS))));
	new_aggr_List = appendToTailOfList(new_aggr_List, (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, ROW_POSSIBLE))));
	namelist_aggr = appendToTailOfList(namelist_aggr, ROW_CERTAIN);
	namelist_aggr = appendToTailOfList(namelist_aggr, ROW_BESTGUESS);
	namelist_aggr = appendToTailOfList(namelist_aggr, ROW_POSSIBLE);

	FOREACH(Node, n, aggr_groupby_list){
		new_groupby_list = appendToTailOfList(new_groupby_list, getAttrRefByName(proj, ((AttributeReference *)n)->name));
		new_groupby_list = appendToTailOfList(new_groupby_list, getAttrRefByName(proj, getUBString(((AttributeReference *)n)->name)));
		new_groupby_list = appendToTailOfList(new_groupby_list, getAttrRefByName(proj, getLBString(((AttributeReference *)n)->name)));
		char *outname = (char *)getNthOfListP(aggr_out_names, pos);
		namelist_gb = appendToTailOfList(namelist_gb, outname);
		namelist_gb = appendToTailOfList(namelist_gb, getUBString(outname));
		namelist_gb = appendToTailOfList(namelist_gb, getLBString(outname));
		pos++;
	}

	namelist_aggr = concatTwoLists(namelist_aggr, namelist_gb);

	QueryOperator *aggrop = (QueryOperator *)createAggregationOp(new_aggr_List, new_groupby_list, proj, NIL, namelist_aggr);
	switchSubtrees(proj, aggrop);
	proj->parents = singleton(aggrop);

	//fix datatypes
	pos = 0;
	FOREACH(Node, n, aggrl){
		DataType dt = typeOf(n);
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
	}
	FOREACH(Node, n, aggr_groupby_list){
		DataType dt = ((AttributeReference *)n)->attrType;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
	}
	((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = DT_INT;
	pos++;
	((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = DT_INT;
	pos++;
	((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = DT_INT;
	pos++;

	INFO_OP_LOG("Range Aggregation with groupby - rewrite aggregation:", aggrop);

	//Rearrange attribute position
//	INFO_LOG("%s", stringListToString(aggr_out_names));

	pos = 0;
	List *proj_bg = NIL;
	List *proj_bd = NIL;
	List *name_bg = NIL;

	FOREACH(Node, n, aggrl){
		INFO_LOG("%s", nodeToString(n));
		char *attrname = (char *)getNthOfListP(aggr_out_names, pos);
		char *ubname = getUBString(attrname);
		char *lbname = getLBString(attrname);
		proj_bg = appendToTailOfList(proj_bg, getAttrRefByName(aggrop, attrname));
		proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, ubname));
		proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, lbname));
		name_bg = appendToTailOfList(name_bg, attrname);
		pos++;
	}
	FOREACH(Node, n, aggr_groupby_list){
		INFO_LOG("%s", nodeToString(n));
		char *attrname = (char *)getNthOfListP(aggr_out_names, pos);
		char *ubname = getUBString(attrname);
		char *lbname = getLBString(attrname);
		proj_bg = appendToTailOfList(proj_bg, getAttrRefByName(aggrop, attrname));
		proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, ubname));
		proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, lbname));
		name_bg = appendToTailOfList(name_bg, attrname);
		pos++;
	}
	proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, ROW_CERTAIN));
	proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, ROW_BESTGUESS));
	proj_bd = appendToTailOfList(proj_bd, getAttrRefByName(aggrop, ROW_POSSIBLE));

	List *proj_list = concatTwoLists(proj_bg, proj_bd);

	QueryOperator *finalproj = (QueryOperator *)createProjectionOp(proj_list, aggrop, NIL, name_bg);
	switchSubtrees(aggrop, finalproj);
	aggrop->parents = singleton(finalproj);

	List *projlist = getNormalAttrProjectionExprs(finalproj);

	FOREACH(Node, n, projlist){
		addRangeAttrToSchema(hmp, finalproj, n);
	}

	addRangeRowToSchema(hmp, finalproj);

	setStringProperty(finalproj, "UNCERT_MAPPING", (Node *)hmp);

	switchSubtrees(op, finalproj);

	return finalproj;
}


static QueryOperator *
rewrite_UncertDuplicateRemoval(QueryOperator *op, boolean attrLevel)
{
	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - DuplicateRemoval");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//record original schema info
	List *projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *attrName = getNormalAttrNames(OP_LCHILD(op));

	// rewrite child first
	if (attrLevel)
	{
		rewriteUncert(OP_LCHILD(op));
	}
	else
	{
		rewriteUncertTuple(OP_LCHILD(op));
	}

	HashMap * hmp = NEW_MAP(Node, Node);
	HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");

	//create Aggregation
	Node *rUExpr = getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn);
	List *aggrList = NIL;
	List *uattrName = NIL;
	FOREACH(Node, nd, projExpr){
		Node *maxExpr =	(Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getUncertaintyExpr(nd, hmpIn)));
		aggrList = appendToTailOfList(aggrList, maxExpr);
		rUExpr = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(rUExpr), getUncertaintyExpr(nd, hmpIn)));
		uattrName = appendToTailOfList(uattrName, getUncertString(((AttributeReference *)nd)->name));
	}
	Node *maxExpr =	(Node *)createFunctionCall(MAX_FUNC_NAME, singleton(rUExpr));
	aggrList = appendToTailOfList(aggrList, maxExpr);
	uattrName = appendToTailOfList(uattrName, UNCERTAIN_FULL_ROW_ATTR);
	uattrName = concatTwoLists(uattrName, attrName);

	QueryOperator *aggrOp = (QueryOperator *)createAggregationOp(aggrList, projExpr, OP_LCHILD(op), NIL, uattrName);

	//create Projection
	List *projExpr2 = getNormalAttrProjectionExprs(aggrOp);

	List *exprInput = sublist(projExpr2, aggrList->length, projExpr2->length-1);
	projExpr2 = sublist(projExpr2, 0, aggrList->length-1);
	exprInput = concatTwoLists(exprInput, projExpr2);

	QueryOperator *projOp =	(QueryOperator *)createProjectionOp(exprInput, aggrOp, NIL, attrName);
	aggrOp->parents = appendToTailOfList(aggrOp->parents, projOp);
	switchSubtrees(op, projOp);

	projExpr = getNormalAttrProjectionExprs(projOp);

	FOREACH(Node, nd, projExpr){
		addUncertAttrToSchema(hmp, projOp, nd);
	}
	addUncertAttrToSchema(hmp, projOp, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(projOp, "UNCERT_MAPPING", (Node *)hmp);

	return projOp;
}

static QueryOperator *
rewrite_UncertAggregation(QueryOperator *op, boolean attrLevel)
{
	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Aggregation");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//record original schema info
	List *projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *attrName = getNormalAttrNames(OP_LCHILD(op));

	// rewrite child first
	if (attrLevel)
	{
		rewriteUncert(OP_LCHILD(op));
	}
	else
	{
		rewriteUncertTuple(OP_LCHILD(op));
	}

	// for tuple level all tuples are uncertain (any group may contain other possible tuples we do not know about changing the aggregation result)
	if(!attrLevel)
	{
		QueryOperator *proj;

		HashMap *hmp = NEW_MAP(Node, Node);

		proj = createProjOnAllAttrs(op);
		proj->inputs = singleton(op);
		switchSubtrees(op, proj);
		op->parents = singleton(proj);

		// add row uncertainty (all rows are uncertain)
		((ProjectionOperator *) proj)->projExprs = appendToTailOfList(((ProjectionOperator *) proj)->projExprs,
			createConstInt(0));
		addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
		setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);

		return proj;
	}


	HashMap* hmp = NEW_MAP(Node, Node);
	HashMap* hmpProj = NEW_MAP(Node, Node);
	HashMap* hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");

	// create projection before aggregation (for now we mark all aggregation results as uncertain)

	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExpr, OP_LCHILD(op), singleton(op), attrName);

	op->inputs = singleton(proj);
	OP_LCHILD(proj)->parents = singleton(proj);

	List *uncertExpr = NIL;
	Set *groupbyRef = makeNodeSetFromList(((AggregationOperator *)op)->groupBy);

	FOREACH(Node, nd, projExpr){
		addUncertAttrToSchema(hmpProj, proj, nd);
		if(hasSetElem(groupbyRef, nd)){
			uncertExpr = appendToTailOfList(uncertExpr, getUncertaintyExpr(nd, hmpIn));
		} else {
			uncertExpr = appendToTailOfList(uncertExpr, (Node *)createConstInt(0));
		}
	}
	addUncertAttrToSchema(hmpProj, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	Node *rU = getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn);
	FOREACH(Node, nd, ((AggregationOperator *)op)->groupBy){
		rU = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(rU), getUncertaintyExpr(nd, hmpIn)));
	}
	uncertExpr = appendToTailOfList(uncertExpr, rU);

	concatTwoLists(((ProjectionOperator *)proj)->projExprs, uncertExpr);
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmpProj);

	//add uncertainty to Aggregation

	List *attrUaggr = NIL;
	projExpr = getNormalAttrProjectionExprs(op);

	FOREACH(Node, nd, projExpr){
		addUncertAttrToSchema(hmp, op, nd);
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));

	int argl = LIST_LENGTH(((AggregationOperator *)op)->aggrs);
	int grpl = LIST_LENGTH(((AggregationOperator *)op)->groupBy);
	if(((AggregationOperator *)op)->groupBy){
		argl = LIST_LENGTH(((AggregationOperator *)op)->aggrs);
		grpl = LIST_LENGTH(((AggregationOperator *)op)->groupBy);
		op->schema->attrDefs = putMidListToEnd(op->schema->attrDefs, argl, argl+grpl);
	}

	FOREACH(Node, nd, ((AggregationOperator *)op)->aggrs){
		Node * tmp = (Node *)getHeadOfListP(((FunctionCall *)nd)->args);
		attrUaggr = appendToTailOfList(attrUaggr, (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getUncertaintyExpr(tmp, hmpProj))));
	}

	FOREACH(Node, nd, ((AggregationOperator *)op)->groupBy){
		attrUaggr = appendToTailOfList(attrUaggr, (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getUncertaintyExpr(nd, hmpProj))));
	}

	rU = getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn);
	attrUaggr = appendToTailOfList(attrUaggr, createFunctionCall(MAX_FUNC_NAME, singleton(rU)));
	concatTwoLists(((AggregationOperator *)op)->aggrs, attrUaggr);
	setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

	//create project to reorder the attributes when groupby.
	if(((AggregationOperator *)op)->groupBy){
		HashMap *hmp3 = NEW_MAP(Node, Node);
		projExpr = getNormalAttrProjectionExprs(op);
		attrName = getNormalAttrNames(op);
		int uncertl = LIST_LENGTH(projExpr)-argl-grpl;
		putMidListToEnd(projExpr, argl, argl+uncertl);
		putMidListToEnd(attrName, argl, argl+uncertl);
		attrName = sublist(attrName, 0, (attrName->length)-uncertl-1);

		QueryOperator *proj2 = (QueryOperator *)createProjectionOp(projExpr, op, NIL, attrName);
		switchSubtrees(op, proj2);
		op->parents = singleton(proj2);

		FOREACH(Node, nd, getNormalAttrProjectionExprs(proj2))
		{
			addUncertAttrToSchema(hmp3, proj2, nd);
		}
		addUncertAttrToSchema(hmp3, proj2, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
		setStringProperty(proj2, "UNCERT_MAPPING", (Node *)hmp3);
		return proj2;
	}
	return op;
}

static QueryOperator *
rewrite_UncertJoin(QueryOperator *op, boolean attrLevel)
{
	HashMap * hmp;

	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Join (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	List *nan = getNormalAttrNames(op);
	// rewrite children first
	if (attrLevel)
	{
		rewriteUncert(OP_LCHILD(op));
		rewriteUncert(OP_RCHILD(op));
	}
	else
	{
		rewriteUncertTuple(OP_LCHILD(op));
		rewriteUncertTuple(OP_RCHILD(op));
	}

	// tuple level uncertainty is much easier
	if(!attrLevel)
	{
		QueryOperator *proj;
	    // create join schema by concatinating child schemas
		op->schema->attrDefs = CONCAT_LISTS(
			copyObject(OP_LCHILD(op)->schema->attrDefs),
			copyObject(OP_RCHILD(op)->schema->attrDefs));
		char *rightUrowName = CONCAT_STRINGS(UNCERTAIN_FULL_ROW_ATTR, "_right");
		//disambiguiate the second row level uncertainty atttribute
		AttributeDef *urow2 = getTailOfListP(op->schema->attrDefs);
		urow2->attrName = rightUrowName;

		// uncertain attribute map
		hmp = NEW_MAP(Node, Node);

		// project on all normal attributes
		makeAttrNamesUnique(op);
		proj = createProjOnAttrsByName(op, nan, NIL);
		proj->inputs = singleton(op);
		switchSubtrees(op, proj);
		op->parents = singleton(proj);

		// add row uncertainty expression
		((ProjectionOperator *) proj)->projExprs = appendToTailOfList(((ProjectionOperator *) proj)->projExprs,
																	  createFunctionCall(LEAST_FUNC_NAME, LIST_MAKE(
																					   getAttrRefByName(op, UNCERTAIN_FULL_ROW_ATTR),
																					   getAttrRefByName(op, rightUrowName))));
		addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));


		// store mappings and switch trees
		setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

		return proj;
	}

	hmp =(HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
	List *dt1 = getDataTypes(OP_LCHILD(op)->schema);
	List *n1 = getAttrNames(OP_LCHILD(op)->schema);
	List *dt2 = getDataTypes(OP_RCHILD(op)->schema);
	List *n2 = getAttrNames(OP_RCHILD(op)->schema);
	int divider = dt1->length;

	List *jdt = concatTwoLists(dt1, dt2);
	List *jn = concatTwoLists(n1, n2);
	jn = removeFromTail(jn);
	op->schema = createSchemaFromLists(op->schema->name, jn, jdt);

	List *attrExpr = getNormalAttrProjectionExprs(op);
	List *expr2nd = sublist(attrExpr, divider, attrExpr->length-1);
	//INFO_LOG("exprlist: %s", nodeToString(expr2nd));
	List *uncertExpr2 = sublist(expr2nd, (expr2nd->length)/2, expr2nd->length-1);
	//INFO_LOG("UncertList: %s", nodeToString(uncertExpr2));


	FOREACH(Node, nd, uncertExpr2) {
		Node *keynd = (Node *)popHeadOfListP(expr2nd);
		((AttributeReference *)keynd)->outerLevelsUp = 0;
		ADD_TO_MAP(hmp, createNodeKeyValue(keynd, nd));
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference("R2"));
	setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

	//INFO_LOG("Join HashMap: \n%s", nodeToString(hmp));

	//add projection
	attrExpr = getNormalAttrProjectionExprs(op);

	List *listR = sublist(attrExpr, divider,attrExpr->length-1);
	Node * uRR = (Node *)getTailOfListP(listR);
	listR = removeFromTail(listR);
	List *uattrR = sublist(listR, (listR->length)/2, listR->length-1);
	List *attrR = sublist(listR, 0, (listR->length)/2-1);

	List *listl = sublist(attrExpr, 0, divider-1);
	Node *uRl = (Node *)getTailOfListP(listl);
	listl = removeFromTail(listl);
	List *uattrl = sublist(listl, (listl->length)/2, listl->length-1);
	List *attrl = sublist(listl, 0, (listl->length)/2-1);

 	Node *rU = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(uRl), uRR));

	List *projExprNew = concatTwoLists(concatTwoLists(attrl, attrR), concatTwoLists(uattrl, uattrR));

	if(((JoinOperator*)op)->joinType == JOIN_INNER && ((JoinOperator*)op)->cond){
		Node *outExpr = (Node *)copyObject(((JoinOperator*)op)->cond);
		outExpr = getOutputExprFromInput(outExpr, divider);
		rU = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(getUncertaintyExpr(outExpr, hmp)), rU));
		//INFO_LOG("orig outexpr: %s", nodeToString(((JoinOperator*)op)->cond));
		//INFO_LOG("uncert outexpr: %s", nodeToString(getUncertaintyExpr(outExpr, hmp)));
	}

	appendToTailOfList(projExprNew, rU);

 	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExprNew, op, NIL, nan);
 	switchSubtrees(op, proj);
	op->parents = singleton(proj);

	HashMap * hmp2 = NEW_MAP(Node, Node);
	List *attrExpr2 = getNormalAttrProjectionExprs(proj);
	FOREACH(Node, nd, attrExpr2){
		addUncertAttrToSchema(hmp2, proj, nd);
	}
	addUncertAttrToSchema(hmp2, proj, (Node *) createAttributeReference(UNCERTAIN_ROW_ATTR));

	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp2);

	return proj;

}

static QueryOperator *
rewrite_RangeJoin(QueryOperator *op){
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	INFO_LOG("REWRITE-RANGE - Join");
	INFO_LOG("Operator tree \n%s", nodeToString(op));

	int divider = getNormalAttrNames(OP_LCHILD(op))->length;

	List * nan = getNormalAttrNames(op);
	List * projattr = copyList(nan);
	List *nan2 = sublist(nan, divider, nan->length-1);
	List *nan1 = sublist(nan, 0, divider-1);
	List *nan1r = NIL;
	List *nan2r = NIL;
	FOREACH(char, nm, nan1){
		nan1r = appendToTailOfList(nan1r, getUBString(nm));
		nan1r = appendToTailOfList(nan1r, getLBString(nm));
	}
	FOREACH(char, nm, nan2){
		nan2r = appendToTailOfList(nan2r, getUBString(nm));
		nan2r = appendToTailOfList(nan2r, getLBString(nm));
	}
	List *range_row1 = LIST_MAKE(ROW_CERTAIN,ROW_BESTGUESS,ROW_POSSIBLE);
	List *range_row2 = LIST_MAKE(ROW_CERTAIN_TWO,ROW_BESTGUESS_TWO,ROW_POSSIBLE_TWO);
	List *n1 = concatTwoLists(nan1, nan1r);
	List *n2 = concatTwoLists(nan2, nan2r);
	n1 = concatTwoLists(n1, range_row1);
	n2 = concatTwoLists(n2, range_row2);
	n1 = concatTwoLists(n1, n2);
	INFO_LOG("n1 ====== %s", stringListToString(n1));

	// rewrite child first
	rewriteRange(OP_LCHILD(op));
	rewriteRange(OP_RCHILD(op));


	List *dt1 = getDataTypes(OP_LCHILD(op)->schema);
	List *dt2 = getDataTypes(OP_RCHILD(op)->schema);
	List *jdt = concatTwoLists(dt1, dt2);

	op->schema = createSchemaFromLists(op->schema->name, n1, jdt);


	//add projection

	//projection list

	int divider2 = divider*3+3;

	List *attrExpr = getNormalAttrProjectionExprs(op);
//	INFO_LOG("alllist:%d  %s", attrExpr->length,nodeToString(attrExpr));
	List *expr2 = sublist(attrExpr, divider2, attrExpr->length-1);
	List *rExpr2 = sublist(expr2, expr2->length-3, expr2->length-1);
	List *arExpr2 = sublist(expr2, ((expr2->length)-3)/3, expr2->length-4);
	expr2 = sublist(expr2, 0, ((expr2->length)-3)/3-1);
//	INFO_LOG("explist2: %s", nodeToString(expr2));
//	INFO_LOG("arexplist2: %s", nodeToString(arExpr2));
//	INFO_LOG("rexplist2: %s", nodeToString(rExpr2));
	List *expr1 = sublist(attrExpr, 0, divider2-1);
	List *rExpr1 = sublist(expr1, expr1->length-3, expr1->length-1);
	List *arExpr1 = sublist(expr1, ((expr1->length)-3)/3, expr1->length-4);
	expr1 = sublist(expr1, 0, ((expr1->length)-3)/3-1);
//	INFO_LOG("explist1: %s", nodeToString(expr1));
//	INFO_LOG("arexplist1: %s", nodeToString(arExpr1));
//	INFO_LOG("rexplist1: %s", nodeToString(rExpr1));

//	FOREACH(Node, nd, expr1) {
//		List *valnd = singleTon((Node *)popHeadOfListP(rExpr1));
//		appendToTailOfList(valnd,(Node *)popHeadOfListP(rExpr1));
//		((AttributeReference *)nd)->outerLevelsUp = 0;
//		ADD_TO_MAP(hmp, createNodeKeyValue(nd, valnd));
//	}
//	FOREACH(Node, nd, expr2) {
//		List *valnd2 = singleTon((Node *)popHeadOfListP(rExpr2));
//		appendToTailOfList(valnd2,(Node *)popHeadOfListP(rExpr2));
//		((AttributeReference *)nd)->outerLevelsUp = 0;
//		ADD_TO_MAP(hmp, createNodeKeyValue(nd, valnd2));
//	}

//	setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

	//INFO_LOG("Join HashMap: \n%s", nodeToString(hmp));

	Node *CT = (Node *)createOpExpr("*", appendToTailOfList(singleton((Node *)popHeadOfListP(rExpr1)), (Node *)popHeadOfListP(rExpr2)));
	Node *BG = (Node *)createOpExpr("*", appendToTailOfList(singleton((Node *)popHeadOfListP(rExpr1)), (Node *)popHeadOfListP(rExpr2)));
	Node *PS = (Node *)createOpExpr("*", appendToTailOfList(singleton((Node *)popHeadOfListP(rExpr1)), (Node *)popHeadOfListP(rExpr2)));

	List *projExprNew = concatTwoLists(concatTwoLists(expr1, expr2), concatTwoLists(arExpr1, arExpr2));
	appendToTailOfList(projExprNew,CT);
	appendToTailOfList(projExprNew,BG);
	appendToTailOfList(projExprNew,PS);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExprNew, op, NIL, projattr);
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

	HashMap * hmp2 = NEW_MAP(Node, Node);
	List *attrExpr2 = getNormalAttrProjectionExprs(proj);
	FOREACH(Node, nd, attrExpr2){
		addRangeAttrToSchema(hmp2, proj, nd);
	}
	addRangeRowToSchema(hmp2, proj);
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp2);

//	INFO_LOG("CT: %s", nodeToString(CT));
//	INFO_LOG("BG: %s", nodeToString(BG));
//	INFO_LOG("PS: %s", nodeToString(PS));

	//rewrite join condition
	if(((JoinOperator*)op)->joinType == JOIN_INNER && ((JoinOperator*)op)->cond){

		//hashmap for join condition rewriting into upper bound.
		//notice that upper bound need to replace original join condition in join operator.
		//And original join condition need to multiply with best guess in projection operator.
		//Rewrite join condition into lower bound to multiply with certain in projection operator.

		HashMap * hmpl = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
		HashMap * hmpr = (HashMap *)getStringProperty(OP_RCHILD(op), "UNCERT_MAPPING");
		HashMap * hmpl2 = copyObject(hmpl);
		HashMap * hmpr2 = copyObject(hmpr);

		FOREACH_HASH_KEY(Node, kv, hmpr){
//			INFO_LOG("hmpr: %s", nodeToString(kv));
			Node *val = getMap(hmpr, kv);
			((AttributeReference *)kv)->fromClauseItem = 1;
			if(val->type==T_List){
				((AttributeReference *)getHeadOfListP((List *)val))->fromClauseItem = 1;
				((AttributeReference *)getTailOfListP((List *)val))->fromClauseItem = 1;
				ADD_TO_MAP(hmpl, createNodeKeyValue((Node *)kv, (Node *)val));
			}
		}
		INFO_LOG("hmpl: %s", nodeToString(hmpl));
		FOREACH_HASH_KEY(Node, kv, hmpr2){
		//	INFO_LOG("hmpr: %s", nodeToString(kv));
			Node *val2 = getMap(hmpr2, kv);
			((AttributeReference *)kv)->attrPosition += divider2;
			if(val2->type==T_List){
				((AttributeReference *)getHeadOfListP((List *)val2))->attrPosition += divider2;
				((AttributeReference *)getTailOfListP((List *)val2))->attrPosition += divider2;
				ADD_TO_MAP(hmpl2, createNodeKeyValue((Node *)kv, (Node *)val2));
			}
		}
//		INFO_LOG("hmpl2: %s", nodeToString(hmpl2));

		Node *condExpr = (Node *)copyObject(((JoinOperator*)op)->cond);

//		INFO_LOG("input expr: %s", nodeToString(condExpr));
		Node *bgExpr = getOutputExprFromInput(copyObject(condExpr), divider2);
//		INFO_LOG("bg expr: %s", nodeToString(bgExpr));
		Node *ubExpr = getUBExpr(copyObject(condExpr), hmpl);
//		INFO_LOG("ub expr: %s", nodeToString(ubExpr));
		Node *lbExpr = getLBExpr(copyObject(bgExpr),hmpl2);
//		INFO_LOG("lb expr: %s", nodeToString(lbExpr));

		BG = (Node *)createOpExpr("*", appendToTailOfList(singleton(BG),(Node *)createCaseOperator(bgExpr)));
		CT = (Node *)createOpExpr("*", appendToTailOfList(singleton(CT),(Node *)createCaseOperator(lbExpr)));

		((JoinOperator*)op)->cond = ubExpr;

		projExprNew = sublist(projExprNew, 0, projExprNew->length-4);
		appendToTailOfList(projExprNew,CT);
		appendToTailOfList(projExprNew,BG);
		appendToTailOfList(projExprNew,PS);

  //		INFO_LOG("Projexpr with join: %s", nodeToString(projExprNew));

		((ProjectionOperator *)proj)->projExprs = projExprNew;
	}

	return proj;
}

static QueryOperator *
rewrite_UncertSelection(QueryOperator *op, boolean attrLevel)
{

	ASSERT(OP_LCHILD(op));

    INFO_LOG("REWRITE-UNCERT - Selection (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	// rewrite child first
	if (attrLevel)
	{
		rewriteUncert(OP_LCHILD(op));
	}
	else
	{
		rewriteUncertTuple(OP_LCHILD(op));
	}

    HashMap *hmp = NEW_MAP(Node, Node);

    //get child hashmap
    //HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
	if(attrLevel)
	{
		List *attrExpr = getNormalAttrProjectionExprs(op);
		FOREACH(Node, nd, attrExpr){
        	addUncertAttrToSchema(hmp, op, nd);
		}
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

    //create projection to calculate row uncertainty
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
    switchSubtrees(op, proj);
    op->parents = singleton(proj);

	if (attrLevel)
	{
		Node *uExpr = (Node *)getTailOfListP(((ProjectionOperator *)proj)->projExprs);
		((ProjectionOperator *)proj)->projExprs = removeFromTail(((ProjectionOperator *)proj)->projExprs);
		Node *newUR = (Node *)createFunctionCall(LEAST_FUNC_NAME, appendToTailOfList(singleton(uExpr), getUncertaintyExpr(((SelectionOperator *)op)->cond, hmp)));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, newUR);
	}
	else
	{
		//TODO check that is ok
	}
    setStringProperty(proj, "UNCERT_MAPPING", (Node *) copyObject(hmp));

	return proj;
}

static QueryOperator *
rewrite_RangeSelection(QueryOperator *op)
{

	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-RANGE - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
	// rewrite child first
    rewriteRange(OP_LCHILD(op));

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    //HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
    List *attrExpr = getNormalAttrProjectionExprs(op);
    FOREACH(Node, nd, attrExpr){
        	addRangeAttrToSchema(hmp, op, nd);
    }
    addRangeRowToSchema(hmp, op);
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
    //modify selection condition to possible
    Node *cond = ((SelectionOperator *)op)->cond;
    Node *ubCond = getUBExpr(((SelectionOperator *)op)->cond, hmp);
    Node *lbCond = getLBExpr(((SelectionOperator *)op)->cond, hmp);
    ((SelectionOperator *)op)->cond = ubCond;
    //create projection to calculate CERT and BG
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
    switchSubtrees(op, proj);
    op->parents = singleton(proj);
    //remove original CERT and BG
    Node *pos_cond = (Node *)getTailOfListP(((ProjectionOperator *)proj)->projExprs);
    ((ProjectionOperator *)proj)->projExprs = removeFromTail(((ProjectionOperator *)proj)->projExprs);
    ((ProjectionOperator *)proj)->projExprs = removeFromTail(((ProjectionOperator *)proj)->projExprs);
    ((ProjectionOperator *)proj)->projExprs = removeFromTail(((ProjectionOperator *)proj)->projExprs);
    //add row conditions
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, (Node *)createCaseOperator(lbCond));
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, (Node *)createCaseOperator(cond));
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs,pos_cond);
    setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
	return proj;
}

static QueryOperator *
rewrite_UncertProjection(QueryOperator *op, boolean attrLevel)
{
    ASSERT(OP_LCHILD(op));

    INFO_LOG("REWRITE-UNCERT - Projection (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
    //rewrite child first
	if (attrLevel)
	{
		rewriteUncert(OP_LCHILD(op));
	}
	else
	{
		rewriteUncertTuple(OP_LCHILD(op));
	}

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
    //INFO_LOG("HashMap: %s", nodeToString((Node *)hmpIn));
	if (attrLevel)
	{
		List *attrExpr = getNormalAttrProjectionExprs(op);
		List *uncertlist = NIL;
		FOREACH(Node, nd, attrExpr)
		{
			addUncertAttrToSchema(hmp, op, nd);
			Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs,LIST_LENGTH(uncertlist));
			Node *reExpr = getUncertaintyExpr(projexpr, hmpIn);
			uncertlist = appendToTailOfList(uncertlist, reExpr);
			replaceNode(((ProjectionOperator *)op)->projExprs, projexpr, removeUncertOpFromExpr(projexpr));
		}
		((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
	}

    addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
    INFO_LOG("List: %s", nodeToString(((ProjectionOperator *)op)->projExprs));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn));
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
    //INFO_LOG("ProjList: %s", nodeToString((Node *)(((ProjectionOperator *)op)->projExprs)));
    return op;
}

static QueryOperator *
rewrite_RangeProjection(QueryOperator *op)
{
    ASSERT(OP_LCHILD(op));

    INFO_LOG("REWRITE-RANGE - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
    //rewrite child first
    rewriteRange(OP_LCHILD(op));

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
    List *attrExpr = getNormalAttrProjectionExprs(op);
//    INFO_LOG("%s", nodeToString(((ProjectionOperator *)op)->projExprs));
//    INFO_LOG("%s", nodeToString(hmpIn));
    List *uncertlist = NIL;
    int ict = 0;
    FOREACH(Node, nd, attrExpr){
        addRangeAttrToSchema(hmp, op, nd);
        Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs,ict);
        Node *ubExpr = getUBExpr(projexpr, hmpIn);
        Node *lbExpr = getLBExpr(projexpr, hmpIn);
        ict ++;
        uncertlist = appendToTailOfList(uncertlist, ubExpr);
        uncertlist = appendToTailOfList(uncertlist, lbExpr);
        replaceNode(((ProjectionOperator *)op)->projExprs, projexpr, removeUncertOpFromExpr(projexpr));
    }
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
    addRangeRowToSchema(hmp, op);
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN)));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS)));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE)));
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
    INFO_LOG("ProjList: %s", nodeToString((Node *)(((ProjectionOperator *)op)->projExprs)));
    return op;
}

static QueryOperator *
rewrite_UncertTableAccess(QueryOperator *op, boolean attrLevel)
{
	INFO_LOG("REWRITE-UNCERT - TableAccess (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	HashMap * hmp = NEW_MAP(Node, Node);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

	if (attrLevel)
	{
		List *attrExpr = getNormalAttrProjectionExprs(op);
		FOREACH(Node, nd, attrExpr){
			addUncertAttrToSchema(hmp, proj, nd);
			appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
		}
	}

	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	if(HAS_STRING_PROP(op,PROP_HAS_UNCERT)){
		INFO_LOG("TableAccess - HAS_UNCERT");
		List *pexpr = getProvAttrProjectionExprs(op);
		//INFO_LOG("pexpr %s", nodeToString(pexpr));
		List *nexpr = getNormalAttrProjectionExprs(op);
		//INFO_LOG("nexpr %s", nodeToString(nexpr));
		((ProjectionOperator *)proj)->projExprs = concatTwoLists(nexpr, pexpr);
	} else {
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	}
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
	//INFO_LOG("HashMap: %s", nodeToString((Node *)hmp));
	return proj;
}

static QueryOperator *
rewrite_RangeTableAccess(QueryOperator *op)
{
	INFO_LOG("REWRITE-RANGE - TableAccess");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));
	HashMap * hmp = NEW_MAP(Node, Node);
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);
	List *attrExpr = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr){
		addRangeAttrToSchema(hmp, proj, nd);
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
	}
	addRangeRowToSchema(hmp, proj);
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
//	INFO_LOG("HashMap: %s", nodeToString((Node *)hmp));

	if(HAS_STRING_PROP(op,PROP_HAS_RANGE)){
		INFO_LOG("TableAccess - HAS_RANGE");
		List *pexpr = getProvAttrProjectionExprs(op);
		//INFO_LOG("pexpr %s", nodeToString(pexpr));
		List *nexpr = getNormalAttrProjectionExprs(op);
		//INFO_LOG("nexpr %s", nodeToString(nexpr));
		((ProjectionOperator *)proj)->projExprs = concatTwoLists(nexpr, pexpr);
	}

	return proj;
}

static void
addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef)
{
	addAttrToSchema(target, getUncertString(((AttributeReference *)aRef)->name), DT_INT);
	((AttributeReference *)aRef)->outerLevelsUp = 0;
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
}

static void
addRangeAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef)
{
	((AttributeReference *)aRef)->outerLevelsUp = 0;
	addAttrToSchema(target, getUBString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
	List *refs = singleton((Node *)getTailOfListP(getNormalAttrProjectionExprs(target)));
	addAttrToSchema(target, getLBString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
	appendToTailOfList(refs, (Node *)getTailOfListP(getNormalAttrProjectionExprs(target)));
	//Map each attribute to their upper&lower bounds list
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)refs));
}

static void
addRangeRowToSchema(HashMap *hmp, QueryOperator *target)
{
	addAttrToSchema(target, ROW_CERTAIN, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_CERTAIN), (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
	addAttrToSchema(target, ROW_BESTGUESS, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_BESTGUESS), (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
	addAttrToSchema(target, ROW_POSSIBLE, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_POSSIBLE), (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
}
