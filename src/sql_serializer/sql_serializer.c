/*-----------------------------------------------------------------------------
 *
 * sql_serializer.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"

#include "log/logger.h"

#include "sql_serializer/sql_serializer.h"
#include "sql_serializer/sql_serializer_oracle.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"

// plugin
static SqlserializerPlugin *plugin = NULL;

// function defs
static SqlserializerPlugin *assembleOraclePlugin(void);
static SqlserializerPlugin *assemblePostgresPlugin(void);
static SqlserializerPlugin *assembleHivePlugin(void);
static SqlserializerPlugin *assembleDLPlugin(void);

// wrapper interface
char *
serializeOperatorModel(Node *q)
{
    ASSERT(plugin);
    return plugin->serializeOperatorModel(q);
}

char *
serializeQuery(QueryOperator *q)
{
    StringInfo str;
    StringInfo viewDef;
    char *result;

    NEW_AND_ACQUIRE_MEMCONTEXT("SQL_SERIALIZER");

    str = makeStringInfo();
    viewDef = makeStringInfo();

    // initialize basic structures and then call the worker
    viewMap = NULL;
    viewNameCounter = 0;

    // simulate non Oracle conformant data types and expressions (boolean)
    makeDTOracleConformant(q);

    // call main entry point for translation
    serializeQueryOperator (q, str);

    /*
     *  prepend the temporary view definition to create something like
     *      WITH a AS (q1), b AS (q2) ... SELECT ...
     */
    if (HASH_COUNT(viewMap) > 0)
    {
        appendStringInfoString(viewDef, "WITH ");

        // loop through temporary views we have defined
        for(TemporaryViewMap *view = viewMap; view != NULL; view = view->hh.next)
        {
            appendStringInfoString(viewDef, view->viewDefinition);
            if (view->hh.next != NULL)
                appendStringInfoString(viewDef, ",\n\n");
        }

        // prepend to query translation
        DEBUG_LOG("views are:\n\n%s", viewDef->data);
        result = CONCAT_STRINGS(viewDef->data, str->data);
    }
    else
        result = str->data;

    // copy result to callers memory context and clean up
    FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
}

/*
 * Replace boolean data types
 */
static void
makeDTOracleConformant(QueryOperator *q)
{
    replaceNonOracleDTs((Node *) q, NULL);
}

static boolean
replaceNonOracleDTs (Node *node, void *context)
{
    if (node == NULL)
        return TRUE;

    //TODO keep context
    //TODO take care of boolean expressions that are used where Oracle expects
    if (isA(node,Constant))
    {
        Constant *c = (Constant *) node;

        if (c->constType == DT_BOOL)
        {
            c->constType = DT_INT;
            if (!c->isNull)
            {
                boolean val = BOOL_VALUE(c);
                c->value = NEW(int);
                INT_VALUE(c) = val ? 1 : 0;
            }
        }
    }

    return visit(node, replaceNonOracleDTs, context);
}

/*
 * Main entry point for serialization.
 */
static List *
serializeQueryOperator (QueryOperator *q, StringInfo str)
{
    // operator with multiple parents
    if (LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q,PROP_MATERIALIZE))
        return createTempView (q, str);
    else if (isA(q, SetOperator))
        return serializeSetOperator(q, str);
    else
        return serializeQueryBlock(q, str);
}

char *
quoteIdentifier (char *ident)
{
    ASSERT(plugin);
    return plugin->quoteIdentifier(ident);
}

// plugin management
void
chooseSqlserializerPlugin(SqlserializerPluginType type)
{
    switch(type)
    {
        case SQLSERIALIZER_PLUGIN_ORACLE:
            plugin = assembleOraclePlugin();
            break;
        case SQLSERIALIZER_PLUGIN_POSTGRES:
            plugin = assemblePostgresPlugin();
            break;
        case SQLSERIALIZER_PLUGIN_HIVE:
            plugin = assembleHivePlugin();
            break;
        case SQLSERIALIZER_PLUGIN_DL:
            plugin = assembleDLPlugin();
            break;
    }
}

static SqlserializerPlugin *
assembleOraclePlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->serializeOperatorModel = serializeOperatorModelOracle;
    p->serializeQuery = serializeQueryOracle;
    p->quoteIdentifier = quoteIdentifierOracle;

    return p;
}

static SqlserializerPlugin *
assemblePostgresPlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

static SqlserializerPlugin *
assembleHivePlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

static SqlserializerPlugin *
assembleDLPlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

void
chooseSqlserializerPluginFromString(char *type)
{
    INFO_LOG("PLUGIN sqlserializer: <%s>", type);

    if (streq(type,"oracle"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_ORACLE);
    else if (streq(type,"postgres"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_POSTGRES);
    else if (streq(type,"hive"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_HIVE);
    else if (streq(type,"dl"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_DL);
    else
        FATAL_LOG("unkown sqlserializer plugin type: <%s>", type);
}
