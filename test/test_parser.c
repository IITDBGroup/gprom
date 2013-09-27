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
#include "parser/parse_internal.h"
#include "../src/parser/sql_parser.tab.h"


int
main (int argc, char* argv[])
{
    initMemManager();
    mallocOptions();
    parseOption(argc, argv);

    initLogger();
    yyparse();

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}
