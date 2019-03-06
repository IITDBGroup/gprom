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
#include "metadata_lookup/metadata_lookup.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include <string.h>


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
/*
	boolean lzy = isPostive("R", "A");
	DEBUG_LOG("The lzy is %d", lzy);
*/

	//HashMap *operatorState = NEW_MAP(Constant,Node);
	//QueryOperator *root = (QueryOperator *) getHeadOfList((List*) qbModel)->data.ptr_value;
	//computeChildOperatorProp(root);
	//computeMinMaxProp(root);
	//HashMap *min_max = getMinAndMax("R","A");
	//DEBUG_NODE_BEATIFY_LOG("The min_max is:",min_max);
	//DEBUG_LOG("The negative is: %d", isNegative("R","A"));
	//check(qbModel, operatorState);
	//DEBUG_NODE_BEATIFY_LOG("The result_state is:",operatorState);
	//List *entries = getEntries(operatorState);//get all operator in the tree.
	HashMap *checkResult = NEW_MAP(Constant,Node);
	Set *operatorSet = STRSET();
	checkMonotone(qbModel, operatorSet);
	if(setSize(operatorSet) == 0){
		DEBUG_LOG("It's Monotone");
		checkResult = getMonotoneResultMap(qbModel); //get the table schema for all sketches
		DEBUG_NODE_BEATIFY_LOG("The result_map is:",checkResult);
		return checkResult;
	}else{
		DEBUG_LOG("It isn't Monotone");
		//char *WindowOperator = "WindowOperator";

		if(hasSetElem(operatorSet, WINDOW_OPERATOR)){
			checkResult = safetyCheck(qbModel, WINDOW_OPERATOR);
			//checkResult = safetyCheck_windowOperator(qbModel);
		}else{
			Set *findOrder = STRSET();
			hasOrder(qbModel, findOrder);
			if(hasSetElem(findOrder, ORDER_OPERATOR)){
				checkResult = safetyCheck(qbModel, ORDER_OPERATOR);
			}else{
				checkResult = safetyCheck(qbModel, AGGREGATION_OPERATOR);
			}
		}
		DEBUG_NODE_BEATIFY_LOG("The result_map is:",checkResult);
		return checkResult;
	}
}//check whether it is monotone

boolean
checkMonotone(Node* node, Set *operatorSet){
	if(node == NULL)
			return TRUE;
		if(node->type == T_AggregationOperator){
			addToSet(operatorSet, AGGREGATION_OPERATOR);
		} //Check aggreationOperator
		if(node->type == T_WindowOperator){
			addToSet(operatorSet, WINDOW_OPERATOR);
		}//Check WindowOperator
		if(node->type == T_SetOperator){
			if(((SetOperator *) node)->setOpType == SETOP_DIFFERENCE){
				addToSet(operatorSet, SET_OPERATOR);
			}
		}//Check set difference
		if(node->type == T_JoinOperator){
			JoinOperator *j = (JoinOperator *) node;
			if(j->joinType == JOIN_LEFT_OUTER || j->joinType == JOIN_RIGHT_OUTER || j->joinType == JOIN_FULL_OUTER){
				addToSet(operatorSet, JOIN_OPERATOR);
			}
		}//Check outer join
		if(node->type == T_NestingOperator){
			addToSet(operatorSet, NESTING_OPERATOR);
		}//Check nesting

		return visit(node, checkMonotone, operatorSet);
}


