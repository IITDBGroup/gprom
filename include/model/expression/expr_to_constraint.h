#include "common.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_block/query_block.h"
#include "utility/enum_magic.h"

#ifndef EXPR_TO_CONSTRAINT_H
#define EXPR_TO_CONSTRAINT_H

NEW_ENUM_WITH_TO_STRING(ConstraintSense,
                        CONSTRAINT_GE,
                        CONSTRAINT_G,
                        CONSTRAINT_LE,
                        CONSTRAINT_L,
                        CONSTRAINT_E
);

typedef struct RenamingCtx {
    HashMap *map;
} RenamingCtx;

typedef struct Constraint {
    ConstraintSense sense;
    List *terms;
	Constant *rhs;
//    int rhs; //TODO should allow floats too
    Node *originalExpr;
} Constraint;

typedef struct {
    int current_expr;
    List *variables;
    HashMap *variableMap;
    HashMap *reuseMap;
    List *constraints;
    List *caseConds; // case condition variable names
    List *deletes; // delete condition resultants
} ConstraintTranslationCtx;

typedef struct {
    // cols
    int ccnt;
    char **colname;
    double *obj;
    double *lb;
    double *ub;
    char *types;
    // rows
    int rcnt;
    int nzcnt;
    double *rhs;
    char *sense;
    int *rmatbeg;
    int *rmatind;
    double *rmatval;
} LPProblem;

/* create renaming ctx */
extern RenamingCtx *newRenamingCtx (void);

/* create constraint translation ctx */
extern ConstraintTranslationCtx *newConstraintTranslationCtx (void);

/* turn history into case statements with fresh variables */
extern List *historyToCaseExprsFreshVars (List *history, ConstraintTranslationCtx *translationCtx, RenamingCtx *renameCtx);

/* create MILP constraints from an expression tree */
extern ConstraintTranslationCtx *exprToConstraints (Node *expr, ConstraintTranslationCtx *ctx);

/* create CPLEX format problem from Constraints  */
extern LPProblem *newLPProblem (ConstraintTranslationCtx *ctx);

extern int executeLPProblem (LPProblem *lp);

extern char *cstringLPProblem (LPProblem *lp, boolean details);
extern char *cstringConstraint (Constraint *constraint, boolean origin, int padLength);
extern char *cstringConstraintTranslationCtx(ConstraintTranslationCtx *ctx, boolean origin);
#endif
