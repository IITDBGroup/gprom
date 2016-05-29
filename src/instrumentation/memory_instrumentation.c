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
#define KEY_UNUSED_SIZE "unusedSize"
#define KEY_NUM_CHUNKS "num_chunks"
#define KEY_TOTAL_CHUNK_SIZE "total_chunk_size"

static inline HashMap *getInfoHash(char *name);



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

static inline HashMap *
getInfoHash(char *name)
{
    HashMap *info = NULL;

    if (!MAP_HAS_STRING_KEY(ctxInfo,name))
    {
        info = NEW_MAP(Constant, Invalid);
        MAP_ADD_STRING_KEY(ctxInfo,name,info);

        MAP_ADD_STRING_KEY(info,KEY_TOTAL_SIZE, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_NUM_ACQUIRE, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_NUM_ALLOC, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_UNUSED_SIZE, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_NUM_CHUNKS, createConstLong(0L));
        MAP_ADD_STRING_KEY(info,KEY_TOTAL_CHUNK_SIZE, createConstLong(0L));
    }
    else
        info = (HashMap *) MAP_GET_STRING(ctxInfo, name);

    return info;
}

#define ADD_TO_LONG_ENTRY(_map,_key,_value) LONG_VALUE(MAP_GET_STRING(_map,_key)) += _value

void
addContext(char *name, unsigned int allocationSize, boolean acquired)
{
    if (ctx == NULL)
        return;

    ACQUIRE_MEM_CONTEXT(ctx);

    HashMap *info = getInfoHash(name);

    ADD_TO_LONG_ENTRY(info,KEY_TOTAL_SIZE,allocationSize);
    if (acquired)
        ADD_TO_LONG_ENTRY(info,KEY_NUM_ACQUIRE,1);
    if (allocationSize != 0)
        ADD_TO_LONG_ENTRY(info,KEY_NUM_ALLOC,1);

    RELEASE_MEM_CONTEXT();
}

void
addContextUnused(char *name, unsigned long unusedSize)
{
    if (ctx == NULL)
        return;

    ACQUIRE_MEM_CONTEXT(ctx);

    HashMap *info = getInfoHash(name);

    ADD_TO_LONG_ENTRY(info,KEY_UNUSED_SIZE,unusedSize);

    RELEASE_MEM_CONTEXT();
}

void
addContextChunkInfo(char *name, unsigned long newChunkSize)
{
    if (ctx == NULL)
        return;

    ACQUIRE_MEM_CONTEXT(ctx);

    HashMap *info = getInfoHash(name);

    ADD_TO_LONG_ENTRY(info,KEY_NUM_CHUNKS,1);
    ADD_TO_LONG_ENTRY(info,KEY_TOTAL_CHUNK_SIZE,newChunkSize);

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
        long numChunks = LONG_VALUE(MAP_GET_STRING(i, KEY_NUM_CHUNKS));
        long totalChunkSize = LONG_VALUE(MAP_GET_STRING(i, KEY_TOTAL_CHUNK_SIZE));
        long unusedSize = LONG_VALUE(MAP_GET_STRING(i, KEY_UNUSED_SIZE));

        printf("mem-context: %*s - total allocated MB: %12f "
                "| #acquired %9ld "
                "| #allocations: %9ld "
                "| context uses: %12f MB "
                "| #chunks:  %9ld "
                "| lost mem: %12f MB"
                "\n",
                maxTimerNameLength,
                name,
                ((double) totalSize) / 1000000.0,
                numAcquired,
                numAlloc,
                ((double) totalChunkSize) / 1000000.0,
                numChunks,
                ((double) unusedSize) / 1000000.0
                );
    }

    RELEASE_MEM_CONTEXT();
}
