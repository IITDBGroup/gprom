/*-------------------------------------------------------------------------
 *
 * mem_mgr.h
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#ifndef MEM_MGR_H_
#define MEM_MGR_H_

#include <stdlib.h>
#include "Common.h"
#include "uthash.h"

typedef struct Allocation
{
    void *address;
    char *file;
    int line;
    UT_hash_handle hh;
} Allocation;

typedef struct MemContext
{
    char *contextName;
    Allocation *hashAlloc;
} MemContext;

MemContext *curMemContext;

extern void *
malloc_(size_t bytes, char *file, unsigned line);
extern void *
calloc_(size_t bytes, unsigned count, char *file, unsigned line);
extern void
free_(void *mem, char *file, unsigned line);

#define MALLOC(bytes) malloc_((bytes), __FILE__, __LINE__)
#define CALLOC(bytes, count) calloc_((bytes), (count), __FILE__, __LINE__)
#define NEW(type) CALLOC(sizeof(type), 1)
#define CNEW(type, count) CALLOC(sizeof(type), (count))
#define FREE(pointer) free_((void *) (pointer), __FILE__, __LINE__)

extern MemContext *
newMemContext(char *contextName);
extern int
memContextSize(MemContext *mc);
extern Allocation *
findAlloc(const MemContext *mc, const void *addr);
extern void
delMemContext(MemContext *mc);
extern void
setCurMemContext(MemContext *mc);
extern MemContext *
getCurMemContext(void);

#endif
