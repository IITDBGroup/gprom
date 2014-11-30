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


static List *X1 = NIL;
static List *Y1 = NIL;
static List *Z1 = NIL;



char *
doCostBasedOptimization(Node *transOutput)
{
    return "";
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

    DEBUG_LOG("optimizer data structures are: X:%s\n, Y:%s\n, Z:%s\n",
            beatify(nodeToString(X1)), beatify(nodeToString(Y1)),
            beatify(nodeToString(Z1)));

    return xVal;
}
