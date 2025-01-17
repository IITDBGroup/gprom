#include "common.h"
#include "duckdb.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"

#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_duckdb.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/set/vector.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "utility/string_utils.h"
#include <stdint.h>
#include <stdlib.h>

// Mem context
#define CONTEXT_NAME "DuckDBMemContext"

// DuckDB ignores case for identifier, but when creating tables quoted case taken literally and stored like this in the stystem catalog
#define QUERY_TABLE_EXISTS "SELECT COUNT(*) FROM information_schema.tables WHERE upper(table_name)='%s' AND table_type = 'BASE TABLE';"
#define QUERY_VIEW_EXISTS "SELECT COUNT(*) FROM information_schema.tables WHERE upper(table_name)='%s' AND table_type = 'VIEW';"
#define QUERY_TABLE_COL_COUNT "SELECT upper(column_name), data_type FROM information_schema.columns WHERE upper(table_name) = '%s';"
#define QUERY_TABLE_PKS "SELECT upper(column_name) FROM information_schema.key_column_usage WHERE upper(table_name) = '%s';"
#define QUERY_TABLE_ATTR_MIN_MAX "SELECT %s FROM %s"
/* #define QUERY_FUNC_IS_AGG "SELECT EXISTS (SELECT * FROM duckdb_functions() WHERE function_type = 'aggregate' AND upper(function_name) = '%s') AS found" */
#define QUERY_FUNC_GET_AGG_FUNCS "SELECT DISTINCT upper(function_name) FROM duckdb_functions() WHERE function_type = 'aggregate';"
#define QUERY_FUNCS "SELECT parameter_types::varchar, return_type FROM duckdb_functions() WHERE lower(function_name) = '%s';"

// Only define real plugin structure and methods if duckdb is present
#ifdef HAVE_DUCKDB_BACKEND

// extends MetadataLookupPlugin with duckdb specific information
typedef struct DuckDBPlugin
{
    MetadataLookupPlugin plugin;
    boolean initialized;
    duckdb_connection conn;
    duckdb_database db;
} DuckDBPlugin;

// global vars
static DuckDBPlugin *plugin = NULL;
static MemContext *memContext = NULL;
static char* boolops[] = { "<", "<=", ">", ">=" "!=", "=", "==", "<>", NULL };

// functions
// static duckdb_result runQuery (char *q);
static DataType stringToDT (char *dataType);
static char *duckdbGetConnectionDescription (void);
static void initCache(CatalogCache *c);
static duckdb_result duckdbQueryInternal(char *sql);
static DataType inferAnyReturnType(List *types, List *argTypes, char *retType);
static boolean isAnyTypeCompatible(List *oids, List *argTypes);
static boolean hasAnyType(List *types);
static List *typeListToDTs(List *strs);
static boolean isBoolOp(char *opname);

typedef struct DuckDBMetaCache
{
    Set *boolOps;
} DuckDBMetaCache;

MetadataLookupPlugin *
assembleDuckDBMetadataLookupPlugin (void)
{
    plugin = NEW(DuckDBPlugin);
    MetadataLookupPlugin *p = (MetadataLookupPlugin *) plugin;

    p->type = METADATA_LOOKUP_PLUGIN_DUCKDB;

    p->initMetadataLookupPlugin = duckdbInitMetadataLookupPlugin;
    p->databaseConnectionOpen = duckdbDatabaseConnectionOpen;
    p->databaseConnectionClose = duckdbDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = duckdbShutdownMetadataLookupPlugin;
    p->isInitialized = duckdbIsInitialized;
    p->catalogTableExists = duckdbCatalogTableExists;
    p->catalogViewExists = duckdbCatalogViewExists;
    p->getAttributes = duckdbGetAttributes;
    p->getAttributeNames = duckdbGetAttributeNames;
    p->isAgg = duckdbIsAgg;
    p->isWindowFunction = duckdbIsWindowFunction;
    p->getFuncReturnType = duckdbGetFuncReturnType;
    p->getOpReturnType = duckdbGetOpReturnType;
    p->getTableDefinition = duckdbGetTableDefinition;
    p->getViewDefinition = duckdbGetViewDefinition;
    p->getTransactionSQLAndSCNs = duckdbGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = duckdbExecuteAsTransactionAndGetXID;
    p->getCostEstimation = duckdbGetCostEstimation;
    p->getKeyInformation = duckdbGetKeyInformation;
    p->executeQuery = duckdbExecuteQuery;
    p->executeQueryIgnoreResult = duckdbExecuteQueryIgnoreResults;
    p->connectionDescription = duckdbGetConnectionDescription;
    p->sqlTypeToDT = duckdbBackendSQLTypeToDT;
    p->dataTypeToSQL = duckdbBackendDatatypeToSQL;
    p->getMinAndMax = duckdbGetMinAndMax;
    return p;
}

