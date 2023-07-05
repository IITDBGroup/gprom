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


static rc testCombineRowByAttr1(void);
static rc testCombineRowByAttr2(void);
static rc testMerge(void);
static rc testSplit(void);

rc
testMergeOperators(void)
{
    RUN_TEST(testCombineRowByAttr1(), "test 1 combine row by attribute");
    RUN_TEST(testCombineRowByAttr2(), "test 2 combine row by attribute");
    RUN_TEST(testMerge(), "test merge 1");
    RUN_TEST(testSplit(), "test split 1");

    return PASS;
}

static rc
testCombineRowByAttr1(void)
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
testCombineRowByAttr2(void)
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
testMerge(void)
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
testSplit(void)
{
    QueryOperator *op;
    char *attrName = "a";
    char *command = "SELECT * FROM uadb1;";
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