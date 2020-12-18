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
#ifdef HAVE_LIBCPLEX
#include <ilcplex/cplex.h>
#endif


#ifdef HAVE_LIBCPLEX

/* internal tests */
//static rc testcopyAttributeReference (void);
//static rc testLogicExpr (void);
static rc testCPLEXCompile (void);
static rc testHLCompile (void);
static rc testCompilationStack (void);
static rc testCheckingForDependentUpdates(void);
static rc testCPLEXOpt (void);
static int optimize (List *l, boolean enforce);
static List *translateUpdatesToCase(char **updates);
static LPProblem *compileToLP(char *updates[]);

/* check equal model */
rc
testCplexExpr(void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testHLCompile(), "test high-level compilation from history to case exprs");
    RUN_TEST(testCPLEXCompile(), "test compiling to cplex format problem");
    RUN_TEST(testCompilationStack(), "test entire compilation stack");
    RUN_TEST(testCPLEXOpt(), "test cplex optimizations of expressions translated into MILPs");
	RUN_TEST(testCheckingForDependentUpdates(), "testing for dependency of updates");
	// RUN_TEST(testLogicExpr(), "test cplex for logical expressions");

    return PASS;
}

/* static rc */
/* testLogicExpr (void) */
/* { */
/*     Node *test = (Node *) createOpExpr("AND", LIST_MAKE(createOpExpr("<", LIST_MAKE(createOpExpr("+", LIST_MAKE(createConstInt(1), createConstInt(2))), createOpExpr("+", LIST_MAKE(createConstInt(2), createConstInt(3))))), createOpExpr("=", LIST_MAKE(createConstInt(1), createConstInt(1))))); */
/*     ConstraintTranslationCtx *ctx = newConstraintTranslationCtx(); */
/*     exprToConstraints(test, ctx); */
/* 	INFO_LOG("Cplex logic expression:\n%s", cstringConstraintTranslationCtx(ctx, TRUE)); */

/*     //TODO compare result against expected result */
/*     return PASS; */
/* } */

#define CPLEX_COMPILE(expr)												\
	do {																\
		Node *test = (Node *) parseFromStringOracle(expr);   			\
		ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();  \
		exprToConstraints(test, ctx);                                   \
		LPProblem *lp = newLPProblem(ctx);                              \
		INFO_LOG(CONCAT_STRINGS(expr, ":\n"), cstringLPProblem(lp, TRUE)); \
	} while(0)

static rc
testCPLEXCompile (void)
{
	CPLEX_COMPILE("(1 + 2) < (2 + 3) AND 1 = 1;");

    return PASS;
}

#define COMPILE_UPDATES_TEST(...)					\
	do {											\
		char *updates[] = { __VA_ARGS__, NULL };	\
		translateUpdatesToCase(updates);			\
	} while(0)

static rc
testHLCompile (void)
{
	/* char *updates[] = { */
	/* 		"UPDATE a SET a = 10 WHERE a < 4;", */
	/* 		"UPDATE a SET a = 10 WHERE a < 4;", */
	/* 		"UPDATE a SET a = 10 WHERE a < 4;", */
	/* 		NULL */
	/* }; */

	COMPILE_UPDATES_TEST(
		"UPDATE r SET a = 10 WHERE a < 4;",
		"UPDATE r SET a = 0 WHERE a = 10;",
		"UPDATE r SET b = 3 WHERE a <= 6;"
		);
	
//	translateUpdatesToCase(updates);
	
    /* List *history = LIST_MAKE( */
    /*     createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(10)))), (Node *) createOpExpr("<", LIST_MAKE(createSQLParameter("a"), createConstInt(4)))), */
    /*     createUpdate("a", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("a"), createConstInt(0)))), (Node *) createOpExpr("=", LIST_MAKE(createSQLParameter("a"), createConstInt(10)))), */
    /*     createUpdate("b", LIST_MAKE(createOpExpr("=", LIST_MAKE(createAttributeReference("b"), createConstInt(3)))), (Node *) createOpExpr("<=", LIST_MAKE(createSQLParameter("a"), createConstInt(60)))) */
    /* ); */

    /* List *caseExprs = historyToCaseExprsFreshVars(history, renamingCtx); */
    /* INFO_LOG("history is %s", beatify(nodeToString(caseExprs))); */

    return PASS;
}

static List *
translateUpdatesToCase(char **updates)
{
	List *transups = NIL;
	List *caseExprs = NIL;
    RenamingCtx* renamingCtx = newRenamingCtx();
	
	while(*updates != NULL)
	{
	    List *update = (List *) parseFromStringOracle(*updates);		
		transups = appendToTailOfList(transups, getHeadOfListP(update));
		updates++;
	}

	caseExprs = historyToCaseExprsFreshVars(transups, renamingCtx);
    DEBUG_LOG("history is %s", beatify(nodeToString(caseExprs)));

	return transups;
}

static LPProblem *
compileToLP(char *updates[])
{
	LPProblem *lp;
	List *caseExprs = translateUpdatesToCase(updates);
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();

    FOREACH(Node, e, caseExprs)
    {
        exprToConstraints(e, ctx);
    }
    lp = newLPProblem(ctx);
    DEBUG_LOG(cstringLPProblem(lp, TRUE));
	
	return lp;
}

#define COMPILE_UPDATES_TO_LP(...)				\
	do {										\
		char *updates[] = { __VA_ARGS__, NULL };	\
		compileToLP(updates);					\
	} while(0)

