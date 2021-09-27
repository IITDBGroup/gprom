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
#include "provenance_rewriter/coarse_grained/gc_prop_inference.h"
#include "provenance_rewriter/coarse_grained/ge_prop_inference.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"
#include "stdlib.h"
#include "symbolic_eval/z3_solver.h"
#include "instrumentation/timing_instrumentation.h"

#if HAVE_Z3

/* consts */
#define RIGHT_ATTR_PREFIX backendifyIdentifier("R")


static Node *condAndCondPrimes(QueryOperator *op);
static boolean checkCountAndSumPositive(FunctionCall *fc, QueryOperator *op);
static Node *generateFunCompExpr(AttributeReference *a, char *op);
static boolean checkSumNegative(FunctionCall *fc, QueryOperator *op);
static boolean checkEqCompForListAttrRefsOfOp(QueryOperator *root, List *l);
static boolean generateNeqPrimeFromNode(Node *node, List **state);



/*
 * bottom-up propagation of gc
 */
void
gcBottomUp (QueryOperator *root, List *attrNames)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_GC_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_GC_DONE_BU))
		    	gcBottomUp(op, attrNames);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			DEBUG_LOG("gcBottomUp TableAccessOperator");
			Node *comp = NULL;
			int cnt = 0;
		    FOREACH(AttributeDef, attr, root->schema->attrDefs)
		    {
		        AttributeReference *lattr = createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType);
		        AttributeReference *rattr = createFullAttrReference(getRightAttrName(attr->attrName), 0, cnt, 0, attr->dataType);
		        Operator *oper = createOpExpr("=",LIST_MAKE(lattr,rattr));
		        if(comp != NULL)
		        	comp = AND_EXPRS(comp,(Node *) oper);
		        else
		        	comp = (Node *) oper;
		        cnt++;
		    }

		    SET_BOOL_STRING_PROP((QueryOperator *)root,PROP_STORE_SET_GC);
		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, comp);
		    DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, TRUE, nodeToString(comp));
		}
		else if(isA(root, SelectionOperator))
		{
			DEBUG_LOG("gcBottomUp SelectionOperator");
			SelectionOperator *sel = (SelectionOperator *) root;
			QueryOperator *childOp = OP_LCHILD(root);
			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
			Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));

			Node *conds = getConds(childOp);
			Node *condsPrime = NULL;
			if(conds != NULL)
			{
				condsPrime = copyObject(conds);
				addPrimeOnAttrsInOperator(condsPrime,"dummy");
			}

			Node *selCond = copyObject(sel->cond);
			Node *selCondPrime = copyObject(sel->cond);
			Node *notSelCondPrime = NULL;
			addPrimeOnAttrsInOperator(selCondPrime,"dummy");

			if(selCond != NULL)
				notSelCondPrime = (Node *) createOpExpr("NOT", singleton(selCondPrime));

			DEBUG_LOG(" -- cond: %s",nodeToString(conds));
			DEBUG_LOG(" -- condPrime: %s",nodeToString(condsPrime));
			DEBUG_LOG(" -- childComp: %s",nodeToString(childComp));
			DEBUG_LOG(" -- selCond: %s",nodeToString(selCond));
			DEBUG_LOG(" -- selCondPrime: %s",nodeToString(selCondPrime));
			DEBUG_LOG(" -- notSelCondPrime: %s",nodeToString(notSelCondPrime));

			//use andExprList not AND_EXPRS
			//AND_EXPRS only use one "AND" and args contains more element
			//andExprList each "AND" args only contains two element
			Node *finalCond = NULL;
			if(conds != NULL)
				finalCond = andExprList(LIST_MAKE(childComp, conds, condsPrime, selCond, selCondPrime));
			else
				finalCond = andExprList(LIST_MAKE(childComp, selCond, notSelCondPrime));
			DEBUG_LOG(" -- gc bottom-up for selection -- ");
			DEBUG_LOG(" -- child gc: %d",childGc);
			DEBUG_LOG(" -- finalCond: %s",nodeToString(finalCond));

			//unSatisfiable is valid
			boolean gc = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
			//test z3ExprIsValid looks uncorrect
			boolean gcValid = z3ExprIsValid((Node *) finalCond, TRUE);

			/*
			 * select a from R where b < 5 where R has a,b
			 * (and (= a r_a) (= b r_b) (< b 5)) -> (not (< r_b 5))
			 * (and (= a r_a) (= b r_b) (< b 5) (not (< r_b 5))) if unSatisfiable, then it is valid
			 */

		    DEBUG_LOG(" -- current gc: %d",gc);
		    DEBUG_LOG(" -- current gcValid: %d",gcValid);

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(gc && childGc));
		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, childComp);
			DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, gc && childGc, nodeToString(childComp));
		}
		else if(isA(root,AggregationOperator))
		{
			DEBUG_LOG("gcBottomUp AggregationOperator");
			AggregationOperator *agg = (AggregationOperator *) root;
			QueryOperator *childOp = OP_LCHILD(root);
			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
			Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));

			boolean gc = checkEqCompForListAttrRefsOfOp(root, agg->groupBy);
