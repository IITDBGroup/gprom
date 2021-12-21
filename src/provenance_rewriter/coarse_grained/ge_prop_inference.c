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

///* consts */
//#define RIGHT_ATTR_PREFIX backendifyIdentifier("R")

static boolean checkCountAndSumPositive(FunctionCall *fc, QueryOperator *op, HashMap *lmap, HashMap *rmap);
static Node *generateFunCompExpr(AttributeReference *a, char *op);
static boolean checkSumNegative(FunctionCall *fc, QueryOperator *op,HashMap *lmap, HashMap *rmap);
static boolean checkEqCompForListAttrRefsOfOp(QueryOperator *root, List *l, HashMap *lmap, HashMap *rmap);
static Node *getAllConds(QueryOperator *op,HashMap *lmp, HashMap *rmap);
static boolean isExistGBAttr(Node *n, List *l);
static boolean isContainAttrName(List *l, char *name);
static Node *removeGBPreds(Node *node, List *gbs);
static boolean isContainOr(Node *node, boolean *found);

static boolean
isExistGBAttr(Node *n, List *l)
{
	boolean res = FALSE;
	if(isA(n,AttributeReference))
	{
		AttributeReference *a = (AttributeReference *) n;
		char *aname = strdup(a->name);
		DEBUG_LOG("isExistGBAttr - checking %s .", aname);
		if(isContainAttrName(l, aname))
		{
			res = TRUE;
		}
	}
	return res;
}

static boolean
isContainAttrName( List *l, char *name)
{
	boolean res = FALSE;
	FOREACH(Node, n, l)
	{
		if(isA(n, AttributeReference))
		{
			AttributeReference *a = (AttributeReference *) n;
			if(streq(name, a->name))
			{
				DEBUG_LOG("isContainAttrName - contain %s = %s.", name, a->name);
				res = TRUE;
				break;
			}
		}
	}

	return res;
}

static boolean
isContainOr(Node *node, boolean *found)
{
    if (node == NULL)
        return TRUE;

    if(isA(node, Operator))
    {
    	Operator *oper = (Operator *) node;
    	char *name = oper->name;
    	DEBUG_LOG("oper name: %s", name);
    	if(streq(name, OPNAME_OR) || streq(name, "or"))
    	{
			DEBUG_LOG("find OR (or) clause!");
			*found = TRUE;
			return FALSE;
    	}
    }

    return visit(node, isContainOr, found);
}



static Node *
removeGBPreds(Node *node, List *gbs)
{
	List *l = NIL;
	List *resl = NIL;
	getSelectionCondOperatorList(node, &l);
	DEBUG_NODE_BEATIFY_LOG("getSelectionCondOperatorList: ", l);

	FOREACH(Operator, o, l)
	{
		Node *arg1 = getHeadOfListP(o->args);
		Node *arg2 = getTailOfListP(o->args);
		if(isExistGBAttr(arg1, gbs) || isExistGBAttr(arg2, gbs))
			continue;
		resl = appendToTailOfList(resl, o);
	}
	DEBUG_NODE_BEATIFY_LOG("getSelectionCondOperatorList: ", resl);

	Node *res = changeListOpToAnOpNode(resl);
	DEBUG_NODE_BEATIFY_LOG("changeListOpToAnOpNode: ", res);

	return res;
}


static boolean
replaceParaWithValues(Node *node, HashMap *map)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, Operator))
    {
    		Operator *oper = (Operator *) node;
    		char *name = oper->name;
    		DEBUG_LOG("oper name: %s", name);
    		if(!streq(name, OPNAME_AND) && !streq(name, "and") && !streq(name, OPNAME_OR) && !streq(name, "or"))
    		{
    			Node *arg1 = getHeadOfListP(oper->args);
    			Node *arg2 = getTailOfListP(oper->args);
    			if(isA(arg1, AttributeReference))
    			{
    				//TODO:need to check first
    				AttributeReference *a = (AttributeReference *) arg1;
    				char *aname = strdup(a->name);
    				char * token = strtok(aname, "\"");
    				token = strtok(NULL, " ");
    				DEBUG_LOG("token: %s", token);
    				a->name = CONCAT_STRINGS("\"",token);
    			}

    			if(isA(arg2, SQLParameter))
    			{
    				SQLParameter *para = (SQLParameter *) arg2;
    				//DEBUG_LOG("name: %s", para->name);
    				int v = INT_VALUE(getMapString(map,para->name));
    				//DEBUG_LOG("value: %d", v);
    				Constant *c = createConstInt(v);
    				oper->args = LIST_MAKE((Node *) arg1, (Node *) c);
    			}
    		}
    }

