/*-------------------------------------------------------------------------
 *
 * logger.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "logger.h"
#include "Configuration/Option.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static char *h[] =
    {"Fatal: ", "Error: ", "Warn: ", "Info: ", "Debug: ", "Trace: "};
static Options *option;

inline char *
getHead(LogLevel level)
{
    return *(h + level);
}

inline FILE *
getOutput(LogLevel level)
{
    return stdout;
}

void
log_(LogLevel level, const char *file, unsigned line, const char *template, ...)
{
    option = getOptions();
    if (level <= option->optionDebug->loglevel)
    {
        va_list args;
        va_start(args, template);
        FILE *out = getOutput(level);

        fprintf(out, "%s", getHead(level));
        vfprintf(out, template, args);
        if (file && line > 0)
        {
            fprintf(out, " (%s:%u)\n", file, line);
        }
        else
        {
            fprintf(out, " (unknown)\n");
        }
        fflush(out);

        va_end(args);
    }
}

