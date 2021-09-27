/*-----------------------------------------------------------------------------
 *
 * prop_inference.c
 *
 *
 *      AUTHOR: xing_niu
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "configuration/option.h"
#include "exception/exception.h"
#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/coarse_grained/common_prop_inference.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"
#include "stdlib.h"
#include "symbolic_eval/z3_solver.h"
#include "instrumentation/timing_instrumentation.h"

#if HAVE_Z3

/* consts */
#define RIGHT_ATTR_PREFIX backendifyIdentifier("R")

static boolean printEXPRProVisitor(QueryOperator *op, void *context);
static boolean printPREDProVisitor(QueryOperator *op, void *context);
static Node *generateFunCompExpr(AttributeReference *a, char *op);


/*
 * bottom-up propagation of expression
 */
void
exprBottomUp (QueryOperator *root)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_EXPR_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_EXPR_DONE_BU))
		    	exprBottomUp(op);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			DEBUG_LOG("-- exprBottomUp TableAccessOperator --");
			Operator *expr = NULL;
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) expr);
			DEBUG_LOG("op(%s) - expr:%s\n ",root->schema->name, nodeToString(expr));
		}
		else if(isA(root,ProjectionOperator))
		{
			DEBUG_LOG("-- exprBottomUp ProjectionOperator --");
			Node *childExpr = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EXPR);
			Node *curExpr = copyObject(childExpr);

			ProjectionOperator *proj = (ProjectionOperator *) root;
			List *attrDefs = root->schema->attrDefs;
			int cnt = 0;
			FOREACH(Node, n, proj->projExprs)
			{
				if(isA(n, Operator))
				{
					AttributeDef *attrDef = (AttributeDef *) getNthOfListP(attrDefs, cnt);
					AttributeReference *attr = createFullAttrReference(attrDef->attrName, 0, cnt, 0, attrDef->dataType);
					Operator *newExpr = createOpExpr("=", LIST_MAKE(attr,copyObject(n)));
					curExpr = AND_EXPRS(curExpr,(Node *) newExpr);
				}
				cnt ++;
			}
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) curExpr);
			DEBUG_LOG("op(%s) - expr:%s\n ",root->schema->name, nodeToString(curExpr));
		}
		else if(isA(root, JoinOperator))
		{
			Node *lChildExpr = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EXPR);
			Node *rChildExpr = getStringProperty(OP_RCHILD(root), PROP_STORE_SET_EXPR);

			Node *curExpr = AND_EXPRS(copyObject(lChildExpr),copyObject(rChildExpr));

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) curExpr);
			DEBUG_LOG("op(%s) - expr:%s\n ",root->schema->name, nodeToString(curExpr));
		}
		else if(isA(root, SetOperator))
		{
			Node *lChildExpr = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EXPR);
			Node *rChildExpr = getStringProperty(OP_RCHILD(root), PROP_STORE_SET_EXPR);
			SetOperator *s = (SetOperator *) root;
			Node *curExpr = NULL;
			if(s->setOpType == SETOP_UNION)
			{
				curExpr = OR_EXPRS(copyObject(lChildExpr),copyObject(rChildExpr));
			}
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) curExpr);
			DEBUG_LOG("op(%s) - expr:%s\n ",root->schema->name, nodeToString(curExpr));
			//TODO: other set operators
		}
		else
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *childExpr = getStringProperty(childOp, PROP_STORE_SET_EXPR);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) copyObject(childExpr));
			DEBUG_LOG("op(%s) - expr:%s\n ",root->schema->name, nodeToString(childExpr));
		}
	}
}

static boolean
printEXPRProVisitor (QueryOperator *op, void *context)
{
	Node *expr = getStringProperty(op, PROP_STORE_SET_EXPR);
	DEBUG_LOG("op(%s) - expr:%s\n ",op->schema->name, nodeToString(expr));
    return TRUE;
}

void
printEXPRPro(QueryOperator *root)
{
    START_TIMER("PropertyInference - EXPR - print");
    visitQOGraph(root, TRAVERSAL_PRE, printEXPRProVisitor, NULL);
    STOP_TIMER("PropertyInference - EXPR - print");
}

/*
 * bottom-up propagation of predicates
 */
void
predBottomUp (QueryOperator *root)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_PRED_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_PRED_DONE_BU))
		    	predBottomUp(op);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			Node *pred = NULL;

