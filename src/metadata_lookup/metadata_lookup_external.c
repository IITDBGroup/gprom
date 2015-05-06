/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_java.c
 *	    - metadata lookup plugin with external implementation using libgprom interface
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

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "libgprom/libgprom.h"
#include "parser/parser.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_external.h"


// wrapper methods
static int externalInitMetadataLookupPlugin (void);
static int externalShutdownMetadataLookupPlugin (void);
static int externalDatabaseConnectionOpen (void);
static int externalDatabaseConnectionClose();
static boolean externalIsInitialized (void);

static boolean externalCatalogTableExists (char * tableName);
static boolean externalCatalogViewExists (char * viewName);
static List *externalGetAttributes (char *tableName);
static List *externalGetAttributeNames (char *tableName);
extern Node *externalGetAttributeDefaultVal (char *schema, char *tableName, char *attrName);
static boolean externalIsAgg(char *functionName);
static boolean externalIsWindowFunction(char *functionName);
static DataType externalGetFuncReturnType (char *fName, List *argTypes);
static DataType externalGetOpReturnType (char *oName, List *argTypes);
static char *externalGetTableDefinition(char *tableName);
static char *externalGetViewDefinition(char *viewName);
static List *externalGetKeyInformation (char *tableName);


static void externalGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
static Node *externalExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
static long externalGetCommitScn (char *tableName, long maxScn, char *xid);
static Relation *externalGenExecQuery (char *query);

#define EXTERNAL_PLUGIN GProMMetadataLookupPlugin *extP = (GProMMetadataLookupPlugin *) activePlugin->cache->cacheHook

// create a plugin
MetadataLookupPlugin *
assembleExternalMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin)
{
    MetadataLookupPlugin *p = NEW(MetadataLookupPlugin);

    p->type = METADATA_LOOKUP_PLUGIN_EXTERNAL;

    p->initMetadataLookupPlugin = externalInitMetadataLookupPlugin;
    p->databaseConnectionOpen = externalDatabaseConnectionOpen;
    p->databaseConnectionClose = externalDatabaseConnectionClose;
    p->shutdownMetadataLookupPlugin = externalShutdownMetadataLookupPlugin;
    p->isInitialized = externalIsInitialized;
    p->catalogTableExists = externalCatalogTableExists;
    p->catalogViewExists = externalCatalogViewExists;
    p->getAttributes = externalGetAttributes;
    p->getAttributeNames = externalGetAttributeNames;
    p->getAttributeDefaultVal = externalGetAttributeDefaultVal;
    p->isAgg = externalIsAgg;
    p->isWindowFunction = externalIsWindowFunction;
    p->getTableDefinition = externalGetTableDefinition;
    p->getViewDefinition = externalGetViewDefinition;
    p->getKeyInformation = externalGetKeyInformation;
    p->getOpReturnType = externalGetOpReturnType;
    p->getFuncReturnType = externalGetFuncReturnType;
    p->getTransactionSQLAndSCNs = externalGetTransactionSQLAndSCNs;
    p->executeAsTransactionAndGetXID = externalExecuteAsTransactionAndGetXID;
    p->getCommitScn = externalGetCommitScn;
    p->executeQuery = externalGenExecQuery;

    p->cache = createCache();
    p->cache->cacheHook = plugin;

    return p;
}

static int
externalInitMetadataLookupPlugin (void)
{
    EXTERNAL_PLUGIN;
    return extP->isInitialized();
}

static int
externalShutdownMetadataLookupPlugin (void)
{
    EXTERNAL_PLUGIN;
    return extP->shutdownMetadataLookupPlugin();
}

static int
externalDatabaseConnectionOpen (void)
{
    EXTERNAL_PLUGIN;
    return extP->databaseConnectionOpen();
}

static int
externalDatabaseConnectionClose()
{
    EXTERNAL_PLUGIN;
    return extP->databaseConnectionClose();
}

