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
#include "model/bitset/bitset.h"

HashMap*
monotoneCheck(Node *qbModel)
{
	DEBUG_LOG("Safety Check");
	test(qbModel);
	/*HashMap *checkResult = NEW_MAP(Constant,Node);
	Set *operatorSet = STRSET();
	checkMonotone(qbModel, operatorSet);
	if(setSize(operatorSet) == 0){
		DEBUG_LOG("It's Monotone");
		checkResult = getMonotoneResultMap(qbModel); //get the table schema for all sketches
		//DEBUG_NODE_BEATIFY_LOG("The result_map is:",checkResult);
		return checkResult;
	}else{
		DEBUG_LOG("It isn't Monotone");
		//char *WindowOperator = "WindowOperator";
		Set *findOperator = STRSET();
		hasOperator(qbModel, findOperator);
		checkResult = safetyCheck(qbModel, findOperator);
		//DEBUG_NODE_BEATIFY_LOG("The result_map is:",checkResult);
		return checkResult;
	}*/
	return NEW_MAP(Constant,Node);
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


boolean
hasOperator(Node* node, Set *operatorSet)
{
	if(node == NULL)
		return TRUE;
	if(node->type == T_OrderOperator){
		addToSet(operatorSet, ORDER_OPERATOR);
	} //Check aggreationOperator
	if(node->type == T_SelectionOperator){
		addToSet(operatorSet, SELECTION_OPERATOR);
	} //Check aggreationOperator
	if(node->type == T_WindowOperator){
		addToSet(operatorSet, WINDOW_OPERATOR);
	} //Check aggreationOperator
	if(node->type == T_AggregationOperator){
		addToSet(operatorSet, AGGREGATION_OPERATOR);
	} //Check aggreationOperator
	return visit(node, hasOperator, operatorSet);
}


HashMap *
getSchema(Node* qbModel){
	HashMap *map = NEW_MAP(Constant,Node);
	//getTableAccessOperator(qbModel, map);
	//getSubset(qbModel, map);
	getTableAccessOperator(qbModel, map);
	return map;
}//get schema of table


HashMap *
getMonotoneResultMap(Node* qbModel) {
	HashMap *map = NEW_MAP(Constant, Node);
	HashMap *page_map = getSchema(qbModel);
	HashMap *range_map = getSchema(qbModel);
	HashMap *hash_map = getSchema(qbModel);
	//Constant *isSafe = createConstInt(1);
	//MAP_ADD_STRING_KEY(map, PAGE, (Node * ) isSafe);
	//HashMap *schema_map = getSchema(qbModel);
	MAP_ADD_STRING_KEY(map, PAGE, (Node * ) page_map);
	MAP_ADD_STRING_KEY(map, RANGE, (Node * ) range_map);
	MAP_ADD_STRING_KEY(map, HASH, (Node * ) hash_map);
	//MAP_ADD_STRING_KEY(map, BLOOM_FILTER, (Node * ) schema_map);
	return map;
}//return the result map for all sketches.


HashMap *
safetyCheck(Node* qbModel, Set *hasOperator) {
	HashMap *map = NEW_MAP(Constant, Node);
	HashMap *data = NEW_MAP(Constant, Node);
	getData(qbModel, data);//get the data of node we need
	boolean result = FALSE;
	if(hasSetElem(hasOperator, ORDER_OPERATOR)){
			//DEBUG_LOG("it's order");
			result = checkPageSafety_rownum(data);		//rownum check
			DEBUG_LOG("The result is: %d", result);
		}else{
			//DEBUG_LOG("it's no order");
			result = checkPageSafety(data, hasOperator); // window and aggregation
			DEBUG_LOG("The result is: %d", result);
	}


	if (!result) {
		Constant *isSafe = createConstInt(0);
		MAP_ADD_STRING_KEY(map, PAGE, (Node * ) isSafe);
	} else {
		Constant *isSafe = createConstInt(1);
		MAP_ADD_STRING_KEY(map, PAGE, (Node * ) isSafe);
	}
	HashMap *schema_map = getSchema(qbModel);
	MAP_ADD_STRING_KEY(map, RANGE, (Node * ) schema_map);
	MAP_ADD_STRING_KEY(map, BLOOM_FILTER, (Node * ) schema_map);
	MAP_ADD_STRING_KEY(map, HASH, (Node * ) schema_map);
	return map;
}

boolean
getSafeProvenanceSketch(Node* node, HashMap *map) {
	if (node == NULL)
		return TRUE;
	if (node->type == T_ProjectionOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);
	}
	if (node->type == T_SelectionOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);
	}
	if (node->type == T_OrderOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);
	} //Check aggreationOperator
	if (node->type == T_WindowOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		HashMap *table_map =
				(HashMap *) MAP_GET_STRING_ENTRY(map, "Table_map")->value;
		boolean isSimple = TRUE;
		HashMap *result = (HashMap *) MAP_GET_STRING_ENTRY(map, "Result")->value;
		WindowOperator *windowOperator = (WindowOperator *) node;
		List *partitionBy = windowOperator->partitionBy;
		char *partitionByColName = ((AttributeReference *) getHeadOfList(
				partitionBy)->data.ptr_value)->name;
		char *tableName = findTable(table_map, partitionByColName);

		if (setSize(findFiltering(operatorList)) == 0) {

			//Safe PS is ALL, intersection with the former Safe PS
		} else if (hasSetElem(findFiltering(operatorList),
		SELECTION_OPERATOR)) {
			DEBUG_LOG("find filtering");
			int size = getListLength(operatorList);
			//DEBUG_LOG("size is: %d", size);
			List *newList = NIL;
			SelectionOperator *selectOperator;
			for (int i = size - 1; i >= 0; i--) {
				Node *n = (Node *) getNthOfList(operatorList, i)->data.ptr_value;
				//DEBUG_NODE_BEATIFY_LOG("The node is:", n);
				if (n->type != T_SelectionOperator) {
					newList = appendToHeadOfList(newList, n);
				} else {
					selectOperator = (SelectionOperator *) n; //check rule1
					break;
				}
			}

			if (setSize(findFiltering(newList)) == 0) {
				OrderOperator *OrderOperator = findOrderOperator(newList);
				if (OrderOperator != NULL) {
					DEBUG_LOG("FIND ORDERBY");
					if (monotonicity2(OrderOperator, node, table_map,
							partitionByColName)) {
						DEBUG_LOG("ps is all");
					} else {
						HashMap *page_map =
								(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
						MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
						map = updateResultMap(map, result, PAGE, page_map);
						//page is not safe
						if (isSimple) {
							//range hash on group is safe
							map = hashRangeOnGroupbySafe(map, result, tableName,
									partitionByColName);
						} else {
							map = hashRangeOnGroupbyNotSafe(map, result,
									tableName, partitionByColName);
							//range hash on group is not safe, no one is safe
						}
					}
					//check rule2
				} else {
					if (monotonicity1(selectOperator, node, table_map)) {
						DEBUG_LOG("ps is all");
					} else {
						HashMap *page_map =
								(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
						MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
						map = updateResultMap(map, result, PAGE, page_map);
						//page is not safe
						if (isSimple) {
							//range hash on group is safe
							map = hashRangeOnGroupbySafe(map, result, tableName,
									partitionByColName);
						} else {
							map = hashRangeOnGroupbyNotSafe(map, result,
									tableName, partitionByColName);
							//range hash on group is not safe, no one is safe
						}
					}
				}
			} else {
				HashMap *page_map =
						(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
				MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
				map = updateResultMap(map, result, PAGE, page_map);
				//page is not safe
				if (isSimple) {
					map = hashRangeOnGroupbySafe(map, result, tableName,
							partitionByColName);
					//range hash on group is safe
				} else {
					map = hashRangeOnGroupbyNotSafe(map, result, tableName,
							partitionByColName);
					//range hash on group is not safe. no one is safe
				}
			}

		} else {
			HashMap *page_map =
					(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
			MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
			map = updateResultMap(map, result, PAGE, page_map);
			//page is not safe
			if (isSimple) {
				map = hashRangeOnGroupbySafe(map, result, tableName,
						partitionByColName);
				//range hash on group is safe
			} else {
				map = hashRangeOnGroupbyNotSafe(map, result, tableName,
						partitionByColName);
				//range hash on group is not safe. no one is safe
			}
		}

		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);

	} //Check aggreationOperator
	if (node->type == T_AggregationOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		HashMap *table_map =
				(HashMap *) MAP_GET_STRING_ENTRY(map, "Table_map")->value;
		boolean isSimple = TRUE;
		HashMap *result = (HashMap *) MAP_GET_STRING_ENTRY(map, "Result")->value;
		//HashMap *schema_map = (HashMap *) MAP_GET_STRING_ENTRY(map, "Schema")->value;
		AggregationOperator *aggregationOperator = (AggregationOperator *) node;
		//List *aggrs = ((AggregationOperator *) node)->aggrs;
		List *groupby = aggregationOperator->groupBy;
		char *groupByColName = ((AttributeReference *) getHeadOfList(groupby)->data.ptr_value)->name;
		char *tableName = findTable(table_map, groupByColName);

		if (findSetDifference(operatorList)) {
			HashMap *page_map = (HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
			MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
			map = updateResultMap(map, result, PAGE, page_map);
			map = hashRangeOnGroupbySafe(map, result, tableName,
					groupByColName);
		}
		if (setSize(findFiltering(operatorList)) == 0) {
			//Safe PS is ALL, intersection with the former Safe PS
		} else if (hasSetElem(findFiltering(operatorList),
				SELECTION_OPERATOR)) {
			DEBUG_LOG("find filtering");
			int size = getListLength(operatorList);
			//DEBUG_LOG("size is: %d", size);
			List *newList = NIL;
			SelectionOperator *selectOperator;
			for (int i = size - 1; i >= 0; i--) {
				Node *n = (Node *) getNthOfList(operatorList, i)->data.ptr_value;
				//DEBUG_NODE_BEATIFY_LOG("The node is:", n);
				if (n->type != T_SelectionOperator) {
					newList = appendToHeadOfList(newList, n);
				} else {
					selectOperator = (SelectionOperator *) n; //check rule1
					break;
				}
			}
			//DEBUG_NODE_BEATIFY_LOG("The node is:", selectOperator);
			if (setSize(findFiltering(newList)) == 0) {
				OrderOperator *OrderOperator = findOrderOperator(newList);
				if (OrderOperator != NULL) {
					DEBUG_LOG("FIND ORDERBY");
					if (monotonicity2(OrderOperator, node, table_map, groupByColName)) {
						DEBUG_LOG("ps is all");
					} else {
						HashMap *page_map = (HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
						MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
						map = updateResultMap(map, result, PAGE, page_map);
						//page is not safe
						if (isSimple) {
							//range hash on group is safe
							map = hashRangeOnGroupbySafe(map, result, tableName,
									groupByColName);
						} else {
							map = hashRangeOnGroupbyNotSafe(map, result,
									tableName, groupByColName);
							//range hash on group is not safe, no one is safe
						}
					}
					//check rule2
				} else {
					//check rule1
					if (monotonicity1(selectOperator, node, table_map)) {
						DEBUG_LOG("ps is all");
					} else {
						HashMap *page_map = (HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
						MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
						map = updateResultMap(map, result, PAGE, page_map);
						//page is not safe
						if (isSimple) {
							//range hash on group is safe
							map = hashRangeOnGroupbySafe(map, result, tableName,
									groupByColName);
						} else {
							map = hashRangeOnGroupbyNotSafe(map, result,
									tableName, groupByColName);
							//range hash on group is not safe, no one is safe
						}
					}

				}
			} else {
				HashMap *page_map = (HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
				MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
				map = updateResultMap(map, result, PAGE, page_map);
				//page is not safe
				if (isSimple) {
					map = hashRangeOnGroupbySafe(map, result, tableName,
							groupByColName);
					//range hash on group is safe
				} else {
					map = hashRangeOnGroupbyNotSafe(map, result, tableName,
							groupByColName);
					//range hash on group is not safe. no one is safe
				}
			}
			//DEBUG_NODE_BEATIFY_LOG("The new list is:", newList);
			//hasSetElem(findFiltering(operatorList), SELECTION_OPERATOR)
			// rule 1, intersection with the former Safe PS
		} else {
			HashMap *page_map = (HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
			MAP_ADD_STRING_KEY(page_map, tableName, (Node *)NIL);
			map = updateResultMap(map, result, PAGE, page_map);
			// page is not safe.
			if (isSimple) {
				map = hashRangeOnGroupbySafe(map, result, tableName,
						groupByColName);
				//range hash on group is safe
			} else {
				map = hashRangeOnGroupbyNotSafe(map, result, tableName,
						groupByColName);
				//range hash on group is not safe
			}
		}
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);
		//map = checkAggregation(node, map);
	} //Check aggreationOperator
	if (node->type == T_TableAccessOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);
	}
	if (node->type == T_SetOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);
	} //Check set difference
	if (node->type == T_JoinOperator) {
		List *operatorList = (List *) MAP_GET_STRING_ENTRY(map, "List")->value;
		operatorList = appendToTailOfList(operatorList, node);
		MAP_ADD_STRING_KEY(map, "List", (Node * )operatorList);

	} //Check outer join
	return visit(node, getSafeProvenanceSketch, map);
}

Set*
findFiltering(List* operatorList){
	Set *operatorSet = STRSET();
	FOREACH(Node,operator,operatorList)
			if(operator->type == T_AggregationOperator){
				addToSet(operatorSet, AGGREGATION_OPERATOR);
			} else if(operator->type == T_SelectionOperator){
				addToSet(operatorSet, SELECTION_OPERATOR);
			} else if(operator->type == T_DuplicateRemoval){
				addToSet(operatorSet, DUPLICATEREMOVAL);
			} else if(operator->type == T_WindowOperator){
				addToSet(operatorSet, WINDOW_OPERATOR);
			} else if(operator->type == T_JoinOperator){
				addToSet(operatorSet, JOIN_OPERATOR);
			} else if(operator->type == T_SetOperator){
				if(((SetOperator *) operator)->setOpType ==  SETOP_INTERSECTION) {
					addToSet(operatorSet, SETOPINTERSECTION);
				}
			} else {
				continue;
			}
	return operatorSet;
}

boolean
findSetDifference(List* operatorList){
	FOREACH(Node,operator,operatorList){
		if (operator->type == T_SetOperator) {
			if(((SetOperator *) operator)->setOpType ==  SETOP_DIFFERENCE) {
				return TRUE;
			}
		}
	}
	return FALSE;
}


OrderOperator*
findOrderOperator(List* operatorList){
	FOREACH(Node,operator,operatorList){
		if (operator->type == T_OrderOperator) {
			return (OrderOperator*) operator;
		}
	}
	return NULL;
}

char*
findTable(HashMap *table_map, char *colName) {
	//DEBUG_NODE_BEATIFY_LOG("The TABLE_map is:",table_map);
	char *tableName;
	List *key_List = getKeys(table_map);
	//DEBUG_NODE_BEATIFY_LOG("The key_List is:", key_List);
	FOREACH(Constant, table, key_List)
	{
		//DEBUG_LOG("TABLENAME: %s", table->value);
		List *attrDef = (List *) MAP_GET_STRING_ENTRY(table_map, table->value)->value;
		FOREACH(AttributeDef, attr, attrDef){
			if(!strcmp(attr->attrName, colName)){
				tableName = table->value;
			}
		}
	}
	return tableName;
}

HashMap*
updateResultMap(HashMap *map, HashMap *orginalResult, char *type, HashMap *newResultMapOfType){
	//HashMap *newMap = NEW_MAP(Constant, Node);
	MAP_ADD_STRING_KEY(orginalResult, type, (Node *)newResultMapOfType);
	MAP_ADD_STRING_KEY(map, "Result", (Node *)orginalResult);
	return map;
}

HashMap*
hashRangeOnGroupbySafe(HashMap *map, HashMap *result, char *tableName, char *groupByColName){
	HashMap *hash_map = (HashMap *) MAP_GET_STRING_ENTRY(result,HASH)->value;
	HashMap *range_map = (HashMap *) MAP_GET_STRING_ENTRY(result,RANGE)->value;
	List *attributesList = (List *) MAP_GET_STRING_ENTRY(hash_map,tableName)->value;
	List *newAttributesList = NIL;
	FOREACH(AttributeDef,attribute,attributesList){
			if (!strcmp(attribute->attrName, groupByColName)) {
				newAttributesList = appendToTailOfList(
				newAttributesList, attribute);
			}
	}
	MAP_ADD_STRING_KEY(hash_map, tableName, (Node * )newAttributesList);
	MAP_ADD_STRING_KEY(range_map, tableName, (Node * )newAttributesList);
	map = updateResultMap(map, result, HASH, hash_map);
	map = updateResultMap(map, result, RANGE, range_map);
	return map;
}

HashMap*
hashRangeOnGroupbyNotSafe(HashMap *map, HashMap *result, char *tableName, char *groupByColName){
	HashMap *hash_map = (HashMap *) MAP_GET_STRING_ENTRY(result,HASH)->value;
	HashMap *range_map = (HashMap *) MAP_GET_STRING_ENTRY(result,RANGE)->value;
	MAP_ADD_STRING_KEY(hash_map, tableName, (Node *)NIL);
	MAP_ADD_STRING_KEY(range_map, tableName, (Node *)NIL);
	map = updateResultMap(map, result, HASH, hash_map);
	map = updateResultMap(map, result, RANGE, range_map);
	return map;
}

boolean
monotonicity1(SelectionOperator *selectOperator, Node *node, HashMap *table_map) {
	char *function_name;
	char *colName;
	if (node->type == T_AggregationOperator) {
		AggregationOperator *aggregationOperator = (AggregationOperator *) node;
		List *aggrs = (List *) aggregationOperator->aggrs;
		function_name = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
		List *args = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
		colName = ((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}
	if (node->type == T_WindowOperator) {
		WindowOperator *windowOperator = (WindowOperator *) node;
		Node *f = windowOperator->f;
		function_name = ((FunctionCall *) f)->functionname;
		List *args = ((FunctionCall *) f)->args;
		colName = ((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}

	char *operator_name = ((Operator *) selectOperator->cond)->name;
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
monotonicity2
(OrderOperator *orderOperator, Node *node, HashMap *table_map, char *groupByColName){
		char *function_name;
		char *colName;

		if (node->type == T_AggregationOperator) {
			AggregationOperator *aggregationOperator = (AggregationOperator *) node;
			List *aggrs = (List *) aggregationOperator->aggrs;
			function_name = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
			List *args = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
			colName = ((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
		}
		if (node->type == T_WindowOperator) {
			WindowOperator *windowOperator = (WindowOperator *) node;
			Node *f = windowOperator->f;
			function_name = ((FunctionCall *) f)->functionname;
			List *args = ((FunctionCall *) f)->args;
			colName = ((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
		}

		List *orderExprs = (List *) orderOperator->orderExprs;
		Node *attribute_reference = ((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->expr;
		char *orderby_name = ((AttributeReference *) attribute_reference)->name;
		SortOrder order = ((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->order;

		//char *aggregation_key = "aggregation";;

		//DEBUG_LOG("The COLNAME is: %s", colName);
		//char *TableAccessOperator_key = "TableAccessOperator";
		if (!strcmp(orderby_name, groupByColName)) {
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
getAll(Node* node, HashMap *map)
{
	if(node == NULL)
		return TRUE;

	if(node->type == T_TableAccessOperator){
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		MAP_ADD_STRING_KEY(map, tablename, (Node *)attrDef);
	}

	return visit(node, getAll, map);
}//get the attribute of the tabe

boolean
getSubset(Node* node, HashMap *map)
{
	if(node == NULL)
		return TRUE;

	if(node->type == T_TableAccessOperator){
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		unsigned int length = (unsigned int) getListLength(attrDef);
		List *result = NIL;
		result = addBitset(length, result);
		MAP_ADD_STRING_KEY(map, tablename, (Node *)result);
	}

	return visit(node, getSubset, map);
}//get the KeyValue of each table




List*
addBitset(unsigned int length, List *result)
{
	char *subset = "SUBSET";
//	char *exact = "EXCAT";
//	unsigned long max = 1 << length;
	for (int i = 0; i < length; i++)
	{
		//unsigned long *value = &i; //TODO why use address of i here?
		/* if (i == (max - 1)) */
		/* { */
		/* 	//KeyValue *element = createStringKeyValue(exact, binDis(length, i)); */

		/* 	BitSet *bitset = singletonBitSet(i); */
		/* 	KeyValue *element = createStringKeyValue(exact, bitSetToString(bitset)); */
		/* 	result = appendToTailOfList(result, element); */
		/* 	break; */
		/* } */
		BitSet *bitset = newSingletonBitSet(i);
		KeyValue *element = createStringKeyValue(subset, bitSetToString(bitset));
		result = appendToTailOfList(result, element);
	}
	return result;
}//get BitSet of all subsets


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

boolean checkPageSafety(HashMap *data, Set *hasOperator) {
	char *function_name;
	char *colName;
	HashMap *table_map =
			(HashMap *) MAP_GET_STRING_ENTRY(data, TABLEACCESS_OPERATOR)->value;
	//char *tableName;
	if (hasSetElem(hasOperator, WINDOW_OPERATOR)) {
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
	if (hasSetElem(hasOperator, AGGREGATION_OPERATOR)) {

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
	if (hasSetElem(hasOperator, SELECTION_OPERATOR)) {
		Node *cond = MAP_GET_STRING_ENTRY(data, SELECTION_OPERATOR)->value;
		char *operator_name = ((Operator *) cond)->name;

		//char *TableAccessOperator_key = "TableAccessOperator";

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
	}
	return FALSE;
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
	Constant *number = (Constant *) MAP_GET_STRING_ENTRY(result, "MIN")->value;
	if (number->constType == DT_INT) {
		int *value = (int *) number->value;
		if (*value <= 0) {
			return FALSE;
		} else {
			return TRUE;
		}
	} else if (number->constType == DT_FLOAT) {
		float *value = (float *) number->value;
		if (*value <= 0.0) {
			return FALSE;
		} else {
			return TRUE;
		}
	} else {
		return TRUE;
	}
	return TRUE;
}

boolean isNegative(char *tableName, char *colName) {
	HashMap *result = getMinAndMax(tableName, colName);
	Constant *number = (Constant *) MAP_GET_STRING_ENTRY(result, "MAX")->value;
	if (number->constType == DT_INT) {
		int *value = (int *) number->value;
		if (*value <= 0) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else if (number->constType == DT_FLOAT) {
		float *value = (float *) number->value;
		if (*value <= 0.0) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return TRUE;
	}
	return TRUE;
}

void
test(Node *qbModel){
	//110, 011
	/*
    unsigned long a = 6, b = 3;
	BitSet *b1 = newBitSet(3, &a ,T_BitSet);
	BitSet *b2 = newBitSet(3, &b ,T_BitSet);
	BitSet *b3 = NULL;
	DEBUG_LOG("The b1 length is %d and value is %ld",b1->length,*b1->value);
	DEBUG_LOG("The new bitset length is %d and value is %ld",bitOr(b1,b2)->length,*bitOr(b1,b2)->value);
	DEBUG_LOG("The new bitset length is %d and value is %ld",bitAnd(b1,b2)->length,*bitAnd(b1,b2)->value);
	DEBUG_LOG("The new bitset length is %d and value is %ld",bitNot(b2)->length,*bitNot(b2)->value);
	DEBUG_LOG("The new bitset is %d",bitsetEquals(b1, b3));
	DEBUG_LOG("The new bitset is %s",bitSetToString(b1));
	setBit(b1,3,FALSE);
	setBit(b1,2,TRUE);
	DEBUG_LOG("The position is %d",isBitSet(b1, 0));
	DEBUG_LOG("The new bitset is %s",bitSetToString(b1));
	int value = getRowNum("R");
	DEBUG_LOG("The numbers of rows is : %d\n", value);*/
	//HashMap *operatorState = NEW_MAP(Constant,Node);
	//QueryOperator *root = (QueryOperator *) getHeadOfList((List*) qbModel)->data.ptr_value;
	//computeChildOperatorProp(root);
	//computeMinMaxProp(root);
	//HashMap *min_max = getMinAndMax("R","A");*/
	HashMap *map_list_result = NEW_MAP(Constant, Node);
	HashMap *table_map = NEW_MAP(Constant, Node);
	List *operatorList = NIL;
	HashMap *result = getMonotoneResultMap(qbModel);
	HashMap *schema_map = getSchema(qbModel);
	getTableAccessOperator(qbModel, table_map);
	MAP_ADD_STRING_KEY(map_list_result, "List", (Node *)operatorList);
	MAP_ADD_STRING_KEY(map_list_result, "Result", (Node *)result);
	MAP_ADD_STRING_KEY(map_list_result, "Schema", (Node *)schema_map);
	MAP_ADD_STRING_KEY(map_list_result, "Table_map", (Node *)table_map);
	getSafeProvenanceSketch(qbModel, map_list_result);
	List *list = (List *) MAP_GET_STRING_ENTRY(map_list_result, "List")->value;
	DEBUG_NODE_BEATIFY_LOG("The list is:", list);
	result = (HashMap *) MAP_GET_STRING_ENTRY(map_list_result, "Result")->value;
	DEBUG_NODE_BEATIFY_LOG("The test is:", result);
	schema_map = (HashMap *) MAP_GET_STRING_ENTRY(map_list_result, "Schema")->value;
	DEBUG_NODE_BEATIFY_LOG("The shema is:", schema_map);
	//DEBUG_NODE_BEATIFY_LOG("The min_max is:",min_max);
	//DEBUG_LOG("The negative is: %d", isNegative("R","A"));
	//DEBUG_LOG("The postive is: %d", isPostive("R","A"));
	//check(qbModel, operatorState);
	//DEBUG_NODE_BEATIFY_LOG("The result_state is:",operatorState);
	//List *entries = getEntries(operatorState);//get all operator in the tree.
	//boolean lzy = isPostive("R", "A");
	//	DEBUG_LOG("The lzy is %d", lzy);
}


void
computeDistinct (QueryOperator *root){
	testhistogrma();
	/*if (root == NULL) {
		return;
	}

	if (root->inputs != NULL){
		FOREACH(QueryOperator, op, root->inputs){
			if (!HAS_STRING_PROP(op,PROP_STORE_COST_DONE))
				computeDistinct(op);
		}
	}

	DEBUG_LOG("BEGIN COMPUTE COST OF %s operator %s", NodeTagToString(root->type), root->schema->name);
	SET_BOOL_STRING_PROP(root, PROP_STORE_COST_DONE);
	if (root->type == T_TableAccessOperator){
			TableAccessOperator *rel = (TableAccessOperator *) root;
			char * tableName = rel->tableName;
			HashMap * Distinct = NEW_MAP(Constant, Node);
			FOREACH(AttributeDef, attrDef, rel->op.schema->attrDefs){
				HashMap *result_map = NEW_MAP(Constant, Node);
				Constant * distinct = createConstInt(getDistinct(tableName,attrDef->attrName));
				Constant * tuples = createConstInt(getRowNum(tableName));
				MAP_ADD_STRING_KEY(result_map, "DISTINCT", (Node *) distinct);
				MAP_ADD_STRING_KEY(result_map, "NUM OF TUPLES", (Node *) tuples);
				MAP_ADD_STRING_KEY(Distinct, attrDef->attrName, (Node *)result_map);
			}
			setStringProperty(root, PROP_STORE_COST, (Node *)Distinct);
			DEBUG_NODE_BEATIFY_LOG("COST are:", Distinct);
		}
	if (root->type == T_ProjectionOperator) {
		QueryOperator * child = (QueryOperator *) getHeadOfList((List *) root->inputs)->data.ptr_value;
		ProjectionOperator * pj = (ProjectionOperator *) root;
		HashMap *childDistinct =(HashMap *) getStringProperty(child,PROP_STORE_COST);
		HashMap *Distinct = NEW_MAP(Constant, Node);
		FOREACH(AttributeDef, attrDef, pj->op.schema->attrDefs){
			//HashMap *result_map = NEW_MAP(Constant, Node);
			HashMap * result_map = (HashMap *) MAP_GET_STRING_ENTRY(childDistinct, attrDef->attrName)->value;
			MAP_ADD_STRING_KEY(Distinct, attrDef->attrName, (Node *)result_map);
			//DEBUG_LOG("attrDef is %s", attrDef->attrName);
		}
		//DEBUG_NODE_BEATIFY_LOG("child are:", childDistinct);
		setStringProperty(root, PROP_STORE_COST, (Node *)Distinct);
		DEBUG_NODE_BEATIFY_LOG("COST are:", Distinct);
	}
	if (root->type == T_SelectionOperator){
		SelectionOperator * sel = (SelectionOperator *) root;
		Operator * cond = (Operator * ) sel->cond;
		double selectivity = computeSelectivity(cond);
		DEBUG_LOG("selectivity IS %f",selectivity);
		QueryOperator * child = (QueryOperator *) getHeadOfList((List *) root->inputs)->data.ptr_value;
		HashMap *childDistinct =(HashMap *) getStringProperty(child,PROP_STORE_COST);
		HashMap *Distinct = NEW_MAP(Constant, Node);
		FOREACH_HASH_ENTRY(el,childDistinct){
			//DEBUG_NODE_BEATIFY_LOG("el are:", el);
			Node* key = el->key;
			Node* value = el->value;

			HashMap *result_map = NEW_MAP(Constant, Node);
			Constant * childValue1 = (Constant *) MAP_GET_STRING_ENTRY((HashMap *)value,"DISTINCT")->value;
			Constant * childValue2 = (Constant *) MAP_GET_STRING_ENTRY((HashMap *)value,"NUM OF TUPLES")->value;
			//DEBUG_NODE_BEATIFY_LOG("childValue1 is:", childValue1);
			//DEBUG_NODE_BEATIFY_LOG("childValue2 is:", childValue2);
			double a = (double) (*((int *) childValue1->value)) * selectivity;
			double b = (double) (*((int *) childValue2->value)) * selectivity;
			Constant * newValue1 =createConstInt(ceil(a));
			Constant * newValue2 =createConstInt(ceil(b));
			MAP_ADD_STRING_KEY(result_map, "DISTINCT", (Node *)newValue1);
			MAP_ADD_STRING_KEY(result_map, "NUM OF TUPLES", (Node *)newValue2);
			addToMap(Distinct, key, (Node *)result_map);
		}
		setStringProperty(root, PROP_STORE_COST, (Node *)Distinct);
		DEBUG_NODE_BEATIFY_LOG("COST are:", Distinct);
	*/

	}

void
testhistogrma(){
	List *result = NIL;
	/*char *t ="CRIMES_ZIP_CODES_BEAT_HIST";
	char test1[] = {};
	for(int i=0; i< strlen(t);i++){
		test1[i]= t[i];
	}
	//DEBUG_LOG("LZY IS %s", test1);
	char *test = "CRIMES_ZIP_CODES_BEAT_HIST";
	char test1[100];
	strcpy(test1, test);
	char test2[] = "CRIMES_ZIP_CODES_BEAT_WARD_HIST";
	DEBUG_LOG("LZY IS %s", test1);

	Set *result1 = STRSET();
	Set *result2 = STRSET();
	char *d = "_";

	char *p1 = strtok(test1, d);
	while (p1 != NULL) {
		//DEBUG_LOG("LZY IS %s", p1);
		addToSet(result1, p1);
		p1 = strtok(NULL, d);
	}
	char *p2 = strtok(test2, d);
	while (p2 != NULL) {
		//DEBUG_LOG("LZY IS %s", p2);
		addToSet(result2, p2);
		p2 = strtok(NULL, d);
	}
	FOREACH_SET(char,n,result1)
	{
		if (hasSetElem(result2, n)) {
			removeSetElem(result1, n);
			removeSetElem(result2, n);
		}
	}
	FOREACH_SET(char,n,result1) {
		DEBUG_LOG("LZY111 IS %s", n);
	}
	FOREACH_SET(char,n,result2) {
		DEBUG_LOG("LZY222 IS %s", n);
	}*/
	//storeInterval("CRIMES","BEAT","50");
	//result = get2DHist("CRIMES", "BEAT","ZIP_CODES","50","20");
	//result = get2DHist("CRIMES", "BEAT","WARD","50","20");
	//result = get2DHist("CRIMES", "BEAT","ZIP_CODES","50","20");
	//result = get2DHist("CRIMES", "BEAT","WARD","50","20");
	//result = get2DHist("CRIMES", "BEAT","DISTRICT","50","20");
	/*char* t1 = get2DHist("CRIMES", "ZIP_CODES","WARD","50","20");
	DEBUG_LOG("t1 IS %s", t1);
	char* t2 = get2DHist("CRIMES", "ZIP_CODES","BEAT","50","20");
	DEBUG_LOG("t2 IS %s", t2);
	char* t3 = get2DHist("CRIMES", "ZIP_CODES","COMMUNITY_AREAS","50","20");
	DEBUG_LOG("t3 IS %s", t3);*/
	/*char* t1 = get2DHist("CRIMES", "BEAT","ZIP_CODES","50","20");
	DEBUG_LOG("t1 IS %s", t1);
	char* t2 = get2DHist("CRIMES", "BEAT","WARD","50","20");
	DEBUG_LOG("t2 IS %s", t2);
	char* t3 = get2DHist("CRIMES", "BEAT","COMMUNITY_AREAS","50","20");
	DEBUG_LOG("t3 IS %s", t3);*/
	//char *t4 = join2Hist("CRIMES#BEAT#ZIP_CODES#HIST","CRIMES#BEAT#WARD#HIST","CRIMES","BEAT");
	//DEBUG_LOG("t3 IS %s", t4);
	//char *t5 = join2Hist("CRIMES#BEAT#ZIP_CODES#WARD#HIST","CRIMES#BEAT#COMMUNITY_AREAS#HIST","CRIMES","BEAT");
	//DEBUG_LOG("t3 IS %s", t5);
	//result = join2Hist("CRIMES#BEAT#ZIP_CODES#HIST","CRIMES#BEAT#WARD#HIST","CRIMES","BEAT");
	//result = join2Hist("CRIMES#BEAT#ZIP_CODES#WARD#HIST","CRIMES#BEAT#DISTRICT#HIST","CRIMES","BEAT");
	/*char* h1 = join2Hist("CRIMES#ZIP_CODES#BEAT#HIST","CRIMES#ZIP_CODES#WARD#HIST","CRIMES","ZIP_CODES");
	DEBUG_LOG("h1 IS %s", h1);
	char* h2 = join2Hist("CRIMES#ZIP_CODES#BEAT#WARD#HIST","CRIMES#ZIP_CODES#COMMUNITY_AREAS#HIST","CRIMES","ZIP_CODES");
	DEBUG_LOG("h2 IS %s", h2);*/
	//result = computeSum("CRIMES_BEAT_ZIP_CODES_WARD_HIST", "BEAT", "WARD");
	//result = computeSum("CRIMES_ZIP_CODES_BEAT_WARD_HIST", "ZIP_CODES", "WARD");
	//result = computeSum("CRIMES_ZIP_CODES_WARD_HIST", "ZIP_CODES", "WARD");
	//result = computeSum("CRIMES#BEAT#ZIP_CODES#WARD#HIST", "BEAT", "WARD");
	//result = computeSum("CRIMES#BEAT#ZIP_CODES#WARD#DISTRICT#HIST", "BEAT", "WARD");
	result = computeSum("CRIMES#ZIP_CODES#BEAT#WARD#DISTRICT#HIST", "ZIP_CODES", "WARD");
	//result = computeSum("CRIMES#ZIP_CODES#BEAT#WARD#COMMUNITY_AREAS#HIST", "ZIP_CODES", "WARD");
	//result = computeSum("CRIMES#BEAT#ZIP_CODES#WARD#COMMUNITY_AREAS#HIST", "BEAT", "WARD");
	//result = computeSum("CRIMES#BEAT#WARD#HIST", "BEAT", "WARD");
	//result = computeSum(join2Hist("CRIMES#ZIP_CODES#BEAT#WARD#HIST","CRIMES#ZIP_CODES#DISTRICT#HIST","CRIMES","ZIP_CODES"), "ZIP_CODES", "WARD");
	//result = join2Hist("CRIMES_ZIP_CODES_BEAT_WARD_HIST","CRIMES_ZIP_CODES_WARD_HIST","CRIMES","ZIP_CODES");
	//result = getHist("CRIMES", "ZIP_CODES", 50);
	//result = storeInterval("CRIMES", "ZIP_CODES", 30);
	//result = getHist("CUSTOMER", "C_CUSTKEY", 63);
	//result = get2DHist("lineitem", "l_partkey", "l_orderkey", "300000");

}

/*
	if (root->type == T_AggregationOperator) {
		//AggregationOperator * agg = (AggregationOperator *) root;
		QueryOperator * child = (QueryOperator *) getHeadOfList(
				(List *) root->inputs)->data.ptr_value;
		HashMap *childDistinct = (HashMap *) getStringProperty(child,
				PROP_STORE_COST);
		HashMap *Distinct = NEW_MAP(Constant, Node);
		FOREACH_HASH_ENTRY(el,childDistinct)
		{
			//DEBUG_NODE_BEATIFY_LOG("el are:", el);
			Node* key = el->key;
			Node* value = el->value;

			addToMap(Distinct, key, value);
		}
		setStringProperty(root, PROP_STORE_COST, (Node *) Distinct);
		DEBUG_NODE_BEATIFY_LOG("COST are:", Distinct);
	}
	if (root->type == T_WindowOperator){


	}
	if (root->type == T_JoinOperator){


	}

*/


double
computeSelectivity(Operator *cond){
	char *operator_name = cond->name;
	DEBUG_LOG("operator_name IS %s",operator_name);
	List *args = cond->args;
	char *name = ((AttributeReference *) getHeadOfList((List *)args)->data.ptr_value)->name;
	DEBUG_LOG("NAME IS %s",name);
	HashMap *minmax = getMinAndMax("R", name);

	Constant *max =(Constant *) MAP_GET_STRING_ENTRY(minmax, "MAX")->value;
	Constant *min =(Constant *) MAP_GET_STRING_ENTRY(minmax, "MIN")->value;
	//DEBUG_NODE_BEATIFY_LOG("min is:", min);
	//DEBUG_NODE_BEATIFY_LOG("max is:", max);
	Constant *constant = (Constant *) getTailOfList((List *)args)->data.ptr_value;
	//DEBUG_NODE_BEATIFY_LOG("constant is:", constant);
	int a = *((int *) max->value);
	int b = *((int *) min->value);
	int c = *((int *) constant->value);
	if(!strcmp(operator_name, ">") || !strcmp(operator_name, ">=")) {

		DEBUG_LOG("a IS %d,b is %d, c is %d",a,b,c);
		double selectivity = (double) ((double)(a-c)/(double)(a-b));
		DEBUG_LOG("s is %f",selectivity);
		return selectivity;

	}
	if(!strcmp(operator_name, "<") || !strcmp(operator_name, "<=")) {

			DEBUG_LOG("a IS %d,b is %d, c is %d",a,b,c);
			double selectivity = (double) ((double)(c-b)/(double)(a-b));
			DEBUG_LOG("s is %f",selectivity);
			return selectivity;

	}
	return 0;
}
/*
void *
constantToValue(Constant *constant){
	switch(constant->constType)
	    {
	        case DT_INT:
	        	return (int *) constant->value;
	            break;
	        case DT_FLOAT:
	        	return (double *) constant->value;
	            break;
	        case DT_LONG:
	        	return (gprom_long_t *) constant->value;
	            break;
	    }
	return 0;
}*/
/*
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
*/

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
