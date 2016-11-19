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

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
#include "common.h"
#include "utility/string_utils.h"
#include "mem_manager/mem_mgr.h"
#include "test_main.h"
#include "log/termcolor.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "rewriter.h"

static boolean fileExists (char *file);

int test_count = 0;
int test_rec_depth = 0;

void
checkResult(int r, char *msg, const char *file, const char *func, int line,
        int tests_passed)
{
    char *indentation = getIndent(test_rec_depth);

    if (tests_passed == -1)
    {
        if (r == PASS)
        {
            printf("%s" T_FG_BG(BLACK,GREEN,"TEST PASS") TBCOL(GREEN,"[%s-%s-%u]:")
                    " %s\n", indentation, file, func, line, msg);
            test_count++;
            free(indentation);
            return;
        }
        else
        {
            printf("%s" T_FG_BG(WHITE,RED,"TEST FAIL") TBCOL(RED,"[%s-%s-%u]:")
                    " %s\n", indentation, file, func,line, msg);
            free(indentation);
            exit(1);
        }
    }
    else
    {
        if (r == PASS)
        {
            printf("%s" T_FG_BG(BLACK,GREEN,"TEST SUITE")
                    TBCOL(GREEN,"[%s-%s-%u]:") " %s - PASSED %u TESTS\n",
                    indentation, file, func, line, msg, tests_passed);
            test_count++;
            free(indentation);
            return;
        }
        else
        {
            printf("%s" T_FG_BG(BLACK,RED,"TEST SUITE FAILED") TBCOL(RED,"[%s-%s-%u]:")
                    " %s AFTER %u TESTS\n",
                    indentation, file, func, line, msg, tests_passed);
            free(indentation);
            exit(1);
        }
    }
}

char *
getIndent(int depth)
{
    char *result = malloc(depth + 1);

    for(int i = 0; i < depth; i++)
        result[i] = '\t';
    result[depth] = '\0';

    return result;
}

boolean
testQuery (char *query, char *expectedResult)
{
    return TRUE;
}

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
    RUN_TEST(testLibGProM(), "Test gprom dynamic link library");
    RUN_TEST(testRPQ(), "Test regular path query features");

    printf("\n" T_FG_BG(WHITE,BLACK,"                                                            ") "\n"
            "Total %d Test(s) Passed\n\n", test_count);
}

int
main(int argc, char* argv[])
{
    READ_OPTIONS_AND_BASIC_INIT("testmain", "Regression test suite. Runs a bunch of whitebox tests on components of the system.");

    // get directory where testmain resides to determine location of the test database
    char *path=argv[0];
    StringInfo dbPath = makeStringInfo();
    path = strRemPostfix(path, 9);
    appendStringInfoString(dbPath, path);
    appendStringInfoString(dbPath, "/../../examples/test.db");
    if (!fileExists(dbPath->data))
        FATAL_LOG("SQLite test database not found where expected:\n", dbPath->data);
    printf("dbPath: %s\n", dbPath->data);

    // setup options to use sqlite
    setOption("connection.db", dbPath->data);
    setOption("plugin.sqlserializer", "sqlite");
    setOption("plugin.metadata", "sqlite");
    setOption("plugin.parser", "oracle");
    setOption("plugin.analyzer", "oracle");
    setOption("plugin.translator", "oracle");

    setupPluginsFromOptions();

    // print options
    DEBUG_LOG("configuration:\n\n");
    printCurrentOptions(stdout);

    testSuites();

    shutdownApplication();

    return EXIT_SUCCESS;
}

boolean
fileExists (char *file)
{
    FILE *f;
    if ((f = fopen(file, "r")))
    {
        fclose(f);
        return 1;
    }
    return 0;
}
