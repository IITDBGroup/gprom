/*-------------------------------------------------------------------------
 *
 * logger.h
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

#ifndef LOGGER_H_
#define LOGGER_H_

#include "common.h"

typedef enum LogLevel
{
    LOG_FATAL = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_TRACE = 5
} LogLevel;

extern void initLogger(void);
extern void log_(LogLevel level, const char *file, unsigned line, const char *template, ...);

#define FATAL_LOG(template, ...) \
    do { \
        log_(LOG_FATAL, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
        exit(1); \
    } while (0)
#define ERROR_LOG(template, ...) \
        log_(LOG_ERROR, __FILE__, __LINE__, (template),  ##__VA_ARGS__)
#define WARN_LOG(template, ...) \
        log_(LOG_WARN, __FILE__, __LINE__, (template),  ##__VA_ARGS__)
#define INFO_LOG(template, ...) \
        log_(LOG_INFO, __FILE__, __LINE__, (template),  ##__VA_ARGS__)
#define DEBUG_LOG(template, ...) \
        log_(LOG_DEBUG, __FILE__, __LINE__, (template),  ##__VA_ARGS__)
#define TRACE_LOG(template, ...) \
        log_(LOG_TRACE, __FILE__, __LINE__, (template),  ##__VA_ARGS__)


#endif
