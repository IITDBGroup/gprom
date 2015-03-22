/*
 * metadata_lookup.c
 *
 *      Author: zephyr
 */

#include "common.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "parser/parser.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"
#include "utility/string_utils.h"

/* If OCILIB and OCI are available then use it */
#if HAVE_ORACLE_BACKEND

#define ORACLE_TNS_CONNECTION_FORMAT "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=" \
		"(PROTOCOL=TCP)(HOST=%s)(PORT=%u)))(CONNECT_DATA=" \
		"(SERVER=DEDICATED)(SID=%s)))"

/*
 * functions and variables for internal use
 */

typedef struct TableBuffer
{
	char *tableName;
	List *attrs;
} TableBuffer;

typedef struct ViewBuffer
{
	char *viewName;
	char *viewDefinition;
} ViewBuffer;

static OCI_Connection *conn=NULL;
static OCI_Statement *st = NULL;
static OCI_TypeInfo *tInfo=NULL;
static OCI_Error *errorCache=NULL;
static MemContext *context=NULL;
static char **aggList=NULL;
static char **winfList = NULL;
static List *tableBuffers=NULL;
static List *viewBuffers=NULL;
static boolean initialized = FALSE;

static int initConnection(void);
static boolean isConnected(void);
static void initAggList(void);
static void freeAggList(void);
static void initWinfList(void);
static void freeWinfList(void);
static OCI_Transaction *createTransaction(IsolationLevel isoLevel);
static OCI_Resultset *executeStatement(char *statement);
static boolean executeNonQueryStatement(char *statement);
static void handleError (OCI_Error *error);
//static inline char *LobToChar (OCI_Lob *lob);
static DataType ociTypeToDT(unsigned int typ);

static void addToTableBuffers(char *tableName, List *attrs);
static void addToViewBuffers(char *viewName, char *viewDef);
static List *searchTableBuffers(char *tableName);
static char *searchViewBuffers(char *viewName);
static void freeBuffers(void);


/* assemble plugin and return */
MetadataLookupPlugin *
assembleOracleMetadataLookupPlugin (void)
{
    MetadataLookupPlugin *plugin = NEW(MetadataLookupPlugin);

    plugin->type = METADATA_LOOKUP_PLUGIN_ORACLE;

    plugin->initMetadataLookupPlugin = oracleInitMetadataLookupPlugin;
    plugin->databaseConnectionOpen = oracleDatabaseConnectionOpen;
    plugin->databaseConnectionClose = oracleDatabaseConnectionClose;
    plugin->shutdownMetadataLookupPlugin = oracleShutdownMetadataLookupPlugin;
    plugin->isInitialized = oracleIsInitialized;
    plugin->catalogTableExists = oracleCatalogTableExists;
    plugin->catalogViewExists = oracleCatalogViewExists;
    plugin->getAttributes = oracleGetAttributes;
    plugin->getAttributeNames = oracleGetAttributeNames;
    plugin->getAttributeDefaultVal = oracleGetAttributeDefaultVal;
    plugin->isAgg = oracleIsAgg;
    plugin->isWindowFunction = oracleIsWindowFunction;
    plugin->getTableDefinition = oracleGetTableDefinition;
    plugin->getViewDefinition = oracleGetViewDefinition;
    plugin->getOpReturnType = oracleGetOpReturnType;
    plugin->getFuncReturnType = oracleGetFuncReturnType;
    plugin->getTransactionSQLAndSCNs = oracleGetTransactionSQLAndSCNs;
    plugin->executeAsTransactionAndGetXID = oracleExecuteAsTransactionAndGetXID;
    plugin->getCommitScn = oracleGetCommitScn;
    plugin->executeQuery = oracleGenExecQuery;
    plugin->getCostEstimation = oracleGetCostEstimation;
    plugin->getKeyInformation = oracleGetKeyInformation;

    return plugin;
}

static void
handleError (OCI_Error *error)
{
    errorCache = error;
    DEBUG_LOG("METADATA LOOKUP - OCILIB Error ORA-%05i - msg : %s\n",
            OCI_ErrorGetOCICode(error), OCI_ErrorGetString(error));
}

static void
initAggList(void)
{
    //malloc space
    aggList = CNEW(char*, AGG_FUNCTION_COUNT);

    //assign string value
    aggList[AGG_MAX] = "max";
    aggList[AGG_MIN] = "min";
    aggList[AGG_AVG] = "avg";
    aggList[AGG_COUNT] = "count";
    aggList[AGG_SUM] = "sum";
    aggList[AGG_FIRST] = "first";
    aggList[AGG_LAST] = "last";
    aggList[AGG_CORR] = "corr";
    aggList[AGG_COVAR_POP] = "covar_pop";
    aggList[AGG_COVAR_SAMP] = "covar_samp";
    aggList[AGG_GROUPING] = "grouping";
    aggList[AGG_REGR] = "regr";
    aggList[AGG_STDDEV] = "stddev";
    aggList[AGG_STDDEV_POP] = "stddev_pop";
    aggList[AGG_STDEEV_SAMP] = "stddev_samp";
    aggList[AGG_VAR_POP] = "var_pop";
    aggList[AGG_VAR_SAMP] = "var_samp";
    aggList[AGG_VARIANCE] = "variance";
    aggList[AGG_XMLAGG] = "xmlagg";
}

