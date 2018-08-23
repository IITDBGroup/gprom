/*
 * lateral_prov_main.c
 *
 *  Created on: July 27, 2018
 *      Author: Xing
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "utility/string_utils.h"

static List *lateralRewriteQueryList(List *list);
static QueryOperator *lateralRewriteQuery(QueryOperator *input);
static List *getListNestingOperator (QueryOperator *op);
static void appendNestingOperator (QueryOperator *op, List **result);
static int checkAttr (char *name, QueryOperator *op);
static void adatpUpNestingAttrDataType(QueryOperator *op, DataType nestingAttrDataType, int pos);
static void getNestCondOperator(Operator *o, List **nestOpLists);

Node *
lateralTranslateQBModel (Node *qbModel)
{
    if (isA(qbModel, List))
        return (Node *) lateralRewriteQueryList((List *) qbModel);
    else if (IS_OP(qbModel))
         return (Node *) lateralRewriteQuery((QueryOperator *) qbModel);

    FATAL_LOG("cannot lateral rewrite node <%s>", nodeToString(qbModel));

    return NULL;
}

static List *
lateralRewriteQueryList(List *list)
{
    FOREACH(QueryOperator,q,list)
        q_his_cell->data.ptr_value = lateralRewriteQuery(q);

    return list;
}


static QueryOperator *
lateralRewriteQuery(QueryOperator *input)
{

	DEBUG_LOG("lateralRewriteQuery");
	List *nestOpList = NIL;
	nestOpList = getListNestingOperator(input);

	FOREACH(QueryOperator, op, nestOpList)
	{
		QueryOperator *rChild = OP_RCHILD(op);
		QueryOperator *lChild = OP_LCHILD(op);

		Constant *c1 = createConstInt(1);
		Constant *c0 = createConstInt(0);

		NestingOperator *no = (NestingOperator *) op;
		char *aggname;
		DataType nestingAttrDataType = DT_INT;

		if(no->nestingType == NESTQ_EXISTS)
		{
			aggname = "COUNT";

			//create aggregation COUNT(1) AS "nesting_eval_1"
			FunctionCall *aggFunc = createFunctionCall("COUNT",
					singleton(copyObject(c1)));
			List *aggrs = singleton(aggFunc);
			AggregationOperator *agg = createAggregationOp(aggrs, NIL, rChild, NIL, singleton("AGGR_0"));
			rChild->parents = singleton(agg);

			//create projectioin SELECT CASE WHEN count(*) > 0 THEN 1 ELSE 0 END AS "nesting_eval_1"
			AttributeReference *aggAttrRef = getAttrRefByName((QueryOperator *) agg, "AGGR_0");

			//WHEN count(*) > 0 THEN 1
			Operator *whenOperator = createOpExpr(">", LIST_MAKE(copyObject(aggAttrRef), copyObject(c0)));
			CaseWhen *when = createCaseWhen((Node *) whenOperator, (Node *) copyObject(c1)); // (Node *) createConstFloat(0.0));
			//ELSE 0
			Constant *el =  copyObject(c0);
			CaseExpr *caseExpr = createCaseExpr(NULL, singleton(when), (Node *) el);

			char *attrName = getTailOfListP(getAttrNames(op->schema));
			ProjectionOperator *proj = createProjectionOp(singleton(caseExpr), (QueryOperator *) agg, NIL, singleton(strdup(attrName)));

			((QueryOperator *) agg)->parents = singleton(proj);
			((QueryOperator *) proj)->parents = singleton(op);
			op->inputs = LIST_MAKE(lChild, proj);

			//used to change nesting_eval_1 datatype from boolean in nesting op
			nestingAttrDataType = (DataType) getTailOfListInt(getDataTypes(((QueryOperator *) proj)->schema));
		}
		else if (no->nestingType == NESTQ_ANY || no->nestingType == NESTQ_ALL)
		{
			if(no->nestingType == NESTQ_ANY)
				aggname = "MAX";
			else if(no->nestingType == NESTQ_ALL)
				aggname = "MIN";

			//create projectioin SELECT MAX(CASE WHEN ((A > D)) THEN 1 ELSE 0 END) AS "nesting_eval_1"
			//WHEN ((A > D))
			Node *cond = copyObject(no->cond);
			no->cond = NULL;

			//adapt attrs outerlevelsUp and fromClauseItem
			List *attrRefs = getAttrReferences((Node *) cond);
			FOREACH(AttributeReference, a, attrRefs)
			{
				DEBUG_LOG("check name: %s", a->name);
				if(checkAttr(a->name, rChild))
				{
					a->fromClauseItem = 0;
					a->outerLevelsUp = 0;
				}
				else
				{
					a->outerLevelsUp = 1;
				}
			}

			//Operator *whenOperator = createOpExpr(">", LIST_MAKE(copyObject(aggAttrRef), copyObject(c0)));
			CaseWhen *when = createCaseWhen((Node *) cond, (Node *) copyObject(c1)); // (Node *) createConstFloat(0.0));
			//ELSE 0
			Constant *el =  copyObject(c0);
			CaseExpr *caseExpr = createCaseExpr(NULL, singleton(when), (Node *) el);

			ProjectionOperator *proj = createProjectionOp(singleton(caseExpr), rChild, NIL, singleton("nesting_eval_help"));
			rChild->parents = singleton(proj);

			//MAX
			AttributeReference *projAttrRef = getAttrRefByName((QueryOperator *) proj, "nesting_eval_help");
			projAttrRef->fromClauseItem = 0;
			projAttrRef->outerLevelsUp = 0;

			FunctionCall *aggFunc = createFunctionCall(aggname,
					singleton(copyObject(projAttrRef)));
			List *aggrs = singleton(aggFunc);
			char *attrName = getTailOfListP(getAttrNames(op->schema));
			AggregationOperator *agg = createAggregationOp(aggrs, NIL, (QueryOperator *) proj, NIL, singleton(strdup(attrName)));

			((QueryOperator *) agg)->parents = singleton(op);
			((QueryOperator *) proj)->parents = singleton(agg);
			op->inputs = LIST_MAKE(lChild, agg);

			//used to change nesting_eval_1 datatype from boolean in nesting op
			//nestingAttrDataType = typeOf(aggFunc);
			nestingAttrDataType = (DataType) getTailOfListInt(getDataTypes(((QueryOperator *) agg)->schema));
		}
		else if(no->nestingType == NESTQ_UNIQUE)
		{

		}
		else if(no->nestingType == NESTQ_SCALAR)
		{
			char *nestAttrName = getTailOfListP(getQueryOperatorAttrNames(op));
			AttributeDef *attrDef = (AttributeDef *) getTailOfListP(rChild->schema->attrDefs);
			attrDef->attrName = nestAttrName;
		}

		no->nestingType = NESTQ_LATERAL;

		////change nesting_eval_1, nesting_eval_2, nesting_eval_... datatype from boolean in nesting op
		AttributeDef *nestAttrDef = (AttributeDef *) getTailOfListP(op->schema->attrDefs);
		nestAttrDef->dataType = nestingAttrDataType;
		int pos = LIST_LENGTH(op->schema->attrDefs) - 1;
		adatpUpNestingAttrDataType(op, nestingAttrDataType, pos);
	}

	//adapt selection: one in condition(nesting_eval_1 = True -> nesting_eval_1 = 1)
	//				 : one in schema (loop nestOpList again since the above loop adapt all nest op firstly, selection need to use the schema nest op)
	FOREACH(QueryOperator, op, nestOpList)
	{
		   QueryOperator *selOp = (QueryOperator *) getHeadOfListP(op->parents);
		   if(isA(selOp, SelectionOperator))
		   {
			   SelectionOperator *sel = (SelectionOperator *) selOp;
			   List *nestOpList = NIL;
			   getNestCondOperator((Operator *) sel->cond, &nestOpList);

			   //one
			   //adapt cond
			   FOREACH(Operator, o, nestOpList)
			   {
				AttributeReference *a = getNthOfListP(o->args, 0);
				//adapt nesting_eval_1 datatype from its children
				AttributeDef *ad = getAttrDefByName(OP_LCHILD(selOp), a->name);
				a->attrType = ad->dataType;

				//change constant datatype from bool to int
				o->args = removeFromTail(o->args);
				o->args = appendToTailOfList(o->args, createConstInt(1));
			   }

			   //one
			   //adapt schema
			   FOREACH(AttributeDef, ad, selOp->schema->attrDefs)
			   {
				   if(strlen(ad->attrName) >= 14)
				   {
					   char *prefix = substr(ad->attrName, 0, 12);
					   if(streq(prefix, "nesting_eval_"))
					   {
						   AttributeDef *adChild = getAttrDefByName(OP_LCHILD(selOp), ad->attrName);
						   ad->dataType = adChild->dataType;
					   }
				   }
		   	   }
		}

	}

	return input;
}

static void
adatpUpNestingAttrDataType(QueryOperator *op, DataType nestingAttrDataType, int pos)
{
	FOREACH(QueryOperator, o, op->parents)
	{
		if(isA(o, NestingOperator))
		{
			AttributeDef *ad = getNthOfListP(o->schema->attrDefs, pos);
			ad->dataType = nestingAttrDataType;
			adatpUpNestingAttrDataType(o, nestingAttrDataType, pos);
		}
	}
}

static void
getNestCondOperator(Operator *o, List **nestOpLists)
{
	if(streq(o->name, "="))
	{
		if(isA(getNthOfListP(o->args, 0), AttributeReference) && isA(getNthOfListP(o->args, 1), Constant))
		{
			AttributeReference *a = getNthOfListP(o->args, 0);
			Constant *c = getNthOfListP(o->args, 1);
			if(strlen(a->name) >= 14)
			{
				char *prefix = substr(a->name, 0, 12);
				if(streq(prefix, "nesting_eval_") && c->constType == 4)
					*nestOpLists = appendToTailOfList(*nestOpLists, o);
			}
		}
	}
	else if(streq(o->name, "OR") || streq(o->name, "AND") )
	{
		getNestCondOperator((Operator *) getNthOfListP(o->args, 0), nestOpLists);
		getNestCondOperator((Operator *) getNthOfListP(o->args, 1), nestOpLists);
	}
}


static List *
getListNestingOperator (QueryOperator *op)
{
    List *result = NIL;
    appendNestingOperator(op, &result);

    return result;
}

static void
appendNestingOperator (QueryOperator *op, List **result)
{
    FOREACH(QueryOperator, p, op->inputs)
    {
    		if(isA(p, NestingOperator))
    			*result = appendToTailOfList(*result, p);

    		 appendNestingOperator(p, result);
    }
}


static int
checkAttr (char *name, QueryOperator *op)
{
	List *attrNames = getQueryOperatorAttrNames(op);
    FOREACH(char, c, attrNames)
	{
    		if(streq(c,name))
    			return 1;
	}

    return 0;
}
