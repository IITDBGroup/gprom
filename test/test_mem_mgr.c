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

static MemContext *context1;

static rc module_Func1(void);
static rc module_Func2(void);

static rc testCreationAndSize(void);
static rc testFreeContextAndChildren(void);

rc
testMemManager(void)
{
    RUN_TEST(testCreationAndSize(), "creation and memory context size");
    RUN_TEST(testFreeContextAndChildren(), "free a context and its children");

    return PASS;
}

static rc
testCreationAndSize(void)
{
    MemContext *context2 = NEW_MEM_CONTEXT("TEST_CONTEXT_2");

    ACQUIRE_MEM_CONTEXT(context2);

    // test MALLOC
    int* i = MALLOC(sizeof(int));
    *i = 6;
    ASSERT_EQUALS_INT(6,(*i),"allocated int is correct");

    // test CALLOC
    char *s = CALLOC(sizeof(char), strlen("abcdefghi") + 1);
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
    CLEAR_CUR_MEM_CONTEXT();
    ASSERT_EQUALS_INT(0,memContextSize(context2), "context2 size is now 0");

    module_Func1();

    FREE_CUR_MEM_CONTEXT();
    RELEASE_MEM_CONTEXT();

    module_Func2();

    return PASS;
}

static rc
testFreeContextAndChildren(void)
{
    MemContext *def;
    MemContext *c;

    def = getCurMemContext();

    NEW_AND_ACQUIRE_MEMCONTEXT("grandpa");
    NEW_AND_ACQUIRE_MEMCONTEXT("pa");
    NEW_AND_ACQUIRE_MEMCONTEXT("child");

    c = freeMemContextAndChildren("grandpa");

    ASSERT_EQUALS_STRINGP(def->contextName, c->contextName, "should be back to default memcontext");

    return PASS;
}


static int
module_Func1(void)
{
    context1 = NEW_MEM_CONTEXT("TEST_CONTEXT_1");
    ACQUIRE_MEM_CONTEXT(context1); // switch to context1

    int *x = NEW(int);
    *x = 5;
    ASSERT_EQUALS_INT(5,*x,"is 5");
    TestStruct *t = CNEW(TestStruct, 5);
    t = NEW(TestStruct);
    t->a = 5;
    ASSERT_EQUALS_INT(5,t->a,"is 5");
    ASSERT_EQUALS_INT(3,memContextSize(context1), "context1 size is now 3");

    RELEASE_MEM_CONTEXT(); // set back curMemContext to previous one. context1 still exists.

    return PASS;
}

static int
module_Func2(void)
{
    ACQUIRE_MEM_CONTEXT(context1); // switch to context1

    ASSERT_EQUALS_INT(3,memContextSize(context1), "context1 size is still 3");
    FREE_CUR_MEM_CONTEXT(); // free context1

    RELEASE_MEM_CONTEXT(); // set back curMemContext to previous one.

    return PASS;
}
