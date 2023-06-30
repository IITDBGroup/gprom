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

#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"

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
    QueryOperator *op1 = NULL;
    QueryOperator *op2 = NULL;
    char *attrName = "A";

    mergeQueries(op1, op2, attrName);
    return PASS;
}
