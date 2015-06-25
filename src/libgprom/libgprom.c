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
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_external.h"

#define LIBARY_REWRITE_CONTEXT "LIBGRPROM_QUERY_CONTEXT"

#include <pthread.h>

/* mutex to prevent reentrance of library methods for now */
pthread_mutex_t gpromLibMutex = PTHREAD_MUTEX_INITIALIZER;

#define LOCK_MUTEX()
#define UNLOCK_MUTEX()

//#define LOCK_MUTEX()
//    printf("try to log mutex");
//    pthread_mutex_lock(&gpromLibMutex)
//
//#define UNLOCK_MUTEX()
//    printf("unlock mutex");
//    pthread_mutex_unlock(&


void
gprom_init(void)
{
    LOCK_MUTEX();
    initMemManager();
    mallocOptions();
    initLogger();
    registerSignalHandler();
    UNLOCK_MUTEX();
}

void
gprom_readOptions(int argc, char * const args[])
{
    LOCK_MUTEX();
    if(parseOption(argc, args) != 0)
    {
        printOptionParseError(stdout);
    }
    UNLOCK_MUTEX();
}

void
gprom_readOptionAndInit(int argc, char *const args[])
{
    LOCK_MUTEX();
    readOptionsAndIntialize("gprom-libary","",argc,(char **) args);
    UNLOCK_MUTEX();
}

void
gprom_configFromOptions(void)
{
    setupPluginsFromOptions();
}

void gprom_shutdown(void)
{
    LOCK_MUTEX();
    deregisterSignalHandler();
    shutdownApplication();
    UNLOCK_MUTEX();
}

const char *
gprom_rewriteQuery(const char *query)
{
    LOCK_MUTEX();
    NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
    char *result = "";
    result = rewriteQuery((char *) query);
    UNLOCK_MUTEX();
    RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
}


void
gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback)
{
    registerLogCallback(callback);
}

void
gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback)
{
    registerExceptionCallback((GProMExceptionCallbackFunctionInternal) callback);
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

void
gprom_registerMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin)
{
    setMetadataLookupPlugin(assembleExternalMetadataLookupPlugin(plugin));
}
