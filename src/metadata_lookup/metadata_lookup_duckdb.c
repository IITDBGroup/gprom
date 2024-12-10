#include "common.h"
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
#include "model/set/vector.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "utility/string_utils.h"
#include <stdlib.h>

// Mem context
#define CONTEXT_NAME "DuckDBMemContext"

#define QUERY_TABLE_COL_COUNT "SELECT column_name, data_type FROM information_schema.columns WHERE table_name = '%s';"
#define QUERY_TABLE_ATTR_MIN_MAX "SELECT %s FROM %s"

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

// functions
// static duckdb_result runQuery (char *q);
static DataType stringToDT (char *dataType);
static char *duckdbGetConnectionDescription (void);
static void initCache(CatalogCache *c);

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
    memContext = getCurMemContext();

    // create cache
    plugin->plugin.cache = createCache();
    initCache(plugin->plugin.cache);

    plugin->initialized = TRUE;

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

    rc = duckdb_open(dbfile, &(plugin->db));
    if(rc != DuckDBSuccess)
    {
          fprintf(stderr, "Can not open database <%s>", dbfile);
          duckdb_close(&(plugin->db)); 
          return EXIT_FAILURE;
    }

    rc = duckdb_connect(plugin->db, &(plugin->conn));
    if(rc != DuckDBSuccess)
    {
        fprintf(stderr, "Can not connect to database <%s>", dbfile);
        duckdb_disconnect(&(plugin->conn)); 
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
duckdbDatabaseConnectionClose()
{

    duckdb_disconnect(&(plugin->conn)); 
    duckdb_close(&(plugin->db)); 

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
    duckdb_connection c = plugin->conn;

    char query[256];
    snprintf(query, sizeof(query),
                "SELECT COUNT(*) FROM information_schema.tables WHERE table_name='%s';", tableName);

    int rc;
    rc = duckdb_query(c, query, &result);

    if (rc != DuckDBSuccess) {
        fprintf(stderr, "Failed to execute query to check duckdbCatalogTableExists.");
        return EXIT_FAILURE;
    }

    int table_count = duckdb_value_int32(&result, 0, 0);

    duckdb_destroy_result(&result);

    return table_count > 0;
}

boolean
duckdbCatalogViewExists (char * viewName)
{
    return FALSE;//TODO
}

List * 
duckdbGetAttributes(char *tableName) {
    duckdb_result result;
    StringInfo q;
    List *resultList = NIL;

    q = makeStringInfo();
    appendStringInfo(q, QUERY_TABLE_COL_COUNT, tableName);

    int rc = duckdb_query(plugin->conn, q->data, &result);

    // Execute the query
    if (rc != DuckDBSuccess) {
        fprintf(stderr, "error getting attributes of table <%s>", tableName);
    }

    // Iterate over the result set
    for (idx_t row = 0; row < duckdb_row_count(&result); row++) {
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

    return resultList;
}

List *
duckdbGetAttributeNames (char *tableName)
{
    return getAttrDefNames(duckdbGetAttributes(tableName));
}

boolean
duckdbIsAgg(char *functionName)
{
    char *f = strToLower(functionName);

    if (hasSetElem(plugin->plugin.cache->aggFuncNames, f))
        return TRUE;

    return FALSE;
}

boolean
duckdbIsWindowFunction(char *functionName)
{
    char *f = strToLower(functionName);

    if (hasSetElem(plugin->plugin.cache->winFuncNames, f))
        return TRUE;

    return FALSE;
}

DataType
duckdbGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
    *funcExists = TRUE;
    return DT_STRING; //TODO
}

DataType
duckdbGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
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
duckdbGetTableDefinition(char *tableName)
{
    return NULL;//TODO
}

char *
duckdbGetViewDefinition(char *viewName)
{
    return NULL;//TODO
}

int
duckdbGetCostEstimation(char *query)
{
    THROW(SEVERITY_RECOVERABLE,"%s","not supported yet");
    return -1;
}

List* 
duckdbGetKeyInformation(char *tableName) {
    duckdb_result result;
    StringInfo q;
    Set *key = STRSET();
    List *keys = NIL;
    int rc;

    q = makeStringInfo();
    appendStringInfo(q, QUERY_TABLE_COL_COUNT, tableName);
    rc = duckdb_query(plugin->conn, q->data, &result);

    if (rc != DuckDBSuccess) {
        fprintf(stderr, "Error getting primary key information for table <%s>", tableName);
        return NULL; 
    }

    for (idx_t row = 0; row < duckdb_row_count(&result); row++) {
        const char *colname = duckdb_value_varchar(&result, row, 0);
        
        addToSet(key, strToUpper(strdup((char *) colname)));

        duckdb_free((void *)colname);
    }

    if (duckdb_row_count(&result) == 0) {
        fprintf(stderr, "No primary key information found for table <%s>", tableName);
    } else {
        DEBUG_LOG("Key for %s are: %s", tableName, beatify(nodeToString(key)));
    }

    if (!EMPTY_SET(key)) {
        keys = singleton(key);
    }

    duckdb_destroy_result(&result);

    return keys;
}

DataType
duckdbBackendSQLTypeToDT (char *sqlType)
{
    if (regExMatch("INT", sqlType))
    {
        return DT_INT;
    }
    if (regExMatch("NUMERIC", sqlType)
            || regExMatch("REAL", sqlType)
            || regExMatch("FLOA", sqlType)
            || regExMatch("DOUB", sqlType))
    {
        return DT_FLOAT;
    }
    if (regExMatch("CHAR", sqlType)
            || regExMatch("CLOB", sqlType)
            || regExMatch("TEXT", sqlType))
    {
        return DT_STRING;
    }

    return DT_FLOAT;
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

HashMap 
*duckdbGetMinAndMax(char* tableName, char* colName) {
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

    if (rc != DuckDBSuccess) {
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

Relation *duckdbExecuteQuery(char *query) {
    Relation *r = makeNode(Relation);
    duckdb_result rs;
    int rc;

    rc = duckdb_query(plugin->conn, query, &rs);
    
    if (rc != DuckDBSuccess) {
        fprintf(stderr, "Failed to execute query <%s>", query);
        return NULL;
    }

    int numFields = duckdb_column_count(&rs);

    r->schema = NIL;
    for (int i = 0; i < numFields; i++) {
        const char *name = duckdb_column_name(&rs, i);
        r->schema = appendToTailOfList(r->schema, strdup((char *) name));
    }

    // Read rows
    r->tuples = makeVector(VECTOR_NODE, T_Vector);
    for (idx_t row = 0; row < duckdb_row_count(&rs); row++) {
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
    duckdb_result rs;
    int rc;

    rc = duckdb_query(plugin->conn, query, &rs);

    if (rc != DuckDBSuccess) {
        fprintf(stderr, "Failed to execute query <%s>", query);
        return;
    }

    duckdb_destroy_result(&rs);
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
stringToDT (char *dataType)
{
   DEBUG_LOG("data type %s", dataType);
   char *lowerDT = strToLower(dataType);

   if (isSubstr(lowerDT, "int"))
       return DT_INT;
   if (isSubstr(lowerDT, "char") || isSubstr(lowerDT, "clob") || isSubstr(lowerDT, "text"))
       return DT_STRING;
   if (isSubstr(lowerDT, "real") || isSubstr(lowerDT, "floa") || isSubstr(lowerDT, "doub"))
       return DT_FLOAT;

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
    ADD_BOTH_FUNC("avg");
    ADD_BOTH_FUNC("count");
    ADD_BOTH_FUNC("group_concat");
    ADD_BOTH_FUNC("max");
    ADD_BOTH_FUNC("min");
    ADD_BOTH_FUNC("sum");
    ADD_BOTH_FUNC("total");

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