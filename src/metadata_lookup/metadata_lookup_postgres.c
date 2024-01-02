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
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/bitset/bitset.h"
#include "model/query_operator/operator_property.h"
#include <stdlib.h>

#if HAVE_POSTGRES_BACKEND
#include "libpq-fe.h"
#endif


// Mem context
#define CONTEXT_NAME "PostgresMemContext"

#define EXPLAIN_FUNC_NAME "_get_explain_json"
#define CREATE_EXPLAIN_FUNC "create or replace function " EXPLAIN_FUNC_NAME "(in qry text, out r jsonb) returns setof jsonb as $$" \
		"declare " \
            "explcmd text; " \
        "begin " \
            "explcmd := ('EXPLAIN (FORMAT JSON) ' || qry); " \
            "for r in execute explcmd loop " \
                "return next; " \
            "end loop; " \
            "return; " \
        "end; $$ language plpgsql;"

// we have to use syntax that works a reasonable range of postgres versions
#define QUERY_GET_SERVER_VERSION " SELECT version[1] AS major, version[2] AS minor FROM " \
	    "(SELECT regexp_split_to_array(substring(version() from 'PostgreSQL ([0-9]+[.][0-9]+)'), '[.]') AS version) getv;"
//#define QUERY_GET_SERVER_VERSION "SELECT version[1] AS major, version[2] AS minor FROM "
//        "(SELECT (regexp_match(version(), '(\\d+).(\\d+)'))::text[] AS version) getv;"

#define NAME_EXPLAIN_FUNC_EXISTS "GProM_CheckExplainFunctionExists"
#define PARAMS_EXPLAIN_FUNC_EXISTS 0
#define QUERY_EXPLAIN_FUNC_EXISTS "SELECT EXISTS (SELECT * FROM pg_catalog.pg_proc WHERE proname = '" EXPLAIN_FUNC_NAME "');"

#define NAME_QUERY_GET_COST "GProM_GetQueryCost"
#define PARAMS_QUERY_GET_COST 1
#define QUERY_QUERY_GET_COST "SELECT (((plan->0->'Plan'->>'Total Cost')::numeric::float8) * 100.0)::int" \
    " FROM " EXPLAIN_FUNC_NAME "($1::text) AS p(plan);"

#define NAME_TABLE_GET_ATTRS "GPRoM_GetTableAttributeNames"
#define PARAMS_TABLE_GET_ATTRS 1
#define QUERY_TABLE_GET_ATTRS "SELECT attname, atttypid " \
		"FROM pg_class c, pg_attribute a " \
		"WHERE c.oid = a.attrelid " \
	    "AND a.atttypid != 0 " \
		"AND relkind = 'r' " \
		"AND relname = $1::text " \
	    "AND attisdropped = false " \
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

#define NAME_IS_WIN_FUNC_11 "GPRoM_IsWinFunc"
#define PARAMS_IS_WIN_FUNC_11 1
#define QUERY_IS_WIN_FUNC_11 "SELECT (bool_or(prokind = 'w' OR prokind = 'a')) is_win FROM pg_proc " \
        "WHERE proname = $1::text;"

#define NAME_IS_AGG_FUNC_11 "GPRoM_IsAggFunc"
#define PARAMS_IS_AGG_FUNC_11 1
#define QUERY_IS_AGG_FUNC_11 "SELECT bool_or(prokind = 'a') AS is_agg FROM pg_proc " \
        "WHERE proname = $1::text;"

#define NAME_IS_WIN_FUNC "GPRoM_IsWinFunc"
#define PARAMS_IS_WIN_FUNC 1
#define QUERY_IS_WIN_FUNC "SELECT bool_or(proiswindow OR proisagg) is_win FROM pg_proc " \
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
#define QUERY_GET_OP_DEFS "SELECT ARRAY[oprleft, oprright]::oidvector, oprresult FROM pg_operator WHERE oprname = $1::name AND oprleft != 0;"

#define NAME_GET_PK "GPRoM_GetPK"
#define PARAMS_GET_PK 1
#define QUERY_GET_PK "SELECT a.attname " \
                     "FROM pg_constraint c, pg_class t, pg_attribute a " \
                     "WHERE c.contype = 'p' AND c.conrelid = t.oid AND t.relname = $1::text AND a.attrelid = t.oid AND a.attnum = ANY(c.conkey);"

#define NAME_GET_HIST "GPRoM_GetHist"
#define PARAMS_GET_HIST 2
#define QUERY_GET_HIST "SELECT histogram_bounds FROM pg_stats WHERE tablename = $1::text AND attname = $2::text;"


//#define NAME_ "GPRoM_"
//#define PARAMS_ 1
//#define QUERY_ "SELECT"
#define QUERY_GET_DT_OIDS "SELECT oid, typname FROM pg_type" //  WHERE typtype = 'b';"
#define QUERY_GET_ANY_OIDS "SELECT oid FROM pg_type WHERE typname = 'any' OR typname = 'anyelement'"

// prepare a catalog lookup query
#define PREP_QUERY(name) prepareQuery(NAME_ ## name, QUERY_ ## name, PARAMS_ ## name, NULL)

// real versions if libpq is present
#ifdef HAVE_POSTGRES_BACKEND

// functions
static void execStmt (char *stmt);
static PGresult *execQuery(char *query);
static void execCommit(void);
static PGresult *execPrepared(char *qName, List *values);
static boolean prepareQuery(char *qName, char *query, int parameters,
        Oid *types);
static void determineServerVersion(void);
static void prepareLookupQueries(void);
static boolean hasAnyType (List *oids);
static boolean isAnyTypeCompaible (List *oids, List *argTypes);
static DataType inferAnyReturnType (List *oids, List *argTypes, int retOid);
static void fillOidToDTMap (HashMap *oidToDT, Set *anyOids);
static char *postgresGetConnectionDescription (void);
static List *oidVecToDTList (char *oidVec);
static List *oidVecToOidList (char *oidVec);
static DataType postgresOidToDT(char *Oid);
static DataType postgresOidIntToDT(int oid);
static DataType postgresTypenameToDT (char *typName);

// closing result sets and connections
#define CLOSE_QUERY() \
		do { \
		    PGresult *_res; \
            _res = PQexec(conn, "CLOSE myportal"); \
            PQclear(_res); \
		} while(0)

#define CLOSE_CONN_AND_FATAL(...) \
		do { \
			PQfinish(plugin->conn); \
            plugin->conn = NULL; \
            STOP_TIMER_IF_RUNNING(METADATA_LOOKUP_QUERY_TIMER); \
			FATAL_LOG(__VA_ARGS__); \
		}

#define CLOSE_RES_CONN_AND_FATAL(res, ...) \
        do { \
            PQclear(res); \
            PQfinish(plugin->conn); \
            plugin->conn = NULL; \
			STOP_TIMER_IF_RUNNING(METADATA_LOOKUP_QUERY_TIMER); \
            FATAL_LOG(__VA_ARGS__); \
        } while(0)


// extends MetadataLookupPlugin with postgres connection
typedef struct PostgresPlugin
{
    MetadataLookupPlugin plugin;
    PGconn *conn;
    boolean initialized;
    int serverMajorVersion;
    int serverMinorVersion;
} PostgresPlugin;

// data types: additional cache entries
typedef struct PostgresMetaCache {
    HashMap *oidToDT;   // maps datatype OID to GProM datatypes
    Set *anyOids;
	HashMap *tableMinMax;
} PostgresMetaCache;

#define GET_CACHE() ((PostgresMetaCache *) plugin->plugin.cache->cacheHook)

// names of timers used here
#define METADATA_LOOKUP_TIMER "module - metadata lookup"
#define METADATA_LOOKUP_QUERY_TIMER "module - metadata lookup - running queries"
#define METADATA_LOOKUP_COMMIT_TIMER "Postgres - execute commit"
#define METADATA_LOOKUP_EXEC_QUERY_TIME "Postgres - execute query"
#define METADATA_LOOKUP_PREPARE_QUERY_TIME "Postgres - execute prepare query"
#define METADATA_LOOKUP_EXEC_PREPARED "Postgres - execute prepared"
#define METADATA_LOOKUP_EXEC_STMT "Postgres - execute stmt"

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
    p->getHistogram = postgresGetHist;
    p->getProvenanceSketch = postgresGetPS;
    p->getProvenanceSketchTemplateFromTable = postgresGetPSTemplateFromTable;
    p->getProvenanceSketchHistogramFromTable = postgresGetPSHistogramFromTable;
    p->getProvenanceSketchInfoFromTable = postgresGetPSInfoFromTable;
    p->storePsTemplates = postgresStorePsTemplate;
    p->storePsHistogram = postgresStorePsHist;
    p->storePsInformation = postgresStorePsInfo;
    p->createProvenanceSketchTemplateTable = postgresCreatePSTemplateTable;
    p->createProvenanceSketchInfoTable = postgresCreatePSInfoTable;
    p->createProvenanceSketchHistTable = postgresCreatePSHistTable;
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
    p->executeQuery = postgresExecuteQuery;
    p->executeQueryIgnoreResult = postgresExecuteQueryIgnoreResult;
    p->connectionDescription = postgresGetConnectionDescription;
    p->sqlTypeToDT = postgresBackendSQLTypeToDT;
    p->dataTypeToSQL = postgresBackendDatatypeToSQL;
	p->getMinAndMax = postgresGetMinAndMax;
	p->getAllMinAndMax = postgresGetAllMinAndMax;

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

    NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);
    memContext = getCurMemContext();

	START_TIMER(METADATA_LOOKUP_TIMER);

    // create cache
    plugin->plugin.cache = createCache();

    // create postgres specific part of the cache
    psqlCache = NEW(PostgresMetaCache);
    psqlCache->oidToDT = NEW_MAP(Constant,Constant);
    psqlCache->anyOids = INTSET();
	psqlCache->tableMinMax = NEW_MAP(Constant,HashMap);
    plugin->plugin.cache->cacheHook = (void *) psqlCache;

    plugin->initialized = TRUE;


	STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
postgresShutdownMetadataLookupPlugin (void)
{
    ACQUIRE_MEM_CONTEXT(memContext);

    // clear cache and postgres cache

    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
	plugin = NULL;

    return EXIT_SUCCESS;
}

