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

#include "analysis_and_translate/translator_oracle.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "utility/string_utils.h"


/* function declarations */
static Node *UncertOp(Operator *expr, HashMap *hmp);
static Node *UncertIf(CaseExpr *expr, HashMap *hmp);
static Node *UncertFun(Node *expr, HashMap *hmp);
static Node *createCaseOperator(Node *expr);
static Node *createReverseCaseOperator(Node *expr);
static Node *getOutputExprFromInput(Node *expr, int offset);

static QueryOperator *rewriteUncertProvComp(QueryOperator *op);
static QueryOperator *rewrite_UncertSelection(QueryOperator *op);
static QueryOperator *rewrite_UncertProjection(QueryOperator *op);
static QueryOperator *rewrite_UncertTableAccess(QueryOperator *op);
static QueryOperator *rewrite_UncertJoin(QueryOperator *op);
static QueryOperator *rewrite_UncertAggregation(QueryOperator *op);
static QueryOperator *rewrite_UncertDuplicateRemoval(QueryOperator *op);
static QueryOperator *rewrite_UncertSet(QueryOperator *op);

static void addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef);

static List *putMidListToEnd(List *in, int p1, int p2);


QueryOperator *
rewriteUncert(QueryOperator * op)
{
	QueryOperator *rewrittenOp;
	switch(op->type)
	{
	    case T_ProvenanceComputation:
	        rewrittenOp = rewriteUncertProvComp(op);
	        break;
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
		case T_DuplicateRemoval:
			rewrittenOp = rewrite_UncertDuplicateRemoval(op);
			INFO_OP_LOG("Uncertainty Rewrite DuplicateRemoval:", rewrittenOp);
			break;
		case T_SetOperator:
			rewrittenOp = rewrite_UncertSet(op);
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
				if(strcmp(((Operator *)expr)->name,"UNCERT")==0) {
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
				if(strcmp(strToUpper(((FunctionCall *)expr)->functionname),"UNCERT")==0) {
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
getUncertaintyExpr(Node *expr, HashMap *hmp)
{
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

char *
getUncertString(char *in)
{
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", "U_");
	appendStringInfo(str, "%s", in);
	return str->data;
}
static QueryOperator *
rewriteUncertProvComp(QueryOperator *op)
{
    ASSERT(LIST_LENGTH(op->inputs) == 1);
    QueryOperator *top = getHeadOfListP(op->inputs);
    List *topSchema;

    top = rewriteUncert(top);

    // make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    topSchema = copyObject(op->schema->attrDefs);
    disambiguiteAttrNames((Node *) top, done);
//    if (isA(top,ProjectionOperator)){
//       // top->schema->attrDefs = topSchema;
//        }
//    else
//    {
//        QueryOperator *proj = createProjOnAllAttrs(top);
//        addChildOperator(proj, top);
//        switchSubtrees((QueryOperator *) top, proj);
//        proj->schema->attrDefs = topSchema;
//    }

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root for uncertainty is:", top);

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
	return (Node *)createCaseExpr(NULL, singleton(cwhen), (Node *)createConstInt(-1));
}

static Node *
createReverseCaseOperator(Node *expr)
{
	CaseWhen * cwhen = createCaseWhen(expr, (Node *)createConstInt(-1));
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
UncertFun(Node *expr, HashMap *hmp)
{
	if(strcmp(strToUpper(((FunctionCall *)expr)->functionname),"UNCERT")==0) {
		return (Node *)createConstInt(-1);
	}
	return NULL;
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
static Node *
UncertOp(Operator *expr, HashMap *hmp)
{
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

static QueryOperator *
rewrite_UncertSet(QueryOperator *op)
{
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Set");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	// rewrite child first
	rewriteUncert(OP_LCHILD(op));
	rewriteUncert(OP_RCHILD(op));

	HashMap * hmp = NEW_MAP(Node, Node);

	List *projExpr = getNormalAttrProjectionExprs(op);

	FOREACH(Node, nd, projExpr){
		addUncertAttrToSchema(hmp, op, nd);
	}
	addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);

	//todo set output row to uncertain in case of set difference
	if(((SetOperator *)op)->setOpType == SETOP_DIFFERENCE){
		List *projExpr = getNormalAttrProjectionExprs(op);
		projExpr = removeFromTail(projExpr);
		projExpr = appendToTailOfList(projExpr, createConstInt(-1));

		QueryOperator *proj = (QueryOperator *)createProjectionOp(projExpr, op, NIL, getNormalAttrNames(op));
		switchSubtrees(op, proj);
		op->parents = singleton(proj);
		setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);

		return proj;
	}
	return op;
}


static QueryOperator *
rewrite_UncertDuplicateRemoval(QueryOperator *op)
{
	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - DuplicateRemoval");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//record original schema info
	List *projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *attrName = getNormalAttrNames(OP_LCHILD(op));

	// rewrite child first
	rewriteUncert(OP_LCHILD(op));

	HashMap * hmp = NEW_MAP(Node, Node);
	HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");

	//create Aggregation
	Node *rUExpr = getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn);
	List *aggrList = NIL;
	List *uattrName = NIL;
	FOREACH(Node, nd, projExpr){
		Node *maxExpr =	(Node *)createFunctionCall("MAX", singleton(getUncertaintyExpr(nd, hmpIn)));
		aggrList = appendToTailOfList(aggrList, maxExpr);
		rUExpr = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(rUExpr), getUncertaintyExpr(nd, hmpIn)));
		uattrName = appendToTailOfList(uattrName, getUncertString(((AttributeReference *)nd)->name));
	}
	Node *maxExpr =	(Node *)createFunctionCall("MAX", singleton(rUExpr));
	aggrList = appendToTailOfList(aggrList, maxExpr);
	uattrName = appendToTailOfList(uattrName, "U_R");
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
rewrite_UncertAggregation(QueryOperator *op)
{
	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Aggregation");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//record original schema info
	List *projExpr = getNormalAttrProjectionExprs(OP_LCHILD(op));
	List *attrName = getNormalAttrNames(OP_LCHILD(op));

	// rewrite child first
	rewriteUncert(OP_LCHILD(op));

	HashMap * hmp = NEW_MAP(Node, Node);
	HashMap * hmpProj = NEW_MAP(Node, Node);
	HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), "UNCERT_MAPPING");

	// create projection before aggregation (for now we mark all aggregation result into uncertain)

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
			uncertExpr = appendToTailOfList(uncertExpr, (Node *)createConstInt(-1));
		}
	}
	addUncertAttrToSchema(hmpProj, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	Node *rU = getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn);
	FOREACH(Node, nd, ((AggregationOperator *)op)->groupBy){
		rU = (Node *)createFunctionCall("LEAST", appendToTailOfList(singleton(rU), getUncertaintyExpr(nd, hmpIn)));
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
		attrUaggr = appendToTailOfList(attrUaggr, (Node *)createFunctionCall("MAX", singleton(getUncertaintyExpr(tmp, hmpProj))));
	}

	FOREACH(Node, nd, ((AggregationOperator *)op)->groupBy){
		attrUaggr = appendToTailOfList(attrUaggr, (Node *)createFunctionCall("MAX", singleton(getUncertaintyExpr(nd, hmpProj))));
	}

	rU = getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn);
	attrUaggr = appendToTailOfList(attrUaggr, createFunctionCall("MAX", singleton(rU)));
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
rewrite_UncertJoin(QueryOperator *op)
{
	ASSERT(OP_LCHILD(op));
	ASSERT(OP_RCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Join");
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
	addUncertAttrToSchema(hmp2, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp2);

	return proj;

}

static QueryOperator *
rewrite_UncertSelection(QueryOperator *op)
{

	ASSERT(OP_LCHILD(op));

	INFO_LOG("REWRITE-UNCERT - Selection");
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
    addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
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

static QueryOperator *
rewrite_UncertProjection(QueryOperator *op)
{

    ASSERT(OP_LCHILD(op));

    INFO_LOG("REWRITE-UNCERT - Projection");
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
        replaceNode(((ProjectionOperator *)op)->projExprs, projexpr, removeUncertOpFromExpr(projexpr));
    }
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
    addUncertAttrToSchema(hmp, op, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, getUncertaintyExpr((Node *)createAttributeReference(UNCERTAIN_ROW_ATTR), hmpIn));
    setStringProperty(op, "UNCERT_MAPPING", (Node *)hmp);
    //INFO_LOG("ProjList: %s", nodeToString((Node *)(((ProjectionOperator *)op)->projExprs)));
    return op;
}

static QueryOperator *
rewrite_UncertTableAccess(QueryOperator *op)
{
	INFO_LOG("REWRITE-UNCERT - TableAccess");
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
	addUncertAttrToSchema(hmp, proj, (Node *)createAttributeReference(UNCERTAIN_ROW_ATTR));
	appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
	setStringProperty(proj, "UNCERT_MAPPING", (Node *)hmp);
	//INFO_LOG("HashMap: %s", nodeToString((Node *)hmp));
	return proj;
}

static void
addUncertAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef)
{
	addAttrToSchema(target, getUncertString(((AttributeReference *)aRef)->name), DT_INT);
	((AttributeReference *)aRef)->outerLevelsUp = 0;
	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)getTailOfListP(getNormalAttrProjectionExprs(target))));
}

