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


int
main (int argc, char* argv[])
{
    Node *result;

    initMemManager();
    mallocOptions();
    parseOption(argc, argv);

    initLogger();

    // read from terminal
    if (getOptions()->optionConnection->sql == NULL)
    {
        result = parseStream(stdin);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
    }
    // parse input string
    else
    {
        result = parseFromString(getOptions()->optionConnection->sql);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
    }
    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}
