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


// macros
#define SEVER_TO_STRING(s) ((s == SEVERITY_PANIC) ? TB("PANIC") : ((s == SEVERITY_RECOVERABLE) ? TB("RECOVERABLE") : TB("SIGSEGV")))

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

        ACQUIRE_MEM_CONTEXT(getDefaultMemContext());
        DEBUG_LOG("exception handler tells us to %s", ExceptionHandlerToString(e));
        RELEASE_MEM_CONTEXT();

        // the handler determines how to deal with the exception
        switch(e)
        {
            // kill ourselves
            case EXCEPTION_DIE:
                fprintf(stderr, TB_FG_BG(WHITE,BLACK,"EXCEPTION") " Handler requested us to die because of exception at " \
                        TCOL(RED,"(%s:%u) ") "\n\n%s",
                        file, line, exceptionMessage);
                exit(1);
                break;
            // abort current query and wipe per query memory context
            case EXCEPTION_ABORT:
                freeMemContextAndChildren(QUERY_MEM_CONTEXT);
                break;
            // abort current query and wipe all but the top memory context
            // afterwards all components have to be reinitialized
            case EXCEPTION_WIPE:
                //TODO
                break;
        }
        if (severity == SEVERITY_PANIC)
        {
            fprintf(stderr, TB_FG_BG(WHITE,BLACK,"EXCEPTION") " Do not know how to recover from this %s exception at " \
                    TCOL(RED,"(%s:%u) ") "\n\n%s",
                    SEVER_TO_STRING(severity), file, line, exceptionMessage);
            exit(1);
        }
    }
    // if no exception handler is available then print exception information and exit
    else
    {
        fprintf(stderr, TB_FG_BG(WHITE,BLACK,"EXCEPTION") " No handler registered for %s exception that has "
                "occured at " TCOL(RED,"(%s:%u) ") "\n\n%s",
                    SEVER_TO_STRING(severity), file, line, exceptionMessage);
        exit(1);
    }
}

void
storeExceptionInfo(ExceptionSeverity s, const char *message, const char *f, int l)
{
    severity = s;
    // copy the message into the default memory context
    ACQUIRE_MEM_CONTEXT(getDefaultMemContext());
    exceptionMessage = strdup((char *) message);
    RELEASE_MEM_CONTEXT();
    file = f;
    line = l;
}

static void
sigsegv_handler(int signo)
{
  if (signo == SIGSEGV)
  {
//      ERROR_LOG("segmentation fault in process %u", getpid());
      THROW(SEVERITY_SIGSEGV, "segmentation fault in process %u", getpid());
  }
//  signal(signo, SIG_DFL);
//  kill(getpid(), signo);
}

