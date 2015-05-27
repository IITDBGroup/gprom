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
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"

#if HAVE_POSTGRES_BACKEND
#include "libpq-fe.h"
#endif


// Mem context
#define CONTEXT_NAME "PostgresMemContext"

#define NAME_TABLE_GET_ATTRS "GPRoM_GetTableAttributeNames"
#define PARAMS_TABLE_GET_ATTRS 1
#define QUERY_TABLE_GET_ATTRS "SELECT attname, atttypid " \
		"FROM pg_class c, pg_attribute a " \
		"WHERE c.oid = a.attrelid " \
		"AND relkind = 'r' " \
		"AND relname = $1::text " \
		"AND attname NOT IN ('tableoid', 'cmax', 'xmax', 'cmin', 'xmin', 'ctid');"

#define NAME_TABLE_EXISTS "GPRoM_CheckTableExists"
#define PARAMS_TABLE_EXISTS 1
#define QUERY_TABLE_EXISTS "SELECT EXISTS (SELECT * FROM pg_class " \
		"WHERE relkind = 'r' AND relname = $1::text);"

#define NAME_VIEW_GET_ATTRS "GPRoM_GetViewAttributeNames"
#define PARAMS_VIEW_GET_ATTRS 1
#define QUERY_VIEW_GET_ATTRS "SELECT attname, atttypid " \
        "FROM pg_class c, pg_attribute a " \
        "WHERE c.oid = a.attrelid " \
        "AND relkind = 'v' " \
        "AND relname = $1::text;"

#define NAME_VIEW_EXISTS "GPRoM_CheckViewExists"
#define PARAMS_VIEW_EXISTS 1
#define QUERY_VIEW_EXISTS "SELECT EXISTS (SELECT * FROM pg_class " \
        "WHERE relkind = 'v' AND relname = $1::text);"

#define NAME_IS_WIN_FUNC "GPRoM_IsWinFunc"
#define PARAMS_IS_WIN_FUNC 1
#define QUERY_IS_WIN_FUNC "SELECT bool_or(proiswindow) is_win FROM pg_proc " \
        "WHERE proname = $1::text;"

#define NAME_IS_AGG_FUNC "GPRoM_IsAggFunc"
#define PARAMS_IS_AGG_FUNC 1
#define QUERY_IS_AGG_FUNC "SELECT bool_or(proisagg) AS is_agg FROM pg_proc " \
		"WHERE proname = $1::text;"

#define NAME_GET_VIEW_DEF "GPRoM_GetViewDefinition"
#define PARAMS_GET_VIEW_DEF 1
#define QUERY_GET_VIEW_DEF "SELECT definition FROM pg_views WHERE viewname = $1::text;"

#define NAME_GET_FUNC_DEFS "GPRoM_GetFuncDefs"
#define PARAMS_GET_FUNC_DEFS 2
#define QUERY_GET_FUNC_DEFS "SELECT prorettype, proargtypes, proallargtypes " \
        "FROM pg_proc WHERE proname = $1::name AND pronargs = $2::smallint;"

#define NAME_GET_OP_DEFS "GPRoM_GetOpDefs"
#define PARAMS_GET_OP_DEFS 1
#define QUERY_GET_OP_DEFS "SELECT oprleft, oprright FROM pg_operator WHERE oprname = $1::name;"

#define NAME_GET_PK "GPRoM_GetPK"
#define PARAMS_GET_PK 1
#define QUERY_GET_PK "SELECT a.attname " \
                     "FROM pg_constraint c, pg_class t, pg_attribute a " \
                     "WHERE c.contype = 'p' AND c.conrelid = t.oid AND t.relname = $1::text AND a.attrelid = t.oid AND a.attnum = ANY(c.conkey);"


//#define NAME_ "GPRoM_"
//#define PARAMS_ 1
//#define QUERY_ "SELECT"
#define QUERY_GET_DT_OIDS "SELECT oid, typname FROM pg_type WHERE typtype = 'b';"

// prepare a catalog lookup query
#define PREP_QUERY(name) prepareQuery(NAME_ ## name, QUERY_ ## name, PARAMS_ ## name, NULL)