//    if(isA(node, SQLParameter))
//    {
//		SQLParameter *para = (SQLParameter *) node;
//		int v = INT_VALUE(getMapString(map,para->name));
//		Constant *c = createConstInt(v);
//		DEBUG_LOG("-- val: %d", INT_VALUE(c));
//		node =  (Node *) c;
//    }
    return visit(node, replaceParaWithValues, map);
}

HashMap *
bindsToHashMap(List *names, List *values)
{
	HashMap *map = NEW_MAP(Constant,Constant);
	FORBOTH(Constant, n, v, names, values)
	{
		MAP_ADD_STRING_KEY(map, STRING_VALUE(n), v);
	}
	return map;
}

//void
//selfTurnDoGeBottomUp(QueryOperator *op, List *paras, List *curParas)
//{
//	ParameterizedQuery *pq = queryToTemplate((QueryOperator *) op);
//	char *pqSql = serializeOperatorModel((Node *) pq->q);
//	List *curParas = pq->parameters;
//	List *paras = charToParameters(cparas);
//	//char *curParas = parameterToCharsSepByComma(pq->parameters);
//	DEBUG_LOG("pqSql: %s", pqSql);
//	//DEBUG_LOG("parameters to chars seperated by comma: %s", cparas);
//
//	//map values to variable, e.g., a -> 10
//	HashMap *lmap = bindsToHashMap(b1, b2);
//	HashMap *rmap = bindsToHashMap(b1, b3);
//
//	geBottomUp(OP_LCHILD(op), lmap, rmap);
//}

void
doGeBottomUp(QueryOperator *op)
{
	List *binds = (List *) getStringProperty(op, PROP_PC_COARSE_GRAINED_BIND);
	List *b1 = (List *) getNthOfListP(binds,0);
	List *b2 = (List *) getNthOfListP(binds,1);
	List *b3 = (List *) getNthOfListP(binds,2);

	DEBUG_LOG("bind parameters 1:");
	FOREACH(Constant, c, b1)
	{
		DEBUG_LOG("%s",STRING_VALUE(c));
	}
	DEBUG_LOG("bind values 2:");
	FOREACH(Constant, c, b2)
	{
		DEBUG_LOG("%d",INT_VALUE(c));
	}
	DEBUG_LOG("bind values 3:");
	FOREACH(Constant, c, b3)
	{
		DEBUG_LOG("%d",INT_VALUE(c));
	}

	//map values to variable, e.g., a -> 10
	HashMap *lmap = bindsToHashMap(b1, b2);
	HashMap *rmap = bindsToHashMap(b1, b3);

	geBottomUp(OP_LCHILD(op), lmap, rmap);
}

/*
 * bottom-up propagation of ge
 */
