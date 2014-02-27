/*
 * test_rewrite.c
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
#include "rewriter.h"


/* if OCI is not available then add dummy versions */
#if HAVE_LIBOCILIB && (HAVE_LIBOCI || HAVE_LIBOCCI)

int
main (int argc, char* argv[])
{
    char *result;

    initMemManager();
    mallocOptions();
    parseOption(argc, argv);
    initLogger();
    initMetadataLookupPlugin();

    // read from terminal
    if (getOptions()->optionConnection->sql == NULL)
    {
        result = rewriteQueryFromStream(stdin);
        ERROR_LOG("REWRITE RESULT FROM STREAM IS <%s>", result);
    }
    // parse input string
    else
    {
        result = rewriteQuery(getOptions()->optionConnection->sql);
        ERROR_LOG("REWRITE RESULT FROM STRING IS:\n%s", result);

//        result = rewriteQueryWithOptimization(
//                getOptions()->optionConnection->sql);
        ERROR_LOG("REWRITE RESULT AFTER OPTIMIZATIONS IS:\n%s", result);
    }

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





