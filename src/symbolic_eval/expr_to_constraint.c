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
#include "model/set/set.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"

static int totalObjects = 0;
static int totalAttr = 0;
static Set *attrSet = NULL;
static double default_lb = 0.0;
//static double default_ub = CPX_INFBOUND;
static double default_ub = 1000000000000;
static int default_num_cols = 100;

// added guard here to only reference cplex data structure if we are linking with cplex
#ifdef HAVE_LIBCPLEX

//TODO should switch to using this instead and eventually remove the global static variable and pass the context around
typedef struct CplexObjects {
 char *tableName;
 HashMap *attrIndex;       // hashmap attributename -> attribute index in obj
 double *obj;
 double *lb;
 double *ub;
 char **colname;
 } CplexObjects;

//TODO static CplexObjects *cplexObjects = NULL; // global pointer to current cplex objects


//static char *tableName;
//TODO having all this global datastructures is bad! whew!
static HashMap *attrIndex;    // hashmap attributename -> attribute index in obj
static int *lastIndex; // hashmap attributename -> get last attribute index that was used
static double *obj;
static double *lb;
static double *ub;
static char **colname;
static char *ctype;

static void setCplexObjects(Node *expr);
static int getObjectIndex(char *attrName);
static List *getOpExpStack(List *stackList, Operator *opExpList);
static double constrToDouble(Constant *cons);
static int setToConstr(Node * query, CPXENVptr env, CPXLPptr lp);
static int condToConstr(Node *cond, CPXENVptr env, CPXLPptr lp);
static int invertCondToConstr(Node *cond, List *selectClause, CPXENVptr env,
		CPXLPptr lp);
static int exprToEval(Node *expr, boolean invert, CPXENVptr env, CPXLPptr lp);
static void setSymbolicObjects(List *exprs);
static void deleteCplexObjects();
static int createU(int upNum, SelectItem *s, CPXENVptr env, CPXLPptr lp);
static int createV(int upNum, char *attr, CPXENVptr env, CPXLPptr lp);
static int setAttUV(int upNum, char *attr, CPXENVptr env, CPXLPptr lp);
static int numOps(Node *cond);
static int setX(int upNum, int index, Node *cond, CPXENVptr env, CPXLPptr lp);
static int setXTrue(int upNum, CPXENVptr env, CPXLPptr lp);
static int exprToSymbol(int upNum, Node *expr, CPXENVptr env, CPXLPptr lp);
static int firstUpExe(int upNum, Node *expr, CPXENVptr env, CPXLPptr lp);
static int firstInsExe(Node *expr, CPXENVptr env, CPXLPptr lp);

static void
setCplexObjects(Node *expr)
{
	totalObjects = 0;
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

	//TODO should free or use memory context here probably (copy out if necessary)
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

//}
}

static int
getObjectIndex(char *attrName)
{
	return INT_VALUE(MAP_GET_STRING(attrIndex,(char *)attrName));
}

static List *
getOpExpStack(List *stackList, Operator *opExpList)
{
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

static double
constrToDouble(Constant *cons)
{
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
		return 0.0;
	}
	return 0.0;
}

