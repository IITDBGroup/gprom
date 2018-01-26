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
/*
 // only include cplex headers if the library is available
 #ifdef HAVE_LIBCPLEX
 #include <ilcplex/cplex.h>
 #endif
 */
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
#include "model/set/set.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"

static int totalObjects = 0;
static int totalAttr = 0;
static Set *attrSet = NULL;
static double default_lb = 0;
static double default_ub = CPX_INFBOUND;
//static const int default_num_cols=100;
//#define default_num_cols 100
//enum {default_num_cols = 100 };
static int default_num_cols = 100;

//#ifdef HAVE_LIBCPLEX
/*
 typedef struct CplexObjects {
 char *tableName;
 HashMap *attrIndex;       // hashmap attributename -> attribute index in obj
 double obj[];
 double lb[];
 double ub[];
 char *colname[];
 } CplexObjects;

 static CplexObjects *cplexObjects = NULL; // global pointer to current cplex objects
 */

//static char *tableName;
static HashMap *attrIndex;    // hashmap attributename -> attribute index in obj
static double *obj;
static double *lb;
static double *ub;
static char **colname;

static void setCplexObjects(Node *expr);
static int getObjectIndex(char *attrName);
static List *getOpExpStack(List *stackList, Operator *opExpList);
static double constrToDouble(Constant *cons);
static int setToConstr(Node * query, CPXENVptr env, CPXLPptr lp);
static int condToConstr(Node *cond, CPXENVptr env, CPXLPptr lp);
static int invertCondToConstr(Node *cond, List *selectClause, CPXENVptr env,
		CPXLPptr lp);
static int exprToEval(Node *expr, boolean invert, CPXENVptr env, CPXLPptr lp);
static void setSymbolicObjects(Node *expr, int numUp);
static int createU(int upNum, SelectItem *s, CPXENVptr env, CPXLPptr lp);
static int createV(int upNum, char *attr, CPXENVptr env, CPXLPptr lp);
static int setAttUV(int upNum, char *attr, CPXENVptr env, CPXLPptr lp);
static int setNexSt(int upNum, Set *aSet, CPXENVptr env, CPXLPptr lp);
static int condToSt(int upNum, Node *cond, CPXENVptr env, CPXLPptr lp);
static int exprToSymbol(int upNum, Node *expr, CPXENVptr env, CPXLPptr lp);

static void setCplexObjects(Node *expr) {
	char *tbName = NULL;
	switch (expr->type) {
	case T_Update:
		tbName = ((Update *) expr)->updateTableName;
		break;
	case T_Delete:
		tbName = ((Delete *) expr)->deleteTableName;
		break;
	case T_Insert:
		tbName = ((Insert *) expr)->insertTableName;
		break;
	default:
		break;
	}

	List *schema = NIL;
	schema = getAttributeNames(tbName);
	if (schema != NIL)
		default_num_cols = getListLength(schema);

	//if (cplexObjects == NULL) {
	//cplexObjects = NEW(CplexObjects);
	/*	cplexObjects = MALLOC(
	 sizeof(struct CplexObjects)
	 + 3 * (sizeof(double) * default_num_cols)
	 + (sizeof(char *) * default_num_cols));
	 attrIndex = NEW_MAP(Constant, Constant);
	 */
	attrIndex = NEW_MAP(Constant, Constant);
	obj = (double *) MALLOC(sizeof(double) * default_num_cols);
	lb = (double *) MALLOC(sizeof(double) * default_num_cols);
	ub = (double *) MALLOC(sizeof(double) * default_num_cols);
	colname = (char **) MALLOC(sizeof(char *) * default_num_cols);

	DEBUG_LOG("working setCplexObjects.\n");

	//tableName = tbName;

	FOREACH(char,a,schema)
	{
		MAP_ADD_STRING_KEY(attrIndex, a, createConstInt(totalObjects));
		colname[totalObjects] = a;
		lb[totalObjects] = default_lb;
		ub[totalObjects] = default_ub;
		obj[totalObjects] = 1;
		totalObjects++;
	}
	DEBUG_LOG(" number of cplex columns %d.\n", totalObjects);

	totalAttr = totalObjects;
//}
}

