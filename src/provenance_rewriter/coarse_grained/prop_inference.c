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
#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/coarse_grained/prop_inference.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"
#include "stdlib.h"
#include "z3.h"
#include "provenance_rewriter/coarse_grained/z3_solver.h"
#include "instrumentation/timing_instrumentation.h"

void display_version()
{
    unsigned major, minor, build, revision;
    Z3_get_version(&major, &minor, &build, &revision);
    DEBUG_LOG("Z3 SOLVER VERSION %d.%d.%d.%d", major, minor, build, revision);
}

/**
   \brief exit gracefully in case of error.
*/
void exitf(const char* message)
{
  fprintf(stderr,"BUG: %s.\n", message);
  exit(1);
}

/**
   \brief Simpler error handler.
 */
void error_handler(Z3_context c, Z3_error_code e)
{
    printf("Error code: %d\n", e);
    exitf("incorrect use of Z3");
}

/**
   brief Create a logical context.
   Enable model construction. Other configuration parameters can be passed in the cfg variable.
   Also enable tracing to stderr and register custom error handler.
*/
Z3_context mk_context_custom(Z3_config cfg, Z3_error_handler err)
{
    Z3_context ctx;

    Z3_set_param_value(cfg, "model", "true");
    ctx = Z3_mk_context(cfg);
    Z3_set_error_handler(ctx, err);

    return ctx;
}

/**
   \brief Create a logical context.
   Enable model construction only.
   Also enable tracing to stderr and register standard error handler.
*/
Z3_context mk_context()
{
    Z3_config  cfg;
    Z3_context ctx;
    cfg = Z3_mk_config();
    ctx = mk_context_custom(cfg, error_handler);
    Z3_del_config(cfg);
    return ctx;
}

/**
   \brief "Hello world" example: create a Z3 logical context, and delete it.
*/
void simple_example()
{
    Z3_context ctx;
    //LOG_MSG("simple_example");
    DEBUG_LOG("Z3 SIMPLE EXAMPLE.");

    ctx = mk_context();

    /* delete logical context */
    Z3_del_context(ctx);
}

Z3_solver mk_solver(Z3_context ctx)
{
  Z3_solver s = Z3_mk_solver(ctx);
  Z3_solver_inc_ref(ctx, s);
  return s;
}

void del_solver(Z3_context ctx, Z3_solver s)
{
  Z3_solver_dec_ref(ctx, s);
}

/**
   \brief Create a variable using the given name and type.
*/
Z3_ast mk_var(Z3_context ctx, const char * name, Z3_sort ty)
{
    Z3_symbol   s  = Z3_mk_string_symbol(ctx, name);
    return Z3_mk_const(ctx, s, ty);
}

/**
   \brief Create an integer variable using the given name.
*/
Z3_ast mk_int_var(Z3_context ctx, const char * name)
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return mk_var(ctx, name, ty);
}

/**
   \brief Create a Z3 integer node using a C int.
*/
Z3_ast mk_int(Z3_context ctx, int v)
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return Z3_mk_int(ctx, v, ty);
}

/**
   \brief Check whether the logical context is satisfiable, and compare the result with the expected result.
   If the context is satisfiable, then display the model.
*/
void check(Z3_context ctx, Z3_solver s, Z3_lbool expected_result)
{
    Z3_model m      = 0;
    Z3_lbool result = Z3_solver_check(ctx, s);
    switch (result) {
    case Z3_L_FALSE:
    		DEBUG_LOG("unsat\n");
        break;
    case Z3_L_UNDEF:
        printf("unknown\n");
        m = Z3_solver_get_model(ctx, s);
        if (m) Z3_model_inc_ref(ctx, m);
        DEBUG_LOG("potential model:\n%s\n", Z3_model_to_string(ctx, m));
        break;
    case Z3_L_TRUE:
        m = Z3_solver_get_model(ctx, s);
        if (m) Z3_model_inc_ref(ctx, m);
        DEBUG_LOG("sat\n%s\n", Z3_model_to_string(ctx, m));
        break;
    }
    if (result != expected_result) {
        exitf("unexpected result");
    }
    if (m) Z3_model_dec_ref(ctx, m);
}


