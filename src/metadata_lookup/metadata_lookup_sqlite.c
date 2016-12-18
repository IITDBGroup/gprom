/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_sqlite.c
 *			  
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
#include "metadata_lookup/metadata_lookup_sqlite.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"


// Mem context
#define CONTEXT_NAME "SQLiteMemContext"

// query templates
#define QUERY_TABLE_COL_COUNT "PRAGMA table_info(%s)"

// Only define real plugin structure and methods if libsqlite3 is present
#ifdef HAVE_SQLITE_BACKEND

// extends MetadataLookupPlugin with sqlite specific information
typedef struct SQlitePlugin
{
    MetadataLookupPlugin plugin;
    boolean initialized;
    sqlite3 *conn;
} SQlitePlugin;

// global vars
static SQlitePlugin *plugin = NULL;
static MemContext *memContext = NULL;

// functions
static sqlite3_stmt *runQuery (char *q);
static DataType stringToDT (char *dataType);
static char *sqliteGetConnectionDescription (void);
static void initCache(CatalogCache *c);

#define HANDLE_ERROR_MSG(_rc,_expected,_message, ...) \
    do { \
        if (_rc != _expected) \
        { \
            StringInfo _newmes = makeStringInfo(); \
            appendStringInfo(_newmes, _message, ##__VA_ARGS__); \
            FATAL_LOG("error (%s)\n%u\n\n%s", _rc, strdup((char *) sqlite3_errmsg(plugin->conn)), _newmes->data); \
        } \
    } while(0)


MetadataLookupPlugin *
assembleSqliteMetadataLookupPlugin (void)
{
    plugin = NEW(SQlitePlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_SQLITE;

    p->initMetadataLookupPlugin = sqliteInitMetadataLookupPlugin;
    p->databaseConnectionOpen = sqliteDatabaseConnectionOpen;
    p->databaseConnectionClose = sqliteDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = sqliteShutdownMetadataLookupPlugin;
    p->isInitialized = sqliteIsInitialized;
    p->catalogTableExists = sqliteCatalogTableExists;
    p->catalogViewExists = sqliteCatalogViewExists;
    p->getAttributes = sqliteGetAttributes;
    p->getAttributeNames = sqliteGetAttributeNames;
    p->isAgg = sqliteIsAgg;
    p->isWindowFunction = sqliteIsWindowFunction;
    p->getFuncReturnType = sqliteGetFuncReturnType;
    p->getOpReturnType = sqliteGetOpReturnType;
    p->getTableDefinition = sqliteGetTableDefinition;
    p->getViewDefinition = sqliteGetViewDefinition;
    p->getTransactionSQLAndSCNs = sqliteGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = sqliteExecuteAsTransactionAndGetXID;
    p->getCostEstimation = sqliteGetCostEstimation;
    p->getKeyInformation = sqliteGetKeyInformation;
    p->executeQuery = sqliteExecuteQuery;
    p->connectionDescription = sqliteGetConnectionDescription;

    return p;
}


/* plugin methods */
int
sqliteInitMetadataLookupPlugin (void)
{
    if (plugin && plugin->initialized)
    {
        INFO_LOG("tried to initialize metadata lookup plugin more than once");
        return EXIT_SUCCESS;
    }
    memContext = getCurMemContext();

    // create cache
    plugin->plugin.cache = createCache();
    initCache(plugin->plugin.cache);

    plugin->initialized = TRUE;

    return EXIT_SUCCESS;
}

int
sqliteShutdownMetadataLookupPlugin (void)
{

    // clear cache
    //TODO

    return EXIT_SUCCESS;
}

int
sqliteDatabaseConnectionOpen (void)
{
    char *dbfile = getStringOption("connection.db");
    int rc;
    if (dbfile == NULL)
        FATAL_LOG("no database file given (<connection.db> parameter)");

    rc = sqlite3_open(dbfile, &(plugin->conn));
    if(rc != SQLITE_OK)
    {
          HANDLE_ERROR_MSG(rc, SQLITE_OK, "Can not open database <%s>", dbfile);
          sqlite3_close(plugin->conn);
          return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
sqliteDatabaseConnectionClose()
{
    int rc;
    rc = sqlite3_close(plugin->conn);

    HANDLE_ERROR_MSG(rc, SQLITE_OK, "Can not close database");

    return EXIT_SUCCESS;
}

boolean
sqliteIsInitialized (void)
{
    if (plugin && plugin->initialized)
    {
        if (plugin->conn == NULL)
        {
            if (sqliteDatabaseConnectionOpen() != EXIT_SUCCESS)
                return FALSE;
        }
//        if (PQstatus(plugin->conn) == CONNECTION_BAD)
//        {
//            char *error = PQerrorMessage(plugin->conn);
//            ERROR_LOG("unable to connect to postgres database\nfailed "
//                    "because of:\n%s", error);
//            return FALSE;
//        }

        return TRUE;
    }

    return FALSE;
}

boolean
sqliteCatalogTableExists (char * tableName)
{
    sqlite3 *c = plugin->conn;
    boolean res = (sqlite3_table_column_metadata(c,NULL,tableName,NULL,NULL,NULL,NULL,NULL, NULL) == SQLITE_OK);

    return res;//TODO
}

boolean
sqliteCatalogViewExists (char * viewName)
{
    return FALSE;//TODO
}

List *
sqliteGetAttributes (char *tableName)
{
    sqlite3_stmt *rs;
    StringInfo q;
    List *result = NIL;
    int rc;

    q = makeStringInfo();
    appendStringInfo(q, QUERY_TABLE_COL_COUNT, tableName);
    rs = runQuery(q->data);

    while((rc = sqlite3_step(rs)) == SQLITE_ROW)
    {
        const unsigned char *colName = sqlite3_column_text(rs,1);
        const unsigned char *dt = sqlite3_column_text(rs,2);
        DataType ourDT = stringToDT((char *) dt);
                ;
        AttributeDef *a = createAttributeDef(
                         strdup((char *) colName),
                         ourDT
                         );
        result = appendToTailOfList(result, a);
    }

    HANDLE_ERROR_MSG(rc, SQLITE_DONE, "error getting attributes of table <%s>", tableName);

    return result;
}

List *
sqliteGetAttributeNames (char *tableName)
{
    return getAttrDefNames(sqliteGetAttributes(tableName));
}

boolean
sqliteIsAgg(char *functionName)
{
    char *f = strdup(functionName);
//    int i = 0;

    for(char *p = f; *p != '\0'; *(p) = tolower(*p), p++)
        ;

    if (hasSetElem(plugin->plugin.cache->aggFuncNames, f))
        return TRUE;

    return FALSE;
}

boolean
sqliteIsWindowFunction(char *functionName)
{
    return FALSE;//TODO
}

DataType
sqliteGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
    *funcExists = TRUE;
    return DT_STRING; //TODO
}

DataType
sqliteGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{
    //TODO
    *opExists = TRUE;
    return DT_STRING;
}

char *
sqliteGetTableDefinition(char *tableName)
{
    return NULL;//TODO
}

char *
sqliteGetViewDefinition(char *viewName)
{
    return NULL;//TODO
}

int
sqliteGetCostEstimation(char *query)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return -1;
}

List *
sqliteGetKeyInformation(char *tableName)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return NIL;
}

void
sqliteGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
}

