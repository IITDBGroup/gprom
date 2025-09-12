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
static void adaptSchema (List *attrDefs, char *attr);
static void adaptSchemaByName (List *attrDefs, char *name, char *attr);
static void upPropAdaptSchemaByName(QueryOperator *op, char *name, char *attr);

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
static void adaptCond(Operator *oper, List *inputs); //might use following one
static boolean adaptCondNode (Node *node, List *defs);
static boolean existsAttr(char *name, List *defs);
static List *duplicateList(List *l);
static List *getCorrelatedAttrRefs(List *conds);
static List *refsToNames(List *attrRefs);
static boolean isCorrelatedCond(Node *node);
//static Node *removeNestCond(Node *n);
//static boolean containNestingAttr(Operator *oper);
static void adaptJoinCondAttrName(Node *n, char *oldName, char* newName);
static List *getAdditionalConds(List *l1, List *l2);
static TableAccessOperator *getTableAccess(QueryOperator *childNestRchild);
static boolean adaptCondsPos (Node *node, List *defs);
static void addSuffixInCond(List *l);

static boolean hasNestingAttr(Node *node);
static boolean hasNestingAttrVisitor(Node *q, boolean *found);
static boolean isExistSpecial(List *nestOps);
static boolean isExistAndNotExist(NestingOperator *nest);
//static boolean isNotContainNestingAttr (Node *node, char *status);
static boolean isContainOverlappedConds(NestingOperator *nest, NestingOperator* childNest);
static boolean isContainOperator(List *l, Operator *oper);
static boolean getAttrRefs (Node *node, List **l);
static boolean isContainAttr(List *l, AttributeReference *ar);
//static TableAccessOperator *getTableAccessByName(QueryOperator *op, char *name);
static void getTableAccessByName(QueryOperator *op, char *name, TableAccessOperator **table);
static List *getUnNestAttrsInExistsSqecial(List *conds);
static void propagateUpSchema(QueryOperator *base, QueryOperator *top);
static List* createDefsByInputs(QueryOperator *op);
static boolean addSuffixAdditionalCond(Node *node, char *state);
static Node *removeNestingConds(Node *node);
static void adaptSchemaByChild(QueryOperator *op1, QueryOperator *op2);
static Node *likeToNotLike(Node *node);
static void getEachConds(Node *expr, List **opList);
static List *removeNotOperator(List *l);
static SelectionOperator *findSelectionUp(QueryOperator *op);
static boolean findTableAccessByAttrName(QueryOperator *op, char *attrName, List **l);
//static boolean isTableContainsAttr(QueryOperator *op, char *attrName);
static boolean isContainAttrInDefs(char *name, List *defs);
static AttributeReference *getContainedAttr(List *attrs, List *defs);
static void adaptAttr(AttributeReference *a, List *defs);


static boolean isNotLikeOperator(Node *node);
static boolean isUnequalOperator(Node *node);
static boolean isNotInLike(NestingOperator *op);
static boolean isNotExists(NestingOperator *nest);

static QueryOperator *unnestScalar(QueryOperator *op);
static QueryOperator *unnestInClause(QueryOperator *op);
static QueryOperator *unnestExists(QueryOperator *op);
static QueryOperator *unnestExistsSpecial(List *l);
static QueryOperator *unnestNotInLike(NestingOperator *nestOp);
static QueryOperator *unnestNotExists(NestingOperator *nest);

//static List *getListNestingOperator (QueryOperator *op);
//static void appendNestingOperator (QueryOperator *op, List **result);
//static int checkAttr (char *name, QueryOperator *op);
//static void adatpUpNestingAttrDataType(QueryOperator *op, DataType nestingAttrDataType, int pos);
//static void getNestCondNode(Node *n, List **nestOpLists);

