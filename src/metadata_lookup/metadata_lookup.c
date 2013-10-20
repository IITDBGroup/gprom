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

#define ORACLE_TNS_CONNECTION_FORMAT "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=%s)(PORT=%u)))(CONNECT_DATA=(SERVER=DEDICATED)(SID=%s)))"

/*
 * functions and variables for internal use
 */
static OCI_Connection* conn=NULL;
static OCI_Statement *st = NULL;
static OCI_TypeInfo* tInfo=NULL;
static OCI_Error* errorCache=NULL;
static MemContext* context=NULL;
static char** aggList=NULL;

static int initConnection(void);
static boolean isConnected(void);
static void initAggList(void);
static void freeAggList(void);
static OCI_Resultset *executeStatement(char *statement);
static void handleError (OCI_Error *error);

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
	aggList[AGG_MAX] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_MIN] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_AVG] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_COUNT] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_SUM] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_FIRST] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_LAST] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_CORR] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_COVAR_POP] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_COVAR_SAMP] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_GROUPING] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_REGR] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_STDDEV] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_STDDEV_POP] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_STDEEV_SAMP] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_VAR_POP] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_VAR_SAMP] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_VARIANCE] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);
	aggList[AGG_XMLAGG] = CNEW(char,AGG_FUNCTION_NAME_MAXSIZE);

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
	int i;
	for(i = 0; i < AGG_FUNCTION_COUNT; i++)
	{
		FREE(aggList[i]);
	}
	FREE(aggList);
}

static int
initConnection()
{
    NEW_AND_ACQUIRE_MEMCONTEXT("metadataContext");
    context=getCurMemContext();

    StringInfo connectString = makeStringInfo();
	Options* options=getOptions();

	char* user=options->optionConnection->user;
	char* passwd=options->optionConnection->passwd;
	char* db=options->optionConnection->db;
	char *host=options->optionConnection->host;
	int port=options->optionConnection->port;
	appendStringInfo(connectString, ORACLE_TNS_CONNECTION_FORMAT, host, port,
	        db);
	if(!OCI_Initialize(handleError, NULL, OCI_ENV_DEFAULT))
	{
	    FATAL_LOG("Cannot initialize OICLIB: %s", OCI_ErrorGetString(errorCache)); //print error type
		return EXIT_FAILURE;
	}
	DEBUG_LOG("Initialized OCILIB");

	conn = OCI_ConnectionCreate(connectString->data,user,passwd,
	        OCI_SESSION_DEFAULT);
	DEBUG_LOG("Try to connect to server <%s,%s,%s>... %s", connectString->data, user, passwd,
	        (conn != NULL) ? "SUCCESS" : "FAILURE");

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
		return (OCI_TypeInfoGet(conn,tableName,OCI_TIF_TABLE)==NULL)? FALSE : TRUE;
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
		return (OCI_TypeInfoGet(conn,viewName,OCI_TIF_VIEW)==NULL)? FALSE : TRUE;
	return FALSE;
}

List*
getAttributes(char *tableName)
{
	if(tableName==NULL)
		return NIL;
	if(conn==NULL)
		initConnection();
	if(isConnected())
	{
		int i,n;
		tInfo = OCI_TypeInfoGet(conn,tableName,OCI_TIF_TABLE);
		n = OCI_TypeInfoGetColumnCount(tInfo);
		List* attrList=NIL;
		for(i = 1; i <= n; i++)
		{
			OCI_Column *col = OCI_TypeInfoGetColumn(tInfo, i);
			AttributeReference *a = createAttributeReference((char *) OCI_GetColumnName(col));
			attrList=appendToTailOfList(attrList,a);
		}
		return attrList;
	}
	ERROR_LOG("Not connected to database.");
	return NIL;
}

boolean
isAgg(char* functionName)
{
	if(functionName == NULL)
		return FALSE;
	if(aggList == NULL)
		initAggList();

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
	char statement[256];
	char *statement1 = "select DBMS_METADATA.GET_DDL('TABLE', '";
	char *statement2 = "') from DUAL";
	strcpy(statement, statement1);
	strcat(statement, tableName);
	strcat(statement, statement2);

	OCI_Resultset *rs = executeStatement(statement);
	if(rs != NULL)
	{
		while(OCI_FetchNext(rs))
		{
			return (char *)OCI_GetString(rs, 1);
		}
	}
	return NULL;
}

char *
getViewDefinition(char *viewName)
{
	char statement[256];
	char *statement1 = "select text from user_views where view_name = '";
	char *statement2 = "'";
	strcpy(statement, statement1);
	strcat(statement, viewName);
	strcat(statement, statement2);

	OCI_Resultset *rs = executeStatement(statement);
	if(rs != NULL)
	{
		while(OCI_FetchNext(rs))
		{
			return (char *)OCI_GetString(rs, 1);
		}
	}
	return NULL;
}

OCI_Resultset *
executeStatement(char *statement)
{
	if(statement == NULL)
		return NULL;
	if((conn = getConnection()) != NULL)
	{
		if(st == NULL)
			st = OCI_StatementCreate(conn);
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
		freeAggList();
		FREE_MEM_CONTEXT(context);
		OCI_Cleanup();
	}
	return EXIT_SUCCESS;
}

/* OCILIB is not available, fake functions */
#else

boolean
catalogTableExists(char *table)
{
    return FALSE;
}

bolean
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
