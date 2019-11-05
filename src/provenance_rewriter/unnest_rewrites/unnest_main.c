/*
 * unnest_main.c
 *
 *  Created on: Nov. 2, 2019
 *      Author: Xing
 */

#include "common.h"
#include "provenance_rewriter/unnest_rewrites/unnest_main.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"

#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"

#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "utility/string_utils.h"

static List *unnestRewriteQueryList(List *list);
static QueryOperator *unnestRewriteQuery(QueryOperator *input);
static boolean switchAttr (Node *node, char *attr);
static void adaptSchema (List *attrDefs, char *attr);

static List *getListAggregationOperator (QueryOperator *op);
static void appendAggregationOperator (QueryOperator *op, List **result);
static boolean containOuterLevelAttr(Operator *oper);
static void upPropagateGroupBys(QueryOperator *op, List *attrRefs, List *attrDefs);
static void resetPos(AttributeReference *ar,  List* attrDefs);

//static List *getListNestingOperator (QueryOperator *op);
//static void appendNestingOperator (QueryOperator *op, List **result);
//static int checkAttr (char *name, QueryOperator *op);
//static void adatpUpNestingAttrDataType(QueryOperator *op, DataType nestingAttrDataType, int pos);
//static void getNestCondNode(Node *n, List **nestOpLists);

Node *
unnestTranslateQBModel (Node *qbModel)
{
	DEBUG_LOG("unnest rewrite:");

    if (isA(qbModel, List))
        return (Node *) unnestRewriteQueryList((List *) qbModel);
    else if (IS_OP(qbModel))
         return (Node *) unnestRewriteQuery((QueryOperator *) qbModel);

    FATAL_LOG("cannot unnest rewrite node <%s>", nodeToString(qbModel));

    return NULL;
}


static List *
unnestRewriteQueryList(List *list)
{
    FOREACH(QueryOperator,q,list)
        q_his_cell->data.ptr_value = unnestRewriteQuery(q);

    return list;
}