Node *
unnestTranslateQBModel(Node *qbModel)
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
	if(isExistSpecial(nestOps))
	{
		DEBUG_LOG("unnest exists special");
		unnestExistsSpecial(nestOps);
		return op;
	}

	FOREACH(NestingOperator, nest, nestOps)
	{
		QueryOperator *nestOp = (QueryOperator *) nest;

	    switch(nest->nestingType)
	    {
	    case NESTQ_SCALAR:
	    		DEBUG_LOG("unnest scalar subquery");
	    		unnestScalar(nestOp);
	    		break;
	    case NESTQ_ANY:
	    		DEBUG_LOG("unnest ANY subqery");
	    		unnestInClause(nestOp);
	    		break;
	    case NESTQ_ALL:
	    		DEBUG_LOG("unnest ALL subquery");
	    		if(isNotInLike(nest)) { //q16
	    			DEBUG_LOG("Special case: not in clause, subquery contains like clause and no correlation.");
	    			unnestNotInLike(nest);
				}
	    		break;
	    case NESTQ_EXISTS:
    			DEBUG_LOG("unnest EXISTS subquery:");
    			if(isNotExists(nest))
    			{
    				unnestNotExists(nest);
    				INFO_OP_LOG("test op : ", op);
    			}
    			else
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
unnestNotExists(NestingOperator *nest)
{
	QueryOperator *nestOp = (QueryOperator *) nest;
	QueryOperator *rchild = OP_RCHILD(nestOp);
	QueryOperator *lchild = OP_LCHILD(nestOp);
	List *parents = nestOp->parents;
	//char *nestAttrName = ((AttributeDef *) getTailOfListP(nestOp->schema->attrDefs))->attrName;

	//TODO: check is selection
	//TODO: check number of conditions in selection: after remove correlated cond, if no conds left, should remove selection, otherwise, keep selection.
	SelectionOperator *selTop = (SelectionOperator *) OP_FIRST_PARENT(nestOp);
	QueryOperator *selOp = OP_LCHILD(rchild);
	SelectionOperator *sel = (SelectionOperator *) selOp;
	List *selCondAttrs = NIL;
	getAttrRefs(sel->cond, &selCondAttrs);

	QueryOperator *right = OP_LCHILD(selOp);
	DEBUG_NODE_BEATIFY_LOG("right op: ", right);
	AttributeReference *rAttr  = copyObject(getContainedAttr(selCondAttrs, right->schema->attrDefs));
	adaptAttr(rAttr, right->schema->attrDefs);
	DEBUG_NODE_BEATIFY_LOG("right attr: ", rAttr);

	List *leftList = NIL;
	FOREACH(AttributeReference, a, selCondAttrs)
		findTableAccessByAttrName(lchild, a->name, &leftList);

	QueryOperator *left = (QueryOperator *) copyObject(getHeadOfListP(leftList));
	DEBUG_NODE_BEATIFY_LOG("left op: ", left);
	AttributeReference *lAttr  = copyObject(getContainedAttr(selCondAttrs, left->schema->attrDefs));
	adaptAttr(lAttr, left->schema->attrDefs);
	DEBUG_NODE_BEATIFY_LOG("left attr: ", lAttr);

	ProjectionOperator *rProj = createProjectionOp (singleton(rAttr), right, NIL, singleton(strdup(rAttr->name)));
	right->parents = singleton(rProj);
	QueryOperator *rProjOp = (QueryOperator *) rProj;

	char *newProjAttr = CONCAT_STRINGS(lAttr->name, "1");
	ProjectionOperator *lProj = createProjectionOp (singleton(lAttr), left, NIL, singleton(strdup(newProjAttr)));
	left->parents = singleton(lProj);
	QueryOperator *lProjOp = (QueryOperator *) lProj;

	DuplicateRemoval *rDup  = createDuplicateRemovalOp (NIL, rProjOp, NIL, singleton(strdup(rAttr->name)));
	rProjOp->parents = singleton(rDup);
	QueryOperator *rDupOp = (QueryOperator *) rDup;

	DuplicateRemoval *lDup  = createDuplicateRemovalOp (NIL, lProjOp, NIL, singleton(strdup(newProjAttr)));
	lProjOp->parents = singleton(lDup);
	QueryOperator *lDupOp = (QueryOperator *) lDup;

	//char *setAttr = CONCAT_STRINGS(lAttr->name, "1");
	SetOperator *set = createSetOperator (SETOP_DIFFERENCE, LIST_MAKE(lDupOp, rDupOp), NIL, singleton(newProjAttr));
	lDupOp->parents = singleton(set);
	rDupOp->parents = singleton(set);
	QueryOperator *setOp = (QueryOperator *) set;

	JoinOperator *join = createJoinOp (JOIN_CROSS, NULL, LIST_MAKE(lchild, set), parents, NIL);
	setOp->parents = singleton(join);
	lchild->parents = singleton(join);
	FOREACH(QueryOperator, p, parents)
		p->inputs = singleton(join);
	QueryOperator *joinOp = (QueryOperator *) join;

	List *selTopConds = NIL;
	getEachConds(selTop->cond, &selTopConds);
	List *newSelTopConds = removeNotOperator(selTopConds);

	Node *newCorCond = copyObject(sel->cond);
	List *attrList = NIL;
	getAttrRefs(newCorCond, &attrList);
	//TODO: now only two attrs
	FOREACH(AttributeReference, a, attrList)
	{
		if(!streq(a->name, lAttr->name))
			a->name = CONCAT_STRINGS(lAttr->name, "1");
	}
	adaptCondsPos(newCorCond, joinOp->schema->attrDefs);
	newSelTopConds = appendToTailOfList(newSelTopConds, newCorCond);
	selTop->cond = andExprList(newSelTopConds);

	QueryOperator *selTopOp = (QueryOperator *) selTop;
	selTopOp->schema->attrDefs = duplicateList(joinOp->schema->attrDefs);


	//DEBUG_NODE_BEATIFY_LOG("test op : ", set);

	return nestOp;
}

static void
adaptAttr(AttributeReference *a, List *defs)
{
	a->outerLevelsUp = 0;
	a->fromClauseItem = 0;
	resetPos(a, defs);
}

static AttributeReference *
getContainedAttr(List *attrs, List *defs)
{
	FOREACH(AttributeReference, a, attrs)
	{
		if(isContainAttrInDefs(a->name, defs))
			return a;
	}

	return NULL;
}

static boolean
isContainAttrInDefs(char *name, List *defs)
{
	FOREACH(AttributeDef, a, defs)
	{
		if(streq(name, a->attrName))
			return TRUE;
	}

	return FALSE;
}

//static boolean
//isTableContainsAttr(QueryOperator *op, char *attrName)
//{
//	FOREACH(AttributeDef, a, op->schema->attrDefs)
//	{
//		if(streq(a->attrName, attrName))
//			return TRUE;
//	}
//
//	return FALSE;
//}

static boolean
findTableAccessByAttrName(QueryOperator *op, char *attrName, List **l)
{
	if(!isA(op, TableAccessOperator))
	{
		FOREACH(QueryOperator, o, op->inputs)
			return findTableAccessByAttrName(o, attrName, l);
	}
	else
	{
		if(isContainAttrInDefs(attrName, op->schema->attrDefs))
		{
			*l = appendToTailOfList(*l, op);
			return TRUE;
		}
	}

	return FALSE;
}

static SelectionOperator *
findSelectionUp(QueryOperator *op)
{
	while(!isA(op, SelectionOperator))
		op = OP_FIRST_PARENT(op);

	return (SelectionOperator *) op;
}

static boolean
isNotExists(NestingOperator *nest)
{
	QueryOperator *nestOp = (QueryOperator *) nest;
	char *name = ((AttributeDef *) getTailOfListP(nestOp->schema->attrDefs))->attrName;

	SelectionOperator *sel = findSelectionUp(nestOp);
//	QueryOperator *selOp = OP_FIRST_PARENT(nestOp);
//	while(!isA(selOp, SelectionOperator))
//		selOp = OP_FIRST_PARENT(selOp);
//
//	SelectionOperator *sel = (SelectionOperator *) selOp;
	List *condsList = NIL;
	getEachConds(sel->cond, &condsList);
	FOREACH(Node, n, condsList)
	{
		if(isA(n, Operator))
		{
			Operator *o = (Operator *) n;
			if(streq(o->name, OPNAME_NOT))
			{
				List* attrsList = NIL;
				getAttrRefs(n, &attrsList);
				FOREACH(AttributeReference, a, attrsList)
				{
					if(streq(a->name, name))
						return TRUE;
				}
			}

		}
	}

	return FALSE;
}



static QueryOperator *
unnestNotInLike(NestingOperator *nest)
{
	QueryOperator *nestOp = (QueryOperator *) nest;
	List *parents = nestOp->parents;
	Node *nestCond = nest->cond;
	QueryOperator *rchild = OP_RCHILD(nestOp);
	QueryOperator *lchild = OP_LCHILD(nestOp);

	//TODO: check selection
	QueryOperator *selBotOp = OP_LCHILD(rchild);
	SelectionOperator *selBot = (SelectionOperator *) selBotOp;
	QueryOperator *selTopOp = OP_FIRST_PARENT(nestOp);
	SelectionOperator *selTop = (SelectionOperator *) selTopOp;

	selBot->cond = likeToNotLike(selBot->cond);
	List *dupAttrNames = getAttrNames(rchild->schema);
	DuplicateRemoval *dup = createDuplicateRemovalOp (NIL, rchild, NIL, dupAttrNames);
	rchild->parents = singleton(dup);
	QueryOperator *dupOp = (QueryOperator *) dup;

	JoinOperator *join = createJoinOp (JOIN_CROSS, NULL, LIST_MAKE(lchild, dup), parents, NIL);
	dupOp->parents = singleton(join);
	lchild->parents = singleton(join);
	QueryOperator *joinOp = (QueryOperator *) join;
	FOREACH(QueryOperator, p, parents)
		p->inputs = singleton(join);

	adaptCondsPos(nestCond, joinOp->schema->attrDefs);
	Operator *nestCondOper = (Operator *) nestCond;
	if(streq(nestCondOper->name, "<>"))
		nestCondOper->name = "=";

	Node *selTopCond = removeNestingConds(selTop->cond);
	selTopCond = AND_EXPRS(nestCond, selTopCond);
	selTop->cond = selTopCond;
	adaptSchemaByChild(selTopOp, joinOp);




	return nestOp;
}

static void
getEachConds(Node *expr, List **opList)
{
    // only are interested in operators here
	if (isA(expr,Operator)) {
	    Operator *op = (Operator *) copyObject(expr);

	    // uppercase operator name
	    char *opName = op->name;
	    while (*opName) {
	      *opName = toupper((unsigned char) *opName);
	      opName++;
	    }

	    if(streq(op->name,OPNAME_AND))
	    {
	        FOREACH(Node,arg,op->args)
				getEachConds(arg,opList);
	    }
	    else
	        *opList = appendToTailOfList(*opList, op);
	}
	 else
		 *opList = appendToTailOfList(*opList, expr);
}

static void
adaptSchemaByChild(QueryOperator *op1, QueryOperator *op2)
{
	op1->schema->attrDefs = duplicateList(op2->schema->attrDefs);
}

static List *
removeNotOperator(List *l)
{
	List *res = NIL;
	FOREACH(Node, n, l)
	{
		if(isA(n, Operator))
		{
			Operator *oper = (Operator *) n;
			if(!streq(oper->name, OPNAME_NOT))
				res = appendToTailOfList(res, n);
		}
		else
			res = appendToTailOfList(res, n);
	}

	return res;
}

static Node *
removeNestingConds(Node *node)
{
	List *condsList = NIL;
	List *notList = NIL;
	//getSelectionCondOperatorList(node, &condsList);
	getEachConds(node, &condsList);
	DEBUG_LOG("LIST LENGTH: %d", LIST_LENGTH(condsList));
	FOREACH(Node,n,condsList)
	{
		if(isA(n, Operator))
		{
			Operator *not = (Operator *) n;
		    // uppercase operator name
//		    char *opName = not->name;
//		    while (*opName) {
//		      *opName = toupper((unsigned char) *opName);
//		      opName++;
//		    }

			if(streq(not->name, OPNAME_NOT))
			{
				getEachConds((Node *) getHeadOfListP(not->args), &notList);
				DEBUG_LOG("NOT LIST LENGTH: %d", LIST_LENGTH(notList));
			}

		}
	}

	if(notList != NIL)
	{
		condsList = removeNotOperator(condsList);
		for(int i=0; i<LIST_LENGTH(notList); i++)
		{
			Node *cur = getNthOfListP(notList, i);
			if(i==0)
				condsList = appendToTailOfList(condsList, createOpExpr(OPNAME_NOT, singleton(cur)));
			else
				condsList = appendToHeadOfList(condsList, cur);
		}
	}

	DEBUG_LOG("NEW LIST LENGTH: %d", LIST_LENGTH(condsList));
	FOREACH(Node, n, condsList)
		DEBUG_NODE_BEATIFY_LOG("new cur node: ", n);

	List *unnestCondList = NIL;
	FOREACH(Node, n, condsList)
	{
		if(!hasNestingAttr(n))
			unnestCondList = appendToTailOfList(unnestCondList, n);
	}

	return andExprList(unnestCondList);
}

//TODO: list of conds?
static Node *
likeToNotLike(Node *node)
{
	Operator *oper = (Operator *) node;
	if(streq(oper->name, "LIKE"))
	{
		return (Node *) createOpExpr(OPNAME_NOT, singleton(node));
	}

	return node;
}


static boolean
isNotInLike(NestingOperator *nest)
{
	QueryOperator *nestOp = (QueryOperator *) nest;
	QueryOperator *child1 = OP_RCHILD(nestOp);
	QueryOperator *child2 = OP_LCHILD(child1);

	//TODO: might not selectionOperator
	if(isA(child2, SelectionOperator))
	{
		SelectionOperator *sel = (SelectionOperator *) child2;
		if(isUnequalOperator(nest->cond) && isNotLikeOperator(sel->cond))
			return TRUE;
	}

	return FALSE;
}

static boolean
isUnequalOperator(Node *node)
{
	if(isA(node, Operator))
	{
		Operator *oper = (Operator *) node;
		if(streq(oper->name, "<>"))
			return TRUE;
	}

	return FALSE;
}

static boolean
isNotLikeOperator(Node *node)
{
	List *conds = NIL;
	getSelectionCondOperatorList(node, &conds);

	FOREACH(Operator, oper, conds)
	{
		if(streq(oper->name,"LIKE"))
			return TRUE;

		//TODO: NOT LIKE
	}

	return FALSE;
}



static QueryOperator *
unnestExistsSpecial(List *l)
{
	DEBUG_LOG("unnestExistsSpecial");
	NestingOperator *nest = (NestingOperator *) getHeadOfListP(l);
	NestingOperator *childNest = (NestingOperator *) OP_LCHILD(nest);
	QueryOperator *nestOp = (QueryOperator *) nest;
	//QueryOperator *childNestOP = (QueryOperator *) childNestOP;

	List *parentNest = nestOp->parents;
	QueryOperator *lchild = OP_LCHILD(childNest);
	QueryOperator *nestRchild = OP_RCHILD(nest);
	QueryOperator *childNestRchild = OP_RCHILD(childNest);

	//TODO: check if selection
	SelectionOperator *selTopNest = (SelectionOperator *) OP_FIRST_PARENT(nestOp);
	SelectionOperator *selNotExist = (SelectionOperator *) OP_LCHILD(nestRchild);
	SelectionOperator *selExist = (SelectionOperator *) OP_LCHILD(childNestRchild);

	//remove nest = true cond
	List *selTopNestConds = NIL;
	getSelectionCondOperatorList(selTopNest->cond, &selTopNestConds);
	List *newSelTopNestConds = getUnNestAttrsInExistsSqecial(selTopNestConds);
	selTopNest->cond = andExprList(newSelTopNestConds);
	QueryOperator *selTopNestOp = (QueryOperator *) selTopNest;

	List *condsNotExist = NIL;
	List *condsExist = NIL;
	getSelectionCondOperatorList(selExist->cond, &condsExist);
	getSelectionCondOperatorList(selNotExist->cond, &condsNotExist);

	List *addConds = getAdditionalConds(condsNotExist, condsExist);

	TableAccessOperator *table= getTableAccess(childNestRchild);
	QueryOperator *tableOp = (QueryOperator *) table;

	//the attr in condsNotExist add suffix 1
	FOREACH(Node, n, addConds)
		addSuffixAdditionalCond(n,"x");

	List *joinAttrNames = NIL;
	FOREACH(char, c, getAttrNames(tableOp->schema))
		joinAttrNames = appendToTailOfList(joinAttrNames, strdup(c));
	FOREACH(char, c, getAttrNames(tableOp->schema))
		joinAttrNames = appendToTailOfList(joinAttrNames, CONCAT_STRINGS(strdup(c),"1"));

	TableAccessOperator *tablel = copyObject(table);
	TableAccessOperator *tabler = copyObject(table);
	QueryOperator *tableOpl = (QueryOperator *) tablel;
	QueryOperator *tableOpr = (QueryOperator *) tabler;
	tableOpl->schema->name = "L1";
	tableOpr->schema->name = "L2";

	JoinOperator *join = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(tableOpl,tableOpr), NIL, joinAttrNames);
	tableOpl->parents = singleton(join);
	tableOpr->parents = singleton(join);
	QueryOperator *joinOp = (QueryOperator *) join;

	addSuffixInCond(condsExist);

	FOREACH(Node, n, condsExist)
		adaptCondsPos(n, joinOp->schema->attrDefs);

	Node *selCond = andExprList(condsExist);

	SelectionOperator *sel = createSelectionOp(selCond, (QueryOperator *) join, NIL, NIL);
	joinOp->parents = singleton(sel);
	QueryOperator *selOp = (QueryOperator *) sel;

	FOREACH(Node, n, addConds)
		adaptCondsPos(n, selOp->schema->attrDefs);

	Node *caseCond = getHeadOfListP(addConds);

	List *projAttrsTemp = NIL;
	getAttrRefs((Node *) condsNotExist, &projAttrsTemp);
//	FOREACH(AttributeReference, a, projAttrsTemp)
//		DEBUG_LOG("projAttrsTemp: %s", a->name);

	List *projAttrs = NIL;
	FOREACH(AttributeReference, a, projAttrsTemp)
	{
		if(!isContainAttr(projAttrs, a))
		{
			AttributeReference *cura = copyObject(a);
			resetPos(cura, selOp->schema->attrDefs);
			projAttrs = appendToTailOfList(projAttrs, cura);
		}
	}

	List *projAttrNames = NIL;
	List *topProjAttrNames = NIL;
	FOREACH(AttributeReference, a, projAttrs)
	{
		projAttrNames = appendToTailOfList(projAttrNames, strdup(a->name));
		topProjAttrNames = appendToTailOfList(topProjAttrNames, strdup(a->name));
	}

	List *groupBy = NIL;
	List *topProjAttrs = NIL;
	FOREACH(AttributeReference, a, projAttrs)
	{
		groupBy = appendToTailOfList(groupBy, copyObject(a));
		topProjAttrs = appendToTailOfList(topProjAttrs, copyObject(a));
	}

	CaseWhen *when = createCaseWhen(caseCond, (Node *) createConstInt(1));
	CaseExpr *caseExpr = createCaseExpr(NULL, singleton(when), (Node *) createConstInt(0));
	projAttrs = appendToTailOfList(projAttrs, caseExpr);

	List *aggAttrNames = NIL;
	FOREACH(char, c, projAttrNames)
		aggAttrNames = appendToTailOfList(aggAttrNames, strdup(c));

	projAttrNames = appendToTailOfList(projAttrNames, "dateb");
	aggAttrNames = appendToHeadOfList(aggAttrNames, "dateb");

	ProjectionOperator *proj = createProjectionOp(projAttrs, selOp, NIL, projAttrNames);
	selOp->parents = singleton(proj);
	QueryOperator *projOp = (QueryOperator *) proj;

	FOREACH(AttributeReference, a, groupBy)
	{
		resetPos(a, projOp->schema->attrDefs);
	}

	AttributeReference *dateb = getAttrRefByName(projOp, "dateb");
	FunctionCall *f = createFunctionCall("sum", singleton(dateb));
	AggregationOperator *agg = createAggregationOp (singleton(f), groupBy, projOp, NIL, aggAttrNames);
	projOp->parents = singleton(agg);
	QueryOperator *aggOp = (QueryOperator *) agg;

	AttributeReference *datebAgg = getAttrRefByName(aggOp, "dateb");
	Operator *selTopCond = createOpExpr("=", LIST_MAKE(datebAgg, createConstInt(0)));
	SelectionOperator *selTop = createSelectionOp((Node *) selTopCond, aggOp, NIL, NIL);
	aggOp->parents = singleton(selTop);
	QueryOperator *selTopOp = (QueryOperator *) selTop;

	FOREACH(AttributeReference, a, topProjAttrs)
	{
		resetPos(a, selTopOp->schema->attrDefs);
	}

	FOREACH(AttributeReference,a,topProjAttrs)
		DEBUG_NODE_BEATIFY_LOG("test a: ",a);

	ProjectionOperator *projTop = createProjectionOp(topProjAttrs, selTopOp, NIL, topProjAttrNames);
	selTopOp->parents = singleton(projTop);
	QueryOperator *projTopOp = (QueryOperator *) projTop;

	DuplicateRemoval *dup = createDuplicateRemovalOp(NIL, projTopOp, NIL, NIL);
	projTopOp->parents = singleton(dup);
	QueryOperator *dupOp = (QueryOperator *) dup;

	TableAccessOperator *swapTable = NULL;
	getTableAccessByName(lchild, table->tableName, &swapTable);
	QueryOperator *swapTableOp = (QueryOperator *) swapTable;
//	QueryOperator *pSwapTable = (QueryOperator *) getHeadOfListP(swapTableOp->parents);
//	QueryOperator *lpSwapTable = OP_LCHILD(pSwapTable);
//	pSwapTable->inputs = LIST_MAKE(lpSwapTable, dup);
//	dupOp->parents = singleton(pSwapTable);


//	QueryOperator *swapTableOp = (QueryOperator *) swapTable;
	switchSubtrees(swapTableOp, dupOp);
	lchild->parents = parentNest;
	FOREACH(QueryOperator, o, parentNest)
		o->inputs = singleton(lchild);

	QueryOperator *baseJoinOp = OP_FIRST_PARENT(dupOp);
	propagateUpSchema(baseJoinOp, selTopNestOp);
	adaptCondNode(selTopNest->cond, OP_LCHILD(selTopNestOp)->schema->attrDefs);

//	dupOp->parents = swapTableOp->parents;
//	FOREACH(QueryOperator, o, dupOp->parents)
//		replaceNode(o->inputs, swapTable, dupOp);


//	JoinOperator *joinTop = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(lchild, dup), parentNest, NIL);
//	dupOp->parents = singleton(joinTop);
//	lchild->parents = singleton(joinTop);
//
//	FOREACH(QueryOperator, o, parentNest)
//		o->inputs = singleton(joinTop);
//
	INFO_OP_LOG("dupOp overview:", dupOp);

	return (QueryOperator *) dupOp;
}

static boolean
addSuffixAdditionalCond(Node *node, char *state)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    		AttributeReference *a = (AttributeReference *) node;
    		a->name = CONCAT_STRINGS(a->name, "1");
    }
    return visit(node, addSuffixAdditionalCond, state);
}

