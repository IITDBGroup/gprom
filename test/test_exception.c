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
static ExceptionHandler handleWipe (const char *message, const char *file, int line, ExceptionSeverity s);
static int hitCallback = 0;

static rc testCatching(void);
static rc testCatchingWipe(void);
static rc testSignalHandling(void);

static void throwsException(void);
static void throwsExceptionNested(void);

rc
testException(void)
{
    RUN_TEST(testCatching(), "test catching and handling an expection");
    RUN_TEST(testCatchingWipe(), "test catching and handling an expection");
    RUN_TEST(testSignalHandling(), "test turning signals into exceptions");

    return PASS;
}


static ExceptionHandler
handleE (const char *message, const char *file, int line, ExceptionSeverity s)
{
    hitCallback++;
    return EXCEPTION_ABORT;
}

static ExceptionHandler
handleWipe (const char *message, const char *file, int line, ExceptionSeverity s)
{
    hitCallback++;
    return EXCEPTION_WIPE;
}

static rc
testCatching(void)
{
    volatile MemContext * volatile cur = getCurMemContext();
    volatile MemContext * volatile after;

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
    volatile int i = 0;

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
    volatile char * volatile result = rewriteQuery("SELECT * FRO R;");
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

    // test rethrow from within other function
    hitCallback = 0;
    cur = getCurMemContext();
    i = -1;

    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        i = 5;
        throwsException();
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // test multilevel rethrow
    hitCallback = 0;
    i = -1;
    cur = getCurMemContext();

    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        i = 5;
        throwsExceptionNested();
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // test query parsing with rethrow
    hitCallback = 0;
    i = -1;

    cur = getCurMemContext();

    TRY
    {
        i = 5;
        rewriteQueryWithRethrow("SELEC a FROM r;");
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // test query parsing and execution with rethrow
    hitCallback = 0;
    i = -1;

    cur = getCurMemContext();

    TRY
    {
        i = 5;
        rewriteQueryWithRethrow("SELEC a FROM r;");
        //processInput("SELECT x(a) FROM r;");
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // test query parsing and execution with rethrow
    hitCallback = 0;
    i = -1;

    cur = getCurMemContext();

    TRY
    {
        i = 5;
//        rewriteQueryWithRethrow("SELEC a FROM r;");
        processInput("SELECT x(a) FROM r;");
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");


    return PASS;
}

static void
throwsException(void)
{
    TRY
    {
        FATAL_LOG("throw exception");
    }
    ON_EXCEPTION
    {
        DEBUG_LOG("in exception handling code");
        RETHROW();
    }
    END_ON_EXCEPTION
}

static void
throwsExceptionNested(void)
{
    TRY
    {
        throwsException();
    }
    ON_EXCEPTION
    {
        DEBUG_LOG("in exception handling code (nested)");
        RETHROW();
    }
    END_ON_EXCEPTION
}

static rc
testCatchingWipe(void)
{
    volatile MemContext *originalContext = getCurMemContext();
    volatile MemContext * volatile cur;
    volatile MemContext * volatile after;
    volatile int i = 0;

    registerExceptionCallback(handleWipe);

    //
    hitCallback = 0;
    i = -1;

    // create memory context structure
    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        NEW_AND_ACQUIRE_MEMCONTEXT("exp2");
        NEW_AND_ACQUIRE_MEMCONTEXT("exp3");
        cur = getCurMemContext();
        i = 1;
        THROW(SEVERITY_RECOVERABLE, "message");
    }
    END_TRY

    after = getCurMemContext();
    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"try code was executed until exception");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "context has not changed");

    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    after = getCurMemContext();
    ASSERT_EQUALS_STRINGP(originalContext->contextName, after->contextName, "back to default context");

    // test multilevel rethrow
    hitCallback = 0;
    i = -1;

    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
    cur = getCurMemContext();

    TRY
    {
        i = 5;
        throwsExceptionNested();
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    after = getCurMemContext();
    ASSERT_EQUALS_STRINGP(originalContext->contextName, after->contextName, "back to default context");

    // test query parsing
    hitCallback = 0;
    i = -1;

    cur = getCurMemContext();

    TRY
    {
        i = 5;
        rewriteQueryWithRethrow("SELEC a FROM r;");
    }
    ON_EXCEPTION
    {
        i = 1;
    }
    END_ON_EXCEPTION

    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();

    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_INT(1,i,"on exception code was executed");
    ASSERT_EQUALS_STRINGP(cur->contextName, after->contextName, "back to context before exception");

    // reset exception handler
    registerExceptionCallback(handleE);

    return PASS;
}


static rc
testSignalHandling (void)
{
    volatile MemContext * volatile cur = getCurMemContext();
    volatile MemContext * volatile after;

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