/* plugin methods */
int
duckdbInitMetadataLookupPlugin (void)
{
    if (plugin && plugin->initialized)
    {
        INFO_LOG("tried to initialize metadata lookup plugin more than once");
        return EXIT_SUCCESS;
    }

    NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);
    memContext = getCurMemContext();

    // create cache
    plugin->plugin.cache = createCache();

    plugin->initialized = TRUE;
    RELEASE_MEM_CONTEXT();

    return EXIT_SUCCESS;
}

int
duckdbShutdownMetadataLookupPlugin (void)
{

    // clear cache
    //TODO

    return EXIT_SUCCESS;
}

int
duckdbDatabaseConnectionOpen (void)
{
    char *dbfile = getStringOption(OPTION_CONN_DB);
    int rc;
    if (dbfile == NULL)
        FATAL_LOG("no database file given (<connection.db> parameter)");

    ACQUIRE_MEM_CONTEXT(memContext);
    rc = duckdb_open(dbfile, &(plugin->db));
    if(rc != DuckDBSuccess)
    {
        RELEASE_MEM_CONTEXT();
        ERROR_LOG("Can not open database <%s>", dbfile);
        duckdb_close(&(plugin->db));
        return EXIT_FAILURE;
    }

    rc = duckdb_connect(plugin->db, &(plugin->conn));
    if(rc != DuckDBSuccess)
    {
        RELEASE_MEM_CONTEXT();
        ERROR_LOG("Can not connect to database <%s>", dbfile);
        duckdb_disconnect(&(plugin->conn));
        return EXIT_FAILURE;
    }

    initCache(plugin->plugin.cache);

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

int
duckdbDatabaseConnectionClose()
{
    ACQUIRE_MEM_CONTEXT(memContext);
    ASSERT(plugin && plugin->initialized);

    duckdb_disconnect(&(plugin->conn));
    duckdb_close(&(plugin->db));

    RELEASE_MEM_CONTEXT();
    return EXIT_SUCCESS;
}

boolean
duckdbIsInitialized (void)
{
    if (plugin && plugin->initialized)
    {
        if (plugin->conn == NULL)
        {
            if (duckdbDatabaseConnectionOpen() != EXIT_SUCCESS)
                return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

boolean
duckdbCatalogTableExists (char * tableName)
{
    duckdb_result result;
    StringInfo str;
    int table_count = -1;

    ACQUIRE_MEM_CONTEXT(memContext);

    str = makeStringInfo();
    appendStringInfo(str, QUERY_TABLE_EXISTS, tableName);

    result = duckdbQueryInternal(str->data);
    table_count = duckdb_value_int32(&result, 0, 0);

    duckdb_destroy_result(&result);

    RELEASE_MEM_CONTEXT();
    return table_count > 0;
}

boolean
duckdbCatalogViewExists (char * viewName)
{
    duckdb_result result;
    StringInfo str;
    int table_count = -1;
    ACQUIRE_MEM_CONTEXT(memContext);

    str = makeStringInfo();
    appendStringInfo(str, QUERY_VIEW_EXISTS, viewName);
    result = duckdbQueryInternal(str->data);

    table_count = duckdb_value_int32(&result, 0, 0);
    duckdb_destroy_result(&result);

    RELEASE_MEM_CONTEXT();
    return table_count > 0;
}

List *
duckdbGetAttributes(char *tableName)
{
    duckdb_result result;
    StringInfo q;
    List *resultList = NIL;

    ACQUIRE_MEM_CONTEXT(memContext);

    q = makeStringInfo();
    appendStringInfo(q, QUERY_TABLE_COL_COUNT, tableName);

    result = duckdbQueryInternal(q->data);

    // Iterate over the result set
    for(idx_t row = 0; row < duckdb_row_count(&result); row++)
    {
        const char *colName = duckdb_value_varchar(&result, 0, row);
        const char *dt = duckdb_value_varchar(&result, 1, row);

        DataType ourDT = stringToDT((char *)dt);

        AttributeDef *a = createAttributeDef(
            strToUpper(strdup((char *) colName)),
            ourDT);
        resultList = appendToTailOfList(resultList, a);

        duckdb_free((void *)colName);
        duckdb_free((void *)dt);
    }

    duckdb_destroy_result(&result);
    DEBUG_NODE_LOG("columns are: ", resultList);

    RELEASE_MEM_CONTEXT();
    return resultList;
}

List *
duckdbGetAttributeNames(char *tableName)
{
    return getAttrDefNames(duckdbGetAttributes(tableName));
}

boolean
duckdbIsAgg(char *functionName)
{
    char *f = strToUpper(functionName);

    if (hasSetElem(plugin->plugin.cache->aggFuncNames, f))
    {
        return TRUE;
    }

    return FALSE;
}

boolean
duckdbIsWindowFunction(char *functionName)
{
    char *f = strToLower(functionName);

    if (hasSetElem(plugin->plugin.cache->winFuncNames, f))
    {
        return TRUE;
    }

    return FALSE;
}

DataType
duckdbGetFuncReturnType(char *fName, List *argTypes, boolean *funcExists)
{
    duckdb_result result;
    DataType resType = DT_STRING;
    *funcExists = FALSE;
    StringInfo str;
    int numrows;
    fName = strToLower(fName);

    // handle non function expressions that are treated as functions by GProM
    if (strcaseeq(fName, GREATEST_FUNC_NAME)
        || strcaseeq(fName, LEAST_FUNC_NAME)
        || strcaseeq(fName, COALESCE_FUNC_NAME)
        || strcaseeq(fName, LEAD_FUNC_NAME)
        || strcaseeq(fName, LAG_FUNC_NAME)
        || strcaseeq(fName, FIRST_VALUE_FUNC_NAME)
        || strcaseeq(fName, LAST_VALUE_FUNC_NAME))
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

    str = makeStringInfo();
    appendStringInfo(str, QUERY_FUNCS, fName);
    result = duckdbQueryInternal(str->data);
    numrows = duckdb_row_count(&result);

    for(int i = 0; i < numrows; i++)
    {
        char *parameters = duckdb_value_varchar(&result, 0, i);
        char *retType = duckdb_value_varchar(&result, 1, i);
        List *typesStrs = NIL;
        List *types = NIL;

        if(!streq(parameters,"[]"))
        {
            parameters = strRemPostfix(strRemPrefix(parameters, 1), 1);
            typesStrs = splitString(parameters, ",");
            types = typeListToDTs(typesStrs);
        }

        if(equal(argTypes, types))
        {
            DEBUG_LOG("return type %s for %s(%s)",
                      DataTypeToString(resType),
                      fName,
                      nodeToString(argTypes));
            resType = stringToDT(retType);
            *funcExists = TRUE;
        }

        // does function take anytype as an input then determine return type
        if (hasAnyType(typesStrs))
        {
            if(isAnyTypeCompatible(typesStrs, argTypes))
            {
                resType = inferAnyReturnType(typesStrs,
                                             argTypes,
                                             retType);
                *funcExists = TRUE;
            }
        }
    }

    RELEASE_MEM_CONTEXT();

    return resType;
}

#define IS_ANY_TYPE(s) streq(s,"ANY")

static boolean
hasAnyType(List *types)
{
    FOREACH(char,o, types)
    {
        if(IS_ANY_TYPE(o))
            return TRUE;
    }
    return FALSE;
}

static boolean
isAnyTypeCompatible(List *types, List *argTypes)
{
    int i = 0;

    FOREACH(char,s,types)
    {
        DataType dt = getNthOfListInt(argTypes, i);

        if(!IS_ANY_TYPE(s)
           && stringToDT(s) != dt)
        {
            return FALSE;
        }

        i++;
    }

    return TRUE;
}

static DataType
inferAnyReturnType(List *types, List *argTypes, char *retType)
{
    int i = 0;

    if (!IS_ANY_TYPE(retType))
        return stringToDT(retType);

    FOREACH(char,typ,types)
    {
        if(IS_ANY_TYPE(typ))
        {
            return getNthOfListInt(argTypes, i);
        }
        i++;
    }

    // keep compiler quiet
    return DT_STRING;
}


static List *
typeListToDTs(List *strs)
{
    List *result;

    FOREACH(char,s,strs)
    {
        result = appendToTailOfListInt(result, stringToDT(s));
    }

    return result;
}

DataType
duckdbGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{
    *opExists = TRUE;

    if(streq(oName, "+") || streq(oName, "*")  || streq(oName, "-") || streq(oName, "/"))
    {
        if (LIST_LENGTH(argTypes) == 2)
        {
            DataType lType = getNthOfListInt(argTypes, 0);
            DataType rType = getNthOfListInt(argTypes, 1);

            if (lType == rType)
            {
                if (lType == DT_INT || lType == DT_FLOAT)
                {
                    return lType;
                }
            }
            else
            {
                DataType lca;
                lca = lcaType(lType, rType);
                if(lca == DT_INT || lca == DT_FLOAT)
                {
                    return lca;
                }
            }
        }
    }

    // known boolean operators
    if(isBoolOp(oName))
    {
        if (LIST_LENGTH(argTypes) == 2)
        {
            DataType lType = getNthOfListInt(argTypes, 0);
            DataType rType = getNthOfListInt(argTypes, 1);

            if (lType == rType
                || (isNumericType(lType) && isNumericType(rType)))
            {
                return DT_BOOL;
            }
        }
    }

    if(streq(oName, OPNAME_LIKE) || streq(oName, "~~"))
    {
        DataType lType = getNthOfListInt(argTypes, 0);
        DataType rType = getNthOfListInt(argTypes, 1);

        if(lType == DT_STRING && rType == DT_STRING)
        {
            return DT_BOOL;
        }
    }

    if(streq(oName, OPNAME_STRING_CONCAT))
    {
        return DT_STRING;
    }
    //TODO more operators
    *opExists = FALSE;

    return DT_STRING;
}

static boolean
isBoolOp(char *opname)
{
    Set *c = ((DuckDBMetaCache *) plugin->plugin.cache->cacheHook)->boolOps;

    return hasSetElem(c, opname);
}

char *
duckdbGetTableDefinition(char *tableName)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return NULL;//TODO
}

char *
duckdbGetViewDefinition(char *viewName)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return NULL;//TODO
}

int
duckdbGetCostEstimation(char *query)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return -1;
}

List*
duckdbGetKeyInformation(char *tableName)
{
    duckdb_result result;
    StringInfo q;
    Set *key;
    List *keys = NIL;
    int numrows;

    ACQUIRE_MEM_CONTEXT(memContext);

    key =  STRSET();

    q = makeStringInfo();
    appendStringInfo(q, QUERY_TABLE_PKS, tableName);
    result = duckdbQueryInternal(q->data);
    numrows = duckdb_row_count(&result);

    for(idx_t row = 0; row < numrows; row++)
    {
        const char *colname = duckdb_value_varchar(&result, 0, row);

        addToSet(key, strToUpper(strdup((char *) colname)));

        duckdb_free((void *)colname);
    }

    if(numrows == 0)
    {
        DEBUG_LOG("No primary key information found for table <%s>", tableName);
    }
    else
    {
        DEBUG_LOG("Key for %s are: %s", tableName, beatify(nodeToString(key)));
        keys = singleton(key);
    }

    duckdb_destroy_result(&result);

    RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List,keys);
}