static QueryOperator *
unnestRewriteQuery(QueryOperator *op)
{
	//TODO: only for scalar
	List *nestOps = getListNestingOperator(op);
	QueryOperator *topOp = NULL;
	FOREACH(QueryOperator, n, nestOps)
	{
		List *inputs = n->inputs;

		//QueryOperator *rchild = getNthOfListP(inputs, 1);
		QueryOperator *rchild = OP_RCHILD(n);
		AttributeDef *agg0 = (AttributeDef *) getHeadOfListP(rchild->schema->attrDefs);

		//TODO: check is it a selection
		SelectionOperator *selTop = (SelectionOperator *) getHeadOfListP(n->parents);
		QueryOperator *selTopOp = (QueryOperator *) selTop;
		switchAttr(selTop->cond, agg0->attrName);
		adaptSchema (selTopOp->schema->attrDefs, agg0->attrName);

		List *aggOpList = getListAggregationOperator (rchild);

		List *ups = NIL;
		List *curs = NIL;
		List *ops = NIL;
		if(LIST_LENGTH(aggOpList) > 0)
		{
			AggregationOperator *agg = (AggregationOperator *) getHeadOfListP(aggOpList);
			QueryOperator *aggOp = (QueryOperator *) agg;
			SelectionOperator *selBot = (SelectionOperator *) OP_LCHILD(agg);
			QueryOperator *selBotOp = (QueryOperator *) selBot;

			List *condList = NIL;
			getSelectionCondOperatorList(selBot->cond, &condList);

			List *newConds = NIL;
			List *rmConds = NIL;
			FOREACH(Operator, oper, condList)
			{
				if(containOuterLevelAttr(oper))
					rmConds = appendToTailOfList(rmConds, oper);
				else
					newConds = appendToTailOfList(newConds, oper);
			}

			DEBUG_NODE_BEATIFY_LOG("newCondList:", newConds);
			DEBUG_NODE_BEATIFY_LOG("rmCondList:", rmConds);
			if(LIST_LENGTH(rmConds) > 0)
			{
				Node *newCond = andExprList(newConds);
				selBot->cond = newCond;

				List *upAttrs = NIL;
				List *curAttrs = NIL;
				FOREACH(Operator, oper, rmConds)
				{
					AttributeReference *a1 = (AttributeReference *) getNthOfListP(oper->args, 0);
					AttributeReference *a2 = (AttributeReference *) getNthOfListP(oper->args, 1);
					ops = appendToTailOfList(ops, oper->name);

					if(a1->outerLevelsUp == 1)
					{
						resetPos(a2, selBotOp->schema->attrDefs);
						upAttrs = appendToTailOfList(upAttrs, copyObject(a1));
						curAttrs = appendToTailOfList(curAttrs, copyObject(a2));
					}
					else
					{
						resetPos(a1, selBotOp->schema->attrDefs);
						upAttrs = appendToTailOfList(upAttrs, copyObject(a2));
						curAttrs = appendToTailOfList(curAttrs, copyObject(a1));
					}
				}

				agg->groupBy = curAttrs;
				List *curAttrsRename = NIL;
				FOREACH(AttributeReference, a, agg->groupBy)
				{
					//TODO: check schema to see if duplicate, then add 1
					char *newName = CONCAT_STRINGS(strdup(a->name), "_1");
					AttributeReference *newa = createFullAttrReference(newName, a->fromClauseItem, a->attrPosition, a->outerLevelsUp, a->attrType);
					curAttrsRename = appendToTailOfList(curAttrsRename, newa);
				}

				List *curDefs = NIL;
				FOREACH(AttributeReference, a, curAttrsRename)
				{

					curDefs = appendToTailOfList(curDefs, createAttributeDef(strdup(a->name), a->attrType));
					selTopOp->schema->attrDefs = appendToTailOfList(selTopOp->schema->attrDefs, createAttributeDef(strdup(a->name), a->attrType));
				}
				aggOp->schema->attrDefs = CONCAT_LISTS(aggOp->schema->attrDefs, copyList(curDefs));

				upPropagateGroupBys(aggOp, curAttrsRename, curDefs);


				ups = upAttrs;
				curs = curAttrsRename;
			}
		}

		JoinOperator *join = createJoinOp (JOIN_CROSS, NULL, inputs, singleton(selTop), NIL);
		QueryOperator *joinOp = (QueryOperator *) join;

		((QueryOperator *)selTop)->inputs = singleton(join);
		FOREACH(QueryOperator, o, inputs)
			o->parents = singleton(join);

		// sel top cond
		if(ups != NIL)
		{
			List *gpConds = NIL;
			//FORBOTH(AttributeReference, a1, a2, ups, curs)
			for(int i=0; i<LIST_LENGTH(ups); i++)
			{
				AttributeReference *a1 = (AttributeReference *) getNthOfListP(ups, i);
				AttributeReference *a2 = (AttributeReference *) getNthOfListP(curs, i);
				char *opName = (char *) getNthOfListP(ops, i);

				AttributeReference *newa1 = createFullAttrReference(strdup(a1->name), 0, 0, 0, a1->attrType);
				AttributeReference *newa2 = createFullAttrReference(strdup(a2->name), 0, 0, 0, a2->attrType);
				resetPos(newa1, joinOp->schema->attrDefs);
				resetPos(newa2, joinOp->schema->attrDefs);
				gpConds = appendToTailOfList(gpConds, createOpExpr(opName, LIST_MAKE(newa1, newa2)));

			}
			selTop->cond = AND_EXPRS(selTop->cond, andExprList(gpConds));
		}

		QueryOperator *topSelOp = OP_FIRST_PARENT(selTopOp);
		List *topSelOpParents = topSelOp->parents;

		DuplicateRemoval *dup = createDuplicateRemovalOp(NIL, topSelOp,  topSelOpParents, NIL);
		topSelOp->parents = singleton(dup);

		if(topSelOpParents != NIL)
		{
			FOREACH(QueryOperator, o, topSelOpParents)
					o->inputs = singleton(dup);
		}

		topOp = (QueryOperator *) dup;
	}

	if(topOp->parents == NIL)
		return topOp;

	return (QueryOperator *) op;
}


