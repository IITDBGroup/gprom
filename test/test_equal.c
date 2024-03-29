/*-----------------------------------------------------------------------------
 *
 * test_equal.c
 *
 *
 *		AUTHOR: Hao
 *
 *
 *
 *-----------------------------------------------------------------------------
 */
#include <string.h>
#include "model/list/list.h"
#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"
#include "analysis_and_translate/analyze_dl.h"
#include "parser/parser_dl.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

/* internal tests */
static rc testequalFunctionCall (void);
static rc testequalAttributeReference (void);
static rc testequalConstant(void);
static rc testequalList(void);
static rc testequalDL(void);

/* check equal model */
rc
testEqual (void)
{
    RUN_TEST(testequalFunctionCall (), "test equal FunctionCall");
    RUN_TEST(testequalAttributeReference(), "test equal AttibuteReference");
    RUN_TEST(testequalConstant(), "test equal Constant");
    RUN_TEST(testequalList(), "test equal List");
	RUN_TEST(testequalDL(), "test equal for Datalog nodes");
    return PASS;
}

static rc
testequalFunctionCall (void)
{
    FunctionCall *a, *b;
    a = createFunctionCall("test", NIL);
    b = makeNode(FunctionCall);
    b->args = NIL;
    b->functionname = "test";

    ASSERT_EQUALS_INT(a->type, T_FunctionCall, "type is a FunctionCall");
    ASSERT_EQUALS_INT(a->type, b->type, "types are the same");
    ASSERT_EQUALS_STRINGP(a->functionname, b->functionname, "names are the same");
    ASSERT_EQUALS_NODE(a->args,b->args,"both args are same");

    return PASS;
}

static rc
testequalAttributeReference (void)
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
    ASSERT_EQUALS_STRINGP(a->name, b->name, "names are the same");
    ASSERT_EQUALS_NODE(a,b,"both attribute references are same");

    return PASS;
}

static rc
testequalConstant(void)
{
    Constant *a, *b;

    a = createConstInt(1);
    b = createConstInt(1);

    ASSERT_EQUALS_INT(1, INT_VALUE(a), "1 integer constant");
    ASSERT_EQUALS_NODE(a,b,"equal int constants");

    b = createConstInt(2);
    ASSERT_FALSE(equal(a,b), "int 1 != int 2");

    return PASS;
}

static rc
testequalList (void)
{
    List *a, *b;

    a = NIL;
    a = appendToTailOfList (a, createAttributeReference("a1"));
    a = appendToTailOfList (a, createAttributeReference("a2"));
    a = appendToTailOfList (a, createAttributeReference("a3"));

    b = NIL;
    b = appendToTailOfList (b, createAttributeReference("a1"));
    b = appendToTailOfList (b, createAttributeReference("a2"));
    b = appendToTailOfList (b, createAttributeReference("a3"));

    ASSERT_EQUALS_NODE(a,b,"node lists are equal");

    a = NIL;
    a = appendToTailOfListInt (a, 1);
    a = appendToTailOfListInt (a, 2);
    a = appendToTailOfListInt (a, 3);

    b = NIL;
    b = appendToTailOfListInt (b, 1);
    b = appendToTailOfListInt (b, 2);
    b = appendToTailOfListInt (b, 3);

    ASSERT_EQUALS_NODE(a,b,"int lists are equal");

    return PASS;
}

#define TEST_EQUAL_DL(ep)											\
	do {															\
		DLProgram *_ep, *_ap;										\
		_ep = (DLProgram *) parseFromStringdl(ep);					\
		_ap = (DLProgram *) parseFromStringdl(ep);					\
		ASSERT_EQUALS_NODE(_ep,_ap,"Datalog programs equal:\n" ep);	\
	} while(0)

#define TEST_EQUAL_DL_WITH_ANALYZE(ep)											\
	do {															\
		DLProgram *_ep, *_ap;										\
		_ep = (DLProgram *) parseFromStringdl(ep);					\
		_ap = (DLProgram *) parseFromStringdl(ep);					\
		analyzeDLModel((Node *) _ep);								\
		analyzeDLModel((Node *) _ap);										\
		ASSERT_EQUALS_NODE(_ep,_ap,"Datalog programs equal:\n" ep);	\
	} while(0)

static rc
testequalDL(void)
{
	TEST_EQUAL_DL("Q(X) :- R(X,Y), S(Y,Z).");
	TEST_EQUAL_DL("Q(X) :- R(X,Y), X < 5.");
	TEST_EQUAL_DL("Q(X) :- R(X,Y), S(Y,Z), X < 5. Q2(X) :- Q(X), Q(X). ANS : Q2.");

	TEST_EQUAL_DL_WITH_ANALYZE("Q(X) :- R(X,Y), S(Y,Z).");
	TEST_EQUAL_DL_WITH_ANALYZE("Q(X) :- R(X,Y), X < 5.");
	TEST_EQUAL_DL_WITH_ANALYZE("Q(X) :- R(X,Y), S(Y,Z), X < 5. Q2(X) :- Q(X), Q(X). ANS : Q2.");

	return PASS;
}
