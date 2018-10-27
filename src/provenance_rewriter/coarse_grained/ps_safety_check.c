/*
 * ps_safety_check.c
 *
 *  Created on: 2018年10月25日
 *      Author: liuziyu
 */
#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/ps_safety_check.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"

void monotone(){
	DEBUG_LOG("test123lzy");
}
void
monotoneCheck(Node *qbModel)
{
	/*
	DEBUG_LOG("Safety Check");
	//DEBUG_NODE_BEATIFY_LOG("check query: ", q);
	QueryOperator *op = NULL;

	if (isA(qbModel, List))
		op = getNthOfListP((List *) qbModel, 0);
	else if (IS_OP(qbModel))
		op = (QueryOperator *) qbModel;
	int a = 0;
	int * num = &a;
	checkM(op, num);
	DEBUG_LOG("num is %d", *num);

	//DEBUG_NODE_BEATIFY_LOG("check query: ", op);
	*/
	DEBUG_LOG("Safety Check");
	int value = 0;
	int * state = &value;
	check(qbModel,state);
	DEBUG_LOG("num is %d", *state);


}

boolean
check(Node* node, int *state)
{
	if(node == NULL)
		return TRUE;

	if(isA(node, JoinOperator)){
		(*state)++;
	}

	return visit(node, check, state);
}

void
checkM(QueryOperator* op, int * num)
{
	if(isA(op, JoinOperator))
	{
		DEBUG_LOG("find join");
		*num = *num + 1;

	}

	FOREACH(QueryOperator, o, op->inputs)
	{

		checkM(o, num);
	}

	DEBUG_LOG("not join");
}
