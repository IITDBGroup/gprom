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
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "test_main.h"
#include "log/termcolor.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "rewriter.h"

static char *getDBPath (int argc, char* argv[]);
static void setupTestMap(void);
static void printTests (void);
static void runOneTest (char *testName);


typedef struct TestNameToFunc {
    char *name;
    rc (*f) (void);
} TestNameToFunc;

static TestNameToFunc testFuncs [] = {
        { "autocast", testAutocast },
		{ "bitset", testBitset },
        { "copy", testCopy },
        { "datalog_model", testDatalogModel },
        { "dynstring", testString },
        { "equal", testEqual },
        { "exception", testException },
        { "expr", testExpr },
        { "hashmap", testHashMap },
        { "list", testList },
        { "logger", testLogger },
        { "mem_mgr", testMemManager },
        { "metadatalookup", testMetadataLookup },
        { "metadatalookup_postgres", testMetadataLookupPostgres },
        { "parameter", testParameter },
        { "parse", testParse },
        { "rpq", testRPQ },
        { "set", testSet },
        { "stringutils", testStringUtils },
        { "tostring", testToString },
        { "vector", testVector },
		{ "z3", testZ3 },
        { NULL, NULL }
};

char *_testStringBuf;
static HashMap *testMap;

void
testSuites(void)
{
    RUN_TEST(testLogger(), "Logger test");
    RUN_TEST(testMemManager(), "Memory manager test");
    RUN_TEST(testList(), "List model");
    RUN_TEST(testSet(), "Set");
    RUN_TEST(testVector(), "Vector");
	RUN_TEST(testBitset(), "Bitset");
    RUN_TEST(testHashMap(), "HashMap");
    RUN_TEST(testExpr(), "Expression model");
    RUN_TEST(testCopy(), "Test generic copy function");
    RUN_TEST(testEqual(), "Test generic equality function");
    RUN_TEST(testStringUtils(), "Test String utilities");
    RUN_TEST(testToString(), "Test generic toString function");
    RUN_TEST(testString(), "Test stringinfo");
    RUN_TEST(testException(), "Exception handling");
    RUN_TEST(testParse(), "Test parser");
    RUN_TEST(testMetadataLookup(), "Test metadata lookup");
    RUN_TEST(testMetadataLookupPostgres(), "Test metadata lookup - Postgres");
    RUN_TEST(testParameter(), "Test SQL parameter functions");
    RUN_TEST(testDatalogModel(), "Test datalog model features");
    RUN_TEST(testHash(), "Test hash computation for nodes");
//    RUN_TEST(testLibGProM(), "Test gprom dynamic link library"); make this work
    RUN_TEST(testRPQ(), "Test regular path query features");
    RUN_TEST(testAutocast(), "Test automatic casting");
	RUN_TEST(testZ3(), "Testing Z3 constraint solving.");

    printf("\n" T_FG_BG(WHITE,BLACK,"                                                            ") "\n"
            "Total %d Test(s) Passed\n\n", test_count);
}

static void
setupTestMap(void)
{
    TestNameToFunc *test;

    testMap = NEW_MAP(Constant, Constant);

    test = testFuncs;

    while((test)->name != NULL)
    {
        MAP_ADD_STRING_KEY(testMap, strdup(test->name),
                createConstLong((gprom_long_t) test->f));
        test++;
    }
}

int
main(int argc, char* argv[])
{
    READ_OPTIONS_AND_BASIC_INIT("testmain", "Regression test suite. Runs a bunch of whitebox tests on components of the system.");
    setupTestMap();

    // get directory where testmain resides to determine location of the test database
    char *path = getDBPath(argc,argv);
    char *test = getStringOption("test");
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

    _testStringBuf = (char *) malloc(STRING_BUFFER_SIZE);

    if (test == NULL)
        testSuites();
    else
        runOneTest(test);

    free(_testStringBuf);

    shutdownApplication();

    return EXIT_SUCCESS;
}

static void
runOneTest (char *testName)
{
    if(!MAP_HAS_STRING_KEY(testMap,testName))
    {
        printf("No test named %s exists!\n\n", testName);
        printTests();
        exit(1);
    }
    else
    {
        rc (*test) (void) = (rc (*)(void)) LONG_VALUE(MAP_GET_STRING(testMap, testName));
        StringInfo s = makeStringInfo();
        appendStringInfo(s, "run test: %s", testName);
        RUN_TEST(test(), s->data);
    }
}

static void
printTests (void)
{
    printf("Available tests:\n\n");
    FOREACH_HASH_KEY(Constant,t,testMap)
    {
        char *tName = STRING_VALUE(t);
        printf("\t%s\n", tName);
    }
    printf("\n\n");
    fflush(stdout);
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
    printf("SQLite test database not found where expected relative to ourselves:\n<%s>\\n\\n", dbPath->data);

    // use TOPDIR to find
    dbPath = makeStringInfo();
    appendStringInfoString(dbPath, GPROM_TOPDIR);
    appendStringInfoString(dbPath, "/examples/test.db");
    if (fileExists(dbPath->data))
    {
        return dbPath->data;
    }
    printf("SQLite test database not found relative to top src dir:\n<%s>\n\n", dbPath->data);

    // check if arg 1 is given check whether it is a database
    if (argc >= 1)
    {
        dbPath = makeStringInfo();
        appendStringInfoString(dbPath, argv[1]);
        if (fileExists(dbPath->data))
        {
            return dbPath->data;
        }
        printf("SQLite test database not found based on the path given as first parameter argument:\n<%s>\n\n", dbPath->data);
    }

    FATAL_LOG("did not find test.db database");
    return NULL;
}
