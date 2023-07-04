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

#include "parser/parser.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"


static rc testMerge1(void);

rc
testMergeOperators(void)
{
    RUN_TEST(testMerge1(), "test merge 1");

    return PASS;
}


static rc
testMerge1(void)
{
    QueryOperator *op1;
    QueryOperator *op2;
    char *attrName = "A";
    char *command = "SELECT * FROM r;";

    Node *t = parseFromString(command);
    printf("analyzed query: %s\n", beatify(nodeToString(t)));
    printf("Translation and taking the head of the list\n");
    op1 =  (QueryOperator *) getHeadOfList((List *) translateParse(t))->data.ptr_value;
    op2 = (QueryOperator *) getHeadOfList((List *) translateParse(t))->data.ptr_value;

    printf("op type is %s\n", NodeTagToString(op1->type));

    printf("translated query: %s\n", beatify(nodeToString(op1)));

    mergeQueries(op1, op2, attrName);
    return PASS;
}