//			List *minmax = getAllMinAndMax((TableAccessOperator *) root);
//			HashMap *min_map = getHeadOfListP(minmax);
//			HashMap *max_map = getTailOfListP(minmax);
//			DEBUG_LOG("min: %s, max: %s", nodeToString(min_map), nodeToString(max_map));
//
//		    List *attrDefs = root->schema->attrDefs;
//		    FOREACH(AttributeDef, def, attrDefs)
//		    {
//		    		AttributeReference *attr = createAttributeReference(def->attrName);
//		    		Constant *cmin = (Constant *) MAP_GET_STRING(min_map,def->attrName);
//		    		Constant *cmax = (Constant *) MAP_GET_STRING(max_map,def->attrName);
//
//	    			Operator *min_oper = createOpExpr(">=", LIST_MAKE(copyObject(attr), copyObject(cmin)));
//	    			Operator *max_oper = createOpExpr("<=", LIST_MAKE(copyObject(attr), copyObject(cmax)));
//	    			pred = appendToTailOfList(pred, min_oper);
//	    			pred = appendToTailOfList(pred, max_oper);
//		    }

//			List *minmax = getAllMinAndMax((TableAccessOperator *) root);
//			HashMap *min_map = getHeadOfListP(minmax);
//			HashMap *max_map = getTailOfListP(minmax);
//			DEBUG_LOG("min: %s, max: %s", nodeToString(min_map), nodeToString(max_map));
//
//			List *attrDefs = root->schema->attrDefs;
//			FOREACH(AttributeDef, def, attrDefs)
//			{
//				AttributeReference *attr = createAttributeReference(def->attrName);
//				Constant *cmin = (Constant *) MAP_GET_STRING(min_map,def->attrName);
//				Constant *cmax = (Constant *) MAP_GET_STRING(max_map,def->attrName);
//
//				Operator *min_oper = createOpExpr(">=", LIST_MAKE(copyObject(attr), copyObject(cmin)));
//				Operator *max_oper = createOpExpr("<=", LIST_MAKE(copyObject(attr), copyObject(cmax)));
//				pred = AND_EXPRS(pred, min_oper);
//				pred = AND_EXPRS(pred, max_oper);
//			}

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
		}
		else if(isA(root,SelectionOperator))
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *childPred = getStringProperty(childOp, PROP_STORE_SET_PRED);
			Node *pred = copyObject(childPred);

			SelectionOperator *sel = (SelectionOperator *) root;
			//DEBUG_LOG("op(%s) - sel->cond:%s\n ",root->schema->name, nodeToString(sel->cond));
			if(pred != NULL)
				pred = AND_EXPRS(pred, (Node *) copyObject(sel->cond));
			else
				pred = copyObject(sel->cond);

			DEBUG_LOG("op(%s) - pred:%s\n ",root->schema->name, nodeToString(pred));
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
		}
		else if(isA(root, JoinOperator))
		{
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *lchildPred = getStringProperty(lchildOp, PROP_STORE_SET_PRED);
			Node *rchildPred = getStringProperty(rchildOp, PROP_STORE_SET_PRED);
			Node *pred = pred = AND_EXPRS(copyObject(lchildPred), copyObject(rchildPred));

			JoinOperator *join = (JoinOperator *) root;
			if(join->joinType == JOIN_INNER)
			{
				pred = AND_EXPRS(pred, (Node *) copyObject(join->cond));
			}

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
		}
		else if(isA(root, SetOperator))
		{
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *lchildPred = getStringProperty(lchildOp, PROP_STORE_SET_PRED);
			Node *rchildPred = getStringProperty(rchildOp, PROP_STORE_SET_PRED);
			SetOperator *s = (SetOperator *) root;
			Node *pred = NULL;
			if(s->setOpType == SETOP_UNION)
			{
				pred = OR_EXPRS(copyObject(lchildPred),copyObject(rchildPred));
			}
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
			//TODO: other set operators
		}
		else
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *childPred = getStringProperty(childOp, PROP_STORE_SET_PRED);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) copyObject(childPred));
		}
	}
}

static boolean
printPREDProVisitor (QueryOperator *op, void *context)
{
    List *pred = (List*) getStringProperty(op, PROP_STORE_SET_PRED);
    DEBUG_LOG("op(%s) - size:%d - pred:%s\n ",op->schema->name, LIST_LENGTH(pred), nodeToString(pred));
    return TRUE;
}

void
printPREDPro(QueryOperator *root)
{
    START_TIMER("PropertyInference - PRED - print");
    visitQOGraph(root, TRAVERSAL_PRE, printPREDProVisitor, NULL);
    STOP_TIMER("PropertyInference - PRED - print");
}


