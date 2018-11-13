/*
 * unnesting.c
 *
 *  Created on: Oct 25, 2018
 *      Author: pengyuanli
 */
/*
 * provenance_rewriter/lateral_rewrites/lateral_prov_main.h
 * */
#include "common.h"
#include "provenance_rewriter/lateral_rewrites/unnesting.h"
#include "model/node/nodetype.h"

#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "utility/string_utils.h"

static boolean getNestingList(Node *node, List **l);
//static void makeJoin(NestingOperator * nestOp);

/*
 * A simple unnesting method for "SELECT * FROM R WHERE A IN (SELECT C FROM S);"
 */
Node *
unnesting(Node *oModel) {

	QueryOperator *op = NULL;
	if (isA(oModel, List)) {
		op = (QueryOperator *) getHeadOfListP((List *) oModel);
	} else if (IS_OP(oModel)) //isA(oModel, QueryOpertor)
			{
		op = (QueryOperator *) oModel;
	}
	DEBUG_LOG("Starting find NestingOperator");
	List *l = NIL;
	getNestingList((Node *) op, &l);

	if (l != NIL) {
		FOREACH(NestingOperator, nestOp, l)
		{

			DEBUG_NODE_BEATIFY_LOG("Check Nesting Operator:", nestOp);
		}
	} else
		DEBUG_LOG("Check Nesting Operator: empty list");

	DEBUG_LOG("STARTING MAKEING JOIN");
	//get the already find NestOperator;
	NestingOperator * nestOp = (NestingOperator *) getHeadOfListP(l);

	//create a JoinOperator

	/*	 createJoinOp(JoinType joinType, Node *cond, List *inputs, List *parents,
    List *attrNames)

    */


//	DEBUG_LOG("NestingOperator inputs:");
//	FOREACH(Node, a, ((QueryOperator *)nestOp)->inputs){
//		DEBUG_NODE_LOG("listNode", a);
//	}


//	JoinOperator * joinOp = createJoinOp(JOIN_INNER, copyObject(nestOp->cond),
//			(List *) ((QueryOperator *) nestOp)->inputs, NIL,
//			((QueryOperator *) nestOp)->provAttrs);

	JoinOperator * joinOp = createJoinOp(JOIN_INNER, copyObject(nestOp->cond),
				(List *) ((QueryOperator *) nestOp)->inputs, NIL, NIL);
	//get the parent of NestingOperator which is SelsctionOpetarot;
	SelectionOperator * selOp = (SelectionOperator *) getHeadOfListP(
			((QueryOperator*)nestOp)->parents);

	//set JoinOperator's parent list to SelectionOperator's parent list
	((QueryOperator*)joinOp)->parents = ((QueryOperator *) selOp)->parents;

	//get the SelectOperator's parent;
	ProjectionOperator * proOp = (ProjectionOperator *) getHeadOfListP(
			((QueryOperator *)selOp)->parents);

	//set SelectOperator's parent's input to list contains JoinOperator;
	List * inputList = NIL;
	inputList = appendToTailOfList(inputList, joinOp);
	((QueryOperator *)proOp)->inputs = inputList;

	FOREACH(Node, n, ((QueryOperator *)nestOp)->inputs) {
		((QueryOperator *) n)->parents = inputList;
	}

	DEBUG_LOG("FINISHED");


	return oModel;


}

static
boolean getNestingList(Node *node, List **l) {

	if (node == NULL)
		return TRUE;

	//QueryOperator * qOp = (QueryOperator *) op;
	//List *l = (List *) context;

	if (isA(node, NestingOperator)) {
		NestingOperator *nestOp = (NestingOperator *) node;
		*l = appendToTailOfList(*l, nestOp);
		DEBUG_LOG("NestingOperator found");
		//(List*)context = appendToTailOfList((List*)context, nestOp);
	}

	return visit(node, getNestingList, l);
	//return visit(node, getNestingList, context);
}