static int
setToConstr(Node *query, CPXENVptr env, CPXLPptr lp)
{
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

static int
condToConstr(Node *cond, CPXENVptr env, CPXLPptr lp)
{

	int status = 0;

	//TODO assumes that it's an opertors and if not then this will fail
	char *opName = ((Operator *) cond)->name;
	if (strcmp(opName, "AND") == 0) {
		DEBUG_LOG("we have AND Operator");
		condToConstr((Node *) getHeadOfListP(((Operator *) cond)->args), env,
				lp);
		condToConstr((Node *) getTailOfListP(((Operator *) cond)->args), env,
				lp);
	}
	else
	{

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

		int notEq = 0;

		if (strcmp(opName, "=") == 0) {
			sense[0] = 'E';
		} else if (strcmp(opName, ">=") == 0) {
			sense[0] = 'G';
		} else if (strcmp(opName, ">") == 0) {
			sense[0] = 'G';
			notEq = 1;
		} else if (strcmp(opName, "<=") == 0) {
			sense[0] = 'L';
		} else if (strcmp(opName, "<") == 0) {
			sense[0] = 'L';
			notEq = -1;
		}

		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
		rhs[0] = constrToDouble((Constant *) right) + notEq;
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

			//TODO these are only several hard-coded cases. This needs to be working recursively like the rules in the paper to work for more general cases (We can still optimize certain special cases later).
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
static void setSymbolicObjects(List *exprsList) {
	DEBUG_LOG("working setCplexObjects for updates.\n");

	List *exprs = copyList(exprsList);
	char *tbName = NULL;
	Node *up = getHeadOfListP(exprs);

	switch (up->type) {
	case T_Update:
		tbName = ((Update *) up)->updateTableName;
		break;
	case T_Delete:
		tbName = ((Delete *) up)->deleteTableName;
		break;
	case T_Insert:
		tbName = ((Insert *) up)->insertTableName;
		break;
	default:
		break;
	}
	//tableName = tbName;
	List *schema = NIL;
	schema = getAttributeNames(tbName);
	attrSet = makeStrSetFromList(schema);
	totalAttr = getListLength(schema);
	//totalAttr + x for all exprs
	int totalIndex = getListLength(exprsList) + totalAttr;
	lastIndex = (int *) MALLOC(sizeof(int) * totalIndex);
	for (int i = 0; i < totalIndex; i++) {
		lastIndex[i] = 0;
	}

	int i = 1;
	int count = totalAttr;

	FOREACH(Node,exp,exprs)
	{
		if (exp->type == T_Update) {
			int sc = getListLength(((Update *) exp)->selectClause);
			//for new attr value and related u,v
			if (i == 1)
				count += sc;
			else
				count += 3 * sc;
			//for x
			count += numOps(((Update *) exp)->cond);
		} else if (exp->type == T_Delete) {
			//for x
			count += numOps(((Delete *) exp)->cond);
		}
		i++;
	}

	//for each update, we need new states ai for each update, u,v for each attribute and update and x for each update
	attrIndex = NEW_MAP(Constant, Constant);
	obj = (double *) MALLOC(sizeof(double) * count);
	lb = (double *) MALLOC(sizeof(double) * count);
	ub = (double *) MALLOC(sizeof(double) * count);
	colname = (char **) MALLOC(sizeof(char *) * count);
	ctype = (char *) MALLOC(sizeof(char) * count);

	totalObjects = 0;
	char *cName = NULL;

//create initial attributes
	FOREACH(char,a,schema)
	{
		cName = CONCAT_STRINGS(a, "_", gprom_itoa(0));
		MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
		colname[totalObjects] = cName;
		lb[totalObjects] = default_lb;
		ub[totalObjects] = default_ub;
		obj[totalObjects] = 0;
		ctype[totalObjects] = 'C';
		totalObjects++;
	}

	Node *selectAttr;
	char *attName;
	List *sClause;

	//TODO find better names (I think these are value before update, after updates, and does tuple exist
	//we must create new attribute states, U,V, and X
	i = 1;
	FOREACH(Node,exp,exprs)
	{
		if (exp->type == T_Update) {
			//for new attr value and related u,v
			sClause = ((Update *) exp)->selectClause;
			FOREACH(SelectItem, s, sClause )
			{
				selectAttr = getHeadOfListP((List *) s->expr);
				attName = ((AttributeReference *) selectAttr)->name;
				cName = CONCAT_STRINGS(attName, "_", gprom_itoa(i));
				MAP_ADD_STRING_KEY(attrIndex, cName,
						createConstInt(totalObjects));
				colname[totalObjects] = cName;
				lb[totalObjects] = default_lb;
				ub[totalObjects] = default_ub;
				obj[totalObjects] = 0;
				ctype[totalObjects] = 'C';
				totalObjects++;
				if (i > 1) {
					cName = CONCAT_STRINGS("u_", attName, "_", gprom_itoa(i));
					MAP_ADD_STRING_KEY(attrIndex, cName,
							createConstInt(totalObjects));
					colname[totalObjects] = cName;
					lb[totalObjects] = default_lb;
					ub[totalObjects] = default_ub;
					obj[totalObjects] = 0;
					ctype[totalObjects] = 'C';
					totalObjects++;

					cName = CONCAT_STRINGS("v_", attName, "_", gprom_itoa(i));
					MAP_ADD_STRING_KEY(attrIndex, cName,
							createConstInt(totalObjects));
					colname[totalObjects] = cName;
					lb[totalObjects] = default_lb;
					ub[totalObjects] = default_ub;
					obj[totalObjects] = 0;
					ctype[totalObjects] = 'C';
					totalObjects++;
				}
			}
			//for x
			cName = CONCAT_STRINGS("x_", gprom_itoa(i));
			MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
			colname[totalObjects] = cName;
			lb[totalObjects] = 0;
			ub[totalObjects] = 1;
			obj[totalObjects] = 0;
			ctype[totalObjects] = 'I';
			totalObjects++;
			count = numOps(((Update *) exp)->cond);
			count--;
			for (int j = 1; j <= count; j++) {

				cName = CONCAT_STRINGS("x_", gprom_itoa(i), "_", gprom_itoa(j));
				MAP_ADD_STRING_KEY(attrIndex, cName,
						createConstInt(totalObjects));
				colname[totalObjects] = cName;
				lb[totalObjects] = 0;
				ub[totalObjects] = 1;
				obj[totalObjects] = 0;
				ctype[totalObjects] = 'I';
				totalObjects++;
			}
			deepFreeList(sClause);
		} else if (exp->type == T_Delete) {
			//for x
			cName = CONCAT_STRINGS("x_", gprom_itoa(i));
			MAP_ADD_STRING_KEY(attrIndex, cName, createConstInt(totalObjects));
			colname[totalObjects] = cName;
			lb[totalObjects] = 0;
			ub[totalObjects] = 1;
			obj[totalObjects] = 0;
			ctype[totalObjects] = 'I';
			totalObjects++;
			count = numOps(((Delete *) exp)->cond);
			count--;
			for (int j = 1; j <= count; j++) {

				cName = CONCAT_STRINGS("x_", gprom_itoa(i), "_", gprom_itoa(j));
				MAP_ADD_STRING_KEY(attrIndex, cName,
						createConstInt(totalObjects));
				colname[totalObjects] = cName;
				lb[totalObjects] = 0;
				ub[totalObjects] = 1;
				obj[totalObjects] = 0;
				ctype[totalObjects] = 'I';
				totalObjects++;
			}
		}
		i++;
	}
	DEBUG_LOG("number of created total cplex Objects %d.\n", totalObjects);
	for (int k = 0; k < totalObjects; k++) {
		DEBUG_LOG("%s.\n", colname[k]);
	}
}

static void deleteCplexObjects() {
	FOREACH_HASH_ENTRY(elm,attrIndex)
	{
		removeAndFreeMapElem(attrIndex, elm->key);
	}
	FREE(attrIndex);
	FREE(lastIndex);
}

static int createU(int upNum, SelectItem *s, CPXENVptr env, CPXLPptr lp) {
	int status = 0;
	int numRows = 3; //setSize(attrSet);
	List *result = NIL;
	result = getAttrNameFromOpExpList(result, (Operator *) s);
	int numCols = getListLength(result);
	int numZ = 2 * numCols + 3;
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
	char *attr;
	//create first constraints for u: u.Aj<= mod(t).Aj
	rmatbeg[j] = i;
	sense[j] = 'L';
	left = getHeadOfListP((List *) s->expr);
	selectAttName = ((AttributeReference *) left)->name;
	uIndex = getObjectIndex(
			CONCAT_STRINGS("u_", selectAttName, "_", gprom_itoa(upNum)));
	rowname[j] = CONCAT_STRINGS("u1_", selectAttName, gprom_itoa(upNum));
	rmatind[i] = uIndex;
	rmatval[i] = 1.0;
	i++;
	right = (Node *) getTailOfListP((List *) s->expr);
	// ex. if A=10 removed this type of update for simplification
	if (isA(right, Constant)) {
		rhs[j] = constrToDouble((Constant *) right);
		DEBUG_LOG(
				"Processed u for update %d to set %s attribute constant value %10f.\n",
				upNum, selectAttName, rhs[j]);
	} else {
		op = ((Operator *) right)->name;
		l = (Node *) getHeadOfListP(((Operator *) right)->args);
		r = (Node *) getTailOfListP(((Operator *) right)->args);
		if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0) {

// ex. if A= A +2 which means A1= A0 +2 => u1<= A0 +2 => u1 - A0 <= 2
			neg = 1;
			//op = ((Operator *) right)->name;
			if (strcmp(op, "-") == 0) {
				neg = -1;
			}

			if (isA(r, AttributeReference) && isA(l, Constant)) {
				attr = ((AttributeReference *) r)->name;
				aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				aIndex = lastIndex[aIndex];
				aIndex = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
				rmatind[i] = aIndex;
				rmatval[i] = -1;
				rhs[j] = constrToDouble((Constant *) l) * neg;

			} else if (isA(l, AttributeReference) && isA(r, Constant)) {
				attr = ((AttributeReference *) l)->name;
				aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				aIndex = lastIndex[aIndex];
				aIndex = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
				rmatind[i] = aIndex;
				rmatval[i] = -1;
				rhs[j] = constrToDouble((Constant *) r) * neg;
			}
			i++;
		} else if (strcmp(op, "*") == 0) {
			if (isA(r, AttributeReference) && isA(l, Constant)) {
				attr = ((AttributeReference *) r)->name;
				aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				aIndex = lastIndex[aIndex];
				aIndex = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
				rmatind[i] = aIndex;
				rmatval[i] = -constrToDouble((Constant *) l);
				rhs[j] = 0;

			} else if (isA(l, AttributeReference) && isA(r, Constant)) {
				attr = ((AttributeReference *) l)->name;
				aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				aIndex = lastIndex[aIndex];
				aIndex = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
				rmatind[i] = aIndex;
				rmatval[i] = -constrToDouble((Constant *) r);
				rhs[j] = 0;
			}
			i++;
		} else if (strcmp(op, "/") == 0) {
			if (isA(r, AttributeReference) && isA(l, Constant)) {
				attr = ((AttributeReference *) r)->name;
				aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				aIndex = lastIndex[aIndex];
				aIndex = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
				rmatind[i] = aIndex;
				rmatval[i] = -1 / constrToDouble((Constant *) l);
				rhs[j] = 0;

			} else if (isA(l, AttributeReference) && isA(r, Constant)) {
				attr = ((AttributeReference *) l)->name;
				aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				aIndex = lastIndex[aIndex];
				aIndex = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
				rmatind[i] = aIndex;
				rmatval[i] = -1 / constrToDouble((Constant *) r);
				rhs[j] = 0;
			}
			i++;
		}
	}
	DEBUG_LOG("Processed %s for update %d for %s attribute value %10f.\n",
			colname[uIndex], upNum, colname[aIndex], rhs[j]);
	j++;

//create second constraints for u: u.Aj<= x*M => u.Aj- x*M <= 0
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("u2_", selectAttName, "_", gprom_itoa(upNum));
	sense[j] = 'L';
	rmatind[i] = uIndex;
	rmatval[i] = 1.0;
	i++;
// xIndex
	xIndex = getObjectIndex(CONCAT_STRINGS("x_", gprom_itoa(upNum)));
	rmatind[i] = xIndex;
	rmatval[i] = -default_ub;
	i++;
	rhs[j] = 0;
	j++;

//create third constraints for u: u.Aj>= mod(t).Aj -(1- x) *M => u.Aj- mod(t).Aj- x*M >=-M

	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("u3_", selectAttName, "_", gprom_itoa(upNum));
	sense[j] = 'G';
	rmatind[i] = uIndex;
	rmatval[i] = 1.0;
	i++;
	if (isA(right, Constant)) {
		rhs[j] = constrToDouble((Constant *) right) - default_ub;
	} else if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0) {
		if (isA(r, AttributeReference) && isA(l, Constant)) {
			rmatind[i] = aIndex;
			rmatval[i] = -1;
			rhs[j] = (constrToDouble((Constant *) l) * neg) - default_ub;

		} else if (isA(l, AttributeReference) && isA(r, Constant)) {
			rmatind[i] = aIndex;
			rmatval[i] = -1;
			rhs[j] = (constrToDouble((Constant *) r) * neg) - default_ub;
		}
		i++;
	} else if (strcmp(op, "*") == 0) {
		if (isA(r, AttributeReference) && isA(l, Constant)) {
			rmatind[i] = aIndex;
			rmatval[i] = -constrToDouble((Constant *) l);
			rhs[j] = -default_ub;

		} else if (isA(l, AttributeReference) && isA(r, Constant)) {
			rmatind[i] = aIndex;
			rmatval[i] = -constrToDouble((Constant *) r);
			rhs[j] = -default_ub;
		}
		i++;
	} else if (strcmp(op, "/") == 0) {
		if (isA(r, AttributeReference) && isA(l, Constant)) {
			rmatind[i] = aIndex;
			rmatval[i] = -1 / constrToDouble((Constant *) l);
			rhs[j] = -default_ub;

		} else if (isA(l, AttributeReference) && isA(r, Constant)) {
			rmatind[i] = aIndex;
			rmatval[i] = -1 / constrToDouble((Constant *) r);
			rhs[j] = -default_ub;
		}
		i++;
	}
	DEBUG_LOG("Processed %s for update %d for %s attribute value %10f.\n",
			colname[uIndex], upNum, colname[aIndex], rhs[j]);
	// xIndex
	rmatind[i] = xIndex;
	rmatval[i] = -default_ub;

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}
	DEBUG_LOG("Processed %s for update %d for %s attribute and %s.\n",
			colname[uIndex], upNum, colname[aIndex], colname[xIndex]);
	return status;
}

