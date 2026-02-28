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
#include "operator_optimizer/optimizer_prop_inference.h"
#include "configuration/option.h"

/* type of uncertainty annotations we produce */
NEW_ENUM_WITH_ONLY_TO_STRING(UncertaintyType,
						UNCERTAIN_TUPLE_LEVEL,
						UNCERTAIN_ATTR_LEVEL,
						UNCERTAIN_ATTR_RANGES
	);

#define LOG_RESULT_UNCERT(mes,op) \
    do { \
        INFO_OP_LOG(mes,op); \
        DEBUG_NODE_BEATIFY_LOG(mes,op); \
    } while(0)

#define UNCERT_FUNC_NAME backendifyIdentifier("uncert")

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

#define UNCERT_MAPPING_PROP "UNCERT_MAPPING"

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
static Node *RangeUBFun(FunctionCall *expr, HashMap *hmp);
static Node *RangeLBFun(FunctionCall *expr, HashMap *hmp);
static Node *rangeLBCase(CaseExpr * expr, HashMap *hmp);
static Node *rangeUBCase(CaseExpr *expr, HashMap *hmp);
static Node *getUBExpr(Node *expr, HashMap *hmp);
static Node *getLBExpr(Node *expr, HashMap *hmp);
// static Node *getUBExprByName(Node *expr, HashMap *hmp, QueryOperator *op);
// static Node *getLBExprByName(Node *expr, HashMap *hmp, QueryOperator *op);
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
static QueryOperator *rewriteRangeLimit(QueryOperator *op);
static QueryOperator *rewrite_RangeTableAccess(QueryOperator *op);
static QueryOperator *rewrite_RangeProjection(QueryOperator *op);
static QueryOperator *rewrite_RangeSelection(QueryOperator *op);
static QueryOperator *rewrite_RangeJoin(QueryOperator *op);
static QueryOperator *rewrite_RangeJoinOptimized(QueryOperator *op);
static QueryOperator *rewrite_RangeAggregation(QueryOperator *op);
static QueryOperator *rewrite_RangeAggregation2(QueryOperator *op);

static QueryOperator *spliceToBG(QueryOperator *op);
static QueryOperator *spliceToBGAggr(QueryOperator *op);
static QueryOperator *spliceToPOS(QueryOperator *op, char *jattr);
static List *splitRanges(List *ranges);
static Node *getMedian(Node *ub, Node *lb);
static Constant* getStringMedian(Constant *ub, Constant *lb);

// mark uncertainty attributes as provenance attributes
static void markUncertAttrsAsProv(QueryOperator *op);

//Range query rewriting combiners
static QueryOperator *combineRowByBG(QueryOperator *op);
static QueryOperator *combineRowMinBg(QueryOperator *op);
static QueryOperator *combinePosToOne(QueryOperator *op);
static QueryOperator *compressPosRow(QueryOperator *op, int n, char *attr); //n: max number of tuples allowed. attr: attribute name the compress based on.

static List *getJoinAttrPair(Node *expr);

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
        ASSERT(HAS_STRING_PROP(op,PROP_XTABLE_PROB));
		rewrittenOp = rewrite_UncertXTable(op, UNCERTAIN_ATTR_LEVEL);
		return rewrittenOp;
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
		ASSERT(HAS_STRING_PROP(op,PROP_XTABLE_PROB));
		rewrittenOp = rewrite_UncertXTable(op, UNCERTAIN_TUPLE_LEVEL);
		return rewrittenOp;
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
	// FATAL_LOG("RANGE_REWRITE");
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
		ASSERT(HAS_STRING_PROP(op,PROP_XTABLE_PROB));
		rewrittenOp = rewrite_UncertXTable(op, UNCERTAIN_ATTR_RANGES);
		return rewrittenOp;
	}

	switch(op->type)
	{
	    case T_ProvenanceComputation:
	        rewrittenOp = rewriteRangeProvComp(op);
	        break;
	    case T_LimitOperator:
	    	rewrittenOp = rewriteRangeLimit(op);
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
			// if(HAS_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE)){
			// 	SET_STRING_PROP(rewrittenOp, PROP_STORE_POSSIBLE_TREE, (Node *)GET_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE));
			// }
			INFO_OP_LOG("Range Rewrite Selection:", rewrittenOp);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_RangeProjection(op);
			// if(HAS_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE)){
			// 	SET_STRING_PROP(rewrittenOp, PROP_STORE_POSSIBLE_TREE, (Node *)GET_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE));
			// }
			INFO_OP_LOG("Range Rewrite Projection:", rewrittenOp);
			break;
		case T_JoinOperator:
			rewrittenOp = rewrite_RangeJoin(op);
			INFO_OP_LOG("Range Rewrite Join:", rewrittenOp);
			break;
		case T_AggregationOperator:
			if(getBoolOption(RANGE_OPTIMIZE_AGG)){
				rewrittenOp = rewrite_RangeAggregation2(op);
			}
			else {
				rewrittenOp = rewrite_RangeAggregation(op);
			}
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
			return rangeUBCase((CaseExpr *) expr, hmp);
		}
		case T_FunctionCall: {
			return RangeUBFun((FunctionCall *) expr, hmp);
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
			return rangeLBCase((CaseExpr *) expr, hmp);
		}
		case T_FunctionCall: {
			return RangeLBFun((FunctionCall *) expr, hmp);
		}
		default: {
			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
		}
	}
	return NULL;
}

// Node *
// getUBExprByName(Node *expr, HashMap *hmp, QueryOperator *op)
// {
// 	switch(expr->type){
// 		case T_Constant: {
// 			return expr;
// 		}
// 		case T_AttributeReference: {
// //			INFO_LOG("AttrExprUB - %s", nodeToString(hmp));
// //			INFO_LOG("AttrExprUB - %s", nodeToString(expr));
// 			char *refname = ((AttributeReference *)expr)->name;
// 			Node * ret = (Node *)getAttrRefByName(op, getUBString(refname));
// 			((AttributeReference *)ret)->outerLevelsUp = 0;
// //			INFO_LOG("AttrExprUB - %s", ((AttributeReference *)ret)->name);
// 			return ret;
// 		}
// 		case T_Operator: {
// 			return RangeUBOp((Operator *) expr, hmp);
// 		}
// 		case T_CaseExpr: {
// 			return rangeUBCase((CaseExpr *) expr, hmp);
// 		}
// 		case T_FunctionCall: {
// 			return RangeUBFun((FunctionCall *) expr, hmp);
// 		}
// 		default: {
// 			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
// 		}
// 	}
// 	return NULL;
// }

// Node *
// getLBExprByName(Node *expr, HashMap *hmp, QueryOperator *op)
// {
// 	switch(expr->type){
// 		case T_Constant: {
// 			return expr;
// 		}
// 		case T_AttributeReference: {
// 			char *refname = ((AttributeReference *)expr)->name;
// 			Node * ret = (Node *)getAttrRefByName(op, getLBString(refname));
// 			((AttributeReference *)ret)->outerLevelsUp = 0;
// //			INFO_LOG("AttrExprLB - %s - %s", ((AttributeReference *)expr)->name, ((AttributeReference *)ret)->name);
// 			return ret;
// 		}
// 		case T_Operator: {
// 			return RangeLBOp((Operator *) expr, hmp);
// 		}
// 		case T_CaseExpr: {
// 			return rangeLBCase((CaseExpr *) expr, hmp);
// 		}
// 		case T_FunctionCall: {
// 			return RangeLBFun((FunctionCall *) expr, hmp);
// 		}
// 		default: {
// 			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
// 		}
// 	}
// 	return NULL;
// }

Node *
getUncertaintyExpr(Node *expr, HashMap *hmp)
{
	INFO_LOG("expression: %s ,  %p", exprToSQL(expr, NULL, FALSE), expr); //TODO deal with nested subqueries
	switch(expr->type){
		case T_Constant: {
			return (Node *)createConstInt(1);
		}
		case T_AttributeReference: {
			if(((AttributeReference *)expr)->outerLevelsUp == -1){
				((AttributeReference *)expr)->outerLevelsUp = 0;
			}
			Node * ret = getMap(hmp, expr);
			((AttributeReference *)ret)->outerLevelsUp = 0; //TODO why?
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
	HashMap * hmpIn = (HashMap *)getStringProperty(op, UNCERT_MAPPING_PROP);

	List *attrExpr = getProjExprsForAllAttrs(op);
	List *oldattrname = getQueryOperatorAttrNames(op);
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

	attrExpr = getProjExprsForAllAttrs(aggrop);
	List *projexpr1 = sublist(attrExpr, attrExpr->length-normalattrlen, attrExpr->length-1);
	INFO_LOG("projexpr1: %s", nodeToString(projexpr1));
	List *projexpr2 = sublist(attrExpr, 0, attrExpr->length-normalattrlen-1);
	INFO_LOG("projexpr2: %s", nodeToString(projexpr2));
	projexpr1 = concatTwoLists(projexpr1, projexpr2);

	oldattrname = sublist(oldattrname, 0, normalattrlen-1);

	QueryOperator *projop = (QueryOperator *)createProjectionOp(projexpr1, aggrop, NIL, oldattrname);
	switchSubtrees(aggrop, projop);
	aggrop->parents = singleton(projop);

	List *projExprs = getProjExprsForAllAttrs(projop);
	List *plist = sublist(projExprs, 0, normalattrlen-1);

	FOREACH(Node, nd, plist){
		addRangeAttrToSchema(hmp, projop, nd);
	}
	addRangeRowToSchema(hmp, projop);
	setStringProperty(projop, UNCERT_MAPPING_PROP, (Node *)hmp);

	return projop;
}

static QueryOperator *combineRowMinBg(QueryOperator *op) {

	HashMap * hmpIn = (HashMap *)getStringProperty(op, UNCERT_MAPPING_PROP);

	List *attrExpr = getProjExprsForAllAttrs(op);
//	List *oldattrname = getQueryOperatorAttrNames(op);
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

	Operator *notnull = createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(aggrop,ROW_POSSIBLE), createConstInt(0)));
	QueryOperator *selpos = (QueryOperator *)createSelectionOp((Node *)notnull, aggrop, NIL, normname);
	switchSubtrees(aggrop, selpos);
	aggrop->parents = singleton(selpos);

	setStringProperty(selpos, UNCERT_MAPPING_PROP, (Node *)hmpIn);

//	INFO_OP_LOG("aggr min bg:", selpos);
	return selpos;
}

static QueryOperator *combinePosToOne(QueryOperator *op) {
	HashMap * hmpIn = (HashMap *)getStringProperty(op, UNCERT_MAPPING_PROP);
	QueryOperator *opdup = copyObject(op);
	Operator *bgSel = createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(op,ROW_BESTGUESS), createConstInt(0)));
	Operator *posSel = createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(opdup,ROW_BESTGUESS), createConstInt(0)));
	List *attrnames = getQueryOperatorAttrNames(op);

	QueryOperator *bg = (QueryOperator *)createSelectionOp((Node *)bgSel, op, NIL, attrnames);
	switchSubtrees(op, bg);
	op->parents = singleton(bg);
	setStringProperty(bg, UNCERT_MAPPING_PROP, (Node *) copyObject(hmpIn));
	markUncertAttrsAsProv(bg);

	QueryOperator *pos = (QueryOperator *)createSelectionOp((Node *)posSel, opdup, NIL, attrnames);
	switchSubtrees(opdup, pos);
	opdup->parents = singleton(pos);
	setStringProperty(pos, UNCERT_MAPPING_PROP, (Node *) copyObject(hmpIn));

	INFO_OP_LOG("bg:", bg);
	INFO_OP_LOG("pos:", pos);
	//
	QueryOperator *onepos = combineRowMinBg(pos);

	QueryOperator *unionop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(bg, onepos), NIL, attrnames);
	switchSubtrees(bg, unionop);
	bg->parents = singleton(unionop);
	onepos->parents = singleton(unionop);

	setStringProperty(unionop, UNCERT_MAPPING_PROP, (Node *) copyObject(hmpIn));
	markUncertAttrsAsProv(unionop);

	return unionop;
}

