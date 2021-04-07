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
#include "symbolic_eval/z3_solver.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"
#include "stdlib.h"
#if HAVE_Z3
#include "z3_api.h"
#include "z3.h"
#endif

#define MEMORY_CONTEXT "z3-expr-context"
#define LONG_TERM_CONTEXT "z3-solver-context"

#if	HAVE_Z3

typedef struct {
	Z3_context ctx;
	Z3_solver s;
} Z3SolverHandle;

static void gprom_z3_error_handler(Z3_context c, Z3_error_code e);

static Z3SolverHandle *solver = NULL;
static MemContext *context = NULL;

void
display_version()
{
    unsigned major, minor, build, revision;
    Z3_get_version(&major, &minor, &build, &revision);
    DEBUG_LOG("Z3 SOLVER VERSION %d.%d.%d.%d", major, minor, build, revision);
}

/**
 * Error handler that throws an exception
 */
static void
gprom_z3_error_handler(Z3_context c, Z3_error_code e)
{
    THROW(SEVERITY_RECOVERABLE,"Z3 ERROR:\nError code: %d\n%s",
		  e,
		  Z3_get_error_msg(c, e));
}

void
createSolver(void)
{
	if(!context)
	{
		NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(LONG_TERM_CONTEXT);
	}
	else
	{
		ACQUIRE_MEM_CONTEXT(context);
	}
	solver = NEW(Z3SolverHandle);
	solver->ctx = mk_context();
	solver->s = mk_solver(solver->ctx);
	RELEASE_MEM_CONTEXT();
}

void
destroySolver(void)
{
	ACQUIRE_MEM_CONTEXT(context);
    del_solver(solver->ctx, solver->s);
    Z3_del_context(solver->ctx);
	FREE(solver);
	RELEASE_MEM_CONTEXT();
}


/**
   brief Create a logical context.
   Enable model construction. Other configuration parameters can be passed in the cfg variable.
   Also enable tracing to stderr and register custom error handler.
*/
Z3_context
mk_context_custom(Z3_config cfg, Z3_error_handler err)
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
Z3_context
mk_context()
{
    Z3_config  cfg;
    Z3_context ctx;

    cfg = Z3_mk_config();
    ctx = mk_context_custom(cfg, gprom_z3_error_handler);
    Z3_del_config(cfg);

    return ctx;
}

Z3_solver
mk_solver(Z3_context ctx)
{
  Z3_solver s = Z3_mk_solver(ctx);
  Z3_solver_inc_ref(ctx, s);
  return s;
}

void
del_solver(Z3_context ctx, Z3_solver s)
{
  Z3_solver_dec_ref(ctx, s);
}

Z3_ast
mk_var(Z3_context ctx, const char * name, Z3_sort ty)
{
    Z3_symbol   s  = Z3_mk_string_symbol(ctx, name);
    return Z3_mk_const(ctx, s, ty);
}

Z3_ast
mk_int_var(Z3_context ctx, const char * name)
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return mk_var(ctx, name, ty);
}

Z3_ast
mk_bool_var(Z3_context ctx, const char * name)
{
    Z3_sort ty = Z3_mk_bool_sort(ctx);
    return mk_var(ctx, name, ty);
}

Z3_ast
mk_string_var(Z3_context ctx, const char *name)
{
   Z3_sort ty = Z3_mk_string_sort(ctx);
   return mk_var(ctx, name, ty);
}

Z3_ast
mk_float_var(Z3_context ctx, const char * name)
{
    Z3_sort ty = Z3_mk_real_sort(ctx);
    return mk_var(ctx, name, ty);
}


Z3_ast
mk_int(Z3_context ctx, int v)
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return Z3_mk_int(ctx, v, ty);
}

Z3_ast
mk_float(Z3_context ctx, float v)
{
	StringInfo s = makeStringInfo();
    Z3_sort ty = Z3_mk_real_sort(ctx);
	Z3_string str;
    appendStringInfo(s, "%f", v);
	str = s->data;
	DEBUG_LOG("float %f to string %s", v, s->data);
	return Z3_mk_numeral(ctx, str, ty);
}

