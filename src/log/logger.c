/*-------------------------------------------------------------------------
 *
 * logger.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    This module is for providing uniform logging for the whole system.
 *
 *        XXX_LOG() macros work like printf() in C. The first argument is a
 *        format string. Additional optional arguments are parameters
 *        substituted in the format string. For example:
 *        int t = 5;
 *        DEBUG_LOG("value of "%s" is %d", "t", t);
 *        There are six log levels: 0-FATAL, 1-ERROR, 2-WARN, 3-INFO, 4-DEBUG,
 *        5-TRACE. Logs of the levels below or equal to the log level value
 *        set from the command line will be printed. For example, if log level
 *        is set to 3 by command line, then logs from FATAL to INFO level will
 *        be printed while logs at DEBUG and TRACE level will not be printed.
 *
 *-------------------------------------------------------------------------
 */

#include "log/logger.h"
#include "configuration/option.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static char *h[] =
    {"FATAL: ", "ERROR: ", "WARN: ", "INFO: ", "DEBUG: ", "TRACE: "};
static int maxLevel = LOG_INFO;

/* internal inlined functions */
static inline char *
getHead(LogLevel level);
static inline FILE *
getOutput(LogLevel level);

static inline char *
getHead(LogLevel level)
{
    return *(h + level);
}

static inline FILE *
getOutput(LogLevel level)
{
    return stdout;
}

void
initLogger(void)
{
    Options *options = getOptions();
    if (options && options->optionDebug)
        maxLevel = options->optionDebug->loglevel;
}

void
log_(LogLevel level, const char *file, unsigned line, const char *template, ...)
{
    if (level <= maxLevel)
    {
        va_list args;
        FILE *out = getOutput(level);

        fprintf(out, "%s", getHead(level));
        if (file && line > 0)
        {
            fprintf(out, "(%s:%u) ", file, line);
        }
        else
        {
            fprintf(out, "(unknown) ");
        }

        va_start(args, template);
        vfprintf(out, template, args);
        va_end(args);

        fprintf(out, "\n");
        fflush(out);
    }
}

