/*
 * experiment.c
 *
 *  Created on: Nov 9, 2022
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
#include "provenance_rewriter/coarse_grained/experiment.h"
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

void generateQuery_Parking() {
	//char *aggr[] = {"SUM", "COUNT", "AVG"};
	//char *aggr[] = {"SUM"};
	//char *lineitem[][2] = { {"L_SUPPKEY","NUMBER"}, {"L_ORDERKEY","NUMBER"}, {"L_PARTKEY","NUMBER"}, {"L_SHIPDATE","VARCHAR"}, {"L_LINENUMBER","NUMBER"}, {"L_QUANTITY","NUMBER"},
	//		{"L_EXTENDEDPRICE","NUMBER"}, {"L_DISCOUNT","NUMBER"}, {"L_TAX","NUMBER"}, {"L_LINESTATUS","VARCHAR"}};
	char *SUM[] = { "LAW_SECTION", "VIOLATION_CODE", "ISSUER_CODE",
			"VIOLATION_PRECINCT" };
	char *parking[] = { "STREET_CODE1,STREET_CODE2", "STREET_CODE1,STREET_NAME",
			"STREET_CODE1,VEHICLE_MAKE", "STREET_CODE1,VEHICLE_COLOR",
			"STREET_CODE1,HOUSE_NUMBER", "STREET_CODE1,VIOLATION_TIME",
			"STREET_CODE1,VIOLATION_PRECINCT", "STREET_CODE1,ISSUER_COMMAND",
			"STREET_CODE2,ISSUER_COMMAND", "STREET_CODE2,VIOLATION_PRECINCT",
			"STREET_CODE2,HOUSE_NUMBER", "STREET_CODE2,STREET_NAME",
			"STREET_CODE2,VEHICLE_COLOR", "STREET_CODE2,VEHICLE_MAKE",
			"ISSUER_CODE,ISSUER_COMMAND", "ISSUER_CODE,VIOLATION_PRECINCT",
			"ISSUER_CODE,VEHICLE_COLOR", "ISSUER_CODE,VEHICLE_MAKE",
			"ISSUER_COMMAND,VIOLATION_PRECINCT",
			"ISSUER_COMMAND,VIOLATION_TIME", "ISSUER_COMMAND,HOUSE_NUMBER",
			"ISSUER_COMMAND,STREET_NAME", "ISSUER_COMMAND,VEHICLE_COLOR",
			"ISSUER_COMMAND,VEHICLE_MAKE", "VIOLATION_PRECINCT,VIOLATION_TIME",
			"VIOLATION_PRECINCT,HOUSE_NUMBER", "VIOLATION_PRECINCT,STREET_NAME",
			"VIOLATION_PRECINCT,VEHICLE_COLOR",
			"VIOLATION_PRECINCT,VEHICLE_MAKE", "VIOLATION_TIME,VEHICLE_COLOR",
			"VIOLATION_TIME,VEHICLE_MAKE", "HOUSE_NUMBER,VEHICLE_COLOR",
			"STREET_NAME,VEHICLE_COLOR", "STREET_NAME,VEHICLE_MAKE",
			"VEHICLE_COLOR,VEHICLE_MAKE" };
	char *gb[] = { "STREET_CODE1#STREET_CODE2", "STREET_CODE1#STREET_NAME",
			"STREET_CODE1#VEHICLE_MAKE", "STREET_CODE1#VEHICLE_COLOR",
			"STREET_CODE1#HOUSE_NUMBER", "STREET_CODE1#VIOLATION_TIME",
			"STREET_CODE1#VIOLATION_PRECINCT", "STREET_CODE1#ISSUER_COMMAND",
			"STREET_CODE2#ISSUER_COMMAND", "STREET_CODE2#VIOLATION_PRECINCT",
			"STREET_CODE2#HOUSE_NUMBER", "STREET_CODE2#STREET_NAME",
			"STREET_CODE2#VEHICLE_COLOR", "STREET_CODE2#VEHICLE_MAKE",
			"ISSUER_CODE#ISSUER_COMMAND", "ISSUER_CODE#VIOLATION_PRECINCT",
			"ISSUER_CODE#VEHICLE_COLOR", "ISSUER_CODE#VEHICLE_MAKE",
			"ISSUER_COMMAND#VIOLATION_PRECINCT",
			"ISSUER_COMMAND#VIOLATION_TIME", "ISSUER_COMMAND#HOUSE_NUMBER",
			"ISSUER_COMMAND#STREET_NAME", "ISSUER_COMMAND#VEHICLE_COLOR",
			"ISSUER_COMMAND#VEHICLE_MAKE", "VIOLATION_PRECINCT#VIOLATION_TIME",
			"VIOLATION_PRECINCT#HOUSE_NUMBER", "VIOLATION_PRECINCT#STREET_NAME",
			"VIOLATION_PRECINCT#VEHICLE_COLOR",
			"VIOLATION_PRECINCT#VEHICLE_MAKE", "VIOLATION_TIME#VEHICLE_COLOR",
			"VIOLATION_TIME#VEHICLE_MAKE", "HOUSE_NUMBER#VEHICLE_COLOR",
			"STREET_NAME#VEHICLE_COLOR", "STREET_NAME#VEHICLE_MAKE",
			"VEHICLE_COLOR#VEHICLE_MAKE" };

	int length_sum = sizeof(SUM) / sizeof(SUM[0]);
	int length_parking = sizeof(parking) / sizeof(parking[0]);
	for (int i = 0; i < length_sum; i++) {
		DEBUG_LOG("LZY length_aggr is %d,%d", length_sum, length_parking);

		for (int j = 0; j < length_parking; j++) {
			DEBUG_LOG("LZY IS %s,%s", parking[j], SUM[i]);
			if (!strstr(parking[j], SUM[i])) {
				char *query = CONCAT_STRINGS("SELECT * FROM (select SUM", "(",
					SUM[i], ") AS SUM_", SUM[i], ", ",parking[j], " FROM PARKING_NOTNULL GROUP BY ",parking[j], ")");
				char *aggName = CONCAT_STRINGS("SUM_", SUM[i]);

				char *max = findTheMax(query, aggName);

				 char *sel_query = printQueryParking(max, 0.7, query, aggName);
				 char *path = CONCAT_STRINGS("/Users/liuziyu/gprom/lzy_experiments/parking/SUM_",
				 SUM[i], "_", gb[j], "_1");
				 FILE *fpWrite = fopen(path, "w");
				 fprintf(fpWrite, "%s ", sel_query);
				 fclose(fpWrite);
				 char *sel_query2 = printQueryParking(max, 0.8, query, aggName);
				 char *path2 = CONCAT_STRINGS("/Users/liuziyu/gprom/lzy_experiments/parking/SUM_",
								 SUM[i], "_", gb[j], "_2");
				 FILE *fpWrite2 = fopen(path2, "w");
				 fprintf(fpWrite2, "%s ", sel_query2);
				 fclose(fpWrite2);

				 char *sel_query3 = printQueryParking(max, 0.9, query, aggName);
				 char *path3 = CONCAT_STRINGS("/Users/liuziyu/gprom/lzy_experiments/parking/SUM_",
								 SUM[i], "_", gb[j], "_3");
				 FILE *fpWrite3 = fopen(path3, "w");
				 fprintf(fpWrite3, "%s ", sel_query3);
				 fclose(fpWrite3);
			} else {
				continue;
			}

		}
	}
}

char*
printQueryParking(char *value, double percent, char *query, char *aggName) {
	double max = atof(value);
	double v = max * percent;
	char t[100];
	sprintf(t, "%.0f", v);
	char *sel_query = CONCAT_STRINGS("\"", query, " WHERE ", aggName, " > ", t,
			";\"");
	DEBUG_LOG("lZY IS %s", sel_query);
	return sel_query;
}
