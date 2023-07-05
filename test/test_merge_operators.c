/*-----------------------------------------------------------------------------
 *
 * test_merge_operators.c
 *			  
 *		
 *		AUTHORS: ffichet ybelkhedra
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "common.h"

#include "rewriter.h"
#include "parser/parser.h"
#include "execution/executor.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_postgres.h"

// void createTable(void);
// void deleteTable(void);

static rc testCombineRowByAttr1();
static rc testCombineRowByAttr2();
static rc testMerge();
static rc testSplit();

/* plugin init routine */


#include "libpq-fe.h"

#define ASSERT_OK(res,mes) ASSERT_TRUE(PQresultStatus(res) == PGRES_COMMAND_OK, mes)
#define EXEC_CHECK(c,query) \
    do { \
        PGresult *res_ = PQexec(c,query); \
        if (PQresultStatus(res_) != PGRES_COMMAND_OK) \
            DEBUG_LOG("error was: %s", PQresultErrorMessage(res_)); \
        PQclear(res_); \
    } while(0)

static PGconn *
setupMetadataLookup(void)
{
    PGconn *c;
//    PGresult *res = NULL;

    initMetadataLookupPlugins();
    chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_POSTGRES);
    initMetadataLookupPlugin();

    c = getPostgresConnection();
    ASSERT_FALSE(c == NULL, "have working connection");

    EXEC_CHECK(c,"DROP TABLE IF EXISTS uadb1 CASCADE;");
    EXEC_CHECK(c,"DROP TABLE IF EXISTS uadb2 CASCADE;");
    EXEC_CHECK(c,"DROP TABLE IF EXISTS u CASCADE;");
    EXEC_CHECK(c,"DROP TABLE IF EXISTS uadb_merged CASCADE;");
    
    EXEC_CHECK(c,"CREATE TABLE uadb1 (A INT, B INT[], row INT[]);");
    EXEC_CHECK(c,"INSERT INTO uadb1 (A, B, row) VALUES (1, '{1,10}', '{1,1}'), (1, '{25,30}', '{2,3}'), (2, '{1,1}', '{0,1}');");
    EXEC_CHECK(c,"CREATE TABLE uadb2 ( A INT, B INT[], row INT[] );");
    EXEC_CHECK(c,"INSERT INTO uadb2 (A, B, row) VALUES (1, '{5,10}', '{1,1}'), (1, '{35,40}', '{0,1}'), (2, '{3,4}', '{1,1}');");
    EXEC_CHECK(c,"CREATE TABLE u (A INT, B INT[], C INT[], row INT[]);");
    EXEC_CHECK(c,"INSERT INTO u (A, B, C, row) VALUES (1, '{1,10}', '{3,15}', '{1,1}'), (1, '{25,30}', '{10,22}', '{2,3}'), (2, '{1,1}', '{1,10}', '{0,1}');");
    EXEC_CHECK(c,"CREATE TABLE uadb_merged (A INT, B INT[], row INT[]);");
    EXEC_CHECK(c,"INSERT INTO uadb_merged (A, B, row) VALUES (1, '{1,10}', '{1,1}'), (2, '{1,1}', '{0,1}');");

    DEBUG_LOG("Created test tables");

    return c;
}

void
deleteTable(PGconn *c)
{
    EXEC_CHECK(c, "DROP TABLE uadb1;");
    EXEC_CHECK(c, "DROP TABLE uadb2;");
    EXEC_CHECK(c, "DROP TABLE u;");
    EXEC_CHECK(c, "DROP TABLE uadb_merged;");
}

rc
testMergeOperators(void)
{
    PGconn *c = setupMetadataLookup();
    RUN_TEST(testCombineRowByAttr1(c), "test 1 combine row by attribute");
    RUN_TEST(testCombineRowByAttr2(c), "test 2 combine row by attribute");
    RUN_TEST(testMerge(c), "test merge 1");
    RUN_TEST(testSplit(c), "test split 1");
    deleteTable(c);
    return PASS;
}

