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

#include "mem_mgr.h"
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
    MemContext *testContext = newMemContext("test context");
    setCurMemContext(testContext);

    int* i = MALLOC(sizeof(int));
    *i = 6;
    assert(*i == 6);

    char *s = CALLOC(sizeof(char), 10);
    strcpy(s, "abcdefghi");
    assert(strcmp(s, "abcdefghi") == 0);

    TestStruct *ts1 = NEW(TestStruct);
    assert(ts1->a == 0);
    assert(ts1->b == NULL);
    assert(ts1->c == 0.0);

    TestStruct *ts2 = CNEW(TestStruct, 2);
    assert(ts2[1].a == 0);

    assert(memContextSize(testContext) == 4);

    FREE(ts1);
    assert(memContextSize(testContext) == 3);

    delMemContext(testContext);

    return PASS;
}

