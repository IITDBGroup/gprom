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
#include "utility/string_utils.h"

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
static DataType externalGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists);
static DataType externalGetOpReturnType (char *oName, List *argTypes, boolean *opExists);
static char *externalGetTableDefinition(char *tableName);
static char *externalGetViewDefinition(char *viewName);
static List *externalGetKeyInformation (char *tableName);


static void externalGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
static Node *externalExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
static gprom_long_t externalGetCommitScn (char *tableName, gprom_long_t maxScn, char *xid);
static Relation *externalGenExecQuery (char *query);
static void externalGenExecQueryIgnoreQuery (char *query);
static int externalGetCostEstimation (char *query);
static char *externalDataTypeToSQL (DataType dt);

#define EXTERNAL_PLUGIN GProMMetadataLookupPlugin *extP = (GProMMetadataLookupPlugin *) activePlugin->cache->cacheHook
#define COPY_STRING(name) char *name ## Copy = strdup(name)
#define ARG(name) name ## Copy

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
    p->executeQueryIgnoreResult = externalGenExecQueryIgnoreQuery;
    p->getCostEstimation = externalGetCostEstimation;
    p->dataTypeToSQL = externalDataTypeToSQL;

    p->cache = createCache();
    p->cache->cacheHook = plugin;

    return p;
}

static int
externalInitMetadataLookupPlugin (void)
{
    if (activePlugin == NULL || activePlugin->cache == NULL)
        return EXIT_FAILURE;
    if (activePlugin->type != METADATA_LOOKUP_PLUGIN_EXTERNAL)
        return EXIT_SUCCESS;
    EXTERNAL_PLUGIN;
    return extP->isInitialized();
}

static int
externalShutdownMetadataLookupPlugin (void)
{
    if (activePlugin == NULL || activePlugin->cache == NULL)
        return EXIT_FAILURE;
    if (activePlugin->type != METADATA_LOOKUP_PLUGIN_EXTERNAL)
        return EXIT_SUCCESS;
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
    if (activePlugin == NULL || activePlugin->cache == NULL)
        return FALSE;
    if (activePlugin->type != METADATA_LOOKUP_PLUGIN_EXTERNAL)
        return FALSE;
    EXTERNAL_PLUGIN;
    return extP->isInitialized();
}

static boolean
externalCatalogTableExists (char *tableName)
{
    EXTERNAL_PLUGIN;
    COPY_STRING(tableName);
    boolean result = extP->catalogTableExists(ARG(tableName));
//    DEBUG_LOG("return value of table exists: %u", result);
    return result;
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
    COPY_STRING(tableName);
    List *result = NULL;
    List *attrNames = NIL;
    List *dts = NIL;
    char *dtString;

    attrNames = externalGetAttributeNames(ARG(tableName));
    dtString = extP->getDataTypes(ARG(tableName));
    dts = splitString(dtString,",");

    FORBOTH(char,a,dt,attrNames,dts)
    {
        AttributeDef *d = createAttributeDef(strdup(a),
                stringToDataType(dt));
        result = appendToTailOfList(result, d);
    }

    return result;
}

static List *
externalGetAttributeNames (char *tableName)
{
    EXTERNAL_PLUGIN;
    COPY_STRING(tableName);
    List *result = NULL;
    char *attList;

    attList = extP->getAttributeNames(ARG(tableName));
    result = splitString(attList,",");

    return result;
}

extern Node *
externalGetAttributeDefaultVal (char *schema, char *tableName, char *attrName)
{
    EXTERNAL_PLUGIN;
    Node *result = NULL;
    COPY_STRING(schema);
    COPY_STRING(tableName);
    COPY_STRING(attrName);

    char *expr = extP->getAttributeDefaultVal(ARG(schema), ARG(tableName), ARG(attrName));
    // parse expression
    if (expr != NULL)
        result = parseFromString(expr);

    return result;
}

static boolean
externalIsAgg(char *functionName)
{
    EXTERNAL_PLUGIN;
    COPY_STRING(functionName);
    return extP->isAgg(ARG(functionName));
}

static boolean
externalIsWindowFunction(char *functionName)
{
    EXTERNAL_PLUGIN;
    COPY_STRING(functionName);
    return extP->isWindowFunction(ARG(functionName));
}

static DataType
externalGetFuncReturnType (char *fName, List *argTypes, boolean *funcExists)
{
    EXTERNAL_PLUGIN;
    char ** args;
    int numArgs;
    int i;
    COPY_STRING(fName);

    numArgs = LIST_LENGTH(argTypes);
    args = MALLOC(sizeof(char *) * numArgs);

    i = 0;
    FOREACH_INT(d,argTypes)
        args[i++] = DataTypeToString(d);

    return stringToDataType(extP->getFuncReturnType(ARG(fName), args, numArgs));
}

static DataType
externalGetOpReturnType (char *oName, List *argTypes, boolean *opExists)
{
    EXTERNAL_PLUGIN;
    char **args;
    int numArgs;
    int i;
    COPY_STRING(oName);

    numArgs = LIST_LENGTH(argTypes);
    args = MALLOC(sizeof(char *) * numArgs);

    i = 0;
    FOREACH_INT(d,argTypes)
        args[i++] = DataTypeToString(d);
//TODO leaking
    return stringToDataType(extP->getFuncReturnType(ARG(oName), args, numArgs));
}

static char *
externalGetTableDefinition(char *tableName)
{
    EXTERNAL_PLUGIN;
    COPY_STRING(tableName);

    return extP->getTableDefinition(ARG(tableName));
}

static char *
externalGetViewDefinition(char *viewName)
{
    EXTERNAL_PLUGIN;
    COPY_STRING(viewName);

    return extP->getViewDefinition(ARG(viewName));
}

static List *
externalGetKeyInformation (char *tableName)
{
    EXTERNAL_PLUGIN;
    List *result = NIL;
    char *exResult = NULL;
    List *atts;
    COPY_STRING(tableName);

    exResult = extP->getKeyInformation(ARG(tableName));
    DEBUG_LOG("keys from external are: %s", exResult);
    atts = splitString(exResult, ",");
    if (LIST_LENGTH(atts) > 0)
        result = singleton(makeStrSetFromList(atts));
    else
        result = NIL;

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

static gprom_long_t
externalGetCommitScn (char *tableName, gprom_long_t maxScn, char *xid)
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

static void
externalGenExecQueryIgnoreQuery (char *query)
{
    //TODO
}

static int
externalGetCostEstimation (char *query)
{
    EXTERNAL_PLUGIN;
    return extP->getCostEstimation(query);
}

static char *
externalDataTypeToSQL (DataType dt)
{
    EXTERNAL_PLUGIN;
    return "VARCHAR";
    char *dtString = DataTypeToString(dt);
    return extP->dataTypeToSQL(dtString);
}
