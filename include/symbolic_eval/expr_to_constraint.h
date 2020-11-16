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

// only include cplex headers if the library is available
#ifdef HAVE_LIBCPLEX
#include <ilcplex/cplex.h>
#endif

#include "common.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"

#define DEFAULT_NUM_COLS 10
#define DEFAULT_COLNAME_SIZE 20

typedef struct CplexObjects {
    char *tableName;
    HashMap *attrIndex;        // hashmap attributename -> attribute index in obj
    double *obj;
    double *lb;
    double *ub;
    char **colname;
} CplexObjects;

extern boolean exprToSat(Node *expr1, boolean inv1, Node *expr2, boolean inv2);
extern List *symbolicHistoryExe(List *exprs);
// extern int setX(int upNum, Node *cond, CPXENVptr env, CPXLPptr lp);
#endif /* INCLUDE_SYMBOLIC_EVAL_EXPR_TO_CONSTRAINT_H_ */
