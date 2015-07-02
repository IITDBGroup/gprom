/*-----------------------------------------------------------------------------
 *
 * test_graph_pi_cs.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test/test_main.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/list/list.h"
#include "model/query_operator/query_operator.h"
#include "parser/parser.h"
#include "configuration/option.h"
#include "analysis_and_translate/translator.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"

//static rc testTableAccessRewrite(void);

rc
testPICSGraph(void)
{
    //RUN_TEST(testTableAccessRewrite(),"test table access rewrite for graphs");
    initMetadataLookupPlugins();
    chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_ORACLE);
    initMetadataLookupPlugin();
    List *res, *first;
    List *second;

    //Test projection
    //Check if the right result is returned
    char *s = "WITH x AS (SELECT a,b FROM test_graph_pi) SELECT a.a FROM x a, (SELECT a AS c, b AS d FROM x) b  WHERE a.a = b.d AND a.a = 2";
    char *sParse = "WITH x AS (SELECT a,b FROM test_graph_pi) SELECT m.a FROM x m, (SELECT a AS c, b AS d FROM x) n  WHERE m.a = n.d AND m.a = 2;";
    res = executeQuery(s);
    DEBUG_LOG("%s", nodeToString(res));
    DEBUG_LOG("%s", beatify(nodeToString(res)));
    first = (List *)getHeadOfListP(res);
    second = (List *)getNthOfListP(res, 1);
    ASSERT_EQUALS_STRING(getHeadOfListP(first), "2", "First value is correct.");
    ASSERT_EQUALS_STRING(getHeadOfListP(second), "2", "Second value is correct.");

    //Then check the node has been rewritten only once
    Node *result;
    Node *qoModel;
    QueryOperator *rewriteQoModel, *op;

    result = parseFromString(sParse);
    qoModel = translateParse(result);
    op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = rewritePI_CS((ProvenanceComputation *) op);
    DEBUG_LOG("rewrite is %s", operatorToOverviewString((Node *) rewriteQoModel));
    ASSERT_EQUALS_P((Node*)OP_LCHILD(OP_LCHILD(rewriteQoModel)), (Node*)OP_LCHILD(OP_LCHILD(OP_RCHILD(OP_LCHILD(rewriteQoModel)))),"The address of the children is same.");


    //Test aggregation
    s = "WITH x AS (SELECT sum(a) as a, b FROM test_graph_pi group by b) SELECT a.a FROM x a, (SELECT a AS c, b AS d FROM x) b  WHERE a.a = b.c";
    res = executeQuery(s);
    first = (List *)getHeadOfListP(res);

    ASSERT_EQUALS_STRING(getHeadOfListP(first), "3", "First value is correct.");

    /*
    result = parseFromString(getStringOption(s));
    qoModel = translateParse(result);
    op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = rewritePI_CS((ProvenanceComputation *) op);
    ASSERT_EQUALS_P((Node*)OP_LCHILD(OP_LCHILD(rewriteQoModel)), (Node*)OP_RCHILD(OP_LCHILD(rewriteQoModel)),"The address of the children is same.");
	*/

    //Test join
    s = "WITH x AS (SELECT test_graph_pi.a, test_graph_pi2.c FROM test_graph_pi join test_graph_pi2 on test_graph_pi.B = test_graph_pi2.C) SELECT a.a FROM x a, (SELECT a AS c, c AS d FROM x) b WHERE a.a = b.c";
    res = executeQuery(s);
    first = (List *)getHeadOfListP(res);
    second = (List *)getNthOfListP(res, 1);
    ASSERT_EQUALS_STRING(getHeadOfListP(first), "1", "First value is correct.");
    ASSERT_EQUALS_STRING(getHeadOfListP(second), "2", "Second value is correct.");

    /*
    result = parseFromString(getStringOption(s));
    qoModel = translateParse(result);
    op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = rewritePI_CS((ProvenanceComputation *) op);
    ASSERT_EQUALS_P((Node*)OP_LCHILD(OP_LCHILD(rewriteQoModel)), (Node*)OP_RCHILD(OP_LCHILD(rewriteQoModel)),"The address of the children is same.");
	*/
	//Test set
    s = "WITH x AS (SELECT a FROM test_graph_pi UNION ALL SELECT a as c FROM test_graph_pi t) SELECT a.a FROM  x a, (SELECT a AS c FROM x) WHERE c = 1 AND a = 1";
    res = executeQuery(s);
    first = (List *)getHeadOfListP(res);
    second = (List *)getNthOfListP(res, 1);
    ASSERT_EQUALS_STRING(getHeadOfListP(first), "1", "First value is correct.");
    ASSERT_EQUALS_STRING(getHeadOfListP(second), "1", "Second value is correct.");
	/*
    result = parseFromString(getStringOption(s));
    qoModel = translateParse(result);
    op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = rewritePI_CS((ProvenanceComputation *) op);
    ASSERT_EQUALS_P((Node*)OP_LCHILD(OP_LCHILD(rewriteQoModel)), (Node*)OP_RCHILD(OP_LCHILD(rewriteQoModel)),"The address of the children is same.");
	*/
	//Test order
	s = "WITH x AS (SELECT * FROM test_graph_pi ORDER BY a) SELECT a.a FROM x a, (SELECT a AS c FROM x) WHERE c = 2 AND a = 2";
	res = executeQuery(s);
	first = (List *)getHeadOfListP(res);
    ASSERT_EQUALS_STRING(getHeadOfListP(first), "2", "First value is correct.");

    /*
    result = parseFromString(getStringOption(s));
    qoModel = translateParse(result);
    op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = rewritePI_CS((ProvenanceComputation *) op);
    ASSERT_EQUALS_P((Node*)OP_LCHILD(OP_LCHILD(rewriteQoModel)), (Node*)OP_RCHILD(OP_LCHILD(rewriteQoModel)),"The address of the children is same.");
	*/

	//Test duplicate removal
	s = "WITH x AS (SELECT a FROM test_graph_pi UNION SELECT a as c FROM test_graph_pi t) SELECT a.a FROM  x a, (SELECT a AS c FROM x) WHERE c = 1 AND a = 1";
	res = executeQuery(s);
	first = (List *)getHeadOfListP(res);
    ASSERT_EQUALS_STRING(getHeadOfListP(first), "1", "First value is correct.");

    /*
    result = parseFromString(getStringOption(s));
    qoModel = translateParse(result);
    op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = rewritePI_CS((ProvenanceComputation *) op);
    ASSERT_EQUALS_P((Node*)OP_LCHILD(OP_LCHILD(rewriteQoModel)), (Node*)OP_RCHILD(OP_LCHILD(rewriteQoModel)),"The address of the children is same.");

	*/

    return PASS;
}
/*
static rc
testTableAccessRewrite(void)
{
    //TODO create expected result AGM, create AGM input, rewrite, compare
    return PASS;
}
*/
/*
int
main(int argc, char* argv[]){
	testPICSGraph();
	return 1;
}
*/
