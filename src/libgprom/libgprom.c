/*-----------------------------------------------------------------------------
 *
 * libgprom.c
 *
 *
 *      AUTHOR: lord_pretzel
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

#define LOCK_NAME gprom_lib_globallock

//printf("\nMUTEX\n%s:%u\n", __FILE__, __LINE__);
//printf("\nUNLOCK\n%s:%u\n", __FILE__, __LINE__);

#if defined(HAVE_LIBPTHREAD) && ! defined(OS_WINDOWS)
#define LOCK_MUTEX() pthread_mutex_lock(&LOCK_NAME); fflush(stdout)
#define UNLOCK_MUTEX()  fflush(stdout); pthread_mutex_unlock(&LOCK_NAME)
#define CREATE_MUTEX static pthread_mutex_t LOCK_NAME = PTHREAD_MUTEX_INITIALIZER;
#define INIT_MUTEX()
#define DESTROY_MUTEX()
#else
#define LOCK_MUTEX() WaitForSingleObject(LOCK_NAME, INFINITE)
#define UNLOCK_MUTEX() ReleaseMutex(LOCK_NAME)
#define CREATE_MUTEX static HANDLE LOCK_NAME = NULL;
#define INIT_MUTEX() LOCK_NAME = CreateMutex( NULL, FALSE, NULL)
#define DESTROY_MUTEX()
#endif

#undef boolean

#ifdef OS_WINDOWS
#include <windows.h>
#include <process.h>
#endif

#define boolean int

CREATE_MUTEX

void
gprom_init(void)
{
    INIT_MUTEX();
    LOCK_MUTEX();
    initMemManager();
    mallocOptions();
    initLogger();
//    registerSignalHandler();
    setWipeContext(LIBARY_REWRITE_CONTEXT);
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
    LOCK_MUTEX();
    setupPluginsFromOptions();
    UNLOCK_MUTEX();
}

void
gprom_reconfPlugins(void)
{
    LOCK_MUTEX();
    resetupPluginsFromOptions();
    UNLOCK_MUTEX();
}

void gprom_shutdown(void)
{
    LOCK_MUTEX();
    shutdownApplication();
//    deregisterSignalHandler();
    UNLOCK_MUTEX();
//    DESTROY_MUTEX();
}

const char *
gprom_rewriteQuery(const char *query)
{
    LOCK_MUTEX();
    NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
    char *result = "";
    char *returnResult = NULL;
    TRY
    {
        result = rewriteQueryWithRethrow((char *) query);
        RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(result,returnResult);
    }
    ON_EXCEPTION
    {
        ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
        UNLOCK_MUTEX();
        returnResult = NULL;
    }
    END_ON_EXCEPTION
    UNLOCK_MUTEX();
    return returnResult;
}


void
gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback)
{
    LOCK_MUTEX();
    registerLogCallback(callback);
    UNLOCK_MUTEX();
}

void
gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback)
{
LOCK_MUTEX();
    registerExceptionCallback((GProMExceptionCallbackFunctionInternal) callback);
    UNLOCK_MUTEX();
}

void
gprom_setMaxLogLevel (int maxLevel)
{
LOCK_MUTEX();
    setMaxLevel((LogLevel) maxLevel);
    UNLOCK_MUTEX();
}


const char *
gprom_getStringOption (const char *name)
{
    LOCK_MUTEX();
    const char *result = getStringOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

int
gprom_getIntOption (const char *name)
{
    LOCK_MUTEX();
    int result = getIntOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

boolean
gprom_getBoolOption (const char *name)
{
    LOCK_MUTEX();
    boolean result = getBoolOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

double
gprom_getFloatOption (const char *name)
{
    LOCK_MUTEX();
    float result = getFloatOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

const char *
gprom_getOptionType(const char *name)
{
    LOCK_MUTEX();
    ASSERT(hasOption((char *) name));
    char *result = OptionTypeToString(getOptionType((char *) name));
    UNLOCK_MUTEX();
    return result;
}

boolean
gprom_optionExists(const char *name)
{
    LOCK_MUTEX();
    boolean result = hasOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

void
gprom_setOptionsFromMap()
{
LOCK_MUTEX();
UNLOCK_MUTEX();
}

void
gprom_setOption(const char *name, const char *value)
{
    LOCK_MUTEX();
    setOption((char *) name, strdup((char *) value));
    UNLOCK_MUTEX();
}

void
gprom_setStringOption (const char *name, const char *value)
{
    LOCK_MUTEX();
    setStringOption((char *) name, strdup((char *) value));
    UNLOCK_MUTEX();
}

void
gprom_setIntOption(const char *name, int value)
{
    LOCK_MUTEX();
    setIntOption((char *) name, value);
    UNLOCK_MUTEX();
}

void
gprom_setBoolOption(const char *name, boolean value)
{
    LOCK_MUTEX();
    setBoolOption((char *) name,value);
    UNLOCK_MUTEX();
}

void
gprom_setFloatOption(const char *name, double value)
{
    LOCK_MUTEX();
    setFloatOption((char *) name,value);
    UNLOCK_MUTEX();
}

void
gprom_registerMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin)
{
    LOCK_MUTEX();
    setMetadataLookupPlugin(assembleExternalMetadataLookupPlugin(plugin));
    UNLOCK_MUTEX();
}