static rc
testCompilationStack (void)
{   
	COMPILE_UPDATES_TO_LP(
		"UPDATE R SET a = 10 WHERE a < 4;",
		"UPDATE R SET a = 0 WHERE a = 10;",
		"UPDATE R SET b = 3 WHERE a <= 60;"
		);
	
    return PASS;
}

static rc
testCheckingForDependentUpdates(void)
{
	return PASS;
}


#define CPLEX_TEST(expr, result, enforce) ASSERT_EQUALS_INT(result, optimize((List *)parseFromStringOracle(expr), enforce), expr);

static rc
testCPLEXOpt (void)
{
	// comparisons with constants
	CPLEX_TEST("-1 = -1;", 1, TRUE);
	CPLEX_TEST("0 = 0;", 1, TRUE);
	CPLEX_TEST("1 = 1;", 1, TRUE);
	CPLEX_TEST("-2 = -1;", 0, TRUE);
	CPLEX_TEST("0 = 1;", 0, TRUE);
	CPLEX_TEST("1 = 2;", 0, TRUE);

	CPLEX_TEST("-1 <> -1;", 0, TRUE);
	CPLEX_TEST("0 <> 0;", 0, TRUE);
	CPLEX_TEST("1 <> 1;", 0, TRUE);
	CPLEX_TEST("-2 <> -1;", 1, TRUE);
	CPLEX_TEST("0 <> 1;", 1, TRUE);
	CPLEX_TEST("1 <> 2;", 1, TRUE);
	
    CPLEX_TEST("-3 < -1;", 1, TRUE);
	CPLEX_TEST("-1 < 0;", 1, TRUE);
    CPLEX_TEST("-1 < 2;", 1, TRUE);
	CPLEX_TEST("0 < 1;", 1, TRUE);
	CPLEX_TEST("1 < 2;", 1, TRUE);	
    CPLEX_TEST("-1 < -3;", 0, TRUE);
	CPLEX_TEST("0 < -1;", 0, TRUE);
    CPLEX_TEST("2 < -2;", 0, TRUE);
	CPLEX_TEST("2 < 1;", 0, TRUE);
	CPLEX_TEST("1 < 0;", 0, TRUE);

    CPLEX_TEST("-3 <= -1;", 1, TRUE);
	CPLEX_TEST("-1 <= 0;", 1, TRUE);
    CPLEX_TEST("-1 <= 2;", 1, TRUE);
	CPLEX_TEST("0 <= 1;", 1, TRUE);
	CPLEX_TEST("1 <= 2;", 1, TRUE);	
    CPLEX_TEST("-1 <= -3;", 0, TRUE);
	CPLEX_TEST("0 <= -1;", 0, TRUE);
    CPLEX_TEST("2 <= -2;", 0, TRUE);
	CPLEX_TEST("2 <= 1;", 0, TRUE);
	CPLEX_TEST("1 <= 0;", 0, TRUE);

    CPLEX_TEST("-3 > -1;", 0, TRUE);
	CPLEX_TEST("-1 > 0;", 0, TRUE);
    CPLEX_TEST("-1 > 2;", 0, TRUE);
	CPLEX_TEST("0 > 1;", 0, TRUE);
	CPLEX_TEST("1 > 2;", 0, TRUE);	
    CPLEX_TEST("-1 > -3;", 1, TRUE);
	CPLEX_TEST("0 > -1;", 1, TRUE);
    CPLEX_TEST("2 > -2;", 1, TRUE);
	CPLEX_TEST("2 > 1;", 1, TRUE);
	CPLEX_TEST("1 > 0;", 1, TRUE);
	
    // Comparisons with assignments
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
	CPLEX_TEST("(x >= 2) AND (x < 10); x := 2;", 1, TRUE);
	CPLEX_TEST("(x >= 2) AND (x < 10);", 1, TRUE);
	CPLEX_TEST("(x < 2) OR (x > 10); x := 5;", 0, TRUE);
	CPLEX_TEST("((x < 2) OR (x > 10)) AND (x = 5); x := 5;", 0, TRUE);
	
    // Case Expressions (w/ or w/o assignment)
    CPLEX_TEST("CASE WHEN (x < 3) THEN x ELSE 10 END;", 1, FALSE);
    CPLEX_TEST("x = 8; y := CASE WHEN (x = 8) THEN 1 ELSE 0 END;", 1, TRUE);
    CPLEX_TEST("x = 8; z := CASE WHEN (x = 8) THEN y + 3.21 ELSE y END;", 1, TRUE);
	CPLEX_TEST("x2 = 15; x1 := 5; y1 := 4; x2 := CASE WHEN y1 = 4 THEN x1 + 10 ELSE x1 END;", 1, TRUE);

	// artihmetics 
	CPLEX_TEST("y = 10; x := 5; y := x + 5;", 1, TRUE);
	
    return PASS;
}

static int
optimize(List *l, boolean enforce)
{
    ConstraintTranslationCtx *ctx = newConstraintTranslationCtx();
    FOREACH(Node, node, l) {
       exprToConstraints(node, ctx);
    }

    if(enforce) {
        Constraint *c = makeNode(Constraint);
        c->sense = CONSTRAINT_E;
        c->rhs = createConstInt(1);
        c->terms = LIST_MAKE(
            createNodeKeyValue((Node*)createConstInt(1), (Node*)getNthOfListP(ctx->variables, 0))
        );

        ctx->constraints = appendToTailOfList(ctx->constraints, c);
    }

    INFO_LOG("MILP program:\n%s", cstringConstraintTranslationCtx(ctx, TRUE));

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

// no cplex support just return pass
#else

rc
testCplexExpr(void)
{
    return PASS;
}


#endif 
