/*-----------------------------------------------------------------------------
 *
 * test_libgprom.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"

#include "libgprom/libgprom.h"
#include "configuration/option.h"
#include "rewriter.h"

static rc testConfiguration(void);
static rc testRewrite(void);
static rc testLoopBackMetadata(void);

rc
testLibGProM(void)
{
    gprom_init();

    RUN_TEST(testConfiguration(), "test configuration interface");
    RUN_TEST(testRewrite(), "test rewrite function");
    RUN_TEST(testLoopBackMetadata(), "test loop back metadata lookup");

    return PASS;
}

static rc
testConfiguration(void)
{
    gprom_setStringOption("backend", "oracle");

    ASSERT_EQUALS_STRING("oracle", gprom_getStringOption("backend"), "backend=oracle");

    return PASS;
}

static rc
testRewrite(void)
{
    gprom_configFromOptions();

    const char *result = gprom_rewriteQuery("SELECT * FROM r;");
    ASSERT_EQUALS_STRING("\nSELECT F0.A AS A, F0.B AS B\nFROM (R F0);\n\n\n", result, "SELECT * FROM r;");

    return PASS;
}

static rc
testLoopBackMetadata(void)
{
    gprom_setStringOption("backend", "oracle");



    return PASS;
}
