/*-----------------------------------------------------------------------------
 *
 * expr_to_constraint.c
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
#include "model/set/hashmap.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"

static CplexObjects *cplexObjects = NULL; // global pointer to current cplex objects
static objectIndex = 0;
static default_lb = 0;
static default_ub = CPX_MAX;

static void setCplexObjects(char *tbName);
static int convertUpdate(Update* f, CPXENVptr env, CPXLPptr lp);
static int convertDelete(Delete * f, CPXENVptr env, CPXLPptr lp);
static int convertInsert(Insert* f, CPXENVptr env, CPXLPptr lp);
static int getObjectIndex(char * attrName);
static int condToConstr(Node * cond, CPXENVptr env, CPXLPptr lp);
static List *getOpExpStack(List *stackList, Operator *opExpList);
static int setToConstr(List * attrList, Node * query, CPXENVptr env,
		CPXLPptr lp);
static int invertCondToConstr(Update* f, CPXENVptr env, CPXLPptr lp);

void setCplexObjects(char *tbName) {
	if (cplexObjects == NULL) {
		cplexObjects = NEW(CplexObjects);
		cplexObjects->tableName = tbName;
		cplexObjects->attrIndex = NEW_MAP(Constant, Constant);
		cplexObjects->obj = (double*) malloc(DEFAULT_NUM_COLS * sizeof(double))
		;
		cplexObjects->lb = (double*) malloc(DEFAULT_NUM_COLS * sizeof(double))
		;
		cplexObjects->ub = (double*) malloc(DEFAULT_NUM_COLS * sizeof(double))
		;
		cplexObjects->colname = (char **) malloc(
				DEFAULT_NUM_COLS * sizeof(char*))
		;
	}
}

static int exprToEval(Node *expr, CPXENVptr env, CPXLPptr lp) {
	int status = 0;

	switch (n->type) {
	case T_Update:
		status = convertUpdate((Update *) n, CPXENVptr env, CPXLPptr lp);
		break;
	case T_Delete:
		status = convertDelete((Delete *) n, CPXENVptr env, CPXLPptr lp);
		break;
	case T_Insert:
		status = convertInsert((Insert *) n, CPXENVptr env, CPXLPptr lp);
		break;
	}

	return status;
}

static int convertUpdate(Update* f, CPXENVptr env, CPXLPptr lp) {
	setCplexObjects(f->updateTableName);
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

	//there is not any common attribute in setClause and condition
	if (!isFound)
		return condToConstr(f->cond, env, lp);
	else
		return invertCondToConstr(f ,env, lp);

}
static int convertDelete(Delete * f, CPXENVptr env, CPXLPptr lp) {
	setCplexObjects(f->deleteTableName);
	return condToConstr(f->cond, env, lp);

}
static int convertInsert(Insert* f, CPXENVptr env, CPXLPptr lp) {
	setCplexObjects(f->insertTableName);
	return setToConstr(f->attrList, f->query, env, lp);
}

static int getObjectIndex(char * attrName) {

	int index = 0;

	if (!MAP_HAS_STRING_KEY(cplexObjects->attrIndex, attrName)) {
		MAP_ADD_STRING_KEY(cplexObjects->attrIndex, attrName, objectIndex);
		cplexObjects->colname[objectIndex] = attrName;
		cplexObjects->lb[objectIndex] = default_lb;
		cplexObjects->ub[objectIndex] = default_ub;
		cplexObjects->obj[objectIndex] = 1;
		index = objectIndex;
		objectIndex++;
	} else {
		index = INT_VALUE(MAP_GET_INT(cplexObjects->attrIndex,attrName));
	}
	return index;

}

static int condToConstr(Node * cond, CPXENVptr env, CPXLPptr lp) {

	int status = 0;
	List *result = NIL;
	if (isA(cond, Operator)) {
		result = getAttrNameFromOpExpList(result, (Operator *) cond);
	}

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

	switch (((Operator *) cond)->name) {
	case "=":
		sense[0] = 'E';
		break;
	case ">":
	case ">=":
		sense[0] = 'G';
		break;
	case "<":
	case "<=":
		sense[0] = 'L';
		break;
	}

	Node *left = (Node *) getHeadOfListP(((Operator *) cond)->args);
	Node *right = (Node *) getTailOfListP(((Operator *) cond)->args);
	rhs[0] = (double *) (((Constant *) right)->value);

	//if the condition likes c>10
	if (isA(right, AttributeReference)) {
		//we have just one attribute
		objIndex = getObjectIndex(((AttributeReference *) right)->name);
		rmatind[0] = objIndex;
		rmatval[0] = 1.0;
	}
	//else every attr should has a coefficient for example 1*a+2*b>10
	else if (isA(right, Operator)) {
		int i = 0;
		int pluse = 1;
		List *stack = NIL;
		stack = getOpExpStack(stack, (Operator *) right);
		int last = getListLength(stack) - 1;
		Node *sign, *right, *left, *op = NIL;
		while (last > i) {
			pluse = 1;
			right = (Node *) getNthOfListP(stack, last--);
			left = (Node *) getNthOfListP(stack, last--);
			op = (Node *) getNthOfListP(stack, last--);
			if (last > i) {
				sign = (Node *) getNthOfListP(stack, i);
				if (streq(((Operator *) sign)->name, "-"))
					pluse = -1;
			}
			//opName should be * based on the creation rule
			//if (streq(opName, "*")) {
			if (isA(right, AttributeReference) && isA(left, Constant)) {
				objIndex = getObjectIndex(((AttributeReference *) right)->name);
				rmatind[i] = objIndex;
				rmatval[i] = (double *) (((Constant *) left)->value) * pluse;
			} else if (isA(left, AttributeReference) && isA(right, Constant)) {
				objIndex = getObjectIndex(((AttributeReference *) left)->name);
				rmatind[i] = objIndex;
				rmatval[i] = (double *) (((Constant *) right)->value) * pluse;

			}
			i++;
			//}
		}

		/*
		 //cond OR and AND wasn't considered
		 else if(streq(o->name, "AND") || streq(o->name, "OR"))
		 {

		 return;
		 }
		 */

		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		return status;

	}

	static List *
	getOpExpStack(List *stackList, Operator *opExpList) {

		stackList = appendToTailOfList(opExpList);
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

	static int setToConstr(List * attrList, Node * query, CPXENVptr env,
			CPXLPptr lp) {

		int status = 0;

		int numCols = getListLength(attrList);

		int objIndex = 0;
		int numRows = numCols;
		int numZ = numCols;
		int rmatbeg[numRows];
		double rhs[numRows];
		char sense[numRows];
		char *rowname[numRows];
		int rmatind[numZ];
		double rmatval[numZ];

		int i, j = 0;
		char *attr;

		FOREACH(Constant,c,query)
		{
			if (isA(c->constType,
					INT) || isA(c->constType, FLOAT) || isA(c->constType, LONG)) {
				rmatbeg[i] = i;
				rowname[i] = "row";
				attr = (char *) getNthOfListP(attrList, j);
				rmatind[i] = getObjectIndex(attr);
				rmatval[i] = 1.0;
				sense[i] = 'E';
				rhs[i] = (double *) c.value;
				i++;
			}
			j++;
		}

		status = CPXaddrows(env, lp, 0, numRows, numZ, rhs, sense, rmatbeg,
				rmatind, rmatval, NULL, rowname);

		return status;

	}

	//to be done
	static int invertCondToConstr(Update* f, CPXENVptr env, CPXLPptr lp) {
		int status = 0;
		return status;
	}
