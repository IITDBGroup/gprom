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

HashMap*
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
	*/
	DEBUG_LOG("Safety Check");
	HashMap *checkResult = NEW_MAP(Constant,Node);
	HashMap *operatorState = NEW_MAP(Constant,Constant);
	check(qbModel, operatorState);
	//DEBUG_NODE_BEATIFY_LOG("The result_state is:",state);
	List *entries = getEntries(operatorState);//get all operator in the tree.
	if(entries == NIL){
		DEBUG_LOG("It's Monotone");
		checkResult = getMonotoneResultMap(qbModel); //get the table schema for all sketches
		DEBUG_NODE_BEATIFY_LOG("The result_map is:",checkResult);
		return checkResult;
	}else{
		DEBUG_LOG("It isn't Monotone");
		char *WindowOperator = "WindowOperator";
		if(hasMapStringKey(operatorState, WindowOperator)){
			checkResult = safetyCheck(qbModel, WindowOperator);
			//checkResult = safetyCheck_windowOperator(qbModel);
		}else{
			char *hasAggregation = "aggregation";
			checkResult = safetyCheck(qbModel, hasAggregation);
			//checkResult = safetyCheck_aggregation(qbModel);
		}
		DEBUG_NODE_BEATIFY_LOG("The result_map is:",checkResult);
		return checkResult;
	}
}//check whether it is monotone

boolean
check(Node* node, HashMap *state)
{
	if(node == NULL)
		return TRUE;
	if(isA(node, AggregationOperator)){
		char *AggregationOperator = "AggregationOperator";
		MAP_ADD_STRING_KEY(state, AggregationOperator, createConstInt (1));
	} //Check aggreationOperator
	if(isA(node, WindowOperator)){
		char *WindowOperator = "WindowOperator";
		MAP_ADD_STRING_KEY(state, WindowOperator, createConstInt (1));
	}//Check WindowOperator
	if(isA(node, SetOperator)){
		if(((SetOperator *) node)->setOpType == SETOP_DIFFERENCE){
			char *SetOperator = "SetOperator";
			MAP_ADD_STRING_KEY(state, SetOperator, createConstInt (1));
		}
	}//Check set difference
	if(isA(node, JoinOperator)){
		JoinOperator *j = (JoinOperator *) node;
		if(j->joinType == JOIN_LEFT_OUTER || j->joinType == JOIN_RIGHT_OUTER || j->joinType == JOIN_FULL_OUTER){
			char *JoinOperator = "JoinOperator";
			MAP_ADD_STRING_KEY(state, JoinOperator, createConstInt (1));
		}
	}//Check outer join
	if(isA(node, NestingOperator)){
		char *NestingOperator = "NestingOperator";
		MAP_ADD_STRING_KEY(state, NestingOperator, createConstInt (1));
	}//Check nesting
	return visit(node, check, state);
}


HashMap *
getSchema(Node* qbModel){
	HashMap *map = NEW_MAP(Constant,Node);
	getTableAccessOperator(qbModel, map);
	return map;
}//get schema of table


HashMap *
getMonotoneResultMap(Node* qbModel) {
	HashMap *map = NEW_MAP(Constant, Node);
	char *PAGE = "PAGE";
	Constant *isSafe = createConstInt(1);
	MAP_ADD_STRING_KEY(map, PAGE, (Node * ) isSafe);
	HashMap *schema_map = getSchema(qbModel);
	char *RANGE = "RANGE";
	MAP_ADD_STRING_KEY(map, RANGE, (Node * ) schema_map);
	char *BLOOM_FILTER = "BLOOM_FILTER";
	MAP_ADD_STRING_KEY(map, BLOOM_FILTER, (Node * ) schema_map);
	char *HASH = "HASH";
	MAP_ADD_STRING_KEY(map, HASH, (Node * ) schema_map);
	return map;
}//return the result map for all sketches.


