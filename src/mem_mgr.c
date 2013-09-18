/*-------------------------------------------------------------------------
 *
 * mem_mgr.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "mem_mgr.h"
#include <stdlib.h>
#include "logger.h"
#include "uthash.h"

static void
addAlloc(MemContext *mc, void *addr, char *file, unsigned line)
{
    Allocation *newAlloc = (Allocation *) malloc(sizeof(Allocation));
    newAlloc->address = addr;
    newAlloc->file = file;
    newAlloc->line = line;
    HASH_ADD_PTR(mc->hashAlloc, address, newAlloc);
    log_(DEBUG, __FILE__, __LINE__,
            "Added [addr:%p, file:'%s', line:%u] to memory context '%s'.", addr,
            file, line, mc->contextName);
}

static void
delAlloc(MemContext *mc, void *addr, char *file, unsigned line)
{
    Allocation *alloc = NULL;
    HASH_FIND_PTR(mc->hashAlloc, &addr, alloc);
    if (alloc)
    {
        char *tmpfile = alloc->file;
        int tmpline = alloc->line;
        HASH_DEL(mc->hashAlloc, alloc);
        free(alloc);
        log_(DEBUG, file, line,
                "Deleted [addr:%p, file:'%s', line:%d] from memory context '%s'.",
                addr, tmpfile, tmpline, mc->contextName);
    }
}

MemContext *
newMemContext(char *contextName)
{
    MemContext *mc = (MemContext *) malloc(sizeof(MemContext));
    mc->contextName = contextName;
    mc->hashAlloc = NULL;
    setCurMemContext(mc);
    return mc;
}

int
memContextSize(MemContext *mc)
{
    return mc ? HASH_COUNT(mc->hashAlloc) : -1;
}

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
        log_(DEBUG, __FILE__, __LINE__,
                "Found [addr:%p, file:'%s', line:%d] in memory context '%s'.",
                alloc->address, alloc->file, alloc->line, mc->contextName);
    }
    else
    {
        log_(DEBUG, __FILE__, __LINE__,
                "Could not find address=%p in memory context '%s'.", addr,
                mc->contextName);
    }
    return alloc;
}

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

extern MemContext *curMemContext;

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

void *
malloc_(size_t bytes, char *file, unsigned line)
{
    void *mem = malloc(bytes);
    if (mem == NULL)
    {
        log_(ERROR, file, line, "Fail to malloc.");
    }
    else
    {
        log_(TRACE, file, line, "%ld bytes memory @%p allocated.", bytes, mem);
    }

    addAlloc(curMemContext, mem, file, line);

    return mem;
}

void *
calloc_(size_t bytes, unsigned count, char *file, unsigned line)
{
    void *mem = calloc(count, bytes);
    if (mem == NULL)
    {
        log_(ERROR, file, line, "Fail to calloc.");
    }
    else
    {
        log_(TRACE, file, line,
                "%ldx%ld bytes memory @%p allocated and initialized with 0.",
                bytes, count, mem);
    }

    addAlloc(curMemContext, mem, file, line);

    return mem;
}

void
free_(void *mem, char *file, unsigned line)
{
    if (mem)
    {
        delAlloc(curMemContext, mem, file, line);
        free(mem);
        log_(TRACE, file, line, "Memory @%p freed.", mem);
    }
}

