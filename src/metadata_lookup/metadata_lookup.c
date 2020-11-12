/*-----------------------------------------------------------------------------
 *
 * metadata_lookup.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "metadata_lookup/metadata_lookup_postgres.h"
#include "metadata_lookup/metadata_lookup_external.h"
#include "metadata_lookup/metadata_lookup_sqlite.h"
#include "metadata_lookup/metadata_lookup_monetdb.h"

#define PLUGIN_NAME_ORACLE "oracle"
#define PLUGIN_NAME_POSTGRES "postgres"
#define PLUGIN_NAME_SQLITE "sqlite"
#define PLUGIN_NAME_MONETDB "monetdb"
#define PLUGIN_NAME_EXTERNAL "external"

MetadataLookupPlugin *activePlugin = NULL;
List *availablePlugins = NIL;

static MetadataLookupPluginType stringToPluginType(char *type);
static char *pluginTypeToString(MetadataLookupPluginType type);

/* create list of available plugins */
int
initMetadataLookupPlugins (void)
{
    availablePlugins = NIL;

// only assemble plugins for which the library is available
#if HAVE_ORACLE_BACKEND
    availablePlugins = appendToTailOfList(availablePlugins, assembleOracleMetadataLookupPlugin());
#endif
#ifdef HAVE_POSTGRES_BACKEND
    availablePlugins = appendToTailOfList(availablePlugins, assemblePostgresMetadataLookupPlugin());
#endif
#if HAVE_SQLITE_BACKEND
    availablePlugins = appendToTailOfList(availablePlugins, assembleSqliteMetadataLookupPlugin());
#endif
#if HAVE_MONETDB_BACKEND
    availablePlugins = appendToTailOfList(availablePlugins, assembleMonetdbMetadataLookupPlugin());
#endif
    availablePlugins = appendToTailOfList(availablePlugins, assembleExternalMetadataLookupPlugin(NULL));



    return EXIT_SUCCESS;
}

int
shutdownMetadataLookupPlugins (void)
{
    FOREACH(MetadataLookupPlugin,p,availablePlugins)
    {
        if (p->isInitialized())
            p->shutdownMetadataLookupPlugin();
    }
    FOREACH(MetadataLookupPlugin,p,availablePlugins)
    {
        FREE(p);
    }
    freeList(availablePlugins);
    availablePlugins = NIL;

    return EXIT_SUCCESS;
}

/* choosePlugins */
void
chooseMetadataLookupPluginFromString (char *plug)
{
    chooseMetadataLookupPlugin(stringToPluginType(plug));
}

void
chooseMetadataLookupPlugin (MetadataLookupPluginType plugin)
{
    FOREACH(MetadataLookupPlugin,p,availablePlugins)
    {
        if (p->type == plugin)
        {
            activePlugin = p;
            if (!(p->isInitialized()))
                p->initMetadataLookupPlugin();
            INFO_LOG("PLUGIN metadatalookup: <%s>", pluginTypeToString(plugin));

            return;
        }
    }
    FATAL_LOG("did not find plugin <%s>", MetadataLookupPluginTypeToString(plugin));
}

void
setMetadataLookupPlugin (MetadataLookupPlugin *p)
{
	activePlugin = p;
	if (!(p->isInitialized()))
		p->initMetadataLookupPlugin();

}

static MetadataLookupPluginType
stringToPluginType(char *type)
{
    if (strcmp(type, PLUGIN_NAME_ORACLE) == 0)
        return METADATA_LOOKUP_PLUGIN_ORACLE;
    if (strcmp(type, PLUGIN_NAME_POSTGRES) == 0)
        return METADATA_LOOKUP_PLUGIN_POSTGRES;
    if (strcmp(type, PLUGIN_NAME_SQLITE) == 0)
        return METADATA_LOOKUP_PLUGIN_SQLITE;
    if (strcmp(type, PLUGIN_NAME_MONETDB) == 0)
        return METADATA_LOOKUP_PLUGIN_MONETDB;
    if (strcmp(type, PLUGIN_NAME_EXTERNAL) == 0)
        return METADATA_LOOKUP_PLUGIN_EXTERNAL;
    FATAL_LOG("unkown plugin type <%s>", type);
    return METADATA_LOOKUP_PLUGIN_ORACLE;
}

