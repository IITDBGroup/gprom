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


char *
doCostBasedOptimization(Node *oModel, boolean applyOptimizations)
{
	StringInfo result = makeStringInfo();
	Node *rewrittenTree;
	char *rewrittenSQL = NULL;
	int n = 0;
	X1 = NIL;
	Y1 = NIL;
	Z1 = NIL;
    cost1 = 99999999;
    plan = NULL;

	while(n<4)
	{
		Node *oModel1 = copyObject(oModel);

		START_TIMER("rewrite");
		rewrittenTree = provRewriteQBModel(oModel1);
		DEBUG_LOG("provenance rewriter returned:\n\n<%s>", beatify(nodeToString(rewrittenTree)));
		INFO_LOG("provenance rewritten query as overview:\n\n%s", operatorToOverviewString(rewrittenTree));
		DOT_TO_CONSOLE(rewrittenTree);
		STOP_TIMER("rewrite");

		ASSERT_BARRIER(
				if (isA(rewrittenTree, List))
					FOREACH(QueryOperator,o,(List *) rewrittenTree)
					TIME_ASSERT(checkModel(o));
				else
					TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
		)

		if(applyOptimizations)
		{
			START_TIMER("OptimizeModel");
			rewrittenTree = optimizeOperatorModel(rewrittenTree);
			INFO_LOG("after merging operators:\n\n%s", operatorToOverviewString(rewrittenTree));
			STOP_TIMER("OptimizeModel");
		}
		else
			if (isA(rewrittenTree, List))
				FOREACH(QueryOperator,o,(List *) rewrittenTree)
				LC_P_VAL(o_his_cell) = materializeProjectionSequences (o);
			else
				rewrittenTree = (Node *) materializeProjectionSequences((QueryOperator *) rewrittenTree);
		DOT_TO_CONSOLE(rewrittenTree);

		START_TIMER("SQLcodeGen");
		appendStringInfo(result, "%s\n", serializeOperatorModel(rewrittenTree));
		STOP_TIMER("SQLcodeGen");

		rewrittenSQL = result->data;

		int cost = getCost(rewrittenSQL);
		DEBUG_LOG("Cost of the rewritten Query is = %d\n", cost);

		if(cost < cost1)
		{
			cost1 = cost;
			plan = rewrittenSQL;
			DEBUG_LOG("PLAN: %s", plan);
		}

		n++;
	}

	FREE(result);
	return plan;

}


int
callback (int numChoices)
{
	int xVal;

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
/*
    DEBUG_LOG("optimizer data structures are: X:%s\n, Y:%s\n, Z:%s\n",
            beatify(nodeToString(X1)), beatify(nodeToString(Y1)),
            beatify(nodeToString(Z1)));

    DEBUG_LOG("optimizer data structures are: X:%s,",nodeToString(X1));
    DEBUG_LOG("optimizer data structures are: Y:%s,",nodeToString(Y1));
    DEBUG_LOG("optimizer data structures are: Z:%s,",nodeToString(Z1));
*/
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
}
