/*-------------------------------------------------------------------------
 *
 * logger.h
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    This module provides uniform logging for the whole system and exception
 *    handling capabilities.
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
 *        Currently, FATAL causes the application to exit.
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
extern void shutdownLogger (void);
extern void log_(LogLevel level, const char *file, unsigned line, const char *template, ...);
extern void _debugNode(void *p);
extern void _debugMessage(char *mes);

extern LogLevel maxLevel;

/* user has deactivated logging (default) */
#ifdef DISABLE_LOGGING

#define FATAL_LOG(template, ...)
#define ERROR_LOG(template, ...)
#define WARN_LOG(template, ...)
#define INFO_LOG(template, ...)
#define DEBUG_LOG(template, ...)
#define TRACE_LOG(template, ...)
#define GENERIC_LOG(level, file, line, template, ...)

/* user has activated logging (default) */
#else

#define FATAL_LOG(template, ...) \
    do { \
        log_(LOG_FATAL, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
        exit(1); \
    } while (0)
#define ERROR_LOG(template, ...) \
    do { \
        if (maxLevel >= LOG_ERROR) \
            log_(LOG_ERROR, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
    } while (0)
#define WARN_LOG(template, ...) \
    do { \
        if (maxLevel >= LOG_WARN) \
            log_(LOG_WARN, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
    } while (0)
#define INFO_LOG(template, ...) \
    do { \
        if (maxLevel >= LOG_INFO) \
            log_(LOG_INFO, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
    } while (0)
#define DEBUG_LOG(template, ...) \
    do { \
        if (maxLevel >= LOG_DEBUG) \
            log_(LOG_DEBUG, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
    } while (0)
#define TRACE_LOG(template, ...) \
    do { \
        if (maxLevel >= LOG_TRACE) \
            log_(LOG_TRACE, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
    } while (0)

#define GENERIC_LOG(level, file, line, template, ...) \
		do { \
			if (maxLevel >= level) \
			    log_(level, file, line, (template),  ##__VA_ARGS__); \
		} while (0)
#endif
/* logging activated switch */

/* header */
#endif
