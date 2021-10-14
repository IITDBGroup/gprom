#include "common.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "parser/parser.h"

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/bitset/bitset.h"
#include "model/query_block/query_block.h"

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"

#include "analysis_and_translate/translator.h"
#include "analysis_and_translate/translate_update.h"
#include "sql_serializer/sql_serializer.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_postgres.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "instrumentation/timing_instrumentation.h"
#include "rewriter.h"
#include "provenance_rewriter/update_ps/table_compress.h"

/*
 * Local functions;
 */
static void updateCDBInsertion(QueryOperator *insertQ, char *tablename,
		psAttrInfo *attrInfo);
static void updateCDBDeletion(QueryOperator *deleteQ, char *tablename,
		psAttrInfo *attrInfo);
static void createAndInitValOfCompressedTable(char *tablename, List *ranges,
		char *psAttr);
static void createCompressedTable(char *tablename, List *attrDefs);
static void initValToCompressedTable(char *tablename, List *attrDefs,
		List *ranges, char *psAttr);
static int binarySearchToFindFragNo(Constant *value, List *ranges);
static int isInRange(Constant *val, Constant *lower, Constant *upper);
static void getCDBInsertUpdateUsingRule(StringInfo str, int dataType,
		Constant *insertV, int index, List *tuples, List *schema);
/*
 * Functions Implementation;
 */

void tableCompress(char *tablename, char *psAttr, List *ranges) {
	//	Check the existance of required compress table;
	boolean compressedTableExist = FALSE;

	StringInfo cmprTable = makeStringInfo();
	appendStringInfo(cmprTable, "compressedtable_%s", tablename);

	if (getBackend() == BACKEND_POSTGRES) {
		compressedTableExist = (postgresCatalogTableExists(cmprTable->data)
				|| postgresCatalogViewExists(cmprTable->data));
	} else if (getBackend() == BACKEND_ORACLE) {
		compressedTableExist = (oracleCatalogTableExists(cmprTable->data)
				|| oracleCatalogViewExists(cmprTable->data));
	}

	// if not exist, create and initial value
	if (!compressedTableExist) {
		createAndInitValOfCompressedTable(tablename, ranges, psAttr);
		return;
	}
}

static void createAndInitValOfCompressedTable(char *tablename, List *ranges,
		char *psAttr) {
	List *attrDefs = NIL;

	if (getBackend() == BACKEND_POSTGRES) {
		attrDefs = postgresGetAttributes(tablename);
	} else if (getBackend() == BACKEND_ORACLE) {

	}

	createCompressedTable(tablename, attrDefs);
	INFO_LOG("START TO BUILD\n");
	initValToCompressedTable(tablename, attrDefs, ranges, psAttr);

}

static void initValToCompressedTable(char *tablename, List *attrDefs,
		List *ranges, char *psAttr) {
	StringInfo dmlQuery = makeStringInfo();
	appendStringInfo(dmlQuery,
			"insert into compressedtable_%s select cid, count(*) as cnt, ",
			tablename);

	StringInfo caseWhens = makeStringInfo();
	for (int i = 0; i < LIST_LENGTH(ranges) - 1; i++) {
		if (i == LIST_LENGTH(ranges) - 2) {
			appendStringInfo(caseWhens, "else %d ", (i + 1));
		} else {
			appendStringInfo(caseWhens, " when %s >= %d and %s < %d then %d ",
					psAttr,
					*((int*) ((Constant*) getNthOfListP(ranges, i))->value),
					psAttr,
					*((int*) ((Constant*) getNthOfListP(ranges, (i + 1)))->value),
					(i + 1));
		}
	}
	appendStringInfo(caseWhens, "end as cid, ");

	for (int i = 0; i < LIST_LENGTH(attrDefs); i++) {
		AttributeDef *attribute = (AttributeDef*) getNthOfListP(attrDefs, i);
		char *attname = attribute->attrName;
		appendStringInfo(dmlQuery,
				"min(%s) as lb_%s, max(%s) as ub_%s, count(distinct %s) as lb_dist_%s, count(distinct %s) as ub_dist_%s, sum(%s) as sum_%s ",
				attname, attname, attname, attname, attname, attname, attname,
				attname, attname, attname);
		if (i != LIST_LENGTH(attrDefs) - 1) {
			appendStringInfo(dmlQuery, ",");
		}

		if (i == LIST_LENGTH(attrDefs) - 1) {
			appendStringInfo(caseWhens, "%s ", attname);
		} else {
			appendStringInfo(caseWhens, "%s, ", attname);
		}
	}

	appendStringInfo(dmlQuery, " from( select case ");
	appendStringInfo(dmlQuery, "%s from %s) a_name group by cid;",
			caseWhens->data, tablename);

	DEBUG_LOG("WHAT IS THE DML:\n%s\n", dmlQuery->data);

	if (getBackend() == BACKEND_POSTGRES) {
		postgresExecuteStatement(dmlQuery->data);
	} else if (getBackend() == BACKEND_ORACLE) {

	}

}