static void
freeAggList()
{
	if(aggList != NULL)
		FREE(aggList);
	aggList = NULL;
}

static void
initWinfList(void)
{
    // malloc space
    winfList = CNEW(char*, WINF_FUNCTION_COUNT);

    // add functions
    winfList[WINF_MAX] = "max";
    winfList[WINF_MIN] = "min";
    winfList[WINF_AVG] = "avg";
    winfList[WINF_COUNT] = "count";
    winfList[WINF_SUM] = "sum";
    winfList[WINF_FIRST] = "first";
    winfList[WINF_LAST] = "last";

    // window specific
    winfList[WINF_FIRST_VALUE] = "first_value";
    winfList[WINF_ROW_NUMBER] = "row_number";
    winfList[WINF_RANK] = "rank";
    winfList[WINF_LAG] = "lag";
    winfList[WINF_LEAD] = "lead";
}

static void
freeWinfList(void)
{
    if (winfList != NULL)
        FREE(winfList);
    winfList = NULL;
}

static void
freeBuffers()
{
	if(tableBuffers != NULL)
	{
		//deep free table buffers
		FOREACH(TableBuffer, t, tableBuffers)
		{
			FREE(t->tableName);
			deepFree(t->attrs);
		}
		freeList(tableBuffers);
	}
	if(viewBuffers != NULL)
	{
		//deep free view buffers
		FOREACH(ViewBuffer, v, viewBuffers)
		{
			FREE(v->viewDefinition);
			FREE(v->viewName);
		}
		freeList(viewBuffers);
	}
	tableBuffers = NIL;
	viewBuffers = NIL;
}

static void
addToTableBuffers(char* tableName, List *attrList)
{
    TableBuffer *t = NEW(TableBuffer);
    char *name = strdup(tableName);
    t->tableName = name;
    t->attrs = attrList;
    tableBuffers = appendToTailOfList(tableBuffers, t);
}

static void
addToViewBuffers(char *viewName, char *viewDef)
{
    ViewBuffer *v = NEW(ViewBuffer);
    char *name = strdup(viewName);
    v->viewName = name;
    v->viewDefinition = viewDef;
    viewBuffers = appendToTailOfList(viewBuffers, v);
}

static List *
searchTableBuffers(char *tableName)
{
    if(tableBuffers == NULL || tableName == NULL)
        return NIL;
    FOREACH(TableBuffer, t, tableBuffers)
    {
        if(strcmp(t->tableName, tableName) == 0)
            return t->attrs;
    }
    return NIL;
}
static char *
searchViewBuffers(char *viewName)
{
    if(viewBuffers == NULL || viewName == NULL)
        return NULL;
    FOREACH(ViewBuffer, v, viewBuffers)
    {
        if(strcmp(v->viewName, viewName) == 0)
            return v->viewDefinition;
    }
    return NULL;
}

static int
initConnection()
{
    ASSERT(initialized);

    ACQUIRE_MEM_CONTEXT(context);

    StringInfo connectString = makeStringInfo();
//    Options* options=getOptions();

    char* user = getStringOption("connection.user");
    char* passwd = getStringOption("connection.passwd");
    char* db = getStringOption("connection.db");
    char *host = getStringOption("connection.host");
    int port = getIntOption("connection.port");

    appendStringInfo(connectString, ORACLE_TNS_CONNECTION_FORMAT,
    		host ? host : "",
    		port ? port : 1521,
            db ? db : "");

    conn = OCI_ConnectionCreate(connectString->data,
    		user ? user : "",
    		passwd ? passwd : "",
            OCI_SESSION_DEFAULT);
    DEBUG_LOG("Try to connect to server <%s,%s,%s>... %s", connectString->data,
    		user ? user : "",
    		passwd ? passwd : "",
            (conn != NULL) ? "SUCCESS" : "FAILURE");

    initAggList();
    initWinfList();

    RELEASE_MEM_CONTEXT();

    return EXIT_SUCCESS;
}

static boolean
isConnected()
{
    if(conn==NULL)
        initConnection();
    if(OCI_IsConnected(conn))
        return TRUE;
    else
    {
        FATAL_LOG("OCI connection lost: %s", OCI_ErrorGetString(errorCache));
        return FALSE;
    }
}

int
oracleInitMetadataLookupPlugin (void)
{
    if (initialized)
    {
        INFO_LOG("tried to initialize metadata lookup plugin more than once");
        return EXIT_SUCCESS;
    }

    NEW_AND_ACQUIRE_MEMCONTEXT("metadataContext");
    context=getCurMemContext();

    if(!OCI_Initialize(handleError, NULL, OCI_ENV_DEFAULT))
    {
        FATAL_LOG("Cannot initialize OICLIB: %s", OCI_ErrorGetString(errorCache)); //print error type
        RELEASE_MEM_CONTEXT();

        return EXIT_FAILURE;
    }

    DEBUG_LOG("Initialized OCILIB");
    RELEASE_MEM_CONTEXT();
    initialized = TRUE;

    return EXIT_SUCCESS;
}