int
postgresDatabaseConnectionOpen (void)
{
    StringInfo connStr;
//    OptionConnection *op = getOptions()->optionConnection;

    ACQUIRE_MEM_CONTEXT(memContext);
	START_TIMER(METADATA_LOOKUP_TIMER);

	connStr = makeStringInfo();

    /* create connection string */
    appendStringInfo(connStr, " host=%s", getStringOption("connection.host"));
    appendStringInfo(connStr, " dbname=%s", getStringOption("connection.db"));
    appendStringInfo(connStr, " user=%s", getStringOption("connection.user"));
    if (optionSet("connection.passwd"))
        appendStringInfo(connStr, " password=%s", getStringOption("connection.passwd"));
    appendStringInfo(connStr, " port=%u", getIntOption("connection.port"));

    /* try to connect to db */
    plugin->conn = PQconnectdb(connStr->data);

    /* check to see that the backend connection was successfully made */
    if (plugin->conn == NULL || PQstatus(plugin->conn) == CONNECTION_BAD)
    {
        if(plugin->conn != NULL)
        {
            char *error = PQerrorMessage(plugin->conn);
            PQfinish(plugin->conn);
            plugin->conn = NULL;
            RELEASE_MEM_CONTEXT();
            ERROR_LOG("unable to connect to postgres database %s\n\nfailed "
                      "because of:\n%s", connStr->data, error);
            return EXIT_FAILURE;
        }
        else
        {
            RELEASE_MEM_CONTEXT();
            ERROR_LOG("unable to connect to postgres, no connection object was created.");
            return EXIT_FAILURE;
        }
    }

    plugin->initialized = TRUE;

    // determine server version
    determineServerVersion();

    // prepare queries
    prepareLookupQueries();

    // initialize cache
    fillOidToDTMap(GET_CACHE()->oidToDT, GET_CACHE()->anyOids);

	STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

static char *
postgresGetConnectionDescription (void)
{
    return CONCAT_STRINGS("Postgres:", getStringOption("connection.user"), "@",
            getStringOption("connection.host"), ":", getStringOption("connection.db"));
}

static void
fillOidToDTMap (HashMap *oidToDT, Set *anyOids)
{
    PGresult *res = NULL;
    int numRes = 0;

    // get OID to DT mapping
	START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
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

    PQclear(res);
    execCommit();

    // get OIDs of any/anyelement types that have to be treated special
	START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    res = execQuery(QUERY_GET_ANY_OIDS);
    numRes = PQntuples(res);

    for(int i = 0; i < numRes; i++)
    {
        char *oid = PQgetvalue(res,i,0);
        int oidInt = atoi(oid);

        DEBUG_LOG("anyoid = %s", oid);
        addIntToSet(anyOids,oidInt);
    }

    PQclear(res);
    execCommit();

    DEBUG_NODE_BEATIFY_LOG("oid -> DT map:", oidToDT);
}

static void
determineServerVersion(void)
{
    PGresult *res = NULL;
    int numRes = 0;

    // get OID to DT mapping
	START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    res = execQuery(QUERY_GET_SERVER_VERSION);
    numRes = PQntuples(res);
    ASSERT(numRes == 1);

    for(int i = 0; i < numRes; i++)
    {
        char *majorStr = PQgetvalue(res,i,0);
        int majorInt = atoi(majorStr);
        char *minorStr = PQgetvalue(res,i,1);
        int minorInt = atoi(minorStr);

        DEBUG_LOG("major = %u, minor = %u", majorInt, minorInt);
        plugin->serverMajorVersion = majorInt;
        plugin->serverMinorVersion = minorInt;
    }

    PQclear(res);
    execCommit();
}

static void
prepareLookupQueries(void)
{
    PGresult *res;
    boolean funcExists = FALSE;
    // create explain function for costing queries if necessary
    PREP_QUERY(EXPLAIN_FUNC_EXISTS);

    res = execPrepared(NAME_EXPLAIN_FUNC_EXISTS, NIL);
    for(int i = 0; i < PQntuples(res); i++)
    {
        char *ex = PQgetvalue(res,i,0);
        if (streq(ex, "True"))
            funcExists = TRUE;
    }
    PQclear(res);

    // create explain function
    if (!funcExists && plugin->serverMajorVersion >= 9)
    {
		START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
        execStmt(CREATE_EXPLAIN_FUNC);
    }

    // prepare other queries used for metadata lookup
	// postgres 8 or older does not support JSON explain output we use to extract query cost
	if (plugin->serverMajorVersion >= 9)
	{
		PREP_QUERY(QUERY_GET_COST);
	}
    PREP_QUERY(TABLE_GET_ATTRS);
    PREP_QUERY(TABLE_EXISTS);
    PREP_QUERY(VIEW_GET_ATTRS);
    PREP_QUERY(VIEW_EXISTS);
    PREP_QUERY(GET_VIEW_DEF);
    PREP_QUERY(GET_FUNC_DEFS);
    PREP_QUERY(GET_OP_DEFS);
    PREP_QUERY(GET_PK);
    PREP_QUERY(GET_HIST);

    // catalog pg_proc has changed in 11
    if (plugin->serverMajorVersion >= 11)
    {
        PREP_QUERY(IS_WIN_FUNC_11);
        PREP_QUERY(IS_AGG_FUNC_11);
    }
    else
    {
        PREP_QUERY(IS_WIN_FUNC);
        PREP_QUERY(IS_AGG_FUNC);
    }
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
postgresGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
    PGresult *res = NULL;
    DataType resType = DT_STRING;
    *funcExists = FALSE;

    // handle non function expressions that are treated as functions by GProM
    if (streq(fName, "greatest") || streq(fName, "least"))
    {
        if(LIST_LENGTH(argTypes)  >= 2)
        {
            return lcaType(getNthOfListInt(argTypes, 0), getNthOfListInt(argTypes, 1));
        }
        else if (LIST_LENGTH(argTypes) == 1)
        {
            return getNthOfListInt(argTypes, 0);
        }
        return DT_INT;
    }

    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    //TODO cache function information
    res = execPrepared(NAME_GET_FUNC_DEFS,
            LIST_MAKE(createConstString(fName),
                    createConstString(gprom_itoa(LIST_LENGTH(argTypes)))));

    for(int i = 0; i < PQntuples(res); i++)
    {
        char *retType = PQgetvalue(res,i,0);
        char *candArgTypes = PQgetvalue(res,i,1);
        List *argDTs = oidVecToDTList(strdup(candArgTypes));
        List *argOids = oidVecToOidList(candArgTypes);

        DEBUG_LOG("argDTs: %s, argTypes: %s", nodeToString(argDTs), nodeToString(argTypes));
        if (equal(argDTs, argTypes)) //TODO compatible data types
        {
            RELEASE_MEM_CONTEXT();
            DataType retDT = postgresOidToDT(retType);
            DEBUG_LOG("return type %s for %s(%s)", DataTypeToString(retDT), fName, nodeToString(argTypes));
            resType = retDT;
            *funcExists = TRUE;
        }

        // does function take anytype as an input then determine return type
        if (hasAnyType(argOids))
        {
            if(isAnyTypeCompaible(argOids, argTypes))
            {
                int retOid = atoi(retType);
                resType = inferAnyReturnType(argOids, argTypes, retOid);
                *funcExists = TRUE;
            }
        }
    }

    PQclear(res);

    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return resType;
}

static boolean
hasAnyType (List *oids)
{
    FOREACH_INT(o, oids)
    {
        if (hasSetIntElem(GET_CACHE()->anyOids, o))
            return TRUE;
    }
    return FALSE;
}

static boolean
isAnyTypeCompaible (List *oids, List *argTypes)
{
    Set *anyOids = GET_CACHE()->anyOids;
    DataType anyDT;
    boolean hasSetAnyDT = FALSE;

    FORBOTH_INT(o,dt,oids,argTypes)
    {
        if (hasSetIntElem(anyOids, o))
        {
            if(!hasSetAnyDT)
            {
                anyDT = dt;
                hasSetAnyDT = TRUE;
            }
            else
            {
                if (anyDT != dt)
                    return FALSE;
            }
        }
        else
        {
            if(postgresOidIntToDT(o) != dt)
                return FALSE;
        }
    }

    return TRUE;
}

static DataType
inferAnyReturnType (List *oids, List *argTypes, int retOid)
{
    Set *anyOids = GET_CACHE()->anyOids;

    if (!hasSetIntElem(anyOids,retOid))
        return postgresOidIntToDT(retOid);

    FORBOTH_INT(o,dt,oids,argTypes)
    {
        if (hasSetIntElem(anyOids, o))
        {
            return dt;
        }
    }

    // keep compiler quiet
    return DT_STRING;
}

DataType
postgresGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{
    PGresult *res = NULL;
    DataType resType = DT_STRING;

    *opExists = FALSE;
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    //TODO cache operator information
    res = execPrepared(NAME_GET_OP_DEFS,
            LIST_MAKE(createConstString(oName)));

    for(int i = 0; i < PQntuples(res); i++)
    {
        char *retType = PQgetvalue(res,i,1);
        char *candArgTypes = PQgetvalue(res,i,0);
        List *argDTs = oidVecToDTList(candArgTypes);

        DEBUG_LOG("argDTs: %s, argTypes: %s", nodeToString(argDTs), nodeToString(argTypes));
        if (equal(argDTs, argTypes)) //TODO compatible data types
        {
            RELEASE_MEM_CONTEXT();
            DataType retDT = postgresOidToDT(retType);
            DEBUG_LOG("return type %s for %s(%s)", DataTypeToString(retDT), oName, nodeToString(argTypes));
            *opExists = TRUE;
            resType = retDT;
        }
    }

    PQclear(res);
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
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
        result = appendToTailOfListInt(result, postgresOidToDT(oid));
        oid = strtok(NULL, " ");
    }

    return result;
}

static List *
oidVecToOidList (char *oidVec)
{
    List *result = NIL;
    char *oid;
    int oidInt;

    oid = strtok(oidVec, " ");
    while(oid != NULL)
    {
        oidInt = atoi(oid);
        result = appendToTailOfListInt(result, oidInt);
        oid = strtok(NULL, " ");
    }

    return result;
}

boolean
postgresCatalogTableExists (char * tableName)
{
    PGresult *res = NULL;
    START_TIMER(METADATA_LOOKUP_TIMER);

    if (hasSetElem(plugin->plugin.cache->tableNames,tableName)){
   	STOP_TIMER(METADATA_LOOKUP_TIMER);
        return TRUE;
    }

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_TABLE_EXISTS, singleton(createConstString(tableName)));
    STOP_TIMER(METADATA_LOOKUP_TIMER);
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
    START_TIMER(METADATA_LOOKUP_TIMER);

    if (hasSetElem(plugin->plugin.cache->viewNames,viewName)){
        STOP_TIMER(METADATA_LOOKUP_TIMER);
        return TRUE;
    }
    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_VIEW_EXISTS, singleton(createConstString(viewName)));
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    if (strcmp(PQgetvalue(res,0,0),"t") == 0)
    {
        addToSet(plugin->plugin.cache->viewNames, viewName);
        PQclear(res);
		RELEASE_MEM_CONTEXT();
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
    START_TIMER(METADATA_LOOKUP_TIMER);
    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrDefs, tableName)){
        STOP_TIMER(METADATA_LOOKUP_TIMER);
        return (List *) (MAP_GET_STRING(plugin->plugin.cache->tableAttrDefs,tableName));
    }
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

    DEBUG_LOG("table %s", tableName);
    DEBUG_NODE_BEATIFY_LOG("attributes are", attrs);
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return attrs;
}

List *
postgresGetAttributeNames (char *tableName)
{
    List *attrs = NIL;
    PGresult *res = NULL;
    ASSERT(postgresCatalogTableExists(tableName));
    START_TIMER(METADATA_LOOKUP_TIMER);
    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName)){
        STOP_TIMER(METADATA_LOOKUP_TIMER);
        return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrs,tableName);
    }
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
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return attrs;
}


HashMap *
postgresGetPS (char *sql, List *attrNames)
{
    //List *attrs = NIL;
    PGresult *res = NULL;
    HashMap *hm = NEW_MAP(Constant,Constant);
    //ASSERT(postgresCatalogTableExists(tableName));

    //if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName))
    //    return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrs,tableName);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute get ps");
	res = execQuery(sql);

    // loop through results
    for(int i = 0; i < PQntuples(res); i++) {
    		for(int j = 0; j < LIST_LENGTH(attrNames); j++) {
    			//attrs = appendToTailOfList(attrs, strdup(PQgetvalue(res,i,j)));
    			char *attrName = getNthOfListP(attrNames, j);
    			MAP_ADD_STRING_KEY(hm, attrName, createConstString(strdup(PQgetvalue(res,i,j))));
    		}
    }

    STOP_TIMER("Postgres - execute get ps");
    execStmt("commit;");
    // clear result
    PQclear(res);

    DEBUG_NODE_LOG("Captured Provenance Sketch :", (Node *) hm);
    //DEBUG_LOG("Captured Provenance Sketch : <%s>", stringListToString(attrs));
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return hm;
}


HashMap *
postgresGetPSTemplateFromTable ()
{
	HashMap *tmap = NEW_MAP(Constant, Constant);
    PGresult *res = NULL;

    char *storeTable = getTemplatesTableName();
    ASSERT(postgresCatalogTableExists(storeTable));

    char *sql = CONCAT_STRINGS("SELECT * FROM ", storeTable, ";");
    DEBUG_LOG("The sql of postgresGetPSTemplateFromTable: %s", sql);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute get stored ps template");
	res = execQuery(sql);

//	HashMap *tmap = getLtempNoMap();
//	if(tmap == NULL)

    // loop through results
    for(int i = 0; i < PQntuples(res); i++) {
    	char *t = strdup(PQgetvalue(res,i,0));
    	char *tid = strdup(PQgetvalue(res,i,1));
    	MAP_ADD_STRING_KEY(tmap,t,createConstInt(atoi(tid)));
    }
//    setLtempNoMap(tmap);

    START_TIMER("Postgres - execute get stored ps template");
    execStmt("commit;");
    // clear result
    PQclear(res);

    //DEBUG_NODE_LOG("Captured Provenance Sketch :", (Node *) hm);
    //DEBUG_LOG("Captured Provenance Sketch : <%s>", stringListToString(attrs));
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return tmap;
}

HashMap *
postgresGetPSInfoFromTable ()
{
	HashMap *map = NEW_MAP(Constant, Node);
    PGresult *res = NULL;

    //ASSERT(postgresCatalogTableExists(tableName));

    //char *storeTable = getStringOption(OPTION_PS_STORE_TABLE);
    char *storeTable = getPSCellsTableName();
    ASSERT(postgresCatalogTableExists(storeTable));

    char *sql = CONCAT_STRINGS("SELECT * FROM ", storeTable, ";");
    DEBUG_LOG("The sql of postgresGetPSInfoFromTable: %s", sql);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute get stored ps information");
	res = execQuery(sql);

    // loop through results
    for(int i = 0; i < PQntuples(res); i++) {
    	int tid = atoi(strdup(PQgetvalue(res,i,0)));
    	char *paras = strdup(PQgetvalue(res,i,1));
//    	char *tableName = strdup(PQgetvalue(res,i,2));
//    	char *attrName = strdup(PQgetvalue(res,i,3));
//    	char *pta = strdup(PQgetvalue(res,i,4)); //prov_table_attr with number
//    	int *numRanges = atoi(strdup(PQgetvalue(res,i,5)));
//    	int *psSize = atoi(strdup(PQgetvalue(res,i,6)));
//    	BitSet *ps = stringToBitset(strdup(PQgetvalue(res,i,7)));

    	psInfoCell *psc = createPSInfoCell(//	storeTable,
    			//strdup(PQgetvalue(res,i,0)), //pqSql - template sql
				//strdup(PQgetvalue(res,i,1)), //parameter values
				strdup(PQgetvalue(res,i,2)), //tableName
				strdup(PQgetvalue(res,i,3)), //attrName
				strdup(PQgetvalue(res,i,4)), //provTableAttr
				atoi(strdup(PQgetvalue(res,i,5))), //numRanges
				atoi(strdup(PQgetvalue(res,i,6))), //psSize
				stringToBitset(strdup(PQgetvalue(res,i,7))));  //ps

    	HashMap *paraMap = NULL;
    	if(MAP_HAS_INT_KEY(map, tid))
    		paraMap = (HashMap *) MAP_GET_INT(map,tid);
    	else
    		paraMap = NEW_MAP(Constant, Node);

    	List *psCellList = NIL;
    	if(MAP_HAS_STRING_KEY(paraMap, paras))
    		psCellList = (List *) MAP_GET_STRING(paraMap,paras);

    	psCellList = appendToTailOfList(psCellList, psc);

    	//update hashmap
    	MAP_ADD_STRING_KEY(paraMap, paras, psCellList);
    	MAP_ADD_INT_KEY(map, tid, paraMap);
    }

    START_TIMER("Postgres - execute get stored ps information");
    execStmt("commit;");
    // clear result
    PQclear(res);

    //DEBUG_NODE_LOG("Captured Provenance Sketch :", (Node *) hm);
    //DEBUG_LOG("Captured Provenance Sketch : <%s>", stringListToString(attrs));
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return map;
}


//List *
//postgresGetPSInfoFromTable ()
//{
//    List *psCells = NIL;
//    PGresult *res = NULL;
//
//    //ASSERT(postgresCatalogTableExists(tableName));
//
//    //char *storeTable = getStringOption(OPTION_PS_STORE_TABLE);
//    char *storeTable = getPSCellsTableName();
//    ASSERT(postgresCatalogTableExists(storeTable));
//
//    char *sql = CONCAT_STRINGS("SELECT * FROM ", storeTable, ";");
//    DEBUG_LOG("The sql of postgresGetPSInfoFromTable: %s", sql);
//
//    // do query
//    ACQUIRE_MEM_CONTEXT(memContext);
//    START_TIMER(METADATA_LOOKUP_TIMER);
//    START_TIMER("Postgres - execute get stored ps information");
//	res = execQuery(sql);
//
//    // loop through results
//    for(int i = 0; i < PQntuples(res); i++) {
//    	psInfoCell *psc = createPSInfoCell(//	storeTable,
//    			//strdup(PQgetvalue(res,i,0)), //pqSql - template sql
//				//strdup(PQgetvalue(res,i,1)), //parameter values
//				strdup(PQgetvalue(res,i,2)), //tableName
//				strdup(PQgetvalue(res,i,3)), //attrName
//				strdup(PQgetvalue(res,i,4)), //provTableAttr
//				atoi(strdup(PQgetvalue(res,i,5))), //numRanges
//				atoi(strdup(PQgetvalue(res,i,6))), //psSize
//				stringToBitset(strdup(PQgetvalue(res,i,7))));  //ps
//    	psCells = appendToTailOfList(psCells, psc);
//    }
//
//    START_TIMER("Postgres - execute get stored ps information");
//    execStmt("commit;");
//    // clear result
//    PQclear(res);
//
//    //DEBUG_NODE_LOG("Captured Provenance Sketch :", (Node *) hm);
//    //DEBUG_LOG("Captured Provenance Sketch : <%s>", stringListToString(attrs));
//    RELEASE_MEM_CONTEXT();
//    STOP_TIMER(METADATA_LOOKUP_TIMER);
//    return psCells;
//}