Z3_ast
exprtoz3(Node *n,Z3_context ctx)
{
	char *name;
	List *argLists = NIL;
	Z3_ast c;

	if(isA(n, AttributeReference))
	{
		AttributeReference *a = (AttributeReference *) n;
		c = mk_int_var(ctx, a->name);
	}
	else if(isA(n, Constant))
	{
		Constant *cst = (Constant *) n;
		c = mk_int(ctx, INT_VALUE(cst));
	}
	else if(isA(n, Operator))
	{
		Operator *opr = (Operator *) n;
		argLists = opr->args;
		name = opr->name;
		//Node *argl = getHeadOfListP(argLists);
		//Node *argr = getTailOfListP(argLists);

		Z3_ast args[2];
		args[0] = exprtoz3(getHeadOfListP(argLists),ctx);
		if(LIST_LENGTH(argLists) > 1)
			args[1] = exprtoz3(getTailOfListP(argLists),ctx);

		if(streq(name, "AND") || streq(name, "and"))
		{
			c = Z3_mk_and(ctx, 2, args);
		}
		else if(streq(name, "OR") || streq(name, "or"))
		{
			c = Z3_mk_or(ctx, 2, args);
		}
		else if(streq(name, "NOT") || streq(name, "not"))
		{
			c = Z3_mk_not(ctx, args[0]);
		}
		else if(streq(name, "+"))
		{
			c = Z3_mk_add(ctx, 2, args);
		}
		else if(streq(name, "-"))
		{
			c = Z3_mk_sub(ctx, 2, args);
		}
		else if(streq(name, "*"))
		{
			c = Z3_mk_mul(ctx, 2, args);
		}
		else if(streq(name, "/"))
		{
			c = Z3_mk_div(ctx, args[0], args[1]);
		}
		else if(streq(name, "<>"))
		{
			c = Z3_mk_eq(ctx, args[0], args[1]);
			c = Z3_mk_not(ctx, c);
		}
		else if(streq(name, "="))
		{
			c = Z3_mk_eq(ctx, args[0], args[1]);
		}
		else if(streq(name,">"))
		{
			c = Z3_mk_gt(ctx, args[0], args[1]);
		}
		else if(streq(name,"<"))
		{
			c = Z3_mk_lt(ctx, args[0], args[1]);
		}
		else if(streq(name,"<="))
		{
			c = Z3_mk_le(ctx, args[0], args[1]);
		}
		else if(streq(name,">="))
		{
			c = Z3_mk_ge(ctx, args[0], args[1]);
		}
		else
		{
			args[0] = mk_int_var(ctx, "a");
			args[1] = mk_int(ctx, 1);
			c = Z3_mk_eq(ctx, args[0], args[1]);
		}
	}
	else
	{
		c = mk_int(ctx, 0);
	}

	return c;
}

/* consts */
#define RIGHT_ATTR_PREFIX backendifyIdentifier("R")



/*
 * bottom-up propagation of expression
 */
void
computeEXPRPropBottomUp (QueryOperator *root)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_EXPR_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_EXPR_DONE_BU))
		    	computeEXPRPropBottomUp(op);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			List *expr = NIL;
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) expr);
		}
		else if(isA(root,ProjectionOperator))
		{
			Node *nChild = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EXPR);
			List *expr = (List *) copyObject(nChild);

			ProjectionOperator *proj = (ProjectionOperator *) root;
			List *attrDefs = root->schema->attrDefs;
			int cnt = 0;
			FOREACH(Node, n, proj->projExprs)
			{
				if(isA(n, Operator))
				{
					AttributeDef *attrDef = (AttributeDef *) getNthOfListP(attrDefs, cnt);
					AttributeReference *attr = createFullAttrReference(attrDef->attrName, 0, cnt, 0, attrDef->dataType);
					Operator *new = createOpExpr("=", LIST_MAKE(attr,copyObject(n)));
					expr = appendToTailOfList(expr, new);
				}
				cnt ++;
			}
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) expr);
		}
		else if(isA(root, JoinOperator))
		{
			Node *lChild = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EXPR);
			List *lexpr = (List *) copyObject(lChild);
			Node *rChild = getStringProperty(OP_RCHILD(root), PROP_STORE_SET_EXPR);
			List *rexpr = (List *) copyObject(rChild);

			List *cond = NIL;
			JoinOperator *join = (JoinOperator *) root;
			if(join->joinType == JOIN_INNER)
			{
				cond = appendToTailOfList(cond, copyObject(join->cond));
			}

			List *expr = NIL;
			if(cond == NIL)
			{
				expr = CONCAT_LISTS(lexpr, rexpr);
			}
			else
			{
				expr = CONCAT_LISTS(cond, lexpr, rexpr);
			}
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) expr);
		}
		else
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getStringProperty(childOp, PROP_STORE_SET_EXPR);
			List *expr = (List *) copyObject(nChild);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EXPR, (Node *) expr);
		}
	}
}

