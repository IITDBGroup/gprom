/*-----------------------------------------------------------------------------
 *
 * parser.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "common.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"

/* types of supported plugins */
typedef enum ParserPluginType
{
    PARSER_PLUGIN_ORACLE,
    PARSER_PLUGIN_POSTGRES,
    PARSER_PLUGIN_HIVE,
    PARSER_PLUGIN_DL
} ParserPluginType;

/* plugin definition */
typedef struct ParserPlugin
{
    ParserPluginType type;
    const char *languageHelp;
    /* functional interface */
    Node *(*parseStream) (FILE *file);
    Node *(*parseFromString) (char *input);
	Node *(*parseExprFromString) (char *input);
} ParserPlugin;

// plugin management
extern void chooseParserPlugin(ParserPluginType type);
extern char *getParserPluginName (void);
extern char *getParserPluginNameFromString (char *name);
extern const char *getParserLanguageHelp (void);
extern const char *getParserPluginLanguageHelp (char *lang);
extern void chooseParserPluginFromString(char *type);

// parser interface wrapper
extern Node *parseStream (FILE *file);
extern Node *parseFromString (char *input);
extern Node *parseExprFromString (char *input);

#endif /* PARSER_H_ */
