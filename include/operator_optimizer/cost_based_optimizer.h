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

char *doCostBasedOptimization(Node *transOutput);
int callback (int numChoices);

#endif /* INCLUDE_OPERATOR_OPTIMIZER_COST_BASED_OPTIMIZER_H_ */
