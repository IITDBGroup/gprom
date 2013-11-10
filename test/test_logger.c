/*-------------------------------------------------------------------------
 *
 * test_logger.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "log/logger.h"
#include "test_main.h"

rc
testLogger(void)
{
//TODO will kill program    FATAL_LOG("this is %s msg, level=%d", "fatal", LOG_FATAL);
    ERROR_LOG("this is %s msg, level=%d", "error", LOG_ERROR);
    WARN_LOG("this is %s msg, level=%d", "warn", LOG_WARN);
    INFO_LOG("this is %s msg, level=%d", "info", LOG_INFO);
    DEBUG_LOG("this is %s msg, level=%d", "debug", LOG_DEBUG);
    TRACE_LOG("this is %s msg, level=%d", "trace", LOG_TRACE);

    return PASS;
}
