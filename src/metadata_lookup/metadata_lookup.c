/*
 * metadata_lookup.c
 *
 *      Author: zephyr
 */

#include "common.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"

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
static List *tableBuffers=NULL;
static List *viewBuffers=NULL;
static boolean initialized = FALSE;

static int initConnection(void);
static boolean isConnected(void);
static void initAggList(void);
static void freeAggList(void);
static OCI_Resultset *executeStatement(char *statement);
static void handleError (OCI_Error *error);

static void addToTableBuffers(char *tableName, List *attrs);
static void addToViewBuffers(char *viewName, char *viewDef);
static List *searchTableBuffers(char *tableName);
static char *searchViewBuffers(char *viewName);
static void freeBuffers(void);

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
		{
			return t->attrs;
		}
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
		{
			return v->viewDefinition;
		}
	}
	return NULL;
}

static int
initConnection()
{
    assert(initialized);

    ACQUIRE_MEM_CONTEXT(context);

    StringInfo connectString = makeStringInfo();
	Options* options=getOptions();

	char* user=options->optionConnection->user;
	char* passwd=options->optionConnection->passwd;
	char* db=options->optionConnection->db;
	char *host=options->optionConnection->host;
	int port=options->optionConnection->port;
	appendStringInfo(connectString, ORACLE_TNS_CONNECTION_FORMAT, host, port,
	        db);

	conn = OCI_ConnectionCreate(connectString->data,user,passwd,
	        OCI_SESSION_DEFAULT);
	DEBUG_LOG("Try to connect to server <%s,%s,%s>... %s", connectString->data, user, passwd,
	        (conn != NULL) ? "SUCCESS" : "FAILURE");

	initAggList();

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
initMetadataLookupPlugin (void)
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

OCI_Connection *
getConnection()
{
    if(isConnected())
        return conn;
    return NULL;
}

boolean
catalogTableExists(char* tableName)
{
	if(NULL==tableName)
		return FALSE;
	if(conn==NULL)
		initConnection();
	if(isConnected())
		return (OCI_TypeInfoGet(conn,tableName,OCI_TIF_TABLE)==NULL) ? FALSE : TRUE;
	return FALSE;
}

boolean
catalogViewExists(char* viewName)
{
	if(NULL==viewName)
		return FALSE;
	if(conn==NULL)
		initConnection();
	if(isConnected())
		return (OCI_TypeInfoGet(conn,viewName,OCI_TIF_VIEW)==NULL) ? FALSE : TRUE;
	return FALSE;
}

List*
getAttributes(char *tableName)
{
	List *attrList=NIL;
	List *restul = NIL;

	ACQUIRE_MEM_CONTEXT(context);

	if(tableName==NULL)
	    RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
	if((attrList = searchTableBuffers(tableName)) != NIL)
	{
	    RELEASE_MEM_CONTEXT();
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
		return attrList;
	}
	ERROR_LOG("Not connected to database.");

	// copy result to callers memory context
	RELEASE_MEM_CONTEXT_AND_RETURN_COPY(List, NIL);
}

boolean
isAgg(char* functionName)
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

char *
getTableDefinition(char *tableName)
{
	StringInfo statement;
	char *result;

	ACQUIRE_MEM_CONTEXT(context);

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
		    RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
		}
	}
	FREE(statement);
	RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(NULL);
}

char *
getViewDefinition(char *viewName)
{
	char *def = NULL;
	StringInfo statement;

	ACQUIRE_MEM_CONTEXT(context);

	if((def = searchViewBuffers(viewName)) != NULL)
	{
	    RELEASE_MEM_CONTEXT();
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
	RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY (NULL);
}

static OCI_Resultset *
executeStatement(char *statement)
{
	if(statement == NULL)
		return NULL;
	if((conn = getConnection()) != NULL)
	{
		if(st == NULL)
			st = OCI_StatementCreate(conn);
		OCI_ReleaseResultsets(st);
		if(OCI_ExecuteStmt(st, statement))
		{
		    OCI_Resultset *rs = OCI_GetResultset(st);
			DEBUG_LOG("Statement: %s executed successfully.", statement);
			DEBUG_LOG("%d row fetched", OCI_GetRowCount(rs));
			return rs;
		}
		else
		{
			ERROR_LOG("Statement: %s failed.", statement);
		}
	}
	return NULL;
}


int
databaseConnectionClose()
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
		freeBuffers();
		OCI_Cleanup();//bugs exist here
		initialized = FALSE;
		FREE_AND_RELEASE_CUR_MEM_CONTEXT();
	}
	return EXIT_SUCCESS;
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

boolean
isAgg(char *table)
{
	return FALSE;
}

char *
getTableDefinition(char *table)
{
	return NULL;
}

char *
getViewDefinition(char *view)
{
	return NULL;
}

char *
executeStatement(char *statement)
{
	return NULL;
}

int
databaseConnectionClose ()
{
    return EXIT_SUCCESS;
}


#endif