static int getObjectIndex(char *attrName) {
	return INT_VALUE(MAP_GET_STRING(attrIndex,(char *)attrName));
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

static double constrToDouble(Constant *cons) {

	switch (cons->constType) {
	case DT_INT:
		return (double) INT_VALUE(cons);
	case DT_FLOAT:
		return (double) FLOAT_VALUE(cons);
	case DT_BOOL:
		return (double) BOOL_VALUE(cons);
	case DT_LONG:
		return (double) LONG_VALUE(cons);
	case DT_VARCHAR2:
	case DT_STRING:
		return 0;
	}
	return 0;
}

static int setToConstr(Node *query, CPXENVptr env, CPXLPptr lp) {
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

		FOREACH(Constant,c,(List *)query)
		{
			if (c->constType == DT_INT || c->constType == DT_FLOAT
					|| c->constType == DT_LONG) {
				rmatbeg[i] = i;
				rowname[i] = "row";
				rmatind[i] = j;
				rmatval[i] = 1.0;
				sense[i] = 'E';
				rhs[i] = constrToDouble((Constant *) c);
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

	char *opName = ((Operator *) cond)->name;
	if (strcmp(opName, "AND") == 0) {
		DEBUG_LOG("we have AND Operator");
		condToConstr((Node *) getHeadOfListP(((Operator *) cond)->args), env,
				lp);
		condToConstr((Node *) getTailOfListP(((Operator *) cond)->args), env,
				lp);
	} else {

		List *result = NIL;
		result = getAttrNameFromOpExpList(result, (Operator *) cond);

		int numCols = getListLength(result);
		int index = 0;
		int numRows = 1;
		int numZ = numRows * numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];

		rmatbeg[0] = 0;
		rowname[0] = "row";

		if (strcmp(opName, "=") == 0) {
			sense[0] = 'E';
		} else if (strcmp(opName, ">") == 0 || strcmp(opName, ">=") == 0) {
			sense[0] = 'G';
		} else if (strcmp(opName, "<") == 0 || strcmp(opName, "<=") == 0) {
			sense[0] = 'L';
		}

		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
		rhs[0] = constrToDouble((Constant *) right);
		DEBUG_LOG("Processing Value =  %10f \n", rhs[0]);
		//if the condition likes c>10
		if (isA(left, AttributeReference)) {
			//we have just one attribute
			index = getObjectIndex(((AttributeReference *) left)->name);
			rmatind[0] = index;
			rmatval[0] = 1.0;
		}
		//if the condition likes c>b => c-b>0
		else if (isA(right, AttributeReference)) {
			index = getObjectIndex(((AttributeReference *) right)->name);
			rmatind[1] = index;
			rmatval[1] = -1.0;
			rhs[0] = 0;
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
					index = getObjectIndex(((AttributeReference *) r)->name);
					rmatind[i] = index;
					rmatval[i] = constrToDouble((Constant *) l) * pluse;
				} else if (isA(l, AttributeReference) && isA(r, Constant)) {
					index = getObjectIndex(((AttributeReference *) l)->name);
					rmatind[i] = index;
					rmatval[i] = constrToDouble((Constant *) r) * pluse;

				}
				i++;
			}
		}

		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		if (status) {
			ERROR_LOG(
					"Failure to convert update and add a row to cplex problem %d.\n",
					status);
		}
	}

	return status;

}

static int invertCondToConstr(Node *cond, List *selectClause, CPXENVptr env,
		CPXLPptr lp) {
	int status = 0;
	char *opName = ((Operator *) cond)->name;
	if (strcmp(opName, "AND") == 0) {
		invertCondToConstr((Node *) getHeadOfListP(((Operator *) cond)->args),
				selectClause, env, lp);
		invertCondToConstr((Node *) getTailOfListP(((Operator *) cond)->args),
				selectClause, env, lp);
	} else {
		boolean isFound = FALSE;
		List *selAttr = NIL;
		List *conAttr = NIL;

		conAttr = getAttrNameFromOpExpList(conAttr, (Operator *) cond);
		Operator *selOpt = createOpExpr("sel", selectClause);
		selAttr = getAttrNameFromOpExpList(selAttr, selOpt);

		FOREACH(char,attr,conAttr)
		{
			if (searchListString(selAttr, attr)) {
				isFound = TRUE;
				break;
			}
		}
		if (!isFound)
			return condToConstr(cond, env, lp);
		else {
			int numCols = getListLength(conAttr);
			int index = 0;
			int numRows = 1;
			int numZ = numRows * numCols;
			int rmatbeg[numRows];
			double rhs[numRows];
			char sense[numRows];
			char *rowname[numRows];
			int rmatind[numZ];
			double rmatval[numZ];

			rmatbeg[0] = 0;
			rowname[0] = "rowInv";

			if (strcmp(opName, "=") == 0) {
				sense[0] = 'E';
			} else if (strcmp(opName, ">") == 0 || strcmp(opName, ">=") == 0) {
				sense[0] = 'G';
			} else if (strcmp(opName, "<") == 0 || strcmp(opName, "<=") == 0) {
				sense[0] = 'L';
			}

			Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
			Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
			char *attrName = ((AttributeReference *) left)->name;

			//if the condition likes c>10
			if (isA(left, AttributeReference)) {
				index = getObjectIndex(attrName);
				rmatind[0] = index;
				rmatval[0] = 1.0;
				double condVal = constrToDouble((Constant *) right);

				Node *projExpr = NULL;
				Node *selectAttr = NULL;
				char *selectAttName;

				FOREACH(SelectItem, s, selectClause)
				{
					selectAttr = getHeadOfListP((List *) s->expr);
					selectAttName = ((AttributeReference *) selectAttr)->name;
					if (strcmp(selectAttName, attrName) == 0) {
						projExpr = copyObject(
								(Node *) getTailOfListP((List *) s->expr));
						break;
					}
				}

				if (projExpr != NULL) {
					Node *invertNode = (Node *) getTailOfListP(
							((Operator *) projExpr)->args);
					double invertVal = constrToDouble((Constant *) invertNode);

					char *op = ((Operator *) projExpr)->name;
					if (strcmp(op, "+") == 0) {
						rhs[0] = condVal + invertVal;
					} else if (strcmp(op, "-") == 0) {
						rhs[0] = condVal - invertVal;
					} else if (strcmp(op, "/") == 0) {
						rhs[0] = condVal / invertVal;
					} else if (strcmp(op, "*") == 0) {
						rhs[0] = condVal * invertVal;
					}
					/*
					 else if (strcmp(op, "%") == 0) {
					 rhs[0] = condVal % invertVal;
					 }
					 */

				} else
					rhs[0] = condVal;
				DEBUG_LOG("Processing Invert Condition Value =  %10f \n",
						rhs[0]);

				status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense,
						rmatbeg, rmatind, rmatval, NULL, rowname);

				if (status) {
					ERROR_LOG(
							"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
							status);
				}
			}
		}
	}

	return status;
}

