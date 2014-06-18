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
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "metadata_lookup/metadata_lookup_postgres.h"

MetadataLookupPlugin *activePlugin = NULL;
List *availablePlugins = NIL;

/* create list of available plugins */
int
initMetadataLookupPlugins (void)
{

// only assemble plugins for which the library is available
#if HAVE_ORACLE_BACKEND
    availablePlugins = appendToTailOfList(availablePlugins, assembleOracleMetadataLookupPlugin());
#endif
#ifdef HAVE_POSTGRES_BACKEND
    availablePlugins = appendToTailOfList(availablePlugins, assemblePostgresMetadataLookupPlugin());
#endif

    return EXIT_SUCCESS;
}

int
shutdownMetadataLookupPlugins (void)
{
    FOREACH(MetadataLookupPlugin,p,availablePlugins)
    {
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
    if (strcmp(plug, "oracle") == 0)
    {
        chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_ORACLE);
        return;
    }
    if (strcmp(plug, "postgres") == 0)
    {
        chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_POSTGRES);
        return;
    }
    FATAL_LOG("unkown plugin type: %s", plug);
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
            return;
        }
    }
    FATAL_LOG("did not find plugin");
}

/* wrappers to plugin methods */
int
initMetadataLookupPlugin (void)
{
    ASSERT(activePlugin);
    return activePlugin->initMetadataLookupPlugin();
}

int
shutdownMetadataLookupPlugin (void)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->shutdownMetadataLookupPlugin();
}

boolean
isInitialized (void)
{
    ASSERT(activePlugin);
    return activePlugin->isInitialized();
}


boolean
catalogTableExists (char * tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->catalogTableExists(tableName);
}

boolean
catalogViewExists (char * viewName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->catalogViewExists(viewName);
}

List *
getAttributes (char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->getAttributes(tableName);
}

List *
getAttributeNames (char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->getAttributeNames(tableName);
}

boolean
isAgg(char *functionName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->isAgg(functionName);
}

boolean
isWindowFunction(char *functionName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->isWindowFunction(functionName);
}

char *
getTableDefinition(char *tableName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->getTableDefinition(tableName);
}

char *
getViewDefinition(char *viewName)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->getViewDefinition(viewName);
}

void
getTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->getTransactionSQLAndSCNs(xid, scns, sqls, sqlBinds, iso, commitScn);
}

long
getCommitScn (char *tableName, long maxScn, char *xid)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->getCommitScn(tableName, maxScn, xid);
}

Node *
executeAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->executeAsTransactionAndGetXID(statements, isoLevel);
}

int
databaseConnectionOpen (void)
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->databaseConnectionOpen();
}


int
databaseConnectionClose()
{
    ASSERT(activePlugin && activePlugin->isInitialized());
    return activePlugin->databaseConnectionClose();
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

    return result;
}