static boolean
printEXPRProVisitor (QueryOperator *op, void *context)
{
    List *expr = (List*) getStringProperty(op, PROP_STORE_SET_EXPR);
    DEBUG_LOG("op(%s) - size:%d - expr:%s\n ",op->schema->name, LIST_LENGTH(expr), nodeToString(expr));
    return TRUE;
}

static void
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
computePREDPropBottomUp (QueryOperator *root)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_PRED_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_PRED_DONE_BU))
		    	computePREDPropBottomUp(op);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			List *pred = NIL;

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

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
		}
		else if(isA(root,SelectionOperator))
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getStringProperty(childOp, PROP_STORE_SET_PRED);
			List *pred = (List *) copyObject(nChild);

			SelectionOperator *sel = (SelectionOperator *) root;
			pred = appendToTailOfList(pred, copyObject(sel->cond));
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
		}
		else if(isA(root, JoinOperator))
		{
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *nlChild = getStringProperty(lchildOp, PROP_STORE_SET_PRED);
			Node *nrChild = getStringProperty(rchildOp, PROP_STORE_SET_PRED);
			List *pred = concatTwoLists((List *) copyObject(nlChild), (List *) copyObject(nrChild));
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
		}
		else
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getStringProperty(childOp, PROP_STORE_SET_PRED);
			List *pred = (List *) copyObject(nChild);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_PRED, (Node *) pred);
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

static void
printPREDPro(QueryOperator *root)
{
    START_TIMER("PropertyInference - PRED - print");
    visitQOGraph(root, TRAVERSAL_PRE, printPREDProVisitor, NULL);
    STOP_TIMER("PropertyInference - PRED - print");
}



static char *
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

static char *
getRightAttrName (char *attr)
{
    return CONCAT_STRINGS(RIGHT_ATTR_PREFIX, "_",
            escapeUnderscore(attr));
}

static boolean
getListOfNonAndOperators(Node *node, List **state)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, Operator))
    {
    		Operator *oper = (Operator *) node;
    		char *name = oper->name;
    		if(!streq(name, "AND") && !streq(name, "and"))
    		{
    			Node *arg2 = getTailOfListP(oper->args);
    			if(isA(arg2, SQLParameter))
    			{
    				*state = appendToTailOfList(*state, copyObject(oper));
    			}
    		}
    }
    return visit(node, getListOfNonAndOperators, state);
}


static List *
checkNonGBPredEqual(Set *gbset, HashMap *lmap, HashMap *rmap, Node *n)
{
	List *res = NIL;
	List *opersList = NIL;

	//get operators which is not a AND operators and contains SQLParameter
	getListOfNonAndOperators(n, &opersList);

	//check whether values are equal or not of the non-group by attributes operators
	List *nonGBList = NIL;
	boolean flag = TRUE;
	FOREACH(Operator, oper, opersList)
	{
		AttributeReference *attr = (AttributeReference *) getHeadOfListP(oper->args);
		SQLParameter *para = (SQLParameter *) getTailOfListP(oper->args);
		if(!hasSetElem(gbset, attr->name))
		{
			nonGBList = appendToTailOfList(nonGBList, strdup(attr->name));
			int lv = INT_VALUE(getMapString(lmap,para->name));
			int rv = INT_VALUE(getMapString(rmap,para->name));
			if(lv != rv)
			{
				flag = FALSE;
			}
		}
	}

	//retrun cmap list
	if(flag == TRUE)
	{
		nonGBList = appendToHeadOfList(nonGBList, "##");
		res = nonGBList;
	}

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
    		if(!streq(name, "AND") && !streq(name, "and"))
    		{
    			Node *arg1 = getHeadOfListP(oper->args);
    			Node *arg2 = getTailOfListP(oper->args);
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
    return visit(node, replaceParaWithValues, map);
}

static boolean
replaceParaWithValuesAndRenameAttr(Node *node, HashMap *map)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, Operator))
    {
    		Operator *oper = (Operator *) node;
    		char *name = oper->name;
    		DEBUG_LOG("oper name: %s", name);
    		if(!streq(name, "AND") && !streq(name, "and"))
    		{
    			Node *arg1 = getHeadOfListP(oper->args);
    			Node *arg2 = getTailOfListP(oper->args);
    			if(isA(arg2, SQLParameter))
    			{
    				AttributeReference *attr = (AttributeReference *) arg1;
    				attr->name = getRightAttrName(attr->name);
    				SQLParameter *para = (SQLParameter *) arg2;
    				//DEBUG_LOG("name: %s", para->name);
    				int v = INT_VALUE(getMapString(map,para->name));
    				//DEBUG_LOG("value: %d", v);
    				Constant *c = createConstInt(v);
    				oper->args = LIST_MAKE((Node *) attr, (Node *) c);
    				//DEBUG_LOG("attr name2: %s", attr->name);
    			}
    		}
    }
    return visit(node, replaceParaWithValuesAndRenameAttr, map);
}

