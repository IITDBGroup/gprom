/*
 * test_rewrite.c
 *
 *      Author: zephyr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "instrumentation/timing_instrumentation.h"

#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parse_internal.h"
#include "parser/parser.h"
#include "../src/parser/sql_parser.tab.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "execution/executor.h"
#include "rewriter.h"


/* if OCI is not available then add dummy versions */
#if HAVE_A_BACKEND

int
main (int argc, char* argv[])
{
    char *result;

    READ_OPTIONS_AND_INIT("testrewriter", "Run all stages on input and output rewritten SQL.");

    START_TIMER("TOTAL");

    // read from terminal
    if (getStringOption("input.sql") == NULL)
    {
        result = rewriteQueryFromStream(stdin);
        ERROR_LOG("REWRITE RESULT FROM STREAM IS <%s>", result);
    }
    // parse input string
    else
    {
        result = rewriteQuery(getStringOption("input.sql"));
        ERROR_LOG("REWRITE RESULT FROM STRING IS:\n%s", result);
    }

    // call executor
    execute(result);

    STOP_TIMER("TOTAL");
    OUT_TIMERS();

    shutdownApplication();
//    freeOptions();
//    destroyMemManager();

    return EXIT_SUCCESS;
}



/* if OCI or OCILIB are not avaible replace with dummy test */
#else

int main()
{
    return EXIT_SUCCESS;
}

#endif