//			QueryOperator *childOp = OP_LCHILD(root);
//			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
//			Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));
//
//			Node *conds = getConds(childOp);
//			Node *condsPrime = NULL;
//			if(conds != NULL)
//			{
//				condsPrime = copyObject(conds);
//				addPrimeOnAttrsInOperator(condsPrime,"dummy");
//			}
//
//			Node *gbEqs = ListAttrRefsToEqCondsForAgg(root, agg->groupBy);
//			Node *notGbEqs = NULL;
//			if(gbEqs != NULL)
//				notGbEqs = (Node *) createOpExpr("NOT", singleton(gbEqs));
//
//			boolean gc = FALSE;
//			Node *finalCond = NULL;
//			if(notGbEqs != NULL)
//			{
//				if(conds != NULL)
//					finalCond = andExprList(LIST_MAKE(childComp, conds, condsPrime, notGbEqs));
//				else
//					finalCond = andExprList(LIST_MAKE(childComp, notGbEqs));
//
//				gc = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
//			}
//			else //no group by is true since full provenance
//			{
//				gc = TRUE;
//			}

		    DEBUG_LOG(" -- current gc: %d",gc);


		    /* compute comp for function attrs*/
		    DEBUG_LOG("-- compute comp: ");
		    Node *funComp = NULL;

		    Set *s = (Set *) ListAttrRefsToNameSetForAgg(root, agg->groupBy);
		    FOREACH_SET(char,c, s)
		    {
		    	DEBUG_LOG("-- set has: %s",c);
		    }
		    //∀𝑥 ∈ 𝑋:∃𝑔 ∈ 𝐺 : 𝑐𝑜𝑛𝑑𝑠(𝑄1) → 𝑥 = g TODO: now just directly compare x = g
		    boolean flag1 = TRUE;
		    /* first case: f = f'*/
		    FOREACH(char, c, attrNames)
		    {
		    	DEBUG_LOG("-- check each attr: %s",c);
		    	if(!hasSetElem(s,c))
		    	{
		    		flag1 = FALSE;
		    		break;
		    	}
		    }

		    /* get function names from its parent operator and store in funAttrs */
		    DEBUG_LOG("-- flag1 is %d",flag1);
	    	QueryOperator *p = OP_FIRST_PARENT(root);
	    	ProjectionOperator *pproj = (ProjectionOperator *) p;
	    	List *funAttrs = NIL;
	    	int cnt = 0;
	    	FOREACH(AttributeReference,arf,pproj->projExprs)
	    	{	DEBUG_LOG("-- pproj->projExprs:  %s", arf->name);
	    		if(isStartAsAGG(arf->name))
	    		{
	    			DEBUG_LOG("-- isStartAsAGG:  %s", arf->name);
	    			AttributeDef *ad = getAttrDef(p,cnt);
	    			AttributeReference *attr = createFullAttrReference(ad->attrName, 0, cnt, 0, ad->dataType);
	    			funAttrs = appendToTailOfList(funAttrs, attr);
	    		}
				cnt++;
	    	}

	    	List *funCompList = NIL;
	    	/* first case: f = f'*/
		    if(flag1 == TRUE)
		    	funComp = generateAttrAndPrimeEq(funAttrs);
		    /* second case: f <= f' and third case: f >= f'*/
		    else
		    {
		    	int pos = 0;
	    		FOREACH(FunctionCall, fc, agg->aggrs)
				{
	    			AttributeReference *curAttr = (AttributeReference *) getNthOfListP(funAttrs, pos);
	    			if(checkCountAndSumPositive(fc, (QueryOperator *) agg))
	    			{
	    				Node *leq = generateFunCompExpr(curAttr, "<=");
	    				funCompList = appendToTailOfList(funCompList, leq);
	    			}
	    			else if(checkSumNegative(fc, (QueryOperator *) agg))
	    			{
	    				Node *leq = generateFunCompExpr(curAttr, ">=");
	    				funCompList = appendToTailOfList(funCompList, leq);
	    			}
	    			pos++;
				}
	    		if(funCompList != NIL)
	    			funComp = andExprList(funCompList);
		    }

		    DEBUG_LOG("-- funComp: %s", nodeToString(funComp));
		    Node *curComp = NULL;
		    if(funComp != NULL)
		    	curComp = AND_EXPRS(childComp, funComp);
		    else
		    	curComp = childComp;

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(gc && childGc));
		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, curComp);
			DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, gc && childGc, nodeToString(curComp));
		}
		else if(isA(root, DuplicateRemoval))
		{
			DuplicateRemoval *rm = (DuplicateRemoval *) root;
			QueryOperator *childOp = OP_LCHILD(root);
			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
			Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));

			boolean gc = checkEqCompForListAttrRefsOfOp(root, rm->attrs);

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(gc && childGc));
		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, childComp);
			DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, gc && childGc, nodeToString(childComp));
		}
		else if(isA(root, JoinOperator))
		{

			/* rules
			 * 𝑔𝑐(𝑄1, 𝑋1) ∧ 𝑔𝑐(𝑄2, 𝑋2)∧
			 * Ψ𝑄1,𝑋1 ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄′1) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) → 𝑎 = 𝑎′∧
			 * Ψ𝑄2,𝑋2 ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄′2) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄2) → 𝑏 = 𝑏′
			 */

			JoinOperator *join = (JoinOperator *) root;
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *lchildComp = copyObject(getStringProperty(lchildOp, PROP_STORE_SET_GC_COMP));
			Node *rchildComp = copyObject(getStringProperty(rchildOp, PROP_STORE_SET_GC_COMP));
			boolean lchildGc = GET_BOOL_STRING_PROP(lchildOp, PROP_STORE_SET_GC);
			boolean rchildGc = GET_BOOL_STRING_PROP(rchildOp, PROP_STORE_SET_GC);

			/* 𝑐𝑜𝑛𝑑𝑠(𝑄′1) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) */
			Node *lallConds = condAndCondPrimes(lchildOp);
			/* 𝑐𝑜𝑛𝑑𝑠(𝑄′2) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄2) */
			Node *rallConds = condAndCondPrimes(rchildOp);

			Node *comp = comp = AND_EXPRS(lchildComp,rchildComp);
			boolean gc = lchildGc && rchildGc;

//			if(join->joinType == JOIN_CROSS)
//			{
//				comp = AND_EXPRS(lchildComp,rchildComp);
//			}
			if(join->joinType == JOIN_INNER)
			{
				List *neqList = NIL;
				generateNeqPrimeFromNode(join->cond,&neqList);
				Node *neq = andExprList(neqList);
				Node *finalConds = andExprList(LIST_MAKE(comp,lallConds,rallConds,neq));
				gc = gc && !z3ExprIsSatisfiable((Node *) finalConds, TRUE);
			}
			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(gc));
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_GC_COMP, (Node *) comp);
			DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, gc && gc, nodeToString(comp));
		}
		else if(isA(root, SetOperator))
		{
			SetOperator *sop = (SetOperator *) root;
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *lchildComp = copyObject(getStringProperty(lchildOp, PROP_STORE_SET_GC_COMP));
			Node *rchildComp = copyObject(getStringProperty(rchildOp, PROP_STORE_SET_GC_COMP));
			boolean lchildGc = GET_BOOL_STRING_PROP(lchildOp, PROP_STORE_SET_GC);
			boolean rchildGc = GET_BOOL_STRING_PROP(rchildOp, PROP_STORE_SET_GC);

			//Node *comp = comp = AND_EXPRS(lchildComp,rchildComp);
			List *compList = NIL;
			Node *comp = NULL;
			boolean gc = lchildGc && rchildGc;

			if(sop->setOpType == SETOP_UNION)
			{
				List *lneqList = generateAttrDefAndPrimeNonEq(lchildOp->schema->attrDefs);
				List *leqList = generateAttrDefAndPrimeEq(lchildOp->schema->attrDefs);
				FORBOTH(Node, n, d, lneqList, leqList)
				{
					if(!z3ExprIsSatisfiable((Node *) AND_EXPRS(copyObject(lchildComp), n), TRUE))
					{
						compList = appendToTailOfList(compList, d);
					}
				}

				List *rneqList = generateAttrDefAndPrimeNonEq(rchildOp->schema->attrDefs);
				List *reqList = generateAttrDefAndPrimeEq(rchildOp->schema->attrDefs);
				FORBOTH(Node, n, d, rneqList, reqList)
				{
					if(!z3ExprIsSatisfiable((Node *) AND_EXPRS(copyObject(rchildComp), n), TRUE))
					{
						compList = appendToTailOfList(compList, d);
					}
				}
				if(compList != NIL)
					comp = andExprList(compList);
			}

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(gc));
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_GC_COMP, (Node *) comp);
			DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, gc && gc, nodeToString(comp));
		}
		else
		{
			DEBUG_LOG("gcBottomUp other operators");
			QueryOperator *childOp = OP_LCHILD(root);

			Node *childComp = NULL;
			if(HAS_STRING_PROP(childOp, PROP_STORE_SET_GC_COMP))
			{
				childComp = GET_STRING_PROP(childOp, PROP_STORE_SET_GC_COMP);
			}
			boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GC, (Node *) createConstBool(childGc));
		    SET_STRING_PROP(root, PROP_STORE_SET_GC_COMP, childComp);
		    DEBUG_LOG("op(%s) gc(%d) - expr:%s\n ",root->schema->name, childGc, nodeToString(childComp));
		}
	}
}