static QueryOperator *
compressPosRow(QueryOperator *op, int n, char *attr)
{
	List *attrnames = getQueryOperatorAttrNames(op);
	HashMap * mmpro = (HashMap *)getStringProperty(op, PROP_STORE_MIN_MAX);
	// INFO_LOG("property: %s", nodeToString(mmpro));
	Node *max = MAP_GET_STRING_ENTRY((HashMap *)MAP_GET_STRING_ENTRY(mmpro,attr)->value, "MAX")->value;
	Node *min = MAP_GET_STRING_ENTRY((HashMap *)MAP_GET_STRING_ENTRY(mmpro,attr)->value, "MIN")->value;
	INFO_LOG("max for %s is: %s", attr, nodeToString(max));
	INFO_LOG("min for %s is: %s", attr, nodeToString(min));
	List *range = LIST_MAKE(max,min);
	while(n>0){
		range = splitRanges(range);
		// INFO_LOG("range for %s is: %s", attr, nodeToString(range));
		n--;
	}
	List *projl = NIL;
	FOREACH(char, n, attrnames){
		if(strcmp(n,attr)==0 && range->length >=3){
			List *cwhens = NIL;
			range = sublist(range, 1, range->length-1);
			INFO_LOG("divider range for %s is: %s", attr, nodeToString(range));
			AttributeReference *tattr = getAttrRefByName(op, n);
			Node *last = NULL;
			FOREACH(Node, nd, range){
				if(!last){
					last = nd;
					Node *whenexpr = (Node *)createOpExpr(OPNAME_GE, LIST_MAKE(tattr, nd));
					cwhens = appendToTailOfList(cwhens, createCaseWhen(whenexpr, nd));
				}
				else{
					Node *lbexpr = (Node *)createOpExpr(OPNAME_LT, LIST_MAKE(tattr, last));
					INFO_LOG("lbexpr: %s",nodeToString(lbexpr));
					Node *ubexpr = (Node *)createOpExpr(OPNAME_GE, LIST_MAKE(tattr, nd));
					INFO_LOG("ubexpr: %s",nodeToString(lbexpr));
					Node *whenexpr = (Node *)AND_EXPRS(lbexpr, ubexpr);
					cwhens = appendToTailOfList(cwhens, createCaseWhen(whenexpr, nd));
					last = nd;
				}
			}
			projl = appendToTailOfList(projl, (Node *)createCaseExpr(NULL, cwhens, last));
		}
		else {
			projl = appendToTailOfList(projl, getAttrRefByName(op, n));
		}
	}
	// INFO_LOG("projl: %s", nodeToString(projl));
	QueryOperator *projop = (QueryOperator *)createProjectionOp(projl,op,NIL,getQueryOperatorAttrNames(op));
	switchSubtrees(op, projop);
	op->parents = singleton(projop);
	setStringProperty(projop, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(op, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(projop);

	INFO_OP_LOG("1st projection:", projop);

	int div = (attrnames->length-3)/3;
	List *normattr = sublist(attrnames, 0, div-1);
	INFO_LOG("normattr: %s",stringListToString(normattr));

	//compress group by bg of target attr
	List *groupby = singleton(getAttrRefByName(projop, attr));
	List *aggrs = NIL;
	List *aname = NIL;
	FOREACH(char, n, normattr){
		INFO_LOG("attrname: %s", n);
		if(strcmp(n,attr)==0){
			aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(MAX_FUNC_NAME,singleton(getAttrRefByName(projop,getUBString(n)))));
			aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(MIN_FUNC_NAME,singleton(getAttrRefByName(projop,getLBString(n)))));
			aname = appendToTailOfList(aname, getUBString(n));
			aname = appendToTailOfList(aname, getLBString(n));
		}
		else {
			aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(MIN_FUNC_NAME,singleton(getAttrRefByName(projop,n))));
			aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(MAX_FUNC_NAME,singleton(getAttrRefByName(projop,getUBString(n)))));
			aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(MIN_FUNC_NAME,singleton(getAttrRefByName(projop,getLBString(n)))));
			aname = appendToTailOfList(aname, n);
			aname = appendToTailOfList(aname, getUBString(n));
			aname = appendToTailOfList(aname, getLBString(n));
		}
	}
	aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(getAttrRefByName(projop,ROW_CERTAIN))));
	aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(getAttrRefByName(projop,ROW_BESTGUESS))));
	aggrs = appendToTailOfList(aggrs, (Node *)createFunctionCall(SUM_FUNC_NAME,singleton(getAttrRefByName(projop,ROW_POSSIBLE))));
	aname = appendToTailOfList(aname, ROW_CERTAIN);
	aname = appendToTailOfList(aname, ROW_BESTGUESS);
	aname = appendToTailOfList(aname, ROW_POSSIBLE);
	aname = appendToTailOfList(aname, attr);
	QueryOperator *aggrop = (QueryOperator *)createAggregationOp(aggrs, groupby, projop, NIL, aname);
	switchSubtrees(projop, aggrop);
	projop->parents = singleton(aggrop);
	setStringProperty(aggrop, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(op, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(aggrop);


	FOREACH(AttributeDef, nd, aggrop->schema->attrDefs){
		if(nd->dataType==DT_LONG){
			nd->dataType=DT_INT;
		}
	}

	INFO_OP_LOG("aggregation:", aggrop);

	List *rplist = NIL;
	List *nplist = NIL;
	FOREACH(char, n, normattr){
		nplist = appendToTailOfList(nplist, getAttrRefByName(aggrop,n));
		rplist = appendToTailOfList(rplist, getAttrRefByName(aggrop,getUBString(n)));
		rplist = appendToTailOfList(rplist, getAttrRefByName(aggrop,getLBString(n)));
	}
	rplist = appendToTailOfList(rplist, getAttrRefByName(aggrop,ROW_CERTAIN));
	rplist = appendToTailOfList(rplist, getAttrRefByName(aggrop,ROW_BESTGUESS));
	rplist = appendToTailOfList(rplist, getAttrRefByName(aggrop,ROW_POSSIBLE));

	QueryOperator *finalproj = (QueryOperator *)createProjectionOp(CONCAT_LISTS(nplist,rplist),aggrop,NIL,getQueryOperatorAttrNames(op));
	switchSubtrees(aggrop, finalproj);
	aggrop->parents = singleton(finalproj);
	setStringProperty(finalproj, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(op, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(finalproj);
	//projection

	INFO_OP_LOG("last projection:", finalproj);

	setStringProperty(finalproj, PROP_STORE_MIN_MAX, (Node *)mmpro);
	SET_BOOL_STRING_PROP(finalproj, PROP_STORE_MIN_MAX_DONE);

	return finalproj;
}

//given a list of bounds, divide each bounds into two
static List *splitRanges(List *ranges){
	INFO_LOG("input ranges %s", nodeToString(ranges));
	if(ranges->length<=1){
		return ranges;
	}
	List *nb = NIL;
	Node *first = NULL;
	FOREACH(Node, nd, ranges){
		if(!first){
			first = nd;
			nb = appendToTailOfList(nb,first);
		}
		else {
                  nb = appendToTailOfList(nb, getMedian(first, nd));
                  nb = appendToTailOfList(nb, nd);
                  first = nd;
		}
	}
	INFO_LOG("output ranges %s", nodeToString(nb));
	return nb;
}

#define STRING_MEDIAN_VALUE "zzzzz"

static Node *
getMedian(Node *ub, Node *lb)
{
	ASSERT(isA(ub,Constant) && isA(lb, Constant));
	Constant *u = (Constant *) ub;
	Constant *l = (Constant *) lb;
	ASSERT(u->constType == l->constType);

	if(l->isNull && u->isNull)
	{
		switch(l->type)
		{
		case DT_INT:
			return (Node *) createConstInt(0);
		case DT_FLOAT:
			return (Node *) createConstFloat(0);
		case DT_LONG:
			return (Node *) createConstLong(0);
		case DT_STRING:
			return (Node *) createConstString(strdup(STRING_MEDIAN_VALUE));
		case DT_VARCHAR2:
			return (Node *) createConstString(strdup(STRING_MEDIAN_VALUE));
		default:
			return NULL;
		}
	}
	else if(l->isNull || u->isNull)
	{
		Constant *nonNull = l->isNull ? u : l;

		switch(nonNull->type)
		{
		case DT_INT:
			return (Node *) createConstInt(INT_VALUE(nonNull) / 2);
		case DT_LONG:
			return (Node *) createConstInt(LONG_VALUE(nonNull) / 2);
		case DT_FLOAT:
			return (Node *) createConstFloat(FLOAT_VALUE(nonNull) / 2.0);
		case DT_STRING:
		case DT_VARCHAR2:
		{
			return (Node *) getStringMedian(nonNull, createConstString(strdup(STRING_MEDIAN_VALUE)));
		}
		default:
			return NULL;
		}
	}

	switch(u->constType)
	{
	case DT_INT:
	{
		int median = (INT_VALUE(ub) + INT_VALUE(lb)) / 2;
		if (INT_VALUE(ub) - INT_VALUE(lb) == 1)
		{
			median = INT_VALUE(ub);
		}
		// INFO_LOG("Median of %f and %f is: %f", INT_VALUE(ub), INT_VALUE(lb),median);
		return (Node *)createConstInt(median);
	}
	case DT_FLOAT:
	{
		double median = (FLOAT_VALUE(ub) + FLOAT_VALUE(lb)) / 2.0;
		// INFO_LOG("Median of %f and %f is: %f", FLOAT_VALUE(ub), FLOAT_VALUE(lb),median);
		return (Node *)createConstFloat(median);
	}
	case DT_LONG:
	{
		gprom_long_t median = (LONG_VALUE(ub) + LONG_VALUE(lb)) / 2.0;
		// INFO_LOG("Median of %f and %f is: %f", LONG_VALUE(ub), LONG_VALUE(lb),median);
		return (Node *)createConstLong(median);
	}
	case DT_STRING:
	case DT_VARCHAR2:
		return (Node *)getStringMedian((Constant *)ub, (Constant *)lb);
	default:
		return ub;
	}
}

static char getCharMedian(char l, char r){
	char *e = strchr(ALPHABET, l);
	int indexl = (int)(e - ALPHABET);
	e = strchr(ALPHABET, r);
	int indexr = (int)(e - ALPHABET);
	int pos = (int)(indexl+indexr)/2;
	return ALPHABET[pos];
}

static Constant*
getStringMedian(Constant *ub, Constant *lb)
{
	ASSERT(ub->constType == DT_STRING || ub->constType == DT_VARCHAR2);
	char *ubs = STRING_VALUE(ub);
	char *lbs = STRING_VALUE(lb);
	int l1 = strlen(ubs);
	int l2 = strlen(lbs);
	INFO_LOG("%d", MAX(l1,l2));
	char val1 = 0;
	char val2 = 0;
	char *res = (char *)MALLOC((MAX(l1,l2)+1)*sizeof(char));
	int i = 0;
	for (i = 0; i < MAX(l1,l2); i++)
    {
        if(i<l1){
        	val1 = (char)ubs[i];
        }
        if(i<l2){
        	val2 = (char)lbs[i];
        }
        res[i] = getCharMedian(val1,val2);
        INFO_LOG("for position %d, it is %c + %c to median %c.", i, val1, val2,res[i]);
        val1=0;
        val2=0;
    }
    res[i+1] = '\0';

	if(ub->constType == DT_STRING)
	{
		INFO_LOG("median for %s and %s is: %s.", ubs,lbs,nodeToString(createConstString(res)));
		return createConstString(res);
	}
	else
	{
		Constant *result = makeConst(DT_VARCHAR2);

		result->value = res;

		return result;
	}
}


static QueryOperator *
rewrite_RangeTIP(QueryOperator *op)
{
	INFO_LOG("rewriteRangeTIP - %s\n", UncertaintyTypeToString(UNCERTAIN_ATTR_RANGES));

	char * TIPName = STRING_VALUE(GET_STRING_PROP(op,PROP_TIP_ATTR));

//	int pos = getAttrRefByName(op,TIPName)->attrPosition;

	Operator *bgcond = createOpExpr(OPNAME_GE, LIST_MAKE(getAttrRefByName(op,TIPName), createConstFloat(0.5)));
	Operator *certcond = createOpExpr(OPNAME_GE, LIST_MAKE(getAttrRefByName(op,TIPName), createConstFloat(1.0)));
	Operator *poscond = createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(op,TIPName), createConstFloat(0.0)));

	HashMap *hmp = NEW_MAP(Node, Node);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(((ProjectionOperator *)createProjOnAllAttrs(op))->projExprs, op, NIL, getAttrNames(op->schema));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

//	INFO_LOG("Range_TIP_proj: %s", nodeToString(((ProjectionOperator *)proj)->projExprs));

	List *attrExpr = getProjExprsForAllAttrs(op);
	FOREACH(Node, nd, attrExpr){
		addRangeAttrToSchema(hmp, proj, nd);
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
	}
	addRangeRowToSchema(hmp, proj);
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator((Node *)certcond));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator((Node *)bgcond));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator((Node *)poscond));
	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

//	INFO_LOG("Range_TIP_HMP: %s", nodeToString(((ProjectionOperator *)proj)->projExprs));
//	INFO_LOG("Range_TIP_HMP: %s", nodeToString(hmp));

	LOG_RESULT_UNCERT("UNCERTAIN RANGE: Rewritten Operator tree [TIP TABLE]", op);

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
	Operator *ltequal = createOpExpr(OPNAME_LE,LIST_MAKE(createConstFloat(0.5),getAttrRefByName(op,TIPName)));

	//create select op with the condition
	QueryOperator *selec = (QueryOperator *)createSelectionOp((Node *)ltequal, op, NIL, getAttrNames(op->schema));

	//Uncert attributes Hashmap
	HashMap * hmp = NEW_MAP(Node, Node);

	//create proj operator on the selection operator results
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(selec), selec, NIL, getQueryOperatorAttrNames(selec));

	//switching subtrees
	switchSubtrees(op, proj);

	//parent pointers for select operator
	selec->parents = singleton(proj);

	//parent pointer for op
	op->parents = singleton(selec);

	if (typ == UNCERTAIN_ATTR_LEVEL)
	{
		//Final projection? U_A.... U_R
		List *attrExpr = getProjExprsForAllAttrs(op);
		FOREACH(Node, nd, attrExpr)
		{
			//Add U_nd->name to the schema, with data type int
			addUncertAttrToSchema(hmp, proj, nd);
			//Set the values of U_nd->name to 1
			appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
		}
	}

	//Create operator expression when P==1
	Node *TIPIsOne = (Node *)createOpExpr(OPNAME_EQ,LIST_MAKE(createConstFloat(1),getAttrRefByName(op,TIPName)));

	//Add U_R to the schema with data type int
	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	//Set the values of U_R using a CASE WHEN TIPisOne is true
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(TIPIsOne));

	//Update string property
	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

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
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(op), op, NIL, getQueryOperatorAttrNames(op));

	//switching subtrees
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

	//Create operator expression when an entry is NULL
	//Node *entryIsNull = (Node *)createOpExpr("is",LIST_MAKE(getQueryOperatorAttrNames(op), (Node *) createConstString("NULL")));

	//Final projection? U_A.... U_R
	List *attrExpr = getProjExprsForAllAttrs(op);
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

	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

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
	/* List *attrExpr1 = getProjExprsForAllAttrs(op); */
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
	List *attrExpr2 = getProjExprsForAllAttrs(op);
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
	List *attrExpr3 = getProjExprsForAllAttrs(op);
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
		selec1Cond = createOpExpr(OPNAME_EQ,LIST_MAKE(getAttrRefByName(prevWOp, maxProbName), copyObject(probRef)));
	}
	// UADB, only show rows that are best guess (maximal probability alternative unless not including an alternative has the highest probability)
	else if (typ == UNCERTAIN_TUPLE_LEVEL || typ == UNCERTAIN_ATTR_LEVEL)
	{
		Operator *oneMinusSum = createOpExpr("-",LIST_MAKE(createConstInt(1),getAttrRefByName(prevWOp, sumProbName)));
		Operator *firstParam = createOpExpr(OPNAME_GT,LIST_MAKE(getAttrRefByName(prevWOp, maxProbName),oneMinusSum));

		Operator *secondParam = createOpExpr(OPNAME_EQ,LIST_MAKE(getAttrRefByName(prevWOp, maxProbName), copyObject(probRef)));
		selec1Cond = createOpExpr(OPNAME_AND, LIST_MAKE(firstParam,secondParam));
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
	Operator *countEqualsOne = createOpExpr(OPNAME_EQ,LIST_MAKE(createConstInt(1),getAttrRefByName(rowNumberByIdWOp, rowNumByIdName)));
	QueryOperator *selecRowNumberIsOne = (QueryOperator *)createSelectionOp((Node *)countEqualsOne, rowNumberByIdWOp, NIL, getAttrNames(rowNumberByIdWOp->schema));
	rowNumberByIdWOp->parents = singleton(selecRowNumberIsOne);

	/* Final Projection */
	List *normalAttrNames = getQueryOperatorAttrNames(op);
	List *normalProjExprs = getProjExprsForAttrNames(op, normalAttrNames);
	QueryOperator *proj = (QueryOperator *)createProjectionOp(normalProjExprs, selecRowNumberIsOne, NIL, normalAttrNames);
	selecRowNumberIsOne->parents = singleton(proj);

	/* either add uncertain attributes or add range bounds */
	List *attrExpr4 = getProjExprsForAllAttrs(op);
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
							   createCaseOperator((Node *)createOpExpr(OPNAME_EQ,LIST_MAKE(createConstFloat(1),getAttrRefByName(selecRowNumberIsOne,maxProbName)))));
		}
	}

	//Condition for U_R
	if (typ == UNCERTAIN_ATTR_RANGES)
	{
		addRangeRowToSchema(hmp, proj);

		// certain = sum probability is 1.0 (there is only one alternative with probability of one
		Node *sumProbIsOne = (Node *)createOpExpr(OPNAME_EQ,LIST_MAKE(createConstFloat(1),getAttrRefByName(selecRowNumberIsOne,sumProbName)));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(sumProbIsOne));

		// best guess (max probability of option is larger equals to 0.5
		Node *maxProbLargerOneMinusSum = (Node *)createOpExpr(OPNAME_LE,LIST_MAKE(
															   createOpExpr("-", LIST_MAKE(createConstFloat(1), getAttrRefByName(selecRowNumberIsOne, sumProbName))),
															   getAttrRefByName(selecRowNumberIsOne,maxProbName)));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(maxProbLargerOneMinusSum));

		// possible (always 1)
		appendToTailOfList(((ProjectionOperator *) proj)->projExprs, createConstInt(1));
	}
	else
	{
		Node *sumProbIsOne = (Node *)createOpExpr(OPNAME_EQ,LIST_MAKE(createConstFloat(1),getAttrRefByName(selecRowNumberIsOne,sumProbName)));
		//Add U_R to the schema with data type int
		addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createCaseOperator(sumProbIsOne));
	}

	switchSubtrees(op, proj);
	op->parents = singleton(maxProbWOp);

	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

	LOG_RESULT_UNCERT(specializeTemplate("$1: Rewritten Operator tree [XTABLE]", singleton(UncertaintyTypeToString(typ))),
			   proj);

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
    DEBUG_NODE_BEATIFY_LOG("rewritten query root for uncertainty is\n:", top);

    return top;
}

static QueryOperator *
rewriteRangeProvComp(QueryOperator *op)
{
    ASSERT(LIST_LENGTH(op->inputs) == 1);
    QueryOperator *top = getHeadOfListP(op->inputs);

    top = rewriteRange(top);

  //   union if pos are spliced from bg
    if (HAS_STRING_PROP(top, PROP_STORE_POSSIBLE_TREE)){
    	INFO_LOG("[PROV] MERGING BG AND POS: ");
    	QueryOperator *bgop = top;
    	QueryOperator *posop = (QueryOperator *)GET_STRING_PROP(bgop, PROP_STORE_POSSIBLE_TREE);
    	INFO_OP_LOG("[PROV] bgop: ",bgop);
    	INFO_OP_LOG("[PROV] posop: ",posop);
    	// top = bgop;
    	QueryOperator *unionop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(bgop, posop), NIL, getQueryOperatorAttrNames(bgop));
		bgop->parents = singleton(unionop);
		posop->parents = singleton(unionop);
	    setStringProperty(unionop, UNCERT_MAPPING_PROP, copyObject(GET_STRING_PROP(bgop,UNCERT_MAPPING_PROP)));
		markUncertAttrsAsProv(unionop);
		// top = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(bgop), bgop, NIL, getQueryOperatorAttrNames(bgop));
		// bgop->parents = singleton(top);
		// unionop->parents = singleton(top);
		top = unionop;
    }

    // make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    disambiguiteAttrNames((Node *) top, done);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root for range is:\n", top);

    return top;
}

static QueryOperator *rewriteRangeLimit(QueryOperator *op){
	ASSERT(OP_LCHILD(op));

    // push down min max attr property if there are any
	if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
	{
		Set *dependency = (Set *)getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS);
		INFO_LOG("[Limit] Pushing minmax prop attr %s to child as: %s", nodeToString(dependency), nodeToString(dependency));
		// setStringProperty(op, PROP_STORE_MIN_MAX_ATTRS, (Node *)newd);
		setStringProperty(OP_LCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)copyObject(dependency));
	}

	//rewrite child first
	QueryOperator *child = rewriteRange(OP_LCHILD(op));

	List *attrExpr = getNormalAttrProjectionExprs(child);

	HashMap * hmp = NEW_MAP(Node, Node);

	FOREACH(Node, nd, attrExpr){
        addRangeAttrToSchema(hmp, op, nd);
    }
    addRangeRowToSchema(hmp, op);
    setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);
	markUncertAttrsAsProv(op);

	return op;
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
RangeUBFun(FunctionCall *expr, HashMap *hmp)
{
	Node *result = NULL;
	List *ubargs = NIL;
	FOREACH(Node,sub,expr->args)
    {
	    ubargs = appendToTailOfList(ubargs, getUBExpr(sub, hmp));
    }
	result = (Node *)createFunctionCall(expr->functionname, ubargs);
	return result;
}

