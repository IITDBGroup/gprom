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

#include "logger.h"
#include "test_main.h"

rc
testLogger(void)
{
    log_(FATAL, __FILE__, __LINE__, "this is %s msg, level=%d", "fatal", FATAL);
    log_(ERROR, __FILE__, __LINE__, "this is %s msg, level=%d", "error", ERROR);
    log_(WARN, __FILE__, __LINE__, "this is %s msg, level=%d", "warning", WARN);
    log_(INFO, __FILE__, __LINE__, "this is %s msg, level=%d", "info", INFO);
    log_(DEBUG, __FILE__, __LINE__, "this is %s msg, level=%d", "debug", DEBUG);
    log_(TRACE, __FILE__, __LINE__, "this is %s msg, level=%d", "trace", TRACE);

    return PASS;
}
