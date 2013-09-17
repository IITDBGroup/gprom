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
#include "mem_mgr.h"
#include <string.h>
#include "test_main.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"

static int count = 0;

void
checkResult(int r, char *msg)
{
    if (r == PASS)
    {
        printf("TEST PASS: %s\n\n", msg);
        count++;
        return;
    }
    else
    {
        printf("TEST FAIL: %s\n\n", msg);
        exit(1);
    }
}

void
testSuites(void)
{
    checkResult(testLogger(), "Logger test.");
    checkResult(testMemManager(), "Memory manager test.");
    printf("Total %d Test(s) Passed\n\n", count);
}

int
main(int argc, char* argv[])
{
    mallocOptions();

    parseOption(argc, argv);

    testSuites();

    freeOptions();
    return EXIT_SUCCESS;
}
