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

#include "provenance_rewriter/prov_utility.h"
#include "utility/string_utils.h"

/* function declarations */
Node *UncertOp(Operator *expr, HashMap *hmp);
Node *UncertIf(CaseExpr *expr, HashMap *hmp);
Node *UncertFun(Node *expr, HashMap *hmp);
Node *createCaseOperator(Node *expr);
Node *createReverseCaseOperator(Node *expr);

static QueryOperator *rewrite_UncertSelection(QueryOperator *op);
static QueryOperator *rewrite_UncertProjection(QueryOperator *op);
static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op);
void addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef);

Node *
createCaseOperator(Node *expr){
	CaseWhen * cwhen = createCaseWhen(expr, (Node *)createConstInt(1));
	return (Node *)createCaseExpr(NULL, singleton(cwhen), (Node *)createConstInt(-1));
}

Node *
createReverseCaseOperator(Node *expr){
	CaseWhen * cwhen = createCaseWhen(expr, (Node *)createConstInt(-1));
	return (Node *)createCaseExpr(NULL, singleton(cwhen), (Node *)createConstInt(1));
}

Node *removeUncertOpFromExpr(Node *expr) {
	if(!expr){
		return NULL;
	}
	switch(expr->type){
			case T_Operator: {
				if(strcmp(((Operator *)expr)->name,"UNCERT")==0) {
						return (Node *)getHeadOfListP(((Operator *)expr)->args);
					}
				FOREACH(Node, nd, ((Operator *)expr)->args){
					removeUncertOpFromExpr(nd);
				}
				return expr;
				break;
			}
			case T_CaseExpr: {
				((CaseExpr *)expr)->elseRes = removeUncertOpFromExpr(((CaseExpr *)expr)->elseRes);
				((CaseExpr *)expr)->expr = removeUncertOpFromExpr(((CaseExpr *)expr)->expr);
				//List *newwcs = newList(T_List);
				FOREACH(Node, nd, ((CaseExpr *)expr)->whenClauses){
					CaseWhen *tmp = (CaseWhen *)nd;
					tmp->when = removeUncertOpFromExpr(tmp->when);
					tmp->then = removeUncertOpFromExpr(tmp->then);
					//appendToTailOfList(newwcs, tmp);
				}
				return expr;
				break;
			}
			case T_FunctionCall: {
				if(strcmp(strToUpper(((FunctionCall *)expr)->functionname),"UNCERT")==0) {
					return (Node *)getHeadOfListP(((FunctionCall *)expr)->args);
				}
				FOREACH(Node, nd, ((FunctionCall *)expr)->args){
					removeUncertOpFromExpr(nd);
				}
				return expr;
				break;
			}
			default: {
				//INFO_LOG("unknown expression type for removing:(%d) %s", expr->type, nodeToString(expr));
				return expr;
				break;
			}
		}
		return expr;
}

Node *
getUncertaintyExpr(Node *expr, HashMap *hmp) {
	//INFO_LOG("expression: %s ,  %p", exprToSQL(expr), expr);
	switch(expr->type){
		case T_Constant: {
			return (Node *)createConstInt(1);
		}
		case T_AttributeReference: {
			((AttributeReference *)expr)->outerLevelsUp = -1;
			return getMap(hmp, expr);
		}
		case T_Operator: {
			return UncertOp((Operator *)expr, hmp);
		}
		case T_CaseExpr: {
			return UncertIf((CaseExpr *)expr, hmp);
		}
		case T_FunctionCall: {
			return UncertFun(expr, hmp);
		}
		default: {
			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
		}
	}
	return NULL;
}

Node *
UncertFun(Node *expr, HashMap *hmp) {
	if(strcmp(strToUpper(((FunctionCall *)expr)->functionname),"UNCERT")==0) {
		return (Node *)createConstInt(-1);
	}
	return NULL;
}

Node *
UncertIf(CaseExpr *expr, HashMap *hmp) {
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
			Node *temp = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(uncertwhen),uncertthen));
			temp = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(temp),evalwhen));
			if(!ret) {
				ret = temp;
			} else {
				ret = (Node *)createFunctionCall("GREATEST", appendToTailOfList(singleton(ret),temp));
			}
			if(elseExpr){
				Node *evalwhen = createReverseCaseOperator(exprtmp);
				temp = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(uncertwhen),evalwhen));
				elseExpr = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(elseExpr),temp));
			}
		}
	} else {
		FOREACH(Node,nd,expr->whenClauses) {
			Node * uncertwhen = getUncertaintyExpr(((CaseWhen *)nd)->when, hmp);
			Node * uncertthen = getUncertaintyExpr(((CaseWhen *)nd)->then, hmp);
			Node * evalwhen = createCaseOperator(((CaseWhen *)nd)->when);
			Node *temp = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(uncertwhen),uncertthen));
			temp = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(temp),evalwhen));
			if(!ret) {
				ret = temp;
			} else {
				ret = (Node *)createFunctionCall("GREATEST", appendToTailOfList(singleton(ret),temp));
			}
			if(elseExpr){
				Node *evalwhen = createReverseCaseOperator(((CaseWhen *)nd)->when);
				temp = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(uncertwhen),evalwhen));
				elseExpr = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(elseExpr),temp));
			}
		}
	}
	if(elseExpr){
		ret = (Node *)createFunctionCall("GREATEST", appendToTailOfList(singleton(ret),elseExpr));
	}
	return ret;
}

