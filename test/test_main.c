/*-------------------------------------------------------------------------
 *
 * test_main.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "utility/string_utils.h"
#include "mem_manager/mem_mgr.h"
#include "test_main.h"
#include "log/termcolor.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "rewriter.h"

static char *getDBPath (int argc, char* argv[]);

void
testSuites(void)
{
    RUN_TEST(testLogger(), "Logger test.");
    RUN_TEST(testMemManager(), "Memory manager test.");
//    RUN_TEST(testException(), "Exception handling.");
    RUN_TEST(testList(), "List model.");
    RUN_TEST(testSet(), "Set.");
    RUN_TEST(testVector(), "Vector.");
    RUN_TEST(testHashMap(), "HashMap.");
    RUN_TEST(testExpr(), "Expression model.");
    RUN_TEST(testCopy(), "Test generic copy function.");
    RUN_TEST(testEqual(), "Test generic equality function.");
    RUN_TEST(testStringUtils(), "Test String utilities.");
    RUN_TEST(testToString(), "Test generic toString function.");
    RUN_TEST(testString(), "Test stringinfo.");
    RUN_TEST(testParse(), "Test parser.");
    RUN_TEST(testMetadataLookup(), "Test metadata lookup.");
    RUN_TEST(testMetadataLookupPostgres(), "Test metadata lookup - Postgres.");
    RUN_TEST(testParameter(), "Test SQL parameter functions.");
    RUN_TEST(testDatalogModel(), "Test datalog model features");
    RUN_TEST(testHash(), "Test hash computation for nodes");
//    RUN_TEST(testLibGProM(), "Test gprom dynamic link library"); make this work
    RUN_TEST(testRPQ(), "Test regular path query features");

    printf("\n" T_FG_BG(WHITE,BLACK,"                                                            ") "\n"
            "Total %d Test(s) Passed\n\n", test_count);
}

#define xstr(s) str(s)
#define str(s) #s
#define TOPDIR xstr(GPROM_TOP_SRCDIR)

int
main(int argc, char* argv[])
{
    READ_OPTIONS_AND_BASIC_INIT("testmain", "Regression test suite. Runs a bunch of whitebox tests on components of the system.");

    // get directory where testmain resides to determine location of the test database
    char *path = getDBPath(argc,argv);
    printf("dbPath: %s\n", path);

    // print options
    DEBUG_LOG("configuration:\n\n");
    printCurrentOptions(stdout);

    // setup options to use sqlite unless the user has specified a backend (in which case we use the user provided connection parameters)
    if(getStringOption("backend") == NULL)
    {
        setOption("connection.db", strdup(path));
        setOption(OPTION_PLUGIN_SQLCODEGEN, "sqlite");
        setOption(OPTION_PLUGIN_METADATA, "sqlite");
        setOption(OPTION_PLUGIN_PARSER, "oracle");
        setOption(OPTION_PLUGIN_ANALYZER, "oracle");
        setOption(OPTION_PLUGIN_TRANSLATOR, "oracle");
    }
    setupPluginsFromOptions();

    // print options
    DEBUG_LOG("configuration:\n\n");
    printCurrentOptions(stdout);

    testSuites();

    shutdownApplication();

    return EXIT_SUCCESS;
}


static char *
getDBPath (int argc, char* argv[])
{
    StringInfo dbPath = makeStringInfo();
    char *self = argv[0];

    // try to find file relative to our own directory
    self = getFullMatchingSubstring(self, "^([^/]*[/])+");
    appendStringInfoString(dbPath, self);
    appendStringInfoString(dbPath, "/../examples/test.db");
    if (fileExists(dbPath->data))
    {
        return dbPath->data;
    }
    printf("SQLite test database not found where expected relative to ourselves:\n%s", dbPath->data);

    // use TOPDIR to find
    dbPath = makeStringInfo();
    appendStringInfoString(dbPath, TOPDIR);
    appendStringInfoString(dbPath, "/examples/test.db");
    if (fileExists(dbPath->data))
    {
        return dbPath->data;
    }
    printf("SQLite test database not found relative to top src dir:\n%s", dbPath->data);

    // check if arg 1 is given check whether it is a database
    if (argc >= 1)
    {
        dbPath = makeStringInfo();
        appendStringInfoString(dbPath, argv[1]);
        if (fileExists(dbPath->data))
        {
            return dbPath->data;
        }
        printf("SQLite test database not found based on the path given as first parameter argument:\n%s", dbPath->data);
    }

    FATAL_LOG("did not find test.db database");
    return NULL;
}