static void
addSuffixInCond(List *l)
{
	FOREACH(Operator,oper,l)
	{
		Node *n1 = getNthOfListP(oper->args,0);
		Node *n2 = getNthOfListP(oper->args,1);
		if(isA(n1, AttributeReference) && isA(n2, AttributeReference))
		{
			AttributeReference *a1 = (AttributeReference *) n1;
			AttributeReference *a2 = (AttributeReference *) n2;
			if(streq(a1->name,a2->name))
			{
				a1->name = CONCAT_STRINGS(a1->name, "1");
			}
		}
	}
}

static void
propagateUpSchema(QueryOperator *base, QueryOperator *top)
{
	while(!ptrEqual(base, top))
	{
		base->schema->attrDefs = createDefsByInputs(base);
		base = OP_FIRST_PARENT(base);
	}
	top->schema->attrDefs = createDefsByInputs(top);
}

static List*
createDefsByInputs(QueryOperator *op)
{
	List *defs = NIL;
	FOREACH(QueryOperator, o, op->inputs)
	{
		FOREACH(AttributeDef, ad, o->schema->attrDefs)
			defs = appendToTailOfList(defs, copyObject(ad));
	}

	return defs;
}

static void
getTableAccessByName(QueryOperator *op, char *name, TableAccessOperator **table)
{

	FOREACH(QueryOperator, o, op->inputs)
	{
		if(isA(o, TableAccessOperator))
		{
			TableAccessOperator *t = (TableAccessOperator *) o;
			DEBUG_LOG("t: %s",t->tableName);
			DEBUG_LOG("name: %s", name);
			if(streq(t->tableName, name))
			{
				*table = t;
			}
		}

		getTableAccessByName(o, name, table);
	}
}

