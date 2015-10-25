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

/* cost of a plan */
#define PLAN_MAX_COST ULLONG_MAX
typedef unsigned long long int PlanCost;

/* current state of the optimizer */
typedef struct OptimizerState
{
    PlanCost bestPlanCost;
    char *bestPlan;
    float bestPlanExpectedTime;
    PlanCost currentCost;
    char *currentPlan;
    int planCount;
    float optTime;
    List *curPath;
    List *numChoices;
    List *fixedPath;
    void *hook;
    int maxPlans;
} OptimizerState;

/* initialize the optimizer's state */
static inline OptimizerState *
createOptState(void)
{
    OptimizerState *state = NEW(OptimizerState);

    state->bestPlanCost = PLAN_MAX_COST;
    state->bestPlan = NULL;
    state->currentCost =  PLAN_MAX_COST;
    state->currentPlan = NULL;
    state->optTime = 0.0;
    state->bestPlanExpectedTime = DBL_MAX;
    state->curPath = NIL;
    state->numChoices = NIL;
    state->fixedPath = NIL;
    state->hook = NULL;
    state->planCount = 0;
    state->maxPlans = -1;

    return state;
}

/*
 * A plugin for the cost-based optimizer.
 *
 */
typedef struct CostBasedOptimizer
{
    boolean (*initialize) (OptimizerState *state);
    boolean (*shouldContinue) (OptimizerState *state);
    boolean (*generateNextChoice) (OptimizerState *state);
    int (*callback) (OptimizerState *state, int numOptions);
} CostBasedOptimizer;

/* the current optimizer to be used */
static CostBasedOptimizer *opt = NULL;
static OptimizerState *state = NULL;

// function for mapping cost units into time (should be backend specific)
static double estimateRuntime (OptimizerState *state);

// exhaustive cost based optimization
static boolean exhaustiveGenerateNextChoice (OptimizerState *state);
static boolean exhaustiveContinueOptimization (OptimizerState *state);
static int exhaustiveCallback (OptimizerState *state, int numChoices);

// simmulated annealing
static boolean simannInitialize(OptimizerState *state);
static boolean simannGenerateNextChoice (OptimizerState *state);
static boolean simannContinueOptimization (OptimizerState *state);
static int simannCallback (OptimizerState *state, int numChoices);


/* 2-competitive version of the continue function that stops optimization when
 * more time has been spend on optimization then the expected runtime of the
 * current best plan. */
static boolean balancedContinueOptimization (OptimizerState *state);


void
chooseOptimizerPlugin(OptimizerPlugin typ)
{
    if (opt == NULL)
        opt = NEW(CostBasedOptimizer);
    switch(typ)
    {
        case OPTIMIZER_BALANCED:
        {
            opt->initialize = NULL;
            opt->shouldContinue = balancedContinueOptimization;
            opt->callback = exhaustiveCallback;
            opt->generateNextChoice = exhaustiveGenerateNextChoice;
        }
        break;
        case OPTIMIZER_SIMMULATED_ANNEALING:
        {
            opt->initialize = simannInitialize;
            opt->shouldContinue = simannContinueOptimization;
            opt->callback = simannCallback;
            opt->generateNextChoice = simannGenerateNextChoice;
        }
        break;
        case OPTIMIZER_EXHAUSTIVE:
        {
            opt->initialize = NULL;
            opt->shouldContinue = exhaustiveContinueOptimization;
            opt->callback = exhaustiveCallback;
            opt->generateNextChoice = exhaustiveGenerateNextChoice;
        }
        break;
    }
}

void
chooseOptimizerPluginFromString(char *pluginName)
{
    if (streq(pluginName,"exhaustive"))
        chooseOptimizerPlugin(OPTIMIZER_EXHAUSTIVE);
    if (streq(pluginName,"sim_ann"))
        chooseOptimizerPlugin(OPTIMIZER_SIMMULATED_ANNEALING);
}


/**
 * Main loop of the cost-based optimizer
 */
char *
doCostBasedOptimization(Node *oModel, boolean applyOptimizations)
{
    // intitialize optimizer state
    state = createOptState();
    if (opt->initialize)
        opt->initialize(state);
    state->maxPlans = getIntOption(OPTION_COST_BASED_MAX_PLANS);

    // main loop -> create one plan in each iteration
    while(opt->shouldContinue(state))
    {
        // keep track of time spent in loop
        struct timeval tvalBefore, tvalAfter;
        gettimeofday (&tvalBefore, NULL);

        // create next plan
        Node *oModel1 = copyObject(oModel);
        state->currentPlan = generatePlan(oModel1, applyOptimizations);

        char *result = strdup(state->currentPlan);
        state->currentCost = getCostEstimation(result);//TODO not what is returned by the function
        DEBUG_LOG("Cost of the rewritten Query is = %d\n", state->currentCost);
        INFO_LOG("plan (%u) for choice %s is\n%s", state->planCount, beatify(nodeToString(state->curPath)),
                state->currentPlan);
        ERROR_LOG("plan %u", state->planCount);

        // update best plan
        if(state->currentCost < state->bestPlanCost)
        {
            state->bestPlanCost = state->currentCost;
            state->bestPlan = strdup(state->currentPlan);
            state->bestPlanExpectedTime = estimateRuntime(state);
            DEBUG_LOG("PLAN: %s", state->bestPlan);
        }

        // determine what options to choose in the next iteration
        if(!opt->generateNextChoice(state))
            break;

        // update state
        gettimeofday (&tvalAfter, NULL);
        state->optTime += (float)(tvalAfter.tv_sec - tvalBefore.tv_sec) / 1000000;
        state->planCount++;
        FREE(result);
        FREE(state->currentPlan);
    }

    INFO_LOG("COST-BASED OPTIMIZATION: considered %u plans in total", state->planCount);
    return state->bestPlan;
}