static boolean
generateNeqPrimeFromNode(Node *node, List **state)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    	AttributeReference *a = (AttributeReference *) node;
    	Node *eq =  generateFunCompExpr(a, "=");
    	Node *neq = (Node *) createOpExpr("NOT", singleton(eq));
    	*state = appendToTailOfList(*state, neq);
    }
    return visit(node, generateNeqPrimeFromNode, state);
}

static Node *
condAndCondPrimes(QueryOperator *op)
{
	Node *res = NULL;
	Node *conds = getConds(op);
	Node *condsPrime = NULL;
	if(conds != NULL)
	{
		condsPrime = copyObject(conds);
		addPrimeOnAttrsInOperator(condsPrime,"dummy");
	}
	return res;
}

/*
 * Used for check something like ∀𝑔 ∈ 𝐺 : Ψ𝑄1,𝑋1 ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) → 𝑔 = 𝑔′
 * Used in aggregation, duplicate removal and join
 */
static boolean
checkEqCompForListAttrRefsOfOp(QueryOperator *root, List *l)
{
	QueryOperator *childOp = OP_LCHILD(root);
	//boolean childGc = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GC);
	Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GC_COMP));

	Node *conds = getConds(childOp);
	Node *condsPrime = NULL;
	if(conds != NULL)
	{
		condsPrime = copyObject(conds);
		addPrimeOnAttrsInOperator(condsPrime,"dummy");
	}

	Node *gbEqs = ListAttrRefsToEqCondsForAgg(root, l);
	Node *notGbEqs = NULL;
	if(gbEqs != NULL)
		notGbEqs = (Node *) createOpExpr("NOT", singleton(gbEqs));

	boolean gc = FALSE;
	Node *finalCond = NULL;
	if(notGbEqs != NULL)
	{
		if(conds != NULL)
			finalCond = andExprList(LIST_MAKE(childComp, conds, condsPrime, notGbEqs));
		else
			finalCond = andExprList(LIST_MAKE(childComp, notGbEqs));

		gc = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
	}
	else //no group by is true since full provenance
	{
		gc = TRUE;
	}

	return gc;
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


