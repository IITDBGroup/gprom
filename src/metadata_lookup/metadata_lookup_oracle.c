/*
 * metadata_lookup.c
 *
 *      Author: zephyr
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/relation/relation.h"
#include "parser/parser.h"
#include "instrumentation/timing_instrumentation.h"
#include "utility/string_utils.h"
#include <stdlib.h>
#include "model/query_operator/query_operator.h"

/* If OCILIB and OCI are available then use it */
#if HAVE_ORACLE_BACKEND

#define ORACLE_TNS_CONNECTION_FORMAT "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=" \
		"(PROTOCOL=TCP)(HOST=%s)(PORT=%u)))(CONNECT_DATA=" \
		"(SERVER=DEDICATED)(SID=%s)))"

#define ORACLE_TNS_CONNECTION_FORMAT_SERVICE "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=" \
        "(PROTOCOL=TCP)(HOST=%s)(PORT=%u)))(CONNECT_DATA=" \
        "(SERVER=DEDICATED)(SERVICE_NAME=%s)))"

/* queries */
#define ORACLE_SQL_GET_AUDIT_FOR_TRANSACTION \
"WITH transSlice AS (\n" \
"    SELECT SCN, SQL_TEXT, SQL_BINDS, ENTRY_ID, STATEMENT_ID, AUDIT_TYPE\n" \
"    FROM %s t1\n" \
"    WHERE TRANSACTION_ID = HEXTORAW('%s'))\n" \
"                                \n" \
"SELECT SCN, \n" \
"      CASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) > 4000 THEN SQL_TEXT ELSE NULL END AS lsql,\n" \
"      CASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_TEXT,4000)  ELSE NULL END AS shortsql,\n" \
"      CASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) > 4000 THEN SQL_BINDS ELSE NULL END AS lbind,\n" \
"      CASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_BINDS,4000)  ELSE NULL END AS shortbind\n" \
"FROM \n" \
"  (SELECT SCN, SQL_TEXT, SQL_BINDS, ENTRY_ID\n" \
"  FROM transSlice t1\n" \
"  WHERE AUDIT_TYPE = 'FineGrainedAudit' \n" \
"        OR NOT EXISTS (SELECT 1\n" \
"                       FROM transSlice t2 \n" \
"                       WHERE t1.STATEMENT_ID = t2.STATEMENT_ID AND AUDIT_TYPE = 'FineGrainedAudit')\n" \
"                       ) x \n" \
"ORDER BY ENTRY_ID\n"

/*
 * functions and variables for internal use
 */

typedef struct TableBuffer {
	char *tableName;
	List *attrs;
} TableBuffer;

typedef struct ViewBuffer {
	char *viewName;
	char *viewDefinition;
} ViewBuffer;

static OCI_Connection *conn = NULL;
static OCI_Statement *st = NULL;
static OCI_TypeInfo *tInfo = NULL;
static OCI_Error *errorCache = NULL;
static MemContext *context = NULL;
static char **aggList = NULL;
static char **winfList = NULL;
static List *tableBuffers = NULL;
static List *viewBuffers = NULL;
static HashMap *keys = NULL;
static Set *haveKeys = NULL;
static boolean initialized = FALSE;

static int initConnection(void);
static boolean isConnected(void);
static void initAggList(void);
static void freeAggList(void);
static void initWinfList(void);
static void freeWinfList(void);
static char *oracleGetConnectionDescription(void);
static OCI_Transaction *createTransaction(IsolationLevel isoLevel);
static OCI_Resultset *executeStatement(char *statement);
static boolean executeNonQueryStatement(char *statement);
static void handleError(OCI_Error *error);
//static inline char *LobToChar (OCI_Lob *lob);
static DataType ociTypeToDT(unsigned int typ);

static void addToTableBuffers(char *tableName, List *attrs);
static void addToViewBuffers(char *viewName, char *viewDef);
static List *searchTableBuffers(char *tableName);
static char *searchViewBuffers(char *viewName);
static void freeBuffers(void);

static char *getHistoryTableName(char *tableName, char *ownerName);

/* assemble plugin and return */
MetadataLookupPlugin *
assembleOracleMetadataLookupPlugin(void) {
	MetadataLookupPlugin *plugin = NEW(MetadataLookupPlugin);

	plugin->type = METADATA_LOOKUP_PLUGIN_ORACLE;

	plugin->initMetadataLookupPlugin = oracleInitMetadataLookupPlugin;
	plugin->databaseConnectionOpen = oracleDatabaseConnectionOpen;
	plugin->databaseConnectionClose = oracleDatabaseConnectionClose;
	plugin->shutdownMetadataLookupPlugin = oracleShutdownMetadataLookupPlugin;
	plugin->connectionDescription = oracleGetConnectionDescription;
	plugin->isInitialized = oracleIsInitialized;
	plugin->catalogTableExists = oracleCatalogTableExists;
	plugin->catalogViewExists = oracleCatalogViewExists;
	plugin->getAttributes = oracleGetAttributes;
	plugin->getAttributeNames = oracleGetAttributeNames;
	plugin->getAttributeDefaultVal = oracleGetAttributeDefaultVal;
	plugin->isAgg = oracleIsAgg;
	plugin->isWindowFunction = oracleIsWindowFunction;
	plugin->getTableDefinition = oracleGetTableDefinition;
	plugin->getViewDefinition = oracleGetViewDefinition;
	plugin->getOpReturnType = oracleGetOpReturnType;
	plugin->getFuncReturnType = oracleGetFuncReturnType;
	plugin->getTransactionSQLAndSCNs = oracleGetTransactionSQLAndSCNs;
	plugin->executeAsTransactionAndGetXID = oracleExecuteAsTransactionAndGetXID;
	plugin->getCommitScn = oracleGetCommitScn;
	plugin->executeQuery = oracleGenExecQuery;
	plugin->executeQueryIgnoreResult = oracleGenExecQueryIgnoreResult;
	plugin->getCostEstimation = oracleGetCostEstimation;
	plugin->getKeyInformation = oracleGetKeyInformation;
	plugin->sqlTypeToDT = oracleBackendSQLTypeToDT;
	plugin->dataTypeToSQL = oracleBackendDatatypeToSQL;

	plugin->checkPostive = oracleCheckPostive;
	plugin->trasnferRawData = oracleTransferRawData;
	plugin->getMinAndMax = oracleGetMinAndMax;
	plugin->getRowNum = oracleGetRowNum;
	plugin->getDistinct = oracleGetDistinct;
	plugin->getHist = oracleGetHist;
	plugin->get2DHist = oracleGet2DHist;
	plugin->storeInterval = oracleStoreInterval;
	plugin->join2Hist = oracleJoin2Hist;
	plugin->computeSum = oracleComputeSumFromHist;
	plugin->projectionFiltering = oracleProjectionFiltering;
	plugin->selectionFiltering = oracleSelectionFiltering;
	plugin->get1Dhist = oracleGet1Dhist;
	plugin->computeSelectivity = oracleComputeSelectivity;
	plugin->getSamples = oracleGetSamples;
	plugin->getSamples2 = oracleGetSamples2;
	plugin->getSampleStat = oracleGetSampleStat;
	plugin->getPartitionSizes = oracleGetPartitionSizes;
	plugin->getPartitionSizes2 = oracleGetPartitionSizes2;
	plugin->createSampleTable = oracleCreateSampleTable;
	plugin->storePartitionSizes = oracleStorePartitionSizes;
	plugin->getSamplesDirectly = oracleGetSamplesDirectly;
	plugin->dropTable = oracleDropTable;
	plugin->storeSelectivty = oracleStoreSelectivty;
	plugin->storeGroupbyCount = oracleStoreGroupbyCount;
	plugin->findTheMax = oracleFindTheMax;
	plugin->partialSample = oraclePartialSample;
	plugin->getCount = oracleGetCount;
	plugin->getStatPartialSample = oracleGetStatPartialSample;
	plugin->storeSelectivty2 = oracleStoreSelectivty2;
	plugin->insertSelectivity = oracleInsertSelectivity;
	plugin->createTable = oracleCreateTable;
	return plugin;
}

static void handleError(OCI_Error *error) {
	errorCache = error;
	ERROR_LOG(
			"METADATA LOOKUP - OCILIB Error ORA-%05i - msg : %s\nSQL\n===\n%s",
			OCI_ErrorGetOCICode(error), OCI_ErrorGetString(error),
			OCI_GetSql(OCI_ErrorGetStatement(error)));
}

static void initAggList(void) {
	//malloc space
	aggList = CNEW(char*, AGG_FUNCTION_COUNT);

	//assign string value
	aggList[AGG_MAX] = "max";
	aggList[AGG_MIN] = "min";
	aggList[AGG_AVG] = "avg";
	aggList[AGG_COUNT] = "count";
	aggList[AGG_SUM] = "sum";
	aggList[AGG_FIRST] = "first";
	aggList[AGG_LAST] = "last";
	aggList[AGG_CORR] = "corr";
	aggList[AGG_COVAR_POP] = "covar_pop";
	aggList[AGG_COVAR_SAMP] = "covar_samp";
	aggList[AGG_GROUPING] = "grouping";
	aggList[AGG_REGR] = "regr";
	aggList[AGG_STDDEV] = "stddev";
	aggList[AGG_STDDEV_POP] = "stddev_pop";
	aggList[AGG_STDEEV_SAMP] = "stddev_samp";
	aggList[AGG_VAR_POP] = "var_pop";
	aggList[AGG_VAR_SAMP] = "var_samp";
	aggList[AGG_VARIANCE] = "variance";
	aggList[AGG_XMLAGG] = "xmlagg";
	aggList[AGG_STRAGG] = "stragg";
}

static void freeAggList() {
	if (aggList != NULL)
		FREE(aggList);
	aggList = NULL;
}

static void initWinfList(void) {
	// malloc space
	winfList = CNEW(char*, WINF_FUNCTION_COUNT);

	// add functions
	winfList[WINF_MAX] = "max";
	winfList[WINF_MIN] = "min";
	winfList[WINF_AVG] = "avg";
	winfList[WINF_COUNT] = "count";
	winfList[WINF_SUM] = "sum";
	winfList[WINF_FIRST] = "first";
	winfList[WINF_LAST] = "last";

	// window specific
	winfList[WINF_FIRST_VALUE] = "first_value";
	winfList[WINF_ROW_NUMBER] = "row_number";
	winfList[WINF_RANK] = "rank";
	winfList[WINF_LAG] = "lag";
	winfList[WINF_LEAD] = "lead";
}

static void freeWinfList(void) {
	if (winfList != NULL)
		FREE(winfList);
	winfList = NULL;
}

static void freeBuffers() {
	if (tableBuffers != NULL) {
		//deep free table buffers
		FOREACH(TableBuffer, t, tableBuffers)
		{
			FREE(t->tableName);
			deepFree(t->attrs);
		}
		freeList(tableBuffers);
	}
	if (viewBuffers != NULL) {
		//deep free view buffers
		FOREACH(ViewBuffer, v, viewBuffers)
		{
			FREE(v->viewDefinition);
			FREE(v->viewName);
		}
		freeList(viewBuffers);
	}
	tableBuffers = NIL;
	viewBuffers = NIL;
	keys = NULL;
	haveKeys = NULL;
}

static void addToTableBuffers(char* tableName, List *attrList) {
	TableBuffer *t = NEW(TableBuffer);
	char *name = strdup(tableName);
	t->tableName = name;
	t->attrs = attrList;
	tableBuffers = appendToTailOfList(tableBuffers, t);
}

static void addToViewBuffers(char *viewName, char *viewDef) {
	ViewBuffer *v = NEW(ViewBuffer);
	char *name = strdup(viewName);
	v->viewName = name;
	v->viewDefinition = viewDef;
	viewBuffers = appendToTailOfList(viewBuffers, v);
}

static List *
searchTableBuffers(char *tableName) {
	if (tableBuffers == NULL || tableName == NULL)
		return NIL;
	FOREACH(TableBuffer, t, tableBuffers)
	{
		if (strcmp(t->tableName, tableName) == 0)
			return t->attrs;
	}
	return NIL;
}
static char *
searchViewBuffers(char *viewName) {
	if (viewBuffers == NULL || viewName == NULL)
		return NULL;
	FOREACH(ViewBuffer, v, viewBuffers)
	{
		if (strcmp(v->viewName, viewName) == 0)
			return v->viewDefinition;
	}
	return NULL;
}

static int initConnection() {
	ASSERT(initialized);

	ACQUIRE_MEM_CONTEXT(context);

	StringInfo connectString = makeStringInfo();
//    Options* options=getOptions();

	char* user = getStringOption("connection.user");
	char* passwd = getStringOption("connection.passwd");
	char* db = getStringOption("connection.db");
	char *host = getStringOption("connection.host");
	int port = getIntOption("connection.port");

	// use different templates depending on whether connecting using an SID or a SERVICE_NAME
	if (getBoolOption(OPTION_ORACLE_USE_SERVICE)) {
		appendStringInfo(connectString, ORACLE_TNS_CONNECTION_FORMAT_SERVICE,
				host ? host : "", port ? port : 1521, db ? db : "");
	} else {
		appendStringInfo(connectString, ORACLE_TNS_CONNECTION_FORMAT,
				host ? host : "", port ? port : 1521, db ? db : "");
	}

	conn = OCI_ConnectionCreate(connectString->data, user ? user : "",
			passwd ? passwd : "",
			OCI_SESSION_DEFAULT);
	DEBUG_LOG("Try to connect to server <%s,%s,%s>... %s", connectString->data,
			user ? user : "", passwd ? passwd : "",
			(conn != NULL) ? "SUCCESS" : "FAILURE");

	initAggList();
	initWinfList();
	keys = NEW_MAP(Constant, List);
	haveKeys = STRSET();
	RELEASE_MEM_CONTEXT();

	return EXIT_SUCCESS;
}

static boolean isConnected() {
	if (conn == NULL)
		initConnection();
	if (OCI_IsConnected(conn))
		return TRUE;
	else {
		FATAL_LOG("OCI connection lost: %s", OCI_ErrorGetString(errorCache));
		return FALSE;
	}
}

int oracleInitMetadataLookupPlugin(void) {
	if (initialized) {
		INFO_LOG("tried to initialize metadata lookup plugin more than once");
		return EXIT_SUCCESS;
	}

	NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT("metadataContext");
	context = getCurMemContext();

	if (!OCI_Initialize(handleError, NULL, OCI_ENV_DEFAULT)) {
		FATAL_LOG("Cannot initialize OICLIB: %s",
				OCI_ErrorGetString(errorCache)); //print error type
		RELEASE_MEM_CONTEXT();

		return EXIT_FAILURE;
	}

	DEBUG_LOG("Initialized OCILIB");
	RELEASE_MEM_CONTEXT();
	initialized = TRUE;

	return EXIT_SUCCESS;
}

int oracleShutdownMetadataLookupPlugin(void) {
	initialized = FALSE;
	return oracleDatabaseConnectionClose();
}

boolean oracleIsInitialized(void) {
	return initialized;
}

OCI_Connection *
getConnection() {
	if (isConnected())
		return conn;
	return NULL;
}

static char *
oracleGetConnectionDescription(void) {
	return CONCAT_STRINGS("Oracle:", getStringOption("connection.user"), "@",
			getStringOption("connection.host"));
}

boolean oracleCatalogTableExists(char* tableName) {
	if (NULL == tableName)
		return FALSE;
	if (conn == NULL)
		initConnection();

	START_TIMER("module - metadata lookup");

	if (isConnected()) {
		STOP_TIMER("module - metadata lookup");
		return (OCI_TypeInfoGet(conn, tableName, OCI_TIF_TABLE) == NULL) ?
		FALSE :
																			TRUE;
	}

	STOP_TIMER("module - metadata lookup");

	return FALSE;
}

boolean oracleCatalogViewExists(char* viewName) {
	if (NULL == viewName)
		return FALSE;
	if (conn == NULL)
		initConnection();

	START_TIMER("module - metadata lookup");
	if (isConnected()) {
		STOP_TIMER("module - metadata lookup");
		return (OCI_TypeInfoGet(conn, viewName, OCI_TIF_VIEW) == NULL) ?
		FALSE :
																			TRUE;
	}

	STOP_TIMER("module - metadata lookup");

	return FALSE;
}

boolean oracleCheckPostive(char* tableName, char* colName) {

	StringInfo statement;
	char *result;
	char *dataType;

	START_TIMER("module - metadata lookup");

	statement = makeStringInfo();
	appendStringInfo(statement,
			"SELECT COLUMN_NAME,LOW_VALUE,DATA_TYPE "
					"FROM ALL_TAB_COLUMNS "
					"WHERE OWNER = 'FGA_USER' AND TABLE_NAME = '%s' AND COLUMN_NAME = '%s'",
			tableName, colName);

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {

				result = strdup((char * )OCI_GetString(rs, 2));
				dataType = strdup((char * )OCI_GetString(rs, 3));
				//result2 = strdup((char * )OCI_GetString(rs, 2));
				//DEBUG_LOG("result is %s", result);
				DEBUG_LOG("The lowest value is %s", result);
				DEBUG_LOG("The datatype is %s", dataType);
				STOP_TIMER("module - metadata lookup");
				//return transferRawData(result, dataType);
				return TRUE;
			} else {
				return TRUE;
			}
		}
	} else {
		DEBUG_LOG("No connection");
	}

	FREE(statement);
	STOP_TIMER("module - metadata lookup");

	return FALSE;
}
/*
 char*
 oracleTransferRawData(char* data, char* dataType) {
 StringInfo statement;
 char *result = "";
 statement = makeStringInfo();
 START_TIMER("module - metadata lookup");
 if (!strcmp(dataType, "NUMBER")) {
 appendStringInfo(statement,
 "SELECT utl_raw.cast_to_number('%s') FROM dual", data);
 } else if (!strcmp(dataType, "DOUBLE")) {
 appendStringInfo(statement,
 "SELECT utl_raw.cast_to_binary_double('%s') FROM dual", data);
 } else if (!strcmp(dataType, "FLOAT")) {
 appendStringInfo(statement,
 "SELECT utl_raw.cast_to_binary_float('%s') FROM dual", data);
 } else {
 return NULL;
 }

 if ((conn = getConnection()) != NULL) {
 OCI_Resultset *rs = executeStatement(statement->data);

 if (rs != NULL) {
 if (OCI_FetchNext(rs)) {

 result = strdup((char * )OCI_GetString(rs, 1));
 DEBUG_LOG("The value is %s", result);
 STOP_TIMER("module - metadata lookup");
 } else {
 return NULL;
 }
 }
 } else {
 DEBUG_LOG("No connection");
 }
 FREE(statement);
 STOP_TIMER("module - metadata lookup");
 return result;
 }
 */
