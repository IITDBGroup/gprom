/*-------------------------------------------------------------------------
 *
 * mem_mgr.h
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    This module is to provide memory management tool that organizes
 *    and reduces the work of allocating and freeing memory.
 *
 *        A memory context can be created to record the allocated memories
 *        and be destroyed to batch free all the memories recorded in it.
 *        The allocated memories information can be traced in logs.
 *
 *        AQUIRE_MEM_CONTEXT(yourContext) MUST be invoked before yourContext
 *        is used.
 *        RELEASE_MEM_CONTEXT() and AQUIRE_MEM_CONTEXT(yourContext) MUST
 *        appear in pair.
 *
 *        Examples:
 *        1.
 *        MemContext *mc = NEW_MEM_CONTEXT("name");
 *        AQUIRE_MEM_CONTEXT(mc);
 *        NEW(int);
 *        NEW(MyStructType);
 *        CNEW(MyStructType, 10);
 *        ...
 *        RELEASE_MEM_CONTEXT();
 *
 *        2.
 *        AQUIRE_MEM_CONTEXT(yourContext);
 *        int size = memContextSize(yourContext);
 *        ...
 *        FREE_CUR_MEM_CONTEXT();
 *        RELEASE_MEM_CONTEXT();
 *
 *-------------------------------------------------------------------------
 */

#ifndef MEM_MGR_H_
#define MEM_MGR_H_

#include "uthash.h"

typedef struct Allocation
{
    void *address; // the allocated memory address
    const char *file; // the file where the allocating code is
    int line; // the line at which the allocating code is
    UT_hash_handle hh;
} Allocation;

typedef struct MemContext
{
    char *contextName;
    Allocation *hashAlloc;
} MemContext;

/*
 * Creates default memory context and pushes it into context stack.
 */
extern void initMemManager(void);
/*
 * Free all contexts in the context stack and clear the stack.
 */
extern void destroyMemManager(void);

extern void *malloc_(size_t bytes, const char *file, unsigned line);
extern void *calloc_(size_t bytes, unsigned count, const char *file, unsigned line);
extern void free_(void *mem, const char *file, unsigned line);

/*
 * Is similar to malloc(size) but will also record the allocated memory
 * information in the memory context pointed by 'curMemContext'.
 */
#define MALLOC(bytes) malloc_((bytes), __FILE__, __LINE__)
/*
 * Is similar to calloc(count, size) but will also record the allocated memory
 * information in the memory context pointed by 'curMemContext'.
 */
#define CALLOC(bytes, count) calloc_((bytes), (count), __FILE__, __LINE__)
/*
 * Allocates memory for the specified data type and initialize the data of
 * the type to 0.
 */
#define NEW(type) (type *) CALLOC(sizeof(type), 1)
/*
 * Allocates an array of the specified data type.
 */
#define CNEW(type, count) CALLOC(sizeof(type), (count))
/*
 * Removes the specified memory allocation record from the current memory context
 * and then free the memory at the address.
 */
#define FREE(pointer) free_((void *) (pointer), __FILE__, __LINE__)
#define FREE_IN_CONTEXT(context, pointer) \
    do { \
        ACQUIRE_MEM_CONTEXT(context); \
        free_((void *) (pointer), __FILE__, __LINE__); \
        RELEASE_MEM_CONTEXT(); \
    } while(0)

extern MemContext *newMemContext(char *contextName, const char *file, unsigned line);
extern void setCurMemContext(MemContext *mc, const char *file, unsigned line);
extern MemContext *getCurMemContext(void);
extern void clearCurMemContext(const char *file, unsigned line);
extern MemContext *releaseCurMemContext(const char *file, unsigned line);
extern void freeCurMemContext(const char *file, unsigned line);
extern char *contextStringDup(char *input);

/*
 * Gets context size.
 */
extern int memContextSize(MemContext *mc);
/*
 * Finds memory allocation record in the memory context by address.
 * Returns NULL if not found.
 */
extern Allocation *findAlloc(const MemContext *mc, const void *addr);

/*
 * Sets current context and pushes it into context stack.
 */
#define ACQUIRE_MEM_CONTEXT(context) setCurMemContext((context), __FILE__, __LINE__)
/*
 * Creates a memory context. The second version also aquires the new context.
 */
#define NEW_MEM_CONTEXT(name) newMemContext((name), __FILE__, __LINE__)
#define NEW_AND_ACQUIRE_MEMCONTEXT(name) \
    do { \
        MemContext *_newcontext_ = NEW_MEM_CONTEXT(name); \
        ACQUIRE_MEM_CONTEXT(_newcontext_); \
    } while (0);
/*
 * Removes all the memory allocation records from the current context
 * and free those memories. Will not destroy the memory context itself.
 */
#define CLEAR_CUR_MEM_CONTEXT() clearCurMemContext(__FILE__, __LINE__)
/*
 * Pops current context and returns to the previous context. Will not free
 * the current context.
 */
#define RELEASE_MEM_CONTEXT() releaseCurMemContext(__FILE__, __LINE__)
/*
 * Release the current memory context, copy the data structure _node
 * to the callers memory context, free _node, and return.
 */
#define RELEASE_MEM_CONTEXT_AND_RETURN_COPY(_type, _node) \
    do { \
    	_type *_resultNode; \
    	_type *_origNode = (_type *) _node; \
    	MemContext *oldC = RELEASE_MEM_CONTEXT(); \
    	_resultNode = (_type *) copyObject(_origNode); \
    	ASSERT(equal(_resultNode,_origNode)); \
    	FREE_IN_CONTEXT(oldC, _origNode); \
    	return (_type *) _resultNode; \
    } while(0)
#define RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(_str) \
    do { \
        char *_resultStr; \
        char *_origStr = (char *) _str; \
        MemContext *oldC = RELEASE_MEM_CONTEXT(); \
        _resultStr = strdup(_origStr); \
        FREE_IN_CONTEXT(oldC, _origStr); \
        return _resultStr; \
    } while(0)


/*
 * Copy _node to callers memory context and free and release the current
 *  memory context.
 */
#define FREE_MEM_CONTEXT_AND_RETURN_COPY(_type, _node) \
    do { \
        _type *_resultNode; \
        _type *_origNode = (_type *) (_node); \
        MemContext *oldC = RELEASE_MEM_CONTEXT(); \
        _resultNode = (_type *) copyObject(_origNode); \
        ASSERT(equal(_resultNode,_origNode)); \
        FREE_MEM_CONTEXT(oldC); \
        return (_type *) _resultNode; \
    } while(0)
#define FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(_str) \
    do { \
        char *_resultStr; \
        char *_origStr = (char *) _str; \
        MemContext *oldC = RELEASE_MEM_CONTEXT(); \
        _resultStr = strdup(_origStr); \
        FREE_MEM_CONTEXT(oldC); \
        return _resultStr; \
    } while(0)
/*
 * Removes all the memory allocation records from the current context
 * and free those memories and finally destroy the memory context itself.
 * The second version also releases the context.
 */
#define FREE_CUR_MEM_CONTEXT() freeCurMemContext(__FILE__, __LINE__)
#define FREE_AND_RELEASE_CUR_MEM_CONTEXT() \
    do { \
        FREE_CUR_MEM_CONTEXT(); \
        RELEASE_MEM_CONTEXT(); \
    } while(0)

#define FREE_MEM_CONTEXT(context) \
    do { \
        ACQUIRE_MEM_CONTEXT(context); \
        FREE_AND_RELEASE_CUR_MEM_CONTEXT(); \
    } while (0)

#endif
