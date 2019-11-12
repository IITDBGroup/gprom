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
static boolean adaptAttrName (Node *node, char *attr);
static boolean getScalarAttrName (Node *node, char **attr);
static boolean isNestAttr(AttributeReference *a);
static void adaptSchema (List *attrDefs, char *attr);
static void adaptSchemaByName (List *attrDefs, char *name, char *attr);

static List *getListAggregationOperator (QueryOperator *op);
static void appendAggregationOperator (QueryOperator *op, List **result);
static List *getListSelectionOperatorUp(QueryOperator *op);
static void appendSelectionOperator (QueryOperator *op, List **result);

static boolean containOuterLevelAttr(Operator *oper);
static void upPropagateGroupBys(QueryOperator *op, List *attrRefs, List *attrDefs);
static void resetPos(AttributeReference *ar,  List* attrDefs);
static void removeOperator(QueryOperator *op);
static int getInAttrPos(List *conds);
static char* getInAttrName(Node *node);
static void adaptCond(Operator *oper, List *inputs);
static boolean existsAttr(char *name, List *defs);
static List *duplicateList(List *l);
static List *getCorrelatedAttrRefs(List *conds);
static List *refsToNames(List *attrRefs);
static boolean isCorrelatedCond(Node *node);
//static Node *removeNestCond(Node *n);
//static boolean containNestingAttr(Operator *oper);
static void adaptJoinCondAttrName(Node *n, char *oldName, char* newName);


static QueryOperator *unnestScalar(QueryOperator *op);
static QueryOperator *unnestInClause(QueryOperator *op);
static QueryOperator *unnestExists(QueryOperator *op);

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

QueryOperator *
unnestRewriteQuery(QueryOperator *op)
{
	List *nestOps = getListNestingOperator(op);
	FOREACH(NestingOperator, nest, nestOps)
	{
		QueryOperator *nestOp = (QueryOperator *) nest;

	    switch(nest->nestingType)
	    {
	    case NESTQ_SCALAR:
	    		DEBUG_LOG("unnest scalar:");
	    		unnestScalar(nestOp);
	    		break;
	    case NESTQ_ANY:
	    		DEBUG_LOG("unnest in clause:");
	    		unnestInClause(nestOp);
	    		break;
	    case NESTQ_EXISTS:
    			DEBUG_LOG("unnest in clause:");
    			unnestExists(nestOp);
    			break;
	    default:
	    		FATAL_LOG("no rewrite implemented this nesting type ", nodeToString(nestOp));
	    		return NULL;
	    }
	}

	return (QueryOperator *) op;
}


static QueryOperator *
unnestExists(QueryOperator *op)
{
	//NestingOperator *nest = (NestingOperator *) op;
	List *inputs = op->inputs;
	//List *parents = op->parents;
	QueryOperator *rchild = OP_RCHILD(op);

	//TODO: check is it a selection
	SelectionOperator *selTop = (SelectionOperator *) OP_FIRST_PARENT(op);
	QueryOperator *selTopOp = (QueryOperator *) selTop;
	SelectionOperator *selBot = (SelectionOperator *) OP_LCHILD(rchild);
	//QueryOperator *selBotOp = (QueryOperator *) selBot;

	List *selTopCondList = NIL;
	getSelectionCondOperatorList(selTop->cond, &selTopCondList);
	int inCondPos = getInAttrPos(selTopCondList);
	//Node *inCond = getNthOfListP(selTopCondList, inCondPos);
	//char *nestAttrName = getInAttrName(inCond);

	List *newCondList = removeListElemAtPos(selTopCondList, inCondPos);
	Node *newCond = andExprList(newCondList);
	selTop->cond = newCond;

	List *selBotCondList = NIL;
	getSelectionCondOperatorList(selBot->cond, &selBotCondList);

	List *newConds = NIL;
	List *rmConds = NIL;
	FOREACH(Operator, oper, selBotCondList)
	{
		if(containOuterLevelAttr(oper))
			rmConds = appendToTailOfList(rmConds, oper);
		else
			newConds = appendToTailOfList(newConds, oper);
	}

	if(LIST_LENGTH(rmConds) > 0) //with correlation:
	{
		//if only contain correlated condition, need to remove selection operator
		if(newConds != NIL)
		{
			Node *newCond = andExprList(newConds);
			selBot->cond = newCond;
		}
		else
		{
			removeOperator((QueryOperator *) selBot);
		}
	}

//	Constant *limitExpr = createConstInt(1);
//	LimitOperator *limit = createLimitOp((Node *) limitExpr, NULL, rchild, NIL);
//	rchild->parents = singleton(limit);
//	replaceNode(inputs, rchild, limit);

	List *newProjExprs = getCorrelatedAttrRefs(rmConds);
	FOREACH(AttributeReference, a, newProjExprs)
		resetPos(a, rchild->schema->attrDefs);
	List *newProjAttrNames = refsToNames(newProjExprs);

	ProjectionOperator *newProj = createProjectionOp(newProjExprs, rchild, NIL, newProjAttrNames);
	rchild->parents = singleton(newProj);
	QueryOperator *newProjOp = (QueryOperator *) newProj;

	List* dupExprs = duplicateList(newProjExprs);
	FOREACH(AttributeReference, a, dupExprs)
		resetPos(a, newProjOp->schema->attrDefs);
	List *dupAttrNames = refsToNames(dupExprs);

	DuplicateRemoval *dup = createDuplicateRemovalOp(dupExprs, newProjOp, NIL, dupAttrNames);
	newProjOp->parents = singleton(dup);
	replaceNode(inputs, rchild, dup);

	JoinOperator *join = createJoinOp (JOIN_INNER, NULL, inputs, singleton(selTop), NIL);
	QueryOperator *joinOp = (QueryOperator *) join;

	((QueryOperator *)selTop)->inputs = singleton(join);
	FOREACH(QueryOperator, o, inputs)
		o->parents = singleton(join);

	List *selTopSchema = duplicateList(joinOp->schema->attrDefs);
	selTopOp->schema->attrDefs = selTopSchema;

	//adapt condition: outlevelup and pos
	FOREACH(Operator, oper, rmConds)
		adaptCond(oper, inputs);

	Node *newJoinCond = andExprList(rmConds);
	join->cond = newJoinCond;


	return op;
}



