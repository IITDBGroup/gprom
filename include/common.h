#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <assert.h>

/*******************************************************************************
 * Portability
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

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
#define exit(retVal) return retVal;
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


/* ptrdiff_t */
#if HAVE_PTRDIFF_T
//TODO
#endif

/* strdup function */
#if HAVE_STRDUP
#else
    char *strdup(const char *input)
    {
        char *result = malloc(strlen(input) + 1);
        result = strcpy(result,input);
        return result;
    }
#endif

/* exit for main function */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#define EXIT_FAILURE  1
#endif



/*******************************************************************************
 * Definitions
 */
typedef short boolean;
#define TRUE 1
#define FALSE 0

#endif /* COMMON_H */