static boolean
getAttrRefs (Node *node, List **l)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    	 	 *l = appendToTailOfList(*l, node);
    }

    return visit(node, getAttrRefs, l);
}


static boolean
isContainAttr(List *l, AttributeReference *ar)
{
	if(l == NIL)
		return FALSE;

	FOREACH(AttributeReference, a, l)
	{
		if(streq(ar->name, a->name))
			return TRUE;
	}

	return FALSE;
}

static boolean
adaptCondsPos (Node *node, List *defs)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    		AttributeReference *a = (AttributeReference *) node;
    		a->fromClauseItem = 0;
    		a->outerLevelsUp = 0;
    		resetPos(a, defs);
    }

    return visit(node, adaptCondsPos, defs);
}

static List *
getAdditionalConds(List *l1, List *l2)
{
	List *conds = NIL;
	FOREACH(Operator, oper, l1)
	{
		if(!isContainOperator(l2, oper))
			conds = appendToTailOfList(conds, copyObject(oper));
	}

    return conds;
}

static TableAccessOperator *
getTableAccess(QueryOperator *op)
{
	while(op->inputs != NIL)
		op = OP_LCHILD(op);

	return (TableAccessOperator *) op;
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
	List *parents = op->parents;
	//QueryOperator *parent = OP_FIRST_PARENT(op);
	//QueryOperator *prchild = OP_RCHILD(parent);

	//QueryOperator *rchild = getNthOfListP(inputs, 1);
	QueryOperator *rchild = OP_RCHILD(op);
	AttributeDef *agg0 = (AttributeDef *) getHeadOfListP(rchild->schema->attrDefs);

	//TODO: check is it a selection
	SelectionOperator *selTop = findSelectionUp(op);
	//SelectionOperator *selTop = (SelectionOperator *) getHeadOfListP(op->parents);
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
	{
		//adaptSchemaByName (selTopOp->schema->attrDefs, nestAttrName, agg0->attrName);
		upPropAdaptSchemaByName(op, nestAttrName, agg0->attrName);
	}
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

	JoinOperator *join = createJoinOp (JOIN_CROSS, NULL, inputs, parents, NIL);
	QueryOperator *joinOp = (QueryOperator *) join;


	FOREACH(QueryOperator, p, parents)
		replaceNode(p->inputs, op, join);

	//((QueryOperator *)selTop)->inputs = singleton(join);
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

	return joinOp;
}


