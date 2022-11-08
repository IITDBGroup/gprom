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
#include "utility/string_utils.h"
#include "parser/parser_oracle.h"

/* internal tests */
static rc testAttributeReference (void);
static rc testFunctionCall (void);
static rc testConstant (void);
static rc testOperator (void);
static rc testExpressionToSQL (void);
static rc testAutoCasting (void);
static rc testMinMaxForConstants (void);

static Node *parseAndType(char *str);
static boolean typeExpression(Node *expr, void *context);

/* check expression model */
rc
testExpr (void)
{
    RUN_TEST(testAttributeReference(), "test attribute references");
    RUN_TEST(testFunctionCall(), "test function call nodes");
    RUN_TEST(testConstant(), "test constant nodes");
    RUN_TEST(testOperator(), "test operator nodes");
    RUN_TEST(testExpressionToSQL(), "test code that translates an expression tree into SQL code");
    RUN_TEST(testAutoCasting(), "test code that introduces casts for function and operator arguments where necessary");
    RUN_TEST(testMinMaxForConstants(), "test code that computes min and max of constants");

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
    c = createConstInt(-1);
    ASSERT_EQUALS_INT(-1, INT_VALUE(c), "constant int -1");

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

    ASSERT_EQUALS_STRING("(1 + 2)", exprToSQL((Node *) o, NULL), "translate expression into SQL code (1 + 2)");

    o = createOpExpr("*", LIST_MAKE(o, createConstFloat(2.0)));
    ASSERT_EQUALS_STRING("((1 + 2) * 2.000000)", exprToSQL((Node *) o, NULL), "translate expression into SQL code (1 + 2) * 2.0");

    o = createOpExpr("+", LIST_MAKE(createAttributeReference("a"), createConstInt(2)));
    ASSERT_EQUALS_STRING("(a + 2)", exprToSQL((Node *) o, NULL), "translate expression into SQL code (a + 2)");

    return PASS;
}

static rc
testMinMaxForConstants (void)
{
	Constant *l, *r, *expect, *n;

	l = createConstInt(1);
	r = createConstInt(3);
	n = createNullConst(DT_INT);
	expect = createConstInt(1);

	ASSERT_EQUALS_INT(INT_VALUE(minConsts(l, r, TRUE)), INT_VALUE(expect), "min(1,3) = 1");

	expect = createConstInt(3);
	ASSERT_EQUALS_INT(INT_VALUE(maxConsts(l, r,TRUE)), INT_VALUE(expect), "max(1,3) = 3");

	expect = createNullConst(DT_INT);
	ASSERT_EQUALS_NODE(minConsts(l, n, TRUE), expect, "min(1,NULL) = NULL");
	ASSERT_EQUALS_NODE(maxConsts(l, n, TRUE), expect, "max(1,NULL) = NULL");
	ASSERT_EQUALS_NODE(minConsts(n, r, TRUE), expect, "min(NULL,3) = NULL");
	ASSERT_EQUALS_NODE(maxConsts(n, r, TRUE), expect, "max(NULL,3) = NULL");
	ASSERT_EQUALS_NODE(minConsts(n, n, TRUE), expect, "min(NULL,NULL) = NULL");
	ASSERT_EQUALS_NODE(maxConsts(n, n, TRUE), expect, "max(NULL,NULL) = NULL");

	return PASS;
}

static rc
testAutoCasting (void)
{
    Operator *o, *result, *exp;

    // check addition with float and int
    o = createOpExpr("+", LIST_MAKE(createConstFloat(1), createConstInt(2)));
    result = (Operator *) addCastsToExpr((Node *) o, FALSE);
    exp = createOpExpr("+", LIST_MAKE(createConstFloat(1), createCastExpr((Node *) createConstInt(2), DT_FLOAT)));

    ASSERT_EQUALS_NODE(exp,result,"1.0 + 2 -> 1 + CAST(2 AS FLOAT)");

    // check cast for equality
    o = createOpExpr(OPNAME_EQ, LIST_MAKE(createConstString("1"), createConstInt(2)));
    result = (Operator *) addCastsToExpr((Node *) o, FALSE);
    exp = createOpExpr(OPNAME_EQ, LIST_MAKE(createConstString("1"), createCastExpr((Node *) createConstInt(2), DT_STRING)));

    ASSERT_EQUALS_NODE(exp,result,"\"1\" = 2 -> \"1\" = CAST(2 AS STRING)");

    // check where both args have to be casted
    o = createOpExpr("||", LIST_MAKE(createConstFloat(1.0), createConstInt(2)));
    result = (Operator *) addCastsToExpr((Node *) o, FALSE);
    exp = createOpExpr("||", LIST_MAKE(createCastExpr((Node *) createConstFloat(1.0), DT_STRING), createCastExpr((Node *) createConstInt(2), DT_STRING)));

    ASSERT_EQUALS_NODE(exp,result,"1.0 || 2 -> CAST(1.0 AS STRING) || CAST(2 AS STRING)");

    // check parse nested expressions
    o = createOpExpr("*", LIST_MAKE(createConstFloat(1), createOpExpr("+", LIST_MAKE(createConstInt(2), createConstFloat(1.0)))));
    result = (Operator *) addCastsToExpr((Node *) o, FALSE);
    exp = createOpExpr("*", LIST_MAKE(createConstFloat(1),
            createOpExpr("+", LIST_MAKE(createCastExpr((Node *) createConstInt(2), DT_FLOAT), createConstFloat(1.0)))));

    ASSERT_EQUALS_NODE(exp,result,"1.0 * (2 + 1.0)  = 2 -> \"1\" = 1.0 * (CAST(2 AS FLOAT) + 1.0)");

	// check case expression
	ASSERT_EQUALS_INT(DT_FLOAT, typeOf(parseAndType("CASE WHEN iA < 5 THEN iB * 0.1 ELSE 0 END")),
		"CASE WHEN iA < 5 THEN iB * 0.1 ELSE 0 END => DT_FLOAT");
	
    return PASS;
}


static Node *
parseAndType(char *str)
{
	Node *expr = parseExprFromStringOracle(str);
	typeExpression(expr, NULL);
	DEBUG_LOG("created expression %s", beatify(nodeToString(expr)));
	return expr;
}


static boolean
typeExpression(Node *expr, void *context)
{
	if (expr == NULL)
		return TRUE;

	if (isA(expr,AttributeReference))
	{
		AttributeReference *a = (AttributeReference *) expr;
		char *name = strRemPrefix(a->name,1);
		char type = a->name[0];

		a->name = name;
		switch(type)
		{
		case 'i':
			a->attrType = DT_INT;
			break;
		case 'f':
			a->attrType = DT_FLOAT;
			break;
		case 's':
			a->attrType = DT_STRING;
			break;
		case 'b':
			a->attrType = DT_BOOL;
			break;
		case 'l':
			a->attrType = DT_LONG;
			break;
		}
		return TRUE;
	}

	return visit(expr, typeExpression, context);
}
