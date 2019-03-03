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

extern HashMap *monotoneCheck(Node *qbModel);
HashMap *getSchema(Node *qbModel);
//HashMap *safetyCheck_aggregation(Node *qbModel);
//HashMap *safetyCheck_windowOperator(Node* qbModel);
HashMap *safetyCheck(Node* qbModel, char *hasOpeator);

boolean check(Node* node, HashMap *state);
boolean getTableAccessOperator(Node* node, HashMap *map);
boolean getSubset(Node* node, HashMap *map);
List *addBitset(int length, List* result);
char *binDis(int length, int value);
//boolean getData_aggregation(Node* node, HashMap *data);
//boolean checkPageSafety_aggregation(HashMap *data);
//boolean getData_windowOperator(Node* node, HashMap *data);
//boolean checkPageSafety_windowOperator(HashMap *data);

boolean getData(Node* node, HashMap *data);
boolean checkPageSafety(HashMap *data, char *hasOpeator);
boolean checkPageSafety_rownum(HashMap *data);

HashMap *getMonotoneResultMap(Node* qbModel);

boolean hasOrder(Node* node, int *find);
boolean checkAllIsPostive(HashMap *table_map, char *colName);
boolean checkAllIsNegative(HashMap *table_map, char *colName);
boolean isPostive(char *tableName, char *colName);
boolean isNegative(char *tableName, char *colName);
#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_ */
