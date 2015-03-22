/*-----------------------------------------------------------------------------
 *
 * cost_based_optimizer.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "rewriter.h"
//#include "model/node/nodetype.h"
#include "configuration/option.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "operator_optimizer/operator_optimizer.h"
#include "sql_serializer/sql_serializer.h"
#include "mem_manager/mem_mgr.h"
#include "metadata_lookup/metadata_lookup.h"

#include "instrumentation/timing_instrumentation.h"
#include "instrumentation/memory_instrumentation.h"
#include <sys/time.h>


static List *X1;
static List *Y1;
static List *Z1;
static unsigned long long int cost1;
static char *plan;


static boolean continueOptimization (long optTime, unsigned long long int expectedTime);

char *
doCostBasedOptimization(Node *oModel, boolean applyOptimizations)
{
    StringInfo result = makeStringInfo();
    char *rewrittenSQL = NULL;

    X1 = NIL;
    Y1 = NIL;
    Z1 = NIL;
    cost1 = ULLONG_MAX;
    plan = NULL;
    float optTime = 0.0;

    while(continueOptimization(optTime,cost1))
    {
        //Keep track of time spent in loop
        struct timeval tvalBefore, tvalAfter;
        gettimeofday (&tvalBefore, NULL);

        Node *oModel1 = copyObject(oModel);
        rewrittenSQL = generatePlan(oModel1, applyOptimizations);

        char *result = strdup(rewrittenSQL);
        unsigned long long int cost = getCostEstimation(result);//TODO not what is returned by the function
        DEBUG_LOG("Cost of the rewritten Query is = %d\n", cost);
        DEBUG_LOG("plan for choice %s is\n%s", beatify(nodeToString(Y1)),
                rewrittenSQL);

	if(cost < cost1)
        {
            cost1 = cost;
            plan = rewrittenSQL;
            DEBUG_LOG("PLAN: %s", plan);
        }

        // compute new X1
        reSetX1();
	if (X1 == NIL)
	    break;

        gettimeofday (&tvalAfter, NULL);
        optTime += (float)(tvalAfter.tv_sec - tvalBefore.tv_sec) / 1000000;
    }
    FREE(result);

    return plan;
}

static boolean
continueOptimization (long optTime, unsigned long long int expectedTime)
{
    return (optTime < expectedTime);
}

int
callback (int numChoices)
{
    int xVal = -1;

	DEBUG_LOG("number of choices are: %u", numChoices);
	if (LIST_LENGTH(X1) == 0)
	{
		Y1 = appendToTailOfListInt(Y1, 0);
		Z1 = appendToTailOfListInt(Z1, numChoices);
	}
	else
	{
		xVal = getHeadOfListInt(X1);
		Y1 = appendToTailOfListInt(Y1, xVal);
		X1 = removeFromHead(X1);
		Z1 = appendToTailOfListInt(Z1, numChoices);
	}

	DEBUG_LOG("optimizer data structures are:\n X:%s\n Y:%s\n Z:%s\n",nodeToString(X1),
			nodeToString(Y1),nodeToString(Z1));

    return xVal;
}

void
reSetX1(){
    int lenY1 = LIST_LENGTH(Y1);
    for(int i=0; i<lenY1; i++)
    {
        int y = getTailOfListInt(Y1);
        int z = getTailOfListInt(Z1);

        if(y+1 == z)
        {
            Y1 = removeFromTail(Y1);
            Z1 = removeFromTail(Z1);
        }
        else
        {
            y = y + 1;
            Y1 = removeFromTail(Y1);
            Y1 = appendToTailOfListInt(Y1,y);
            break;
        }
    }
    X1 = copyList(Y1);
    Y1 = NIL;
    Z1 = NIL;

    DEBUG_LOG("new X is %s", beatify(nodeToString(X1)));
}