Constant*
oracleTransferRawData(char* data, char* dataType) {
	StringInfo statement;
	char *result = "";
	statement = makeStringInfo();
	START_TIMER("module - metadata lookup");
	if (!strcmp(dataType, "NUMBER")) {
		appendStringInfo(statement,
				"SELECT utl_raw.cast_to_number('%s') FROM dual", data);
	} else if (!strcmp(dataType, "DOUBLE")) {
		appendStringInfo(statement,
				"SELECT utl_raw.cast_to_binary_double('%s') FROM dual", data);
	} else if (!strcmp(dataType, "FLOAT")) {
		appendStringInfo(statement,
				"SELECT utl_raw.cast_to_binary_float('%s') FROM dual", data);
	} else {
		return NULL;
	}

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {

				result = strdup((char * )OCI_GetString(rs, 1));
				if (!strcmp(dataType, "NUMBER")) {
					Constant *value = createConstInt(atoi(result));
					DEBUG_LOG("The value is %d", *((int * ) value->value));
					return value;
				}
				if (!strcmp(dataType, "DOUBLE") || !strcmp(dataType, "FLOAT")) {
					Constant *value = createConstFloat(atof(result));
					DEBUG_LOG("The value is %d", *((float * ) value->value));
					return value;
				}
				STOP_TIMER("module - metadata lookup");
			} else {
				return NULL;
			}
		}
	} else {
		DEBUG_LOG("No connection");
	}
	FREE(statement);
	STOP_TIMER("module - metadata lookup");
	return NULL;
}

HashMap *
oracleGetMinAndMax(char* tableName, char* colName) {
	StringInfo statement;
	char *lowest;
	char *highest;
	char *dataType;
	HashMap *result_map = NEW_MAP(Constant, Node);
	START_TIMER("module - metadata lookup");

	statement = makeStringInfo();
	appendStringInfo(statement,
			"SELECT COLUMN_NAME,LOW_VALUE,HIGH_VALUE,DATA_TYPE "
					"FROM ALL_TAB_COLUMNS "
					"WHERE OWNER = 'TPCH_1GB' AND TABLE_NAME = '%s' AND COLUMN_NAME = '%s'",
			tableName, colName);

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {

				lowest = strdup((char * )OCI_GetString(rs, 2));
				highest = strdup((char * )OCI_GetString(rs, 3));
				dataType = strdup((char * )OCI_GetString(rs, 4));
				//result2 = strdup((char * )OCI_GetString(rs, 2));
				//DEBUG_LOG("result is %s", result);
				DEBUG_LOG("The lowest value is %s", lowest);
				DEBUG_LOG("The highest value is %s", highest);
				DEBUG_LOG("The datatype is %s", dataType);
				STOP_TIMER("module - metadata lookup");
				MAP_ADD_STRING_KEY(result_map, "MIN",
						(Node * ) transferRawData(lowest, dataType));
				MAP_ADD_STRING_KEY(result_map, "MAX",
						(Node * ) transferRawData(highest, dataType));
				return result_map;
			} else {
				return NULL;
			}
		}
	} else {
		DEBUG_LOG("No connection");
	}

	FREE(statement);
	STOP_TIMER("module - metadata lookup");

	return result_map;
}
/*
 HashMap *
 oracleGetHistogram(char* tableName, char* colName) {
 StringInfo statement1;
 StringInfo statement2;
 char *lowest;
 char *highest;

 HashMap *result_map = NEW_MAP(Constant, Node);
 START_TIMER("module - metadata lookup");

 statement1 = makeStringInfo();
 appendStringInfo(statement1,
 "BEGIN  DBMS_STATS.GATHER_TABLE_STATS ("
 "ownname          => 'crimes',"
 "tabname          => '%s',"
 "method_opt       => 'FOR COLUMNS %s SIZE 100'"
 "estimate_percent => 100 );"
 "END",
 tableName, colName);
 statement2 = makeStringInfo();
 appendStringInfo(statement2,
 "SELECT ENDPOINT_NUMBER, ENDPOINT_VALUE"
 "FROM USER_HISTOGRAMS"
 "WHERE TABLE_NAME='%s'"
 "AND COLUMN_NAME='%s';",
 tableName, colName);

 if ((conn = getConnection()) != NULL) {
 OCI_Resultset *rs = executeStatement(statement1->data);
 rs = executeStatement(statement2->data);

 if (rs != NULL) {
 if (OCI_FetchNext(rs)) {

 lowest = strdup((char * )OCI_GetString(rs, 1));
 highest = strdup((char * )OCI_GetString(rs, 2));

 //result2 = strdup((char * )OCI_GetString(rs, 2));
 //DEBUG_LOG("result is %s", result);
 DEBUG_LOG("The lowest value is %s", lowest);
 DEBUG_LOG("The highest value is %s", highest);
 //DEBUG_LOG("The datatype is %s", dataType);
 STOP_TIMER("module - metadata lookup");
 MAP_ADD_STRING_KEY(result_map, "MIN", (Node *) transferRawData(lowest, "NUMBER"));
 MAP_ADD_STRING_KEY(result_map, "MAX", (Node *) transferRawData(highest, "NUMBER"));
 return result_map;
 } else {
 return NULL;
 }
 }
 } else {
 DEBUG_LOG("No connection");
 }

 FREE(statement1);
 FREE(statement2);
 STOP_TIMER("module - metadata lookup");


 return result_map;

 }
 */
int oracleGetRowNum(char* tableName) {
	StringInfo statement;
	char *rowNum;
	START_TIMER("module - metadata lookup");

	statement = makeStringInfo();
	appendStringInfo(statement,
			"SELECT NUM_ROWS FROM ALL_TABLES WHERE OWNER = 'TPCH_1GB' AND TABLE_NAME = '%s'",
			tableName);

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {

				rowNum = strdup((char * )OCI_GetString(rs, 1));
				//DEBUG_LOG("LZY IS%s",rowNum);
				STOP_TIMER("module - metadata lookup");
				return atoi(rowNum);
			} else {
				return 0;
			}
		}
	} else {
		DEBUG_LOG("No connection");
	}

	FREE(statement);
	STOP_TIMER("module - metadata lookup");

	return 0;
}
int oracleGetDistinct(char* tableName, char* colName) {
	StringInfo statement;
	char *rowNum;
	START_TIMER("module - metadata lookup");

	statement = makeStringInfo();
	appendStringInfo(statement,
			"SELECT NUM_DISTINCT "
					"FROM ALL_TAB_COLUMNS "
					"WHERE OWNER = 'TPCH_1GB' AND TABLE_NAME = '%s' AND COLUMN_NAME = '%s'",
			tableName, colName);

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {
				rowNum = strdup((char * )OCI_GetString(rs, 1));
				STOP_TIMER("module - metadata lookup");
				return atoi(rowNum);
			} else {
				return 0;
			}
		}
	} else {
		DEBUG_LOG("No connection");
	}

	FREE(statement);
	STOP_TIMER("module - metadata lookup");

	return 0;
}
/*List *
 oracleGet2DHist(char *tableName, char *attrName, char *attrName2, char* numPartitions) {
 StringInfo statement;
 char *defaultExpr="";
 //char *rowNum;
 START_TIMER("module - metadata lookup");
 statement = makeStringInfo();

 char * test = appendStatement(tableName,attrName,attrName2,numPartitions);
 DEBUG_LOG("lzy is %s",test);
 appendStringInfo(statement,
 "select * from (select l_partkey,l_suppkey,pos,cnt,numTuples, flr, count(distinct l_partkey) OVER (partition by flr) as dist, minl, maxl,minlpartkey "
 "from (select l_partkey, l_suppkey, pos, cnt, ROUND(cnt / 300000,0) as numTuples, FLOOR((pos-1)/(ROUND(cnt / 300000,0))) as flr, minl, maxl,minlpartkey "
 "from ( select l_partkey,l_suppkey, row_number() OVER (ORDER BY l_partkey,l_suppkey) as pos ,count(*) OVER () AS cnt ,min(l_suppkey) over() as minl,max(l_suppkey) over() as maxl, min(l_partkey) over() as minlpartkey "
 "from lineitem ) \n"
 ")) where MOD(pos,(ROUND(cnt / 300000,0))) = 0",
 tableName, attrName, attrName2, numPartitions);
 appendStringInfo(statement,test);

 if ((conn = getConnection()) != NULL) {
 OCI_Resultset *rs = executeStatement(statement->data);

 if (rs != NULL) {
 while(OCI_FetchNext(rs)){
 defaultExpr = (char *) OCI_GetString(rs,2);
 DEBUG_LOG("lzy111 is: %s",defaultExpr);
 }
 }

 } else {
 DEBUG_LOG("No connection");
 }

 FREE(statement);
 STOP_TIMER("module - metadata lookup");


 return NULL;
 } */
char *
oracleGet2DHist(char *tableName, char *attrName, char *attrName2,
		char *numPartitions1, char *numPartitions2) {

	//StringInfo setStatement = makeStringInfo();
	//StringInfo statement = makeStringInfo();
	//StringInfo intervalPoints = makeStringInfo();
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo statement2 = makeStringInfo();

	ACQUIRE_MEM_CONTEXT(context);
	START_TIMER("module - metadata lookup");

	DEBUG_LOG("Get histogram for %s.%s with number of ps %d", tableName,
			attrName, numPartitions1);
	List *lastInterval = getLastInterval(tableName, attrName, numPartitions1);
	char *sp = getHeadOfList(lastInterval)->data.ptr_value;
	char *ep = getTailOfList(lastInterval)->data.ptr_value;
	DEBUG_LOG("lzy are %s,%s:", sp, ep);
	char *st0 = CONCAT_STRINGS("DROP TABLE ", tableName, "#", attrName, "#",
			attrName2, "#HIST ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", tableName, "#", attrName, "#",
			attrName2, "#HIST AS (");
	st1 = CONCAT_STRINGS(st1, "SELECT ", attrName, ", ", attrName2,
			", POS,CNT,SP,EP,FLOOR(CNT / ", numPartitions2,
			") AS TUPLENUM, DIS_", attrName, ", DIS_", attrName2, " FROM (");
	st1 = CONCAT_STRINGS(st1, "SELECT ", attrName, ", ", attrName2,
			", row_number() OVER (PARTITION BY SP ORDER BY ", attrName, ",",
			attrName2,
			") AS POS, COUNT(*) OVER (PARTITION BY SP) AS CNT, COUNT(distinct ",
			attrName, ") OVER (PARTITION BY SP) AS DIS_", attrName,
			", COUNT(distinct ", attrName2, ") OVER (PARTITION BY SP) AS DIS_",
			attrName2, ", SP, EP FROM (");
	st1 = CONCAT_STRINGS(st1, "SELECT * FROM ((SELECT ", attrName, ", ",
			attrName2, ", SP, EP FROM ", tableName, " JOIN ", tableName, "_",
			attrName, " ON (", attrName, ">=SP AND EP>", attrName,
			")) UNION ALL (SELECT ", attrName, ", ", attrName2, ", ", sp,
			" AS SP, ", ep, " AS EP FROM ", tableName, " WHERE ", attrName, "=",
			ep, ")) ORDER BY ", attrName, ", ", attrName2, " )) ");
	st1 = CONCAT_STRINGS(st1, "WHERE MOD(POS,(FLOOR(CNT / ", numPartitions2,
			"))) = 0) ");
	DEBUG_LOG("lzy are %s:", st1);

	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	return CONCAT_STRINGS(tableName, "#", attrName, "#", attrName2, "#HIST");;
}
char *
appendStatement(char *tableName, char *attrName, char *attrName2,
		char* numPartitions) {

	//DEBUG_LOG("lzy is %s,%s,%s,%d",tableName,attrName,attrName2,numPartitions);
	//st = concatStrings("select * from (select ", attrName, attrName2);
	char *st = CONCAT_STRINGS("select ", attrName, " ,", attrName2,
			", pos, numTuples, dist_", attrName, ", dist_", attrName2, " ,min_",
			attrName2, ", max_", attrName2, ", min_", attrName, " from (");
	st = CONCAT_STRINGS(st, "select * from (select ", attrName, " ,", attrName2,
			", pos, cnt, numTuples, flr, count(distinct ", attrName,
			") OVER (partition by flr) as dist_", attrName, ", count(distinct ",
			attrName2, ") OVER (partition by flr) as dist_", attrName2,
			", min_", attrName2, ", max_", attrName2, ", min_", attrName, " ");
	st = CONCAT_STRINGS(st, "from (select ", attrName, " ,", attrName2,
			", pos, cnt, ROUND(cnt /", numPartitions,
			",0) as numTuples, FLOOR((pos-1)/(ROUND(cnt /", numPartitions,
			",0))) as flr, min_", attrName2, ", max_", attrName2, ", min_",
			attrName, " ");
	st = CONCAT_STRINGS(st, "from (select ", attrName, " ,", attrName2,
			", row_number() OVER (ORDER BY ", attrName, " ,", attrName2,
			") as pos ,count(*) OVER () AS cnt ,min(", attrName2,
			") over() as min_", attrName2, ", max(", attrName2,
			") over() as max_", attrName2, ", min(", attrName,
			") over() as min_", attrName, ", max(", attrName,
			") over() as max_", attrName, " ");
	st = CONCAT_STRINGS(st, "from ", tableName, ") \n");
	st = CONCAT_STRINGS(st, ")) where MOD(pos,(ROUND(cnt / ", numPartitions,
			",0))) = 0)");
	//st = CONCAT_STRINGS("select * from (select ", attrName);//, "," ,attrName,"");//, ",pos, cnt, numTuples, flr, count(distinct ", attrName); // ") OVER (partition by flr) as dist, minl, maxl,minlpartkey ");
	//st = CONCAT_STRINGS(st, "from (select ", attrName, attrName2, ", pos, cnt, ROUND(cnt /", numPartitions, ",0) as numTuples, FLOOR((pos-1)/(ROUND(cnt / ", numPartitions,",0))) as flr, minl, maxl,minlpartkey ");
	//CONCAT_STRINGS("select * from (select ", attrName, attrName2,"pos,cnt,numTuples, flr");
	//DEBUG_LOG("lzy is %s",st);
	return st;
}