DataType
duckdbBackendSQLTypeToDT (char *sqlType)
{
    return stringToDT(sqlType);
}

char *
duckdbBackendDatatypeToSQL (DataType dt)
{
    switch(dt)
    {
        case DT_INT:
        case DT_LONG:
            return "INT";
            break;
        case DT_FLOAT:
            return "DOUBLE";
            break;
        case DT_STRING:
        case DT_VARCHAR2:
            return "TEXT";
            break;
        case DT_BOOL:
            return "BOOLEAN";
            break;
    }

    // keep compiler quiet
    return "TEXT";
}

HashMap *
duckdbGetMinAndMax(char* tableName, char* colName)
{
    HashMap *result_map = NEW_MAP(Constant, HashMap);
    duckdb_result rs;
    StringInfo q;
    StringInfo colMinMax;
    List *attr = duckdbGetAttributes(tableName);
    List *aNames = getAttrDefNames(attr);
    List *aDTs = getAttrDataTypes(attr);
    int rc;

    q = makeStringInfo();
    colMinMax = makeStringInfo();

    // Construct the min and max query for each attribute
    FOREACH(char, a, aNames) {
        appendStringInfo(colMinMax, "min(%s) AS min_%s, max(%s) AS max_%s", a, a, a, a);
        appendStringInfo(colMinMax, "%s", FOREACH_HAS_MORE(a) ? ", " : "");
    }

    appendStringInfo(q, QUERY_TABLE_ATTR_MIN_MAX, colMinMax->data, tableName);

    rc = duckdb_query(plugin->conn, q->data, &rs);

    if (rc != DuckDBSuccess)
    {
        fprintf(stderr, "Error executing query to get min and max values for table <%s>", tableName);
        return NULL;
    }

    // Iterate over the result set
    for (idx_t pos = 0, attr_idx = 0; attr_idx < getListLength(aNames); ++attr_idx) {
        char *aname = getNthOfListP(aNames, attr_idx);
        DataType dt = (DataType) getNthOfListInt(aDTs, attr_idx);
        HashMap *minmax = NEW_MAP(Constant, Constant);
        const char *minVal = duckdb_value_varchar(&rs, 0, pos++);
        const char *maxVal = duckdb_value_varchar(&rs, 0, pos++);
        Constant *min, *max;

        switch(dt) {
            case DT_INT:
                min = createConstInt(atoi((char *) minVal));
                max = createConstInt(atoi((char *) maxVal));
                break;
            case DT_LONG:
                min = createConstLong(atol((char *) minVal));
                max = createConstLong(atol((char *) maxVal));
                break;
            case DT_FLOAT:
                min = createConstFloat(atof((char *) minVal));
                max = createConstFloat(atof((char *) maxVal));
                break;
            case DT_STRING:
                min = createConstString(strdup((char *) minVal));
                max = createConstString(strdup((char *) maxVal));
                break;
            default:
                THROW(SEVERITY_RECOVERABLE, "Received unknown data type from DuckDB: %s", DataTypeToString(dt));
                duckdb_destroy_result(&rs);
                return NULL;
        }

        MAP_ADD_STRING_KEY(minmax, MIN_KEY, min);
        MAP_ADD_STRING_KEY(minmax, MAX_KEY, max);
        MAP_ADD_STRING_KEY(result_map, aname, minmax);

        duckdb_free((void *)minVal);
        duckdb_free((void *)maxVal);
    }

    duckdb_destroy_result(&rs);

    DEBUG_NODE_BEATIFY_LOG("min maxes", MAP_GET_STRING(result_map, colName));

    return (HashMap *) MAP_GET_STRING(result_map, colName);
}

