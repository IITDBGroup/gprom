/*
 * lateral_prov_main.c
 *
 *  Created on: July 27, 2018
 *      Author: Xing
 */

#include "common.h"
#include "configuration/option.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "log/logger.h"
#include "model/list/list.h"

#define AGGNAME_SUM backendifyIdentifier("sum")
#define AGGNAME_COUNT backendifyIdentifier("count")
#define AGGNAME_AVG backendifyIdentifier("avg")
#define AGGNAME_MIN backendifyIdentifier("min")
#define AGGNAME_MAX backendifyIdentifier("max")

static List *lateralRewriteQueryList(List *list);
static QueryOperator *lateralRewriteQuery(QueryOperator *input);
static List *getListNestingOperator (QueryOperator *op);
static void appendNestingOperator (QueryOperator *op, List **result);
static int checkAttr (char *name, QueryOperator *op);

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

			ProjectionOperator *proj = createProjectionOp(singleton(caseExpr), (QueryOperator *) agg, NIL, singleton("nesting_eval_1"));

			((QueryOperator *) agg)->parents = singleton(proj);
			((QueryOperator *) proj)->parents = singleton(op);
			op->inputs = LIST_MAKE(lChild, proj);
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
			List *condAttrs = ((Operator*) cond)->args;
			FOREACH(AttributeReference, a, condAttrs)
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
			AggregationOperator *agg = createAggregationOp(aggrs, NIL, (QueryOperator *) proj, NIL, singleton("nesting_eval_1"));

			((QueryOperator *) agg)->parents = singleton(op);
			((QueryOperator *) proj)->parents = singleton(agg);
			op->inputs = LIST_MAKE(lChild, agg);
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
	}

	return input;
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
