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
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "utility/string_utils.h"
#include <stdlib.h>

// Mem context
#define CONTEXT_NAME "SQLiteMemContext"

// query templates
#define QUERY_TABLE_COL_COUNT "PRAGMA table_info(%s)"
#define QUERY_TABLE_ATTR_MIN_MAX "SELECT %s FROM %s"

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
            StringInfo _errMes = makeStringInfo(); \
            appendStringInfo(_errMes, strdup((char *) sqlite3_errmsg(plugin->conn))); \
            FATAL_LOG("error (%s)\n%u\n\n%s", _errMes, _rc, _newmes->data); \
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
    p->executeQueryIgnoreResult = sqliteExecuteQueryIgnoreResults;
    p->connectionDescription = sqliteGetConnectionDescription;
    p->sqlTypeToDT = sqliteBackendSQLTypeToDT;
    p->dataTypeToSQL = sqliteBackendDatatypeToSQL;
	p->getMinAndMax = sqliteGetMinAndMax;
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
    char *dbfile = getStringOption(OPTION_CONN_DB);
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
    boolean res = (sqlite3_table_column_metadata(c,NULL,tableName,strdup("rowid"),NULL,NULL,NULL,NULL, NULL) == SQLITE_OK);

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

        AttributeDef *a = createAttributeDef(
			strToUpper(strdup((char *) colName)),
			ourDT);
        result = appendToTailOfList(result, a);
    }

    HANDLE_ERROR_MSG(rc, SQLITE_DONE, "error getting attributes of table <%s>", tableName);

    DEBUG_NODE_LOG("columns are: ", result);

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
    char *f = strToLower(functionName);

    if (hasSetElem(plugin->plugin.cache->aggFuncNames, f))
        return TRUE;

    return FALSE;
}

boolean
sqliteIsWindowFunction(char *functionName)
{
    char *f = strToLower(functionName);

    if (hasSetElem(plugin->plugin.cache->winFuncNames, f))
        return TRUE;

    return FALSE;
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
    sqlite3_stmt *rs;
    StringInfo q;
    Set *key = STRSET();
	List *keys = NIL;
    int rc;

    q = makeStringInfo();
    appendStringInfo(q, QUERY_TABLE_COL_COUNT, tableName);
    rs = runQuery(q->data);

    while((rc = sqlite3_step(rs)) == SQLITE_ROW)
    {
        boolean pk = sqlite3_column_int(rs,5) > 0;
        const unsigned char *colname = sqlite3_column_text(rs,1);

		if(pk)
		{
			addToSet(key, strToUpper(strdup((char *) colname)));
		}
    }

    HANDLE_ERROR_MSG(rc, SQLITE_DONE, "error getting attributes of table <%s>", tableName);

    DEBUG_LOG("Key for %s are: %s", tableName, beatify(nodeToString(key)));

	if(!EMPTY_SET(key))
	{
		keys = singleton(key);
	}

    return keys;
}

DataType
sqliteBackendSQLTypeToDT (char *sqlType)
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
sqliteBackendDatatypeToSQL (DataType dt)
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
sqliteGetMinAndMax(char* tableName, char* colName)
{
	HashMap *result_map = NEW_MAP(Constant, HashMap);
    sqlite3_stmt *rs;
    StringInfo q;
	StringInfo colMinMax;
	List *attr = sqliteGetAttributes(tableName);
	List *aNames = getAttrDefNames(attr);
	List *aDTs = getAttrDataTypes(attr);
    int rc;

    q = makeStringInfo();
	colMinMax = makeStringInfo();

	// calculate min and max for each attribute
	FOREACH(char,a,aNames)
	{
		appendStringInfo(colMinMax, "min(%s) AS min_%s, max(%s) AS max_%s", a, a, a, a);
		appendStringInfo(colMinMax, "%s", FOREACH_HAS_MORE(a) ? ", " : "");
	}

    appendStringInfo(q, QUERY_TABLE_ATTR_MIN_MAX, colMinMax->data, tableName);
    rs = runQuery(q->data);

    while((rc = sqlite3_step(rs)) == SQLITE_ROW)
    {
		int pos = 0;
		FORBOTH_LC(ac, dtc, aNames, aDTs)
		{
			char *aname = LC_STRING_VAL(ac);
			DataType dt = (DataType) LC_INT_VAL(dtc);
			HashMap *minmax = NEW_MAP(Constant,Constant);
			const unsigned char *minVal = sqlite3_column_text(rs,pos++);
			const unsigned char *maxVal = sqlite3_column_text(rs,pos++);
			Constant *min, *max;

			switch(dt)
			{
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
				min = createConstString((char *) minVal);
				max = createConstString((char *) maxVal);
				break;
			default:
				THROW(SEVERITY_RECOVERABLE, "received unkown DT from sqlite: %s", DataTypeToString(dt));
				break;
			}
			MAP_ADD_STRING_KEY(minmax, MIN_KEY, min);
			MAP_ADD_STRING_KEY(minmax, MAX_KEY, max);
			MAP_ADD_STRING_KEY(result_map, aname, minmax);
		}
    }

    HANDLE_ERROR_MSG(rc, SQLITE_DONE, "error getting min and max values of attributes for table <%s>", tableName);

	DEBUG_NODE_BEATIFY_LOG("min maxes", MAP_GET_STRING(result_map, colName));

	return (HashMap *) MAP_GET_STRING(result_map, colName);
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

void
sqliteExecuteQueryIgnoreResults(char *query)
{
    sqlite3_stmt *rs = runQuery(query);
    int rc = SQLITE_OK;

    while((rc = sqlite3_step(rs)) == SQLITE_ROW)
        ;

    HANDLE_ERROR_MSG(rc,SQLITE_DONE, "failed to execute query <%s>", query);

    rc = sqlite3_finalize(rs);
    HANDLE_ERROR_MSG(rc,SQLITE_OK, "failed to finalize query <%s>", query);
}

static sqlite3_stmt *
runQuery (char *q)
{
    sqlite3 *conn = plugin->conn;
    sqlite3_stmt *stmt;
    int rc;

    DEBUG_LOG("run query:\n<%s>", q);
    rc = sqlite3_prepare(conn, strdup(q), -1, &stmt, NULL);
//    HANDLE_ERROR_MSG(rc, SQLITE_OK, "failed to prepare query <%s>", q);

   if (rc != SQLITE_OK)
   {
       StringInfo _newmes = makeStringInfo();
       appendStringInfo(_newmes, "failed to prepare query <%s>", q);
       StringInfo _errMes = makeStringInfo();
       appendStringInfo(_errMes, strdup((char *) sqlite3_errmsg(plugin->conn)));
       FATAL_LOG("error (%s)\n%u\n\n%s", _errMes->data, rc, _newmes->data);
   }

    return stmt;
}


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
sqliteGetConnectionDescription (void)
{
    return CONCAT_STRINGS("SQLite:", getStringOption("connection.db"));
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

DataType
sqliteBackendSQLTypeToDT (char *sqlType)
{
	return DT_INT;
}

char *
sqliteBackendDatatypeToSQL (DataType dt)
{
	return NULL;
}

HashMap *
sqliteGetMinAndMax(char* tableName, char* colName)
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
