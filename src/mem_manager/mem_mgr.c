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
#include "exception/exception.h"
#include "log/logger.h"
#include "uthash.h"
#include "instrumentation/timing_instrumentation.h"
#include "instrumentation/memory_instrumentation.h"

#define DEFAULT_MEM_CONTEXT_NAME "DEFAULT_MEMORY_CONTEXT"

// override the defaults for UT_hash memory allocation to use standard malloc
#undef uthash_malloc
#undef uthash_free
#define uthash_malloc(sz) malloc(sz)
#define uthash_free(ptr,sz) free(ptr)

// use actual malloc and free functions
#ifndef MALLOC_REDEFINED
#undef free
#undef malloc
#endif

// helper macros
#define ADD_NEWLINE(mes) ( mes "\n" )
#define EXIT_WITH_ERROR(mes) \
    do { \
    	    fprintf(stderr,(ADD_NEWLINE(mes))); \
        fflush(stderr); \
        exit(1); \
    } while(0)

#define PRINT_ERROR(mes, ...) \
    do { \
        fprintf(stderr, ADD_NEWLINE(mes), __VA_ARGS__); \
        fflush(stderr); \
    } while(0)


typedef struct MemContextNode
{
    MemContext *mc;
    struct MemContextNode *next;
} MemContextNode; // context stack node

static inline void createFirstChunk(MemContext *mc);

//static inline void addAlloc(MemContext *mc, void *addr, const char *file,
//        unsigned line);
//static inline void delAlloc(MemContext *mc, void *addr, const char *file,
//        unsigned line);
//static inline void freeAlloc (Allocation *alloc, MemContext *mc, void *addr,
//        const char *file, unsigned line);
static inline void createChunk (MemContext *mc, size_t size, const char *file,
        unsigned line);
static void errPrintMemContextStack (void);
static void printMemContextStack(FILE *o);
static char *contextStackToString(void);
static char *memContextToString(MemContext *m, boolean overviewOnly);
static void internalFreeMemContext (MemContext *m, const char *file, unsigned line);

static MemContext *curMemContext = NULL; // global pointer to current memory context
static MemContext *defaultMemContext = NULL;
static MemContextNode *topContextNode = NULL;
static int contextStackSize = 0;
static boolean destroyed = FALSE;
static boolean initialized = FALSE;

struct mem_manager
{
    MemContext *curMemContext;
    MemContext *defaultContext;
    MemContextNode *topContextNode;
    int contextStackSize;
    boolean destroyed;
    boolean initialized;
};

/*
 * Creates default memory context and pushes it into context stack.
 */
void
initMemManager(void)
{
    if (initialized)
    {
        EXIT_WITH_ERROR("trying to initialize memory manager twice");
    }
    if (destroyed)
    {
        EXIT_WITH_ERROR("trying to initialize memory manager after it was destroyed");
    }
    defaultMemContext = NEW_MEM_CONTEXT(DEFAULT_MEM_CONTEXT_NAME);
    initialized = TRUE;
    ACQUIRE_MEM_CONTEXT(defaultMemContext);
    // default context always lies on the bottom of the stack
}

/*
 * Free all contexts in the context stack and clear the stack.
 */
void
destroyMemManager(void)
{
    if (!initialized)
    {
        EXIT_WITH_ERROR("trying to destroy memory manager that was not initialized yet");
    }
    if (destroyed)
    {
        EXIT_WITH_ERROR("trying to destroy memory manager after it was destroyed");
    }

    while (topContextNode->next)
    {
        FREE_CUR_MEM_CONTEXT();
        RELEASE_MEM_CONTEXT();
    }
    // free all contexts in the context stack except default context

//    ASSERT(topContextNode->mc == defaultMemContext);
//    ASSERT(curMemContext == defaultMemContext);
//    int size = memContextSize(defaultMemContext);
//    if (size > 0)
//    {
//        CLEAR_CUR_MEM_CONTEXT();
//        free(defaultMemContext);
//    }
//    else if (size == 0)
//    {
//        free(defaultMemContext);
//    }
    // free default context
    internalFreeMemContext(defaultMemContext, __FILE__, __LINE__);
    free(topContextNode); // free default context node
//    DEBUG_LOG("Freed memory context '%s'.", DEFAULT_MEM_CONTEXT_NAME);
    destroyed = TRUE;
}

