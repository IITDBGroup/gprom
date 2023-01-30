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
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include <string.h>
#include "model/bitset/bitset.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

HashMap*
monotoneCheck(Node *qbModel) {
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
	return NEW_MAP(Constant, Node);
} //check whether it is monotone

boolean checkMonotone(Node* node, Set *operatorSet) {
	if (node == NULL)
		return TRUE;
	if (node->type == T_AggregationOperator) {
		addToSet(operatorSet, AGGREGATION_OPERATOR);
	} //Check aggreationOperator
	if (node->type == T_WindowOperator) {
		addToSet(operatorSet, WINDOW_OPERATOR);
	} //Check WindowOperator
	if (node->type == T_SetOperator) {
		if (((SetOperator *) node)->setOpType == SETOP_DIFFERENCE) {
			addToSet(operatorSet, SET_OPERATOR);
		}
	} //Check set difference
	if (node->type == T_JoinOperator) {
		JoinOperator *j = (JoinOperator *) node;
		if (j->joinType == JOIN_LEFT_OUTER || j->joinType == JOIN_RIGHT_OUTER
				|| j->joinType == JOIN_FULL_OUTER) {
			addToSet(operatorSet, JOIN_OPERATOR);
		}
	} //Check outer join
	if (node->type == T_NestingOperator) {
		addToSet(operatorSet, NESTING_OPERATOR);
	} //Check nesting

	return visit(node, checkMonotone, operatorSet);
}

boolean hasOperator(Node* node, Set *operatorSet) {
	if (node == NULL)
		return TRUE;
	if (node->type == T_OrderOperator) {
		addToSet(operatorSet, ORDER_OPERATOR);
	} //Check aggreationOperator
	if (node->type == T_SelectionOperator) {
		addToSet(operatorSet, SELECTION_OPERATOR);
	} //Check aggreationOperator
	if (node->type == T_WindowOperator) {
		addToSet(operatorSet, WINDOW_OPERATOR);
	} //Check aggreationOperator
	if (node->type == T_AggregationOperator) {
		addToSet(operatorSet, AGGREGATION_OPERATOR);
	} //Check aggreationOperator
	return visit(node, hasOperator, operatorSet);
}

HashMap *
getSchema(Node* qbModel) {
	HashMap *map = NEW_MAP(Constant, Node);
	//getTableAccessOperator(qbModel, map);
	//getSubset(qbModel, map);
	getTableAccessOperator(qbModel, map);
	return map;
}	//get schema of table

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
}	//return the result map for all sketches.