HashMap *
getSchema(Node* qbModel){
	HashMap *map = NEW_MAP(Constant,Node);
	//getTableAccessOperator(qbModel, map);
	getSubset(qbModel, map);
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


HashMap *
safetyCheck(Node* qbModel, char *hasOpeator) {
	HashMap *map = NEW_MAP(Constant, Node);
	HashMap *data = NEW_MAP(Constant, Node);
	getData(qbModel, data);//get the data of node we need

	boolean result = FALSE;
	if(!strcmp(hasOpeator,ORDER_OPERATOR)){
		//DEBUG_LOG("it's order");
		result = checkPageSafety_rownum(data);		//rownum check
		DEBUG_LOG("The result is: %d", result);
	}else{
		//DEBUG_LOG("it's no order");
		result = checkPageSafety(data, hasOpeator); // window and aggregation
		DEBUG_LOG("The result is: %d", result);
	}

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

	if(node->type == T_TableAccessOperator){
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		MAP_ADD_STRING_KEY(map, tablename, (Node *)attrDef);
	}

	return visit(node, getTableAccessOperator, map);
}//get the table

boolean
getSubset(Node* node, HashMap *map)
{
	if(node == NULL)
		return TRUE;

	if(node->type == T_TableAccessOperator){
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		int length = getListLength(attrDef);
		List *result = NIL;
		result = addBitset(length, result);
		MAP_ADD_STRING_KEY(map, tablename, (Node *)result);
	}

	return visit(node, getSubset, map);
}//get the KeyValue of each table

List*
addBitset(int length, List *result)
{
	char *subset = "SUBSET";
	char *exact = "EXCAT";
	int max = 1 << length;
	for (int i = 1; i < max; i++) {
		if (i == (max - 1)){
			KeyValue *element = createStringKeyValue(exact, binDis(length, i));
			result = appendToTailOfList(result, element);
			break;
		}
		KeyValue *element = createStringKeyValue(subset, binDis(length, i));
		result = appendToTailOfList(result, element);
	}
	return result;
}//get BitSet of all subsets

char*
binDis(int length, int value)
{
	//List *stringList = NIL;
	StringInfo stringResult  = makeStringInfo();
	while(length--)
	{
		if(value&1<<length){
			char *bit = "1";
			appendStringInfoString(stringResult, bit);
			//appendToTailOfList(stringList, bit);
		}else{
			char *bit = "0";
			appendStringInfoString(stringResult, bit);
			//appendToTailOfList(stringList, bit);
		}
	}
	//DEBUG_LOG("The bin is: %s", stringResult->data);
	return stringResult->data;
}


boolean
getData(Node* node, HashMap *data)
{
	if(node == NULL)
		return TRUE;

	if(node->type == T_AggregationOperator){
		//char *aggregation_key = "aggregation";
		HashMap *aggreation_map = NEW_MAP(Constant,Node);

		char *aggrs_key = "aggrs";
		char *groupby_key = "groupby";
		List *aggrs = ((AggregationOperator *) node)->aggrs;
		List *groupby = ((AggregationOperator *) node)->groupBy;

		MAP_ADD_STRING_KEY(aggreation_map, aggrs_key, (Node *)aggrs);
		MAP_ADD_STRING_KEY(aggreation_map, groupby_key, (Node *)groupby);

		MAP_ADD_STRING_KEY(data, AGGREGATION_OPERATOR, (Node *)aggreation_map);
	}
	if(node->type == T_WindowOperator){
		//char *WindowOperator_key = "WindowOperator";
		HashMap *WindowOperator_map = NEW_MAP(Constant,Node);
		char *f_key = "f";
		char *partitionBy_key = "partitionBy";
		Node *f = ((WindowOperator *) node)->f;
		List *partitionBy = ((WindowOperator *) node)->partitionBy;

		MAP_ADD_STRING_KEY(WindowOperator_map, f_key, (Node *)f);
		MAP_ADD_STRING_KEY(WindowOperator_map, partitionBy_key, (Node *)partitionBy);

		MAP_ADD_STRING_KEY(data, WINDOW_OPERATOR, (Node *)WindowOperator_map);
	}
	if(node->type == T_SelectionOperator){
		//char *SelectionOperator_key = "SelectionOperator";
		Node *cond = ((SelectionOperator *) node)->cond;
		MAP_ADD_STRING_KEY(data, SELECTION_OPERATOR, (Node *)cond);
	}
	if(node->type == T_OrderOperator){
		//char *OrderOperator_key = "OrderOperator";
		List *orderExprs = ((OrderOperator *) node)->orderExprs;
		MAP_ADD_STRING_KEY(data, ORDER_OPERATOR, (Node *)orderExprs);
	}
	if (node->type == T_TableAccessOperator) {
		//char *TableAccessOperator_key = "TableAccessOperator";
		if (hasMapStringKey(data, TABLEACCESS_OPERATOR)) {

			HashMap *TableAccessOperator_map = NEW_MAP(Constant, Node);
			char *tablename = ((TableAccessOperator *) node)->tableName;
			Schema *schema = ((TableAccessOperator *) node)->op.schema;
			List *attrDef = schema->attrDefs;
			TableAccessOperator_map = (HashMap *)MAP_GET_STRING_ENTRY(data, TABLEACCESS_OPERATOR)->value;
			MAP_ADD_STRING_KEY(TableAccessOperator_map, tablename, (Node * )attrDef);
			MAP_ADD_STRING_KEY(data, TABLEACCESS_OPERATOR, TableAccessOperator_map);
		} else {
			HashMap *TableAccessOperator_map = NEW_MAP(Constant,Node);
			char *tablename = ((TableAccessOperator *) node)->tableName;
			Schema *schema = ((TableAccessOperator *) node)->op.schema;
			List *attrDef = schema->attrDefs;
			MAP_ADD_STRING_KEY(TableAccessOperator_map, tablename, (Node * )attrDef);
			MAP_ADD_STRING_KEY(data, TABLEACCESS_OPERATOR, TableAccessOperator_map);
		}
	}
	return visit(node, getData, data);
}

boolean checkPageSafety(HashMap *data, char *hasOpeator) {
	char *function_name;
	char *colName;
	//char *tableName;
	if (!strcmp(hasOpeator, WINDOW_OPERATOR)) {
		//DEBUG_LOG("it's window");
		//char *WindowOperator_key = "WindowOperator";
		HashMap *WindowOperator_map =
				(HashMap *) MAP_GET_STRING_ENTRY(data, WINDOW_OPERATOR)->value;
		char *f_key = "f";
		Node *f =
				(Node *) MAP_GET_STRING_ENTRY(WindowOperator_map, f_key)->value;
		function_name = ((FunctionCall *) f)->functionname;
		List *args = ((FunctionCall *) f)->args;
		colName =
				((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}
	if (!strcmp(hasOpeator, AGGREGATION_OPERATOR)) {
		//char *aggregation_key = "aggregation";
		HashMap *aggreation_map =
				(HashMap *) MAP_GET_STRING_ENTRY(data, AGGREGATION_OPERATOR)->value;

		char *aggrs_key = "aggrs";
		List *aggrs =
				(List *) MAP_GET_STRING_ENTRY(aggreation_map, aggrs_key)->value;
		function_name =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
		List *args =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
		colName =
				((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}
	//DEBUG_LOG("The COLNAME is: %s", colName);
	//DEBUG_LOG("The TABLENAME is: %s", tableName);

	//char *SelectionOperator_key = "SelectionOperator";
	Node *cond = MAP_GET_STRING_ENTRY(data, SELECTION_OPERATOR)->value;
	char *operator_name = ((Operator *) cond)->name;
	//char *TableAccessOperator_key = "TableAccessOperator";
	HashMap *table_map =
			(HashMap *) MAP_GET_STRING_ENTRY(data, TABLEACCESS_OPERATOR)->value;
	if (!strcmp(function_name, "SUM")) {

		if (checkAllIsPostive(table_map, colName)) {
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
		} else if (checkAllIsNegative(table_map, colName)) {
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
		} else {
			return FALSE;
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
hasOrder(Node* node, Set *operatorSet)
{
	if(node == NULL)
		return TRUE;
	if(node->type == T_OrderOperator){
		addToSet(operatorSet, ORDER_OPERATOR);
	} //Check aggreationOperator
	return visit(node, hasOrder, operatorSet);
}


boolean checkPageSafety_rownum(HashMap *data){
	//char *OrderOperator_key = "OrderOperator";
	List *orderExprs = (List *) MAP_GET_STRING_ENTRY(data, ORDER_OPERATOR)->value;
	Node *attribute_reference = ((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->expr;
	char *orderby_name = ((AttributeReference *) attribute_reference)->name;
	SortOrder order = ((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->order;

	//char *aggregation_key = "aggregation";
	HashMap *aggreation_map = (HashMap *) MAP_GET_STRING_ENTRY(data, AGGREGATION_OPERATOR)->value;
	char *groupby_key = "groupby";
	List *groupby = (List *) MAP_GET_STRING_ENTRY(aggreation_map, groupby_key)->value;
	char *groupby_name = ((AttributeReference *) getHeadOfList(groupby)->data.ptr_value)->name;

	char *aggrs_key = "aggrs";
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(aggreation_map, aggrs_key)->value;
	char *function_name = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *colName = ((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;

	//DEBUG_LOG("The COLNAME is: %s", colName);
	//char *TableAccessOperator_key = "TableAccessOperator";
	HashMap *table_map  = (HashMap *) MAP_GET_STRING_ENTRY(data, TABLEACCESS_OPERATOR)->value;
	if (!strcmp(orderby_name, groupby_name)) {
		return TRUE;
	} else {
		if (!strcmp(function_name, "SUM")) {
			if (checkAllIsPostive(table_map, colName)) {
			if (order == SORT_ASC) {
				return FALSE;
			} else {
				return TRUE;
			}
			} else {
				return FALSE;
			}
		}
		if (!strcmp(function_name, "AVG")) {
			return FALSE;
		}
		if (!strcmp(function_name, "COUNT")) {
			if (order == SORT_ASC) {
				return FALSE;
			} else {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "MAX")) {
			if (order == SORT_ASC) {
				return FALSE;
			} else {
				return TRUE;
			}
		}
		if (!strcmp(function_name, "MIN")) {
			if (order == SORT_ASC) {
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}
	return TRUE;
}

boolean checkAllIsPostive(HashMap *table_map, char *colName) {
	//DEBUG_NODE_BEATIFY_LOG("The TABLE_map is:",table_map);
	List *key_List = getKeys(table_map);
	boolean postive;
	//DEBUG_NODE_BEATIFY_LOG("The key_List is:", key_List);
	FOREACH(Constant, table, key_List)
	{
		//DEBUG_LOG("TABLENAME: %s", table->value);
		List *attrDef = (List *) MAP_GET_STRING_ENTRY(table_map, table->value)->value;
		FOREACH(AttributeDef, attr, attrDef){
			if(!strcmp(attr->attrName, colName)){
				postive = isPostive(table->value, colName) && postive;
			}
		}


	}
	return postive;
}

boolean checkAllIsNegative(HashMap *table_map, char *colName) {
	List *key_List = getKeys(table_map);
	boolean negative;
	//DEBUG_NODE_BEATIFY_LOG("The key_List is:", key_List);
	FOREACH(Constant, table, key_List)
	{
		//DEBUG_LOG("TABLENAME: %s", table->value);
		List *attrDef = (List *) MAP_GET_STRING_ENTRY(table_map, table->value)->value;
		FOREACH(AttributeDef, attr, attrDef)
		{
			if (!strcmp(attr->attrName, colName)) {
				negative = isNegative(table->value, colName) && negative;
			}
		}
	}
	return negative;
}

boolean isPostive(char *tableName, char *colName) {
	HashMap *result = getMinAndMax(tableName, colName);
	char *MIN = ((Constant *) MAP_GET_STRING_ENTRY(result, "MIN")->value)->value;
	if (*MIN == '-') {
		return FALSE;
	}
	return TRUE;
}

boolean isNegative(char *tableName, char *colName) {
	HashMap *result = getMinAndMax(tableName, colName);
	char *MAX = ((Constant *) MAP_GET_STRING_ENTRY(result, "MAX")->value)->value;
	if (*MAX == '-') {
		return TRUE;
	}
	return FALSE;
}
/*
boolean
check(Node* node, HashMap *state)
{
	if(node == NULL)
		return TRUE;
	if(isA(node, AggregationOperator)){
		char *AggregationOperator = "AggregationOperator";
		MAP_ADD_STRING_KEY(state, AggregationOperator, (Node *)createConstInt (1));
	} //Check aggreationOperator
	if(isA(node, WindowOperator)){
		char *WindowOperator = "WindowOperator";
		MAP_ADD_STRING_KEY(state, WindowOperator, (Node *)createConstInt (1));
	}//Check WindowOperator
	if(isA(node, SetOperator)){
		if(((SetOperator *) node)->setOpType == SETOP_DIFFERENCE){
			char *SetOperator = "SetOperator";
			MAP_ADD_STRING_KEY(state, SetOperator, (Node *)createConstInt (1));
		}
	}//Check set difference
	if(isA(node, JoinOperator)){
		JoinOperator *j = (JoinOperator *) node;
		if(j->joinType == JOIN_LEFT_OUTER || j->joinType == JOIN_RIGHT_OUTER || j->joinType == JOIN_FULL_OUTER){
			char *JoinOperator = "JoinOperator";
			MAP_ADD_STRING_KEY(state, JoinOperator, (Node *)createConstInt (1));
		}
	}//Check outer join
	if(isA(node, NestingOperator)){
		char *NestingOperator = "NestingOperator";
		MAP_ADD_STRING_KEY(state, NestingOperator, (Node *)createConstInt (1));
	}//Check nesting
	return visit(node, check, state);
}
*/
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
