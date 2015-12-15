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

typedef struct BalancedInterval
{
	int lhFlag;            // lhFlag = 0 means "Low", lhFlag = 1 means "High", lhFlag = 2 means "need low add one", lhFlag = 3 means "need high minus one"
	List *beginInterval;   // example: [000, 111]   beginIterval = 000, endInterval = 111
	List *endInterval;
	int numContinues;      // When using callback generate next plan, get the first several continuous same number, example: [000, 010], numContinues = 1, the first 0 same
    int previousLH;        // If previous one go left "0", this one should go right "1"
    List *helpPath;        // if lhFlag = 2 or 3, store some value into helpPath used to generate next path when call callback function, like the fixedPath in heuristic method
    List *helpEnd;         // if helpPath store the beginInterval, the helpEnd store endInterval
    int helpFlag;
    //List *currentList;     // The current path (the result, the middle one which will be return after all callback fucntions) and path it to generationNextPlan function
}BalancedInterval;

typedef struct BalancedState
{
	int count;
	List *fifo;
	BalancedInterval *bl;
}BalancedState;

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

// balanced annealing
static boolean balancedInitialize(OptimizerState *state);
static boolean balancedGenerateNextChoice (OptimizerState *state);
static boolean balancedContinueOptimization (OptimizerState *state);
static int balancedCallback (OptimizerState *state, int numChoices);

static List *addOne(List *curPath, List *numChoices);
static List* minusOne(List *curPath, List *numChoices);
static boolean checkEqual(List *l1, List *l2);
static BalancedInterval* copyBalancedInterval(BalancedInterval *bl);
static int findCommonPrefixLength(List *l1, List *l2);
static List* generateNextPlan(int hlflag, List *l, List *h, List *numChoice);

/* 2-competitive version of the continue function that stops optimization when
 * more time has been spend on optimization then the expected runtime of the
 * current best plan. */
//static boolean balancedContinueOptimization (OptimizerState *state);

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
//            opt->initialize = NULL;
//            opt->shouldContinue = balancedContinueOptimization;
//            opt->callback = exhaustiveCallback;
//            opt->generateNextChoice = exhaustiveGenerateNextChoice;
        	opt->initialize = balancedInitialize;
        	opt->shouldContinue = balancedContinueOptimization;
        	opt->callback = balancedCallback;
        	opt->generateNextChoice = balancedGenerateNextChoice;
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
    if (streq(pluginName,"balance"))
        chooseOptimizerPlugin(OPTIMIZER_BALANCED);
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

//static boolean
//balancedContinueOptimization (OptimizerState *state)
//{
//    return (state->optTime < state->bestPlanExpectedTime);
//}

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


static boolean
balancedInitialize(OptimizerState *state)
{
	BalancedState *balancedState = NEW(BalancedState);
	balancedState->count = 1;
	balancedState->fifo = NIL;

	BalancedInterval *balancedItvl = NEW(BalancedInterval);
	balancedItvl->lhFlag = 0;
	balancedItvl->beginInterval = NIL;
	balancedItvl->endInterval = NIL;
	balancedItvl->numContinues = -1;
	balancedItvl->previousLH = -1;
	balancedItvl->helpPath = NIL;
	balancedItvl->helpEnd = NIL;
	balancedItvl->helpFlag = 0;

	balancedState->bl = balancedItvl;

    state->hook = balancedState;

    state->numChoices = NIL;
    state->curPath = NIL;

    return TRUE;
}

static List*
addOne(List *curPath, List *numChoices)
{
	List *ycurPath = copyObject(curPath);
	List *znumChoices = copyObject(numChoices);

	int cnt = 0;
	int len = LIST_LENGTH(curPath);
	for(int i=0; i<len; i++)
	{
		int y = getTailOfListInt(ycurPath);
		int z = getTailOfListInt(znumChoices);

		if(y+1 == z)
		{
			ycurPath = removeFromTail(ycurPath);
			znumChoices = removeFromTail(znumChoices);
			cnt ++;
		}
		else
		{
			y = y + 1;
			ycurPath = removeFromTail(ycurPath);
			ycurPath = appendToTailOfListInt(ycurPath,y);
			for(int j=0; j<cnt; j++)
			{
				ycurPath = appendToTailOfListInt(ycurPath,0);
			}

			break;
		}
	}

	return ycurPath;
}

