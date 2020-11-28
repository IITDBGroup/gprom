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

extern void odbcCreateEnvironment(ODBCPlugin *p);
extern void odbcDestroyEnvironment(ODBCPlugin *p);
extern SQLHDBC odbcOpenDatabaseConnectionFromConnStr(ODBCPlugin *p, char *connstr);
extern SQLHDBC odbcOpenDatabaseConnection(ODBCPlugin *p);
extern int odbcCloseDatabaseConnection(ODBCPlugin *p);
extern SQLHSTMT odbcCreateStatement(ODBCPlugin *p);
extern boolean odbcTableExistsAsType (ODBCPlugin *p, char *type, char *table);
extern Relation *odbcExecuteQueryGetResult(ODBCPlugin *p, char *query);
extern void odbcExecuteQueryWithPluginIgnoreResult (ODBCPlugin *p, char *query);

#endif

/* plugin methods */
extern int odbcInitMetadataLookupPlugin (void);
extern int odbcShutdownMetadataLookupPlugin (void);
extern int odbcDatabaseConnectionOpen (void);
extern int odbcDatabaseConnectionClose();
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
