/*-----------------------------------------------------------------------------
 *
 * z3_solver.c
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
#include "provenance_rewriter/coarse_grained/z3_solver.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"
#include "z3.h"
#include "stdlib.h"

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


static Z3_ast
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

		if(streq(name, "and"))
		{
			c = Z3_mk_and(ctx, 2, args);
		}
		else if(streq(name, "or"))
		{
			c = Z3_mk_or(ctx, 2, args);
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


void
testp()
{
	DEBUG_LOG("TEST Z3 SOLVER .");

	List *argLists0 = NIL;
	AttributeReference *a = createAttributeReference("a");
	AttributeReference *b = createAttributeReference("b");
	argLists0 = LIST_MAKE(a,b);
	Operator *opr0 = createOpExpr("+", argLists0);

	List *argLists1 = NIL;
	//AttributeReference *a1 = createAttributeReference("area");
	Constant *cst1 = createConstInt(5);
	argLists1 = LIST_MAKE(opr0,cst1);
	Operator *opr1 = createOpExpr("=", argLists1);

	List *argLists2 = NIL;
	AttributeReference *a2 = createAttributeReference("c");
	Constant *cst2 = createConstInt(20);
	argLists2 = LIST_MAKE(a2,cst2);
	Operator *opr2 = createOpExpr(">=", argLists2);

	List *argLists3 = NIL;
	argLists3 = LIST_MAKE(opr1,opr2);
	Operator *opr3 = createOpExpr("and", argLists3);

    	Z3_context ctx = mk_context();

    Z3_ast c = exprtoz3((Node *) opr3, ctx);
    DEBUG_LOG("expr to z3 result: %s", Z3_ast_to_string(ctx, c));
}