static List*
minusOne(List *curPath, List *numChoices)
{
	List *ycurPath = copyObject(curPath);
	List *znumChoices = copyObject(numChoices);
    List *leftNumChoices = NIL;

	//int cnt = 0;
	int len = LIST_LENGTH(curPath);
	for(int i=0; i<len; i++)
	{
		int y = getTailOfListInt(ycurPath);
		int z = getTailOfListInt(znumChoices);

		if(y == 0)
		{
			ycurPath = removeFromTail(ycurPath);
			znumChoices = removeFromTail(znumChoices);
			leftNumChoices = appendToHeadOfListInt(leftNumChoices, z);
			//cnt ++;
		}
		else
		{
			y = y - 1;
			ycurPath = removeFromTail(ycurPath);
			ycurPath = appendToTailOfListInt(ycurPath,y);
			FOREACH_INT(c, leftNumChoices)
			{
				ycurPath = appendToHeadOfListInt(ycurPath,c-1);
			}
			break;
		}
	}

	return ycurPath;
}

//TODO just use equal
static boolean
checkEqual(List *l1, List *l2)
{
	boolean flag = TRUE;
	int len1 = LIST_LENGTH(l1);
	int len2 = LIST_LENGTH(l1);

	if(len1 == len2)
	{
		FORBOTH(int, n1, n2, l1, l2)
		{
			if(n1 != n2)
				flag = FALSE;
		}
	}
	else
	{
		flag = FALSE;
	}

	return flag;
}

static BalancedInterval*
copyBalancedInterval(BalancedInterval *bl)
{
	BalancedInterval *balancedItvl = NEW(BalancedInterval);
	balancedItvl->lhFlag = bl->lhFlag;
	balancedItvl->beginInterval = copyObject(bl->beginInterval);
	balancedItvl->endInterval = copyObject(bl->endInterval);
	balancedItvl->numContinues = bl->numContinues;
	balancedItvl->previousLH = bl->previousLH;
	balancedItvl->helpPath = copyObject(bl->helpPath);

	return balancedItvl;
}

static int
findCommonPrefixLength(List *l1, List *l2)
{
	int prefixLength = 0;
	FORBOTH(int, l, h, l1, l2)
	{
		if(l == h)
			prefixLength ++;
		else
			break;
	}

	return prefixLength;
}

static List*
generateNextPlan(int hlflag, List *l, List *h, List *numChoice)
{
	int commonPrefixLength = findCommonPrefixLength(l,h);
	List *num = copyObject(numChoice);
	List *result = NIL;
    int flag = hlflag;

//	int i = 0;
//	FOREACH_INT(c, l)
//	{
//		if(i<commonPrefixLength)
//			result = appendToTailOfListInt(result, c);
//		else
//		{
//			if(flag == 0)
//			{
//				result = appendToTailOfListInt(result, 0);
//				flag = 1;
//			}
//			else
//			{
//				result = appendToTailOfListInt(result, 1);
//				flag = 0;
//			}
//		}
//	    i ++;
//	}

    int i = 0;
    int helpFlag = 0;
	FORBOTH_INT(cl, ch, l, h)
	{
		int numValue = getHeadOfListInt(num);
		num = removeFromHead(num);
		if(i<commonPrefixLength)
			result = appendToTailOfListInt(result, ch);
		else
		{
			int len = ch - cl;
			if (helpFlag ==0 && len > 1 && len % 2 == 0){  //middle case, the number of choice is a odd value
				result = appendToTailOfListInt(result, len/2);
				helpFlag = 1;
			}
			else if(helpFlag == 1 && numValue>2 && numValue % 2 != 0)
				result = appendToTailOfListInt(result, (numValue - 1) / 2);
			else if(flag == 0)
			{
				result = appendToTailOfListInt(result, 0);
				flag = 1;
			}
			else
			{
				result = appendToTailOfListInt(result, numValue-1);
				flag = 0;
			}
		}
	    i ++;
	}


	return result;
}