//TODO variable naming the LP program is ad hoc. It would be better to fix some naming scheme and for ease of development provide methods to track mappings between LP variables and expression parts / objects in some context
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
	rowname[j] = CONCAT_STRINGS("v1_", attr, "_", gprom_itoa(upNum));
	sense[j] = 'L';
	vIndex = getObjectIndex(CONCAT_STRINGS("v_", attr, "_", gprom_itoa(upNum)));
	rmatind[i] = vIndex;
	rmatval[i] = 1.0;
	i++;
	aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
	aIndex = lastIndex[aIndex];
	aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(aIndex)));
	rmatind[i] = aIndex;
	rmatval[i] = -1.0;
	i++;
	rhs[j] = 0;
	j++;
//create second constraints for v: v.Aj<= (1-x)*M => v.Aj+ x*M <= M
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("v2_", attr, "_", gprom_itoa(upNum));
	sense[j] = 'L';
	rmatind[i] = vIndex;
	rmatval[i] = 1.0;
	i++;
// xIndex
	xIndex = getObjectIndex(CONCAT_STRINGS("x_", gprom_itoa(upNum)));
	rmatind[i] = xIndex;
	rmatval[i] = default_ub;
	i++;
	rhs[j] = default_ub;
	j++;
//create third constraints for v: v.Aj>= Aj - x*M => v.Aj- Aj+ x*M >=0
	rmatbeg[j] = i;
	rowname[j] = CONCAT_STRINGS("v3_", attr, "_", gprom_itoa(upNum));
	sense[j] = 'G';
	rmatind[i] = vIndex;
	rmatval[i] = 1.0;
	i++;
	rmatind[i] = aIndex;
	rmatval[i] = -1;
	i++;
