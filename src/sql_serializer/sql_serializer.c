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
#include "sql_serializer/sql_serializer_lb.h"
#include "sql_serializer/sql_serializer_sqlite.h"
#include "sql_serializer/sql_serializer_duckdb.h"
#include "sql_serializer/sql_serializer_postgres.h"
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
static SqlserializerPlugin *assembleLBPlugin(void);
static SqlserializerPlugin *assembleSQLitePlugin(void);
static SqlserializerPlugin *assembleDuckDBPlugin(void);

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
        case SQLSERIALIZER_PLUGIN_LB:
            plugin = assembleLBPlugin();
            break;
        case SQLSERIALIZER_PLUGIN_SQLITE:
            plugin = assembleSQLitePlugin();
        case SQLSERIALIZER_PLUGIN_DUCKDB:
            plugin = assembleDuckDBPlugin();
            break;
    }
}

static SqlserializerPlugin *
assembleOraclePlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->type = SQLSERIALIZER_PLUGIN_ORACLE;
    p->serializeOperatorModel = serializeOperatorModelOracle;
    p->serializeQuery = serializeQueryOracle;
    p->quoteIdentifier = quoteIdentifierOracle;

    return p;
}

static SqlserializerPlugin *
assemblePostgresPlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->type = SQLSERIALIZER_PLUGIN_POSTGRES;
    p->serializeOperatorModel = serializeOperatorModelPostgres;
    p->serializeQuery = serializeQueryPostgres;
    p->quoteIdentifier = quoteIdentifierPostgres;

    return p;
}

static SqlserializerPlugin *
assembleHivePlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->type = SQLSERIALIZER_PLUGIN_HIVE;
    FATAL_LOG("not implemented yet");

    return p;
}

static SqlserializerPlugin *
assembleDLPlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->type = SQLSERIALIZER_PLUGIN_DL;
    p->serializeOperatorModel = serializeOperatorModelDL;
    p->serializeQuery = serializeQueryDL;
    p->quoteIdentifier = quoteIdentifierDL;

    return p;
}

static SqlserializerPlugin *
assembleLBPlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->serializeOperatorModel = serializeOperatorModelLB;
    p->serializeQuery = serializeQueryLB;
    p->quoteIdentifier = quoteIdentifierLB;

    return p;
}

static SqlserializerPlugin *
assembleSQLitePlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    p->type = SQLSERIALIZER_PLUGIN_SQLITE;
    p->serializeOperatorModel = serializeOperatorModelSQLite;
    p->serializeQuery = serializeQuerySQLite;
    p->quoteIdentifier = quoteIdentifierSQLite;

    return p;
}

static SqlserializerPlugin *
assembleDuckDBPlugin(void)
{
    SqlserializerPlugin *p = NEW(SqlserializerPlugin);

    // p->type = SQLSERIALIZER_PLUGIN_DUCKDB;
    // p->serializeOperatorModel = serializeOperatorModelDuckDB;
    // p->serializeQuery = serializeQueryDuckDB;
    // p->quoteIdentifier = quoteIdentifierDuckDB;

    p->type = SQLSERIALIZER_PLUGIN_SQLITE;
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
    else if (streq(type,"lb"))
            chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_LB);
    else if (streq(type,"sqlite"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_SQLITE);
    else if (streq(type,"duckdb"))
        chooseSqlserializerPlugin(SQLSERIALIZER_PLUGIN_DUCKDB);
    else
        FATAL_LOG("unkown sqlserializer plugin type: <%s>", type);
}

SqlserializerPluginType
getActiveSqlserializerPlugin(void)
{
    if (plugin == NULL)
        FATAL_LOG("no sql serializer plugin selected yet");

    return plugin->type;
}