static Node *
RangeLBFun(FunctionCall *expr, HashMap *hmp)
{
	Node *result = NULL;
	List *ubargs = NIL;
	FOREACH(Node,sub,expr->args)
    {
	    ubargs = appendToTailOfList(ubargs, getLBExpr(sub, hmp));
    }
	result = (Node *)createFunctionCall(expr->functionname, ubargs);
	return result;
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
			Node *exprtmp = (Node *)createOpExpr(OPNAME_EQ, appendToTailOfList(singleton(expr->expr),((CaseWhen *)nd)->when));
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
		Node *c2 = (Node *)createOpExpr(OPNAME_EQ, appendToTailOfList(singleton(e1), (Node *)createConstInt(0)));
		Node *ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c1), createCaseOperator(c2)));
		Node *c3 = (Node *)createOpExpr(OPNAME_EQ, appendToTailOfList(singleton(e2), (Node *)createConstInt(0)));
		ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(ret), createCaseOperator(c3)));
		return ret;
	}
	else if(strcmp(strToUpper(expr->name),OPNAME_OR)==0){
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
	else if(strcmp(strToUpper(expr->name),OPNAME_AND)==0) {
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
			// INFO_LOG("REWRITE_RANGE_EXPR_PLUS: %s", nodeToString(ret));
			return ret;
		}
		if(strcmp(expr->name,"-")==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			//Upper bound of subtraction is the ub-lb
			Node *ret = (Node *)createOpExpr("-", appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_EQ)==0) {
			// INFO_LOG("rewrite = ");
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node * c1 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			Node * c2 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			// Node *c1 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			// Node *c2 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			// Node *c3 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			// Node *c4 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			// Node *c5 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			// Node *c6 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));

			// Node *c12 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c1),c2));
			// Node *c34 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c3),c4));
			// Node *c56 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c5),c6));
			// Node *c1234 = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(c12),c34));
			// Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(c1234),c56));
			Node *ret = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(c1,c2));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_GT)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_GT, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_GE)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_LT)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_LT, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_LE)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
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
		else if(strcmp(strToUpper(expr->name),OPNAME_OR)==0){
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),OPNAME_AND)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		else {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			return (Node *)createOpExpr(expr->name,LIST_MAKE(getUBExpr(e1, hmp),getUBExpr(e2, hmp)));
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
		if(strcmp(expr->name,OPNAME_EQ)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *c1 = (Node *)createOpExpr(OPNAME_EQ, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e1, hmp)));
			Node *c2 = (Node *)createOpExpr(OPNAME_EQ, appendToTailOfList(singleton(getUBExpr(e2, hmp)),getLBExpr(e2, hmp)));
			Node *c12 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c1),c2));
			Node *ret = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(expr),c12));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_GT)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_GT, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_GE)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_LT)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_LT, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		if(strcmp(expr->name,OPNAME_LE)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),OPNAME_OR)==0){
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
			return ret;
		}
		else if(strcmp(strToUpper(expr->name),OPNAME_AND)==0) {
			Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
			Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
			Node *ret = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
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

static Node *
rangeLBCase(CaseExpr *expr, HashMap *hmp)
{
	//TODO deal with expression cause
    //  case with a single else clause
	if (LIST_LENGTH(expr->whenClauses) == 0)
	{
		return getLBExpr(expr->elseRes, hmp);
	}
	//  case with a single else clause
	if (LIST_LENGTH(expr->whenClauses) == 1 && expr->elseRes == NULL)
	{
		CaseWhen *when = getHeadOfListP(expr->whenClauses);
		Node *lbCond = getLBExpr(when->then, hmp);
		Node *lbThen = getLBExpr(when->then, hmp);

		return (Node *) createCaseExpr(NULL,
									   singleton(createCaseWhen(lbCond, lbThen)),
									   NULL
			);
	}
    // more than one when clause, process one when clause and recurse
	CaseWhen *firstWhen = getHeadOfListP(expr->whenClauses);
	List *newWhen = removeFromHead(copyObject(expr->whenClauses));
	CaseExpr *reduced = createCaseExpr(NULL, newWhen, expr->elseRes);
	Node *reducedLB = rangeLBCase(reduced, hmp);
	Node *certTrue = getLBExpr(firstWhen->when, hmp);
	Node *cretFalse = (Node *) createOpExpr(OPNAME_NOT, singleton(getLBExpr(firstWhen->when, hmp)));
	Node *lbThen = getLBExpr(firstWhen->then, hmp);

	return (Node *) createCaseExpr(NULL,
						  LIST_MAKE(createCaseWhen(certTrue,lbThen),
									createCaseWhen(cretFalse,reducedLB)
							  ),
						  (Node *) createFunctionCall(LEAST_FUNC_NAME,
											 LIST_MAKE(copyObject(lbThen),copyObject(reducedLB))
							  )
		);
}

static Node *
rangeUBCase(CaseExpr *expr, HashMap *hmp)
{
	//TODO deal with expression cause
    //  case with a single else clause
	if (LIST_LENGTH(expr->whenClauses) == 0)
	{
		return getUBExpr(expr->elseRes, hmp);
	}
	//  case with a single else clause
	if (LIST_LENGTH(expr->whenClauses) == 1 && expr->elseRes == NULL)
	{
		CaseWhen *when = getHeadOfListP(expr->whenClauses);
		Node *lbCond = getUBExpr(when->then, hmp);
		Node *lbThen = getUBExpr(when->then, hmp);

		return (Node *) createCaseExpr(NULL,
									   singleton(createCaseWhen(lbCond, lbThen)),
									   NULL
			);
	}
    // more than one when clause, process one when clause and recurse
	CaseWhen *firstWhen = getHeadOfListP(expr->whenClauses);
	List *newWhen = removeFromHead(copyObject(expr->whenClauses));
	CaseExpr *reduced = createCaseExpr(NULL, newWhen, expr->elseRes);
	Node *reducedLB = rangeLBCase(reduced, hmp);
	Node *certTrue = getUBExpr(firstWhen->when, hmp);
	Node *certFalse = (Node *)createOpExpr(OPNAME_NOT, singleton(getUBExpr(firstWhen->when, hmp)));
	Node *lbThen = getUBExpr(firstWhen->then, hmp);

	return (Node *)createCaseExpr(
		NULL,
		LIST_MAKE(createCaseWhen(certTrue, lbThen),
				  createCaseWhen(certFalse, reducedLB)),
		(Node *)createFunctionCall(
			LEAST_FUNC_NAME,
			LIST_MAKE(copyObject(lbThen), copyObject(reducedLB))));
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

	List *projExpr = getProjExprsForAllAttrs(op);

	if (attrLevel)
	{
		FOREACH(Node, nd, projExpr){
			addUncertAttrToSchema(hmp, op, nd);
		}
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

	//TODO intersection is not handled correctly

	// set difference
	if(((SetOperator *)op)->setOpType == SETOP_DIFFERENCE)
	{
		List *projExpr = getProjExprsForAllAttrs(op);
		projExpr = removeFromTail(projExpr);
		projExpr = appendToTailOfList(projExpr, createConstInt(0));

		QueryOperator *proj = (QueryOperator *)createProjectionOp(projExpr, op, NIL, getQueryOperatorAttrNames(op));
		switchSubtrees(op, proj);
		op->parents = singleton(proj);
		setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *) copyObject(hmp));

		return proj;
	}

	LOG_RESULT_UNCERT("UNCERTAIN: Rewritten Operator tree [SET]", op);

	return op;
}

extern char *getAttrTwoString(char *in){
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", in);
	appendStringInfo(str, "%s", SELFJOIN_AFFIX);
	return backendifyIdentifier(str->data);
}

/*
Add the hashmap property given list of normal attribute names and assume range annotations are named properly.
*/
static void create_Mapping_rewritten(QueryOperator* op, List *attrs, boolean row){
	HashMap * hmp = NEW_MAP(Node, Node);
	FOREACH(char, an, attrs){
		INFO_LOG("adding %s", an);
		Node * aref = (Node *)getAttrRefByName(op, an);
		Node * arefub = (Node *)getAttrRefByName(op, getUBString(an));
		Node * areflb = (Node *)getAttrRefByName(op, getLBString(an));
		ADD_TO_MAP(hmp, createNodeKeyValue((Node *)aref, (Node *)LIST_MAKE(arefub, areflb)));
		INFO_LOG("added %s", an);
	}
	if(row){
		ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_BESTGUESS), (Node *)getAttrRefByName(op, ROW_BESTGUESS)));
		ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_POSSIBLE), (Node *)getAttrRefByName(op, ROW_POSSIBLE)));
		ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_CERTAIN), (Node *)getAttrRefByName(op, ROW_CERTAIN)));
	}
	INFO_LOG(nodeToString((Node *)hmp));
	setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);
}

static QueryOperator *
rewrite_RangeAggregation(QueryOperator *op){
	//TODO
	ASSERT(OP_LCHILD(op));

	//push minmax to child
	if(getBoolOption(RANGE_OPTIMIZE_AGG) && ((AggregationOperator *)op)->groupBy){
		Set *newdep = MAKE_STR_SET(((AttributeReference *)getHeadOfListP(((AggregationOperator *)op)->groupBy))->name);
		if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
		{
			Set *dependency = copyObject((Set *)getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS));
			newdep = unionSets(newdep, dependency);
		}
		INFO_LOG("[Aggregation] Pushing minmax prop attr to child: %s", nodeToString(newdep));
		setStringProperty(OP_LCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)newdep);
	}

	//record original schema info
	List *proj_projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *pro_attrName = getNormalAttrNames(OP_LCHILD(op));
	List *p_attrName = getNormalAttrNames(OP_LCHILD(op));
