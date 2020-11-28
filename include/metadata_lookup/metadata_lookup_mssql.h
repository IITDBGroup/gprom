/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_mssql.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef METADATA_LOOKUP_MSSQL_H_
#define METADATA_LOOKUP_MSSQL_H_

#include "metadata_lookup/metadata_lookup.h"

#ifdef HAVE_MSSQL_BACKEND
#include "sql.h"
#include "sqlext.h"
#endif

extern MetadataLookupPlugin *assembleMssqlMetadataLookupPlugin (void);

/* plugin methods */
extern int mssqlInitMetadataLookupPlugin (void);
extern int mssqlShutdownMetadataLookupPlugin (void);
extern int mssqlDatabaseConnectionOpen (void);
extern int mssqlDatabaseConnectionClose();
extern boolean mssqlIsInitialized (void);

#ifdef HAVE_MSSQL_BACKEND
extern PGconn *getMssqlConnection(void);
#endif

extern boolean mssqlCatalogTableExists (char * tableName);
extern boolean mssqlCatalogViewExists (char * viewName);
extern List *mssqlGetAttributes (char *tableName);
extern List *mssqlGetAttributeNames (char *tableName);
extern boolean mssqlIsAgg(char *functionName);
extern boolean mssqlIsWindowFunction(char *functionName);
extern DataType mssqlGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists);
extern DataType mssqlGetOpReturnType (char *oName, List *argTypes, boolean *opExists);
extern char *mssqlGetTableDefinition(char *tableName);
extern char *mssqlGetViewDefinition(char *viewName);
extern int mssqlGetCostEstimation(char *query);
extern List *mssqlGetKeyInformation(char *tableName);
extern DataType mssqlBackendSQLTypeToDT (char *sqlType);
extern char * mssqlBackendDatatypeToSQL (DataType dt);
extern HashMap *mssqlGetMinAndMax(char* tableName, char* colName);

extern void mssqlGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Node *mssqlExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
extern Relation *mssqlExecuteQuery(char *query);
extern void mssqlExecuteQueryIgnoreResult (char *query);

#endif /* METADATA_LOOKUP_MSSQL_H_ */
