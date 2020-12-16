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
#include "parser/parser_oracle.h"
#include <ilcplex/cplex.h>

/* internal tests */
//static rc testcopyAttributeReference (void);
static rc testLogicExpr (void);
static rc testCPLEXCompile (void);
static rc testHLCompile (void);
static rc testCompilationStack (void);
static rc testCPLEXOpt (void);
static int optimize (List *l, boolean enforce);

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
        INFO_LOG(cstringConstraint(c, TRUE));
    }

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
    INFO_LOG(cstringLPProblem(lp, TRUE));

    return PASS;
}

static rc
testHLCompile (void)
{
    RenamingCtx* renamingCtx = newRenamingCtx();

    List *history = LIST_MAKE(
        createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))),
        createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))),
        createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60))))
    );

    List *caseExprs = historyToCaseExprsFreshVars(history, renamingCtx);
    INFO_LOG("history is %s", beatify(nodeToString(caseExprs)));

    return PASS;
}

static rc
testCompilationStack (void)
{
    RenamingCtx* renamingCtx = newRenamingCtx();
    List *history = LIST_MAKE(
        createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))),
        createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))),
        createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60))))
    );
    
    List *caseExprs = historyToCaseExprsFreshVars(history, renamingCtx);
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    FOREACH(Node, e, caseExprs)
    {
        exprToConstraints(e, ctx);
    }
    LPProblem *lp = newLPProblem(ctx);
    INFO_LOG(cstringLPProblem(lp, TRUE));

    return PASS;
}

#define CPLEX_TEST(expr, result, enforce) ASSERT_EQUALS_INT(result, optimize((List *)parseFromStringOracle(expr), enforce), expr);

static rc
testCPLEXOpt (void)
{
    // Comparisons
    CPLEX_TEST(":x = 1;", 1, FALSE);

    CPLEX_TEST("x = 3; x := 3;", 1, TRUE);
    CPLEX_TEST("x = 4; x := 3;", 0, TRUE);
    CPLEX_TEST("x < 4; x := 3;", 1, TRUE);
    CPLEX_TEST("x > 4; x := 5;", 1, TRUE);

    CPLEX_TEST("x = 3.5; x := 3.5;", 1, TRUE);
    CPLEX_TEST("x < 3.5; x := 3.25;", 1, TRUE);
    CPLEX_TEST("x > 3.5; x := 3.75;", 1, TRUE);
    
    // Logical Operators
    CPLEX_TEST("(x > 2) AND (x < 1);", 1, FALSE);
    CPLEX_TEST("(x > 2) AND (x < 1);", 0, TRUE);

    // Case Expressions (w/ or w/o assignment)
    CPLEX_TEST("CASE WHEN (x < 3) THEN x ELSE 10 END;", 1, FALSE);
    CPLEX_TEST("x = 8; y := CASE WHEN (x = 8) THEN 1 ELSE 0 END;", 1, TRUE);
    CPLEX_TEST("x = 8; z := CASE WHEN (x = 8) THEN y + 3.21 ELSE y END;", 1, TRUE);

    return PASS;
}

static int optimize(List *l, boolean enforce)
{
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    FOREACH(Node, node, l) {
       exprToConstraints(node, ctx);
    }

    if(enforce) {
        Constraint *c = makeNode(Constraint);
        c->sense = CONSTRAINT_E;
        c->rhs = 1;
        c->terms = LIST_MAKE(
            createNodeKeyValue((Node*)createConstInt(1), (Node*)getNthOfListP(ctx->variables, 0))
        );

        ctx->constraints = appendToTailOfList(ctx->constraints, c);
    }

    FOREACH(Constraint, c, ctx->constraints) {
        INFO_LOG(cstringConstraint(c, TRUE));
    }

    LPProblem *lp = newLPProblem(ctx);

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
        return -1;
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
            return 1;
        case 103:
            INFO_LOG("Integer infeasible.");
            return 0;
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

    return 0;
}