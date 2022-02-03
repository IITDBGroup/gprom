/*
 *------------------------------------------------------------------------------
 *
 * expr_to_z3.h - translate GProM expressions to constraints and check their satisfiability with Z3.
 *
 *
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2020-12-05
 *        SUBDIR: include/symbolic_eval/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _Z3_SOLVER_H_
#define _Z3_SOLVER_H_

#include "model/query_operator/query_operator.h"

/* **************************************** */
#if HAVE_Z3
#include "z3.h"

// utility functions
extern void display_version();
extern Z3_context mk_context();
extern Z3_solver mk_solver(Z3_context ctx);
extern Z3_context mk_context_custom(Z3_config cfg, Z3_error_handler err);
extern void del_solver(Z3_context ctx, Z3_solver s);
extern void createSolver(void);
extern void destroySolver(void);

// translate expression to Z3 internal contraint format
extern Z3_ast exprtoz3(Node *n, Z3_context ctx);

// checking satisfiability and getting models
extern void check(Z3_context ctx, Z3_solver s, Z3_lbool expected_result);
extern boolean z3IsSatisfiable(Z3_context ctx, Z3_ast constraints, boolean exceptionOnUndef);
extern boolean z3ExprIsSatisfiable(Node *expr, boolean exceptionOnUndef);
extern boolean z3ExprIsValid(Node *expr, boolean exceptionOnUndef);
extern void testp(); //remove later

// functions to create constraint elements
extern Z3_ast mk_var(Z3_context ctx, const char * name, Z3_sort ty);
extern Z3_ast mk_int_var(Z3_context ctx, const char * name);
extern Z3_ast mk_int(Z3_context ctx, int v);
extern Z3_ast mk_bool_var(Z3_context ctx, const char * name);
extern Z3_ast mk_float_var(Z3_context ctx, const char * name);
extern Z3_ast mk_float(Z3_context ctx, float v);
extern Z3_ast mk_string_var(Z3_context ctx, const char *name);

/* **************************************** */
#else // keep compiler quiet
extern void *mk_context();
extern void *exprtoz3(Node *n, void *ctx);
#endif


#endif /* _Z3_SOLVER_H_ */