int
oracleShutdownMetadataLookupPlugin (void)
{
    return oracleDatabaseConnectionClose();
}

boolean
oracleIsInitialized (void)
{
    return initialized;
}


OCI_Connection *
getConnection()
{
    if(isConnected())
        return conn;
    return NULL;
}

boolean
oracleCatalogTableExists(char* tableName)
{
    if(NULL==tableName)
        return FALSE;
    if(conn==NULL)
        initConnection();

    START_TIMER("module - metadata lookup");

    if(isConnected())
    {
        STOP_TIMER("module - metadata lookup");
        return (OCI_TypeInfoGet(conn,tableName,OCI_TIF_TABLE)==NULL) ? FALSE : TRUE;
    }

    STOP_TIMER("module - metadata lookup");

    return FALSE;
}

boolean
oracleCatalogViewExists(char* viewName)
{
    if(NULL==viewName)
        return FALSE;
    if(conn==NULL)
        initConnection();

    START_TIMER("module - metadata lookup");
    if(isConnected())
    {
        STOP_TIMER("module - metadata lookup");
        return (OCI_TypeInfoGet(conn,viewName,OCI_TIF_VIEW)==NULL) ? FALSE : TRUE;
    }

    STOP_TIMER("module - metadata lookup");

    return FALSE;
}

List *
oracleGetAttributeNames (char *tableName)
{
    List *attrNames = NIL;
    List *attrs = getAttributes(tableName);
    //TODO use attribute defition instead
    FOREACH(AttributeDef,a,attrs)
        attrNames = appendToTailOfList(attrNames, a->attrName);

    return attrNames;
}

Node *
oracleGetAttributeDefaultVal (char *schema, char *tableName, char *attrName)
{
    StringInfo statement = makeStringInfo();

    ACQUIRE_MEM_CONTEXT(context);
    START_TIMER("module - metadata lookup");

    DEBUG_LOG("Get default for %s.%s.%s", schema, tableName, attrName);

    // run query to fetch default value for attribute if it exists and return it
    appendStringInfo(statement,
            "SELECT DATA_DEFAULT "
            "FROM SYS.DBA_TAB_COLS_V$ "
            "WHERE TABLE_NAME = '%s' "
            "AND OWNER = '%s' "
            "AND COLUMN_NAME = '%s'",
            tableName,
            schema,
            attrName);
    if ((conn = getConnection()) != NULL)
    {
        OCI_Resultset *rs = executeStatement(statement->data);
        char *defaultExpr = NULL;
        Node *result = NULL;

        // loop through
        while(OCI_FetchNext(rs))
        {
            defaultExpr = (char *) OCI_GetString(rs,1);

            DEBUG_LOG("default expr for %s.%s.%s is <%s>",
                    schema ? schema : "", tableName, attrName, defaultExpr);
        }

        DEBUG_LOG("Statement: %s executed successfully.", statement->data);
        FREE(statement);

        // parse expression
        if (defaultExpr != NULL)
            result = parseFromString(defaultExpr);

        RELEASE_MEM_CONTEXT_AND_RETURN_COPY(Node, result);
    }
    else
    {
        FATAL_LOG("Statement: %s failed.", statement);
        FREE(statement);
    }
    STOP_TIMER("module - metadata lookup");

    // return NULL if no default and return to callers memory context
    RELEASE_MEM_CONTEXT_AND_RETURN_COPY(Node, NULL);
}

List*
oracleGetAttributes(char *tableName)
{
    List *attrList=NIL;

    ACQUIRE_MEM_CONTEXT(context);

    START_TIMER("module - metadata lookup");

    if(tableName==NULL)
    {
        STOP_TIMER("module - metadata lookup");
        RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
    }
    if((attrList = searchTableBuffers(tableName)) != NIL)
    {
        RELEASE_MEM_CONTEXT();
        STOP_TIMER("module - metadata lookup");
        return copyObject(attrList);
    }
    //TODO use query SELECT c.COLUMN_NAME, c.DATA_TYPE FROM DBA_TAB_COLUMNS c WHERE OWNER='FGA_USER' AND TABLE_NAME = 'R' ORDER BY COLUMN_ID;
    // how to figure out schema
    if(conn==NULL)
        initConnection();
    if(isConnected())
    {
        int i,n;
        tInfo = OCI_TypeInfoGet(conn,tableName,OCI_TIF_TABLE);
        n = OCI_TypeInfoGetColumnCount(tInfo);

        for(i = 1; i <= n; i++)
        {
            OCI_Column *col = OCI_TypeInfoGetColumn(tInfo, i);

            AttributeDef *a = createAttributeDef(strdup((char *) OCI_GetColumnName(col)),
                    ociTypeToDT(OCI_GetColumnType(col)));
            attrList = appendToTailOfList(attrList,a);
        }

        //add to table buffer list as cache to improve performance
        //user do not have to free the attrList by themselves
        addToTableBuffers(tableName, attrList);

        RELEASE_MEM_CONTEXT();
        STOP_TIMER("module - metadata lookup");
        return copyObject(attrList); //TODO copying
    }

    ERROR_LOG("Not connected to database.");
    STOP_TIMER("module - metadata lookup");
    RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
}