// xIndex
	rmatind[i] = xIndex;
	rmatval[i] = default_ub;
	rhs[j] = 0;

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}

	DEBUG_LOG("Processed %s for update %d to set %s attribute and %s.\n",
			colname[vIndex], upNum, colname[aIndex], colname[xIndex]);
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

	int i = 0, uIndex, vIndex;

//create first constraints for Aj: Aj= U + V => Aj-U-V = 0
	rmatbeg[0] = 0;
	rowname[0] = CONCAT_STRINGS("uv", gprom_itoa(upNum));
	sense[0] = 'E';
	rmatind[i] = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(upNum)));
	rmatval[i] = 1.0;
	i++;
	uIndex = getObjectIndex(CONCAT_STRINGS("u_", attr, "_", gprom_itoa(upNum)));
	rmatind[i] = uIndex;
	rmatval[i] = -1.0;
	i++;
//vIndex
	vIndex = getObjectIndex(CONCAT_STRINGS("v_", attr, "_", gprom_itoa(upNum)));
	rmatind[i] = vIndex;
	rmatval[i] = -1.0;
	rhs[0] = 0;

	DEBUG_LOG("Processed uv for update %d to set %s attribute.\n", upNum, attr);

	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);

	//increase lastIndex for the modified attribute
	int aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
	lastIndex[aIndex] = upNum;

	if (status) {
		ERROR_LOG(
				"Failure to convert and invert update and did not add a row to cplex problem %d.\n",
				status);
	}

	return status;
}

int numOps(Node *cond) {
	int count = 1;
	if (cond == NULL)
		return 0;
	else {
		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
		if (isA(right, Operator)) {
			count += numOps(right);
		}
		if (isA(left, Operator)) {
			count += numOps(left);
		}
		return count;
	}
}