//void
//postgresStorePsInfo(psInfoCell *psc)
//{   DEBUG_LOG("postgresStorePsInfo: START");
//	START_TIMER("Postgres - store ps information");
//	StringInfo insertInfo = makeStringInfo();
//
//	/*
//	 *  template | parameters (,) | tableName | attribute | numPartitions | psSize |   ps   | ranges?
//	 *  select ..|  1,4,5         |     R     |    a      |   10000       |   50   | 10010..|  1,10,..
//	 *  select ..|  1,4,5         |     R     |    b      |   10000       |   20   | 00010..|  1,10,..
//	 *   ...
//	 */
//
//    appendStringInfo(insertInfo,"insert into %s values ('%s','%s','%s','%s','%s',%d,%d,'%s');",
//    			psc->storeTable,psc->pqSql,
//				psc->paraValues,psc->tableName,
//				psc->attrName,psc->provTableAttr,
//				psc->numRanges,psc->psSize,
//				bitSetToString(psc->ps));
//	//appendStringInfo(insertInfo,"create table %s (a int,b int); commit;",
//	//					storeTable);
//    DEBUG_LOG("postgresStorePsInfo: %s", insertInfo->data);
//	execStmt(insertInfo->data);
//
//	STOP_TIMER("Postgres - store ps information");
//}

HashMap *
postgresGetPSHistogramFromTable ()
{
	HashMap *tmap = NEW_MAP(Constant, Constant);
    PGresult *res = NULL;

    char *storeTable = getHistTableName();
    ASSERT(postgresCatalogTableExists(storeTable));

    char *sql = CONCAT_STRINGS("SELECT * FROM ", storeTable, ";");
    DEBUG_LOG("The sql of postgresGetPSHistogramFromTable: %s", sql);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute get stored ps histogram");
	res = execQuery(sql);

    // loop through results
    for(int i = 0; i < PQntuples(res); i++) {
    	char *tableName = strdup(PQgetvalue(res,i,1));
    	char *attrName  = strdup(PQgetvalue(res,i,2));
    	char *numRanges = strdup(PQgetvalue(res,i,3));
    	char *hist 		= strdup(PQgetvalue(res,i,4));
		char *histMapKey = getHistMapKey(tableName,attrName, numRanges);
    	MAP_ADD_STRING_KEY(tmap,histMapKey,createConstString(hist));
    }

    START_TIMER("Postgres - execute get stored ps template");
    execStmt("commit;");
    // clear result
    PQclear(res);

    //DEBUG_NODE_LOG("Load PS Histogram from table :", (Node *) tmap);
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return tmap;
}


void
postgresCreatePSTemplateTable()
{
	char *tplTable = getTemplatesTableName();
//	char *histTable = getHistTableName();
//	char *psTable = getPSCellsTableName();

	START_TIMER("Postgres - create ps Template table");
	StringInfo createSql = makeStringInfo();

	/*
	 *  template | template Number
	 *  select ..|  1
	 *  select ..|  1
	 *   ...
	 */
	appendStringInfo(createSql,"create table %s (template VARCHAR(2000),tid int); commit;",
			tplTable);
//	appendStringInfo(createSql,"create table %s (tid int,parameters VARCHAR(255),tableName VARCHAR(50),attribte VARCHAR(50),tableAttr VARCHAR(50),numPartitions int,psSize int,ps text);",
//			psTable);
//	appendStringInfo(createSql,"create table %s (hid int, tablename VARCHAR(50),attribte VARCHAR(50),numPartitions int,hist text); commit;",
//			histTable);

	//appendStringInfo(insertInfo,"create table %s (a int,b int); commit;",
	//					storeTable);
    DEBUG_LOG("postgresCreatePSTemplateTable: %s", createSql->data);
	execStmt(createSql->data);

	STOP_TIMER("Postgres - create ps Template table");
}

void
postgresCreatePSInfoTable()
{
//	char *tplTable = getTemplatesTableName();
//	char *histTable = getHistTableName();
	char *psTable = getPSCellsTableName();

	START_TIMER("Postgres - create ps info table");
	StringInfo createSql = makeStringInfo();

	/*
	 *  template | template Number
	 *  select ..|  1
	 *  select ..|  1
	 *   ...
	 */
//	appendStringInfo(createSql,"create table %s (template VARCHAR(2000),tid int); commit;",
//			tplTable);
	appendStringInfo(createSql,"create table %s (tid int,parameters VARCHAR(255),tableName VARCHAR(50),attribte VARCHAR(50),tableAttr VARCHAR(50),numPartitions int,psSize int,ps text);",
			psTable);
//	appendStringInfo(createSql,"create table %s (hid int, tablename VARCHAR(50),attribte VARCHAR(50),numPartitions int,hist text); commit;",
//			histTable);

	//appendStringInfo(insertInfo,"create table %s (a int,b int); commit;",
	//					storeTable);
    DEBUG_LOG("postgresCreatePSInfoTable: %s", createSql->data);
	execStmt(createSql->data);

	STOP_TIMER("Postgres - create ps info table");
}

void
postgresCreatePSHistTable()
{
//	char *tplTable = getTemplatesTableName();
	char *histTable = getHistTableName();
//	char *psTable = getPSCellsTableName();

	START_TIMER("Postgres - create ps hist table");
	StringInfo createSql = makeStringInfo();

	/*
	 *  template | template Number
	 *  select ..|  1
	 *  select ..|  1
	 *   ...
	 */
//	appendStringInfo(createSql,"create table %s (template VARCHAR(2000),tid int); commit;",
//			histTable);
//	appendStringInfo(createSql,"create table %s (tid int,parameters VARCHAR(255),tableName VARCHAR(50),attribte VARCHAR(50),tableAttr VARCHAR(50),numPartitions int,psSize int,ps text);",
//			psTable);
	appendStringInfo(createSql,"create table %s (hid int, tablename VARCHAR(50),attribte VARCHAR(50),numPartitions int,hist text); commit;",
			histTable);

	//appendStringInfo(insertInfo,"create table %s (a int,b int); commit;",
	//					storeTable);
    DEBUG_LOG("postgresCreatePSHistTable: %s", createSql->data);
	execStmt(createSql->data);

	STOP_TIMER("Postgres - create ps hist table");
}


void
postgresStorePsHist(KeyValue *kv, int n)
{
	char *tableName = getHistTableName();


	START_TIMER("Postgres - store ps hist information");
	StringInfo insertInfo = makeStringInfo();

	/*
	 * hid| tableName | attribute | numPartitions  |  ranges
	 *  1 |     R     |    a      |   10000      |  1,10,..
	 *  2 |     R     |    b      |   10000      |  1,10,..
	 *   ...
	 */

	List *keyList = splitHistMapKey(STRING_VALUE(kv->key));
    appendStringInfo(insertInfo,"insert into %s values (%d,'%s','%s',%d,'%s');",
    			tableName,
    			n, //id
				STRING_VALUE(getNthOfListP(keyList,0)),
				STRING_VALUE(getNthOfListP(keyList,1)),
				atoi(STRING_VALUE(getNthOfListP(keyList,2))),
				STRING_VALUE(kv->value));
	//appendStringInfo(insertInfo,"create table %s (a int,b int); commit;",
	//					storeTable);
    DEBUG_LOG("postgresStorePsHist: %s", insertInfo->data);
	execStmt(insertInfo->data);

	STOP_TIMER("Postgres - store ps hist information");
}

void
postgresStorePsTemplate(KeyValue *kv)
{
	char *tableName = getTemplatesTableName();


	START_TIMER("Postgres - store ps Template information");
	StringInfo insertInfo = makeStringInfo();

	/*
	 *  template | template Number
	 *  select ..|  1
	 *  select ..|  1
	 *   ...
	 */

    appendStringInfo(insertInfo,"insert into %s values ('%s','%d');",
    			tableName,
				STRING_VALUE(kv->key),
				INT_VALUE(kv->value));
	//appendStringInfo(insertInfo,"create table %s (a int,b int); commit;",
	//					storeTable);
    DEBUG_LOG("postgresStorePsTemplate: %s", insertInfo->data);
	execStmt(insertInfo->data);

	STOP_TIMER("Postgres - store ps Template information");
}

void
postgresStorePsInfo(int tNo, char *paras, psInfoCell *psc)
{
	char *tableName = getPSCellsTableName();


	START_TIMER("Postgres - store ps information");
	StringInfo insertInfo = makeStringInfo();

	/*
	 *  template Number | parameters (,) | tableName | attribute | numPartitions | psSize |   ps
	 *  1				|  1,4,5         |     R     |    a      |   10000       |   50   | 10010..
	 *  2				|  1,4,5         |     R     |    b      |   10000       |   20   | 00010..
	 *   ...
	 */

    appendStringInfo(insertInfo,"insert into %s values (%d,'%s','%s','%s','%s',%d,%d,'%s');",
    			tableName,
				tNo,
				paras,
    			psc->tableName,
				psc->attrName,
				psc->provTableAttr,
				psc->numRanges,
				psc->psSize,
				bitSetToString(psc->ps));

    DEBUG_LOG("postgresStorePsInfo: %s", insertInfo->data);
	execStmt(insertInfo->data);

	STOP_TIMER("Postgres - store ps information");
}

List *
postgresGetHist (char *tableName, char *attrName, int numPartitions)
{
    List *attrs = NIL;
    PGresult *res = NULL;
    PGresult *resMinMax = NULL;
    ASSERT(postgresCatalogTableExists(tableName));

    //if (MAP_HAS_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName))
    //    return (List *) MAP_GET_STRING(plugin->plugin.cache->tableAttrs,tableName);

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    if(getBoolOption(OPTION_PS_ANALYZE))
    {
                START_TIMER("Postgres - execute ps analyze");
    		StringInfo setStatics = makeStringInfo();
    		appendStringInfo(setStatics,"ALTER TABLE %s ALTER COLUMN %s SET STATISTICS %d; Analyze %s;",
    				tableName, attrName, (numPartitions > 10000)? 10000:numPartitions, tableName);

    		execStmt(setStatics->data);
                STOP_TIMER("Postgres - execute ps analyze");
    }
    START_TIMER("Postgres - execute get hist");
    res = execPrepared(NAME_GET_HIST, LIST_MAKE(createConstString(tableName),createConstString(attrName)));

    // loop through results
    for(int i = 0; i < PQntuples(res); i++)
        attrs = appendToTailOfList(attrs, strdup(PQgetvalue(res,i,0)));

    STOP_TIMER("Postgres - execute get hist");
    // clear result
    PQclear(res);
    //MAP_ADD_STRING_KEY(plugin->plugin.cache->tableAttrs, tableName, attrs);

    DEBUG_LOG("table %s attributes %s with hist <%s>", tableName, attrName, stringListToString(attrs));
    RELEASE_MEM_CONTEXT();


    // do query
    ACQUIRE_MEM_CONTEXT(memContext);

        START_TIMER("Postgres - execute get minmax");
	StringInfo minMax = makeStringInfo();
	appendStringInfo(minMax,"SELECT MIN(%s), MAX(%s) FROM %s;",
			attrName, attrName, tableName);
	resMinMax = execQuery(minMax->data);
        STOP_TIMER("Postgres - execute get minmax");
	//have to commit, otherwise, if you run another query, will get warning: there is already a transaction in progress
	execStmt("commit;");
    // loop through results
    for(int i = 0; i < PQntuples(resMinMax); i++) {
        attrs = appendToTailOfList(attrs, strdup(PQgetvalue(resMinMax,i,0)));
        attrs = appendToTailOfList(attrs, strdup(PQgetvalue(resMinMax,i,1)));
    }
    // clear result
    PQclear(resMinMax);

    DEBUG_LOG("SELECT MinMax on %s attributes on %s table <%s>", attrName, tableName, stringListToString(attrs));
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
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
		RELEASE_MEM_CONTEXT();
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
    START_TIMER(METADATA_LOOKUP_TIMER);
    if (MAP_HAS_STRING_KEY(plugin->plugin.cache->viewDefs, viewName)){
         STOP_TIMER(METADATA_LOOKUP_TIMER);
         return STRING_VALUE(MAP_GET_STRING(plugin->plugin.cache->viewDefs,viewName));
    }
    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    res = execPrepared(NAME_GET_VIEW_DEF, singleton(createConstString(viewName)));
    if (PQntuples(res) == 1)
    {
        Constant *def = createConstString(PQgetvalue(res,0,0));
        MAP_ADD_STRING_KEY(plugin->plugin.cache->viewDefs, viewName,
                def);
        PQclear(res);
        STOP_TIMER(METADATA_LOOKUP_TIMER);
        return STRING_VALUE(def);
    }
    PQclear(res);
    RELEASE_MEM_CONTEXT();
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return NULL;
}

int
postgresGetCostEstimation(char *query)
{
    PGresult *res = NULL;
    int cost = 0;

	if (plugin->serverMajorVersion <= 8)
	{
		THROW(SEVERITY_RECOVERABLE, "postgres server major version is %d, but JSON output for explain is only support in version 9+!");
	}

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    START_TIMER(METADATA_LOOKUP_TIMER);
    res = execPrepared(NAME_QUERY_GET_COST, singleton(createConstString(query)));
    // loop through results
    for(int i = 0; i < PQntuples(res); i++)
    {
        cost = atoi(PQgetvalue(res,i,0));
    }
    RELEASE_MEM_CONTEXT();

    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return cost;
}

List *
postgresGetKeyInformation(char *tableName)
{
    List *result = NIL;
    PGresult *res = NULL;
    Set *keySet;

    // do query
    ACQUIRE_MEM_CONTEXT(memContext);
    keySet = STRSET();
    START_TIMER(METADATA_LOOKUP_TIMER);
    res = execPrepared(NAME_GET_PK, singleton(createConstString(tableName)));

    // loop through results
    for(int i = 0; i < PQntuples(res); i++)
	{
        addToSet(keySet, strdup(PQgetvalue(res,i,0)));
	}

	if (PQntuples(res) > 0)
	{
		result = singleton(keySet);
	}

    // cleanup
    PQclear(res);

    STOP_TIMER(METADATA_LOOKUP_TIMER);
    RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List,result);
}

DataType
postgresBackendSQLTypeToDT (char *sqlType)
{
    return postgresTypenameToDT(sqlType);
}

char *
postgresBackendDatatypeToSQL (DataType dt)
{
    switch(dt)
    {
        case DT_INT:
        case DT_LONG:
            return "int8";
            break;
        case DT_FLOAT:
            return "float8";
            break;
        case DT_STRING:
        case DT_VARCHAR2:
            return "text";
            break;
        case DT_BOOL:
            return "bool";
            break;
    }

    // keep compiler quiet
    return "text";
}


