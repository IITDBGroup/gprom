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
    LOG_FATAL = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_TRACE = 5
} LogLevel;

extern void log_(LogLevel level, const char *file, unsigned line, const char *template, ...);

#define FATAL_LOG(template, ...) \
        log_(LOG_FATAL, __FILE__, __LINE__, (template),  ##__VA_ARGS__)
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
