/*-----------------------------------------------------------------------------
 *
 * exception.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_EXCEPTION_EXCEPTION_H_
#define INCLUDE_EXCEPTION_EXCEPTION_H_

#include "common.h"

/*
 * return value for exception handler to indicate what action to take
 *
 * EXECPTION_DIE: exit(1)
 * EXCEPTION_ABORT: abort processing the current query
 * EXCEPTION_WIPE: abort processing the curren query. Also  wipe all mem contexts except the top-level one, and reinitialize plugins
 */
typedef enum ExceptionHandler {
    EXCEPTION_DIE,
    EXCEPTION_ABORT,
    EXCEPTION_WIPE
} ExceptionHandler;

/*
 * Enum indicating severity of an exception
 */
typedef enum ExceptionSeverity {
    SEVERITY_PANIC,
    SEVERITY_RECOVERABLE
} ExceptionSeverity;

// callback function for exception handling
typedef ExceptionHandler (*GProMExceptionCallbackFunctionInternal) (const char *, const char *, int, ExceptionSeverity);

// register a callback function that is called when an exception is thrown
extern void registerExceptionCallback (GProMExceptionCallbackFunctionInternal callback);
extern void processException(void);
extern void storeExceptionInfo(ExceptionSeverity s, const char *message, const char *f, int l);

extern sigjmp_buf *exceptionBuf;

// macro try-catch block implementation
#define TRY \
    do { \
        sigjmp_buf _exceptionBuf; \
        if(!setjmp(_exceptionBuf)) { \
        	exceptionBuf = &_exceptionBuf; \


#define END_TRY \
        } else { \
			processException(); \
		}   \
		exceptionBuf = NULL; \
    } while (0);

//TODO
#define CATCH \
    } else {


#define END_CATCH \
    } \
    while (0);

#define THROW(severity,message) \
    do { \
        FATAL_LOG(message); \
        storeExceptionInfo(severity, message,__FILE__,__LINE__); \
        longjmp(*_exceptionBuf, 1); \
    } while(0)

#endif /* INCLUDE_EXCEPTION_EXCEPTION_H_ */