static DataType
ociTypeToDT (unsigned int typ)
{
    /* - OCI_CDT_NUMERIC     : short, int, long long, float, double
     * - OCI_CDT_DATETIME    : OCI_Date *
     * - OCI_CDT_TEXT        : dtext *
     * - OCI_CDT_LONG        : OCI_Long *
     * - OCI_CDT_CURSOR      : OCI_Statement *
     * - OCI_CDT_LOB         : OCI_Lob  *
     * - OCI_CDT_FILE        : OCI_File *
     * - OCI_CDT_TIMESTAMP   : OCI_Timestamp *
     * - OCI_CDT_INTERVAL    : OCI_Interval *
     * - OCI_CDT_RAW         : void *
     * - OCI_CDT_OBJECT      : OCI_Object *
     * - OCI_CDT_COLLECTION  : OCI_Coll *
     * - OCI_CDT_REF         : OCI_Ref *
     */
    switch(typ)
    {
        case OCI_CDT_NUMERIC:
            return DT_INT;
        case OCI_CDT_TEXT:
            return DT_STRING;
        //TODO distinguish between int and float
    }

    return DT_STRING;
}

boolean
oracleIsAgg(char* functionName)
{
    if(functionName == NULL)
        return FALSE;

    for(int i = 0; i < AGG_FUNCTION_COUNT; i++)
    {
        if(strcasecmp(aggList[i], functionName) == 0)
            return TRUE;
    }
    return FALSE;
}

boolean
oracleIsWindowFunction(char *functionName)
{
    if (functionName == NULL)
        return FALSE;

    for(int i = 0; i < WINF_FUNCTION_COUNT; i++)
    {
        if (strcasecmp(winfList[i], functionName) == 0)
            return TRUE;
    }

    return FALSE;
}

char *
oracleGetTableDefinition(char *tableName)
{
    StringInfo statement;
    char *result;

    ACQUIRE_MEM_CONTEXT(context);

    START_TIMER("module - metadata lookup");

    statement = makeStringInfo();
    appendStringInfo(statement, "select DBMS_METADATA.GET_DDL('TABLE', '%s\')"
            " from DUAL", tableName);

    OCI_Resultset *rs = executeStatement(statement->data);
    if(rs != NULL)
    {
        if(OCI_FetchNext(rs))
        {
            FREE(statement);
            result = strdup((char *)OCI_GetString(rs, 1));
            STOP_TIMER("module - metadata lookup");
            RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
        }
    }
    FREE(statement);
    STOP_TIMER("module - metadata lookup");
    RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(NULL);
}