List *
postgresGetAllMinAndMax(TableAccessOperator* table)
{
	List *minmaxList = NIL;  //one min hashmap, one max hashmap
	HashMap *min_map = NEW_MAP(Constant, Constant);
	HashMap *max_map = NEW_MAP(Constant, Constant);

    PGresult *res = NULL;
    StringInfo statement;
//	HashMap *tableMap = (HashMap *) MAP_GET_STRING(GET_CACHE()->tableMinMax, tableName);
//
//	if(tableMap == NULL)
//	{
//		tableMap = NEW_MAP(Constant,HashMap);
//		MAP_ADD_STRING_KEY(GET_CACHE()->tableMinMax, tableName, tableMap);
//	}
	// table cache exists, return attribute if we have it already
//	else
//	{
//		result_map = (HashMap *) MAP_GET_STRING(tableMap, colName);
//		if(result_map != NULL)
//		{
//		    DEBUG_LOG("POSTGRES_GET_MINMAX: REUSE (%s.%s)\n%s",
//					 tableName,
//					 colName,
//					 nodeToString(result_map));
//			return result_map;
//		}
//	}

//	result_map = NEW_MAP(Constant, Node);
    char *tableName = table->tableName;
    List *attrDefs = ((QueryOperator *) table)->schema->attrDefs;
    List *attrNames = getAttrDefNames(attrDefs);
    statement = makeStringInfo();

	appendStringInfo(statement, "SELECT ");
    FOREACH(char, c, attrNames)
    {
    		appendStringInfo(statement,
            "MIN(%s),MAX(%s),",c,c);
    }
    removeTailingStringInfo(statement, 1);
    appendStringInfo(statement,
            " FROM %s;",tableName);

    res = execQuery(statement->data);

    int numRes = PQntuples(res);

    for(int i = 0; i < numRes; i++)
    {
    		int cnt = 0;
    		FOREACH(AttributeDef, def, attrDefs)
		{
    			//char *name = def->attrName;
    			char *min = PQgetvalue(res,i,cnt);
    			char *max = PQgetvalue(res,i,cnt+1);

    	        Constant *cmin;
    	        Constant *cmax;
    			if (def->dataType==DT_INT)
    			{
    				cmin = createConstInt(atoi(min));
    				cmax = createConstInt(atoi(max));
    			}
    			else if(def->dataType==DT_LONG)
    			{
    				cmin = createConstLong(atol(min));
    				cmax = createConstLong(atol(max));
    			}
    			else if(def->dataType==DT_FLOAT)
    			{
    				cmin = createConstFloat(atof(min));
    				cmax = createConstFloat(atof(max));
    			}
    			else
    			{
    				cmin = createConstString(min);
    				cmax = createConstString(max);
    			}

    	        MAP_ADD_STRING_KEY(min_map, def->attrName, cmin);
    	        MAP_ADD_STRING_KEY(max_map, def->attrName, cmax);
    	        DEBUG_LOG("cnt = %d, attr = %s, min = %s, max = %s", cnt, def->attrName,nodeToString(cmin), nodeToString(cmax));

    			cnt = cnt + 2;
		}
    }

    PQclear(res);
    execCommit();

//	MAP_ADD_STRING_KEY(tableMap, colName, result_map);
//    DEBUG_LOG("POSTGRES_GET_MINMAX: GOT (%s.%s)\n%s",
//			 tableName,
//			 colName,
//			 nodeToString(result_map));

    minmaxList = LIST_MAKE(min_map,max_map);
	return minmaxList;
}

HashMap *
postgresGetMinAndMax(char* tableName, char* colName)
{
	HashMap *result_map;

    PGresult *res = NULL;
    StringInfo statement;
	HashMap *tableMap = (HashMap *) MAP_GET_STRING(GET_CACHE()->tableMinMax, tableName);

	if(tableMap == NULL)
	{
		tableMap = NEW_MAP(Constant,HashMap);
		MAP_ADD_STRING_KEY(GET_CACHE()->tableMinMax, tableName, tableMap);
	}
	// table cache exists, return attribute if we have it already
	else
	{
		result_map = (HashMap *) MAP_GET_STRING(tableMap, colName);
		if(result_map != NULL)
		{
		    DEBUG_LOG("POSTGRES_GET_MINMAX: REUSE (%s.%s)\n%s",
					 tableName,
					 colName,
					 nodeToString(result_map));
			return result_map;
		}
	}

	result_map = NEW_MAP(Constant, Node);
    statement = makeStringInfo();
    appendStringInfo(statement,
            "SELECT MIN(%s),MAX(%s) FROM %s;",colName,colName,tableName);

    res = execQuery(statement->data);

    int numRes = PQntuples(res);

    for(int i = 0; i < numRes; i++)
    {
        char *min = PQgetvalue(res,i,0);
        // int oidInt = atoi(oid);
        char *max = PQgetvalue(res,i,1);
        Constant *cmin;
        Constant *cmax;

        List *dts = getAttributes(tableName);

        FOREACH(AttributeDef,n,dts)
		{
            INFO_LOG(n->attrName);
            if(strcmp(n->attrName,colName)==0)
			{
                if (n->dataType==DT_INT)
                {
                    cmin = createConstInt(atoi(min));
                    cmax = createConstInt(atoi(max));
                }
                else if(n->dataType==DT_FLOAT)
				{
                    cmin = createConstFloat(atof(min));
                    cmax = createConstFloat(atof(max));
                }
                else
				{
                    cmin = createConstString(min);
                    cmax = createConstString(max);
                }
                DEBUG_LOG("min = %s, max = %s", nodeToString(cmin), nodeToString(cmax));
            }
        }

        MAP_ADD_STRING_KEY(result_map, "MIN", cmin);
        MAP_ADD_STRING_KEY(result_map, "MAX", cmax);
    }

    PQclear(res);
    execCommit();

	MAP_ADD_STRING_KEY(tableMap, colName, result_map);
    DEBUG_LOG("POSTGRES_GET_MINMAX: GOT (%s.%s)\n%s",
			 tableName,
			 colName,
			 nodeToString(result_map));

	return result_map;
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

static void
execStmt (char *stmt)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;;
    START_TIMER(METADATA_LOOKUP_EXEC_STMT);
    res = PQexec(c, "BEGIN TRANSACTION;");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        CLOSE_RES_CONN_AND_FATAL(res, "BEGIN TRANSACTION for statement failed: %s",
                                PQerrorMessage(c));
    }
    PQclear(res);

    DEBUG_LOG("execute statement %s", stmt);
    res = PQexec(c, stmt);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        CLOSE_RES_CONN_AND_FATAL(res, "DECLARE CURSOR failed: %s",
                                PQerrorMessage(c));
    }
    PQclear(res);

    STOP_TIMER(METADATA_LOOKUP_EXEC_STMT);

    execCommit();
}

static PGresult *
execQuery(char *query)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;
    START_TIMER(METADATA_LOOKUP_EXEC_QUERY_TIME);
    START_TIMER("Postgres - execute query - BEGIN TRANSACTION");
    res = PQexec(c, "BEGIN TRANSACTION;");
        if (PQresultStatus(res) != PGRES_COMMAND_OK){
            STOP_TIMER("Postgres - execute query - BEGIN TRANSACTION");
            STOP_TIMER(METADATA_LOOKUP_EXEC_QUERY_TIME);
            CLOSE_RES_CONN_AND_FATAL(res, "BEGIN TRANSACTION for DECLARE CURSOR failed: %s",
                    PQerrorMessage(c));
        }
    PQclear(res);
    STOP_TIMER("Postgres - execute query - BEGIN TRANSACTION");

    START_TIMER("Postgres - execute query - DECLARE CURSOR");
    DEBUG_LOG("create cursor for %s", query);
    res = PQexec(c, CONCAT_STRINGS("DECLARE myportal CURSOR FOR ", query));
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        STOP_TIMER("Postgres - execute query - DECLARE CURSOR");
        STOP_TIMER(METADATA_LOOKUP_EXEC_QUERY_TIME);
        CLOSE_RES_CONN_AND_FATAL(res, "DECLARE CURSOR failed: %s",
                PQerrorMessage(c));
    }
    PQclear(res);
    STOP_TIMER("Postgres - execute query - DECLARE CURSOR");

    START_TIMER("Postgres - execute query - FETCH ALL");
    res = PQexec(c, "FETCH ALL in myportal");
    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        STOP_TIMER("Postgres - execute query - FETCH ALL");
        STOP_TIMER(METADATA_LOOKUP_EXEC_QUERY_TIME);
        CLOSE_RES_CONN_AND_FATAL(res, "FETCH ALL failed: %s", PQerrorMessage(c));
    }
    STOP_TIMER("Postgres - execute query - FETCH ALL");
    STOP_TIMER(METADATA_LOOKUP_EXEC_QUERY_TIME);
    return res;
}

static void
execCommit(void)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;
    START_TIMER(METADATA_LOOKUP_COMMIT_TIMER);
    res = PQexec(c, "COMMIT;");
        if (PQresultStatus(res) != PGRES_COMMAND_OK){
            STOP_TIMER(METADATA_LOOKUP_COMMIT_TIMER);
            CLOSE_RES_CONN_AND_FATAL(res, "COMMIT for DECLARE CURSOR failed: %s",
                    PQerrorMessage(c));
        }
    STOP_TIMER(METADATA_LOOKUP_COMMIT_TIMER);
    PQclear(res);

	STOP_TIMER_IF_RUNNING(METADATA_LOOKUP_QUERY_TIMER);
}

static PGresult *
execPrepared(char *qName, List *values)
{
    char **params;
    int i;
    int nParams = LIST_LENGTH(values);
    PGresult *res = NULL;
    params = CALLOC(sizeof(char*),LIST_LENGTH(values));

    START_TIMER(METADATA_LOOKUP_EXEC_PREPARED);
    ASSERT(postgresIsInitialized());
	START_TIMER(METADATA_LOOKUP_QUERY_TIMER);

    i = 0;
    FOREACH(Constant,c,values)
        params[i++] = STRING_VALUE(c);

    DEBUG_LOG("run query %s with parameters <%s>",
			  qName, exprToSQL((Node *) values, NULL, FALSE));

    res = PQexecPrepared(plugin->conn,
            qName,
            nParams,
            (const char *const *) params,
            NULL,
            NULL,
            0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK){
        STOP_TIMER(METADATA_LOOKUP_EXEC_PREPARED);
        CLOSE_RES_CONN_AND_FATAL(res, "query %s failed:\n%s", qName,
                PQresultErrorMessage(res));
    }
    STOP_TIMER(METADATA_LOOKUP_EXEC_PREPARED);
	STOP_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    return res;
}

static boolean
prepareQuery(char *qName, char *query, int parameters, Oid *types)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;
    START_TIMER(METADATA_LOOKUP_PREPARE_QUERY_TIME);
    res = PQprepare(c,
                    qName,
                    query,
                    parameters,
                    types);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        STOP_TIMER(METADATA_LOOKUP_PREPARE_QUERY_TIME);
        CLOSE_RES_CONN_AND_FATAL(res, "prepare query %s failed: %s",
                qName, PQresultErrorMessage(res));
    }
    PQclear(res);

    DEBUG_LOG("prepared query: %s AS\n%s", qName, query);
    STOP_TIMER(METADATA_LOOKUP_PREPARE_QUERY_TIME);
    return TRUE;
}

static DataType
postgresOidToDT(char *Oid)
{
    int oid = atoi(Oid);
    Constant *c = (Constant *) MAP_GET_INT(GET_CACHE()->oidToDT,oid);

    if (c == NULL)
        FATAL_LOG("did not find datatype for oid %s", Oid);
    return (DataType) INT_VALUE(c);
}

static DataType
postgresOidIntToDT(int oid)
{
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
            || streq(typName, "numeric")
			|| streq(typName, "float")
            )
        return DT_FLOAT;

    // boolean
    if (streq(typName,"bool"))
        return DT_BOOL;

    return DT_STRING;
}

Relation *
postgresExecuteQuery(char *query)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    Relation *r = makeNode(Relation);
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);

    // set schema
    r->schema = NIL;
    for(int i = 0; i < numFields; i++)
    {
        char *name = PQfname(rs, i);
        r->schema = appendToTailOfList(r->schema, strdup((char *) name));
    }

    // read rows
    // r->tuples = NIL;
    r->tuples = makeVector(VECTOR_NODE, T_Vector);
    for(int i = 0; i < numRes; i++)
    {
        // List *tuple = NIL;
        Vector *tuple = makeVectorOfSize(VECTOR_STRING, -1, numFields);
        for (int j = 0; j < numFields; j++)
        {
            if (PQgetisnull(rs,i,j))
            {
                // tuple = appendToTailOfList(tuple, strdup("NULL"));
                vecAppendString(tuple, strdup("NULL"));
            }
            else
            {
                char *val = PQgetvalue(rs,i,j);
                // tuple = appendToTailOfList(tuple, strdup(val));
                vecAppendString(tuple, strdup(val));
            }
        }
        // r->tuples = appendToTailOfList(r->tuples, tuple);
        // DEBUG_LOG("read tuple <%s>", stringListToString(tuple));
        VEC_ADD_NODE(r->tuples, tuple);
        DEBUG_NODE_LOG("read tuple <%s>", tuple);
    }
    PQclear(rs);
    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQuery");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
    return r;
}

void
postgresExecuteQueryIgnoreResult (char *query)
{
    PGresult *res = NULL;
    ASSERT(postgresIsInitialized());
    PGconn *c = plugin->conn;
    boolean done = FALSE;
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQueryIgnoreResult");
    START_TIMER("Postgres - execute ExecuteQueryIgnoreResult - Begin Transaction");
    // start transaction
    res = PQexec(c, CONCAT_STRINGS("BEGIN TRANSACTION;"));
        if (PQresultStatus(res) != PGRES_COMMAND_OK){
            STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult - Begin Transaction");
            CLOSE_RES_CONN_AND_FATAL(res, "DECLARE CURSOR failed: %s",
                    PQerrorMessage(c));
        }
    PQclear(res);
    STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult - Begin Transaction");

    // create a cursor
    DEBUG_LOG("create cursor for %s", query);
    START_TIMER("Postgres - execute ExecuteQueryIgnoreResult - Declare Cursor");
    res = PQexec(c, CONCAT_STRINGS("DECLARE myportal CURSOR FOR ", query));
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult - Declare Cursor");
        CLOSE_RES_CONN_AND_FATAL(res, "DECLARE CURSOR failed: %s",
                PQerrorMessage(c));
    }
    PQclear(res);
    STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult - Declare Cursor");

    START_TIMER("Postgres - execute ExecuteQueryIgnoreResult - FETCH");
    while(!done)
    {
        res = PQexec(c, "FETCH 1000 FROM myportal");
        if (PQresultStatus(res) != PGRES_TUPLES_OK){
            STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult - FETCH");
            CLOSE_RES_CONN_AND_FATAL(res, "FETCH ALL failed: %s", PQerrorMessage(c));
        }
        int numRes = PQntuples(res);
        if (numRes == 0)
            done = TRUE;
    }
    STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult - FETCH");

    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQueryIgnoreResult");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

void postgresExecuteStatement(char* sql) {
	execStmt(sql);
}

