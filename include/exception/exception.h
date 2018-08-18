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
#include "log/logger.h"
#include "utility/enum_magic.h"

/*
 * return value for exception handler to indicate what action to take
 *
 * EXECPTION_DIE: exit(1)
 * EXCEPTION_ABORT: abort processing the current query
 * EXCEPTION_WIPE: abort processing the curren query. Also  wipe all mem contexts except the top-level one, and reinitialize plugins
 */
NEW_ENUM_WITH_TO_STRING(ExceptionHandler,
    EXCEPTION_DIE,
    EXCEPTION_ABORT,
    EXCEPTION_WIPE
);

/*
 * Enum indicating severity of an exception
 */
NEW_ENUM_WITH_TO_STRING(ExceptionSeverity,
    SEVERITY_PANIC,
    SEVERITY_RECOVERABLE,
    SEVERITY_SIGSEGV
);

// callback function for exception handling
typedef ExceptionHandler (*GProMExceptionCallbackFunctionInternal) (const char *, const char *, int, ExceptionSeverity);

// register a callback function that is called when an exception is thrown
extern void registerExceptionCallback (GProMExceptionCallbackFunctionInternal callback);
extern void registerSignalHandler(void);
extern void deregisterSignalHandler(void);
extern void processException(void);
extern void storeExceptionInfo(ExceptionSeverity s, const char *message, const char *f, int l);
extern char *currentExceptionToString(void);
extern void setWipeContext(char *wContext);

extern sigjmp_buf *exceptionBuf;

// macro try block implementation
#define TRY \
    do { \
        sigjmp_buf *save_previous_jmpbuf = exceptionBuf; \
        sigjmp_buf _exceptionBuf; \
        if(sigsetjmp(_exceptionBuf, 0) == 0) { \
        	    exceptionBuf = &_exceptionBuf; \

#define END_TRY \
        } else { \
        	    exceptionBuf = save_previous_jmpbuf; \
			processException(); \
		}   \
		exceptionBuf = save_previous_jmpbuf; \
    } while (0);

// try-on-exception block implementation
#define ON_EXCEPTION \
    } else { \
        exceptionBuf = save_previous_jmpbuf;

#define END_ON_EXCEPTION \
            processException(); \
        } \
		exceptionBuf = save_previous_jmpbuf; \
    } while (0);

//            exceptionBuf = save_previous_jmpbuf;

#define PROCESS_EXCEPTION_AND_DIE() \
    do { \
         processException(); \
         exit(1); \
    } while (0)

//TODO
#define CATCH \
    } else {


#define END_CATCH \
		processException(); \
		exceptionBuf = NULL; \
    } \
    while (0);

// throw an exception which results in a longjmp to the next try-catch block. If there
// is no such block then we exit
#define THROW(severity,format,...) \
    do { \
        storeExceptionInfo(severity, formatMes(format, ##__VA_ARGS__),__FILE__,__LINE__); \
        if (exceptionBuf != NULL) \
            siglongjmp(*exceptionBuf, 1); \
        else \
            exit(1); \
    } while(0)

// rethrow an exception in an ON_EXCEPTION block
#define RETHROW() \
    do { \
    	    if (exceptionBuf != NULL)  { \
            siglongjmp(*exceptionBuf, 1); \
        } \
    } while(0)


#endif /* INCLUDE_EXCEPTION_EXCEPTION_H_ */
