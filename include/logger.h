/*-------------------------------------------------------------------------
 *
 * logger.h
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

typedef enum
{
    FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
} LogLevel;

extern void
log_(LogLevel level, const char *file, unsigned line, const char *template, ...);

#endif
