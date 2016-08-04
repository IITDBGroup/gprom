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

#include "math.h"

// Added own implementation of *pow*
// to avoid messing with compiler options
// (Needs to link with -lm flag otherwise)

#define MAX_DELTA_DOUBLE 1.0E-15
#define EULERS_NUMBER 2.718281828459045

double MathAbs_Double (double x) {
    return ((x >= 0) ? x : -x);
}

int MathAbs_Int (int x) {
    return ((x >= 0) ? x : -x);
}

double MathPow_Double_Int(double x, int n) {
    double ret;
    if ((x == 1.0) || (n == 1)) {
        ret = x;
    } else if (n < 0) {
        ret = 1.0 / MathPow_Double_Int(x, -n);
    } else {
        ret = 1.0;
        while (n--) {
            ret *= x;
        }
    }
    return (ret);
}

double MathLn_Double(double x) {
    double ret = 0.0, d;
    if (x > 0) {
        int n = 0;
        do {
            int a = 2 * n + 1;
            d = (1.0 / a) * MathPow_Double_Int((x - 1) / (x + 1), a);
            ret += d;
            n++;
        } while (MathAbs_Double(d) > MAX_DELTA_DOUBLE);
    } else {
        printf("\nerror: x < 0 in ln(x)\n");
        exit(-1);
    }
    return (ret * 2);
}

double MathExp_Double(double x) {
    double ret;
    if (x == 1.0) {
        ret = EULERS_NUMBER;
    } else if (x < 0) {
        ret = 1.0 / MathExp_Double(-x);
    } else {
        int n = 2;
        double d;
        ret = 1.0 + x;
        do {
            d = x;
            for (int i = 2; i <= n; i++) {
                d *= x / i;
            }
            ret += d;
            n++;
        } while (d > MAX_DELTA_DOUBLE);
    }
    return (ret);
}

double pow(double x, double a) {
    double ret;
    if ((x == 1.0) || (a == 1.0)) {
        ret = x;
    } else if (a < 0) {
        ret = 1.0 / pow(x, -a);
    } else {
        ret = MathExp_Double(a * MathLn_Double(x));
    }
    return (ret);
}
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
    //Added by Xing
    PlanCost previousPlanCost;
    char *previousPlan;
    float previousPlanExpectedTime;
    List *previousPath;
} OptimizerState;

typedef struct AnnealingState
{
    PlanCost previousPlanCost;
    char *previousPlan;
    float previousPlanExpectedTime;
    List *previousPath;
    double temp;
    double coolingRate;
    PlanCost currentCost;
    char *currentPlan;
    List *curPath;

} AnnealingState;



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
    //Added by Xing

    //AnnealingState *annealState = NEW(AnnealingState);
    //annealState->previousPlanCost = PLAN_MAX_COST;
    //annealState->previousPlan = NULL;
    //annealState->previousPlanExpectedTime =  DBL_MAX;
    //annealState->previousPath = NIL;
    //state->hook = annealState;

    //state->previousPlanCost = PLAN_MAX_COST;
    //state->previousPlan = NULL;
    //state->previousPlanExpectedTime = DBL_MAX;
    //state->previousPath = NIL;
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

// update best plan
static void updateBestPlan (OptimizerState *state);

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
		/*
        // update best plan
        if(state->currentCost < state->bestPlanCost)
        {
            state->bestPlanCost = state->currentCost;
            state->bestPlan = strdup(state->currentPlan);
            state->bestPlanExpectedTime = estimateRuntime(state);
            DEBUG_LOG("PLAN: %s", state->bestPlan);
        }
		 */
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