static boolean
externalIsInitialized (void)
{
    EXTERNAL_PLUGIN;
    return extP->isInitialized();
}

static boolean
externalCatalogTableExists (char * tableName)
{
    EXTERNAL_PLUGIN;
    return extP->catalogTableExists(tableName);
}

static boolean
externalCatalogViewExists (char * viewName)
{
    EXTERNAL_PLUGIN;
    return extP->catalogViewExists(viewName);
}

static List *
externalGetAttributes (char *tableName)
{
    EXTERNAL_PLUGIN;
    List *result = NULL;
    char **atts = NULL;
    char **dts = NULL;
    int numAtts = 0;

    extP->getAttributes(tableName, &atts, &dts, &numAtts);
    for(int i = 0; i < numAtts; i++)
    {
        AttributeDef *d = createAttributeDef(strdup(atts[i]),
                stringToDataType(dts[i]));
        result = appendToTailOfList(result, d);
    }

    return result;
}

static List *
externalGetAttributeNames (char *tableName)
{
    EXTERNAL_PLUGIN;

    List *result = NULL;
    char **atts = NULL;
    int numAtts = 0;

    extP->getAttributeNames(tableName, &atts, &numAtts);
    for(int i = 0; i < numAtts; i++)
    {
        result = appendToTailOfList(result, strdup(atts[i]));
    }

    return result;
}

extern Node *
externalGetAttributeDefaultVal (char *schema, char *tableName, char *attrName)
{
    EXTERNAL_PLUGIN;
    Node *result = NULL;
    char *expr = extP->getAttributeDefaultVal(schema, tableName, attrName);
    // parse expression
    if (expr != NULL)
        result = parseFromString(expr);

    return result;
}

static boolean
externalIsAgg(char *functionName)
{
    EXTERNAL_PLUGIN;
    return extP->isAgg(functionName);
}

static boolean
externalIsWindowFunction(char *functionName)
{
    EXTERNAL_PLUGIN;
    return extP->isWindowFunction(functionName);
}

static DataType
externalGetFuncReturnType (char *fName, List *argTypes)
{
    EXTERNAL_PLUGIN;
    char ** args;
    int numArgs;
    int i;

    numArgs = LIST_LENGTH(argTypes);
    args = MALLOC(sizeof(char *) * numArgs);

    i = 0;
    FOREACH_INT(d,argTypes)
        args[i++] = DataTypeToString(d);

    return stringToDataType(extP->getFuncReturnType(fName, args, numArgs));
}

static DataType
externalGetOpReturnType (char *oName, List *argTypes)
{
    EXTERNAL_PLUGIN;
    char **args;
    int numArgs;
    int i;

    numArgs = LIST_LENGTH(argTypes);
    args = MALLOC(sizeof(char *) * numArgs);

    i = 0;
    FOREACH_INT(d,argTypes)
        args[i++] = DataTypeToString(d);
//TODO leaking
    return stringToDataType(extP->getFuncReturnType(oName, args, numArgs));
}

static char *
externalGetTableDefinition(char *tableName)
{
    EXTERNAL_PLUGIN;

    return extP->getTableDefinition(tableName);
}

static char *
externalGetViewDefinition(char *viewName)
{
    EXTERNAL_PLUGIN;

    return extP->getViewDefinition(viewName);
}

static List *
externalGetKeyInformation (char *tableName)
{
    EXTERNAL_PLUGIN;
    List *result = NIL;
    char **exResult = NULL;

    exResult = extP->getKeyInformation(tableName);

    return result;
}

static void externalGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn)
{
    //TODO
}

static Node *
externalExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel)
{
    //TODO
    return NULL;
}

static long
externalGetCommitScn (char *tableName, long maxScn, char *xid)
{
    //TODO
    return 0;
}

static Relation *
externalGenExecQuery (char *query)
{
    //TODO
    return NULL;
}