static boolean
isExistSpecial(List *nestOps)
{

	NestingOperator *nest = (NestingOperator *) getHeadOfListP(nestOps);
	QueryOperator *childNestOp = OP_LCHILD(nest);

	if(!isA(childNestOp, NestingOperator))
		return FALSE;

	NestingOperator *childNest =  (NestingOperator *) childNestOp;
	//TODO: also need to check same table
	if(childNest->nestingType == NESTQ_EXISTS && isExistAndNotExist(nest) && isContainOverlappedConds(nest, childNest))
		return TRUE;

	return FALSE;
}

static boolean
isContainOverlappedConds(NestingOperator *nest, NestingOperator* childNest)
{
	QueryOperator *op1 = OP_RCHILD(nest);
	QueryOperator *op2 = OP_RCHILD(childNest);

	//TODO: check if selection
	SelectionOperator *selNotExist = (SelectionOperator *) OP_LCHILD(op1);
	SelectionOperator *selExist = (SelectionOperator *) OP_LCHILD(op2);

	List *condsNotExist = NIL;
	List *condsExist = NIL;
	getSelectionCondOperatorList(selExist->cond, &condsExist);
	getSelectionCondOperatorList(selNotExist->cond, &condsNotExist);

	List *resCondsExist = NIL;
	FOREACH(Operator, oper, condsNotExist)
	{
		if(isContainOperator(condsExist, oper))
			resCondsExist = appendToTailOfList(resCondsExist, oper);
	}

	if(LIST_LENGTH(resCondsExist) == LIST_LENGTH(condsExist))
		return TRUE;

	return FALSE;
}

