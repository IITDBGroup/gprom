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
#include "../src/parser/sql_parser.tab.h"


int
main (int argc, char* argv[])
{
    Node *result;

    initMemManager();
    mallocOptions();
    parseOption(argc, argv);

    initLogger();

    if (yyparse())
        FATAL_LOG("PARSE ERROR!");
    else
    {
        ERROR_LOG("ADDRESS OF PARSE RESULT: %p", bisonParseResult);
        ERROR_LOG("%u", isA(bisonParseResult, QueryBlock));
        ERROR_LOG("PARSE RESULT TO AS STRING <%s>", nodeToString(bisonParseResult));
    }
    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}
