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
static void updateCDBInsertion(QueryOperator *insertQ, char *tablename, psAttrInfo *attrInfo, boolean isForUpdate);
static void updateCDBDeletion(QueryOperator *deleteQ, char *tablename, psAttrInfo *attrInfo);
static void updateCDBUpdate(QueryOperator *updateQ, char *tablenam, psAttrInfo *attrInfo);
static void createAndInitValOfCompressedTable(char *tablename, List *ranges, char *psAttr);
static void createCompressedTable(char *tablename, List *attrDefs);
static void initValToCompressedTable(char *tablename, List *attrDefs, List *ranges, char *psAttr);
static int binarySearchToFindFragNo(Constant *value, List *ranges);
static int isInRange(Constant *val, Constant *lower, Constant *upper);
static void getCDBInsertUpdateUsingRule(StringInfo str, int dataType, Constant *insertV, int index, List *tuples, List *schema);
static void updateBaseTable(QueryOperator *stmt);
static Relation* executeQueryLocal(char *query);
static void executeStatementLocal(char *query);
static void getCDBDeleteUsingRules(StringInfo str, int dataType, List *cdbTupleList, int cdbAttrIndex, List *delTuples, int delAttrIndex, List *schema);

/*
 * Functions Implementation;
 */

/*
 *	The method check whether it needs to create and initialize the corresponding compressed table. 
 */
void
tableCompress(char *tablename, char *psAttr, List *ranges)
{

	// check existence of required compressed table;
	boolean compressedTableExist = FALSE;
	StringInfo cmprTable = makeStringInfo();
	appendStringInfo(cmprTable, "compressedtable_%s", tablename);

	compressedTableExist = (catalogTableExists(cmprTable->data)
			|| catalogViewExists(cmprTable->data));

	// if not exist, create and initial value
	if (!compressedTableExist) {
		createAndInitValOfCompressedTable(tablename, ranges, psAttr);
	}
}

static void
createAndInitValOfCompressedTable(char *tablename, List *ranges, char *psAttr)
{
	List *attrDefs = getAttributes(tablename);

	// if (getBackend() == BACKEND_POSTGRES) {
	//     attrDefs = postgresGetAttributes(tablename);
	// } else if (getBackend() == BACKEND_ORACLE) {
	//
	// }

	INFO_LOG("START CREATING AND INITIALIZING CDB\n");
	createCompressedTable(tablename, attrDefs);
	initValToCompressedTable(tablename, attrDefs, ranges, psAttr);
	INFO_LOG("FINISH CREATING AND INITIALIZING CDB\n");
}

// TODO Further research to initialize the compressed table based on how it is created.
static void
initValToCompressedTable(char *tablename, List *attrDefs, List *ranges,
		char *psAttr)
{
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

	executeStatementLocal(dmlQuery->data);
}

// TODO Further research to create the compressed table based on different mechanism.
static void
createCompressedTable(char *tablename, List *attrDefs)
{
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

	executeStatementLocal(ddlQuery);
}

/*
 * This method supports maintaining and updating the compressed table.
 */
//TODO Further research to use database low level mechanism to do updating.
void
updateCompressedTable(QueryOperator *updateQuery, char *tablename,
		psAttrInfo *attrInfo)
{
	// 1. update compressed table;
	switch (nodeTag(((DLMorDDLOperator* )updateQuery)->stmt)) {
	case T_Insert: {
		INFO_LOG("CDB: INSERT\n");
		updateCDBInsertion(updateQuery, tablename, attrInfo, FALSE);
	}
		break;
	case T_Delete: {
		INFO_LOG("CDB: DELETE\n");
		updateCDBDeletion(updateQuery, tablename, attrInfo);

	}
		break;
	case T_Update:
		INFO_LOG("CDB: UPDATE\n");
		updateCDBUpdate(updateQuery, tablename, attrInfo);
		break;
	default:
		break;
	}

	// 2. derictly execute the updateQuery to original table;
	updateBaseTable(updateQuery);
}

