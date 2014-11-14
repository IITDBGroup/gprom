/*-----------------------------------------------------------------------------
 *
 * test_expr.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"

/* internal tests */
static rc testAttributeReference (void);
static rc testFunctionCall (void);
static rc testConstant (void);
static rc testOperator (void);
static rc testExpressionToSQL (void);

/* check expression model */
rc
testExpr (void)
{
    RUN_TEST(testAttributeReference(), "test attribute references");
    RUN_TEST(testFunctionCall(), "test function call nodes");
    RUN_TEST(testConstant(), "test constant nodes");
    RUN_TEST(testOperator(), "test operator nodes");
    RUN_TEST(testExpressionToSQL(), "test code that translates an expression tree into SQL code");

    return PASS;
}

/* */
static rc
testAttributeReference (void)
{
    AttributeReference *a, *b;

    a = createAttributeReference("test");
    b = makeNode(AttributeReference);
    b->name = "test";
    b->fromClauseItem = INVALID_ATTR;
    b->attrPosition = INVALID_ATTR;
    b->outerLevelsUp = INVALID_ATTR;
    b->attrType = DT_STRING;

    ASSERT_EQUALS_INT(a->type, T_AttributeReference, "type is attribute reference");
    ASSERT_EQUALS_INT(a->type, b->type, "types are the same");
    ASSERT_EQUALS_INT(a->fromClauseItem, b->fromClauseItem, "FCI are the same");
    ASSERT_EQUALS_INT(INVALID_ATTR, b->fromClauseItem, "b has FCI=invalid");
    ASSERT_EQUALS_INT(INVALID_ATTR, a->fromClauseItem, "a has FCI=invlaid");
    ASSERT_EQUALS_STRINGP(a->name, b->name, "names are the same");
    ASSERT_EQUALS_NODE(a,b,"both attribute references are same");

    return PASS;
}

/* */
static rc
testFunctionCall(void)
{
    FunctionCall *a;
    Constant *c;

    a = createFunctionCall ("f", LIST_MAKE(createConstInt(1), createConstInt(2)));
    c = (Constant *) getNthOfListP(a->args, 0);

    ASSERT_EQUALS_INT(1, INT_VALUE(c), "first arg is 1 const");
    ASSERT_EQUALS_STRING("f", a->functionname, "function name is f");

    return PASS;
}

static rc
testConstant (void)
{
    Constant *c;
    char *str;

    c = createConstInt(1);
    ASSERT_EQUALS_INT(1, INT_VALUE(c), "constant int 1");

    c = createConstFloat(2.0);
    ASSERT_EQUALS_FLOAT(2.0, FLOAT_VALUE(c), "constant float 2.0");

    c = createConstBool(TRUE);
    ASSERT_EQUALS_INT(TRUE, BOOL_VALUE(c), "constant boolean TRUE");

    str = strdup("test");
    c = createConstString(str);
    ASSERT_EQUALS_STRING("test", STRING_VALUE(c), "constant string \"test\"");

    return PASS;
}

static rc
testOperator (void)
{
    Operator *a;
    Constant *c;

    a = createOpExpr("f", LIST_MAKE(createConstInt(1), createConstInt(2)));
    c = (Constant *) getNthOfListP(a->args, 0);

    ASSERT_EQUALS_INT(1, INT_VALUE(c), "first arg is 1 const");
    ASSERT_EQUALS_STRING("f", a->name, "op name is f");

    return PASS;
}

/* */
static rc
testExpressionToSQL()
{
//    Constant *c1, *c2;
    Operator *o;

    o = createOpExpr("+", LIST_MAKE(createConstInt(1), createConstInt(2)));

    ASSERT_EQUALS_STRING("(1 + 2)", exprToSQL((Node *) o), "translate expression into SQL code (1 + 2)");

    o = createOpExpr("*", LIST_MAKE(o, createConstFloat(2.0)));
    ASSERT_EQUALS_STRING("((1 + 2) * 2.000000)", exprToSQL((Node *) o), "translate expression into SQL code (1 + 2) * 2.0");

    o = createOpExpr("+", LIST_MAKE(createAttributeReference("a"), createConstInt(2)));
    ASSERT_EQUALS_STRING("(a + 2)", exprToSQL((Node *) o), "translate expression into SQL code (a + 2)");

    return PASS;
}