// real versions if libpq is present
#ifdef HAVE_POSTGRES_BACKEND

// functions
static PGresult *execQuery(char *query);
static PGresult *execPrepared(char *qName, List *values);
static boolean prepareQuery(char *qName, char *query, int parameters,
        Oid *types);
static void prepareLookupQueries(void);
static void fillOidToDTMap (HashMap *oidToDT);
static List *oidVecToDTList (char *oidVec);
static DataType postgresOidToDT(char *Oid);
static DataType postgresTypenameToDT (char *typName);
static List *oidVecToDTList (char *oidVec);


// closing result sets and connections
#define CLOSE_QUERY() \
		do { \
		    PGresult *_res; \
            _res = PQexec(conn, "CLOSE myportal"); \
            PQclear(_res); \
		} while(0)

#define CLOSE_CONN_AND_FATAL(__VA_ARGS__) \
		do { \
			PQfinish(plugin->conn); \
			FATAL_LOG(__VA_ARGS__); \
		}

#define CLOSE_RES_CONN_AND_FATAL(res, ...) \
        do { \
            PQfinish(plugin->conn); \
            PQclear(res); \
            FATAL_LOG(__VA_ARGS__); \
        } while(0)


// extends MetadataLookupPlugin with postgres connection
typedef struct PostgresPlugin
{
    MetadataLookupPlugin plugin;
    PGconn *conn;
    boolean initialized;
} PostgresPlugin;

// data types: additional cache entries
typedef struct PostgresMetaCache {
    HashMap *oidToDT;   // maps datatype OID to GProM datatypes
} PostgresMetaCache;

#define GET_CACHE() ((PostgresMetaCache *) plugin->plugin.cache->cacheHook)

// global vars
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
    p->getFuncReturnType = postgresGetFuncReturnType;
    p->getOpReturnType = postgresGetOpReturnType;
    p->getTableDefinition = postgresGetTableDefinition;
    p->getViewDefinition = postgresGetViewDefinition;
    p->getTransactionSQLAndSCNs = postgresGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = postgresExecuteAsTransactionAndGetXID;
    p->getCostEstimation = postgresGetCostEstimation;
    p->getKeyInformation = postgresGetKeyInformation;

    return p;
}