static boolean
renameAttrInOperator(Node *node, char *state)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, AttributeReference))
    {
    		AttributeReference *attr = (AttributeReference *) node;
    		attr->name = getRightAttrName(attr->name);
    }
    return visit(node, renameAttrInOperator, state);
}

static boolean
extractOperators(Node *node, List **state)
{
    if (node == NULL)
        return FALSE;

    if(isA(node, Operator))
    {
    		Operator *oper = (Operator *) node;
    		char *name = oper->name;
    		if(!streq(name, "AND") && !streq(name, "and"))
    		{
    			*state = appendToTailOfList(*state, copyObject(oper));
    		}
    }
    return visit(node, extractOperators, state);
}

static List *
removeGBOperators(List *opers, Set *gbNames)
{
	List *res = NIL;
	FOREACH(Operator, oper, opers)
	{
		Node *n = getHeadOfListP(oper->args);
		if(isA(n, AttributeReference))
		{
			AttributeReference *attr = (AttributeReference *) n;
			if(!hasSetElem(gbNames, attr->name))
			{
				res = appendToTailOfList(res, oper);
			}
		}
	}
	return res;
}

static List *
renameRight(List *opers)
{
	List *res = NIL;
	FOREACH(Operator, oper, opers)
	{
		Node *n = getHeadOfListP(oper->args);
		if(isA(n, AttributeReference))
		{
			AttributeReference *attr = (AttributeReference *) n;
			attr->name = getRightAttrName(attr->name);
		}
		res = appendToTailOfList(res, oper);
	}

	//DEBUG_NODE_BEATIFY_LOG("0000000000: ", opers);
	//DEBUG_NODE_BEATIFY_LOG("0000000001: ", res);
	return res;
}

static boolean
checkCountAndSumPositive(FunctionCall *fc, QueryOperator *agChild, HashMap* map)
{
	boolean f = FALSE;

	if(streq(fc->functionname, "count"))
	{
		f = TRUE;
	}
	else if(streq(fc->functionname, "sum") || streq(fc->functionname, "max"))
	{
		Node *exprNode  =  getStringProperty(agChild, PROP_STORE_SET_EXPR);
		List *expr = (List *) copyObject(exprNode);
		//Node *andexpr = andExprList(expr);

		Node *predNode  =  getStringProperty(agChild, PROP_STORE_SET_PRED);
		replaceParaWithValues(predNode, map);
		List *pred = (List *) copyObject(predNode);
		Node *n = getHeadOfListP(fc->args);
		if(isA(n, Constant))
		{
			f = TRUE;
		}
		else if(isA(n, AttributeReference))
		{
			AttributeReference *attr = (AttributeReference *) copyObject(n);
			//char *name = attr->name;
			Operator *oper = createOpExpr("<",LIST_MAKE(attr,createConstInt(0)));
			Node *andPred = andExprList(CONCAT_LISTS(copyObject(pred), copyObject(expr), singleton(copyObject(oper))));
			DEBUG_NODE_BEATIFY_LOG("andPred: ", andPred);

			Z3_context ctx = mk_context();
			Z3_ast c = exprtoz3((Node *) andPred, ctx);
			DEBUG_LOG("pred and expr and not a > 0 to z3 result: %s", Z3_ast_to_string(ctx, c));
		    Z3_solver s;
		    s = mk_solver(ctx);
		    Z3_solver_assert(ctx, s, c);
		    if(Z3_solver_check(ctx, s) == Z3_L_FALSE)
		    {
		    		f = TRUE;
		    }
		}
	}

	DEBUG_LOG("checkCountAndSumPositive return %d", f);
	return f;
}