void
postgresGetDataChunkFromDeltaTable(char *query, DataChunk *dcIns, DataChunk *dcDel, int psAttrPos, Vector *rangeList, char *psName)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);
	HashMap *posToDT = dcIns->posToDatatype;
    INFO_LOG("res len: %d", numRes);
	if (numRes < 1) {
		return;
	}

    char *updateIdentName = getStringOption(OPTION_UPDATE_PS_DELTA_TABLE_UPDIDENT);
    Vector *updType = makeVector(VECTOR_INT, T_Vector);
    int insNum = 0;
    int delNum = 0;
    int updTypeCol = numFields - 1;
    if (strcmp(updateIdentName, "DELETETUPLES") == 0) {
        for (int i = 0; i < numRes; i++) {
            vecAppendInt(updType, -1);
        }
        delNum = numRes;
    } else if (strcmp(updateIdentName, "INSERTTUPLES") == 0) {
        for (int i = 0; i < numRes; i++) {
            vecAppendInt(updType, 1);
        }
        insNum = numRes;
    } else {
        for (int col = 0; col  < numFields; col++) {
            char *name = PQfname(rs, col);
            if (strcmp(name, updateIdentName) == 0) {
                updTypeCol = col;
                for (int row = 0; row < numRes; row++) {
                    int value = atoi(PQgetvalue(rs, row, col));
                    vecAppendInt(updType, value);
                    if (value == 1) {
                        insNum++;
                    } else {
                        delNum++;
                    }
                }
                break;
            }
        }
    }
    dcIns->numTuples = insNum;
    dcDel->numTuples = delNum;
    // append value to data chunks;

    int *updTypeVals = VEC_TO_IA(updType);
    Vector *psVecIns = NULL;
    Vector *psVecDel = NULL;
    if (psAttrPos != -1) {
        psVecIns = makeVector(VECTOR_INT, T_Vector);
        psVecDel = makeVector(VECTOR_INT, T_Vector);
    }
    for (int col = 0; col < numFields; col++) {
        if (col == updTypeCol) {
            continue;
        }
        DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(posToDT, col));
        Vector *colVecIns = NULL;
        Vector *colVecDel = NULL;
        boolean isPSCol = (col == psAttrPos);
        switch (dataType) {
            case DT_INT:
            {
                colVecIns = makeVector(VECTOR_INT, T_Vector);
                colVecDel = makeVector(VECTOR_INT, T_Vector);

                for (int row = 0; row < numRes; row++) {
                    int value = atoi(PQgetvalue(rs, row, col));
                    if (updTypeVals[row] == 1) {
                        vecAppendInt(colVecIns, value);
                    } else {
                        vecAppendInt(colVecDel, value);
                    }
                    if (isPSCol) {
                        int bitVal = setFragmengtToInt(value, rangeList);
                        if (updTypeVals[row] == 1) {
                            vecAppendInt(psVecIns, bitVal);
                        } else {
                            vecAppendInt(psVecDel, bitVal);
                        }
                    }
                }
            }
            break;
            case DT_LONG:
            {
                colVecIns = makeVector(VECTOR_LONG, T_Vector);
                colVecDel = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					gprom_long_t value = atol(PQgetvalue(rs, row, col));
                    if (updTypeVals[row] == 1) {
					    vecAppendLong(colVecIns, value);
                    } else {
					    vecAppendLong(colVecDel, value);
                    }
				}
            }
            break;
            case DT_FLOAT:
            {
                colVecIns = makeVector(VECTOR_FLOAT, T_Vector);
                colVecDel = makeVector(VECTOR_FLOAT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					double value = atof(PQgetvalue(rs, row, col));
                    if (updTypeVals[row] == 1) {
					    vecAppendFloat(colVecIns, value);
                    } else {
					    vecAppendFloat(colVecDel, value);
                    }
				}
            }
            break;
            case DT_BOOL:
            {
                colVecIns = makeVector(VECTOR_INT, T_Vector);
                colVecDel = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true")) {
                        if (updTypeVals[row] == 1) {
                            vecAppendInt(colVecIns, 1);
                        } else {
                            vecAppendInt(colVecDel, 1);

                        }
                    }
    				if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false")) {
                        if (updTypeVals[row] == 1) {
                            vecAppendInt(colVecIns, 0);
                        } else {
                            vecAppendInt(colVecDel, 0);
                        }
                    }
				}
            }
            break;
            case DT_STRING:
            case DT_VARCHAR2:
            {
                colVecIns = makeVector(VECTOR_STRING, T_Vector);
                colVecDel = makeVector(VECTOR_STRING, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
                    if (updTypeVals[row] == 1) {
					    vecAppendString(colVecIns, strdup(value));
                    } else {
					    vecAppendString(colVecDel, strdup(value));
                    }
				}
            }
            break;
            default:
				FATAL_LOG("not supported");
        }
        if (insNum > 0) {
            vecAppendNode(dcIns->tuples, (Node *) colVecIns);
        }
        if (delNum > 0) {
            vecAppendNode(dcDel->tuples, (Node *) colVecDel);
        }

        if (isPSCol) {
            if (insNum > 0) {
                addToMap(dcIns->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVecIns);
                addToMap(dcIns->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
                dcIns->isAPSChunk = TRUE;
            }
            if (delNum > 0) {
                addToMap(dcDel->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVecDel);
                addToMap(dcDel->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
                dcDel->isAPSChunk = TRUE;
            }
        }
    }
    for (int i = 0; i < insNum; i++) {
        vecAppendInt(dcIns->updateIdentifier, 1);
    }
    for (int i = 0; i < delNum; i++) {
        vecAppendInt(dcDel->updateIdentifier, -1);
    }
    PQclear(rs);
    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQuery");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

void
postgresGetDataChunkFromDelta(char *query, DataChunk *dc, int psAttrPos, Vector *rangeList, char *psName, boolean isIns)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);
	HashMap *posToDT = dc->posToDatatype;
	dc->numTuples = numRes;
    INFO_LOG("res len: %d", numRes);
	if (numRes < 1) {
		return;
	}

	Vector *psVec = NULL;
	if (psAttrPos != -1) {
		psVec = makeVector(VECTOR_INT, T_Vector);
	}
	for (int col = 0; col < numFields; col++) {
		DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(posToDT, col));
		Vector *colVec = NULL;
		boolean isPSCol = (col == psAttrPos);
		switch (dataType) {
			case DT_INT:
			{
				colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					int value = atoi(PQgetvalue(rs, row, col));
					vecAppendInt(colVec, value);
					if (isPSCol) {
						// BitSet *bitset = setFragmentToBitSet(value, rangeList);
                        int bitSet = setFragmengtToInt(value, rangeList);
						// vecAppendNode(psVec, (Node *) bitset);
                        // INFO_LOG("bitset %d", bitSet);
                        vecAppendInt(psVec, bitSet);
					}
				}
			}
				break;
			case DT_LONG:
			{
				colVec = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					gprom_long_t value = atol(PQgetvalue(rs, row, col));
					vecAppendLong(colVec, value);
				}
			}
				break;
			case DT_FLOAT:
			{
				colVec = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					double value = atof(PQgetvalue(rs, row, col));
					vecAppendFloat(colVec, value);
				}
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				colVec = makeVector(VECTOR_STRING, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					vecAppendString(colVec, strdup(value));
				}
			}
				break;
			case DT_BOOL:
			{
				colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true"))
					{ vecAppendInt(colVec, 1); }
    				if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false"))
					{ vecAppendInt(colVec, 0); }
				}
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		vecAppendNode(dc->tuples, (Node *) colVec);
		if (isPSCol) {
			addToMap(dc->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVec);
            addToMap(dc->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
			dc->isAPSChunk = TRUE;
		}
	}
    int updIde = 1;
    if (!isIns) {
        updIde = -1;
    }
	for (int row = 0; row < numRes; row++) {
		vecAppendInt(dc->updateIdentifier, updIde);
	}
    // DEBUG_NODE_BEATIFY_LOG("PS:", dc->fragmentsInfo);
    // DEBUG_NODE_BEATIFY_LOG("DELETE CHUNK IN POSTGRES", dc);
    PQclear(rs);
    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQuery");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

void
postgresGetDataChunkFromStmtInsDel(char *query, DataChunk* dc, int psAttrPos, Vector *rangeList, char *psName, int type)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);
	HashMap *posToDT = dc->posToDatatype;
	dc->numTuples = numRes;
    INFO_LOG("res len: %d", numRes);
	if (numRes < 1) {
		return;
	}

	Vector *psVec = NULL;
	if (psAttrPos != -1) {
		psVec = makeVector(VECTOR_INT, T_Vector);
	}
	for (int col = 0; col < numFields; col++) {
		DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(posToDT, col));
		Vector *colVec = NULL;
		boolean isPSCol = (col == psAttrPos);
		switch (dataType) {
			case DT_INT:
			{
				colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					int value = atoi(PQgetvalue(rs, row, col));
					vecAppendInt(colVec, value);
					if (isPSCol) {
						// BitSet *bitset = setFragmentToBitSet(value, rangeList);
                        int bitSet = setFragmengtToInt(value, rangeList);
						// vecAppendNode(psVec, (Node *) bitset);
                        // INFO_LOG("bitset %d", bitSet);
                        vecAppendInt(psVec, bitSet);
					}
				}
			}
				break;
			case DT_LONG:
			{
				colVec = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					gprom_long_t value = atol(PQgetvalue(rs, row, col));
					vecAppendLong(colVec, value);
				}
			}
				break;
			case DT_FLOAT:
			{
				colVec = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					double value = atof(PQgetvalue(rs, row, col));
					vecAppendFloat(colVec, value);
				}
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				colVec = makeVector(VECTOR_STRING, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					vecAppendString(colVec, strdup(value));
				}
			}
				break;
			case DT_BOOL:
			{
				colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true"))
					{ vecAppendInt(colVec, 1); }
    				if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false"))
					{ vecAppendInt(colVec, 0); }
				}
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		vecAppendNode(dc->tuples, (Node *) colVec);
		if (isPSCol) {
			addToMap(dc->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVec);
            addToMap(dc->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
			dc->isAPSChunk = TRUE;
		}
	}
    if (type == 1) {
        for (int row = 0; row < numRes; row++) {
		    vecAppendInt(dc->updateIdentifier, 1);
	    }
    } else if (type == -1) {
        for (int row = 0; row < numRes; row++) {
		    vecAppendInt(dc->updateIdentifier, -1);
	    }
    }

    DEBUG_NODE_BEATIFY_LOG("PS:", dc->fragmentsInfo);
    DEBUG_NODE_BEATIFY_LOG("DELETE CHUNK IN POSTGRES", dc);
    PQclear(rs);
    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQuery");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

void
postgresGetDataChunkDelete(char *query, DataChunk* dc, int psAttrPos, Vector *rangeList, char *psName)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);
	HashMap *posToDT = dc->posToDatatype;
	dc->numTuples = numRes;
    INFO_LOG("res len: %d", numRes);
	if (numRes < 1) {
		return;
	}

	Vector *psVec = NULL;
	if (psAttrPos != -1) {
		psVec = makeVector(VECTOR_INT, T_Vector);
	}
	for (int col = 0; col < numFields; col++) {
		DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(posToDT, col));
		Vector *colVec = NULL;
		boolean isPSCol = (col == psAttrPos);
		switch (dataType) {
			case DT_INT:
			{
				colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					int value = atoi(PQgetvalue(rs, row, col));
					vecAppendInt(colVec, value);
					if (isPSCol) {
						// BitSet *bitset = setFragmentToBitSet(value, rangeList);
                        int bitSet = setFragmengtToInt(value, rangeList);
						// vecAppendNode(psVec, (Node *) bitset);
                        // INFO_LOG("bitset %d", bitSet);
                        vecAppendInt(psVec, bitSet);
					}
				}
			}
				break;
			case DT_LONG:
			{
				colVec = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					gprom_long_t value = atol(PQgetvalue(rs, row, col));
					vecAppendLong(colVec, value);
				}
			}
				break;
			case DT_FLOAT:
			{
				colVec = makeVector(VECTOR_LONG, T_Vector);
				for (int row = 0; row < numRes; row++) {
					double value = atof(PQgetvalue(rs, row, col));
					vecAppendFloat(colVec, value);
				}
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				colVec = makeVector(VECTOR_STRING, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					vecAppendString(colVec, strdup(value));
				}
			}
				break;
			case DT_BOOL:
			{
				colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true"))
					{ vecAppendInt(colVec, 1); }
    				if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false"))
					{ vecAppendInt(colVec, 0); }
				}
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		vecAppendNode(dc->tuples, (Node *) colVec);
		if (isPSCol) {
			addToMap(dc->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVec);
            addToMap(dc->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
			dc->isAPSChunk = TRUE;
		}
	}

	for (int row = 0; row < numRes; row++) {
		vecAppendInt(dc->updateIdentifier, -1);
	}
    DEBUG_NODE_BEATIFY_LOG("PS:", dc->fragmentsInfo);
    DEBUG_NODE_BEATIFY_LOG("DELETE CHUNK IN POSTGRES", dc);
    PQclear(rs);
    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQuery");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