static double
estimateRuntime (OptimizerState *state)
{
    return DBL_MAX; //TODO compute time
}



int
callback (int numChoices)
{
    return opt->callback(state,numChoices);
//    int xVal = -1;
//
//	DEBUG_LOG("number of choices are: %u", numChoices);
//	if (LIST_LENGTH(X1) == 0)
//	{
//		Y1 = appendToTailOfListInt(Y1, 0);
//		Z1 = appendToTailOfListInt(Z1, numChoices);
//	}
//	else
//	{
//		xVal = getHeadOfListInt(X1);
//		Y1 = appendToTailOfListInt(Y1, xVal);
//		X1 = removeFromHead(X1);
//		Z1 = appendToTailOfListInt(Z1, numChoices);
//	}
//
//	DEBUG_LOG("optimizer data structures are:\n X:%s\n Y:%s\n Z:%s\n",nodeToString(X1),
//			nodeToString(Y1),nodeToString(Z1));
//
//    return xVal;
}

//void
//reSetX1(){
//    int lenY1 = LIST_LENGTH(Y1);
//    for(int i=0; i<lenY1; i++)
//    {
//        int y = getTailOfListInt(Y1);
//        int z = getTailOfListInt(Z1);
//
//        if(y+1 == z)
//        {
//            Y1 = removeFromTail(Y1);
//            Z1 = removeFromTail(Z1);
//        }
//        else
//        {
//            y = y + 1;
//            Y1 = removeFromTail(Y1);
//            Y1 = appendToTailOfListInt(Y1,y);
//            break;
//        }
//    }
//    X1 = copyList(Y1);
//    Y1 = NIL;
//    Z1 = NIL;
//
//    DEBUG_LOG("new X is %s", beatify(nodeToString(X1)));
//}

static boolean
exhaustiveGenerateNextChoice (OptimizerState *state)
{
    List *X1 = state->curPath;
    List *Y1 = state->fixedPath;
    List *Z1 = state->numChoices;

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
    state->curPath = NIL;
    state->fixedPath = copyList(Y1);
    state->numChoices = NIL;

    DEBUG_LOG("new X is %s", beatify(nodeToString(X1)));

    return (state->fixedPath != NIL);
}

static boolean
exhaustiveContinueOptimization (OptimizerState *state)
{
    return TRUE;
}

static int
exhaustiveCallback (OptimizerState *state, int numChoices)
{
    int choice = -1;
    List *curPath = state->curPath;
    List *fixedPath = state->fixedPath;
    List *numChoicesOnPath = state->numChoices;
    DEBUG_LOG("number of choices are: %u", numChoices);
    if (LIST_LENGTH(fixedPath) == 0)
    {
        curPath = appendToTailOfListInt(curPath, 0);
        numChoicesOnPath = appendToTailOfListInt(numChoicesOnPath, numChoices);
    }
    else
    {
        choice = getHeadOfListInt(fixedPath);
        curPath = appendToTailOfListInt(curPath, choice);
        fixedPath = removeFromHead(fixedPath);
        numChoicesOnPath = appendToTailOfListInt(numChoicesOnPath, numChoices);
    }

    state->curPath = curPath;
    state->fixedPath = fixedPath;
    state->numChoices = numChoicesOnPath;

    DEBUG_LOG("optimizer data structures are:\n X:%s\n Y:%s\n Z:%s\n",nodeToString(fixedPath),
            nodeToString(curPath),nodeToString(numChoicesOnPath));

    return choice;
}

static boolean
balancedContinueOptimization (OptimizerState *state)
{
    return (state->optTime < state->bestPlanExpectedTime);
}

static boolean
simannInitialize(OptimizerState *state)
{
    return TRUE;
}

static boolean
simannGenerateNextChoice (OptimizerState *state)
{
    return TRUE;
}

static boolean
simannContinueOptimization (OptimizerState *state)
{
    return TRUE;
}

static int
simannCallback (OptimizerState *state, int numChoices)
{
    int choice = -1;
    List *curPath = state->curPath;
    List *fixedPath = state->fixedPath;
    List *numChoicesOnPath = state->numChoices;
    DEBUG_LOG("number of choices are: %u", numChoices);
    if (LIST_LENGTH(fixedPath) == 0)
    {
        int nextChoice = rand() % numChoices;
        curPath = appendToTailOfListInt(curPath, nextChoice);
        numChoicesOnPath = appendToTailOfListInt(numChoicesOnPath, numChoices);
    }
    else
    {
        choice = getHeadOfListInt(fixedPath);
        curPath = appendToTailOfListInt(curPath, choice);
        fixedPath = removeFromHead(fixedPath);
        numChoicesOnPath = appendToTailOfListInt(numChoicesOnPath, numChoices);
    }

    state->curPath = curPath;
    state->fixedPath = fixedPath;
    state->numChoices = numChoicesOnPath;

    DEBUG_LOG("optimizer data structures are:\n X:%s\n Y:%s\n Z:%s\n",nodeToString(fixedPath),
            nodeToString(curPath),nodeToString(numChoicesOnPath));

    return choice;
}
