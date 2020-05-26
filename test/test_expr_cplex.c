/*-----------------------------------------------------------------------------
 *
 * test_expr_cplex.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */


#include "model/list/list.h"
#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

/* internal tests */
//static rc testcopyAttributeReference (void);
static rc initializeCPlex(void);
static rc testLogicExpr (void);
static rc testCPLEXCompile (void);


/* check equal model */
rc
testCplexExpr(void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testLogicExpr(), "test cplex for logical expressions");
    RUN_TEST(testCPLEXCompile(), "test compiling to cplex format problem");

    return PASS;
}

static rc
initializeCPlex(void)
{
    return PASS;
}

static rc
testLogicExpr (void)
{
    initializeCPlex();
    // Node *test = (Node *) createOpExpr("<", LIST_MAKE(createConstInt(3), createConstInt(4)));
    Node *test = (Node *) createOpExpr("<", LIST_MAKE(createOpExpr("+", LIST_MAKE(createConstInt(1), createConstInt(2))), createOpExpr("+", LIST_MAKE(createConstInt(2), createConstInt(3)))));
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    exprToConstraints(test, ctx);
    FOREACH(Constraint, c, ctx->constraints)
    {
        if(c != NULL)
        {
            ERROR_LOG("---BEGIN CONSTRAINT---");
            FOREACH(KeyValue, t, c->terms)
            {
                if(t != NULL)
                {
                    ERROR_LOG("Constraint term: %d * %s", INT_VALUE(t->key), ((SQLParameter *)(t->value))->name);
                }
            }
            ERROR_LOG("%s %d", ConstraintSenseToString(c->sense), c->rhs);
            ERROR_LOG("---END CONSTRAINT---");
        }
    }
    //TODO create constraint
    //TODO solve constraint
    //TODO LOG constraints
    //TODO compare result against expected result
    return FAIL;
}

static rc
testCPLEXCompile (void)
{
    initializeCPlex();
    Node *test = (Node *) createOpExpr("<", LIST_MAKE(createConstInt(3), createConstInt(4)));
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    exprToConstraints(test, ctx);
    LPProblem *lp = newLPProblem(ctx);
    ERROR_LOG("%d columns should equal %d variables", lp->ccnt, getListLength(ctx->variables)-1); // null head of list?

    return FAIL;
}