void
duckdbGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
}

Node *
duckdbExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return NULL;
}

Relation *
duckdbExecuteQuery(char *query)
{
    Relation *r = makeNode(Relation);
    duckdb_result rs;
    int rc;

    rc = duckdb_query(plugin->conn, query, &rs);

    if (rc != DuckDBSuccess)
    {
        FATAL_LOG("Failed to run query [%i]\n%s\n\n%s",
                  rc,
                  duckdb_result_error(&rs),
                  query);
        return NULL;
    }

    int numFields = duckdb_column_count(&rs);
    int numrows = duckdb_row_count(&rs);

    r->schema = NIL;
    for (int i = 0; i < numFields; i++) {
        const char *name = duckdb_column_name(&rs, i);
        r->schema = appendToTailOfList(r->schema, strdup((char *) name));
    }

    // Read rows
    r->tuples = makeVector(VECTOR_NODE, T_Vector);
    for (idx_t row = 0; row < numrows; row++) {
        Vector *tuple = makeVector(VECTOR_STRING, -1);
        for (int j = 0; j < numFields; j++) {
            if (duckdb_value_is_null(&rs, j, row)) {
                vecAppendString(tuple, strdup("NULL"));
            } else {
                const char *val = duckdb_value_varchar(&rs, j, row);
                vecAppendString(tuple, strdup((char *) val));
                duckdb_free((void *)val);
            }
        }
        VEC_ADD_NODE(r->tuples, tuple);
        DEBUG_NODE_LOG("read tuple <%s>", tuple);
    }

    duckdb_destroy_result(&rs);

    return r;
}