HashMap *safetyCheck(Node* qbModel, char *hasOpeator) {
	HashMap *map = NEW_MAP(Constant, Node);
	HashMap *data = NEW_MAP(Constant, Node);
	getData(qbModel, data);

	boolean result = FALSE;
	result = checkPageSafety(data, hasOpeator);

	char *PAGE = "PAGE";
	if (!result) {
		Constant *isSafe = createConstInt(0);
		MAP_ADD_STRING_KEY(map, PAGE, (Node * ) isSafe);
	} else {
		Constant *isSafe = createConstInt(1);
		MAP_ADD_STRING_KEY(map, PAGE, (Node * ) isSafe);
	}
	HashMap *schema_map = getSchema(qbModel);
	char *RANGE = "RANGE";
	MAP_ADD_STRING_KEY(map, RANGE, (Node * ) schema_map);
	char *BLOOM_FILTER = "BLOOM_FILTER";
	MAP_ADD_STRING_KEY(map, BLOOM_FILTER, (Node * ) schema_map);
	char *HASH = "HASH";
	MAP_ADD_STRING_KEY(map, HASH, (Node * ) schema_map);
	return map;
}



boolean
getTableAccessOperator(Node* node, HashMap *map)
{
	if(node == NULL)
		return TRUE;

	if(isA(node, TableAccessOperator)){
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		MAP_ADD_STRING_KEY(map, tablename, (Node *)attrDef);
	}

	return visit(node, getTableAccessOperator, map);
}//get the table


boolean
getData(Node* node, HashMap *data)
{
	if(node == NULL)
		return TRUE;

	if(isA(node, AggregationOperator)){
		char *aggregation_key = "aggregation";
		HashMap *aggreation_map = NEW_MAP(Constant,Node);

		char *aggrs_key = "aggrs";
		char *groupby_key = "groupby";
		List *aggrs = ((AggregationOperator *) node)->aggrs;
		List *groupby = ((AggregationOperator *) node)->groupBy;

		MAP_ADD_STRING_KEY(aggreation_map, aggrs_key, (Node *)aggrs);
		MAP_ADD_STRING_KEY(aggreation_map, groupby_key, (Node *)groupby);

		MAP_ADD_STRING_KEY(data, aggregation_key, (Node *)aggreation_map);
	}
	if(isA(node, SelectionOperator)){
		char *SelectionOperator_key = "SelectionOperator";

		Node *cond = ((SelectionOperator *) node)->cond;
		MAP_ADD_STRING_KEY(data, SelectionOperator_key, (Node *)cond);
	}
	if(isA(node, WindowOperator)){
		char *WindowOperator_key = "WindowOperator";
		HashMap *WindowOperator_map = NEW_MAP(Constant,Node);

		char *f_key = "f";
		char *partitionBy_key = "partitionBy";
		Node *f = ((WindowOperator *) node)->f;
		List *partitionBy = ((WindowOperator *) node)->partitionBy;

		MAP_ADD_STRING_KEY(WindowOperator_map, f_key, (Node *)f);
		MAP_ADD_STRING_KEY(WindowOperator_map, partitionBy_key, (Node *)partitionBy);

		MAP_ADD_STRING_KEY(data, WindowOperator_key, (Node *)WindowOperator_map);
	}
	return visit(node, getData, data);
}


boolean checkPageSafety(HashMap *data, char *hasOpeator)
{
	char *function_name;
	if(!strcmp(hasOpeator, "WindowOperator")){
		char *WindowOperator_key = "WindowOperator";
		HashMap *WindowOperator_map = (HashMap *) MAP_GET_STRING_ENTRY(data, WindowOperator_key)->value;
		char *f_key = "f";
		Node *f = (Node *) MAP_GET_STRING_ENTRY(WindowOperator_map, f_key)->value;
		function_name =((FunctionCall *) f)->functionname;
	}
	if(!strcmp(hasOpeator, "aggregation")){
		char *aggregation_key = "aggregation";
		HashMap *aggreation_map = (HashMap *) MAP_GET_STRING_ENTRY(data, aggregation_key)->value;

		char *aggrs_key = "aggrs";
		List *aggrs = (List *) MAP_GET_STRING_ENTRY(aggreation_map, aggrs_key)->value;
		function_name = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	}

	char *SelectionOperator_key = "SelectionOperator";
	Node *cond = MAP_GET_STRING_ENTRY(data, SelectionOperator_key)->value;
	char *operator_name = ((Operator *) cond)->name;

	if (!strcmp(function_name, "SUM")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return TRUE;
			}
			if (!strcmp(operator_name, ">=")) {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "AVG")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">=")) {
				return FALSE;
			}
		}
		if (!strcmp(function_name, "COUNT")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return TRUE;
			}
			if (!strcmp(operator_name, ">=")) {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "MAX")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return TRUE;
			}
			if (!strcmp(operator_name, ">=")) {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "MIN")) {
			if (!strcmp(operator_name, "<")) {
				return TRUE;
			}
			if (!strcmp(operator_name, "<=")) {
				return TRUE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">=")) {
				return FALSE;
			}
		}
		return FALSE;
}