static int exprToEval(Node *expr, boolean invert, CPXENVptr env, CPXLPptr lp) {
	int status = 0;

	switch (expr->type) {
	case T_Update:
		if (!invert)
			status = condToConstr(((Update *) expr)->cond, env, lp);
		else
			status = invertCondToConstr(((Update *) expr)->cond,
					((Update *) expr)->selectClause, env, lp);
		break;
	case T_Delete:
		status = condToConstr(((Delete *) expr)->cond, env, lp);
		break;
	case T_Insert:
		status = setToConstr(((Insert *) expr)->query, env, lp);
		break;
	default:
		break;
	}
	return status;
}

boolean exprToSat(Node *expr1, boolean inv1, Node *expr2, boolean inv2) {
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
	env = CPXopenCPLEX(&status);

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

	status = CPXsetintparam(env, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN);
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

	setCplexObjects(expr1);

	status = CPXnewcols(env, lp, totalObjects, obj, lb, ub, NULL, colname);
	if (status) {
		ERROR_LOG("Failure to create cplex columns %d.\n", status);
	}

	/* Now populate the problem with the data.  For building large
	 problems, consider setting the row, column and nonzero growth
	 parameters before performing this task. */
	status = exprToEval(expr1, inv1, env, lp);

	if (status) {
		ERROR_LOG("Failed to populate problem for the first expression.\n");
	}

	status = exprToEval(expr2, inv2, env, lp);

	if (status) {
		ERROR_LOG("Failed to populate problem for the second expression.\n");
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

//check solution result

	DEBUG_LOG("\nSolution status = %d\n", solstat);
	DEBUG_LOG("Solution value  = %f\n\n", objval);

	int j;
	for (j = 0; j < cur_numcols; j++) {
		DEBUG_LOG("Column %d:  Value =  %10f  Reduced cost = %10f\n", j, x[j],
				dj[j]);
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
//////////////////////////////////////////
//Implement Symbolic Execution
/////////////////////////////////////////
static void setSymbolicObjects(Node *expr, int numUp) {
	/* if (cplexObjects == NULL) {
	 cplexObjects = NEW(CplexObjects);
	 attrIndex = NEW_MAP(Constant, Constant);
	 */
	attrIndex = NEW_MAP(Constant, Constant);
	DEBUG_LOG("working setCplexObjects for %d updates.\n", numUp);

	char *tbName = NULL;
	char *cName = NULL;

	switch (expr->type) {
	case T_Update:
		tbName = ((Update *) expr)->updateTableName;
		break;
	case T_Delete:
		tbName = ((Delete *) expr)->deleteTableName;
		break;
	case T_Insert:
		tbName = ((Insert *) expr)->insertTableName;
		break;
	default:
		break;
	}
	//tableName = tbName;
	List *schema = NIL;
	schema = getAttributeNames(tbName);
	int numAttr = getListLength(schema);
	attrSet = makeStrSetFromList(schema);

	//for each update, we need new states ai for each update, u,v for each attribute and update and x for each update
	default_num_cols = ((1 + numUp) * numAttr) + (2 * numUp * numAttr) + numUp;
	attrIndex = NEW_MAP(Constant, Constant);
	obj = (double *) MALLOC(sizeof(double) * default_num_cols);
	lb = (double *) MALLOC(sizeof(double) * default_num_cols);
	ub = (double *) MALLOC(sizeof(double) * default_num_cols);
	colname = (char **) MALLOC(sizeof(char *) * default_num_cols);
	for (int i = 0; i <= numUp; i++) {
		FOREACH(char,a,schema)
		{
			cName = CONCAT_STRINGS(a, "_", itoa(i));
			MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
			colname[totalObjects] = cName;
			lb[totalObjects] = default_lb;
			ub[totalObjects] = default_ub;
			obj[totalObjects] = 1;
			totalObjects++;
		}
	}

	for (int i = 1; i <= numUp; i++) {
		FOREACH(char,a,schema)
		{
			cName = CONCAT_STRINGS("u_", a, "_", itoa(i));
			MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
			colname[totalObjects] = cName;
			lb[totalObjects] = default_lb;
			ub[totalObjects] = default_ub;
			obj[totalObjects] = 1;
			totalObjects++;

			cName = CONCAT_STRINGS("v_", a, "_", itoa(i));
			MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
			colname[totalObjects] = cName;
			lb[totalObjects] = default_lb;
			ub[totalObjects] = default_ub;
			obj[totalObjects] = 1;
			totalObjects++;
		}

		cName = CONCAT_STRINGS("x_", itoa(i));
		MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
		colname[totalObjects] = cName;
		lb[totalObjects] = 0;
		ub[totalObjects] = 1;
		obj[totalObjects] = 1;
		totalObjects++;
	}

	DEBUG_LOG("number of symbolic cplex columns %d.\n", totalObjects);
}

static int createU(int upNum, SelectItem *s, CPXENVptr env, CPXLPptr lp) {

	/*
	 List *selAttr = NIL;
	 Operator *selOpt = createOpExpr("sel", selectClause);
	 selAttr = getAttrNameFromOpExpList(selAttr, selOpt);
	 */
	int status = 0;
	int numRows = 3; //setSize(attrSet);
	int numZ = 7;
	int rmatbeg[numRows];
	double rhs[numRows];
	char sense[numRows];
	char *rowname[numRows];
	int rmatind[numZ];
	double rmatval[numZ];

	Node *left = NULL, *right = NULL, *l = NULL, *r = NULL;
	int i = 0, j = 0, aIndex = 0, uIndex, xIndex;
	char *op;
	char *selectAttName;
	int neg = 1;

//create first constraints for u: u.Aj<= mod(t).Aj
	rmatbeg[j] = i;
	sense[j] = 'L';
	left = getHeadOfListP((List *) s->expr);
	selectAttName = ((AttributeReference *) left)->name;
	uIndex = getObjectIndex(
			CONCAT_STRINGS("u_", selectAttName, "_", itoa(upNum)));
	rowname[j] = CONCAT_STRINGS("u1_", selectAttName, itoa(upNum));
	rmatind[i] = uIndex;
	rmatval[i] = 1.0;
	i++;
	right = (Node *) getTailOfListP((List *) s->expr);
	// ex. if A=10 removed this type of update for simplification
	if (isA(right, Constant)) {
		rhs[j] = constrToDouble((Constant *) right);
	} else {
// ex. if A= A +2 which means A1= A0 +2 => u1<= A0 +2 => u1 - A0 <= 2
		neg = 1;
		l = (Node *) getHeadOfListP(((Operator *) right)->args);
		r = (Node *) getTailOfListP(((Operator *) right)->args);
		op = ((Operator *) right)->name;
		if (strcmp(op, "-") == 0) {
			neg = -1;
		}

		if (isA(r, AttributeReference) && isA(l, Constant)) {
			aIndex = getObjectIndex(
					CONCAT_STRINGS(((AttributeReference * ) r)->name, "_",
							itoa(upNum - 1)));
			rmatind[i] = aIndex;
			rmatval[i] = -1;
			rhs[j] = constrToDouble((Constant *) l) * neg;

		} else if (isA(l, AttributeReference) && isA(r, Constant)) {
			aIndex = getObjectIndex(
					CONCAT_STRINGS(((AttributeReference * ) l)->name, "_",
							itoa(upNum - 1)));
			rmatind[i] = aIndex;
			rmatval[i] = -1;
			rhs[j] = constrToDouble((Constant *) r) * neg;
		}
		i++;
	}
	j++;
//create second constraints for u: u.Aj<= x*M => u.Aj- x*M <= 0
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("u2_", selectAttName, "_", itoa(upNum));
	sense[j] = 'L';
	rmatind[i] = uIndex;
	rmatval[i] = 1.0;
	i++;
// xIndex
	xIndex = getObjectIndex(CONCAT_STRINGS("x_", itoa(upNum)));
	rmatind[i] = xIndex;
	rmatval[i] = -1.0 * default_ub;
	i++;
	rhs[j] = 0;
	j++;

//create third constraints for u: u.Aj>= mod(t).Aj -(1- x) *M => u.Aj- mod(t).Aj- x*M >=-M
// for A1= A0 +2 : i did not consider +2 as it doesn't have an effect on -M
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("u3_", selectAttName, "_", itoa(upNum));
	sense[j] = 'G';
	rmatind[i] = uIndex;
	rmatval[i] = 1.0;
	i++;
	rmatind[i] = aIndex;
	rmatval[i] = -1;
	i++;
// xIndex
	rmatind[i] = xIndex;
	rmatval[i] = -1.0 * default_ub;
	i++;
	rhs[j] = -1.0 * default_ub;

	DEBUG_LOG("Processed u for update %d.\n", upNum);

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}

	return status;
}

static int createV(int upNum, char *attr, CPXENVptr env, CPXLPptr lp) {
	int status = 0;
	int numRows = 3; //setSize(attrSet);
	int numZ = 7;
	int rmatbeg[numRows];
	double rhs[numRows];
	char sense[numRows];
	char *rowname[numRows];
	int rmatind[numZ];
	double rmatval[numZ];

	int i = 0, j = 0, vIndex, aIndex, xIndex;

//create first constraints for v: v.Aj<= Aj => v.Aj- Aj <=0
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("v1_", attr, "_", itoa(upNum));
	sense[j] = 'L';
	vIndex = getObjectIndex(CONCAT_STRINGS("v_", attr, "_", itoa(upNum)));
	rmatind[i] = vIndex;
	rmatval[i] = 1.0;
	i++;
	aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", itoa(upNum - 1)));
	rmatind[i] = aIndex;
	rmatval[i] = -1.0;
	i++;
	rhs[j] = 0;
	j++;
//create second constraints for v: v.Aj<= (1-x)*M => v.Aj+ x*M <= M
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("v2_", attr, "_", itoa(upNum));
	sense[j] = 'L';
	rmatind[i] = vIndex;
	rmatval[i] = 1.0;
	i++;
// xIndex
	xIndex = getObjectIndex(CONCAT_STRINGS("x_", itoa(upNum)));
	rmatind[i] = xIndex;
	rmatval[i] = 1.0 * default_ub;
	i++;
	rhs[j] = default_ub;
	j++;

//create third constraints for v: v.Aj>= Aj - x*M => v.Aj- Aj+ x*M >=0
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("v3_", attr, "_", itoa(upNum));
	sense[j] = 'G';
	rmatind[i] = vIndex;
	rmatval[i] = 1.0;
	i++;
	rmatind[i] = aIndex;
	rmatval[i] = -1;
	i++;
// xIndex
	rmatind[i] = xIndex;
	rmatval[i] = 1.0 * default_ub;
	rhs[j] = 0;
//i++;
//j++;

	DEBUG_LOG("Processed v for update %d.\n", upNum);

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}

	return status;
}

static int setAttUV(int upNum, char *attr, CPXENVptr env, CPXLPptr lp) {
	int status = 0;
	int numRows = 1; //setSize(attrSet);
	int numZ = 3;
	int rmatbeg[numRows];
	double rhs[numRows];
	char sense[numRows];
	char *rowname[numRows];
	int rmatind[numZ];
	double rmatval[numZ];

	int i = 0, uIndex;

//create first constraints for Aj: Aj= U + V => Aj-U-V = 0
	rmatbeg[0] = 0;
	rowname[0] = CONCAT_STRINGS("uv", itoa(upNum));
	sense[0] = 'E';
	rmatind[i] = getObjectIndex(CONCAT_STRINGS(attr, "_", itoa(upNum)));
	rmatval[i] = 1.0;
	i++;
	uIndex = getObjectIndex(CONCAT_STRINGS("u_", attr, "_", itoa(upNum)));
	rmatind[i] = uIndex;
	rmatval[i] = -1.0;
	i++;
//vIndex
	rmatind[i] = uIndex + 1;
	rmatval[i] = -1.0;
	rhs[0] = 0;

	DEBUG_LOG("Processed uv for update %d.\n", upNum);

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}

	return status;
}
static int setNexSt(int upNum, Set *aSet, CPXENVptr env, CPXLPptr lp) {

	int status = 0;
	int numRows = setSize(aSet);
	int numZ = numRows * 2;
	int rmatbeg[numRows];
	double rhs[numRows];
	char sense[numRows];
	char *rowname[numRows];
	int rmatind[numZ];
	double rmatval[numZ];

	int i = 0, j = 0, index;

	FOREACH_SET(char, s, aSet)
	{
		rmatbeg[j] = i;
		rowname[j] = CONCAT_STRINGS("upSt", itoa(upNum));
		sense[j] = 'E';
		index = getObjectIndex(CONCAT_STRINGS(s, "_", itoa(upNum)));
		rmatind[i] = index;
		rmatval[i] = 1.0;
		i++;
		index = getObjectIndex(CONCAT_STRINGS(s, "_", (itoa(upNum - 1))));
		rmatind[i] = index;
		rmatval[i] = -1.0;
		i++;
		rhs[j] = 0;
		j++;
		DEBUG_LOG("Processed next state for update %s.\n", s);
	}

	DEBUG_LOG("Processed next state for update %d.\n", upNum);

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}

	return status;
}
/*
 static int setInsSt(int upNum, Node *query, CPXENVptr env, CPXLPptr lp) {
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
 int numAttr = setSize(attrSet);

 FOREACH(Constant,c,(List *)query)
 {
 if (c->constType == DT_INT || c->constType == DT_FLOAT
 || c->constType == DT_LONG) {
 rmatbeg[i] = i;
 rowname[i] = "insRow";
 rmatind[i] = upNum * numAttr + j;
 rmatval[i] = 1.0;
 sense[i] = 'E';
 rhs[i] = constrToDouble((Constant *) c);
 i++;
 }
 j++;
 }

 status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
 rmatind, rmatval, NULL, rowname);
 }
 return status;
 }
 */
