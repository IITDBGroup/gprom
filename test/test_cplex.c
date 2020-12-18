/*-----------------------------------------------------------------------------
 *
 * test_cplex.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_LIBCPLEX
#include <ilcplex/cplex.h>
#endif
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
//#include "common.h"
/* Shortcut for infinity. */
#ifdef HAVE_LIBCPLEX

#define INF CPX_INFBOUND

static int populatebyrow(CPXENVptr env, CPXLPptr lp);
static void free_and_null(char **ptr);

int
main(int argc, char *argv[])
{
    int solstat;
    double objval;
    double *x = NULL;
    double *pi = NULL;
    double *slack = NULL;
    double *dj = NULL;

    CPXENVptr env = NULL;
    CPXLPptr lp = NULL;
    int status = 0;
    int cur_numrows, cur_numcols;

    /* Initialize the CPLEX environment */

    env = CPXopenCPLEX(&status);

    /* If an error occurs, the status value indicates the reason for
     failure.  A call to CPXgeterrorstring will produce the text of
     the error message.  Note that CPXopenCPLEX produces no output,
     so the only way to see the cause of the error is to use
     CPXgeterrorstring.  For other CPLEX routines, the errors will
     be seen if the CPXPARAM_ScreenOutput indicator is set to CPX_ON.  */

    if (env == NULL) {
        char errmsg[CPXMESSAGEBUFSIZE];
        fprintf(stderr, "Could not open CPLEX environment.\n");
        CPXgeterrorstring(env, status, errmsg);
        fprintf(stderr, "%s", errmsg);
        goto TERMINATE;
    }

    /* Turn on output to the screen */

    status = CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
    if (status) {
        fprintf(stderr, "Failure to turn on screen indicator, error %d.\n",
                status);
        goto TERMINATE;
    }

    /* Turn on data checking */

    status = CPXsetintparam(env, CPXPARAM_Read_DataCheck,
    CPX_DATACHECK_WARN);
    if (status) {
        fprintf(stderr, "Failure to turn on data checking, error %d.\n",
                status);
        goto TERMINATE;
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
        fprintf(stderr, "Failed to create LP.\n");
        goto TERMINATE;
    }

    /* Now populate the problem with the data.  For building large
     problems, consider setting the row, column and nonzero growth
     parameters before performing this task. */

    status = populatebyrow(env, lp);

    if (status) {
        fprintf(stderr, "Failed to populate problem.\n");
        goto TERMINATE;
    }

    /* Optimize the problem and obtain solution. */

    status = CPXlpopt(env, lp);
    if (status) {
        fprintf(stderr, "Failed to optimize LP.\n");
        goto TERMINATE;
    }

    /* The size of the problem should be obtained by asking CPLEX what
     the actual size is, rather than using sizes from when the problem
     was built.  cur_numrows and cur_numcols store the current number
     of rows and columns, respectively.  */

    cur_numrows = CPXgetnumrows(env, lp);
    cur_numcols = CPXgetnumcols(env, lp);

    x = (double *) malloc(cur_numcols * sizeof(double));
    slack = (double *) malloc(cur_numrows * sizeof(double));
    dj = (double *) malloc(cur_numcols * sizeof(double));
    pi = (double *) malloc(cur_numrows * sizeof(double));

    if (x == NULL || slack == NULL || dj == NULL || pi == NULL) {
        status = CPXERR_NO_MEMORY;
        fprintf(stderr, "Could not allocate memory for solution.\n");
        goto TERMINATE;
    }

    status = CPXsolution(env, lp, &solstat, &objval, x, pi, slack, dj);
    if (status) {
        fprintf(stderr, "False: Failed to obtain solution.\n");
        goto TERMINATE;
    } else {
        fprintf(stderr, "True: Obtained solution.\n");
        goto TERMINATE;
    }

    TERMINATE:

    /* Free up the problem as allocated by CPXcreateprob, if necessary */

    if (lp != NULL) {
        status = CPXfreeprob(env, &lp);
        if (status) {
            fprintf(stderr, "CPXfreeprob failed, error code %d.\n", status);
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
            //char errmsg[CPXMESSAGEBUFSIZE];
            fprintf(stderr, "Could not close CPLEX environment.\n");
            //CPXgeterrorstring(env, status, errmsg);
            //fprintf(stderr, "%s", errmsg);
        }
    }

    return (status);

} /* END main */

/* This simple routine frees up the pointer *ptr, and sets *ptr to NULL */

static void free_and_null(char **ptr) {
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
} /* END free_and_null */

#define NUMROWS    1
#define NUMCOLS    3
#define NUMNZ      3

/* To populate by row, we first create the columns, and then add the
 rows.  */

static int populatebyrow(CPXENVptr env, CPXLPptr lp) {

    //here we should create constraints base on the parser output

    /* These example populate the problem with data for the following
       linear program:

          Maximize
           obj: x1 + x2 + x3
          Subject To
           c1: x1 + x2 + x3 <= -20
          Bounds
           0 <= x1 <= 40
           0 <= x1 <= 10
           0 <= x1 <= 20
          End
     */

    int status = 0;
    double obj[NUMCOLS];
    double lb[NUMCOLS];
    double ub[NUMCOLS];
    char *colname[NUMCOLS];
    int rmatbeg[NUMROWS];
    int rmatind[NUMNZ];
    double rmatval[NUMNZ];
    double rhs[NUMROWS];
    char sense[NUMROWS];
    char *rowname[NUMROWS];

    status = CPXchgobjsen(env, lp, CPX_MAX); /* Problem is maximization */
    if (status)
        goto TERMINATE;

    /* Now create the new columns.  First, populate the arrays. */

    obj[0] = 1.0;
    obj[1] = 1.0;
    obj[2] = 1.0;

    lb[0] = 0.0;
    lb[1] = 0.0;
    lb[2] = 0.0;
    ub[0] = 40.0;
    ub[1] = 10;
    ub[2] = 20;

    colname[0] = "x1";
    colname[1] = "x2";
    colname[2] = "x3";

    status = CPXnewcols(env, lp, NUMCOLS, obj, lb, ub, NULL, colname);
    if (status)
        goto TERMINATE;

    /* Now add the constraints.  */

    rmatbeg[0] = 0;
    rowname[0] = "c1";

    rmatind[0] = 0;
    rmatind[1] = 1;
    rmatind[2] = 2;
    sense[0] = 'L';
    rmatval[0] = 1.0;
    rmatval[1] = 1.0;
    rmatval[2] = 1.0;
    rhs[0] = -20.0;

    status = CPXaddrows(env, lp, 0, NUMROWS, NUMNZ, rhs, sense, rmatbeg,
            rmatind, rmatval, NULL, rowname);

    if (status)
        goto TERMINATE;

    TERMINATE:

    return (status);

} /* END populatebyrow */
#else
int
main(int argc, char *argv[])
{
    return EXIT_SUCCESS;
}
#endif