void
oracleGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls, List **sqlBinds,
        IsolationLevel *iso, Constant *commitScn)
{
    START_TIMER("module - metadata lookup");

    if(xid != NULL)
    {
        StringInfo statement;
        statement = makeStringInfo();
        char *auditTable = strToUpper(getStringOption("backendOpts.oracle.logtable"));
        *scns = NIL;
        *sqls = NIL;
        *sqlBinds = NIL;

        // FETCH statements, SCNs, and parameter bindings
        if (streq(auditTable, "FGA_LOG$"))
        {
            appendStringInfo(statement, "SELECT SCN, "
                    " CASE WHEN DBMS_LOB.GETLENGTH(lsqltext) > 4000 THEN lsqltext ELSE NULL END AS lsql,"
                    " CASE WHEN DBMS_LOB.GETLENGTH(lsqltext) <= 4000 THEN DBMS_LOB.SUBSTR(lsqltext,4000)  ELSE NULL END AS shortsql,"
                    " CASE WHEN DBMS_LOB.GETLENGTH(lsqlbind) > 4000 THEN lsqlbind ELSE NULL END AS lbind,"
                    " CASE WHEN DBMS_LOB.GETLENGTH(lsqlbind) <= 4000 THEN DBMS_LOB.SUBSTR(lsqlbind,4000)  ELSE NULL END AS shortbind"
                    " FROM "
                    "(SELECT SCN, LSQLTEXT, LSQLBIND, ntimestamp#, "
                    "   DENSE_RANK() OVER (PARTITION BY statement ORDER BY policyname) AS rnum "
                    "      FROM SYS.fga_log$ "
                    "      WHERE xid = HEXTORAW('%s')) x "
                    "WHERE rnum = 1 "
                    "ORDER BY ntimestamp#", xid);
        }
        else if (streq(auditTable, "UNIFIED_AUDIT_TRAIL"))
        {
            appendStringInfo(statement, "SELECT SCN, \n"
                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) > 4000 THEN SQL_TEXT ELSE NULL END AS lsql,\n"
                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_TEXT,4000)  ELSE NULL END AS shortsql,\n"
                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) > 4000 THEN SQL_BINDS ELSE NULL END AS lbind,\n"
                    "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_BINDS,4000)  ELSE NULL END AS shortbind\n"
                    "FROM \n"
                    "\t(SELECT SCN, SQL_TEXT, SQL_BINDS, ENTRY_ID\n"
                    "\tFROM SYS.UNIFIED_AUDIT_TRAIL \n"
                    "\tWHERE TRANSACTION_ID = HEXTORAW(\'%s\')) x \n"
                    "ORDER BY ENTRY_ID", xid);
        }
        else
        {
            appendStringInfo(statement, "SELECT SCN, \n"
                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) > 4000 THEN SQL_TEXT ELSE NULL END AS lsql,\n"
                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_TEXT) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_TEXT,4000)  ELSE NULL END AS shortsql,\n"
                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) > 4000 THEN SQL_BINDS ELSE NULL END AS lbind,\n"
                                "\t\tCASE WHEN DBMS_LOB.GETLENGTH(SQL_BINDS) <= 4000 THEN DBMS_LOB.SUBSTR(SQL_BINDS,4000)  ELSE NULL END AS shortbind\n"
                                "FROM \n"
                                "\t(SELECT SCN, SQL_TEXT, SQL_BINDS, ENTRY_ID\n"
                                "\tFROM %s \n"
                                "\tWHERE TRANSACTION_ID = HEXTORAW(\'%s\')) x \n"
                                "ORDER BY ENTRY_ID",
                                auditTable,
                                xid);
        }

        if((conn = getConnection()) != NULL)
        {
            OCI_Resultset *rs = executeStatement(statement->data);

            // loop through
            while(OCI_FetchNext(rs))
            {
                START_TIMER("module - metadata lookup - fetch transaction info");

                START_TIMER("module - metadata lookup - fetch transaction info - fetch SCN");
                long scn = (long) OCI_GetBigInt(rs,1); // SCN
                STOP_TIMER("module - metadata lookup - fetch transaction info - fetch SCN");

                START_TIMER("module - metadata lookup - fetch transaction info - fetch SQL");
                const char *sql;
                if (OCI_IsNull(rs,2))
                    sql = OCI_GetString(rs,3);
                else
                    sql = OCI_GetString(rs,2);
                STOP_TIMER("module - metadata lookup - fetch transaction info - fetch SQL");

                START_TIMER("module - metadata lookup - fetch transaction info - fetch bind");
                const char *bind; // SQLBIND
                if (OCI_IsNull(rs,4))
                    bind = OCI_GetString(rs,5);
                else
                    bind = OCI_GetString(rs,4);
                STOP_TIMER("module - metadata lookup - fetch transaction info - fetch bind");

                START_TIMER("module - metadata lookup - fetch transaction info - concat strings");
                char *sqlPlusSemicolon = CONCAT_STRINGS(sql, ";");
                STOP_TIMER("module - metadata lookup - fetch transaction info - concat strings");

                START_TIMER("module - metadata lookup - fetch transaction info - append");
                *sqls = appendToTailOfList(*sqls, sqlPlusSemicolon);
                *scns = appendToTailOfList(*scns, createConstLong(scn));
                *sqlBinds = appendToTailOfList(*sqlBinds, strdup( (char *) bind));
                DEBUG_LOG("Current statement at SCN %u\n was:\n%s\nwithBinds:%s", scn, sql, bind);
                STOP_TIMER("module - metadata lookup - fetch transaction info - append");

                STOP_TIMER("module - metadata lookup - fetch transaction info");
            }

            DEBUG_LOG("Statement: %s executed successfully.", statement->data);
            DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
            FREE(statement);
        }
        else
        {
            ERROR_LOG("Statement: %s failed.", statement);
            FREE(statement);
            STOP_TIMER("module - metadata lookup");
            return;
        }

        INFO_LOG("transaction statements are:\n\n%s", beatify(stringListToString(*sqls)));

        // infer isolation level
        statement = makeStringInfo();
        if (streq(auditTable, "FGA_LOG$"))
        {
            appendStringInfo(statement, "SELECT "
                             "CASE WHEN (count(DISTINCT scn) > 1) "
                             "THEN 1 "
                             "ELSE 0 "
                             "END AS readCommmit\n"
                             "FROM SYS.fga_log$\n"
                             "WHERE xid = HEXTORAW(\'%s\')",
                             xid);
        }
        else if (streq(auditTable, "UNIFIED_AUDIT_TRAIL"))
        {
            appendStringInfo(statement, "SELECT "
                    "CASE WHEN (count(DISTINCT scn) > 1) "
                    "THEN 1 "
                    "ELSE 0 "
                    "END AS readCommmit\n"
                    "FROM SYS.UNIFIED_AUDIT_TRAIL\n"
                    "WHERE TRANSACTION_ID = HEXTORAW(\'%s\')",
                    xid);
        }
        else
        {
            appendStringInfo(statement, "SELECT "
                                "CASE WHEN (count(DISTINCT scn) > 1) "
                                "THEN 1 "
                                "ELSE 0 "
                                "END AS readCommmit\n"
                                "FROM %s\n"
                                "WHERE TRANSACTION_ID = HEXTORAW(\'%s\')",
                                auditTable,
                                xid);
        }

        if ((conn = getConnection()) != NULL)
        {
            START_TIMER("module - metadata lookup - get isolation level");

            OCI_Resultset *rs = executeStatement(statement->data);

            // loop through
            while(OCI_FetchNext(rs))
            {
                long isoA = (long) OCI_GetBigInt(rs,1); // ISOLEVEL

                switch(isoA)
                {
                    case 0:
                        *iso = ISOLATION_SERIALIZABLE;
                        break;
                    case 1:
                        *iso = ISOLATION_READ_COMMITTED;
                        break;
                    default:
                        *iso = ISOLATION_READ_ONLY;
                        break;
                }
                DEBUG_LOG("Transaction ISOLEVEL %u", iso);
            }

            DEBUG_LOG("Statement: %s executed successfully.", statement->data);
            DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
            FREE(statement);

            STOP_TIMER("module - metadata lookup - get isolation level");
        }
        else
        {
            FATAL_LOG("Statement: %s failed.", statement);
            FREE(statement);
        }

        // get COMMIT SCN
        long commitS = -1; // getCommitScn("",
