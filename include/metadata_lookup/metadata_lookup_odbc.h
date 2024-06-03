/*
 *------------------------------------------------------------------------------
 *
 * metadata_lookup_odbc.h - generic ODBC metadatalookup plugin
 *
 *     Connecting to an ODBC data source. Also used as utility functions by
 *     metadata lookup plugins that are based on specific ODBC drivers (e.g.,
 *     mssql).
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2020-11-27
 *        SUBDIR: include/metadata_lookup/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _METADATA_LOOKUP_ODBC_H_
#define _METADATA_LOOKUP_ODBC_H_

#include "metadata_lookup/metadata_lookup.h"

#ifdef HAVE_ODBC_BACKEND
// these need to be undefined here because unixodbc defines them
#undef PACKAGE_VERSION
#undef VERSION
#undef PACKAGE_TARNAME
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE
#include "sqltypes.h"
#endif

#include "log/logger.h"

extern MetadataLookupPlugin *assembleOdbcMetadataLookupPlugin (void);

/* additional methods */
extern char *odbcCreateConnectionString(char *driver);

#ifdef HAVE_ODBC_BACKEND
// extends MetadataLookupPlugin with ODBC handles
typedef struct ODBCPlugin
{
    MetadataLookupPlugin plugin;
	boolean initialized;
	SQLHENV environment;
	SQLHDBC connection;
	SQLHSTMT stmt;
	//connection handle
} ODBCPlugin;

#define ODBC_COLNUMBER_SQLCOLUMNS_TYPE_COLUMN_NAME 4
#define ODBC_COLNUMBER_SQLCOLUMNS_SQL_TYPE_NAME 6

#define ODBC_COLNUMBER_SQLPRIMARYKEYS_COLUMN_NAME 4

extern void odbcCreateEnvironment(ODBCPlugin *p);
extern void odbcDestroyEnvironment(ODBCPlugin *p);
extern SQLHDBC odbcOpenDatabaseConnectionFromConnStr(ODBCPlugin *p, char *connstr);
extern SQLHDBC odbcOpenDatabaseConnection(ODBCPlugin *p);
extern int odbcDatabaseConnectionClose(void);
extern int odbcCloseDatabaseConnection(ODBCPlugin *p);
extern SQLHSTMT odbcCreateStatement(ODBCPlugin *p);
extern void odbcDestoryStatement(ODBCPlugin *p, SQLHSTMT stmt);
extern boolean odbcTableExistsAsType (ODBCPlugin *p, char *type, char *table);
extern Relation *odbcExecuteQueryGetResult(ODBCPlugin *p, char *query);
extern void odbcExecuteQueryWithPluginIgnoreResult (ODBCPlugin *p, char *query);
extern List *odbcGetAttributesWithTypeConversion(ODBCPlugin *p, char *tableName, DataType (*convert) (char *typeName));
extern List *odbcGetKeyInformationWithPlugin(ODBCPlugin *p, char *tableName);

extern void odbcHandleError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
extern StringInfo odbcDiagnosticsToStringInfo(SQLHANDLE handle, SQLSMALLINT htype, RETCODE retCode);
extern void odbcLogDiagnostics(char *message, LogLevel l, SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

#define ODBC_SUCCESS(_rc) (_rc == SQL_SUCCESS || _rc == SQL_SUCCESS_WITH_INFO)
#define ODBC_NO_ERROR(_rc) (_rc == SQL_SUCCESS || _rc == SQL_SUCCESS_WITH_INFO || _rc == SQL_NO_DATA)
#define HANDLE_STMT_ERROR(_stmt,_rc)					\
	do {												\
		if (!ODBC_NO_ERROR(_rc))						\
		{												\
			odbcHandleError(_stmt,SQL_HANDLE_STMT,_rc); \
		}												\
	} while(0)

#define WITH_STMT(_p,_s,_code)					\
	do {										\
		SQLHSTMT _s;							\
		_s = odbcCreateStatement(_p);			\
		_code									\
			odbcDestoryStatement(_p,_s);		\
	} while(0);

#define EXEC_QUERY(_p,_code)											\
	do {																\
		SQLRETURN _retcode;												\
		SQLHSTMT _stmt;													\
		_stmt = odbcCreateStatement(_p);								\
		_retcode = SQLExecDirect(_stmt, (SQLCHAR *) query, SQL_NTS);	\
		HANDLE_STMT_ERROR(_stmt,_retcode);								\
		_code															\
			HANDLE_STMT_ERROR(_stmt,_retcode);							\
		odbcDestoryStatement(_p,_stmt);									\
	} while(0)
#define FOR_EXEC_RESULTS() for(_retcode = SQLFetch(_stmt) ;  ODBC_SUCCESS(_retcode) ; _retcode = SQLFetch(_stmt))
#define EXEC_STMT _stmt
#define FOR_RESULTS(s) for(SQLRETURN _retcode = SQLFetch(s) ;  ODBC_SUCCESS(_retcode) ; _retcode = SQLFetch(s))

#define HANDLE_STMT_RESULT_ERROR(_s) if(!ODBC_NO_ERROR(_retcode)) { odbcHandleError(_s,SQL_HANDLE_STMT,_retcode); }
#define RUN_WITH_ERROR_HANDLING(handle,htype,op)						\
	do {																\
		RETCODE _rc = (op);												\
		if (!ODBC_NO_ERROR(_rc))											\
		{																\
			odbcHandleError(handle,htype,_rc);							\
		}																\
		if (_rc == SQL_SUCCESS_WITH_INFO)								\
		{																\
			odbcLogDiagnostics("Statement ODBC info:\n", LOG_DEBUG, handle,htype,_rc); \
		}																\
	} while(0)

#endif

extern char *odbcGetConnectionDescription(void);

/* plugin methods */
extern int odbcInitMetadataLookupPlugin (void);
extern int odbcShutdownMetadataLookupPlugin (void);
extern int odbcDatabaseConnectionOpen (void);
extern boolean odbcIsInitialized (void);

extern boolean odbcCatalogTableExists (char *tableName);
extern boolean odbcCatalogViewExists (char *viewName);
extern List *odbcGetAttributes (char *tableName);
extern List *odbcGetAttributeNames (char *tableName);
extern boolean odbcIsAgg(char *functionName);
extern boolean odbcIsWindowFunction(char *functionName);
extern DataType odbcGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists);
extern DataType odbcGetOpReturnType (char *oName, List *argTypes, boolean *opExists);
extern char *odbcGetTableDefinition(char *tableName);
extern char *odbcGetViewDefinition(char *viewName);
extern int odbcGetCostEstimation(char *query);
extern List *odbcGetKeyInformation(char *tableName);
extern DataType odbcBackendSQLTypeToDT (char *sqlType);
extern char * odbcBackendDatatypeToSQL (DataType dt);
extern HashMap *odbcGetMinAndMax(char *tableName, char *colName);

extern void odbcGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Node *odbcExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
extern Relation *odbcExecuteQuery(char *query);
extern void odbcExecuteQueryIgnoreResult (char *query);

#endif /* _METADATA_LOOKUP_ODBC_H_ */