static boolean
checkSumNegative(FunctionCall *fc, QueryOperator *op)
{
	QueryOperator *childOp = OP_LCHILD(op);
	ProjectionOperator *proj = (ProjectionOperator *) childOp;
	List *attrRefs = proj->projExprs;
	boolean isValid = FALSE;

	if(streq(fc->functionname, SUM_FUNC_NAME) || streq(fc->functionname, MIN_FUNC_NAME))
	{
		Node *cond = getConds(childOp);
		Node *curAttr = getHeadOfListP(fc->args);

		if(isA(curAttr, AttributeReference))
		{
			AttributeReference *attr = (AttributeReference *) curAttr;
			AttributeReference *childAttrOrOper = getNthOfListP(attrRefs, attr->attrPosition);

			Node *oper = (Node *) createOpExpr(">=",LIST_MAKE(childAttrOrOper,createConstInt(0)));
			Node *finalCond = NULL;
			if(cond != NULL  && oper != NULL)
			{
				finalCond = AND_EXPRS(cond,oper);
			}
			DEBUG_NODE_BEATIFY_LOG("finalCond: ", finalCond);
			isValid = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
			DEBUG_LOG("sum or min and not constant: %d",isValid);
		}
	}

	DEBUG_LOG("checkSumNegative is valid? 0 is invalid, 1 is valid: %d", isValid);
	return isValid;
}

