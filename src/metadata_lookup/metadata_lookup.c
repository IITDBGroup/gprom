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

/* If OCILIB is available then use it */
#if HAVE_OCILIB

/*
 * functions and variables for internal use
 */
static OCI_Connection* conn=NULL;
static OCI_TypeInfo* tInfo=NULL;
static MemContext* context=NULL;
static int initConnection();
static boolean isConnected();

static int
initConnection()
{
	NEW_AND_ACQUIRE_MEM_CONTEXT("metadataContext");
	context=getCurMemContext();
	Options* options=getOptions();
	char* user=options->optionConnection->user;
	char* passwd=options->optionConnection->passwd;
	char* db=options->optionConnection->db;

	if(!OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT))
	{
	    FATAL_LOG("Cannot connect to Oracle database"); //TODO print error type
		return EXIT_FAILURE;
	}

	conn = OCI_ConnectionCreate(db,user,passwd,OCI_SESSION_DEFAULT);
	return EXIT_SUCCESS;
}

static boolean
isConnected()
{
	if(OCI_isConnected(conn))
		return TRUE;
	else
	{
		FATAL_LOG("OCI connection lost");
		return FALSE;
	}
}

boolean
catalogTableExists(char* tableName)
{
	if(NULL==tableName)
		return FALSE;
	if(conn==NULL)
		initConnection();
	if(isConnected())
		return tInfo=OCI_TypeInfoGet(conn,tableName,OCI_TIF_TABLE)==NULL?FALSE:TRUE;
	return FALSE;
}

List*
getAttributes(char* tableName)
{
	if(tableName==NULL)
		return NIL;
	if(conn==NULL)
		initConnection();
	if(isConnected())
	{
		int i,n;
		n = OCI_TypeInfoGetColumnCount(tInfo);
		List* attrList=NIL;
		for(i = 1; i <= n; i++)
		{
			OCI_Column *col = OCI_TypeInfoGetColumn(tInfo, i);
			AttributeReference* a=createAttributeReference(OCI_GetColumnName(col));
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
