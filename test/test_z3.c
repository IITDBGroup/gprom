/*
 *------------------------------------------------------------------------------
 *
 * test_z3.c - testing expression solving with Z3
 *
 *     Testing checking satisfiability of expressions with Z3.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2020-12-05
 *        SUBDIR: test/
 *
 *-----------------------------------------------------------------------------
 */


#include "model/list/list.h"
#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "parser/parser_oracle.h"
#include "provenance_rewriter/coarse_grained/prop_inference.h"
#include "symbolic_eval/z3_solver.h"
#include "utility/string_utils.h"

#if HAVE_Z3
/* internal tests */
static rc testSatisfiability (void);
static boolean typeExpression(Node *expr, void *context);
static Node *parseAndType(char *str);
#endif

/* check expression model */
rc
testZ3 (void)
{
	#if HAVE_Z3
	RUN_TEST(testSatisfiability(), "test satisfiability");
	#endif
    return PASS;
}

#if HAVE_Z3
/* */
static rc
testSatisfiability (void)
{
	ASSERT_TRUE(z3ExprIsSatisfiable(parseAndType("ia = ib"),
									FALSE),
				"(a = b) is satisfiable");

	ASSERT_FALSE(z3ExprIsSatisfiable(parseAndType("ia < 3 AND ia > 4"),
									FALSE),
				"a < 3 AND a > 4 is unsatisfiable");

	ASSERT_TRUE(z3ExprIsSatisfiable(parseAndType("ia < 3 AND ia > 1"),
									FALSE),
				"a < 3 AND a > 1 is satisfiable");

	ASSERT_TRUE(z3ExprIsValid(parseAndType("ia < 3 OR ia >= 3"),
									FALSE),
				"a < 3 OR a >= 3 is valid");

	ASSERT_FALSE(z3ExprIsValid(parseAndType("ia < 3 AND ib < 10"),
									FALSE),
				"a < 3 AND b < 10 is not valid");

	ASSERT_TRUE(z3ExprIsValid(parseAndType("ia + 3 < ia + 5"),
							  FALSE),
				"a + 3 < a + 5 is valid");

	ASSERT_TRUE(z3ExprIsValid(parseAndType("(NOT (ia <> ib)) OR (ia < ib OR ia > ib)"),
							  FALSE),
				"(NOT (a <> b)) OR (a < b OR a > b) is valid");

	ASSERT_TRUE(z3ExprIsSatisfiable(parseAndType("'Peter' = 'Peter'"),
							  FALSE),
				"'Peter' = 'Peter' is satisfiable");

	ASSERT_TRUE(z3ExprIsValid(parseAndType("(NOT(sa = 'Peter')) OR (sa <> 'Bob')"),
							  FALSE),
				"NOT(sa = 'Peter') OR (sa <> 'Bob') is valid");


	ASSERT_TRUE(z3ExprIsValid(parseAndType("3.0 < 4.0"),
							  FALSE),
				"3.0 < 4.0 is valid");

	ASSERT_TRUE(z3ExprIsValid(parseAndType("3.0 < 4.0 AND 3.0 + 5.0 > 5.0 - 3.0"),
							  FALSE),
				"3.0 < 4.0 AND 3.0 + 5.0 > 5.0 - 3.0 is valid");

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

#endif