//uncertainty func
Node *
UncertOp(Operator *expr, HashMap *hmp) {
	if(!expr){
		return NULL;
	}
	if(strcmp(expr->name,"UNCERT")==0) {
		return (Node *)createConstInt(-1);
	}
	if(strcmp(expr->name,"*")==0) {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		Node *c1 = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(getUncertaintyExpr(e1, hmp)),getUncertaintyExpr(e2, hmp)));
		Node *c2 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e1), (Node *)createConstInt(0)));
		Node *ret = (Node *)createFunctionCall("GREATEST", appendToTailOfList(singleton(c1), createCaseOperator(c2)));
		Node *c3 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e2), (Node *)createConstInt(0)));
		ret = (Node *)createFunctionCall("GREATEST", appendToTailOfList(singleton(ret), createCaseOperator(c3)));
		return ret;
	}
	else if(strcmp(strToUpper(expr->name),"OR")==0){
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		List * argList = singleton(getUncertaintyExpr(e1, hmp));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		Node *ret = (Node *)createFunctionCall("LEAST", argList);
		argList = singleton(createCaseOperator(e1));
		appendToTailOfList(argList, getUncertaintyExpr(e1, hmp));
		Node *temp = (Node *)createFunctionCall("LEAST", argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall("GREATEST", argList);
		argList = singleton(createCaseOperator(e2));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		temp = (Node *)createFunctionCall("LEAST", argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall("GREATEST", argList);
		return ret;
	}
	else if(strcmp(strToUpper(expr->name),"AND")==0) {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		List * argList = singleton(getUncertaintyExpr(e1, hmp));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		Node *ret = (Node *)createFunctionCall("LEAST", argList);
		argList = singleton(createReverseCaseOperator(e1));
		appendToTailOfList(argList, getUncertaintyExpr(e1, hmp));
		Node *temp = (Node *)createFunctionCall("LEAST", argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall("GREATEST", argList);
		argList = singleton(createReverseCaseOperator(e2));
		appendToTailOfList(argList, getUncertaintyExpr(e2, hmp));
		temp = (Node *)createFunctionCall("LEAST", argList);
		argList = singleton(ret);
		appendToTailOfList(argList, temp);
		ret = (Node *)createFunctionCall("GREATEST", argList);
		return ret;
	}
	else {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		return (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(getUncertaintyExpr(e1, hmp)),getUncertaintyExpr(e2, hmp)));
	}
	return NULL;
}

QueryOperator *
rewriteUncert(QueryOperator * op) {
	QueryOperator *rewrittenOp;
	switch(op->type)
	{
		case T_TableAccessOperator:
			rewrittenOp =rewrite_UncertTableAccess(op);
			INFO_OP_LOG("Rewrite TableAccess:", rewrittenOp);
			break;
		case T_SelectionOperator:
			rewrittenOp = rewrite_UncertSelection(op);
			INFO_OP_LOG("Rewrite Selection:", rewrittenOp);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_UncertProjection(op);
			INFO_OP_LOG("Rewrite Projection:", rewrittenOp);
			break;
		case T_JoinOperator:
			//todo
			rewrittenOp = op;
			break;
		case T_AggregationOperator:
			//todo
			rewrittenOp = op;
			break;
		default:
			FATAL_LOG("rewrite for %u not implemented", op->type);
			rewrittenOp = NULL;
			break;
	}
	return rewrittenOp;
}

static QueryOperator *rewrite_UncertSelection(QueryOperator *op) {

	ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-UNCERT - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
		// rewrite child first

    rewriteUncert(OP_LCHILD(op));
	return op;
}

static QueryOperator *rewrite_UncertProjection(QueryOperator *op) {

	ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-UNCERT - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
		// rewrite child first
    //INFO_LOG("NormalAttrName: %s",stringListToString(getNormalAttrNames(op)));
    //INFO_LOG("ProjExpr: %s",nodeToString(((ProjectionOperator *)op)->projExprs));
    rewriteUncert(OP_LCHILD(op));
    HashMap * hmp = NEW_MAP(Node, Node);
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
    //INFO_LOG("HashMap: %s", nodeToString((Node *)hmpIn));
    List *attrExpr = getNormalAttrProjectionExprs(op);
    //INFO_LOG("AttrExpr: %s",nodeToString(attrExpr));
    List *uncertlist = NIL;
    FOREACH(Node, nd, attrExpr){
    	addUncertAttrToSchema(hmp, op, nd);
    	//INFO_LOG("nth list: %d", LIST_LENGTH(uncertlist));
    	Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs,LIST_LENGTH(uncertlist));
    	Node *reExpr = getUncertaintyExpr(projexpr, hmpIn);
    	//INFO_LOG("nth list: %s", exprToSQL(reExpr));
    	uncertlist = appendToTailOfList(uncertlist, reExpr);
    	removeUncertOpFromExpr(projexpr);
    	//INFO_LOG("rewrite expression: %s\n", exprToSQL(reExpr));
    }
    //INFO_LOG("rewrite expression list: %s\n", nodeToString(uncertlist));
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
    addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference("R"));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, getUncertaintyExpr((Node *)createAttributeReference("R"), hmpIn));
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
	return op;
}

static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op) {
	DEBUG_LOG("REWRITE-UNCERT - TableAccess");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));
	HashMap * hmp = NEW_MAP(Node, Node);
	//List *modAttrName = getNormalAttrNames(op);
	//List *modAttrExpr = getNormalAttrProjectionExprs(op);
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
	switchSubtrees(op, proj);
	List *attrExpr = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr){
		addUncertAttrToSchema(hmp, proj, nd);
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	}
	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference("R"));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
	//INFO_LOG("Operator tree \n%s", nodeToString(proj));
	return proj;
}

void addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef){
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", "U_");
	appendStringInfo(str, "%s", ((AttributeReference *)aRef)->name);
	addAttrToSchema(target, str->data, DT_INT);
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
}