boolean
z3ExprIsValid(Node *expr, boolean exceptionOnUndef)
{
	boolean result;

	result = z3ExprIsSatisfiable(
		(Node *) createOpExpr(OPNAME_NOT,
							  singleton(copyObject(expr))),
		exceptionOnUndef);

	DEBUG_LOG("constraint is %s: \n%s", result ? "not valid" : "valid",  exprToSQL(expr, NULL));
	return !result;
}

boolean
z3ExprIsSatisfiable(Node *expr, boolean exceptionOnUndef)
{
	Z3_ast constraints;
	boolean result;

	createSolver();

	// create context to get rid of Z3_ast afterwards
	NEW_AND_ACQUIRE_MEMCONTEXT(MEMORY_CONTEXT);

	constraints = exprtoz3(expr, solver->ctx);

	DEBUG_LOG("translated expr:\n%s\ninto constraint\n%s",
			  exprToSQL(expr, NULL),
			  Z3_ast_to_string(solver->ctx, constraints));

	result = z3IsSatisfiable(NULL, constraints, exceptionOnUndef);

	DEBUG_LOG("constraint is %s: \n%s", result ? "satisfiable" : "unsatisfiable",  exprToSQL(expr, NULL));

	FREE_AND_RELEASE_CUR_MEM_CONTEXT();

	return result;
}

boolean
z3IsSatisfiable(Z3_context ctx, Z3_ast constraints, boolean exceptionOnUndef)
{
	Z3_context usectx;
	Z3_solver s;
	boolean result = FALSE;
	Z3_lbool z3result;
	Z3_model m = NULL;

	if(!ctx)
	{
		if(solver == NULL)
		{
			createSolver();
		}
		usectx = solver->ctx;
		s = solver->s;
	}
	else
	{
		usectx = mk_context();
		s = mk_solver(usectx);
	}

//	Z3_solver_push(usectx, s);
	Z3_solver_assert(usectx, s, constraints);

	z3result = Z3_solver_check(usectx, s);

    switch (z3result)
	{
    case Z3_L_FALSE:
	{
		result = FALSE;
		DEBUG_LOG("constraint is unsatisfiable");
	}
	break;
    case Z3_L_UNDEF:
	{
		result = FALSE;
		m = Z3_solver_get_model(usectx, s);
		if (m)
		{
			Z3_model_inc_ref(usectx, m);
			DEBUG_LOG("unknown: Z3 failed to determine satisfiability potential counter example:\n%s",
					  Z3_model_to_string(usectx, m));
		}
		else
		{
			DEBUG_LOG("unknown: Z3 failed to determine satisfiability no potential counter example");
		}
	}
	break;
    case Z3_L_TRUE:
	{
		result = TRUE;
        m = Z3_solver_get_model(usectx, s);
        if (m)
		{
			Z3_model_inc_ref(usectx, m);
			DEBUG_LOG("constraint is satisfiable with model: \n%s",
				  Z3_model_to_string(usectx, m));
		}
	}
	break;
    }
    if (m)
	{
		Z3_model_dec_ref(usectx, m);
	}

	if(!ctx) // reusing context just pop the assertions
	{
		/* restore scope */
//		Z3_solver_pop(usectx, s, 1);
	}
	else // user provide context del solver
	{
		del_solver(usectx, s);
	}

	return result;
}