/*
HashMap *
safetyCheck_aggregation(Node* qbModel){
	HashMap *map = NEW_MAP(Constant,Node);
	HashMap *data = NEW_MAP(Constant,Node);
	getData_aggregation(qbModel, data);
	//char *page = "PAGE";
	boolean result = FALSE;
	result = checkPageSafety_aggregation(data);
	char *PAGE = "PAGE";
	if(!result){
		Constant *isSafe = createConstInt (0);
		MAP_ADD_STRING_KEY(map, PAGE, (Node *) isSafe);
	}else{
		Constant *isSafe = createConstInt (1);
		MAP_ADD_STRING_KEY(map, PAGE, (Node *) isSafe);
	}
	HashMap *schema_map = getSchema(qbModel);
	char *RANGE = "RANGE";
	MAP_ADD_STRING_KEY(map, RANGE, (Node *) schema_map);
	char *BLOOM_FILTER = "BLOOM_FILTER";
	MAP_ADD_STRING_KEY(map, BLOOM_FILTER, (Node *) schema_map);
	char *HASH = "HASH";
	MAP_ADD_STRING_KEY(map, HASH, (Node *) schema_map);
	return map;
}//if it isn't Monotone and has aggregation.

HashMap *
safetyCheck_windowOperator(Node* qbModel){
	HashMap *map = NEW_MAP(Constant,Node);
	HashMap *data = NEW_MAP(Constant,Node);
	getData_windowOperator(qbModel, data);
	//char *page = "PAGE";
	boolean result = FALSE;
	result = checkPageSafety_windowOperator(data);
	char *PAGE = "PAGE";
	if(!result){
		Constant *isSafe = createConstInt (0);
		MAP_ADD_STRING_KEY(map, PAGE, (Node *) isSafe);
	}else{
		Constant *isSafe = createConstInt (1);
		MAP_ADD_STRING_KEY(map, PAGE, (Node *) isSafe);
	}
	HashMap *schema_map = getSchema(qbModel);
	char *RANGE = "RANGE";
	MAP_ADD_STRING_KEY(map, RANGE, (Node *) schema_map);
	char *BLOOM_FILTER = "BLOOM_FILTER";
	MAP_ADD_STRING_KEY(map, BLOOM_FILTER, (Node *) schema_map);
	char *HASH = "HASH";
	MAP_ADD_STRING_KEY(map, HASH, (Node *) schema_map);
	return map;
}
*/