static rc
testCombineRowByAttr1()
{
    QueryOperator *op;
    char *attrName = "a";
    char *command = "SELECT * FROM uadb1;";
    char *resultSQL = NULL; 
    char *opToSQL = NULL;

    Node *t = parseFromString(command);
    op = (QueryOperator *) getHeadOfList((List *) translateParse(t))->data.ptr_value;

    QueryOperator *result = combineRowByAttr(op, attrName, TRUE);

    INFO_OP_LOG("result", result);

    if (result == NULL)
        return FAIL;

    opToSQL = generatePlan((Node *)op, FALSE);
    resultSQL = generatePlan((Node *)result, FALSE);

    //printf("rewritten SQL: %s\n", resultSQL);

    printf("Initial table : \n");
    execute(opToSQL);
    printf("Result table : \n");
    execute(resultSQL);

    return PASS;
}


static rc
testCombineRowByAttr2()
{
    QueryOperator *op;
    char *attrName = "a";
    char *command = "SELECT * FROM u;";
    char *resultSQL = NULL; 
    char *opToSQL = NULL;

    Node *t = parseFromString(command);
    op = (QueryOperator *) getHeadOfList((List *) translateParse(t))->data.ptr_value;

    QueryOperator *result = combineRowByAttr(op, attrName, TRUE);

    INFO_OP_LOG("result", result);

    if (result == NULL)
        return FAIL;

    opToSQL = generatePlan((Node *)op, FALSE);
    resultSQL = generatePlan((Node *)result, FALSE);

    //printf("rewritten SQL: %s\n", resultSQL);

    printf("Initial table : \n");
    execute(opToSQL);
    printf("Result table : \n");
    execute(resultSQL);

    return PASS;
}

static rc
testMerge()
{
    QueryOperator *op1;
    QueryOperator *op2;
    char *attrName = "a";
    char *command1 = "SELECT * FROM uadb1;";
    char *command2 = "SELECT * FROM uadb2;";
    char *rewrittenSQL = NULL;
    char *opToSQL1 = NULL;
    char *opToSQL2 = NULL;


    Node *t1 = parseFromString(command1);
    Node *t2 = parseFromString(command2);
    op1 =  (QueryOperator *) getHeadOfList((List *) translateParse(t1))->data.ptr_value;
    op2 = (QueryOperator *) getHeadOfList((List *) translateParse(t2))->data.ptr_value;

    QueryOperator *result = mergeQueries(op1, op2, attrName);
    if (result == NULL)
        return FAIL;

    rewrittenSQL = generatePlan((Node *)result, FALSE);
    opToSQL1 = generatePlan((Node *)op1, FALSE);
    opToSQL2 = generatePlan((Node *)op2, FALSE);

    //printf ("rewritten SQL: %s\n", rewrittenSQL);

    printf("Initial table 1 : \n");
    execute(opToSQL1);
    printf("Initial table 2 : \n");
    execute(opToSQL2);
    printf("Result table : \n");
    execute(rewrittenSQL);

    return PASS;
}

static rc
testSplit()
{
    QueryOperator *op;
    char *attrName = "b";
    char *command = "SELECT * FROM uadb_merged;";
    char *opToSQL = NULL;
    char *op1ToSQL = NULL;
    char *op2ToSQL = NULL;

    Node *t = parseFromString(command);
    op = (QueryOperator *) getHeadOfList((List *) translateParse(t))->data.ptr_value;

    List *result = splitQueries(op, attrName);
    if (result == NULL)
        return FAIL;
    
    QueryOperator *op1 = (QueryOperator *) getHeadOfList(result)->data.ptr_value;
    QueryOperator *op2 = (QueryOperator *) getTailOfList(result)->data.ptr_value;

    opToSQL = generatePlan((Node *)op, FALSE);
    op1ToSQL = generatePlan((Node *)op1, FALSE);
    op2ToSQL = generatePlan((Node *)op2, FALSE);

    //printf ("rewritten SQL: %s\n", rewrittenSQL);

    printf("Initial table : \n");
    execute(opToSQL);
    printf("Result table 1 : \n");
    execute(op1ToSQL);
    printf("Result table 2 : \n");
    execute(op2ToSQL);

    return PASS;
}