List *
oracleGetHist(char *tableName, char *attrName, char *numPartitions) {
	List *l = NIL;
	//StringInfo setStatement = makeStringInfo();
	//StringInfo statement = makeStringInfo();
	//StringInfo intervalPoints = makeStringInfo();
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo statement2 = makeStringInfo();

	ACQUIRE_MEM_CONTEXT(context);
	START_TIMER("module - metadata lookup");

	DEBUG_LOG("Get histogram for %s.%s with number of ps %d", tableName,
			attrName, numPartitions);
	List *lastInterval = getLastInterval(tableName, attrName, numPartitions);
	char *sp = getHeadOfList(lastInterval)->data.ptr_value;
	char *ep = getTailOfList(lastInterval)->data.ptr_value;
	DEBUG_LOG("lzy are %s,%s:", sp, ep);
	char *st0 = CONCAT_STRINGS("DROP TABLE ", tableName, "_", attrName,
			"_beat_hist ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", tableName, "_", attrName,
			"_beat_hist AS (");
	st1 = CONCAT_STRINGS(st1, "SELECT ", attrName,
			", BEAT, POS,CNT,SP,EP,FLOOR(CNT / 20) AS TUPLENUM, DIS_", attrName,
			" FROM (");
	st1 =
			CONCAT_STRINGS(st1, "SELECT ", attrName,
					", BEAT, row_number() OVER (PARTITION BY SP ORDER BY BEAT) AS POS, COUNT(*) OVER (PARTITION BY SP) AS CNT, COUNT(distinct ",
					attrName, ") OVER (PARTITION BY SP) AS DIS_", attrName,
					",  SP, EP FROM (");
	st1 = CONCAT_STRINGS(st1, "SELECT * FROM ((SELECT ", attrName,
			", BEAT, SP, EP FROM ", tableName, " JOIN ", tableName, "_",
			attrName, " ON (", attrName, ">=SP AND EP>", attrName,
			")) UNION ALL (SELECT ", attrName, ", BEAT,", sp, " AS SP, ", ep,
			" AS EP FROM ", tableName, " WHERE ", attrName, "=", ep,
			")) ORDER BY ", attrName, ", BEAT )) ");
	st1 = CONCAT_STRINGS(st1, "WHERE MOD(POS,(FLOOR(CNT / 20))) = 0) ");
	DEBUG_LOG("lzy are %s:", st1);

	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		FREE(statement0);
		FREE(statement1);
		//executeStatement(statement2->data);
		//executeStatement(setStatement->data);
		//executeNonQueryStatement(setStatement->data);
		//executeNonQueryStatement(statement1->data);
		//OCI_Resultset *rs = executeStatement(statement0->data);
		//DEBUG_NODE_BEATIFY_LOG("lzy are:", rs);
		/*if (rs != NULL) {

		 DEBUG_LOG("lzy are successfully");



		 }

		 FREE(statement0);

		 // FREE(statement3);
		 STOP_TIMER("module - metadata lookup");
		 RELEASE_MEM_CONTEXT_AND_RETURN_STRINGLIST_COPY(l);*/
	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");

	/*appendStringInfo(setStatement, "BEGIN DBMS_STATS.GATHER_TABLE_STATS (  \n"
	 "  ownname  => 'crimes' \n"
	 ", tabname  => 'CRIMES' \n"
	 ", method_opt  => 'FOR COLUMNS ZIP_CODES SIZE 30' \n"
	 ", estimate_percent => 100 \n"
	 "); \n"
	 "END;");
	 appendStringInfo(setStatement, "BEGIN DBMS_STATS.GATHER_TABLE_STATS (  \n"
	 "  ownname  => 'crimes' \n"
	 ", tabname  => '%s' \n"
	 ", method_opt  => 'FOR COLUMNS %s SIZE %d' \n"
	 ", estimate_percent => 100 \n"
	 "); \n"
	 "END;",
	 tableName, attrName, numPartitions);
	 //appendStringInfo(setStatement,"EXECUTE DBMS_STATS.GATHER_TABLE_STATS ( ownname  => 'TPCH_1GB', tabname  => '%s', method_opt  => 'FOR COLUMNS %s SIZE %d', estimate_percent => 100);",tableName, attrName, numPartitions);

	 appendStringInfo(statement, "SELECT ENDPOINT_NUMBER, ENDPOINT_VALUE FROM USER_HISTOGRAMS WHERE TABLE_NAME='%s' AND COLUMN_NAME='%s'",
	 tableName, attrName);

	 if ((conn = getConnection()) != NULL)
	 {
	 //executeStatement(setStatement->data);
	 executeNonQueryStatement(setStatement->data);
	 OCI_Resultset *rs = executeStatement(statement->data);
	 //DEBUG_NODE_BEATIFY_LOG("lzy are:", rs);
	 char *defaultExpr = NULL;
	 //Node *result = NULL;

	 // loop through
	 int cnt = 0;
	 char *min = "";
	 char *max = "";
	 appendStringInfo(intervalPoints, "{");
	 while(OCI_FetchNext(rs))
	 {
	 defaultExpr = (char *) OCI_GetString(rs,2);
	 if(cnt == 0)
	 min = strdup(defaultExpr);
	 if(cnt == numPartitions)
	 max = strdup(defaultExpr);
	 appendStringInfo(intervalPoints, defaultExpr);
	 appendStringInfo(intervalPoints, ",");

	 cnt ++;
	 DEBUG_LOG("histogram for %s.%s is <%s>",
	 tableName, attrName, defaultExpr);
	 }
	 removeTailingStringInfo(intervalPoints,1);
	 appendStringInfo(intervalPoints, "}");
	 // DEBUG_LOG("Statement: %s executed successfully.", statement->data);
	 DEBUG_LOG("intervalPoints: %s .", intervalPoints->data);
	 l = appendToTailOfList(l, intervalPoints->data);
	 l = appendToTailOfList(l, min);
	 l = appendToTailOfList(l, max);

	 FREE(statement);

	 STOP_TIMER("module - metadata lookup");
	 RELEASE_MEM_CONTEXT_AND_RETURN_STRINGLIST_COPY(l);
	 }
	 else
	 {
	 FATAL_LOG("Statement: %s failed.", statement);
	 FREE(statement);
	 }
	 STOP_TIMER("module - metadata lookup");
	 RELEASE_MEM_CONTEXT_AND_RETURN_STRINGLIST_COPY(l);
	 */
	return l;
}
char*
oracleGet1Dhist(char *tableName, char *colName, char *numPartitions) {
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo statement2 = makeStringInfo();
	ACQUIRE_MEM_CONTEXT(context);
	START_TIMER("module - metadata lookup");

	DEBUG_LOG("Get histogram for %s.%s with number of ps %d", tableName,
			colName, numPartitions);
	List *lastInterval = getLastInterval(tableName, colName, numPartitions);
	char *sp = getHeadOfList(lastInterval)->data.ptr_value;
	char *ep = getTailOfList(lastInterval)->data.ptr_value;
	char *resultTable = CONCAT_STRINGS(tableName, "#", colName, "#ONEDHIST");
	char *table2 = CONCAT_STRINGS(tableName, "#", colName);
	DEBUG_LOG("lzy are %s,%s:", sp, ep);
	char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable,
			" AS (SELECT * FROM(");
	st1 =
			CONCAT_STRINGS(st1, "SELECT DISTINCT SP AS ", colName,
					",COUNT(*) OVER (PARTITION BY SP) AS ESTIMATEDNUM FROM (SELECT * FROM ((SELECT ",
					tableName, ".", colName, ", ", table2, ".SP, ", table2,
					".EP FROM ", tableName, " JOIN ", table2, " ON (",
					tableName, ".", colName, ">=", table2, ".SP AND ", table2,
					".EP>", tableName, ".", colName, ")) UNION ALL (SELECT ",
					colName, ",", sp, " AS SP, ", ep, " AS EP FROM ", tableName,
					" WHERE ", colName, "=", ep, ")))ORDER BY SP)) ");
	DEBUG_LOG("lzy are %s:", st1);

	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		//DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	return resultTable;
}
List*
getLastInterval(char *tableName, char *attrName, char *numPartitions) {
	List *l = NIL;
	StringInfo statement = makeStringInfo();
	appendStringInfo(statement,
			"SELECT * FROM %s#%s WHERE ENDPOINT_NUMBER = (%s-1)", tableName,
			attrName, numPartitions);
	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);
		//DEBUG_NODE_BEATIFY_LOG("lzy are:", rs);
		if (rs != NULL) {
			while (OCI_FetchNext(rs)) {
				char *sp = (char *) OCI_GetString(rs, 2);
				char *ep = (char *) OCI_GetString(rs, 3);
				//DEBUG_LOG("LZY is %s, %s", sp, ep);
				l = appendToTailOfList(l, sp);
				l = appendToTailOfList(l, ep);

			}
		}

		FREE(statement);
		// FREE(statement3);
		STOP_TIMER("module - metadata lookup");
		RELEASE_MEM_CONTEXT_AND_RETURN_STRINGLIST_COPY(l);
	} else {
		FATAL_LOG("Statement: %s failed.", statement);
		FREE(statement);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	RELEASE_MEM_CONTEXT_AND_RETURN_STRINGLIST_COPY(l);
	return l;

}
void oracleStoreInterval(char *tableName, char *attrName, char *numPartitions) {
	StringInfo setStatement = makeStringInfo();
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo statement2 = makeStringInfo();
	//StringInfo statement3 = makeStringInfo();
	//StringInfo intervalPoints = makeStringInfo();

	ACQUIRE_MEM_CONTEXT(context);
	START_TIMER("module - metadata lookup");

	DEBUG_LOG("Get histogram for %s.%s with number of ps %s", tableName,
			attrName, numPartitions);
	appendStringInfo(setStatement, "BEGIN DBMS_STATS.GATHER_TABLE_STATS (  \n"
			"  ownname  => 'crimes' \n"
			", tabname  => '%s' \n"
			", method_opt  => 'FOR COLUMNS %s SIZE %s' \n"
			", estimate_percent => 100 \n"
			"); \n"
			"END;", tableName, attrName, numPartitions);
	char *st0 = CONCAT_STRINGS("DROP TABLE ", tableName, "#", attrName, " ");
	char *st1 =
			CONCAT_STRINGS("CREATE TABLE ", tableName, "#", attrName,
					" AS ( SELECT * FROM (SELECT ENDPOINT_NUMBER, ENDPOINT_VALUE AS SP, LEAD(ENDPOINT_VALUE,1) OVER (ORDER BY ENDPOINT_VALUE) AS EP FROM USER_HISTOGRAMS WHERE TABLE_NAME='",
					tableName, "' AND COLUMN_NAME='", attrName, "' ))");
	//char *st2 = CONCAT_STRINGS("DELETE FROM ", tableName,"_",attrName," WHERE ENDPOINT_NUMBER = ",numPartitions," ");

	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	//appendStringInfo(statement2, "DELETE FROM %s_%s WHERE ENDPOINT_NUMBER = %d ;",tableName, attrName, numPartitions);
	//appendStringInfo(statement2, "SELECT * FROM %s_%s WHERE ENDPOINT_NUMBER = (%s-1) ", tableName, attrName, numPartitions);
	//DEBUG_LOG("LZY is %s", statement2->data);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(setStatement->data);
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		//executeStatement(statement2->data);
		//executeStatement(setStatement->data);
		//executeNonQueryStatement(setStatement->data);
		//executeNonQueryStatement(statement1->data);
		// OCI_Resultset *rs = executeStatement(statement2->data);
		//DEBUG_NODE_BEATIFY_LOG("lzy are:", rs);
		/*if (rs != NULL) {
		 while (OCI_FetchNext(rs)) {
		 char *sp = (char *) OCI_GetString(rs, 2);
		 char *ep = (char *) OCI_GetString(rs, 3);
		 DEBUG_LOG("LZY is %s, %s", sp, ep);
		 l = appendToTailOfList(l, sp);
		 l = appendToTailOfList(l, ep);

		 }
		 }*/

		FREE(statement0);
		FREE(statement1);
		// FREE(statement2);
		// FREE(statement3);
		STOP_TIMER("module - metadata lookup");
	} else {
		FATAL_LOG("Statement: %s failed.", statement1);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement2);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");

}

char*
oracleJoin2Hist(char *hist1, char *hist2, char *tableName, char *attrName) {
	//List *l = NIL;

	//HashMap *attributes = getAttributeFromHist(hist1,hist2);
	//if(hasMapStringKey(attributes,"HIST1")){
	List *res = getAttributeFromHist(hist1, hist2);
	//Set *res2 =  (Set *) getMapString(attributes,"HIST2");
	Set *res1 = (Set *) getHeadOfList(res)->data.ptr_value;
	Set *res2 = (Set *) getTailOfList(res)->data.ptr_value;

	char *histname = CONCAT_STRINGS(tableName, "#", attrName, "#");
	FOREACH_SET(Constant,n,res1)
	{
		DEBUG_NODE_BEATIFY_LOG("res11111 are:", n);
		histname = CONCAT_STRINGS(histname, n->value, "#");
	}
	FOREACH_SET(Constant,n,res2)
	{
		//histname = CONCAT_STRINGS(histname,"_",n,"_");
		DEBUG_NODE_BEATIFY_LOG("res222 are:", n);
		histname = CONCAT_STRINGS(histname, n->value, "#");
	}
	histname = CONCAT_STRINGS(histname, "HIST");
	DEBUG_LOG("lzy555 is %s", histname);
	/*FOREACH_SET(char,n,res2) {
	 DEBUG_LOG("res2 is %s", n);
	 }
	 }*/

	/*char *st0 = "DROP TABLE CRIMES_ZIP_CODES_";
	 FOREACH_SET(char, s, res1) {
	 st0 = CONCAT_STRINGS(st0,s);
	 }
	 st0 = CONCAT_STRINGS(st0,"_WARD_HIST");
	 DEBUG_LOG("st0 IS %s",st0);*/
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", histname, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", histname, " AS (");
	st1 = CONCAT_STRINGS(st1, "SELECT ", hist1, ".", attrName, ", ");
	FOREACH_SET(Constant,n,res1)
	{
		st1 = CONCAT_STRINGS(st1, hist1, ".", n->value, ",");
	}
	FOREACH_SET(Constant,n,res2)
	{
		st1 = CONCAT_STRINGS(st1, hist2, ".", n->value, ",");
	}
	st1 = CONCAT_STRINGS(st1, hist1, ".POS, ", hist1, ".CNT, ", hist1, ".SP, ",
			hist1, ".EP, ", hist1, ".DIS_", attrName, ", ");
	FOREACH_SET(Constant,n,res1)
	{
		st1 = CONCAT_STRINGS(st1, hist1, ".DIS_", n->value, ",");
	}
	FOREACH_SET(Constant,n,res2)
	{
		st1 = CONCAT_STRINGS(st1, hist2, ".DIS_", n->value, ", ");
	}
	st1 = CONCAT_STRINGS(st1, hist1, ".TUPLENUM FROM ");
	st1 = CONCAT_STRINGS(st1, hist1, " JOIN ", hist2, " ON (", hist1, ".SP = ",
			hist2, ".SP AND ", hist1, ".POS = ", hist2, ".POS))");

	DEBUG_LOG("lzy are %s:", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("join successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	return histname;
}

List*
getAttributeFromHist(char *hist1, char *hist2) {
	char h1[100];
	char h2[100];
	strcpy(h1, hist1);
	strcpy(h2, hist2);
	List *res = NIL;

	Set *result1 = NODESET();
	Set *result2 = NODESET();
	char *d = "#";

	char *p1 = strtok(h1, d);
	while (p1 != NULL) {
		//DEBUG_LOG("LZY IS %s", p1);
		addToSet(result1, createConstString(p1));
		p1 = strtok(NULL, d);
	}
	char *p2 = strtok(h2, d);
	while (p2 != NULL) {
		//DEBUG_LOG("LZY IS %s", p2);
		addToSet(result2, createConstString(p2));
		p2 = strtok(NULL, d);
	}
	//result1 = setDifference(result1,intersectSets (result1,result2));
	//result2 = setDifference(result2,intersectSets (result1,result2));
	FOREACH_SET(Constant,n,result1)
	{
		if (hasSetElem(result2, n)) {
			removeSetElem(result2, n);
			removeSetElem(result1, n);
		}
	}
	FOREACH_SET(Constant,n,result1)
	{
		//	DEBUG_LOG("res1111 is %s", n);
		//DEBUG_LOG("lzy3");
		DEBUG_NODE_BEATIFY_LOG("res111 are:", n);
	}
	FOREACH_SET(Constant,n,result2)
	{
		DEBUG_NODE_BEATIFY_LOG("res222 are:", n);
		//DEBUG_LOG("res2222 IS %s", n);
	}

	res = appendToTailOfList(res, result1);
	res = appendToTailOfList(res, result2);
	//List *test = NIL;

	/*
	 Set *res3 =  (Set *) getHeadOfList(res)->data.ptr_value;
	 Set *res4 =  (Set *) getTailOfList(res)->data.ptr_value;

	 FOREACH_SET(Constant,n,res3) {
	 DEBUG_NODE_BEATIFY_LOG("res111 are:", n);

	 }
	 FOREACH_SET(Constant,n,res4) {
	 DEBUG_NODE_BEATIFY_LOG("res111 are:", n);

	 }*/
	return res;

}

char*
oracleComputeSumFromHist(char *tableName, char *attrName, char *sumAttrName,
		char *aggName) {

	//select zip_codes, ward, LAG(ward,1) OVER (ORDER BY SP,pos) AS lag_ward, POS,CNT,SP,
	//LAG(SP,1) OVER (ORDER BY SP,pos) as LAG_SP,EP,TUPLENUM,dis_zip_codes,DIS_WARD
	//FROM CRIMES_ZIP_CODES_BEAT_WARD_DISTRICT_HIST
	//List *res = NIL;

	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE SUM#", tableName, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE SUM#", tableName,
			" AS ( SELECT * FROM ( WITH VIEW1 AS(");
	st1 = CONCAT_STRINGS(st1, "SELECT SUM(SUM_", sumAttrName,
			") AS ESTIAMTE_SUM_", sumAttrName, ", SP, ", attrName, " FROM(");
	st1 = CONCAT_STRINGS(st1, "SELECT ", attrName, ", ", sumAttrName,
			", CASE WHEN ", attrName, "= LAG_", attrName, " THEN (",
			sumAttrName, "+LAG_", sumAttrName, ")*TUPLENUM/2 ELSE (",
			sumAttrName, "+1)*TUPLENUM/2 END AS SUM_", sumAttrName,
			", SP, TUPLENUM FROM (");
	st1 =
			CONCAT_STRINGS(st1, "SELECT ", attrName, ", CASE WHEN LAG_",
					attrName, " IS NULL THEN 111 ELSE LAG_", attrName,
					" END AS LAG_", attrName, ", ", sumAttrName,
					", CASE WHEN LAG_", sumAttrName,
					" IS NULL THEN 1 ELSE LAG_", sumAttrName, " END AS LAG_",
					sumAttrName,
					", SP, CASE WHEN LAG_SP IS NULL THEN SP ELSE LAG_SP END AS LAG_SP, EP, TUPLENUM FROM (");
	st1 = CONCAT_STRINGS(st1, "SELECT ", attrName, ", LAG(", attrName,
			",1) OVER (ORDER BY SP, POS) AS LAG_", attrName, ", ", sumAttrName,
			", LAG(", sumAttrName, ",1) OVER (ORDER BY SP, POS) AS LAG_",
			sumAttrName,
			", SP, LAG(SP,1) OVER (ORDER BY SP, POS) AS LAG_SP, EP, TUPLENUM ");
	st1 = CONCAT_STRINGS(st1, "FROM ", tableName, ")))GROUP BY SP, ", attrName,
			" ORDER BY SP, ", attrName, "), ");
	st1 = CONCAT_STRINGS(st1, "VIEW2 AS (",
			computeNumOfGroupFromHist(tableName, attrName, sumAttrName), ")");
	st1 = CONCAT_STRINGS(st1, "SELECT ESTIAMTE_SUM_", sumAttrName,
			", ESTIAMTE_SUM_", sumAttrName, "/ESTIMATEDGROUP AS ", aggName,
			", SP, NUM, ESTIMATEDNUM, ESTIMATEDGROUP FROM (");
	st1 =
			CONCAT_STRINGS(st1, "SELECT ESTIAMTE_SUM_", sumAttrName, ", ",
					attrName,
					", SP, NUM, NUM/ESTIMATEDDIS AS ESTIMATEDNUM, AVGGROUPS/ESTIMATEDDIS AS ESTIMATEDGROUP FROM (");
	st1 = CONCAT_STRINGS(st1, "SELECT ESTIAMTE_SUM_", sumAttrName, ", ",
			attrName, ", SP, NUM, COUNT (DISTINCT ", attrName,
			") OVER (PARTITION BY SP) AS ESTIMATEDDIS, AVGGROUPS FROM (");
	st1 =
			CONCAT_STRINGS(st1, "SELECT ESTIAMTE_SUM_", sumAttrName, ",",
					attrName,
					", VIEW1.SP AS SP, CNT AS NUM, AVGGROUPS FROM VIEW1 JOIN VIEW2 ON VIEW1.SP = VIEW2.SP) ORDER BY SP)))) ");
	DEBUG_LOG("lzy are %s:", st1);
	DEBUG_LOG("lzy are %s:", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("compute successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	return CONCAT_STRINGS("SUM#", tableName);;
}
char*
oracleProjectionFiltering(char *tableName, char *num, Set *attrNames) {
	char *resultTable = CONCAT_STRINGS("FILTEREDPROJECTION", num);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	StringInfo temp = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable, " AS (");
	st1 = CONCAT_STRINGS(st1, "SELECT ");
	FOREACH_SET(Constant,n,attrNames)
	{
		st1 = CONCAT_STRINGS(st1, n, ",");
	}
	appendStringInfo(temp, st1);
	removeTailingStringInfo(temp, 1);

	st1 = CONCAT_STRINGS(temp->data, " FROM ", tableName, ")");

	DEBUG_LOG("st1 are %s:", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	return resultTable;
}
char*
oracleSelectionFiltering(char *tableName, char *num, char *selection) {
	char *resultTable = CONCAT_STRINGS("FILTEREDSELECTION", num);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo temp = makeStringInfo();
	//Operator *cond = (Operator *) ((SelectionOperator *) selection)->cond;
	char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable,
			" AS (SELECT * FROM ", tableName, " WHERE ", selection, ")");
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	STOP_TIMER("module - metadata lookup");
	return resultTable;
}
double oracleComputeSelectivity(char *tableName, char *selection) {
	char *total;
	char *num;
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("SELECT SUM(ESTIMATEDNUM) FROM ", tableName);
	char *st1 = CONCAT_STRINGS("SELECT SUM(ESTIMATEDNUM) FROM (SELECT * FROM ",
			tableName, " WHERE ", selection, ")");
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement0->data);

		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {

				total = strdup((char * )OCI_GetString(rs, 1));
				DEBUG_LOG("The total value is %s", total);
			} else {
				return 0.0;
			}
		}
		rs = executeStatement(statement1->data);
		if (rs != NULL) {
			if (OCI_FetchNext(rs)) {

				num = strdup((char * )OCI_GetString(rs, 1));
				DEBUG_LOG("The NUM value is %s", num);
			} else {
				return 0.0;
			}
		}
	} else {
		DEBUG_LOG("No connection");
	}

	FREE(statement0);
	STOP_TIMER("module - metadata lookup");

	/* statement;
	 char *lowest;
	 char *highest;
	 char *dataType;

	 HashMap *result_map = NEW_MAP(Constant, Node);
	 START_TIMER("module - metadata lookup");

	 statement = makeStringInfo();
	 appendStringInfo(statement,
	 "SELECT COLUMN_NAME,LOW_VALUE,HIGH_VALUE,DATA_TYPE "
	 "FROM ALL_TAB_COLUMNS "
	 "WHERE OWNER = 'TPCH_1GB' AND TABLE_NAME = '%s' AND COLUMN_NAME = '%s'",
	 tableName, colName);

	 if ((conn = getConnection()) != NULL) {
	 OCI_Resultset *rs = executeStatement(statement->data);

	 if (rs != NULL) {
	 if (OCI_FetchNext(rs)) {

	 lowest = strdup((char * )OCI_GetString(rs, 2));
	 highest = strdup((char * )OCI_GetString(rs, 3));
	 dataType = strdup((char * )OCI_GetString(rs, 4));
	 //result2 = strdup((char * )OCI_GetString(rs, 2));
	 //DEBUG_LOG("result is %s", result);
	 DEBUG_LOG("The lowest value is %s", lowest);
	 DEBUG_LOG("The highest value is %s", highest);
	 DEBUG_LOG("The datatype is %s", dataType);
	 STOP_TIMER("module - metadata lookup");
	 MAP_ADD_STRING_KEY(result_map, "MIN", (Node *) transferRawData(lowest, dataType));
	 MAP_ADD_STRING_KEY(result_map, "MAX", (Node *) transferRawData(highest, dataType));
	 return result_map;
	 } else {
	 return NULL;
	 }
	 }
	 } else {
	 DEBUG_LOG("No connection");
	 }

	 FREE(statement);
	 STOP_TIMER("module - metadata lookup");*/

	return atof(num) / atof(total);
}
char*
oracleCreateSampleTable(char * sampleName, char *aggrName, char *groupBy) {
	char *resultTable = CONCAT_STRINGS("SAMPLE_", sampleName);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo temp = makeStringInfo();
	//Operator *cond = (Operator *) ((SelectionOperator *) selection)->cond;
	char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
	/*char *st1 =
			CONCAT_STRINGS("CREATE TABLE ", resultTable,
					" (zip_codes number, beat number, ward number, district number, pos number, bid number)");*/
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable,
								" (U_REPUTATION number, U_VIEWS number, U_UPVOTES number, U_DOWNVOTES number, pos number, bid number)");
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
//	STOP_TIMER("module - metadata lookup");
	return resultTable;
}