static boolean
balancedGenerateNextChoice (OptimizerState *state)
{
	updateBestPlan(state);
	BalancedState *balSt = (BalancedState *)(state->hook);
	int cnt = balSt->count;
	List *fifoList = balSt->fifo;
	BalancedInterval *bl = balSt->bl;
	bl->helpFlag = 0;

	DEBUG_LOG("new structures are:\n curPath:%s\n beginInterval:%s\n endInterval:%s\n",
	               nodeToString(state->curPath),nodeToString(bl->beginInterval),nodeToString(bl->endInterval));

	//boolean returnFlag = FALSE;
	if(cnt == 1)
	{
		bl->beginInterval = copyObject(state->curPath);
		state->curPath = NIL;
	}
	else if(cnt == 2)
	{
		bl->endInterval = copyObject(state->curPath);
		bl->numContinues = findCommonPrefixLength(bl->beginInterval, bl->endInterval);
		state->curPath = NIL;
		bl->helpPath = copyObject(bl->beginInterval);   //used to get common value, use the numContinues to control
		bl->helpEnd = copyObject(bl->endInterval);    //used to compute the middle case
		fifoList = appendToTailOfList(fifoList, copyBalancedInterval(bl));
	}
	else
	{
		//bl = copyBalancedInterval ((BalancedInterval *) getHeadOfListP(fifoList));

	    boolean needLowToMid = TRUE;
	    boolean needMidToHigh = TRUE;

	    // do we need to generate [low,mid]
	    List *temp1 = addOne(bl->beginInterval, state->numChoices);
	    if (checkEqual(state->curPath,temp1))
	        needLowToMid = FALSE;
	    // do we need to generate [mid,high]
	    List *temp2 = addOne(state->curPath, state->numChoices);
        if (checkEqual(bl->endInterval,temp2))
            needMidToHigh = FALSE;

        if(needLowToMid)
        {
        	BalancedInterval *bl1 = NEW(BalancedInterval);
        	//set bl1
        	bl1->lhFlag = 0;
        	bl1->helpPath = NIL;
        	bl1->beginInterval = copyObject(bl->beginInterval);
        	bl1->endInterval = copyObject(state->curPath);
        	bl1->previousLH = -1;
        	bl1->numContinues = findCommonPrefixLength(bl1->beginInterval, bl1->endInterval);
        	bl1->helpEnd = copyObject(bl1->endInterval);

        	//predict next plan: if equal to low, set bl1->lhFlag = 2; if equal to high, set bl1->lfFlag = 3.
        	//when callback, check lfFlag, if 2, do not do left right left... directly add one to low and return; if 3, minus to high and return.
        	List *nextPlan = generateNextPlan(bl1->lhFlag, bl1->beginInterval, bl1->endInterval, state->numChoices);
        	if(checkEqual(nextPlan, bl1->beginInterval))
        	{
        		bl1->lhFlag = 2;
        		bl1->helpPath = addOne(bl1->beginInterval, state->numChoices);
        	}
        	else if(checkEqual(nextPlan, bl1->endInterval))
        	{
        		bl1->lhFlag = 3;
        		bl1->helpPath = minusOne(bl1->endInterval, state->numChoices);
        	}
        	else
        		bl1->helpPath = copyObject(bl1->beginInterval);
        	//add bl1 to the fifo
            fifoList = appendToTailOfList(fifoList, bl1);
        }

        if(needMidToHigh)
        {
        	BalancedInterval *bl2 = NEW(BalancedInterval);
        	//set bl2
        	bl2->lhFlag = 1;
        	bl2->helpPath = NIL;
        	bl2->beginInterval = copyObject(state->curPath);
        	bl2->endInterval = copyObject(bl->endInterval);
        	bl2->previousLH = -1;
        	bl2->numContinues = findCommonPrefixLength(bl2->beginInterval, bl2->endInterval);
        	bl2->helpEnd = copyObject(bl2->endInterval);

        	//predict next plan: if equal to low, set bl1->lhFlag = 2; if equal to high, set bl1->lfFlag = 3.
        	//when callback, check lfFlag, if 2, do not do left right left... directly add one to low and return; if 3, minus to high and return.
        	List *nextPlan = generateNextPlan(bl2->lhFlag, bl2->beginInterval, bl2->endInterval, state->numChoices);
        	if(checkEqual(nextPlan, bl2->beginInterval))
        	{
        		bl2->lhFlag = 2;
        		bl2->helpPath = addOne(bl2->beginInterval, state->numChoices);
        	}
        	else if(checkEqual(nextPlan, bl2->endInterval))
        	{
        		bl2->lhFlag = 3;
        		bl2->helpPath = minusOne(bl2->endInterval, state->numChoices);
        	}
        	else
        		bl2->helpPath = copyObject(bl2->beginInterval);

        	//add bl1 to the fifo
            fifoList = appendToTailOfList(fifoList, bl2);
        }

		if(fifoList != NIL)
			fifoList = removeFromHead(fifoList);

		if(fifoList != NIL)
			balSt->bl = copyBalancedInterval ((BalancedInterval *) getHeadOfListP(fifoList));
	}

    //print fifo
//    if(fifoList != NIL)
//    {
//    	FOREACH(BalancedInterval, bll, fifoList)
//		{
//    		DEBUG_LOG("fifo are:\n Y:%s\n Z:%s\n",
//    		      nodeToString(bll->beginInterval),nodeToString(bll->endInterval));
//		}
//    }
//    DEBUG_LOG("fifo2 are:\n Y:%s\n Z:%s\n",
//        		      nodeToString(balSt->bl->beginInterval),nodeToString(balSt->bl->endInterval));
	(balSt->count) ++;
	state->numChoices = NIL;
	state->curPath = NIL;
	balSt->fifo = fifoList;
	state->hook = balSt;

    return (fifoList != NIL || cnt == 1);

}