static char *
pluginTypeToString(MetadataLookupPluginType type)
{
    switch(type)
    {
    case METADATA_LOOKUP_PLUGIN_ORACLE:
        return PLUGIN_NAME_ORACLE;
    case METADATA_LOOKUP_PLUGIN_POSTGRES:
        return PLUGIN_NAME_POSTGRES;
    case METADATA_LOOKUP_PLUGIN_SQLITE:
        return PLUGIN_NAME_SQLITE;
    case METADATA_LOOKUP_PLUGIN_MONETDB:
            return PLUGIN_NAME_MONETDB;
    case METADATA_LOOKUP_PLUGIN_EXTERNAL:
        return PLUGIN_NAME_EXTERNAL;
    }
    THROW(SEVERITY_RECOVERABLE, "unkown plugin type <%u>", type);
    return NULL; //keep compiler quiet
}


/* wrappers to plugin methods */
int
initMetadataLookupPlugin (void)
{
    ASSERT(activePlugin);

	activePlugin->metadataLookupContext = NEW_LONGLIVED_MEMCONTEXT("METADATA_LOOKUP_PLUGIN_CONTEXT");
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    int returnVal = activePlugin->initMetadataLookupPlugin();
    RELEASE_MEM_CONTEXT();

    return returnVal;
}

int
shutdownMetadataLookupPlugin (void)
{
    ASSERT(activePlugin && activePlugin->isInitialized());

    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    int resultVal = activePlugin->shutdownMetadataLookupPlugin();
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    activePlugin->metadataLookupContext = NULL;

    return resultVal;
}

boolean
isInitialized (void)
{
    ASSERT(activePlugin);
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    boolean result = activePlugin->isInitialized();
    RELEASE_MEM_CONTEXT();
    return result;
}

char *
getConnectionDescription (void)
{
    ASSERT(activePlugin);
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    char *result = activePlugin->connectionDescription();
    RELEASE_MEM_CONTEXT();
    return result;
}

boolean
catalogTableExists (char * tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    boolean result = activePlugin->catalogTableExists(tableName);
    RELEASE_MEM_CONTEXT();
    return result;
}

boolean
catalogViewExists (char * viewName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    boolean result = activePlugin->catalogViewExists(viewName);
    RELEASE_MEM_CONTEXT();
    return result;
}

List *
getAttributes (char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    List *result = activePlugin->getAttributes(tableName);
    RELEASE_MEM_CONTEXT();
    return result;
}

List *
getAttributeNames (char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    List *result = activePlugin->getAttributeNames(tableName);
    RELEASE_MEM_CONTEXT();
    return result;
}

Node *
getAttributeDefaultVal (char *schema, char *tableName, char *attrName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    Node *result = activePlugin->getAttributeDefaultVal(schema, tableName,attrName);
    RELEASE_MEM_CONTEXT();
    return result;
}

List *
getAttributeDataTypes (char *tableName)
{
    //List *result = NIL;
    List *attrs = NIL;
    ASSERT(activePlugin && activePlugin->isInitialized());

    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    attrs = activePlugin->getAttributes(tableName);
    List *result = NIL;
    FOREACH(AttributeDef,a,attrs)
        result = appendToTailOfListInt(result, a->dataType);
    RELEASE_MEM_CONTEXT();
    return result;
}

boolean
isAgg(char *functionName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    boolean result = activePlugin->isAgg(functionName);
    RELEASE_MEM_CONTEXT();
    return result;
}

boolean
isWindowFunction(char *functionName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    boolean result = activePlugin->isWindowFunction(functionName);
    RELEASE_MEM_CONTEXT();
    return result;
}

DataType
getFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    DataType result = activePlugin->getFuncReturnType(fName, argTypes, funcExists);
    RELEASE_MEM_CONTEXT();
    return result;
}

DataType
getOpReturnType (char *oName, List *argTypes, boolean *funcExists)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    DataType result = activePlugin->getOpReturnType(oName, argTypes, funcExists);
    RELEASE_MEM_CONTEXT();
    return result;
}

DataType
backendSQLTypeToDT (char *sqlType)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    DataType result = activePlugin->sqlTypeToDT(sqlType);
    RELEASE_MEM_CONTEXT();
    return result;
}

char *
backendDatatypeToSQL (DataType dt)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    char *result = activePlugin->dataTypeToSQL(dt);
    RELEASE_MEM_CONTEXT();
    return result;
}


char *
getTableDefinition(char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    char *result = activePlugin->getTableDefinition(tableName);
    RELEASE_MEM_CONTEXT();
    return result;
}

char *
getViewDefinition(char *viewName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    char *result = activePlugin->getViewDefinition(viewName);
    RELEASE_MEM_CONTEXT();
    return result;
}

List *
getKeyInformation (char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    List *result = activePlugin->getKeyInformation(tableName);
    RELEASE_MEM_CONTEXT();
    return result;
}

