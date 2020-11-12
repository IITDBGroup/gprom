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

void startTimer(char *name, int line, const char *function, const char *sourceFile);
void endTimer(char *name, int line, const char *function, const char *sourceFile);
void outputTimers (void);

/* timing activated? */
#ifndef DISABLE_TIMING
#define START_TIMER(name) startTimer(name, __LINE__, __func__, __FILE__)
#define STOP_TIMER(name) endTimer(name, __LINE__, __func__, __FILE__)
#define OUT_TIMERS() outputTimers()
/* timing deactivated? */
#else
#define START_TIMER(name)
#define STOP_TIMER(name)
#define OUT_TIMERS()
#endif

#endif /* TIMING_INSTRUMENTATION_H_ */
