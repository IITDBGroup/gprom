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
        FREE(p);
    }
    freeList(availablePlugins);

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
    FATAL_LOG("did not find plugin");
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
    if (strcmp(type, "oracle") == 0)
        return METADATA_LOOKUP_PLUGIN_ORACLE;
    if (strcmp(type, "postgres") == 0)
        return METADATA_LOOKUP_PLUGIN_POSTGRES;
    if (strcmp(type, "external") == 0)
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
        return "oracle";
    case METADATA_LOOKUP_PLUGIN_POSTGRES:
        return "postgres";
    case METADATA_LOOKUP_PLUGIN_EXTERNAL:
        return "external";
    }
    return NULL; //keep compiler quiet
}


/* wrappers to plugin methods */
int
initMetadataLookupPlugin (void)
{
    //MetadataLookupPlugin *myPlugin;
    ASSERT(activePlugin);

    //activePlugin->metadataLookupContext = CREATE_MEM_CONTEXT("METADATA_LOOKUP_PLUGIN_CONTEXT");
	activePlugin->metadataLookupContext = NEW_MEM_CONTEXT("METADATA_LOOKUP_PLUGIN_CONTEXT");
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
getFuncReturnType (char *fName, List *argTypes)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    DataType result = activePlugin->getFuncReturnType(fName, argTypes);
    RELEASE_MEM_CONTEXT();
    return result;
}

DataType
getOpReturnType (char *oName, List *argTypes)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    DataType result = activePlugin->getOpReturnType(oName, argTypes);
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
    ASSERT(activePlugin && activePlugin->isInitialized() && activePlugin->executeQuery);
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    Relation *result = activePlugin->executeQuery(sql);
    RELEASE_MEM_CONTEXT();
    return result;
}

long
getCommitScn (char *tableName, long maxScn, char *xid)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    ACQUIRE_MEM_CONTEXT(activePlugin->metadataLookupContext);
    long result = activePlugin->getCommitScn(tableName, maxScn, xid);
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