//	List *agg_attrName = getQueryOperatorAttrNames(OP_LCHILD(op));
	List *agg_projExpr = getNormalAttrProjectionExprs(op);

	List *aggr_groupby_list = copyObject(((AggregationOperator *)op)->groupBy);
	List *aggr_out_names = getQueryOperatorAttrNames(op);

	// rewrite child first
	QueryOperator *childop = rewriteRange(OP_LCHILD(op));

	if(0){
			childop = combineRowByBG(childop);
	}

	INFO_LOG("REWRITE-RANGE - Aggregation");
	HashMap * hmp = NEW_MAP(Node, Node);
	// HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);

	//rewrite non-groupby case
	if(((AggregationOperator *)op)->groupBy == NIL){
		INFO_LOG("RANGE_Aggregation - No groupby");

		int ptr = 0;

		// List *proj_ExprList = NIL;
		// List *proj_NameList = NIL;

		List *aggrl = copyList(((AggregationOperator *)op)->aggrs);

		//add projection
		FOREACH(Node, nd, aggrl){
			Node * funattr = getHeadOfListP(((FunctionCall *)nd)->args);
			ptr = ((AttributeReference *)funattr)->attrPosition;
			char *aName = getNthOfListP(pro_attrName, ptr);
			Node * funattrub = (Node *)getAttrRefByName(childop, getUBString(aName));
			Node * funattrlb = (Node *)getAttrRefByName(childop, getLBString(aName));
			if(strcmp(((FunctionCall *)nd)->functionname, COUNT_FUNC_NAME)==0)
			{
				getNthOfList(proj_projExpr,ptr)->data.ptr_value = getAttrRefByName(childop, ROW_BESTGUESS);
				proj_projExpr = appendToTailOfList(proj_projExpr, getAttrRefByName(childop, ROW_POSSIBLE));
				proj_projExpr = appendToTailOfList(proj_projExpr, getAttrRefByName(childop, ROW_CERTAIN));
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, SUM_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				Node *bgMult = (Node *)createOpExpr("*", LIST_MAKE(funattr,getAttrRefByName(childop, ROW_BESTGUESS)));
				Node *ubCase = (Node *)createCaseExpr(NULL,
					singleton((Node *)createCaseWhen(
						(Node *)createOpExpr(OPNAME_GT, LIST_MAKE(funattrub,createConstInt(0)))
						,(Node *)getAttrRefByName(childop, ROW_POSSIBLE)
					)),
					(Node *)getAttrRefByName(childop, ROW_CERTAIN)
				);
				Node *lbCase = (Node *)createCaseExpr(NULL,
					singleton((Node *)createCaseWhen(
						(Node *)createOpExpr(OPNAME_GT,
							LIST_MAKE(funattrlb,createConstInt(0)))
						,(Node *)getAttrRefByName(childop, ROW_CERTAIN)
					)),
					(Node *)getAttrRefByName(childop, ROW_POSSIBLE)
				);
				Node *ubMult = (Node *)createOpExpr("*", LIST_MAKE(funattrub,ubCase));
				Node *lbMult = (Node *)createOpExpr("*", LIST_MAKE(funattrlb,lbCase));
				getNthOfList(proj_projExpr,ptr)->data.ptr_value = bgMult;
				proj_projExpr = appendToTailOfList(proj_projExpr, ubMult);
				proj_projExpr = appendToTailOfList(proj_projExpr, lbMult);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			//TODO upper bound can be optimized if there are certain tuples exist
			if(strcmp(((FunctionCall *)nd)->functionname, MIN_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrub);
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrlb);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			//TODO lower bound can be optimized if there are certain tuples exist
			if(strcmp(((FunctionCall *)nd)->functionname, MAX_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrub);
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrlb);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			ptr++;
		}
		// Node *cr = getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN));
		// Node *br = getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS));
		// Node *pr = getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE));
		// proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(OPNAME_GT,LIST_MAKE(cr, createConstInt(1)))));
		// proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(OPNAME_GT,LIST_MAKE(br, createConstInt(1)))));
		// proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(OPNAME_GT,LIST_MAKE(pr, createConstInt(1)))));
		// pro_attrName = appendToTailOfList(pro_attrName,ROW_CERTAIN);
		// pro_attrName = appendToTailOfList(pro_attrName,ROW_BESTGUESS);
		// pro_attrName = appendToTailOfList(pro_attrName,ROW_POSSIBLE);
		QueryOperator *proj = (QueryOperator *)createProjectionOp(proj_projExpr, childop, NIL, pro_attrName);
		switchSubtrees(childop, proj);
		childop->parents = singleton(proj);
		op->inputs = singleton(proj);

		// store UNCERT_MAPPING and mark uncertain attributes
		create_Mapping_rewritten(proj, p_attrName, FALSE);
		markUncertAttrsAsProv(proj);

		INFO_OP_LOG("Range Aggregation no groupby - add projection:", proj);

		//rewrite aggregation

		/* int pos = 0; */
		int aggpos = 0;

		FOREACH(Node, nd, aggrl){
			Node * funattr = getHeadOfListP(((FunctionCall *)nd)->args);
			INFO_LOG("%s", nodeToString(funattr));
			char * aName = ((AttributeReference *)funattr)->name;
			Node * aref = (Node *)getAttrRefByName(proj, aName);
			Node * arefub = (Node *)getAttrRefByName(proj, getUBString(aName));
			Node * areflb = (Node *)getAttrRefByName(proj, getLBString(aName));
			/* pos = ((AttributeReference *)funattr)->attrPosition; */
			if(strcmp(((FunctionCall *)nd)->functionname, COUNT_FUNC_NAME)==0){
				getNthOfList(((AggregationOperator *)op)->aggrs,aggpos)->data.ptr_value = createFunctionCall(SUM_FUNC_NAME,singleton(aref));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(SUM_FUNC_NAME,singleton(arefub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(SUM_FUNC_NAME,singleton(areflb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, MIN_FUNC_NAME)==0){
				// Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				// Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME,singleton(arefub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MIN_FUNC_NAME,singleton(areflb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, MAX_FUNC_NAME)==0){
				// Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				// Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME,singleton(arefub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MIN_FUNC_NAME,singleton(areflb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, SUM_FUNC_NAME)==0){
				// Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				// Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				Node *ubAgg = (Node *) createFunctionCall(SUM_FUNC_NAME,singleton(arefub));
				Node *lbAgg = (Node *) createFunctionCall(SUM_FUNC_NAME,singleton(areflb));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, ubAgg);
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, lbAgg);
			}
			aggpos++;
		}
		// addRangeRowToSchema(hmp, op);
		// Node* ct = (Node *)getAttrRefByName(proj,ROW_CERTAIN);
		// Node* bg = (Node *)getAttrRefByName(proj,ROW_BESTGUESS);
		// Node* ps = (Node *)getAttrRefByName(proj,ROW_POSSIBLE);
		// ((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(ct)));
		// ((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(bg)));
		// ((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(ps)));

		setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);
		markUncertAttrsAsProv(op);
//		INFO_LOG("%s", nodeToString(hmp));
		INFO_OP_LOG("Range Aggregation no groupby - rewrite aggregation:", op);


		//Add Projection for row annotations
		QueryOperator *fproj = createProjOnAllAttrs(op);
		HashMap *fprojhmp = (HashMap *)copyObject(hmp);
		((ProjectionOperator *)fproj)->projExprs = appendToTailOfList(((ProjectionOperator *)fproj)->projExprs, createConstInt(1));
		((ProjectionOperator *)fproj)->projExprs = appendToTailOfList(((ProjectionOperator *)fproj)->projExprs, createConstInt(1));
		((ProjectionOperator *)fproj)->projExprs = appendToTailOfList(((ProjectionOperator *)fproj)->projExprs, createConstInt(1));
		addRangeRowToSchema(fprojhmp, fproj);
		fproj->inputs = singleton(op);
		switchSubtrees(op, fproj);
		op->parents = singleton(fproj);
		create_Mapping_rewritten(fproj, getNormalAttrNames(op), TRUE);
		setStringProperty(fproj, UNCERT_MAPPING_PROP, (Node *)fprojhmp);
		INFO_OP_LOG("Range Aggregation no groupby - final projection:", fproj);
		return fproj;
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

	//fix datatypes
	FOREACH(Node, n, aggr_groupby_list){
		char *aname = ((AttributeReference *)n)->name;
		int apos = getAttrPos(preaggr, aname);
		int ubpos = getAttrPos(preaggr, getUBString(aname));
		int lbpos = getAttrPos(preaggr, getLBString(aname));
		List *ad = preaggr->schema->attrDefs;
		DataType dt = ((AttributeDef *)(getNthOfList(ad,apos)->data.ptr_value))->dataType;
		INFO_LOG("dt for %s is %d", aname, dt);
		DataType dt2 = ((AttributeDef *)(getNthOfList(ad,ubpos)->data.ptr_value))->dataType;
		INFO_LOG("dt for %s is %d", getUBString(aname), dt2);
		DataType dt3 = ((AttributeDef *)(getNthOfList(ad,lbpos)->data.ptr_value))->dataType;
		INFO_LOG("dt for %s is %d", getLBString(aname), dt3);
		((AttributeDef *)(getNthOfList(preaggr->schema->attrDefs,ubpos)->data.ptr_value))->dataType = dt;
		((AttributeDef *)(getNthOfList(preaggr->schema->attrDefs,lbpos)->data.ptr_value))->dataType = dt;
	}

	// create_Mapping_rewritten(preaggr, aggr_groupby_list, FALSE);

	INFO_OP_LOG("Range Aggregation with groupby - left pre-aggregation:", preaggr);

	//do the join

	List *attrn1 = getQueryOperatorAttrNames(preaggr);
	List *attrn2_1 = NIL;
	List *attrn2_2 = NIL;

//	List *expr1 = getProjExprsForAllAttrs(child);
//	List *expr2 = getProjExprsForAllAttrs(childdup);

	//right attrs from join rename
	FOREACH(char, n, getNormalAttrNames(childdup)){
		attrn2_1 = appendToTailOfList(attrn2_1, getAttrTwoString(n));
		attrn2_2 = appendToTailOfList(attrn2_2, getUBString(getAttrTwoString(n)));
		attrn2_2 = appendToTailOfList(attrn2_2, getLBString(getAttrTwoString(n)));
	}
	attrn2_2 = appendToTailOfList(attrn2_2, ROW_CERTAIN);
	attrn2_2 = appendToTailOfList(attrn2_2, ROW_BESTGUESS);
	attrn2_2 = appendToTailOfList(attrn2_2, ROW_POSSIBLE);
	// List *attrn2dup = deepCopyStringList(attrn2);
	List *attrJoin = CONCAT_LISTS(attrn1,attrn2_1,attrn2_2);
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
		Node *refExprCase1 = (Node *)createOpExpr(OPNAME_GE ,LIST_MAKE(aRef1_ub,aRef2_lb));
		Node *refExprCase2 = (Node *)createOpExpr(OPNAME_GE ,LIST_MAKE(aRef2_ub,aRef1_lb));
		// Node *refExprCase3 = (Node *)createOpExpr(OPNAME_AND ,LIST_MAKE(createOpExpr(OPNAME_GE, LIST_MAKE(aRef1_lb,aRef2_lb)), createOpExpr(OPNAME_LE, LIST_MAKE(aRef1_ub,aRef2_ub))));
		Node *refExpr = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(refExprCase1,refExprCase2));
		if(joinExpr == NULL){
			joinExpr = refExpr;
		}
		else {
			joinExpr = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(refExpr, joinExpr));
		}
	}
	INFO_LOG(nodeToString(joinExpr));

	//best guess matching
	Node *joinExprBg = NULL;
	FOREACH(Node, n, aggr_groupby_list){
		Node *aRef1 = (Node *)getAttrRefByName(preaggr, ((AttributeReference *)n)->name);
		Node *aRef2 = (Node *)getAttrRefByName(childdup, ((AttributeReference *)n)->name);
		((AttributeReference *)aRef2)->fromClauseItem = 1;
		Node *refExpr = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(aRef1,aRef2));
		if(joinExprBg == NULL){
			joinExprBg = refExpr;
		}
		else {
			joinExprBg = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(refExpr, joinExprBg));
		}
	}
	INFO_LOG(nodeToString(joinExprBg));


	//if optimization
	//TODO
	QueryOperator *bgVer = NULL;
	if(HAS_STRING_PROP(childdup, PROP_STORE_POSSIBLE_TREE)){
		bgVer = (QueryOperator *)copyObject(childdup);
		QueryOperator *poschild = (QueryOperator *)GET_STRING_PROP((QueryOperator *)copyObject(childdup), PROP_STORE_POSSIBLE_TREE);
		// QueryOperator *uop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(bgchild, poschild), NIL, getQueryOperatorAttrNames(bgchild));
		// switchSubtrees(bgchild, uop);
		// poschild->parents = singleton(uop);
		// bgchild->parents = singleton(uop);
		// setStringProperty(uop, UNCERT_MAPPING_PROP,
					  // copyObject(getStringProperty(childdup, UNCERT_MAPPING_PROP)));
		childdup = poschild;
		// markUncertAttrsAsProv(uop);
	}
	else if(getBoolOption(RANGE_OPTIMIZE_AGG)){
		bgVer = spliceToBGAggr((QueryOperator *)copyObject(childdup));
		QueryOperator *poschild = spliceToPOS((QueryOperator *)copyObject(childdup), ((AttributeReference *)getHeadOfListP(aggr_groupby_list))->name);
		// QueryOperator *uop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(bgchild, poschild), NIL, getQueryOperatorAttrNames(bgchild));
		// switchSubtrees(bgchild, uop);
		// poschild->parents = singleton(uop);
		// bgchild->parents = singleton(uop);
		// setStringProperty(poschild, UNCERT_MAPPING_PROP,
					  // copyObject(getStringProperty(childdup, UNCERT_MAPPING_PROP)));
		childdup = poschild;
		// markUncertAttrsAsProv(poschild);
		INFO_OP_LOG("[Aggregation OPTIMIZED] SPLICED AGGREGATION CHILD: ", childdup);
	}

	QueryOperator *join = (QueryOperator *)createJoinOp(JOIN_INNER, joinExpr ,LIST_MAKE(preaggr, childdup), NIL, attrJoin);
	switchSubtrees(preaggr, join);
	childdup->parents = singleton(join);
	preaggr->parents = singleton(join);

	List *attrname_normal = NIL;
	FOREACH(char, an, getNormalAttrNames(childdup)){
		attrname_normal = appendToTailOfList(attrname_normal, getAttrTwoString(an));
	}

	// INFO_LOG(stringListToString(attrname_normal));

	create_Mapping_rewritten(join, attrname_normal, TRUE);
	markUncertAttrsAsProv(join);

	if(bgVer != NULL){
		QueryOperator *preaggr2 = (QueryOperator *)copyObject(preaggr);
		QueryOperator *join2 = (QueryOperator *)createJoinOp(JOIN_INNER, joinExprBg ,LIST_MAKE(preaggr2, bgVer), NIL, attrJoin);
		// switchSubtrees(preaggr, join2);
		bgVer->parents = singleton(join2);
		preaggr2->parents = singleton(join2);

		create_Mapping_rewritten(join2, attrname_normal, TRUE);
		markUncertAttrsAsProv(join2);

		QueryOperator *posj = join;

		QueryOperator *uop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(join2, posj), NIL, attrJoin);
		switchSubtrees(join, uop);
		join2->parents = singleton(uop);
		posj->parents = singleton(uop);

		create_Mapping_rewritten(uop, attrname_normal, TRUE);
		markUncertAttrsAsProv(uop);

		join = uop;
	}

	INFO_OP_LOG("Range Aggregation with groupby - cross join:", join);

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

		Node * cert_eq_1 = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(join, fname_ub), getAttrRefByName(join, fname_lb)));
		Node * cert_eq_2 = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(join, getAttrTwoString(fname_ub)), getAttrRefByName(join, getAttrTwoString(fname_lb))));
		Node * cert_eq_3 = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(join, fname_ub), getAttrRefByName(join, getAttrTwoString(fname_ub))));

		Node * bg_eq = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(join, fname), getAttrRefByName(join, getAttrTwoString(fname))));

		Node * cert_eq = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(cert_eq_3, createOpExpr(OPNAME_AND, LIST_MAKE(cert_eq_1, cert_eq_2))));
		if(cert_case == NULL){
			cert_case = cert_eq;
		} else {
			cert_case = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(cert_case, cert_eq));
		}
		if(bg_case == NULL){
			bg_case = bg_eq;
		} else {
			bg_case = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(bg_case, bg_eq));
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
			projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_BESTGUESS));
			projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_POSSIBLE));
			projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_CERTAIN));
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
		if(strcmp(((FunctionCall *)n)->functionname, SUM_FUNC_NAME)==0){
			Node *bgAndExpr = NULL;
			FOREACH(Node, bn, aggr_groupby_list){
				char *gbn = ((AttributeReference *)bn)->name;
				Node *bgeq = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(join, gbn),getAttrRefByName(join, getAttrTwoString(gbn))));
				if(bgAndExpr == NULL){
					bgAndExpr = bgeq;
				} else {
					bgAndExpr = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(bgAndExpr, bgeq));
				}
			}
			Node *bgCase = (Node *)createCaseExpr(NULL,
				singleton((Node *)createCaseWhen(
					bgAndExpr
					,(Node *)createConstInt(1)/*(Node *)getAttrRefByName(join, ROW_BESTGUESS)*/
				)),
				(Node *)createConstInt(0)
			);
			Node *ubCase = (Node *)createCaseExpr(NULL,
				singleton((Node *)createCaseWhen(
					(Node *)createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(join, fname_ub),createConstInt(0)))
					,(Node *)getAttrRefByName(join, ROW_POSSIBLE)
				)),
				(Node *)createConstInt(0)
			);
			Node *lbCase = (Node *)createCaseExpr(NULL,
				singleton((Node *)createCaseWhen(
					(Node *)createOpExpr(OPNAME_GT,
						LIST_MAKE(getAttrRefByName(join, fname_lb),createConstInt(0)))
						,(Node *)createConstInt(0)
					)),
				(Node *)getAttrRefByName(join, ROW_POSSIBLE)
			);
			Node *bgMult = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(join, fname),bgCase));
			Node *ubMult = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(join, fname_ub),ubCase));
			Node *lbMult = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(join, fname_lb),lbCase));
			projList = appendToTailOfList(projList, bgMult);
			projList = appendToTailOfList(projList, ubMult);
			projList = appendToTailOfList(projList, lbMult);
			nameList = appendToTailOfList(nameList,fname);
			nameList = appendToTailOfList(nameList,fname_ub);
			nameList = appendToTailOfList(nameList,fname_lb);
		}
	}
	//TODO optimize annotations

	projList = appendToTailOfList(projList, createCaseOperator(cert_case));
	projList = appendToTailOfList(projList, createCaseOperator(bg_case));
	projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_POSSIBLE));
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
		if(strcmp(((FunctionCall *)n)->functionname, SUM_FUNC_NAME)==0){
			Node *bgfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname)));
			new_aggr_List = appendToTailOfList(new_aggr_List, bgfunc);
			Node *ubfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname_ub)));
			new_aggr_List = appendToTailOfList(new_aggr_List, ubfunc);
			Node *lbfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname_lb)));
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
	new_aggr_List = appendToTailOfList(new_aggr_List, (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(proj, ROW_BESTGUESS))));
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
		INFO_LOG("Aggregation attr: %s", nodeToString(n));
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
		INFO_LOG("groupby attr: %s", nodeToString(n));
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

	List *projlist = getProjExprsForAllAttrs(finalproj);

	FOREACH(Node, n, projlist)
	{
		addRangeAttrToSchema(hmp, finalproj, n);
	}

	addRangeRowToSchema(hmp, finalproj);

	setStringProperty(finalproj, UNCERT_MAPPING_PROP, (Node *)hmp);
	markUncertAttrsAsProv(finalproj);

	switchSubtrees(op, finalproj);

	return finalproj;
}

// static QueryOperator *rewrite_RangeAggregationOptimizedWithGroupby(QueryOperator *op){
// 	return op;
// }

// Try the 2nd version of optimized aggregation rewriting.
// Do two phase aggregations instead of self join best guess to test out the difference.
static QueryOperator *
rewrite_RangeAggregation2(QueryOperator *op){
	//TODO
	ASSERT(OP_LCHILD(op));

	//push minmax to child
	if(getBoolOption(RANGE_OPTIMIZE_AGG) && ((AggregationOperator *)op)->groupBy){
		Set *newdep = MAKE_STR_SET(((AttributeReference *)getHeadOfListP(((AggregationOperator *)op)->groupBy))->name);
		if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
		{
			Set *dependency = copyObject((Set *)getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS));
			newdep = unionSets(newdep, dependency);
		}
		INFO_LOG("[Aggregation] Pushing minmax prop attr to child: %s", nodeToString(newdep));
		setStringProperty(OP_LCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)newdep);
	}

	//record original schema info
	List *proj_projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *pro_attrName = getNormalAttrNames(OP_LCHILD(op));
	// List *p_attrName = getNormalAttrNames(OP_LCHILD(op));
