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
static rc testHLCompile (void);
static rc testCompilationStack (void);


/* check equal model */
rc
testCplexExpr(void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testLogicExpr(), "test cplex for logical expressions");
    RUN_TEST(testHLCompile(), "test high-level compilation from history to case exprs");
    RUN_TEST(testCPLEXCompile(), "test compiling to cplex format problem");
    RUN_TEST(testCompilationStack(), "test entire compilation stack");

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
    Node *test = (Node *) createOpExpr("<", LIST_MAKE(createOpExpr("+", LIST_MAKE(createConstInt(1), createConstInt(2))), createOpExpr("+", LIST_MAKE(createConstInt(2), createConstInt(3)))));
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    exprToConstraints(test, ctx);
    FOREACH(Constraint, c, ctx->constraints)
    {
        INFO_LOG("---BEGIN CONSTRAINT---");
        FOREACH(KeyValue, t, c->terms)
        {
            INFO_LOG("Constraint term: %d * %s", INT_VALUE(t->key), ((SQLParameter *)(t->value))->name);
        }
        INFO_LOG("%s %d", ConstraintSenseToString(c->sense), c->rhs);
        INFO_LOG("---END CONSTRAINT---");
    }
    //TODO create constraint
    //TODO solve constraint
    //TODO LOG constraints
    //TODO compare result against expected result
    return PASS;
}

static rc
testCPLEXCompile (void)
{
    initializeCPlex();
    Node *test = (Node *) createOpExpr("<", LIST_MAKE(createConstInt(3), createConstInt(4)));
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    exprToConstraints(test, ctx);
    LPProblem *lp = newLPProblem(ctx);
    for(int i = 0; i < lp->ccnt; i++) {
        INFO_LOG("- Variable %s", lp->colname[i]);
    }
    for(int i = 0; i < lp->rcnt; i++)
    {
        INFO_LOG("- Constraint beginning at rmatval/rmatind index %d", lp->rmatbeg[i]);
        INFO_LOG("- Sense is %c", lp->sense[i]);
        INFO_LOG("- RHS is %f", lp->rhs[i]);
    }

    return PASS;
}

static rc
testHLCompile (void)
{
    initializeCPlex();
    List *history = NIL;
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))));
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))));
    history = appendToTailOfList(history, createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60)))));
    List *caseExprs = historyToCaseExprsFreshVars(history);
    INFO_LOG("history is %s", beatify(nodeToString(caseExprs)));

    return PASS;
}

static rc
testCompilationStack (void)
{
    initializeCPlex();
    List *history = NIL;
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))));
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))));
    history = appendToTailOfList(history, createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60)))));
    
    List *caseExprs = historyToCaseExprsFreshVars(history);
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    FOREACH(Node, e, caseExprs)
    {
        exprToConstraints(e, ctx);
    }
    LPProblem *lp = newLPProblem(ctx);
    INFO_LOG("entire compilation stack is");
    for(int i = 0; i < lp->ccnt; i++) {
        INFO_LOG("- Variable %s", lp->colname[i]);
    }
    for(int i = 0; i < lp->rcnt; i++)
    {
        INFO_LOG("- Constraint beginning at rmatval/rmatind index %d", lp->rmatbeg[i]);
        INFO_LOG("- Sense is %c", lp->sense[i]);
        INFO_LOG("- RHS is %f", lp->rhs[i]);
    }

    return PASS;
}