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
Node *getOutputExprFromInput(Node *expr, int offset);

static QueryOperator *rewrite_UncertSelection(QueryOperator *op);
static QueryOperator *rewrite_UncertProjection(QueryOperator *op);
static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op);
static QueryOperator * rewrite_UncertJoin(QueryOperator *op);
static QueryOperator *rewrite_UncertAggregation(QueryOperator *op);
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
				FOREACH(Node, nd, ((CaseExpr *)expr)->whenClauses){
					CaseWhen *tmp = (CaseWhen *)nd;
					tmp->when = removeUncertOpFromExpr(tmp->when);
					tmp->then = removeUncertOpFromExpr(tmp->then);
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
				return expr;
				break;
			}
		}
		return expr;
}

Node *getOutputExprFromInput(Node *expr, int offset) {
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

Node *
getUncertaintyExpr(Node *expr, HashMap *hmp) {
	//INFO_LOG("expression: %s ,  %p", exprToSQL(expr), expr);
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
			INFO_OP_LOG("Uncertainty Rewrite TableAccess:", rewrittenOp);
			break;
		case T_SelectionOperator:
			rewrittenOp = rewrite_UncertSelection(op);
			INFO_OP_LOG("Uncertainty Rewrite Selection:", rewrittenOp);
			break;
		case T_ProjectionOperator:
			rewrittenOp = rewrite_UncertProjection(op);
			INFO_OP_LOG("Uncertainty Rewrite Projection:", rewrittenOp);
			break;
		case T_JoinOperator:
			rewrittenOp = rewrite_UncertJoin(op);
			INFO_OP_LOG("Uncertainty Rewrite Join:", rewrittenOp);
			break;
		case T_AggregationOperator:
			rewrittenOp = rewrite_UncertAggregation(op);
			INFO_OP_LOG("Uncertainty Rewrite Aggregation:", rewrittenOp);
			break;
		default:
			FATAL_LOG("rewrite for %u not implemented", op->type);
			rewrittenOp = NULL;
			break;
	}
	return rewrittenOp;
}

static QueryOperator *rewrite_UncertAggregation(QueryOperator *op) {
	ASSERT(OP_LCHILD(op));

	//INFO_LOG("REWRITE-UNCERT - Aggregation");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	// rewrite child first
	rewriteUncert(OP_LCHILD(op));

	//INFO_LOG("AttrRefList: %s", nodeToString(nan));

	HashMap * hmp = NEW_MAP(Node, Node);
	HashMap * hmpProj = NEW_MAP(Node, Node);
	HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");

	// create projection before aggregation

	List *attrName = getNormalAttrNames(OP_LCHILD(op));
	List *projExpr = NIL;
	FOREACH(Node, nd, ((AggregationOperator *)op)->aggrs){
		appendToTailOfList(projExpr, (Node *)createConstInt(-1));
	}
	if(((AggregationOperator *)op)->groupBy){
		FOREACH(Node, nd, ((AggregationOperator *)op)->groupBy){
			appendToTailOfList(projExpr, nd);
		}
	}
	appendToTailOfList(projExpr, getUncertaintyExpr((Node *)createAttributeReference("R"), hmpIn));

	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExpr, OP_LCHILD(op), singleton(op), attrName);
	op->inputs = singleton(proj);
	OP_LCHILD(proj)->parents = singleton(proj);

	FOREACH(Node, nd, attrName){
		addUncertAttrToSchema(hmpProj, proj, nd);
	}
	addUncertAttrToSchema(hmpProj, proj, (Node *)createAttributeReference("R"));


	//add uncertainty to Aggregation

	FOREACH(Node, nd, ((AggregationOperator *)op)->aggrs){
			addUncertAttrToSchema(hmp, op, nd);
			appendToTailOfList(projExpr, (Node *)createConstInt(-1));
		}

	if(((AggregationOperator *)op)->groupBy){

		FOREACH(Node, nd, ((AggregationOperator *)op)->groupBy){
			addUncertAttrToSchema(hmp, op, nd);
			appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall("MAX", singleton(getUncertaintyExpr(nd, hmpIn))));
		}
	}

	Node *rU = getUncertaintyExpr((Node *)createAttributeReference("R"), hmpIn);
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference("R"));
	appendToTailOfList(((AggregationOperator *)op)->aggrs, createFunctionCall("MAX", singleton(rU)));

	return op;
}

