/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_monetdb.c
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
#include "metadata_lookup/metadata_lookup_monetdb.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"


#ifdef HAVE_MONETDB_BACKEND
#include <monetdb/mapi.h>

// extends MetadataLookupPlugin with postgres connection
typedef struct MonetDBPlugin
{
    MetadataLookupPlugin plugin;
    boolean initialized;
    Mapi dbConn;
} MonetDBPlugin;

#define TABLE_EXISTS_QUERY "SELECT DISTINCT 1 AS e FROM sys.tables t, sys.table_types p WHERE name = '?' AND p.table_type_name = 'TABLE' AND p.table_type_id = t.type;"
#define VIEW_EXISTS_QUERY "SELECT DISTINCT 1 AS e FROM sys.tables t, sys.table_types p WHERE name = '?' AND p.table_type_name = 'VIEW' AND p.table_type_id = t.type;"
#define GET_TABLE_COL_QUERY "SELECT c.name, c.type FROM sys.tables t, sys.columns c WHERE t.name = '?' AND t.id = c.table_id ORDER BY c.number;"
#define FUNC_IS_AGG_QUERY "SELECT DISTINCT 1 AS e FROM sys.functions WHERE mod = 'aggr' AND name = '?';"
#define TABLE_GET_KEY_QUERY "SELECT o.name FROM sys.keys k, sys.objects o, sys.tables t WHERE o.id = k.id AND k.table_id = t.id AND t.name = '?' ORDER BY nr;"

// plugin
static MonetDBPlugin *plugin;

// static methods
static void handleConnectionError (void);
static void handleResultSetError (MapiHdl handle, char *query, List *parameters);
static void handleResultSetErrorNoQuery (MapiHdl handle);
static MapiHdl executeParamQuery (char *query, char *params, ...);
static MapiHdl executeQueryInternal (char *query);
static DataType monetdbTypeToDT(char *dt);
static char *monetdbGetConnectionDescription (void);

#define EXEC_PARAM_QUERY(query, ...) executeParamQuery(query, __VA_ARGS__, NULL)

#define THROW_ON_ERROR(handle, mes) \
    do { \
        if (result == NULL || mapi_error(plugin->dbConn) != MOK) \
        { \
            handleResultSetErrorNoQuery(result); \
            THROW(SEVERITY_RECOVERABLE, mes); \
        } \
    } while(0)

