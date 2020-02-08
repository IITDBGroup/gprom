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

/* check equal model */
rc
testCplexExpr(void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testLogicExpr(), "test cplex for logical expressions");

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
    Node *test = (Node *) createOpExpr("AND", LIST_MAKE(createConstBool(TRUE), createConstBool(FALSE)));
	test = NULL;
    //TODO create constraint
    //TODO solve constraint
    //TODO LOG constraints
    //TODO compare result against expected result
    return PASS;
}