char*
oracleGetSamples(int num, char *query, char*sampleTable) {
	//char r[100];
	//sprintf(r,"%d",num);
	//char *resultTable = CONCAT_STRINGS("BUCKET",r,"_2");
	//StringInfo statement0 = makeStringInfo();
	//StringInfo temp = makeStringInfo();
	//Operator *cond = (Operator *) ((SelectionOperator *) selection)->cond;
	StringInfo statement0 = makeStringInfo();
	//char *st0 = CONCAT_STRINGS("INSERT INTO ", sampleTable,
	//		" (ZIP_CODES, BEAT, WARD, DISTRICT, POS, BID)(", query, ")");
	char *st0 = CONCAT_STRINGS("INSERT INTO ", sampleTable,
				" (U_REPUTATION, U_VIEWS, U_UPVOTES, U_DOWNVOTES, POS, BID)(", query, ")");
	//appendStringInfo(statement0, st0);
	appendStringInfo(statement0, st0);
	if ((conn = getConnection()) != NULL) {
		DEBUG_LOG("Lzy successfully!");
		//executeNonQueryStatement(statement0->data);
		// OCI_Resultset *rs = executeStatement(statement1->data);
		//executeQueryIgnoreResult(statement0->data);
		executeNonQueryStatement(statement0->data);
		OCI_Commit(conn);
		DEBUG_LOG("INSERTING successfully!");
		//	FREE(statement0);

	} else {
		//  FATAL_LOG("Statement: %s failed.", statement0);
		// FREE(statement0);
		FREE(statement0);
		// FREE(statement3);
	}
	//STOP_TIMER("module - metadata lookup");
	return sampleTable;
}
/*
char *
oracleGetSamples2(char *groupbyAttr, char* groupbyAttr1_groupbyAttr2, char *psAttr, char*sampleRate) {
	char* res = CONCAT_STRINGS("SAMPLE_", groupbyAttr1_groupbyAttr2, "_PS_",psAttr,"_",sampleRate);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", res, " AS (SELECT * FROM LINEITEM WHERE (",groupbyAttr,") IN (SELECT ",groupbyAttr," FROM SAMPLE_TPCH#",psAttr,"_BID_",sampleRate,"))");
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		DEBUG_LOG("Lzy successfully!");
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		//OCI_Commit(conn);

		//	FREE(statement0);

	} else {
		//  FATAL_LOG("Statement: %s failed.", statement0);
		// FREE(statement0);
		FREE(statement0);
		// FREE(statement3);
	}
	//STOP_TIMER("module - metadata lookup");
	return res;
}*/
char*
oracleCreateTable(char* query, char* tablename){
	char *res = tablename;
	char *st0 = CONCAT_STRINGS("DROP TABLE ", tablename);
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", tablename, " AS (",query,")");
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {

		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);

		//	FREE(statement0);

	} else {
		//  FATAL_LOG("Statement: %s failed.", statement0);
		// FREE(statement0);
		FREE(statement0);
		FREE(statement1);
	}
	//STOP_TIMER("module - metadata lookup");
	return res;
}
char *
oracleGetSamples2(char *groupbyAttr, char* groupbyAttr1_groupbyAttr2, char *psAttr, char*sampleRate) {
	char* res = CONCAT_STRINGS("SAMPLE#", groupbyAttr1_groupbyAttr2, "#PS#",psAttr,"#",sampleRate);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", res, " AS (SELECT * FROM CRIMES WHERE (",groupbyAttr,") IN (SELECT ",groupbyAttr," FROM SAMPLE#CRIMES#",psAttr,"#BID#",sampleRate,"))");
	DEBUG_LOG("st1 is %s",st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {

		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		OCI_Commit(conn);

		//	FREE(statement0);

	} else {
		//  FATAL_LOG("Statement: %s failed.", statement0);
		// FREE(statement0);
		FREE(statement0);
		FREE(statement1);
	}
	//STOP_TIMER("module - metadata lookup");
	return res;
}
/*
char *
oraclePartialSample(char *tableName, char *groupbyAttr, int num){
	char* res = CONCAT_STRINGS("PARTIAL_",tableName);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 = "";
	if (num > 2000000) {
		st1 =
				CONCAT_STRINGS(st1, "CREATE TABLE ", res,
						" AS (select * from (SELECT case when max_pos > 3 then floor(DBMS_RANDOM.value(1,max_pos)) else 0 end as ran,round(MAX_POS * 0.33), max_pos,pos,");
		st1 =
				CONCAT_STRINGS(st1,
						"L_SUPPKEY, L_ORDERKEY,L_PARTKEY, L_EXTENDEDPRICE, L_QUANTITY, L_LINENUMBER, L_DISCOUNT, L_TAX from(select max(pos) OVER(PARTITION BY ",
						groupbyAttr,
						") as max_pos,pos, L_SUPPKEY, L_ORDERKEY,L_PARTKEY, L_EXTENDEDPRICE, L_QUANTITY, L_LINENUMBER, L_DISCOUNT, L_TAX from (select row_number() OVER(PARTITION BY ",
						groupbyAttr, " ORDER BY ", groupbyAttr,
						") AS POS,L_SUPPKEY, L_ORDERKEY,L_PARTKEY, L_EXTENDEDPRICE, L_QUANTITY, L_LINENUMBER, L_DISCOUNT, L_TAX from ",
						tableName,
						")))WHERE ran <= round(MAX_POS * 0.5) or pos = round(MAX_POS * 0.3))");

	} else {
		st1 =
				CONCAT_STRINGS(st1, "CREATE TABLE ", res,
						" AS (select * from (SELECT L_SUPPKEY, L_ORDERKEY,L_PARTKEY, L_EXTENDEDPRICE, L_QUANTITY, L_LINENUMBER, L_DISCOUNT, L_TAX from ",tableName,"))");

	}

		appendStringInfo(statement0, st0);
		appendStringInfo(statement1, st1);
		if ((conn = getConnection()) != NULL) {
			DEBUG_LOG("Lzy successfully!");
			executeNonQueryStatement(statement0->data);
			executeNonQueryStatement(statement1->data);
			//OCI_Commit(conn);

			//	FREE(statement0);

		} else {
			//  FATAL_LOG("Statement: %s failed.", statement0);
			// FREE(statement0);
			FREE(statement0);
			// FREE(statement3);
		}
		//STOP_TIMER("module - metadata lookup");
		return res;

}*/
char *
oraclePartialSample(char *tableName, char *groupbyAttr, int num){
	char* res = CONCAT_STRINGS("PARTIAL#",tableName);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 = "";
	if (num > 2000000) {
		st1 =
				CONCAT_STRINGS(st1, "CREATE TABLE ", res,
						" AS (select * from (SELECT case when max_pos > 3 then floor(DBMS_RANDOM.value(1,max_pos)) else 0 end as ran,round(MAX_POS * 0.33), max_pos,pos,");
		st1 =
				CONCAT_STRINGS(st1,
						"CID,BEAT,COMMUNITY_AREA,LATITUDE, LONGITUDE,ZIP_CODES, CYEAR,DISTRICT, WARD, CBLOCK, PRIMARY_TYPE,CDESCRIPTION, LOCATION_DESCRIPTION, X_COORDINATE,Y_COORDINATE from(select max(pos) OVER(PARTITION BY ",
						groupbyAttr,
						") as max_pos,pos, CID,BEAT,COMMUNITY_AREA,LATITUDE, LONGITUDE,ZIP_CODES, CYEAR,DISTRICT, WARD, CBLOCK, PRIMARY_TYPE,CDESCRIPTION, LOCATION_DESCRIPTION, X_COORDINATE,Y_COORDINATE from (select row_number() OVER(PARTITION BY ",
						groupbyAttr, " ORDER BY ", groupbyAttr,
						") AS POS, CID,BEAT,COMMUNITY_AREA,LATITUDE, LONGITUDE,ZIP_CODES, CYEAR,DISTRICT, WARD, CBLOCK, PRIMARY_TYPE,CDESCRIPTION, LOCATION_DESCRIPTION, X_COORDINATE,Y_COORDINATE from ",
						tableName,
						")))WHERE ran <= round(MAX_POS * 0.33) or pos = round(MAX_POS * 0.3))");

	} else {
		st1 =
				CONCAT_STRINGS(st1, "CREATE TABLE ", res,
						" AS (select * from (SELECT CID,BEAT,COMMUNITY_AREA,LATITUDE, LONGITUDE,ZIP_CODES, CYEAR,DISTRICT, WARD, CBLOCK, PRIMARY_TYPE,CDESCRIPTION, LOCATION_DESCRIPTION, X_COORDINATE,Y_COORDINATE from ",tableName,"))");

	}
	DEBUG_LOG("st1 is %s",st1);

		appendStringInfo(statement0, st0);
		appendStringInfo(statement1, st1);
		if ((conn = getConnection()) != NULL) {

			executeNonQueryStatement(statement0->data);
			executeNonQueryStatement(statement1->data);
			//OCI_Commit(conn);

			//	FREE(statement0);

		} else {
			//  FATAL_LOG("Statement: %s failed.", statement0);
			// FREE(statement0);
			FREE(statement0);
			FREE(statement1);
		}
		//STOP_TIMER("module - metadata lookup");*/
		return res;

}
/*
char *
oracleGetStatPartialSample(char *partialSampleTableName, char *aggrName, char*groupbyAttr,char* count_table){
	char* res = CONCAT_STRINGS("STAT_", partialSampleTableName, "_AGG_",
			aggrName);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 =
			CONCAT_STRINGS("CREATE TABLE ", res, " AS (select ",
			groupbyAttr, ", SUM_", aggrName, "*COUNT_L/COUNT_", aggrName,
			" AS EST_SUM_", aggrName, "2, COUNT_", aggrName,
			", COUNT_L AS EST_COUNT_", aggrName, ", MIN_", aggrName, ",MAX_",
			aggrName, ",AVG_", aggrName, " FROM ");
	st1 = CONCAT_STRINGS(st1,"(select * from (select ",groupbyAttr,", SUM(",aggrName,") as SUM_",aggrName,", COUNT(*) as COUNT_",aggrName,
			", MIN(",aggrName,") as MIN_",aggrName,", MAX(",aggrName,") as MAX_",aggrName,", AVG(",aggrName,") as AVG_",aggrName," from ",
			partialSampleTableName, " group by ",groupbyAttr," order by ",groupbyAttr,") natural join ",count_table,"))");
	DEBUG_LOG("st1 is %s", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		DEBUG_LOG("Lzy successfully!");
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);

	} else {
		FREE(statement0);
	}

	return res;
}*/
char *
oracleGetStatPartialSample(char *partialSampleTableName, char *aggrName, char*groupbyAttr,char* count_table){
	char* res = CONCAT_STRINGS("STAT#", partialSampleTableName, "#AGG#",
			aggrName);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 =
			CONCAT_STRINGS("CREATE TABLE ", res, " AS (select ",
			groupbyAttr, ", SUM_", aggrName, "*COUNT_L/COUNT_", aggrName,
			" AS EST_SUM_", aggrName, "2, COUNT_", aggrName,
			", COUNT_L AS EST_COUNT_", aggrName, ", MIN_", aggrName, ",MAX_",
			aggrName, ",AVG_", aggrName, " FROM ");
	st1 = CONCAT_STRINGS(st1,"(select * from (select ",groupbyAttr,", SUM(",aggrName,") as SUM_",aggrName,", COUNT(*) as COUNT_",aggrName,
			", MIN(",aggrName,") as MIN_",aggrName,", MAX(",aggrName,") as MAX_",aggrName,", AVG(",aggrName,") as AVG_",aggrName," from ",
			partialSampleTableName, " group by ",groupbyAttr," order by ",groupbyAttr,") natural join ",count_table,"))");
	DEBUG_LOG("st1 is %s", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		DEBUG_LOG("Lzy successfully!");
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);

	} else {
		FREE(statement0);
	}

	return res;
}
/*
char *
oracleStoreSelectivty2(char * stateName, char *psAttribute, char *aggregation, char *aggregationAttribute, char *groupbyAttribute, char *tableName, char *constant ,char* res, char *query, char *SampleRate){
			//StringInfo statement0 = makeStringInfo();
			StringInfo statement1 = makeStringInfo();
			char *st1 = "";
			//char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
				char *rate = "";
				char h1[100];
				strcpy(h1, stateName);
				char *d = "#";
				char *p1 = strtok(h1, d);
				char *ps = "";
				DEBUG_LOG("Lzy is %s", p1);
				for (int i = 0; i < 10; ++i) {

					p1 = strtok(NULL, d);
					DEBUG_LOG("Lzy is %s", p1);

					if(i == 5) {
						ps = CONCAT_STRINGS(ps,p1);
					}
					if(i == 6) {
						rate = CONCAT_STRINGS(rate,p1);
					}
				}
				DEBUG_LOG("PS is %s", ps);

	if (strstr(groupbyAttribute, ps)) {
		st1 =
				CONCAT_STRINGS("INSERT INTO ", res,
						" (est_sel,original_sel,cos_value, grouby, agg_attr, PS_ATTR, AGG_NAME, SAMPLERATE) (select est_sel,original_sel, ", constant,
						" as cos_value, '", groupbyAttribute, "' as grouby, '",
						aggregationAttribute, "' as agg_attr, '", ps,
						"' as PS_ATTR , '", aggregation, "' as agg_name, ",
						rate, " as sampleRate from (");
		st1 =
				CONCAT_STRINGS(st1,
						"(select count(*)/6190639 as est_sel from (select * from ",
						tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (SELECT * FROM ",stateName," WHERE EST_SUM_", aggregationAttribute,
						"2 > ", constant, ") JOIN ",
						tableName, "#", ps, " ON (", ps,
						">=SP AND EP>", ps, ")  )order by sp))) natural join");
		st1 =
				CONCAT_STRINGS(st1,
						"(select count(*)/6190639 as original_sel from (select * from ",
						tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (",query,") JOIN ",
						tableName, "#", ps, " ON (", ps,
						">=SP AND EP>", ps, ")  )order by sp)) ");
		st1 = CONCAT_STRINGS(st1, ")))");
		DEBUG_LOG("Lzy is %s", st1);
		DEBUG_LOG("LZY IS %s,%s", groupbyAttribute, ps);
	} else {
		st1 =
				CONCAT_STRINGS("INSERT INTO ", res,
						" (est_sel,original_sel,cos_value, grouby, agg_attr, PS_ATTR, AGG_NAME, SAMPLERATE) (select est_sel,original_sel, ",
						constant, " as cos_value, '", groupbyAttribute,
						"' as grouby, '", aggregationAttribute,
						"' as agg_attr, '", ps, "' as PS_ATTR , '",
						aggregation, "' as agg_name, ", rate,
						" as sampleRate from (");
		st1 =
				CONCAT_STRINGS(st1,
						"(select count(*)/6190639 as est_sel from (select * from ",
						tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (SELECT ",
						ps,",",groupbyAttribute, " FROM CRIMES where (",
						groupbyAttribute, ") in (SELECT ", groupbyAttribute,
						" FROM (SELECT * FROM ",stateName," WHERE EST_SUM_", aggregationAttribute,
						"2 > ", constant, ") ))JOIN ", tableName, "#", ps,
						" ON (", ps, ">=SP AND EP>", ps,
						")  )order by sp))) natural join");
		st1 =
				CONCAT_STRINGS(st1,
						"(select count(*)/6190639 as original_sel from (select * from ",
						tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (SELECT ",
						ps,",",groupbyAttribute, " FROM CRIMES where (",
						groupbyAttribute, ") in (SELECT ", groupbyAttribute,
						" FROM (",query,") ))JOIN ", tableName, "#", ps,
						" ON (", ps, ">=SP AND EP>", ps,
						")  )order by sp)) )");
		st1 = CONCAT_STRINGS(st1,"))");
		DEBUG_LOG("Lzy is %s", st1);
	}
			//appendStringInfo(statement0, st0);
			//appendStringInfo(statement0, st0);
			appendStringInfo(statement1, st1);
			if ((conn = getConnection()) != NULL) {
				DEBUG_LOG("Lzy successfully!");
			//	executeNonQueryStatement(statement0->data);
				executeNonQueryStatement(statement1->data);
				OCI_Commit(conn);
				//	FREE(statement0);

			} else {
				FREE(statement1);

			}
			return res;

	return res;
 }*/
char *
oracleStoreSelectivty2(char * stateName, char *psAttribute, char *aggregation,
		char *aggregationAttribute, char *groupbyAttribute, char *tableName,
		char *constant, char* res, char *query, char *SampleRate) {
	//StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st1 = "";
	//char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
	char *rate = "";
	char h1[100];
	strcpy(h1, stateName);
	char *d = "#";
	char *p1 = strtok(h1, d);
	char *ps = "";
	DEBUG_LOG("Lzy is %s", p1);
	for (int i = 0; i < 10; ++i) {

		p1 = strtok(NULL, d);
		DEBUG_LOG("Lzy is %s", p1);

		if (i == 5) {
			ps = CONCAT_STRINGS(ps, p1);
		}
		if (i == 6) {
			rate = CONCAT_STRINGS(rate, p1);
		}
	}
	DEBUG_LOG("PS is %s", ps);

	if (strstr(groupbyAttribute, ps)) {

		char *query_est =
				CONCAT_STRINGS("select * from ", tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (SELECT * FROM ",
						stateName, " WHERE EST_SUM_", aggregationAttribute,
						"2 > ", constant, ") JOIN ", tableName, "#", ps,
						" ON (", ps, ">=SP AND EP>", ps, ")  )order by sp)");
		char *query_ori = CONCAT_STRINGS("select * from ", tableName, "#", ps,
				"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (",
				query, ") JOIN ", tableName, "#", ps, " ON (", ps,
				">=SP AND EP>", ps, ")  )order by sp) ");
		DEBUG_LOG("query_est is %s", query_est);
		DEBUG_LOG("query_ori is %s", query_ori);
		char *est_result = oracleCreateTable(query_est, "est_result");
		char *ori_result = oracleCreateTable(query_ori, "ori_result");
		int est_count = oracleGetCount(est_result);
		int ori_count = oracleGetCount(ori_result);
		double est_sel = (double) est_count / 6190639;
		double ori_sel = (double) ori_count / 6190639;
		char est_sel2[100];
		sprintf(est_sel2, "%.6f", est_sel);
		char ori_sel2[100];
		sprintf(ori_sel2, "%.6f", ori_sel);
		//oracleDropTable(est_result);
		//oracleDropTable(ori_result);
		st1 =
				CONCAT_STRINGS("INSERT INTO ", res,
						" (est_sel,original_sel,cos_value, grouby, agg_attr, PS_ATTR, AGG_NAME, SAMPLERATE) values(",
						est_sel2, ",", ori_sel2, ", ", constant, ", '",
						groupbyAttribute, "', '", aggregationAttribute, "', '",
						ps, "', '", aggregation, "', ", rate, ")");
		DEBUG_LOG("Lzy is %s", st1);
	//	DEBUG_LOG("LZY IS %s,%s", groupbyAttribute, ps);
	} else {
		char *query_est =
				CONCAT_STRINGS("select * from ", tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (SELECT ",
						ps, ",", groupbyAttribute, " FROM CRIMES NATURAL JOIN (SELECT DISTINCT ", groupbyAttribute,
						" FROM (SELECT * FROM ", stateName, " WHERE EST_SUM_",
						aggregationAttribute, "2 > ", constant, ") ))JOIN ",
						tableName, "#", ps, " ON (", ps, ">=SP AND EP>", ps,
						")  )order by sp)");
		char *query_ori =
				CONCAT_STRINGS("select * from ", tableName, "#", ps,
						"_bid natural join (SELECT DISTINCT SP FROM (SELECT SP FROM (SELECT ",
						ps, ",", groupbyAttribute, " FROM CRIMES NATURAL JOIN (SELECT DISTINCT ", groupbyAttribute,
						" FROM (", query, ") ))JOIN ", tableName, "#", ps,
						" ON (", ps, ">=SP AND EP>", ps, ")  )order by sp)");
		DEBUG_LOG("query_est is %s", query_est);
		DEBUG_LOG("query_ori is %s", query_ori);
		char *est_result = oracleCreateTable(query_est, "est_result");
		char *ori_result = oracleCreateTable(query_ori, "ori_result");
		int est_count = oracleGetCount(est_result);
		int ori_count = oracleGetCount(ori_result);
		double est_sel = (double) est_count / 6190639;
		double ori_sel = (double) ori_count / 6190639;
		char est_sel2[100];
		sprintf(est_sel2, "%.6f", est_sel);
		char ori_sel2[100];
		sprintf(ori_sel2, "%.6f", ori_sel);
		//oracleDropTable(est_result);
		//oracleDropTable(ori_result);
		st1 =
				CONCAT_STRINGS("INSERT INTO ", res,
						" (est_sel,original_sel,cos_value, grouby, agg_attr, PS_ATTR, AGG_NAME, SAMPLERATE) values(",
						est_sel2, ",", ori_sel2, ", ", constant, ", '",
						groupbyAttribute, "', '", aggregationAttribute, "', '",
						ps, "', '", aggregation, "', ", rate, ")");
		DEBUG_LOG("Lzy is %s", st1);
		//DEBUG_LOG("LZY IS %s,%s", groupbyAttribute, ps);
	}
	//appendStringInfo(statement0, st0);
	//appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		DEBUG_LOG("Lzy successfully!");
		//	executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		OCI_Commit(conn);
		//	FREE(statement0);

	} else {
		FREE(statement1);

	}
	return res;
}
int
oracleGetCount(char *tableName){
	StringInfo statement0 = makeStringInfo();
	//char *count = "";
	char *st1 = CONCAT_STRINGS("SELECT COUNT(cid) AS COUNT_L FROM ", tableName);
	appendStringInfo(statement0,st1);

		if ((conn = getConnection()) != NULL) {
			OCI_Resultset *rs = executeStatement(statement0->data);

			if (rs != NULL) {
				if (OCI_FetchNext(rs)) {

					int c = (int) OCI_GetInt(rs, 1);
					DEBUG_LOG("LZY IS %d",c);
					return c;
					//return atoi(count);
				} else {
					return 0;
				}
			}
		} else {
			DEBUG_LOG("No connection");
		}

		FREE(statement0);



	return 0;
}
char*
oracleInsertSelectivity(char *constant, char *groupbyAttr, char *agg_attr, char* ps_attr, char *agg_name, char* table, char *sampleRate){
	StringInfo statement0 = makeStringInfo();
	char *st0 = "";
	st0 = CONCAT_STRINGS("INSERT INTO ", table,
							" (est_sel,original_sel,cos_value, grouby, agg_attr, PS_ATTR, AGG_NAME, SAMPLERATE) values (1, 1, ", constant,
							", '", groupbyAttr, "', '",
							agg_attr, "', '", ps_attr,
							"', '", agg_name, "', ",
							sampleRate, ")");
	appendStringInfo(statement0, st0);
	if ((conn = getConnection()) != NULL) {
				DEBUG_LOG("Lzy successfully!");
				executeNonQueryStatement(statement0->data);
				OCI_Commit(conn);
	} else {
		//  FATAL_LOG("Statement: %s failed.", statement0);
		// FREE(statement0);
		FREE(statement0);
		// FREE(statement3);
	}
	//STOP_TIMER("module - metadata lookup");
	return table;
}



char*
oracleGetSamplesDirectly(char *attributes, char *partitionAttributes, char *sampleRate, char*tableName) {
		char* res =  CONCAT_STRINGS("SAMPLE_",tableName,"_5");
		StringInfo statement0 = makeStringInfo();
		StringInfo statement1 = makeStringInfo();
		char *st0 = CONCAT_STRINGS("DROP TABLE ", res);
		char *st1 = CONCAT_STRINGS("CREATE TABLE ",res," AS (SELECT * FROM (SELECT ",attributes,", POS, BID FROM (SELECT ran, bid, POS, max(POS) OVER (PARTITION BY ran,bid) AS maxPerBucket, ",attributes,",MAX_POS ");
		st1 = CONCAT_STRINGS(st1,"FROM (SELECT floor (DBMS_RANDOM.value(1,POS)) AS ran, POS,bid,",attributes,", MAX_POS FROM(SELECT POS,bid, ",attributes,", MAX(POS) OVER (PARTITION BY bid) AS MAX_POS FROM");
		st1 = CONCAT_STRINGS(st1," (SELECT row_number() OVER(PARTITION BY bid order by ",partitionAttributes,") AS POS, bid, ",attributes," FROM ",tableName,")))WHERE ran <= round(MAX_POS * ",sampleRate,"))WHERE POS = maxPerBucket order by bid ))");

		//appendStringInfo(statement0, st0);
		appendStringInfo(statement0, st0);
		appendStringInfo(statement1, st1);
		if ((conn = getConnection()) != NULL) {
			DEBUG_LOG("Lzy successfully!");
			executeNonQueryStatement(statement0->data);
			executeNonQueryStatement(statement1->data);
			//OCI_Commit(conn);

			//	FREE(statement0);

		} else {
			//  FATAL_LOG("Statement: %s failed.", statement0);
			// FREE(statement0);
			FREE(statement0);
			// FREE(statement3);
		}
		//STOP_TIMER("module - metadata lookup");
		return res;
}

char *
oracleFindTheMax(char *query, char *aggName){
		char *max;
		StringInfo statement0 = makeStringInfo();
		char *st0 = CONCAT_STRINGS("select max(",aggName,") from (", query,")");
		appendStringInfo(statement0, st0);

		if ((conn = getConnection()) != NULL) {
				OCI_Resultset *rs = executeStatement(statement0->data);

				if (rs != NULL) {
					if (OCI_FetchNext(rs)) {

						max = strdup((char * )OCI_GetString(rs, 1));
						DEBUG_LOG("The max value is %s", max);
						return max;
					} else {
						return NULL;
					}
				}
			} else {
				DEBUG_LOG("No connection");
			}
		return NULL;

}

/*
char *
oracleGetSampleStat(char * sampleName, char *aggrName, char *groupBy, char *sampleRate, char* count_table) {
	char *resultTable = CONCAT_STRINGS("STAT_", sampleName, "_");
	char h1[100];
	strcpy(h1, groupBy);
	char *d = ",";
	char *p1 = strtok(h1, d);
	while (p1 != NULL) {
		DEBUG_LOG("LZY IS %s", p1);
		resultTable = CONCAT_STRINGS(resultTable,p1,"_");
		p1 = strtok(NULL, d);
	}
	resultTable = CONCAT_STRINGS(resultTable,"agg_",aggrName);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo temp = makeStringInfo();
	//Operator *cond = (Operator *) ((SelectionOperator *) selection)->cond;
	char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable, " AS (select ",
			groupBy, ", SUM_", aggrName, "* ",sampleRate," AS EST_SUM_",aggrName,"2, (MIN_", aggrName, " + MAX_",
			aggrName, ")* ",sampleRate,"* COUNT_", aggrName, "/2 AS EST_SUM_", aggrName,
			",COUNT_", aggrName, ",COUNT_", aggrName, "*",sampleRate," AS EST_COUNT_",
			aggrName, ",MIN_", aggrName, ",MAX_", aggrName, ",AVG_",aggrName," FROM ");
	st1 = CONCAT_STRINGS(st1, "(select ", groupBy, ", SUM(", aggrName,
			") as SUM_", aggrName, ", COUNT(*) as COUNT_", aggrName, ", MIN(",
			aggrName, ") as MIN_", aggrName, ", MAX(", aggrName, ") as MAX_",
			aggrName, ", AVG(",aggrName,") as AVG_",aggrName," from ", sampleName, " group by ", groupBy, " order by ",groupBy,")  )");
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	//STOP_TIMER("module - metadata lookup");
	return resultTable;
}*/
char *
oracleGetSampleStat(char * sampleName, char *aggrName, char *groupBy, char *sampleRate, char* count_table) {
	/* char *resultTable = CONCAT_STRINGS("STAT_", sampleName, "_");

	char h1[100];
	strcpy(h1, groupBy);
	char *d = ",";
	char *p1 = strtok(h1, d);
	while (p1 != NULL) {
		resultTable = CONCAT_STRINGS(resultTable,p1,"_");
		p1 = strtok(NULL, d);
	}
	resultTable = CONCAT_STRINGS(resultTable,"agg_",aggrName);*/
	char *resultTable = CONCAT_STRINGS("STAT_TEST");
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	//StringInfo temp = makeStringInfo();
	//Operator *cond = (Operator *) ((SelectionOperator *) selection)->cond;
	char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
	char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable, " AS (select ",
			groupBy, ", SUM_", aggrName, "* COUNT_L/COUNT_",aggrName," AS EST_SUM_",aggrName,"2, (MIN_", aggrName, " + MAX_",
			aggrName, ")* COUNT_L/2 AS EST_SUM_", aggrName,
			",COUNT_", aggrName, ",COUNT_L AS EST_COUNT_",
			aggrName, ",MIN_", aggrName, ",MAX_", aggrName, ",AVG_",aggrName," FROM ");
	st1 = CONCAT_STRINGS(st1, "(select * from (select ", groupBy, ", SUM(", aggrName,
			") as SUM_", aggrName, ", COUNT(*) as COUNT_", aggrName, ", MIN(",
			aggrName, ") as MIN_", aggrName, ", MAX(", aggrName, ") as MAX_",
			aggrName, ", AVG(",aggrName,") as AVG_",aggrName," from ", sampleName, " group by ", groupBy, " order by ",groupBy,") NATURAL JOIN ",count_table,") )");
	DEBUG_LOG("LZY IS %s", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);
	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	//STOP_TIMER("module - metadata lookup");
	return resultTable;
}
char *
oracleStoreGroupbyCount(char *groupby, char *groupby2, char* tablename){
	char *resultTable = CONCAT_STRINGS("COUNT_", tablename, "_",groupby2);
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
		//StringInfo temp = makeStringInfo();
		//Operator *cond = (Operator *) ((SelectionOperator *) selection)->cond;
		char *st0 = CONCAT_STRINGS("DROP TABLE ", resultTable, " ");
		char *st1 = CONCAT_STRINGS("CREATE TABLE ", resultTable, " AS (SELECT COUNT(*) AS COUNT_L,",groupby," FROM ",tablename," GROUP BY ",groupby,")");
		appendStringInfo(statement0, st0);
		appendStringInfo(statement1, st1);
		if ((conn = getConnection()) != NULL) {
			executeNonQueryStatement(statement0->data);
			executeNonQueryStatement(statement1->data);
			FREE(statement0);
			FREE(statement1);

		} else {
			FATAL_LOG("Statement: %s failed.", statement0);
			FREE(statement0);
			FREE(statement1);
			// FREE(statement3);
		}
		//STOP_TIMER("module - metadata lookup");
		return resultTable;
	}




char *
oracleStoreSelectivty(char * stateName, char *psAttribute, char *aggregation, char *aggregationAttribute, char *groupbyAttribute, char *tableName, char *constant ,char* res, char *query, char *SampleRate){
			//StringInfo statement0 = makeStringInfo();
			StringInfo statement1 = makeStringInfo();
			//char *st0 = CONCAT_STRINGS("DROP TABLE ", res);

			char *st1 = CONCAT_STRINGS("INSERT INTO ", res, " (est_sel,original_sel,cos_value,grouby, agg_attr, PS_ATTR, AGG_NAME, SAMPLERATE) (select est_sel,original_sel, ",constant," as cos_value, '",groupbyAttribute,"' as grouby, '",aggregationAttribute,"' as agg_attr, '",psAttribute,"' as PS_ATTR , '",aggregation,"' as agg_name, ",SampleRate," as samplerate from (");
			if (!strcmp(aggregation, "SUM")) {
				st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 as est_sel  from (select * from( select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from( SELECT DISTINCT SP FROM ((SELECT SP,EP FROM (SELECT * FROM ",stateName, " where EST_SUM_",aggregationAttribute,"2 > ",constant,")");
			}
			if (!strcmp(aggregation, "COUNT")) {
				st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 as est_sel  from (select * from( select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from( SELECT DISTINCT SP FROM ((SELECT SP,EP FROM (SELECT * FROM ",stateName, " where EST_COUNT_",aggregationAttribute," > ",constant,")");
			}
			if (!strcmp(aggregation, "AVG")) {
				st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 as est_sel  from (select * from( select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from( SELECT DISTINCT SP FROM ((SELECT SP,EP FROM (SELECT * FROM ",stateName, " where AVG_",aggregationAttribute," > ",constant,")");
			}
			if (!strcmp(aggregation, "MAX")) {
				st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 as est_sel  from (select * from( select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from( SELECT DISTINCT SP FROM ((SELECT SP,EP FROM (SELECT * FROM ",stateName, " where MAX_",aggregationAttribute," > ",constant,")");
			}
			if (!strcmp(aggregation, "MIN")) {
				st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 as est_sel  from (select * from( select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from( SELECT DISTINCT SP FROM ((SELECT SP,EP FROM (SELECT * FROM ",stateName, " where MIN_",aggregationAttribute," > ",constant,")");
			}
			//st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 6190639 as est_sel  from (select * from( select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from( SELECT DISTINCT SP FROM ((SELECT SP,EP FROM (SELECT * FROM ",stateName, " where EST_SUM_L_PARTKEY2 > ",constant,")");
			st1 = CONCAT_STRINGS(st1,"JOIN ",tableName,"#",psAttribute," ON (",psAttribute,">=SP AND EP>",psAttribute,") ) )order by sp))) ) ) natural join ");
			st1 = CONCAT_STRINGS(st1,"(select count(*)/6001215 as original_sel from (select * from ",tableName,"#",psAttribute,"_bid where SP in (select * from((SELECT DISTINCT SP FROM((SELECT SP,EP FROM (",query,") JOIN ",tableName,"#",psAttribute," ON (",psAttribute,">=SP AND EP>",psAttribute,") ) )order by sp)))) )");
			st1 = CONCAT_STRINGS(st1,"))");
			DEBUG_LOG("Lzy is %s",st1);
			//appendStringInfo(statement0, st0);
			//appendStringInfo(statement0, st0);
			appendStringInfo(statement1, st1);
			if ((conn = getConnection()) != NULL) {
				DEBUG_LOG("Lzy successfully!");
			//	executeNonQueryStatement(statement0->data);
				executeNonQueryStatement(statement1->data);
				OCI_Commit(conn);
				//	FREE(statement0);

			} else {
				//  FATAL_LOG("Statement: %s failed.", statement0);
				// FREE(statement0);
				FREE(statement1);
				// FREE(statement3);
			}
			//STOP_TIMER("module - metadata lookup");
			return res;

	return res;
}

void oracleGetPartitionSizes(char* tableName, char* attr, char* partition[],
		char* size[], int length) {

	for (int i = 0; i < length - 1; i++) {
		char *rowNum = "";
		//StringInfo statement0 = makeStringInfo();
		StringInfo statement1 = makeStringInfo();
		char *st1 = CONCAT_STRINGS("SELECT COUNT(*) FROM (SELECT * FROM ",
				tableName, " WHERE ", partition[i], "<= ", attr, " and ", attr,
				" < ", partition[i + 1], ")");
		DEBUG_LOG("LZY is %s", st1);
		appendStringInfo(statement1, st1);
		if ((conn = getConnection()) != NULL) {
			OCI_Resultset *rs = executeStatement(statement1->data);

			if (rs != NULL) {
				if (OCI_FetchNext(rs)) {
					rowNum = strdup((char * )OCI_GetString(rs, 1));
				} else {
					DEBUG_LOG("No connection");
				}
			}
		} else {
			DEBUG_LOG("No connection");
		}
		FREE(statement1);
		//DEBUG_LOG("Lzy is %s", rowNum);
		size[i] = rowNum;
		//DEBUG_LOG("Lzy is %s", size[i]);
	}

}

char*
oracleStorePartitionSizes(char* tableName, char* attr, char* partition[], int length) {
	char *res = CONCAT_STRINGS(tableName, "_U_UPVOTES_BUCKET_SIZES ");
	StringInfo statement0 = makeStringInfo();
	StringInfo statement1 = makeStringInfo();
	char *st0 =  CONCAT_STRINGS("DROP TABLE ", res);
	char *st1 =  CONCAT_STRINGS("CREATE TABLE ", res," AS (SELECT * FROM (");
	for (int i = 0; i < length - 2; i++) {
		char r[100];
		sprintf(r,"%d",i);
		st1 = CONCAT_STRINGS(st1, "(SELECT COUNT(*) AS BUCKET_SIZE, ",r," as BUCKET_NUM FROM (SELECT * FROM ",
				tableName, " WHERE ", partition[i], "<= ", attr, " and ", attr,
				" < ", partition[i + 1], ") )UNION ALL");

	}
	char s[100];
	sprintf(s,"%d",length - 2);
	st1 = CONCAT_STRINGS(st1, "(SELECT COUNT(*) AS BUCKET_SIZE, ",s," as BUCKET_NUM FROM (SELECT * FROM ",
					tableName, " WHERE ", partition[length - 2], "<= ", attr, " and ", attr,
					" < ", partition[length - 1], ") )");
	st1 =  CONCAT_STRINGS(st1," ))");
	DEBUG_LOG("LZY is %s", st1);
	appendStringInfo(statement0, st0);
	appendStringInfo(statement1, st1);

	if ((conn = getConnection()) != NULL) {
		executeNonQueryStatement(statement0->data);
		executeNonQueryStatement(statement1->data);
		DEBUG_LOG("projection successfully!");
		FREE(statement0);
		FREE(statement1);

	} else {
		FATAL_LOG("Statement: %s failed.", statement0);
		FREE(statement0);
		FREE(statement1);
		// FREE(statement3);
	}
	return res;

}
char *
oracleGetPartitionSizes2(char *tableName, char*num){
		StringInfo statement1 = makeStringInfo();
		char *result = "";
			char *st1 = CONCAT_STRINGS("SELECT * FROM ",
					tableName, " WHERE BUCKET_NUM =",num);
			DEBUG_LOG("LZY is %s", st1);
			appendStringInfo(statement1, st1);
			if ((conn = getConnection()) != NULL) {
				OCI_Resultset *rs = executeStatement(statement1->data);

				if (rs != NULL) {
					if (OCI_FetchNext(rs)) {
						result = strdup((char * )OCI_GetString(rs, 1));
					} else {
						DEBUG_LOG("No connection");
					}
				}
			} else {
				DEBUG_LOG("No connection");
			}
			FREE(statement1);
	return result;
}
char*
oracleDropTable(char *table) {


		StringInfo statement0 = makeStringInfo();

			char *st0 = CONCAT_STRINGS("DROP TABLE ",table);
			appendStringInfo(statement0, st0);

			if ((conn = getConnection()) != NULL) {
				executeNonQueryStatement(statement0->data);

				//OCI_Commit(conn);
				DEBUG_LOG("projection successfully!");
				FREE(statement0);

			} else {
				FATAL_LOG("Statement: %s failed.", statement0);
				FREE(statement0);
				// FREE(statement3);
			}


	return NULL;
}
/*
 List*
 oracleComputeSumFromHist(char *tableName, char *attrName, char *sumAttrName){

 //select zip_codes, ward, LAG(ward,1) OVER (ORDER BY SP,pos) AS lag_ward, POS,CNT,SP,
 //LAG(SP,1) OVER (ORDER BY SP,pos) as LAG_SP,EP,TUPLENUM,dis_zip_codes,DIS_WARD
 //FROM CRIMES_ZIP_CODES_BEAT_WARD_DISTRICT_HIST
 //List *res = NIL;

 StringInfo statement0 = makeStringInfo();
 StringInfo statement1 = makeStringInfo();
 char *st0 = CONCAT_STRINGS("DROP TABLE SUM#", tableName, " ");
 char *st1 = CONCAT_STRINGS("CREATE TABLE SUM#", tableName, " AS ( SELECT * FROM ( WITH VIEW1 AS(");
 st1 = CONCAT_STRINGS(st1, "SELECT SUM(SUM_", sumAttrName,
 ") AS ESTIAMTE_SUM_", sumAttrName, ", SP FROM(");
 st1 = CONCAT_STRINGS(st1, "SELECT ", attrName, ", ", sumAttrName,
 ", CASE WHEN SP = LAG_SP THEN (", sumAttrName, "+LAG_", sumAttrName,
 ")*TUPLENUM/2 ELSE (", sumAttrName, "+1)*TUPLENUM/2 END AS SUM_",
 sumAttrName, ", SP, TUPLENUM FROM (");
 st1 =
 CONCAT_STRINGS(st1, "SELECT ", attrName, ", ", sumAttrName,
 ", CASE WHEN LAG_", sumAttrName,
 " IS NULL THEN 1 ELSE LAG_", sumAttrName, " END AS LAG_",
 sumAttrName,
 ", SP, CASE WHEN LAG_SP IS NULL THEN SP ELSE LAG_SP END AS LAG_SP, EP, TUPLENUM FROM (");
 st1 = CONCAT_STRINGS(st1, "SELECT ", attrName, ", ", sumAttrName, ", LAG(",
 sumAttrName, ",1) OVER (ORDER BY SP, POS) AS LAG_", sumAttrName,
 ", SP, LAG(SP,1) OVER (ORDER BY SP, POS) AS LAG_SP, EP, TUPLENUM ");
 st1 = CONCAT_STRINGS(st1, "FROM ", tableName, ")))GROUP BY SP), ");
 st1 = CONCAT_STRINGS(st1,"VIEW2 AS (", computeNumOfGroupFromHist(tableName, attrName, sumAttrName),")");
 st1 = CONCAT_STRINGS(st1,"SELECT ESTIAMTE_SUM_",sumAttrName,", ESTIAMTE_SUM_",sumAttrName,"/AVGGROUPS AS ESTIAMTEDSUM, SP, AVGGROUPS FROM (SELECT ESTIAMTE_SUM_",sumAttrName,", VIEW1.SP AS SP, AVGGROUPS FROM VIEW1 JOIN VIEW2 ON VIEW1.SP = VIEW2.SP) ORDER BY SP)) ");
 DEBUG_LOG("lzy are %s:", st1);
 DEBUG_LOG("lzy are %s:", st1);
 appendStringInfo(statement0, st0);
 appendStringInfo(statement1, st1);
 if ((conn = getConnection()) != NULL) {
 executeNonQueryStatement(statement0->data);
 executeNonQueryStatement(statement1->data);
 DEBUG_LOG("compute successfully!");
 FREE(statement0);
 FREE(statement1);

 } else {
 FATAL_LOG("Statement: %s failed.", statement0);
 FREE(statement0);
 FREE(statement1);
 // FREE(statement3);
 }
 STOP_TIMER("module - metadata lookup");
 return NIL;
 }
 */
char*
computeNumOfGroupFromHist(char *tableName, char *attrName, char *sumAttrName) {
	char *st = "";
	List *res = getAttributeFromHist(tableName,
			CONCAT_STRINGS("CRIMES#", attrName, "#", sumAttrName, "#HIST"));
	Set *attrset = (Set *) getHeadOfList(res)->data.ptr_value;
	if (!EMPTY_SET(attrset)) {
		addToSet(attrset, createConstString(attrName));
		Set *check = NODESET();
		FOREACH_SET(Constant,n,attrset)
		{
			addToSet(check, n);
		}
		st =
				CONCAT_STRINGS(st,
						"SELECT SP, CNT, (MAXGROUPS+MINGROUPS)/2 AS AVGGROUPS FROM ( SELECT DISTINCT SP, CNT, ");
		char *st1 = "";
		FOREACH_SET(Constant,n,attrset)
		{
			st = CONCAT_STRINGS(st, "DIS_", n->value, ", ");
			if (FOREACH_SET_HAS_NEXT(n)) {
				st1 = CONCAT_STRINGS(st1, "DIS_", n->value, "*");
			} else {
				st1 = CONCAT_STRINGS(st1, "DIS_", n->value, " AS MAXGROUPS, ");
			}

		}

		st = CONCAT_STRINGS(st, st1, "CASE ");
		FOREACH_SET(Constant,n,attrset)
		{
			st = CONCAT_STRINGS(st, "WHEN ");
			char *t = "";
			removeSetElem(check, n);
			FOREACH_SET(Constant,m,check)
			{
				if (FOREACH_SET_HAS_NEXT(m)) {
					t = CONCAT_STRINGS(t, "DIS_", n->value, ">= DIS_", m->value,
							" AND ");
				} else {
					t = CONCAT_STRINGS(t, "DIS_", n->value, ">= DIS_", m->value,
							" ");
				}
				DEBUG_LOG("lzy are %s", t);
			}
			st = CONCAT_STRINGS(st, t, "THEN DIS_", n->value, " ");
			addToSet(check, n);
		}
		st = CONCAT_STRINGS(st, "END AS MINGROUPS FROM ", tableName,
				") ORDER BY SP");
		DEBUG_LOG("lzy are %s", st);
	} else {
		st = CONCAT_STRINGS(st, "SELECT DISTINCT SP, CNT, DIS_", attrName,
				" AS AVGGROUPS FROM ", tableName, " ORDER BY SP");
		DEBUG_LOG("lzy are %s", st);
	}
	return st;
}
List *
oracleGetAttributeNames(char *tableName) {
	List *attrNames = NIL;
	List *attrs = oracleGetAttributes(tableName);
	//TODO use attribute defition instead
	FOREACH(AttributeDef,a,attrs)
		attrNames = appendToTailOfList(attrNames, a->attrName);

	return attrNames;
}

Node *
oracleGetAttributeDefaultVal(char *schema, char *tableName, char *attrName) {
	StringInfo statement = makeStringInfo();

	ACQUIRE_MEM_CONTEXT(context);
	START_TIMER("module - metadata lookup");

	DEBUG_LOG("Get default for %s.%s.%s", schema, tableName, attrName);

	// run query to fetch default value for attribute if it exists and return it
	appendStringInfo(statement, "SELECT DATA_DEFAULT "
			"FROM SYS.DBA_TAB_COLS_V$ "
			"WHERE TABLE_NAME = '%s' "
			"AND OWNER = '%s' "
			"AND COLUMN_NAME = '%s'", tableName, schema, attrName);
	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);
		char *defaultExpr = NULL;
		Node *result = NULL;

		// loop through
		while (OCI_FetchNext(rs)) {
			defaultExpr = (char *) OCI_GetString(rs, 1);

			DEBUG_LOG("default expr for %s.%s.%s is <%s>", schema ? schema : "",
					tableName, attrName, defaultExpr);
		}

		DEBUG_LOG("Statement: %s executed successfully.", statement->data);
		FREE(statement);

		// parse expression
		if (defaultExpr != NULL)
			result = parseFromString(defaultExpr);

		STOP_TIMER("module - metadata lookup");
		RELEASE_MEM_CONTEXT_AND_RETURN_COPY(Node, result);
	} else {
		FATAL_LOG("Statement: %s failed.", statement);
		FREE(statement);
	}
	STOP_TIMER("module - metadata lookup");

	// return NULL if no default and return to callers memory context
	RELEASE_MEM_CONTEXT_AND_RETURN_COPY(Node, NULL);
}

List*
oracleGetAttributes(char *tableName) {
	List *attrList = NIL;

	ACQUIRE_MEM_CONTEXT(context);

	START_TIMER("module - metadata lookup");

	if (tableName == NULL) {
		STOP_TIMER("module - metadata lookup");
		RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
	}
	if ((attrList = searchTableBuffers(tableName)) != NIL) {
		RELEASE_MEM_CONTEXT();
		STOP_TIMER("module - metadata lookup");
		return copyObject(attrList);
	}
	//TODO use query SELECT c.COLUMN_NAME, c.DATA_TYPE FROM DBA_TAB_COLUMNS c WHERE OWNER='FGA_USER' AND TABLE_NAME = 'R' ORDER BY COLUMN_ID;
	// how to figure out schema
	if (conn == NULL)
		initConnection();
	if (isConnected()) {
		int i, n;
		tInfo = OCI_TypeInfoGet(conn, tableName, OCI_TIF_TABLE);
		n = OCI_TypeInfoGetColumnCount(tInfo);

		for (i = 1; i <= n; i++) {
			OCI_Column *col = OCI_TypeInfoGetColumn(tInfo, i);

			AttributeDef *a = createAttributeDef(
					strdup((char *) OCI_GetColumnName(col)),
					ociTypeToDT(OCI_GetColumnType(col)));
			attrList = appendToTailOfList(attrList, a);
		}

		//add to table buffer list as cache to improve performance
		//user do not have to free the attrList by themselves
		addToTableBuffers(tableName, attrList);

		RELEASE_MEM_CONTEXT();
		STOP_TIMER("module - metadata lookup");
		return copyObject(attrList); //TODO copying
	}

	FATAL_LOG("Not connected to database.");
	STOP_TIMER("module - metadata lookup");
	RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
}

static DataType ociTypeToDT(unsigned int typ) {
	/* - OCI_CDT_NUMERIC     : short, int, long long, float, double
	 * - OCI_CDT_DATETIME    : OCI_Date *
	 * - OCI_CDT_TEXT        : dtext *
	 * - OCI_CDT_LONG        : OCI_Long *
	 * - OCI_CDT_CURSOR      : OCI_Statement *
	 * - OCI_CDT_LOB         : OCI_Lob  *
	 * - OCI_CDT_FILE        : OCI_File *
	 * - OCI_CDT_TIMESTAMP   : OCI_Timestamp *
	 * - OCI_CDT_INTERVAL    : OCI_Interval *
	 * - OCI_CDT_RAW         : void *
	 * - OCI_CDT_OBJECT      : OCI_Object *
	 * - OCI_CDT_COLLECTION  : OCI_Coll *
	 * - OCI_CDT_REF         : OCI_Ref *
	 */
	switch (typ) {
	case OCI_CDT_NUMERIC:
		return DT_INT;
	case OCI_CDT_TEXT:
		return DT_STRING;
		//TODO distinguish between int and float
	}

	return DT_STRING;
}

boolean oracleIsAgg(char* functionName) {
	if (functionName == NULL)
		return FALSE;

	for (int i = 0; i < AGG_FUNCTION_COUNT; i++) {
		if (strcasecmp(aggList[i], functionName) == 0)
			return TRUE;
	}
	return FALSE;
}

boolean oracleIsWindowFunction(char *functionName) {
	if (functionName == NULL)
		return FALSE;

	for (int i = 0; i < WINF_FUNCTION_COUNT; i++) {
		if (strcasecmp(winfList[i], functionName) == 0)
			return TRUE;
	}

	return FALSE;
}

char *
oracleGetTableDefinition(char *tableName) {
	StringInfo statement;
	char *result;

	ACQUIRE_MEM_CONTEXT(context);

	START_TIMER("module - metadata lookup");

	statement = makeStringInfo();
	appendStringInfo(statement, "select DBMS_METADATA.GET_DDL('TABLE', '%s\')"
			" from DUAL", tableName);

	OCI_Resultset *rs = executeStatement(statement->data);
	if (rs != NULL) {
		if (OCI_FetchNext(rs)) {
			FREE(statement);
			result = strdup((char * )OCI_GetString(rs, 1));
			STOP_TIMER("module - metadata lookup");
			RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
		}
	}
	FREE(statement);
	STOP_TIMER("module - metadata lookup");
	RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(NULL);
}

void oracleGetTransactionSQLAndSCNs(char *xid, List **scns, List **sqls,
		List **sqlBinds, IsolationLevel *iso, Constant *commitScn) {
	START_TIMER("module - metadata lookup");

	if (xid != NULL) {
		StringInfo statement;
		statement = makeStringInfo();
		char *auditTable = strToUpper(
				getStringOption("backendOpts.oracle.logtable"));
		*scns = NIL;
		*sqls = NIL;
		*sqlBinds = NIL;

		// FETCH statements, SCNs, and parameter bindings
		if (streq(auditTable, "FGA_LOG$")) {
			appendStringInfo(statement,
					"SELECT SCN, "
							" CASE WHEN DBMS_LOB.GETLENGTH(lsqltext) > 4000 THEN lsqltext ELSE NULL END AS lsql,"
							" CASE WHEN DBMS_LOB.GETLENGTH(lsqltext) <= 4000 THEN DBMS_LOB.SUBSTR(lsqltext,4000)  ELSE NULL END AS shortsql,"
							" CASE WHEN DBMS_LOB.GETLENGTH(lsqlbind) > 4000 THEN lsqlbind ELSE NULL END AS lbind,"
							" CASE WHEN DBMS_LOB.GETLENGTH(lsqlbind) <= 4000 THEN DBMS_LOB.SUBSTR(lsqlbind,4000)  ELSE NULL END AS shortbind"
							" FROM "
							"(SELECT SCN, LSQLTEXT, LSQLBIND, ntimestamp#, "
							"   DENSE_RANK() OVER (PARTITION BY statement ORDER BY policyname) AS rnum "
							"      FROM SYS.fga_log$ "
							"      WHERE xid = HEXTORAW('%s')) x "
							"WHERE rnum = 1 "
							"ORDER BY ntimestamp#", xid);
		} else if (streq(auditTable, "UNIFIED_AUDIT_TRAIL")) {
//            appendStringInfo(statement, "SELECT SCN, \n"
//                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) > 4000 THEN SQL_TEXT ELSE NULL END AS lsql,\n"
//                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_TEXT,4000)  ELSE NULL END AS shortsql,\n"
//                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) > 4000 THEN SQL_BINDS ELSE NULL END AS lbind,\n"
//                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_BINDS,4000)  ELSE NULL END AS shortbind\n"
//                    "FROM \n"
//                    "\t(SELECT SCN, SQL_TEXT, SQL_BINDS, ENTRY_ID\n"
//                    "\tFROM SYS.UNIFIED_AUDIT_TRAIL \n"
//                    "\tWHERE TRANSACTION_ID = HEXTORAW(\'%s\')) x \n"
//                    "ORDER BY ENTRY_ID", xid);
			appendStringInfo(statement, ORACLE_SQL_GET_AUDIT_FOR_TRANSACTION,
					"SYS.UNIFIED_AUDIT_TRAIL", xid);
		} else {
//            appendStringInfo(statement, "SELECT SCN, \n"
//                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) > 4000 THEN SQL_TEXT ELSE NULL END AS lsql,\n"
//                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_TEXT,4000)  ELSE NULL END AS shortsql,\n"
//                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) > 4000 THEN SQL_BINDS ELSE NULL END AS lbind,\n"
//                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_BINDS,4000)  ELSE NULL END AS shortbind\n"
//                                "FROM \n"
//                                "\t(SELECT SCN, SQL_TEXT, SQL_BINDS, ENTRY_ID\n"
//                                "\tFROM %s \n"
//                                "\tWHERE TRANSACTION_ID = HEXTORAW(\'%s\')) x \n"
//                                "ORDER BY ENTRY_ID",
//                                auditTable,
//                                xid);
			appendStringInfo(statement, ORACLE_SQL_GET_AUDIT_FOR_TRANSACTION,
					auditTable, xid);
		}

		if ((conn = getConnection()) != NULL) {
			OCI_Resultset *rs = executeStatement(statement->data);

			// loop through
			while (OCI_FetchNext(rs)) {
				START_TIMER(
						"module - metadata lookup - fetch transaction info");

				START_TIMER(
						"module - metadata lookup - fetch transaction info - fetch SCN");
				gprom_long_t scn = (gprom_long_t) OCI_GetBigInt(rs, 1); // SCN
				STOP_TIMER(
						"module - metadata lookup - fetch transaction info - fetch SCN");

				START_TIMER(
						"module - metadata lookup - fetch transaction info - fetch SQL");
				const char *sql;
				if (OCI_IsNull(rs, 2))
					sql = OCI_GetString(rs, 3);
				else
					sql = OCI_GetString(rs, 2);
				STOP_TIMER(
						"module - metadata lookup - fetch transaction info - fetch SQL");

				START_TIMER(
						"module - metadata lookup - fetch transaction info - fetch bind");
				const char *bind; // SQLBIND
				if (OCI_IsNull(rs, 4))
					bind = OCI_GetString(rs, 5);
				else
					bind = OCI_GetString(rs, 4);
				STOP_TIMER(
						"module - metadata lookup - fetch transaction info - fetch bind");

				START_TIMER(
						"module - metadata lookup - fetch transaction info - concat strings");
				char *sqlPlusSemicolon = CONCAT_STRINGS(sql, ";");
				STOP_TIMER(
						"module - metadata lookup - fetch transaction info - concat strings");

				START_TIMER(
						"module - metadata lookup - fetch transaction info - append");
				*sqls = appendToTailOfList(*sqls, sqlPlusSemicolon);
				*scns = appendToTailOfList(*scns, createConstLong(scn));
				*sqlBinds = appendToTailOfList(*sqlBinds,
						strdup((char * ) bind));
				DEBUG_LOG(
						"Current statement at SCN %u\n was:\n%s\nwithBinds:%s",
						scn, sql, bind);
				STOP_TIMER(
						"module - metadata lookup - fetch transaction info - append");

				STOP_TIMER("module - metadata lookup - fetch transaction info");
			}

			DEBUG_LOG("Statement: %s executed successfully.", statement->data);
			DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
			FREE(statement);
		} else {
			ERROR_LOG("Statement: %s failed.", statement);
			FREE(statement);
			STOP_TIMER("module - metadata lookup");
			return;
		}

		INFO_LOG("transaction statements are:\n\n%s",
				beatify(stringListToString(*sqls)));

		// infer isolation level
		statement = makeStringInfo();
		if (streq(auditTable, "FGA_LOG$")) {
			appendStringInfo(statement, "SELECT "
					"CASE WHEN (count(DISTINCT scn) > 1) "
					"THEN 1 "
					"ELSE 0 "
					"END AS readCommmit\n"
					"FROM SYS.fga_log$\n"
					"WHERE xid = HEXTORAW(\'%s\')", xid);
		} else if (streq(auditTable, "UNIFIED_AUDIT_TRAIL")) {
			appendStringInfo(statement, "SELECT "
					"CASE WHEN (count(DISTINCT scn) > 1) "
					"THEN 1 "
					"ELSE 0 "
					"END AS readCommmit\n"
					"FROM SYS.UNIFIED_AUDIT_TRAIL\n"
					"WHERE TRANSACTION_ID = HEXTORAW(\'%s\')", xid);
		} else {
			appendStringInfo(statement, "SELECT "
					"CASE WHEN (count(DISTINCT scn) > 1) "
					"THEN 1 "
					"ELSE 0 "
					"END AS readCommmit\n"
					"FROM %s\n"
					"WHERE TRANSACTION_ID = HEXTORAW(\'%s\')", auditTable, xid);
		}

		if ((conn = getConnection()) != NULL) {
			START_TIMER("module - metadata lookup - get isolation level");

			OCI_Resultset *rs = executeStatement(statement->data);

			// loop through
			while (OCI_FetchNext(rs)) {
				gprom_long_t isoA = (gprom_long_t) OCI_GetBigInt(rs, 1); // ISOLEVEL

				switch (isoA) {
				case 0:
					*iso = ISOLATION_SERIALIZABLE;
					break;
				case 1:
					*iso = ISOLATION_READ_COMMITTED;
					break;
				default:
					*iso = ISOLATION_READ_ONLY;
					break;
				}
				DEBUG_LOG("Transaction ISOLEVEL %u", iso);
			}

			DEBUG_LOG("Statement: %s executed successfully.", statement->data);
			DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
			FREE(statement);

			STOP_TIMER("module - metadata lookup - get isolation level");
		} else {
			FATAL_LOG("Statement: %s failed.", statement);
			FREE(statement);
		}

		// get COMMIT SCN
		gprom_long_t commitS = -1; // getCommitScn("",
//                LONG_VALUE(getTailOfListP(*scns)),
//                xid); // LONG_VALUE(getTailOfListP(*scns)) + 1;
		(*((gprom_long_t *) commitScn->value)) = commitS; //TODO write query to get real COMMIT SCN
	}
	STOP_TIMER("module - metadata lookup");
}

gprom_long_t oracleGetCommitScn(char *tableName, gprom_long_t maxScn, char *xid) {
	StringInfo statement = makeStringInfo();
	gprom_long_t commitScn = 0;
	const char *histTable = NULL;

	START_TIMER("module - metadata lookup");
	START_TIMER("module - metadata lookup - get commit SCN");

	// get history table name
	histTable = getHistoryTableName(tableName,
			strToUpper(getStringOption("connection.user")));

	// run query to get commit SCN
	appendStringInfo(statement, "SELECT DISTINCT STARTSCN \n"
			"FROM (SELECT STARTSCN \n"
			"    FROM SYS_FBA_HIST_%s\n"
			"    WHERE XID = HEXTORAW('%s')\n"
			"    UNION ALL \n"
			"    SELECT STARTSCN\n"
			"    FROM SYS_FBA_TCRV_%s\n"
			"    WHERE XID = HEXTORAW('%s')) sub", histTable, xid, histTable,
			xid);

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		// loop through
		while (OCI_FetchNext(rs))
			commitScn = (gprom_long_t) OCI_GetBigInt(rs, 1);

		if (commitScn == 0) {
			FREE(statement);
			return INVALID_SCN;
		}

		DEBUG_LOG("statement %s \n\nfinished and returned commit SCN %u",
				statement->data, maxScn);

		FREE(statement);
	} else {
		FATAL_LOG("statement %s execution failed", statement->data);
		FREE(statement);
	}

	STOP_TIMER("module - metadata lookup - get commit SCN");
	STOP_TIMER("module - metadata lookup");

	return commitScn;
}

static char *
getHistoryTableName(char *tableName, char *ownerName) {

	StringInfo statement = makeStringInfo();
	char *histName = NULL;

	START_TIMER("module - metadata lookup - get history table name");

	// get history table name
	appendStringInfo(statement, "SELECT ARCHIVE_TABLE_NAME "
			"FROM SYS.dba_flashback_archive_tables "
			"WHERE TABLE_NAME = '%s' AND OWNER_NAME = '%s'", tableName,
			ownerName);

	if ((conn = getConnection()) != NULL) {
		OCI_Resultset *rs = executeStatement(statement->data);

		// loop through
		while (OCI_FetchNext(rs))
			histName = (char *) OCI_GetString(rs, 1);

		ASSERT(histName != NULL);

		DEBUG_LOG(
				"statement %s \n\nfinished and returned history table name %s",
				statement->data, histName);

		FREE(statement);
	} else {
		FATAL_LOG("statement %s execution failed", statement->data);
		FREE(statement);
	}

	histName = getMatchingSubstring(histName, "[_]([0-9]+)$");

	STOP_TIMER("module - metadata lookup - get history table name");

	return histName;
}

char *
oracleGetViewDefinition(char *viewName) {
	char *def = NULL;
	StringInfo statement;

	START_TIMER("module - metadata lookup");

	ACQUIRE_MEM_CONTEXT(context);

	if ((def = searchViewBuffers(viewName)) != NULL) {
		RELEASE_MEM_CONTEXT();
		STOP_TIMER("module - metadata lookup");
		return def;
	}

	statement = makeStringInfo();
	appendStringInfo(statement, "select text from user_views where "
			"view_name = '%s'", viewName);

	OCI_Resultset *rs = executeStatement(statement->data);
	if (rs != NULL) {
		if (OCI_FetchNext(rs)) {
			char *def = strdup((char * ) OCI_GetString(rs, 1));
			//add view definition to view buffers to improve performance
			//user should not free def by themselves
			addToViewBuffers(viewName, def);
			FREE(statement);
			RELEASE_MEM_CONTEXT();
			return def;
		}
	}
	FREE(statement);

	STOP_TIMER("module - metadata lookup");
	RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(NULL);
}

DataType oracleGetOpReturnType(char *oName, List *dataTypes, boolean *opExists) {
	*opExists = TRUE;

	if (streq(oName,
			"+") || streq(oName, "*") || streq(oName, "-") || streq(oName, "/")) {
		if (LIST_LENGTH(dataTypes) == 2) {
			DataType lType = getNthOfListInt(dataTypes, 0);
			DataType rType = getNthOfListInt(dataTypes, 1);

			if (lType == rType) {
				if (lType == DT_INT || lType == DT_FLOAT || lType == DT_LONG)
					return lType;
			}
			return lcaType(lType, rType);
		}
	}

	if (streq(oName, "||")) {
		DataType lType = getNthOfListInt(dataTypes, 0);
		DataType rType = getNthOfListInt(dataTypes, 1);

		if (lType == rType && lType == DT_STRING)
			return DT_STRING;
	}

	oName = strToUpper(oName);
	if (streq(oName, "LIKE")) {
//        DataType lDt = getNthOfListInt(dataTypes, 0);
//        DataType rDt = getNthOfListInt(dataTypes, 1);
//
//        if(lDt == rDt && lDt == DT_STRING)
		return DT_BOOL;
	}

	//TODO more operators
	*opExists = FALSE;

	return DT_STRING;
}

DataType oracleGetFuncReturnType(char *fName, List *dataTypes,
boolean *funcExists) {
	*funcExists = TRUE;
	char *capName = strToUpper(fName);

	// uncertainty dummy functions
	if (streq(capName, "UNCERT")) {
		ASSERT(LIST_LENGTH(dataTypes) == 1);
		DataType argType = getNthOfListInt(dataTypes, 0);
		return argType;
	}

	// aggregation functions
	if (streq(capName,"SUM") || streq(capName, "MIN") || streq(capName, "MAX")) {
		ASSERT(LIST_LENGTH(dataTypes) == 1);
		DataType argType = getNthOfListInt(dataTypes, 0);

		switch (argType) {
		case DT_INT:
			if (streq(capName, "SUM"))
				return DT_LONG;
			else
				return DT_INT;
		case DT_LONG:
			return DT_LONG;
		case DT_FLOAT:
			return DT_FLOAT;
		default:
			return DT_STRING;
		}
	}

	if (streq(capName, "AVG")) {
		if (LIST_LENGTH(dataTypes) == 1) {
			DataType argType = getNthOfListInt(dataTypes, 0);

			switch (argType) {
			case DT_INT:
			case DT_LONG:
			case DT_FLOAT:
				return DT_FLOAT;
			default:
				return DT_STRING;
			}
		}
	}

	if (streq(capName, "COUNT"))
		return DT_LONG;

	if (streq(capName, "XMLAGG"))
		return DT_STRING;

	if (streq(fName, "ROW_NUMBER"))
		return DT_INT;
	if (streq(fName, "DENSE_RANK"))
		return DT_INT;

	if (streq(capName, "CEIL")) {
		if (LIST_LENGTH(dataTypes) == 1) {
			DataType argType = getNthOfListInt(dataTypes, 0);
			switch (argType) {
			case DT_INT:
				return DT_INT;
			case DT_LONG:
			case DT_FLOAT:
				return DT_LONG;
			default:
				;
			}
		}
	}

	if (streq(capName, "ROUND")) {
		if (LIST_LENGTH(dataTypes) == 2) {
			DataType argType = getNthOfListInt(dataTypes, 0);
			DataType parType = getNthOfListInt(dataTypes, 1);

			if (parType == DT_INT) {
				switch (argType) {
				case DT_INT:
					return DT_INT;
				case DT_LONG:
				case DT_FLOAT:
					return DT_LONG;
				default:
					;
				}
			}
		}
	}

	if (streq(capName, "GREATEST") || streq(capName, "LEAST")
	|| streq(capName, "COALESCE") || streq(capName, "LEAD")
	|| streq(capName, "LAG") || streq(capName, "FIRST_VALUE")
	|| streq(capName, "LAST_VALUE")) {
		DataType dt = getNthOfListInt(dataTypes, 0);

		FOREACH_INT(argDT, dataTypes)
		{
			dt = lcaType(dt, argDT);
		}

		return dt;
	}
	//TODO

	*funcExists = FALSE;

	return DT_STRING;
}

long getBarrierScn(void) {
	long barrier = -1;
	StringInfo statement;

	START_TIMER("module - metadata lookup");

	ACQUIRE_MEM_CONTEXT(context);

	statement = makeStringInfo();
	appendStringInfo(statement,
			"SELECT BARRIERSCN FROM SYS.SYS_FBA_BARRIERSCN;");

	OCI_Resultset *rs = executeStatement(statement->data);
	if (rs != NULL) {
		if (OCI_FetchNext(rs)) {
			barrier = (long) OCI_GetBigInt(rs, 1);
			FREE(statement);
			RELEASE_MEM_CONTEXT();
			STOP_TIMER("module - metadata lookup");
			return barrier;
		}
	}
	FREE(statement);

	RELEASE_MEM_CONTEXT();
	STOP_TIMER("module - metadata lookup");
	return barrier;
}

int oracleGetCostEstimation(char *query) {
	/* Remove the newline characters from the Query */
//    int len = strlen(query);
//    int i = 0;
//    for (i = 0; i < len; i++)
//    {
//        if (query[i] == '\n' || query[i] == ';')
//            query[i] = ' ';
//    }
	unsigned long long int cost = 0L;

	StringInfo statement;
	statement = makeStringInfo();
	appendStringInfo(statement, "EXPLAIN PLAN FOR %s", query);
	replaceStringInfoChar(statement, '\n', ' ');
	replaceStringInfoChar(statement, ';', ' ');

	ERROR_LOG("cost query %s", statement->data);

	executeStatement(statement->data);
	FREE(statement);

	StringInfo statement1;
	statement1 = makeStringInfo();
	appendStringInfo(statement1, "SELECT MAX(COST) FROM PLAN_TABLE");

	OCI_Resultset *rs1 = executeStatement(statement1->data);
	if (rs1 != NULL) {
		while (OCI_FetchNext(rs1)) {
			cost = (unsigned long long int) OCI_GetUnsignedBigInt(rs1, 1);
			DEBUG_LOG("Cost is : %u \n", cost);
			break;
		}
	} else
		FATAL_LOG("cost estimation failed for %s", statement1->data);
	FREE(statement1);

	StringInfo statement2;
	statement2 = makeStringInfo();
	appendStringInfo(statement2, "DELETE FROM PLAN_TABLE");
	executeStatement(statement2->data);
	FREE(statement2);

//    StringInfo statement3;
//    statement3 = makeStringInfo();
//    appendStringInfo(statement3, "TRUNCATE TABLE PLAN_TABLE");
//    executeStatement(statement3->data);
//    FREE(statement3);

	return cost;
}

List *
oracleGetKeyInformation(char *tableName) {
	List *keyList = NIL;

	if (hasSetElem(haveKeys, tableName))
		return (List *) getMapString(keys, tableName); //TODO copy necessary?

	StringInfo statement;
	statement = makeStringInfo();
	appendStringInfo(statement, "SELECT cols.column_name "
			"FROM all_constraints cons, all_cons_columns cols "
			"WHERE cols.table_name = '%s' "
			"AND cons.constraint_type = 'P' "
			"AND cons.constraint_name = cols.constraint_name "
			"AND cons.owner = cols.owner "
			"ORDER BY cols.table_name, cols.position", tableName);

	OCI_Resultset *rs1 = executeStatement(statement->data);

	if (rs1 != NULL) {
		if (OCI_FetchNext(rs1)) {
			Set *keySet = STRSET();
			do {
				addToSet(keySet, strdup(((char * ) OCI_GetString(rs1, 1))));
			} while (OCI_FetchNext(rs1));
			keyList = appendToTailOfList(keyList, keySet);
		}
	}

	FREE(statement);
	MAP_ADD_STRING_KEY(keys, tableName, copyObject(keyList));
	addToSet(haveKeys, strdup(tableName));
	return keyList;
}

DataType oracleBackendSQLTypeToDT(char *sqlType) {
	if (isPrefix(sqlType, "NUMERIC") || isPrefix(sqlType, "NUMBER")) {
		if (regExMatch(sqlType, "[(][0-9 ]*[,][0-9 ]*[)]"))
			return DT_FLOAT;
		else
			return DT_INT;
	}
	if (isPrefix(sqlType, "VARCHAR") || isPrefix(sqlType, "CHAR"))
		return DT_STRING;
	if (streq(sqlType, "BINARY_FLOAT"))
		return DT_FLOAT;

	return DT_STRING;
}

char *
oracleBackendDatatypeToSQL(DataType dt) {
	switch (dt) {
	case DT_INT:
	case DT_LONG:
		return "NUMBER";
		break;
	case DT_FLOAT:
		return "BINARY_FLOAT";
		break;
	case DT_STRING:
	case DT_VARCHAR2:
		return "VARCHAR2(2000)";
		break;
	case DT_BOOL:
		return "NUMBER(1)";
		break;
	}

	// keep compiler quiet
	return "VARCHAR2(2000)";
}

static OCI_Resultset *
executeStatement(char *statement) {
	if (statement == NULL)
		return NULL;
	if ((conn = getConnection()) != NULL) {
		START_TIMER("Oracle - execute SQL");
		if (st == NULL) {
			st = OCI_StatementCreate(conn);
			OCI_SetFetchSize(st, 1000);
			OCI_SetPrefetchSize(st, 1000);
		}
		OCI_ReleaseResultsets(st);
		if (OCI_ExecuteStmt(st, statement)) {
			OCI_Resultset *rs = OCI_GetResultset(st);
			DEBUG_LOG("Statement: %s executed successfully.", statement);
			DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
			STOP_TIMER("Oracle - execute SQL");
			return rs;
		} else {
			ERROR_LOG("Statement: %s failed.", statement);
		}
		STOP_TIMER("Oracle - execute SQL");
	}
	return NULL;
}

static boolean executeNonQueryStatement(char *statement) {
	if (statement == NULL)
		return FALSE;
	if ((conn = getConnection()) != NULL) {
		START_TIMER("module - metadata lookup");
		if (st == NULL)
			st = OCI_StatementCreate(conn);
		OCI_ReleaseResultsets(st);
		if (OCI_ExecuteStmt(st, statement)) {
			DEBUG_LOG("Statement: %s executed successfully.", statement);
			STOP_TIMER("module - metadata lookup");
			return TRUE;
		} else {
			ERROR_LOG("Statement: %s failed.", statement);
			STOP_TIMER("module - metadata lookup");
			return FALSE;
		}
	}
	return FALSE;
}

Node *
oracleExecuteAsTransactionAndGetXID(List *statements, IsolationLevel isoLevel) {
	OCI_Transaction *t;
	OCI_Resultset *rs;
	Constant *xid = NULL;

	if (!isConnected())
		FATAL_LOG("No connection to database");

	START_TIMER("module - metadata lookup");

	// create transaction
	t = createTransaction(isoLevel);
	if (t == NULL)
		FATAL_LOG("failed creating transaction");
	if (!OCI_SetTransaction(conn, t))
		FATAL_LOG("failed setting current transaction");

	// execute SQL
	FOREACH(char,sql,statements)
		if (!executeNonQueryStatement(sql)) {
			ERROR_LOG("statement %s failed", sql);
			if (!OCI_Rollback(conn))
				FATAL_LOG("Failed rolling back current transaction");
			return NULL;
		}

	// get Transaction XID
	rs = executeStatement("SELECT RAWTOHEX(XID) AS XID FROM v$transaction");
	if (rs != NULL) {
		if (OCI_FetchNext(rs)) {
			const char *xidString =
					OCI_IsNull(rs, 1) ? NULL : OCI_GetString(rs, 1);
			if (xidString == NULL)
				FATAL_LOG("query to retrieve XID did not return any value");
			DEBUG_LOG("Transaction executed with XID: <%s>",
					(char * ) xidString);
			xid = createConstString((char *) xidString);
		} else
			FATAL_LOG("query to get back transaction xid failed");
	} else
		FATAL_LOG("query to get back transaction xid failed");
	// commit transaction and cleanup
	OCI_Commit(conn);
	if (!OCI_TransactionFree(t))
		FATAL_LOG("Failed freeing transaction");

	STOP_TIMER("module - metadata lookup");

	return (Node *) xid;
}

Relation *
oracleGenExecQuery(char *query) {
	List *rel = NIL;
	int numAttrs;
	OCI_Resultset *rs;
	Relation *r = makeNode(Relation);

	rs = executeStatement(query);
	numAttrs = OCI_GetColumnCount(rs);

	// fetch attributes
	r->schema = NIL;
	for (int i = 1; i <= OCI_GetColumnCount(rs); i++) {
		OCI_Column *aInfo = OCI_GetColumn(rs, i);
		const char *name = OCI_ColumnGetName(aInfo);
		r->schema = appendToTailOfList(r->schema, strdup((char * ) name));
	}

	// fetch tuples
	while (OCI_FetchNext(rs)) {
		List *tuple = NIL;

		for (int i = 1; i <= numAttrs; i++)
			tuple = appendToTailOfList(tuple,
					strdup((char * ) OCI_GetString(rs, i)));

		rel = appendToTailOfList(rel, tuple);
	}
	r->tuples = rel;

	// cleanup
	OCI_ReleaseResultsets(st);

	return r;
}

void oracleGenExecQueryIgnoreResult(char *query) {
	OCI_Resultset *rs;

	rs = executeStatement(query);

	// fetch tuples
	while (OCI_FetchNext(rs))
		;

	// cleanup
	OCI_ReleaseResultsets(st);
}

static OCI_Transaction *
createTransaction(IsolationLevel isoLevel) {
	unsigned int mode = 0;
	OCI_Transaction *result = NULL;

	START_TIMER("module - metadata lookup");

	// get OCI isolevel constant
	switch (isoLevel) {
	case ISOLATION_SERIALIZABLE:
		mode = OCI_TRS_SERIALIZABLE;
		break;
	case ISOLATION_READ_COMMITTED:
		mode = OCI_TRS_READWRITE;
		break;
	case ISOLATION_READ_ONLY:
		mode = OCI_TRS_READONLY;
		break;
	}

	// create transaction
	if ((conn = getConnection()) != NULL)
		result = OCI_TransactionCreate(conn, 0, mode, NULL);
	else
		ERROR_LOG("Cannot create transaction: No connection established yet.");

	STOP_TIMER("module - metadata lookup");

	return result;
}

int oracleDatabaseConnectionOpen() {
	if (conn == NULL)
		initConnection();
	if (isConnected())
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

int oracleDatabaseConnectionClose() {
	if (context == NULL) {
		ERROR_LOG("Metadata context already freed.");
		return EXIT_SUCCESS;
	} else {
		ACQUIRE_MEM_CONTEXT(context);
		freeAggList();
		freeWinfList();
		freeBuffers();
		OCI_Cleanup(); //bugs exist here
		initialized = FALSE;
		conn = NULL;
		st = NULL;
		tInfo = NULL;
		errorCache = NULL;

		FREE_AND_RELEASE_CUR_MEM_CONTEXT()
		;
	}

	return EXIT_SUCCESS;
}

//#define maxRead 8000

//static inline char *
//LobToChar (OCI_Lob *lob)
//{
//    unsigned int read = 1;
//    unsigned int byteRead = 0;
//    static char buf[maxRead];
//    StringInfo str = makeStringInfo();
//
//    if (lob == NULL)
//        return "";
//
//    while(OCI_LobRead2(lob, buf, &read, &byteRead) && read > 0)
//    {
//        buf[read] = '\0';
//        appendStringInfoString(str,buf);
//        DEBUG_LOG("read CLOB (%u): %s", read, buf);
//        read = maxRead - 1;
//    }
//
//    DEBUG_LOG("read CLOB: %s", str->data);
//
//    return str->data;
//}

/* OCILIB is not available, fake functions */
#else

int
oracleInitMetadataLookupPlugin (void)
{
	return EXIT_SUCCESS;
}

boolean
oracleCatalogTableExists(char *table)
{
	return FALSE;
}

boolean
oracleCatalogViewExists(char *view)
{
	return FALSE;
}

List *
oracleGetAttributes (char *table)
{
	return NIL;
}

List *
oracleGetAttributeNames (char *tableName)
{
	return NIL;
}

boolean
oracleIsAgg(char *table)
{
	return FALSE;
}

boolean
oracleIsWindowFunction(char *functionName)
{
	return FALSE;
}

char *
oracleGetTableDefinition(char *table) {
	return NULL;
}

void oracleGetTransactionSQLAndSCNs(char *xid, List **scns, List **sqls, List **sqlBinds,
		IsolationLevel *iso, Constant *commitScn)
{
}

char *
oracleGetViewDefinition(char *view) {
	return NULL;
}

char *
oracleExecuteStatement(char *statement)
{
	return NULL;
}

void
oracleGenExecQueryIgnoreResult (char *query)
{

}

Node *
oracleExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
	return NULL;
}

int
oracleDatabaseConnectionOpen (void)
{
	return EXIT_SUCCESS;
}

int
oracleDatabaseConnectionClose ()
{
	return EXIT_SUCCESS;
}

MetadataLookupPlugin *
assembleOracleMetadataLookupPlugin (void)
{
	return NULL;
}

DataType
oracleGetOpReturnType (char *oName, List *dataTypes, boolean *opExists)
{
	return DT_STRING;
}

DataType
oracleGetFuncReturnType (char *fName, List *dataTypes, boolean *funcExists)
{
	return DT_STRING;
}

int
oracleGetCostEstimation(char *query)
{
	return 0;
}

List *
oracleGetKeyInformation(char *tableName)
{
	return NULL;
}

#endif
