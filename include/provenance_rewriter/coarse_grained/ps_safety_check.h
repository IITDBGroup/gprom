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
#define DUPLICATEREMOVAL "DuplicateRemoval"
#define SETOPINTERSECTION "SetOperator_Intersection"
#define PAGE "PAGE"
#define RANGE "RANGE"
#define BLOOM_FILTER "BLOOM_FILTER"
#define HASH "HASH"

void test(Node *qbModel);
extern HashMap *monotoneCheck(Node *qbModel);
HashMap *getSchema(Node *qbModel);

//HashMap *safetyCheck_aggregation(Node *qbModel);
//HashMap *safetyCheck_windowOperator(Node* qbModel);
HashMap *safetyCheck(Node* qbModel, Set *hasOpeator);
boolean getSafeProvenanceSketch(Node* node, HashMap *map);
Set *findFiltering(List* operatorList);
boolean findSetDifference(List* operatorList);
OrderOperator* findOrderOperator(List* operatorList);
boolean monotonicity1(SelectionOperator *selectOperator, Node *node, HashMap *table_map);
boolean monotonicity2(OrderOperator *orderOperator, Node *node, HashMap *table_map, char *groupByColName);
HashMap* hashRangeOnGroupbySafe(HashMap *map, HashMap *result, char *tableName, char *groupByColName);
HashMap* hashRangeOnGroupbyNotSafe(HashMap *map, HashMap *result, char *tableName, char *groupByColName);

char* findTable(HashMap *table_map, char *colName);
HashMap* updateResultMap(HashMap *map, HashMap *orginalResult, char *type, HashMap *newResultMapOfType);
boolean check(Node* node, HashMap *state);
boolean checkMonotone(Node* node, Set *operatorSet);
boolean getTableAccessOperator(Node* node, HashMap *map);
boolean getSubset(Node* node, HashMap *map);
boolean getAll(Node* node, HashMap *map);

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



void computeDistinct (QueryOperator *root);
double computeSelectivity(Operator *cond);
void *constantToValue(Constant *constant);
void testhistogrma();
//void joinHist(char *hist1, char *hist2);

#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_ */
