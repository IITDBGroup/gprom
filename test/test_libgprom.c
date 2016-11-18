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
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "rewriter.h"

static rc testConfiguration(void);
static rc testRewrite(void);
static rc testLoopBackMetadata(void);

static HashMap *options = NULL;

static void setOpts (void);


rc
testLibGProM(void)
{
    options = optionsToHashMap();

    gprom_init();

    RUN_TEST(testConfiguration(), "test configuration interface");
    RUN_TEST(testRewrite(), "test rewrite function");
    RUN_TEST(testLoopBackMetadata(), "test loop back metadata lookup");

    return PASS;
}

static void
setOpts (void)
{
    // copy connection options provided by user
    gprom_setStringOption("backend", "sqlite");
    gprom_setStringOption("plugin.parser", "oracle");
    gprom_setStringOption("plugin.analyzer", "oracle");
    gprom_setStringOption("plugin.translator", "oracle");
    gprom_setOption(OPTION_CONN_USER, STRING_VALUE(MAP_GET_STRING(options, OPTION_CONN_USER)));
    gprom_setOption(OPTION_CONN_DB, STRING_VALUE(MAP_GET_STRING(options, OPTION_CONN_DB)));
    gprom_setOption(OPTION_CONN_PORT, STRING_VALUE(MAP_GET_STRING(options, OPTION_CONN_PORT)));
    gprom_setOption(OPTION_CONN_HOST, STRING_VALUE(MAP_GET_STRING(options, OPTION_CONN_HOST)));
    gprom_setOption(OPTION_CONN_PASSWD, STRING_VALUE(MAP_GET_STRING(options, OPTION_CONN_PASSWD)));
}

static rc
testConfiguration(void)
{
    setOpts();

    ASSERT_EQUALS_STRING("sqlite", gprom_getStringOption("backend"), "backend=sqlite");
    ASSERT_EQUALS_STRING(STRING_VALUE(MAP_GET_STRING(options,OPTION_CONN_HOST)),
            gprom_getStringOption(OPTION_CONN_HOST),
            "connection host as provided by user");
    ASSERT_EQUALS_STRING(STRING_VALUE(MAP_GET_STRING(options,OPTION_CONN_USER)),
                gprom_getStringOption(OPTION_CONN_USER),
                "connection host as provided by user");
    return PASS;
}

static rc
testRewrite(void)
{
    setOpts();
    gprom_configFromOptions();

    const char *result = gprom_rewriteQuery("SELECT * FROM r;");
    ASSERT_EQUALS_STRING("\nSELECT F0.A AS A, F0.B AS B\nFROM R AS F0;\n\n\n", result, "SELECT * FROM r;");

    return PASS;
}

static rc
testLoopBackMetadata(void)
{
    setOpts();
    return PASS;
}
