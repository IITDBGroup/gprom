/*-----------------------------------------------------------------------------
 *
 * translator.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

/* types of supported plugins */
typedef enum TranslatorPluginType
{
    TRANSLATOR_PLUGIN_ORACLE,
    TRANSLATOR_PLUGIN_POSTGRES,
    TRANSLATOR_PLUGIN_HIVE,
    TRANSLATOR_PLUGIN_DL,
    TRANSLATOR_PLUGIN_DUMMY,
	TRANSLATOR_PLUGIN_SQLITE
} TranslatorPluginType;

/* plugin definition */
typedef struct TranslatorPlugin
{
    TranslatorPluginType type;

    /* functional interface */
    Node *(*translateParse) (Node *q);
    QueryOperator *(*translateQuery) (Node *node);

} TranslatorPlugin;

// plugin management
extern void chooseTranslatorPlugin(TranslatorPluginType type);
extern void chooseTranslatorPluginFromString(char *type);

// wrapper interface
extern Node *translateParse(Node *q);
extern QueryOperator *translateQuery (Node *node);

#endif /* TRANSLATOR_H_ */
