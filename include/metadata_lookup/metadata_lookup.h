/*
 * metadata_lookup.h
 *
 *      Author: zephyr
 *      Get the metadata by table name.
 *
 *      catalogTableExists() checks if the given table exists
 *      getAttributes() gets all the attribute names of the given table
 *      Don't worry about the connection establishment. It will automatically connect to oracle server
 *      if not created.
 *      But you should call databaseConnectionClose() at the end of the whole program.
 *      Note that it need only be called once because it frees the list of attributes and all resources allocated
 *      for OCI connection.
 */

#ifndef METADATA_LOOKUP_H_
#define METADATA_LOOKUP_H_

#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/query_block/query_block.h"


/* types of supported plugins */
typedef enum MetadataLookupPluginType
{
    METADATA_LOOKUP_PLUGIN_ORACLE,
    METADATA_LOOKUP_PLUGIN_POSTGRES
} MetadataLookupPluginType;

/* catalog cache */
typedef struct CatalogCache
{
    HashMap *tableAttrs;        // hashmap tablename -> attribute names
    HashMap *tableAttrDefs;     // hashmap tablename -> attribute definitions
    HashMap *viewAttrs;         // hashmap viewname -> attribute names
    Set *tableNames;            // set of existing table names
    Set *viewNames;             // set of existing view names
    Set *aggFuncNames;          // names of aggregate functions
    Set *winFuncNames;          // names of window functions

} CatalogCache;

/* plugin definition */
typedef struct MetadataLookupPlugin
{
    MetadataLookupPluginType type;

    /* functional interface */

    /* init and shutdown plugin and connection */
    boolean (*isInitialized) (void);
    int (*initMetadataLookupPlugin) (void);
    int (*databaseConnectionOpen) (void);
    int (*databaseConnectionClose) (void);
    int (*shutdownMetadataLookupPlugin) (void);

    /* catalog lookup */
    boolean (*catalogTableExists) (char * tableName);
    boolean (*catalogViewExists) (char * viewName);
    List * (*getAttributes) (char *tableName);
    List * (*getAttributeNames) (char *tableName);
    boolean (*isAgg) (char *functionName);
    boolean (*isWindowFunction) (char *functionName);
    char * (*getTableDefinition) (char *tableName);
    char * (*getViewDefinition) (char *viewName);

    /* audit log access */
    void (*getTransactionSQLAndSCNs) (char *xid, List **scns, List **sqls,
            List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
    long (*getCommitScn) (char *tableName, long maxScn, char *xid);

    /* execute transaction */
    Node * (*executeAsTransactionAndGetXID) (List *statements, IsolationLevel isoLevel);

    /* cache for catalog information */
    CatalogCache *cache;

} MetadataLookupPlugin;


/* store active plugin */
extern MetadataLookupPlugin *activePlugin;
extern List *availablePlugins;

/* plugin handling methods */
extern int initMetadataLookupPlugins (void);
extern int shutdownMetadataLookupPlugins (void);
extern void chooseMetadataLookupPlugin (MetadataLookupPluginType plugin);

/* generic methods */
extern int initMetadataLookupPlugin (void);
extern int shutdownMetadataLookupPlugin (void);
extern int databaseConnectionOpen (void);
extern int databaseConnectionClose(void);
extern boolean isInitialized (void);

extern boolean catalogTableExists (char * tableName);
extern boolean catalogViewExists (char * viewName);
extern List *getAttributes (char *tableName);
extern List *getAttributeNames (char *tableName);
extern boolean isAgg(char *functionName);
extern boolean isWindowFunction(char *functionName);
extern char *getTableDefinition(char *tableName);
extern char *getViewDefinition(char *viewName);

extern void getTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern long getCommitScn (char *tableName, long maxScn, char *xid);
extern Node *executeAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);

/* helper functions for createing the cache */
extern CatalogCache *createCache(void);

#endif /* METADATA_LOOKUP_H_ */
