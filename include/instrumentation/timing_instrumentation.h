/*-----------------------------------------------------------------------------
 *
 * timing_instrumentation.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef TIMING_INSTRUMENTATION_H_
#define TIMING_INSTRUMENTATION_H_

#include "common.h"

extern void startTimer(char *name, int line, const char *function, const char *sourceFile);
extern void endTimer(char *name, int line, const char *function, const char *sourceFile);
extern void outputTimers(void);
extern boolean isTimerRunning(char *name);


/* timing activated? */
#ifndef DISABLE_TIMING
#define START_TIMER(name) startTimer(name, __LINE__, __func__, __FILE__)
#define STOP_TIMER(name) endTimer(name, __LINE__, __func__, __FILE__)
#define STOP_TIMER_IF_RUNNING(name)                                            \
  do {                                                                         \
    if (isTimerRunning(name))                                                  \
      endTimer(name, __LINE__, __func__, __FILE__);                            \
  } while (0)

#define START_TIMER_IF_NOT_RUNNING(res, name)                                  \
  do {                                                                         \
    res = isTimerRunning(name);                                                \
    if (!res)                                                                  \
      startTimer(name, __LINE__, __func__, __FILE__);                          \
  } while (0)

#define OUT_TIMERS() outputTimers()
/* timing deactivated? */
#else
#define START_TIMER(name)
#define STOP_TIMER(name)
#define OUT_TIMERS()
#endif

#endif /* TIMING_INSTRUMENTATION_H_ */