void
geBottomUp (QueryOperator *root, HashMap *lmap, HashMap *rmap)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_GE_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_GE_DONE_BU))
		    	geBottomUp(op, lmap, rmap);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			DEBUG_LOG("geBottomUp TableAccessOperator");
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

		    SET_BOOL_STRING_PROP((QueryOperator *)root, PROP_STORE_SET_GE);
		    SET_STRING_PROP(root, PROP_STORE_SET_GE_COMP, comp);
		    DEBUG_LOG("op(%s) ge(%d) - expr:%s\n ",root->schema->name, TRUE, nodeToString(comp));
		}
		else if(isA(root,AggregationOperator))
		{
			DEBUG_LOG("geBottomUp AggregationOperator");
			AggregationOperator *agg = (AggregationOperator *) root;
			QueryOperator *childOp = OP_LCHILD(root);
			boolean childGe = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GE);
			Node *childComp = copyObject(GET_STRING_PROP(childOp, PROP_STORE_SET_GE_COMP));
			Node *childExpr = copyObject(getStringProperty(childOp, PROP_STORE_SET_EXPR));
			Node *childPred = copyObject(getStringProperty(childOp, PROP_STORE_SET_PRED));

			// check ge
			boolean ge = checkEqCompForListAttrRefsOfOp(root, agg->groupBy, lmap, rmap) && childGe;
			DEBUG_LOG(" -- child ge: %d",childGe);
		    DEBUG_LOG(" -- current ge: %d",ge);

		    /* compute comp for function attrs b and b'*/

		    DEBUG_LOG("-- compute comp: ");
		    Node *funComp = NULL;

			boolean cond1IsValid = FALSE;
			boolean cond2IsValid = FALSE;

			if(childPred == NULL)
			{
				DEBUG_LOG("-- childPred is NULL!");
				cond1IsValid = TRUE;
				cond2IsValid = TRUE;
			}
			else
			{
				/* prepare */

				//replace the variables in the pred with values
				Node *childPred1 = NULL;
				Node *childPred2 = NULL;
				Node *childExpr1 = NULL;
				Node *childExpr2 = NULL;
				Node *notChildPred1 = NULL;
				Node *notChildPred2 = NULL;

				if(childPred != NULL)
				{
					DEBUG_NODE_BEATIFY_LOG("groupBy: ", agg->groupBy);
					//List *rpGroupBy = removePrefixOfAttrs(agg->groupBy);
					List *rpGroupBy = agg->groupBy;
					DEBUG_NODE_BEATIFY_LOG("rpGroupBy: ", rpGroupBy);

					DEBUG_LOG("-- test!");
					DEBUG_LOG("-- childPred before: %s",nodeToString(childPred));

					//if contains or clause, just keep pred, else remove pred contains group by attrs
					boolean foundOr = FALSE;
					isContainOr(childPred, &foundOr);
					DEBUG_LOG("is Contains Or: %d", foundOr);
					if(!foundOr)
						childPred = removeGBPreds(childPred, rpGroupBy);

					DEBUG_LOG("-- childPred after: %s",nodeToString(childPred));

					DEBUG_LOG("-- childPred is not NULL!");
					childPred1 = copyObject(childPred);
					childPred2 = copyObject(childPred);
					replaceParaWithValues(childPred1, lmap); //Q
					replaceParaWithValues(childPred2, rmap); //Q' (current one)

					//add prime "'" to left side pred (Q')
					addPrimeOnAttrsInOperator(childPred2,"dummy");
					notChildPred1 = (Node *) createOpExpr("NOT", singleton(copyObject(childPred1)));
					notChildPred2 = (Node *) createOpExpr("NOT", singleton(copyObject(childPred2)));
				}

				if(childExpr != NULL)
				{
					DEBUG_LOG("-- childExpr is not NULL!");
					childExpr1 = copyObject(childExpr);
					childExpr2 = copyObject(childExpr);

					//add prime "'" to left side expr (Q')
					addPrimeOnAttrsInOperator(childExpr2,"dummy");
				}

				DEBUG_LOG(" -- childPred1: %s",nodeToString(childPred1));
				DEBUG_LOG(" -- childPred2: %s",nodeToString(childPred2));
				DEBUG_LOG(" -- childExpr1: %s",nodeToString(childExpr1));
				DEBUG_LOG(" -- childExpr2: %s",nodeToString(childExpr2));
				DEBUG_LOG(" -- notChildPred1: %s",nodeToString(notChildPred1));
				DEBUG_LOG(" -- notChildPred2: %s",nodeToString(notChildPred2));

				//1. ΨQ1′,Q1 ∧non-grp-pred(Q1)∧expr(Q1)∧expr(Q1′)→non-grp-pred(Q1′)
				//2. ΨQ1′,Q1 ∧non-grp-pred(Q1′)∧expr(Q1′)∧expr(Q1)→non-grp-pred(Q1)
				Node *cond1 = NULL;
				Node *cond2 = NULL;
				if(childExpr1 != NULL)
				{
					cond2 = andExprList(LIST_MAKE(copyObject(childComp),childExpr1,childExpr2,childPred2,notChildPred1));
					cond1 = andExprList(LIST_MAKE(copyObject(childComp),childExpr1,childExpr2,childPred1,notChildPred2));
				}
				else
				{
					cond2 = andExprList(LIST_MAKE(copyObject(childComp),childPred2,notChildPred1));
					cond1 = andExprList(LIST_MAKE(copyObject(childComp),childPred1,notChildPred2));
				}

				//unSatisfiable is valid
				cond1IsValid = !z3ExprIsSatisfiable((Node *) cond1, TRUE);
				cond2IsValid = !z3ExprIsSatisfiable((Node *) cond2, TRUE);
				DEBUG_LOG("-- cond1IsValid:  %d, cond2IsValid: %d", cond1IsValid, cond2IsValid);
			}

			/* get function names from its parent operator and store in funAttrs */
			QueryOperator *p = OP_FIRST_PARENT(root);
			ProjectionOperator *pproj = (ProjectionOperator *) p;
			List *funAttrs = NIL;
			int cnt = 0;

			INFO_OP_LOG("pproj: ", pproj);
			/*
			Projection [count(1) F0_0."beat" ] ("cnt": DT_LONG, "beat": DT_LONG) [0x1107f6fa8](0x1107f67f7)
			  Aggregation [(count(1))] GROUP BY [(F0_0."beat")] ("AGGR_0": DT_LONG, "GROUP_0": DT_LONG) [0x1107f775d](0x1107f6fa8)
			    Projection [1 F0_0."beat" ] ("AGG_GB_ARG0": DT_INT, "AGG_GB_ARG1": DT_LONG) [0x1107f7fe0](0x1107f775d)
			 */