static void
updateBaseTable(QueryOperator *updateQuery)
{
	// update the base tables;
	// support: insert, delete and update queries;
	char *dmlQuery = serializeQuery(updateQuery);
	executeStatementLocal(dmlQuery);
}

static void
updateCDBInsertion(QueryOperator *insertQ, char *tablename,
		psAttrInfo *attrInfo, boolean isForUpdate)
{
	if (1 == 2)
		return;
	if (isForUpdate) {

	}
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
	Relation *rel = executeQueryLocal(query->data);

	for (int i = 0; i < getListLength(rel->tuples); i++) {
		StringInfo updQ = makeStringInfo();
		appendStringInfo(updQ, "update compressedtable_%s set ", tablename);

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
			//update rule based on updType;
			getCDBInsertUpdateUsingRule(updQ, type,
					getNthOfListP(csr->values, oriTblIdx), attIndex,
					getNthOfListP(rel->tuples, i), rel->schema);

			attIndex += 5;
			oriTblIdx += 1;
		}

		appendStringInfo(updQ, " where %s = %d;", "cid", rangeIndex);
		INFO_LOG("UPDATE CDB INSERT QUERY:\n%s \n", updQ->data);
		executeStatementLocal(updQ->data);
	}
}

static void
getCDBInsertUpdateUsingRule(StringInfo str, int dataType, Constant *insertV,
		int index, List *tuples, List *schema)
{
	// DataType
	// DT_INT, DT_LONG, DT_STRING, DT_FLOAT, DT_BOOL, DT_VARCHAR2
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

static void
updateCDBDeletion(QueryOperator *deleteQ, char *tablename, psAttrInfo *attrInfo)
{
	/*
	 * this method supports two cases:
	 * 1. 'delete from table;'
	 * 2. 'delete from table where conditions'
	 */
	INFO_LOG("Start To Update CDB::Deletion\n");
	Delete *delete = (Delete*) ((DLMorDDLOperator*) deleteQ)->stmt;

	// for case 1: no delete conditions, so truncate all table;
	if (!(delete->cond)) {
		INFO_LOG("\n Deletion Without Conditions\n");
		StringInfo dmlQ = makeStringInfo();
		appendStringInfo(dmlQ, "delete from compressedtable_%s;",
				delete->deleteTableName);

		char *dmlQ_origT = serializeQuery(deleteQ);
		executeStatementLocal(dmlQ->data);
		executeStatementLocal(dmlQ_origT);
		return;
	}

	// for case 2: delete query containing conditions;
	StringInfo query = makeStringInfo();
	appendStringInfo(query, "select * from %s where %s;",
			delete->deleteTableName, exprToSQL(delete->cond, NULL));

	Relation *rel = executeQueryLocal(query->data);
	INFO_LOG("LENGTH OF DELETED TUPLES: %d\n", getListLength(rel->tuples));

	int psAttrIndex = 0;
	for (int i = 0; i < getListLength(rel->schema); i++) {
		if (strcmp(attrInfo->attrName, getNthOfListP(rel->schema, i)) == 0) {
			psAttrIndex = i;
			break;
		}
	}
	INFO_LOG("psattrindex = %d\n", psAttrIndex);
	// iterate each tuple, to update the compressed table;
	int delTupleListLength = getListLength(rel->tuples);

	for (int i = 0; i < delTupleListLength; i++) {
		List *nthOfDelTupList = (List*) getNthOfListP(rel->tuples, i);
		int rangeIndex = binarySearchToFindFragNo(
				createConstInt(
						atoi(getNthOfListP(nthOfDelTupList, psAttrIndex))),
				attrInfo->rangeList) + 1;

		//get current cdb value;
		StringInfo query_cdb = makeStringInfo();
		appendStringInfo(query_cdb,
				"select * from compressedtable_%s where cid = %d",
				delete->deleteTableName, rangeIndex);
		Relation *updT = executeQueryLocal(query_cdb->data);
		StringInfo updCDBQ = makeStringInfo();

		appendStringInfo(updCDBQ, "update compressedtable_%s set ",
				delete->deleteTableName);

		// set cid;
		appendStringInfo(updCDBQ, "%s = %s", "cid", "cid");

		// set cnt;
		appendStringInfo(updCDBQ, ", %s = %s - 1 ", "cnt", "cnt");
		// for each attribute;
		int attrIndex = 2;
		int oriTblIdx = 0;
		int schemaLen = getListLength(rel->schema);
		while (oriTblIdx < schemaLen) {
			int dataType = ((AttributeDef*) getNthOfListP(delete->schema,
					oriTblIdx))->dataType;
			getCDBDeleteUsingRules(updCDBQ, dataType,
					(List*) getNthOfListP(updT->tuples, 0), attrIndex,
					nthOfDelTupList, oriTblIdx, rel->schema);
			oriTblIdx += 1;
			attrIndex += 5;
		}

		appendStringInfo(updCDBQ, " where cid = %d;", rangeIndex);
		INFO_LOG("\nwhat is the query:\n%s \n", updCDBQ->data);
		executeStatementLocal(updCDBQ->data);
	}

	// check compressed table's tuple 'cnt' field, if 0, delete this tuple;
	StringInfo dml_remove_tuples = makeStringInfo();
	appendStringInfo(dml_remove_tuples,
			"delete from compressedtable_%s where cnt = 0;", tablename);
	executeStatementLocal(dml_remove_tuples->data);
}

static void
getCDBDeleteUsingRules(StringInfo str, int dataType, List *cdbTupleList,
		int cdbAttrIndex, List *delTuples, int delAttrIndex, List *schema)
{
	// DataType
	// DT_INT, DT_LONG, DT_STRING, DT_FLOAT, DT_BOOL, DT_VARCHAR2
	int cnt = atoi(getNthOfListP(cdbTupleList, 1));
	char *attrName = getNthOfListP(schema, delAttrIndex);
	if (dataType == 0) {
		int deleteVal = atoi(getNthOfListP(delTuples, delAttrIndex));
		int sumVal = atoi(getNthOfListP(cdbTupleList, cdbAttrIndex + 4));
		int upperBound = atoi(getNthOfListP(cdbTupleList, cdbAttrIndex + 1));
		int lowerBound = atoi(getNthOfListP(cdbTupleList, cdbAttrIndex));
		int distUpperBound = atoi(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 3));
		int distlowerBound = atoi(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 2));
		if (upperBound * (cnt - 1) == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 1", attrName);
			appendStringInfo(str, ", ub_dist_%s = 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %d", attrName, attrName,
					deleteVal);
		} else if (lowerBound * (cnt - 1) == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 1", attrName);
			appendStringInfo(str, ", ub_dist_%s = 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %d", attrName, attrName,
					deleteVal);
		} else if (lowerBound + upperBound == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 2", attrName);
			appendStringInfo(str, ", ub_dist_%s = 2", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %d", attrName, attrName,
					deleteVal);
		} else if (distUpperBound > (cnt - 1)) {
			if (distlowerBound < cnt - 1) {
				appendStringInfo(str, ", lb_%s = %d", attrName, distlowerBound);
			} else {
				appendStringInfo(str, ", lb_%s = %d", attrName, cnt - 1);
			}
			appendStringInfo(str, ", ub_%s = cnt - 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %d", attrName, attrName,
					deleteVal);
		} else {
			appendStringInfo(str, ", sum_%s = sum_%s - %d", attrName, attrName,
					deleteVal);
		}
	} else if (dataType == 1) {
		char *aux;
		long deleteVal = (long) strtol(getNthOfListP(delTuples, delAttrIndex),
				&aux, 10);
		long sumVal = (long) strtol(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 4), &aux, 10);
		long upperBound = (long) strtol(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 1), &aux, 10);
		long lowerBound = (long) strtol(
				getNthOfListP(cdbTupleList, cdbAttrIndex), &aux, 10);
		int distUpperBound = atoi(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 3));
		int distlowerBound = atoi(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 2));
		//TODO think about the following two conditional cases that if long will exceed the bounds.
		if (upperBound * (cnt - 1) == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 1", attrName);
			appendStringInfo(str, ", ub_dist_%s = 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %ld", attrName, attrName,
					deleteVal);
		} else if (lowerBound * (cnt - 1) == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 1", attrName);
			appendStringInfo(str, ", ub_dist_%s = 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %ld", attrName, attrName,
					deleteVal);
		} else if (lowerBound + upperBound == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 2", attrName);
			appendStringInfo(str, ", ub_dist_%s = 2", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %ld", attrName, attrName,
					deleteVal);
		} else if (distUpperBound > (cnt - 1)) {
			if (distlowerBound < cnt - 1) {
				appendStringInfo(str, ", lb_%s = %d", attrName, distlowerBound);
			} else {
				appendStringInfo(str, ", lb_%s = %d", attrName, cnt - 1);
			}
			appendStringInfo(str, ", ub_%s = cnt - 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %ld", attrName, attrName,
					deleteVal);
		} else {
			appendStringInfo(str, ", sum_%s = sum_%s - %ld", attrName, attrName,
					deleteVal);
		}
	} else if (dataType == 2 || dataType == 5) {
		return;
	} else if (dataType == 3) {
		char *aux;
		double deleteVal = (double) strtod(
				getNthOfListP(delTuples, delAttrIndex), &aux);
		double sumVal = (double) strtod(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 4), &aux);
		double upperBound = (double) strtod(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 1), &aux);
		double lowerBound = (double) strtod(
				getNthOfListP(cdbTupleList, cdbAttrIndex), &aux);
		int distUpperBound = atoi(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 3));
		int distlowerBound = atoi(
				getNthOfListP(cdbTupleList, cdbAttrIndex + 2));
		//TODO think about the following two conditional cases that if long will exceed the bounds.
		if (upperBound * (cnt - 1) == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 1", attrName);
			appendStringInfo(str, ", ub_dist_%s = 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %lf", attrName, attrName,
					deleteVal);
		} else if (lowerBound * (cnt - 1) == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 1", attrName);
			appendStringInfo(str, ", ub_dist_%s = 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %lf", attrName, attrName,
					deleteVal);
		} else if (lowerBound + upperBound == sumVal - deleteVal) {
			appendStringInfo(str, ", lb_%s = lb_%s", attrName, attrName);
			appendStringInfo(str, ", ub_%s = ub_%s", attrName, attrName);
			appendStringInfo(str, ", lb_dist_%s = 2", attrName);
			appendStringInfo(str, ", ub_dist_%s = 2", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %lf", attrName, attrName,
					deleteVal);
		} else if (distUpperBound > (cnt - 1)) {
			if (distlowerBound < cnt - 1) {
				appendStringInfo(str, ", lb_%s = %d", attrName, distlowerBound);
			} else {
				appendStringInfo(str, ", lb_%s = %d", attrName, cnt - 1);
			}
			appendStringInfo(str, ", ub_%s = cnt - 1", attrName);
			appendStringInfo(str, ", sum_%s = sum_%s - %lf", attrName, attrName,
					deleteVal);
		} else {
			appendStringInfo(str, ", sum_%s = sum_%s - %lf", attrName, attrName,
					deleteVal);
		}

	} else if (dataType == 4) {
		return;
	}

}

static void
updateCDBUpdate(QueryOperator *updateQ, char *tablenam, psAttrInfo *attrInfo)
{

}

/*
 * This method uses binary search to find which fragment it locate for given value.
 */
static int
binarySearchToFindFragNo(Constant *value, List *ranges)
{
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

static int
isInRange(Constant *val, Constant *lower, Constant *upper)
{
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

static Relation*
executeQueryLocal(char *query)
{
	Relation *relation = NULL;
	if (getBackend() == BACKEND_POSTGRES) {
		relation = postgresExecuteQuery(query);
	} else if (getBackend() == BACKEND_ORACLE) {

	}

	return relation;
}

static void
executeStatementLocal(char *stmt)
{
	if (getBackend() == BACKEND_POSTGRES) {
		postgresExecuteStatement(stmt);
	} else if (getBackend() == BACKEND_ORACLE) {

	}
}