//	List *agg_attrName = getQueryOperatorAttrNames(OP_LCHILD(op));
	List *agg_projExpr = getNormalAttrProjectionExprs(op);

	List *aggr_groupby_list = copyObject(((AggregationOperator *)op)->groupBy);
	List *aggr_out_names = getQueryOperatorAttrNames(op);

	// rewrite child first
	QueryOperator *childop = rewriteRange(OP_LCHILD(op));

	if(0){
			childop = combineRowByBG(childop);
	}

	INFO_LOG("REWRITE-RANGE - Aggregation");
	HashMap * hmp = NEW_MAP(Node, Node);
	// HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);

	//rewrite non-groupby case
	if(((AggregationOperator *)op)->groupBy == NIL){
		INFO_LOG("RANGE_Aggregation - No groupby");

		int ptr = 0;

		// List *proj_ExprList = NIL;
		// List *proj_NameList = NIL;

		List *aggrl = copyList(((AggregationOperator *)op)->aggrs);

		//add projection
		FOREACH(Node, nd, aggrl){
			Node * funattr = getHeadOfListP(((FunctionCall *)nd)->args);
			ptr = ((AttributeReference *)funattr)->attrPosition;
			char *aName = getNthOfListP(pro_attrName, ptr);
			Node * funattrub = (Node *)getAttrRefByName(childop, getUBString(aName));
			Node * funattrlb = (Node *)getAttrRefByName(childop, getLBString(aName));
			if(strcmp(((FunctionCall *)nd)->functionname, COUNT_FUNC_NAME)==0)
			{
				getNthOfList(proj_projExpr,ptr)->data.ptr_value = getAttrRefByName(childop, ROW_BESTGUESS);
				proj_projExpr = appendToTailOfList(proj_projExpr, getAttrRefByName(childop, ROW_POSSIBLE));
				proj_projExpr = appendToTailOfList(proj_projExpr, getAttrRefByName(childop, ROW_CERTAIN));
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, SUM_FUNC_NAME)==0){
				INFO_LOG("%s", nodeToString(funattr));
				Node *bgMult = (Node *)createOpExpr("*", LIST_MAKE(funattr,getAttrRefByName(childop, ROW_BESTGUESS)));
				Node *ubCase = (Node *)createCaseExpr(NULL,
					singleton((Node *)createCaseWhen(
						(Node *)createOpExpr(OPNAME_GT, LIST_MAKE(funattrub,createConstInt(0)))
						,(Node *)getAttrRefByName(childop, ROW_POSSIBLE)
					)),
					(Node *)getAttrRefByName(childop, ROW_CERTAIN)
				);
				Node *lbCase = (Node *)createCaseExpr(NULL,
					singleton((Node *)createCaseWhen(
						(Node *)createOpExpr(OPNAME_GT,
							LIST_MAKE(funattrlb,createConstInt(0)))
						,(Node *)getAttrRefByName(childop, ROW_CERTAIN)
					)),
					(Node *)getAttrRefByName(childop, ROW_POSSIBLE)
				);
				Node *ubMult = (Node *)createOpExpr("*", LIST_MAKE(funattrub,ubCase));
				Node *lbMult = (Node *)createOpExpr("*", LIST_MAKE(funattrlb,lbCase));
				getNthOfList(proj_projExpr,ptr)->data.ptr_value = bgMult;
				proj_projExpr = appendToTailOfList(proj_projExpr, ubMult);
				proj_projExpr = appendToTailOfList(proj_projExpr, lbMult);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			//TODO upper bound can be optimized if there are certain tuples exist
			if(strcmp(((FunctionCall *)nd)->functionname, MIN_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrub);
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrlb);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			//TODO lower bound can be optimized if there are certain tuples exist
			if(strcmp(((FunctionCall *)nd)->functionname, MAX_FUNC_NAME)==0){
//				INFO_LOG("%s", nodeToString(funattr));
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrub);
				proj_projExpr = appendToTailOfList(proj_projExpr, funattrlb);
				pro_attrName = appendToTailOfList(pro_attrName, getUBString(aName));
				pro_attrName = appendToTailOfList(pro_attrName, getLBString(aName));
			}
			ptr++;
		}
		// Node *cr = getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN));
		// Node *br = getMap(hmpIn, (Node *)createAttributeReference(ROW_BESTGUESS));
		// Node *pr = getMap(hmpIn, (Node *)createAttributeReference(ROW_POSSIBLE));
		// proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(OPNAME_GT,LIST_MAKE(cr, createConstInt(1)))));
		// proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(OPNAME_GT,LIST_MAKE(br, createConstInt(1)))));
		// proj_projExpr = appendToTailOfList(proj_projExpr, createCaseOperator((Node *)createOpExpr(OPNAME_GT,LIST_MAKE(pr, createConstInt(1)))));
		// pro_attrName = appendToTailOfList(pro_attrName,ROW_CERTAIN);
		// pro_attrName = appendToTailOfList(pro_attrName,ROW_BESTGUESS);
		// pro_attrName = appendToTailOfList(pro_attrName,ROW_POSSIBLE);
		QueryOperator *proj = (QueryOperator *)createProjectionOp(proj_projExpr, childop, NIL, pro_attrName);
		switchSubtrees(childop, proj);
		childop->parents = singleton(proj);
		op->inputs = singleton(proj);

		// INFO_OP_LOG("Range Aggregation no groupby - add projection[pre]:", proj);

		// store UNCERT_MAPPING and mark uncertain attributes
		// create_Mapping_rewritten(proj, p_attrName, FALSE);
		// markUncertAttrsAsProv(proj);

		INFO_OP_LOG("Range Aggregation no groupby - add projection:", proj);

		//rewrite aggregation

		/* int pos = 0; */
		int aggpos = 0;

		FOREACH(Node, nd, aggrl){
			Node * funattr = getHeadOfListP(((FunctionCall *)nd)->args);
			INFO_LOG("%s", nodeToString(funattr));
			char * aName = ((AttributeReference *)funattr)->name;
			Node * aref = (Node *)getAttrRefByName(proj, aName);
			Node * arefub = (Node *)getAttrRefByName(proj, getUBString(aName));
			Node * areflb = (Node *)getAttrRefByName(proj, getLBString(aName));
			/* pos = ((AttributeReference *)funattr)->attrPosition; */
			if(strcmp(((FunctionCall *)nd)->functionname, COUNT_FUNC_NAME)==0){
				getNthOfList(((AggregationOperator *)op)->aggrs,aggpos)->data.ptr_value = createFunctionCall(SUM_FUNC_NAME,singleton(aref));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(SUM_FUNC_NAME,singleton(arefub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(SUM_FUNC_NAME,singleton(areflb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, MIN_FUNC_NAME)==0){
				// Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				// Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME,singleton(arefub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MIN_FUNC_NAME,singleton(areflb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, MAX_FUNC_NAME)==0){
				// Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				// Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME,singleton(arefub)));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MIN_FUNC_NAME,singleton(areflb)));
			}
			if(strcmp(((FunctionCall *)nd)->functionname, SUM_FUNC_NAME)==0){
				// Node* funattrlb = (Node *)getAttrRefByName(proj,getLBString(((AttributeReference *)funattr)->name));
				// Node* funattrub = (Node *)getAttrRefByName(proj,getUBString(((AttributeReference *)funattr)->name));
				Node *lbAgg = (Node *) createFunctionCall(SUM_FUNC_NAME,singleton(areflb));
				Node *ubAgg = (Node *) createFunctionCall(SUM_FUNC_NAME,singleton(arefub));
				addRangeAttrToSchema(hmp, op, getNthOfListP(agg_projExpr, aggpos));
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, ubAgg);
				((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, lbAgg);
			}
			aggpos++;
		}
		// addRangeRowToSchema(hmp, op);
		// Node* ct = (Node *)getAttrRefByName(proj,ROW_CERTAIN);
		// Node* bg = (Node *)getAttrRefByName(proj,ROW_BESTGUESS);
		// Node* ps = (Node *)getAttrRefByName(proj,ROW_POSSIBLE);
		// ((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(ct)));
		// ((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(bg)));
		// ((AggregationOperator *)op)->aggrs = appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall(MAX_FUNC_NAME, singleton(ps)));

		setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);
		markUncertAttrsAsProv(op);
//		INFO_LOG("%s", nodeToString(hmp));
		INFO_OP_LOG("Range Aggregation no groupby - rewrite aggregation:", op);


		//Add Projection for row annotations
		QueryOperator *fproj = createProjOnAllAttrs(op);
		HashMap *fprojhmp = (HashMap *)copyObject(hmp);
		((ProjectionOperator *)fproj)->projExprs = appendToTailOfList(((ProjectionOperator *)fproj)->projExprs, createConstInt(1));
		((ProjectionOperator *)fproj)->projExprs = appendToTailOfList(((ProjectionOperator *)fproj)->projExprs, createConstInt(1));
		((ProjectionOperator *)fproj)->projExprs = appendToTailOfList(((ProjectionOperator *)fproj)->projExprs, createConstInt(1));
		addRangeRowToSchema(fprojhmp, fproj);
		fproj->inputs = singleton(op);
		switchSubtrees(op, fproj);
		op->parents = singleton(fproj);
		create_Mapping_rewritten(fproj, getNormalAttrNames(op), TRUE);
		setStringProperty(fproj, UNCERT_MAPPING_PROP, (Node *)fprojhmp);
		INFO_OP_LOG("Range Aggregation no groupby - final projection:", fproj);
		return fproj;
	}

	//rewrite groupby case

	INFO_LOG("RANGE_Aggregation - With groupby");

	//two branches for "selfjoin"

	QueryOperator *child = OP_LCHILD(op);
	QueryOperator *childdup = copyObject(child);

	//pre-aggregate group by attributes, bg aggregation results and

	List *aggrlist = NIL;
	List *gattrn = NIL;
	List *rattrn = NIL;

	List *agg_f_bg = ((AggregationOperator *)op)->aggrs;

	//pre projection for correct bag results
	//Assumes input order: aggregation, groupby attributes
	QueryOperator *preproj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(child), child, NIL, getQueryOperatorAttrNames(child));
	switchSubtrees(child, preproj);
	child->parents = singleton(preproj);

	setStringProperty(preproj, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(child, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(preproj);

	List *preproj_projExtr = ((ProjectionOperator *)preproj)->projExprs;
	FOREACH(Node, n, agg_f_bg){
		if(strcmp(((FunctionCall *)n)->functionname, COUNT_FUNC_NAME)==0){
			Node * funattr = getHeadOfListP(((FunctionCall *)n)->args);
			int ptr = ((AttributeReference *)funattr)->attrPosition;
			getNthOfList(preproj_projExtr,ptr)->data.ptr_value = (Node *)getAttrRefByName(child, ROW_BESTGUESS);
		}
		else if(strcmp(((FunctionCall *)n)->functionname, SUM_FUNC_NAME)==0){
			Node * funattr = getHeadOfListP(((FunctionCall *)n)->args);
			Node * multOp = (Node *)createOpExpr("*", LIST_MAKE(funattr, getAttrRefByName(child, ROW_BESTGUESS)));
			int ptr = ((AttributeReference *)funattr)->attrPosition;
			getNthOfList(preproj_projExtr,ptr)->data.ptr_value = multOp;
		}
	}
	//certain annotation conditions
	Node * cert_case = NULL;

	FOREACH(Node, n, aggr_groupby_list){
		char *fname = ((AttributeReference *)n)->name;
		char *fname_ub = getUBString(fname);
		char *fname_lb = getLBString(fname);

		Node * cert_eq = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(child, fname_ub), getAttrRefByName(child, fname_lb)));

		if(cert_case == NULL){
			cert_case = cert_eq;
		} else {
			cert_case = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(cert_case, cert_eq));
		}
	}
	Node *certainCond = (Node *)createOpExpr("*", LIST_MAKE(createCaseOperator(cert_case), getAttrRefByName(child, ROW_CERTAIN)));
	int ptr = getAttrRefByName(child, ROW_CERTAIN)->attrPosition;
	getNthOfList(preproj_projExtr,ptr)->data.ptr_value = certainCond;

	child = preproj;

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
	FOREACH(Node, n, agg_f_bg){
		aggrlist = appendToTailOfList(aggrlist, n);
		if(strcmp(((FunctionCall *)n)->functionname, COUNT_FUNC_NAME)==0){
			((FunctionCall *)n)->functionname = SUM_FUNC_NAME;
		}
		Node * funattr = getHeadOfListP(((FunctionCall *)n)->args);
		char * fname = ((AttributeReference *)funattr)->name;
		rattrn = appendToTailOfList(rattrn, fname);
	}
	Node *ctAggr = (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(child, ROW_CERTAIN)));
	Node *bgAggr = (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(child, ROW_BESTGUESS)));
	aggrlist = appendToTailOfList(aggrlist, ctAggr);
	aggrlist = appendToTailOfList(aggrlist, bgAggr);
	rattrn = appendToTailOfList(rattrn, ROW_CERTAIN);
	rattrn = appendToTailOfList(rattrn, ROW_BESTGUESS);

	List *attrnames = concatTwoLists(rattrn, gattrn);

	QueryOperator *preaggr = (QueryOperator *)createAggregationOp(aggrlist, aggr_groupby_list, child, NIL, attrnames);
	switchSubtrees(child, preaggr);
	child->parents = singleton(preaggr);
	// create_Mapping_rewritten(preaggr, aggr_groupby_list, FALSE);

	// //fix datatypes
	// FOREACH(Node, n, aggr_groupby_list){
	// 	char *aname = ((AttributeReference *)n)->name;
	// 	int apos = getAttrPos(preaggr, aname);
	// 	int ubpos = getAttrPos(preaggr, getUBString(aname));
	// 	int lbpos = getAttrPos(preaggr, getLBString(aname));
	// 	List *ad = preaggr->schema->attrDefs;
	// 	DataType dt = ((AttributeDef *)(getNthOfList(ad,apos)->data.ptr_value))->dataType;
	// 	INFO_LOG("dt for %s is %d", aname, dt);
	// 	DataType dt2 = ((AttributeDef *)(getNthOfList(ad,ubpos)->data.ptr_value))->dataType;
	// 	INFO_LOG("dt for %s is %d", getUBString(aname), dt2);
	// 	DataType dt3 = ((AttributeDef *)(getNthOfList(ad,lbpos)->data.ptr_value))->dataType;
	// 	INFO_LOG("dt for %s is %d", getLBString(aname), dt3);
	// 	((AttributeDef *)(getNthOfList(preaggr->schema->attrDefs,ubpos)->data.ptr_value))->dataType = dt;
	// 	((AttributeDef *)(getNthOfList(preaggr->schema->attrDefs,lbpos)->data.ptr_value))->dataType = dt;
	// }

	INFO_OP_LOG("Range Aggregation with groupby - left pre-aggregation:", preaggr);

	//do the join

	List *attrn1 = getQueryOperatorAttrNames(preaggr);
	List *attrn2_1 = NIL;
	List *attrn2_2 = NIL;

