/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_sqlite.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_METADATA_LOOKUP_METADATA_LOOKUP_SQLITE_H_
#define INCLUDE_METADATA_LOOKUP_METADATA_LOOKUP_SQLITE_H_

#include "metadata_lookup/metadata_lookup.h"

#ifdef HAVE_SQLITE_BACKEND
#include "sqlite3.h"
#endif

extern MetadataLookupPlugin *assembleSqliteMetadataLookupPlugin (void);

/* plugin methods */
extern int sqliteInitMetadataLookupPlugin (void);
extern int sqliteShutdownMetadataLookupPlugin (void);
extern int sqliteDatabaseConnectionOpen (void);
extern int sqliteDatabaseConnectionClose();
extern boolean sqliteIsInitialized (void);

extern boolean sqliteCatalogTableExists (char * tableName);
extern boolean sqliteCatalogViewExists (char * viewName);
extern List *sqliteGetAttributes (char *tableName);
extern List *sqliteGetAttributeNames (char *tableName);
extern boolean sqliteIsAgg(char *functionName);
extern boolean sqliteIsWindowFunction(char *functionName);
extern DataType sqliteGetFuncReturnType (char *fName, List *argTypes);
extern DataType sqliteGetOpReturnType (char *oName, List *argTypes);
extern char *sqliteGetTableDefinition(char *tableName);
extern char *sqliteGetViewDefinition(char *viewName);
extern int sqliteGetCostEstimation(char *query);
extern List *sqliteGetKeyInformation(char *tableName);

extern Relation *sqliteExecuteQuery(char *query);
extern void sqliteGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Node *sqliteExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);


#endif /* INCLUDE_METADATA_LOOKUP_METADATA_LOOKUP_SQLITE_H_ */