static boolean
isContainOperator(List *l, Operator *oper)
{
	AttributeReference *operAttr1 = (AttributeReference *) getHeadOfListP(oper->args);
	AttributeReference *operAttr2 = (AttributeReference *) getTailOfListP(oper->args);

	FOREACH(Operator, o, l)
	{
		Node *oAttr1 = getHeadOfListP(o->args);
		Node *oAttr2 = getTailOfListP(o->args);

		if(isA(oAttr1, AttributeReference) && isA(oAttr2, AttributeReference))
		{
			AttributeReference *a1 = (AttributeReference *) oAttr1;
			AttributeReference *a2 = (AttributeReference *) oAttr2;

			if(((streq(operAttr1->name, a1->name) && streq(operAttr2->name, a2->name)) ||
					(streq(operAttr1->name, a2->name) && streq(operAttr2->name, a1->name))) &&
						streq(oper->name,o->name))
				return TRUE;
		}

	}

	return FALSE;
}

static boolean
isExistAndNotExist(NestingOperator *nest)
{
	QueryOperator *op = OP_FIRST_PARENT(nest);
	if(isA(op, SelectionOperator))
	{
		SelectionOperator *sel = (SelectionOperator *) OP_FIRST_PARENT(nest);
		List *condList = NIL;
		getSelectionCondOperatorList(sel->cond, &condList);
		int containExist = FALSE;
		int containNotExist = FALSE;
		FOREACH(Operator, oper, condList)
		{

			if(!streq(oper->name, "NOT") && hasNestingAttr((Node *) oper))
				containExist = TRUE;

			if(streq(oper->name, "NOT") && hasNestingAttr((Node *) oper))
				containNotExist = TRUE;
		}

		return containExist && containNotExist;
	}

	return FALSE;
}