//                LONG_VALUE(getTailOfListP(*scns)),
//                xid); // LONG_VALUE(getTailOfListP(*scns)) + 1;
        (*((long *) commitScn->value)) = commitS; //TODO write query to get real COMMIT SCN
    }
    STOP_TIMER("module - metadata lookup");
}

long
oracleGetCommitScn (char *tableName, long maxScn, char *xid)
{
    StringInfo statement = makeStringInfo();
    long commitScn = 0;

    START_TIMER("module - metadata lookup");
    START_TIMER("module - metadata lookup - get commit SCN");

    appendStringInfo(statement, "SELECT DISTINCT VERSIONS_STARTSCN FROM "
            "%s VERSIONS BETWEEN SCN %u AND MAXVALUE "
            "WHERE VERSIONS_XID = HEXTORAW('%s')", tableName, maxScn, xid);


    if((conn = getConnection()) != NULL)
    {
        OCI_Resultset *rs = executeStatement(statement->data);

        // loop through
        while(OCI_FetchNext(rs))
            commitScn = (long) OCI_GetBigInt(rs,1);

        ASSERT(commitScn != 0);

        DEBUG_LOG("statement %s \n\nfinished and returned commit SCN %u",
                statement->data, maxScn);

        FREE(statement);
    }
    else
    {
        FATAL_LOG("statement %s execution failed", statement->data);
        FREE(statement);
    }

    STOP_TIMER("module - metadata lookup - get commit SCN");
    STOP_TIMER("module - metadata lookup");

    return commitScn;
}

char *
oracleGetViewDefinition(char *viewName)
{
    char *def = NULL;
    StringInfo statement;

    START_TIMER("module - metadata lookup");

    ACQUIRE_MEM_CONTEXT(context);

    if((def = searchViewBuffers(viewName)) != NULL)
    {
        RELEASE_MEM_CONTEXT();
        STOP_TIMER("module - metadata lookup");
        return def;
    }

    statement = makeStringInfo();
    appendStringInfo(statement, "select text from user_views where "
            "view_name = '%s'", viewName);

    OCI_Resultset *rs = executeStatement(statement->data);
    if(rs != NULL)
    {
        if(OCI_FetchNext(rs))
        {
            char *def = strdup((char *) OCI_GetString(rs, 1));
            //add view definition to view buffers to improve performance
            //user should not free def by themselves
            addToViewBuffers(viewName, def);
            FREE(statement);
            RELEASE_MEM_CONTEXT();
            return def;
        }
    }
    FREE(statement);

    STOP_TIMER("module - metadata lookup");
    RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY (NULL);
}

DataType
oracleGetOpReturnType (char *oName, List *dataTypes)
{
    return DT_STRING;
}

DataType
oracleGetFuncReturnType (char *fName, List *dataTypes)
{
    // aggregation functions
    if (streq(fName,"sum")
            || streq(fName, "min")
            || streq(fName, "max")
        )
    {
        ASSERT(LIST_LENGTH(dataTypes) == 1);
        DataType argType = getNthOfListInt(dataTypes,0);

        switch(argType)
        {
            case DT_INT:
            case DT_LONG:
                return DT_LONG;
            case DT_FLOAT:
                return DT_FLOAT;
            default:
                return DT_STRING;
        }
    }

    if (streq(fName,"avg"))
    {
        ASSERT(LIST_LENGTH(dataTypes) == 1);
        DataType argType = getNthOfListInt(dataTypes,0);

        switch(argType)
        {
            case DT_INT:
            case DT_LONG:
            case DT_FLOAT:
                return DT_FLOAT;
            default:
                return DT_STRING;
        }
    }

    if (streq(fName,"count"))
        return DT_LONG;

    if (streq(fName,"xmlagg"))
        return DT_STRING;

    if (streq(fName,"ROW_NUMBER"))
        return DT_INT;
    if (streq(fName, "DENSE_RANK"))
        return DT_INT;

    return DT_STRING;
}


long
getBarrierScn(void)
{
    long barrier = -1;
    StringInfo statement;

    START_TIMER("module - metadata lookup");

    ACQUIRE_MEM_CONTEXT(context);

    statement = makeStringInfo();
    appendStringInfo(statement, "SELECT BARRIERSCN FROM SYS.SYS_FBA_BARRIERSCN;");

    OCI_Resultset *rs = executeStatement(statement->data);
    if(rs != NULL)
    {
        if(OCI_FetchNext(rs))
        {
            barrier = (long) OCI_GetBigInt(rs,1);
            FREE(statement);
            RELEASE_MEM_CONTEXT();
            STOP_TIMER("module - metadata lookup");
            return barrier;
        }
    }
    FREE(statement);

    RELEASE_MEM_CONTEXT();
    STOP_TIMER("module - metadata lookup");
    return barrier;
}


