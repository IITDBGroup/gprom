#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

/*******************************************************************************
 * Portability
 */
//#if HAVE_CONFIG_H
#include <config.h>
//#endif /* HAVE_CONFIG_H */

/* <inttypes.h> integer type definitions */
#if HAVE_INTTYPES_H
#include <inttypes.h>
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
#if HAVE_STDARGS_H
#include <stdargs.h>
#endif

/* ptrdiff_t */
#if HAVE_PTRDIFF_T
//TODO
#endif

/* strdup function */
#if HAVE_STRDUP
#undef strdup
#define strdup(input) contextStringDup(input)
#else
#define strdup(input) contextStringDup(input)
#endif

/* exit for main function */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#define EXIT_FAILURE  1
#endif

/* OCI stuff */
#if HAVE_LIBOCILIB
#include <ocilib.h>
#endif

/*******************************************************************************
 * Definitions
 */
#ifndef boolean
#define boolean int
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// override free to make sure nobody is using free directly
#define free(_p) "DO NOT USE free DIRECTLY USE \"FREE\" FROM THE MEMORY MANAGER"; @

#endif /* COMMON_H */
