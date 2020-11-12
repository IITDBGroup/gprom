/*-----------------------------------------------------------------------------
 *
 * executor.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_EXECUTION_EXECUTOR_H_
#define INCLUDE_EXECUTION_EXECUTOR_H_

#include "common.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"

/* types of supported plugins */
typedef enum ExecutorPluginType
{
    EXECUTOR_PLUGIN_OUTPUT_SQL,
    EXECUTOR_PLUGIN_OUTPUT_GP,
    EXECUTOR_PLUGIN_OUTPUT_DATALOG,
    EXECUTOR_PLUGIN_RUN_QUERY
} ExecutorPluginType;

/* plugin definition */
typedef struct ExecutorPlugin
{
    ExecutorPluginType type;

    /* functional interface */
    void (*execute) (void *code);
} ExecutorPlugin;

// plugin management
extern void chooseExecutorPlugin(ExecutorPluginType type);
extern void chooseExecutorPluginFromString(char *type);

extern void execute (void *code);

#endif /* INCLUDE_EXECUTION_EXECUTOR_H_ */