static void createCompressedTable(char *tablename, List *attrDefs) {
	INFO_LOG("START TO REWRITE COMPRESSED TABLE\n");

	// set the name of compressed table
	StringInfo cmprTbl = makeStringInfo();
	appendStringInfo(cmprTbl, "compressedtable_%s", tablename);

	// make attrdefs of compressed table;
	List *cmprTblAttrDefs = NIL;

	cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
			createAttributeDef("cid", (DataType) DT_INT));
	cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
			createAttributeDef("cnt", (DataType) DT_INT));

	for (int i = 0; i < LIST_LENGTH(attrDefs); i++) {
		AttributeDef *attrDef = (AttributeDef*) getNthOfListP(attrDefs, i);
		// lower bound
		StringInfo lb = makeStringInfo();
		appendStringInfo(lb, "lb_%s", attrDef->attrName);
		cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
				createAttributeDef(lb->data, attrDef->dataType));

		// upper bound
		StringInfo ub = makeStringInfo();
		appendStringInfo(ub, "ub_%s", attrDef->attrName);
		cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
				createAttributeDef(ub->data, attrDef->dataType));

		// distinct lower bound
		StringInfo dist_lb = makeStringInfo();
		appendStringInfo(dist_lb, "lb_dist_%s", attrDef->attrName);
		cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
				createAttributeDef(dist_lb->data, DT_INT));

		// distinct upper bound
		StringInfo dist_ub = makeStringInfo();
		appendStringInfo(dist_ub, "ub_dist_%s", attrDef->attrName);
		cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
				createAttributeDef(dist_ub->data, DT_INT));

		// sum val
		StringInfo sum = makeStringInfo();
		appendStringInfo(sum, "sum_%s", attrDef->attrName);
		cmprTblAttrDefs = appendToTailOfList(cmprTblAttrDefs,
				createAttributeDef(sum->data, attrDef->dataType));
	}

	CreateTable *createTable = createCreateTable(cmprTbl->data,
			cmprTblAttrDefs);
	QueryOperator *createTableOp = translateCreateTable(createTable);
	char *ddlQuery = serializeQuery(createTableOp);

	if (getBackend() == BACKEND_POSTGRES) {
		postgresExecuteStatement(ddlQuery);
	} else if (getBackend() == BACKEND_ORACLE) {

	}

}

void updateCompressedTable(QueryOperator *updateQuery, char *tablename,
		psAttrInfo *attrInfo) {
	//1. derictly execute the updateQuery to original table;
	//2. update compressed table;
	updateCDBInsertion(updateQuery, tablename, attrInfo);
	updateCDBDeletion(updateQuery, tablename, attrInfo);

}