static boolean
checkSumNegative(FunctionCall *fc, QueryOperator *agChild, HashMap* map)
{
	boolean f = FALSE;

	if(streq(fc->functionname, "sum") || streq(fc->functionname, "min"))
	{
		Node *exprNode  =  getStringProperty(agChild, PROP_STORE_SET_EXPR);
		List *expr = (List *) copyObject(exprNode);

		Node *predNode  =  getStringProperty(agChild, PROP_STORE_SET_PRED);
		replaceParaWithValues(predNode, map);
		List *pred = (List *) copyObject(predNode);
		Node *n = getHeadOfListP(fc->args);
		if(isA(n, AttributeReference))
		{
			AttributeReference *attr = (AttributeReference *) copyObject(n);
			//char *name = attr->name;
			Operator *oper = createOpExpr(">=",LIST_MAKE(attr,createConstInt(0)));
			Node *andPred = andExprList(CONCAT_LISTS(copyObject(pred), copyObject(expr), singleton(copyObject(oper))));

			Z3_context ctx = mk_context();
			Z3_ast c = exprtoz3((Node *) andPred, ctx);
			DEBUG_LOG("pred and expr and not a < 0 to z3 result: %s", Z3_ast_to_string(ctx, c));
		    Z3_solver s;
		    s = mk_solver(ctx);
		    Z3_solver_assert(ctx, s, c);
		    if(Z3_solver_check(ctx, s) == Z3_L_FALSE)
		    {
		    		f = TRUE;
		    }
		}
	}

	DEBUG_LOG("checkSumNegative return %d", f);
	return f;
}

static Node *
createNotOperator(Node *n)
{
	Operator *notOperator = createOpExpr("NOT", singleton(n));

	return (Node *) notOperator;
}

static boolean
checkIfReuse(QueryOperator *root, HashMap *lmap, HashMap *rmap)
{
	boolean res = FALSE;

	Node *n = getStringProperty(root, PROP_STORE_SET_PRED);
	Node *n1 = copyObject(n);
	Node *n2 = copyObject(n);
	replaceParaWithValues(n1, lmap);
	replaceParaWithValuesAndRenameAttr(n2, rmap);
	List *lpred = (List *) n1;
	List *rpred = (List *) n2;

	Node *cmapNode = getStringProperty(root, PROP_STORE_SET_CMAP);
	List *cmap = (List *) copyObject(cmapNode);

	Node *notLPred = createNotOperator(andExprList(lpred));
	Node *AndAll = andExprList(CONCAT_LISTS(copyObject(cmap), copyObject(rpred), singleton(copyObject(notLPred))));


	Z3_context ctx1 = mk_context();
	Z3_ast c1 = exprtoz3(andExprList(cmap), ctx1);
	//DEBUG_LOG("cmap and pred' and not pred to z3 result: %s", Z3_ast_to_string(ctx3, c3));
	DEBUG_LOG("only cmap to z3 result: %s", Z3_ast_to_string(ctx1, c1));

	Z3_context ctx2 = mk_context();
	Z3_ast c2 = exprtoz3(andExprList(rpred), ctx2);
	//DEBUG_LOG("cmap and pred' and not pred to z3 result: %s", Z3_ast_to_string(ctx3, c3));
	DEBUG_LOG("only right pred' to z3 result: %s", Z3_ast_to_string(ctx2, c2));

	Z3_context ctx3 = mk_context();
	Z3_ast c3 = exprtoz3(notLPred, ctx3);
	//DEBUG_LOG("cmap and pred' and not pred to z3 result: %s", Z3_ast_to_string(ctx3, c3));
	DEBUG_LOG("only not left pred to z3 result: %s", Z3_ast_to_string(ctx3, c3));

	Z3_context ctx = mk_context();
	Z3_ast c = exprtoz3(AndAll, ctx);
	DEBUG_LOG("cmap and pred' and not pred to z3 result: %s", Z3_ast_to_string(ctx, c));
    Z3_solver s;
    s = mk_solver(ctx);
    Z3_solver_assert(ctx, s, c);
    if(Z3_solver_check(ctx, s) == Z3_L_FALSE)
    {
    		res = TRUE;
    }

    DEBUG_LOG("checkIfReuse is %d", res);
	return res;
}




/*
 * bottom-up propagation of cmap
 */