MetadataLookupPlugin *
assembleMonetdbMetadataLookupPlugin (void)
{
    plugin = NEW(MonetDBPlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_MONETDB;

    p->initMetadataLookupPlugin = monetdbInitMetadataLookupPlugin;
    p->databaseConnectionOpen = monetdbDatabaseConnectionOpen;
    p->databaseConnectionClose = monetdbDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = monetdbShutdownMetadataLookupPlugin;
    p->isInitialized = monetdbIsInitialized;
    p->catalogTableExists = monetdbCatalogTableExists;
    p->catalogViewExists = monetdbCatalogViewExists;
    p->getAttributes = monetdbGetAttributes;
    p->getAttributeNames = monetdbGetAttributeNames;
    p->isAgg = monetdbIsAgg;
    p->isWindowFunction = monetdbIsWindowFunction;
    p->getFuncReturnType = monetdbGetFuncReturnType;
    p->getOpReturnType = monetdbGetOpReturnType;
    p->getTableDefinition = monetdbGetTableDefinition;
    p->getViewDefinition = monetdbGetViewDefinition;
    p->getTransactionSQLAndSCNs = monetdbGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = monetdbExecuteAsTransactionAndGetXID;
    p->getCostEstimation = monetdbGetCostEstimation;
    p->getKeyInformation = monetdbGetKeyInformation;
    p->executeQuery = monetdbExecuteQuery;
    p->executeQueryIgnoreResult = monetdbExecuteQueryIgnoreResults;
    p->connectionDescription = monetdbGetConnectionDescription;

    plugin->dbConn = NULL;
    plugin->initialized = FALSE;

    return p;
}
int
monetdbInitMetadataLookupPlugin (void)
{
    if (plugin->initialized)
        return EXIT_SUCCESS;

    plugin->dbConn = NULL;
    plugin->initialized = TRUE;

    return EXIT_SUCCESS;
}

int
monetdbShutdownMetadataLookupPlugin (void)
{
    if (plugin != NULL && plugin->dbConn != NULL)
        FREE(plugin->dbConn);
    plugin->dbConn = NULL;

    return EXIT_SUCCESS;
}

static void
handleConnectionError (void)
{
    if(plugin->dbConn != NULL)
    {
        const char *error = mapi_error_str(plugin->dbConn);
        ERROR_LOG("mapi error:\n%s", error);
        mapi_destroy(plugin->dbConn);
    }
    else
        ERROR_LOG("mapi is NULL");
    //TODO throw error
}

static void
handleResultSetError (MapiHdl handle, char *query, List *parameters)
{
    ERROR_LOG("error in query execution of %s<%s>", query,
            stringListToString(parameters));

    if (plugin->dbConn != NULL)
    {
        if (handle != NULL)
        {
            mapi_explain_query(handle, stderr);
            do {
                const char *e = mapi_result_error(handle);
                if (e != NULL)
                    ERROR_LOG("mapi query error:\n%s", e);
            } while (mapi_next_result(handle) == 1);
            mapi_close_handle(handle);
            mapi_destroy(plugin->dbConn);
        }
    }
    //TODO throw error
}

static void
handleResultSetErrorNoQuery (MapiHdl handle)
{
    if (plugin->dbConn != NULL)
    {
        if (handle != NULL)
        {
            mapi_explain_query(handle, stderr);
            do {
                const char *e = mapi_result_error(handle);
                if (e != NULL)
                    ERROR_LOG("mapi query error:\n%s", e);
            } while (mapi_next_result(handle) == 1);
            mapi_close_handle(handle);
            mapi_destroy(plugin->dbConn);
        }
    }
}

int
monetdbDatabaseConnectionOpen (void)
{
    char *host, *db, *user, *passwd;
    int port;

    host = getStringOption("connection.host");
    db =  getStringOption("connection.db");
    user = getStringOption("connection.user");
    if (optionSet("connection.passwd"))
        passwd = optionSet("connection.passwd") ? getStringOption("connection.passwd") : NULL;
    port = getIntOption("connection.port");

    plugin->dbConn = mapi_connect(host, port, user, passwd, "sql", db);

    if (mapi_error(plugin->dbConn))
    {
        handleConnectionError();
        ERROR_LOG("failed to connect to MonetDB database %s", monetdbGetConnectionDescription());
        return EXIT_FAILURE;
    }

    DEBUG_LOG("connected to MonetDB database %s", monetdbGetConnectionDescription());

    return EXIT_SUCCESS;
}

int
monetdbDatabaseConnectionClose()
{
    if (plugin->dbConn != NULL)
        mapi_destroy(plugin->dbConn);
    return EXIT_SUCCESS;
}

boolean
monetdbIsInitialized (void)
{
    if (plugin == NULL || ! plugin->initialized)
        return FALSE;

    if (plugin->dbConn == NULL)
    {
        if (monetdbDatabaseConnectionOpen() != EXIT_SUCCESS)
            return FALSE;
    }

    if (plugin->dbConn == NULL || mapi_ping(plugin->dbConn) != MOK)
    {
        handleConnectionError();
        return FALSE;
    }
    return TRUE;
}

boolean
monetdbCatalogTableExists (char * tableName)
{
    MapiHdl result = NULL;
    int numRows = 0;

    result = EXEC_PARAM_QUERY(TABLE_EXISTS_QUERY, tableName);

    THROW_ON_ERROR(result, "error during table exists");

    while(mapi_fetch_row(result))
        numRows++;

    THROW_ON_ERROR(result, "error during table exists");

    mapi_close_handle(result);

    DEBUG_LOG("table %s exists %u", tableName, numRows);

    return (numRows == 1);
}

boolean
monetdbCatalogViewExists (char * viewName)
{
    MapiHdl result = NULL;
    int numRows = 0;

    result = EXEC_PARAM_QUERY(TABLE_EXISTS_QUERY, viewName);

    THROW_ON_ERROR(result, "error during view exists");

    while(mapi_fetch_row(result))
        numRows++;

    THROW_ON_ERROR(result, "error during view exists");

    mapi_close_handle(result);

    DEBUG_LOG("view %s exists %u", viewName, numRows);

    return (numRows == 1);
}

List *
monetdbGetAttributes (char *tableName)
{
    MapiHdl result = NULL;
    List *attrs = NIL;
    result = EXEC_PARAM_QUERY(GET_TABLE_COL_QUERY, tableName);

    THROW_ON_ERROR(result, "get table attributes");

    while(mapi_fetch_row(result))
    {
        AttributeDef *a = createAttributeDef(
                strdup(mapi_fetch_field(result, 0)),
                monetdbTypeToDT(strdup(mapi_fetch_field(result, 1)))
        );
        attrs = appendToTailOfList(attrs, a);
    }

    THROW_ON_ERROR(result, "get table attributes");

    mapi_close_handle(result);

    DEBUG_LOG("table %s has attributes %s", tableName, beatify(nodeToString(attrs)));

    return attrs;
}

static DataType
monetdbTypeToDT(char *dt)
{
    if (streq(dt, "int"))
        return DT_INT;
    if (streq(dt, "varchar"))
        return DT_STRING; //TOTO handle more types

    return DT_STRING;
}

List *
monetdbGetAttributeNames (char *tableName)
{
    List *attrs = monetdbGetAttributes(tableName);
    List *result = NIL;

    FOREACH(AttributeDef,a,attrs)
        result = appendToTailOfList(result, a->attrName);

    return result;
}

boolean
monetdbIsAgg(char *functionName)
{
    MapiHdl result = NULL;
    int numRows = 0;

    result = EXEC_PARAM_QUERY(FUNC_IS_AGG_QUERY, functionName);

    THROW_ON_ERROR(result, "function is agg");

    while(mapi_fetch_row(result))
        numRows++;

    THROW_ON_ERROR(result, "function is agg");

    mapi_close_handle(result);

    DEBUG_LOG("function %s is an aggregate: %u", functionName, numRows);

    return (numRows == 1);
}

boolean
monetdbIsWindowFunction(char *functionName)
{
    return monetdbIsAgg(functionName); //TODO check
}


DataType
monetdbGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
    *funcExists = TRUE;
    return DT_STRING; //TODO
}