static QueryOperator *rewrite_UncertJoin(QueryOperator *op) {
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	//INFO_LOG("REWRITE-UNCERT - Join");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	List * nan = getNormalAttrNames(op);
	// rewrite child first
	rewriteUncert(OP_LCHILD(op));
	rewriteUncert(OP_RCHILD(op));
	HashMap * hmp =(HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
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

	Node *rU = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(uRl), uRR));

	List *projExprNew = concatTwoLists(concatTwoLists(attrl, attrR), concatTwoLists(uattrl, uattrR));

	if(((JoinOperator*)op)->joinType == JOIN_INNER && ((JoinOperator*)op)->cond){
		Node *outExpr = (Node *)copyObject(((JoinOperator*)op)->cond);
		outExpr = getOutputExprFromInput(outExpr, divider);
		rU = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(getUncertaintyExpr(outExpr, hmp)), rU));
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
	addUncertAttrToSchema(hmp2, proj, (Node *)createAttributeReference("R"));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp2);

	return proj;

}

static QueryOperator *rewrite_UncertSelection(QueryOperator *op) {

	ASSERT(OP_LCHILD(op));

	//INFO_LOG("REWRITE-UNCERT - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
	// rewrite child first
    rewriteUncert(OP_LCHILD(op));

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    //HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
    List *attrExpr = getNormalAttrProjectionExprs(op);
    FOREACH(Node, nd, attrExpr){
        	addUncertAttrToSchema(hmp, op, nd);
    }
    addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference("R"));
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
    //create projection to calculate row uncertainty
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
    switchSubtrees(op, proj);
    op->parents = singleton(proj);
    Node *uExpr = (Node *)getTailOfListP(((ProjectionOperator *)proj)->projExprs);
    ((ProjectionOperator *)proj)->projExprs = removeFromTail(((ProjectionOperator *)proj)->projExprs);
    Node *newUR = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(uExpr), getUncertaintyExpr(((SelectionOperator *)op)->cond, hmp)));
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, newUR);
    setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
	return proj;
}

static QueryOperator *rewrite_UncertProjection(QueryOperator *op) {

	ASSERT(OP_LCHILD(op));

    //INFO_LOG("REWRITE-UNCERT - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));
    //rewrite child first
    rewriteUncert(OP_LCHILD(op));

    HashMap * hmp = NEW_MAP(Node, Node);
    //get child hashmap
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");
    //INFO_LOG("HashMap: %s", nodeToString((Node *)hmpIn));
    List *attrExpr = getNormalAttrProjectionExprs(op);
    List *uncertlist = NIL;
    FOREACH(Node, nd, attrExpr){
    	addUncertAttrToSchema(hmp, op, nd);
    	Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs,LIST_LENGTH(uncertlist));
    	Node *reExpr = getUncertaintyExpr(projexpr, hmpIn);
    	uncertlist = appendToTailOfList(uncertlist, reExpr);
    	removeUncertOpFromExpr(projexpr);
    }
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
    addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference("R"));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, getUncertaintyExpr((Node *)createAttributeReference("R"), hmpIn));
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
    //INFO_LOG("ProjList: %s", nodeToString((Node *)(((ProjectionOperator *)op)->projExprs)));
	return op;
}

static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op) {
	//INFO_LOG("REWRITE-UNCERT - TableAccess");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));
	HashMap * hmp = NEW_MAP(Node, Node);
	QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getNormalAttrNames(op));
	switchSubtrees(op, proj);
	op->parents = singleton(proj);
	List *attrExpr = getNormalAttrProjectionExprs(op);
	FOREACH(Node, nd, attrExpr){
		addUncertAttrToSchema(hmp, proj, nd);
		appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	}
	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference("R"));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
	//INFO_LOG("HashMap: %s", nodeToString((Node *)hmp));
	return proj;
}

void addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef){
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", "U_");
	appendStringInfo(str, "%s", ((AttributeReference *)aRef)->name);
	addAttrToSchema(target, str->data, DT_INT);
	((AttributeReference *)aRef)->outerLevelsUp = 0;
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
}