int setX(int upNum, int index, Node *cond, CPXENVptr env, CPXLPptr lp) {
	//DEBUG_LOG("Start Processing x for update %d.\n", upNum);
	int status = 0;

	if (lp == NULL) {
		ERROR_LOG("CPXLPptr is NULL.\n");
	}
	if (cond == NULL) {
		return status;
	}

	char *opName = ((Operator *) cond)->name;
	if (strcmp(opName, "AND") == 0) {
		int xindex, x1index, x2index;
		//increase lastIndex for the modified attribute
		int xIndex = totalAttr - 1 + upNum;
		int curIndex = lastIndex[xIndex];
		lastIndex[xIndex] += 2;
		DEBUG_LOG(
				"we have AND Operator and we create %d and %d indexes and set last xIndex %d",
				curIndex + 1, curIndex + 2, lastIndex[xIndex]);
		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
		setX(upNum, curIndex + 1, left, env, lp);
		setX(upNum, curIndex + 2, right, env, lp);
		if (index == 0)
			xindex = getObjectIndex(CONCAT_STRINGS("x_", gprom_itoa(upNum)));
		else
			xindex = getObjectIndex(
					CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(index)));
		DEBUG_LOG("we have AND Operator for %s", colname[xindex]);
		x1index = getObjectIndex(
				CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(curIndex + 1)));
		x2index = getObjectIndex(
				CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(curIndex + 2)));
		int numCols = 3;
		int numRows = 2;
		int numZ = numRows * numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];
		int i = 0, j = 0;
		rmatbeg[j] = i;
		rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
		rmatind[i] = x1index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = x2index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = xindex;
		rmatval[i] = -2.0;
		i++;
		sense[j] = 'L';
		rhs[j] = 1;
		j++;
		rmatbeg[j] = i;
		rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
		rmatind[i] = x1index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = x2index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = xindex;
		rmatval[i] = -2.0;
		i++;
		sense[j] = 'G';
		rhs[j] = 0;
		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		if (status) {
			ERROR_LOG(
					"Failure to convert and for x for update %d and add a row to cplex problem.\n",
					upNum);
		}
		return status;

	} else if (strcmp(opName, "OR") == 0) {
		int xindex, x1index, x2index;
		int xIndex = totalAttr - 1 + upNum;
		int curIndex = lastIndex[xIndex];
		lastIndex[xIndex] += 2;
		DEBUG_LOG(
				"we have AND Operator and we create %d and %d indexes and set last xIndex %d",
				curIndex + 1, curIndex + 2, lastIndex[xIndex]);
		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
		setX(upNum, curIndex + 1, left, env, lp);
		setX(upNum, curIndex + 2, right, env, lp);
		if (index == 0)
			xindex = getObjectIndex(CONCAT_STRINGS("x_", gprom_itoa(upNum)));
		else
			xindex = getObjectIndex(
					CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(index)));
		DEBUG_LOG("we have AND Operator for %s", colname[xindex]);
		x1index = getObjectIndex(
				CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(curIndex + 1)));
		x2index = getObjectIndex(
				CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(curIndex + 2)));
		int numCols = 3;
		int numRows = 2;
		int numZ = numRows * numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];
		int i = 0, j = 0;
		rmatbeg[j] = i;
		rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
		rmatind[i] = x1index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = x2index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = xindex;
		rmatval[i] = -2.0;
		i++;
		sense[j] = 'L';
		rhs[j] = 0;
		j++;
		rmatbeg[j] = i;
		rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
		rmatind[i] = x1index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = x2index;
		rmatval[i] = 1.0;
		i++;
		rmatind[i] = xindex;
		rmatval[i] = -1.0;
		i++;
		sense[j] = 'G';
		rhs[j] = 0;
		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		if (status) {
			ERROR_LOG(
					"Failure to convert and for x for update %d and add a row to cplex problem.\n",
					upNum);
		}
		return status;
	} else {
		List *result = NIL;
		result = getAttrNameFromOpExpList(result, (Operator *) cond);

		int numCols = getListLength(result) + 1;
		int v1index = 0, v2index = 0, xindex = 0;
		int numRows = 2;

		if (strcmp(opName, "=") == 0) {
			numRows = 4;
		}
		int numZ = numRows * numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];
		int i = 0, j = 0;
		int notEq = 0;
		int less = 1;
		char *attr;

		Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
		Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);

		if (index == 0)
			xindex = getObjectIndex(CONCAT_STRINGS("x_", gprom_itoa(upNum)));
		else
			xindex = getObjectIndex(
					CONCAT_STRINGS("x_", gprom_itoa(upNum), "_", gprom_itoa(index)));
		if (isA(left, AttributeReference)) {
			attr = ((AttributeReference *) left)->name;
			v1index = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
			v1index = lastIndex[v1index];
			v1index = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(v1index)));
			DEBUG_LOG("Processing x for update %d and attribute %s.\n", upNum,
					colname[v1index]);
		} else {
			ERROR_LOG(
					"The condition for update %d does not start with an attribute and it is invalid.\n",
					upNum);
		}

		if (strcmp(opName, "=") != 0) {

			if (strcmp(opName, ">=") == 0) {
				less = -1;
			} else if (strcmp(opName, ">") == 0) {
				notEq = 1;
				less = -1;
			} else if (strcmp(opName, "<=") == 0) {
			} else if (strcmp(opName, "<") == 0) {
				notEq = 1;
			}
			DEBUG_LOG("we have %s Operator for %s", opName, colname[xindex]);
			//if the condition likes c<10, c<=10, c>10 or c>=10
			if (isA(right, Constant)) {
				//we have just one attribute
				double cons = constrToDouble((Constant *) right);
				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = 1.0 * less;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = cons * less + 1 - notEq;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = -1.0 * less;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = -default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = -cons * less - default_ub + notEq;
				j++;

				DEBUG_LOG("generated x for update %d to set value %f.\n", upNum,
						cons);
			}
			//if the condition likes c>b, c>=b,c<b or c<=b
			else if (isA(right, AttributeReference)) {
				attr = ((AttributeReference *) right)->name;
				v2index = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				v2index = lastIndex[v2index];
				v2index = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(v2index)));
				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = 1.0 * less;
				i++;
				rmatind[i] = v2index;
				rmatval[i] = -1.0 * less;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = 1 - notEq;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				;
				rmatind[i] = v2index;
				rmatval[i] = 1.0 * less;
				i++;
				rmatind[i] = v1index;
				rmatval[i] = -1.0 * less;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = -default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = -default_ub + notEq;
				j++;
			}
		} else if (strcmp(opName, "=") == 0) {
			DEBUG_LOG("we have %s Operator for %s", opName, colname[xindex]);
			//if the condition likes c>10
			if (isA(right, Constant)) {
				//we have just one attribute
				double cons = constrToDouble((Constant *) right);
				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_e_1", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = 1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = cons + 1;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_e_2", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = -1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = -default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = -cons - default_ub;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_e_3", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = -1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = -cons + 1;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_e_4", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = 1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = -default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = cons - default_ub;
				j++;
			}
			//if the condition likes c>b
			else if (isA(right, AttributeReference)) {
				attr = ((AttributeReference *) right)->name;
				v2index = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
				v2index = lastIndex[v2index];
				v2index = getObjectIndex(
						CONCAT_STRINGS(attr, "_", gprom_itoa(v2index)));
				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = 1.0;
				i++;
				rmatind[i] = v2index;
				rmatval[i] = -1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = 1;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v2index;
				rmatval[i] = 1.0;
				i++;
				rmatind[i] = v1index;
				rmatval[i] = -1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = -default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = -default_ub;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v1index;
				rmatval[i] = -1.0;
				i++;
				rmatind[i] = v2index;
				rmatval[i] = 1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = 1;
				j++;

				rmatbeg[j] = i;
				rowname[j] = CONCAT_STRINGS("x_", gprom_itoa(upNum));
				rmatind[i] = v2index;
				rmatval[i] = -1.0;
				i++;
				rmatind[i] = v1index;
				rmatval[i] = 1.0;
				i++;
				rmatind[i] = xindex;
				rmatval[i] = -default_ub;
				i++;
				sense[j] = 'G';
				rhs[j] = -default_ub;
				j++;
			}
		}

		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		if (status) {
			ERROR_LOG(
					"Failure to convert x for update %d and add a row to cplex problem.\n",
					upNum);
		}
	}

	if (index == 0)
		DEBUG_LOG("Finished Processing x for update %d.\n", upNum);

	return status;
}

