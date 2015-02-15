/*-----------------------------------------------------------------------------
 *
 * libgprom.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "log/logger.h"
#include "libgprom/libgprom.h"
#include "rewriter.h"

#define LIBARY_REWRITE_CONTEXT "LIBGRPROM_QUERY_CONTEXT"

void
gprom_init(void)
{
    initMemManager();
    mallocOptions();
    initLogger();
}

void
gprom_readOptions(int argc, char * const args[])
{
    if(parseOption(argc, args) != 0)
    {
        printOptionParseError(stdout);
    }
}

void
gprom_readOptionAndInit(int argc, char *const args[])
{
    readOptionsAndIntialize("gprom-libary","",argc,(char **) args);
}

void
gprom_configFromOptions(void)
{
    setupPluginsFromOptions();
}

void gprom_shutdown(void)
{
    shutdownApplication();
}

const char *
gprom_rewriteQuery(const char *query)
{
    NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
    RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(rewriteQuery((char *) query));
}


void
gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback)
{
    registerLogCallback(callback);
}

void
gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback)
{

}

void
gprom_setMaxLogLevel (int maxLevel)
{
    setMaxLevel((LogLevel) maxLevel);
}


const char *
gprom_getStringOption (const char *name)
{
    return getStringOption((char *) name);
}

int
gprom_getIntOption (const char *name)
{
    return getIntOption((char *) name);
}

boolean
gprom_getBoolOption (const char *name)
{
    return getBoolOption((char *) name);
}

double
gprom_getFloatOption (const char *name)
{
    return getFloatOption((char *) name);
}

const char *
gprom_getOptionType(const char *name)
{
    ASSERT(hasOption((char *) name));
    return OptionTypeToString(getOptionType((char *) name));
}

boolean
gprom_optionExists(const char *name)
{
    return hasOption((char *) name);
}

void
gprom_setStringOption (const char *name, const char *value)
{
    return setStringOption((char *) name, strdup((char *) value));
}

void
gprom_setIntOption(const char *name, int value)
{
    return setIntOption((char *) name, value);
}

void
gprom_setBoolOption(const char *name, boolean value)
{
    return setBoolOption((char *) name,value);
}

void
gprom_setFloatOption(const char *name, double value)
{
    return setFloatOption((char *) name,value);
}