void
getTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    activePlugin->getTransactionSQLAndSCNs(xid, scns, sqls, sqlBinds, iso, commitScn);
    RELEASE_MEM_CONTEXT();
    //return result;
}

Relation *
executeQuery (char *sql)
{
    //ASSERT(activePlugin && activePlugin->isInitialized() && activePlugin->executeQuery);
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    Relation *result = activePlugin->executeQuery(sql);
    RELEASE_MEM_CONTEXT();
    return result;
}

void
executeQueryIgnoreResult (char *sql)
{
    ASSERT(activePlugin && activePlugin->isInitialized() && activePlugin->executeQuery);
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    activePlugin->executeQueryIgnoreResult(sql);
    RELEASE_MEM_CONTEXT();
}

gprom_long_t
getCommitScn (char *tableName, gprom_long_t maxScn, char *xid)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    gprom_long_t result = activePlugin->getCommitScn(tableName, maxScn, xid);
    RELEASE_MEM_CONTEXT();
    return result;
}

Node *
executeAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    Node *result = activePlugin->executeAsTransactionAndGetXID(statements, isoLevel);
    RELEASE_MEM_CONTEXT();
    return result;
}

int
getCostEstimation(char *query)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    int result = activePlugin->getCostEstimation(query);
    RELEASE_MEM_CONTEXT();
    return result;
}

int
databaseConnectionOpen (void)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    int result = activePlugin->databaseConnectionOpen();
    RELEASE_MEM_CONTEXT();
    return result;
}


int
databaseConnectionClose()
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    int result = activePlugin->databaseConnectionClose();
    RELEASE_MEM_CONTEXT();
    return result;
}

CatalogCache *
createCache(void)
{
    CatalogCache *result = NEW(CatalogCache);

    result->tableAttrs = NEW_MAP(Constant,List);
    result->tableAttrDefs = NEW_MAP(Constant,List);
    result->viewAttrs = NEW_MAP(Constant,List);
    result->viewDefs = NEW_MAP(Constant,Constant);
    result->viewNames = STRSET();
    result->tableNames = STRSET();
    result->aggFuncNames = STRSET();
    result->winFuncNames = STRSET();
    result->cacheHook = NULL;

    return result;
}

/*
boolean
isPostive(char *tableName, char *colName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    boolean result = activePlugin->checkPostive(tableName, colName);
    RELEASE_MEM_CONTEXT();
    return result;
}
*/

Constant*
transferRawData(char *data, char *dataType){
	ASSERT(activePlugin && activePlugin->isInitialized());
	ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
	Constant* result = activePlugin->trasnferRawData(data, dataType);
    RELEASE_MEM_CONTEXT();
    return result;
}

HashMap *
getMinAndMax(char *tableName, char *colName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    HashMap * result = activePlugin->getMinAndMax(tableName, colName);
    RELEASE_MEM_CONTEXT();
    return result;
}

int
getRowNum(char* tableName)
{
	 ASSERT(activePlugin && activePlugin->isInitialized());
	    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
	    int result = activePlugin->getRowNum(tableName);
	    RELEASE_MEM_CONTEXT();
	    return result;

}


int
getDistinct(char *tableName, char *colName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    int result = activePlugin->getDistinct(tableName, colName);
    RELEASE_MEM_CONTEXT();
    return result;
}

List *
getHist(char *tableName, char *colName, char *numPartitions)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    List * result = activePlugin->getHist(tableName, colName, numPartitions);
    RELEASE_MEM_CONTEXT();
    return result;
}

char *
get2DHist(char *tableName, char *colName, char *colName2, char *numPartitions1, char *numPartitions2)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    char * result = activePlugin->get2DHist(tableName, colName, colName2, numPartitions1, numPartitions2);
    RELEASE_MEM_CONTEXT();
    return result;
}

void
storeInterval(char *tableName, char *attrName, char *numPartitions)
{
	ASSERT(activePlugin && activePlugin->isInitialized());
	ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
	activePlugin->storeInterval(tableName, attrName, numPartitions);
    RELEASE_MEM_CONTEXT();
}
char*
join2Hist(char *hist1, char *hist2, char *tableName, char *attrName)
{
	ASSERT(activePlugin && activePlugin->isInitialized());
	ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
	char * result = activePlugin->join2Hist(hist1, hist2, tableName, attrName);
	RELEASE_MEM_CONTEXT();
	return result;
}
List*
computeSum(char *tableName, char *attrName, char *sumAttrName)
{
	ASSERT(activePlugin && activePlugin->isInitialized());
	ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
	List * result = activePlugin->computeSum(tableName, attrName, sumAttrName);
	RELEASE_MEM_CONTEXT();
	return result;
}
