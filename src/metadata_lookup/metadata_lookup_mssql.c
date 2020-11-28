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

#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_odbc.h"
#include "metadata_lookup/metadata_lookup_mssql.h"
#include "model/query_operator/query_operator.h"

#if HAVE_MSSQL_BACKEND
#include "sql.h"
#include "sqlext.h"
#endif

// Mem context
#define CONTEXT_NAME "MSSQLMemContext"

// real versions if MSSQL is present
#ifdef HAVE_MSSQL_BACKEND

// global vars
static MemContext *memContext = NULL;
static MSSQLPlugin *plugin = NULL;

// extends MetadataLookupPlugin with postgres connection
typedef struct MSSQLPlugin
{
    ODBCPlugin plugin;
} MSSQLPlugin;

#define RUN_WITH_ERROR_HANDLING(handle,htype,op)			\
	do {													\
	     RETCODE _rc = (op);								\
		 if (_rc != SQL_SUCCESS)							\
		 {													\
			 handleError(handle,htype,_rc);					\
		 }													\
	} while(0)

#define DRIVER_NAME "ODBC Driver 17 for SQL Server"
#define CONNECTION_STRING_TEMPLATE "Driver=" DRIVER_NAME ";Server=tcp:%s,%d;UID=%s;PWD=%s"

// static methods
static void handleError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

MetadataLookupPlugin *
assemblePostgresMetadataLookupPlugin (void)
{
    plugin = NEW(MSSQLPlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_POSTGRES;

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
    p->connectionDescription = mssqlGetConnectionDescription;
    p->sqlTypeToDT = mssqlBackendSQLTypeToDT;
    p->dataTypeToSQL = mssqlBackendDatatypeToSQL;
	p->getMinAndMax = mssqlGetMinAndMax;

    return p;
}

int
mssqlInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
mssqlShutdownMetadataLookupPlugin(void)
{
	mssqlDatabaseConnectionClode();
	SQLFreeHandle(SQL_HANDLE_ENV,plugin->environment);
	//TODO error handling
    return EXIT_SUCCESS;
}

int
mssqlDatabaseConnectionOpen (void)
{
    // Allocate a connection
    RUN_WITH_ERROR_HANDLING(plugin->environment,
							SQL_HANDLE_ENV,
							SQLAllocHandle(SQL_HANDLE_DBC,
										   plugin->environment,
										   plugin->connection));

    RUN_WITH_ERROR_HANDLING(plugin->connection,
        SQL_HANDLE_DBC,
        SQLDriverConnect(plugin->connection,
                         NULL,
                         pwszConnStr,
                         SQL_NTS,
                         NULL,
                         0,
                         NULL,
                         SQL_DRIVER_COMPLETE));


    return EXIT_SUCCESS;
}

static char*
createConnectionString(void)
{
	StringInfo str = makeStringInfo();

	appendStringInfo(str, CONNECTION_STRING_TEMPLATE,
					 getStringOption(OPTION_CONN_HOST),
					 getIntOption(OPTION_CONN_PORT),
					 getStringOption(OPTION_CONN_USER),
					 getStringOption(OPTION_CONN_PASSWD));

	return str->data;
}

int
mssqlDatabaseConnectionClose(void)
{
	SQLFreeHandle(SQL_HANDLE_DBC,plugin->connection);
    return EXIT_SUCCESS;
}

static SQLHSTMT
createStatement(void)
{
	SQLHSTMT stmt;

	RUN_WITH_ERROR_HANDLING(plugin->connection,
            SQL_HANDLE_DBC,
            SQLAllocHandle(SQL_HANDLE_STMT, plugin->connection, &stmt));

	return stmt;
}

static void
handleError(SQLHANDLE handle, SQLSMALLINT htype, RETCODE retCode)
{
    SQLSMALLINT recPos = 0;
    SQLINTEGER error;
    WCHAR message[1000];
    WCHAR state[SQL_SQLSTATE_SIZE+1];
	StringInfo str = makeStringInfo();

    if (RetCode == SQL_INVALID_HANDLE)
    {
		FATAL_LOG("invalid handle!");
    }

	while (SQLGetDiagRec(htype,
                         handle,
                         ++recPos,
                         state,
                         &error,
                         message,
                         (SQLSMALLINT)(sizeof(message) / sizeof(WCHAR)),
                         (SQLSMALLINT *)NULL) == SQL_SUCCESS)
    {
        // Hide data truncated..
        if (wcsncmp(state, L"01004", 5))
        {
            appendStringInfo(str, "[%5.5s] %s (%d)\n", state, message, error);
        }
    }

	FATAL_LOG("ODBC errors:\n%s" str->data);
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
    return odbcExecuteQueryGetResult(plugin, query);
}

void
mssqlExecuteQueryIgnoreResult (char *query)
{
	odbcExecuteQueryWithPluginIgnoreResult(plugin, query);
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