void
computeCMAPPropBottomUp (QueryOperator *root, HashMap *lmap, HashMap *rmap)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_SET_CMAP_DONE_BU);

    if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_STORE_SET_CMAP_DONE_BU))
		    	computeCMAPPropBottomUp(op, lmap, rmap);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			List *cmap = NIL;
			int cnt = 0;
		    FOREACH(AttributeDef, attr, root->schema->attrDefs)
		    {
		        AttributeReference *lattr = createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType);
		        AttributeReference *rattr = createFullAttrReference(getRightAttrName(attr->attrName), 0, cnt, 0, attr->dataType);
		        Operator *oper = createOpExpr("=",LIST_MAKE(lattr,rattr));
		        cmap = appendToTailOfList(cmap, oper);
		        cnt++;
		    }

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_CMAP, (Node *) cmap);
		}
		else if(isA(root,AggregationOperator))
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *cmapNode = getStringProperty(childOp, PROP_STORE_SET_CMAP);
			List *cmap = (List *) copyObject(cmapNode);

			Node *exprNode = getStringProperty(childOp, PROP_STORE_SET_EXPR);
			List *expr = (List *) copyObject(exprNode);
			Node *andExpr = andExprList(expr);

			AggregationOperator *agOp = (AggregationOperator *) root;
			QueryOperator *agChild = OP_LCHILD(root);
			List *gbcmap = NIL;
			List *gbnames = NIL;
			//List *gbattrs = NIL;
			FunctionCall *fc = (FunctionCall *) getHeadOfListP(agOp->aggrs);
			//FOREACH(FunctionCall, fc, agOp->aggrs)
			//{
			Node *nfun = getHeadOfListP(fc->args);

			//handle count(*) where group by attrs should get from below projection
			//others like sum(a) or count(a) directly get from current aggregation operator
			if(streq(fc->functionname, "count") && isA(nfun, AttributeReference))
			{
				if(streq(((AttributeReference *) nfun)->name, "AGG_GB_ARG0") && isA(agChild, ProjectionOperator))
				{
					ProjectionOperator *proj = (ProjectionOperator *) OP_LCHILD(root);
					FOREACH(AttributeReference, attr, agOp->groupBy)
					{
						AttributeReference *pattr = (AttributeReference *) getNthOfListP(proj->projExprs,attr->attrPosition);
						AttributeReference *lattr = (AttributeReference *) copyObject(pattr);
				        AttributeReference *rattr = createFullAttrReference(getRightAttrName(pattr->name), 0, pattr->attrPosition, 0, pattr->attrType);
				        Operator *oper = createOpExpr(OPNAME_NEQ,LIST_MAKE(lattr,rattr));
				        gbcmap = appendToTailOfList(gbcmap, oper);
				        gbnames = appendToTailOfList(gbnames, strdup(pattr->name));
				        //gbattrs = appendToTailOfList(gbattrs, copyObject(pattr));
					}
				}
			}
			else
			{
				FOREACH(AttributeReference, attr, agOp->groupBy)
				{
					AttributeReference *lattr = (AttributeReference *) copyObject(attr);
			        AttributeReference *rattr = createFullAttrReference(getRightAttrName(attr->name), 0, attr->attrPosition, 0, attr->attrType);
			        Operator *oper = createOpExpr(OPNAME_NEQ,LIST_MAKE(lattr,rattr));
			        gbcmap = appendToTailOfList(gbcmap, oper);
			        gbnames = appendToTailOfList(gbnames, strdup(attr->name));
			        //gbattrs = appendToTailOfList(gbattrs, copyObject(pattr));
				}
			}
			Set *gbset = makeStrSetFromList(gbnames);
			Node *lexpr = copyObject(andExpr);
			Node *rexpr = copyObject(andExpr);
			renameAttrInOperator(rexpr,"a");

			Node *r1 = andExprList(CONCAT_LISTS(copyObject(cmap),singleton(copyObject(lexpr)), singleton(copyObject(rexpr)),copyObject(gbcmap)));
			//DEBUG_LOG("cmap + gbcmap %s", nodeToString(r1));
			//DEBUG_LOG("cmap %s %s", nodeToString(lexpr), nodeToString(rexpr));

			Z3_context ctx = mk_context();

			Z3_ast cc = exprtoz3((Node *) r1, ctx);
			DEBUG_LOG("cmap + gbcmap + expr + expr' to z3 result: %s", Z3_ast_to_string(ctx, cc));
		    Z3_solver s;
		    s = mk_solver(ctx);
		    Z3_solver_assert(ctx, s, cc);

		    boolean f1 = FALSE;
		    boolean f2 = TRUE;
		    DEBUG_LOG("f1: %d, f2: %d", f1, f2);
		    Node *predNode  =  getStringProperty(agChild, PROP_STORE_SET_PRED);
			List *pred = (List *) copyObject(predNode);
			Node *andPred = andExprList(pred);

		    if(Z3_solver_check(ctx, s) == Z3_L_FALSE)
		    {
				List *nonGBnames = NIL;
				nonGBnames = checkNonGBPredEqual(gbset, lmap, rmap, (Node *) andPred);
				if(nonGBnames == NIL)
				{
					f2 = FALSE;
				}
				else if(LIST_LENGTH(nonGBnames) == 1)
				{
					f2 = TRUE;
				}
				else
				{
					DEBUG_LOG("checkNonGBPredEqual return equal ...");
					popHeadOfListP(nonGBnames);
					List *eqOpers = NIL;
					FOREACH(char, a, nonGBnames)
					{
						AttributeReference *lattr = createAttributeReference(a);
						AttributeReference *rattr = createAttributeReference(getRightAttrName(a));
						Operator *eqOper = createOpExpr("=", LIST_MAKE(lattr,rattr));
						eqOpers = appendToTailOfList(eqOpers, eqOper);
					}
					Node *andEqOpers = andExprList(eqOpers);
					Node *notAndEqOpers = createNotOperator(andEqOpers);

					Node *r22 = andExprList(CONCAT_LISTS(copyObject(cmap),
							singleton(copyObject(lexpr)),
							singleton(copyObject(rexpr)),
							singleton(copyObject(notAndEqOpers))));

					Z3_context ctx1 = mk_context();

					Z3_ast cc1 = exprtoz3((Node *) r22, ctx1);
					DEBUG_LOG("cmap + expr + expr' + not non-gb cmap to z3 result: %s", Z3_ast_to_string(ctx1, cc1));
				    Z3_solver s1;
				    s1 = mk_solver(ctx1);
				    Z3_solver_assert(ctx1, s1, cc1);
				    if(Z3_solver_check(ctx, s) == Z3_L_FALSE)
				    {
				    		f2 = TRUE;
				    }
				    else
				    {
				    		f2 = FALSE;
				    }


//					Set *nonGBnamesSet = makeStrSetFromList(nonGBnames);
//					FOREACH(Operator, oper, cmap)
//					{
//						char *name = STRING_VALUE(getHeadOfListP(oper->args));
//						if(hasSetElem(nonGBnamesSet, name))
//						{
//							if(!streq(oper->name,"="))
//							{
//								f2 = FALSE;
//								break;
//							}
//						}
//					}
				}
		    }
		    else
		    {
		    		f1 = TRUE;
		    }

		    DEBUG_LOG("f1: %d, f2: %d", f1, f2);

		    boolean f3 = FALSE;
		    if(f2 == FALSE)
		    {
		    	 	Node *n1 = copyObject(andPred);
		    	 	Node *n2 = copyObject(andPred);
		    	 	DEBUG_NODE_BEATIFY_LOG("lopers0: ", (n1));
		    	 	DEBUG_NODE_BEATIFY_LOG("ropers0: ", (n2));

		    	 	 replaceParaWithValues(n1, lmap);
		    	 	 replaceParaWithValues(n2, rmap);
		    	 	 List *lopers = NIL;
		    	 	 List *ropers = NIL;
		    	 	DEBUG_NODE_BEATIFY_LOG("lopers1: ", (n1));
		    	 	DEBUG_NODE_BEATIFY_LOG("ropers1: ", (n2));

		    	 	 extractOperators(n1, &lopers);
		     	 extractOperators(n2, &ropers);
		     	DEBUG_NODE_BEATIFY_LOG("lopers2: ", (lopers));
		     	DEBUG_NODE_BEATIFY_LOG("ropers2: ", (ropers));

		     	 lopers = removeGBOperators(lopers, gbset);
		     	 ropers = removeGBOperators(ropers, gbset);
		     	 ropers = renameRight(ropers);
		     	DEBUG_NODE_BEATIFY_LOG("lopers3: ", (lopers));
		     	DEBUG_NODE_BEATIFY_LOG("ropers3 rename: ", (ropers));

		     	Operator *notLopers = createOpExpr("NOT", singleton(andExprList(lopers)));
		     	DEBUG_NODE_BEATIFY_LOG("not",notLopers);
		     	Node *r2 = andExprList(CONCAT_LISTS(copyObject(cmap),
		     			singleton(copyObject(lexpr)),
						singleton(copyObject(rexpr)),
						copyObject(ropers),
						singleton(copyObject(notLopers))));


				Z3_context ctx2 = mk_context();

				Z3_ast c2 = exprtoz3((Node *) r2, ctx2);
				DEBUG_LOG("cmap and expr and expr and pred' and not pred to z3 result: %s", Z3_ast_to_string(ctx2, c2));
			    Z3_solver s2;
			    s2 = mk_solver(ctx2);
			    Z3_solver_assert(ctx2, s2, c2);
			    if(Z3_solver_check(ctx2, s2) == Z3_L_FALSE)
			    {
			    		f3 = TRUE;
			    }
			    DEBUG_LOG("f1: %d, f2: %d, f3: %d", f1, f2, f3);
		    }

		    boolean f4 = TRUE;
		    boolean f5 = TRUE;
		    if(f3 == TRUE)
		    {
		    		//check f4 count or sum positive
		    		FOREACH(FunctionCall, fc, agOp->aggrs)
				{
		    			if(!checkCountAndSumPositive(fc, agChild, lmap))
		    			{
		    				f4 = FALSE;
		    				break;
		    			}
				}
		    }

		    if(f3 == TRUE && f4 == FALSE)
		    {
	    			//check f5 sum negative
	    			FOREACH(FunctionCall, fc, agOp->aggrs)
				{
	    				if(!checkSumNegative(fc, agChild, lmap))
	    				{
	    					f5 = FALSE;
	    					break;
	    				}
				}
		    }

		    DEBUG_LOG("f1: %d, f2: %d, f3: %d, f4: %d, f5: %d", f1, f2, f3, f4, f5);

			//TODO: get function attribute name, need check
			QueryOperator *agParent = OP_FIRST_PARENT(root);
			AttributeDef *adef = (AttributeDef *) getTailOfListP(agParent->schema->attrDefs);
			AttributeReference *lattr = createFullAttrReference(strdup(adef->attrName), 0, LIST_LENGTH(agParent->schema->attrDefs), 0, adef->dataType);
			AttributeReference *rattr = createFullAttrReference(getRightAttrName(strdup(adef->attrName)), 0, LIST_LENGTH(agParent->schema->attrDefs), 0, adef->dataType);
		    if(f1 == TRUE)
		    {

		    }
		    else if(f2 == TRUE)
		    {
		        Operator *oper = createOpExpr("=",LIST_MAKE(lattr,rattr));
		        cmap = appendToTailOfList(cmap, oper);		    }
		    else if(f4 == TRUE)
		    {
		        Operator *oper = createOpExpr(">=",LIST_MAKE(lattr,rattr));
		        cmap = appendToTailOfList(cmap, oper);		    }
		    else if(f5 == TRUE)
		    {
		        Operator *oper = createOpExpr("<=",LIST_MAKE(lattr,rattr));
		        cmap = appendToTailOfList(cmap, oper);
		    }
		    setStringProperty((QueryOperator *)root, PROP_STORE_SET_CMAP, (Node *) cmap);
		}
		else if(isA(root, JoinOperator))
		{
			QueryOperator *lchildOp = OP_LCHILD(root);
			QueryOperator *rchildOp = OP_RCHILD(root);
			Node *nlChild = getStringProperty(lchildOp, PROP_STORE_SET_CMAP);
			Node *nrChild = getStringProperty(rchildOp, PROP_STORE_SET_CMAP);
			List *cmap = concatTwoLists((List *) copyObject(nlChild), (List *) copyObject(nrChild));
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_CMAP, (Node *) cmap);
		}
		else
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getStringProperty(childOp, PROP_STORE_SET_CMAP);
			List *cmap = (List *) copyObject(nChild);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_CMAP, (Node *) cmap);
		}
	}
}