/* plugin methods */
int
postgresInitMetadataLookupPlugin (void)
{
    PostgresMetaCache *psqlCache;

    if (plugin && plugin->initialized)
    {
        INFO_LOG("tried to initialize metadata lookup plugin more than once");
        return EXIT_SUCCESS;
    }

    NEW_AND_ACQUIRE_MEMCONTEXT(CONTEXT_NAME);
    memContext = getCurMemContext();

    // create cache
    plugin->plugin.cache = createCache();

    // create postgres specific part of the cache
    psqlCache = NEW(PostgresMetaCache);
    psqlCache->oidToDT = NEW_MAP(Constant,Constant);
    plugin->plugin.cache->cacheHook = (void *) psqlCache;

    plugin->initialized = TRUE;

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
postgresShutdownMetadataLookupPlugin (void)
{
    ACQUIRE_MEM_CONTEXT(memContext);

    // clear cache and postgres cache

    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
postgresDatabaseConnectionOpen (void)
{
    StringInfo connStr = makeStringInfo();
//    OptionConnection *op = getOptions()->optionConnection;

    ACQUIRE_MEM_CONTEXT(memContext);

    /* create connection string */
//    if (op->host)
        appendStringInfo(connStr, " host=%s", getStringOption("connection.host"));
//    if (op->db)
        appendStringInfo(connStr, " dbname=%s", getStringOption("connection.db"));
//    if (op->user)
        appendStringInfo(connStr, " user=%s", getStringOption("connection.user"));
    if (optionSet("connection.passwd"))
        appendStringInfo(connStr, " password=%s", getStringOption("connection.passwd"));
//    if (op->port)
        appendStringInfo(connStr, " port=%u", getIntOption("connection.port"));

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

    // prepare queries
    prepareLookupQueries();

    // initialize cache
    fillOidToDTMap(GET_CACHE()->oidToDT);

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

static void
fillOidToDTMap (HashMap *oidToDT)
{
    PGresult *res = NULL;
    int numRes = 0;

    res = execQuery(QUERY_GET_DT_OIDS);
    numRes = PQntuples(res);

    for(int i = 0; i < numRes; i++)
    {
        char *oid = PQgetvalue(res,i,0);
        int oidInt = atoi(oid);
        char *typName = PQgetvalue(res,i,1);

        DEBUG_LOG("oid = %s, typename = %s", oid, typName);
        MAP_ADD_INT_KEY(oidToDT,oidInt,
                createConstInt(postgresTypenameToDT(typName)));
    }
    //TODO FINISH transaction
    PQclear(res);
    DEBUG_LOG("oid -> DT map:\n%s", beatify(nodeToString(oidToDT)));
}

static void
prepareLookupQueries(void)
{
    PREP_QUERY(TABLE_GET_ATTRS);
    PREP_QUERY(TABLE_EXISTS);
    PREP_QUERY(VIEW_GET_ATTRS);
    PREP_QUERY(VIEW_EXISTS);
    PREP_QUERY(IS_WIN_FUNC);
    PREP_QUERY(IS_AGG_FUNC);
    PREP_QUERY(GET_VIEW_DEF);
    PREP_QUERY(GET_FUNC_DEFS);
    PREP_QUERY(GET_OP_DEFS);
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

PGconn *
getPostgresConnection(void)
{
    ASSERT(postgresIsInitialized());

    return plugin->conn;
}

boolean
postgresIsInitialized (void)
{
    if (plugin && plugin->initialized)
    {
        if (plugin->conn == NULL)
        {
            if (postgresDatabaseConnectionOpen() != EXIT_SUCCESS)
                return FALSE;
        }
        if (PQstatus(plugin->conn) == CONNECTION_BAD)
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

DataType
postgresGetFuncReturnType (char *fName, List *argTypes)
{
    PGresult *res = NULL;
    DataType resType = DT_STRING;

    ACQUIRE_MEM_CONTEXT(memContext);

    //TODO cache operator information
    res = execPrepared(NAME_GET_FUNC_DEFS,
            LIST_MAKE(createConstString(fName),
                    createConstInt(LIST_LENGTH(argTypes))));

    for(int i = 0; i < PQntuples(res); i++)
    {
        char *retType = PQgetvalue(res,i,0);
        char *argTypes = PQgetvalue(res,i,1);
        List *argDTs = oidVecToDTList(argTypes);

        if (equal(argDTs, argTypes)) //TODO compatible data types
            return postgresOidToDT(retType);
    }

    PQclear(res);

    RELEASE_MEM_CONTEXT();

    return resType;
}

DataType
postgresGetOpReturnType (char *oName, List *argTypes)
{
    PGresult *res = NULL;
    DataType resType = DT_STRING;

    ACQUIRE_MEM_CONTEXT(memContext);

    //TODO cache operator information
    res = execPrepared(NAME_GET_OP_DEFS,
            LIST_MAKE(createConstString(oName),
                    createConstInt(LIST_LENGTH(argTypes))));

    for(int i = 0; i < PQntuples(res); i++)
    {

    }

    PQclear(res);
    RELEASE_MEM_CONTEXT();

    return resType;
}

static List *
oidVecToDTList (char *oidVec)
{
    List *result = NIL;
    char *oid;

    oid = strtok(oidVec, " ");
    while(oid != NULL)
    {
        result = appendToTailOfListInt(result, atoi(oid));
        oid = strtok(NULL, " ");
    }

    return result;
}

boolean
postgresCatalogTableExists (char * tableName)
{
    PGresult *res = NULL;

    if (hasSetElem(plugin->plugin.cache->tableNames,tableName))
        return TRUE;

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_TABLE_EXISTS, singleton(createConstString(tableName)));
    if (strcmp(PQgetvalue(res,0,0),"t") == 0)
    {
        addToSet(plugin->plugin.cache->tableNames, tableName);
        PQclear(res);
        return TRUE;
    }
    PQclear(res);
    RELEASE_MEM_CONTEXT();

    // run query
    return FALSE;
}

boolean
postgresCatalogViewExists (char * viewName)
{
    PGresult *res = NULL;

    if (hasSetElem(plugin->plugin.cache->viewNames,viewName))
        return TRUE;

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_VIEW_EXISTS, singleton(createConstString(viewName)));
    if (strcmp(PQgetvalue(res,0,0),"t") == 0)
    {
        addToSet(plugin->plugin.cache->viewNames, viewName);
        PQclear(res);
        return TRUE;
    }
    PQclear(res);
    RELEASE_MEM_CONTEXT();

    return FALSE;
}


List *
postgresGetAttributes (char *tableName)
{
    PGresult *res = NULL;
    List *attrs = NIL;
    ASSERT(postgresCatalogTableExists(tableName));

    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrDefs, tableName))
        return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrDefs,tableName);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_TABLE_GET_ATTRS, singleton(createConstString(tableName)));

    // loop through results
    for(int i = 0; i < PQntuples(res); i++)
    {
        AttributeDef *a = createAttributeDef(
                strdup(PQgetvalue(res,i,0)),
                postgresOidToDT(strdup(PQgetvalue(res,i,1)))
                );
        attrs = appendToTailOfList(attrs, a);
    }

    // clear result
    PQclear(res);
    MAP_ADD_STRING_KEY(plugin->plugin.cache->tableAttrDefs, tableName, attrs);

    DEBUG_LOG("table %s attributes are <%s>", tableName, beatify(nodeToString(attrs)));
    RELEASE_MEM_CONTEXT();

    return attrs;
}

