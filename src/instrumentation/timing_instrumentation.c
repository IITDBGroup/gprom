/*-----------------------------------------------------------------------------
 *
 * timing_instrumentation.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "uthash.h"

#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "instrumentation/timing_instrumentation.h"

// store timings and summary data for a certain timer
typedef struct Timer
{
    // time position and info
    char *name;
    char *file;
    char *func;
    int line;

    // current timer status
    struct timeval curStart;
    struct timeval curEnd;
    long fullStart;
    long fullEnd;

    // time stats
    long totalTime;
    long minTime;
    long maxTime;
    long avgTime;
    long numCalls;

    // complete list of times
    List *times;

    // sanity check to avoid starting a new timer before the old one is finished
    boolean isRunning;

    // hash handle
    UT_hash_handle hh;
} Timer;

// variables
static Timer *allTimers = NULL;
static MemContext *context = NULL;

// static functions
static Timer *getOrCreateTimer (char *name, int line, const char *function,
        const char *sourceFile);
static void updateStats (Timer *t);
static int compareTimerName (const void *a, const void *b);

#define CREATE_OR_USE_MEMCONTEXT() \
    do { \
    	if (context == NULL)    \
            context = NEW_LONGLIVED_MEMCONTEXT("TimerContext");  \
        ACQUIRE_MEM_CONTEXT(context); \
    } while (0)

/*
 * Start a timer
 */
void
startTimer(char *name, int line, const char *function, const char *sourceFile)
{
    Timer *t = NULL;
    struct timeval st;

    if(!isRewriteOptionActivated(OPTION_TIMING))
        return;

    CREATE_OR_USE_MEMCONTEXT();

    t = getOrCreateTimer(name, line, function, sourceFile);
    if (t->isRunning)
        FATAL_LOG("try to start timer <%s>: %s-%s-%d that is still running", t->name, t->file, t->func, t->line);
    gettimeofday(&st, NULL);
    DEBUG_LOG("FULLTIME %ld", st.tv_sec * 1000000 + st.tv_usec);
    t->fullStart = st.tv_sec * 1000000 + st.tv_usec;
    t->curStart = st;
    t->isRunning = TRUE;
    DEBUG_LOG("Start time <%s> %ld sec %ld usec at <%s-%s-%d>", name, st.tv_sec, st.tv_usec, sourceFile, function, line);

    RELEASE_MEM_CONTEXT();
}

/*
 * Stop a timer
 */
void
endTimer(char *name, int line, const char *function, const char *sourceFile)
{
    Timer *t;
    struct timeval st;

    if(!isRewriteOptionActivated("timing"))
        return;

    CREATE_OR_USE_MEMCONTEXT();

    t = getOrCreateTimer(name, line, function, sourceFile);
    if(!t->isRunning)
        FATAL_LOG("try to stop timer <%s>: %s-%s-%d that is not running", t->name, t->file, t->func, t->line);
    gettimeofday(&st, NULL);
    t->curEnd = st;
    t->fullEnd = st.tv_sec * 1000000 + st.tv_usec;
    t->isRunning = FALSE;
    DEBUG_LOG("Stop time <%s> %ld sec %ld usec", name, st.tv_sec, st.tv_usec);
    updateStats(t);

    RELEASE_MEM_CONTEXT();
}

boolean
isTimerRunning(char *name)
{
  Timer *t;
  boolean isRunning = FALSE;

  if (!isRewriteOptionActivated("timing"))
    return isRunning;

  HASH_FIND_STR(allTimers, name, t);

  isRunning = t != NULL && t->isRunning;

  CREATE_OR_USE_MEMCONTEXT();
  RELEASE_MEM_CONTEXT();
  return isRunning;
}

static void
updateStats (Timer *t)
{
    DEBUG_LOG("%ld - %ld - %ld", t->fullStart, t->fullEnd, t->fullEnd - t->fullStart);

    long newT = t->fullEnd - t->fullStart;
    t->totalTime += newT;
    t->minTime = (newT < t->minTime) ? newT : t->minTime;
    t->maxTime = (newT > t->maxTime) ? newT : t->maxTime;
    t->numCalls++;
    t->avgTime = t->totalTime / t->numCalls;
    t->times = appendToTailOfList(t->times, createConstFloat(newT));
}

static Timer *
getOrCreateTimer (char *name, int line, const char *function, const char *sourceFile)
{
    Timer *t = NULL;

    HASH_FIND_STR(allTimers, name, t);

    if (t == NULL)
    {
        t = NEW(Timer);
        t->name = strdup(name);
        t->times = NIL;
        t->line = line;
        t->func = strdup((char *) function);
        t->file = strdup((char *) sourceFile);
        t->minTime = LONG_MAX;

        HASH_ADD_KEYPTR(hh, allTimers, t->name, strlen(t->name), t);
        DEBUG_LOG("created new timer for <%s>", t->name);
    }

    if (strcmp(t->file, sourceFile) != 0 || strcmp(t->func, function) != 0 || t->line != line)
    {
        t->line = line;
        t->func = strdup((char *) function);
        t->file = strdup((char *) sourceFile);
    }

    return t;
}



void
outputTimers (void)
{
    Timer *t, *total;
    int maxTimerNameLength = 30;
    int numTimers = -1;
    Timer **timers = NULL;

    CREATE_OR_USE_MEMCONTEXT();
    total = NULL;
    t = NULL;

    for(t = allTimers; t != NULL; t = t->hh.next)
    {
        int len = strlen(t->name);
        maxTimerNameLength = (maxTimerNameLength < len) ? len : maxTimerNameLength;
        numTimers++;
    }

    if(!isRewriteOptionActivated(OPTION_TIMING))
        return;

    timers = MALLOC(sizeof(Timer *) * numTimers);
    int i = 0;
    for(t = allTimers; t != NULL; t = t->hh.next)
    {
        if (!streq(t->name, "TOTAL"))
            timers[i++] = t;
        else
            total = t;
    }
    qsort(timers, numTimers, sizeof(Timer*), compareTimerName);

    for(int i = 0; i < numTimers; i++) //t = allTimers; t != NULL; t = t->hh.next)
    {
        Timer *t = timers[i];
        printf("timer: %-*s - total: %12f sec calls: %9ld avg: %12f min: %12f max: %12f\n",
                maxTimerNameLength,
                t->name,
                ((double) t->totalTime) / 1000000.0,
                t->numCalls,
                ((double) t->avgTime) / 1000000.0,
                ((double) t->minTime) / 1000000.0,
                ((double) t->maxTime) / 1000000.0
                );
    }
    printf("timer: ====================================================================\n");
    if (total != NULL)
      {
    printf("timer: %-*s - total: %12f sec calls: %9ld avg: %12f min: %12f max: %12f\n",
            maxTimerNameLength,
            total->name,
            ((double) total->totalTime) / 1000000.0,
            total->numCalls,
            ((double) total->avgTime) / 1000000.0,
            ((double) total->minTime) / 1000000.0,
            ((double) total->maxTime) / 1000000.0
            );
      }
    RELEASE_MEM_CONTEXT();
}

static int
compareTimerName (const void *a, const void *b)
{
    const Timer **ta, **tb;

    ta = (const Timer **) a;
    tb = (const Timer **) b;

    return strcmp((*ta)->name, (*tb)->name);
}
