/*-----------------------------------------------------------------------------
 *
 * test_temporal.c
 *			  
 *		
 *		AUTHOR: felix
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"

static rc testNormalization(void);

rc
testSet()
{
    RUN_TEST(testNormalization(), "test normalization");

    return PASS;
}

static rc
testNormalization(void)
{
    return PASS;
}