char *
escapeUnderscore (char *str)
{
    int len = strlen(str);
    int newLen = len;
    char *result;

    for(char *s = str; *s != '\0'; s++, newLen = newLen + (*s == '_' ? 1 : 0));

    result = (char *) MALLOC(newLen + 1);

    for(int i = 0, j = 0; i <= len; i++, j++)
    {
        if (str[i] == '_')
        {
            result[j++] = '_';
            result[j] = '_';
        }
        else
            result[j] = str[i];
    }

    return result;
}


char *
getRightAttrName (char *attr)
{
    return CONCAT_STRINGS(RIGHT_ATTR_PREFIX, "_",
            escapeUnderscore(attr));
}

boolean
addPrimeOnAttrsInOperator(Node *node, char *state)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    		AttributeReference *attr = (AttributeReference *) node;
    		attr->name = getRightAttrName(attr->name);
    }
    return visit(node, addPrimeOnAttrsInOperator, state);
}

static Node *
generateFunCompExpr(AttributeReference *a, char *op)
{
	Node *res = NULL;
	AttributeReference *at = copyObject(a);
	AttributeReference *ap = copyObject(at);
	ap->name = getRightAttrName(at->name);

	res = (Node *) createOpExpr(op,LIST_MAKE(at,ap));
	return res;
}

boolean
isStartAsAGG(char *name)
{
	boolean f = FALSE;

	if(strlen(name) >= 5)
	{	//DEBUG_LOG("name[0]:%c,name[1]:%c,name[2]:%c,name[3]:%c,name[4]:%c",name[0],name[1],name[2],name[3],name[4]);
		if(name[0] == 'A' && name[1] == 'G' && name[2] == 'G' && name[3] == 'R' && name[4] == '_')
		{
			f = TRUE;
		}
	}
	return f;
}

Node *
generateAttrAndPrimeEq(List *l)
{
	Node *res = NULL;

    FOREACH(AttributeReference, attr, l)
    {
        Node *oper = generateFunCompExpr(attr, "=");

        if(res != NULL)
        	res = AND_EXPRS(res, oper);
        else
        	res = oper;
    }
	return res;
}

List *
generateAttrDefAndPrimeEq(List *l)
{
	List *res = NIL;

	int cnt = 0;
    FOREACH(AttributeDef, attr, l)
    {
    	AttributeReference *a = createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType);
        Node *oper = generateFunCompExpr(a, "=");

      	res = appendToTailOfList(res, oper);
        cnt++;
    }
	return res;
}

Node *
getConds(QueryOperator *op)
{
	//QueryOperator *childOp = OP_LCHILD(op);
	//boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
	//Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));
	Node *childExpr = copyObject(getStringProperty(op, PROP_STORE_SET_EXPR));
	Node *childPred = copyObject(getStringProperty(op, PROP_STORE_SET_PRED));

//	Node *childExprPrime = copyObject(childExpr);
//	Node *childPredPrime = copyObject(childPred);
//	addPrimeOnAttrsInOperator(childExprPrime,"dummy");
//	addPrimeOnAttrsInOperator(childPredPrime,"dummy");

	Node *conds = NULL;
	//Node *condPrime = NULL;
	//Node *notCondPrime = NULL;
	//Node *finalCond = NULL;

	if(childExpr != NULL && childPred != NULL)
	{
		conds = AND_EXPRS(childExpr,childPred);
		//ondPrime = AND_EXPRS(childExprPrime,childPredPrime);
	}
	else if(childExpr != NULL)
	{
		conds = childExpr;
		//condPrime = childExprPrime;
	}
	else if(childPred != NULL)
	{
		conds = childPred;
		//condPrime = childPredPrime;
	}

	return conds;
}


/* used for aggregation in checking gc and ge */
Node *
ListAttrRefsToNameSetForAgg(QueryOperator *op, List *l)
{
	Set *s = STRSET();
	QueryOperator *childOp = OP_LCHILD(op);

	//TODO: might use a map to store a->AGG_GB_ARG1 when do the ps rewrite of aggregation
	ProjectionOperator *proj = (ProjectionOperator *) childOp;

	List *attrRefs = proj->projExprs;
	FOREACH(AttributeReference, attr, l)
	{
		AttributeReference *a = getNthOfListP(attrRefs, attr->attrPosition);
		addToSet(s,a->name);
	}
	return (Node *) s;
}

