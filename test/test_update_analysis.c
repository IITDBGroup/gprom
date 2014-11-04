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
#include "parser/parser.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/translate_update.h"
#include "rewriter.h"

int
main (int argc, char* argv[])
{
    Node *result;

    READ_OPTIONS_AND_INIT("testupdateanalysis", "Run parser + analysis stages only.");

    initLogger();

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

    QueryOperator *anaResult = translateUpdate(result);
    ERROR_LOG("TRANSLATION RESULT IS <%s>", nodeToString(anaResult));

    return shutdownApplication();
}