Node *
sqliteExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return NULL;
}



Relation *
sqliteExecuteQuery(char *query)
{
    Relation *r = makeNode(Relation);
    sqlite3_stmt *rs = runQuery(query);
    int numFields = sqlite3_column_count(rs);
    int rc = SQLITE_OK;

    // set schema
    r->schema = NIL;
    for(int i = 0; i < numFields; i++)
    {
        const char *name = sqlite3_column_name(rs, i);
        r->schema = appendToTailOfList(r->schema, strdup((char *) name));
    }

    // read rows
    r->tuples = NIL;
    while((rc = sqlite3_step(rs)) == SQLITE_ROW)
    {
        List *tuple = NIL;
        for (int j = 0; j < numFields; j++)
        {
            if (sqlite3_column_type(rs,j) == SQLITE_NULL)
            {
                tuple = appendToTailOfList(tuple, strdup("NULL"));
            }
            else
            {
                const unsigned char *val = sqlite3_column_text(rs,j);
                tuple = appendToTailOfList(tuple, strdup((char *) val));
            }
        }
        r->tuples = appendToTailOfList(r->tuples, tuple);
        DEBUG_LOG("read tuple <%s>", stringListToString(tuple));
    }

    HANDLE_ERROR_MSG(rc,SQLITE_DONE, "failed to execute query <%s>", query);

    rc = sqlite3_finalize(rs);
    HANDLE_ERROR_MSG(rc,SQLITE_OK, "failed to finalize query <%s>", query);

    return r;
}

static sqlite3_stmt *
runQuery (char *q)
{
    sqlite3 *conn = plugin->conn;
    sqlite3_stmt *stmt;
    int rc;

    DEBUG_LOG("run query:\n<%s>", q);
    rc = sqlite3_prepare(conn, q, -1, &stmt, NULL);
    HANDLE_ERROR_MSG(rc, SQLITE_OK, "failed to prepare query <%s>", q);

    return stmt;
}


static DataType
stringToDT (char *dataType)
{
   DEBUG_LOG("data type %s", dataType);

   if (streq(dataType, "NUMERIC") || streq(dataType, "REAL"))//TODO
       return DT_FLOAT;
   if (streq(dataType, "int"))
       return DT_INT;

   return DT_STRING;
}

static char *
sqliteGetConnectionDescription (void)
{
    return CONCAT_STRINGS("SQLite:", getStringOption("connection.db"));
}

#define ADD_AGGR_FUNC(name) addToSet(plugin->plugin.cache->aggFuncNames, strdup(name));
static void
initCache(CatalogCache *c)
{
    ADD_AGGR_FUNC("avg");
    ADD_AGGR_FUNC("count");
    ADD_AGGR_FUNC("group_concat");
    ADD_AGGR_FUNC("max");
    ADD_AGGR_FUNC("min");
    ADD_AGGR_FUNC("sum");
    ADD_AGGR_FUNC("total");
}


#else

MetadataLookupPlugin *
assembleSqliteMetadataLookupPlugin (void)
{
    return NULL;
}

int
sqliteInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
sqliteShutdownMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
sqliteDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}

int
sqliteDatabaseConnectionClose()
{
    return EXIT_SUCCESS;
}

boolean
sqliteIsInitialized (void)
{
    return FALSE;
}

boolean
sqliteCatalogTableExists (char * tableName)
{
    return FALSE;
}

boolean
sqliteCatalogViewExists (char * viewName)
{
    return FALSE;
}

List *
sqliteGetAttributes (char *tableName)
{
    return NIL;
}

List *
sqliteGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
sqliteIsAgg(char *functionName)
{
    return FALSE;
}

boolean
sqliteIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
sqliteGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
sqliteGetViewDefinition(char *viewName)
{
    return NULL;
}

void
sqliteGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
sqliteExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
sqliteGetCostEstimation(char *query)
{
    return 0;
}

List *
sqliteGetKeyInformation(char *tableName)
{
    return NULL;
}

Relation *
sqliteExecuteQuery(char *query)
{
    return NULL;
}

#endif