static boolean
balancedContinueOptimization (OptimizerState *state)
{

    return TRUE;
}

static int
balancedCallback (OptimizerState *state, int numChoices)
{
	int cnt = ((BalancedState *)(state->hook))->count;
	int choice = -1;
    //List *fixedPath = state->fixedPath;
    List *curPath = state->curPath;
    List *numChoicesOnPath = state->numChoices;
	if(cnt == 1)
	{
        curPath = appendToTailOfListInt(curPath, 0);
	}
	else if(cnt == 2)
	{
		choice = numChoices-1;
		curPath = appendToTailOfListInt(curPath, choice);
	}
	else
	{
		//BalancedInterval *bl = copyBalancedInterval (((BalancedState *)(state->hook))->bl);
		BalancedInterval *bl = ((BalancedState *)(state->hook))->bl;

		if(bl->lhFlag < 2 )
		{
			if(bl->numContinues > 0)  //copy first several same value
			{
				choice = getHeadOfListInt(bl->helpPath);
				curPath = appendToTailOfListInt(curPath, choice);
				bl->helpPath = removeFromHead(bl->helpPath);
				bl->helpEnd = removeFromHead(bl->helpEnd);
				bl->numContinues --;
			}
			else  //after copy same, do left first or right first
			{
				int cl = getHeadOfListInt(bl->helpPath);
				int ch = getHeadOfListInt(bl->helpEnd);
				bl->helpPath = removeFromHead(bl->helpPath);
				bl->helpEnd = removeFromHead(bl->helpEnd);
				int len = ch - cl;

				if(bl->previousLH == -1)  //check first one
				{
					if(bl->helpFlag == 0 && len > 1 && len % 2 == 0) //after numContinues iterations (after copy the common prefix), next iteration helpFlag = 0, after that helpFlag = 1.
					{                                                //For middle case, helpFlag == 0, choose middle value from the interval, after this time(helpFlag = 1), choose middle value from the numChoice.
						choice = len/2;
						curPath = appendToTailOfListInt(curPath, choice);
						bl->helpFlag = 1;
					}
					else if(bl->helpFlag == 1 && numChoices>2 && numChoices % 2 != 0)
					{
						choice = (numChoices-1)/2;
						curPath = appendToTailOfListInt(curPath, choice);
					}
					else if(bl->lhFlag == 0)  //check left first
					{
						choice = 0;
						curPath = appendToTailOfListInt(curPath, choice);
					}
					else if(bl->lhFlag == 1) //check right first
					{
						choice = numChoices - 1;
						curPath = appendToTailOfListInt(curPath, choice);
					}
					bl->previousLH = choice;
				}
				else  //after first one
				{
					if (len > 1 && len % 2 == 0){  //middle case
						choice = len/2;
						curPath = appendToTailOfListInt(curPath, len/2);
					}
					else if(bl->previousLH == 0)
					{
						choice = numChoices - 1;
						curPath = appendToTailOfListInt(curPath, choice);
					}
					else if(bl->previousLH == 1)
					{
						choice = 0;
						curPath = appendToTailOfListInt(curPath, choice);
					}
					bl->previousLH = choice;
				}
			}
		}
		else
		{
			choice = getHeadOfListInt(bl->helpPath);
			curPath = appendToTailOfListInt(curPath, choice);
			bl->helpPath = removeFromHead(bl->helpPath);
		}


//		if(bl->lhFlag == 0)
//		{
//			if(bl->numContinues > 0)  //copy first several same value
//			{
//				choice = getHeadOfListInt(bl->beginInterval);
//				curPath = appendToTailOfListInt(curPath, choice);
//				bl->beginInterval = removeFromHead(bl->beginInterval);
//				//bl->endInterval = removeFromHead(bl->endInterval);
//				bl->numContinues --;
//			}
//			else  //after copy same, do left first or right first
//			{
//				if(bl->previousLH == -1)  //check first one
//				{
//					choice = 0;
//					curPath = appendToTailOfListInt(curPath, choice);
//					bl->previousLH = choice;
//				}
//				else  //after first one
//				{
//					if(bl->previousLH == 0)
//					{
//						choice = numChoices - 1;
//						curPath = appendToTailOfListInt(curPath, choice);
//					}
//					else if(bl->previousLH == 1)
//					{
//						choice = 0;
//						curPath = appendToTailOfListInt(curPath, choice);
//					}
//					bl->previousLH = choice;
//				}
//			}
//		}
//		else if(bl->lhFlag == 1)
//		{
//			if(bl->numContinues > 0)  //copy first several same value
//			{
//				choice = getHeadOfListInt(bl->beginInterval);
//				curPath = appendToTailOfListInt(curPath, choice);
//				bl->beginInterval = removeFromHead(bl->beginInterval);
//				//bl->endInterval = removeFromHead(bl->endInterval);
//				bl->numContinues --;
//			}
//			else  //after copy same, do left first or right first
//			{
//				if(bl->previousLH == -1)  //check first one
//				{
//					choice = numChoices - 1;
//					curPath = appendToTailOfListInt(curPath, choice);
//					bl->previousLH = choice;
//				}
//				else  //after first one
//				{
//					if(bl->previousLH == 0)
//					{
//						choice = numChoices - 1;
//						curPath = appendToTailOfListInt(curPath, choice);
//					}
//					else if(bl->previousLH == 1)
//					{
//						choice = 0;
//						curPath = appendToTailOfListInt(curPath, choice);
//					}
//					bl->previousLH = choice;
//				}
//			}
//		}
//		else if(bl->lhFlag == 2 || bl->lhFlag == 3)
//		{
//			choice = getHeadOfListInt(bl->helpPath);
//			curPath = appendToTailOfListInt(curPath, choice);
//			bl->helpPath = removeFromHead(bl->helpPath);
//		}




//		else if(bl->lhFlag == 3)
//		{
//			choice = getHeadOfListInt(bl->helpPath);
//			curPath = appendToTailOfListInt(curPath, choice);
//			bl->helpPath = removeFromHead(bl->helpPath);
//		}
	}
	numChoicesOnPath = appendToTailOfListInt(numChoicesOnPath, numChoices);

    //state->fixedPath = fixedPath;
    state->curPath = curPath;
    state->numChoices = numChoicesOnPath;

    DEBUG_LOG("optimizer data structures are:\n Y:%s\n Z:%s\n",
               nodeToString(curPath),nodeToString(numChoicesOnPath));


    return choice;
}