void
postgresGetDataChunkGroupJoin(char *query, DataChunk* dcIns, DataChunk *dcDel, int whichBranch, HashMap *psAttrAndLevel)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery For Join Operator");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    START_TIMER("POSTGRES - GET RESULT FROM DB");
    PGresult *rs = execQuery(query);
    STOP_TIMER("POSTGRES - GET RESULT FROM DB");
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);
	if (numRes < 1) {
		return;
	}

    START_TIMER("POSTGRES - BUILD DATA CHUNK");
    // get all update identifier from res;
    Vector *updateIdent = makeVector(VECTOR_INT, T_Vector);
    int leftUpdPos = -1;
    int rightUpdPos = -1;

    // if (whichBranch == -1 || whichBranch == 1) {
    //     int updateIdentPos = -1;
    //     if (whichBranch == -1) {
    //         // leftUpdPos = PQfnumber(rs, JOIN_LEFT_BRANCH_IDENTIFIER);
    //         updateIdentPos = leftUpdPos;
    //     } else if (whichBranch == 1) {
    //         rightUpdPos= PQfnumber(rs, JOIN_RIGHT_BRANCH_IDENTIFIER);
    //         updateIdentPos = rightUpdPos;
    //     }
    //     for (int row = 0; row < numRes; row++) {
    //         vecAppendInt(updateIdent, atoi(PQgetvalue(rs, row, updateIdentPos)));
    //     }
    // } else if (whichBranch == 0) {
    //     leftUpdPos = PQfnumber(rs, JOIN_LEFT_BRANCH_IDENTIFIER);
    //     rightUpdPos = PQfnumber(rs, JOIN_RIGHT_BRANCH_IDENTIFIER);
    //     for (int row = 0; row < numRes; row++) {
    //         int left = atoi(PQgetvalue(rs, row, leftUpdPos));
    //         int right = atoi(PQgetvalue(rs, row, rightUpdPos));
    //         vecAppendInt(updateIdent, left * right);
    //     }
    // }
    //

    int *updValsArr = (int *) VEC_TO_IA(updateIdent);
    for (int col = 0; col < numFields; col++) {
        if (col == leftUpdPos || col == rightUpdPos) {
            continue;
        }

        char *attrName = PQfname(rs, col);
        if (MAP_HAS_STRING_KEY(psAttrAndLevel, attrName)) {
            dcIns->isAPSChunk = TRUE;
            dcDel->isAPSChunk = TRUE;
            List *prov_level_num = (List *) MAP_GET_STRING(psAttrAndLevel, attrName);
            int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num, 0));
            Vector *psVecIns = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, attrName);
            Vector *psVecDel = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, attrName);
            if (provLevel < 2) { // prov is an integer;
                if (psVecIns == NULL)
                {
                    psVecIns = makeVector(VECTOR_INT, T_Vector);
                }
                if (psVecDel == NULL)
                {
                    psVecDel = makeVector(VECTOR_INT, T_Vector);
                }

                for (int row = 0; row < numRes; row++) {
                    int psVal = atoi(PQgetvalue(rs, row, col));
                    if (updValsArr[row] == 1) {
                        vecAppendInt(psVecIns, psVal);
                    } else {
                        vecAppendInt(psVecDel, psVal);
                    }
                }
                if (psVecIns->length > 0) {
                    addToMap(dcIns->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecIns);
                    addToMap(dcIns->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(TRUE));
                }
                if (psVecDel->length > 0) {
                    addToMap(dcDel->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecDel);
                    addToMap(dcDel->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(TRUE));
                }
            } else {
                if (psVecIns == NULL) {
                    psVecIns = makeVector(VECTOR_NODE, T_Vector);
                }
                if (psVecDel == NULL) {
                    psVecDel = makeVector(VECTOR_NODE, T_Vector);
                }
                for (int row = 0; row < numRes; row++) {
                    char *psVal = PQgetvalue(rs, row, col);
                    if (updValsArr[row] == 1) {
                        vecAppendNode(psVecIns, (Node *) stringToBitset(psVal));
                    } else {
                        vecAppendNode(psVecDel, (Node *) stringToBitset(psVal));
                    }
                }
                if (psVecIns->length > 0) {
                    addToMap(dcIns->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecIns);
                    addToMap(dcIns->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(FALSE));
                }
                if (psVecDel->length > 0) {
                    addToMap(dcDel->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecDel);
                    addToMap(dcDel->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(FALSE));
                }
            }
        } else {
            int attrPos = INT_VALUE((Constant *) MAP_GET_STRING(dcIns->attriToPos, attrName));
            DataType attrType = INT_VALUE((Constant *) MAP_GET_INT(dcIns->posToDatatype, attrPos));

            Vector *vecIns = (Vector *) getVecNode(dcIns->tuples, attrPos);
            Vector *vecDel = (Vector *) getVecNode(dcDel->tuples, attrPos);

            switch (attrType) {
                case DT_INT:
                {
                    for (int row = 0; row < numRes; row++) {
                        int value = atoi(PQgetvalue(rs, row, col));
                        if (updValsArr[row] == 1) {
                            vecAppendInt(vecIns, value);
                        } else {
                            vecAppendInt(vecDel, value);
                        }
                    }
                }
                    break;
                case DT_LONG:
                {
                    for (int row = 0; row < numRes; row++) {
                        gprom_long_t value = atol(PQgetvalue(rs, row, col));
                        if (updValsArr[row] == 1) {
                            vecAppendLong(vecIns, value);
                        } else {
                            vecAppendLong(vecDel, value);
                        }
                    }
                }
                    break;
                case DT_FLOAT:
                {
                    for (int row = 0; row < numRes; row++) {
                        double value = atof(PQgetvalue(rs, row, col));
                        if (updValsArr[row] == 1) {
                            vecAppendFloat(vecIns, value);
                        } else {
                            vecAppendFloat(vecDel, value);
                        }
                    }
                }
                    break;
                case DT_BOOL:
                {
                    for (int row = 0; row < numRes; row++) {
                        char *value = PQgetvalue(rs, row, col);
                        if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true")) {
                            if (updValsArr[row] == 1) {
                                vecAppendInt(vecIns, 1);
                            } else {
                                vecAppendInt(vecDel, 1);
                            }
                        }
    				    if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false")) {
                            if (updValsArr[row] == 1) {
                                vecAppendInt(vecIns, 0);
                            } else {
                                vecAppendInt(vecDel, 0);
                            }
                        }

                    }
                }
                    break;
                case DT_STRING:
                case DT_VARCHAR2:
                {
                    for (int row = 0; row < numRes; row++) {
                        char *value = PQgetvalue(rs, row, col);
                        if (updValsArr[row] == 1) {
                            vecAppendString(vecIns, strdup(value));
                        } else {
                            vecAppendString(vecDel, strdup(value));
                        }
                    }
                }
                    break;
                default:
                    FATAL_LOG("not supported");
            }
        }

    }
    // append update identifier;
    for (int row = 0; row < numRes; row++) {
        // if (updValsArr[row] == 1) {
            // vecAppendInt(dcIns->updateIdentifier, 1);
        // } else {
            vecAppendInt(dcDel->updateIdentifier, -1);
        // }
    }
    dcIns->numTuples = dcIns->updateIdentifier->length;
    dcDel->numTuples = dcDel->updateIdentifier->length;

    PQclear(rs);
    execCommit();
    STOP_TIMER("POSTGRES - BUILD DATA CHUNK");
    STOP_TIMER("Postgres - execute ExecuteQuery For Join Operator");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

/*
    get data chunk for join operator:
    branch -1: left, 1: right, 0: both;
*/
void
postgresGetDataChunkJoin(char *query, DataChunk* dcIns, DataChunk *dcDel, int whichBranch, HashMap *psAttrAndLevel)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery For Join Operator");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    START_TIMER("POSTGRES - GET RESULT FROM DB");
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);
	if (numRes < 1) {
        INFO_LOG("NO RESULTS");
        PQclear(rs);
        execCommit();
        // STOP_TIMER(METADATA_LOOKUP_TIMER);
        // STOP_TIMER(METADATA_LOOKUP_QUERY_TIMER);
        STOP_TIMER("POSTGRES - GET RESULT FROM DB");
        STOP_TIMER("Postgres - execute ExecuteQuery For Join Operator");
		return;
	}

    STOP_TIMER("POSTGRES - GET RESULT FROM DB");
    START_TIMER("POSTGRES - BUILD DATA CHUNK");
    // get all update identifier from res;
    Vector *updateIdent = makeVector(VECTOR_INT, T_Vector);
    int leftUpdPos = -1;
    int rightUpdPos = -1;

    if (whichBranch == -1 || whichBranch == 1) {
        int updateIdentPos = -1;
        if (whichBranch == -1) {
            leftUpdPos = PQfnumber(rs, JOIN_LEFT_BRANCH_IDENTIFIER);
            updateIdentPos = leftUpdPos;
        } else if (whichBranch == 1) {
            rightUpdPos= PQfnumber(rs, JOIN_RIGHT_BRANCH_IDENTIFIER);
            updateIdentPos = rightUpdPos;
        }
        for (int row = 0; row < numRes; row++) {
            vecAppendInt(updateIdent, atoi(PQgetvalue(rs, row, updateIdentPos)));
        }
    } else if (whichBranch == 0) {
        leftUpdPos = PQfnumber(rs, JOIN_LEFT_BRANCH_IDENTIFIER);
        rightUpdPos = PQfnumber(rs, JOIN_RIGHT_BRANCH_IDENTIFIER);
        for (int row = 0; row < numRes; row++) {
            int left = atoi(PQgetvalue(rs, row, leftUpdPos));
            int right = atoi(PQgetvalue(rs, row, rightUpdPos));
            vecAppendInt(updateIdent, left * right);
        }
    }

    int *updValsArr = (int *) VEC_TO_IA(updateIdent);
    for (int col = 0; col < numFields; col++) {
        if (col == leftUpdPos || col == rightUpdPos) {
            continue;
        }

        char *attrName = PQfname(rs, col);
        if (MAP_HAS_STRING_KEY(psAttrAndLevel, attrName)) {
            dcIns->isAPSChunk = TRUE;
            dcDel->isAPSChunk = TRUE;
            List *prov_level_num = (List *) MAP_GET_STRING(psAttrAndLevel, attrName);
            int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num, 0));
            Vector *psVecIns = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, attrName);
            Vector *psVecDel = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, attrName);
            if (provLevel < 2) { // prov is an integer;
                if (psVecIns == NULL)
                {
                    psVecIns = makeVector(VECTOR_INT, T_Vector);
                }
                if (psVecDel == NULL)
                {
                    psVecDel = makeVector(VECTOR_INT, T_Vector);
                }

                for (int row = 0; row < numRes; row++) {
                    int psVal = atoi(PQgetvalue(rs, row, col));
                    if (updValsArr[row] == 1) {
                        vecAppendInt(psVecIns, psVal);
                    } else {
                        vecAppendInt(psVecDel, psVal);
                    }
                }
                if (psVecIns->length > 0) {
                    addToMap(dcIns->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecIns);
                    addToMap(dcIns->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(TRUE));
                }
                if (psVecDel->length > 0) {
                    addToMap(dcDel->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecDel);
                    addToMap(dcDel->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(TRUE));
                }
            } else {
                if (psVecIns == NULL) {
                    psVecIns = makeVector(VECTOR_NODE, T_Vector);
                }
                if (psVecDel == NULL) {
                    psVecDel = makeVector(VECTOR_NODE, T_Vector);
                }
                for (int row = 0; row < numRes; row++) {
                    char *psVal = PQgetvalue(rs, row, col);
                    if (updValsArr[row] == 1) {
                        vecAppendNode(psVecIns, (Node *) stringToBitset(psVal));
                    } else {
                        vecAppendNode(psVecDel, (Node *) stringToBitset(psVal));
                    }
                }
                if (psVecIns->length > 0) {
                    addToMap(dcIns->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecIns);
                    addToMap(dcIns->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(FALSE));
                }
                if (psVecDel->length > 0) {
                    addToMap(dcDel->fragmentsInfo, (Node *) createConstString(attrName), (Node *) psVecDel);
                    addToMap(dcDel->fragmentsIsInt, (Node *) createConstString(attrName), (Node *) createConstBool(FALSE));
                }
            }
        } else {
            int attrPos = INT_VALUE((Constant *) MAP_GET_STRING(dcIns->attriToPos, attrName));
            DataType attrType = INT_VALUE((Constant *) MAP_GET_INT(dcIns->posToDatatype, attrPos));

            Vector *vecIns = (Vector *) getVecNode(dcIns->tuples, attrPos);
            Vector *vecDel = (Vector *) getVecNode(dcDel->tuples, attrPos);

            switch (attrType) {
                case DT_INT:
                {
                    for (int row = 0; row < numRes; row++) {
                        int value = atoi(PQgetvalue(rs, row, col));
                        if (updValsArr[row] == 1) {
                            vecAppendInt(vecIns, value);
                        } else {
                            vecAppendInt(vecDel, value);
                        }
                    }
                }
                    break;
                case DT_LONG:
                {
                    for (int row = 0; row < numRes; row++) {
                        gprom_long_t value = atol(PQgetvalue(rs, row, col));
                        if (updValsArr[row] == 1) {
                            vecAppendLong(vecIns, value);
                        } else {
                            vecAppendLong(vecDel, value);
                        }
                    }
                }
                    break;
                case DT_FLOAT:
                {
                    for (int row = 0; row < numRes; row++) {
                        double value = atof(PQgetvalue(rs, row, col));
                        if (updValsArr[row] == 1) {
                            vecAppendFloat(vecIns, value);
                        } else {
                            vecAppendFloat(vecDel, value);
                        }
                    }
                }
                    break;
                case DT_BOOL:
                {
                    for (int row = 0; row < numRes; row++) {
                        char *value = PQgetvalue(rs, row, col);
                        if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true")) {
                            if (updValsArr[row] == 1) {
                                vecAppendInt(vecIns, 1);
                            } else {
                                vecAppendInt(vecDel, 1);
                            }
                        }
    				    if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false")) {
                            if (updValsArr[row] == 1) {
                                vecAppendInt(vecIns, 0);
                            } else {
                                vecAppendInt(vecDel, 0);
                            }
                        }

                    }
                }
                    break;
                case DT_STRING:
                case DT_VARCHAR2:
                {
                    for (int row = 0; row < numRes; row++) {
                        char *value = PQgetvalue(rs, row, col);
                        if (updValsArr[row] == 1) {
                            vecAppendString(vecIns, strdup(value));
                        } else {
                            vecAppendString(vecDel, strdup(value));
                        }
                    }
                }
                    break;
                default:
                    FATAL_LOG("not supported");
            }
        }

    }
    // append update identifier;
    for (int row = 0; row < numRes; row++) {
        if (updValsArr[row] == 1) {
            vecAppendInt(dcIns->updateIdentifier, 1);
        } else {
            vecAppendInt(dcDel->updateIdentifier, -1);
        }
    }
    dcIns->numTuples = dcIns->updateIdentifier->length;
    dcDel->numTuples = dcDel->updateIdentifier->length;

    PQclear(rs);
    execCommit();
    STOP_TIMER("POSTGRES - BUILD DATA CHUNK");
    STOP_TIMER("Postgres - execute ExecuteQuery For Join Operator");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

