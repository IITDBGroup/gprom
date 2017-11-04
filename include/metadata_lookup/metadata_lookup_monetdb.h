/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_monetdb.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef METADATA_LOOKUP_MONETDB_H_
#define METADATA_LOOKUP_MONETDB_H_

#include "metadata_lookup/metadata_lookup.h"

#ifdef HAVE_MONETDB_BACKEND
#include <monetdb/mapi.h>
#endif

extern MetadataLookupPlugin *assembleMonetdbMetadataLookupPlugin (void);

/* plugin methods */
extern int monetdbInitMetadataLookupPlugin (void);
extern int monetdbShutdownMetadataLookupPlugin (void);
extern int monetdbDatabaseConnectionOpen (void);
extern int monetdbDatabaseConnectionClose();
extern boolean monetdbIsInitialized (void);

extern boolean monetdbCatalogTableExists (char * tableName);
extern boolean monetdbCatalogViewExists (char * viewName);
extern List *monetdbGetAttributes (char *tableName);
extern List *monetdbGetAttributeNames (char *tableName);
extern boolean monetdbIsAgg(char *functionName);
extern boolean monetdbIsWindowFunction(char *functionName);
extern DataType monetdbGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists);
extern DataType monetdbGetOpReturnType (char *oName, List *argTypes, boolean *opExists);
extern char *monetdbGetTableDefinition(char *tableName);
extern char *monetdbGetViewDefinition(char *viewName);
extern int monetdbGetCostEstimation(char *query);
extern List *monetdbGetKeyInformation(char *tableName);

extern void monetdbGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Node *monetdbExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
extern Relation *monetdbExecuteQuery(char *query);
extern void monetdbExecuteQueryIgnoreResults(char *query);

#endif /* METADATA_LOOKUP_MONETDB_H_ */