/*
	// intitialize optimizer state
    state = createOptState();
    if (opt->initialize)
        opt->initialize(state);
    state->maxPlans = getIntOption(OPTION_COST_BASED_MAX_PLANS);

    //double temp = 10000;
    //double coolingRate = 0.5;

    // main loop -> create one plan in each iteration
    while(temp > 1)
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

    	if(state->previousPlanCost != PLAN_MAX_COST)
    	{
    		// compute acceptance probability
    		double p = ((double) (state->previousPlanCost - state->currentCost))/temp;
    		double ap = pow(2.71828, p);
    		int random = 1; //need to be changed to random value between [0,1)
    		//double random = rand()/(RAND_MAX+1.0);
    		DEBUG_LOG("random = %f, ap = %f, previous cost = %lld, current cost = %lld", random, ap, state->previousPlanCost, state->currentCost);

    		//
    		//  back to previous plan or just use current plan
    		//  if ap > random, apply this current plan
    		//  if app <= random, back to previous plan and based on previous plan to generate next plan
    		//  current means the neighbor of previous one
    		//  random is between [0,1)
    		//
    		DEBUG_LOG("***************Back to previous plan***************** \n");
    		if(ap <= random)
    		{
//    			if(state->previousPlanCost != PLAN_MAX_COST)
    				state->currentCost = state->previousPlanCost;
//    			if(state->previousPlan != NULL)
    				state->currentPlan = strdup(state->previousPlan);
//    			if(state->previousPath != NIL)
    				state->curPath = copyObject(state->previousPath);
    			//state->currentPlanExpectedTime = state->previousPlanExpectedTime;
    			DEBUG_LOG("PLAN: %s", state->currentPlan);
    			//DEBUG_LOG("modify current X is %s, previous X is %s", beatify(nodeToString(state->curPath)),beatify(nodeToString(state->previousPath)));
    		}
    		else
    		{
    			//before to generate next plan, store current plan
    			state->previousPlanCost = state->currentCost;
    			state->previousPlan = strdup(state->currentPlan);
    			DEBUG_LOG("best cost = %lld, previous cost = %lld, current cost = %lld", state->bestPlanCost, state->previousPlanCost, state->currentCost);
    		}
    	}
    	else
    	{
    		state->previousPlanCost = state->currentCost;
    		state->previousPlan = strdup(state->currentPlan);
    	}

    	// update best plan
    	DEBUG_LOG("***************Compare to best plan***************** \n");
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

    	//temp *= 1 - coolingRate;
    }
    */
    DEBUG_LOG("BEST PLAN COST: %d \n", state->bestPlanCost);
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
	updateBestPlan(state);

    //List *X1 = state->curPath;
    //List *Y1 = state->fixedPath;
    List *X1 = state->fixedPath;
    List *Y1 = state->curPath;
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
    state->fixedPath = copyList(Y1); //X1
    state->curPath = NIL;  //Y1
    state->numChoices = NIL; //Z1

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
    List *fixedPath = state->fixedPath;
    List *curPath = state->curPath;
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

    state->fixedPath = fixedPath;
    state->curPath = curPath;
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

static void
updateBestPlan (OptimizerState *state)
{
	if(state->currentCost < state->bestPlanCost)
	{
		state->bestPlanCost = state->currentCost;
		state->bestPlan = strdup(state->currentPlan);
		state->bestPlanExpectedTime = estimateRuntime(state);
		DEBUG_LOG("PLAN: %s", state->bestPlan);
	}
}

static boolean
simannInitialize(OptimizerState *state)
{
    AnnealingState *annealState = NEW(AnnealingState);
    annealState->previousPlanCost = PLAN_MAX_COST;
    annealState->previousPlan = NULL;
    annealState->previousPlanExpectedTime =  DBL_MAX;
    annealState->previousPath = NIL;
    annealState->temp = 10000;
    annealState->coolingRate = 0.5;

    state->hook = annealState;


    return TRUE;
}