int
oracleGetCostEstimation(char *query)
{
    /* Remove the newline characters from the Query */
    int len = strlen(query);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        if (query[i] == '\n' || query[i] == ';')
            query[i] = ' ';
    }

    unsigned long long int cost = 0L;

    StringInfo statement;
    statement = makeStringInfo();
    appendStringInfo(statement, "EXPLAIN PLAN FOR %s", query);
    executeStatement(statement->data);
    FREE(statement);

    StringInfo statement1;
    statement1 = makeStringInfo();
    appendStringInfo(statement1, "SELECT MAX(COST) FROM PLAN_TABLE");

    OCI_Resultset *rs1 = executeStatement(statement1->data);
    if (rs1 != NULL)
    {
	while(OCI_FetchNext(rs1))
        {
            cost = (unsigned long long int) OCI_GetBigInt(rs1,1);
            DEBUG_LOG("Cost is : %u \n", cost);
            break;
        }
    }

    FREE(statement1);

    StringInfo statement2;
    statement2 = makeStringInfo();
    appendStringInfo(statement2, "DELETE FROM PLAN_TABLE");
    executeStatement(statement2->data);
    FREE(statement2);

    StringInfo statement3;
    statement3 = makeStringInfo();
    appendStringInfo(statement3, "COMMIT");
    executeStatement(statement3->data);
    FREE(statement3);

    return cost;
}

List *
oracleGetKeyInformation(char *tableName)
{
    List *keyList = NIL;

    StringInfo statement;
    statement = makeStringInfo();
    appendStringInfo(statement, "SELECT cols.column_name "
                                "FROM all_constraints cons, all_cons_columns cols "
                                "WHERE cols.table_name = '%s' "
                                "AND cons.constraint_type = 'P' "
                                "AND cons.constraint_name = cols.constraint_name "
                                "AND cons.owner = cols.owner "
                                "ORDER BY cols.table_name, cols.position",
                                tableName);

    OCI_Resultset *rs1 = executeStatement(statement->data);

    if (rs1 != NULL)
    {
        while(OCI_FetchNext(rs1))
        {
            keyList = appendToTailOfList(keyList, strdup((char *) OCI_GetString(rs1, 1)));
        }
    }

    FREE(statement);
    return keyList;
}

static OCI_Resultset *
executeStatement(char *statement)
{
    if(statement == NULL)
        return NULL;
    if((conn = getConnection()) != NULL)
    {
        START_TIMER("Oracle - execute SQL");
        if(st == NULL)
        {
            st = OCI_StatementCreate(conn);
            OCI_SetFetchSize(st,1000);
            OCI_SetPrefetchSize(st,1000);
        }
        OCI_ReleaseResultsets(st);
        if(OCI_ExecuteStmt(st, statement))
        {
            OCI_Resultset *rs = OCI_GetResultset(st);
            DEBUG_LOG("Statement: %s executed successfully.", statement);
            DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
            STOP_TIMER("Oracle - execute SQL");
            return rs;
        }
        else
        {
            ERROR_LOG("Statement: %s failed.", statement);
        }
        STOP_TIMER("Oracle - execute SQL");
    }
    return NULL;
}

static boolean
executeNonQueryStatement(char *statement)
{
    if(statement == NULL)
        return FALSE;
    if((conn = getConnection()) != NULL)
    {
        START_TIMER("module - metadata lookup");
        if(st == NULL)
            st = OCI_StatementCreate(conn);
        OCI_ReleaseResultsets(st);
        if(OCI_ExecuteStmt(st, statement))
        {
            DEBUG_LOG("Statement: %s executed successfully.", statement);
            STOP_TIMER("module - metadata lookup");
            return TRUE;
        }
        else
        {
            ERROR_LOG("Statement: %s failed.", statement);
            STOP_TIMER("module - metadata lookup");
            return FALSE;
        }
    }
    return FALSE;
}

Node *
oracleExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    OCI_Transaction *t;
    OCI_Resultset *rs;
    Constant *xid;

    if (!isConnected())
        FATAL_LOG("No connection to database");

    START_TIMER("module - metadata lookup");

    // create transaction
    t = createTransaction(isoLevel);
    if (t == NULL)
        FATAL_LOG("failed creating transaction");
    if (!OCI_SetTransaction(conn, t))
        FATAL_LOG("failed setting current transaction");

    // execute SQL
    FOREACH(char,sql,statements)
        if (!executeNonQueryStatement(sql))
        {
            ERROR_LOG("statement %s failed", sql);
            if (!OCI_Rollback(conn))
                FATAL_LOG("Failed rolling back current transaction");
            return NULL;
        }

    // get Transaction XID
    rs = executeStatement("SELECT RAWTOHEX(XID) AS XID FROM v$transaction");
    if (rs != NULL)
    {
        if(OCI_FetchNext(rs))
        {
            const char *xidString = OCI_IsNull(rs,1) ? NULL : OCI_GetString(rs,1);
            if (xidString == NULL)
                FATAL_LOG("query to retrieve XID did not return any value");
            DEBUG_LOG("Transaction executed with XID: <%s>", (char *) xidString);
            xid = createConstString((char *) xidString);
        }
        else
            FATAL_LOG("query to get back transaction xid failed");
    }
    else
        FATAL_LOG("query to get back transaction xid failed");
    // commit transaction and cleanup
    OCI_Commit(conn);
    if (!OCI_TransactionFree(t))
        FATAL_LOG("Failed freeing transaction");

    STOP_TIMER("module - metadata lookup");

    return (Node *) xid;
}

