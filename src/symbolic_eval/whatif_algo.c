/*-----------------------------------------------------------------------------
 *
 * whatif_algo.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */
#include <ilcplex/cplex.h>
#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "parser/parser.h"
#include "symbolic_eval/expr_to_constraint.h"
#include "symbolic_eval/whatif_algo.h"
#include "model/set/hashmap.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"

static List *cond = NIL; // global pointer to the list of conditions
static List *tables = NIL; // global pointer to the list of tables
static List *depend = NIL; // global pointer to the list of dependent updates

static int exprToProblem(List *expr);
static void free_and_null(char **ptr);

static int exprToProblem(List *expr) {
	int status = 0;
	int solstat;
	double objval;
	double *x = NULL;
	double *pi = NULL;
	double *slack = NULL;
	double *dj = NULL;
	int cur_numrows, cur_numcols;

	CPXENVptr env = NULL;
	CPXLPptr lp = NULL;

	/* Initialize the CPLEX environment */
	if (env == NULL) {
		char errmsg[CPXMESSAGEBUFSIZE];
		CPXgeterrorstring(env, status, errmsg);
		ERROR_LOG("Could not open CPLEX environment.\n%s", errmsg);
	}

	env = CPXopenCPLEX(&status);
	/* Turn on output to the screen */

	status = CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
	if (status) {
		ERROR_LOG("Failure to turn on screen indicator, error %d.\n", status);
	}

	/* Turn on data checking */

	status = CPXsetintparam(env, CPXPARAM_Read_DataCheck,
			CPX_DATACHECK_WARN);
	if (status) {
		ERROR_LOG("Failure to turn on data checking, error %d.\n", status);
	}

	/* Create the problem. */

	lp = CPXcreateprob(env, &status, "lpex1");

	/* A returned pointer of NULL may mean that not enough memory
	 was available or there was some other problem.  In the case of
	 failure, an error message will have been written to the error
	 channel from inside CPLEX.  In this example, the setting of
	 the parameter CPXPARAM_ScreenOutput causes the error message to
	 appear on stdout.  */

	if (lp == NULL) {
		ERROR_LOG("Failed to create LP.\n");
	}

	/* Now populate the problem with the data.  For building large
	 problems, consider setting the row, column and nonzero growth
	 parameters before performing this task. */
	//status = populatebyrow(env, lp);
	//to be done
	FOREACH(Node,n,expr)
	{
		status = exprToEval(n, env, lp);
	}

	if (status) {
		ERROR_LOG("Failed to populate problem.\n");
	}

	/* Optimize the problem and obtain solution. */

	status = CPXlpopt(env, lp);
	if (status) {
		ERROR_LOG("Failed to optimize LP.\n");
	}

	/* The size of the problem should be obtained by asking CPLEX what
 the actual size is, rather than using sizes from when the problem
 was built.  cur_numrows and cur_numcols store the current number
 of rows and columns, respectively.  */

	cur_numrows = CPXgetnumrows(env, lp);
	cur_numcols = CPXgetnumcols(env, lp);

	x = (double *) malloc(cur_numcols * sizeof(double))
		;
	slack = (double *) malloc(cur_numrows * sizeof(double))
		;
	dj = (double *) malloc(cur_numcols * sizeof(double))
		;
	pi = (double *) malloc(cur_numrows * sizeof(double))
		;

	if (x == NULL || slack == NULL || dj == NULL || pi == NULL) {
		status = CPXERR_NO_MEMORY;
		ERROR_LOG("Could not allocate memory for solution.\n");
	}

	status = CPXsolution(env, lp, &solstat, &objval, x, pi, slack, dj);
	if (status) {
		DEBUG_LOG("False: Failed to obtain solution.\n");

	} else {
		DEBUG_LOG("True: Obtained solution.\n");
	}

	//TERMINATE:

	/* Free up the problem as allocated by CPXcreateprob, if necessary */

	if (lp != NULL) {
		status = CPXfreeprob(env, &lp);
		if (status) {
			DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
		}
	}

	/* Free up the CPLEX environment, if necessary */

	if (env != NULL) {
		status = CPXcloseCPLEX(&env);

		/* Note that CPXcloseCPLEX produces no output,
	 so the only way to see the cause of the error is to use
	 CPXgeterrorstring.  For other CPLEX routines, the errors will
	 be seen if the CPXPARAM_ScreenOutput indicator is set to CPX_ON. */

		if (status) {
			DEBUG_LOG("Could not close CPLEX environment.\n");
		}

		return status;
	}

	static void free_and_null(char **ptr) {
		if (*ptr != NULL) {
			free(*ptr);
			*ptr = NULL;
		}
	}