void
duckdbExecuteQueryIgnoreResults(char *query) {
    duckdb_result result;
    int rc;

    rc = duckdb_query(plugin->conn, query, &result);

    if (rc != DuckDBSuccess) {
        FATAL_LOG("Failed to execute query. Error:\n%s\n\n%s",
                  duckdb_result_error(&result),
                  query);
        return;
    }

    duckdb_destroy_result(&result);
}

// static duckdb_result
// runQuery(char *q) {
//     duckdb_result result;
//     duckdb_state rc;

//     DEBUG_LOG("run query:\n<%s>", q);

//     rc = duckdb_query(plugin->conn, q, &result);

//     if (rc != DuckDBSuccess) {
//         StringInfo _newmes = makeStringInfo();
//         appendStringInfo(_newmes, "failed to prepare query <%s>", q);
//         StringInfo _errMes = makeStringInfo();
//         appendStringInfo(_errMes, duckdb_result_error(&result)); // DuckDB provides error message
//         FATAL_LOG("error (%s)\n%u\n\n%s", _errMes->data, rc, _newmes->data);

//         duckdb_destroy_result(&result);
//     }

//     return result;
// }

static DataType
stringToDT(char *dataType)
{
    DEBUG_LOG("data type %s", dataType);
    char *lowerDT = strToLower(dataType);

    if(streq(lowerDT, "interval"))
    {
        return DT_STRING;
    }
    if(isSubstr(lowerDT, "int"))
    {
        return DT_INT;
    }
    if(isSubstr(lowerDT, "float") || isSubstr(lowerDT, "decimal") || isSubstr(lowerDT, "double"))
    {
       return DT_FLOAT;
    }
    if(isSubstr(lowerDT, "char") || isSubstr(lowerDT, "clob") || isSubstr(lowerDT, "text"))
    {
        return DT_STRING;
    }

    return DT_STRING;
}

