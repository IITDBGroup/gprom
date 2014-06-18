/*-----------------------------------------------------------------------------
 *
 * test_parser.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
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
#include "analysis_and_translate/translate_update.h"

int
main (int argc, char* argv[])
{
    Node *result;

    initMemManager();
    mallocOptions();
    if(parseOption(argc, argv) != 0)
    {
        printOptionParseError(stdout);
        printOptionsHelp(stdout, "testanalysis", "Run parser + analysis stages only.");
        return EXIT_FAILURE;
    }

    initLogger();

    // read from terminal
    if (getStringOption("input.sql") == NULL)
    {
        result = parseStream(stdin);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(bisonParseResult)));
    }
    // parse input string
    else
    {
        result = parseFromString(getStringOption("input.sql"));

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(bisonParseResult));
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(bisonParseResult)));
    }

    QueryOperator *anaResult = translateUpdate(result);
    ERROR_LOG("TRANSLATION RESULT IS <%s>", nodeToString(anaResult));

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}
