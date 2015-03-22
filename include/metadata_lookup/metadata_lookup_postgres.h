/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_postgres.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef METADATA_LOOKUP_POSTGRES_H_
#define METADATA_LOOKUP_POSTGRES_H_

#include "metadata_lookup/metadata_lookup.h"

#ifdef HAVE_POSTGRES_BACKEND
#include "libpq-fe.h"
#endif

extern MetadataLookupPlugin *assemblePostgresMetadataLookupPlugin (void);

/* plugin methods */
extern int postgresInitMetadataLookupPlugin (void);
extern int postgresShutdownMetadataLookupPlugin (void);
extern int postgresDatabaseConnectionOpen (void);
extern int postgresDatabaseConnectionClose();
extern boolean postgresIsInitialized (void);

#ifdef HAVE_POSTGRES_BACKEND
extern PGconn *getPostgresConnection(void);
#endif

extern boolean postgresCatalogTableExists (char * tableName);
extern boolean postgresCatalogViewExists (char * viewName);
extern List *postgresGetAttributes (char *tableName);
extern List *postgresGetAttributeNames (char *tableName);
extern boolean postgresIsAgg(char *functionName);
extern boolean postgresIsWindowFunction(char *functionName);
extern DataType postgresGetFuncReturnType (char *fName, List *argTypes);
extern DataType postgresGetOpReturnType (char *oName, List *argTypes);
extern char *postgresGetTableDefinition(char *tableName);
extern char *postgresGetViewDefinition(char *viewName);
extern int postgresGetCostEstimation(char *query);
extern List *postgresGetKeyInformation(char *tableName);

extern void postgresGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Node *postgresExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);


#endif /* METADATA_LOOKUP_POSTGRES_H_ */
