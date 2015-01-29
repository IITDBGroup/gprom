/*-----------------------------------------------------------------------------
 *
 * executor.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

#include "execution/executor.h"
#include "execution/exe_output_dl.h"
#include "execution/exe_output_gp.h"
#include "execution/exe_output_sql.h"
#include "execution/exe_run_query.h"

// plugin
static ExecutorPlugin *plugin = NULL;

// wrapper interface
void
execute (void *code)
{
    ASSERT(plugin);

    plugin->execute(code);
}

// plugin management
void
chooseExecutorPlugin(ExecutorPluginType type)
{
    plugin = NEW(ExecutorPlugin);
    switch(type)
    {
        case EXECUTOR_PLUGIN_OUTPUT_GP:
            plugin->execute = executeOutputGP;
            break;
        case EXECUTOR_PLUGIN_OUTPUT_SQL:
            plugin->execute = executeOutputSQL;
            break;
        case EXECUTOR_PLUGIN_RUN_QUERY:
            plugin->execute = exeRunQuery;
            break;
        case EXECUTOR_PLUGIN_OUTPUT_DATALOG:
            plugin->execute = executeOutputDL;
            break;
    }
}

void
chooseExecutorPluginFromString(char *type)
{
    INFO_LOG("PLUGIN analyzer: <%s>", type);

    if (streq(type,"sql"))
        chooseExecutorPlugin(EXECUTOR_PLUGIN_OUTPUT_SQL);
    else if (streq(type,"gp"))
        chooseExecutorPlugin(EXECUTOR_PLUGIN_OUTPUT_GP);
    else if (streq(type,"run"))
        chooseExecutorPlugin(EXECUTOR_PLUGIN_RUN_QUERY);
    else if (streq(type,"dl"))
        chooseExecutorPlugin(EXECUTOR_PLUGIN_OUTPUT_DATALOG);
    else
        FATAL_LOG("unkown analyzer plugin type: <%s>", type);
}