void
postgresGetDataChunkFromStmtUpd(char *query, DataChunk* dcIns, DataChunk* dcDel, int psAttrPos, Vector *rangeList, char *psName)
{
    START_TIMER(METADATA_LOOKUP_TIMER);
    START_TIMER("Postgres - execute ExecuteQuery");
    START_TIMER(METADATA_LOOKUP_QUERY_TIMER);
    // Relation *r = makeNode(Relation);
    PGresult *rs = execQuery(query);
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);

    if (numRes < 1) {

        return;
    }

    dcIns->numTuples = numRes;
    dcIns->tupleFields = numFields / 2;
    dcDel->numTuples = numRes;
    dcDel->tupleFields = numFields / 2;



    Vector *psVecIns = NULL;
    Vector *psVecDel = NULL;
    if (psAttrPos != -1) {
        psVecIns = makeVector(VECTOR_INT, T_Vector);
        psVecDel = makeVector(VECTOR_INT, T_Vector);
    }


    for (int col = 0; col < numFields; col++) {
        int acturalCol = col / 2;
        Vector *colVec = NULL;
        boolean isPSCol = (acturalCol == psAttrPos);

        Vector *psVec = NULL;
        if (isPSCol) {
            if (col % 2 == 1) {
                psVec = psVecIns;
            } else {
                psVec = psVecDel;
            }
        }
        DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(dcIns->posToDatatype, acturalCol));
        switch (dataType) {
            case DT_INT:
            {
                colVec = makeVector(VECTOR_INT, T_Vector);
                for (int row = 0; row < numRes; row++) {
                    int value = atoi(PQgetvalue(rs, row, col));
                    vecAppendInt(colVec, value);
                    if (isPSCol) {
                        int bitset = setFragmengtToInt(value, rangeList);
                        vecAppendInt(psVec, bitset);
                    }
                }
            }
                break;
            case DT_LONG:
            {
                colVec = makeVector(VECTOR_LONG, T_Vector);
                for (int row = 0; row < numRes; row++) {
                    gprom_long_t value = atol(PQgetvalue(rs, row, col));
                    vecAppendLong(colVec, value);
                }
            }
                break;
            case DT_FLOAT:
            {
                colVec = makeVector(VECTOR_FLOAT, T_Vector);
                for (int row = 0; row < numRes; row++) {
                    double value = atof(PQgetvalue(rs, row, col));
                    vecAppendFloat(colVec, value);
                }
            }
                break;
            case DT_VARCHAR2:
            case DT_STRING:
            {
                colVec = makeVector(VECTOR_NODE, T_Vector);
                for (int row = 0; row < numRes; row++) {
                    char *value = PQgetvalue(rs, row, col);
                    vecAppendString(colVec, strdup(value));
                }
            }
                break;
            case DT_BOOL:
            {
                colVec = makeVector(VECTOR_INT, T_Vector);
				for (int row = 0; row < numRes; row++) {
					char *value = PQgetvalue(rs, row, col);
					if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true"))
					{ vecAppendInt(colVec, 1); }
    				if (streq(value, "FALSE") || streq(value, "f") || streq(value, "false"))
					{ vecAppendInt(colVec, 0); }
				}
            }
                break;
            default:
                FATAL_LOG("not supported");
        }

        if (col % 2 == 1) {
            vecAppendNode(dcIns->tuples, (Node *) colVec);
            if (isPSCol) {
                addToMap(dcIns->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVecIns);
                addToMap(dcIns->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
                dcIns->isAPSChunk = TRUE;
            }
        } else {
            vecAppendNode(dcDel->tuples, (Node *) colVec);
            if (isPSCol) {
                addToMap(dcDel->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVecDel);
                addToMap(dcDel->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
                dcDel->isAPSChunk = TRUE;
            }
        }
    }

    // fill update identifier;
    for (int row = 0; row < numRes; row++) {
        vecAppendInt(dcIns->updateIdentifier, 1);
        vecAppendInt(dcDel->updateIdentifier, -1);
    }

    PQclear(rs);
    execCommit();
    STOP_TIMER("Postgres - execute ExecuteQuery");
    STOP_TIMER(METADATA_LOOKUP_TIMER);
}

List *
postgresCopyDeltaToDBWithBF(HashMap *dataChunks, char *deltaTableName, char *updIdent, Vector *insCheck, Vector *delCheck)
{
    List *deltaAttrDefs = NIL;
    // totalNum indicate all tuple numbers of both ins and del DC;
    int totalNum = 0;
    INFO_LOG("COPY DELTA TO DB");
    DataChunk *dcIns = NULL;
    DataChunk *dcDel = NULL;

    if (MAP_HAS_STRING_KEY(dataChunks, PROP_DATA_CHUNK_INSERT)) {
        dcIns = (DataChunk *) MAP_GET_STRING(dataChunks, PROP_DATA_CHUNK_INSERT);
    }

    if (MAP_HAS_STRING_KEY(dataChunks, PROP_DATA_CHUNK_DELETE)) {
        dcDel = (DataChunk *) MAP_GET_STRING(dataChunks, PROP_DATA_CHUNK_DELETE);
    }

    // init delta infomations
    StringInfo createTableSQL = makeStringInfo();
    Vector *deltaTuples = makeVector(VECTOR_NODE, T_Vector);

    // delta table name;
    appendStringInfo(createTableSQL, "create table %s ", deltaTableName);

    DataChunk *dcInfo = NULL;
    int insNum = 0;
    int delNum = 0;
    int *insCheckValue = NULL;
    int *delCheckValue = NULL;
    START_TIMER("Postgres - JOIN OPERATOR COPY DATA INTO DB");
    // init deltatuples;
    if (dcIns) {
        insNum = dcIns->numTuples;
        insCheckValue = VEC_TO_IA(insCheck);
        for (int row = 0; row < insNum; row++) {
            if (insCheckValue[row] == 1) {
                totalNum++;
                vecAppendNode(deltaTuples, (Node *) makeStringInfo());
            }
        }
        dcInfo = dcIns;
    }
    if (dcDel) {
        delNum = dcDel->numTuples;
        delCheckValue = VEC_TO_IA(delCheck);
        for (int row = 0; row < delNum; row++) {
            if (delCheckValue[row] == 1) {
                totalNum++;
                vecAppendNode(deltaTuples, (Node *) makeStringInfo());
            }
        }
        dcInfo = dcDel;
    }

    if (totalNum < 1) {
        return NIL;
    }

    int col = 0;
    // for create table sql;
    appendStringInfoString(createTableSQL, "(");
    // attr;
    FOREACH(AttributeDef, ad, dcInfo->attrNames) {
        char *format = NULL;
        if (col > 0) {
            appendStringInfoString(createTableSQL, ",");
            // format = ",%s";
            format = "\t%s";
        } else {
            format = "%s";
        }
        //attr name;
        deltaAttrDefs = appendToTailOfList(deltaAttrDefs, (AttributeDef *) copyObject(ad));
        appendStringInfo(createTableSQL, "%s ", strdup(ad->attrName));

        int attrPos = INT_VALUE((Constant *) MAP_GET_STRING(dcInfo->attriToPos, ad->attrName));
        DataType dt = INT_VALUE((Constant *) MAP_GET_INT(dcInfo->posToDatatype, attrPos));

        switch(dt) {
            case DT_INT:
            {
                appendStringInfoString(createTableSQL, "int ");

                int *dcInsVals = NULL;
                int *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_IA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_IA(vec);
                }

                // for ins;
                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, gprom_itoa(dcInsVals[row]));
                    }
                }

                // for del;
                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1){
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, gprom_itoa(dcDelVals[row]));
                    }
                }
            }
            break;
            case DT_BOOL:
            {
                appendStringInfoString(createTableSQL, "bool ");
                int *dcInsVals = NULL;
                int *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_IA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_IA(vec);
                }
                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, dcInsVals[row] == 1 ? strdup("true") : strdup("false"));
                    }
                }

                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, dcDelVals[row] == 1 ? strdup("true") : strdup("false"));

                    }
                }
            }
            break;
            case DT_LONG:
            {
                appendStringInfoString(createTableSQL, "bigint ");
                gprom_long_t *dcInsVals = NULL;
                gprom_long_t *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_LA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_LA(vec);
                }
                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, gprom_ltoa(dcInsVals[row]));
                    }
                }

                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, gprom_ltoa(dcDelVals[row]));
                    }
                }
            }
            break;
            case DT_FLOAT:
            {
                appendStringInfoString(createTableSQL, "float ");
                double *dcInsVals = NULL;
                double *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_FA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_FA(vec);
                }

                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, gprom_ftoa(dcInsVals[row]));
                    }
                }

                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, gprom_ftoa(dcDelVals[row]));

                    }
                }
            }
            break;
            case DT_VARCHAR2:
            case DT_STRING:
            {
                appendStringInfoString(createTableSQL, "varchar ");
                char **dcInsVals = NULL;
                char **dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = (char **) VEC_TO_ARR(vec, char);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = (char **) VEC_TO_ARR(vec, char);
                }

                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, strdup(dcInsVals[row]));
                    }
                }

                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1) {
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), format, strdup(dcDelVals[row]));
                    }
                }
            }
            break;
        }
        col++;
    }

    // provenance sketch;
    if (dcInfo->isAPSChunk) {
        // col = 0;
        FOREACH_HASH_KEY(Constant, c, dcInfo->fragmentsInfo) {
            // if (col > 0) {
                appendStringInfoString(createTableSQL, ",");
            // }

            boolean isPSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(dcInfo->fragmentsIsInt, STRING_VALUE(c)));
            if (isPSInt) {
                deltaAttrDefs = appendToTailOfList(deltaAttrDefs, createAttributeDef(strdup(STRING_VALUE(c)), DT_INT));
                appendStringInfo(createTableSQL, "%s %s", strdup(STRING_VALUE(c)), strdup("int"));
                int *psInsVals = NULL;
                int *psDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, STRING_VALUE(c));
                    psInsVals = VEC_TO_IA(vec);
                }

                if (dcDel) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, STRING_VALUE(c));
                    psDelVals = VEC_TO_IA(vec);
                }

                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        // appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), ",%s", gprom_itoa(psInsVals[row]));
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), "\t%s", gprom_itoa(psInsVals[row]));
                    }
                }
                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1) {
                        // appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), ",%s", gprom_itoa(psDelVals[row]));
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), "\t%s", gprom_itoa(psDelVals[row]));
                    }
                }
            } else {
                deltaAttrDefs = appendToTailOfList(deltaAttrDefs, createAttributeDef(strdup(STRING_VALUE(c)), DT_STRING));
                appendStringInfo(createTableSQL, "%s %s", strdup(STRING_VALUE(c)), strdup("varchar"));
                BitSet **psInsVals = NULL;
                BitSet **psDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, STRING_VALUE(c));
                    psInsVals = (BitSet **) VEC_TO_ARR(vec, BitSet);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, STRING_VALUE(c));
                    psDelVals = (BitSet **) VEC_TO_ARR(vec, BitSet);
                }
                int infoIdx = 0;
                for (int row = 0; row < insNum; row++) {
                    if (insCheckValue[row] == 1) {
                        // appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), ",%s", bitSetToString(psInsVals[row]));
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), "\t%s", bitSetToString(psInsVals[row]));
                    }
                }
                for (int row = 0; row < delNum; row++) {
                    if (delCheckValue[row] == 1) {
                        // appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), ",%s", bitSetToString(psDelVals[row]));
                        appendStringInfo((StringInfo) getVecNode(deltaTuples, infoIdx++), "\t%s", bitSetToString(psDelVals[row]));
                    }
                }
            }
        }

    }

    // update iden;
    deltaAttrDefs = appendToTailOfList(deltaAttrDefs, createAttributeDef(strdup(updIdent), DT_INT));
    appendStringInfo(createTableSQL, ", %s %s", strdup(updIdent), "int");
    int infoIdx = 0;
    for (int row = 0; row < insNum; row++) {
        if (insCheckValue[row] == 1) {
            // appendStringInfoString((StringInfo) getVecNode(deltaTuples, infoIdx++), ",1");
            appendStringInfoString((StringInfo) getVecNode(deltaTuples, infoIdx++), "\t1");
        }
    }
    for (int row = 0; row < delNum; row++) {
        if (delCheckValue[row] == 1) {
            // appendStringInfoString((StringInfo) getVecNode(deltaTuples, infoIdx++), ",-1");
            appendStringInfoString((StringInfo) getVecNode(deltaTuples, infoIdx++), "\t-1");
        }
    }
    appendStringInfoString(createTableSQL, ");");

    // Connection, Copy;
    PGresult *res = NULL;
    PGconn *conn = plugin->conn;

    INFO_LOG("Create Delta SQL: %s", createTableSQL->data);
    res = PQexec(conn, createTableSQL->data);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        INFO_LOG("ERROR MSG: %s", PQerrorMessage(conn));
        // FATAL_LOG("ERROR");
        CLOSE_RES_CONN_AND_FATAL(res, "create table error: %s", PQerrorMessage(conn));
    }
    // PQclear(res);
    const char *errormsg = NULL;

    // copy data: create "COPY XXX FROM STDIN(DELIMITER\',\');"
    StringInfo copyStartSQL = makeStringInfo();
    // appendStringInfo(copyStartSQL, "COPY %s FROM STDIN(DELIMITER \',\');", strdup(deltaTableName));
    appendStringInfo(copyStartSQL, "COPY %s FROM STDIN(DELIMITER \'\t\');", strdup(deltaTableName));
    // INFO_LOG("COPY START SQL %s", copyStartSQL->data);
    // PQexec(conn, copyStartSQL->data);
    // // copy data: PQputCopyData()
    // for (int row = 0; row < insNum + delNum; row++) {
    //     StringInfo str = (StringInfo) getVecNode(deltaTuples, row);
    //     appendStringInfoString(str, "\r");
    //     INFO_LOG("what is copy data: %s", str->data);
    //     PQputCopyData(conn, str->data, str->len);
    // }

    // approach 2: one bulk
    StringInfo allDeltas = makeStringInfo();
    for (int row = 0; row < totalNum; row++) {
        StringInfo str = (StringInfo) getVecNode(deltaTuples, row);
        appendStringInfo(allDeltas, "%s\r", str->data);
    }
    PQexec(conn, copyStartSQL->data);
    PQputCopyData(conn, allDeltas->data, allDeltas->len);


    // copy data: PQputCopyEnd()
    PQputCopyEnd(conn, errormsg);
    // PQexec(conn, "COMMIT;");
    execCommit();
    STOP_TIMER("Postgres - JOIN OPERATOR COPY DATA INTO DB");
    return deltaAttrDefs;
}

Vector *
postgresGetByteATest()
{
    PGresult *rs = execQuery("select * from bytea;");
    int numRes = PQntuples(rs);
    int numFields = PQnfields(rs);

    Vector *vec = makeVector(VECTOR_STRING, T_Vector);
    for (int row = 0; row < numRes; row++) {
        for (int col = 0; col < numFields; col++) {
            char * val = PQgetvalue(rs, row, col);
            size_t to_length;
            unsigned char *vv = (unsigned char *) PQunescapeBytea((unsigned char *) val, &to_length);
            vecAppendString(vec, (char *) vv);
        }
    }

    return vec;

}


void
postgresByteATest(Node *node)
{
    // parameterized query;
    // postgres prepareSQL;
    PGresult *res = NULL;
    PGconn *conn = plugin->conn;


    Vector *v = (Vector *) node;
    StringInfo sqll = makeStringInfo();
    appendStringInfo(sqll, "insert into bytea (a) values ");
    for (int i = 0; i < v->length; i++) {
        StringInfo info = (StringInfo) getVecNode(v, i);
        size_t to_length;

        unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) info->data, info->len, &to_length);
        if (i > 0)
            appendStringInfo(sqll, ",('%s')", (char *) bytea);
        else
            appendStringInfo(sqll, "('%s')", (char *) bytea);

    }
	res = PQexec(conn, sqll->data);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        INFO_LOG("ERROR MSG: %s", PQerrorMessage(conn));
        // FATAL_LOG("ERROR");
        CLOSE_RES_CONN_AND_FATAL(res, "create table error: %s", PQerrorMessage(conn));
    }
    // PQclear(res);
    StringInfo sql = makeStringInfo();
    for (int i = 0; i < v->length; i++) {
        StringInfo info = (StringInfo) getVecNode(v, i);
        size_t to_length;

        unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) info->data, info->len, &to_length);
        appendStringInfo(sql, "'%s'\r", (char *) bytea);
    }
    const char *errormsg = NULL;
    // copy data: create "COPY XXX FROM STDIN(DELIMITER\',\');"
    StringInfo copyStartSQL = makeStringInfo();
    INFO_LOG("WHAT IS STRINGINFO %s", sql->data);
    appendStringInfo(copyStartSQL, "COPY bytea2 (a) FROM STDIN (DELIMITER \',\');");

    // approach 2: one bulk
    PQexec(conn, copyStartSQL->data);
    PQputCopyData(conn, sql->data, sql->len);

    PQputCopyEnd(conn, errormsg);
    execCommit();

}