static QueryOperator *
unnestInClause(QueryOperator *op)
{
	NestingOperator *nest = (NestingOperator *) op;
	List *inputs = op->inputs;
	List *parents = op->parents;
	QueryOperator *rchild = OP_RCHILD(op);

	//TODO: change name
	AttributeDef *ad = getAttrDef(rchild, 0);
	char *oldName = strdup(ad->attrName);
	char *newName = CONCAT_STRINGS(strdup(ad->attrName), "_1");
	ad->attrName = newName;
	//adapt nest cond
	adaptJoinCondAttrName(nest->cond, oldName, newName);

	// add duplicate removal operator
	DuplicateRemoval *dup = createDuplicateRemovalOp(NIL, rchild,  NIL, NIL);
	rchild->parents = singleton(dup);
	inputs = LIST_MAKE(OP_LCHILD(op), dup);

	// change nesting operator to join
	JoinOperator *join = createJoinOp (JOIN_INNER, copyObject(nest->cond), inputs, parents, NIL);
	//QueryOperator *joinOp = (QueryOperator *) join;

	FOREACH(QueryOperator, o, parents)
		replaceNode(o->inputs, op, join);
	//o->inputs = singleton(join);

	FOREACH(QueryOperator, o, inputs)
		o->parents = singleton(join);

	QueryOperator *topSelOp = OP_FIRST_PARENT(op);
	while(!isA(topSelOp, SelectionOperator))
	{
		topSelOp = OP_FIRST_PARENT(topSelOp);
	}

	if(isA(topSelOp, SelectionOperator))
	{
		SelectionOperator *topSel = (SelectionOperator *) topSelOp;

		List *condList = NIL;
		getSelectionCondOperatorList(topSel->cond, &condList);
		int inCondPos = getInAttrPos(condList);
		DEBUG_LOG("len: %d, inCondPos: %d", LIST_LENGTH(condList), inCondPos);
		Node *inCond = getNthOfListP(condList, inCondPos);
		char *nestAttrName = getInAttrName(inCond);
		DEBUG_LOG("nestAttrName: %s", nestAttrName);

		List *newCondList = removeListElemAtPos(condList, inCondPos);
		Node *newCond = andExprList(newCondList);
		topSel->cond = newCond;

		AttributeDef *nestAttr = (AttributeDef *) getHeadOfListP(rchild->schema->attrDefs);

		QueryOperator *topNestOp = OP_FIRST_PARENT(op);
		while(!isA(topNestOp, SelectionOperator))
		{
			adaptSchemaByName(topNestOp->schema->attrDefs, nestAttrName, nestAttr->attrName);
			topNestOp = OP_FIRST_PARENT(topNestOp);
		}
		adaptSchemaByName(topNestOp->schema->attrDefs, nestAttrName, nestAttr->attrName);

//		Node *newCond = removeNestCond(topSel->cond);
//		topSel->cond = newCond;
//		AttributeDef *nestAttr = (AttributeDef *) getHeadOfListP(rchild->schema->attrDefs);
//		adaptSchema (topNestOp->schema->attrDefs, nestAttr->attrName);
	}


	return op;
}