int setXTrue(int upNum, CPXENVptr env, CPXLPptr lp) {
	DEBUG_LOG("Start setting x True for update %d.\n", upNum);
	int status = 0;

	if (lp == NULL) {
		ERROR_LOG("temp_lp is NULL.\n");
	}

	int xindex = getObjectIndex(CONCAT_STRINGS("x_", gprom_itoa(upNum)));
	int rmatbeg[1];
	double rhs[1];
	char sense[1];
	char *rowname[1];
	int rmatind[1];
	double rmatval[1];

	rmatbeg[0] = 0;
	rowname[0] = CONCAT_STRINGS("x_", gprom_itoa(upNum), "_true");
	rmatind[0] = xindex;
	rmatval[0] = 1.0;
	sense[0] = 'E';
	rhs[0] = 1;

	status = CPXaddrows(env, lp, 0, 1, 1, rhs, sense, rmatbeg, rmatind, rmatval,
	NULL, rowname);

	if (status) {
		ERROR_LOG("Failure to set x for update %d True.\n", upNum);
	}
	return status;
}

static int exprToSymbol(int upNum, Node *expr, CPXENVptr env, CPXLPptr lp) {
	int status = 0;
	Node *left = NULL;
	char *selectAttName;
	List *sClause = NIL;

	switch (expr->type) {
	case T_Update:
		status = setX(upNum, 0, ((Update *) expr)->cond, env, lp);
		sClause = ((Update *) expr)->selectClause;
		FOREACH(SelectItem, s, sClause )
		{
			status = createU(upNum, s, env, lp);
			left = getHeadOfListP((List *) s->expr);
			selectAttName = ((AttributeReference *) left)->name;
			status = createV(upNum, selectAttName, env, lp);
			status = setAttUV(upNum, selectAttName, env, lp);
		}
		break;
	case T_Delete:
		status = setX(upNum, 0, ((Delete *) expr)->cond, env, lp);
		break;
	case T_Insert:
		break;
	default:
		break;
	}
	return status;
}

