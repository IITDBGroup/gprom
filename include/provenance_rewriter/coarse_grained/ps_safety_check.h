/*
 * ps_safety_check.h
 *
 *  Created on: 2018年10月25日
 *      Author: liuziyu
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_

#include "model/list/list.h"
#include "model/set/hashmap.h"

#define AGGREGATION_OPERATOR "AggregationOperator"
#define WINDOW_OPERATOR "WindowOperator"
#define SET_OPERATOR "SetOperator"
#define JOIN_OPERATOR "JoinOperator"
#define NESTING_OPERATOR "NestingOperator"
#define ORDER_OPERATOR "OrderOperator"
#define SELECTION_OPERATOR "SelectionOperator"
#define TABLEACCESS_OPERATOR "TableAccessOperator"

void test(Node *qbModel);
extern HashMap *monotoneCheck(Node *qbModel);
HashMap *getSchema(Node *qbModel);
//HashMap *safetyCheck_aggregation(Node *qbModel);
//HashMap *safetyCheck_windowOperator(Node* qbModel);
HashMap *safetyCheck(Node* qbModel, Set *hasOpeator);

boolean check(Node* node, HashMap *state);
boolean checkMonotone(Node* node, Set *operatorSet);
boolean getTableAccessOperator(Node* node, HashMap *map);
boolean getSubset(Node* node, HashMap *map);
List *addBitset(unsigned int length, List* result);
//char *binDis(int length, int value);
//boolean getData_aggregation(Node* node, HashMap *data);
//boolean checkPageSafety_aggregation(HashMap *data);
//boolean getData_windowOperator(Node* node, HashMap *data);
//boolean checkPageSafety_windowOperator(HashMap *data);

boolean getData(Node* node, HashMap *data);
boolean checkPageSafety(HashMap *data, Set *hasOpeator);
boolean checkPageSafety_rownum(HashMap *data);

HashMap *getMonotoneResultMap(Node* qbModel);

boolean hasOperator(Node* node, Set *operatorSet);
boolean checkAllIsPostive(HashMap *table_map, char *colName);
boolean checkAllIsNegative(HashMap *table_map, char *colName);
boolean isPostive(char *tableName, char *colName);
boolean isNegative(char *tableName, char *colName);
#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_ */
