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
Node *deepReplaceAttrRef(Node * expr, Node *af);//replace all attributeReferences to af
//QueryOperator * addSemiringCombiner(QueryOperator * result);
Node *UncertOp(Operator *expr, HashMap *hmp, Set *st);
Node *UncertIf(CaseExpr *expr, HashMap *hmp, Set *st);

Node *deepReplaceAttrRef(Node * expr, Node *af){
	switch(expr->type){
		case T_AttributeReference: {
			return (Node *)copyObject(af);
		}
		case T_Operator: {
			FOREACH_LC(lc,((Operator *)expr)->args){
				lc->data.ptr_value = (Node *)deepReplaceAttrRef((Node *)(lc->data.ptr_value), af);
			}
			break;
		}
		default: {
			return expr;
		}
	}
	return expr;
}



boolean
isSemiringCombinerActivatedOp(QueryOperator * op)
{
    return HAS_STRING_PROP((QueryOperator *)op,PROP_PC_SEMIRING_COMBINER);
}

boolean
isSemiringCombinerActivatedPs(ProvenanceStmt *stmt)
{
	FOREACH(KeyValue,kv,stmt->options){
		//INFO_LOG(STRING_VALUE(kv->key));
		if(strcmp(STRING_VALUE(kv->key),PROP_PC_SEMIRING_COMBINER)==0)
			return TRUE;
	}
	return FALSE;
}

char *
getSemiringCombinerFuncName(QueryOperator *op){
	Node *p = getStringProperty(op, PROP_PC_SEMIRING_COMBINER);
	switch(p->type){
		case T_Constant: {
			return "sum";
		}
		case T_List: {
			return ((FunctionCall *)getNthOfListP((List *)p,0))->functionname;
		}
		default: {
			FATAL_LOG("unknown expression type for SC option: %s", nodeToString(p));
		}
	}
	return NULL;
}

Node *
getSemiringCombinerExpr(QueryOperator *op){
	Node *p = getStringProperty(op, PROP_PC_SEMIRING_COMBINER);
	switch(p->type){
		case T_Constant: {
			return (Node *)createOpExpr("*", appendToTailOfList(singleton(createAttributeReference("x")),createAttributeReference("y")));
		}
		case T_List: {
			return (Node *)((Operator *)getNthOfListP((List *)p,1));
		}
		default: {
			FATAL_LOG("unknown expression type for SC option: %s", nodeToString(p));
		}
	}
	return NULL;
}

DataType
getSemiringCombinerDatatype(ProvenanceStmt *stmt, List *dts) {
	FOREACH(KeyValue,kv,stmt->options){
		if(streq(STRING_VALUE(kv->key),PROP_PC_SEMIRING_COMBINER)) {
			INFO_LOG(nodeToString(kv->value));
			char * scop = NULL;
			char * funcn = NULL;
			switch(kv->value->type){
				case T_Constant: {
					scop = "*";
					funcn = "sum";
					break;
				}
				case T_List: {
					scop = ((Operator *)getNthOfListP(((List *)kv->value),1))->name;
					funcn = ((FunctionCall *)getNthOfListP(((List *)kv->value),0))->functionname;
					break;
				}
				default: {
					FATAL_LOG("unknown expression type for SC option: %s", nodeToString(kv->value));
				}
			}
			DataType dtFuncIn = getOpReturnType(scop,sublist(dts,0,1));
			//INFO_LOG("SC: getting SC datatype: %d.", dtFuncIn);
			return getFuncReturnType(funcn,singletonInt(dtFuncIn));
		}
	}
	FATAL_LOG("No semiring combiner info in provenance options.");
}

extern void addSCOptionToChild(QueryOperator *op, QueryOperator *to) {
	if(HAS_STRING_PROP(op,PROP_PC_SC_AGGR_OPT)){
		SET_STRING_PROP(to, PROP_PC_SC_AGGR_OPT, GET_STRING_PROP(op,PROP_PC_SC_AGGR_OPT));
	}
}

//uncertainty func
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
//uncertainty func
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
	if(strcmp(expr->name,"*")==0) {
		Node * e1 = (Node *)(getNthOfListP(expr->args, 0));
		Node * e2 = (Node *)(getNthOfListP(expr->args, 1));
		Node *c1 = AND_EXPRS(getUncertaintyExpr(e1, hmp, st),getUncertaintyExpr(e2, hmp, st));
		Node *c2 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e1), (Node *)createConstInt(0)));
		Node *c3 = (Node *)createOpExpr("=", appendToTailOfList(singleton(e2), (Node *)createConstInt(0)));
		return OR_EXPRS(c1,c2,c3);
	}
	else if(strcmp(expr->name,"OR")==0){
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
	else if(strcmp(expr->name,"AND")==0) {
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

QueryOperator * addSemiringCombiner(QueryOperator * result, char * funcN, Node * expr) {
	if(funcN==NULL || expr==NULL){
		FATAL_LOG("SC: function and expression can not be null.");
	}
	List *attrNames = getNormalAttrNames((QueryOperator *)result);
	List *projExprs = getNormalAttrProjectionExprs((QueryOperator *)result);
	List *provExprs = getProvAttrProjectionExprs((QueryOperator *)result);
	Node *expre = NULL;
	char *opN = ((Operator *)expr)->name;
	Node *singExpr = (Node *)getNthOfListP(((Operator *)expr)->args,0);
	FOREACH(Node,nd,provExprs) {
		if(!expre)
			expre = deepReplaceAttrRef((Node *)copyObject(singExpr),nd);
		else
			expre = (Node *)createOpExpr(opN,appendToTailOfList(singleton(expre),deepReplaceAttrRef((Node *)copyObject(singExpr),nd)));
	}
	appendToTailOfList(projExprs,expre);
	appendToTailOfList(attrNames,replaceSubstr(exprToSQL(expre), " ", ""));
	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExprs, (QueryOperator *)result, NIL, attrNames);
	proj->provAttrs = singletonInt(getListLength(attrNames)-1);
	switchSubtrees(result, proj);
	result->parents = singleton(proj);
	//add aggregation
	Node *fc = (Node *)createFunctionCall(funcN,getProvAttrProjectionExprs(proj));
	List *gby = getNormalAttrProjectionExprs(proj);
	attrNames = removeFromTail(attrNames);
	appendToHeadOfList(attrNames,exprToSQL(fc));
	QueryOperator *aggr = (QueryOperator *)createAggregationOp(singleton(fc),gby,proj,NIL,attrNames);
	aggr->provAttrs = singletonInt(0);
	switchSubtrees(proj, aggr);
	proj->parents = singleton(aggr);
	//add projection
	attrNames = appendToTailOfList(getNormalAttrNames(aggr),"PROV");
	projExprs = concatTwoLists(getNormalAttrProjectionExprs(aggr),getProvAttrProjectionExprs(aggr));
	QueryOperator *proj2 = (QueryOperator *)createProjectionOp(projExprs, (QueryOperator *)aggr, NIL, attrNames);
	proj2->provAttrs = singletonInt(getListLength(attrNames)-1);
	switchSubtrees(aggr, proj2);
	aggr->parents = singleton(proj2);
	result = proj2;
	return result;
}


