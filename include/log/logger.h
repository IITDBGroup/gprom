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
#include "exception/exception.h"

typedef enum LogLevel
{
    LOG_FATAL = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_TRACE = 5
} LogLevel;

extern void reinitLogger(void);
extern void initLogger(void);
extern void shutdownLogger (void);
extern void setMaxLevel (LogLevel level);
extern void logNodes_(LogLevel level, const char *file, unsigned line,
        boolean beatify, char * (*toStringFunc) (void *),
        const char *message, ...);
extern void log_(LogLevel level, const char *file, unsigned line,
        const char *template, ...);
extern char *formatMes(const char *template, ...);
extern void _debugNode(void *p);
extern void _debugMessage(char *mes);
extern void registerLogCallback (void (*callback) (const char *,const char *,
        int,int));

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
#define INFO_NODE_LOG(message, ...)
#define DEBUG_NODE_LOG(message, ...)
#define ERROR_NODE_BEATIFY_LOG(message, ...)
#define INFO_NODE_BEATIFY_LOG(message, ...)
#define DEBUG_NODE_BEATIFY_LOG(message, ...)
#define INFO_DL_LOG(message, ...)
#define ERROR_OP_LOG(message, ...)
#define INFO_OP_LOG(message, ...)

/* user has activated logging (default) */
#else

#define FATAL_LOG(template, ...) \
    do { \
        log_(LOG_FATAL, __FILE__, __LINE__, (template),  ##__VA_ARGS__); \
        THROW(SEVERITY_RECOVERABLE,(template), ##__VA_ARGS__); \
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

#define NODE_LOG(loglevel,message, ...) \
    do { \
        if (maxLevel >= loglevel) \
        logNodes_(loglevel, __FILE__, __LINE__, FALSE, nodeToString, (message),  ##__VA_ARGS__, NULL); \
    } while (0)

#define DEBUG_NODE_LOG(message, ...) NODE_LOG(LOG_DEBUG,message, ##__VA_ARGS__)
#define INFO_NODE_LOG(message, ...) NODE_LOG(LOG_INFO,message, ##__VA_ARGS__)
#define ERROR_NODE_LOG(message, ...) NODE_LOG(LOG_ERROR,message, ##__VA_ARGS__)
#define FATAL_NODE_LOG(message, ...) \
    do { \
        NODE_LOG(LOG_FATAL,message, ##__VA_ARGS__); \
        THROW(SEVERITY_RECOVERABLE,(message), ##__VA_ARGS__); \
    } while (0);

#define NODE_BEATIFY_LOG(loglevel,message, ...) \
    do { \
        if (maxLevel >= loglevel) \
        logNodes_(loglevel, __FILE__, __LINE__, TRUE, nodeToString, (message),  ##__VA_ARGS__, NULL); \
    } while (0)

#define DEBUG_NODE_BEATIFY_LOG(message, ...) NODE_BEATIFY_LOG(LOG_DEBUG,message, ##__VA_ARGS__)
#define INFO_NODE_BEATIFY_LOG(message, ...) NODE_BEATIFY_LOG(LOG_INFO,message, ##__VA_ARGS__)
#define ERROR_NODE_BEATIFY_LOG(message, ...) NODE_BEATIFY_LOG(LOG_ERROR,message, ##__VA_ARGS__)
#define FATAL_NODE_BEATIFY_LOG(message, ...) \
    do { \
        NODE_BEATIFY_LOG(LOG_FATAL,message, ##__VA_ARGS__); \
        THROW(SEVERITY_RECOVERABLE,(message), ##__VA_ARGS__); \
    } while (0);


#define DL_LOG(loglevel, message, ...) \
    do { \
        if (maxLevel >= loglevel) \
            logNodes_(loglevel, __FILE__, __LINE__, FALSE, datalogToOverviewString, (message),  ##__VA_ARGS__, NULL); \
    } while (0)

#define DEBUG_DL_LOG(message, ...) DL_LOG(LOG_DEBUG,message, ##__VA_ARGS__)
#define INFO_DL_LOG(message, ...) DL_LOG(LOG_INFO,message, ##__VA_ARGS__)
#define ERROR_DL_LOG(message, ...) DL_LOG(LOG_ERROR,message, ##__VA_ARGS__)
#define FATAL_DL_LOG(message, ...) \
    do { \
        DL_LOG(LOG_FATAL,message, ##__VA_ARGS__); \
        THROW(SEVERITY_RECOVERABLE,(message), ##__VA_ARGS__); \
    } while (0);

#define OP_LOG(loglevel, message, ...) \
    do { \
        if (maxLevel >= loglevel) \
            logNodes_(loglevel, __FILE__, __LINE__, FALSE, operatorToOverviewString, (message),  ##__VA_ARGS__, NULL); \
    } while (0)

#define DEBUG_OP_LOG(message, ...) OP_LOG(LOG_DEBUG,message, ##__VA_ARGS__)
#define INFO_OP_LOG(message, ...) OP_LOG(LOG_INFO,message, ##__VA_ARGS__)
#define ERROR_OP_LOG(message, ...) OP_LOG(LOG_ERROR,message, ##__VA_ARGS__)
#define FATAL_OP_LOG(message, ...) \
    do { \
        OP_LOG(LOG_FATAL,message, ##__VA_ARGS__); \
        THROW(SEVERITY_RECOVERABLE,(message), ##__VA_ARGS__); \
    } while (0);


#endif
/* logging activated switch */

/* header */
#endif