static void updateCDBInsertion(QueryOperator *insertQ, char *tablename,
		psAttrInfo *attrInfo) {
	ConstRelOperator *csr = (ConstRelOperator*) getNthOfListP(insertQ->inputs,
			1);
	DEBUG_NODE_BEATIFY_LOG("tba:\n", csr);

	Constant *psAttrValue = NULL;
	for (int i = 0; i < getListLength((List*) csr->op.schema->attrDefs); i++) {
		AttributeDef *ad = (AttributeDef*) getNthOfListP(
				csr->op.schema->attrDefs, i);
		if (strcmp(attrInfo->attrName, ad->attrName) == 0) {
			psAttrValue = (Constant*) getNthOfListP(csr->values, i);
			break;
		}
	}

	int rangeIndex = binarySearchToFindFragNo(psAttrValue, attrInfo->rangeList)
			+ 1;
	if (rangeIndex < 1) {
		// TODO
		// this means that the insert tuple does not locate in any frag, need add a new one
	}

	StringInfo query = makeStringInfo();
	appendStringInfo(query, "select * from compressedtable_%s where cid=%d;",
			tablename, rangeIndex);
//	INFO_LOG("\n query: \n %s\n", query->data);
	Relation *rel = NULL;
	if (getBackend() == BACKEND_POSTGRES) {
		rel = postgresExecuteQuery(query->data);
	} else if (getBackend() == BACKEND_ORACLE) {

	}
	printf("tuple list length: %d\n", getListLength(rel->tuples));
	printf("schema list length: %d\n", getListLength(rel->schema));

	for (int i = 0; i < getListLength(rel->tuples); i++) {
		StringInfo updQ = makeStringInfo();
		appendStringInfo(updQ, "update compressedtable_%s set ",
				tablename);

		// set cid
		appendStringInfo(updQ, "%s = %s ", "cid", "cid");
		// set cnt
		appendStringInfo(updQ, ",cnt = cnt + 1 ");

		int attIndex = 2;
		int oriTblIdx = 0;
		int schemaLen = getListLength(rel->schema);
		while (attIndex < schemaLen) {
			int type = ((AttributeDef*) getNthOfListP(csr->op.schema->attrDefs,
					oriTblIdx))->dataType;
			getCDBInsertUpdateUsingRule(updQ, type,
					getNthOfListP(csr->values, oriTblIdx), attIndex,
					getNthOfListP(rel->tuples, i), rel->schema);

			//update rule based on updType;

			attIndex += 5;
			oriTblIdx += 1;
		}

		appendStringInfo(updQ, " where %s = %d;", "cid", rangeIndex);
		printf("\n %s\n", updQ->data);
		if (getBackend() == BACKEND_POSTGRES) {
			postgresExecuteStatement(updQ->data);
		} else if (getBackend() == BACKEND_ORACLE) {

		}
	}

}