boolean
memManagerUsable(void)
{
    if (destroyed || !initialized)
        return FALSE;
    return TRUE;
}

/*
 * Sets current context and pushes it into context stack.
 */
void
setCurMemContext(MemContext *mc, const char *file, unsigned line)
{
	if(opt_memmeasure)
	{
		fprintf(stderr,"*********************\nACQUIRE CONTEXT [%s] at %s:%d\n********************\n",
				curMemContext->contextName,
				file,
				line);
	}

    if (topContextNode != NULL && mc == topContextNode->mc)
        return;

    // check that context is not already on the stack which may lead to unrecoverable errors
    MemContextNode *el = topContextNode;

    while(el != NULL)
    {
        if (el->mc == mc)
        {
            MemContext *tempContext = NEW_MEM_CONTEXT("TEMP_MEM_MGR_UTIL_CONTEXT");
            curMemContext=tempContext;
            ERROR_LOG("mem context already on stack %s\\n\\n%s", mc->contextName, contextStackToString());
//            THROW(SEVERITY_PANIC, "memory context %s is already on the stack", mc->contextName);
            internalFreeMemContext(tempContext, __FILE__, __LINE__);
        }
        el = el->next;
    }

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

        if (opt_memmeasure)
            fprintf(stderr,"*********************\nACQUIRE %s\n********************\n%s", curMemContext->contextName, dumpMemContexInfo());
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
	if(opt_memmeasure && topContextNode)
	{
		fprintf(stderr,"*********************\nRELEASE CONTEXT [%s] at %s:%u\n********************\n",
				topContextNode->mc->contextName,
				file,
				line);
	}
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
//static void
//addAlloc(MemContext *mc, void *addr, const char *file, unsigned line)
//{
//    Allocation *newAlloc = (Allocation *) malloc(sizeof(Allocation));
//    ASSERT(mc != NULL);
//    newAlloc->address = addr;
//    newAlloc->file = file;
//    newAlloc->line = line;
//    HASH_ADD_PTR(mc->hashAlloc, address, newAlloc); // add to hash table. Use address as key
//
//    GENERIC_LOG(LOG_TRACE, file, line,
//        "Added [addr:%p, file:'%s', line:%u] to memory context '%s'.", addr,
//        file, line, mc->contextName);
//}

/*
 * Removes memory allocation information from a memory context.
 */
//static void
//delAlloc(MemContext *mc, void *addr, const char *file, unsigned line)
//{
//    Allocation *alloc = NULL;
//    HASH_FIND_PTR(mc->hashAlloc, &addr, alloc); // find allocation info by address first
//    if (alloc)
//    {
//        freeAlloc(alloc, mc, addr, file, line);
//        return;
//    }
//    // allocation not found search in ancestor contexts on the stack
//    else
//    {
//        MemContextNode *c = topContextNode;
//        MemContext *curC;
//
//        GENERIC_LOG(LOG_TRACE, file, line,
//                "Not found [addr:%p] from memory context '%s'.",
//                            addr, mc->contextName);
//
//        while(c->mc != mc)
//            c = c->next;
//        c = c->next;
//
//        for(; c != NULL; c = c->next)
//        {
//            curC = c->mc;
//            HASH_FIND_PTR(curC->hashAlloc, &addr, alloc);
//
//            if (alloc)
//            {
//                freeAlloc(alloc, curC, addr, file, line);
//                return;
//            }
//            else
//                GENERIC_LOG(LOG_TRACE, file, line,
//                        "Not found [addr:%p] from memory context '%s'.",
//                        addr, curC->contextName);
//       }
//    }
//
//    // did not find allocation
//    FATAL_LOG("did not find allocation info for <%p> in memory context <%s>", addr, mc->contextName);
//}

/*
 * Free an allocation
 */

//static void
//freeAlloc (Allocation *alloc, MemContext *mc, void *addr, const char *file, unsigned line)
//{
//    const char *tmpfile = alloc->file;
//    int tmpline = alloc->line;
//    HASH_DEL(mc->hashAlloc, alloc); // remove the allocation info
////    free(alloc);
//    GENERIC_LOG(LOG_TRACE, file, line,
//            "Deleted [addr:%p, file:'%s', line:%d] from memory context '%s'.",
//            addr, tmpfile, tmpline, mc->contextName);
//}


char *
dumpMemContexInfo (void)
{
    StringInfo result = makeStringInfo();
    MemContextNode *cur = topContextNode;

    appendStringInfo(result, "DEFAULT CONTEXT - %s\n", memContextToString(defaultMemContext, TRUE));

    // search for memcontext with the given name
    for(;cur != NULL; cur = cur->next)
    {
        appendStringInfoString(result, memContextToString(cur->mc, TRUE));
    }

    return result->data;
}

static void
errPrintMemContextStack (void)
{
    printMemContextStack(stderr);
}

static void
printMemContextStack(FILE *o)
{
    MemContextNode *c = topContextNode;
    int i = 0;

    for(;c != NULL; c = c->next)
    {
        fprintf(o, "%u - %s", i++, memContextToString(c->mc, TRUE));
    }
    fflush(o);
}

static char *
contextStackToString(void)
{
    StringInfo str = makeStringInfo();
    char *result;
    MemContextNode *c = topContextNode;
    int i = 0;

    for(;c != NULL; c = c->next)
    {
        appendStringInfo(str, "[%u] - %s", i++, memContextToString(c->mc, FALSE));
    }

    result = str->data;
    return result;
}

static char *
memContextToString(MemContext *m, boolean overviewOnly)
{
    StringInfo result = makeStringInfo();

    appendStringInfo (result, "Context[%s %p] - numChunks: %u - BytesInCurChunk: %u - LongLived: %u\n",
            m->contextName, m, m->numChunks, m->memLeftInChunk, m->longLived);

    if (!overviewOnly)
    {
        for(int i = 0; i < m->numChunks; i++)
        {
            appendStringInfo(result, "[%u] ", m->chunkSizes[i]);
        }
        appendStringInfoString(result, "\n");
    }
    return result->data;
}

/*
 * Creates a memory context.
 */
MemContext *
newMemContext(char *contextName, const char *file, unsigned line, boolean longLived)
{
    MemContext *mc = (MemContext *) malloc(sizeof(MemContext));
    mc->contextName = contextName;
    mc->hashAlloc = NULL;
    mc->chunks = (char **) malloc(INIT_CHUNK_ARRAY_SIZE * sizeof(char*));
    mc->chunkSizes = (unsigned long*) malloc(INIT_CHUNK_ARRAY_SIZE * sizeof(unsigned long));
    mc->curChunkArraySize = INIT_CHUNK_ARRAY_SIZE;
    mc->numChunks = 1;
    mc->unusedBytes = 0;
    mc->freedUnusedBytes = 0;
    mc->longLived = longLived;

    /* create first chunk */
    createFirstChunk(mc);

    GENERIC_LOG(LOG_DEBUG, file, line, "Created memory context '%s'.",
            mc->contextName);
    //fprintf(stderr,"%s", dumpMemContexInfo());
    return mc;
}

static inline void
createFirstChunk(MemContext *mc)
{
    mc->chunks[0] = (char *) malloc(DEFAULT_CHUNK_SIZE);
    mc->chunkSizes[0] = DEFAULT_CHUNK_SIZE;
    mc->curAllocPos = mc->chunks[0];
    mc->memLeftInChunk = DEFAULT_CHUNK_SIZE;
    mc->numChunks = 1;

    // inform memdebug of new chunk if activated
     if (opt_memmeasure && !streq(curMemContext->contextName,MEMDEBUG_CONTEXT_NAME))
     {
         addContextChunkInfo(curMemContext->contextName, DEFAULT_CHUNK_SIZE);
     }
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
 * and free those memory chunks. Will not destroy the memory context itself.
 */
void
clearCurMemContext(const char *file, unsigned line)
{
    clearAMemContext(curMemContext, file, line);
}

/*
 * Removes all the memory allocation records from the current context
 * and free those memory chunks. Will not destroy the memory context itself.
 */
void
clearAMemContext(MemContext *c, const char *file, unsigned line)
{
    Allocation *curAlloc, *tmp;
    HASH_ITER(hh, c->hashAlloc, curAlloc, tmp)
    {
        free_(curAlloc->address, file, line);
    }
    for(int i = 0; i < c->numChunks; i++)
    {
        free(c->chunks[i]);
        c->chunks[i] = NULL;
    }
    c->numChunks = 0;
    c->memLeftInChunk = 0;
}

/*
 * Removes all the memory allocation records from the current context
 * and free those memories and finally destroy the memory context itself.
 */
void
freeCurMemContext(const char *file, unsigned line)
{
    if (opt_memmeasure)
        fprintf(stderr,"*********************\nFREE %s\n********************\n%s", curMemContext->contextName, dumpMemContexInfo());

    if (topContextNode->next) // does not free default context and its node
    {
        internalFreeMemContext(curMemContext, file, line);
    }
}

static void
internalFreeMemContext (MemContext *m, const char *file, unsigned line)
{
    int size = memContextSize(m);
    char *name = m->contextName;
    if (size > 0)
        clearAMemContext(m, file, line);
    free(m->chunks);
    free(m->chunkSizes);
    free(m);
    m = NULL;
    GENERIC_LOG(LOG_DEBUG, file, line, "Freed memory context '%s'.", name);
}

/*
 * Free a memory context and its children. The context has to be on the stack.
 * Children that are long lived contexts are not free'd.
 * Returns and acquires the parent of the memory context.
 */
MemContext *
freeMemContextAndChildren(char *contextName)
{
    MemContextNode *cur = topContextNode;
    boolean found = FALSE;
    char *curName = NULL;

    if (opt_memmeasure)
        fprintf(stderr,"%s", dumpMemContexInfo());

    // search for memcontext with the given name
    for(;cur != NULL; cur = cur->next)
    {
        if (streq(cur->mc->contextName,contextName))
        {
            found = TRUE;
            break;
        }
    }

    // there is not much hope to recover here
    if (!found)
    {
        fprintf(stderr, "trying to free memory context that currently not on the stack %s\n", contextName);
        errPrintMemContextStack();
        fflush(stderr);
        exit(1);
    }

    // free all children and requested memory context
    do
    {
        // do not free long lived contexts like the metadatalook one which hold information that should survive a wipe
        if (curMemContext->longLived)
        {
            if (curName)
                free(curName);
            curName = malloc(strlen(curMemContext->contextName) + 1);
            strcpy(curName,curMemContext->contextName);
            RELEASE_MEM_CONTEXT();
        }
        else
        {
            // cannot use memory manager MALLOC here
            if (curName)
                free(curName);
            curName = malloc(strlen(curMemContext->contextName) + 1);
            strcpy(curName,curMemContext->contextName);
            FREE_AND_RELEASE_CUR_MEM_CONTEXT();
        }
        cur = topContextNode;
    } while(!streq(curName,contextName));
    if (curName)
        free(curName);

    INFO_LOG("now in context %s with stack:\n\n%s",
            curMemContext->contextName, contextStackToString());
//    printMemContextStack(stderr);

    return cur->mc;
}

/*
 * Adds a new chunk of at least requested size to mem context
 */

static inline void
createChunk (MemContext *mc, size_t size, const char *file, unsigned line)
{
    size_t actualSize = (size < DEFAULT_CHUNK_SIZE) ?
            DEFAULT_CHUNK_SIZE :
            size;
    void *mem;
    unsigned long int unused = mc->memLeftInChunk;
    mc->unusedBytes += unused;

    // round up to multiple of default chunk size
    if (size % DEFAULT_CHUNK_SIZE != 0)
    {
        size += (DEFAULT_CHUNK_SIZE - (size % DEFAULT_CHUNK_SIZE));
    }
    mem = malloc(actualSize);

    if (mem == NULL)
    {
        GENERIC_LOG(LOG_FATAL, file, line, "Fail to malloc.");
    }
    else
    {
        GENERIC_LOG(LOG_TRACE, file, line, "%ld bytes memory @%p allocated.", size,
                mem);
    }

    // inform memdebug of new chunk if activated
    if (opt_memmeasure && !streq(curMemContext->contextName,MEMDEBUG_CONTEXT_NAME))
    {
        addContextUnused(curMemContext->contextName, unused);
        addContextChunkInfo(curMemContext->contextName, actualSize);
    }

    INFO_LOG("Create chunk of size %d in context %s", actualSize, mc->contextName);

    // double chunk array size
    if (mc->numChunks == mc->curChunkArraySize)
    {
        char **oldChunks = mc->chunks;
        unsigned long *oldChunkSizes = mc->chunkSizes;
        unsigned int newSize;
        unsigned int oldSize = mc->curChunkArraySize;

        mc->curChunkArraySize *= 2;
        newSize = mc->curChunkArraySize;
        mc->chunks = malloc(sizeof(char *) * newSize);
        mc->chunkSizes = malloc(sizeof(unsigned long) * newSize);

        // copy old arrays
        for(int i = 0; i < oldSize; i++)
        {
            mc->chunks[i] = oldChunks[i];
            mc->chunkSizes[i] = oldChunkSizes[i];
        }
    }

    unsigned int numChunks = mc->numChunks;
    mc->chunks[numChunks] = mem;
    mc->chunkSizes[numChunks] = actualSize;
    mc->curAllocPos = mc->chunks[numChunks];
    mc->numChunks++;
    mc->memLeftInChunk = actualSize;
}

/*
 * Allocates memory and records it in the current memory context.
 */
void *
malloc_(size_t bytes, const char *file, unsigned line)
{
    MemContext *c = curMemContext;

    if (!initialized)
    {
        PRINT_ERROR("trying to allocate %u at %s:%u", (unsigned int) bytes, file, line);
        EXIT_WITH_ERROR("trying to allocate memory before mem manager has been initialized");
    }
    if (destroyed)
    {
        PRINT_ERROR("trying to allocate %u at %s:%u", (unsigned int) bytes, file, line);
        EXIT_WITH_ERROR("trying to allocate memory after mem manager has been destroyed");
    }

    // if there is not enough memory in current chunk then allocate new chunk
    if(c->memLeftInChunk < bytes)
        createChunk(c, bytes, file, line);


    void *mem = c->curAllocPos;
    c->curAllocPos += bytes;
    c->memLeftInChunk -= bytes;
//    memset(mem, 178, bytes);
//    if (mem == NULL)
//    {
//        GENERIC_LOG(LOG_FATAL, file, line, "Fail to malloc.");
//    }
//    else
//    {
//        GENERIC_LOG(LOG_TRACE, file, line, "%ld bytes memory @%p allocated.", bytes,
//                mem);
//    }

    // track mem allocations, but not of the context used for memory debugging
    if (opt_memmeasure && !streq(curMemContext->contextName,MEMDEBUG_CONTEXT_NAME))
        addContext(curMemContext->contextName, bytes, FALSE);

//    addAlloc(curMemContext, mem, file, line);

    return mem;
}

/*
 * Allocates memory and initializes it with 0 and records it in the current
 * memory context.
 */
void *
calloc_(size_t bytes, unsigned count, const char *file, unsigned line)
{
    size_t allocSize = bytes * count;
    MemContext *c = curMemContext;
    void *mem; // = calloc(count, bytes);

    // if there is not enough memory in current chunk then allocate new chunk
    if(c->memLeftInChunk < allocSize)
        createChunk(c, allocSize, file, line);

    mem = c->curAllocPos;
    memset(c->curAllocPos,0,allocSize);
    c->curAllocPos += allocSize;
    c->memLeftInChunk -= allocSize;


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
        addContext(curMemContext->contextName, allocSize, FALSE);

//    addAlloc(curMemContext, mem, file, line);

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
//        delAlloc(curMemContext, mem, file, line);
//        free(mem);
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

MemContext *
getDefaultMemContext(void)
{
    return defaultMemContext;
}
