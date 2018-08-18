/*-----------------------------------------------------------------------------
 *
 * exception.c - Exception handling code.
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "exception/exception.h"
#include "rewriter.h"
#include "log/termcolor.h"

typedef void (*ExceptionCallbackFunction) (const char *,const char *,int,int);

// callback function
static GProMExceptionCallbackFunctionInternal exceptionCallback = NULL;
static ExceptionSeverity severity;
static const char *exceptionMessage = NULL;
static const char *file = NULL;
static int line = -1;
static void sigsegv_handler(int signo);
static char *wipeContext = QUERY_MEM_CONTEXT;

#define FALLBACK_BUFFER_SIZE 4096
#define MAX_FALLBACKSTRING (FALLBACK_BUFFER_SIZE - 1)

static char fallbackBuffer[FALLBACK_BUFFER_SIZE];
static char filenameBuffer[FALLBACK_BUFFER_SIZE];

// macros
#define SEVER_TO_STRING(s) ((s == SEVERITY_PANIC) ? TB("PANIC") : ((s == SEVERITY_RECOVERABLE) ? TB("RECOVERABLE") : TB("SIGSEGV")))
#define SEVER_TO_STRING_NO_COLOR(s) ((s == SEVERITY_PANIC) ? "PANIC" : ((s == SEVERITY_RECOVERABLE) ? "RECOVERABLE" : "SIGSEGV"))

#define EXCEPTION_CONTEXT "_EXCEPTION_HANDLING_CONTEXT"

// for storing pointer to long jmp stack
sigjmp_buf *exceptionBuf = NULL;

// information about exception
void
registerExceptionCallback (GProMExceptionCallbackFunctionInternal callback)
{
    exceptionCallback = callback;
}

//
void
registerSignalHandler(void)
{
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGILL, sigsegv_handler);
}

void
deregisterSignalHandler(void)
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
}

void
processException(void)
{
    if (exceptionCallback != NULL)
    {
        ExceptionHandler e = exceptionCallback(exceptionMessage, file, line, severity);

        if (memManagerUsable())
        {
            NEW_AND_ACQUIRE_MEMCONTEXT(EXCEPTION_CONTEXT);
            DEBUG_LOG("exception handler tells us to %s", ExceptionHandlerToString(e));
            FREE_AND_RELEASE_CUR_MEM_CONTEXT();
        }
        else
        {
            fprintf(stderr, "exception handler tells us to %s", ExceptionHandlerToString(e));
            fflush(stderr);
        }

        // the handler determines how to deal with the exception
        switch(e)
        {
            // kill ourselves
            case EXCEPTION_DIE:
                fprintf(stderr, TBLINK_FG_BG(WHITE,RED,"EXCEPTION") " Handler requested us to die because of exception at " \
                        TCOL(RED,"(%s:%u) ") "\n\n%s\n",
                        file, line, exceptionMessage);
                fflush(stderr);
                exit(1);
                break;
            // abort current query and wipe per query memory context
            case EXCEPTION_ABORT:
                fprintf(stderr, "ABORT BASED ON EXCEPTION wipe <%s>\n\n", wipeContext);
                fflush(stderr);
                freeMemContextAndChildren(wipeContext);
                break;
            // abort current query and wipe all but the top memory context
            // afterwards all components have to be reinitialized
            case EXCEPTION_WIPE:
                fprintf(stderr, "ABORT WITHOUT WIPE BASED ON EXCEPTION\n\n");
                fflush(stderr);
                break;
        }
        if (severity == SEVERITY_PANIC)
        {
            fprintf(stderr, TBLINK_FG_BG(WHITE,RED,"EXCEPTION") " Do not know how to recover from this %s exception at " \
                    TCOL(RED,"(%s:%u) ") "\n\n%s\n",
                    SEVER_TO_STRING(severity), file, line, exceptionMessage);
            fflush(stderr);
            exit(1);
        }
    }
    // if no exception handler is available then print exception information and exit
    else
    {
        fprintf(stderr, TBLINK_FG_BG(WHITE,RED,"EXCEPTION") " No handler registered for %s exception that has "
                "occured at " TCOL(RED,"(%s:%u) ") "\n\n%s\n",
                    SEVER_TO_STRING(severity), file, line, exceptionMessage);
        fflush(stderr);
        exit(1);
    }
}

void
storeExceptionInfo(ExceptionSeverity s, const char *message, const char *f, int l)
{
    severity = s;
//    file = f;
    line = l;
    // copy the message into the default memory context if the memory manager is usable
    if (memManagerUsable())
    {
        NEW_AND_ACQUIRE_MEMCONTEXT(EXCEPTION_CONTEXT);
//        ACQUIRE_MEM_CONTEXT(getDefaultMemContext());
        exceptionMessage = strdup((char *) message);
        file = strdup((char *) f);
        FREE_AND_RELEASE_CUR_MEM_CONTEXT();
        ERROR_LOG("exception was thrown %s\n", currentExceptionToString());
    }
    // use fallback buffer to not loose message otherwise
    else
    {
        exceptionMessage = fallbackBuffer;
        file = filenameBuffer;

        // make sure that fallbackBuffer is a valid string
        fallbackBuffer[MAX_FALLBACKSTRING] = '\0';
        strncpy(fallbackBuffer, message, MAX_FALLBACKSTRING);

        filenameBuffer[MAX_FALLBACKSTRING] = '\0';
        strncpy(filenameBuffer, f, MAX_FALLBACKSTRING);

        fprintf(stderr, "exception was thrown: %s\n", fallbackBuffer);
        fflush(stderr);
    }
}

char *
currentExceptionToString(void)
{
    StringInfo result = makeStringInfo();

    appendStringInfo(result, "(%s) %s - %u - <%s>", SEVER_TO_STRING_NO_COLOR(severity), file, line, exceptionMessage);

    return result->data;
}

void
setWipeContext(char *wContext)
{
    wipeContext = wContext;
}

//TODO should not call longjmp from signal handler!!!!
static void
sigsegv_handler(int signo)
{
  if (signo == SIGSEGV)
  {
//      ERROR_LOG("segmentation fault in process %u", getpid());
      fprintf(stderr, "segmentation fault in process %u\n", getpid());
      THROW(SEVERITY_SIGSEGV, "segmentation fault in process %u", getpid());
  }
  if (signo == SIGILL)
  {
      fprintf(stderr, "segmentation fault in process %u\n", getpid());
      THROW(SEVERITY_PANIC, "illegal instruction in process %u", getpid());
  }
//  signal(signo, SIG_DFL);
//  kill(getpid(), signo);
}