/* used for aggregation in checking gc and ge */
Node *
ListAttrRefsToEqCondsForAgg(QueryOperator *op, List *l)
{
	Node *res = NULL;
	QueryOperator *childOp = OP_LCHILD(op);

	//TODO: might use a map to store a->AGG_GB_ARG1 when do the ps rewrite of aggregation
	ProjectionOperator *proj = (ProjectionOperator *) childOp;

	List *attrRefs = proj->projExprs;
	FOREACH(AttributeReference, attr, l)
	{
		AttributeReference *a = copyObject(getNthOfListP(attrRefs, attr->attrPosition));
		AttributeReference *ap = copyObject(a);
		ap->name = getRightAttrName(a->name);

        Operator *oper = createOpExpr("=",LIST_MAKE(a,ap));
        if(res != NULL)
        	res = AND_EXPRS(res,(Node *) oper);
        else
        	res = (Node *) oper;
	}

	return res;
}

List *
generateAttrDefAndPrimeNonEq(List *l)
{
	List *res = NIL;

	int cnt = 0;
    FOREACH(AttributeDef, attr, l)
    {
    	AttributeReference *a = createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType);
        Node *oper = generateFunCompExpr(a, "=");
        Node *nonOper = (Node *) createOpExpr("NOT", singleton(oper));

      	res = appendToTailOfList(res, nonOper);
        cnt++;
    }
	return res;
}

/*
 * Used for aggregation in checking gc and ge
 * Used for check something like ∀𝑔 ∈ 𝐺 : Ψ𝑄1,𝑋1 ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) → 𝑔 = 𝑔′
 * Used in aggregation, duplicate removal and join
 */
//boolean
//checkEqCompForListAttrRefsOfOp(QueryOperator *root, List *l, char *isGcOrGe)
//{
//	QueryOperator *childOp = OP_LCHILD(root);
//	//boolean childGe = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GE);
//	Node *childComp = copyObject(getStringProperty(childOp, isGcOrGe));
//
//	Node *conds = getConds(childOp);
//	Node *condsPrime = NULL;
//	if(conds != NULL)
//	{
//		condsPrime = copyObject(conds);
//		addPrimeOnAttrsInOperator(condsPrime,"dummy");
//	}
//
//	Node *gbEqs = ListAttrRefsToEqCondsForAgg(root, l);
//	Node *notGbEqs = NULL;
//	if(gbEqs != NULL)
//		notGbEqs = (Node *) createOpExpr("NOT", singleton(gbEqs));
//
//	boolean gce = FALSE;
//	Node *finalCond = NULL;
//	if(notGbEqs != NULL)
//	{
//		if(conds != NULL)
//			finalCond = andExprList(LIST_MAKE(childComp, conds, condsPrime, notGbEqs));
//		else
//			finalCond = andExprList(LIST_MAKE(childComp, notGbEqs));
//
//		gce = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
//	}
//	else //no group by is true since full provenance
//	{
//		gce = TRUE;
//	}
//
//	return gce;
//}

//static char *
//escapeUnderscore (char *str)
//{
//    int len = strlen(str);
//    int newLen = len;
//    char *result;
//
//    for(char *s = str; *s != '\0'; s++, newLen = newLen + (*s == '_' ? 1 : 0));
//
//    result = (char *) MALLOC(newLen + 1);
//
//    for(int i = 0, j = 0; i <= len; i++, j++)
//    {
//        if (str[i] == '_')
//        {
//            result[j++] = '_';
//            result[j] = '_';
//        }
//        else
//            result[j] = str[i];
//    }
//
//    return result;
//}

//static char *
//getRightAttrName (char *attr)
//{
//    return CONCAT_STRINGS(RIGHT_ATTR_PREFIX, "_",
//            escapeUnderscore(attr));
//}
//
//static boolean
//addPrimeOnAttrsInOperator(Node *node, char *state)
//{
//    if (node == NULL)
//        return FALSE;
//
//    if(isA(node, AttributeReference))
//    {
//    		AttributeReference *attr = (AttributeReference *) node;
//    		attr->name = getRightAttrName(attr->name);
//    }
//    return visit(node, addPrimeOnAttrsInOperator, state);
//}


/*
 * bottom-up propagation of gc
 */
