/*-----------------------------------------------------------------------------
 *
 * test_parameter.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "analysis_and_translate/parameter.h"
#include "parser/parser.h"
#include "analysis_and_translate/analyze_qb.h"
#include "metadata_lookup/metadata_lookup.h"

static rc testParseBinds (void);
static rc testSetParameterValues (void);

rc
testParameter(void)
{
    RUN_TEST(testParseBinds(), "test parse binds");
    RUN_TEST(testSetParameterValues(), "test setting values for parameters");
    return PASS;
}

static rc
testParseBinds (void)
{
    char *binds = " #1(3):123 #2(5):abcde";
    List *consts = NIL;
    List *expected = LIST_MAKE(createConstString("123"), createConstString("abcde"));

    consts = oracleBindToConsts(binds);
    ASSERT_EQUALS_NODE(expected, consts, "parsed parameters should be [123,abcde]");

    return PASS;
}

static rc
testSetParameterValues (void)
{
    char *expSQL = "SELECT a FROM r WHERE a = '5';";
    char *inSQL = "SELECT a FROM r WHERE a = :param;";
    Node *expParse = parseFromString(expSQL);
    Node *inParse = parseFromString(inSQL);
    Node *val = (Node *) createConstString("5");
    SQLParameter *p = (SQLParameter *) getNthOfListP(findParameters(inParse), 0);

    initMetadataLookupPlugins();
    chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_ORACLE);
    initMetadataLookupPlugin();

    analyzeQueryBlockStmt(expParse, NIL);
    analyzeQueryBlockStmt(inParse, NIL);
    p->position = 1;

    inParse = setParameterValues(inParse, singleton(val));

    ASSERT_EQUALS_NODE(expParse, inParse, "setting parameter is '5'");

    return PASS;
}

static boolean
findParamVisitor(Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, SQLParameter))
        *state = appendToTailOfList(*state, node);

    return visit(node, findParamVisitor, (void *) state);
}