/**
   \brief Check whether the logical context is satisfiable, and compare the result with the expected result.
   If the context is satisfiable, then display the model.
*/
void
check(Z3_context ctx, Z3_solver s, Z3_lbool expected_result)
{
    Z3_model m      = 0;
    Z3_lbool result = Z3_solver_check(ctx, s);
    switch (result) {
    case Z3_L_FALSE:
    		DEBUG_LOG("unsat\n");
        break;
    case Z3_L_UNDEF:
        DEBUG_LOG("unknown\n");
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
    if (result != expected_result)
	{
        THROW(SEVERITY_RECOVERABLE, "unexpected result");
    }
    if (m) Z3_model_dec_ref(ctx, m);
}


/*
 * Translates GProM expression into an Z3 AST.
 */
Z3_ast
exprtoz3(Node *n, Z3_context ctx)
{
	char *name;
	List *argLists = NIL;
	Z3_ast c;

	if(isA(n, AttributeReference))
	{
		AttributeReference *a = (AttributeReference *) n;
		switch(a->attrType)
		{
		case DT_BOOL:
			c = mk_bool_var(ctx, a->name);
			break;
		case DT_FLOAT:
			c = mk_float_var(ctx, a->name);
			break;
		case DT_LONG:
		case DT_INT:
			c = mk_int_var(ctx, a->name);
			break;
		case DT_STRING:
		case DT_VARCHAR2:
			c = mk_string_var(ctx, a->name);
			break;
		}
	}
	else if(isA(n, Constant))
	{
		Constant *cst = (Constant *) n;
		switch(cst->constType)
		{
		case DT_INT:
		case DT_LONG:
		{
			c = mk_int(ctx, INT_VALUE(cst));
		}
		break;
		case DT_FLOAT:
		{
			c = mk_float(ctx, FLOAT_VALUE(cst));
		}
		break;
		case DT_STRING:
		case DT_VARCHAR2:
		{
			c = Z3_mk_string(ctx, STRING_VALUE(cst));
		}
		break;
		case DT_BOOL:
		{
			if(BOOL_VALUE(cst))
			{
				c = Z3_mk_true(ctx);
			}
			else
			{
				c = Z3_mk_false(ctx);
			}
		}
		break;
		}
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

		if(streq(name, OPNAME_NOT))
		{
			c = Z3_mk_not(ctx, args[0]);
		}
		else if(streq(name, OPNAME_AND))
		{
			c = Z3_mk_and(ctx, 2, args);
		}
		else if(streq(name, OPNAME_OR))
		{
			c = Z3_mk_or(ctx, 2, args);
		}
		else if(streq(name, OPNAME_ADD))
		{
			c = Z3_mk_add(ctx, 2, args);
		}
		else if(streq(name, OPNAME_MINUS))
		{
			c = Z3_mk_sub(ctx, 2, args);
		}
		else if(streq(name, OPNAME_MULT))
		{
			c = Z3_mk_mul(ctx, 2, args);
		}
		else if(streq(name, OPNAME_DIV))
		{
			c = Z3_mk_div(ctx, args[0], args[1]);
		}
		else if(streq(name, OPNAME_NEQ)
				|| streq(name, OPNAME_NEQ_BANG)
				|| streq(name, OPNAME_NEQ_HAT))
		{
			c = Z3_mk_eq(ctx, args[0], args[1]);
			c = Z3_mk_not(ctx, c);
		}
		else if(streq(name, OPNAME_EQ))
		{
			c = Z3_mk_eq(ctx, args[0], args[1]);
		}
		else if(streq(name, OPNAME_GT))
		{
			c = Z3_mk_gt(ctx, args[0], args[1]);
		}
		else if(streq(name, OPNAME_LT))
		{
			c = Z3_mk_lt(ctx, args[0], args[1]);
		}
		else if(streq(name, OPNAME_LE))
		{
			c = Z3_mk_le(ctx, args[0], args[1]);
		}
		else if(streq(name, OPNAME_GE))
		{
			c = Z3_mk_ge(ctx, args[0], args[1]);
		}
		else //TODO why?
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


// NO libz3 present. Provide dummy methods to keep compiler quiet
#else
void
testp()
{

}

void *
mk_context()
{
	return NULL;
}

void *
exprtoz3(Node *n, void *ctx)
{
	return NULL;
}

#endif
