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

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "log/logger.h"
#include "log/termcolor.h"
#include "model/node/nodetype.h"
#include "configuration/option.h"

#define INIT_BUF_SIZE 1 // 4096

static char *h[] =
    {"FATAL", "ERROR ", "WARN", "INFO", "DEBUG", "TRACE"};
static StringInfo buffer;

LogLevel maxLevel = LOG_INFO;

/* internal inlined functions */
static inline char *getHead(LogLevel level);
static inline FILE *getOutput(LogLevel level);
static boolean vAppendBuf(StringInfo str, const char *format, va_list args);

// use normal versions of free and malloc instead of memory manager ones
#undef free
#undef malloc

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
//    Options *options = getOptions();

    buffer = (StringInfo) malloc(sizeof(StringInfoData));
    buffer->len = 0;
    buffer->maxlen = INIT_BUF_SIZE;
    buffer->cursor = 0;
    buffer->data = (char *) malloc(INIT_BUF_SIZE);

//    if (options && options->optionDebug)
//        maxLevel = options->optionDebug->loglevel;

    maxLevel = getIntOption("log.level");
}

void
shutdownLogger (void)
{
    free(buffer->data);
    free(buffer);
}

void
_debugNode(void *p)
{
    log_(LOG_ERROR, "debugger", 0, "%s", beatify(nodeToString(p)));
}

void
_debugMessage(char *mes)
{
    log_(LOG_ERROR, "debugger", 0, "%s", mes);
}

void
log_(LogLevel level, const char *file, unsigned line, const char *template, ...)
{
    if (level <= maxLevel)
    {
        boolean success = FALSE;
        FILE *out = getOutput(level);
        buffer->len = 0;
        buffer->cursor = 0;
        buffer->data[0] = '\0';

        // output loglevel and location of log statement
        fprintf(out, TB_FG_BG(WHITE,BLACK,"%s"), getHead(level));
        if (file && line > 0)
            fprintf(out, TCOL(RED,"(%s:%u) "), file, line);
        else
            fprintf(out, "(unknown) ");

        // use string info as buffer to deal with large strings
        while(!success)
        {
            va_list args;

            va_start(args, template);
            success = vAppendBuf(buffer, template, args);
            va_end(args);
        }

        // output a fixed number of chars at a time to not reach fprintf limit
        int todo = buffer->len;
        char *curBuf = buffer->data;
        while(todo > 0)
        {
            size_t write = (todo >= 10240) ? 10240 : todo;
            size_t fw = fwrite(curBuf, sizeof(char), write, out);
            ASSERT(fw == write);
            curBuf += write;
            todo -= write;
            fflush(out);
        }

        // flush output stream
        fprintf(out, "\n");
        fflush(out);
    }
}

static boolean
vAppendBuf(StringInfo str, const char *format, va_list args)
{
    int needed, have;

    have = str->maxlen - str->len - 1;

    needed = vsnprintf(str->data + str->len, have, format, args);

    if (needed >= 0 && needed <= have)
    {
        str->len += needed;
        return TRUE;
    }
    if (needed < 0)
        fprintf(stderr, "encoding error in appendStringInfo <%s>", format);

    while(str->len + needed >= str->maxlen)
    {
        char *newData;

        str->maxlen *= 2;
        newData = malloc(str->maxlen); // change after we have REALLOC
        memcpy(newData, str->data, str->len + 1);
        free(str->data);
        str->data = newData;
    }

    return FALSE;
}
