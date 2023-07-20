/*
 *------------------------------------------------------------------------------
 *
 * metadata_lookup_odbc.c - MS SQLServer metadata lookup plugin
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
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "src/parser/postgres_parser.tab.h"

#if HAVE_ODBC_BACKEND
#include "sql.h"
#include "sqlext.h"
#include "sqltypes.h"

// global vars
static MemContext *memContext = NULL;
#endif

// don't use unicode string
//#undef UNICODE

#define ODBC_STRING_BUFFER_SIZE 1024

// Mem context
#define CONTEXT_NAME "ODBCMemContext"
#define METADATA_LOOKUP_TIMER "module - metadata lookup"
#define METADATA_LOOKUP_QUERY_TIMER "module - metadata lookup - running queries"



// real versions if ODBC is present
#ifdef HAVE_ODBC_BACKEND

// global vars
static ODBCPlugin *plugin = NULL;

#define CONNECTION_STRING_TEMPLATE "Driver={%s};Server={tcp:%s,%d};Database={%s};UID={%s};PWD={%s};"

// static methods
static char *rsGetColumnName(SQLHANDLE stmt, int pos);

MetadataLookupPlugin *
assembleOdbcMetadataLookupPlugin (void)
{
    plugin = NEW(ODBCPlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_ODBC;

    p->initMetadataLookupPlugin = odbcInitMetadataLookupPlugin;
    p->databaseConnectionOpen = odbcDatabaseConnectionOpen;
    p->databaseConnectionClose = odbcDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = odbcShutdownMetadataLookupPlugin;
    p->isInitialized = odbcIsInitialized;
    p->catalogTableExists = odbcCatalogTableExists;
    p->catalogViewExists = odbcCatalogViewExists;
    p->getAttributes = odbcGetAttributes;
    p->getAttributeNames = odbcGetAttributeNames;
    p->isAgg = odbcIsAgg;
    p->isWindowFunction = odbcIsWindowFunction;
    p->getFuncReturnType = odbcGetFuncReturnType;
    p->getOpReturnType = odbcGetOpReturnType;
    p->getTableDefinition = odbcGetTableDefinition;
    p->getViewDefinition = odbcGetViewDefinition;
    p->getTransactionSQLAndSCNs = odbcGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = odbcExecuteAsTransactionAndGetXID;
    p->getCostEstimation = odbcGetCostEstimation;
    p->getKeyInformation = odbcGetKeyInformation;
    p->executeQuery = odbcExecuteQuery;
    p->executeQueryIgnoreResult = odbcExecuteQueryIgnoreResult;
    p->connectionDescription = odbcGetConnectionDescription;
    p->sqlTypeToDT = odbcBackendSQLTypeToDT;
    p->dataTypeToSQL = odbcBackendDatatypeToSQL;
	p->getMinAndMax = odbcGetMinAndMax;

    return p;
}

int
odbcInitMetadataLookupPlugin (void)
{
    if (plugin && plugin->initialized)
    {
        INFO_LOG("tried to initialize metadata lookup plugin more than once");
        return EXIT_SUCCESS;
    }

    NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);
	memContext = getCurMemContext();

	START_TIMER(METADATA_LOOKUP_TIMER);

	odbcCreateEnvironment(plugin);

	plugin->initialized = TRUE;

	STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT();

	DEBUG_LOG("inited ODBC metadata lookup plugin.");
    return EXIT_SUCCESS;
}

void
odbcCreateEnvironment(ODBCPlugin *p)
{
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &p->environment) == SQL_ERROR)
	{
		FATAL_LOG("was not able to allocate environment handle for ODBC ODBC driver");
	}

	// set ODBC version for environment
	RUN_WITH_ERROR_HANDLING(p->environment,
							SQL_HANDLE_ENV,
							SQLSetEnvAttr(p->environment,
										  SQL_ATTR_ODBC_VERSION,
										  (SQLPOINTER) SQL_OV_ODBC3,
										  0));

	DEBUG_LOG("created ODBC environment handle.");
}


int
odbcShutdownMetadataLookupPlugin(void)
{
	if (memContext)
	{
		ACQUIRE_MEM_CONTEXT(memContext);

		odbcDestroyEnvironment(plugin);

		FREE_AND_RELEASE_CUR_MEM_CONTEXT();
		plugin = NULL;
	}

	DEBUG_LOG("shutdown ODBC metadata lookup plugin.");
	return EXIT_SUCCESS;
}

void
odbcDestroyEnvironment(ODBCPlugin *p)
{
	odbcDatabaseConnectionClose(p);
	if (p->environment != SQL_NULL_HENV)
	{
		SQLFreeHandle(SQL_HANDLE_ENV,p->environment);
	}
	DEBUG_LOG("destoryed ODBC environment.");
}


int
odbcDatabaseConnectionOpen(void)
{
    ACQUIRE_MEM_CONTEXT(memContext);
	START_TIMER(METADATA_LOOKUP_TIMER);

	char *connStr = odbcCreateConnectionString(
		getStringOption(OPTION_ODBC_DRIVER));

	INFO_LOG("will open ODBC connection to <%s> ...", connStr);
	odbcOpenDatabaseConnectionFromConnStr(plugin, connStr);

	DEBUG_LOG("opened ODBC connection to <%s>", connStr);
	STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

SQLHDBC
odbcOpenDatabaseConnection(ODBCPlugin *p)
{
	return odbcOpenDatabaseConnectionFromConnStr(p,
		odbcCreateConnectionString(getStringOption(OPTION_ODBC_DRIVER)));
}


char *
odbcGetConnectionDescription(void)
{
    return CONCAT_STRINGS("ODBC[",
						  getStringOption(OPTION_ODBC_DRIVER), "]:",
						  getStringOption(OPTION_CONN_USER), "@",
						  getStringOption(OPTION_CONN_HOST), ":",
						  getOptionAsString(OPTION_CONN_PORT), "/",
						  getStringOption(OPTION_CONN_DB));
}

SQLHDBC
odbcOpenDatabaseConnectionFromConnStr(ODBCPlugin *p, char *connstr)
{
	// connection string as SQLSTRING
	SQLCHAR *conn = (SQLCHAR*) connstr;

	DEBUG_LOG("open ODBC connection to <%s> ...", connstr);



    // Allocate a connection
    RUN_WITH_ERROR_HANDLING(p->environment,
							SQL_HANDLE_ENV,
							SQLAllocHandle(SQL_HANDLE_DBC,
										   p->environment,
										   &(p->connection)));

	DEBUG_LOG("allocated handle");

	// create connection
    RUN_WITH_ERROR_HANDLING(p->connection,
        SQL_HANDLE_DBC,
        SQLDriverConnect(p->connection,
                         NULL,
                         conn,
                         SQL_NTS,
                         NULL,
                         0,
                         NULL,
                         SQL_DRIVER_NOPROMPT));

	DEBUG_LOG("opened ODBC connection to <%s>", connstr);

    return p->connection;
}

char *
odbcCreateConnectionString(char *driver)
{
	StringInfo str = makeStringInfo();

	appendStringInfo(str,
					 CONNECTION_STRING_TEMPLATE,
					 driver,
					 getStringOption(OPTION_CONN_HOST),
					 getIntOption(OPTION_CONN_PORT),
					 getStringOption(OPTION_CONN_DB),
					 getStringOption(OPTION_CONN_USER),
					 getStringOption(OPTION_CONN_PASSWD));

	return str->data;
}

int
odbcDatabaseConnectionClose(void)
{
	return odbcCloseDatabaseConnection(plugin);
}

int
odbcCloseDatabaseConnection(ODBCPlugin *p)
{
	DEBUG_LOG("close ODBC connection");
	if (p->connection != SQL_NULL_HDBC)
	{
		SQLFreeHandle(SQL_HANDLE_DBC,p->connection);
	}
    return EXIT_SUCCESS;
}

SQLHSTMT
odbcCreateStatement(ODBCPlugin *p)
{
	SQLHSTMT stmt;

	DEBUG_LOG("create ODBC statement");
	RUN_WITH_ERROR_HANDLING(p->connection,
            SQL_HANDLE_DBC,
            SQLAllocHandle(SQL_HANDLE_STMT,
						   p->connection,
						   &stmt));

	return stmt;
}

void
odbcDestoryStatement(ODBCPlugin *p, SQLHSTMT stmt)
{
	DEBUG_LOG("destroy ODBC statement");
	if(stmt != SQL_NULL_HSTMT)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
	}
}

void
odbcHandleError(SQLHANDLE handle, SQLSMALLINT htype, RETCODE retCode)
{
	StringInfo str = odbcDiagnosticsToStringInfo(handle,htype,retCode);

	FATAL_LOG("ODBC errors:\n%s", str->data);
}

void
odbcLogDiagnostics(char *message, LogLevel l, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	DEBUG_LOG("message:\n%s", odbcDiagnosticsToStringInfo(hHandle, hType, RetCode)->data);
}


StringInfo
odbcDiagnosticsToStringInfo(SQLHANDLE handle, SQLSMALLINT htype, RETCODE retCode)
{
	SQLINTEGER recPos = 0;
    SQLINTEGER error;
    SQLCHAR *message = CNEW(SQLCHAR,1000);
    SQLCHAR *state = CNEW(SQLCHAR, (SQL_SQLSTATE_SIZE+1));
	StringInfo str = makeStringInfo();

	if (retCode == SQL_INVALID_HANDLE)
    {
		RELEASE_MEM_CONTEXT();
		FATAL_LOG("invalid handle!");
    }

	appendStringInfo(str, "ERROR CODE [%d]\n\n", retCode);

	while (SQLGetDiagRec(htype,
                         handle,
                         ++recPos,
                         state,
                         &error,
                         message,
                         (SQLSMALLINT)(sizeof(message) - 1),
                         (SQLSMALLINT *)NULL) == SQL_SUCCESS)
    {
        // Hide data truncated..
		DEBUG_LOG("error %s", message);
        appendStringInfo(str, "[%5.5s] %s (%d)\n", state, message, error);
    }

	return str;
}


boolean
odbcIsInitialized (void)
{
    return plugin && plugin->initialized;
}

boolean
odbcCatalogTableExists (char *tableName)
{
    return odbcTableExistsAsType(plugin, "TABLE", tableName);
}

boolean
odbcCatalogViewExists (char *viewName)
{
    return odbcTableExistsAsType(plugin, "VIEW", viewName);
}

boolean
odbcTableExistsAsType(ODBCPlugin *p, char *type, char *table)
{
	boolean exists = FALSE;

	DEBUG_LOG("check whether %s %s exists ...", type, table);

	WITH_STMT(p,stmt,
			  RUN_WITH_ERROR_HANDLING(stmt, SQL_HANDLE_STMT,
									  SQLTables(stmt,
												NULL,
												SQL_NTS,
												NULL,
												SQL_NTS,
												(SQLCHAR *) table,
												SQL_NTS,
												(SQLCHAR*) type,
												SQL_NTS)
				  );

			  FOR_RESULTS(stmt)
			  {
				  DEBUG_LOG("fetched a row");
				  exists = TRUE;
				  HANDLE_STMT_RESULT_ERROR(stmt);
			  }

		);

	DEBUG_LOG("... %s", exists ? "TRUE": "FALSE");

    return exists;
}

List *
odbcGetAttributes (char *tableName)
{
	TODO_IMPL;
    return NIL;
}

List *
odbcGetAttributesWithTypeConversion(ODBCPlugin *p,
									char *tableName,
									DataType (*convert) (char *typeName))
{
	List *result = NIL;
	SQLSMALLINT numCols;
	SQLCHAR *colName = (SQLCHAR *) CALLOC(sizeof(char), ODBC_STRING_BUFFER_SIZE);
	SQLCHAR *colDT = (SQLCHAR *) CALLOC(sizeof(char), ODBC_STRING_BUFFER_SIZE);
	SQLLEN length;

	ASSERT(p->initialized);

	WITH_STMT((ODBCPlugin *) p, stmt,
			  SQLRETURN _retcode = SQLColumns(stmt,
											  NULL,
											  SQL_NTS,
											  NULL,
											  SQL_NTS,
											  (SQLCHAR *) tableName,
											  SQL_NTS,
											  NULL,
											  SQL_NTS);

			  SQLNumResultCols(stmt, &numCols);

			  HANDLE_STMT_ERROR(stmt, _retcode);
			  FOR_RESULTS(stmt)
			  {
				  HANDLE_STMT_RESULT_ERROR(stmt);

				  RUN_WITH_ERROR_HANDLING(stmt, SQL_HANDLE_STMT,
										  SQLGetData(stmt,
													 ODBC_COLNUMBER_SQLCOLUMNS_TYPE_COLUMN_NAME,
													 SQL_CHAR,
													 colName,
													 ODBC_STRING_BUFFER_SIZE,
													 &length
											  )
					  );

				  RUN_WITH_ERROR_HANDLING(stmt, SQL_HANDLE_STMT,
										  SQLGetData(stmt,
													 ODBC_COLNUMBER_SQLCOLUMNS_SQL_TYPE_NAME,
													 SQL_CHAR,
													 colDT,
													 ODBC_STRING_BUFFER_SIZE,
													 &length
											  )
					  );

				  DEBUG_LOG("column %s with dt: %s", colName, colDT);
				  result = appendToTailOfList(
					  result,
					  createAttributeDef(strdup((char *) colName),
										 convert((char *) colDT)));
			  }
		);

    return result;
}


List *
odbcGetAttributeNames (char *tableName)
{
	TODO_IMPL;
    return NIL;
}

boolean
odbcIsAgg(char *functionName)
{
	TODO_IMPL;
    return FALSE;
}

boolean
odbcIsWindowFunction(char *functionName)
{
	TODO_IMPL;
    return FALSE;
}

char *
odbcGetTableDefinition(char *tableName)
{
	TODO_IMPL;
    return NULL;
}

char *
odbcGetViewDefinition(char *viewName)
{
	TODO_IMPL;
    return NULL;
}

void
odbcGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
	TODO_IMPL;
}

Node *
odbcExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
	TODO_IMPL;
    return NULL;
}

int
odbcGetCostEstimation(char *query)
{
	TODO_IMPL;
    return 0;
}

List *
odbcGetKeyInformation(char *tableName)
{
	TODO_IMPL;
	return NIL;
}

List *
odbcGetKeyInformationWithPlugin(ODBCPlugin *p, char *tableName)
{
	List *result = NIL;
	Set *keySet;
	SQLCHAR buf[ODBC_STRING_BUFFER_SIZE];
	SQLLEN length;

	ASSERT(p->initialized);
	ACQUIRE_MEM_CONTEXT(memContext);

    keySet = STRSET();

	WITH_STMT((ODBCPlugin *) p, stmt,
			  SQLRETURN _retcode = 	SQLPrimaryKeys(stmt,
												   NULL,
												   SQL_NTS,
												   NULL,
												   SQL_NTS,
												   (SQLCHAR *) tableName,
												   SQL_NTS);

			  HANDLE_STMT_ERROR(stmt, _retcode);
			  FOR_RESULTS(stmt)
			  {
				  HANDLE_STMT_RESULT_ERROR(stmt);

				  RUN_WITH_ERROR_HANDLING(stmt, SQL_HANDLE_STMT,
										  SQLGetData(stmt,
													 ODBC_COLNUMBER_SQLPRIMARYKEYS_COLUMN_NAME,
													 SQL_CHAR,
													 buf,
													 ODBC_STRING_BUFFER_SIZE,
													 &length
											  )
					  );

				  DEBUG_LOG("key attribute %s", buf);
				  addToSet(keySet, strdup((char *) buf));
			  }
		);

	if(!EMPTY_SET(keySet))
	{
		result = singleton(keySet);
	}

	RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List,result);
}

Relation *
odbcExecuteQuery(char *query)
{
	return odbcExecuteQueryGetResult(plugin, query);
}

Relation *
odbcExecuteQueryGetResult(ODBCPlugin *p, char *query)
{
	SQLSMALLINT numcols;
	SQLRETURN retcode;
	Relation *r = makeNode(Relation);

	EXEC_QUERY(p,
			   RUN_WITH_ERROR_HANDLING(EXEC_STMT, SQL_HANDLE_STMT,
									   SQLNumResultCols(EXEC_STMT, &numcols)
				   );

			   // setup schema
			   for(int i = 0; i < numcols; i++)
			   {
				   char *name = rsGetColumnName(EXEC_STMT, i);
				   r->schema = appendToTailOfList(r->schema, strdup((char *) name));
			   }

			   // loop over tuples
			   FOR_EXEC_RESULTS()
			   {
				   List *tuple = NIL;
				   for(int i = 1; i <= numcols; i++)
				   {
					   SQLLEN  len;
					   SQLCHAR buf[ODBC_STRING_BUFFER_SIZE];

					   //TODO deal with larger size
					   // Retrieve column data as a string
					   retcode = SQLGetData(EXEC_STMT,
											i,
											SQL_C_CHAR,
											buf,
											sizeof(buf),
											&len);
					   if(SQL_SUCCEEDED(retcode))
					   {
						   // Handle null columns
						   if(len == SQL_NULL_DATA)
						   {
							   strcpy((char *) buf, "NULL");
						   }
						   tuple = appendToTailOfList(tuple,
														  strdup((char *) buf));
					   }
					   else
					   {
						   odbcHandleError(EXEC_STMT, SQL_HANDLE_STMT, retcode);
					   }
				   }
				   VEC_ADD_NODE(r->tuples, tuple);
			   }
		);

	return r;
}

static char *
rsGetColumnName(SQLHANDLE stmt, int pos)
{
	StringInfo str = makeStringInfo();
	SQLSMALLINT len = -1;

	// if there is not enough space, enlarge to needed size and try again
    do
	{
		if(len > str->maxlen)
		{
			enlargeStringInfo(str, len + 1);
		}
		RUN_WITH_ERROR_HANDLING(stmt, SQL_HANDLE_STMT,
								SQLColAttribute(stmt,
												pos + 1,
												SQL_DESC_NAME,
												str->data,
												str->maxlen,
												&len,
												NULL
									)
		);
	} while(len > str->maxlen);

	return str->data;
}

void
odbcExecuteQueryIgnoreResult (char *query)
{
	odbcExecuteQueryWithPluginIgnoreResult(plugin, query);
}

void
odbcExecuteQueryWithPluginIgnoreResult (ODBCPlugin *p, char *query)
{
	SQLSMALLINT numcols;
	Relation *r = makeNode(Relation);
	int numtup = 0;

	RUN_WITH_ERROR_HANDLING(plugin->stmt, SQL_HANDLE_STMT,
							SQLNumResultCols(plugin->stmt, &numcols)
		);

	// setup schema
	for(int i = 0; i < numcols; i++)
	{
		char *name = rsGetColumnName(plugin->stmt, i);
		r->schema = appendToTailOfList(r->schema, strdup((char *) name));
	}

	// loop over tuples
	FOR_RESULTS(plugin->stmt)
	{
		numtup++;
	}
}

DataType
odbcBackendSQLTypeToDT (char *sqlType)
{
    return DT_STRING; //TODO
}

char *
odbcBackendDatatypeToSQL (DataType dt)
{
    switch(dt)
    {
        case DT_INT:
        case DT_LONG:
            return "NUMERIC";
            break;
        case DT_FLOAT:
            return "NUMERIC";
            break;
        case DT_STRING:
        case DT_VARCHAR2:
            return "VARCHAR";
            break;
        case DT_BOOL:
            return "BOOL";
            break;
    }

    // keep compiler quiet
    return "VARCHAR";
}

DataType
odbcGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
	TODO_IMPL;
	return DT_STRING;
}

DataType
odbcGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{
	return DT_STRING;
}


HashMap *
odbcGetMinAndMax(char* tableName, char* colName)
{
	TODO_IMPL;
	return NULL;
}



// NO ODBC driver present. Provide dummy methods to keep compiler quiet
#else

MetadataLookupPlugin *
assembleOdbcMetadataLookupPlugin (void)
{
    return NULL;
}

int
odbcInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
odbcShutdownMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
odbcDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}

int
odbcDatabaseConnectionClose()
{
    return EXIT_SUCCESS;
}

boolean
odbcIsInitialized (void)
{
    return FALSE;
}

boolean
odbcCatalogTableExists (char * tableName)
{
    return FALSE;
}

boolean
odbcCatalogViewExists (char * viewName)
{
    return FALSE;
}

List *
odbcGetAttributes (char *tableName)
{
    return NIL;
}

List *
odbcGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
odbcIsAgg(char *functionName)
{
    return FALSE;
}

boolean
odbcIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
odbcGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
odbcGetViewDefinition(char *viewName)
{
    return NULL;
}

char *
odbcBackendDatatypeToSQL (DataType dt)
{
	return NULL;
}

void
odbcGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
odbcExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
odbcGetCostEstimation(char *query)
{
    return 0;
}

List *
odbcGetKeyInformation(char *tableName)
{
    return NULL;
}

Relation *
odbcExecuteQuery(char *query)
{
    return NULL;
}

void
odbcExecuteQueryIgnoreResult (char *query)
{

}

#endif