List *
postgresGetAttributeNames (char *tableName)
{
    List *attrs = NIL;
    PGresult *res = NULL;
    ASSERT(postgresCatalogTableExists(tableName));

    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName))
        return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrs,tableName);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_TABLE_GET_ATTRS, singleton(createConstString(tableName)));

    // loop through results
    for(int i = 0; i < PQntuples(res); i++)
        attrs = appendToTailOfList(attrs, strdup(PQgetvalue(res,i,0)));

    // clear result
    PQclear(res);
    MAP_ADD_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName, attrs);

    DEBUG_LOG("table %s attributes are <%s>", tableName, stringListToString(attrs));
    RELEASE_MEM_CONTEXT();

    return attrs;
}

boolean
postgresIsAgg(char *functionName)
{
    PGresult *res = NULL;
    char *f = strdup(functionName);
//    int i = 0;

    for(char *p = f; *p != '\0'; *(p) = tolower(*p), p++)
        ;

    if (hasSetElem(plugin->plugin.cache->aggFuncNames, f))
        return TRUE;

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_IS_AGG_FUNC, singleton(createConstString(f)));
    if (strcmp(PQgetvalue(res,0,0),"t") == 0)
    {
        addToSet(plugin->plugin.cache->aggFuncNames, f);
        PQclear(res);
        return TRUE;
    }
    PQclear(res);
    RELEASE_MEM_CONTEXT();

    return FALSE;
}

boolean
postgresIsWindowFunction(char *functionName)
{
    PGresult *res = NULL;
    if (hasSetElem(plugin->plugin.cache->winFuncNames, functionName))
        return TRUE;

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_IS_WIN_FUNC, singleton(createConstString(functionName)));
    if (strcmp(PQgetvalue(res,0,0),"t") == 0)
    {
        addToSet(plugin->plugin.cache->winFuncNames, functionName);
        PQclear(res);
        return TRUE;
    }
    PQclear(res);
    RELEASE_MEM_CONTEXT();

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
    PGresult *res = NULL;

    ASSERT(postgresCatalogViewExists(viewName));
    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->viewDefs, viewName))
         return STRING_VALUE(MAP_GET_STRING(plugin->plugin.cache->viewDefs,viewName));

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_GET_VIEW_DEF, singleton(createConstString(viewName)));
    if (PQntuples(res) == 1)
    {
        Constant *def = createConstString(PQgetvalue(res,0,0));
        MAP_ADD_STRING_KEY(plugin->plugin.cache->viewDefs, viewName,
                def);
        PQclear(res);
        return STRING_VALUE(def);
    }
    PQclear(res);
    RELEASE_MEM_CONTEXT();

    return NULL;
}

