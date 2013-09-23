/*-------------------------------------------------------------------------
 *
 * mem_mgr.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    This module is developed to provide memory management that organizes
 *    and reduces the work of allocating and freeing memory.
 *
 *        A memory context can be created to record the allocated memories
 *        and be destroyed to batch free all the memories recorded in it.
 *        The allocated memories information can be traced in logs.
 *
 *-------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "uthash.h"

static void addAlloc(MemContext *mc, void *addr, char *file, unsigned line);
static void delAlloc(MemContext *mc, void *addr, char *file, unsigned line);

MemContext *curMemContext;

inline void
setCurMemContext(MemContext *mc)
{
    curMemContext = mc;
}

inline MemContext *
getCurMemContext(void)
{
    return curMemContext;
}

/*
 * Adds allocated memory information to a memory context.
 */
static void
addAlloc(MemContext *mc, void *addr, char *file, unsigned line)
{
    Allocation *newAlloc = (Allocation *) malloc(sizeof(Allocation));
    assert(mc != NULL);
    newAlloc->address = addr;
    newAlloc->file = file;
    newAlloc->line = line;
    HASH_ADD_PTR(mc->hashAlloc, address, newAlloc); // add to hash table. Use address as key
    DEBUG_LOG("Added [addr:%p, file:'%s', line:%u] to memory context '%s'.",
            addr, file, line, mc->contextName);
}

/*
 * Removes memory allocation information from a memory context.
 */
static void
delAlloc(MemContext *mc, void *addr, char *file, unsigned line)
{
    Allocation *alloc = NULL;
    HASH_FIND_PTR(mc->hashAlloc, &addr, alloc); // find allocation info by address first
    if (alloc)
    {
        char *tmpfile = alloc->file;
        int tmpline = alloc->line;
        HASH_DEL(mc->hashAlloc, alloc); // remove the allocation info
        free(alloc);
        DEBUG_LOG(
                "Deleted [addr:%p, file:'%s', line:%d] from memory context '%s'.",
                addr, tmpfile, tmpline, mc->contextName);
    }
}

/*
 * Creates a new memory context.
 */
MemContext *
newMemContext(char *contextName)
{
    MemContext *mc = (MemContext *) malloc(sizeof(MemContext));
    mc->contextName = contextName;
    mc->hashAlloc = NULL;
    setCurMemContext(mc);
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
        DEBUG_LOG("Found [addr:%p, file:'%s', line:%d] in memory context '%s'.",
                alloc->address, alloc->file, alloc->line, mc->contextName);
    }
    else
    {
        DEBUG_LOG("Could not find address %p in memory context '%s'.", addr,
                mc->contextName);
    }
    return alloc;
}

/*
 * Removes all the memory allocation records from the specified memory context
 * and free those memories. Will not destroy the memory context itself.
 */
void
clearMemContext(MemContext *mc)
{
    if (mc)
    {
        Allocation *curAlloc, *tmp;
        HASH_ITER(hh, mc->hashAlloc, curAlloc, tmp)
        {
            free_(curAlloc->address, __FILE__, __LINE__);
        }
    }
}

/*
 * Removes all the memory allocation records from the specified memory context
 * and free those memories and finally destroy the memory context.
 */
void
freeMemContext(MemContext *mc)
{
    int size = memContextSize(mc);
    if (size > 0)
    {
        clearMemContext(mc);
        free(mc);
    }
    else if (size == 0)
    {
        free(mc);
    }
}



/*
 * Allocates memory and records it in the current memory context.
 */
void *
malloc_(size_t bytes, char *file, unsigned line)
{
    void *mem = malloc(bytes);
    if (mem == NULL)
    {
        log_(LOG_ERROR, file, line, "Fail to malloc.");
    }
    else
    {
        log_(LOG_TRACE, file, line, "%ld bytes memory @%p allocated.", bytes,
                mem);
    }

    addAlloc(curMemContext, mem, file, line);

    return mem;
}

/*
 * Allocates memory and initializes it with 0 and records it in the current
 * memory context.
 */
void *
calloc_(size_t bytes, unsigned count, char *file, unsigned line)
{
    void *mem = calloc(count, bytes);
    if (mem == NULL)
    {
        log_(LOG_ERROR, file, line, "Fail to calloc.");
    }
    else
    {
        log_(LOG_TRACE, file, line,
                "%ldx%ld bytes memory @%p allocated and initialized with 0.",
                bytes, count, mem);
    }

    addAlloc(curMemContext, mem, file, line);

    return mem;
}

/*
 * Removes the specified memory allocation record from the current memory context
 * and then free the memory at the address.
 */
void
free_(void *mem, char *file, unsigned line)
{
    if (mem)
    {
        delAlloc(curMemContext, mem, file, line);
        free(mem);
        log_(LOG_TRACE, file, line, "Memory @%p freed.", mem);
    }
}

