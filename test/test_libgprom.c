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
#include "utility/string_utils.h"
#include "configuration/option.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "rewriter.h"

static rc testConfiguration(void);
static rc testRewrite(void);
static rc testLoopBackMetadata(void);
static rc testExceptionCatching(void);


static int hitCallback = 0;
static HashMap *options = NULL;

static ExceptionHandler handleE (const char *message, const char *file, int line, ExceptionSeverity s);
//static void setup(void);
static void setOpts (void);
static void resetOpts (void);

int
main(int argc, char* argv[])
{
    READ_OPTIONS_AND_BASIC_INIT("testlibgprom", "Regression test suite. Runs a bunch of whitebox tests on libgprom.");

    // get directory where testmain resides to determine location of the test database
    char *path=argv[0];
    StringInfo dbPath = makeStringInfo();
    path = getFullMatchingSubstring(path, "^([^/]*[/])+");
    appendStringInfoString(dbPath, path);
    appendStringInfoString(dbPath, "../../examples/test.db");
    if (!fileExists(dbPath->data))
        FATAL_LOG("SQLite test database not found where expected:\n", dbPath->data);
    printf("dbPath: %s\n", dbPath->data);

    // setup options to use sqlite unless the user has specified a backend (in which case we use the user provided connection parameters)
    if(getStringOption("backend") == NULL)
    {
        setOption("connection.db", dbPath->data);
        setOption(OPTION_BACKEND, "sqlite");
        setOption(OPTION_PLUGIN_SQLSERIALIZER, "sqlite");
        setOption(OPTION_PLUGIN_METADATA, "sqlite");
        setOption(OPTION_PLUGIN_PARSER, "oracle");
        setOption(OPTION_PLUGIN_ANALYZER, "oracle");
        setOption(OPTION_PLUGIN_TRANSLATOR, "oracle");
    }

    // print options
    DEBUG_LOG("configuration:\n\n");
    printCurrentOptions(stdout);

    testLibGProM();

    printf("\n" T_FG_BG(WHITE,BLACK,"                                                            ") "\n"
            "Total %d Test(s) Passed\n\n", test_count);

    return EXIT_SUCCESS;
}

rc
testLibGProM(void)
{
    options = optionsToHashMap();

    gprom_init();

    RUN_TEST(testConfiguration(), "test configuration interface");
    RUN_TEST(testRewrite(), "test rewrite function");
    RUN_TEST(testLoopBackMetadata(), "test loop back metadata lookup");
    RUN_TEST(testExceptionCatching(), "test exception mechanism");

    resetOpts();

    return PASS;
}

#define RESET_OPT(_opt) setOption(_opt, STRING_VALUE(MAP_GET_STRING(options, _opt)))

static void
resetOpts (void)
{
     RESET_OPT("log.active");
     RESET_OPT("log.level");
     RESET_OPT("backend");
     RESET_OPT("plugin.parser");
     RESET_OPT("plugin.analyzer");
     RESET_OPT("plugin.translator");
     RESET_OPT(OPTION_CONN_USER);
     RESET_OPT(OPTION_CONN_DB);
     RESET_OPT(OPTION_CONN_PORT);
     RESET_OPT(OPTION_CONN_HOST);
     RESET_OPT(OPTION_CONN_PASSWD);
}

static void
setOpts (void)
{
    // copy connection options provided by user
    gprom_setOption("log.active", "true");
    gprom_setOption("log.level", STRING_VALUE(MAP_GET_STRING(options, "log.level")));
    gprom_setOption("backend", "sqlite");
    gprom_setOption("plugin.parser", "oracle");
    gprom_setOption("plugin.analyzer", "oracle");
    gprom_setOption("plugin.translator", "oracle");
    gprom_setOption("plugin.metadata", "sqlite");
    gprom_setOption("plugin.sqlserializer", "sqlite");
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

static rc
testExceptionCatching(void)
{
//    MemContext *cur = getCurMemContext();
    MemContext *after;

    // try the same with libgprom
    hitCallback = 0;
    gprom_registerExceptionCallbackFunction((GProMExceptionCallbackFunction) handleE);

    char *result = (char *) gprom_rewriteQuery("SELECT * FRO R;");
    after = getCurMemContext();

    ASSERT_EQUALS_INT(1,hitCallback, "exception handler was called once");
    ASSERT_EQUALS_STRINGP(NULL, result, "empty string result");
    ASSERT_EQUALS_STRINGP("LIBGRPROM_QUERY_CONTEXT", after->contextName, "back to context before exception");

    gprom_shutdown();

    return PASS;
}

static ExceptionHandler
handleE (const char *message, const char *file, int line, ExceptionSeverity s)
{
    hitCallback++;
    return EXCEPTION_ABORT;
}