static void getCDBInsertUpdateUsingRule(StringInfo str, int dataType,
		Constant *insertV, int index, List *tuples, List *schema) {
//	DataType
//	DT_INT, DT_LONG, DT_STRING, DT_FLOAT, DT_BOOL, DT_VARCHAR2
	if (dataType == 0) { //int
		int val = INT_VALUE(insertV);
		int currMin = (int) atoi(getNthOfListP(tuples, index));
		int currMax = (int) atoi(getNthOfListP(tuples, index + 1));

		if (val < currMin) {
			// set min
			appendStringInfo(str, ", %s = %d", getNthOfListP(schema, index),
					INT_VALUE(insertV));
			// set lb_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 2),
					getNthOfListP(schema, index + 2));
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum
			appendStringInfo(str, ", %s = %s + %d",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), INT_VALUE(insertV));
		} else if (val == currMin) {
			// set sum
			appendStringInfo(str, ", %s = %s + %d",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), INT_VALUE(insertV));
		} else if (val > currMin && val < currMax) {
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum;
			appendStringInfo(str, ", %s = %s + %d",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), INT_VALUE(insertV));
		} else if (val == currMax) {
			// set sum;
			appendStringInfo(str, ", %s = %s + %d",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), INT_VALUE(insertV));
		} else if (val > currMax) {
			// set max
			appendStringInfo(str, ", %s = %d", getNthOfListP(schema, index + 1),
					INT_VALUE(insertV));
			// set lb_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 2),
					getNthOfListP(schema, index + 2));
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum
			appendStringInfo(str, ", %s = %s + %d",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), INT_VALUE(insertV));
		}
	} else if (dataType == 1) {
		long val = LONG_VALUE(insertV);
		char *aux;
		long currMin = (long) strtol(getNthOfListP(tuples, index), &aux, 10);
		long currMax = (long) strtol(getNthOfListP(tuples, index + 1), &aux,
				10);

		if (val < currMin) {
			// set min
			appendStringInfo(str, ", %s = %ld", getNthOfListP(schema, index),
					LONG_VALUE(insertV));
			// set lb_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 2),
					getNthOfListP(schema, index + 2));
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum
			appendStringInfo(str, ", %s = %s + %ld",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), LONG_VALUE(insertV));
		} else if (val == currMin) {
			// set sum
			appendStringInfo(str, ", %s = %s + %ld",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), LONG_VALUE(insertV));
		} else if (val > currMin && val < currMax) {
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum;
			appendStringInfo(str, ", %s = %s + %ld",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), LONG_VALUE(insertV));
		} else if (val == currMax) {
			// set sum;
			appendStringInfo(str, ", %s = %s + %ld",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), LONG_VALUE(insertV));
		} else if (val > currMax) {
			// set max
			appendStringInfo(str, ", %s = %ld",
					getNthOfListP(schema, index + 1), LONG_VALUE(insertV));
			// set lb_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 2),
					getNthOfListP(schema, index + 2));
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum
			appendStringInfo(str, ", %s = %s + %ld",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), LONG_VALUE(insertV));
		}
	} else if (dataType == 2 || dataType == 5) {
		return;
	} else if (dataType == 3) {
		double val = FLOAT_VALUE(insertV);
		char *aux;
		double currMin = (double) strtod(getNthOfListP(tuples, index), &aux);
		double currMax = (double) strtod(getNthOfListP(tuples, index + 1),
				&aux);
		if (val < currMin) {
			// set min
			appendStringInfo(str, ", %s = %lf", getNthOfListP(schema, index),
					FLOAT_VALUE(insertV));
			// set lb_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 2),
					getNthOfListP(schema, index + 2));
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum
			appendStringInfo(str, ", %s = %s + %lf",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), FLOAT_VALUE(insertV));
		} else if (val == currMin) {
			// set sum
			appendStringInfo(str, ", %s = %s + %lf",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), FLOAT_VALUE(insertV));
		} else if (val > currMin && val < currMax) {
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum;
			appendStringInfo(str, ", %s = %s + %lf",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), FLOAT_VALUE(insertV));
		} else if (val == currMax) {
			// set sum;
			appendStringInfo(str, ", %s = %s + %lf",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), FLOAT_VALUE(insertV));
		} else if (val > currMax) {
			// set max
			appendStringInfo(str, ", %s = %lf",
					getNthOfListP(schema, index + 1), FLOAT_VALUE(insertV));
			// set lb_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 2),
					getNthOfListP(schema, index + 2));
			// set ub_dist_xxx
			appendStringInfo(str, ", %s = %s + 1",
					getNthOfListP(schema, index + 3),
					getNthOfListP(schema, index + 3));
			// set sum
			appendStringInfo(str, ", %s = %s + %lf",
					getNthOfListP(schema, index + 4),
					getNthOfListP(schema, index + 4), FLOAT_VALUE(insertV));
		}
	} else if (dataType == 4) {
		return;
	}

}

static void updateCDBDeletion(QueryOperator *deleteQ, char *tablename,
		psAttrInfo *attrInfo) {
	return;
}

static int binarySearchToFindFragNo(Constant *value, List *ranges) {
	int index = -1;
	int start = 0;
	int end = getListLength(ranges) - 2;

	while (start + 1 < end) {
		int mid = start + (end - start) / 2;
		Constant *lower = (Constant*) getNthOfListP(ranges, mid);
		Constant *upper = (Constant*) getNthOfListP(ranges, mid + 1);
		int compare = isInRange(value, lower, upper);
		if (compare == 0) {
			index = mid;
			break;
		} else if (compare < 0) {
			end = mid;
		} else if (compare > 0) {
			start = mid;
		}
	}
	if (isInRange(value, (Constant*) getNthOfListP(ranges, start),
			(Constant*) getNthOfListP(ranges, start + 1)) == 0) {
		index = start;
	} else if (isInRange(value, (Constant*) getNthOfListP(ranges, end),
			(Constant*) getNthOfListP(ranges, end + 1)) == 0) {
		index = end;
	}
	return index;
}

