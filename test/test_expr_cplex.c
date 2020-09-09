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
#include "model/expression/expr_to_constraint.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include <ilcplex/cplex.h>

/* internal tests */
//static rc testcopyAttributeReference (void);
static rc testLogicExpr (void);
static rc testCPLEXCompile (void);
static rc testHLCompile (void);
static rc testCompilationStack (void);
static rc testCPLEXOpt (void);

/* check equal model */
rc
testCplexExpr(void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testHLCompile(), "test high-level compilation from history to case exprs");
    RUN_TEST(testCPLEXCompile(), "test compiling to cplex format problem");
    RUN_TEST(testCompilationStack(), "test entire compilation stack");
    RUN_TEST(testCPLEXOpt(), "test simple cplex optimization");
    RUN_TEST(testLogicExpr(), "test cplex for logical expressions");

    return PASS;
}

static rc
testLogicExpr (void)
{
    Node *test = (Node *) createOpExpr("AND", LIST_MAKE(createOpExpr("<", LIST_MAKE(createOpExpr("+", LIST_MAKE(createConstInt(1), createConstInt(2))), createOpExpr("+", LIST_MAKE(createConstInt(2), createConstInt(3))))), createOpExpr("=", LIST_MAKE(createConstInt(1), createConstInt(1)))));
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
    Node *test = (Node *) createOpExpr("AND", LIST_MAKE(createOpExpr("<", LIST_MAKE(createOpExpr("+", LIST_MAKE(createConstInt(1), createConstInt(2))), createOpExpr("+", LIST_MAKE(createConstInt(2), createConstInt(3))))), createOpExpr("=", LIST_MAKE(createConstInt(1), createConstInt(1)))));
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
    RenamingCtx* renamingCtx = newRenamingCtx();

    List *history = NIL;
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))));
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))));
    history = appendToTailOfList(history, createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60)))));
    List *caseExprs = historyToCaseExprsFreshVars(history, renamingCtx);
    INFO_LOG("history is %s", beatify(nodeToString(caseExprs)));

    return PASS;
}

static rc
testCompilationStack (void)
{
    RenamingCtx* renamingCtx = newRenamingCtx();
    List *history = NIL;
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))));
    history = appendToTailOfList(history, createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))));
    history = appendToTailOfList(history, createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60)))));
    
    List *caseExprs = historyToCaseExprsFreshVars(history, renamingCtx);
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

static rc
testCPLEXOpt (void)
{
    // test < 6 AND test >= 10
    // Node *test = (Node *) createOpExpr("AND", LIST_MAKE(createOpExpr("<", LIST_MAKE(createSQLParameter("test"), createConstInt(6))), createOpExpr("NOT", singleton(createOpExpr("<", LIST_MAKE(createSQLParameter("test"), createConstInt(10)))))));
    Node *test = (Node *) createCaseExpr(NULL, singleton(createCaseWhen((Node *)createOpExpr("<", LIST_MAKE(createConstInt(2), createConstInt(3))), (Node *)createConstInt(10))), (Node *)createConstInt(100));
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    exprToConstraints(test, ctx);
    LPProblem *lp = newLPProblem(ctx);

    INFO_LOG("%s", beatify(nodeToString(ctx->variableMap)));
    Node *previousOrigin = NULL;
    FOREACH(Constraint, c, ctx->constraints)
    {
        if(c->originalExpr != previousOrigin)
        {
            INFO_LOG("========================");
            INFO_LOG("%s", nodeToString(c->originalExpr));
            INFO_LOG("------------------------");
        }
        INFO_LOG(cstringConstraint(c, FALSE));
        previousOrigin = c->originalExpr;
    }
    INFO_LOG(cstringLPProblem(lp, TRUE));

    int cplexStatus = 0;
    CPXENVptr cplexEnv = CPXopenCPLEX(&cplexStatus);
    if (cplexEnv == NULL) ERROR_LOG("Could not open CPLEX environment."); else INFO_LOG("CPLEX environment opened.");
	cplexStatus = CPXsetintparam(cplexEnv, CPXPARAM_ScreenOutput, CPX_ON);
    cplexStatus ^= CPXsetintparam(cplexEnv, CPXPARAM_Read_DataCheck, CPX_DATACHECK_ASSIST);
    if(cplexStatus) ERROR_LOG("Couldn't turn on screen output or data checking...");

    CPXLPptr cplexLp = CPXcreateprob(cplexEnv, &cplexStatus, "gpromlp");
    cplexStatus = CPXchgobjsen(cplexEnv, cplexLp, CPX_MAX);

    cplexStatus = CPXnewcols(cplexEnv, cplexLp, lp->ccnt, lp->obj, lp->lb, lp->ub, lp->types, lp->colname); // TODO: lb and ub
    if(cplexStatus) ERROR_LOG("CPLEX failure - CPXnewcols"); else INFO_LOG("CPXnewcols succeeded.");

    cplexStatus = CPXaddrows(cplexEnv, cplexLp, 0, lp->rcnt, lp->nzcnt, lp->rhs, lp->sense, lp->rmatbeg,
                    lp->rmatind, lp->rmatval, NULL, NULL);
    if(cplexStatus) ERROR_LOG("CPLEX failure - CPXaddrows"); else INFO_LOG("CPXaddrows succeded.");

    INFO_LOG("CPXgetnumnz %d", CPXgetnumnz(cplexEnv, cplexLp));

    cplexStatus = CPXmipopt(cplexEnv, cplexLp);
    if(cplexStatus) {
        INFO_LOG("Error evaluating LP.");
        return FAIL;
    }
    else
    {
        INFO_LOG("Evaluated LP.");
    }

    double x[lp->ccnt];
    switch(CPXgetstat(cplexEnv, cplexLp)) {
        case 101:
            INFO_LOG("Optimal solution found");
            CPXgetx(cplexEnv, cplexLp, x, 0, lp->ccnt-1);
            for(int i = 0; i < lp->ccnt; i++) INFO_LOG("Col. %s opt. val. is %f", lp->colname[i], x[i]);
            break;
        case 103:
            INFO_LOG("Integer infeasible.");
            break;
        case 110:        
        case 114:
            INFO_LOG("No solution exists.");
            break;
        default:
            INFO_LOG("Something else... value %d", CPXgetstat(cplexEnv, cplexLp));
    }

    cplexStatus = CPXfreeprob(cplexEnv, &cplexLp);
    cplexStatus = CPXcloseCPLEX(&cplexEnv);
    if(cplexStatus) ERROR_LOG("Problem closing CPLEX environment.");

    return PASS;
}