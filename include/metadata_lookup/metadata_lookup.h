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
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/relation/relation.h"
#include "model/query_block/query_block.h"
#include "mem_manager/mem_mgr.h"


/* types of supported plugins */
NEW_ENUM_WITH_TO_STRING(MetadataLookupPluginType,
    METADATA_LOOKUP_PLUGIN_ORACLE,
    METADATA_LOOKUP_PLUGIN_POSTGRES,
    METADATA_LOOKUP_PLUGIN_SQLITE,
    METADATA_LOOKUP_PLUGIN_MONETDB,
    METADATA_LOOKUP_PLUGIN_EXTERNAL
);

/* catalog cache */
typedef struct CatalogCache
{
    HashMap *tableAttrs;        // hashmap tablename -> attribute names
    HashMap *tableAttrDefs;     // hashmap tablename -> attribute definitions
    HashMap *viewAttrs;         // hashmap viewname -> attribute names
    HashMap *viewDefs;          // hashmap viewname -> view definition SQL query
    Set *tableNames;            // set of existing table names
    Set *viewNames;             // set of existing view names
    Set *aggFuncNames;          // names of aggregate functions
    Set *winFuncNames;          // names of window functions
    void *cacheHook;            // used to store
//    void (*cleanAddCache) (CatalogCache *cache); // function to clean up additional cache
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
    char * (*connectionDescription) (void);

    /* catalog lookup */
    boolean (*catalogTableExists) (char * tableName);
    boolean (*catalogViewExists) (char * viewName);
    boolean (*checkPostive) (char *tableName, char *colName);
    Constant * (*trasnferRawData) (char *data, char *dataType);
    HashMap * (*getMinAndMax) (char *tableName, char *colName);
    List * (*getHist) (char *tableName, char *colName, char *numPartitions);
    char * (*get2DHist) (char *tableName, char *colName, char *colName2, char *numPartitions1, char *numPartitions2);
    void (*storeInterval) (char *tableName, char *attrName, char *numPartitions);
    char * (*join2Hist) (char *hist1, char *hist2, char *tableName, char *attrName);
    List * (*computeSum) (char *tableName, char *attrName, char *sumAttrName);
    int (*getRowNum) (char *tableName);
    int (*getDistinct) (char *tableName, char *colName);

    List * (*getAttributes) (char *tableName);
    List * (*getAttributeNames) (char *tableName);
    Node * (*getAttributeDefaultVal) (char *schema, char *tableName, char *attrName);

    boolean (*isAgg) (char *functionName);
    boolean (*isWindowFunction) (char *functionName);
    DataType (*getFuncReturnType) (char *fName, List *argTypes, boolean *funcExists);
    DataType (*getOpReturnType) (char *oName, List *argTypes, boolean *funcExists);
    DataType (*sqlTypeToDT) (char *sqlType);
    char * (*dataTypeToSQL) (DataType dt);

    char * (*getTableDefinition) (char *tableName);
    char * (*getViewDefinition) (char *viewName);
    List * (*getKeyInformation) (char *tableName);
    /* audit log access */
    void (*getTransactionSQLAndSCNs) (char *xid, List **scns, List **sqls,
            List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
    gprom_long_t (*getCommitScn) (char *tableName, gprom_long_t maxScn, char *xid);

    /* execution */
    Node * (*executeAsTransactionAndGetXID) (List *statements, IsolationLevel isoLevel);
    Relation * (*executeQuery) (char *query);       // returns a list of stringlist (tuples)
    void (*executeQueryIgnoreResult) (char *query);
    int (*getCostEstimation)(char *query);

    /* cache for catalog information */
    CatalogCache *cache;
    MemContext *metadataLookupContext;

} MetadataLookupPlugin;

#define INVALID_SCN -1

/* store active plugin */
extern MetadataLookupPlugin *activePlugin;
extern List *availablePlugins;

/* plugin handling methods */
extern int initMetadataLookupPlugins (void);
extern int shutdownMetadataLookupPlugins (void);
extern void chooseMetadataLookupPluginFromString (char *plug);
extern void chooseMetadataLookupPlugin (MetadataLookupPluginType plugin);
extern void setMetadataLookupPlugin (MetadataLookupPlugin *p);

/* generic methods */
extern int initMetadataLookupPlugin (void);
extern int shutdownMetadataLookupPlugin (void);
extern int databaseConnectionOpen (void);
extern int databaseConnectionClose(void);
extern boolean isInitialized (void);
extern char *getConnectionDescription (void);

extern boolean catalogTableExists (char * tableName);
extern boolean catalogViewExists (char * viewName);
extern List *getAttributes (char *tableName);
extern List *getAttributeNames (char *tableName);
extern Node *getAttributeDefaultVal (char *schema, char *tableName, char *attrName);
extern List *getAttributeDataTypes (char *tableName);
extern boolean isAgg(char *functionName);
extern boolean isWindowFunction(char *functionName);
extern DataType getFuncReturnType (char *fName, List *argTypes, boolean *funcExists);
extern DataType getOpReturnType (char *oName, List *argTypes, boolean *opExists);
extern DataType backendSQLTypeToDT (char *sqlType);
extern char * backendDatatypeToSQL (DataType dt);
extern char *getTableDefinition(char *tableName);
extern char *getViewDefinition(char *viewName);
extern List *getKeyInformation (char *tableName);

extern void getTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Relation *executeQuery (char *sql);
extern void executeQueryIgnoreResult (char *sql);
extern gprom_long_t getCommitScn (char *tableName, gprom_long_t maxScn, char *xid);
extern Node *executeAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
extern int getCostEstimation(char *query);

/* helper functions for createing the cache */
extern CatalogCache *createCache(void);

//extern boolean isPostive(char *tableName, char *colName);
extern Constant *transferRawData(char *data, char *dataType);
extern HashMap *getMinAndMax(char *tableName, char *colName);
extern HashMap *getHistogram(char *tableName, char *colName);
extern int getRowNum(char* tableName);
extern int getDistinct(char *tableName, char *colName);
extern List *getHist(char *tableName, char *colName, char *numPartitions);
extern char *get2DHist(char *tableName, char *colName, char *colName2, char *numPartitions, char *numPartitions2);
extern void storeInterval(char *tableName, char *attrName, char *numPartitions);
extern char *join2Hist(char *hist1, char *hist2, char *tableName, char *attrName);
extern List *computeSum(char *tableName, char *attrName, char *sumAttrName);
#endif /* METADATA_LOOKUP_H_ */
