/*
 * test_analysis.c
 *
 *      Author: zephyr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem_manager/mem_mgr.h"
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
#include "analysis_and_translate/analyze_qb.h"


/* if OCI is not available then add dummy versions */
#if HAVE_A_BACKEND

int
main (int argc, char* argv[])
{
    Node *result;

    initMemManager();
    mallocOptions();
    if(parseOption(argc, argv) != 0)
    {
        printOptionParseError(stdout);
        printOptionsHelp(stdout, "testrewriter", "Run all stages on input and outpur rewritten SQL.");
        return EXIT_FAILURE;
    }
    initLogger();

    initMetadataLookupPlugins();
    chooseMetadataLookupPluginFromString(getStringOption("backend"));
    initMetadataLookupPlugin();

    // read from terminal
    if (getStringOption("input.sql") == NULL)
    {
        result = parseStream(stdin);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
    }
    // parse input string
    else
    {
        result = parseFromString(getStringOption("input.sql"));

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
    }

    analyzeQueryBlockStmt(result, NULL);
    ERROR_LOG("ANALYSIS RESULT IS <%s>", nodeToString(result));
    ERROR_LOG("ANALYSIS RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}



/* if OCI or OCILIB are not avaible replace with dummy test */
#else

int main()
{
	return EXIT_SUCCESS;
}

#endif


