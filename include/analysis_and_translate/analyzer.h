/*-----------------------------------------------------------------------------
 *
 * analyzer.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZER_H_
#define INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZER_H_


#include "common.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"

/* types of supported plugins */
typedef enum AnalyzerPluginType
{
    ANALYZER_PLUGIN_ORACLE,
    ANALYZER_PLUGIN_POSTGRES,
    ANALYZER_PLUGIN_HIVE,
    ANALYZER_PLUGIN_DL,
	ANALYZER_PLUGIN_SQLITE
} AnalyzerPluginType;

/* plugin definition */
typedef struct AnalyzerPlugin
{
    AnalyzerPluginType type;

    /* functional interface */
    Node *(*analyzeParserModel) (Node *parserModel);
} AnalyzerPlugin;

// plugin management
extern void chooseAnalyzerPlugin(AnalyzerPluginType type);
extern void chooseAnalyzerPluginFromString(char *type);

// analyzer interface wrapper
extern Node *analyzeParseModel (Node *parseModel);

#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZER_H_ */