//			DEBUG_NODE_BEATIFY_LOG("pproj->projExprs: ", pproj->projExprs);
//			FOREACH(AttributeReference,arf,removePrefixOfAttrs(pproj->projExprs))
//			{	DEBUG_LOG("-- pproj->projExprs:  %s", arf->name);
//				if(isStartAsAGG(arf->name))
//				{
//					DEBUG_LOG("-- isStartAsAGG:  %s", arf->name);
//					AttributeDef *ad = getAttrDef(p,cnt);
//					AttributeReference *attr = createFullAttrReference(ad->attrName, 0, cnt, 0, ad->dataType);
//					funAttrs = appendToTailOfList(funAttrs, attr);
//				}
//				cnt++;
//			}

			List *projDefs = p->schema->attrDefs;
			FOREACH(AttributeReference,arf,pproj->projExprs)
			{
				char *arfName = arf->name;
				if(arfName[0] != 'F') //F0_0."beat"
				{
					AttributeDef *ad = (AttributeDef *) getNthOfListP(projDefs, cnt);
					AttributeReference *newAttr = copyObject(arf);
					newAttr->name = strdup(ad->attrName);
					funAttrs = appendToTailOfList(funAttrs,newAttr); //count(1) -> cnt, add "cnt"
				}
				cnt ++;
			}

	    	List *funCompList = NIL;
	    	DEBUG_LOG("-- cond1IsValid:  %d, cond2IsValid: %d", cond1IsValid, cond2IsValid);
		    /* case 1: 1 ∧ 2 then f = f'*/
			if(cond1IsValid && cond2IsValid)
			{
				funComp = generateAttrAndPrimeEq(funAttrs);
				//DEBUG_NODE_BEATIFY_LOG("funComp: ", funComp);
			}
			else if(cond2IsValid) // case 2 & 3
			{
		    	int pos = 0;
	    		FOREACH(FunctionCall, fc, agg->aggrs)
				{
	    			AttributeReference *curAttr = (AttributeReference *) getNthOfListP(funAttrs, pos);
	    			if(checkCountAndSumPositive(fc, (QueryOperator *) agg, lmap, rmap)) //case 3
	    			{
	    				Node *geq = generateFunCompExpr(curAttr, ">=");
	    				funCompList = appendToTailOfList(funCompList, geq);
	    			}
	    			else if(checkSumNegative(fc, (QueryOperator *) agg, lmap, rmap)) //case 2
	    			{
	    				Node *leq = generateFunCompExpr(curAttr, "<=");
	    				funCompList = appendToTailOfList(funCompList, leq);
	    			}
	    			pos++;
				}
	    		if(funCompList != NIL)
	    			funComp = andExprList(funCompList);
			}

			Node *curComp = NULL;
			if(funComp != NULL)
			{
				DEBUG_NODE_BEATIFY_LOG("funComp: ", funComp);
				curComp = AND_EXPRS(childComp, funComp);
			}
			else
				curComp = childComp;

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GE, (Node *) createConstBool(ge));
		    SET_STRING_PROP(root, PROP_STORE_SET_GE_COMP, curComp);
		    DEBUG_LOG("op(%s) ge(%d) - expr:%s\n ",root->schema->name, ge, nodeToString(curComp));
		}
		else if(isA(root, DuplicateRemoval))
		{
			DuplicateRemoval *rm = (DuplicateRemoval *) root;
			QueryOperator *childOp = OP_LCHILD(root);
			boolean childGe = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GE);
			Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GE_COMP));

			boolean ge = checkEqCompForListAttrRefsOfOp(root, rm->attrs, lmap, rmap) && childGe;

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GE, (Node *) createConstBool(ge));
		    SET_STRING_PROP(root, PROP_STORE_SET_GE_COMP, childComp);
			DEBUG_LOG("op(%s) ge(%d) - expr:%s\n ",root->schema->name, ge, nodeToString(childComp));
		}
		else if(isA(root, JoinOperator))
		{
			JoinOperator *join = (JoinOperator *) root;
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *lChildComp = getStringProperty(lchildOp, PROP_STORE_SET_GE_COMP);
			Node *rChildComp = getStringProperty(rchildOp, PROP_STORE_SET_GE_COMP);
			boolean lchildGe = GET_BOOL_STRING_PROP(lchildOp, PROP_STORE_SET_GE);
			boolean rchildGe = GET_BOOL_STRING_PROP(rchildOp, PROP_STORE_SET_GE);

			Node *cmap = NULL;
			boolean ge = FALSE;
			if(join->joinType == JOIN_CROSS)
			{
				cmap = AND_EXPRS(lChildComp,rChildComp);
				ge = lchildGe && rchildGe;
			}

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GE, (Node *) createConstBool(ge));
		    SET_STRING_PROP(root, PROP_STORE_SET_GE_COMP, cmap);
		    DEBUG_LOG("op(%s) ge(%d) - expr:%s\n ",root->schema->name, ge, nodeToString(cmap));
		}
		else if(isA(root, SetOperator))
		{
			SetOperator *sop = (SetOperator *) root;
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *lchildComp = copyObject(getStringProperty(lchildOp, PROP_STORE_SET_GE_COMP));
			Node *rchildComp = copyObject(getStringProperty(rchildOp, PROP_STORE_SET_GE_COMP));
			boolean lchildGe = GET_BOOL_STRING_PROP(lchildOp, PROP_STORE_SET_GE);
			boolean rchildGe = GET_BOOL_STRING_PROP(rchildOp, PROP_STORE_SET_GE);

			//Node *comp = comp = AND_EXPRS(lchildComp,rchildComp);
			List *compList = NIL;
			Node *comp = NULL;
			boolean ge = lchildGe && rchildGe;

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

			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GE, (Node *) createConstBool(ge));
			SET_STRING_PROP(root, PROP_STORE_SET_GE_COMP, comp);
			DEBUG_LOG("op(%s) ge(%d) - expr:%s\n ",root->schema->name, ge, nodeToString(comp));
		}
		else
		{
//			QueryOperator *childOp = OP_LCHILD(root);
//			Node *nChild = getStringProperty(childOp, PROP_STORE_SET_CMAP);
//			List *cmap = (List *) copyObject(nChild);
//			setStringProperty((QueryOperator *)root, PROP_STORE_SET_CMAP, (Node *) cmap);
			DEBUG_LOG("geBottomUp other operators (Selection, Projection)");
			QueryOperator *childOp = OP_LCHILD(root);

			Node *childComp = NULL;
			if(HAS_STRING_PROP(childOp, PROP_STORE_SET_GE_COMP))
			{
				childComp = copyObject(GET_STRING_PROP(childOp, PROP_STORE_SET_GE_COMP));
			}
			boolean childGe = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GE);
			setStringProperty((QueryOperator *) root, PROP_STORE_SET_GE, (Node *) createConstBool(childGe));
		    SET_STRING_PROP(root, PROP_STORE_SET_GE_COMP, childComp);
		    DEBUG_LOG("op(%s) ge(%d) - expr:%s\n ",root->schema->name, childGe, nodeToString(childComp));
		}
	}
}