static boolean
simannGenerateNextChoice (OptimizerState *state)
{
	AnnealingState *state1 = (AnnealingState *)(state->hook);
	if(state1->previousPlanCost != PLAN_MAX_COST)
	{
		// compute acceptance probability
		double p = ((double) (state1->previousPlanCost - state->currentCost))/state1->temp;
		double ap = pow(2.71828, p);
		int random = 1; //need to be changed to random value between [0,1)
		//double random = rand()/(RAND_MAX+1.0);
		DEBUG_LOG("temp = %f, ap = %f, previous cost = %lld, current cost = %lld", state1->temp, ap, state1->previousPlanCost, state->currentCost);

		//  back to previous plan or just use current plan
		//  if ap > random, apply this current plan
		//  if ap <= random, back to previous plan and based on previous plan to generate next plan
		//  current means the neighbor of previous one
		//  random is between [0,1)
		if(ap <= random)
		{
			DEBUG_LOG("**Back to previous plan** \n");
			state->currentCost = state1->previousPlanCost;
			state->currentPlan = strdup(state1->previousPlan);
			state->curPath = copyObject(state1->previousPath);
			DEBUG_LOG("PLAN: %s", state->currentPlan);
		}
		else
		{
			//before to generate next plan, store current plan
			state1->previousPlanCost = state->currentCost;
			state1->previousPlan = strdup(state->currentPlan);
			DEBUG_LOG("best cost = %lld, previous cost = %lld, current cost = %lld", state->bestPlanCost, state1->previousPlanCost, state->currentCost);
		}
	}
	else
	{
		state1->previousPlanCost = state->currentCost;
		state1->previousPlan = strdup(state->currentPlan);
	}

	// update best plan
	updateBestPlan(state);
	//minus temp
	state1->temp *= 1 - state1->coolingRate;
    //store previous path before generate next path
	state1->previousPath = copyObject(state->curPath);
	DEBUG_LOG("old X is %s", beatify(nodeToString(state1->previousPath)));


	//following are generating next path
	//List *fixedPath = state->fixedPath;  //X1 0
	List *curPath = state->curPath;        //Y1 [0 0]
	List *numChoices = state->numChoices;  //Z1 [2 2]

	List *fixedPathHelp = NIL;
	List *numChoicesHelp = NIL;

	int curPathLen = LIST_LENGTH(curPath);
	int pos = rand() % curPathLen;
	DEBUG_LOG("pos = %d\n",pos);

	for(int i=0; i <= pos; i++)
	{
		int c = getHeadOfListInt(curPath);
		int n = getHeadOfListInt(numChoices);
		fixedPathHelp = appendToTailOfListInt(fixedPathHelp, c);
		numChoicesHelp = appendToTailOfListInt(numChoicesHelp, n);
		curPath = removeFromHead(curPath);
		numChoices = removeFromHead(numChoices);
	}

	state->fixedPath = copyList(fixedPathHelp);
	state->curPath = NIL;
	state->numChoices = NIL;

	DEBUG_LOG("new X is %s", beatify(nodeToString(fixedPathHelp)));

	return (state->fixedPath != NIL);

	//return TRUE;
}

static boolean
simannContinueOptimization (OptimizerState *state)
{
	AnnealingState *state1 = (AnnealingState *)(state->hook);
	return state1->temp > 1;
    //return TRUE;
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
        DEBUG_LOG("thisChoice = %d",nextChoice);
    }
    else
    {
        choice = getHeadOfListInt(fixedPath);
        curPath = appendToTailOfListInt(curPath, choice);
        fixedPath = removeFromHead(fixedPath);
        numChoicesOnPath = appendToTailOfListInt(numChoicesOnPath, numChoices);
        DEBUG_LOG("nextChoice = %d",choice);
    }

    state->curPath = curPath;
    state->fixedPath = fixedPath;
    state->numChoices = numChoicesOnPath;

    DEBUG_LOG("optimizer data structures are:\n X:%s\n Y:%s\n Z:%s\n",nodeToString(fixedPath),
            nodeToString(curPath),nodeToString(numChoicesOnPath));

    return choice;
}