void
postgresCopyToDB(StringInfo createTBL, StringInfo dataInfo, char *tableName)
{

    // Connection, Copy;
    PGresult *res = NULL;
    PGconn *conn = plugin->conn;

    START_TIMER("Postgres - ORDER OPERATOR COPY DATA INTO DB");
    INFO_LOG("Create data: %s", tableName);
    INFO_LOG("RESTATE %s", dataInfo->data);
    res = PQexec(conn, createTBL->data);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        INFO_LOG("ERROR MSG: %s", PQerrorMessage(conn));
        // FATAL_LOG("ERROR");
        CLOSE_RES_CONN_AND_FATAL(res, "create table error: %s", PQerrorMessage(conn));
    }
    const char *errormsg = NULL;
    // copy data: create "COPY XXX FROM STDIN(DELIMITER\',\');"
    StringInfo copyStartSQL = makeStringInfo();
    appendStringInfo(copyStartSQL, "COPY %s FROM STDIN(DELIMITER \'\t\');", strdup(tableName));

    // approach 2: one bulk
    PQexec(conn, copyStartSQL->data);
    PQputCopyData(conn, dataInfo->data, dataInfo->len);


    int retV = PQputCopyEnd(conn, errormsg);
    if (retV < 0) {
        FATAL_LOG(errormsg);
    //    CLOSE_CONN_AND_FATAL(PQerrorMessage(conn));
    }
    execCommit();
    STOP_TIMER("Postgres - ORDER OPERATOR COPY DATA INTO DB");
}


List *
postgresCopyDeltaToDB(HashMap *dataChunks, char *deltaTableName, char *updIdent)
{
    List *deltaAttrDefs = NIL;
    INFO_LOG("COPY DELTA TO DB");
    DataChunk *dcIns = NULL;
    DataChunk *dcDel = NULL;

    if (MAP_HAS_STRING_KEY(dataChunks, PROP_DATA_CHUNK_INSERT)) {
        dcIns = (DataChunk *) MAP_GET_STRING(dataChunks, PROP_DATA_CHUNK_INSERT);
    }

    if (MAP_HAS_STRING_KEY(dataChunks, PROP_DATA_CHUNK_DELETE)) {
        dcDel = (DataChunk *) MAP_GET_STRING(dataChunks, PROP_DATA_CHUNK_DELETE);
    }

    // init delta infomations
    StringInfo createTableSQL = makeStringInfo();
    Vector *deltaTuples = makeVector(VECTOR_NODE, T_Vector);

    // delta table name;
    appendStringInfo(createTableSQL, "create table %s ", deltaTableName);

    DataChunk *dcInfo = NULL;
    int insNum = 0;
    int delNum = 0;
    START_TIMER("Postgres - JOIN OPERATOR COPY DATA INTO DB");
    // init deltatuples;
    if (dcIns) {
        insNum = dcIns->numTuples;
        for (int row = 0; row < insNum; row++) {
            vecAppendNode(deltaTuples, (Node *) makeStringInfo());
        }
        dcInfo = dcIns;
    }
    if (dcDel) {
        delNum = dcDel->numTuples;
        for (int row = 0; row < delNum; row++) {
            vecAppendNode(deltaTuples, (Node *) makeStringInfo());
        }
        dcInfo = dcDel;
    }

    int col = 0;
    // for create table sql;
    appendStringInfoString(createTableSQL, "(");
    // attr;
    FOREACH(AttributeDef, ad, dcInfo->attrNames) {
        char *format = NULL;
        if (col > 0) {
            appendStringInfoString(createTableSQL, ",");
            // format = ",%s";
            format = "\t%s";
        } else {
            format = "%s";
        }
        //attr name;
        deltaAttrDefs = appendToTailOfList(deltaAttrDefs, (AttributeDef *) copyObject(ad));
        appendStringInfo(createTableSQL, "%s ", strdup(ad->attrName));

        int attrPos = INT_VALUE((Constant *) MAP_GET_STRING(dcInfo->attriToPos, ad->attrName));
        DataType dt = INT_VALUE((Constant *) MAP_GET_INT(dcInfo->posToDatatype, attrPos));

        switch(dt) {
            case DT_INT:
            {
                appendStringInfoString(createTableSQL, "int ");

                int *dcInsVals = NULL;
                int *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_IA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_IA(vec);
                }

                // for ins;
                for (int row = 0; row < insNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), format, gprom_itoa(dcInsVals[row]));
                }

                // for del;
                for (int row = 0; row < delNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row + insNum), format, gprom_itoa(dcDelVals[row]));
                }
            }
            break;
            case DT_BOOL:
            {
                appendStringInfoString(createTableSQL, "bool ");
                int *dcInsVals = NULL;
                int *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_IA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_IA(vec);
                }

                for (int row = 0; row < insNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), format, dcInsVals[row] == 1 ? strdup("true") : strdup("false"));
                }

                for (int row = 0; row < delNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, insNum + row), format, dcDelVals[row] == 1 ? strdup("true") : strdup("false"));
                }
            }
            break;
            case DT_LONG:
            {
                appendStringInfoString(createTableSQL, "bigint ");
                gprom_long_t *dcInsVals = NULL;
                gprom_long_t *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_LA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_LA(vec);
                }
                for (int row = 0; row < insNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), format, gprom_ltoa(dcInsVals[row]));
                }

                for (int row = 0; row < delNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, insNum + row), format, gprom_ltoa(dcDelVals[row]));
                }
            }
            break;
            case DT_FLOAT:
            {
                appendStringInfoString(createTableSQL, "float ");
                double *dcInsVals = NULL;
                double *dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = VEC_TO_FA(vec);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = VEC_TO_FA(vec);
                }
                for (int row = 0; row < insNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), format, gprom_ftoa(dcInsVals[row]));
                }

                for (int row = 0; row < delNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, insNum + row), format, gprom_ftoa(dcDelVals[row]));
                }
            }
            break;
            case DT_VARCHAR2:
            case DT_STRING:
            {
                appendStringInfoString(createTableSQL, "varchar ");
                char **dcInsVals = NULL;
                char **dcDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) getVecNode(dcIns->tuples, attrPos);
                    dcInsVals = (char **) VEC_TO_ARR(vec, char);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) getVecNode(dcDel->tuples, attrPos);
                    dcDelVals = (char **) VEC_TO_ARR(vec, char);
                }
                for (int row = 0; row < insNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), format, strdup(dcInsVals[row]));
                }

                for (int row = 0; row < delNum; row++) {
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, insNum + row), format, strdup(dcDelVals[row]));
                }
            }
            break;
        }
        col++;
    }

    // provenance sketch;
    if (dcInfo->isAPSChunk) {
        // col = 0;
        FOREACH_HASH_KEY(Constant, c, dcInfo->fragmentsInfo) {
            // if (col > 0) {
                appendStringInfoString(createTableSQL, ",");
            // }

            boolean isPSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(dcInfo->fragmentsIsInt, STRING_VALUE(c)));
            if (isPSInt) {
                deltaAttrDefs = appendToTailOfList(deltaAttrDefs, createAttributeDef(strdup(STRING_VALUE(c)), DT_INT));
                appendStringInfo(createTableSQL, "%s %s", strdup(STRING_VALUE(c)), strdup("int"));
                int *psInsVals = NULL;
                int *psDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, STRING_VALUE(c));
                    psInsVals = VEC_TO_IA(vec);
                }

                if (dcDel) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, STRING_VALUE(c));
                    psDelVals = VEC_TO_IA(vec);
                }

                for (int row = 0; row < insNum; row++) {
                    // appendStringInfo((StringInfo) getVecNode(deltaTuples, row), ",%s", gprom_itoa(psInsVals[row]));
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), "\t%s", gprom_itoa(psInsVals[row]));
                }
                for (int row = 0; row < delNum; row++) {
                    // appendStringInfo((StringInfo) getVecNode(deltaTuples, row + insNum), ",%s", gprom_itoa(psDelVals[row]));
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row + insNum), "\t%s", gprom_itoa(psDelVals[row]));
                }
            } else {
                deltaAttrDefs = appendToTailOfList(deltaAttrDefs, createAttributeDef(strdup(STRING_VALUE(c)), DT_STRING));
                appendStringInfo(createTableSQL, "%s %s", strdup(STRING_VALUE(c)), strdup("varchar"));
                BitSet **psInsVals = NULL;
                BitSet **psDelVals = NULL;
                if (dcIns) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, STRING_VALUE(c));
                    psInsVals = (BitSet **) VEC_TO_ARR(vec, BitSet);
                }
                if (dcDel) {
                    Vector *vec = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, STRING_VALUE(c));
                    psDelVals = (BitSet **) VEC_TO_ARR(vec, BitSet);
                }

                for (int row = 0; row < insNum; row++) {
                    // appendStringInfo((StringInfo) getVecNode(deltaTuples, row), ",%s", bitSetToString(psInsVals[row]));
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row), "\t%s", bitSetToString(psInsVals[row]));
                }
                for (int row = 0; row < delNum; row++) {
                    // appendStringInfo((StringInfo) getVecNode(deltaTuples, row + insNum), ",%s", bitSetToString(psDelVals[row]));
                    appendStringInfo((StringInfo) getVecNode(deltaTuples, row + insNum), "\t%s", bitSetToString(psDelVals[row]));
                }
            }
        }

    }

    // update iden;
    deltaAttrDefs = appendToTailOfList(deltaAttrDefs, createAttributeDef(strdup(updIdent), DT_INT));
    appendStringInfo(createTableSQL, ", %s %s", strdup(updIdent), "int");
    for (int row = 0; row < insNum; row++) {
        // appendStringInfoString((StringInfo) getVecNode(deltaTuples, row), ",1");
        appendStringInfoString((StringInfo) getVecNode(deltaTuples, row), "\t1");
    }
    for (int row = 0; row < delNum; row++) {
        // appendStringInfoString((StringInfo) getVecNode(deltaTuples, insNum + row), ",-1");
        appendStringInfoString((StringInfo) getVecNode(deltaTuples, insNum + row), "\t-1");
    }
    appendStringInfoString(createTableSQL, ");");

    // Connection, Copy;
    PGresult *res = NULL;
    PGconn *conn = plugin->conn;

    INFO_LOG("Create Delta SQL: %s", createTableSQL->data);
    res = PQexec(conn, createTableSQL->data);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        INFO_LOG("ERROR MSG: %s", PQerrorMessage(conn));
        // FATAL_LOG("ERROR");
        CLOSE_RES_CONN_AND_FATAL(res, "create table error: %s", PQerrorMessage(conn));
    }
    // PQclear(res);
    const char *errormsg = NULL;

    // copy data: create "COPY XXX FROM STDIN(DELIMITER\',\');"
    StringInfo copyStartSQL = makeStringInfo();
    // appendStringInfo(copyStartSQL, "COPY %s FROM STDIN(DELIMITER \',\');", strdup(deltaTableName));
    appendStringInfo(copyStartSQL, "COPY %s FROM STDIN(DELIMITER \'\t\');", strdup(deltaTableName));
    // INFO_LOG("COPY START SQL %s", copyStartSQL->data);
    // PQexec(conn, copyStartSQL->data);
    // // copy data: PQputCopyData()
    // for (int row = 0; row < insNum + delNum; row++) {
    //     StringInfo str = (StringInfo) getVecNode(deltaTuples, row);
    //     appendStringInfoString(str, "\r");
    //     INFO_LOG("what is copy data: %s", str->data);
    //     PQputCopyData(conn, str->data, str->len);
    // }

    // approach 2: one bulk
    StringInfo allDeltas = makeStringInfo();
    for (int row = 0; row < insNum + delNum; row++) {
        StringInfo str = (StringInfo) getVecNode(deltaTuples, row);
        appendStringInfo(allDeltas, "%s\r", str->data);
    }
    PQexec(conn, copyStartSQL->data);
    PQputCopyData(conn, allDeltas->data, allDeltas->len);


    // copy data: PQputCopyEnd()
    PQputCopyEnd(conn, errormsg);
    // PQexec(conn, "COMMIT;");
    execCommit();
    STOP_TIMER("Postgres - JOIN OPERATOR COPY DATA INTO DB");
    return deltaAttrDefs;
}

void postgresDropTemporalDeltaTable(char *tableName)
{
    // PGresult *res = NULL;
    // PGconn *conn = plugin->conn;
    // // postgresExecuteQueryIgnoreResult(CONCAT_STRINGS("DROP TABLE IF EXIST ", tableName, ";"));

    // // res = PQexec(conn, CONCAT_STRINGS("DROP TABLE IF EXIST ", tableName, ";"));
    // if (PQresultStatus(res) != PGRES_COMMAND_OK){
    //     CLOSE_RES_CONN_AND_FATAL(res, "DROP TEMPORAL DELTA TABLE FAIL: %s", PQerrorMessage(conn));
    // }
    // PQclear(res);
    execStmt(CONCAT_STRINGS("DROP TABLE IF EXISTS ", tableName, ";"));
    // execStmt(CONCAT_STRINGS("DROP TABLE IF EXISTS ", tableName));
    // executeQueryIgnoreResult(CONCAT_STRINGS("DROP TABLE IF EXISTS ", tableName));
}

void postgresPrepareUpdatePS(char *query, char *qName, int nParams)
{
    PGconn *conn = plugin->conn;
    PGresult * res;
    res = PQprepare(conn, qName, query, nParams, NULL);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        // STOP_TIMER(METADATA_LOOKUP_PREPARE_QUERY_TIME);
        CLOSE_RES_CONN_AND_FATAL(res, "prepare query %s failed: %s",
                qName, PQresultErrorMessage(res));
    }
    PQclear(res);
}

void postgresExecPrepareUpdatePS(char *query, char *qName, int nParams, char **params)
{
    PGconn *conn = plugin->conn;
    PGresult * res;
    res = PQexecPrepared(conn,
                qName,
                nParams,
                (const char *const *) params,
                NULL,
                NULL,
                0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        // STOP_TIMER(METADATA_LOOKUP_PREPARE_QUERY_TIME);
        CLOSE_RES_CONN_AND_FATAL(res, "prepare query %s failed: %s",
                qName, PQresultErrorMessage(res));
    }
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

char *
postgresBackendDatatypeToSQL (DataType dt)
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

Relation *
postgresExecuteQuery(char *query)
{
    return NULL;
}

void
postgresExecuteQueryIgnoreResult (char *query)
{

}

void
postgresExecuteStatement(char* sql)
{

}


#endif
