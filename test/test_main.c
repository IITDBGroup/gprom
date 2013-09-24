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
#include "mem_manager/mem_mgr.h"
#include <string.h>
#include "test_main.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"

static int count = 0;

int test_rec_depth = 0;

void
checkResult(int r, char *msg, const char *file, const char *func, int line)
{
    char *indentation = getIndent(test_rec_depth);

    if (r == PASS)
    {
        printf("%sTEST PASS [%s-%s-%u]: %s\n", indentation, file, func, line, msg);
        count++;
        free(indentation);
        return;
    }
    else
    {
        printf("%sTEST FAIL [%s-%s-%u]: %s\n", indentation, file, func, line, msg);
        free(indentation);
        exit(1);
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
    RUN_TEST(testExpr(), "Expression model.");
    RUN_TEST(testCopy(), "Test generic copy function.");
    RUN_TEST(testEqual(), "Test generic equality function.");

    printf("Total %d Test(s) Passed\n\n", count);
}

int
main(int argc, char* argv[])
{
    mallocOptions();
    MemContext *testContext = newMemContext("TEST CONTEXT");

    parseOption(argc, argv);
    testSuites();

    freeMemContext(testContext);

    return EXIT_SUCCESS;
}