List *
removePrefixOfAttrs(List *l)
{
	// l is a list of attributeReference
	// remove F0_0. from F0_0."beat" for each attr name
	List *res = NIL;
	FOREACH(AttributeReference, a, l)
	{
		AttributeReference *newa = (AttributeReference *) copyObject(a);
		char *aname = strdup(newa->name);
		char * token = strtok(aname, "\"");
		token = strtok(NULL, " ");
		DEBUG_LOG("token: %s", token);
		newa->name = CONCAT_STRINGS("\"",token);
		res = appendToTailOfList(res, newa);
		DEBUG_LOG("new name: %s", newa->name);
	}

	return res;
}

/*
 * Used for check something like ∀𝑔 ∈ 𝐺 : Ψ𝑄1,𝑋1 ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) ∧ 𝑐𝑜𝑛𝑑𝑠(𝑄1) → 𝑔 = 𝑔′
 * Used in aggregation, duplicate removal and join
 */
static boolean
checkEqCompForListAttrRefsOfOp(QueryOperator *root, List *l, HashMap *lmap, HashMap *rmap)
{
	QueryOperator *childOp = OP_LCHILD(root);
	Node *allconds = getAllConds(childOp,lmap,rmap);
	//boolean childGe = GET_BOOL_STRING_PROP(childOp, PROP_STORE_SET_GE);
	Node *childComp = copyObject(getStringProperty(childOp, PROP_STORE_SET_GE_COMP));
	DEBUG_NODE_BEATIFY_LOG("childComp: ", childComp);


	l = removePrefixOfAttrs(l);
	DEBUG_NODE_BEATIFY_LOG("group by lists: ", l);
	Node *gbEqs = ListAttrRefsToEqCondsForAgg(root, l);
	DEBUG_NODE_BEATIFY_LOG("group by equivalent conditions: ", gbEqs);
	//(F0_0."beat" = r_F0__0."beat"): if translate to template, then attr name with F0_0
	//("beat" = r_"beat") should be this

	Node *notGbEqs = NULL;
	if(gbEqs != NULL)
		notGbEqs = (Node *) createOpExpr("NOT", singleton(gbEqs));

	//INFO_NODE_BEATIFY_LOG(" -- gbEqs: ", gbEqs);
	boolean ge = FALSE;
	Node *finalCond = NULL;

	if(notGbEqs != NULL)
	{
		if(allconds != NULL)
			finalCond = andExprList(LIST_MAKE(childComp, allconds, notGbEqs));
		else
			finalCond = andExprList(LIST_MAKE(childComp, notGbEqs));

		INFO_NODE_BEATIFY_LOG(" -- finalCond: ", finalCond);

		ge = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
	}
	else //no group by is true since full provenance
	{
		ge = TRUE;
	}

	return ge;
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


static Node *
getAllConds(QueryOperator *op, HashMap *lmap, HashMap *rmap)
{
	Node *allconds = NULL;
	//Node *childComp = copyObject(getStringProperty(op, PROP_STORE_SET_GE_COMP));
	Node *childExpr = copyObject(getStringProperty(op, PROP_STORE_SET_EXPR));
	Node *childPred = copyObject(getStringProperty(op, PROP_STORE_SET_PRED));

	Node *childPred1 = NULL;
	Node *childPred2 = NULL;

	if(childPred != NULL)
	{
		childPred1 = copyObject(childPred);
		childPred2 = copyObject(childPred);
		INFO_NODE_BEATIFY_LOG(" -- childPred1: ", childPred1);
	    replaceParaWithValues(childPred1, lmap);
	    replaceParaWithValues(childPred2, rmap);
	    //add prime "'" to left side pred and expr
	    addPrimeOnAttrsInOperator(childPred1,"dummy");
	}

	Node *childExpr1 = NULL;
	Node *childExpr2 = NULL;
	if(childExpr != NULL)
	{
		childExpr1 = copyObject(childExpr);
		childExpr2 = copyObject(childExpr);
	    //add prime "'" to left side pred and expr
	    addPrimeOnAttrsInOperator(childExpr1,"dummy");
	}


	if(childPred != NULL && childExpr != NULL)
		allconds = andExprList(LIST_MAKE(childPred1, childPred2, childExpr1, childExpr2));
	else if(childPred != NULL && childExpr == NULL)
		allconds = andExprList(LIST_MAKE(childPred1, childPred2));
	else if(childPred == NULL && childExpr != NULL)
		allconds = andExprList(LIST_MAKE(childExpr1, childExpr2));

    return allconds;
}

static boolean
checkCountAndSumPositive(FunctionCall *fc, QueryOperator *op, HashMap *lmap, HashMap *rmap)
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
		Node *cond = getAllConds(childOp,lmap,rmap);
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
			Node *operPrime = copyObject(oper);
			addPrimeOnAttrsInOperator(operPrime,"dummy");
			Node *allOper = AND_EXPRS(oper,operPrime);

			Node *finalCond = NULL;
			if(cond != NULL  && oper != NULL)
			{
				finalCond = AND_EXPRS(cond,allOper);
			}
			DEBUG_NODE_BEATIFY_LOG("finalCond: ", finalCond);
			isValid = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
			DEBUG_LOG("sum or max and not constant: %d",isValid);
		}
	}

	DEBUG_LOG("checkCountAndSumPositive is valid? 0 is invalid, 1 is valid: %d", isValid);
	return isValid;
}