//	List *expr1 = getProjExprsForAllAttrs(child);
//	List *expr2 = getProjExprsForAllAttrs(childdup);

	//right attrs from join rename
	FOREACH(char, n, getNormalAttrNames(childdup)){
		attrn2_1 = appendToTailOfList(attrn2_1, getAttrTwoString(n));
		attrn2_2 = appendToTailOfList(attrn2_2, getUBString(getAttrTwoString(n)));
		attrn2_2 = appendToTailOfList(attrn2_2, getLBString(getAttrTwoString(n)));
	}
	attrn2_2 = appendToTailOfList(attrn2_2, ROW_CERTAIN_TWO);
	attrn2_2 = appendToTailOfList(attrn2_2, ROW_BESTGUESS_TWO);
	attrn2_2 = appendToTailOfList(attrn2_2, ROW_POSSIBLE_TWO);
	// List *attrn2dup = deepCopyStringList(attrn2);
	List *attrJoin = CONCAT_LISTS(attrn1,attrn2_1,attrn2_2);

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
		Node *refExprCase1 = (Node *)createOpExpr(OPNAME_GE ,LIST_MAKE(aRef1_ub,aRef2_lb));
		Node *refExprCase2 = (Node *)createOpExpr(OPNAME_GE ,LIST_MAKE(aRef2_ub,aRef1_lb));
		// Node *refExprCase3 = (Node *)createOpExpr(OPNAME_AND ,LIST_MAKE(createOpExpr(OPNAME_GE, LIST_MAKE(aRef1_lb,aRef2_lb)), createOpExpr(OPNAME_LE, LIST_MAKE(aRef1_ub,aRef2_ub))));
		Node *refExpr = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(refExprCase1,refExprCase2));
		if(joinExpr == NULL){
			joinExpr = refExpr;
		}
		else {
			joinExpr = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(refExpr, joinExpr));
		}
	}
	INFO_LOG(nodeToString(joinExpr));

	// //best guess matching
	// Node *joinExprBg = NULL;
	// FOREACH(Node, n, aggr_groupby_list){
	// 	Node *aRef1 = (Node *)getAttrRefByName(preaggr, ((AttributeReference *)n)->name);
	// 	Node *aRef2 = (Node *)getAttrRefByName(childdup, ((AttributeReference *)n)->name);
	// 	((AttributeReference *)aRef2)->fromClauseItem = 1;
	// 	Node *refExpr = (Node *)createOpExpr(OPNAME_EQ, LIST_MAKE(aRef1,aRef2));
	// 	if(joinExprBg == NULL){
	// 		joinExprBg = refExpr;
	// 	}
	// 	else {
	// 		joinExprBg = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(refExpr, joinExprBg));
	// 	}
	// }
	// INFO_LOG(nodeToString(joinExprBg));


	//if optimization
	//TODO
	// QueryOperator *bgVer = NULL;
	if(HAS_STRING_PROP(childdup, PROP_STORE_POSSIBLE_TREE)){
		// bgVer = (QueryOperator *)copyObject(childdup);
		QueryOperator *poschild = (QueryOperator *)GET_STRING_PROP((QueryOperator *)copyObject(childdup), PROP_STORE_POSSIBLE_TREE);
		childdup = poschild;
		// markUncertAttrsAsProv(uop);
	}
	else if(getBoolOption(RANGE_OPTIMIZE_AGG)){
		// bgVer = spliceToBGAggr((QueryOperator *)copyObject(childdup));
		QueryOperator *poschild = spliceToPOS((QueryOperator *)copyObject(childdup), ((AttributeReference *)getHeadOfListP(aggr_groupby_list))->name);
		childdup = poschild;
	}

	INFO_LOG("Cross Join attributes: %s", stringListToString(attrJoin));

	QueryOperator *join = (QueryOperator *)createJoinOp(JOIN_INNER, joinExpr ,LIST_MAKE(preaggr, childdup), NIL, attrJoin);
	switchSubtrees(preaggr, join);
	childdup->parents = singleton(join);
	preaggr->parents = singleton(join);

	// List *attrname_normal = NIL;
	// FOREACH(char, an, getNormalAttrNames(childdup)){
		// attrname_normal = appendToTailOfList(attrname_normal, getAttrTwoString(an));
	// }

	// INFO_LOG(stringListToString(attrname_normal));

	// create_Mapping_rewritten(join, attrname_normal, FALSE);
	// markUncertAttrsAsProv(join);

	INFO_OP_LOG("Range Aggregation with groupby - cross join:", join);

	//pre_projection
	List *projList = NIL;
	List *nameList = NIL;

	//a,ub_a,lb_a,b,ub_b,lb_b,...
	// Node *cert_case = NULL;
	// Node *bg_case = NULL;

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
	}

	List *aggrl = copyList(((AggregationOperator *)op)->aggrs);

	//projection on aggregation attributes
	//urange(select a,count(b) from r is radb group by a);

	FOREACH(Node, n, aggrl){
		Node * funattr = getHeadOfListP(((FunctionCall *)n)->args);
		char * fnameo = ((AttributeReference *)funattr)->name;
		char * fname = getAttrTwoString(((AttributeReference *)funattr)->name);
		char * fname_ub = getUBString(fname);
		char * fname_lb = getLBString(fname);
		INFO_LOG("%s, %s, %s, %s",fnameo, fname, fname_ub, fname_lb);
		if(strcmp(((FunctionCall *)n)->functionname, COUNT_FUNC_NAME)==0){
			projList = appendToTailOfList(projList, getAttrRefByName(join, fnameo));
			projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_POSSIBLE_TWO));
			projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_CERTAIN_TWO));
			nameList = appendToTailOfList(nameList,fname);
			nameList = appendToTailOfList(nameList,fname_ub);
			nameList = appendToTailOfList(nameList,fname_lb);
		}
		if(strcmp(((FunctionCall *)n)->functionname, MIN_FUNC_NAME)==0 || strcmp(((FunctionCall *)n)->functionname, MAX_FUNC_NAME)==0){
			projList = appendToTailOfList(projList, getAttrRefByName(join, fnameo));
			projList = appendToTailOfList(projList, getAttrRefByName(join, fname_ub));
			projList = appendToTailOfList(projList, getAttrRefByName(join, fname_lb));
			nameList = appendToTailOfList(nameList,fname);
			nameList = appendToTailOfList(nameList,fname_ub);
			nameList = appendToTailOfList(nameList,fname_lb);
		}
		if(strcmp(((FunctionCall *)n)->functionname, SUM_FUNC_NAME)==0){
			Node *ubCase = (Node *)createCaseExpr(NULL,
				singleton((Node *)createCaseWhen(
					(Node *)createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(join, fname_ub),createConstInt(0)))
					,(Node *)getAttrRefByName(join, ROW_POSSIBLE_TWO)
				)),
				(Node *)createConstInt(0)
			);
			Node *lbCase = (Node *)createCaseExpr(NULL,
				singleton((Node *)createCaseWhen(
					(Node *)createOpExpr(OPNAME_GT,
						LIST_MAKE(getAttrRefByName(join, fname_lb),createConstInt(0)))
						,(Node *)createConstInt(0)
					)),
				(Node *)getAttrRefByName(join, ROW_POSSIBLE_TWO)
			);
			Node *bgMult = (Node *)getAttrRefByName(join, fnameo);
			Node *ubMult = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(join, fname_ub),ubCase));
			Node *lbMult = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(join, fname_lb),lbCase));
			projList = appendToTailOfList(projList, bgMult);
			projList = appendToTailOfList(projList, ubMult);
			projList = appendToTailOfList(projList, lbMult);
			nameList = appendToTailOfList(nameList,fname);
			nameList = appendToTailOfList(nameList,fname_ub);
			nameList = appendToTailOfList(nameList,fname_lb);
		}
	}
	//TODO optimize annotations

	Node *cetExpr = (Node *)createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(join, ROW_CERTAIN), createConstInt(0)));
	Node *bgExpr = (Node *)createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(join, ROW_BESTGUESS), createConstInt(0)));
	projList = appendToTailOfList(projList, createCaseOperator(cetExpr));
	projList = appendToTailOfList(projList, createCaseOperator(bgExpr));
	projList = appendToTailOfList(projList, getAttrRefByName(join, ROW_POSSIBLE_TWO));
	nameList = appendToTailOfList(nameList, ROW_CERTAIN);
	nameList = appendToTailOfList(nameList, ROW_BESTGUESS);
	nameList = appendToTailOfList(nameList, ROW_POSSIBLE);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(projList, join, NIL, nameList);
	switchSubtrees(join, proj);
	join->parents = singleton(proj);

	INFO_OP_LOG("Range Aggregation with groupby - projection:", proj);

	//	new aggregation groupby list
	// List *new_aggr_groupby_names = getQueryOperatorAttrNames(preaggr);
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
			Node *bgfunc = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, fname)));
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
		if(strcmp(((FunctionCall *)n)->functionname, SUM_FUNC_NAME)==0){
			Node *bgfunc = (Node *)createFunctionCall(MIN_FUNC_NAME, singleton(getAttrRefByName(proj, fname)));
			new_aggr_List = appendToTailOfList(new_aggr_List, bgfunc);
			Node *ubfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname_ub)));
			new_aggr_List = appendToTailOfList(new_aggr_List, ubfunc);
			Node *lbfunc = (Node *)createFunctionCall(SUM_FUNC_NAME, singleton(getAttrRefByName(proj, fname_lb)));
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
		pos++;
	}
	new_aggr_List = appendToTailOfList(new_aggr_List, (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(proj, ROW_CERTAIN))));
	new_aggr_List = appendToTailOfList(new_aggr_List, (Node *)createFunctionCall(MAX_FUNC_NAME, singleton(getAttrRefByName(proj, ROW_BESTGUESS))));
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

	// INFO_LOG("BGLIST: %s", nodeToString(new_groupby_list));

	// ASSERT(1==0);

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
	((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = DT_INT;
	pos++;
	((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = DT_INT;
	pos++;
	((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = DT_INT;
	pos++;
	FOREACH(Node, n, aggr_groupby_list){
		DataType dt = ((AttributeReference *)n)->attrType;
		INFO_LOG("dt for %s is %d", ((AttributeReference *)n)->name, dt);
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
		((AttributeDef *)getNthOfListP((aggrop->schema)->attrDefs, pos))->dataType = dt;
		pos++;
	}

	// ASSERT(1==0);

	INFO_LOG("FIXED DataTypes: %s", nodeToString(getDataTypes(aggrop->schema)));

	INFO_OP_LOG("Range Aggregation with groupby - rewrite aggregation:", aggrop);

	//Rearrange attribute position
//	INFO_LOG("%s", stringListToString(aggr_out_names));

	pos = 0;
	List *proj_bg = NIL;
	List *proj_bd = NIL;
	List *name_bg = NIL;

	FOREACH(Node, n, aggrl){
		INFO_LOG("Aggregation attr: %s", nodeToString(n));
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
		INFO_LOG("groupby attr: %s", nodeToString(n));
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

	List *projlist = getProjExprsForAllAttrs(finalproj);

	FOREACH(Node, n, projlist)
	{
		addRangeAttrToSchema(hmp, finalproj, n);
	}

	addRangeRowToSchema(hmp, finalproj);

	setStringProperty(finalproj, UNCERT_MAPPING_PROP, (Node *)hmp);
	markUncertAttrsAsProv(finalproj);

	switchSubtrees(op, finalproj);

	return finalproj;
}

static void
markUncertAttrsAsProv(QueryOperator *op)
{
	HashMap *hmp = (HashMap *) getStringProperty(op, UNCERT_MAPPING_PROP);
	Set *uncertAttrs = STRSET();
	int pos = 0;

	INFO_LOG("MARK ATTRIBUTES AS UNCERTAIN FOR: \n%s",
			  singleOperatorToOverview(op));

	FOREACH_HASH(Node,n,hmp)
	{
		if(isA(n,List))
		{
			List *l = (List *) n;
			AttributeReference *la = (AttributeReference *) (getHeadOfListP(l));
			AttributeReference *ra = (AttributeReference *) getNthOfListP(l, 1);
			addToSet(uncertAttrs, la->name);
			addToSet(uncertAttrs, ra->name);
		}
	}

	INFO_LOG("Uncertainty Attributes: %s",
			  nodeToString(uncertAttrs));

	FOREACH(AttributeDef,a,op->schema->attrDefs)
	{
		INFO_LOG("check attribute %s", a->attrName);
		if(streq(a->attrName,ROW_CERTAIN) ||
		   streq(a->attrName,ROW_BESTGUESS) ||
		   streq(a->attrName,ROW_POSSIBLE) ||
		   hasSetElem(uncertAttrs, a->attrName)
			)
		{
			op->provAttrs = appendToTailOfListInt(op->provAttrs, pos);
		}
		pos++;
	}

	DEBUG_LOG("after marking attributes as uncertain: \n%s",
			  singleOperatorToOverview(op));
}


static QueryOperator *
rewrite_UncertDuplicateRemoval(QueryOperator *op, boolean attrLevel)
{
	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - DuplicateRemoval");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//record original schema info
	List *projExpr = getProjExprsForAllAttrs(OP_LCHILD(op));
	List *attrName = getQueryOperatorAttrNames(OP_LCHILD(op));

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
	HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);

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
	List *projExpr2 = getProjExprsForAllAttrs(aggrOp);

	List *exprInput = sublist(projExpr2, aggrList->length, projExpr2->length-1);
	projExpr2 = sublist(projExpr2, 0, aggrList->length-1);
	exprInput = concatTwoLists(exprInput, projExpr2);

	QueryOperator *projOp =	(QueryOperator *)createProjectionOp(exprInput, aggrOp, NIL, attrName);
	aggrOp->parents = appendToTailOfList(aggrOp->parents, projOp);
	switchSubtrees(op, projOp);

	projExpr = getProjExprsForAllAttrs(projOp);

	FOREACH(Node, nd, projExpr){
		addUncertAttrToSchema(hmp, projOp, nd);
	}
	addUncertAttrToSchema(hmp, projOp, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(projOp, UNCERT_MAPPING_PROP, (Node *)hmp);

	LOG_RESULT_UNCERT("UNCERTAIN: Rewritten Operator tree [DUPLICATE REMOVAL]", op);

	return projOp;
}

static QueryOperator *
rewrite_UncertAggregation(QueryOperator *op, boolean attrLevel)
{
	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Aggregation");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//record original schema info
	List *projExpr = getProjExprsForAllAttrs(OP_LCHILD(op));
	List *attrName = getQueryOperatorAttrNames(OP_LCHILD(op));

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
		setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

		return proj;
	}


	HashMap* hmp = NEW_MAP(Node, Node);
	HashMap* hmpProj = NEW_MAP(Node, Node);
	HashMap* hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);

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
	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmpProj);

	//add uncertainty to Aggregation

	List *attrUaggr = NIL;
	projExpr = getProjExprsForAllAttrs(op);

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
	setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

	//create project to reorder the attributes when groupby.
	if(((AggregationOperator *)op)->groupBy){
		HashMap *hmp3 = NEW_MAP(Node, Node);
		projExpr = getProjExprsForAllAttrs(op);
		attrName = getQueryOperatorAttrNames(op);
		int uncertl = LIST_LENGTH(projExpr)-argl-grpl;
		putMidListToEnd(projExpr, argl, argl+uncertl);
		putMidListToEnd(attrName, argl, argl+uncertl);
		attrName = sublist(attrName, 0, (attrName->length)-uncertl-1);

		QueryOperator *proj2 = (QueryOperator *)createProjectionOp(projExpr, op, NIL, attrName);
		switchSubtrees(op, proj2);
		op->parents = singleton(proj2);

		FOREACH(Node, nd, getProjExprsForAllAttrs(proj2))
		{
			addUncertAttrToSchema(hmp3, proj2, nd);
		}
		addUncertAttrToSchema(hmp3, proj2, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
		setStringProperty(proj2, UNCERT_MAPPING_PROP, (Node *)hmp3);
		return proj2;
	}

	LOG_RESULT_UNCERT("UNCERTAIN: Rewritten Operator tree [AGGREGATION]", op);
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

	List *nan = getQueryOperatorAttrNames(op);
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
		setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

		return proj;
	}

	hmp =(HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
	List *dt1 = getDataTypes(OP_LCHILD(op)->schema);
	List *n1 = getAttrNames(OP_LCHILD(op)->schema);
	List *dt2 = getDataTypes(OP_RCHILD(op)->schema);
	List *n2 = getAttrNames(OP_RCHILD(op)->schema);
	int divider = dt1->length;

	List *jdt = concatTwoLists(dt1, dt2);
	List *jn = concatTwoLists(n1, n2);
	jn = removeFromTail(jn);
	op->schema = createSchemaFromLists(op->schema->name, jn, jdt);

	List *attrExpr = getProjExprsForAllAttrs(op);
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
	setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

	//INFO_LOG("Join HashMap: \n%s", nodeToString(hmp));

	//add projection
	attrExpr = getProjExprsForAllAttrs(op);

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
	List *attrExpr2 = getProjExprsForAllAttrs(proj);
	FOREACH(Node, nd, attrExpr2){
		addUncertAttrToSchema(hmp2, proj, nd);
	}
	addUncertAttrToSchema(hmp2, proj, (Node *) createAttributeReference(UNCERTAIN_ROW_ATTR));

	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp2);
	LOG_RESULT_UNCERT("UNCERTAIN: Rewritten Operator tree [JOIN]", op);

	return proj;

}

static QueryOperator *
rewrite_RangeJoin(QueryOperator *op){
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	if(((JoinOperator*)op)->cond && getBoolOption(RANGE_OPTIMIZE_JOIN)){
		return rewrite_RangeJoinOptimized(op);
	}

	int divider = getQueryOperatorAttrNames(OP_LCHILD(op))->length;

	List * nan = getQueryOperatorAttrNames(op);
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
	// INFO_LOG("n1 ====== %s", stringListToString(n1));

	// rewrite child first
	rewriteRange(OP_LCHILD(op));
	rewriteRange(OP_RCHILD(op));

	INFO_LOG("REWRITE-RANGE - Join");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));


	List *dt1 = getDataTypes(OP_LCHILD(op)->schema);
	List *dt2 = getDataTypes(OP_RCHILD(op)->schema);
	List *jdt = concatTwoLists(dt1, dt2);

	op->schema = createSchemaFromLists(op->schema->name, n1, jdt);


	//add projection

	//projection list

	int divider2 = divider*3+3;

	List *attrExpr = getProjExprsForAllAttrs(op);
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

//	setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

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
	List *attrExpr2 = getProjExprsForAllAttrs(proj);
	FOREACH(Node, nd, attrExpr2){
		addRangeAttrToSchema(hmp2, proj, nd);
	}
	addRangeRowToSchema(hmp2, proj);
	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp2);
	markUncertAttrsAsProv(proj);

//	INFO_LOG("CT: %s", nodeToString(CT));
//	INFO_LOG("BG: %s", nodeToString(BG));
//	INFO_LOG("PS: %s", nodeToString(PS));

	//rewrite join condition
	if(((JoinOperator*)op)->joinType == JOIN_INNER && ((JoinOperator*)op)->cond){

		//hashmap for join condition rewriting into upper bound.
		//notice that upper bound need to replace original join condition in join operator.
		//And original join condition need to multiply with best guess in projection operator.
		//Rewrite join condition into lower bound to multiply with certain in projection operator.

		HashMap * hmpl = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
		HashMap * hmpr = (HashMap *)getStringProperty(OP_RCHILD(op), UNCERT_MAPPING_PROP);
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

	LOG_RESULT_UNCERT("UNCERTAIN RANGE: Rewritten Operator tree [JOIN]", proj);

	return proj;
}

static QueryOperator *
spliceToBG(QueryOperator *op){
	List *attrnames = getNormalAttrNames(op);
	// INFO_LOG("GETNAMES: %s, INPUTNAMES: %s", stringListToString(getNormalAttrNames(op)), stringListToString(attrnames));
	Operator *bgSel = createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(op,ROW_BESTGUESS), createConstInt(0)));

	QueryOperator *bg = (QueryOperator *)createSelectionOp((Node *)bgSel, op, NIL, getQueryOperatorAttrNames(op));
	switchSubtrees(op, bg);
	op->parents = singleton(bg);
	setStringProperty(bg, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(op, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(bg);

	//best guess projections
	List *normalProjList = NIL;
	List *rangeProjList = NIL;
	Node *ctcase = NULL;
	FOREACH(char, an, attrnames){
		if(!ctcase){
			ctcase = (Node *)createOpExpr(OPNAME_EQ,LIST_MAKE(getAttrRefByName(bg,getUBString(an)),getAttrRefByName(bg,getLBString(an))));
		}
		else {
			Node *temp = (Node *)createOpExpr(OPNAME_EQ,LIST_MAKE(getAttrRefByName(bg,getUBString(an)),getAttrRefByName(bg,getLBString(an))));
			ctcase = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(ctcase,temp));
		}
		normalProjList = appendToTailOfList(normalProjList,getAttrRefByName(bg,an));
		rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg,an));
		rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg,an));
	}
	CaseWhen * cwhen = createCaseWhen(ctcase, (Node *)getAttrRefByName(bg, ROW_CERTAIN));
	Node *ctProj = (Node *)createCaseExpr(NULL, singleton(cwhen), (Node *)createConstInt(0));
	rangeProjList = appendToTailOfList(rangeProjList,ctProj);
	rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg, ROW_BESTGUESS));
	rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg, ROW_BESTGUESS));

	QueryOperator *bgProj = (QueryOperator *)createProjectionOp(concatTwoLists(normalProjList,rangeProjList), bg, NIL, getQueryOperatorAttrNames(bg));
	switchSubtrees(bg,bgProj);
	bg->parents = singleton(bgProj);
	setStringProperty(bgProj, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(bg, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(bgProj);

	return bgProj;
}

static QueryOperator *
spliceToBGAggr(QueryOperator *op){
	List *attrnames = getNormalAttrNames(op);
	// INFO_LOG("GETNAMES: %s, INPUTNAMES: %s", stringListToString(getNormalAttrNames(op)), stringListToString(attrnames));
	Operator *bgSel = createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(op,ROW_BESTGUESS), createConstInt(0)));

	QueryOperator *bg = (QueryOperator *)createSelectionOp((Node *)bgSel, op, NIL, getQueryOperatorAttrNames(op));
	switchSubtrees(op, bg);
	op->parents = singleton(bg);
	setStringProperty(bg, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(op, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(bg);

	//best guess projections
	List *normalProjList = NIL;
	List *rangeProjList = NIL;
	// Node *ctcase = NULL;
	FOREACH(char, an, attrnames){
		normalProjList = appendToTailOfList(normalProjList,getAttrRefByName(bg,an));
		rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg,getUBString(an)));
		rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg,getLBString(an)));
	}
	rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg, ROW_CERTAIN));
	rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg, ROW_BESTGUESS));
	rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(bg, ROW_BESTGUESS));

	QueryOperator *bgProj = (QueryOperator *)createProjectionOp(concatTwoLists(normalProjList,rangeProjList), bg, NIL, getQueryOperatorAttrNames(bg));
	switchSubtrees(bg,bgProj);
	bg->parents = singleton(bgProj);
	setStringProperty(bgProj, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(bg, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(bgProj);

	return bgProj;
}

static void duplicateMinMaxNameProp(QueryOperator *from, QueryOperator *to){
	if (HAS_STRING_PROP(from, PROP_STORE_MIN_MAX_ATTRS))
	{
		Set *dep = copyObject((Set *)getStringProperty(from, PROP_STORE_MIN_MAX_ATTRS));
		// INFO_LOG("Pushed name prop to rewritten op: %s", nodeToString(dep));
		setStringProperty(to, PROP_STORE_MIN_MAX_ATTRS, (Node *)dep);
	}
}

static void duplicateMinMaxResProp(QueryOperator *from, QueryOperator *to){
	if (HAS_STRING_PROP(from, PROP_STORE_MIN_MAX))
	{
		HashMap *dep = copyObject((HashMap *)getStringProperty(from, PROP_STORE_MIN_MAX));
		// INFO_LOG("Pushed minmax prop to rewritten op: %s", nodeToString(dep));
		setStringProperty(to, PROP_STORE_MIN_MAX, (Node *) dep);
	}
}

static QueryOperator *
spliceToPOS(QueryOperator *op, char *jattr){
	//possible projections
	List *attrnames = getNormalAttrNames(op);
	List *normalProjList = NIL;
	List *rangeProjList = NIL;

	FOREACH(char, an, attrnames){
		normalProjList = appendToTailOfList(normalProjList,getAttrRefByName(op,an));
		rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(op,getUBString(an)));
		rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(op,getLBString(an)));
	}

	rangeProjList = appendToTailOfList(rangeProjList,(Node *)createConstInt(0));
	rangeProjList = appendToTailOfList(rangeProjList,(Node *)createConstInt(0));
	rangeProjList = appendToTailOfList(rangeProjList,getAttrRefByName(op, ROW_BESTGUESS));

	// INFO_LOG("list: %s", nodeToString(normalProjList));
	// INFO_LOG("list: %s", nodeToString(rangeProjList));

	QueryOperator *posProj = (QueryOperator *)createProjectionOp(concatTwoLists(normalProjList,rangeProjList), op, NIL, getQueryOperatorAttrNames(op));
	switchSubtrees(op,posProj);
	op->parents = singleton(posProj);
	setStringProperty(posProj, UNCERT_MAPPING_PROP,
					  copyObject(getStringProperty(op, UNCERT_MAPPING_PROP)));
	markUncertAttrsAsProv(posProj);

	// remove property (input may have been rewritten so it may be unsafe to reuse)
	// removeMinMaxProps(posProj);

	Set* attrset = MAKE_STR_SET(jattr);

	if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
	{
		attrset = unionSets(attrset, copyObject((Set *)getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS)));
	}
	INFO_LOG("[Splice to possible] computeminmax on attrset: %s", nodeToString(attrset));
	computeMinMaxPropForSubset(posProj, attrset);

	// HashMap * mmpro = (HashMap *)getStringProperty(posProj, PROP_STORE_MIN_MAX);
	// INFO_LOG("property: %s", nodeToString(mmpro));
	// setStringProperty(posProj, PROP_STORE_MIN_MAX, (Node *) mmpro);

	INFO_OP_LOG("posproj:", posProj);

	//compress possibles
	int iter = getIntOption(RANGE_COMPRESSION_RATE);
	QueryOperator *compposProj = compressPosRow(posProj, iter, jattr);

	INFO_OP_LOG("compressed possible:", compposProj);

	duplicateMinMaxResProp(posProj, compposProj);

	return compposProj;
}

