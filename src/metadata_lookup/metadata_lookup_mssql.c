/*
 *------------------------------------------------------------------------------
 *
 * metadata_lookup_mssql.c - MS SQLServer metadata lookup plugin
 *
 *     Uses ODBC driver
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2020-11-26
 *        SUBDIR: src/metadata_lookup/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"
#include "utility/string_utils.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_odbc.h"
#include "metadata_lookup/metadata_lookup_mssql.h"
#include "model/list/list.h"
#include "model/query_operator/query_operator.h"
#include "sqltypes.h"

#if HAVE_MSSQL_BACKEND
#include "sql.h"
#include "sqlext.h"
#endif

typedef struct AggInfo {
	char *name;
} AggInfo;

static AggInfo aggs[] = {
	{ "min" },
	{ "max" },
	{ "avg" },
	{ "checksum_agg" },
	{ "count" },
	{ "count_big" },
	{ "max" },
	{ "min" },
	{ "stdev" },
	{ "stdevp" },
	{ "sum" },
	{ "var" },
	{ "varp" },
	{ NULL } // end marker
};

// Mem context
#define CONTEXT_NAME "MSSQLMemContext"
#define METADATA_LOOKUP_TIMER "module - metadata lookup"
#define METADATA_LOOKUP_QUERY_TIMER "module - metadata lookup - running queries"

// real versions if MSSQL is present
#ifdef HAVE_MSSQL_BACKEND

// global vars
// extends MetadataLookupPlugin with MSSQL ODBC connection
typedef struct MSSQLPlugin
{
    ODBCPlugin plugin;
} MSSQLPlugin;

static MemContext *memContext = NULL;
static MSSQLPlugin *plugin = NULL;

#define DRIVER_NAME "ODBC Driver 17 for SQL Server"
#define CONNECTION_STRING_TEMPLATE "Driver=" DRIVER_NAME ";Server=tcp:%s,%d;UID=%s;PWD=%s"

MetadataLookupPlugin *
assembleMssqlMetadataLookupPlugin (void)
{
    plugin = NEW(MSSQLPlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_MSSQL;

    p->initMetadataLookupPlugin = mssqlInitMetadataLookupPlugin;
    p->databaseConnectionOpen = mssqlDatabaseConnectionOpen;
    p->databaseConnectionClose = mssqlDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = mssqlShutdownMetadataLookupPlugin;
    p->isInitialized = mssqlIsInitialized;
    p->catalogTableExists = mssqlCatalogTableExists;
    p->catalogViewExists = mssqlCatalogViewExists;
    p->getAttributes = mssqlGetAttributes;
    p->getAttributeNames = mssqlGetAttributeNames;
    p->isAgg = mssqlIsAgg;
    p->isWindowFunction = mssqlIsWindowFunction;
    p->getFuncReturnType = mssqlGetFuncReturnType;
    p->getOpReturnType = mssqlGetOpReturnType;
    p->getTableDefinition = mssqlGetTableDefinition;
    p->getViewDefinition = mssqlGetViewDefinition;
    p->getTransactionSQLAndSCNs = mssqlGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = mssqlExecuteAsTransactionAndGetXID;
    p->getCostEstimation = mssqlGetCostEstimation;
    p->getKeyInformation = mssqlGetKeyInformation;
    p->executeQuery = mssqlExecuteQuery;
    p->executeQueryIgnoreResult = mssqlExecuteQueryIgnoreResult;
    p->connectionDescription = odbcGetConnectionDescription;
    p->sqlTypeToDT = mssqlBackendSQLTypeToDT;
    p->dataTypeToSQL = mssqlBackendDatatypeToSQL;
	p->getMinAndMax = mssqlGetMinAndMax;

    return p;
}

int
mssqlInitMetadataLookupPlugin (void)
{
    if (plugin && plugin->plugin.initialized)
    {
        INFO_LOG("tried to initialize MSSQL metadata lookup plugin more than once");
        return EXIT_SUCCESS;
    }
	INFO_LOG("initialize MSSQL metadata lookup plugin.");

	setOption(OPTION_ODBC_DRIVER, DRIVER_NAME);

	NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);
	memContext = getCurMemContext();

	START_TIMER(METADATA_LOOKUP_TIMER);

	odbcCreateEnvironment((ODBCPlugin *) plugin);

	plugin->plugin.initialized = TRUE;

	DEBUG_LOG("initialized MSSQL metadata lookup plugin.");

	STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
mssqlShutdownMetadataLookupPlugin(void)
{
	DEBUG_LOG("shutdown MSSQL metadata lookup plugin.");
	if (memContext)
	{
		ACQUIRE_MEM_CONTEXT(memContext);

		odbcDestroyEnvironment((ODBCPlugin *) plugin);

		FREE_AND_RELEASE_CUR_MEM_CONTEXT();
		plugin = NULL;
	}

	return EXIT_SUCCESS;
}

int
mssqlDatabaseConnectionOpen (void)
{
	ACQUIRE_MEM_CONTEXT(memContext);
	START_TIMER(METADATA_LOOKUP_TIMER);

	DEBUG_LOG("open MSSQL metadata lookup plugin database connection...");

	odbcOpenDatabaseConnectionFromConnStr(
		(ODBCPlugin *) plugin,
		odbcCreateConnectionString(DRIVER_NAME)
		);

//TODO reuse plugin->plugin.stmt = odbcCreateStatement((ODBCPlugin *) plugin);

	DEBUG_LOG("opened MSSQL metadata lookup plugin database connection...");

	STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
mssqlDatabaseConnectionClose(void)
{
	SQLRETURN rc;

	DEBUG_LOG("close MSSQL metadata lookup plugin database connection...");
	rc = SQLDisconnect(plugin->plugin.connection);

	SQLFreeHandle(SQL_HANDLE_DBC,plugin->plugin.connection);
    return EXIT_SUCCESS;
}

boolean
mssqlIsInitialized (void)
{
	ODBCPlugin *p = (ODBCPlugin *) plugin;
    if (p && p->initialized)
    {
        if (p->connection == NULL)
        {
            if (mssqlDatabaseConnectionOpen() != EXIT_SUCCESS)
                return FALSE;
        }

		//TODO check connection

        return TRUE;
    }

    return FALSE;


	return plugin && plugin->plugin.initialized;
}

boolean
mssqlCatalogTableExists (char * tableName)
{
	ASSERT(mssqlIsInitialized());
	return odbcTableExistsAsType((ODBCPlugin *) plugin, "TABLE", tableName);
}

boolean
mssqlCatalogViewExists (char * viewName)
{
	ASSERT(mssqlIsInitialized());
	return odbcTableExistsAsType((ODBCPlugin *) plugin, "VIEW", viewName);
}

List *
mssqlGetAttributes (char *tableName)
{
	ASSERT(mssqlIsInitialized());
	return odbcGetAttributesWithTypeConversion((ODBCPlugin *) plugin, tableName, mssqlBackendSQLTypeToDT);
}

List *
mssqlGetAttributeNames (char *tableName)
{
	ASSERT(mssqlIsInitialized());
	List *attrs = mssqlGetAttributes(tableName);
	List *result = NIL;

	FOREACH(AttributeDef,a,attrs)
	{
		result = appendToTailOfList(result, a->attrName);
	}

    return result;
}

boolean
mssqlIsAgg(char *functionName)
{
	ASSERT(mssqlIsInitialized());
	functionName = strToLower(functionName);

	for(AggInfo *a = aggs; a->name != NULL; a++)
	{
		if(streq(functionName, a->name))
		{
			return TRUE;
		}
	}

    return FALSE;
}

boolean
mssqlIsWindowFunction(char *functionName)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
    return FALSE;
}

char *
mssqlGetTableDefinition(char *tableName)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
    return NULL;
}

char *
mssqlGetViewDefinition(char *viewName)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
    return NULL;
}

char *
mssqlBackendDatatypeToSQL (DataType dt)
{
	ASSERT(mssqlIsInitialized());
	switch(dt)
	{
	case DT_INT:
		return "INT"; // 4 byte int
	case DT_LONG:
		return "BIGINT"; // 8 byte int
	case DT_FLOAT:
		return "FLOAT";
	case DT_BOOL: // do not support boolean
		return "BIT";
	case DT_STRING:
	case DT_VARCHAR2:
		return "VARCHAR";
	}
	return NULL;
}

DataType
mssqlGetFuncReturnType(char *fName, List *argTypes, boolean *funcExists)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
	return DT_STRING;
}

DataType
mssqlGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
	return DT_STRING;
}

DataType
mssqlBackendSQLTypeToDT (char *sqlType)
{
	ASSERT(mssqlIsInitialized());

	//TODO fetch type information from system tables: https://docs.microsoft.com/en-us/sql/relational-databases/system-catalog-views/sys-types-transact-sql?view=sql-server-ver15

	// string data types
    if (isPrefix(sqlType, "char")
		|| isPrefix(sqlType, "varchar")
		|| streq(sqlType, "xml")
		)
        return DT_STRING;

    // integer data types
    if (isPrefix(sqlType, "int")
		|| isPrefix(sqlType, "smallint")
		|| isPrefix(sqlType, "tinyint")
		|| streq(sqlType,"int4"))
        return DT_INT;

    // long data types
    if (streq(sqlType, "bigint"))
        return DT_LONG;

    // numeric data types
    if (isPrefix(sqlType, "float")
            || streq(sqlType, "real")
            || streq(sqlType, "numeric")
            )
        return DT_FLOAT;

    // boolean
    if (streq(sqlType,"bit"))
        return DT_BOOL;

    return DT_STRING;
}

HashMap *
mssqlGetMinAndMax(char* tableName, char* colName)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
	return NULL;
}

void
mssqlGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
}

Node *
mssqlExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
    return NULL;
}

int
mssqlGetCostEstimation(char *query)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
    return 0;
}

List *
mssqlGetKeyInformation(char *tableName)
{
	ASSERT(mssqlIsInitialized());
	TODO_IMPL;
    return NULL;
}

Relation *
mssqlExecuteQuery(char *query)
{
	ASSERT(mssqlIsInitialized());
    return odbcExecuteQueryGetResult((ODBCPlugin *) plugin, query);
}

void
mssqlExecuteQueryIgnoreResult (char *query)
{
	ASSERT(mssqlIsInitialized());
	odbcExecuteQueryWithPluginIgnoreResult((ODBCPlugin *) plugin, query);
}

// NO ODBC driver present. Provide dummy methods to keep compiler quiet
#else

MetadataLookupPlugin *
assembleMssqlMetadataLookupPlugin (void)
{
    return NULL;
}

int
mssqlInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
mssqlShutdownMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
mssqlDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}

int
mssqlDatabaseConnectionClose()
{
    return EXIT_SUCCESS;
}

boolean
mssqlIsInitialized (void)
{
    return FALSE;
}

boolean
mssqlCatalogTableExists (char * tableName)
{
    return FALSE;
}

boolean
mssqlCatalogViewExists (char * viewName)
{
    return FALSE;
}

List *
mssqlGetAttributes (char *tableName)
{
    return NIL;
}

List *
mssqlGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
mssqlIsAgg(char *functionName)
{
    return FALSE;
}

boolean
mssqlIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
mssqlGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
mssqlGetViewDefinition(char *viewName)
{
    return NULL;
}

char *
mssqlBackendDatatypeToSQL (DataType dt)
{
	return NULL;
}

void
mssqlGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
mssqlExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
mssqlGetCostEstimation(char *query)
{
    return 0;
}

List *
mssqlGetKeyInformation(char *tableName)
{
    return NULL;
}

Relation *
mssqlExecuteQuery(char *query)
{
    return NULL;
}

void
mssqlExecuteQueryIgnoreResult (char *query)
{

}

#endif
