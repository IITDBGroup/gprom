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

    /* functional interface */
    Node *(*parseStream) (FILE *file);
    Node *(*parseFromString) (char *input);

} ParserPlugin;

// plugin management
extern void chooseParserPlugin(ParserPluginType type);
extern void chooseParserPluginFromString(char *type);

// parser interface wrapper
extern Node *parseStream (FILE *file);
extern Node *parseFromString (char *input);

#endif /* PARSER_H_ */