int
postgresGetCostEstimation(char *query)
{
    FATAL_LOG("not supported yet");
    return 0;
}

List *
postgresGetKeyInformation(char *tableName)
{
    List *result = NIL;
    PGresult *res = NULL;

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_GET_PK, singleton(createConstString(tableName)));

    // loop through results
    for(int i = 0; i < PQntuples(res); i++)
        result = appendToTailOfList(result, strdup(PQgetvalue(res,i,0)));

    // cleanup
    PQclear(res);
    RELEASE_MEM_CONTEXT();

    return result;
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

    FATAL_LOG("not supported for postgres yet");

    return (Node *) xid;
}

static PGresult *
execQuery(char *query)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;

    res = PQexec(c, CONCAT_STRINGS("BEGIN TRANSACTION;"));
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
            CLOSE_RES_CONN_AND_FATAL(res, "DECLARE CURSOR failed: %s",
                    PQerrorMessage(c));
    PQclear(res);

    DEBUG_LOG("create cursor for %s", query);
    res = PQexec(c, CONCAT_STRINGS("DECLARE myportal CURSOR FOR ", query));
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        CLOSE_RES_CONN_AND_FATAL(res, "DECLARE CURSOR failed: %s",
                PQerrorMessage(c));
    PQclear(res);

    res = PQexec(c, "FETCH ALL in myportal");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        CLOSE_RES_CONN_AND_FATAL(res, "FETCH ALL failed: %s", PQerrorMessage(c));

    return res;
}

static PGresult *
execPrepared(char *qName, List *values)
{
    char **params;
    int i;
    int nParams = LIST_LENGTH(values);
    PGresult *res = NULL;
    params = CALLOC(sizeof(char*),LIST_LENGTH(values));

    ASSERT(postgresIsInitialized());

    i = 0;
    FOREACH(Constant,c,values)
        params[i++] = STRING_VALUE(c);

    DEBUG_LOG("run query %s with parameters <%s>",
            qName, exprToSQL((Node *) values));

    res = PQexecPrepared(plugin->conn,
            qName,
            nParams,
            (const char *const *) params,
            NULL,
            NULL,
            0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
        CLOSE_RES_CONN_AND_FATAL(res, "query %s failed:\n%s", qName,
                PQresultErrorMessage(res));

    return res;
}

static boolean
prepareQuery(char *qName, char *query, int parameters, Oid *types)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;

    res = PQprepare(c,
                    qName,
                    query,
                    parameters,
                    types);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        CLOSE_RES_CONN_AND_FATAL(res, "prepare query %s failed: %s",
                qName, PQresultErrorMessage(res));
    PQclear(res);

    DEBUG_LOG("prepared query: %s AS\n%s", qName, query);

    return TRUE;
}

static DataType
postgresOidToDT(char *Oid)
{
    int oid = atoi(Oid);

    return (DataType) INT_VALUE(MAP_GET_INT(GET_CACHE()->oidToDT,oid));
}

static DataType
postgresTypenameToDT (char *typName)
{
    // string data types
    if (streq(typName,"char")
           || streq(typName,"name")
           || streq(typName,"text")
           || streq(typName,"tsquery")
           || streq(typName,"varchar")
           || streq(typName,"xml")
            )
        return DT_STRING;

    // integer data types
    if (streq(typName,"int2")
            || streq(typName,"int4"))
        return DT_INT;

    // long data types
    if (streq(typName, "int8"))
        return DT_LONG;

    // numeric data types
    if (streq(typName, "float4")
            || streq(typName, "float8")
            )
        return DT_FLOAT;

    // boolean
    if (streq(typName,"bool"))
        return DT_BOOL;

    return DT_STRING;
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

int
postgresGetCostEstimation(char *query)
{
    return 0;
}

List *
postgresGetKeyInformation(char *tableName)
{
    return NULL;
}

#endif