static int firstUpExe(int upNum, Node *expr, CPXENVptr env, CPXLPptr lp) {
	DEBUG_LOG("Processing the first update");
	int status = 0;
	List *sClause = NIL;

	sClause = ((Update *) expr)->selectClause;
	FOREACH(SelectItem, s, sClause )
	{
		int numRows = 1;
		List *result = NIL;
		result = getAttrNameFromOpExpList(result, (Operator *) s);
		int numZ = getListLength(result);
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];

		Node *left = NULL, *right = NULL, *l = NULL, *r = NULL;
		int i = 0, aIndex = 0, uIndex = 0;
		char *op;
		char *selectAttName;
		int neg = 1;

		//create first constraints for u: A1= mod(t).A0
		rmatbeg[0] = i;
		sense[0] = 'E';
		left = getHeadOfListP((List *) s->expr);
		selectAttName = ((AttributeReference *) left)->name;
		uIndex = getObjectIndex(
				CONCAT_STRINGS(selectAttName, "_", gprom_itoa(upNum)));
		rowname[0] = CONCAT_STRINGS(selectAttName, gprom_itoa(upNum));
		rmatind[i] = uIndex;
		rmatval[i] = 1.0;
		i++;
		right = (Node *) getTailOfListP((List *) s->expr);
		// ex. if A=10
		if (isA(right, Constant)) {
			rhs[0] = constrToDouble((Constant *) right);
			DEBUG_LOG(
					"Processed u for update %d to set %s attribute constant value %10f.\n",
					upNum, selectAttName, rhs[0]);
		} else {
			l = (Node *) getHeadOfListP(((Operator *) right)->args);
			r = (Node *) getTailOfListP(((Operator *) right)->args);
			op = ((Operator *) right)->name;
			if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0) {
				// ex. if A= A +2 which means A1= A0 +2 => u1<= A0 +2 => u1 - A0 <= 2
				neg = 1;
				if (strcmp(op, "-") == 0) {
					neg = -1;
				}
				if (isA(r, AttributeReference) && isA(l, Constant)) {
					aIndex = getObjectIndex(
							CONCAT_STRINGS(((AttributeReference * ) r)->name,
									"_", gprom_itoa(upNum - 1)));
					rmatind[i] = aIndex;
					rmatval[i] = -1;
					rhs[0] = constrToDouble((Constant *) l) * neg;

				} else if (isA(l, AttributeReference) && isA(r, Constant)) {
					aIndex = getObjectIndex(
							CONCAT_STRINGS(((AttributeReference * ) l)->name,
									"_", gprom_itoa(upNum - 1)));
					rmatind[i] = aIndex;
					rmatval[i] = -1;
					rhs[0] = constrToDouble((Constant *) r) * neg;
				}
			} else if (strcmp(op, "*") == 0) {
				if (isA(r, AttributeReference) && isA(l, Constant)) {
					aIndex = getObjectIndex(
							CONCAT_STRINGS(((AttributeReference * ) r)->name,
									"_", gprom_itoa(upNum - 1)));
					rmatind[i] = aIndex;
					rmatval[i] = -constrToDouble((Constant *) l);
					rhs[0] = 0;

				} else if (isA(l, AttributeReference) && isA(r, Constant)) {
					aIndex = getObjectIndex(
							CONCAT_STRINGS(((AttributeReference * ) l)->name,
									"_", gprom_itoa(upNum - 1)));
					rmatind[i] = aIndex;
					rmatval[i] = -constrToDouble((Constant *) r);
					rhs[0] = 0;
				}
			} else if (strcmp(op, "/") == 0) {
				if (isA(r, AttributeReference) && isA(l, Constant)) {
					aIndex = getObjectIndex(
							CONCAT_STRINGS(((AttributeReference * ) r)->name,
									"_", gprom_itoa(upNum - 1)));
					rmatind[i] = aIndex;
					rmatval[i] = -(1 / constrToDouble((Constant *) l));
					rhs[0] = 0;

				} else if (isA(l, AttributeReference) && isA(r, Constant)) {
					aIndex = getObjectIndex(
							CONCAT_STRINGS(((AttributeReference * ) l)->name,
									"_", gprom_itoa(upNum - 1)));
					rmatind[i] = aIndex;
					rmatval[i] = -(1 / constrToDouble((Constant *) r));
					rhs[0] = 0;
				}
			}
		}
		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		if (status) {
			ERROR_LOG(
					"Failure to convert update and add a row to cplex problem %d.\n",
					status);
		}
		//addToSet(newAttrSet, selectAttName);
		//increase lastIndex for the modified attribute
		aIndex = getObjectIndex(CONCAT_STRINGS(selectAttName, "_", gprom_itoa(0)));
		lastIndex[aIndex] = upNum;
	}
	return status;
}

static int firstInsExe(Node *up, CPXENVptr env, CPXLPptr lp) {
	DEBUG_LOG("Processing the first insert");
	int status = 0;
	int uIndex, aIndex;
	Set *newAttrSet = STRSET();
	//Set *tempSet = STRSET();
	List *query = (List *) ((Insert *) up)->query;
	List *attrList = ((Insert *) up)->attrList;

	int numRows = getListLength(attrList);
	int numZ = numRows;
	int rmatbeg[numRows];
	double rhs[numRows];
	char sense[numRows];
	char *rowname[numRows];
	int rmatind[numZ];
	double rmatval[numZ];

	int i = 0;
	char *attr;

	FOREACH(Constant,c,(List *)query)
	{

		rmatbeg[i] = i;
		rowname[i] = "row";
		attr = popHeadOfListP(attrList);
		addToSet(newAttrSet, attr);
		DEBUG_LOG("setting %s for insert operation.\n", attr);
		uIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(1)));
		DEBUG_LOG("index %d for insert operation.\n", uIndex);
		rowname[i] = CONCAT_STRINGS("ins", gprom_itoa(i));
		rmatind[i] = uIndex;
		rmatval[i] = 1.0;
		sense[i] = 'E';
		rhs[i] = constrToDouble((Constant *) c);
		i++;
		//increase lastIndex for the modified attribute
		aIndex = getObjectIndex(CONCAT_STRINGS(attr, "_", gprom_itoa(0)));
		lastIndex[aIndex] = 1;

	}
	DEBUG_LOG("develop rows for insert operation.\n");
	status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg, rmatind,
			rmatval, NULL, rowname);
	DEBUG_LOG("set next state for insert operation.\n");
	//tempSet = setDifference(attrSet, newAttrSet);
	//status = setNexSt(1, tempSet, env, lp);
	return status;
}