static int isInRange(Constant *val, Constant *lower, Constant *upper) {
	int res = 0;
	switch (val->constType) {
	case DT_INT: {
		if (INT_VALUE(val) < INT_VALUE(lower)) {
			res = -1;
		} else if (INT_VALUE(val) >= INT_VALUE(upper)) {
			res = 1;
		} else {
			res = 0;
		}
	}
		break;
	case DT_LONG: {
		if (LONG_VALUE(val) < LONG_VALUE(lower)) {
			res = -1;
		} else if (LONG_VALUE(val) >= LONG_VALUE(upper)) {
			res = 1;
		} else {
			res = 0;
		}
	}

		break;
	case DT_STRING:
	case DT_VARCHAR2: {
		if (strcmp(STRING_VALUE(val), STRING_VALUE(lower)) == -1) {
			res = -1;
		} else if (strcmp(STRING_VALUE(val), STRING_VALUE(upper)) > -1) {
			res = 1;
		} else {
			res = 0;
		}
	}

		break;
	case DT_FLOAT: {
		if (FLOAT_VALUE(val) < FLOAT_VALUE(lower)) {
			res = -1;
		} else if (FLOAT_VALUE(val) >= FLOAT_VALUE(upper)) {
			res = 1;
		} else {
			res = 0;
		}
	}
		break;
	default:
		break;
	}
	return res;
}
/*


 create cdb code backup



 StringInfo ddlQuery = makeStringInfo();
 appendStringInfo(ddlQuery,
 "create table compressedtable_%s(cid int, ccnt int, ",
 oriTablename);

 List *attributes = NIL;

 INFO_LOG("SUCCESS\n");
 if (getBackend() == BACKEND_POSTGRES) {
 //	Get all attributes of table 'tablename'
 attributes = postgresGetAttributes(oriTablename);
 } else if (getBackend() == BACKEND_ORACLE) {

 }
 INFO_LOG("SUCCESS2\n");
 // here should go to tableCompressRewrite();
 if (1 == 2) {
 tableCompressRewrite(cprTablename, attributes, ranges, psAttr);
 }

 //		if (1 == 1) {
 //			tableCompressRewrite(tablename, attributes, ranges, psAttr);
 //			return;
 //		}


 //	 Append to the 'ddlQuery' and then create this compressed table "compressedtable_'tablename' "

 for (int i = 0; i < LIST_LENGTH(attributes); i++) {
 AttributeDef *attribute = (AttributeDef*) getNthOfListP(attributes, i);

 char *type = NULL;
 switch (attribute->dataType) {
 case DT_INT:
 type = "int";
 break;
 case DT_LONG:
 type = "long";
 break;
 case DT_STRING:
 type = "varchar2";
 break;
 case DT_FLOAT:
 type = "float";
 break;
 case DT_VARCHAR2:
 type = "varchar2";
 break;
 default:
 break;
 }

 char *attname = attribute->attrName;

 appendStringInfo(ddlQuery,
 "lb_%s int, ub_%s int, lb_dist_%s int, ub_dist_%s int, sum_%s %s ",
 attname, attname, attname, attname, attname, type);
 if (i != LIST_LENGTH(attributes) - 1) {
 appendStringInfo(ddlQuery, ", ");
 }

 }
 appendStringInfo(ddlQuery, ");");
 DEBUG_LOG("DDL STATEMENT:\n%s\n", ddlQuery->data);
 //		executeQuery("select * from r");
 //		postgresExecuteQuery(ddlQuery->data);

 if(getBackend() == BACKEND_POSTGRES) {
 postgresExecuteStatement(ddlQuery->data);
 } else if(getBackend() == BACKEND_ORACLE) {

 }
 */

/*
 * insert into cdb code backup
 * */