/*
boolean
getData_aggregation(Node* node, HashMap *data)
{
	if(node == NULL)
		return TRUE;

	if(isA(node, AggregationOperator)){
		char *aggrs_key = "aggrs";
		//char *groupby_key = "groupby";
		List *aggrs = ((AggregationOperator *) node)->aggrs;
		//List *groupby = ((AggregationOperator *) node)->groupBy;
		MAP_ADD_STRING_KEY(data, aggrs_key, (Node *)aggrs);
		//MAP_ADD_STRING_KEY(data, groupby_key, (Node *)groupby);
	}
	if(isA(node, SelectionOperator)){
		char *cond_key = "cond";
		Node *cond = ((SelectionOperator *) node)->cond;
		MAP_ADD_STRING_KEY(data, cond_key, (Node *)cond);
	}
	return visit(node, getData_aggregation, data);
}// get the data of aggregation and selection if there is a aggregation operator.

boolean checkPageSafety_aggregation(HashMap *data) {
	char *aggrs_key = "aggrs";
	//char *groupby_key = "groupby";
	char *cond_key = "cond";
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(data, aggrs_key)->value;
	//List *groupby = (List *)MAP_GET_STRING_ENTRY(data, groupby_key)->value;
	Node *cond = MAP_GET_STRING_ENTRY(data, cond_key)->value;

	char *function_name = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	char *operator_name = ((Operator *) cond)->name;
	if (!strcmp(function_name, "SUM")) {
		if (!strcmp(operator_name, "<")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "<=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">")) {
			return TRUE;
		}
		if (!strcmp(operator_name, ">=")) {
			return TRUE;
		}
	}
	if (!strcmp(function_name, "AVG")) {
		if (!strcmp(operator_name, "<")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "<=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">=")) {
			return FALSE;
		}
	}
	if (!strcmp(function_name, "COUNT")) {
		if (!strcmp(operator_name, "<")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "<=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">")) {
			return TRUE;
		}
		if (!strcmp(operator_name, ">=")) {
			return TRUE;
		}
	}
	if (!strcmp(function_name, "MAX")) {
		if (!strcmp(operator_name, "<")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "<=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, "=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">")) {
			return TRUE;
		}
		if (!strcmp(operator_name, ">=")) {
			return TRUE;
		}
	}
	if (!strcmp(function_name, "MIN")) {
		if (!strcmp(operator_name, "<")) {
			return TRUE;
		}
		if (!strcmp(operator_name, "<=")) {
			return TRUE;
		}
		if (!strcmp(operator_name, "=")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">")) {
			return FALSE;
		}
		if (!strcmp(operator_name, ">=")) {
			return FALSE;
		}
	}
	return FALSE;
}

boolean
getData_windowOperator(Node* node, HashMap *data)
{
	if(node == NULL)
		return TRUE;

	if(isA(node, WindowOperator)){
		char *f_key = "f";
		//char *groupby_key = "groupby";
		Node *f = ((WindowOperator *) node)->f;
		//List *groupby = ((AggregationOperator *) node)->groupBy;
		MAP_ADD_STRING_KEY(data, f_key, (Node *)f);
		//MAP_ADD_STRING_KEY(data, groupby_key, (Node *)groupby);
	}
	if(isA(node, SelectionOperator)){
		char *cond_key = "cond";
		Node *cond = ((SelectionOperator *) node)->cond;
		MAP_ADD_STRING_KEY(data, cond_key, (Node *)cond);
	}
	return visit(node, getData_windowOperator, data);
}// get the data of aggregation and selection if there is a aggregation operator.

boolean checkPageSafety_windowOperator(HashMap *data)
{
	char *f_key = "f";
	//char *groupby_key = "groupby";
	char *cond_key = "cond";
	Node *f = (Node *) MAP_GET_STRING_ENTRY(data, f_key)->value;
	//List *groupby = (List *)MAP_GET_STRING_ENTRY(data, groupby_key)->value;
	Node *cond = MAP_GET_STRING_ENTRY(data, cond_key)->value;

	char *function_name =((FunctionCall *) f)->functionname;
	char *operator_name = ((Operator *) cond)->name;
	if (!strcmp(function_name, "SUM")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return TRUE;
			}
			if (!strcmp(operator_name, ">=")) {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "AVG")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">=")) {
				return FALSE;
			}
		}
		if (!strcmp(function_name, "COUNT")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return TRUE;
			}
			if (!strcmp(operator_name, ">=")) {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "MAX")) {
			if (!strcmp(operator_name, "<")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "<=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return TRUE;
			}
			if (!strcmp(operator_name, ">=")) {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "MIN")) {
			if (!strcmp(operator_name, "<")) {
				return TRUE;
			}
			if (!strcmp(operator_name, "<=")) {
				return TRUE;
			}
			if (!strcmp(operator_name, "=")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">")) {
				return FALSE;
			}
			if (!strcmp(operator_name, ">=")) {
				return FALSE;
			}
		}
		return FALSE;
}
*/

/*
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
*/
