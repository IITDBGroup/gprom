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
#include "model/relation/relation.h"
#include "metadata_lookup/metadata_lookup.h"
#include "execution/exe_run_query.h"
#include "utility/string_utils.h"
#include "configuration/option.h"
#include "provenance_rewriter/game_provenance/gp_bottom_up_program.h"

static void outputResult(Relation *res);
static void printDBsample(List *stmts);


void
exeRunQuery (void *code)
{
    Relation *res = NULL;
    char *adaptedQuery;
    boolean showResult = getBoolOption(OPTION_SHOW_QUERY_RESULT);
    boolean showTime = getBoolOption(OPTION_TIME_QUERIES);
    struct timeval st;
    struct timeval et;
    char *format = getStringOption(OPTION_TIME_QUERY_OUTPUT_FORMAT);
    int repeats = getIntOption(OPTION_REPEAT_QUERY);

    	if (getBoolOption(OPTION_INPUTDB))
    	{
    	    List *codes = splitString(code, ";");
    	    printDBsample(codes);
    	    return;
    	}
	
    // remove semicolon
    adaptedQuery = replaceSubstr(code, ";", ""); //TODO not safe if ; in strings

    // execute query
    INFO_LOG("run query (show results: %s, time query: %s):\n%s", showResult ? "yes" : "no", showTime ? "yes" : "no", (char *) adaptedQuery);

    for (int i = 0; i < repeats; i++)
    {
        if (showTime)
        {
            gettimeofday(&st, NULL);
        }

        if (showResult)
            res = executeQuery((char *) adaptedQuery);
        else
            executeQueryIgnoreResult((char *) adaptedQuery);

        if (showTime)
        {
            gettimeofday(&et, NULL);
        }

        if (showResult == TRUE)
        {
            outputResult(res);
        }

        if (showTime)
        {
            long usecDiff;
            long secDiff;
            double msecs;

            secDiff = et.tv_sec - st.tv_sec;
            usecDiff = et.tv_usec - st.tv_usec;

            msecs = secDiff * 1000 + (((double) usecDiff) / 1000.0);
            if (showResult)
                printf("\n");

            if (format != NULL)
                printf(format, msecs);
            else
                printf("query took %12f msec\n", msecs);
            fflush(stdout);
        }
    }
}

static void
printDBsample(List *stmts)
{
    int s = 0;
    Relation *res = NULL;

    FOREACH(char,c, stmts)
    {
        char *r = (char *) MAP_GET_STRING(edbRels,gprom_itoa(++s));
        printf("%s", r);
        printf("\n");
        res = executeQuery((char *) c);
        outputResult(res);
    }
}


static void
outputResult(Relation *res)
{
    int *colSizes;
    int numCol;
    int totalSize = 0;
    int i = 0;
    int l = 0;
    numCol = LIST_LENGTH(res->schema);
    colSizes = MALLOC(numCol * sizeof(int));

    // determine column sizes
    i = 0;
    FOREACH(char,a,res->schema)
    {
        colSizes[i++] = strlen(a) + 2;
    }

    FOREACH(List,t,res->tuples)
    {
        i = 0;
        FOREACH(char,a,t)
        {
            int len = a ? strlen(a) : 4;
            colSizes[i] = colSizes[i] < len + 2 ? len + 2 : colSizes[i];
            i++;
        }
    }

    for (i = 0; i < numCol; i++)
        totalSize += colSizes[i] + 1;

    // output columns
    i = 0;
    FOREACH(char,a,res->schema)
    {
        printf(" %s", a);
        for(int j = strlen(a) + 1; j < colSizes[i]; j++)
            printf(" ");
        printf("|");
        i++;
    }
    printf("\n");
    for (int j = 0; j < totalSize; j++)
        printf("-");
    printf("\n");

    // output results
    FOREACH(List,t,res->tuples)
    {
        i = 0;
        FOREACH(char,a,t)
        {
            char *out = a ? a : "NULL";
            printf(" %s", out);
            for(int j = strlen(out) + 1; j < colSizes[i]; j++)
                printf(" ");
            printf("|");
            i++;
        }
        printf("\n");

        if ((l++ % 1000) == 0)
            fflush(stdout);
    }
}