static boolean
checkSumNegative(FunctionCall *fc, QueryOperator *op, HashMap *lmap, HashMap *rmap)
{
	QueryOperator *childOp = OP_LCHILD(op);
	ProjectionOperator *proj = (ProjectionOperator *) childOp;
	List *attrRefs = proj->projExprs;
	boolean isValid = FALSE;

	if(streq(fc->functionname, SUM_FUNC_NAME) || streq(fc->functionname, MIN_FUNC_NAME))
	{
		Node *cond = getAllConds(childOp,lmap,rmap);
		Node *curAttr = getHeadOfListP(fc->args);

		if(isA(curAttr, AttributeReference))
		{
			AttributeReference *attr = (AttributeReference *) curAttr;
			AttributeReference *childAttrOrOper = getNthOfListP(attrRefs, attr->attrPosition);

			Node *oper = (Node *) createOpExpr(">=",LIST_MAKE(childAttrOrOper,createConstInt(0)));
			Node *operPrime = copyObject(oper);
			addPrimeOnAttrsInOperator(operPrime,"dummy");
			Node *allOper = AND_EXPRS(oper,operPrime);

			Node *finalCond = NULL;
			if(cond != NULL  && oper != NULL)
			{
				finalCond = AND_EXPRS(cond,allOper);
			}
			DEBUG_NODE_BEATIFY_LOG("finalCond: ", finalCond);
			isValid = !z3ExprIsSatisfiable((Node *) finalCond, TRUE);
			DEBUG_LOG("sum or min and not constant: %d",isValid);
		}
	}

	DEBUG_LOG("checkSumNegative is valid? 0 is invalid, 1 is valid: %d", isValid);
	return isValid;
}

