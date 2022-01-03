/*-----------------------------------------------------------------------------
 *
 * expr_to_constraint.c
 *
 *
 *		AUTHOR: Bahareh
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

// only include cplex headers if the library is available
#ifdef HAVE_LIBCPLEX
#include <ilcplex/cplex.h>
#endif

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
#include "model/set/hashmap.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"

#ifdef HAVE_LIBCPLEX

static CplexObjects *cplexObjects = NULL; // global pointer to current cplex objects
static int objectIndex = 0;
static double default_lb = 0;
static double default_ub = CPX_MAX;

static void setCplexObjects(char *tbName);
static int getObjectIndex(char *attrName);
static List *getOpExpStack(List *stackList, Operator *opExpList);
static int setToConstr(List *attrList, Node * query, CPXENVptr env, CPXLPptr lp);
static int condToConstr(Node *cond, CPXENVptr env, CPXLPptr lp);
static int invertCondToConstr(Update *f, CPXENVptr env, CPXLPptr lp);
static int exprToEval(Node *expr, boolean invert, CPXENVptr env, CPXLPptr lp);

static void setCplexObjects(char *tbName) {
	if (cplexObjects == NULL) {
		cplexObjects = NEW(CplexObjects);
		cplexObjects->tableName = tbName;
		cplexObjects->attrIndex = NEW_MAP(Constant, Constant);
		cplexObjects->obj = (double *) MALLOC(
				DEFAULT_NUM_COLS * sizeof(double));
		cplexObjects->lb = (double *) MALLOC(DEFAULT_NUM_COLS * sizeof(double));
		cplexObjects->ub = (double *) MALLOC(DEFAULT_NUM_COLS * sizeof(double));
		cplexObjects->colname = (char **) MALLOC(
				DEFAULT_NUM_COLS * sizeof(char*));
	}
}

static int getObjectIndex(char *attrName) {

	int index = 0;

	if (!MAP_HAS_STRING_KEY(cplexObjects->attrIndex, attrName)) {
		MAP_ADD_STRING_KEY(cplexObjects->attrIndex, attrName,
				createConstInt(objectIndex));
		cplexObjects->colname[objectIndex] = attrName;
		cplexObjects->lb[objectIndex] = default_lb;
		cplexObjects->ub[objectIndex] = default_ub;
		cplexObjects->obj[objectIndex] = 1;
		index = objectIndex;
		objectIndex++;
	} else {
		index = INT_VALUE(MAP_GET_STRING(cplexObjects->attrIndex,attrName));
	}
	return index;
}

static List *
getOpExpStack(List *stackList, Operator *opExpList) {

	stackList = appendToTailOfList(stackList, opExpList);
	Node *left = (Node *) getHeadOfListP(opExpList->args);
	Node *right = (Node *) getTailOfListP(opExpList->args);

	if (isA(left, Operator)) {
		stackList = getOpExpStack(stackList, (Operator *) left);
	} else {
		stackList = appendToTailOfList(stackList, left);
	}

	if (isA(right, Operator)) {
		stackList = getOpExpStack(stackList, (Operator *) right);
	} else {
		stackList = appendToTailOfList(stackList, right);
	}

	return stackList;

}

static int setToConstr(List *attrList, Node *query, CPXENVptr env, CPXLPptr lp) {

	int status = 0;

	if (isA(query, List)) {

		int numCols = 0;
		FOREACH(Constant,c,(List *)query)
		{
			if (c->constType == DT_INT || c->constType == DT_FLOAT
					|| c->constType == DT_LONG) {
				numCols++;
			}
		}

		int numRows = numCols;
		int numZ = numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];

		int i = 0, j = 0;
		char *attr;

		FOREACH(Constant,c,(List *)query)
		{
			if (c->constType == DT_INT || c->constType == DT_FLOAT
					|| c->constType == DT_LONG) {
				rmatbeg[i] = i;
				rowname[i] = "row";
				attr = (char *) getNthOfListP(attrList, j);
				rmatind[i] = getObjectIndex(attr);
				rmatval[i] = 1.0;
				sense[i] = 'E';
				rhs[i] = LONG_VALUE(c);
				//rhs[i] = (double)(((Constant) c)->value);
				i++;
			}
			j++;
		}

		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);
	}

	return status;
}

static int condToConstr(Node *cond, CPXENVptr env, CPXLPptr lp) {

	int status = 0;
	List *result = NIL;
	result = getAttrNameFromOpExpList(result, (Operator *) cond);

	int numCols = getListLength(result);

	int objIndex = 0;
	int numRows = 1;
	int numZ = numRows * numCols;
	int rmatbeg[numRows];
	double rhs[numRows];
	char sense[numRows];
	char *rowname[numRows];
	int rmatind[numZ];
	double rmatval[numZ];

	rmatbeg[0] = 0;
	rowname[0] = "row1";

	char *opName = ((Operator *) cond)->name;

	if (strcmp(opName, OPNAME_EQ) == 0) {
		sense[0] = 'E';
	} else if (strcmp(opName, ">") == 0 || strcmp(opName, ">=") == 0) {
		sense[0] = 'G';
	} else if (strcmp(opName, OPNAME_LT) == 0 || strcmp(opName, "<=") == 0) {
		sense[0] = 'L';
	}

	Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
	Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
	rhs[0] = LONG_VALUE(right);
	//if the condition likes c>10
	if (isA(left, AttributeReference)) {
		//we have just one attribute
		objIndex = getObjectIndex(((AttributeReference *) left)->name);
		rmatind[0] = objIndex;
		rmatval[0] = 1.0;
	}
	//else every attr should has a coefficient for example 1*a+2*b>10
	else if (isA(left, Operator)) {
		int i = 0;
		int pluse = 1;
		List *stack = NIL;
		stack = getOpExpStack(stack, (Operator *) left);
		int last = getListLength(stack) - 1;
		Node *sign, *r, *l;
		while (last > i) {
			pluse = 1;
			r = (Node *) getNthOfListP(stack, last--);
			l = (Node *) getNthOfListP(stack, last--);
			//op = (Node *) getNthOfListP(stack, last--);
			if (last > i) {
				sign = (Node *) getNthOfListP(stack, i);
				if (streq(((Operator *) sign)->name, "-"))
					pluse = -1;
			}

			//operator name should be * based on the creation rule
			//if (streq(((Operator *) op)->name, "*"))
			if (isA(r, AttributeReference) && isA(l, Constant)) {
				objIndex = getObjectIndex(((AttributeReference *) r)->name);
				rmatind[i] = objIndex;
				rmatval[i] = LONG_VALUE(l) * pluse;
			} else if (isA(l, AttributeReference) && isA(r, Constant)) {
				objIndex = getObjectIndex(((AttributeReference *) l)->name);
				rmatind[i] = objIndex;
				rmatval[i] = LONG_VALUE(r) * pluse;

			}
			i++;
			//}
		}
	}
	/*
	 //cond OR and AND wasn't considered
	 else if(streq(o->name, OPNAME_AND) || streq(o->name, OPNAME_OR))
	 {

	 return;
	 }
	 */

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	return status;

}

