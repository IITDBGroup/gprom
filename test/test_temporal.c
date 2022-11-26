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
testTemporal()
{
    RUN_TEST(testNormalization(), "test normalization");

    return PASS;
}

static rc
testNormalization(void)
{
    return PASS;
}
