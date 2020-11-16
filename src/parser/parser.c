/*-----------------------------------------------------------------------------
 *
 * parser.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "parser/parser.h"
#include "parser/parser_hive.h"
#include "parser/parser_oracle.h"
#include "parser/parser_postgres.h"
#include "parser/parser_dl.h"
#include "model/node/nodetype.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"


// plugin
static ParserPlugin *plugin = NULL;

// function defs
static ParserPlugin *assembleOraclePlugin(void);
static ParserPlugin *assemblePostgresPlugin(void);
static ParserPlugin *assembleHivePlugin(void);
static ParserPlugin *assembleDLPlugin(void);

static ParserPluginType getPluginTypeFromString (char *type);

// wrapper interface
Node *
parseStream (FILE *stream)
{
    ASSERT(plugin);
    return plugin->parseStream(stream);
}

Node *
parseFromString (char *input)
{
    ASSERT(plugin);

    INFO_LOG("parse SQL:\n%s", input);
    return plugin->parseFromString(input);
}

// plugin management
void
chooseParserPlugin(ParserPluginType type)
{
    switch(type)
    {
        case PARSER_PLUGIN_ORACLE:
            plugin = assembleOraclePlugin();
            break;
        case PARSER_PLUGIN_POSTGRES:
            plugin = assemblePostgresPlugin();
            break;
        case PARSER_PLUGIN_HIVE:
            plugin = assembleHivePlugin();
            break;
        case PARSER_PLUGIN_DL:
            plugin = assembleDLPlugin();
            break;
    }
}

char *
getParserPluginNameFromString (char *name)
{
    chooseParserPluginFromString(name);
    return getParserPluginName();
}

char *
getParserPluginName (void)
{
    switch(plugin->type)
    {
        case PARSER_PLUGIN_ORACLE:
            return "Oracle SQL";
        case PARSER_PLUGIN_POSTGRES:
            return "Postgres SQL";
        case PARSER_PLUGIN_HIVE:
            return "HiveQL";
        case PARSER_PLUGIN_DL:
            return "Datalog";
    }
}

const char *
getParserLanguageHelp (void)
{
    return plugin->languageHelp;
}

const char *
getParserPluginLanguageHelp (char *lang)
{
    chooseParserPluginFromString(lang);
    return getParserLanguageHelp();
}

static ParserPlugin *
assembleOraclePlugin(void)
{
    ParserPlugin *p = NEW(ParserPlugin);

    p->type = PARSER_PLUGIN_ORACLE;
    p->parseStream = parseStreamOracle;
    p->parseFromString = parseFromStringOracle;
    p->languageHelp = languageHelpOracle();

    return p;
}

static ParserPlugin *
assemblePostgresPlugin(void)
{
    ParserPlugin *p = NEW(ParserPlugin);

    p->type = PARSER_PLUGIN_POSTGRES;
    p->parseStream = parseStreamPostgres;
    p->parseFromString = parseFromStringPostgres;
    p->languageHelp = "";

    return p;
}

static ParserPlugin *
assembleHivePlugin(void)
{
    ParserPlugin *p = NEW(ParserPlugin);

    //    p->parseStream = parseStreamHive;
    //    p->parseFromString = parseFromStringHive;
    FATAL_LOG("not implemented yet");

    return p;
}

static ParserPlugin *
assembleDLPlugin(void)
{
    ParserPlugin *p = NEW(ParserPlugin);

    p->type = PARSER_PLUGIN_DL;
    p->parseStream = parseStreamdl;
    p->parseFromString = parseFromStringdl;
    p->languageHelp = languageHelpDL();

    return p;
}

void
chooseParserPluginFromString(char *type)
{
    INFO_LOG("PLUGIN parser: <%s>", type ? type : "NULL");

    chooseParserPlugin(getPluginTypeFromString(type));
}

static ParserPluginType
getPluginTypeFromString (char *type)
{
    if (strpleq(type,"oracle"))
        return (PARSER_PLUGIN_ORACLE);
    else if (strpleq(type,"postgres"))
        return (PARSER_PLUGIN_POSTGRES);
    else if (strpleq(type,"hive"))
        return (PARSER_PLUGIN_HIVE);
    else if (strpleq(type,"dl"))
        return (PARSER_PLUGIN_DL);
    else
        FATAL_LOG("unkown parser plugin type: <%s>", type);
}
