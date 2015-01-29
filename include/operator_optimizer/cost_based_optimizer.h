/*-----------------------------------------------------------------------------
 *
 * cost_based_optimizer.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_OPERATOR_OPTIMIZER_COST_BASED_OPTIMIZER_H_
#define INCLUDE_OPERATOR_OPTIMIZER_COST_BASED_OPTIMIZER_H_

#include "model/node/nodetype.h"

extern char *doCostBasedOptimization(Node *oModel, boolean applyOptimizations);
extern int callback (int numChoices);
extern void reSetX1();


#endif /* INCLUDE_OPERATOR_OPTIMIZER_COST_BASED_OPTIMIZER_H_ */