static boolean
printCMAPProVisitor (QueryOperator *op, void *context)
{
    List *cmap = (List*) getStringProperty(op, PROP_STORE_SET_CMAP);
    DEBUG_LOG("op(%s) - size:%d - cmap:%s\n ",op->schema->name, LIST_LENGTH(cmap), nodeToString(cmap));
    return TRUE;
}

static void
printCMAPPro(QueryOperator *root)
{
    START_TIMER("PropertyInference - CMAP - print");
    visitQOGraph(root, TRAVERSAL_PRE, printCMAPProVisitor, NULL);
    STOP_TIMER("PropertyInference - CMAP - print");
}


void
bottomUpInference(QueryOperator *root, HashMap *lmap, HashMap *rmap)
{
	DEBUG_LOG("current algebra tree: %s", operatorToOverviewString((Node *) root));
	DEBUG_NODE_BEATIFY_LOG("tree:", root);
	//DEBUG_LOG("current algebra tree: %s", nodeToString((Node *) root));

	computeEXPRPropBottomUp(root);
	printEXPRPro(root);
	computePREDPropBottomUp(root);
	printPREDPro(root);
	computeCMAPPropBottomUp(root, lmap, rmap);
	printCMAPPro(root);

	boolean reuse = checkIfReuse(root, lmap, rmap);
	DEBUG_LOG("If reuse? %d", reuse);
}