static int condToSt(int upNum, Node *cond, CPXENVptr env, CPXLPptr lp) {
	DEBUG_LOG("Start Processing condition for update %d.\n", upNum);
	int status = 0;
	if (lp == NULL) {
		ERROR_LOG("temp_lp is NULL.\n");
	}
	if (cond == NULL) {
		ERROR_LOG("cond is NULL.\n");
	}
	DEBUG_LOG("condition type is %s.\n", cond->type);
	char *opName = ((Operator *) cond)->name;
	DEBUG_LOG("Start Processing condition for update with operation %s.\n",
			*opName);
	if (strcmp(opName, "AND") == 0) {
		DEBUG_LOG("we have AND Operator");
		condToSt(upNum, (Node *) getHeadOfListP(((Operator *) cond)->args), env,
				lp);
		condToSt(upNum, (Node *) getTailOfListP(((Operator *) cond)->args), env,
				lp);
	} else {

		DEBUG_LOG("Start Processing condition for update in else %d.\n", upNum);

		List *result = NIL;
		result = getAttrNameFromOpExpList(result, (Operator *) cond);

		int numCols = getListLength(result);
		int index = 0;
		int numRows = 1;
		int numZ = numRows * numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];

		rmatbeg[0] = 0;
		rowname[0] = "crow";

		if (strcmp(opName, "=") == 0) {
			sense[0] = 'E';
		} else if (strcmp(opName, ">") == 0 || strcmp(opName, ">=") == 0) {
			sense[0] = 'G';
		} else if (strcmp(opName, "<") == 0 || strcmp(opName, "<=") == 0) {
			sense[0] = 'L';
		}

		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);

		if (isA(left, AttributeReference)) {
			index = getObjectIndex(
					CONCAT_STRINGS(((AttributeReference * ) left)->name, "_",
							itoa(upNum - 1)));
			rmatind[0] = index;
			rmatval[0] = 1.0;
			DEBUG_LOG("Processed condition for update and attribute %s.\n",
					((AttributeReference * ) left)->name);
		}
		//if the condition likes c>10
		if (isA(right, Constant)) {
			//we have just one attribute
			rhs[0] = constrToDouble((Constant *) right);
			DEBUG_LOG("Processed condition for update with value %d.\n",
					rhs[0]);
		}
		//if the condition likes c>b => c-b>0
		else if (isA(right, AttributeReference)) {
			index = getObjectIndex(
					CONCAT_STRINGS(((AttributeReference * ) right)->name, "_",
							itoa(upNum - 1)));
			rmatind[1] = index;
			rmatval[1] = -1.0;
			rhs[0] = 0;
		}

		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		if (status) {
			ERROR_LOG(
					"Failure to convert update and add a row to cplex problem %d.\n",
					status);
		}
	}
	DEBUG_LOG("Finished Processing condition for update %d.\n", upNum);

	return status;
}

