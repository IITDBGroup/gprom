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


char *
doCostBasedOptimization(Node *transOutput)
{

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

    DEBUG_LOG("optimizer data structures are: X:%s\n, Y:%s\n, Z:%s\n",
            beatify(nodeToString(X1)), beatify(nodeToString(Y1)),
            beatify(nodeToString(Z1)));

    return xVal;
}