static char *
duckdbGetConnectionDescription (void)
{
    return CONCAT_STRINGS("DuckDB:", getStringOption("connection.db"));
}

#define ADD_AGGR_FUNC(name) addToSet(plugin->plugin.cache->aggFuncNames, strdup(name))
#define ADD_WIN_FUNC(name) addToSet(plugin->plugin.cache->winFuncNames, strdup(name))
#define ADD_BOTH_FUNC(name) \
    do { \
        addToSet(plugin->plugin.cache->aggFuncNames, strdup(name)); \
        addToSet(plugin->plugin.cache->winFuncNames, strdup(name)); \
    } while (0)

static void
initCache(CatalogCache *c)
{
    duckdb_result result;
    int numRows;
    DuckDBMetaCache *duckCache;

    NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(CONTEXT_NAME);

    // aggregation function names from duckdb
    result = duckdbQueryInternal(QUERY_FUNC_GET_AGG_FUNCS);
    numRows = duckdb_row_count(&result);

    for(int i = 0; i < numRows; i++)
    {
        char *fname = duckdb_value_varchar(&result, 0, i);
        ADD_BOTH_FUNC(fname);

        duckdb_free(fname);
    }

    duckdb_destroy_result(&result);

    /* ADD_BOTH_FUNC("avg"); */
    /* ADD_BOTH_FUNC("count"); */
    /* ADD_BOTH_FUNC("group_concat"); */
    /* ADD_BOTH_FUNC("max"); */
    /* ADD_BOTH_FUNC("min"); */
    /* ADD_BOTH_FUNC("sum"); */
    /* ADD_BOTH_FUNC("total"); */

    ADD_WIN_FUNC("row_number");
    ADD_WIN_FUNC("rank");
    ADD_WIN_FUNC("dense_rank");
    ADD_WIN_FUNC("percent_rank");
    ADD_WIN_FUNC("cum_dist");
    ADD_WIN_FUNC("ntile");
    ADD_WIN_FUNC("lag");
    ADD_WIN_FUNC("lead");
    ADD_WIN_FUNC("first_value");
    ADD_WIN_FUNC("last_value");
    ADD_WIN_FUNC("nth_value");

    duckCache = NEW(DuckDBMetaCache);
    duckCache->boolOps = STRSET();

    for(int i = 0; boolops[i] != NULL; i++)
    {
        addToSet(duckCache->boolOps, strdup(boolops[i]));
    }

    plugin->plugin.cache->cacheHook = (void*) duckCache;

    RELEASE_MEM_CONTEXT();
}

