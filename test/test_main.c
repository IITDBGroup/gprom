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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem_manager/mem_mgr.h"
#include "test_main.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"


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
            printf("%sTEST PASS [%s-%s-%u]: %s\n", indentation, file, func,
                    line, msg);
            test_count++;
            free(indentation);
            return;
        }
        else
        {
            printf("%sTEST FAIL [%s-%s-%u]: %s\n", indentation, file, func,
                    line, msg);
            free(indentation);
            exit(1);
        }
    }
    else
    {
        if (r == PASS)
        {
            printf("%sTEST SUITE [%s-%s-%u]: %s - PASSED %u TESTS\n",
                    indentation, file, func, line, msg, tests_passed);
            test_count++;
            free(indentation);
            return;
        }
        else
        {
            printf("%sTEST SUITE FAILED [%s-%s-%u]: %s AFTER %u TESTS\n",
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

void
testSuites(void)
{
    RUN_TEST(testLogger(), "Logger test.");
    RUN_TEST(testMemManager(), "Memory manager test.");
    RUN_TEST(testList(), "List model.");
    RUN_TEST(testExpr(), "Expression model.");
    RUN_TEST(testCopy(), "Test generic copy function.");
    RUN_TEST(testEqual(), "Test generic equality function.");
    RUN_TEST(testToString(), "Test generic toString function.");
    RUN_TEST(testParse(), "Test parser.");

    printf("Total %d Test(s) Passed\n\n", test_count);
}

int
main(int argc, char* argv[])
{
    initMemManager();
    mallocOptions();
    parseOption(argc, argv);

    initLogger();
    testSuites();

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}
