/*
 * lateral_prov_main.c
 *
 *  Created on: July 27, 2018
 *      Author: Xing
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "model/set/hashmap.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "utility/string_utils.h"

#define NESTING_HELP_ATTR_NAME backendifyIdentifier("nesting_eval_help")
#define FIRST_AGG_NAME backendifyIdentifier("aggr_0")

static List *lateralRewriteQueryList(List *list);
static QueryOperator *lateralRewriteQuery(QueryOperator *input);
//static List *getListNestingOperator (QueryOperator *op);
static void appendNestingOperator (QueryOperator *op, List **result);
static int checkAttr (char *name, QueryOperator *op);
static void adatpUpNestingAttrDataType(QueryOperator *op, DataType nestingAttrDataType, int pos);
static void getNestCondNode(Node *n, List **nestOpLists);

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
	HashMap *nestingResultAttrToNewExpr = NEW_MAP(Constant,Node);

	FOREACH(QueryOperator, op, nestOpList)
	{
		QueryOperator *rChild = OP_RCHILD(op);
		QueryOperator *lChild = OP_LCHILD(op);

		Constant *c0 = createConstInt(0);
		Constant *c1a = createConstInt(1);
		Constant *c2  = createConstInt(2);

		NestingOperator *no = (NestingOperator *) op;
		char *aggname;
		DataType nestingAttrDataType = DT_INT;
		char *nestResultAttr = strdup(((AttributeDef *) getTailOfListP(op->schema->attrDefs))->attrName);

		/* ********************************************************************************
		 * EXISTS SUBQUERIES
		 *
		 * EXISTS Q -> (SELECT count(1) > 0 FROM Q)
		 */
		if(no->nestingType == NESTQ_EXISTS)
		{
			aggname = COUNT_FUNC_NAME;

			//create aggregation COUNT(1) AS "nesting_eval_1"
			FunctionCall *aggFunc = createFunctionCall(COUNT_FUNC_NAME,
													   singleton(copyObject(c1a)));
			List *aggrs = singleton(aggFunc);
			AggregationOperator *agg = createAggregationOp(aggrs, NIL, rChild, NIL, singleton(FIRST_AGG_NAME));
			rChild->parents = singleton(agg);
			SET_BOOL_STRING_PROP(agg, PROP_OPT_AGGREGATION_BY_LATREAL_WRITE);

			//create projectioin SELECT count(*) > 0 AS "nesting_eval_1"
			AttributeReference *aggAttrRef = getAttrRefByName((QueryOperator *) agg, FIRST_AGG_NAME);

			// count(*) > 0 ONLY!
			//WHEN count(*) > 0 THEN 1
			Operator *whenOperator = createOpExpr(OPNAME_GT, LIST_MAKE(copyObject(aggAttrRef), copyObject(c0)));
			/* CaseWhen *when = createCaseWhen((Node *) whenOperator, (Node *) copyObject(c1a)); // (Node *) createConstFloat(0.0)); */
			//ELSE 0
			/* Constant *el =  copyObject(c0); */
			/* CaseExpr *caseExpr = createCaseExpr(NULL, singleton(when), (Node *) el); */



			char *attrName = getTailOfListP(getAttrNames(op->schema));
			ProjectionOperator *proj = createProjectionOp(singleton(whenOperator), (QueryOperator *) agg, NIL, singleton(strdup(attrName)));

			((QueryOperator *) agg)->parents = singleton(proj);
			((QueryOperator *) proj)->parents = singleton(op);
			op->inputs = LIST_MAKE(lChild, proj);

			//used to change nesting_eval_1 datatype from boolean in nesting op
			nestingAttrDataType = DT_BOOL; // (DataType) getTailOfListInt(getDataTypes(((QueryOperator *) proj)->schema));

			// reference to the booling nesting_eval_xxx is replaced with nesting_eval_xxx = 1
			/* MAP_ADD_STRING_KEY(nestingResultAttrToNewExpr, nestResultAttr, */
			/* 				   createOpExpr(OPNAME_EQ, */
			/* 								LIST_MAKE(createFullAttrReference(strdup(nestResultAttr), */
			/* 																  0, */
			/* 																  0, */
			/* 																  INVALID_ATTR, */
			/* 																  nestingAttrDataType), */
			/* 										  copyObject(c1a)))); */
		}
		/* ********************************************************************************
		 * ANY OR ALL
		 *
		 * we add additional correlations for the expression E using in E op ANY / ALL (SELECT expr2 ...
		 *
		 * for expr op ANY (SELECT expr2 ...)
		 * -> SELECT (CASE WHEN nesting_helper IS NULL THEN FALSE    -- empty subquery ANY -> FALSE
		 *                 WHEN nesting_helper = 1 THEN NULL         -- only NULL or NULL and FALSE -> NULL
		 *			       ELSE nesting_helper = 2) AS nesting_eval_xxx  -- otherwise return max
		 *	  FROM (SELECT max(CASE WHEN expr op expr2 THEN 2 WHEN expr IS NULL OR expr2 IS NULL THEN 1 ELSE 0 END) AS nesting
		 *	  ...
		 *
		 * for expr op ALL (SELECT expr2 ...)
		 * -> SELECT (CASE WHEN nesting_helper IS NULL THEN TRUE     -- empty subquery ALL -> TRUE
		 *                 WHEN nesting_helper = 1 THEN NULL         -- some NULL and maybe some TRUE -> NULL
		 *			       ELSE nesting_helper = 2) AS nesting_eval_xxx -- only TRUE comparisons? If yes then -> TRUE
		 *	  FROM (SELECT min(CASE WHEN expr op expr2 THEN 2 WHEN expr IS NULL OR expr2 IS NULL THEN 1 ELSE 0 END) AS nesting
		 *	  ...
		 */
		else if (no->nestingType == NESTQ_ANY || no->nestingType == NESTQ_ALL)
		{
			if(no->nestingType == NESTQ_ANY)
				aggname = MAX_FUNC_NAME;
			else if(no->nestingType == NESTQ_ALL)
				aggname = MIN_FUNC_NAME;

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

			/*
			 * for condition "expr op expr2"
			 * SELECT MAX((
			 * CASE WHEN expr op expr2 THEN 2        -- 2 encodes a TRUE condition
      	  	 * WHEN (expr op expr2) IS NULL THEN 1   -- 1 encodes NULL
      	  	 * ELSE 0 END)) AS "nesting_eval_1"      -- 0 encodes FALSE
			 */

			// 2 (condition is true)
			CaseWhen *when3 = createCaseWhen((Node *) cond, (Node *) copyObject(c2));

			// 1 (condition is null)
			List *attrRefsInCond = getAttrReferences((Node *) cond);
			List *isNullList = NIL;
			FOREACH(AttributeReference, a, attrRefsInCond)
			{
				IsNullExpr *inulExpr = createIsNullExpr((Node *) singleton(copyObject(a)));
				isNullList = appendToTailOfList(isNullList, inulExpr);
			}
			Operator *orExpr = createOpExpr(OPNAME_OR, isNullList);
			CaseWhen *when2 = createCaseWhen((Node *) orExpr, (Node *) copyObject(c1a));

			// 0 else (condition is false)
			Constant *el =  copyObject(c0);
			CaseExpr *caseExpr = createCaseExpr(NULL, LIST_MAKE(when3, when2), (Node *) el);

			ProjectionOperator *proj = createProjectionOp(singleton(caseExpr), rChild, NIL, singleton(NESTING_HELP_ATTR_NAME));
			rChild->parents = singleton(proj);

			// MAX
			AttributeReference *projAttrRef = getAttrRefByName((QueryOperator *) proj, NESTING_HELP_ATTR_NAME);
			projAttrRef->fromClauseItem = 0;
			projAttrRef->outerLevelsUp = 0;

			FunctionCall *aggFunc = createFunctionCall(aggname,
					singleton(copyObject(projAttrRef)));
			List *aggrs = singleton(aggFunc);
			char *attrName = getTailOfListP(getAttrNames(op->schema));
			AggregationOperator *agg = createAggregationOp(aggrs, NIL, (QueryOperator *) proj, NIL, singleton(strdup(attrName)));

//			((QueryOperator *) agg)->parents = singleton(op);
			((QueryOperator *) proj)->parents = singleton(agg);
//			op->inputs = LIST_MAKE(lChild, agg);
			SET_BOOL_STRING_PROP(agg, PROP_OPT_AGGREGATION_BY_LATREAL_WRITE);

			/*
			 * SELECT CASE WHEN "nesting_eval_1" IS NULL THEN FALSE / TRUE  (for ANY / ALL)
			 *             WHEN "nesting_eval_1" = 1 THEN NULL
			 *             ELSE "nesting_eval_1" END AS "nesting_eval_1"
			 */
			//WHEN "nesting_eval_1" = 2 THEN NULL
			AttributeReference *projUpAttrRef = getAttrRefByName((QueryOperator *) agg, attrName);
			Constant *cNull = createNullConst(DT_BOOL); //projUpAttrRef->attrType);
			Operator *opr2 = createOpExpr(OPNAME_EQ, LIST_MAKE(copyObject(projUpAttrRef), copyObject(c1a)));
			CaseWhen *projUpwhen2 = createCaseWhen((Node *) opr2, (Node *) cNull);

			//SELECT CASE WHEN "nesting_eval_1" IS NULL THEN TRUE  (handle ALL - NULL CASE)
			//SELECT CASE WHEN "nesting_eval_1" IS NULL THEN FALSE  (handle NOT ANY - NULL CASE)
			IsNullExpr *projUpinulExpr = createIsNullExpr((Node *) singleton(copyObject(projUpAttrRef)));
			CaseWhen *projUpwhen31 = createCaseWhen((Node *) projUpinulExpr, (Node *) createConstBool(TRUE));
			CaseWhen *projUpwhen30 = createCaseWhen((Node *) projUpinulExpr, (Node *) createConstBool(FALSE));
			List *projUpWhenClasues = NIL;
			if(no->nestingType == NESTQ_ANY)
				projUpWhenClasues = LIST_MAKE(projUpwhen30, projUpwhen2);
			else if(no->nestingType == NESTQ_ALL)
				projUpWhenClasues = LIST_MAKE(projUpwhen31, projUpwhen2);

			//ELSE
			Operator *oprTrue = createOpExpr(OPNAME_EQ, LIST_MAKE(copyObject(projUpAttrRef), copyObject(c2)));

			CaseExpr *projUpCaseExpr = createCaseExpr(NULL, projUpWhenClasues, (Node *) oprTrue);

			ProjectionOperator *projUp = createProjectionOp(singleton(projUpCaseExpr), (QueryOperator *) agg, NIL, singleton(strdup(attrName)));
			((QueryOperator *) agg)->parents = singleton(projUp);

			((QueryOperator *) projUp)->parents = singleton(op);
			op->inputs = LIST_MAKE(lChild, projUp);

			//used to change nesting_eval_1 datatype from boolean in nesting op
			//nestingAttrDataType = typeOf(aggFunc);
			nestingAttrDataType = DT_BOOL; // (DataType) getTailOfListInt(getDataTypes(((QueryOperator *) agg)->schema));

			// the new condition becomes
			MAP_ADD_STRING_KEY(nestingResultAttrToNewExpr, nestResultAttr,
							   createOpExpr(OPNAME_EQ,
											LIST_MAKE(createFullAttrReference(strdup(nestResultAttr),
																			  0,
																			  0,
																			  INVALID_ATTR,
																			  nestingAttrDataType),
													  copyObject(c1a))));
		}
		// ********************************************************************************
		// UNIQUE
		else if(no->nestingType == NESTQ_UNIQUE)
		{
			FATAL_LOG("lateral rewrite for UNIQUE not supported yet.");
		}
		/* ********************************************************************************
		 * SCALAR
		 *
		 */
		//TODO need to inject a failure if subquery returns more than one tuple (unless we can statically prove that this cannot happen
		else if(no->nestingType == NESTQ_SCALAR)
		{
			char *nestAttrName = getTailOfListP(getQueryOperatorAttrNames(op));
			AttributeDef *attrDef = (AttributeDef *) getTailOfListP(rChild->schema->attrDefs);
			attrDef->attrName = nestAttrName;
            nestingAttrDataType = attrDef->dataType;
		}

		// change nesting type to LATERAL
		no->nestingType = NESTQ_LATERAL;

		////change nesting_eval_1, nesting_eval_2, nesting_eval_... datatype from boolean in nesting op
		AttributeDef *nestAttrDef = (AttributeDef *) getTailOfListP(op->schema->attrDefs);
		nestAttrDef->dataType = nestingAttrDataType;
		int pos = LIST_LENGTH(op->schema->attrDefs) - 1;
		adatpUpNestingAttrDataType(op, nestingAttrDataType, pos); //TODO should be unnecessary for salar!
	}

	//adapt selection: one in condition(nesting_eval_1 = True -> nesting_eval_1 = 3)
	//				 : one in schema (loop nestOpList again since the above loop adapt all nest op firstly, selection need to use the schema nest op)
	FOREACH(QueryOperator, op, nestOpList)
	{
		   QueryOperator *selOp = (QueryOperator *) getHeadOfListP(op->parents);
		   if(isA(selOp, SelectionOperator))
		   {
			   SelectionOperator *sel = (SelectionOperator *) selOp;
			   List *nestOpList = NIL;
			   getNestCondNode(sel->cond, &nestOpList);

			   //one
			   //adapt cond
			   FOREACH(Node, n, nestOpList)
			   {
				   if(isA(n, Operator))
				   {
					   Operator *o = (Operator *) n;
					   AttributeReference *a = getNthOfListP(o->args, 0);
					   //adapt nesting_eval_1 datatype from its children
					   AttributeDef *ad = getAttrDefByName(OP_LCHILD(selOp), a->name);
					   a->attrType = ad->dataType;

					   //change constant datatype from bool to int
					   o->args = removeFromTail(o->args);
					   o->args = appendToTailOfList(o->args, createConstInt(1));
				   }
				   else if(isA(n, IsNullExpr))
				   {
					   IsNullExpr *inulExpr = (IsNullExpr *) n;
					   List *argList = (List *)(inulExpr->expr);
					   AttributeReference *a = getNthOfListP(argList, 0);
					   //adapt nesting_eval_1 datatype from its children
					   AttributeDef *ad = getAttrDefByName(OP_LCHILD(selOp), a->name);
					   a->attrType = ad->dataType;
				   }
			   }

			   //one
			   //adapt schema
			   FOREACH(AttributeDef, ad, selOp->schema->attrDefs)
			   {
				   if(isPrefix(ad->attrName, getNestingAttrPrefix()))
				   {
					   AttributeDef *adChild = getAttrDefByName(OP_LCHILD(selOp), ad->attrName);
					   ad->dataType = adChild->dataType;
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
getNestCondNode(Node *n, List **nestOpLists)
{
	if(isA(n, Operator))
	{
		Operator *o = (Operator *) n;
		if(streq(o->name, OPNAME_EQ))
		{
			if(isA(getNthOfListP(o->args, 0), AttributeReference) && isA(getNthOfListP(o->args, 1), Constant))
			{
				AttributeReference *a = getNthOfListP(o->args, 0);
				Constant *c = getNthOfListP(o->args, 1);
				if(isPrefix(a->name, getNestingAttrPrefix()) && c->constType == 4) //TODO DT_BOOL?
				{
						*nestOpLists = appendToTailOfList(*nestOpLists, n);
				}
			}
		}
		else if(streq(strToUpper(o->name), OPNAME_OR) || streq(strToUpper(o->name), OPNAME_AND) || streq(strToUpper(o->name), OPNAME_NOT))
		{
			FOREACH(Node, n, o->args)
				getNestCondNode(n, nestOpLists);
		}
	}
	else if(isA(n, IsNullExpr))
	{
		IsNullExpr *inulExpr = (IsNullExpr *) n;
		List *argList = (List *)(inulExpr->expr);
		if(isA(getNthOfListP(argList, 0), AttributeReference))
		{
			AttributeReference *a = getNthOfListP(argList, 0);
			if(isPrefix(a->name, getNestingAttrPrefix()))
			{
				*nestOpLists = appendToTailOfList(*nestOpLists, n);
			}
		}
	}
}


List *
getListNestingOperator(QueryOperator *op)
{
    List *result = NIL;
    appendNestingOperator(op, &result);

    return result;
}

static void
appendNestingOperator(QueryOperator *op, List **result)
{
    FOREACH(QueryOperator, p, op->inputs)
    {
		if(isA(p, NestingOperator))
			*result = appendToTailOfList(*result, p);

		appendNestingOperator(p, result);
    }
}


static int
checkAttr(char *name, QueryOperator *op)
{
	List *attrNames = getQueryOperatorAttrNames(op);
    FOREACH(char, c, attrNames)
	{
		if(streq(c,name))
			return 1;
	}

    return 0;
}