static duckdb_result
duckdbQueryInternal(char *sql)
{
    duckdb_result result;
    duckdb_connection c = plugin->conn;
    int rc;

    rc = duckdb_query(c, sql, &result);

    if (rc != DuckDBSuccess)
    {
        RELEASE_MEM_CONTEXT();
        FATAL_LOG("Failed to run query [%i]\n%s\n\n%s",
                  rc,
                  duckdb_result_error(&result),
                  sql);
    }

    return result;
}

#else


MetadataLookupPlugin *
assembleDuckDBMetadataLookupPlugin (void)
{
    return NULL;
}

int
duckdbInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
duckdbShutdownMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

int
duckdbDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}

int
duckdbDatabaseConnectionClose()
{
    return EXIT_SUCCESS;
}

boolean
duckdbIsInitialized (void)
{
    return FALSE;
}

boolean
duckdbCatalogTableExists (char * tableName)
{
    return FALSE;
}

boolean
duckdbCatalogViewExists (char * viewName)
{
    return FALSE;
}

List *
duckdbGetAttributes (char *tableName)
{
    return NIL;
}

List *
duckdbGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
duckdbIsAgg(char *functionName)
{
    return FALSE;
}

boolean
duckdbIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
duckdbGetTableDefinition(char *tableName)
{
    return NULL;
}

char *
duckdbGetViewDefinition(char *viewName)
{
    return NULL;
}

DataType
duckdbBackendSQLTypeToDT (char *sqlType)
{
    return DT_INT;
}

char *
duckdbBackendDatatypeToSQL (DataType dt)
{
    return NULL;
}

HashMap *
duckdbGetMinAndMax(char* tableName, char* colName)
{
    return NULL;
}

void
duckdbGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
}

Node *
duckdbExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
duckdbGetCostEstimation(char *query)
{
    return 0;
}

List *
duckdbGetKeyInformation(char *tableName)
{
    return NULL;
}

Relation *
duckdbExecuteQuery(char *query)
{
    return NULL;
}

#endif
