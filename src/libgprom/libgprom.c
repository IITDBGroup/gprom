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
    return (const char *) rewriteQuery((char *) query);
}


void
gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback)
{
    registerLogCallback(callback);
}


char *
gprom_getStringOption (char *name)
{
    return getStringOption(name);
}

int
gprom_getIntOption (char *name)
{
    return getIntOption(name);
}

boolean
gprom_getBoolOption (char *name)
{
    return getBoolOption(name);
}

double
gprom_getFloatOption (char *name)
{
    return getFloatOption(name);
}

void
gprom_setStringOption (char *name, char *value)
{
    return setStringOption(name,value);
}

void
gprom_setIntOption(char *name, int value)
{
    return setIntOption(name,value);
}

void
gprom_setBoolOption(char *name, boolean value)
{
    return setBoolOption(name,value);
}

void
gprom_setFloatOption(char *name, double value)
{
    return setFloatOption(name,value);
}