DataType
monetdbGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{

    *opExists = TRUE;

    if (streq(oName, "+") || streq(oName, "*")  || streq(oName, "-") || streq(oName, "/"))
    {
        if (LIST_LENGTH(argTypes) == 2)
        {
            DataType lType = getNthOfListInt(argTypes, 0);
            DataType rType = getNthOfListInt(argTypes, 1);

            if (lType == rType)
            {
                if (lType == DT_INT)
                    return DT_INT;
                if (lType == DT_FLOAT)
                    return DT_FLOAT;
            }
        }
    }

    if (streq(oName, "||"))
    {
        DataType lType = getNthOfListInt(argTypes, 0);
        DataType rType = getNthOfListInt(argTypes, 1);

        if (lType == rType && lType == DT_STRING)
            return DT_STRING;
    }
    //TODO more operators
    *opExists = FALSE;

    return DT_STRING;
}


char *
monetdbGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
monetdbGetViewDefinition(char *viewName)
{
    return NULL;
}

void
monetdbGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
monetdbExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
monetdbGetCostEstimation(char *query)
{
    return 0;//TODO exists?
}

List *
monetdbGetKeyInformation(char *tableName)
{
    MapiHdl result = NULL;
    List *attrs = NIL;
    result = EXEC_PARAM_QUERY(TABLE_GET_KEY_QUERY, tableName);

    THROW_ON_ERROR(result, "get table key attributes");

    while(mapi_fetch_row(result))
    {
        AttributeDef *a = createAttributeDef(
                strdup(mapi_fetch_field(result, 0)),
                monetdbTypeToDT(strdup(mapi_fetch_field(result, 1)))
        );
        attrs = appendToTailOfList(attrs, a);
    }

    THROW_ON_ERROR(result, "get table key attributes");

    mapi_close_handle(result);

    DEBUG_LOG("table %s has key attributes %s", tableName, stringListToString(attrs));

    return attrs;
}

