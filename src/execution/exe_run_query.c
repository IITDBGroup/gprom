/*-----------------------------------------------------------------------------
 *
 * exe_run_query.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "metadata_lookup/metadata_lookup.h"
#include "execution/exe_run_query.h"
#include "utility/string_utils.h"

void
exeRunQuery (void *code)
{
    List *res;
    int i = 0;
    char *adaptedQuery;

    // remove semicolon
    adaptedQuery = replaceSubstr(code, ";", ""); //TODO not safe if ; in strings

    // execute query
    INFO_LOG("run query:\n%s", (char *) adaptedQuery);
    res = executeQuery((char *) adaptedQuery);

    // output results
    FOREACH(List,t,res)
    {
        FOREACH(char,a,t)
            printf("%s|", a);
        printf("\n");

        if ((i++ % 1000) == 0)
            fflush(stdout);
    }
}
