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
Node *UncertOp(Operator *expr, HashMap *hmp, Set *st);
Node *UncertIf(CaseExpr *expr, HashMap *hmp, Set *st);

static QueryOperator *rewrite_UncertSelection(QueryOperator *op);
static QueryOperator *rewrite_Uncertprojection(QueryOperator *op);
static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op);

Node *
getUncertaintyExpr(Node *expr, HashMap *hmp, Set *st) {
	//INFO_LOG("expression: %s ,  %p", exprToSQL(expr), expr);
	if(hasSetElem(st,expr)){
		return (Node *)createConstBool(FALSE);
	}
	switch(expr->type){
		case T_Constant: {
			return (Node *)createConstBool(TRUE);
		}
		case T_AttributeReference: {
			return getMap(hmp, expr);
		}
		case T_Operator: {
			return UncertOp((Operator *)expr, hmp, st);
		}
		case T_CaseExpr: {
			return UncertIf((CaseExpr *)expr, hmp, st);
		}
		default: {
			FATAL_LOG("unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
		}
	}
	return NULL;
}

Node *
UncertIf(CaseExpr *expr, HashMap *hmp, Set* st) {
	List *result = NIL;
	List *elselist = NIL;
	if(expr->elseRes){
		elselist = singleton(getUncertaintyExpr(expr->elseRes, hmp, st));
	}
	Node *c;
	if(expr->expr){
		FOREACH(Node,nd,expr->whenClauses) {
			Node *tmp = (Node *)createOpExpr("=", appendToTailOfList(singleton(expr->expr),((CaseWhen *)nd)->when));
			c = AND_EXPRS(getUncertaintyExpr(tmp, hmp, st),getUncertaintyExpr(((CaseWhen *)nd)->then, hmp, st),tmp);
			if(!result) {
				result = singleton(c);
			} else {
				appendToTailOfList(result, c);
			}
			if(elselist) {
				appendToTailOfList(elselist, (Node *)createOpExpr("NOT",singleton(tmp)));
				appendToTailOfList(elselist, getUncertaintyExpr(tmp, hmp, st));
			}
		}
	} else {
		FOREACH(Node,nd,expr->whenClauses) {
			c = AND_EXPRS(getUncertaintyExpr(((CaseWhen *)nd)->when, hmp, st),getUncertaintyExpr(((CaseWhen *)nd)->then, hmp, st),((CaseWhen *)nd)->when);
			if(!result) {
				result = singleton(c);
			} else {
				appendToTailOfList(result, c);
			}
			c = AND_EXPRS((Node *)createOpExpr("NOT",singleton(((CaseWhen *)nd)->when)), getUncertaintyExpr(((CaseWhen *)nd)->when, hmp, st));
			if(elselist) {
				appendToTailOfList(elselist, (Node *)createOpExpr("NOT",singleton(((CaseWhen *)nd)->when)));
				appendToTailOfList(elselist, getUncertaintyExpr(((CaseWhen *)nd)->when, hmp, st));
			}
		}
	}
	return OR_EXPRS(orExprList(result),andExprList(elselist));
}

//uncertainty func
Node *
UncertOp(Operator *expr, HashMap *hmp, Set* st) {
	if(!expr){
		return NULL;
	}
	if(strcmp(expr->name,"*")==0) {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		Node *c1 = AND_EXPRS(getUncertaintyExpr(e1, hmp, st),getUncertaintyExpr(e2, hmp, st));
		Node *c2 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e1), (Node *)createConstInt(0)));
		Node *c3 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e2), (Node *)createConstInt(0)));
		return OR_EXPRS(c1,c2,c3);
	}
	else if(strcmp(strToUpper(expr->name),"OR")==0){
		List *c1 = NIL;
		List *all = NIL;
		FOREACH(Node,nd,expr->args) {
			if(!c1) {
				c1 = singleton(getUncertaintyExpr(nd, hmp, st));
			} else {
				appendToTailOfList(c1, getUncertaintyExpr(nd, hmp, st));
			}
			if(!all) {
				all = singleton(AND_EXPRS(nd, getUncertaintyExpr(nd, hmp, st)));
			} else {
				appendToTailOfList(all, AND_EXPRS(nd, getUncertaintyExpr(nd, hmp, st)));
			}
		}
		appendToTailOfList(all, andExprList(c1));
		return orExprList(all);
	}
	else if(strcmp(strToUpper(expr->name),"AND")==0) {
		List *c1 = NIL;
		List *all = NIL;
		FOREACH(Node,nd,expr->args) {
			if(!c1) {
				c1 = singleton(getUncertaintyExpr(nd, hmp, st));
			} else {
				appendToTailOfList(c1, getUncertaintyExpr(nd, hmp, st));
			}
			if(!all) {
				all = singleton(AND_EXPRS((Node *)createOpExpr("NOT",singleton(nd)), getUncertaintyExpr(nd, hmp, st)));
			} else {
				appendToTailOfList(all, AND_EXPRS((Node *)createOpExpr("NOT",singleton(nd)), getUncertaintyExpr(nd, hmp, st)));
			}
		}
		appendToTailOfList(all, andExprList(c1));
		return orExprList(all);
	}
	else {
		List * lst = NIL;
		FOREACH(Node,nd,expr->args) {
			if(!lst) {
				lst = singleton(getUncertaintyExpr(nd, hmp, st));
			} else {
				appendToTailOfList(lst, getUncertaintyExpr(nd, hmp, st));
			}
		}
		//INFO_LOG("listlen: %d", LIST_LENGTH(lst));
		return andExprList(lst);
	}
	return NULL;
}

QueryOperator *
rewriteUncert(QueryOperator * op) {
	QueryOperator *rewrittenOp = NULL;
	switch(op->type)
	{
		case T_TableAccessOperator:
			rewrittenOp =rewrite_UncertTableAccess(op);
			break;
		case T_SelectionOperator:
			rewrittenOp = rewrite_UncertSelection(op);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_UncertProjection(op);
			//todo
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
	return NULL;
}

static QueryOperator *rewrite_UncertProjection(QueryOperator *op) {

	ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-UNCERT - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
		// rewrite child first

    rewriteUncert(OP_LCHILD(op));
	return NULL;
}

static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op) {
	INFO_LOG("CHECKING UNCERT ATTRREF MAPPING.");
	return op;
}
