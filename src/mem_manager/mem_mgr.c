/*-------------------------------------------------------------------------
 *
 * mem_mgr.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    This module is to provide memory management tool that organizes
 *    and reduces the work of allocating and freeing memory.
 *
 *        A memory context can be created to record the allocated memories
 *        and be destroyed to batch free all the memories recorded in it.
 *        The allocated memories information can be traced in logs.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "uthash.h"
#include "instrumentation/timing_instrumentation.h"
#include "instrumentation/memory_instrumentation.h"

// override the defaults for UT_hash memory allocation to use standard malloc
#undef uthash_malloc
#undef uthash_free
#define uthash_malloc(sz) malloc(sz)
#define uthash_free(ptr,sz) free(ptr)

// use actual malloc and free functions
#undef free
#undef malloc

#define DEFAULT_MEM_CONTEXT_NAME "DEFAULT_MEMORY_CONTEXT"

typedef struct MemContextNode
{
    MemContext *mc;
    struct MemContextNode *next;
} MemContextNode; // context stack node

static inline void addAlloc(MemContext *mc, void *addr, const char *file,
        unsigned line);
static inline void delAlloc(MemContext *mc, void *addr, const char *file,
        unsigned line);
static inline void freeAlloc (Allocation *alloc, MemContext *mc, void *addr,
        const char *file, unsigned line);


static MemContext *curMemContext = NULL; // global pointer to current memory context
static MemContext *defaultMemContext = NULL;
static MemContextNode *topContextNode = NULL;
static int contextStackSize = 0;

/*
 * Creates default memory context and pushes it into context stack.
 */
void
initMemManager(void)
{
    defaultMemContext = NEW_MEM_CONTEXT(DEFAULT_MEM_CONTEXT_NAME);
    ACQUIRE_MEM_CONTEXT(defaultMemContext);
    // default context always lies on the bottom of the stack
}

/*
 * Free all contexts in the context stack and clear the stack.
 */
void
destroyMemManager(void)
{
    while (topContextNode->next)
    {
        FREE_CUR_MEM_CONTEXT();
        RELEASE_MEM_CONTEXT();
    }
    // free all contexts in the context stack except default context

    ASSERT(topContextNode->mc == defaultMemContext);
    ASSERT(curMemContext == defaultMemContext);
    int size = memContextSize(defaultMemContext);
    if (size > 0)
    {
        CLEAR_CUR_MEM_CONTEXT();
        free(defaultMemContext);
    }
    else if (size == 0)
    {
        free(defaultMemContext);
    }
    // free default context

    free(topContextNode); // free default context node
    DEBUG_LOG("Freed memory context '%s'.", DEFAULT_MEM_CONTEXT_NAME);
}

/*
 * Sets current context and pushes it into context stack.
 */
void
setCurMemContext(MemContext *mc, const char *file, unsigned line)
{
    if (mc)
    {
        MemContextNode *node = calloc(1, sizeof(MemContextNode));
        node->mc = mc;
        node->next = topContextNode;
        topContextNode = node;
        contextStackSize++;
        // push the passed-in context into context stack

        // track mem allocations, but not of the context used for memory debugging
        if (opt_memmeasure && !streq(mc->contextName,MEMDEBUG_CONTEXT_NAME))
            addContext(mc->contextName, 0, TRUE);

        curMemContext = topContextNode->mc;
        GENERIC_LOG(LOG_DEBUG, file, line, "Set current memory context to '%s'@%p.",
                curMemContext->contextName, curMemContext);
    }

}

inline MemContext *
getCurMemContext(void)
{
    return curMemContext;
}

/*
 * Pops current context and returns to the previous context. Will not free
 * the current context.
 */
MemContext *
releaseCurMemContext(const char *file, unsigned line)
{
    if (topContextNode->next) // does not free the bottom node holding default context
    {
        MemContextNode *oldTop = topContextNode;
        MemContext *oldContext = oldTop->mc;
        topContextNode = oldTop->next;
        free(oldTop);
        contextStackSize--;
        // pop current context from context stack

        curMemContext = topContextNode->mc; // switch to previous context
        GENERIC_LOG(LOG_DEBUG, file, line,
                "Set back current memory context to '%s'@%p.",
                curMemContext->contextName, curMemContext);
        return oldContext;
    }

    return NULL;
}

/*
 * Adds allocated memory information to a memory context.
 */
static void
addAlloc(MemContext *mc, void *addr, const char *file, unsigned line)
{
    Allocation *newAlloc = (Allocation *) malloc(sizeof(Allocation));
    ASSERT(mc != NULL);
    newAlloc->address = addr;
    newAlloc->file = file;
    newAlloc->line = line;
    HASH_ADD_PTR(mc->hashAlloc, address, newAlloc); // add to hash table. Use address as key

    GENERIC_LOG(LOG_TRACE, file, line,
        "Added [addr:%p, file:'%s', line:%u] to memory context '%s'.", addr,
        file, line, mc->contextName);
}

/*
 * Removes memory allocation information from a memory context.
 */
static void
delAlloc(MemContext *mc, void *addr, const char *file, unsigned line)
{
    Allocation *alloc = NULL;
    HASH_FIND_PTR(mc->hashAlloc, &addr, alloc); // find allocation info by address first
    if (alloc)
    {
        freeAlloc(alloc, mc, addr, file, line);
        return;
    }
    // allocation not found search in ancestor contexts on the stack
    else
    {
        MemContextNode *c = topContextNode;
        MemContext *curC;

        GENERIC_LOG(LOG_TRACE, file, line,
                "Not found [addr:%p] from memory context '%s'.",
                            addr, mc->contextName);

        while(c->mc != mc)
            c = c->next;
        c = c->next;

        for(; c != NULL; c = c->next)
        {
            curC = c->mc;
            HASH_FIND_PTR(curC->hashAlloc, &addr, alloc);

            if (alloc)
            {
                freeAlloc(alloc, curC, addr, file, line);
                return;
            }
            else
                GENERIC_LOG(LOG_TRACE, file, line,
                        "Not found [addr:%p] from memory context '%s'.",
                        addr, curC->contextName);
       }
    }

    // did not find allocation
    FATAL_LOG("did not find allocation info for <%p> in memory context <%s>", addr, mc->contextName);
}

