/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_postgres.c
 *			  
 *		- Catalog lookup for postgres database
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */


#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"

#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_postgres.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"

#if HAVE_LIBPQ_FE_H
#include "libpq-fe.h"
#endif


#define CONTEXT_NAME "PostgresMemContext"


#ifdef HAVE_LIBPQ

// extends MetadataLookupPlugin with postgres connection
typedef struct PostgresPlugin
{
    MetadataLookupPlugin plugin;
    PGconn *conn;
    boolean initialized;
} PostgresPlugin;

static PostgresPlugin *plugin = NULL;
static MemContext *memContext = NULL;


MetadataLookupPlugin *
assemblePostgresMetadataLookupPlugin (void)
{
    plugin = NEW(PostgresPlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_POSTGRES;

    p->initMetadataLookupPlugin = postgresInitMetadataLookupPlugin;
    p->databaseConnectionOpen = postgresDatabaseConnectionOpen;
    p->databaseConnectionClose = postgresDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = postgresShutdownMetadataLookupPlugin;
    p->isInitialized = postgresIsInitialized;
    p->catalogTableExists = postgresCatalogTableExists;
    p->catalogViewExists = postgresCatalogViewExists;
    p->getAttributes = postgresGetAttributes;
    p->getAttributeNames = postgresGetAttributeNames;
    p->isAgg = postgresIsAgg;
    p->isWindowFunction = postgresIsWindowFunction;
    p->getTableDefinition = postgresGetTableDefinition;
    p->getViewDefinition = postgresGetViewDefinition;
    p->getTransactionSQLAndSCNs = postgresGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = postgresExecuteAsTransactionAndGetXID;

    return p;
}

/* plugin methods */
int
postgresInitMetadataLookupPlugin (void)
{
    NEW_AND_ACQUIRE_MEMCONTEXT(CONTEXT_NAME);
    memContext = getCurMemContext();

    plugin->plugin.cache = createCache();

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
postgresShutdownMetadataLookupPlugin (void)
{
    ACQUIRE_MEM_CONTEXT(memContext);

    // clear cache

    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
postgresDatabaseConnectionOpen (void)
{
    StringInfo connStr = makeStringInfo();
    OptionConnection *op = getOptions()->optionConnection;

    ACQUIRE_MEM_CONTEXT(memContext);

    /* create connection string */
    if (op->host)
        appendStringInfo(connStr, " host=%s", op->host);
    if (op->db)
        appendStringInfo(connStr, " dbname=%s", op->db);
    if (op->user)
        appendStringInfo(connStr, " user=%s", op->user);
    if (op->passwd)
        appendStringInfo(connStr, " password=%s", op->passwd);
    if (op->port)
        appendStringInfo(connStr, " port=%u", op->port);

    /* try to connect to db */
    plugin->conn = PQconnectdb(connStr->data);

    /* check to see that the backend connection was successfully made */
    if (plugin->conn == NULL || PQstatus(plugin->conn) == CONNECTION_BAD)
    {
        char *error = PQerrorMessage(plugin->conn);
        PQfinish(plugin->conn);
        FATAL_LOG("unable to connect to postgres database %s\n\nfailed "
                "because of:\n%s", connStr->data, error);
    }

    plugin->initialized = TRUE;

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
postgresDatabaseConnectionClose()
{
    ACQUIRE_MEM_CONTEXT(memContext);
    ASSERT(plugin && plugin->initialized);

    PQfinish(plugin->conn);

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

boolean
postgresIsInitialized (void)
{
    if (plugin && plugin->initialized)
    {
        if (plugin->conn == NULL || PQstatus(plugin->conn) == CONNECTION_BAD)
        {
            char *error = PQerrorMessage(plugin->conn);
            ERROR_LOG("unable to connect to postgres database\nfailed "
                    "because of:\n%s", error);
            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

boolean
postgresCatalogTableExists (char * tableName)
{
    if (hasSetElem(plugin->plugin.cache->tableNames,tableName))
        return TRUE;


    // run query
    return FALSE;
}

boolean
postgresCatalogViewExists (char * viewName)
{
    if (hasSetElem(plugin->plugin.cache->viewNames,viewName))
        return TRUE;


    // run query
    return FALSE;
}


List *
postgresGetAttributes (char *tableName)
{
    List *attrs = NIL;
    ASSERT(hasSetElem(plugin->plugin.cache->tableNames,tableName));

    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrDefs, tableName))
        return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrDefs,tableName);

    // do query
    return attrs;
}

List *
postgresGetAttributeNames (char *tableName)
{
    List *attrs = NIL;
    ASSERT(hasSetElem(plugin->plugin.cache->tableNames,tableName));

    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName))
        return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrs,tableName);

    // do query
    return attrs;
}

boolean
postgresIsAgg(char *functionName)
{
    if (hasSetElem(plugin->plugin.cache->aggFuncNames, functionName))
        return TRUE;

    // do query

    return FALSE;
}

boolean
postgresIsWindowFunction(char *functionName)
{
    if (hasSetElem(plugin->plugin.cache->winFuncNames, functionName))
        return TRUE;

    // do query

    return FALSE;
}

char *
postgresGetTableDefinition(char *tableName)
{
    return tableName;
}

char *
postgresGetViewDefinition(char *viewName)
{
    return viewName;
}

void
postgresGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
    FATAL_LOG("not supported for postgres yet");
}

Node *
postgresExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    Constant *xid = NULL;

    return (Node *) xid;
}

// NO libpq present. Provide dummy methods to keep compiler quiet
#else

MetadataLookupPlugin *
assemblePostgresMetadataLookupPlugin (void)
{
    return NULL;
}

int
postgresInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
postgresShutdownMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
postgresDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}

int
postgresDatabaseConnectionClose()
{
    return EXIT_SUCCESS;
}

boolean
postgresIsInitialized (void)
{
    return FALSE;
}

boolean
postgresCatalogTableExists (char * tableName)
{
    return FALSE;
}

boolean
postgresCatalogViewExists (char * viewName)
{
    return FALSE;
}

List *
postgresGetAttributes (char *tableName)
{
    return NIL;
}

List *
postgresGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
postgresIsAgg(char *functionName)
{
    return FALSE;
}

boolean
postgresIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
postgresGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
postgresGetViewDefinition(char *viewName)
{
    return NULL;
}

void
postgresGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
postgresExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

#endif

