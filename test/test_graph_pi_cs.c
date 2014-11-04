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

static rc testTableAccessRewrite(void);

rc
testPICSGraph(void)
{
    RUN_TEST(testTableAccessRewrite(),"test table access rewrite for graphs");

    return PASS;
}

static rc
testTableAccessRewrite(void)
{
    //TODO create expected result AGM, create AGM input, rewrite, compare
    return PASS;
}
