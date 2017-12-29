/*-----------------------------------------------------------------------------
 *
 * analyzer.c
 *			 : Main entry point for analyzer component. Calls selected analyzer plugin
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
#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/analyze_oracle.h"
#include "analysis_and_translate/analyze_dl.h"
#include "analysis_and_translate/analyze_sqlite.h"

// plugin
static AnalyzerPlugin *plugin = NULL;

// function defs
static AnalyzerPlugin *assembleOraclePlugin(void);
static AnalyzerPlugin *assemblePostgresPlugin(void);
static AnalyzerPlugin *assembleHivePlugin(void);
static AnalyzerPlugin *assembleDLPlugin(void);
static AnalyzerPlugin *assembleSqlitePlugin(void);


// wrapper interface
Node *
analyzeParseModel (Node *parseModel)
{
    ASSERT(plugin);
    return plugin->analyzeParserModel(parseModel);
}

// plugin management
void
chooseAnalyzerPlugin(AnalyzerPluginType type)
{
    switch(type)
    {
        case ANALYZER_PLUGIN_ORACLE:
            plugin = assembleOraclePlugin();
            break;
        case ANALYZER_PLUGIN_POSTGRES:
            plugin = assemblePostgresPlugin();
            break;
        case ANALYZER_PLUGIN_HIVE:
            plugin = assembleHivePlugin();
            break;
        case ANALYZER_PLUGIN_DL:
            plugin = assembleDLPlugin();
            break;
        case ANALYZER_PLUGIN_SQLITE:
			plugin = assembleSqlitePlugin();
			break;

    }
}

static AnalyzerPlugin *
assembleOraclePlugin(void)
{
    AnalyzerPlugin *p = NEW(AnalyzerPlugin);

    p->analyzeParserModel = analyzeOracleModel;

    return p;
}

static AnalyzerPlugin *
assemblePostgresPlugin(void)
{
    AnalyzerPlugin *p = NEW(AnalyzerPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

static AnalyzerPlugin *
assembleHivePlugin(void)
{
    AnalyzerPlugin *p = NEW(AnalyzerPlugin);

    FATAL_LOG("not implemented yet");

    return p;
}

static AnalyzerPlugin *
assembleDLPlugin(void)
{
    AnalyzerPlugin *p = NEW(AnalyzerPlugin);

    p->analyzeParserModel = analyzeDLModel;

    return p;
}

static AnalyzerPlugin *
assembleSqlitePlugin(void)
{
    AnalyzerPlugin *p = NEW(AnalyzerPlugin);

    p->analyzeParserModel = analyzeSqliteModel;

    return p;
}

void
chooseAnalyzerPluginFromString(char *type)
{
    INFO_LOG("PLUGIN analyzer: <%s>", type);

    if (streq(type,"oracle"))
        chooseAnalyzerPlugin(ANALYZER_PLUGIN_ORACLE);
    else if (streq(type,"postgres"))
        chooseAnalyzerPlugin(ANALYZER_PLUGIN_POSTGRES);
    else if (streq(type,"hive"))
        chooseAnalyzerPlugin(ANALYZER_PLUGIN_HIVE);
    else if (streq(type,"dl"))
        chooseAnalyzerPlugin(ANALYZER_PLUGIN_DL);
    else if (streq(type,"sqlite"))
		chooseAnalyzerPlugin(ANALYZER_PLUGIN_SQLITE);
    else
        FATAL_LOG("unkown analyzer plugin type: <%s>", type);
}