// * /*
//	 *  build dml query and fill data into compressedtable
//	 */
//	/*
//	 StringInfo dmlQuery = makeStringInfo();
//	 appendStringInfo(dmlQuery,
//	 "insert into compressedtable_%s select cid, count(*) as ccnt, ",
//	 tablename);
//
//	 StringInfo caseWhens = makeStringInfo();
//	 for (int i = 0; i < LIST_LENGTH(ranges) - 1; i++) {
//	 if (i == LIST_LENGTH(ranges) - 2) {
//	 appendStringInfo(caseWhens, "else %d ", (i + 1));
//	 } else {
//	 appendStringInfo(caseWhens,
//	 " when %s >= %d and %s < %d then %d ", psAttr,
//	 *((int*) ((Constant*) getNthOfListP(ranges, i))->value),
//	 psAttr,
//	 *((int*) ((Constant*) getNthOfListP(ranges, (i + 1)))->value),
//	 (i + 1));
//	 }
//	 }
//	 appendStringInfo(caseWhens, "end as cid, ");
//
//	 for (int i = 0; i < LIST_LENGTH(attributes); i++) {
//	 AttributeDef *attribute = (AttributeDef*) getNthOfListP(attributes,
//	 i);
//	 char *attname = attribute->attrName;
//	 appendStringInfo(dmlQuery,
//	 "min(%s) as lb_%s, max(%s) as ub_%s, count(distinct %s) as lb_dist_%s, count(distinct %s) as ub_dist_%s, sum(%s) as sum_%s ",
//	 attname, attname, attname, attname, attname, attname,
//	 attname, attname, attname, attname);
//	 if (i != LIST_LENGTH(attributes) - 1) {
//	 appendStringInfo(dmlQuery, ",");
//	 }
//
//	 if (i == LIST_LENGTH(attributes) - 1) {
//	 appendStringInfo(caseWhens, "%s ", attname);
//	 } else {
//	 appendStringInfo(caseWhens, "%s, ", attname);
//	 }
//	 }
//
//	 appendStringInfo(dmlQuery, " from( select case ");
//	 appendStringInfo(dmlQuery, "%s from %s) a_name group by cid;",
//	 caseWhens->data, tablename);
//
//	 DEBUG_LOG("WHAT IS THE DML:\n%s\n", dmlQuery->data);
//	 */
//	/*
//	 * ddlQuery : create compressed table
//	 * dmlQuery : fill compressed table with data;
//	 */
////	postgresExecuteStatement(ddlQuery->data);
////	postgresExecuteStatement(dmlQuery->data);
// CREATE INSERT OPERATOR;
//	char* q = rewriteParserOutput(node);
//	processInput(dmlQuery->data);
//	printf("\n what is q \n %s\n", q);
/*
 INFO_LOG("START TO INITIALIZE THE COMPRESSED TABLE\n");
 // STEP 1.1 build the query block;
 QueryBlock *innerQB = createQueryBlock();

 //
 // STEP 1.2 build select clause
 List *innerQBSelectClauseList = NIL;

 //
 // STEP 1.2.1 build first select item "case when"
 CaseExpr *caseExpr = createCaseExpr(NULL, NIL, NULL);
 List *caseExprWhenClauseList = NIL;

 // STEP 1.2.1.1 find psAttr DT and attrPos;
 int attPos = 0;
 int dataType = 0;
 for (int i = 0; i < getListLength(attrDefs); i++) {
 AttributeDef *ad = (AttributeDef*) getNthOfListP(attrDefs, i);
 if (0 == strcmp(ad->attrName, psAttr)) {
 attPos = i;
 dataType = ad->dataType;
 }

 //ref_1
 innerQBSelectClauseList = appendToTailOfList(innerQBSelectClauseList,
 createSelectItem(ad->attrName,
 (Node*) createFullAttrReference(ad->attrName, 0, i, 0,
 ad->dataType)));
 }

 //
 // STEP 1.2.1.2 build attribute reference for psAttr;
 AttributeReference *psAttrRef = createFullAttrReference(psAttr, 0, attPos,
 0, dataType);
 for (int i = 0; i < getListLength(ranges) - 2; i++) {
 // build psAttr >= x and psAttr < y
 Constant *left = getNthOfListP(ranges, i);
 Constant *right = getNthOfListP(ranges, i + 1);
 Operator *leftOp = createOpExpr(">=", LIST_MAKE(psAttrRef, left));
 Operator *rightOp = createOpExpr("<", LIST_MAKE(psAttrRef, right));
 List *exprs = LIST_MAKE(leftOp, rightOp);

 // and expr list;
 Node *when = andExprList(exprs);
 Node *then = (Node*) (createConstInt(i + 1));
 CaseWhen *caseWhen = createCaseWhen(when, then);

 caseExprWhenClauseList = appendToTailOfList(caseExprWhenClauseList,
 caseWhen);
 }

 //
 // STEP 1.2.1.3 set caseExpr->when: case when list caseExpr;
 caseExpr->whenClauses = caseExprWhenClauseList;
 caseExpr->elseRes = (Node*) createConstInt(getListLength(ranges) - 1);

 //
 // STEP 1.2.1.4 build the first SELECT_ITEM
 // 				append to first, since the other items are already in the list;
 innerQBSelectClauseList = appendToHeadOfList(innerQBSelectClauseList,
 createSelectItem("cid", (Node*) caseExpr));

 //
 // STEP 1.2.2 loop through "attDefs", add all of the to select clause list;
 // 			 This step is finished in 1.2.1.1: in ref_1

 //
 // STEP 1.3 build "fromClause" for innerQB;
 // 			only one table for fromClause list;
 FromTableRef *fromTableRef = (FromTableRef*) createFromTableRef(tablename,
 getAttrDefNames(attrDefs), tablename, getAttrDataTypes(attrDefs));
 //	fromTableRef->from = *((FromItem*) fromTableRef);
 fromTableRef->tableId = tablename;
 innerQB->fromClause = LIST_MAKE(fromTableRef);
 innerQB->selectClause = innerQBSelectClauseList;
 DEBUG_NODE_BEATIFY_LOG("INNER BOLCK:\n", innerQB);

 //
 // STEP 2, build outer query block
 QueryBlock *outerQB = createQueryBlock();

 //
 // STEP 2.1 build outerQB's from Clause
 //			this fromClause is built from innerQB
 List *outerQBFromClauseList = NIL;

 //
 // STEP 2.1.2 build "FromSubquery"
 //			two components: FromItem, Node* subquery
 FromSubquery *fromSubquery = (FromSubquery*) createFromSubquery("a_name",
 getQBAttrNames((Node*) innerQB), (Node*) innerQB);
 fromSubquery->from.dataTypes = getQBAttrDTs((Node*) innerQB);

 // STEP 2.1.2.1 build FromItem for fromSubquery
 //	FromItem * fromItem = createFromItem("a_name", getQBAttrNames((Node*) innerQB));
 //	DEBUG_NODE_BEATIFY_LOG("fromitem:\n", (FromItem*) fromItem);

 //
 // STEP 2.1.2.3 build subquery for fromSubquery
 //				it is QueryBlock innerQB;

 // STEP 2.1.2.4 build fromSubquery
 //	fromSubquery->from = *fromItem;
 //	fromSubquery->subquery = (Node*) innerQB;
 //	DEBUG_NODE_BEATIFY_LOG("WHAT IS THE FROMSUBQUERY:\n", fromSubquery);

 //
 // STEP 2.1.2.5: make fromClause list;
 outerQBFromClauseList = appendToTailOfList(outerQBFromClauseList,
 fromSubquery);
 outerQB->fromClause = outerQBFromClauseList;
 //	outerQB->fromClause = LIST_MAKE(fromSubquery);

 //
 // STEP 2.2 build outerQB's selectClause;
 List *outerQBSelectClauseList = NIL;

 //
 // STEP 2.2.1 build first 2 items cid, and cnt;
 AttributeReference *cidRef = createFullAttrReference("cid", 0, 0, 0,
 (DataType) DT_INT);
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem("cid", (Node*) cidRef));

 FunctionCall *fc = createFunctionCall("count",
 LIST_MAKE(createConstInt(1)));
 fc->isAgg = TRUE;
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem("cnt", (Node*) fc));

 List *cmprTblAttrDefs = NIL;
 if (getBackend() == BACKEND_POSTGRES) {
 cmprTblAttrDefs = postgresGetAttributes(tablename);
 } else if (getBackend() == BACKEND_ORACLE) {

 }

 //
 // STEP 2.2.2 LOOP through to build other SelectItem s
 int nameIndex = 2; // need to use attrname from cmprTblAttrDefs

 for (int i = 0; i < getListLength(attrDefs); i++) {
 AttributeDef *ad = (AttributeDef*) getNthOfListP(attrDefs, i);
 AttributeReference *ar = createFullAttrReference(ad->attrName, 0,
 (i + 1), 0, ad->dataType);
 List *args = LIST_MAKE(ar);

 // for lb_xx
 fc = createFunctionCall("min", args);
 fc->isAgg = TRUE;
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem(
 ((AttributeDef*) getNthOfListP(cmprTblAttrDefs,
 nameIndex++))->attrName, (Node*) fc));

 //for ub_xx
 fc = createFunctionCall("max", args);
 fc->isAgg = TRUE;
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem(
 ((AttributeDef*) getNthOfListP(cmprTblAttrDefs,
 nameIndex++))->attrName, (Node*) fc));

 //for dist_xxx_lb
 fc = createFunctionCall("count", args);
 fc->isAgg = TRUE;
 fc->isDistinct = TRUE;
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem(
 ((AttributeDef*) getNthOfListP(cmprTblAttrDefs,
 nameIndex++))->attrName, (Node*) fc));

 //for dist_xxx_ub
 fc = createFunctionCall("count", args);
 fc->isAgg = TRUE;
 fc->isDistinct = TRUE;
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem(
 ((AttributeDef*) getNthOfListP(cmprTblAttrDefs,
 nameIndex++))->attrName, (Node*) fc));

 fc = createFunctionCall("sum", args);
 fc->isAgg = TRUE;
 fc->isDistinct = FALSE;
 outerQBSelectClauseList = appendToTailOfList(outerQBSelectClauseList,
 createSelectItem(
 ((AttributeDef*) getNthOfListP(cmprTblAttrDefs,
 nameIndex++))->attrName, (Node*) fc));
 }

 //
 // STEP 2.2.3 set outerQB selectClause;
 outerQB->selectClause = outerQBSelectClauseList;
 outerQB->groupByClause = LIST_MAKE(cidRef);

 DEBUG_NODE_BEATIFY_LOG("OUTTER BLOCK:\n", outerQB);

 // STEP 3, build "Insert"
 StringInfo cmprTbl = makeStringInfo();
 appendStringInfo(cmprTbl, "compressedtable_%s", tablename);
 Insert *insert = createInsert(cmprTbl->data, (Node*) outerQB, NIL);
 insert->insertTableName = cmprTbl->data;
 //	insert->schema = LIST_MAKE(createSchema(NULL, cmprTblAttrDefs));
 insert->attrList = NIL;
 List *schemas = NIL;
 for (int i = 0; i < getListLength(cmprTblAttrDefs); i++) {
 AttributeDef *ad = getNthOfListP(cmprTblAttrDefs, i);
 schemas = appendToTailOfList(schemas, ad);
 insert->attrList = appendToTailOfList(insert->attrList,
 ((AttributeDef*) getNthOfListP(cmprTblAttrDefs, i))->attrName);
 }
 insert->schema = schemas;
 INFO_NODE_BEATIFY_LOG("INSERT QUERY:\n", insert);

 //		QueryOperator *insertOp = translateUpdate((Node*) insert);
 //		char *dmlQuery = serializeQuery(insertOp);
 //		printf("insert query: %s\n", dmlQuery);
 if (getBackend() == BACKEND_POSTGRES) {
 //		postgresExecuteStatement(dmlQuery);
 } else if (getBackend() == BACKEND_ORACLE) {

 }
 */