/*
 * Free an allocation
 */

static void
freeAlloc (Allocation *alloc, MemContext *mc, void *addr, const char *file, unsigned line)
{
    const char *tmpfile = alloc->file;
    int tmpline = alloc->line;
    HASH_DEL(mc->hashAlloc, alloc); // remove the allocation info
    free(alloc);
    GENERIC_LOG(LOG_TRACE, file, line,
            "Deleted [addr:%p, file:'%s', line:%d] from memory context '%s'.",
            addr, tmpfile, tmpline, mc->contextName);
}


/*
 * Creates a memory context.
 */
MemContext *
newMemContext(char *contextName, const char *file, unsigned line)
{
    MemContext *mc = (MemContext *) malloc(sizeof(MemContext));
    mc->contextName = contextName;
    mc->hashAlloc = NULL;
    GENERIC_LOG(LOG_DEBUG, file, line, "Created memory context '%s'.",
            mc->contextName);
    return mc;
}

/*
 * Gets context size.
 */
int
memContextSize(MemContext *mc)
{
    return mc ? HASH_COUNT(mc->hashAlloc) : -1;
}

/*
 * Finds memory allocation information in the memory context by address.
 * Returns NULL if not found.
 */
Allocation *
findAlloc(const MemContext *mc, const void *addr)
{
    if (!mc)
    {
        return NULL;
    }
    Allocation *alloc = NULL;
    HASH_FIND_PTR(mc->hashAlloc, &addr, alloc);
    if (alloc)
    {
        TRACE_LOG("Found [addr:%p, file:'%s', line:%d] in memory context '%s'.",
                alloc->address, alloc->file, alloc->line, mc->contextName);
    }
    else
    {
        TRACE_LOG("Could not find address %p in memory context '%s'.", addr,
                mc->contextName);
    }
    return alloc;
}

/*
 * Removes all the memory allocation records from the current context
 * and free those memories. Will not destroy the memory context itself.
 */
void
clearCurMemContext(const char *file, unsigned line)
{
    Allocation *curAlloc, *tmp;
    HASH_ITER(hh, curMemContext->hashAlloc, curAlloc, tmp)
    {
        free_(curAlloc->address, file, line);
    }
}

/*
 * Removes all the memory allocation records from the current context
 * and free those memories and finally destroy the memory context itself.
 */
void
freeCurMemContext(const char *file, unsigned line)
{
    if (topContextNode->next) // does not free default context and its node
    {
        int size = memContextSize(curMemContext);
        char *name = curMemContext->contextName;
        if (size > 0)
        {
            clearCurMemContext(file, line);
            free(curMemContext);
            curMemContext = NULL;
        }
        else if (size == 0)
        {
            free(curMemContext);
            curMemContext = NULL;
        }
        GENERIC_LOG(LOG_DEBUG, file, line, "Freed memory context '%s'.", name);
    }
}

/*
 * Allocates memory and records it in the current memory context.
 */
void *
malloc_(size_t bytes, const char *file, unsigned line)
{
    void *mem = malloc(bytes);
//    memset(mem, 178, bytes);
    if (mem == NULL)
    {
        GENERIC_LOG(LOG_ERROR, file, line, "Fail to malloc.");
    }
    else
    {
        GENERIC_LOG(LOG_TRACE, file, line, "%ld bytes memory @%p allocated.", bytes,
                mem);
    }

    // track mem allocations, but not of the context used for memory debugging
    if (opt_memmeasure && !streq(curMemContext->contextName,MEMDEBUG_CONTEXT_NAME))
        addContext(curMemContext->contextName, bytes, FALSE);

    addAlloc(curMemContext, mem, file, line);

    return mem;
}

/*
 * Allocates memory and initializes it with 0 and records it in the current
 * memory context.
 */
void *
calloc_(size_t bytes, unsigned count, const char *file, unsigned line)
{
    void *mem = calloc(count, bytes);
    if (mem == NULL)
    {
        GENERIC_LOG(LOG_ERROR, file, line, "Fail to calloc.");
    }
    else
    {
        GENERIC_LOG(LOG_TRACE, file, line,
                "%ldx%ld bytes memory @%p allocated and initialized with 0.",
                bytes, count, mem);
    }

    // track mem allocations, but not of the context used for memory debugging
    if (opt_memmeasure && !streq(curMemContext->contextName,MEMDEBUG_CONTEXT_NAME))
        addContext(curMemContext->contextName, bytes, FALSE);

    addAlloc(curMemContext, mem, file, line);

    return mem;
}

/*
 * Removes the specified memory allocation record from the current memory context
 * and then free the memory at the address.
 */
void
free_(void *mem, const char *file, unsigned line)
{
    if (mem)
    {
        delAlloc(curMemContext, mem, file, line);
        free(mem);
        GENERIC_LOG(LOG_TRACE, file, line, "Memory @%p freed.", mem);
    }
}

/*
 *
 */
char *
contextStringDup(char *input)
{
    if (input == NULL)
        return NULL;
    char *result = MALLOC(strlen(input) + 1);
    result = strcpy(result, input);

    return result;
}