boolean
isReusable(QueryOperator *op, HashMap *lmap, HashMap *rmap)
{
	/*
	 * ge(Q′,Q) ∧ uconds(Q′,Q) ⇒ PS is safe for Q′ and D
	 * uconds(Q′,Q) = ΨQ′,Q∧pred(Q′)∧expr(Q′)∧expr(Q) →pred(Q)
	 */

	boolean ge = GET_BOOL_STRING_PROP(op, PROP_STORE_SET_GE);
	DEBUG_LOG("isReusable ge: %d", ge);
	DEBUG_NODE_BEATIFY_LOG("cur op: ", op);
	Node *comp = copyObject(GET_STRING_PROP(op, PROP_STORE_SET_GE_COMP));
	Node *expr = copyObject(getStringProperty(op, PROP_STORE_SET_EXPR));
	Node *pred = copyObject(getStringProperty(op, PROP_STORE_SET_PRED));

	// pred2 ∧ expr2 ∧ expr1 -> pred1 (check whether ps of 1 (lmap) can be used for 2 (rmap))
    //replace the variables in the pred with values
    Node *pred1 = copyObject(pred);
    Node *pred2 = copyObject(pred);
    replaceParaWithValues(pred1, lmap); //this one is cached
    replaceParaWithValues(pred2, rmap);
    Node *npred1 = (Node *) createOpExpr("NOT", singleton(copyObject(pred1)));
    addPrimeOnAttrsInOperator(pred2,"dummy");

    Node *expr1 = copyObject(expr);
    Node *expr2 = copyObject(expr);

    Node *uconds = NULL;
    //TODO: if no predicates
    if(expr1 == NULL)
    	uconds = andExprList(LIST_MAKE(comp, pred2, npred1));
    else
    {
    	addPrimeOnAttrsInOperator(expr2,"dummy");
    	uconds = andExprList(LIST_MAKE(comp, pred2, expr2, expr1, npred1));
    }
    DEBUG_NODE_BEATIFY_LOG("pred1: ", pred1);
    DEBUG_NODE_BEATIFY_LOG("pred2: ", pred2);
    DEBUG_NODE_BEATIFY_LOG("expr1: ", expr1);
    DEBUG_NODE_BEATIFY_LOG("expr1: ", expr2);
    DEBUG_NODE_BEATIFY_LOG("psi: ", comp);
    DEBUG_NODE_BEATIFY_LOG("uconds: ", uconds);

	//unSatisfiable is valid
    boolean sat = z3ExprIsSatisfiable((Node *) uconds, TRUE);
	boolean ucondsIsValid = !sat;
	boolean isReusable = ucondsIsValid && ge;

	DEBUG_LOG("sat: %d", sat);
	DEBUG_LOG("ge: %d", ge);
	DEBUG_LOG("ucondsIsValid: %d", ucondsIsValid);
	DEBUG_LOG("isReusable: %d", isReusable);

	//return ucondsIsValid;
	return isReusable;
	//test z3ExprIsValid looks uncorrect
	//boolean gcValid = z3ExprIsValid((Node *) uconds, TRUE);

    //add prime "'" to left side pred and expr
//    addPrimeOnAttrsInOperator(childPred1,"dummy");
//    addPrimeOnAttrsInOperator(childExpr1,"dummy");
//    Node *notChildPred1 = (Node *) createOpExpr("NOT", singleton(copyObject(childPred1)));
//    Node *notChildPred2 = (Node *) createOpExpr("NOT", singleton(copyObject(childPred2)));
}

//static boolean
//printCMAPProVisitor (QueryOperator *op, void *context)
//{
//    List *cmap = (List*) getStringProperty(op, PROP_STORE_SET_CMAP);
//    DEBUG_LOG("op(%s) - size:%d - cmap:%s\n ",op->schema->name, LIST_LENGTH(cmap), nodeToString(cmap));
//    return TRUE;
//}

//static void
//printCMAPPro(QueryOperator *root)
//{
//    START_TIMER("PropertyInference - CMAP - print");
//    visitQOGraph(root, TRAVERSAL_PRE, printCMAPProVisitor, NULL);
//    STOP_TIMER("PropertyInference - CMAP - print");
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

boolean
isReusable(QueryOperator *op, HashMap *lmap, HashMap *rmap)
{
       return TRUE;
}

void
geBottomUp(QueryOperator *root, HashMap *lmap, HashMap *rmap)
{
	THROW(SEVERITY_PANIC, "%s", "cannot call inference when not compiled with Z3 support");
}

#endif
