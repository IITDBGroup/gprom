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
#include "exception/exception.h"
#include "rewriter.h"

typedef void (*ExceptionCallbackFunction) (const char *,const char *,int,int);

// callback function
static GProMExceptionCallbackFunctionInternal exceptionCallback = NULL;
static ExceptionSeverity severity;
static const char *exceptionMessage = NULL;
static const char *file = NULL;
static int line = -1;

// macros
#define SEVER_TO_STRING(s) ((s == SEVERITY_PANIC) ? "PANIC" : "RECOVERABLE")

// for storing pointer to long jmp stack
sigjmp_buf *exceptionBuf = NULL;

// information about exception
void
registerExceptionCallback (GProMExceptionCallbackFunctionInternal callback)
{
    exceptionCallback = callback;
}

void
processException(void)
{
    if (exceptionCallback != NULL)
    {
        ExceptionHandler e = exceptionCallback(exceptionMessage, file, line, severity);

        // the handler determines how to deal with the exception
        switch(e)
        {
            // kill ourselves
            case EXCEPTION_DIE:
                fprintf(stderr, "Handler requested us to die because of exception %s-%u\n\n%s",
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
            fprintf(stderr, "Do not know how to recover from this %s exception at %s-%u\n\n%s",
                    SEVER_TO_STRING(severity), file, line, exceptionMessage);
            exit(1);
        }
    }
    // if no exception handler is available then print exception information and exit
    else
    {
        fprintf(stderr, "No handler registered for %s exception that has "
                "occured %s-%u\n\n%s",
                    SEVER_TO_STRING(severity), file, line, exceptionMessage);
        exit(1);
    }
}

void
storeExceptionInfo(ExceptionSeverity s, const char *message, const char *f, int l)
{
    severity = s;
    exceptionMessage = message;
    file = f;
    line = l;
}