static boolean
checkCountAndSumPositive(FunctionCall *fc, QueryOperator *op)
{
	QueryOperator *childOp = OP_LCHILD(op);
	ProjectionOperator *proj = (ProjectionOperator *) childOp;
	List *attrRefs = proj->projExprs;
	boolean isValid = FALSE;

	if(streq(fc->functionname, COUNT_FUNC_NAME))
	{
		isValid = TRUE;
	}
	else if(streq(fc->functionname, SUM_FUNC_NAME) || streq(fc->functionname, MAX_FUNC_NAME))
	{
		Node *cond = getConds(childOp);
		//sum(a+b)? then need to construct a+b >= 0 or only a >= 0
		//since we need negation, then just construct a+b<0 or a<0
		Node *curAttr = getHeadOfListP(fc->args);
		if(isA(curAttr, Constant))
		{
			isValid = TRUE;
			DEBUG_LOG("sum or max and constant: %d",isValid);
		}
		else if(isA(curAttr, AttributeReference))
		{
			//curAttr is AGG_GB_ARG0 -> a+b or AGG_GB_ARG0 -> a where sum(a+b) or sum(a)
			AttributeReference *attr = (AttributeReference *) curAttr;
			AttributeReference *childAttrOrOper = getNthOfListP(attrRefs, attr->attrPosition);

			Node *oper = (Node *) createOpExpr("<",LIST_MAKE(childAttrOrOper,createConstInt(0)));
			Node *finalCond = NULL;
			if(cond != NULL  && oper != NULL)
			{
				finalCond = AND_EXPRS(cond,oper);
			}
			DEBUG_NODE_BEATIFY_LOG("finalCond: ", finalCond);
			isValid = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
			DEBUG_LOG("sum or max and not constant: %d",isValid);
		}
	}

	DEBUG_LOG("checkCountAndSumPositive is valid? 0 is invalid, 1 is valid: %d", isValid);
	return isValid;
}



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
gcBottomUp(QueryOperator *root, List *l)
{
	THROW(SEVERITY_PANIC, "%s", "cannot call inference when not compiled with Z3 support");
}

#endif