List *
oracleGenExecQuery (char *query)
{
    List *rel = NIL;
    int numAttrs;
    OCI_Resultset *rs;

    rs = executeStatement(query);
    numAttrs = OCI_GetColumnCount(rs);

    while(OCI_FetchNext(rs))
    {
        List *tuple = NIL;

        for(int i = 1; i <= numAttrs; i++)
            tuple = appendToTailOfList(tuple, strdup((char *) OCI_GetString(rs, i)));

        rel = appendToTailOfList(rel, tuple);
    }

    return rel;
}

static OCI_Transaction *
createTransaction(IsolationLevel isoLevel)
{
    unsigned int mode = 0;
    OCI_Transaction *result = NULL;

    START_TIMER("module - metadata lookup");

    // get OCI isolevel constant
    switch(isoLevel)
    {
        case ISOLATION_SERIALIZABLE:
            mode = OCI_TRS_SERIALIZABLE;
            break;
        case ISOLATION_READ_COMMITTED:
            mode = OCI_TRS_READWRITE;
            break;
        case ISOLATION_READ_ONLY:
            mode = OCI_TRS_READONLY;
            break;
    }

    // create transaction
    if((conn = getConnection()) != NULL)
        result = OCI_TransactionCreate(conn, 0, mode, NULL);
    else
        ERROR_LOG("Cannot create transaction: No connection established yet.");

    STOP_TIMER("module - metadata lookup");

    return result;
}

int
oracleDatabaseConnectionOpen()
{
    if(conn==NULL)
        initConnection();
    if(isConnected())
        return EXIT_SUCCESS;
    return EXIT_FAILURE;
}

int
oracleDatabaseConnectionClose()
{
	if(context==NULL)
	{
		ERROR_LOG("Metadata context already freed.");
		return EXIT_FAILURE;
	}
	else
	{
	    ACQUIRE_MEM_CONTEXT(context);
		freeAggList();
		freeWinfList();
		freeBuffers();
		OCI_Cleanup();//bugs exist here
		initialized = FALSE;
		conn=NULL;
	    st = NULL;
	    tInfo=NULL;
	    errorCache=NULL;

		FREE_AND_RELEASE_CUR_MEM_CONTEXT();
	}
	return EXIT_SUCCESS;
}

//#define maxRead 8000

//static inline char *
//LobToChar (OCI_Lob *lob)
//{
//    unsigned int read = 1;
//    unsigned int byteRead = 0;
//    static char buf[maxRead];
//    StringInfo str = makeStringInfo();
//
//    if (lob == NULL)
//        return "";
//
//    while(OCI_LobRead2(lob, buf, &read, &byteRead) && read > 0)
//    {
//        buf[read] = '\0';
//        appendStringInfoString(str,buf);
//        DEBUG_LOG("read CLOB (%u): %s", read, buf);
//        read = maxRead - 1;
//    }
//
//    DEBUG_LOG("read CLOB: %s", str->data);
//
//    return str->data;
//}

/* OCILIB is not available, fake functions */
#else

int
oracleInitMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

boolean
oracleCatalogTableExists(char *table)
{
    return FALSE;
}

boolean
oracleCatalogViewExists(char *view)
{
	return FALSE;
}

List *
oracleGetAttributes (char *table)
{
    return NIL;
}

List *
oracleGetAttributeNames (char *tableName)
{
    return NIL;
}

boolean
oracleIsAgg(char *table)
{
	return FALSE;
}

boolean
oracleIsWindowFunction(char *functionName)
{
    return FALSE;
}

char *
oracleGetTableDefinition(char *table) {
    return NULL;
}

void oracleGetTransactionSQLAndSCNs(char *xid, List **scns, List **sqls, List **sqlBinds,
        IsolationLevel *iso, Constant *commitScn)
{
}

char *
oracleGetViewDefinition(char *view) {
    return NULL;
}

char *
oracleExecuteStatement(char *statement)
{
	return NULL;
}

Node *
oracleExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
oracleDatabaseConnectionOpen (void)
{
    return EXIT_SUCCESS;
}


int
oracleDatabaseConnectionClose ()
{
    return EXIT_SUCCESS;
}

MetadataLookupPlugin *
assembleOracleMetadataLookupPlugin (void)
{
    return NULL;
}

DataType
oracleGetOpReturnType (char *oName, List *dataTypes)
{
    return DT_STRING;
}

DataType
oracleGetFuncReturnType (char *fName, List *dataTypes)
{
    return DT_STRING;
}

int
oracleGetCostEstimation(char *query)
{
    return 0;
}

List *
oracleGetKeyInformation(char *tableName)
{
    return NULL;
}


#endif