static int exprToSymbol(int upNum, Node *expr, CPXENVptr env, CPXLPptr lp) {

	int status = 0;
	Node *left = NULL;
	char *selectAttName;
//Set *newAttrSet = MAKE_STR_SET();
	Set *newAttrSet = STRSET();
	Set *tempSet = STRSET();
	List *sClause = NIL;

	switch (expr->type) {
	case T_Update:
		sClause = ((Update *) expr)->selectClause;
		FOREACH(SelectItem, s, sClause )
		{
			status = createU(upNum, s, env, lp);
			//if (!status) {
			left = getHeadOfListP((List *) s->expr);
			selectAttName = ((AttributeReference *) left)->name;
			status = createV(upNum, selectAttName, env, lp);
			addToSet(newAttrSet, selectAttName);
			//}
			//if (!status) {
			status = setAttUV(upNum, selectAttName, env, lp);
			//}
		}
		//if (!status) {
		tempSet = setDifference(attrSet, newAttrSet);
		status = setNexSt(upNum, tempSet, env, lp);
		//}
		break;
		/*
		 case T_Delete:
		 status = deleteToSymbol((Delete *) expr, env, lp);
		 break;
		 case T_Insert:
		 status = insertToConstr((Insert *) expr, env, lp);
		 break;
		 */
	default:
		break;
	}
	return status;
}