//static boolean
//isNotContainNestingAttr (Node *node, char *status)
//{
//    if (node == NULL)
//        return FALSE;
//
//    if(isA(node, AttributeReference))
//    {
//    		AttributeReference *a = (AttributeReference *) node;
//    		if(isNestAttr(a))
//    		{
//    			DEBUG_LOG("attr: %s", a->name);
//    			return FALSE;
//    		}
//    }
//
//    return visit(node, isNotContainNestingAttr, status);
//}


static boolean
hasNestingAttr(Node *node)
{
    boolean found = FALSE;

    hasNestingAttrVisitor(node, &found);
    return found;
}

static boolean
hasNestingAttrVisitor(Node *q, boolean *found)
{
    if(q == NULL)
        return TRUE;

    if(isA(q, AttributeReference))
    {
    		AttributeReference *a = (AttributeReference *) q;
    		if(isNestAttr(a))
    		{
    			DEBUG_LOG("attr: %s", a->name);
    			*found = TRUE;
    			return FALSE;
    		}
    }

    return visit(q, hasNestingAttrVisitor, found);
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

			if(isPrefix(namel, getNestingAttrPrefix()))
			{
				return TRUE;
			}

			if(isPrefix(namer, getNestingAttrPrefix()))
			{
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



static boolean
adaptCondNode (Node *node, List *defs)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    		AttributeReference *a = (AttributeReference *) node;
    		resetPos(a, defs);
    }
    return visit(node, adaptCondNode, defs);
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

static List *
getUnNestAttrsInExistsSqecial(List *conds)
{
	List *l = NIL;
	FOREACH(Node, n, conds){
		if(!hasNestingAttr(n))
			l = appendToTailOfList(l, n);
	}
	return l;
}

static int
getInAttrPos(List *conds)
{
	int count = 0;
	FOREACH(Operator, oper, conds)
	{
	    Node *nl = getNthOfListP(oper->args, 0);
	    Node *nr = getNthOfListP(oper->args, 1);
	    if((isA(nl, AttributeReference) && isA(nr, Constant) &&
	    		((Constant *)nr)->constType == DT_BOOL && isNestAttr((AttributeReference *) nl))
	    		|| (isA(nr, AttributeReference) && isA(nl, Constant) &&
	    		   ((Constant *)nl)->constType == DT_BOOL && isNestAttr((AttributeReference *) nr)) )
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

static void upPropAdaptSchemaByName(QueryOperator *op, char *name, char *attr)
{
	while(!isA(op, SelectionOperator))
	{
		adaptSchemaByName(op->schema->attrDefs, name, attr);
		op = OP_FIRST_PARENT(op);
	}

	adaptSchemaByName(op->schema->attrDefs, name, attr);
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
		   if(isPrefix(name, getNestingAttrPrefix()))
	       {
			   ad->attrName = strdup(attr);
	       }
	}
}

boolean
isNestAttrName(char* c)
{
	return isPrefix(c, getNestingAttrPrefix());
}

boolean
isNestAttr(AttributeReference *a)
{
	return isNestAttrName(a->name);
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


boolean
isNestOp(Node* n)
{
	if(isA(n, Operator))
	{
		Operator *o = (Operator *) n;
		Node *arg = getHeadOfListP(o->args);
		if(isA(arg, AttributeReference))
		{
			AttributeReference *a = (AttributeReference *) arg;
			if(isNestAttr(a))
				return TRUE;

			return FALSE;
		}

		return FALSE;
	}

	return FALSE;
}