//TODO this creates AND runs the problem. This is the only implemented externally visible function. Would be better to structure this into several parts: compile expression to constraint, API for solving generated constraints, helper methods to manipule and output constraints. One option to consider is to introduce data structures in GProM to represent constraints, i.e., we compile: expr -> GProM LP datastructures -> cplex
List *symbolicHistoryExe(List *exprs) {
	List *depUps = NIL;
	int status = 0;
	//int solstat;
	//double objval;
	double *x = NULL;
	double *pi = NULL;
	double *slack = NULL;
	double *dj = NULL;
	int cur_numrows, cur_numcols;

	CPXENVptr env = NULL;
	CPXLPptr lp = NULL;
	CPXLPptr temp_lp = NULL;

	/* Initialize the CPLEX environment */
	env = CPXopenCPLEX(&status);

	if (env == NULL) {
		char errmsg[CPXMESSAGEBUFSIZE];
		CPXgeterrorstring(env, status, errmsg);
		ERROR_LOG("Could not open CPLEX environment.\n%s", errmsg);
	}

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

	status = CPXchgobjsen(env, lp, CPX_MAX); /* Problem is maximization */
	setSymbolicObjects(exprs);

	/* Now populate the problem with the data.  For building large
	 problems, consider setting the row, column and nonzero growth
	 parameters before performing this task. */

	status = CPXnewcols(env, lp, totalObjects, obj, lb, ub, ctype, colname);
	if (status) {
		ERROR_LOG("Failure to create cplex columns %d.\n", status);
	}
	DEBUG_LOG("after set seymbolic obj");
	/* Now populate the problem with the data.  For building large
	 problems, consider setting the row, column and nonzero growth
	 parameters before performing this task. */
	int i = 1;
	//int numUp = getListLength(exprs);
	Node *up = popHeadOfListP(exprs);
	if (up->type == T_Update) {
		status = setX(1, 0, ((Update *) up)->cond, env, lp);
		status = firstUpExe(1, up, env, lp);
		status = setXTrue(1, env, lp);
	} else if (up->type == T_Delete) {
		status = setX(1, 0, ((Delete *) up)->cond, env, lp);
		status = setXTrue(1, env, lp);
	} else if (up->type == T_Insert) {
		status = firstInsExe(up, env, lp);
	}
	depUps = appendToTailOfList(depUps, up);
	i++;

	FOREACH(Node,ex,exprs)
	{
		status = 0;
		DEBUG_LOG("symbolic execution for update %d.\n", i);
		status = exprToSymbol(i, ex, env, lp);
		//status = firstUpExe(i, ex, env, lp);
		if (ex->type == T_Update || ex->type == T_Delete) {
			temp_lp = CPXcloneprob(env, lp, &status);
			if (status) {
				DEBUG_LOG("Failed to clone problem.\n");

			} else
				DEBUG_LOG(
						"copied the main problem for the symbolic execution.\n");

			if (temp_lp == NULL) {
				ERROR_LOG("Failed to create temp_lp.\n");
			}

			status = setXTrue(i, env, temp_lp);
			if (status) {
				ERROR_LOG(
						"Failed to populate new temp problem for the condition of expression.\n");
			}

			//status = CPXlpopt(env, temp_lp);
			status = CPXmipopt(env, temp_lp);
			if (status) {
				ERROR_LOG("Failed to optimize LP.\n");
			}

			cur_numrows = CPXgetnumrows(env, temp_lp);
			cur_numcols = CPXgetnumcols(env, temp_lp);

			x = (double *) MALLOC(cur_numcols * sizeof(double));
			slack = (double *) MALLOC(cur_numrows * sizeof(double));
			dj = (double *) MALLOC(cur_numcols * sizeof(double));
			pi = (double *) MALLOC(cur_numrows * sizeof(double));

			if (x == NULL || slack == NULL || dj == NULL || pi == NULL) {
				ERROR_LOG("Could not allocate memory for solution.\n");
			}

			status = CPXgetstat(env, temp_lp);
			//status = CPXsolution(env, temp_lp, &solstat, &objval, x, pi, slack,dj);

			status = CPXgetx(env, temp_lp, x, 0, cur_numcols - 1);
			if (status) {
				DEBUG_LOG("False: Failed to obtain solution for update %d.\n",
						i);
				//result = FALSE;

			} else {
				DEBUG_LOG(
						"True: Obtained solution for update %d with templp.\n",
						i);
				//result = TRUE;
				depUps = appendToTailOfList(depUps, ex);
			}

			//DEBUG_LOG("\nSolution status = %d\n", solstat);
			//printf("Solution value  = %f\n\n", objval);
			int j;
			/*
			 for (j = 0; j < cur_numrows; j++) {
			 printf ("Row %d:  Slack = %10f  Pi = %10f\n", j, slack[j], pi[j]);
			 }
			 */
			for (j = 0; j < cur_numcols; j++) {
				DEBUG_LOG("Column %s:  Value = %10f \n", colname[j], x[j]);
			}

			//}
			if (x != NULL) {
				FREE(x);
				x = NULL;
			}
			if (slack != NULL) {
				FREE(slack);
				slack = NULL;
			}
			if (dj != NULL) {
				FREE(dj);
				dj = NULL;
			}
			if (pi != NULL) {
				FREE(pi);
				pi = NULL;
			}
/*
			 if (temp_lp != NULL) {
			 status = CPXfreeprob(env, &temp_lp);
			 if (status) {
			 DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
			 }
			 }
*/
		}
		i++;
	}

	DEBUG_LOG("End of symbolic execution for finding dependent updates .\n");

	//TERMINATE:
	/* Free up the problem as allocated by CPXcreateprob, if necessary */

	if (lp != NULL) {
		status = CPXfreeprob(env, &lp);
		if (status) {
			DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
		}
	}

	if (temp_lp != NULL) {
		status = CPXfreeprob(env, &temp_lp);
		if (status) {
			DEBUG_LOG("CPXfreeprob failed, error code %d.\n", status);
		}

	}

	DEBUG_LOG("free tempCPXfreeprob.\n");

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

	deleteCplexObjects();
	DEBUG_LOG("deleteCplexObjects.\n");

	DEBUG_LOG("Returning dependent updates.\n");
	return depUps;
}

// ********************************************************************************
// dummy replacement if cplex is not available

// have dummy stub to keep compiler quiet if we do not have cplex
#else

boolean
exprToSat(Node *expr1, boolean inv1, Node *expr2, boolean inv2)
{
	return TRUE;
}

#endif
