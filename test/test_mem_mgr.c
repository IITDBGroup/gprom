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

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "mem_manager/mem_mgr.h"
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
    MemContext *oldContext = getCurMemContext();
    MemContext *context1 = newMemContext("test context 1");
    MemContext *context2 = newMemContext("test context 2");

    // test MALLOC
    int* i = MALLOC(sizeof(int));
    *i = 6;
    ASSERT_EQUALS_INT(6,*i,"allocated int is correct");

    // test CALLOC
    char *s = CALLOC(sizeof(char), 10);
    strcpy(s, "abcdefghi");
    ASSERT_EQUALS_STRING("abcdefghi",s,"allocated string is correct");

    // test NEW
    TestStruct *ts1 = NEW(TestStruct);
    ASSERT_EQUALS_INT(0, ts1->a, "ts1->a is 0");
    ASSERT_EQUALS_P(NULL,ts1->b, "ts1->b is NULL");
    ASSERT_EQUALS_FLOAT(0.0,ts1->c, "ts1->c is 0.0");

    // test CNEW
    TestStruct *ts2 = CNEW(TestStruct, 2);
    ASSERT_EQUALS_INT(0,ts2[1].a, "ts2[1].a is 0");
    ASSERT_EQUALS_FLOAT(0.0, ts2[1].c, "ts2[1].c is 0.0");

    // test memory context size
    ASSERT_EQUALS_INT(4,memContextSize(context2), "context2 size is 4");
    FREE(ts1);
    ASSERT_EQUALS_INT(3,memContextSize(context2), "context2 size is now 3");

    // test clearing memory context
    clearMemContext(context2);
    ASSERT_EQUALS_INT(0,memContextSize(context2), "context2 size is now 0");

    // test switching memory context
    setCurMemContext(context1);
    i = NEW(int);
    ts1 = CNEW(TestStruct, 5);
    ts2 = NEW(TestStruct);
    ASSERT_EQUALS_INT(3,memContextSize(context1), "context1 size is now 3");
    freeMemContext(context1);

    freeMemContext(context2);
    setCurMemContext(oldContext);

    return PASS;
}

