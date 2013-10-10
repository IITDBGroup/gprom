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
#if HAVE_LIBOCILIB && HAVE_LIBOCI

#define ORACLE_TNS_CONNECTION_FORMAT "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=%s)(PORT=%u)))(CONNECT_DATA=(SERVER=DEDICATED)(SID=%s)))"

/*
 * functions and variables for internal use
 */
static OCI_Connection* conn=NULL;
static OCI_TypeInfo* tInfo=NULL;
static OCI_Error* errHandler=NULL;
static MemContext* context=NULL;
static int initConnection();
static boolean isConnected();

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
	if(!OCI_Initialize(errHandler, NULL, OCI_ENV_DEFAULT))
	{
	    FATAL_LOG("Cannot initialize OICLIB: %s", OCI_ErrorGetString(errHandler)); //print error type
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
		FATAL_LOG("OCI connection lost: %s", OCI_ErrorGetString(errHandler));
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

List *
getAttributes (char *table)
{
    return NIL;
}

int
databaseConnectionClose ()
{
    return EXIT_SUCCESS;
}

#endif
