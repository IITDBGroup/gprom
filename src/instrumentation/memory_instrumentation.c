/*-----------------------------------------------------------------------------
 *
 * memory_instrumentation.c
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
#include "log/logger.h"
#include "instrumentation/memory_instrumentation.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/expression/expression.h"

// bookkeeping data structures
static HashMap *ctxInfo;
static MemContext *ctx = NULL;

// dictionary fields used
#define KEY_TOTAL_SIZE "total_size"
#define KEY_NUM_ALLOC "num_alloc"
#define KEY_NUM_ACQUIRE "num_acquire"

void
setupMemInstrumentation(void)
{
    ctx = NEW_MEM_CONTEXT(MEMDEBUG_CONTEXT_NAME);
    ACQUIRE_MEM_CONTEXT(ctx);

    ctxInfo = NEW_MAP(Constant,HashMap);

    RELEASE_MEM_CONTEXT();
}

void
shutdownMemInstrumentation(void)
{
    // deactivate memory tracking to avoid errors after we are done
    opt_memmeasure = FALSE;

    FREE_MEM_CONTEXT(ctx);
}

#define ADD_TO_LONG_ENTRY(_map,_key,_value) LONG_VALUE(MAP_GET_STRING(_map,_key)) += _value

void
addContext(char *name, int allocationSize, boolean acquired)
{
    if (ctx == NULL)
        return;

    ACQUIRE_MEM_CONTEXT(ctx);

    HashMap *info = NULL;

    if (!MAP_HAS_STRING_KEY(ctxInfo,name))
    {
        info = NEW_MAP(Constant, Invalid);
        MAP_ADD_STRING_KEY(ctxInfo,name,info);

        MAP_ADD_STRING_KEY(info,KEY_TOTAL_SIZE, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_NUM_ACQUIRE, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_NUM_ALLOC, createConstLong(0L));
    }
    else
        info = (HashMap *) MAP_GET_STRING(ctxInfo, name);

    ADD_TO_LONG_ENTRY(info,KEY_TOTAL_SIZE,allocationSize);
    if (acquired)
        ADD_TO_LONG_ENTRY(info,KEY_NUM_ACQUIRE,1);
    if (allocationSize != 0)
        ADD_TO_LONG_ENTRY(info,KEY_NUM_ALLOC,1);

    RELEASE_MEM_CONTEXT();
}

void
outputMemstats(boolean showDetails)
{
    int maxTimerNameLength = 30;

    ACQUIRE_MEM_CONTEXT(ctx);

    // find maximal name length
    FOREACH_HASH_KEY(Constant, n, ctxInfo)
    {
        int len = strlen(STRING_VALUE(n));
        maxTimerNameLength = (maxTimerNameLength < len) ? len : maxTimerNameLength;
    }

    // output one line per context
    FOREACH_HASH_ENTRY(e, ctxInfo)
    {
        char *name = STRING_VALUE(e->key);
        HashMap *i = (HashMap *) e->value;

        long numAlloc = LONG_VALUE(MAP_GET_STRING(i, KEY_NUM_ALLOC));
        long numAcquired = LONG_VALUE(MAP_GET_STRING(i, KEY_NUM_ACQUIRE));
        long totalSize = LONG_VALUE(MAP_GET_STRING(i, KEY_TOTAL_SIZE));

        printf("mem-context: %*s - total MB allocated: %12f | was acquired %9ld times | total number of allocations: %9ld \n",
                maxTimerNameLength,
                name,
                ((double) totalSize) / 1000000.0,
                numAcquired,
                numAlloc
                );
    }

    RELEASE_MEM_CONTEXT();
}
