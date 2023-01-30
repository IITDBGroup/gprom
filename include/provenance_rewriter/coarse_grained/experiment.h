/*
 * experiment.h
 *
 *  Created on: Nov 9, 2022
 *      Author: liuziyu
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_EXPERIMENT_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_EXPERIMENT_H_

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
#define PROP_STORE_APPROX "STORE APPROX PROPERTY"

void generateQuery_Parking();
char* printQueryParking(char *value, double percent, char *query, char *aggName);

#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_EXPERIMENT_H_ */
