/*-----------------------------------------------------------------------------
 *
 * expr_to_constraint.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SYMBOLIC_EVAL_EXPR_TO_CONSTRAINT_H_
#define INCLUDE_SYMBOLIC_EVAL_EXPR_TO_CONSTRAINT_H_

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
#include "model/set/hashmap.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"

#define DEFAULT_NUM_COLS 100
#define DEFAULT_COLNAME_SIZE 20

typedef struct CplexObjects {
	char *tableName;
	HashMap *attrIndex;        // hashmap attributename -> attribute index in obj
	double *obj[DEFAULT_NUM_COLS];
	double *lb[DEFAULT_NUM_COLS];
	double *ub[DEFAULT_NUM_COLS];
	char **colname;
} CplexObjects;


extern int exprToEval(Node *expr, CPXENVptr env, CPXLPptr lp);


#endif /* INCLUDE_SYMBOLIC_EVAL_EXPR_TO_CONSTRAINT_H_ */
