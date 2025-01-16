/*-----------------------------------------------------------------------------
 *
 * sql_serializer.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SQL_SERIALIZER_H_
#define SQL_SERIALIZER_H_

#include "model/query_operator/query_operator.h"

/* types of supported plugins */
typedef enum SqlserializerPluginType
{
    SQLSERIALIZER_PLUGIN_ORACLE,
    SQLSERIALIZER_PLUGIN_POSTGRES,
    SQLSERIALIZER_PLUGIN_HIVE,
    SQLSERIALIZER_PLUGIN_DL,
	SQLSERIALIZER_PLUGIN_LB,
    SQLSERIALIZER_PLUGIN_SQLITE,
    SQLSERIALIZER_PLUGIN_DUCKDB
} SqlserializerPluginType;

/* plugin definition */
typedef struct SqlserializerPlugin
{
    SqlserializerPluginType type;

    /* functional interface */
    char *(*serializeOperatorModel) (Node *q);
    char *(*serializeQuery) (QueryOperator *q);
    char *(*quoteIdentifier) (char *ident);

} SqlserializerPlugin;

// plugin management
extern void chooseSqlserializerPlugin(SqlserializerPluginType type);
extern void chooseSqlserializerPluginFromString(char *type);
extern SqlserializerPluginType getActiveSqlserializerPlugin(void);

// sqlserializer interface wrapper
extern char *serializeOperatorModel(Node *q);
extern char *serializeQuery(QueryOperator *q);
extern char *quoteIdentifier (char *ident);

#endif /* SQL_SERIALIZER_H_ */
