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

NEW_ENUM_WITH_TO_STRING
(
    OptimizerPlugin,
    OPTIMIZER_EXHAUSTIVE,
    OPTIMIZER_SIMMULATED_ANNEALING,
    OPTIMIZER_BALANCED,
    OPTIMIZER_EXHAUSTIVE_ADAPTIVE
);

extern void chooseOptimizerPlugin(OptimizerPlugin typ);
extern void chooseOptimizerPluginFromString(char *pluginName);
extern char *doCostBasedOptimization(Node *oModel, boolean applyOptimizations);
extern int callback (int numChoices);

#endif /* INCLUDE_OPERATOR_OPTIMIZER_COST_BASED_OPTIMIZER_H_ */
