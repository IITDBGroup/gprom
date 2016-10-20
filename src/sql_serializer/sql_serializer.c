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
#include "sql_serializer/sql_serializer_dl.h"
#include "sql_serializer/sql_serializer_sqlite.h"
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
static SqlserializerPlugin *assembleSQLitePlugin(void);

// wrapper interface
char *
serializeOperatorModel(Node *q)
{
    char *result;
    ASSERT(plugin);
    NEW_AND_ACQUIRE_MEMCONTEXT("SQL_SERIALIZER_CONTEXT");
    result = plugin->serializeOperatorModel(q);
    FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
}

char *
serializeQuery(QueryOperator *q)
{
    char *result;
    ASSERT(plugin);
    NEW_AND_ACQUIRE_MEMCONTEXT("SQL_SERIALIZER_CONTEXT");
    result = plugin->serializeQuery(q);
    FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
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
        case SQLSERIALIZER_PLUGIN_SQLITE:
            plugin = assembleSQLitePlugin();
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

    p->serializeOperatorModel = serializeOperatorModelDL;
    p->serializeQuery = serializeQueryDL;
    p->quoteIdentifier = quoteIdentifierDL;

    return p;
}

static SqlserializerPlugin *
assembleSQLitePlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->serializeOperatorModel = serializeOperatorModelSQLite;
    p->serializeQuery = serializeQuerySQLite;
    p->quoteIdentifier = quoteIdentifierSQLite;

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
    else if (streq(type,"sqlite"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_SQLITE);
    else
        FATAL_LOG("unkown sqlserializer plugin type: <%s>", type);
}
