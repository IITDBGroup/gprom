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


static rc testCombineRowByAttr(void);
static rc testMerge1(void);

rc
testMergeOperators(void)
{
    RUN_TEST(testCombineRowByAttr(), "test combine row by attribute");
    RUN_TEST(testMerge1(), "test merge 1");

    return PASS;
}

static rc
testCombineRowByAttr(void)
{
    QueryOperator *op;
    char *attrName = "a";
    char *command = "SELECT * FROM uadb1;";
    char *rewrittenSQL = NULL;

    Node *t = parseFromString(command);
    op = (QueryOperator *) getHeadOfList((List *) translateParse(t))->data.ptr_value;

    QueryOperator *result = combineRowByAttr(op, attrName, TRUE);

    INFO_OP_LOG("result", result);

    if (result == NULL)
        return FAIL;

    rewrittenSQL = generatePlan((Node *)result, FALSE);

    printf("rewritten SQL: %s\n", rewrittenSQL);

    execute(rewrittenSQL);

    return PASS;
}

static rc
testMerge1(void)
{
    QueryOperator *op1;
    QueryOperator *op2;
    char *attrName = "a";
    char *command1 = "SELECT * FROM uadb1;";
    char *command2 = "SELECT * FROM uadb2;";
    char *rewrittenSQL = NULL;


    Node *t1 = parseFromString(command1);
    Node *t2 = parseFromString(command2);
    op1 =  (QueryOperator *) getHeadOfList((List *) translateParse(t1))->data.ptr_value;
    op2 = (QueryOperator *) getHeadOfList((List *) translateParse(t2))->data.ptr_value;

    QueryOperator *result = mergeQueries(op1, op2, attrName);
    if (result == NULL)
        return FAIL;

    rewrittenSQL = generatePlan((Node *)result, FALSE);

    printf ("rewritten SQL: %s\n", rewrittenSQL);

    execute(rewrittenSQL);

    return PASS;
}