static void
resetPos(AttributeReference *ar,  List* attrDefs)
{

	int count1 = 0;
	FOREACH(AttributeDef, ad, attrDefs)
	{
//		if(streq(ar->name, "AGG_GB_ARG1"))
//			DEBUG_LOG("name AGG_GB_ARG1 %s", nodeToString(ar));
		DEBUG_LOG("compare attrDef name %s, count %d", ad->attrName, count1);
		if(streq(ar->name,ad->attrName))
		{
			DEBUG_LOG("map name %s to %s", ar->name, ad->attrName);
			ar->attrPosition = count1;
			DEBUG_LOG("set attr pos of %s to %d", ar->name, count1);
			break;
		}
		count1++;
	}

	//DEBUG_LOG("set attr pos of %s to %d", ar->name, count1);
}


static void
upPropagateGroupBys(QueryOperator *op, List *attrRefs, List *attrDefs)
{
	FOREACH(QueryOperator, p, op->parents)
	{
		if(isA(p, NestingOperator))
			break;

		if(isA(p, ProjectionOperator))
		{
			ProjectionOperator *proj = (ProjectionOperator *) p;
			List *newAttrRefs = NIL;
			FOREACH(AttributeReference, a, attrRefs)
			{
				AttributeReference *newa = copyObject(a);
				resetPos(newa, op->schema->attrDefs);
				newAttrRefs = appendToTailOfList(newAttrRefs, newa);
			}

			proj->projExprs = CONCAT_LISTS(proj->projExprs, copyList(newAttrRefs));
		}
		p->schema->attrDefs = CONCAT_LISTS(p->schema->attrDefs,copyList(attrDefs));
		upPropagateGroupBys(p, attrRefs, attrDefs);
	}
}

static boolean
containOuterLevelAttr(Operator *oper)
{
	List *args = oper->args;
	Node *l = getNthOfListP(args, 0);
	Node *r = getNthOfListP(args, 1);
	if( (isA(l, AttributeReference) && ((AttributeReference *) l)->outerLevelsUp == 1) ||
			(isA(r, AttributeReference) && ((AttributeReference *) r)->outerLevelsUp == 1) )
		return TRUE;
	else
		return FALSE;
}


static List *
getListAggregationOperator (QueryOperator *op)
{
    List *result = NIL;
    appendAggregationOperator(op, &result);

    return result;
}

static void
appendAggregationOperator (QueryOperator *op, List **result)
{
	if(isA(op, AggregationOperator))
	{
		QueryOperator *child = OP_LCHILD(op);
		if(isA(child, SelectionOperator))
			*result = appendToTailOfList(*result, op);
	}

    FOREACH(QueryOperator, p, op->inputs)
    {
    		 appendAggregationOperator(p, result);
    }
}


static void
adaptSchema (List *attrDefs, char *attr)
{
	FOREACH(AttributeDef, ad, attrDefs)
	{
		   char *name = strdup(ad->attrName);
	       if(strlen(name) > 12)
	       {
	    	   	   char *prefix = substr(strdup(name), 0, 12);
	    	   	   if (streq(prefix, "nesting_eval_"))
	    	   		   ad->attrName = strdup(attr);
	       }
	}
}

static boolean
switchAttr (Node *node, char *attr)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *name = strdup(a->name);

       if(strlen(name) > 12)
       {
    	   	   char *prefix = substr(strdup(name), 0, 12);

    	   	   if (streq(prefix, "nesting_eval_"))
    	   		   a->name = strdup(attr);
       }
    }

    return visit(node, switchAttr, attr);
}

