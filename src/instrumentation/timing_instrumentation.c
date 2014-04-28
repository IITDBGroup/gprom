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

    // time stats
    long totalTime;
    long minTime;
    long maxTime;
    long avgTime;
    long numCalls;

    // complete list of times
    List *times;

    // hash handle
    UT_hash_handle hh;
} Timer;

static Timer *allTimers = NULL;

// static functions
static Timer *getOrCreateTimer (char *name, int line, const char *function,
        const char *sourceFile);
static void updateStats (Timer *t);
static long mytimediff (struct timeval *start, struct timeval *end);

/*
 * Start a timer
 */
void
startTimer(char *name, int line, const char *function, const char *sourceFile)
{
    Timer *t = getOrCreateTimer(name, line, function, sourceFile);
    struct timeval st;

    gettimeofday(&st, NULL);
    t->curStart = st;
    DEBUG_LOG("Start time <%s> %ld sec %ld usec", name, st.tv_sec, st.tv_usec);
}

/*
 * Stop a timer
 */
void
endTimer(char *name, int line, const char *function, const char *sourceFile)
{
    Timer *t = getOrCreateTimer(name, line, function, sourceFile);
    struct timeval st;

    gettimeofday(&st, NULL);
    t->curEnd = st;
    DEBUG_LOG("Stop time <%s> %ld sec %ld usec", name, st.tv_sec, st.tv_usec);
    updateStats(t);
}

static void
updateStats (Timer *t)
{
    long newT = mytimediff(&(t->curEnd), &(t->curStart));
    t->totalTime += newT;
    t->minTime = (newT < t->minTime) ? newT : t->minTime;
    t->maxTime = (newT > t->maxTime) ? newT : t->maxTime;
    t->numCalls++;
    t->avgTime = t->totalTime / t->numCalls;
    t->times = appendToTailOfList(t->times, createConstFloat(newT));
}

static long
mytimediff (struct timeval *start, struct timeval *end)
{
    long diff;
    long usecDiff;
    time_t secDiff;

    DEBUG_LOG("\nstart: %ld sec %ld usec\nend: %ld sec %ld usec",
            start->tv_sec, start->tv_usec, end->tv_sec, end->tv_usec);
    usecDiff = (end->tv_usec / 1000L) - (start->tv_usec / 1000L);
    secDiff = (end->tv_sec - start->tv_sec);
    diff = usecDiff + (1000 * secDiff);
    DEBUG_LOG("secdiff: %ld, usecdiff: %ld, diff: %ld", usecDiff, secDiff, diff);

    return diff;
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
        t->line = line;
        t->func = strdup((char *) function);
        t->file = strdup((char *) sourceFile);

        HASH_ADD_KEYPTR(hh, allTimers, t->name, strlen(t->name), t);
        DEBUG_LOG("created new timer for <%s>", t->name);
    }

    return t;
}



void
outputTimers (void)
{
    Timer *t;

    if(!isRewriteOptionActivated("timing"))
        return;

    for(t = allTimers; t != NULL; t = t->hh.next)
    {
        printf("timer: %s   - total: %ld calls: %ld avg: %ld\n",
                t->name,
                t->totalTime,
                t->numCalls,
                t->avgTime);
    }
}
