/*-----------------------------------------------------------------------------
 *
 * whatif_algo.h
 *		
 *
 *		AUTHOR: Bahareh
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_WHATIF_ALGORITHM_H_
#define INCLUDE_WHATIF_ALGORITHM_H_

#include <ilcplex/cplex.h>
#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "parser/parser.h"
#include "symbolic_eval/expr_to_constraint.h"
#include "symbolic_eval/whatif_algo.h"
#include "model/set/hashmap.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"

extern List *dependAlgo(List *exprs);

#endif /* WHATIF_ALGORITHM_H_ */
