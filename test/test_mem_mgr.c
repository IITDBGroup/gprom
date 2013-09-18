/*-------------------------------------------------------------------------
 *
 * test_mem_mgr.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "mem_manager/mem_mgr.h"
#include "assert.h"
#include "string.h"
#include <stdio.h>
#include "test_main.h"

typedef struct TestStruct
{
    int a;
    char *b;
    float c;
} TestStruct;

rc
testMemManager(void)
{
    MemContext *context1 = newMemContext("test context 1");
    MemContext *context2 = newMemContext("test context 2");

    // test MALLOC
    int* i = MALLOC(sizeof(int));
    *i = 6;
    assert(*i == 6);

    // test CALLOC
    char *s = CALLOC(sizeof(char), 10);
    strcpy(s, "abcdefghi");
    assert(strcmp(s, "abcdefghi") == 0);

    // test NEW
    TestStruct *ts1 = NEW(TestStruct);
    assert(ts1->a == 0);
    assert(ts1->b == NULL);
    assert(ts1->c == 0.0);

    // test CNEW
    TestStruct *ts2 = CNEW(TestStruct, 2);
    assert(ts2[1].a == 0);
    assert(ts2[1].c == 0.0);

    // test memory context size
    assert(memContextSize(context2) == 4);
    FREE(ts1);
    assert(memContextSize(context2) == 3);

    // test clearing memory context
    clearMemContext(context2);
    assert(memContextSize(context2) == 0);

    // test switching memory context
    setCurMemContext(context1);
    i = NEW(int);
    ts1 = CNEW(TestStruct, 5);
    ts2 = NEW(TestStruct);
    assert(memContextSize(context1) == 3);
    freeMemContext(context1);

    freeMemContext(context2);

    return PASS;
}