static QueryOperator *
rewrite_RangeJoinOptimized(QueryOperator *op){
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	//get output names for join
	int divider = getQueryOperatorAttrNames(OP_LCHILD(op))->length;
	List * nan = getQueryOperatorAttrNames(op);
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

	List *lattrnamesRename = deepCopyStringList(nan1);
	List *rattrnamesRename = deepCopyStringList(nan2);

	List *lattrnames = getQueryOperatorAttrNames(OP_LCHILD(op));
	List *rattrnames = getQueryOperatorAttrNames(OP_RCHILD(op));

	List *joinNames = CONCAT_LISTS(deepCopyStringList(nan1),deepCopyStringList(nan1r),deepCopyStringList(range_row1),deepCopyStringList(nan2),deepCopyStringList(nan2r),deepCopyStringList(range_row2));

	List *alist = CONCAT_LISTS(deepCopyStringList(nan1),deepCopyStringList(nan2),deepCopyStringList(nan1r),deepCopyStringList(nan2r),deepCopyStringList(range_row1));

	//push needed attributes to childs

	List *attpair = getJoinAttrPair(((JoinOperator*)op)->cond);

	Set *ldep = MAKE_STR_SET(getHeadOfListP(attpair));
	Set *rdep = MAKE_STR_SET(getTailOfListP(attpair));
	Set *jminmax = unionSets(copyObject(ldep),copyObject(rdep));
	if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
	{
		Set *pminmax = (Set *) getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS);
		Set *lcattr = makeStrSetFromList(getQueryOperatorAttrNames(OP_LCHILD(op)));
		Set *rcattr = makeStrSetFromList(getQueryOperatorAttrNames(OP_RCHILD(op)));
		FOREACH_SET(char, c, pminmax){
			if(hasSetElem(lcattr,c)){
				addToSet(ldep, c);
			}
			if(hasSetElem(rcattr,c)){
				addToSet(rdep, c);
			}
		}
		// rdep = unionSets(rdep, copyObject((Set *) getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS)));
		jminmax = unionSets(jminmax, copyObject((Set *) getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS)));
	}
	// INFO_LOG("MINMAX for l_child: %s", nodeToString(ldep));
	// INFO_LOG("MINMAX for r_child: %s", nodeToString(rdep));
	// ldep = getInputSchemaDependencies(OP_LCHILD(op), ldep, TRUE);
	// rdep = getInputSchemaDependencies(OP_RCHILD(op), rdep, FALSE);
	INFO_LOG("[Join] Pushing minmax prop attr to lchild: %s", nodeToString(ldep));
	INFO_LOG("[Join] Pushing minmax prop attr to rchild: %s", nodeToString(rdep));
	setStringProperty(OP_LCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)ldep);
	setStringProperty(OP_RCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)rdep);

	//get minmax prop before rewriting
	// computeMinMaxPropForSubset(op, jminmax);

	// rewrite child first
	rewriteRange(OP_LCHILD(op));
	rewriteRange(OP_RCHILD(op));

	INFO_LOG("REWRITE-RANGE - Join(optimized)");

	//divide into bg and possible parts
	QueryOperator *lop = OP_LCHILD(op);
	QueryOperator *lopdup = copyObject(lop);
	QueryOperator *rop = OP_RCHILD(op);
	QueryOperator *ropdup = copyObject(rop);

	QueryOperator *lbg = NULL;
	QueryOperator *rbg = NULL;
	QueryOperator *lpos = NULL;
	QueryOperator *rpos = NULL;

	if(HAS_STRING_PROP(lop, PROP_STORE_POSSIBLE_TREE)){
		lbg = lop;
		lpos = (QueryOperator *)GET_STRING_PROP(lop, PROP_STORE_POSSIBLE_TREE);
	} else {
		lbg = spliceToBG(lop);
		lpos = spliceToPOS(lopdup,getHeadOfListP(attpair));
	}
	if(HAS_STRING_PROP(rop, PROP_STORE_POSSIBLE_TREE)){
		rbg = rop;
		rpos = (QueryOperator *)GET_STRING_PROP(rop, PROP_STORE_POSSIBLE_TREE);
	} else {
		rbg = spliceToBG(rop);
		rpos = spliceToPOS(ropdup,getTailOfListP(attpair));
	}

	INFO_OP_LOG("lbg:", lbg);
	printECPro(lpos);
	INFO_OP_LOG("rbg:", rbg);
	printECPro(rpos);

	// INFO_OP_LOG("lpos:", lpos);
	// INFO_OP_LOG("rpos:", rpos);

	//best guess join
	// List *latt = copyList(lattrnames);
	// List *ratt = copyList(rattrnames);
	// FOREACH(char, an, lattrnames){
	// 	latt = appendToTailOfList(latt, getUBString(an));
	// 	latt = appendToTailOfList(latt, getLBString(an));
	// }
	// FOREACH(char, an, rattrnames){
	// 	ratt = appendToTailOfList(ratt, getUBString(an));
	// 	ratt = appendToTailOfList(ratt, getLBString(an));
	// }
	// List *range_row1 = LIST_MAKE(ROW_CERTAIN,ROW_BESTGUESS,ROW_POSSIBLE);
	// List *range_row2 = LIST_MAKE(ROW_CERTAIN_TWO,ROW_BESTGUESS_TWO,ROW_POSSIBLE_TWO);
	// List *alist = CONCAT_LISTS(latt,range_row1,ratt,range_row2);

	// List *pjlist = copyList(alist);
	// INFO_LOG("BG join attr List: %s", stringListToString(alist));

	int offset = (lattrnames->length)*3+3;
	Node *condExpr = (Node *)copyObject(((JoinOperator*)op)->cond);
	Node *bgExpr = getOutputExprFromInput(copyObject(condExpr), offset);

	QueryOperator *bgjoin = (QueryOperator *)createJoinOp(JOIN_INNER, ((JoinOperator*)op)->cond, LIST_MAKE(lbg,rbg), NIL, joinNames);
	lbg->parents = singleton(bgjoin);
	rbg->parents = singleton(bgjoin);
	INFO_OP_LOG("bg join: ", bgjoin);

	//projection to rearrange attributes
	List *bgNprojList = NIL;
	List *bgRprojList = NIL;
	FOREACH(char, an, lattrnamesRename){
		bgNprojList = appendToTailOfList(bgNprojList, getAttrRefByName(bgjoin, an));
		bgRprojList = appendToTailOfList(bgRprojList, getAttrRefByName(bgjoin, getUBString(an)));
		bgRprojList = appendToTailOfList(bgRprojList, getAttrRefByName(bgjoin, getLBString(an)));
	}
	FOREACH(char, an, rattrnamesRename){
		bgNprojList = appendToTailOfList(bgNprojList, getAttrRefByName(bgjoin, an));
		bgRprojList = appendToTailOfList(bgRprojList, getAttrRefByName(bgjoin, getUBString(an)));
		bgRprojList = appendToTailOfList(bgRprojList, getAttrRefByName(bgjoin, getLBString(an)));
	}
	Node *opcet = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(bgjoin, ROW_CERTAIN),getAttrRefByName(bgjoin, ROW_CERTAIN_TWO)));
	Node *opbg = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(bgjoin, ROW_BESTGUESS),getAttrRefByName(bgjoin, ROW_BESTGUESS_TWO)));
	Node *oppos = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(bgjoin, ROW_POSSIBLE),getAttrRefByName(bgjoin, ROW_POSSIBLE_TWO)));
	bgRprojList = appendToTailOfList(bgRprojList, opcet);
	bgRprojList = appendToTailOfList(bgRprojList, opbg);
	bgRprojList = appendToTailOfList(bgRprojList, oppos);
	List *bgproj = CONCAT_LISTS(bgNprojList,bgRprojList);
 	INFO_LOG("BG proj List: %s", stringListToString(alist));

	QueryOperator *bgProj = (QueryOperator *)createProjectionOp(bgproj,bgjoin,NIL,alist);
	bgjoin->parents = singleton(bgProj);

	//possible join

	QueryOperator *posjoin = (QueryOperator *)createJoinOp(JOIN_INNER, NULL, LIST_MAKE(lpos,rpos), NIL, joinNames);
	lpos->parents = singleton(posjoin);
	rpos->parents = singleton(posjoin);

	//prepair hashmaps for expression bounds
	HashMap * hmpin = NEW_MAP(Node, Node); //hashmap with intput of join
	HashMap * hmpout = NEW_MAP(Node, Node); //hashmap with output of join
	FOREACH(char, an, lattrnames){
		Node *key = (Node *)getAttrRefByName(lop,an);
		((AttributeReference *)key)->outerLevelsUp = 0;
		Node *val = (Node *)LIST_MAKE(getAttrRefByName(lop,getUBString(an)),getAttrRefByName(lop,getLBString(an)));
		ADD_TO_MAP(hmpin, createNodeKeyValue(key, val));
	}
	FOREACH(char, an, rattrnames){
		AttributeReference *key = getAttrRefByName(rop,an);
		key->outerLevelsUp = 0;
		key->fromClauseItem = 1;
		AttributeReference *ub = getAttrRefByName(rop,getUBString(an));
		AttributeReference *lb = getAttrRefByName(rop,getLBString(an));
		ub->fromClauseItem = 1;
		lb->fromClauseItem = 1;
		Node *val = (Node *)LIST_MAKE(ub,lb);
		ADD_TO_MAP(hmpin, createNodeKeyValue((Node *)key, val));
	}
	// INFO_LOG("map_in: %s", nodeToString(hmpin));

	FOREACH(char, an, lattrnamesRename){
		Node *key = (Node *)getAttrRefByName(posjoin,an);
		((AttributeReference *)key)->outerLevelsUp = 0;
		Node *val = (Node *)LIST_MAKE(getAttrRefByName(posjoin,getUBString(an)),getAttrRefByName(posjoin,getLBString(an)));
		ADD_TO_MAP(hmpout, createNodeKeyValue(key, val));
	}
	FOREACH(char, an, rattrnamesRename){
		AttributeReference *key = getAttrRefByName(posjoin,an);
		key->outerLevelsUp = 0;
		AttributeReference *ub = getAttrRefByName(posjoin,getUBString(an));
		AttributeReference *lb = getAttrRefByName(posjoin,getLBString(an));
		Node *val = (Node *)LIST_MAKE(ub,lb);
		ADD_TO_MAP(hmpout, createNodeKeyValue((Node *)key, val));
	}
	// INFO_LOG("map_out: %s", nodeToString(hmpout));

	// INFO_LOG("bg expr: %s", nodeToString(bgExpr));
	Node *ubExpr = getUBExpr(copyObject(condExpr), hmpin);
	((JoinOperator *)posjoin)->cond = ubExpr;

	INFO_OP_LOG("pos join: ", posjoin);

	// INFO_LOG("ub expr: %s", nodeToString(ubExpr));

	Node *lbExpr = getLBExpr(copyObject(bgExpr),hmpout);
	INFO_LOG("lb expr: %s", nodeToString(lbExpr));

	//projection to rearrange attributes
	List *posNprojList = NIL;
	List *posRprojList = NIL;
	FOREACH(char, an, lattrnamesRename){
		posNprojList = appendToTailOfList(posNprojList, getAttrRefByName(posjoin, an));
		posRprojList = appendToTailOfList(posRprojList, getAttrRefByName(posjoin, getUBString(an)));
		posRprojList = appendToTailOfList(posRprojList, getAttrRefByName(posjoin, getLBString(an)));
	}
	FOREACH(char, an, rattrnamesRename){
		posNprojList = appendToTailOfList(posNprojList, getAttrRefByName(posjoin, an));
		posRprojList = appendToTailOfList(posRprojList, getAttrRefByName(posjoin, getUBString(an)));
		posRprojList = appendToTailOfList(posRprojList, getAttrRefByName(posjoin, getLBString(an)));
	}
	// opcet = (Node *)createFunctionCall(LEAST_FUNC_NAME, LIST_MAKE(getAttrRefByName(posjoin, ROW_CERTAIN),getAttrRefByName(posjoin, ROW_CERTAIN_TWO)));
	// opbg = (Node *)createFunctionCall(LEAST_FUNC_NAME, LIST_MAKE(getAttrRefByName(posjoin, ROW_BESTGUESS),getAttrRefByName(posjoin, ROW_BESTGUESS_TWO)));
	// oppos = (Node *)createFunctionCall(LEAST_FUNC_NAME, LIST_MAKE(getAttrRefByName(posjoin, ROW_POSSIBLE),getAttrRefByName(posjoin, ROW_POSSIBLE_TWO)));
	// opcet = (Node *)createFunctionCall(LEAST_FUNC_NAME, LIST_MAKE(opcet,(Node *)createCaseOperator(lbExpr)));
	// opbg = (Node *)createFunctionCall(LEAST_FUNC_NAME, LIST_MAKE(opbg,(Node *)createCaseOperator(bgExpr)));
	opcet = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(posjoin, ROW_CERTAIN),getAttrRefByName(posjoin, ROW_CERTAIN_TWO)));
	opbg = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(posjoin, ROW_BESTGUESS),getAttrRefByName(posjoin, ROW_BESTGUESS_TWO)));
	oppos = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(posjoin, ROW_POSSIBLE),getAttrRefByName(posjoin, ROW_POSSIBLE_TWO)));
	opcet = (Node *)createOpExpr("*", LIST_MAKE(opcet,(Node *)createCaseOperator(lbExpr)));
	opbg = (Node *)createOpExpr("*", LIST_MAKE(opbg,(Node *)createCaseOperator(bgExpr)));
	// opcet = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(posjoin, ROW_CERTAIN),getAttrRefByName(posjoin, ROW_CERTAIN_TWO)));
	// opbg = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(posjoin, ROW_BESTGUESS),getAttrRefByName(posjoin, ROW_BESTGUESS_TWO)));
	// oppos = (Node *)createOpExpr("*", LIST_MAKE(getAttrRefByName(posjoin, ROW_POSSIBLE),getAttrRefByName(posjoin, ROW_POSSIBLE_TWO)));
	// opcet = (Node *)createOpExpr("*", LIST_MAKE(opcet,(Node *)createCaseOperator(lbExpr)));
	// opbg = (Node *)createOpExpr("*", LIST_MAKE(opbg,(Node *)createCaseOperator(bgExpr)));

	posRprojList = appendToTailOfList(posRprojList, opcet);
	posRprojList = appendToTailOfList(posRprojList, opbg);
	posRprojList = appendToTailOfList(posRprojList, oppos);
	List *posproj = CONCAT_LISTS(posNprojList,posRprojList);

	QueryOperator *posProj = (QueryOperator *)createProjectionOp(posproj,posjoin,NIL,alist);
	posjoin->parents = singleton(posProj);

	//save pos to bg
	HashMap *hmpret = NEW_MAP(Node, Node);
	FOREACH(char, an, lattrnamesRename){
		Node *key = (Node *)getAttrRefByName(bgProj,an);
		((AttributeReference *)key)->outerLevelsUp = 0;
		Node *val = (Node *)LIST_MAKE(getAttrRefByName(bgProj,getUBString(an)),getAttrRefByName(bgProj,getLBString(an)));
		ADD_TO_MAP(hmpret, createNodeKeyValue(key, val));
	}
	FOREACH(char, an, rattrnamesRename){
		AttributeReference *key = getAttrRefByName(bgProj,an);
		key->outerLevelsUp = 0;
		AttributeReference *ub = getAttrRefByName(bgProj,getUBString(an));
		AttributeReference *lb = getAttrRefByName(bgProj,getLBString(an));
		Node *val = (Node *)LIST_MAKE(ub,lb);
		ADD_TO_MAP(hmpret, createNodeKeyValue((Node *)key, val));
	}
	ADD_TO_MAP(hmpret, createNodeKeyValue((Node *)createAttributeReference(ROW_CERTAIN), (Node *)getAttrRefByName(bgProj, ROW_CERTAIN)));
	ADD_TO_MAP(hmpret, createNodeKeyValue((Node *)createAttributeReference(ROW_BESTGUESS), (Node *)getAttrRefByName(bgProj, ROW_BESTGUESS)));
	ADD_TO_MAP(hmpret, createNodeKeyValue((Node *)createAttributeReference(ROW_POSSIBLE), (Node *)getAttrRefByName(bgProj, ROW_POSSIBLE)));

	// INFO_LOG("joinNames: %s", stringListToString(joinNames));
	// INFO_LOG("unionNames: %s", stringListToString(alist));

	// QueryOperator* ret = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(unionop),unionop,NIL,alist);
	// switchSubtrees(unionop, ret);
	// unionop->parents = singleton(ret);
	setStringProperty(bgProj, UNCERT_MAPPING_PROP, (Node *)hmpret);
	setStringProperty(posProj, UNCERT_MAPPING_PROP, (Node *)hmpret);
	markUncertAttrsAsProv(bgProj);
	markUncertAttrsAsProv(posProj);

	INFO_OP_LOG("bg proj: ", bgProj);
	INFO_OP_LOG("pos proj: ", posProj);

	setStringProperty(bgProj, PROP_STORE_POSSIBLE_TREE, (Node *)posProj);

	switchSubtrees(op, bgProj);

	return bgProj;

	//union

	// QueryOperator *unionop = (QueryOperator *)createSetOperator(SETOP_UNION, LIST_MAKE(bgProj, posProj), NIL, alist);
	// switchSubtrees(op, unionop);
	// bgProj->parents = singleton(unionop);
	// posProj->parents = singleton(unionop);


	// HashMap *hmpunion = NEW_MAP(Node, Node);
	// FOREACH(char, an, lattrnamesRename){
	// 	Node *key = (Node *)getAttrRefByName(unionop,an);
	// 	((AttributeReference *)key)->outerLevelsUp = 0;
	// 	Node *val = (Node *)LIST_MAKE(getAttrRefByName(unionop,getUBString(an)),getAttrRefByName(unionop,getLBString(an)));
	// 	ADD_TO_MAP(hmpunion, createNodeKeyValue(key, val));
	// }
	// FOREACH(char, an, rattrnamesRename){
	// 	AttributeReference *key = getAttrRefByName(unionop,an);
	// 	key->outerLevelsUp = 0;
	// 	AttributeReference *ub = getAttrRefByName(unionop,getUBString(an));
	// 	AttributeReference *lb = getAttrRefByName(unionop,getLBString(an));
	// 	Node *val = (Node *)LIST_MAKE(ub,lb);
	// 	ADD_TO_MAP(hmpunion, createNodeKeyValue((Node *)key, val));
	// }
	// ADD_TO_MAP(hmpunion, createNodeKeyValue((Node *)createAttributeReference(ROW_CERTAIN), (Node *)getAttrRefByName(unionop, ROW_CERTAIN)));
	// ADD_TO_MAP(hmpunion, createNodeKeyValue((Node *)createAttributeReference(ROW_BESTGUESS), (Node *)getAttrRefByName(unionop, ROW_BESTGUESS)));
	// ADD_TO_MAP(hmpunion, createNodeKeyValue((Node *)createAttributeReference(ROW_POSSIBLE), (Node *)getAttrRefByName(unionop, ROW_POSSIBLE)));

	// // INFO_LOG("joinNames: %s", stringListToString(joinNames));
	// // INFO_LOG("unionNames: %s", stringListToString(alist));

	// // QueryOperator* ret = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(unionop),unionop,NIL,alist);
	// // switchSubtrees(unionop, ret);
	// // unionop->parents = singleton(ret);
	// setStringProperty(unionop, UNCERT_MAPPING_PROP, (Node *)hmpunion);

	// return unionop;
}

