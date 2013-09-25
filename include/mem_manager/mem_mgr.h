/*-------------------------------------------------------------------------
 *
 * mem_mgr.h
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    This module is developed to provide memory management that organizes
 *    and reduces the work of allocating and freeing memory.
 *
 *        A memory context can be created to record the allocated memories
 *        and be destroyed to batch free all the memories recorded in it.
 *        The allocated memories information can be traced in logs.
 *
 *        1. Create a memory context:
 *        MemContext *context = newMemContext("Context Name");
 *        The global variable 'curMemContext' is by default set to the latest
 *        created context.
 *
 *        2. Use one of the following macros to allocate memory:
 *        MALLOC, CALLOC, NEW and CNEW.
 *        The allocated memory will be remembered by the current memory context.
 *        NEW and CNEW is recommended to make the code clean.
 *        e.g.,
 *        int *i = MALLOC(sizeof(int));
 *        char *s = MALLOC(100);
 *        char *s = CALLOC(sizeof(char), 100); // *s initialized to an empty string
 *        int *i = NEW(int); // *i is initialized to 0
 *        typedef struct MyStruct{...} MyStruct;
 *        MyStruct *t = NEW(MyStruct); // members of MyStruct initialized to 0
 *        int *i = CNEW(int, 10); // allocates an integer array with length 10.
 *        MyStruct *t = CNEW(MyStruct, 5); // allocates a struct array with length 5.
 *
 *        3. Use setCurMemContext(MemContext *mc) to switch among multiple memory
 *        contexts. And get the current memory context by invoking getCurMemContext().
 *
 *        4. Freeing allocated memories one by one is NOT necessary. Simply
 *        invoking freeMemContext(MemContext *mc) at the end will free all the
 *        allocated memory recorded by the specified memory context. For example:
 *        MemContext *context = newMemContext("Context Name");
 *        ...
 *        ...MALLOC(...); // line 21
 *        ...NEW(...); // line 22
 *        ...
 *        freeMemContext(context);
 *        will free the memory allocated by the code at line 21 and 22.
 *
 *        5. Please be careful of switching memory context. For example:
 *        MemContext *context1 = newMemContext("Context Name1");
 *        MemContext *context2 = newMemContext("Context Name2");
 *        ...
 *        ...NEW(...); // line 21
 *        ...CNEW(...); // line 22
 *        ...
 *        setCurMemContext(context1);
 *        ...
 *        ...NEW(...); // line 33
 *        ...NEW(...); // line 34
 *        ...
 *        freeMemContext(context1);
 *        freeMemContext(context2);
 *        The memories allocated at line 21 and 22 are recorded by context2
 *        and the memories allocated at line 33 and 34 are recorded by context1.
 *
 *-------------------------------------------------------------------------
 */

#ifndef MEM_MGR_H_
#define MEM_MGR_H_

#include <stdlib.h>
#include "common.h"
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

extern void initMemManager(void);
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
#define NEW(type) CALLOC(sizeof(type), 1)
/*
 * Allocates an array of the specified data type.
 */
#define CNEW(type, count) CALLOC(sizeof(type), (count))
/*
 * Removes the specified memory allocation record from the current memory context
 * and then free the memory at the address.
 */
#define FREE(pointer) free_((void *) (pointer), __FILE__, __LINE__)

/*
 * Creates a new memory context.
 */
extern MemContext *newMemContext(char *contextName, const char *file, unsigned line);
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
 * Switches current memory context to another one.
 */
extern void setCurMemContext(MemContext *mc, const char *file, unsigned line);
extern MemContext *getCurMemContext(void);
/*
 * Removes all the memory allocation records from the specified memory context
 * and free those memories. Will not destroy the memory context itself.
 */
extern void clearCurMemContext(const char *file, unsigned line);
/*
 *
 */
extern void releaseCurMemContext(const char *file, unsigned line);
/*
 * Removes all the memory allocation records from the specified memory context
 * and free those memories and finally destroy the memory context.
 */
extern void freeCurMemContext(const char *file, unsigned line);

#define NEW_MEM_CONTEXT(name) newMemContext((name), __FILE__, __LINE__)
#define AQUIRE_MEM_CONTEXT(context) setCurMemContext((context), __FILE__, __LINE__)
#define CLEAR_CUR_MEM_CONTEXT() clearCurMemContext(__FILE__, __LINE__)
#define FREE_CUR_MEM_CONTEXT() freeCurMemContext(__FILE__, __LINE__)
#define RELEASE_MEM_CONTEXT() releaseCurMemContext(__FILE__, __LINE__)

#endif