List *symbolicExe(List *exprs) {
	List *depUps = NIL;
	int status = 0;
	//int temp_status=0;
	//boolean result = FALSE;
	int solstat;
	double objval;
	double *x = NULL;
	double *pi = NULL;
	double *slack = NULL;
	double *dj = NULL;
	int cur_numrows, cur_numcols;

	CPXENVptr env = NULL;
	CPXLPptr lp = NULL;
	//CPXENVptr temp_env = NULL;
	CPXLPptr temp_lp = NULL;

	/* Initialize the CPLEX environment */
	env = CPXopenCPLEX(&status);
	//temp_env = CPXopenCPLEX(&temp_status);

	if (env == NULL) {
		char errmsg[CPXMESSAGEBUFSIZE];
		CPXgeterrorstring(env, status, errmsg);
		ERROR_LOG("Could not open CPLEX environment.\n%s", errmsg);
	}

	/*
	 if (temp_env == NULL) {
	 char errmsg[CPXMESSAGEBUFSIZE];
	 CPXgeterrorstring(temp_env, temp_status, errmsg);
	 ERROR_LOG("Could not open CPLEX environment.\n%s", errmsg);
	 }
	 */
	/* Turn on output to the screen */

	status = CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON);
	if (status) {
		ERROR_LOG("Failure to turn on screen indicator, error %d.\n", status);
	}

	/*
	 temp_status = CPXsetintparam(temp_env, CPXPARAM_ScreenOutput, CPX_ON);
	 if (temp_status) {
	 ERROR_LOG("Failure to turn on screen indicator, error %d.\n",
	 temp_status);
	 }
	 */
	/* Turn on data checking */

	status = CPXsetintparam(env, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN);
	if (status) {
		ERROR_LOG("Failure to turn on data checking, error %d.\n", status);
	}

	/*
	 temp_status = CPXsetintparam(temp_env, CPXPARAM_Read_DataCheck,
	 CPX_DATACHECK_WARN);
	 if (temp_status) {
	 ERROR_LOG("Failure to turn on data checking, error %d.\n", temp_status);
	 }
	 */
	/* Create the problem. */

	lp = CPXcreateprob(env, &status, "lpex1");
	//temp_lp = CPXcreateprob(temp_env, &temp_status, "temp_lpex1");

	/* A returned pointer of NULL may mean that not enough memory
	 was available or there was some other problem.  In the case of
	 failure, an error message will have been written to the error
	 channel from inside CPLEX.  In this example, the setting of
	 the parameter CPXPARAM_ScreenOutput causes the error message to
	 appear on stdout.  */

	if (lp == NULL) {
		ERROR_LOG("Failed to create LP.\n");
	}

	int numUp = getListLength(exprs);
	Node *up = popHeadOfListP(exprs);

	setSymbolicObjects(up, numUp);

	/* Now populate the problem with the data.  For building large
	 problems, consider setting the row, column and nonzero growth
	 parameters before performing this task. */

	status = CPXnewcols(env, lp, totalObjects, obj, lb, ub, NULL, colname);
	if (status) {
		ERROR_LOG("Failure to create cplex columns %d.\n", status);
	}

	/*
	 temp_status = CPXnewcols(temp_env, temp_lp, totalObjects, obj,
	 lb, ub, NULL, colname);
	 if (temp_status) {
	 ERROR_LOG("Failure to create cplex columns %d.\n", temp_status);
	 }
	 */

	/* Now populate the problem with the data.  For building large
	 problems, consider setting the row, column and nonzero growth
	 parameters before performing this task. */
	int i = 1;
	DEBUG_LOG("symbolic execution for update %d.\n", i);
	exprToSymbol(i, up, env, lp);
	condToSt(i, up, env, lp);
	depUps = appendToTailOfList(depUps, up);
	i++;

	FOREACH(Node,ex,exprs)
	{
		DEBUG_LOG("symbolic execution for update %d.\n", i);
		exprToSymbol(i, ex, env, lp);
		//temp_env = env;
		//temp_lp = lp;
		//temp_status = status;
		//temp_lp = CPXcreateprob(temp_env, &temp_status, "temp_lpex1");

		/*
		 temp_status = CPXnewcols(temp_env, temp_lp, totalObjects, obj, lb, ub,
		 NULL, colname);
		 if (temp_status) {
		 ERROR_LOG("Failure to create cplex columns %d.\n", temp_status);
		 }
		 */
		//temp_status = condToSt(i, ex, temp_env, temp_lp);
		if (ex->type == T_Update || ex->type == T_Delete) {
			temp_lp = CPXcloneprob(env, lp, &status);
			DEBUG_LOG("copied the main problem for the symbolic execution.\n");

			if (temp_lp == NULL) {
				ERROR_LOG("Failed to create temp_lp.\n");
			}
			if (ex->type == T_Update)
				status = condToSt(i, ((Update *) ex)->cond, env, temp_lp);
			else
				status = condToSt(i, ((Delete *) ex)->cond, env, temp_lp);

			if (status) {
				ERROR_LOG(
						"Failed to populate new temp problem for the condition of expression.\n");
			}

			status = CPXlpopt(env, temp_lp);
			if (status) {
				ERROR_LOG("Failed to optimize LP.\n");
			}

			/*
			 if (temp_status) {
			 ERROR_LOG(
			 "Failed to populate problem for the condition of expression.\n");
			 }

			 temp_status = CPXlpopt(temp_env, temp_lp);
			 if (temp_status) {
			 ERROR_LOG("Failed to optimize LP.\n");
			 }

			 cur_numrows = CPXgetnumrows(temp_env, temp_lp);
			 cur_numcols = CPXgetnumcols(temp_env, temp_lp);
			 */
			cur_numrows = CPXgetnumrows(env, temp_lp);
			cur_numcols = CPXgetnumcols(env, temp_lp);

			x = (double *) MALLOC(cur_numcols * sizeof(double));
			slack = (double *) MALLOC(cur_numrows * sizeof(double));
			dj = (double *) MALLOC(cur_numcols * sizeof(double));
			pi = (double *) MALLOC(cur_numrows * sizeof(double));

			if (x == NULL || slack == NULL || dj == NULL || pi == NULL) {
				ERROR_LOG("Could not allocate memory for solution.\n");
			}
			/*
			 temp_status = CPXsolution(temp_env, temp_lp, &solstat, &objval, x, pi,
			 slack, dj);
			 if (temp_status) {
			 DEBUG_LOG("False: Failed to obtain solution.\n");
			 //result = FALSE;
			 */

			status = CPXsolution(env, temp_lp, &solstat, &objval, x, pi, slack,
					dj);
			if (status) {
				DEBUG_LOG("False: Failed to obtain solution for update %d.\n",
						i);
				//result = FALSE;

			} else {
				DEBUG_LOG("True: Obtained solution for update %d.\n", i);
				//result = TRUE;
				depUps = appendToTailOfList(depUps, ex);
			}

			if (temp_lp != NULL) {
				status = CPXfreeprob(env, &temp_lp);
				if (status) {
					DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
				}
			}
		}
		/*
		 if (temp_lp != NULL) {
		 temp_status = CPXfreeprob(temp_env, &temp_lp);
		 if (temp_status) {
		 DEBUG_LOG("CPXfreeprob failed, error code %d.\n", temp_status);
		 }
		 }
		 */
		i++;
	}

	DEBUG_LOG("End of symbolic execution for finding dependent updates .\n");
	/*
	 status = exprToEval(expr1, inv1, env, lp);

	 if (status) {
	 ERROR_LOG("Failed to populate problem for the first expression.\n");
	 }
	 */

	/* Optimize the problem and obtain solution. */
	/*
	 status = CPXlpopt(env, lp);
	 if (status) {
	 ERROR_LOG("Failed to optimize LP.\n");
	 }
	 */

	/* The size of the problem should be obtained by asking CPLEX what
	 the actual size is, rather than using sizes from when the problem
	 was built.  cur_numrows and cur_numcols store the current number
	 of rows and columns, respectively.  */
	/*
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

	 //check solution result

	 DEBUG_LOG("\nSolution status = %d\n", solstat);
	 DEBUG_LOG("Solution value  = %f\n\n", objval);

	 int j;
	 for (j = 0; j < cur_numcols; j++) {
	 DEBUG_LOG("Column %d:  Value =  %10f  Reduced cost = %10f\n", j, x[j],
	 dj[j]);
	 }
	 */

//TERMINATE:
	/* Free up the problem as allocated by CPXcreateprob, if necessary */

	if (lp != NULL) {
		status = CPXfreeprob(env, &lp);
		if (status) {
			DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
		}
	}

	DEBUG_LOG("free CPXfreeprob.\n");

	if (temp_lp != NULL) {
		status = CPXfreeprob(env, &temp_lp);
		if (status) {
			DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
		}
	}
	DEBUG_LOG("free tempCPXfreeprob.\n");
	/*
	 if (temp_env != NULL) {
	 temp_status = CPXcloseCPLEX(&temp_env);

	 if (temp_status) {
	 DEBUG_LOG("Could not close CPLEX environment.\n");
	 }
	 }

	 DEBUG_LOG("free tempCPXcloseCPLEX.\n");
	 */
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
	DEBUG_LOG("free CPXcloseCPLEX.\n");

	DEBUG_LOG("Returning dependent updates.\n");
	return depUps;
}

// ********************************************************************************
// dummy replacement if cplex is not available
/*
 #else

 boolean exprToSat(Node *expr1, boolean inv1, Node *expr2, boolean inv2) {
 return TRUE;
 }

 #endif
 */