HashMap *
safetyCheck(Node* qbModel, Set *hasOperator) {
	HashMap *map = NEW_MAP(Constant, Node);
	HashMap *data = NEW_MAP(Constant, Node);
	getData(qbModel, data);	//get the data of node we need
	boolean result = FALSE;
	if (hasSetElem(hasOperator, ORDER_OPERATOR)) {
		//DEBUG_LOG("it's order");
		result = checkPageSafety_rownum(data);		//rownum check
		DEBUG_LOG("The result is: %d", result);
	} else {
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

boolean getSafeProvenanceSketch(Node* node, HashMap *map) {
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
		char *groupByColName =
				((AttributeReference *) getHeadOfList(groupby)->data.ptr_value)->name;
		char *tableName = findTable(table_map, groupByColName);

		if (findSetDifference(operatorList)) {
			HashMap *page_map =
					(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
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
					if (monotonicity2(OrderOperator, node, table_map,
							groupByColName)) {
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
						HashMap *page_map =
								(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
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
				HashMap *page_map =
						(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
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
			HashMap *page_map =
					(HashMap *) MAP_GET_STRING_ENTRY(result,PAGE)->value;
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
findFiltering(List* operatorList) {
	Set *operatorSet = STRSET();
	FOREACH(Node,operator,operatorList)
		if (operator->type == T_AggregationOperator) {
			addToSet(operatorSet, AGGREGATION_OPERATOR);
		} else if (operator->type == T_SelectionOperator) {
			addToSet(operatorSet, SELECTION_OPERATOR);
		} else if (operator->type == T_DuplicateRemoval) {
			addToSet(operatorSet, DUPLICATEREMOVAL);
		} else if (operator->type == T_WindowOperator) {
			addToSet(operatorSet, WINDOW_OPERATOR);
		} else if (operator->type == T_JoinOperator) {
			addToSet(operatorSet, JOIN_OPERATOR);
		} else if (operator->type == T_SetOperator) {
			if (((SetOperator *) operator)->setOpType == SETOP_INTERSECTION) {
				addToSet(operatorSet, SETOPINTERSECTION);
			}
		} else {
			continue;
		}
	return operatorSet;
}

boolean findSetDifference(List* operatorList) {
	FOREACH(Node,operator,operatorList)
	{
		if (operator->type == T_SetOperator) {
			if (((SetOperator *) operator)->setOpType == SETOP_DIFFERENCE) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

OrderOperator*
findOrderOperator(List* operatorList) {
	FOREACH(Node,operator,operatorList)
	{
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
		List *attrDef =
				(List *) MAP_GET_STRING_ENTRY(table_map, table->value)->value;
		FOREACH(AttributeDef, attr, attrDef)
		{
			if (!strcmp(attr->attrName, colName)) {
				tableName = table->value;
			}
		}
	}
	return tableName;
}

HashMap*
updateResultMap(HashMap *map, HashMap *orginalResult, char *type,
		HashMap *newResultMapOfType) {
	//HashMap *newMap = NEW_MAP(Constant, Node);
	MAP_ADD_STRING_KEY(orginalResult, type, (Node * )newResultMapOfType);
	MAP_ADD_STRING_KEY(map, "Result", (Node * )orginalResult);
	return map;
}

HashMap*
hashRangeOnGroupbySafe(HashMap *map, HashMap *result, char *tableName,
		char *groupByColName) {
	HashMap *hash_map = (HashMap *) MAP_GET_STRING_ENTRY(result,HASH)->value;
	HashMap *range_map = (HashMap *) MAP_GET_STRING_ENTRY(result,RANGE)->value;
	List *attributesList =
			(List *) MAP_GET_STRING_ENTRY(hash_map,tableName)->value;
	List *newAttributesList = NIL;
	FOREACH(AttributeDef,attribute,attributesList)
	{
		if (!strcmp(attribute->attrName, groupByColName)) {
			newAttributesList = appendToTailOfList(newAttributesList,
					attribute);
		}
	}
	MAP_ADD_STRING_KEY(hash_map, tableName, (Node * )newAttributesList);
	MAP_ADD_STRING_KEY(range_map, tableName, (Node * )newAttributesList);
	map = updateResultMap(map, result, HASH, hash_map);
	map = updateResultMap(map, result, RANGE, range_map);
	return map;
}

HashMap*
hashRangeOnGroupbyNotSafe(HashMap *map, HashMap *result, char *tableName,
		char *groupByColName) {
	HashMap *hash_map = (HashMap *) MAP_GET_STRING_ENTRY(result,HASH)->value;
	HashMap *range_map = (HashMap *) MAP_GET_STRING_ENTRY(result,RANGE)->value;
	MAP_ADD_STRING_KEY(hash_map, tableName, (Node *)NIL);
	MAP_ADD_STRING_KEY(range_map, tableName, (Node *)NIL);
	map = updateResultMap(map, result, HASH, hash_map);
	map = updateResultMap(map, result, RANGE, range_map);
	return map;
}

boolean monotonicity1(SelectionOperator *selectOperator, Node *node,
		HashMap *table_map) {
	char *function_name;
	char *colName;
	if (node->type == T_AggregationOperator) {
		AggregationOperator *aggregationOperator = (AggregationOperator *) node;
		List *aggrs = (List *) aggregationOperator->aggrs;
		function_name =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
		List *args =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
		colName =
				((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}
	if (node->type == T_WindowOperator) {
		WindowOperator *windowOperator = (WindowOperator *) node;
		Node *f = windowOperator->f;
		function_name = ((FunctionCall *) f)->functionname;
		List *args = ((FunctionCall *) f)->args;
		colName =
				((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
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

boolean monotonicity2(OrderOperator *orderOperator, Node *node,
		HashMap *table_map, char *groupByColName) {
	char *function_name;
	char *colName;

	if (node->type == T_AggregationOperator) {
		AggregationOperator *aggregationOperator = (AggregationOperator *) node;
		List *aggrs = (List *) aggregationOperator->aggrs;
		function_name =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
		List *args =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
		colName =
				((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}
	if (node->type == T_WindowOperator) {
		WindowOperator *windowOperator = (WindowOperator *) node;
		Node *f = windowOperator->f;
		function_name = ((FunctionCall *) f)->functionname;
		List *args = ((FunctionCall *) f)->args;
		colName =
				((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;
	}

	List *orderExprs = (List *) orderOperator->orderExprs;
	Node *attribute_reference =
			((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->expr;
	char *orderby_name = ((AttributeReference *) attribute_reference)->name;
	SortOrder order =
			((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->order;

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

boolean getTableAccessOperator(Node* node, HashMap *map) {
	if (node == NULL)
		return TRUE;

	if (node->type == T_TableAccessOperator) {
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		MAP_ADD_STRING_KEY(map, tablename, (Node * )attrDef);
	}

	return visit(node, getTableAccessOperator, map);
}		//get the table

boolean getAll(Node* node, HashMap *map) {
	if (node == NULL)
		return TRUE;

	if (node->type == T_TableAccessOperator) {
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		MAP_ADD_STRING_KEY(map, tablename, (Node * )attrDef);
	}

	return visit(node, getAll, map);
}		//get the attribute of the tabe

boolean getSubset(Node* node, HashMap *map) {
	if (node == NULL)
		return TRUE;

	if (node->type == T_TableAccessOperator) {
		char *tablename = ((TableAccessOperator *) node)->tableName;
		Schema *schema = ((TableAccessOperator *) node)->op.schema;
		List *attrDef = schema->attrDefs;
		unsigned int length = (unsigned int) getListLength(attrDef);
		List *result = NIL;
		result = addBitset(length, result);
		MAP_ADD_STRING_KEY(map, tablename, (Node * )result);
	}

	return visit(node, getSubset, map);
}		//get the KeyValue of each table

List*
addBitset(unsigned int length, List *result) {
	char *subset = "SUBSET";
//	char *exact = "EXCAT";
//	unsigned long max = 1 << length;
	for (int i = 0; i < length; i++) {
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
		KeyValue *element = createStringKeyValue(subset,
				bitSetToString(bitset));
		result = appendToTailOfList(result, element);
	}
	return result;
}		//get BitSet of all subsets

boolean getData(Node* node, HashMap *data) {
	if (node == NULL)
		return TRUE;

	if (node->type == T_AggregationOperator) {
		//char *aggregation_key = "aggregation";
		HashMap *aggreation_map = NEW_MAP(Constant, Node);

		char *aggrs_key = "aggrs";
		char *groupby_key = "groupby";
		List *aggrs = ((AggregationOperator *) node)->aggrs;
		List *groupby = ((AggregationOperator *) node)->groupBy;

		MAP_ADD_STRING_KEY(aggreation_map, aggrs_key, (Node * )aggrs);
		MAP_ADD_STRING_KEY(aggreation_map, groupby_key, (Node * )groupby);

		MAP_ADD_STRING_KEY(data, AGGREGATION_OPERATOR, (Node * )aggreation_map);
	}
	if (node->type == T_WindowOperator) {
		//char *WindowOperator_key = "WindowOperator";
		HashMap *WindowOperator_map = NEW_MAP(Constant, Node);
		char *f_key = "f";
		char *partitionBy_key = "partitionBy";
		Node *f = ((WindowOperator *) node)->f;
		List *partitionBy = ((WindowOperator *) node)->partitionBy;

		MAP_ADD_STRING_KEY(WindowOperator_map, f_key, (Node * )f);
		MAP_ADD_STRING_KEY(WindowOperator_map, partitionBy_key,
				(Node * )partitionBy);

		MAP_ADD_STRING_KEY(data, WINDOW_OPERATOR, (Node * )WindowOperator_map);
	}
	if (node->type == T_SelectionOperator) {
		//char *SelectionOperator_key = "SelectionOperator";
		Node *cond = ((SelectionOperator *) node)->cond;
		MAP_ADD_STRING_KEY(data, SELECTION_OPERATOR, (Node * )cond);
	}
	if (node->type == T_OrderOperator) {
		//char *OrderOperator_key = "OrderOperator";
		List *orderExprs = ((OrderOperator *) node)->orderExprs;
		MAP_ADD_STRING_KEY(data, ORDER_OPERATOR, (Node * )orderExprs);
	}
	if (node->type == T_TableAccessOperator) {
		//char *TableAccessOperator_key = "TableAccessOperator";
		if (hasMapStringKey(data, TABLEACCESS_OPERATOR)) {

			HashMap *TableAccessOperator_map = NEW_MAP(Constant, Node);
			char *tablename = ((TableAccessOperator *) node)->tableName;
			Schema *schema = ((TableAccessOperator *) node)->op.schema;
			List *attrDef = schema->attrDefs;
			TableAccessOperator_map =
					(HashMap *) MAP_GET_STRING_ENTRY(data, TABLEACCESS_OPERATOR)->value;
			MAP_ADD_STRING_KEY(TableAccessOperator_map, tablename,
					(Node * )attrDef);
			MAP_ADD_STRING_KEY(data, TABLEACCESS_OPERATOR,
					TableAccessOperator_map);
		} else {
			HashMap *TableAccessOperator_map = NEW_MAP(Constant, Node);
			char *tablename = ((TableAccessOperator *) node)->tableName;
			Schema *schema = ((TableAccessOperator *) node)->op.schema;
			List *attrDef = schema->attrDefs;
			MAP_ADD_STRING_KEY(TableAccessOperator_map, tablename,
					(Node * )attrDef);
			MAP_ADD_STRING_KEY(data, TABLEACCESS_OPERATOR,
					TableAccessOperator_map);
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

boolean checkPageSafety_rownum(HashMap *data) {
	//char *OrderOperator_key = "OrderOperator";
	List *orderExprs =
			(List *) MAP_GET_STRING_ENTRY(data, ORDER_OPERATOR)->value;
	Node *attribute_reference =
			((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->expr;
	char *orderby_name = ((AttributeReference *) attribute_reference)->name;
	SortOrder order =
			((OrderExpr *) getHeadOfList(orderExprs)->data.ptr_value)->order;

	//char *aggregation_key = "aggregation";
	HashMap *aggreation_map =
			(HashMap *) MAP_GET_STRING_ENTRY(data, AGGREGATION_OPERATOR)->value;
	char *groupby_key = "groupby";
	List *groupby =
			(List *) MAP_GET_STRING_ENTRY(aggreation_map, groupby_key)->value;
	char *groupby_name =
			((AttributeReference *) getHeadOfList(groupby)->data.ptr_value)->name;

	char *aggrs_key = "aggrs";
	List *aggrs =
			(List *) MAP_GET_STRING_ENTRY(aggreation_map, aggrs_key)->value;
	char *function_name =
			((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *colName =
			((AttributeReference *) getHeadOfList(args)->data.ptr_value)->name;

	//DEBUG_LOG("The COLNAME is: %s", colName);
	//char *TableAccessOperator_key = "TableAccessOperator";
	HashMap *table_map =
			(HashMap *) MAP_GET_STRING_ENTRY(data, TABLEACCESS_OPERATOR)->value;
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
		List *attrDef =
				(List *) MAP_GET_STRING_ENTRY(table_map, table->value)->value;
		FOREACH(AttributeDef, attr, attrDef)
		{
			if (!strcmp(attr->attrName, colName)) {
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
		List *attrDef =
				(List *) MAP_GET_STRING_ENTRY(table_map, table->value)->value;
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

void test(Node *qbModel) {
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
	MAP_ADD_STRING_KEY(map_list_result, "List", (Node * )operatorList);
	MAP_ADD_STRING_KEY(map_list_result, "Result", (Node * )result);
	MAP_ADD_STRING_KEY(map_list_result, "Schema", (Node * )schema_map);
	MAP_ADD_STRING_KEY(map_list_result, "Table_map", (Node * )table_map);
	getSafeProvenanceSketch(qbModel, map_list_result);
	List *list = (List *) MAP_GET_STRING_ENTRY(map_list_result, "List")->value;
	DEBUG_NODE_BEATIFY_LOG("The list is:", list);
	result = (HashMap *) MAP_GET_STRING_ENTRY(map_list_result, "Result")->value;
	DEBUG_NODE_BEATIFY_LOG("The test is:", result);
	schema_map =
			(HashMap *) MAP_GET_STRING_ENTRY(map_list_result, "Schema")->value;
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

boolean decodeQuery(Node* node, HashMap *data) {
	if (node == NULL)
		return TRUE;
	if (node->type == T_AggregationOperator) {
		//char *aggregation_key = "aggregation";
		//HashMap *aggreation_map = NEW_MAP(Constant, Node);
		char *aggrs_key = "aggrs";
		char *groupby_key = "groupby";
		List *aggrs = ((AggregationOperator *) node)->aggrs;
		List *groupby = ((AggregationOperator *) node)->groupBy;
		MAP_ADD_STRING_KEY(data, aggrs_key, (Node * )aggrs);
		MAP_ADD_STRING_KEY(data, groupby_key, (Node * )groupby);

	}
	if (node->type == T_SelectionOperator) {
		//char *SelectionOperator_key = "SelectionOperator";
		Node *cond = ((SelectionOperator *) node)->cond;
		MAP_ADD_STRING_KEY(data, "selection", (Node * )cond);
	}
	return visit(node, decodeQuery, data);
}
/*
void runSelectivity(Node *qbModel) {
	HashMap *data = NEW_MAP(Constant, Node);
	decodeQuery(qbModel,data);
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(data, "aggrs")->value;
	char *functionname = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args =((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *aggrName =((AttributeReference *) getHeadOfList((List *) args)->data.ptr_value)->name;
	List *groupby = (List *) MAP_GET_STRING_ENTRY(data, "groupby")->value;
	char *groupbyAttr = "";
	int length = groupby->length;
	int count = 0;
	FOREACH(AttributeReference, gb, groupby)
	{

		groupbyAttr = CONCAT_STRINGS(groupbyAttr, gb->name);
		if (count == (length -1)) {
			continue;
		}
		groupbyAttr = CONCAT_STRINGS(groupbyAttr, ",");
		count++;

	}
	Operator *cond = (Operator *) MAP_GET_STRING_ENTRY(data, "selection")->value;
	List *condArgs = cond->args;
	//AttributeReference *attributeReference =(AttributeReference *) getHeadOfList(args)->data.ptr_value;
	Constant *constant = (Constant *) getTailOfList(condArgs)->data.ptr_value;
	int selectionConstant = *((int *) constant->value);
	char t[100];
	sprintf(t, "%d", selectionConstant);
	//DEBUG_NODE_BEATIFY_LOG("lzy is:",constant);
	DEBUG_LOG("LZY functionname is %s, aggrName is %s, groupbyAttr is %s, constant is %s",functionname, aggrName, groupbyAttr, t);
	FOREACH(AttributeReference, gb, groupby){
		if (!strcmp(gb->name, "U_VIEWS")|| !strcmp(gb->name, "U_REPUTATION") || !strcmp(gb->name, "U_UPVOTES")) {
			char *tableNmae = CONCAT_STRINGS("USERS#",gb->name,"_bid");
			//char* res = getSamplesDirectly("CID,BEAT,COMMUNITY_AREA,LATITUDE, LONGITUDE,ZIP_CODES, CYEAR,DISTRICT, WARD, CBLOCK, PRIMARY_TYPE,CDESCRIPTION, LOCATION_DESCRIPTION, X_COORDINATE,Y_COORDINATE",gb->name,"0.05",tableNmae);
			char *res = CONCAT_STRINGS("SAMPLE_",tableNmae,"_5");
			char *sampleStat = getSampleStat(res, aggrName, groupbyAttr, "20");
			char *query =  CONCAT_STRINGS("SELECT * FROM (select ",functionname,"(",aggrName,") AS ",functionname,"_",aggrName,", ",groupbyAttr," from users GROUP BY ",groupbyAttr,") WHERE ",functionname,"_",aggrName," >",t);
			DEBUG_LOG("lzy is %s", query);
			storeSelectivty(sampleStat, gb->name, functionname, aggrName, groupbyAttr,"users", t, "selectivity_store_users", query,"5");
		}
	}

	//storeSelectivty(sampleStat, "L_ORDERKEY", "SUM", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "202000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >202000","100");
	//storeSelectivty(sampleStat, "L_ORDERKEY", "SUM", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "352000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >352000","100");

}
*/
void
generateQuery_CRIMES() {
	char *aggr[] = {"SUM"};
	//char *aggr[] = { "SUM"};
	//char *lineitem[][2] = { {"L_SUPPKEY","NUMBER"}, {"L_ORDERKEY","NUMBER"}, {"L_PARTKEY","NUMBER"}, {"L_SHIPDATE","VARCHAR"}, {"L_LINENUMBER","NUMBER"}, {"L_QUANTITY","NUMBER"},
	//		{"L_EXTENDEDPRICE","NUMBER"}, {"L_DISCOUNT","NUMBER"}, {"L_TAX","NUMBER"}, {"L_LINESTATUS","VARCHAR"}};
	char *USERS[][2] = { { "CID", "NUMBER" },{ "ZIP_CODES", "NUMBER" },{ "DISTRICT", "NUMBER" },{ "WARD", "NUMBER" },
			{ "BEAT", "NUMBER" }, {"COMMUNITY_AREA", "NUMBER" },{ "LATITUDE", "NUMBER" },{ "LONGITUDE", "NUMBER" },{ "CYEAR", "NUMBER" }, { "X_COORDINATE", "NUMBER" },
			{ "Y_COORDINATE", "NUMBER" },{"CBLOCK", "VARCHAR" },{"PRIMARY_TYPE", "VARCHAR" },{"CDESCRIPTION", "VARCHAR" },{"LOCATION_DESCRIPTION", "VARCHAR" }};
	int length_aggr = sizeof(aggr) / sizeof(aggr[0]);
	int length_lineitem = sizeof(USERS) / sizeof(USERS[0]);
	for (int i = 0; i < length_aggr; i++) {
		DEBUG_LOG("LZY length_aggr is %d,%d", length_aggr, length_lineitem);
		for (int j = 0; j < length_lineitem; j++) {
			if (!strcmp(USERS[j][1], "NUMBER")) {
				if (!strcmp(USERS[j][0], "CID")) {
					continue;
				} else {
					for (int k = 0; k < (length_lineitem - 1); k++) {
						for (int m = k + 1; m < length_lineitem; m++) {
							if (!strcmp(USERS[k][0], USERS[j][0])|| !strcmp(USERS[m][0],USERS[j][0])) {
								continue;
							} else {
								if (strcmp(USERS[k][0], "ZIP_CODES")
										&& strcmp(USERS[k][0], "BEAT")
										&& strcmp(USERS[k][0], "COMMUNITY_AREA")
										&& strcmp(USERS[k][0], "WARD")
										&& strcmp(USERS[k][0], "LONGITUDE")
										&& strcmp(USERS[k][0], "LATITUDE")
										&& strcmp(USERS[m][0], "ZIP_CODES")
										&& strcmp(USERS[m][0], "BEAT")
										&& strcmp(USERS[m][0], "COMMUNITY_AREA")
										&& strcmp(USERS[k][0], "WARD")
										&& strcmp(USERS[k][0], "LONGITUDE")
										&& strcmp(USERS[k][0], "LONGITUDE")) {
									continue;
								} else {
									char *query = CONCAT_STRINGS(
											"SELECT * FROM (select ", aggr[i],
											"(", USERS[j][0], ") AS ",
											aggr[i], "_", USERS[j][0], ", ",
											USERS[k][0], ", ",
											USERS[m][0],
											" FROM CRIMES GROUP BY ",
											USERS[k][0], ", ",
											USERS[m][0], ")");
									char *aggName = CONCAT_STRINGS(aggr[i], "_",
											USERS[j][0]);
									char *max = findTheMax(query, aggName);

									char *sel_query = printQuery(max, 0.7,
											query, aggr[i], USERS[j][0]);
									char *path = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy_crimes2/",
											aggr[i], "_", USERS[j][0], "_",
											USERS[k][0], "_", USERS[m][0],
											"_1");
									FILE *fpWrite = fopen(path, "w");
									fprintf(fpWrite, "%s ", sel_query);
									fclose(fpWrite);

									char *sel_query2 = printQuery(max, 0.8,
											query, aggr[i], USERS[j][0]);
									char *path2 = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy_crimes2/",
											aggr[i], "_", USERS[j][0], "_",
											USERS[k][0], "_", USERS[m][0],
											"_2");
									FILE *fpWrite2 = fopen(path2, "w");
									fprintf(fpWrite2, "%s ", sel_query2);
									fclose(fpWrite2);

									char *sel_query3 = printQuery(max, 0.9,
											query, aggr[i], USERS[j][0]);
									char *path3 = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy_crimes2/",
											aggr[i], "_", USERS[j][0], "_",
											USERS[k][0], "_", USERS[m][0],
											"_3");
									FILE *fpWrite3 = fopen(path3, "w");
									fprintf(fpWrite3, "%s ", sel_query3);
									fclose(fpWrite3);
								}
							}
						}
					}
				}
			}
		}
	}

}
/*
void runSelectivity(Node *qbModel) {
	HashMap *data = NEW_MAP(Constant, Node);
	decodeQuery(qbModel,data);
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(data, "aggrs")->value;
	char *functionname = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args =((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *aggrName =((AttributeReference *) getHeadOfList((List *) args)->data.ptr_value)->name;
	List *groupby = (List *) MAP_GET_STRING_ENTRY(data, "groupby")->value;
	char *groupbyAttr = "";
	int length = groupby->length;
	int count = 0;
	FOREACH(AttributeReference, gb, groupby)
	{

		groupbyAttr = CONCAT_STRINGS(groupbyAttr, gb->name);
		if (count == (length -1)) {
			continue;
		}
		groupbyAttr = CONCAT_STRINGS(groupbyAttr, ",");
		count++;

	}
	Operator *cond = (Operator *) MAP_GET_STRING_ENTRY(data, "selection")->value;
	List *condArgs = cond->args;
	//AttributeReference *attributeReference =(AttributeReference *) getHeadOfList(args)->data.ptr_value;
	Constant *constant = (Constant *) getTailOfList(condArgs)->data.ptr_value;
	int selectionConstant = *((int *) constant->value);
	char t[100];
	sprintf(t, "%d", selectionConstant);
	//DEBUG_NODE_BEATIFY_LOG("lzy is:",constant);
	DEBUG_LOG("LZY functionname is %s, aggrName is %s, groupbyAttr is %s, constant is %s",functionname, aggrName, groupbyAttr, t);
	FOREACH(AttributeReference, gb, groupby){
		if (!strcmp(gb->name, "U_VIEWS")|| !strcmp(gb->name, "U_UPVOTES") || !strcmp(gb->name, "U_REPUTATION")) {
			char *tableNmae = CONCAT_STRINGS("users#",gb->name,"_bid");
			//char* res = getSamplesDirectly("U_REPUTATION, U_ID,U_DISPLAYNAME, U_VIEWS, U_UPVOTES, U_DOWNVOTES",gb->name,"0.05",tableNmae);
			char *res = CONCAT_STRINGS("SAMPLE_",tableNmae,"_5");
			char *sampleStat = getSampleStat(res, aggrName, groupbyAttr, "20");
			char *query =  CONCAT_STRINGS("SELECT * FROM (select ",functionname,"(",aggrName,") AS ",functionname,"_",aggrName,", ",groupbyAttr," from users GROUP BY ",groupbyAttr,") WHERE ",functionname,"_",aggrName," >",t);
			DEBUG_LOG("lzy is %s", query);
			storeSelectivty(sampleStat, gb->name, functionname, aggrName, groupbyAttr,"users", t, "selectivity_store_users2", query,"5");
		}
	}

	//storeSelectivty(sampleStat, "L_ORDERKEY", "SUM", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "202000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >202000","100");
	//storeSelectivty(sampleStat, "L_ORDERKEY", "SUM", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "352000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >352000","100");

}
void
generateQuery() {
	char *aggr[] = {"SUM", "COUNT", "AVG"};
	//char *aggr[] = { "SUM"};
	//char *lineitem[][2] = { {"L_SUPPKEY","NUMBER"}, {"L_ORDERKEY","NUMBER"}, {"L_PARTKEY","NUMBER"}, {"L_SHIPDATE","VARCHAR"}, {"L_LINENUMBER","NUMBER"}, {"L_QUANTITY","NUMBER"},
	//		{"L_EXTENDEDPRICE","NUMBER"}, {"L_DISCOUNT","NUMBER"}, {"L_TAX","NUMBER"}, {"L_LINESTATUS","VARCHAR"}};
	char *USERS[][2] = { { "U_REPUTATION", "NUMBER" },
			{ "U_ID", "NUMBER" }, { "U_DISPLAYNAME", "VARCHAR" }, {
					"U_VIEWS", "NUMBER" }, { "U_UPVOTES", "NUMBER" }, {
					"U_DOWNVOTES", "NUMBER" }};
	int length_aggr = sizeof(aggr) / sizeof(aggr[0]);
	int length_lineitem = sizeof(USERS) / sizeof(USERS[0]);
	for (int i = 0; i < length_aggr; i++) {
		DEBUG_LOG("LZY length_aggr is %d,%d", length_aggr, length_lineitem);
		for (int j = 0; j < length_lineitem; j++) {
			if (!strcmp(USERS[j][1], "NUMBER")) {
				if (!strcmp(USERS[j][0], "U_ID")) {
					continue;
				} else {
					for (int k = 0; k < (length_lineitem - 1); k++) {
						for (int m = k + 1; m < length_lineitem; m++) {
							if (!strcmp(USERS[k][0], USERS[j][0])|| !strcmp(USERS[m][0],USERS[j][0])) {
								continue;
							} else {
								if (strcmp(USERS[k][0], "U_VIEWS")
										&& strcmp(USERS[k][0], "U_UPVOTES")
										&& strcmp(USERS[k][0], "U_REPUTATION")
										&& strcmp(USERS[m][0], "U_VIEWS")
										&& strcmp(USERS[m][0], "U_UPVOTES")
										&& strcmp(USERS[m][0], "U_REPUTATION")) {
									continue;
								} else {
									char *query = CONCAT_STRINGS(
											"SELECT * FROM (select ", aggr[i],
											"(", USERS[j][0], ") AS ",
											aggr[i], "_", USERS[j][0], ", ",
											USERS[k][0], ", ",
											USERS[m][0],
											" FROM USERS GROUP BY ",
											USERS[k][0], ", ",
											USERS[m][0], ")");
									char *aggName = CONCAT_STRINGS(aggr[i], "_",
											USERS[j][0]);
									char *max = findTheMax(query, aggName);

									char *sel_query = printQuery(max, 0.3,
											query, aggr[i], USERS[j][0]);
									char *path = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy_USERS/",
											aggr[i], "_", USERS[j][0], "_",
											USERS[k][0], "_", USERS[m][0],
											"_1");
									FILE *fpWrite = fopen(path, "w");
									fprintf(fpWrite, "%s ", sel_query);
									fclose(fpWrite);

									char *sel_query2 = printQuery(max, 0.5,
											query, aggr[i], USERS[j][0]);
									char *path2 = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy_USERS/",
											aggr[i], "_", USERS[j][0], "_",
											USERS[k][0], "_", USERS[m][0],
											"_2");
									FILE *fpWrite2 = fopen(path2, "w");
									fprintf(fpWrite2, "%s ", sel_query2);
									fclose(fpWrite2);

									char *sel_query3 = printQuery(max, 0.7,
											query, aggr[i], USERS[j][0]);
									char *path3 = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy_USERS/",
											aggr[i], "_", USERS[j][0], "_",
											USERS[k][0], "_", USERS[m][0],
											"_3");
									FILE *fpWrite3 = fopen(path3, "w");
									fprintf(fpWrite3, "%s ", sel_query3);
									fclose(fpWrite3);
								}
							}
						}
					}
				}
			}
		}
	}

}*/

void runSelectivity(Node *qbModel) {
	HashMap *data = NEW_MAP(Constant, Node);
	decodeQuery(qbModel,data);
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(data, "aggrs")->value;
	char *functionname = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args =((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *aggrName =((AttributeReference *) getHeadOfList((List *) args)->data.ptr_value)->name;
	List *groupby = (List *) MAP_GET_STRING_ENTRY(data, "groupby")->value;
	char *groupbyAttr = "";
	char *groupbyAttr2 = "";
	int length = groupby->length;
	int count = 0;
	FOREACH(AttributeReference, gb, groupby)
	{

		groupbyAttr = CONCAT_STRINGS(groupbyAttr, gb->name);
		groupbyAttr2 = CONCAT_STRINGS(groupbyAttr2, gb->name);
		if (count == (length -1)) {
			continue;
		}
		groupbyAttr = CONCAT_STRINGS(groupbyAttr, ",");
		groupbyAttr2 = CONCAT_STRINGS(groupbyAttr2, "_");
		count++;

	}
	Operator *cond = (Operator *) MAP_GET_STRING_ENTRY(data, "selection")->value;
	List *condArgs = cond->args;
	//AttributeReference *attributeReference =(AttributeReference *) getHeadOfList(args)->data.ptr_value;
	Constant *constant = (Constant *) getTailOfList(condArgs)->data.ptr_value;
	int selectionConstant = *((int *) constant->value);
	char t[100];
	sprintf(t, "%d", selectionConstant);
	//DEBUG_NODE_BEATIFY_LOG("lzy is:",constant);
	DEBUG_LOG("LZY functionname is %s, aggrName is %s, groupbyAttr is %s, constant is %s",functionname, aggrName, groupbyAttr, t);
	char *count_table = storeGroupbyCount(groupbyAttr,groupbyAttr2,"LINEITEM");

	FOREACH(AttributeReference, gb, groupby){
		if (!strcmp(gb->name, "L_SUPPKEY")|| !strcmp(gb->name, "L_ORDERKEY") || !strcmp(gb->name, "L_PARTKEY")|| !strcmp(gb->name, "L_EXTENDEDPRICE")) {
			char *tableNmae = CONCAT_STRINGS("tpch#",gb->name,"_bid");
			char* res = getSamplesDirectly("L_SUPPKEY, L_ORDERKEY,L_PARTKEY, L_EXTENDEDPRICE, L_QUANTITY, L_LINENUMBER, L_DISCOUNT, L_TAX",gb->name,"0.05",tableNmae);
			//char *res = CONCAT_STRINGS("SAMPLE_",tableNmae,"_5");
			char *sampleStat = getSampleStat(res, aggrName, groupbyAttr, "20", count_table);
			char *query =  CONCAT_STRINGS("SELECT * FROM (select ",functionname,"(",aggrName,") AS ",functionname,"_",aggrName,", ",groupbyAttr," from lineitem GROUP BY ",groupbyAttr,") WHERE ",functionname,"_",aggrName," >",t);
			DEBUG_LOG("lzy is %s", query);
			storeSelectivty(sampleStat, gb->name, functionname, aggrName, groupbyAttr,"tpch", t, "selectivity_store_test", query,"5");
		}
	}

	//storeSelectivty(sampleStat, "L_ORDERKEY", "SUM", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "202000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >202000","100");
	//storeSelectivty(sampleStat, "L_ORDERKEY", "SUM", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "352000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >352000","100");

}

void runSelectivity2(Node *qbModel) {
	HashMap *data = NEW_MAP(Constant, Node);
	decodeQuery(qbModel,data);
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(data, "aggrs")->value;
	char *functionname = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args =((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *aggrName =((AttributeReference *) getHeadOfList((List *) args)->data.ptr_value)->name;
	List *groupby = (List *) MAP_GET_STRING_ENTRY(data, "groupby")->value;
	char *groupbyAttr = "";
	char *groupbyAttr1_groupbyAttr2 = "";
	char *groupbyAttr1 = "";
	char *groupbyAttr2 = "";
	int length = groupby->length;
	int count = 0;
	FOREACH(AttributeReference, gb, groupby)
	{

		groupbyAttr = CONCAT_STRINGS(groupbyAttr, gb->name);
		groupbyAttr1_groupbyAttr2 = CONCAT_STRINGS(groupbyAttr1_groupbyAttr2, gb->name);
		if (count == 0) {
			groupbyAttr1 = CONCAT_STRINGS(groupbyAttr1, gb->name);
		}
		if (count == 1) {
			groupbyAttr2 = CONCAT_STRINGS(groupbyAttr2, gb->name);
		}
		if (count == (length -1)) {
			continue;
		}
		groupbyAttr = CONCAT_STRINGS(groupbyAttr, ",");
		groupbyAttr1_groupbyAttr2 = CONCAT_STRINGS(groupbyAttr1_groupbyAttr2, "_");
		count++;

	}
	Operator *cond = (Operator *) MAP_GET_STRING_ENTRY(data, "selection")->value;
	List *condArgs = cond->args;
	//AttributeReference *attributeReference =(AttributeReference *) getHeadOfList(args)->data.ptr_value;
	Constant *constant = (Constant *) getTailOfList(condArgs)->data.ptr_value;
	int selectionConstant = *((int *) constant->value);
	char t[100];
	sprintf(t, "%d", selectionConstant);
	char *query = CONCAT_STRINGS("SELECT * FROM (select ", functionname, "(",
				aggrName, ") AS ", functionname, "_", aggrName, ", ", groupbyAttr,
				" from lineitem GROUP BY ", groupbyAttr, ") WHERE ", functionname,
				"_", aggrName, " >", t);
	/*char *count_table = "";
	if(catalogTableExists(CONCAT_STRINGS("COUNT_LINEITEM_",groupbyAttr1_groupbyAttr2))){

	} else {

	}*/
	DEBUG_LOG("LZY query is %s", query);
	/*DEBUG_LOG("LZY query is %s", query);
	char *ps[] = {"L_QUANTITY","L_LINENUMBER","L_TAX","L_DISCOUNT"};
	for(int i=0; i<sizeof(ps)/sizeof(ps[0]); i++){
		insertSelectivity(t, groupbyAttr, aggrName, ps[i], "SUM", "selectivity_store_lzy", "5");
		insertSelectivity(t, groupbyAttr, aggrName, ps[i], "SUM", "selectivity_store_lzy", "10");
	}*/

//////////////////////////////////////////////////////
	//int count0 = 0;
	//List *samples_test = NIL;
	//char *ps[] = {"ZIP_CODES","BEAT","COMMUNITY_AREA","WARD"};
	//for(int i=0; i<sizeof(ps)/sizeof(ps[0]); i++){
		//char* sampleName_5 = CONCAT_STRINGS("SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","5");
		//char* sampleName_10 = CONCAT_STRINGS("SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","10");
		//char* partsampleName_5 = CONCAT_STRINGS("PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","5");
		//char* partsampleName_10 = CONCAT_STRINGS("PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","10");
		//char* statpartsampleName_5 = CONCAT_STRINGS("STAT_PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","5");
		//char* statpartsampleName_10 = CONCAT_STRINGS("STAT_PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","10");
		//char* statpartsampleName_5_agg = CONCAT_STRINGS("STAT_PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","5_AGG_",aggrName);
		//char* statpartsampleName_10_agg = CONCAT_STRINGS("STAT_PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","10_AGG_",aggrName);
		//char* new = CONCAT_STRINGS("STAT_SAMPLE_CRIMES#", ps[i],"_BID_5_",groupbyAttr1_groupbyAttr2,"_AGG_",aggrName);
		//char* new2 = CONCAT_STRINGS("STAT_PARTIAL_JOIN_COUNT_PARTIAL_",groupbyAttr1_groupbyAttr2,"_",aggrName,"_PS_",ps[i],"_5");
		//char* new3 = CONCAT_STRINGS("STAT_PARTIAL_JOIN_COUNT_PARTIAL_",groupbyAttr1_groupbyAttr2,"_",aggrName,"_PS_",ps[i]);
		//dropTable(partsampleName_5);
		//dropTable(partsampleName_10);
		//dropTable(statpartsampleName_5);
		//dropTable(statpartsampleName_10);
		//dropTable(statpartsampleName_5_agg);
		//dropTable(statpartsampleName_10_agg);
	//	dropTable(new);
		//dropTable(new2);
		//dropTable(new3);

//	}

	char *ps[] = {"STAT_SAMPLE_CRIMES#BEAT_BID_5_BEAT_CLOCK_AGG_LONGITUDE",
			"STAT_SAMPLE_CRIMES#COMMUNITY_AREA_BID_5_COMMUNITY_AREA_CLOCK_AGG_LONGITUDE",
			"STAT_SAMPLE_CRIMES#COMMUNITY_AREA_BID_5_WARD_COMMUNITY_AREA_AGG_LONGITUDE",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_5_ZIP_CODES_PRIMARY_TYPE_AGG_LONCITUDE",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_5_ZIP_CODES_WARD_AGG_LONGITUDE",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_5_ZIP_CODES_X_COORDINATE_AGG_LONGITUDE",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_5_ZIP_CODES_Y_COORDINATE_AGG_LONGITUDE",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_BEAT_WARD_AGG_DISTRICT",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_BEAT_WARD_AGG_ZIP_CODES",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_DISTRICT_BEAT_AGG_ZIP_CODES",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_DISTRICT_WARD_AGG_BEAT",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_DISTRICT_WARD_AGG_ZIP_CODES",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_ZIP_CODES_BEAT_AGG_DISTRICT",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_ZIP_CODES_BEAT_AGG_WARD",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_ZIP_CODES_DISTRICT_AGG_BEAT",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_ZIP_CODES_WARD_AGC_BEAT",
			"STAT_SAMPLE_CRIMES#ZIP_CODES_BID_ZIP_CODES_WARD_AGG_DISTRICT"};
	for(int i=0; i<sizeof(ps)/sizeof(ps[0]); i++){
		dropTable(ps[i]);
	}

 /* List *partialSamples_test = NIL;
	char *ps[] = {"L_ORDERKEY","L_SUPPKEY","L_PARTKEY","L_EXTENDEDPRICE"};
		for(int i=0; i<sizeof(ps)/sizeof(ps[0]); i++){
			char* sampleName_5 = CONCAT_STRINGS("PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","5");
			char* sampleName_10 = CONCAT_STRINGS("PARTIAL_SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","10");
			partialSamples_test = appendToTailOfList(partialSamples_test,createConstString(sampleName_5));
			partialSamples_test = appendToTailOfList(partialSamples_test,createConstString(sampleName_10));
			//char* sampleName_5 = CONCAT_STRINGS("SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","5");
			//char* sampleName_10 = CONCAT_STRINGS("SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",ps[i],"_","10");
			//dropTable(sampleName_5);
			//dropTable(sampleName_10);

	}
	List *statPartialSamples = step3StatSample(partialSamples_test, groupbyAttr1_groupbyAttr2,groupbyAttr, aggrName);
		DEBUG_NODE_BEATIFY_LOG("The LZY is:", statPartialSamples);

		FOREACH(Constant, sps, statPartialSamples) {
					char *statPartSampleName = sps->value;
					storeSelectivty2(statPartSampleName, aggrName, functionname, aggrName,
						groupbyAttr, "TPCH", t, "selectivity_store_lzy", query, "5");
	}*/



	/*
	int count1 = 0;
	List *partialSamples_test = NIL;
	FOREACH(Constant, s, samples_test){
		char *tableName = s->value;
		char *partSampleName = CONCAT_STRINGS("PARTIAL_",tableName);
		partialSamples_test = appendToTailOfList(partialSamples_test,createConstString(partSampleName));
		if (catalogTableExists(partSampleName)) {
			count1 += 1;
		}
	}
	if (count1 == 8) {
		DEBUG_LOG("LZY888888888");
	}*/
	//List *partialSamples = step2PartialSample(samples_test, groupbyAttr);








	/*List *samples = NIL;
	for (int i = 0; i < sizeof(ps) / sizeof(ps[0]); i++) {
		char* sampleName_5 = CONCAT_STRINGS("SAMPLE_",
				groupbyAttr1_groupbyAttr2, "_PS_", ps[i], "_", "5");
		if (catalogTableExists(sampleName_5)) {
			//DEBUG_LOG("LZY already");
			samples = appendToTailOfList(samples,
					createConstString(sampleName_5));
		} else {
			sampleName_5 = getSamples2(groupbyAttr, groupbyAttr1_groupbyAttr2,
					ps[i], "5");
			samples = appendToTailOfList(samples,
					createConstString(sampleName_5));
		}
		char* sampleName_10 = CONCAT_STRINGS("SAMPLE_",
				groupbyAttr1_groupbyAttr2, "_PS_", ps[i], "_", "10");
		if (catalogTableExists(sampleName_10)) {
			//DEBUG_LOG("LZY already");
			samples = appendToTailOfList(samples,
					createConstString(sampleName_10));
		} else {
			sampleName_10 = getSamples2(groupbyAttr, groupbyAttr1_groupbyAttr2,
					ps[i], "10");
			samples = appendToTailOfList(samples,
					createConstString(sampleName_10));
		}
	}

	List *statPartialSamples = step3StatSample(partialSamples, groupbyAttr1_groupbyAttr2,groupbyAttr, aggrName);
	FOREACH(Constant, sps, statPartialSamples) {
			char *statPartSampleName = sps->value;
			storeSelectivty2(statPartSampleName, aggrName, functionname, aggrName,
					groupbyAttr, "TPCH", t, "selectivity_store_lzy", query, "5");

	}*/




	//DEBUG_NODE_BEATIFY_LOG("The LZY is:", samples);
	//int num = 0;
	/*List *partialSamples = NIL;
	FOREACH(Constant, s, samples){
		char *tableName = s->value;
		char *partSampleName = CONCAT_STRINGS("PARTIAL_",tableName);
		if (catalogTableExists(partSampleName)) {
			partialSamples = appendToTailOfList(partialSamples,createConstString(partSampleName));
		} else {
			//num = getCount(tableName);
			partSampleName = partialSample(tableName,groupbyAttr,2000001);
			partialSamples = appendToTailOfList(partialSamples,createConstString(partSampleName));
		}

		//int COUNT = getRowNum(part);

	}*/

	//DEBUG_NODE_BEATIFY_LOG("The LZY is:", partialSamples);

/*
	char *count_table = "";
	if (catalogTableExists(CONCAT_STRINGS("COUNT_",groupbyAttr1_groupbyAttr2))) {
		count_table = CONCAT_STRINGS("COUNT_",groupbyAttr1_groupbyAttr2);
	} else {
		count_table = storeGroupbyCount(groupbyAttr,groupbyAttr1_groupbyAttr2,"LINEITEM");
	}
	FOREACH(Constant, ps, partialSamples){
			char *partialSampleTableName = ps->value;
			DEBUG_LOG("LZY functionname is %s", partialSampleTableName);
			char *statPartSampleName = CONCAT_STRINGS("STAT_",partialSampleTableName,"_AGG_",aggrName);
			if (catalogTableExists(statPartSampleName)) {
				statPartialSamples = appendToTailOfList(statPartialSamples,createConstString(statPartSampleName));
			} else {

				statPartSampleName = getStatPartialSample(partialSampleTableName,aggrName,groupbyAttr,count_table);
				statPartialSamples = appendToTailOfList(statPartialSamples,createConstString(statPartSampleName));
			}

			//int COUNT = getRowNum(part);

		}

*/
	//EBUG_NODE_BEATIFY_LOG("The LZY is:", statPartialSamples);



	//DEBUG_LOG("LZY functionname is %s, aggrName is %s, groupbyAttr is %s,%s, constant is %s",functionname, aggrName, groupbyAttr1, groupbyAttr2,t);
}
void runSelectivity3(Node *qbModel) {
	HashMap *data = NEW_MAP(Constant, Node);
	decodeQuery(qbModel,data);
	List *aggrs = (List *) MAP_GET_STRING_ENTRY(data, "aggrs")->value;
	char *functionname = ((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
	List *args =((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args;
	char *aggrName =((AttributeReference *) getHeadOfList((List *) args)->data.ptr_value)->name;
	List *groupby = (List *) MAP_GET_STRING_ENTRY(data, "groupby")->value;
	char *groupbyAttr = "";
	char *groupbyAttr1_groupbyAttr2 = "";
	char *groupbyAttr1 = "";
	char *groupbyAttr2 = "";
	int length = groupby->length;
	int count = 0;
	FOREACH(AttributeReference, gb, groupby)
	{

		groupbyAttr = CONCAT_STRINGS(groupbyAttr, gb->name);
		groupbyAttr1_groupbyAttr2 = CONCAT_STRINGS(groupbyAttr1_groupbyAttr2, gb->name);
		if (count == 0) {
			groupbyAttr1 = CONCAT_STRINGS(groupbyAttr1, gb->name);
		}
		if (count == 1) {
			groupbyAttr2 = CONCAT_STRINGS(groupbyAttr2, gb->name);
		}
		if (count == (length -1)) {
			continue;
		}
		groupbyAttr = CONCAT_STRINGS(groupbyAttr, ",");
		groupbyAttr1_groupbyAttr2 = CONCAT_STRINGS(groupbyAttr1_groupbyAttr2, "#");
		count++;

	}
	Operator *cond = (Operator *) MAP_GET_STRING_ENTRY(data, "selection")->value;
	List *condArgs = cond->args;
	//AttributeReference *attributeReference =(AttributeReference *) getHeadOfList(args)->data.ptr_value;
	Constant *constant = (Constant *) getTailOfList(condArgs)->data.ptr_value;
	int selectionConstant = *((int *) constant->value);
	char t[100];
	sprintf(t, "%d", selectionConstant);
	char *query = CONCAT_STRINGS("SELECT * FROM (select ", functionname, "(",
				aggrName, ") AS ", functionname, "_", aggrName, ", ", groupbyAttr,
				" from CRIMES GROUP BY ", groupbyAttr, ") WHERE ", functionname,
				"_", aggrName, " >", t);
	/*char *count_table = "";
	if(catalogTableExists(CONCAT_STRINGS("COUNT_LINEITEM_",groupbyAttr1_groupbyAttr2))){

	} else {

	}*/

	DEBUG_LOG("LZY query is %s", query);
	//int count0 =  0;
	/*List *samples = NIL;
	char *ps[] = {"ZIP_CODES","BEAT","COMMUNITY_AREA","WARD","LONGITUDE","LATITUDE"};
	for (int i = 0; i < sizeof(ps) / sizeof(ps[0]); i++) {
			char* sampleName_5 = CONCAT_STRINGS("SAMPLE#",
					groupbyAttr1_groupbyAttr2, "#PS#", ps[i], "#", "5");
			if (catalogTableExists(sampleName_5)) {
				//DEBUG_LOG("LZY already");
				samples = appendToTailOfList(samples,
						createConstString(sampleName_5));
			} else {
				sampleName_5 = getSamples2(groupbyAttr, groupbyAttr1_groupbyAttr2,
						ps[i], "5");
				samples = appendToTailOfList(samples,
						createConstString(sampleName_5));
			}
			char* sampleName_10 = CONCAT_STRINGS("SAMPLE#",
					groupbyAttr1_groupbyAttr2, "#PS#", ps[i], "#", "10");
			if (catalogTableExists(sampleName_10)) {
				//DEBUG_LOG("LZY already");
				samples = appendToTailOfList(samples,
						createConstString(sampleName_10));
			} else {
				sampleName_10 = getSamples2(groupbyAttr, groupbyAttr1_groupbyAttr2,
						ps[i], "10");
				samples = appendToTailOfList(samples,
						createConstString(sampleName_10));
			}
		}
	List *partialSamples = step2PartialSample(samples, groupbyAttr);
	List *statPartialSamples = step3StatSample(partialSamples, groupbyAttr1_groupbyAttr2,
			groupbyAttr,aggrName);


	char *ps[] = {"ZIP_CODES","BEAT","COMMUNITY_AREA","WARD","LONGITUDE","LATITUDE"};
		for (int i = 0; i < sizeof(ps) / sizeof(ps[0]); i++) {
			char* sampleName_5 = CONCAT_STRINGS("SAMPLE#",
								groupbyAttr1_groupbyAttr2, "#PS#", ps[i], "#", "5");
			char* sampleName_10 = CONCAT_STRINGS("SAMPLE#",
								groupbyAttr1_groupbyAttr2, "#PS#", ps[i], "#", "10");
			dropTable(sampleName_5);
			dropTable(sampleName_10);
	}
*/
	char *count_table = CONCAT_STRINGS("COUNT_CRIMES_",groupbyAttr1_groupbyAttr2);
	if (!catalogTableExists(count_table)) {
		count_table = storeGroupbyCount(groupbyAttr, groupbyAttr1_groupbyAttr2,"CRIMES");
	}
//char *ps[] = {"ZIP_CODES","BEAT","COMMUNITY_AREA","WARD","LONGITUDE","LATITUDE"};
	char *ps[] = {"ZIP_CODES","BEAT","COMMUNITY_AREA","WARD"};
		for (int i = 0; i < sizeof(ps) / sizeof(ps[0]); i++) {

			char* sampleName_5 = getSamples2(groupbyAttr, groupbyAttr1_groupbyAttr2,
					ps[i], "5");
			int num = getCount(sampleName_5);
			DEBUG_LOG("LZY already %d", num);
			char* partSampleName = partialSample(sampleName_5, groupbyAttr, num);

			char *statPartSampleName = getStatPartialSample(partSampleName,
								aggrName, groupbyAttr, count_table);
			storeSelectivty2(statPartSampleName, aggrName, functionname, aggrName,
								groupbyAttr, "CRIMES", t, "selectivity_store_crimes_lzy", query, "5");

			char* sampleName_10 = getSamples2(groupbyAttr, groupbyAttr1_groupbyAttr2,
								ps[i], "10");
			int num2 = getCount(sampleName_10);
			char* partSampleName2 = partialSample(sampleName_10, groupbyAttr, num2);
			char *statPartSampleName2 = getStatPartialSample(partSampleName2,
											aggrName, groupbyAttr, count_table);
			storeSelectivty2(statPartSampleName2, aggrName, functionname, aggrName,
											groupbyAttr, "CRIMES", t, "selectivity_store_crimes_lzy", query, "10");

			dropTable(sampleName_5);
			dropTable(partSampleName);
			dropTable(statPartSampleName);
			dropTable(sampleName_10);
			dropTable(partSampleName2);
			dropTable(statPartSampleName2);

		}
		/*char *ps[] = {"ZIP_CODES","BEAT","COMMUNITY_AREA","WARD"};
			for (int i = 0; i < sizeof(ps) / sizeof(ps[0]); i++) {
					char*  sampleName_5 = CONCAT_STRINGS("SAMPLE#", groupbyAttr1_groupbyAttr2, "#PS#",ps[i],"#5");
					char*  sampleName_10 = CONCAT_STRINGS("SAMPLE#", groupbyAttr1_groupbyAttr2, "#PS#",ps[i],"#10");
					dropTable(sampleName_5);
					dropTable(sampleName_10);
			}*/
	//dropTable(count_table);


}
List *
step2PartialSample(List *samples, char* groupbyAttr) {
	List *res = NIL;
	FOREACH(Constant, s, samples)
	{
		char *tableName = s->value;
		char *partSampleName = CONCAT_STRINGS("PARTIAL#", tableName);
		/*if (catalogTableExists(partSampleName)) {
			res = appendToTailOfList(res,
					createConstString(partSampleName));
		} else {
			int num = getCount(tableName);
			partSampleName = partialSample(tableName, groupbyAttr, 2000001);
			res = appendToTailOfList(res,
					createConstString(partSampleName));
		}*/
		int num = getCount(tableName);
		partSampleName = partialSample(tableName, groupbyAttr, num);
		res = appendToTailOfList(res,
		createConstString(partSampleName));
	}

	return res;
}
List *
step3StatSample(List *partialSamples, char* groupbyAttr1_groupbyAttr2,
		char* groupbyAttr, char* aggrName) {
	List *res = NIL;
	char *count_table = storeGroupbyCount(groupbyAttr, groupbyAttr1_groupbyAttr2,"CRIMES");
	//char *count_table = CONCAT_STRINGS("COUNT_LINEITEM_", groupbyAttr1_groupbyAttr2);
	FOREACH(Constant, ps, partialSamples)
	{
		char *partialSampleTableName = ps->value;
		char *statPartSampleName = CONCAT_STRINGS("STAT#",
				partialSampleTableName, "#AGG#", aggrName);
		if (catalogTableExists(statPartSampleName)) {
			res = appendToTailOfList(res,
					createConstString(statPartSampleName));
		} else {

			statPartSampleName = getStatPartialSample(partialSampleTableName,
					aggrName, groupbyAttr, count_table);
			res = appendToTailOfList(res,
					createConstString(statPartSampleName));
		}

	}
	return res;
}

void
generateQuery() {
	//char *aggr[] = {"SUM", "COUNT", "AVG"};
	char *aggr[] = { "SUM"};
	//char *lineitem[][2] = { {"L_SUPPKEY","NUMBER"}, {"L_ORDERKEY","NUMBER"}, {"L_PARTKEY","NUMBER"}, {"L_SHIPDATE","VARCHAR"}, {"L_LINENUMBER","NUMBER"}, {"L_QUANTITY","NUMBER"},
	//		{"L_EXTENDEDPRICE","NUMBER"}, {"L_DISCOUNT","NUMBER"}, {"L_TAX","NUMBER"}, {"L_LINESTATUS","VARCHAR"}};
	char *lineitem[][2] = { { "L_SUPPKEY", "NUMBER" },
			{ "L_ORDERKEY", "NUMBER" }, { "L_PARTKEY", "NUMBER" }, {
					"L_LINENUMBER", "NUMBER" }, { "L_QUANTITY", "NUMBER" }, {
					"L_EXTENDEDPRICE", "NUMBER" }, { "L_DISCOUNT", "NUMBER" }, {
					"L_TAX", "NUMBER" } };
	int length_aggr = sizeof(aggr) / sizeof(aggr[0]);
	int length_lineitem = sizeof(lineitem) / sizeof(lineitem[0]);
	for (int i = 0; i < length_aggr; i++) {
		DEBUG_LOG("LZY length_aggr is %d,%d", length_aggr, length_lineitem);
		for (int j = 0; j < length_lineitem; j++) {
			if (!strcmp(lineitem[j][1], "NUMBER")) {
				if (!strcmp(lineitem[j][0], "L_DISCOUNT")|| !strcmp(lineitem[j][0], "L_TAX")) {
					continue;
				} else {
					for (int k = 0; k < (length_lineitem - 1); k++) {
						for (int m = k + 1; m < length_lineitem; m++) {
							if (!strcmp(lineitem[k][0], lineitem[j][0])|| !strcmp(lineitem[m][0],lineitem[j][0])) {
								continue;
							} else {
								if (strcmp(lineitem[k][0], "L_SUPPKEY")
										&& strcmp(lineitem[k][0], "L_ORDERKEY")
										&& strcmp(lineitem[k][0], "L_PARTKEY")
										&& strcmp(lineitem[k][0], "L_PARTKEY")
										&& strcmp(lineitem[m][0], "L_SUPPKEY")
										&& strcmp(lineitem[m][0], "L_ORDERKEY")
										&& strcmp(lineitem[m][0], "L_PARTKEY")
										&& strcmp(lineitem[m][0], "L_PARTKEY")) {
									continue;
								} else {
									char *query = CONCAT_STRINGS(
											"SELECT * FROM (select ", aggr[i],
											"(", lineitem[j][0], ") AS ",
											aggr[i], "_", lineitem[j][0], ", ",
											lineitem[k][0], ", ",
											lineitem[m][0],
											" FROM LINEITEM GROUP BY ",
											lineitem[k][0], ", ",
											lineitem[m][0], ")");
									char *aggName = CONCAT_STRINGS(aggr[i], "_",
											lineitem[j][0]);
									char *max = findTheMax(query, aggName);

									char *sel_query = printQuery(max, 0.7,
											query, aggr[i], lineitem[j][0]);
									char *path = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy2_postgres/",
											aggr[i], "_", lineitem[j][0], "_",
											lineitem[k][0], "_", lineitem[m][0],
											"_1");
									FILE *fpWrite = fopen(path, "w");
									fprintf(fpWrite, "%s ", sel_query);
									fclose(fpWrite);

									char *sel_query2 = printQuery(max, 0.8,
											query, aggr[i], lineitem[j][0]);
									char *path2 = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy2_postgres/",
											aggr[i], "_", lineitem[j][0], "_",
											lineitem[k][0], "_", lineitem[m][0],
											"_2");
									FILE *fpWrite2 = fopen(path2, "w");
									fprintf(fpWrite2, "%s ", sel_query2);
									fclose(fpWrite2);

									char *sel_query3 = printQuery(max, 0.9,
											query, aggr[i], lineitem[j][0]);
									char *path3 = CONCAT_STRINGS(
											"/Users/liuziyu/gprom/lzy2_postgres/",
											aggr[i], "_", lineitem[j][0], "_",
											lineitem[k][0], "_", lineitem[m][0],
											"_3");
									FILE *fpWrite3 = fopen(path3, "w");
									fprintf(fpWrite3, "%s ", sel_query3);
									fclose(fpWrite3);
								}
							}
						}
					}
				}
			}
		}
	}

}
/*
char*
printQuery(char *value, double percent, char *query, char *agg, char* agg_attr) {
	double max = atof(value);
	double v = max * percent;
	char t[100];
	sprintf(t, "%.0f", v);
	char *sel_query =  CONCAT_STRINGS("\"",query, "WHERE ",agg,"_",agg_attr," > ", t,";\"");
	DEBUG_LOG("lZY IS %s", sel_query);
	return sel_query;
}*/

char*
printQuery(char *value, double percent, char *query, char *agg, char* agg_attr) {
	double max = atof(value);
	double v = max * percent;
	char t[100];
	sprintf(t, "%.0f", v);
	char *sel_query =  CONCAT_STRINGS(query, "aaa WHERE ",agg,"_",agg_attr," > ", t);
	DEBUG_LOG("lZY IS %s", sel_query);
	return sel_query;
}

void computeDistinct(QueryOperator *root) {

	//testhistogrma();

/*
	char* res = getSamplesDirectly("L_SUPPKEY,L_ORDERKEY,L_PARTKEY,L_EXTENDEDPRICE","L_PARTKEY","0.05","tpch#L_PARTKEY_bid");
	char *sampleStat = getSampleStat(res, "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY", "20");
	storeSelectivty(sampleStat, "L_PARTKEY", "sum", "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY","tpch", "10000", "selectivity_store","SELECT * FROM (select sum(L_SUPPKEY) AS SUM_u_views,L_PARTKEY,L_ORDERKEY from lineitem GROUP BY L_PARTKEY,L_ORDERKEY) WHERE SUM_u_views >10000","5");
	storeSelectivty(sampleStat, "L_PARTKEY", "sum", "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY","tpch", "12500", "selectivity_store","SELECT * FROM (select sum(L_SUPPKEY) AS SUM_u_views,L_PARTKEY,L_ORDERKEY from lineitem GROUP BY L_PARTKEY,L_ORDERKEY) WHERE SUM_u_views >12500","5");
	storeSelectivty(sampleStat, "L_PARTKEY", "sum", "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY","tpch", "15000", "selectivity_store","SELECT * FROM (select sum(L_SUPPKEY) AS SUM_u_views,L_PARTKEY,L_ORDERKEY from lineitem GROUP BY L_PARTKEY,L_ORDERKEY) WHERE SUM_u_views >15000","5");

	char* res2 = getSamplesDirectly("L_SUPPKEY,L_ORDERKEY,L_PARTKEY,L_EXTENDEDPRICE","L_ORDERKEY","0.05","tpch#L_ORDERKEY_bid");
	char *sampleStat2 = getSampleStat(res2, "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY", "20");
	storeSelectivty(sampleStat2, "L_ORDERKEY", "sum", "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY","tpch", "10000", "selectivity_store","SELECT * FROM (select sum(L_SUPPKEY) AS SUM_u_views,L_PARTKEY,L_ORDERKEY from lineitem GROUP BY L_PARTKEY,L_ORDERKEY) WHERE SUM_u_views >10000","5");
	storeSelectivty(sampleStat2, "L_ORDERKEY", "sum", "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY","tpch", "12500", "selectivity_store","SELECT * FROM (select sum(L_SUPPKEY) AS SUM_u_views,L_PARTKEY,L_ORDERKEY from lineitem GROUP BY L_PARTKEY,L_ORDERKEY) WHERE SUM_u_views >12500","5");
	storeSelectivty(sampleStat2, "L_ORDERKEY", "sum", "L_SUPPKEY", "L_PARTKEY,L_ORDERKEY","tpch", "15000", "selectivity_store","SELECT * FROM (select sum(L_SUPPKEY) AS SUM_u_views,L_PARTKEY,L_ORDERKEY from lineitem GROUP BY L_PARTKEY,L_ORDERKEY) WHERE SUM_u_views >15000","5");
*/


	//char* res2 = getSamplesDirectly("L_SUPPKEY,L_ORDERKEY,L_PARTKEY,L_EXTENDEDPRICE","L_SUPPKEY","1","tpch#L_SUPPKEY_bid");
	//char *sampleStat2 = getSampleStat(res2, "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY", "1");
	//storeSelectivty(sampleStat2, "L_SUPPKEY", "sum", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "352000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >352000","100");
	//storeSelectivty(sampleStat2, "L_SUPPKEY", "sum", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "302000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >302000","100");
	//storeSelectivty(sampleStat2, "L_SUPPKEY", "sum", "L_PARTKEY", "L_SUPPKEY,L_ORDERKEY","tpch", "202000", "selectivity_store","SELECT * FROM (select sum(L_PARTKEY) AS SUM_u_views,L_SUPPKEY,L_ORDERKEY from lineitem GROUP BY L_SUPPKEY,L_ORDERKEY) WHERE SUM_u_views >202000","100");

	//computeCost((Node *)root, SafeOpeartors);
	//selectKItems(208, 20);
	//char *sampleTable = createSampleTable("CRIMES3","","");
	//DEBUG_LOG("Size is %s", sampleTable);
	//char *bucket[] = {"2733","4299","4300","4446","4449","4451","14310","14914","14920","14924","16197","21184","21186","21190","21194","21202","21538","21546","21554","21559","21560","21569","21572","21853","21861","21867","22212","22216","22248","22254","22257","22260","22268","22535","22538","22615","22616","22618","26912"};
	//int length = sizeof(bucket)/sizeof(bucket[0]);
	/*char *bucket[] = { "1", "6", "22", "202", "212", "218", "224", "232", "238",
	 "242", "250", "254", "262", "266", "270", "274", "282", "288",
	 "294", "298", "320", "324", "334", "342", "352", "358", "364",
	 "372", "382", "388", "396", "400", "404", "410", "420", "424",
	 "426", "432", "437", "442", "445", "448", "452", "458", "462",
	 "468", "472", "477", "479", "484", "486", "488", "490", "494",
	 "497", "499", "503", "505", "507", "509", "512", "517", "519",
	 "522", "525", "527", "530", "532", "536", "538", "540", "543",
	 "546", "550", "554", "558", "562", "564", "566", "568", "571",
	 "574", "576", "580", "585", "589", "593", "597", "599", "603",
	 "606", "608", "611", "613", "615", "617", "620", "624", "626",
	 "630", "632", "635", "637", "639", "642", "645", "648", "651",
	 "653", "656", "659", "661", "664", "667", "670", "673", "676",
	 "679", "682", "685", "689", "692", "695", "697", "700", "703",
	 "706", "709", "712", "715", "720", "723", "727", "731", "734",
	 "736", "739", "742", "745", "748", "752", "755", "759", "763",
	 "767", "771", "775", "779", "781", "785", "789", "793", "797",
	 "801", "806", "810", "815", "819", "822", "826", "831", "834",
	 "839", "843", "847", "851", "855", "860", "863", "869", "874",
	 "878", "883", "888", "892", "897", "901", "906", "911", "917",
	 "921", "926", "931", "935", "941", "946", "953", "958", "963",
	 "968", "973", "979", "986", "992", "999", "1005", "1010", "1017",
	 "1023", "1030", "1037", "1045", "1051", "1057", "1063", "1070",
	 "1077", "1083", "1090", "1096", "1102", "1110", "1116", "1123",
	 "1130", "1138", "1145", "1154", "1161", "1168", "1176", "1185",
	 "1194", "1201", "1207", "1215", "1226", "1234", "1241", "1250",
	 "1260", "1266", "1275", "1282", "1293", "1302", "1310", "1321",
	 "1332", "1342", "1353", "1363", "1371", "1378", "1385", "1392",
	 "1403", "1413", "1425", "1437", "1448", "1462", "1473", "1482",
	 "1494", "1507", "1519", "1533", "1546", "1555", "1568", "1580",
	 "1591", "1605", "1621", "1632", "1642", "1655", "1670", "1687",
	 "1703", "1716", "1731", "1746", "1761", "1774", "1789", "1808",
	 "1822", "1840", "1855", "1870", "1886", "1902", "1922", "1942",
	 "1959", "1981", "2001", "2020", "2041", "2060", "2081", "2099",
	 "2124", "2148", "2169", "2190", "2212", "2229", "2248", "2279",
	 "2304", "2333", "2355", "2381", "2411", "2437", "2463", "2492",
	 "2517", "2547", "2573", "2605", "2649", "2680", "2715", "2750",
	 "2777", "2810", "2842", "2884", "2931", "2968", "3004", "3031",
	 "3069", "3113", "3147", "3183", "3232", "3285", "3328", "3374",
	 "3419", "3474", "3521", "3581", "3643", "3695", "3742", "3826",
	 "3869", "3921", "3981", "4045", "4110", "4183", "4250", "4331",
	 "4413", "4505", "4574", "4684", "4751", "4840", "4955", "5085",
	 "5205", "5293", "5406", "5524", "5629", "5765", "5900", "6073",
	 "6231", "6383", "6554", "6739", "6970", "7147", "7352", "7536",
	 "7791", "8001", "8239", "8631", "8963", "9313", "9680", "10089",
	 "10482", "10913", "11446", "12079", "12794", "13411", "14242",
	 "15138", "15999", "17376", "18723", "20519", "22629", "26546",
	 "31264", "36346", "42070", "54463", "90633", "955465" };

	 int length = sizeof(bucket) / sizeof(bucket[0]);
	 storePartitionSizes ("USERS", "U_REPUTATION", bucket, length);*/
	/*char *bucket[] = { "0", "270", "297", "305", "306", "313", "314", "329",
			"333", "336", "341", "351", "354", "364", "368", "369", "370",
			"373", "376", "377", "384", "385", "386", "388", "392", "395",
			"400", "401", "402", "407", "409", "415", "416", "424", "425",
			"427", "433", "434", "434", "436", "437", "438", "439", "440",
			"443", "444", "446", "450", "451", "453", "456", "458", "459",
			"460", "462", "463", "465", "467", "468", "470", "473", "475",
			"476", "478", "479", "480", "485", "487", "488", "491", "493",
			"494", "497", "498", "501", "502", "504", "506", "507", "510",
			"511", "512", "513", "515", "518", "520", "522", "523", "525",
			"527", "528", "531", "532", "534", "535", "537", "539", "541",
			"544", "545", "549", "552", "554", "555", "557", "558", "560",
			"562", "564", "566", "568", "570", "572", "573", "576", "578",
			"581", "582", "584", "586", "588", "591", "592", "594", "597",
			"599", "601", "603", "604", "606", "609", "613", "614", "617",
			"618", "620", "624", "626", "628", "630", "632", "634", "636",
			"639", "642", "643", "647", "649", "653", "655", "657", "659",
			"661", "663", "666", "670", "673", "675", "677", "678", "681",
			"683", "685", "687", "690", "692", "694", "697", "700", "702",
			"705", "707", "710", "712", "714", "717", "721", "724", "729",
			"733", "739", "744", "746", "748", "755", "758", "761", "765",
			"768", "771", "774", "776", "782", "787", "790", "795", "797",
			"799", "803", "806", "810", "814", "818", "822", "824", "827",
			"832", "836", "839", "844", "848", "856", "861", "867", "872",
			"875", "879", "883", "888", "893", "899", "901", "906", "910",
			"912", "917", "921", "927", "933", "939", "941", "945", "951",
			"955", "961", "965", "970", "975", "984", "990", "992", "1002",
			"1008", "1013", "1019", "1024", "1029", "1037", "1042", "1045",
			"1049", "1053", "1058", "1063", "1072", "1076", "1082", "1089",
			"1092", "1101", "1108", "1113", "1120", "1124", "1128", "1131",
			"1143", "1148", "1152", "1157", "1162", "1169", "1175", "1184",
			"1190", "1195", "1203", "1209", "1220", "1228", "1237", "1242",
			"1248", "1256", "1264", "1274", "1282", "1291", "1295", "1312",
			"1321", "1324", "1333", "1342", "1346", "1362", "1370", "1377",
			"1381", "1389", "1398", "1411", "1420", "1429", "1444", "1450",
			"1459", "1468", "1481", "1490", "1494", "1501", "1508", "1515",
			"1525", "1533", "1543", "1557", "1563", "1579", "1589", "1599",
			"1613", "1628", "1642", "1651", "1667", "1683", "1695", "1709",
			"1736", "1747", "1760", "1772", "1783", "1798", "1819", "1830",
			"1843", "1863", "1882", "1892", "1912", "1930", "1939", "1963",
			"1974", "1998", "2017", "2049", "2072", "2086", "2119", "2130",
			"2163", "2190", "2214", "2242", "2247", "2276", "2295", "2323",
			"2345", "2373", "2393", "2430", "2478", "2504", "2539", "2562",
			"2604", "2616", "2658", "2733", "2814", "2857", "2954", "3020",
			"3074", "3143", "3225", "3345", "3426", "3477", "3625", "3729",
			"3851", "4034", "4151", "4348", "4452", "4682", "4870", "5048",
			"5503", "5619", "6295", "6630", "7426", "9140", "11763", "33782" };
	int length = sizeof(bucket) / sizeof(bucket[0]);
	storePartitionSizes ("USERS", "U_UPVOTES", bucket, length);*/
	//materlizeSample(root);
	//char *sampleStat = getSampleStat("SAMPLE_USERS3", "U_VIEWS",
	//		"U_REPUTATION,U_DOWNVOTES,U_UPVOTES", "100");
	//DEBUG_LOG("Size is %s", sampleStat);
	//char *sampleStat = getSampleStat("BUCKET2","WARD","ZIP_CODES,BEAT");
	//DEBUG_LOG("Size is %s", sampleStat);

	/*Set *SafeOpeartors = STRSET();
	 int a[] = {0,0};
	 int *count=a;
	 //char* t1 = get2DHist("CRIMES", "ZIP_CODES","WARD","50","20");
	 char *name = get2DHist("crimes", "BEAT","ZIP_CODES","50","20");
	 countSelection((Node *)root,count);
	 computeCosts (root,SafeOpeartors);
	 char *table = getTable(root);
	 //List *result = computeSum("CRIMES#ZIP_CODES#BEAT#WARD#DISTRICT#HIST", "ZIP_CODES", "WARD");
	 //result = computeSum("CRIMES#ZIP_CODES#WARD#HIST", "ZIP_CODES", "WARD");
	 DEBUG_LOG("lzy999 is %s,%s",table,name);
	 */

	//DEBUG_NODE_BEATIFY_LOG("The shema is:", result);
	//char* t1 = get2DHist("CRIMES", "ZIP_CODES","WARD","50","20");
	//DEBUG_LOG("t1 IS %s", t1);
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

void testhistogrma() {
	//List *result = NIL;
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
	storeInterval("CRIMES", "BEAT", "50");
	//result = get2DHist("CRIMES", "BEAT","ZIP_CODES","50","20");
	//result = get2DHist("CRIMES", "BEAT","WARD","50","20");
	//result = get2DHist("CRIMES", "BEAT","ZIP_CODES","50","20");
	//result = get2DHist("CRIMES", "BEAT","WARD","50","20");
	//result = get2DHist("CRIMES", "BEAT","DISTRICT","50","20");
	char* t1 = get2DHist("CRIMES", "ZIP_CODES", "WARD", "50", "20");
	DEBUG_LOG("t1 IS %s", t1);
	/*char* t2 = get2DHist("CRIMES", "ZIP_CODES","BEAT","50","20");
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
	//result = computeSum("CRIMES#ZIP_CODES#BEAT#WARD#DISTRICT#HIST", "ZIP_CODES", "WARD");
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

double computeSelectivity2(Operator *cond) {
	char *operator_name = cond->name;
	DEBUG_LOG("operator_name IS %s", operator_name);
	List *args = cond->args;
	char *name =
			((AttributeReference *) getHeadOfList((List *) args)->data.ptr_value)->name;
	DEBUG_LOG("NAME IS %s", name);
	HashMap *minmax = getMinAndMax("R", name);

	Constant *max = (Constant *) MAP_GET_STRING_ENTRY(minmax, "MAX")->value;
	Constant *min = (Constant *) MAP_GET_STRING_ENTRY(minmax, "MIN")->value;
	//DEBUG_NODE_BEATIFY_LOG("min is:", min);
	//DEBUG_NODE_BEATIFY_LOG("max is:", max);
	Constant *constant =
			(Constant *) getTailOfList((List *) args)->data.ptr_value;
	//DEBUG_NODE_BEATIFY_LOG("constant is:", constant);
	int a = *((int *) max->value);
	int b = *((int *) min->value);
	int c = *((int *) constant->value);
	if (!strcmp(operator_name, ">") || !strcmp(operator_name, ">=")) {

		DEBUG_LOG("a IS %d,b is %d, c is %d", a, b, c);
		double selectivity = (double) ((double) (a - c) / (double) (a - b));
		DEBUG_LOG("s is %f", selectivity);
		return selectivity;

	}
	if (!strcmp(operator_name, "<") || !strcmp(operator_name, "<=")) {

		DEBUG_LOG("a IS %d,b is %d, c is %d", a, b, c);
		double selectivity = (double) ((double) (c - b) / (double) (a - b));
		DEBUG_LOG("s is %f", selectivity);
		return selectivity;

	}
	return 0;
}

boolean computeCost(Node* node, Set *SafeOpeartors) {
	if (node == NULL)
		return TRUE;
	if (node->type == T_SelectionOperator) {
		List *args = ((Operator *) ((SelectionOperator *) node)->cond)->args;
		char *selectionAttr = ((AttributeReference *) getHeadOfList(
				(List *) args)->data.ptr_value)->name;
		List* childLsit = ((SelectionOperator *) node)->op.inputs;
		Node* child = (Node*) getHeadOfList((List *) childLsit)->data.ptr_value;
		getStringProperty((QueryOperator *) child, selectionAttr);
	}	//Check nesting
	if (node->type == T_ProjectionOperator) {
		DEBUG_LOG("Lzy2");
		List *projExprs = ((ProjectionOperator *) node)->projExprs;
		char *projectionName = ((AttributeReference *) getHeadOfList(
				(List *) projExprs)->data.ptr_value)->name;
		addToSet(SafeOpeartors, projectionName);
		List* childLsit = ((ProjectionOperator *) node)->op.inputs;
		Node* child = (Node*) getHeadOfList((List *) childLsit)->data.ptr_value;
		getStringProperty((QueryOperator *) child, "a");
	}	//Check nesting
	if (node->type == T_AggregationOperator) {
		List *aggrs = ((AggregationOperator *) node)->aggrs;
		List *groupby = ((AggregationOperator *) node)->groupBy;
		char *aggregationName =
				((AttributeReference *) getHeadOfList(
						((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args)->data.ptr_value)->name;
		Set *groupbyName = STRSET();
		FOREACH(AttributeReference, gb, groupby)
		{
			addToSet(groupbyName, gb->name);
		}
		addToSet(groupbyName, aggregationName);

		List* childLsit = ((AggregationOperator *) node)->op.inputs;
		Node* child = (Node*) getHeadOfList((List *) childLsit)->data.ptr_value;
		getStringProperty((QueryOperator *) child, "a");
	} //Check aggreationOperator

	if (node->type == T_JoinOperator) {
		JoinOperator *j = (JoinOperator *) node;
		if (j->joinType == JOIN_LEFT_OUTER || j->joinType == JOIN_RIGHT_OUTER
				|| j->joinType == JOIN_FULL_OUTER) {
			addToSet(SafeOpeartors, JOIN_OPERATOR);
		}
	} //Check outer join
	if (node->type == T_TableAccessOperator) {
		//setStringProperty(node, PROP_STORE_MIN_MAX, (Node *)MIN_MAX);
		DEBUG_LOG("Lzy1");
	} //Check outer join

	return visit(node, computeCost, SafeOpeartors);
}
char*
selectionCondition(Operator * cond) {
	List *args = cond->args;

	if (((Node *) (getHeadOfList(args)->data.ptr_value))->type
			== T_AttributeReference) {
		DEBUG_LOG("find successfully!");
		AttributeReference *attributeReference =
				(AttributeReference *) getHeadOfList(args)->data.ptr_value;
		Constant *constant = (Constant *) getTailOfList(args)->data.ptr_value;
		char text[20];
		char *num = text;
		sprintf(text, "%d", *((int * )constant->value));
		return CONCAT_STRINGS(attributeReference->name, " ", cond->name, " ",
				num);
	}
	Operator *op1 = (Operator *) getHeadOfList(args)->data.ptr_value;
	Operator *op2 = (Operator *) getTailOfList(args)->data.ptr_value;

	return CONCAT_STRINGS(selectionCondition(op1), " ", cond->name, " ",
			selectionCondition(op2));
}

void computeCosts(QueryOperator *root, Set *SafeOpeartors) {

	if (root == NULL) {
		return;
	}

	if (root->inputs != NULL) {
		FOREACH(QueryOperator, op, root->inputs)
		{
			if (!HAS_STRING_PROP(op, PROP_STORE_MIN_MAX_DONE))
				computeCosts(op, SafeOpeartors);
		}
	}
	DEBUG_LOG("BEGIN COMPUTE COSTS OF %s operator %s",
			NodeTagToString(root->type), root->schema->name);
	if (root->type == T_SelectionOperator) {
		//List *args= ((Operator *)((SelectionOperator *) root)->cond)->args;
		//char *selectionAttr = ((AttributeReference *) getHeadOfList((List *)args)->data.ptr_value)->name;
		List* childLsit = ((SelectionOperator *) root)->op.inputs;
		QueryOperator* child = (QueryOperator*) getHeadOfList(
				(List *) childLsit)->data.ptr_value;
		//DEBUG_NODE_BEATIFY_LOG("The LZY3 is:", child);
		//get2DHist()
		Constant *count = (Constant *) getStringProperty(root,
				"STORE OPERATOR COUNT");
		Constant *childTable = (Constant *) getStringProperty(child,
		PROP_STORE_APPROX);
		char* tablename = (char *) childTable->value;
		Operator *cond = (Operator *) ((SelectionOperator *) root)->cond;
		char *selection = selectionCondition(cond);
		char *selection2 = selectionCondition(cond);
		char *approxResultName = "NULL";
		DEBUG_LOG("HERE %s", selection);
		if (*((int *) count->value) == 0) {
			if (child->type == T_ProjectionOperator
					&& !strcmp(
							(char *) ((Constant *) getStringProperty(child,
									"THE PROJECTION ABOVE THE AGGREGATION"))->value,
							"TRUE")) {

				double selectivity = computeSelectivity(tablename, selection);
				DEBUG_LOG("SELECTIVITY is %.5f", selectivity);
			} else {
				char *p1 = strtok(selection2, " ");
				DEBUG_LOG("FIND TOP SELECTION %s,%s", p1, tablename);
				storeInterval(tablename, p1, "50");
				//DEBUG_LOG("here %s%s",tablename,p1);
				char *dHist = get1Dhist(tablename, p1, "50"); // get 1D histgprom
				DEBUG_LOG("lzy7 %s", dHist);
				double selectivity = computeSelectivity(dHist,
						"ZIP_CODES > 22615");
				DEBUG_LOG("SELECTIVITY is %.5f", selectivity);
			}

		} else {

			//DEBUG_LOG("Lzy2 is %s", selection);

			/*FOREACH_SET(char, attr, SafeOpeartors){
			 char *hist = get2DHist(childTable->value, attr, selectionAttr, "50","20");
			 DEBUG_LOG("Lzy3 is %s",hist);
			 }*/
			char text[20];
			char *num = text;
			sprintf(text, "%d", *((int * )count->value));
			approxResultName = selectionFiltering(tablename, num, selection);
		}

		Constant *approxResult = createConstString(approxResultName);
		setStringProperty(root, PROP_STORE_APPROX, (Node *) approxResult);
		//setStringProperty(root,PROP_STORE_APPROX,(Node *)childTable);
		//DEBUG_LOG("Lzy2 is %s,%s", childTable->value,selectionAttr);

	}
	if (root->type == T_ProjectionOperator) {
		//Constant *count = (Constant *)getStringProperty(root,"STORE OPERATOR COUNT");
		//List *projExprs = ((ProjectionOperator *) root)->projExprs;

		List *attrDefs = ((ProjectionOperator *) root)->op.schema->attrDefs;
		//char *projectionName = ((AttributeReference *) getHeadOfList((List *)projExprs)->data.ptr_value)->name;
		Set *attrs = STRSET();
		FOREACH_SET(char, attr, SafeOpeartors)
		{
			addToSet(attrs, attr);
		}
		/*FOREACH(AttributeReference,op,projExprs){
		 char *projectionName = op->name;
		 addToSet(attrs, projectionName);
		 DEBUG_LOG("Lzy5 is %s", projectionName);
		 }*/
		FOREACH(AttributeDef,op,attrDefs)
		{
			char *projectionName = op->attrName;
			addToSet(attrs, projectionName);
			DEBUG_LOG("Lzy5 is %s", projectionName);
		}
		//DEBUG_NODE_BEATIFY_LOG("lzy3 are:", attrs);

		List* childLsit = ((ProjectionOperator *) root)->op.inputs;
		QueryOperator* child = (QueryOperator*) getHeadOfList(
				(List *) childLsit)->data.ptr_value;
		Constant *childTable = (Constant *) getStringProperty(child,
		PROP_STORE_APPROX);
		Constant *count = (Constant *) getStringProperty(root,
				"STORE OPERATOR COUNT");
		DEBUG_NODE_BEATIFY_LOG("The LZY3 is:", childTable);
		if (*((int *) count->value) == 0) {
			DEBUG_LOG("FIND TOP PROJECTION");
		} else if (child->type == T_AggregationOperator) {
			setStringProperty(root, PROP_STORE_APPROX, (Node *) childTable);
			Constant *flag = createConstString("TRUE");
			setStringProperty(root, "THE PROJECTION ABOVE THE AGGREGATION",
					(Node *) flag);
			DEBUG_LOG("Lzy_projection%s is %s", (char * )childTable->value);
		} else {
			char* tablename = (char *) childTable->value;
			char text[20];
			char *num = text;
			sprintf(text, "%d", *((int * )count->value));
			DEBUG_LOG("Lzy3 is %s%d%s", tablename, *((int * )count->value),
					num);
			DEBUG_NODE_BEATIFY_LOG("The LZY3 is:", attrs);
			char *approxResultName = projectionFiltering(tablename, num, attrs);
			Constant *approxResult = createConstString(approxResultName);
			setStringProperty(root, PROP_STORE_APPROX, (Node *) approxResult);
			DEBUG_LOG("Lzy_projection%s is %s", num, approxResult->value);
			Constant *flag = createConstString("FALSE");
			setStringProperty(root, "THE PROJECTION ABOVE THE AGGREGATION",
					(Node *) flag);
		}
	}
	if (root->type == T_AggregationOperator) {
		//List * parents = ((AggregationOperator *) root)->op.parents;
		ProjectionOperator *parent =
				(ProjectionOperator *) getHeadOfList(
						(List *) ((AggregationOperator *) root)->op.parents)->data.ptr_value;
		List *attrDefs = parent->op.schema->attrDefs;
		char *aggName =
				((AttributeDef *) getHeadOfList(attrDefs)->data.ptr_value)->attrName;
		DEBUG_LOG("agg name is %s", aggName);

		List *aggrs = ((AggregationOperator *) root)->aggrs;
		List *groupby = ((AggregationOperator *) root)->groupBy;
		char *aggregationName =
				((AttributeReference *) getHeadOfList(
						((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->args)->data.ptr_value)->name;
		Set *twoDHist = STRSET();

		char *functionname =
				((FunctionCall *) getHeadOfList(aggrs)->data.ptr_value)->functionname;
		DEBUG_LOG("Lzy_agg is %s%s", functionname, aggregationName);
		//DEBUG_NODE_BEATIFY_LOG("The LZY5 is:", groupbyName);
		//addToSet(groupbyName, aggregationName);
		//zip_codes ward, zip_codes_beat

		List* childLsit = ((AggregationOperator *) root)->op.inputs;
		QueryOperator* child = (QueryOperator*) getHeadOfList(
				(List *) childLsit)->data.ptr_value;
		Constant *childTable = (Constant *) getStringProperty(child,
		PROP_STORE_APPROX);
		char *tablename = (char *) childTable->value;
		char *attName =
				((AttributeReference *) getHeadOfList((List *) groupby)->data.ptr_value)->name;
		char *firstGroupby = ((AttributeReference *) getHeadOfList(
				(List *) groupby)->data.ptr_value)->name;
		DEBUG_LOG("Lzy_agg is %s", firstGroupby);
		FOREACH(AttributeReference, gb, groupby)
		{
			if (strcmp(firstGroupby, gb->name)) {
				char* t1 = get2DHist(tablename, firstGroupby, gb->name, "50",
						"20");
				DEBUG_LOG("Lzy_agg is %s", t1);
				addToSet(twoDHist, t1);
			}
		}
		char *histName;
		char *approxResultName = "";
		DEBUG_LOG("Lzy_agg is %s", firstGroupby);
		DEBUG_NODE_BEATIFY_LOG("The Lzy_agg is:", twoDHist);

		histName = get2DHist(tablename, firstGroupby, aggregationName, "50",
				"20");
		if (setSize(twoDHist) > 0) {
			FOREACH_SET(char, name, twoDHist)
			{
				histName = join2Hist(name, histName, tablename, firstGroupby);
			}
		}
		DEBUG_LOG("Lzy_agg_hist is %s", histName);
		//char* t1 = get2DHist(tablename, "ZIP_CODES",aggregationName,"50","20")
		if (!strcmp(functionname, "SUM")) {

			approxResultName = computeSum(histName, attName, aggregationName,
					aggName); //group by , aggregation name projection name;
			DEBUG_LOG("Lzy6 is %s%s", histName, approxResultName);
		}
		Constant *approxResult = createConstString(approxResultName);
		setStringProperty(root, PROP_STORE_APPROX, (Node *) approxResult);

	} //Check aggreationOperator
	if (root->type == T_TableAccessOperator) {
		char *tableName = ((TableAccessOperator *) root)->tableName;
		tableName = checkUncertainty(root, SafeOpeartors);
		Constant *table = createConstString(tableName);
		setStringProperty(root, PROP_STORE_APPROX, (Node *) table);
		DEBUG_LOG("Lzy1");
	}

	SET_BOOL_STRING_PROP(root, "DONE");

}

char*
checkUncertainty(QueryOperator *root, Set *SafeOpeartors) {
	char *table = getTable(root);
	if (!isSkewed(table)) {
		char *compressedTable = NULL;
		FOREACH_SET(char, attr, SafeOpeartors)
		{
			compressedTable = compressTable(attr, table);
		}

		char *filteredTable = urange(compressedTable);
		return filteredTable;
	}
	return table;
}
char*
getTable(QueryOperator *root) {
	if (root == NULL) {
		return NULL;
	}
	char *res = "";
	if (root->inputs != NULL) {
		FOREACH(QueryOperator, op, root->inputs)
		{
			res = getTable(op);
		}
	}
	if (root->type == T_TableAccessOperator) {
		return ((TableAccessOperator *) root)->tableName;
	}
	return res;
}

boolean countSelection(Node* node, int* count) {
	if (node == NULL)
		return TRUE;

	if (node->type == T_SelectionOperator) {
		Constant *countSelection = createConstInt(*(count + 0));
		setStringProperty((QueryOperator*) node, "STORE OPERATOR COUNT",
				(Node *) countSelection);
		(*(count + 0))++;
	} //Check nesting

	if (node->type == T_ProjectionOperator) {
		Constant *countSelection = createConstInt(*(count + 1));
		setStringProperty((QueryOperator*) node, "STORE OPERATOR COUNT",
				(Node *) countSelection);
		(*(count + 1))++;
	} //Check nesting
	return visit(node, countSelection, count);
}

boolean isSkewed(char *tableName) {
	return TRUE;
}
char*
compressTable(char* attr, char *table) {

	return NULL;
}

char*
urange(char* table) {

	return NULL;
}

char*
materlizeSample(QueryOperator *root) {
	/*char *bucket[] = { "2733", "4299", "4300", "4446", "4449", "4451", "14310",
	 "14914", "14920", "14924", "16197", "21184", "21186", "21190",
	 "21194", "21202", "21538", "21546", "21554", "21559", "21560",
	 "21569", "21572", "21853", "21861", "21867", "22212", "22216",
	 "22248", "22254", "22257", "22260", "22268", "22535", "22538",
	 "22615", "22616", "22618", "26912" };*/

	char *bucket[] = { "1", "6", "22", "202", "212", "218", "224", "232", "238",
			"242", "250", "254", "262", "266", "270", "274", "282", "288",
			"294", "298", "320", "324", "334", "342", "352", "358", "364",
			"372", "382", "388", "396", "400", "404", "410", "420", "424",
			"426", "432", "437", "442", "445", "448", "452", "458", "462",
			"468", "472", "477", "479", "484", "486", "488", "490", "494",
			"497", "499", "503", "505", "507", "509", "512", "517", "519",
			"522", "525", "527", "530", "532", "536", "538", "540", "543",
			"546", "550", "554", "558", "562", "564", "566", "568", "571",
			"574", "576", "580", "585", "589", "593", "597", "599", "603",
			"606", "608", "611", "613", "615", "617", "620", "624", "626",
			"630", "632", "635", "637", "639", "642", "645", "648", "651",
			"653", "656", "659", "661", "664", "667", "670", "673", "676",
			"679", "682", "685", "689", "692", "695", "697", "700", "703",
			"706", "709", "712", "715", "720", "723", "727", "731", "734",
			"736", "739", "742", "745", "748", "752", "755", "759", "763",
			"767", "771", "775", "779", "781", "785", "789", "793", "797",
			"801", "806", "810", "815", "819", "822", "826", "831", "834",
			"839", "843", "847", "851", "855", "860", "863", "869", "874",
			"878", "883", "888", "892", "897", "901", "906", "911", "917",
			"921", "926", "931", "935", "941", "946", "953", "958", "963",
			"968", "973", "979", "986", "992", "999", "1005", "1010", "1017",
			"1023", "1030", "1037", "1045", "1051", "1057", "1063", "1070",
			"1077", "1083", "1090", "1096", "1102", "1110", "1116", "1123",
			"1130", "1138", "1145", "1154", "1161", "1168", "1176", "1185",
			"1194", "1201", "1207", "1215", "1226", "1234", "1241", "1250",
			"1260", "1266", "1275", "1282", "1293", "1302", "1310", "1321",
			"1332", "1342", "1353", "1363", "1371", "1378", "1385", "1392",
			"1403", "1413", "1425", "1437", "1448", "1462", "1473", "1482",
			"1494", "1507", "1519", "1533", "1546", "1555", "1568", "1580",
			"1591", "1605", "1621", "1632", "1642", "1655", "1670", "1687",
			"1703", "1716", "1731", "1746", "1761", "1774", "1789", "1808",
			"1822", "1840", "1855", "1870", "1886", "1902", "1922", "1942",
			"1959", "1981", "2001", "2020", "2041", "2060", "2081", "2099",
			"2124", "2148", "2169", "2190", "2212", "2229", "2248", "2279",
			"2304", "2333", "2355", "2381", "2411", "2437", "2463", "2492",
			"2517", "2547", "2573", "2605", "2649", "2680", "2715", "2750",
			"2777", "2810", "2842", "2884", "2931", "2968", "3004", "3031",
			"3069", "3113", "3147", "3183", "3232", "3285", "3328", "3374",
			"3419", "3474", "3521", "3581", "3643", "3695", "3742", "3826",
			"3869", "3921", "3981", "4045", "4110", "4183", "4250", "4331",
			"4413", "4505", "4574", "4684", "4751", "4840", "4955", "5085",
			"5205", "5293", "5406", "5524", "5629", "5765", "5900", "6073",
			"6231", "6383", "6554", "6739", "6970", "7147", "7352", "7536",
			"7791", "8001", "8239", "8631", "8963", "9313", "9680", "10089",
			"10482", "10913", "11446", "12079", "12794", "13411", "14242",
			"15138", "15999", "17376", "18723", "20519", "22629", "26546",
			"31264", "36346", "42070", "54463", "90633", "955465" };
	int length = sizeof(bucket) / sizeof(bucket[0]);
	//char *ps[length - 1];
	//getPartitionSizes("CRIMES", "ZIP_CODES", bucket, ps, length);

	//char *sampleTable = createSampleTable("CRIMES0", "", "");
	//char *sampleTable = createSampleTable("USERS1", "", "");
	//DEBUG_LOG("Size is %s", sampleTable);
	char *res = "";
	//char *query = "select * from CRIMES";
	//char *b1 = getBucket("2733","4299",query);
	/*for (int i = 1; i < length - 1; i++) {
	 char t[100];
	 sprintf(t, "%d", i);
	 char *size = getPartitionSizes2("USERS_BUCKET_SIZES", t);
	 DEBUG_LOG("Size is %s", size);
	 }*/

	//for (int i = 3; i < length - 1; i++) {
	for (int i = 0; i < 1; i++) {
		char t[100];
		sprintf(t, "%d", i);
		//char *size = getPartitionSizes2("CRIMES_BUCKET_SIZES", t);
		char *size = getPartitionSizes2("USERS_BUCKET_SIZES", t);
		DEBUG_LOG("Size is %s", size);
		//int partitionSize = atoi(ps[i]);
		int partitionSize = atoi(size);

		int sampleSize = partitionSize / 100;

		int s[sampleSize];
		int *sample = s;
		DEBUG_LOG("sample Size is %d", sampleSize);
		//selectKItems(partitionSize, sampleSize, sample);
		kkkk(partitionSize, sampleSize, sample);
		char *condition = "";
		for (int j = 0; j < sampleSize - 1; j++) {
			char r[100];
			sprintf(r, "%d", sample[j]);
			condition = CONCAT_STRINGS(condition, " POS=", r, " OR");
		}

		char r[100];
		sprintf(r, "%d", sample[sampleSize - 1]);
		condition = CONCAT_STRINGS(condition, " POS=", r);

		/*char *query =
		 CONCAT_STRINGS(
		 "select * from (select zip_codes, beat, ward, district ,row_number() OVER (order by zip_codes) AS POS, ",
		 t, " AS BID from (select * from CRIMES where ",
		 bucket[i], " <= zip_codes and  zip_codes < ",
		 bucket[i + 1], ")) where ", condition);*/
		char *query =
				CONCAT_STRINGS(
						"select * from (select U_REPUTATION, U_VIEWS, U_UPVOTES, U_DOWNVOTES ,row_number() OVER (order by U_REPUTATION) AS POS, ",
						t, " AS BID from (select * from USERS where ",
						bucket[i], " <= U_REPUTATION and  U_REPUTATION < ",
						bucket[i + 1], ")) where ", condition);
		DEBUG_LOG("Size is %d,%d,%d,%s", partitionSize, sampleSize, sample[0],
				query);
		res = getSamples(i, query, "SAMPLE_USERS1");

		//char p[100];
		//sprintf(p,"%d",i);
		//char *sampleName = CONCAT_STRINGS("BUCKET",t,"_2");
	}

	/*char *sampleStat = getSampleStat(res, "WARD", "ZIP_CODES,BEAT", "50");
	 DEBUG_LOG("Size is %s", res);
	 DEBUG_LOG("Size is %s", sampleStat);*/

	DEBUG_LOG("Size is %s%d", res, length);
	return NULL;
}

void selectKItems(int n, int k, int *sample) {
	printf("%d \n", INT_MAX);
	DEBUG_LOG("lzy is %d,%d ", n, k);
	int i;   // index for elements in stream[]
	//int stream[n];
	int *stream = (int *) MALLOC(n * sizeof(int));
	// int reservoir[k];
	// reservoir[] is the output array. Initialize it with
	// first k elements from stream[]
	for (int i = 0; i < n; i++) {
		stream[i] = i + 1;
	}
	for (i = 0; i < k; i++)
		sample[i] = stream[i];
	srand((unsigned) time(NULL));
	// Iterate from the (k+1)th element to nth element
	// DEBUG_LOG("i is %d", i);
	for (; i < n; i++) {
		// Pick a random index from 0 to i.
		int j = (int) (round(1.0 * rand() / RAND_MAX * i));

		// If the randomly  picked index is smaller than k,
		// then replace the element present at the index
		// with new element from stream
		if (j < k)
			sample[j] = stream[i];
	}
	//DEBUG_LOG("lzy is ");
}
void kkkk(int n, int k, int *sample) {

	srand((unsigned) time(NULL));
	// Iterate from the (k+1)th element to nth element
	// DEBUG_LOG("i is %d", i);
	for (int i = 0; i < k; i++) {
		// Pick a random index from 0 to i.
		//int j = (int) (round(1.0 * rand() / RAND_MAX * i));

		// If the randomly  picked index is smaller than k,
		// then replace the element present at the index
		// with new element from stream

		sample[i] = (int) (round(1.0 * rand() / RAND_MAX * n));
		DEBUG_LOG("lzy is %d,%d,%d", n, k, sample[i]);
	}

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