static QueryOperator *
unnestScalar(QueryOperator *op)
{
	List *inputs = op->inputs;

	//QueryOperator *rchild = getNthOfListP(inputs, 1);
	QueryOperator *rchild = OP_RCHILD(op);
	AttributeDef *agg0 = (AttributeDef *) getHeadOfListP(rchild->schema->attrDefs);
	//TODO: check is it a selection
	SelectionOperator *selTop = (SelectionOperator *) getHeadOfListP(op->parents);
	QueryOperator *selTopOp = (QueryOperator *) selTop;
	char *nestAttrName = NULL;

	//TODO: handle having
	if(!isCorrelatedCond(selTop->cond))
	{
		List *selUpList = getListSelectionOperatorUp(selTopOp);
		FOREACH(SelectionOperator, s, selUpList)
		{
			if(isCorrelatedCond(s->cond))
			{
				selTop = s;
				selTopOp = (QueryOperator *) selTop;
			}
		}
	}

	getScalarAttrName(selTop->cond, &nestAttrName);
	adaptAttrName(selTop->cond, agg0->attrName);
	if(nestAttrName != NULL)
		adaptSchemaByName (selTopOp->schema->attrDefs, nestAttrName, agg0->attrName);
	else
		adaptSchema (selTopOp->schema->attrDefs, agg0->attrName);
	//adaptSchema (selTopOp->schema->attrDefs, agg0->attrName);

	List *aggOpList = getListAggregationOperator (rchild);

	List *ups = NIL;
	List *curs = NIL;
	List *ops = NIL;
	if(LIST_LENGTH(aggOpList) > 0) //with aggregation
	{
		AggregationOperator *agg = (AggregationOperator *) getHeadOfListP(aggOpList);
		QueryOperator *aggOp = (QueryOperator *) agg;

		QueryOperator *selBotOp = OP_LCHILD(agg); //
		while(!isA(selBotOp,SelectionOperator))
		{
			if(selBotOp->inputs != NIL)
				selBotOp = OP_LCHILD(selBotOp);
			else
				break; //TODO: handle no selection

		}
		//SelectionOperator *selBot = (SelectionOperator *) OP_LCHILD(agg);
		//QueryOperator *selBotOp = (QueryOperator *) selBot;
		SelectionOperator *selBot = (SelectionOperator *) selBotOp;

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

		/*
		 * with correlation
		 * 1. remove correlated condition from bottom selection
		 * 2. add group by attribute into aggregation and propagate up until nesting operator
		 * 3. add correlated condition into top selection and add correlated attribute into schema
		 */
		if(LIST_LENGTH(rmConds) > 0) //with correlation:
		{
			//if only contain correlated condition, need to remove selection operator
			if(newConds != NIL)
			{
				Node *newCond = andExprList(newConds);
				selBot->cond = newCond;
			}
			else
			{
				removeOperator((QueryOperator *)selBot);
			}

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
		else //no correlation: add duplicate removal to the right child of nesting operator
		{
			DuplicateRemoval *dup = createDuplicateRemovalOp(NIL, rchild,  NIL, NIL);
			rchild->parents = singleton(dup);
			inputs = LIST_MAKE(OP_LCHILD(op), dup);
		}
	}

	JoinOperator *join = createJoinOp (JOIN_CROSS, NULL, inputs, singleton(selTop), NIL);
	QueryOperator *joinOp = (QueryOperator *) join;

	((QueryOperator *)selTop)->inputs = singleton(join);
	FOREACH(QueryOperator, o, inputs)
		o->parents = singleton(join);

	// sel top cond: 3. add correlation cond
	if(ups != NIL)
	{
		List *gpConds = NIL;
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

	return op;
}


//TODO: might use visit
static void
adaptJoinCondAttrName(Node *n, char *oldName, char* newName)
{
	Operator *oper = (Operator *) n;
	List *attrs = oper->args;
	Node *n1 = getNthOfListP(attrs, 0);
	Node *n2 = getNthOfListP(attrs, 1);

	if(isA(n1, AttributeReference))
	{
		AttributeReference *a1 = (AttributeReference *) n1;
		if(streq(a1->name, oldName))
			a1->name = newName;

	}

	if(isA(n2, AttributeReference))
	{
		AttributeReference *a2 = (AttributeReference *) n2;
		if(streq(a2->name, oldName))
			a2->name = newName;
	}
}

//static Node *
//removeNestCond(Node *n)
//{
//	List *condList = NIL;
//	getSelectionCondOperatorList(n, &condList);
//
//	List *newConds = NIL;
//	FOREACH(Operator, oper, condList)
//	{
//		if(!containNestingAttr(oper))
//			newConds = appendToTailOfList(newConds, oper);
//	}
//
//	Node *newCond = andExprList(newConds);
//
//	return newCond;
//}

//static boolean
//containNestingAttr(Operator *oper)
//{
//	List *args = oper->args;
//	Node *l = getNthOfListP(args, 0);
//	Node *r = getNthOfListP(args, 1);
//
//	if(isA(l, AttributeReference))
//	{
//		char *lName = ((AttributeReference *) l)->name;
//		if(strlen(lName) > 12)
//		{
//			char *prefix = substr(strdup(lName), 0, 12);
//
//			if (streq(prefix, "nesting_eval_"))
//				return TRUE;
//		}
//	}
//
//	if(isA(r, AttributeReference))
//	{
//		char *rName = ((AttributeReference *) r)->name;
//		if(strlen(rName) > 12)
//		{
//			char *prefix = substr(strdup(rName), 0, 12);
//
//			if (streq(prefix, "nesting_eval_"))
//				return TRUE;
//		}
//	}
//
//	return FALSE;
//}

static boolean
isCorrelatedCond(Node *node)
{
	List *condList = NIL;
	getSelectionCondOperatorList(node, &condList);

	FOREACH(Operator, oper, condList)
	{
		Node *nl = getNthOfListP(oper->args, 0);
		Node *nr = getNthOfListP(oper->args, 1);

		//|| isA(nr, Constant) && ((Constant *)nr)->constType != DT_BOOL
		if(isA(nl, AttributeReference) && isA(nr, AttributeReference) )
		{
			AttributeReference *al = (AttributeReference *) nl;
			AttributeReference *ar = (AttributeReference *) nr;

			char *namel = strdup(al->name);
			char *namer = strdup(ar->name);

			if(strlen(namel) > 12)
			{
				char *prefix = substr(strdup(namel), 0, 12);

				if (streq(prefix, "nesting_eval_"))
					return TRUE;
			}

			if(strlen(namer) > 12)
			{
				char *prefix = substr(strdup(namer), 0, 12);

				if (streq(prefix, "nesting_eval_"))
					return TRUE;
			}

		}
	}

	return FALSE;
}

static List *
getListSelectionOperatorUp (QueryOperator *op)
{
    List *result = NIL;
    appendSelectionOperator(op, &result);

    return result;
}

static void
appendSelectionOperator (QueryOperator *op, List **result)
{
	FOREACH(QueryOperator, p, op->parents)
    	{
		if(isA(op, SelectionOperator))
			*result = appendToTailOfList(*result, op);

		appendSelectionOperator(p, result);
    	}
}

static List *
refsToNames(List *attrRefs)
{
	List *l = NIL;
	FOREACH(AttributeReference, a, attrRefs)
	{
		//AttributeDef *ad = createAttributeDef(strdup(a->name), a->attrType);
		l = appendToTailOfList(l, strdup(a->name));
	}

	return l;
}


static List *
getCorrelatedAttrRefs(List *conds)
{
	List *l = NIL;

	FOREACH(Operator, oper, conds)
	{
		AttributeReference *al = (AttributeReference *) getNthOfListP(oper->args, 0);
		AttributeReference *ar = (AttributeReference *) getNthOfListP(oper->args, 1);

		if(al->fromClauseItem != 1)
		{
			//AttributeDef *ad = createAttributeDef(strdup(al->name), al->attrType);
			l = appendToTailOfList(l, copyObject(al));
		}
		else
		{
			//AttributeDef *ad = createAttributeDef(strdup(ar->name), ar->attrType);
			l = appendToTailOfList(l, copyObject(ar));
		}
	}

	return l;
}

static List *
duplicateList(List *l)
{
	List *result = NIL;
	FOREACH(Node, n, l)
		result = appendToTailOfList(result, copyObject(n));

	return result;
}

static boolean
existsAttr(char *name, List *defs)
{
	FOREACH(AttributeDef, ad, defs)
	{
		if(streq(name, ad->attrName))
			return TRUE;
	}

	return FALSE;
}

static void
adaptCond(Operator *oper, List *inputs)
{
	AttributeReference *al = (AttributeReference *) getNthOfListP(oper->args, 0);
	AttributeReference *ar = (AttributeReference *) getNthOfListP(oper->args, 1);

	QueryOperator *lchild =  (QueryOperator *) getNthOfListP(inputs, 0);
	QueryOperator *rchild =  (QueryOperator *) getNthOfListP(inputs, 1);

	List *lschema = lchild->schema->attrDefs;
	List *rschema = rchild->schema->attrDefs;

	if(existsAttr(al->name, lschema))
	{
		resetPos(al, lschema);
		resetPos(ar, rschema);
		al->fromClauseItem = 0;
		ar->fromClauseItem = 1;

	}
	else
	{
		resetPos(ar, lschema);
		resetPos(al, rschema);
		al->fromClauseItem = 1;
		ar->fromClauseItem = 0;
	}

	al->outerLevelsUp = 0;
	ar->outerLevelsUp = 0;
}


static void
removeOperator(QueryOperator *op)
{
	QueryOperator *child = OP_LCHILD(op);

	child->parents = op->parents;
	FOREACH(QueryOperator, o, op->parents)
		o->inputs = singleton(child);
}

static char*
getInAttrName (Node *node)
{
	Operator *oper = (Operator *) node;
    Node *nl = getNthOfListP(oper->args, 0);
    Node *nr = getNthOfListP(oper->args, 1);

    if(isA(nl, AttributeReference))
    		return ((AttributeReference *) nl)->name;
    else
    		return ((AttributeReference *) nr)->name;
}

static int
getInAttrPos(List *conds)
{
	int count = 0;
	FOREACH(Operator, oper, conds)
	{
	    Node *nl = getNthOfListP(oper->args, 0);
	    Node *nr = getNthOfListP(oper->args, 1);
	    if((isA(nl, AttributeReference) && isA(nr, Constant) && ((Constant *)nr)->constType == DT_BOOL) ||
	    		(isA(nr, AttributeReference) && isA(nl, Constant) && ((Constant *)nl)->constType == DT_BOOL))
	    {
	    		return count;
	    }
	    count ++;
	}

	return -1;
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
adaptSchemaByName (List *attrDefs, char *name, char *attr)
{
	FOREACH(AttributeDef, ad, attrDefs)
	{
	       if(streq(ad->attrName, name))
	    	   	   ad->attrName = strdup(attr);
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
isNestAttr(AttributeReference *a)
{
	char *name = strdup(a->name);
	if(strlen(name) > 12)
	{
		char *prefix = substr(strdup(name), 0, 12);

		if (streq(prefix, "nesting_eval_"))
			return TRUE;
	}

	return FALSE;
}

static boolean
adaptAttrName (Node *node, char *attr)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, Operator))
    {
    		Operator *oper = (Operator *) node;
    		Node *nl = getNthOfListP(oper->args, 0);
    		Node *nr = getNthOfListP(oper->args, 1);

    		//|| isA(nr, Constant) && ((Constant *)nr)->constType != DT_BOOL
    		if(isA(nl, AttributeReference) && isA(nr, AttributeReference) )
    		{
    			AttributeReference *al = (AttributeReference *) nl;
    			AttributeReference *ar = (AttributeReference *) nr;
    			if(isNestAttr(al))
    				al->name = attr;
    			if(isNestAttr(ar))
    				ar->name = attr;
    		}
    }

//    if (isA(node, AttributeReference))
//    {
//        AttributeReference *a = (AttributeReference *) node;
//        char *name = strdup(a->name);
//
//       if(strlen(name) > 12)
//       {
//    	   	   char *prefix = substr(strdup(name), 0, 12);
//
//    	   	   if (streq(prefix, "nesting_eval_"))
//    	   		   a->name = strdup(attr);
//       }
//    }

    return visit(node, adaptAttrName, attr);
}



static boolean
getScalarAttrName (Node *node, char **attr)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, Operator))
    {
    		Operator *oper = (Operator *) node;
    		Node *nl = getNthOfListP(oper->args, 0);
    		Node *nr = getNthOfListP(oper->args, 1);

    		//|| isA(nr, Constant) && ((Constant *)nr)->constType != DT_BOOL
    		if(isA(nl, AttributeReference) && isA(nr, AttributeReference) )
    		{
    			AttributeReference *al = (AttributeReference *) nl;
    			AttributeReference *ar = (AttributeReference *) nr;
    			if(isNestAttr(al))
    				*attr = al->name;
    			if(isNestAttr(ar))
    				*attr = ar->name;
    		}
    }
    return visit(node, getScalarAttrName, attr);
}
