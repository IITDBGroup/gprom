/*-------------------------------------------------------------------------
 *
 * translator.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"

#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"

#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "analysis_and_translate/translator_oracle.h"
#include "analysis_and_translate/translator_dl.h"
#include "analysis_and_translate/translator_sqlite.h"

#include "parser/parser.h"

// plugin
static TranslatorPlugin *plugin = NULL;

// function defs
//static Node *parseInternal (void);

static TranslatorPlugin *assembleOraclePlugin(void);
static TranslatorPlugin *assemblePostgresPlugin(void);
static TranslatorPlugin *assembleHivePlugin(void);
static TranslatorPlugin *assembleDLPlugin(void);
static TranslatorPlugin *assembleDummyPlugin(void);
static TranslatorPlugin *assembleSqlitePlugin(void);

static Node *echoNode (Node *in);
static QueryOperator *echoNodeAsQB (Node *in);

Node *
translateParse(Node *q)
{
    Node *result;

    ASSERT(plugin);

    NEW_AND_ACQUIRE_MEMCONTEXT("TRANSLATOR_CONTEXT");
    analyzeParseModel(q);
    result = plugin->translateParse(q);

    FREE_MEM_CONTEXT_AND_RETURN_COPY(Node,result);
}



QueryOperator *
translateQuery (Node *node)
{
    ASSERT(plugin);
    DEBUG_LOG("translate query <%s>", nodeToString(node));

    return plugin->translateQuery(node);
}

// plugin management
void
chooseTranslatorPlugin(TranslatorPluginType type)
{
    switch(type)
    {
        case TRANSLATOR_PLUGIN_ORACLE:
            plugin = assembleOraclePlugin();
            break;
        case TRANSLATOR_PLUGIN_POSTGRES:
            plugin = assemblePostgresPlugin();
            break;
        case TRANSLATOR_PLUGIN_HIVE:
            plugin = assembleHivePlugin();
            break;
        case TRANSLATOR_PLUGIN_DL:
            plugin = assembleDLPlugin();
            break;
        case TRANSLATOR_PLUGIN_DUMMY:
            plugin = assembleDummyPlugin();
            break;
        case TRANSLATOR_PLUGIN_SQLITE:
			plugin = assembleSqlitePlugin();
			break;
    }
}

static TranslatorPlugin *
assembleOraclePlugin(void)
{
    TranslatorPlugin *p = NEW(TranslatorPlugin);

    p->translateParse = translateParseOracle;
    p->translateQuery = translateQueryOracle;

    return p;
}

static TranslatorPlugin *
assemblePostgresPlugin(void)
{
    TranslatorPlugin *p = NEW(TranslatorPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

static TranslatorPlugin *
assembleHivePlugin(void)
{
    TranslatorPlugin *p = NEW(TranslatorPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

static TranslatorPlugin *
assembleDLPlugin(void)
{
    TranslatorPlugin *p = NEW(TranslatorPlugin);

    p->translateParse = translateParseDL;
    p->translateQuery = translateQueryDL;

    return p;
}

static TranslatorPlugin *
assembleDummyPlugin(void)
{
    TranslatorPlugin *p = NEW(TranslatorPlugin);

    p->translateParse = echoNode;
    p->translateQuery = echoNodeAsQB;

    return p;
}

static TranslatorPlugin *
assembleSqlitePlugin(void)
{
    TranslatorPlugin *p = NEW(TranslatorPlugin);

    p->translateParse = translateParseSqlite;
    p->translateQuery = translateQuerySqlite;

    return p;
}

static Node *
echoNode (Node *in)
{
    return in;
}

static QueryOperator *
echoNodeAsQB (Node *in)
{
    return NULL;
}


void
chooseTranslatorPluginFromString(char *type)
{
    INFO_LOG("PLUGIN translator: <%s>", type);

    if (streq(type,"oracle"))
        chooseTranslatorPlugin(TRANSLATOR_PLUGIN_ORACLE);
    else if (streq(type,"postgres"))
        chooseTranslatorPlugin(TRANSLATOR_PLUGIN_POSTGRES);
    else if (streq(type,"hive"))
        chooseTranslatorPlugin(TRANSLATOR_PLUGIN_HIVE);
    else if (streq(type,"dl"))
        chooseTranslatorPlugin(TRANSLATOR_PLUGIN_DL);
    else if (streq(type,"dummy"))
        chooseTranslatorPlugin(TRANSLATOR_PLUGIN_DUMMY);
    else if (streq(type,"sqlite"))
            chooseTranslatorPlugin(TRANSLATOR_PLUGIN_SQLITE);
    else
        FATAL_LOG("unkown translator plugin type: <%s>", type);
}
