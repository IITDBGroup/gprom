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
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"

/* If OCILIB and OCI are available then use it */
#if HAVE_LIBOCILIB && (HAVE_LIBOCI || HAVE_LIBOCCI)

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
static inline char *LobToChar (OCI_Lob *lob);

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
    plugin->isAgg = oracleIsAgg;
    plugin->isWindowFunction = oracleIsWindowFunction;
    plugin->getTableDefinition = oracleGetTableDefinition;
    plugin->getViewDefinition = oracleGetViewDefinition;
    plugin->getTransactionSQLAndSCNs = oracleGetTransactionSQLAndSCNs;
    plugin->executeAsTransactionAndGetXID = oracleExecuteAsTransactionAndGetXID;
    plugin->getCommitScn = oracleGetCommitScn;

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
    Options* options=getOptions();

    char* user=options->optionConnection->user;
    char* passwd=options->optionConnection->passwd;
    char* db=options->optionConnection->db;
    char *host=options->optionConnection->host;
    int port=options->optionConnection->port;
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
        FATAL_LOG("tried to initialize metadata lookup plugin more than once");

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

    FOREACH(AttributeReference,a,attrs)
        attrNames = appendToTailOfList(attrNames, a->name);

    return attrNames;
}

List*
oracleGetAttributes(char *tableName)
{
    List *attrList=NIL;

    ACQUIRE_MEM_CONTEXT(context);

    START_TIMER("module - metadata lookup");

    if(tableName==NULL)
        RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
    if((attrList = searchTableBuffers(tableName)) != NIL)
    {
        RELEASE_MEM_CONTEXT();
        STOP_TIMER("module - metadata lookup");
        return attrList;
    }

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
            AttributeReference *a = createAttributeReference((char *) OCI_GetColumnName(col));
            attrList=appendToTailOfList(attrList,a);
        }

        //add to table buffer list as cache to improve performance
        //user do not have to free the attrList by themselves
        addToTableBuffers(tableName, attrList);
        RELEASE_MEM_CONTEXT();
        STOP_TIMER("module - metadata lookup");
        return attrList;
    }
    ERROR_LOG("Not connected to database.");

    STOP_TIMER("module - metadata lookup");

    // copy result to callers memory context
    RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
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

        *scns = NIL;
        *sqls = NIL;
        *sqlBinds = NIL;

        // FETCH statements, SCNs, and parameter bindings
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

        // infer isolation level
        statement = makeStringInfo();
        appendStringInfo(statement, "SELECT "
                "CASE WHEN (count(DISTINCT scn) > 1) "
                "THEN 1 "
                "ELSE 0 "
                "END AS readCommmit\n"
                "FROM SYS.fga_log$\n"
                "WHERE xid = HEXTORAW(\'%s\')",
                xid);

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
            //user do not have to free def by themselves
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

static OCI_Resultset *
executeStatement(char *statement)
{
    if(statement == NULL)
        return NULL;
    if((conn = getConnection()) != NULL)
    {
        START_TIMER("Oracle - execute SQL");
        if(st == NULL)
            st = OCI_StatementCreate(conn);
        OCI_ReleaseResultsets(st);
        OCI_SetFetchSize(st,1000);
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

static OCI_Transaction *
createTransaction(IsolationLevel isoLevel)
{
    unsigned int mode;
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

#define maxRead 8000

static inline char *
LobToChar (OCI_Lob *lob)
{
    unsigned int read = 1;
    unsigned int byteRead = 0;
    static char buf[maxRead];
    StringInfo str = makeStringInfo();

    if (lob == NULL)
        return "";

    while(OCI_LobRead2(lob, buf, &read, &byteRead) && read > 0)
    {
        buf[read] = '\0';
        appendStringInfoString(str,buf);
        DEBUG_LOG("read CLOB (%u): %s", read, buf);
        read = maxRead - 1;
    }

    DEBUG_LOG("read CLOB: %s", str->data);

    return str->data;
}

/* OCILIB is not available, fake functions */
#else

int
initMetadataLookupPlugin (void)
{
    return EXIT_SUCCESS;
}

boolean
catalogTableExists(char *table)
{
    return FALSE;
}

boolean
catalogViewExists(char *view)
{
	return FALSE;
}

List *
getAttributes (char *table)
{
    return NIL;
}

List *getAttributeNames (char *tableName)
{
    return NIL;
}

boolean
isAgg(char *table)
{
	return FALSE;
}

boolean
isWindowFunction(char *functionName)
{
    return FALSE;
}

char *
getTableDefinition(char *table) {
    return NULL;
}

void getTransactionSQLAndSCNs(char *xid, List **scns, List **sqls, List **sqlBinds) {
}

char *
getViewDefinition(char *view) {
    return NULL;
}

char *
executeStatement(char *statement)
{
	return NULL;
}

Node *
executeAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    return NULL;
}

int
oracleDatabaseConnectionOpen (void)
{

}


int
databaseConnectionClose ()
{
    return EXIT_SUCCESS;
}



#endif
