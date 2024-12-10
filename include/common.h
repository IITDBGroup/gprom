#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

/*******************************************************************************
 * Portability
 */
//#if HAVE_CONFIG_H
#ifndef IMPORTED_CONFIG_H
#define IMPORTED_CONFIG_H
#include <config.h>
#endif

//#endif /* HAVE_CONFIG_H */

/* <inttypes.h> integer type definitions */
#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if HAVE_FLOAT_H
#include <float.h>
#endif

/* <sys/types.h> */
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

/* <stdlib.h> exit() */
#if HAVE_STDLIB_H
#include <stdlib.h>
#else
#define exit(retVal) return;
#endif

/* <stdint.h> convenience integer types */
#if HAVE_STDINT_H
#include <stdint.h>
typedef intptr_t gprom_long_t;
#elif HAVE_LONG_LONG_INT
typedef long long int gprom_long_t;
#else /* use pointer type as fallback? */
typedef (void *) gprom_long_t;
#endif

/* <stddef.h> */
#if HAVE_STDDEF_H
#include <stddef.h>
#endif

/* <string.h> */
#if HAVE_STRING_H
#include <string.h>
#endif

/* <strings.h> */
#if HAVE_STRINGS_H
#include <strings.h>
#endif

/* <stdargs.h> */
#if HAVE_STDARG_H
#include <stdarg.h>
#endif

/* <time.h> */
#if HAVE_TIME_H
#include <time.h>
#include <sys/time.h>
#endif

/* <limits.h> */
#if HAVE_LIMITS_H
#include <limits.h>
#endif

/* ptrdiff_t */
#if HAVE_PTRDIFF_T
//TODO
#endif

/* <regex.h> */
#if HAVE_REGEX_H
#include <regex.h>
#endif

/* longjmp for exception handling */
#if HAVE_SETJMP_H
#include <setjmp.h>
#endif

/* signal handler */
#if HAVE_SIGNAL_H
#include <signal.h>
#endif

/* pthread */
#if HAVE_PTHREAD_H
#include <pthread.h>
#elif OS_WINDOWS
#include <windows.h>
#include <process.h>
#endif

/* unistd handler */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

/* math */
#if HAVE_MATH_H
#include <math.h>
#endif

/* pwd */
#if HAVE_PWD_H
#include <pwd.h>
#endif

/* regex */
#if HAVE_REGEX_H
#include <regex.h>
#endif

/* strdup function */
#if HAVE_STRDUP
#undef strdup
#define strdup(input) contextStringDup(input)
#else
#define strdup(input) contextStringDup(input)
#endif

/* sigsetjmp function is called setjmp on windows */
#ifndef HAVE_SIGJMP_BUF
#define sigjmp_buf jmp_buf
#define sigsetjmp(x,y) setjmp(x)
#define siglongjmp longjmp
#endif


/* streq function */
#if HAVE_STRCMP
#define streq(_l,_r) (strcmp(_l,_r) == 0)
#define strpeq(_l,_r) (((_l) == (_r)) || ((_l != NULL) && (_r != NULL) && (strcmp(_l,_r) == 0)))
#define strpleq(_l,_r) ((_l != NULL) && (strcmp(_l,_r) == 0))
#define strneq(_l,_r,n) (strncmp(_l,_r,n) == 0)
#define strcaseeq(_l,_r) (strcasecmp(_l,_r) == 0)
#define strStartsWith(_str,_prefix) (strncmp(_str,_prefix,strlen(_prefix)) == 0)
#endif



/* exit for main function */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#define EXIT_FAILURE  1
#endif

/*******************************************************************************
 * Database Backends
 */

// postgres
#if HAVE_LIBPQ && HAVE_LIBPQ_FE_H
#define HAVE_POSTGRES_BACKEND 1
#endif

// oracle
#if HAVE_LIBOCILIB && (HAVE_LIBOCI || (HAVE_LIBOCCI && HAVE_LIBCLNTSH))
#define HAVE_ORACLE_BACKEND 1
#endif

// sqlite
#if HAVE_LIBSQLITE3
#define HAVE_SQLITE_BACKEND 1
#endif

// duckdb
#if HAVE_LIBDUCKDB
#define HAVE_DUCKDB_BACKEND 1
#endif

// monetdb
#if HAVE_LIBMAPI && HAVE_MONETDB_MAPI_H
#define HAVE_MONETDB_BACKEND 1
#endif

// odbc
#if HAVE_LIBODBC && HAVE_SQL_H && HAVE_SQLEXT_H
#define HAVE_ODBC_BACKEND 1
#endif

// mssql
#if HAVE_MSSQL_ODBC_DRIVER && HAVE_ODBC_BACKEND
#define HAVE_MSSQL_BACKEND 1
#endif

// any backend
#if HAVE_POSTGRES_BACKEND || HAVE_ORACLE_BACKEND || HAVE_SQLITE_BACKEND || HAVE_DUCKDB_BACKEND || HAVE_MONETDB_BACKEND || HAVE_ODBC_BACKEND || HAVE_ODBC_BACKEND
#define HAVE_A_BACKEND 1
#endif

/* OCI stuff */
#if HAVE_ORACLE_BACKEND
#include <ocilib.h>
#endif

/********************************************************************************
 * Z3
 */
#if HAVE_LIBZ3 && HAVE_Z3_H
#define HAVE_Z3 1
#endif

/********************************************************************************
 * Readline
 */
#if HAVE_LIBREADLINE
#define HAVE_READLINE 1
#endif

/*******************************************************************************
 * Definitions
 */
#ifndef NULL
#define NULL (void *) 0
#endif

/* boolean type and consts */
#ifndef boolean
#define boolean int
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// override free to make sure nobody is using free directly
#ifndef malloc
#define free(_p) "DO NOT USE free DIRECTLY USE \"FREE\" FROM THE MEMORY MANAGER"; @
#define malloc(_p) "DO NOT USE malloc DIRECTLY USE \"MALLOC\" FROM THE MEMORY MANAGER"; @
#else
#define MALLOC_REDEFINED
#endif

// provide ASSERT macro if not deactivated by user
// use exception system instead
#ifdef DISABLE_ASSERT
#define ASSERT(expr)
#define ASSERT_BARRIER(code)
#define TIME_ASSERT(expr)
#else
#define ASSERT(expr) \
    if (!(expr)) { \
    	THROW(SEVERITY_RECOVERABLE, "%s: %s\n", "failed assertion", #expr); \
    }

#define TIME_ASSERT(expr) \
    do { \
        START_TIMER("ASSERT"); \
        ASSERT(expr); \
        STOP_TIMER("ASSERT"); \
    } while(0)
#define ASSERT_BARRIER(code) code
#endif

// min and max
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

// throw error for unimplemented method
#define TODO_IMPL FATAL_LOG("method not implemented yet!")

#endif /* COMMON_H */