//to be done
static int invertCondToConstr(Update *f, CPXENVptr env, CPXLPptr lp) {
	int status = 0;
	boolean isFound = FALSE;
	List *selAttr = NIL;
	List *conAttr = NIL;
	selAttr = getAttrNameFromOpExpList(selAttr, (Operator *) f->selectClause);
	conAttr = getAttrNameFromOpExpList(conAttr, (Operator *) f->cond);
	FOREACH(char,attr,conAttr)
	{
		if (searchListString(selAttr, attr)) {
			isFound = TRUE;
			break;
		}
	}

	if (isFound == FALSE) {
		return condToConstr(((Update *) f)->cond, env, lp);
	}
	//to be done
	else {
		return status;
	}
}

static int exprToEval(Node *expr, boolean invert, CPXENVptr env, CPXLPptr lp) {
	int status = 0;

	switch (expr->type) {
	case T_Update:
		setCplexObjects(((Update *) expr)->updateTableName);
		if (!invert)
			status = condToConstr(((Update *) expr)->cond, env, lp);
		else
			status = invertCondToConstr((Update *) expr, env, lp);
		break;
	case T_Delete:
		setCplexObjects(((Delete *) expr)->deleteTableName);
		status = condToConstr(((Delete *) expr)->cond, env, lp);
		break;
	case T_Insert:
		setCplexObjects(((Insert *) expr)->insertTableName);
		status = setToConstr(((Insert *) expr)->attrList,
				((Insert *) expr)->query, env, lp);
		break;
	default:
		break;
	}
	return status;
}

boolean
exprToSat(Node *expr1, boolean inv1, Node *expr2, boolean inv2)
{
	int status = 0;
	boolean result = FALSE;
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
	status = exprToEval(expr1, inv1, env, lp);

	if (status) {
		ERROR_LOG("Failed to populate problem.\n");
	}

	status = exprToEval(expr2, inv2, env, lp);

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

	x = (double *) MALLOC(cur_numcols * sizeof(double));
	slack = (double *) MALLOC(cur_numrows * sizeof(double));
	dj = (double *) MALLOC(cur_numcols * sizeof(double));
	pi = (double *) MALLOC(cur_numrows * sizeof(double));

	if (x == NULL || slack == NULL || dj == NULL || pi == NULL) {
		ERROR_LOG("Could not allocate memory for solution.\n");
	}

	status = CPXsolution(env, lp, &solstat, &objval, x, pi, slack, dj);
	if (status) {
		DEBUG_LOG("False: Failed to obtain solution.\n");
		result = FALSE;

	} else {
		DEBUG_LOG("True: Obtained solution.\n");
		result = TRUE;
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
	}

	return result;
}

// ********************************************************************************
// dummy replacement if cplex is not available
#else

boolean
exprToSat(Node *expr1, boolean inv1, Node *expr2, boolean inv2)
{
    return TRUE;
}

#endif
