/*-----------------------------------------------------------------------------
 *
 * test_rpq.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "model/list/list.h"
#include "model/rpq/rpq_model.h"
#include "parser/parser_rpq.h"
#include "rpq/rpq_to_datalog.h"
#include "sql_serializer/sql_serializer_dl.h"

static rc testRPQToString(void);
static rc testRPQParsing(void);
static rc testRPQToDL(void);

rc
testRPQ(void)
{
    RUN_TEST(testRPQToString(), "test rpq to string");
    RUN_TEST(testRPQParsing(), "test rpq parsing");
    RUN_TEST(testRPQToDL(), "test rpq to datalog translation");

    return PASS;
}


static rc
testRPQToString(void)
{
    Regex *r;

    r = makeRegex(singleton(makeRegexLabel("a")), "+");
    ASSERT_EQUALS_STRING("(a)+", rpqToShortString(r), "should be a+");

    r = makeRegex(
               LIST_MAKE(makeRegex(
                       LIST_MAKE(makeRegexLabel("a"),
                               makeRegexLabel("b")),
                         "|"),
                       makeRegexLabel("c")
                       ),
                         ".");
    ASSERT_EQUALS_STRING("(a|b).c", rpqToShortString(r), "should be (a|b).c");

    return PASS;
}



static rc
testRPQParsing(void)
{
    Regex *r;
    Regex *exp;

    r = (Regex *) parseFromStringrpq("(a|b).c");
    exp = makeRegex(
           LIST_MAKE(makeRegex(
                   LIST_MAKE(makeRegexLabel("a"),
                           makeRegexLabel("b")),
                     "|"),
                   makeRegexLabel("c")
                   ),
                 ".");
    ASSERT_EQUALS_NODE(exp,r,"parser expression is (a|b).c");

    r = (Regex *) parseFromStringrpq("((a) . (b))* | (c)+");
    ASSERT_EQUALS_STRING("((a.b)*|(c)+)", rpqToShortString(r), "should be (a.b)*|c+");

    r = (Regex *) parseFromStringrpq("((a) . (b) . (d))* | c+");
    ASSERT_EQUALS_STRING("((a.b.d)*|(c)+)", rpqToShortString(r), "should be (a.b.d)*|c+");

    return PASS;
}

static rc
testRPQToDL(void)
{
    Regex *r;
    Node *dl;

    r = (Regex *) parseFromStringrpq("(a|b).c");
    dl = rpqToDatalog(r,RPQ_QUERY_PROV,"e","result");

    DEBUG_LOG("%s", beatify(nodeToString(dl)));
    INFO_LOG("%s", datalogToOverviewString(dl));
    INFO_LOG("%s", serializeOperatorModelDL(dl));

    return PASS;
}