Relation *
monetdbExecuteQuery(char *query)
{
    MapiHdl result;
    Relation *r = makeNode(Relation);
    int numFields = -1;

    result = executeQueryInternal(query);

    THROW_ON_ERROR(result, query);

    // read rows
    r->tuples = NIL;
    while(mapi_fetch_row(result)) //TODO deal with errors during fetch
    {
        List *tuple = NIL;

        if (numFields == -1)
            numFields = mapi_get_field_count(result);

        for (int j = 0; j < numFields; j++)
        {
            char *val = mapi_fetch_field(result, j);
            tuple = appendToTailOfList(tuple, strdup(val));
        }
        r->tuples = appendToTailOfList(r->tuples, tuple);
        DEBUG_LOG("read tuple <%s>", stringListToString(tuple));
    }

    THROW_ON_ERROR(result, query);

    // set schema
    r->schema = NIL;
    for(int i = 0; i < numFields; i++)
    {
        const char *name = CONCAT_STRINGS("F", gprom_itoa(i));
        r->schema = appendToTailOfList(r->schema, strdup((char *) name));
    }

    mapi_close_handle(result);

    return r;
}

void
monetdbExecuteQueryIgnoreResults(char *query)
{
    MapiHdl result;

    result = executeQueryInternal(query);

    THROW_ON_ERROR(result, query);

    // read rows
    while(mapi_fetch_row(result)) //TODO deal with errors during fetch
        ;

    THROW_ON_ERROR(result, query);

    mapi_close_handle(result);
}

static MapiHdl
executeQueryInternal (char *query)
{
    MapiHdl result = NULL;
    result =  mapi_query(plugin->dbConn, query);
    return result;
}

static MapiHdl
executeParamQuery (char *query, char *params, ...)
{
    MapiHdl result = NULL;
    va_list args;
    List *parameters = singleton(params);
    char *cur;
    char **argv;
    int pos = 0;
//    MapiMsg resMes;

    // gather parameters
    va_start(args, params);

    while((cur = va_arg(args, char*)))
         parameters = appendToTailOfList(parameters, cur);

    va_end(args);

    DEBUG_LOG("execute query %s with parameters <%s>", query,
            stringListToString(parameters));

    // construct char ** array for monet db
    argv = MALLOC(sizeof(char *) * (LIST_LENGTH(parameters)));
    FOREACH(char,a, parameters)
    {
        argv[pos++] = strdup(a);
    }
    //argv[pos] = NULL; // monetdb expects null termination of argv array

    // create query and check that no error was raised
    // new mapi api uses prepare and
    //result =  mapi_query_array(plugin->dbConn, query, argv);
    result = mapi_prepare(plugin->dbConn, query);
    for(int i = 0; i < pos; i++)
    {
        mapi_param(result, i, &(argv[i]));
        THROW_ON_ERROR(result, query);
    }

    mapi_execute(result);
    THROW_ON_ERROR(result, query);

    if (result == NULL || mapi_error(plugin->dbConn) != MOK)
        handleResultSetError(result, query, parameters);

    return result;
}

static char *
monetdbGetConnectionDescription (void)
{
    return CONCAT_STRINGS("MonetDB:", getStringOption("connection.user"), "@",
            getStringOption("connection.host"), ":", getStringOption("connection.db"));
}


// if monetdb not available then create dummy implementations
#else

MetadataLookupPlugin *
assembleMonetdbMetadataLookupPlugin (void)
{
    return NULL;
}

int
monetdbInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
monetdbShutdownMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
monetdbDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}

int
monetdbDatabaseConnectionClose()
{
    return EXIT_SUCCESS;
}

boolean
monetdbIsInitialized (void)
{
    return FALSE;
}

boolean
monetdbCatalogTableExists (char * tableName)
{
    return FALSE;
}

boolean
monetdbCatalogViewExists (char * viewName)
{
    return FALSE;
}

List *
monetdbGetAttributes (char *tableName)
{
    return NIL;
}

List *
monetdbGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
monetdbIsAgg(char *functionName)
{
    return FALSE;
}

boolean
monetdbIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
monetdbGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
monetdbGetViewDefinition(char *viewName)
{
    return NULL;
}

void
monetdbGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
monetdbExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
monetdbGetCostEstimation(char *query)
{
    return 0;
}

List *
monetdbGetKeyInformation(char *tableName)
{
    return NULL;
}

Relation *
monetdbExecuteQuery(char *query)
{
    return NULL;
}

#endif


