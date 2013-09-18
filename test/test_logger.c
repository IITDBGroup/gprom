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
    FATAL_LOG("this is %s msg, level=%d", "fatal", LOG_FATAL);
    ERROR_LOG("this is %s msg, level=%d", "fatal", LOG_ERROR);
    WARN_LOG("this is %s msg, level=%d", "fatal", LOG_WARN);
    INFO_LOG("this is %s msg, level=%d", "fatal", LOG_INFO);
    DEBUG_LOG("this is %s msg, level=%d", "fatal", LOG_DEBUG);
    TRACE_LOG("this is %s msg, level=%d", "fatal", LOG_TRACE);

    return PASS;
}
