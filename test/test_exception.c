/*-----------------------------------------------------------------------------
 *
 * test_exception.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "mem_manager/mem_mgr.h"
#include "exception/exception.h"
#include "test_main.h"
#include "rewriter.h"

static ExceptionHandler handleE (const char *message, const char *file, int line, ExceptionSeverity s);
static int hitCallback = 0;

static rc testCatching(void);
static rc testSignalHandling(void);

rc
testException(void)
{
    RUN_TEST(testCatching(), "test catching and handling an expection");
    RUN_TEST(testSignalHandling(), "test turning signals into exceptions");

    return PASS;
}


static ExceptionHandler
handleE (const char *message, const char *file, int line, ExceptionSeverity s)
{
    hitCallback++;
    return EXCEPTION_ABORT;
}

static rc
testCatching(void)
{
    MemContext *cur = getCurMemContext();
    MemContext *after;

    registerExceptionCallback(handleE);

    // create memory context structure
    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        NEW_AND_ACQUIRE_MEMCONTEXT("exp2");
        NEW_AND_ACQUIRE_MEMCONTEXT("exp3");
        THROW(SEVERITY_RECOVERABLE, "message");
    }
    END_TRY

    after = getCurMemContext();
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // create memory context structure
    NEW_AND_ACQUIRE_MEMCONTEXT("exp1");
    cur = getCurMemContext();
    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        NEW_AND_ACQUIRE_MEMCONTEXT("exp2");
        NEW_AND_ACQUIRE_MEMCONTEXT("exp3");
        THROW(SEVERITY_RECOVERABLE, "message");
    }
    END_TRY

    after = getCurMemContext();

    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();

    // test ON_EXCEPTION
    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
    int i = 0;

    TRY
    {
        i = 5;
        THROW(SEVERITY_RECOVERABLE, "message");
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    ASSERT_EQUALS_INT(1,i,"on exception code was executed");

    // test with rewriter
    cur = getCurMemContext();
    hitCallback = 0;

    // try query with parse error
    char *result = rewriteQuery("SELECT * FRO R;");
    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_STRING("", result, "empty string result");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // try again to check that wiping the mem context did not cause any trouble
    hitCallback = 0;
    result = rewriteQuery("SELECT * FRO R;");
    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_STRING("", result, "empty string result");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    return PASS;
}

static rc
testSignalHandling (void)
{
    MemContext *cur = getCurMemContext();
    MemContext *after;

    registerSignalHandler();
    registerExceptionCallback(handleE);
    hitCallback = 0;

    // create memory context structure
    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        Node *nPointer = NULL;
        NodeTag result = nPointer->type; // cause SIGSEGV
        DEBUG_LOG("result is %u", result);
    }
    END_TRY

    after = getCurMemContext();
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");
    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");

    return PASS;
}