static List*
getJoinAttrPair(Node *expr)
{
	Node *op = expr;
	ASSERT(op->type==T_Operator);
	ASSERT(strcmp(((Operator *)expr)->name,OPNAME_EQ)==0);
	Node *ref1 = getHeadOfListP(((Operator *)op)->args);
	Node *ref2 = getTailOfListP(((Operator *)op)->args);
	ASSERT(ref1->type == T_AttributeReference);
	ASSERT(ref2->type == T_AttributeReference);
	return LIST_MAKE(((AttributeReference *)ref1)->name,((AttributeReference *)ref2)->name);
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
    //HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
	if(attrLevel)
	{
		List *attrExpr = getProjExprsForAllAttrs(op);
		FOREACH(Node, nd, attrExpr){
        	addUncertAttrToSchema(hmp, op, nd);
		}
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

    //create projection to calculate row uncertainty
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(op), op, NIL, getQueryOperatorAttrNames(op));
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
    setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *) copyObject(hmp));

	LOG_RESULT_UNCERT("UNCERTAIN: Rewritten Operator tree [SELECTION]", op);

	return proj;
}

static QueryOperator *
rewrite_RangeSelection(QueryOperator *op)
{

	ASSERT(OP_LCHILD(op));

	// push down min max attr property if there are any
	if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
	{
		Set *dependency = (Set *)getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS);
		INFO_LOG("[Selection] Pushing minmax prop attr to child: %s", nodeToString(dependency));
		setStringProperty(OP_LCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)dependency);
	}

	// rewrite child first
    rewriteRange(OP_LCHILD(op));
    QueryOperator *pos = NULL;
    if(HAS_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE)){
    	pos = (QueryOperator *)GET_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE);
    }

    INFO_LOG("REWRITE-RANGE - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    //HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
    List *attrExpr = getProjExprsForAllAttrs(op);
    FOREACH(Node, nd, attrExpr){
        addRangeAttrToSchema(hmp, op, nd);
    }
    addRangeRowToSchema(hmp, op);
    setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);
    //modify selection condition to possible
    Node *cond = ((SelectionOperator *)op)->cond;
    Node *ubCond = getUBExpr(((SelectionOperator *)op)->cond, hmp);
    Node *lbCond = getLBExpr(((SelectionOperator *)op)->cond, hmp);
    ((SelectionOperator *)op)->cond = ubCond;
    //create projection to calculate CERT and BG
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(op), op, NIL, getQueryOperatorAttrNames(op));
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
    setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

    duplicateMinMaxNameProp(op, proj);

    if(pos){
    	QueryOperator *cpop = copyObject(op);
    	cpop->inputs = singleton(pos);
    	pos->parents = singleton(cpop);
    	QueryOperator *cpproj = copyObject(proj);
    	cpproj->inputs = singleton(cpop);
    	cpop->parents = singleton(cpproj);
    	cpproj->parents = NIL;
    	SET_STRING_PROP(proj, PROP_STORE_POSSIBLE_TREE, (Node *)cpproj);
    	INFO_OP_LOG("[Selection] REWRITTEN POS BRACH: ", cpproj);
    }

	markUncertAttrsAsProv(proj);
	LOG_RESULT_UNCERT("UNCERTAIN RANGE: Rewritten Operator tree [SELECTION]", proj);

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
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
    //INFO_LOG("HashMap: %s", nodeToString((Node *)hmpIn));
	if (attrLevel)
	{
		List *attrExpr = getProjExprsForAllAttrs(op);
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
    setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

	LOG_RESULT_UNCERT("UNCERTAIN: Rewritten Operator tree [PROJECTION]", op);
    //INFO_LOG("ProjList: %s", nodeToString((Node *)(((ProjectionOperator *)op)->projExprs)));
    return op;
}

static QueryOperator *
rewrite_RangeProjection(QueryOperator *op)
{
    ASSERT(OP_LCHILD(op));

    // push down min max attr property if there are any
	if (HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_ATTRS))
	{
		Set *dependency = (Set *)getStringProperty(op, PROP_STORE_MIN_MAX_ATTRS);
		// removeStringProperty(op, PROP_STORE_MIN_MAX_ATTRS);
		Set *newd = getInputSchemaDependencies(op, dependency, TRUE);
		INFO_OP_LOG("[Projection] minmax prop piushing to child:", op);
		INFO_LOG("[Projection] Pushing minmax prop attr %s to child as: %s", nodeToString(dependency), nodeToString(newd));
		// setStringProperty(op, PROP_STORE_MIN_MAX_ATTRS, (Node *)newd);
		setStringProperty(OP_LCHILD(op), PROP_STORE_MIN_MAX_ATTRS, (Node *)newd);
	}

    //rewrite child first
    rewriteRange(OP_LCHILD(op));
    QueryOperator *pos = NULL;
    if(HAS_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE)){
    	pos = (QueryOperator *)GET_STRING_PROP(OP_LCHILD(op), PROP_STORE_POSSIBLE_TREE);
    }

    INFO_LOG("REWRITE-RANGE - Projection");
    INFO_OP_LOG("Operator tree ", op);

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
    List *attrExpr = getProjExprsForAllAttrs(op);
   	INFO_LOG("%s", nodeToString(((ProjectionOperator *)op)->projExprs));
   	INFO_LOG("Rangeprojection hashmaps: %s", nodeToString(hmpIn));
    List *uncertlist = NIL;
    int ict = 0;
    FOREACH(Node, nd, attrExpr){
        addRangeAttrToSchema(hmp, op, nd);
        Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs,ict);
        INFO_LOG("Proj: %s", nodeToString(projexpr));
        // Node *ubExpr = getUBExprByName(projexpr, hmpIn, OP_LCHILD(op));
        Node *ubExpr = getUBExpr(projexpr, hmpIn);
        INFO_LOG("Ub: %s", nodeToString(ubExpr));
        // Node *lbExpr = getLBExprByName(projexpr, hmpIn, OP_LCHILD(op));
        Node *lbExpr = getLBExpr(projexpr, hmpIn);
        INFO_LOG("Lb: %s", nodeToString(lbExpr));
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
    setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);
	markUncertAttrsAsProv(op);

    // INFO_LOG("ProjList: %s", nodeToString((Node *)(((ProjectionOperator *)op)->projExprs)));
    if(pos){
    	QueryOperator *cpop = copyObject(op);
    	cpop->inputs = singleton(pos);
    	pos->parents = singleton(cpop);
    	cpop->parents = NIL;
    	SET_STRING_PROP(op, PROP_STORE_POSSIBLE_TREE, (Node *)cpop);
    	INFO_OP_LOG("[PROJECTION] REWRITTEN POS BRACH: ", cpop);
    }


	LOG_RESULT_UNCERT("UNCERTAIN RANGE: Rewritten Operator tree [PROJECTION]", op);

    return op;
}

static QueryOperator *
rewrite_UncertTableAccess(QueryOperator *op, boolean attrLevel)
{
	INFO_LOG("REWRITE-UNCERT - TableAccess (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	HashMap * hmp = NEW_MAP(Node, Node);

	QueryOperator *proj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(op), op, NIL, getQueryOperatorAttrNames(op));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);

	if (attrLevel)
	{
		List *attrExpr = getProjExprsForAllAttrs(op);
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
		List *nexpr = getProjExprsForAllAttrs(op);
		//INFO_LOG("nexpr %s", nodeToString(nexpr));
		((ProjectionOperator *)proj)->projExprs = concatTwoLists(nexpr, pexpr);
	} else {
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	}
	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);
	//INFO_LOG("HashMap: %s", nodeToString((Node *)hmp));
	LOG_RESULT_UNCERT("UNCERTAIN RANGE: Rewritten Operator tree [TABLE ACCESS]", op);

	return proj;
}

static QueryOperator *
rewrite_RangeTableAccess(QueryOperator *op)
{
	INFO_LOG("REWRITE-RANGE - TableAccess");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));
	HashMap * hmp = NEW_MAP(Node, Node);
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getQueryOperatorAttrNames(op));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);
	List *attrExpr = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr)
	{
		addRangeAttrToSchema(hmp, proj, nd);
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
	}
	addRangeRowToSchema(hmp, proj);
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);
	markUncertAttrsAsProv(proj);
//	INFO_LOG("HashMap: %s", nodeToString((Node *)hmp));

	if(HAS_STRING_PROP(op,PROP_HAS_RANGE)){
		INFO_LOG("TableAccess - HAS_RANGE");
		List *pexpr = getProvAttrProjectionExprs(op);
		//INFO_LOG("pexpr %s", nodeToString(pexpr));
		List *nexpr = getNormalAttrProjectionExprs(op);
		//INFO_LOG("nexpr %s", nodeToString(nexpr));
		((ProjectionOperator *)proj)->projExprs = concatTwoLists(nexpr, pexpr);

		duplicateMinMaxNameProp(op, proj);
	}

	return proj;
}

static void
addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef)
{
	addAttrToSchema(target, getUncertString(((AttributeReference *)aRef)->name), DT_INT);
	((AttributeReference *)aRef)->outerLevelsUp = 0;
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
}

static void
addRangeAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef)
{
	((AttributeReference *)aRef)->outerLevelsUp = 0;
	addAttrToSchema(target, getUBString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
	List *refs = singleton((Node *)getTailOfListP(getProjExprsForAllAttrs(target)));
	addAttrToSchema(target, getLBString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
	appendToTailOfList(refs, (Node *)getTailOfListP(getProjExprsForAllAttrs(target)));
	//Map each attribute to their upper&lower bounds list
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)refs));
}

static void
addRangeRowToSchema(HashMap *hmp, QueryOperator *target)
{
	addAttrToSchema(target, ROW_CERTAIN, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_CERTAIN), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
	addAttrToSchema(target, ROW_BESTGUESS, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_BESTGUESS), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
	addAttrToSchema(target, ROW_POSSIBLE, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_POSSIBLE), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
}