//static void
//gcBottomUp (QueryOperator *root)
//{
//    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_GC_DONE_BU);
//
//    if(root->inputs != NULL)
//	{
//		FOREACH(QueryOperator, op, root->inputs)
//		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_GC_DONE_BU))
//		    	gcBottomUp(op);
//	}
//
//	if(root != NULL)
//	{
//		if(isA(root, TableAccessOperator))
//		{
//			//List *cmap = NIL;
//			Node *comp = NULL;
//			int cnt = 0;
//		    FOREACH(AttributeDef, attr, root->schema->attrDefs)
//		    {
//		        AttributeReference *lattr = createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType);
//		        AttributeReference *rattr = createFullAttrReference(getRightAttrName(attr->attrName), 0, cnt, 0, attr->dataType);
//		        Operator *oper = createOpExpr("=",LIST_MAKE(lattr,rattr));
//		        comp = andExprs(comp,(Node *) oper);
//		        cnt++;
//		    }
//
//		    SET_BOOL_STRING_PROP((QueryOperator *)root,PROP_STORE_SET_GC);
//		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, comp);
//		}
//		if(isA(root, SelectionOperator))
//		{
//			QueryOperator *childOp = OP_LCHILD(root);
//			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
//			Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));
//			Node *childExpr = copyObject(getStringProperty(childOp, PROP_STORE_SET_EXPR));
//			Node *childPred = copyObject(getStringProperty(childOp, PROP_STORE_SET_PRED));
//
//			Node *childExprPrime = copyObject(childExpr);
//			Node *childPredPrime = copyObject(childPred);
//			addPrimeOnAttrsInOperator(childExprPrime,"dummy");
//			addPrimeOnAttrsInOperator(childPredPrime,"dummy");
//
//			Node *cond = andExprs(childExpr,childPred);
//			Node *condPrime = andExprs(childExprPrime,childPredPrime);
//			Node *notCondPrime = (Node *) createOpExpr("NOT", singleton(condPrime));
//
//			Node *finalCond = andExprs(childComp, cond, notCondPrime);
//			DEBUG_LOG(" -- gc bottom-up for selection -- ");
//			DEBUG_LOG(" -- child gc: %s",childGc);
//			DEBUG_LOG(" -- allCond: %s",nodeToString(finalCond));
//
//			Z3_context ctx = mk_context();
//			Z3_ast cc = exprtoz3((Node *) finalCond, ctx);
//			DEBUG_LOG("-- z3 allCond: %s", Z3_ast_to_string(ctx, cc));
//		    Z3_solver s;
//		    s = mk_solver(ctx);
//		    Z3_solver_assert(ctx, s, cc);
//
//		    boolean gc = FALSE;
//		    if(Z3_solver_check(ctx, s) == Z3_L_FALSE)
//		    {
//		    	gc = TRUE;
//		    }
//		    DEBUG_LOG(" -- current gc: %s",gc);
//
//			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(gc && childGc));
//		    //SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, comp);
//
//		}
//		else if(isA(root,AggregationOperator))
//		{
//
//		}
//		else if(isA(root, JoinOperator))
//		{
//			QueryOperator *lchildOp = OP_LCHILD(root);
//			QueryOperator *rchildOp = OP_RCHILD(root);
//			Node *nlChild = getStringProperty(lchildOp, PROP_STORE_SET_GC_COMP);
//			Node *nrChild = getStringProperty(rchildOp, PROP_STORE_SET_GC_COMP);
//			List *cmap = concatTwoLists((List *) copyObject(nlChild), (List *) copyObject(nrChild));
//			setStringProperty((QueryOperator *)root, PROP_STORE_SET_GC_COMP, (Node *) cmap);
//		}
//		else
//		{
//			QueryOperator *childOp = OP_LCHILD(root);
//			Node *childComp = getStringProperty(childOp, PROP_STORE_SET_GC_COMP);
//			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
//
//			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(childGc));
//		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, childComp);
//		}
//	}
//}


//void
//bottomUpInference(QueryOperator *root)
//{
//	DEBUG_LOG("current algebra tree: %s", operatorToOverviewString((Node *) root));
//	DEBUG_NODE_BEATIFY_LOG("tree:", root);
//	//DEBUG_LOG("current algebra tree: %s", nodeToString((Node *) root));
//
//	exprBottomUp(root);
//	printEXPRPro(root);
//	predBottomUp(root);
//	printPREDPro(root);
//	//geBottomUp(root, lmap, rmap);
//	//printGcPro(root);
//	//gcBottomUp(root);
////	boolean reuse = checkIfReuse(root, lmap, rmap);
////	DEBUG_LOG("If reuse? %d", reuse);
//}


#else // dummy implementations to keep compiler quiet

void
exprBottomUp(QueryOperator *root)
{
	THROW(SEVERITY_PANIC, "%s", "cannot call inference when not compiled with Z3 support");
}

void
predBottomUp(QueryOperator *root)
{
	THROW(SEVERITY_PANIC, "%s", "cannot call inference when not compiled with Z3 support");
}

void
printEXPRPro(QueryOperator *root)
{
	THROW(SEVERITY_PANIC, "%s", "cannot call inference when not compiled with Z3 support");
}

void
printPREDPro(QueryOperator *root)
{
	THROW(SEVERITY_PANIC, "%s", "cannot call inference when not compiled with Z3 support");
}

#endif
