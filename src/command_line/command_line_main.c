/*-------------------------------------------------------------------------
 *
 * command_line_main.c
 *    commandline interface for user
 *
 *        This file includes main method for the program. It takes a query
 *   	  and database connection parameters as input, parses the options,
 *   	  and print out some help information.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "command_line/command_line_main.h"

#include "mem_manager/mem_mgr.h"
#include "instrumentation/timing_instrumentation.h"

#include "log/logger.h"
#include "log/termcolor.h"
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


static void process(char *sql);
static void inputSQL(void);
static ExceptionHandler handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s);

int
main(int argc, char* argv[])
{
    char *result;
    boolean optionsInit;

    optionsInit = readOptionsAndIntialize("gprom", "GProM command line application.", argc, argv);
    if (optionsInit)
        return EXIT_FAILURE;

    registerExceptionCallback(handleCLIException);

    START_TIMER("TOTAL");

    // read from terminal
    if (getBoolOption("help"))
    {

    }
    else if (getStringOption("input.sql") != NULL)
    {
        result = rewriteQuery(getStringOption("input.sql"));
        // call executor
        execute(result);
    }
    else if (getStringOption("input.sqlFile") != NULL)
    {
        char *fName = getStringOption("input.sqlFile");
        FILE *file = fopen(fName, "r");

        if (file == NULL)
            FATAL_LOG("could not open file %s with error %s", fName, strerror(errno));

        result =  rewriteQueryFromStream(file);
        fclose(file);

        // call executor
        execute(result);
    }
    // parse input string
    else
    {
        printf("GProM Commandline Client\n");
        printf("Please input a SQL or '(q)uit' to exit the program\n");
        printf("=============================================\n\n");
        inputSQL();
    }

    STOP_TIMER("TOTAL");
    OUT_TIMERS();

    shutdownApplication();

    return EXIT_SUCCESS;
}

static void
inputSQL(void)
{
	char* sql=(char*) CALLOC(100000,1);
	while(TRUE)
	{
	    char *returnVal;

	    printf(TB_FG_BG(WHITE,BLACK,"%s"), "GProm $");
	    printf(" ");
	    returnVal = gets(sql);

		// deal with failure
		if (returnVal == NULL && ferror(stdin))
		{
		    printf("\n\nError reading from stdin\n");
		    exit(EXIT_FAILURE);
		}

		DEBUG_LOG("process SQL code: ", sql);

		if(*sql=='q')
		{
			printf(TB_FG_BG(WHITE,BLACK,"%s"),"\n\nExit GProM.\n");
			printf("\n");
			break;
		}

		TRY
		{
            NEW_AND_ACQUIRE_MEMCONTEXT("PROCESS_CONTEXT");
            process(sql);
            FREE_AND_RELEASE_CUR_MEM_CONTEXT();
		}
		ON_EXCEPTION
		{
		    printf("\nError occured\n");
		}
		END_ON_EXCEPTION
	}
	FREE(sql);
}

/*
 * Parse -> Translate -> Provenance Rewrite -> Serialize -> Execute
 */
static void
process(char *sql)
{
    processInput(sql);
}

/*
 * Function that handles exceptions
 */
static ExceptionHandler
handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s)
{
    if (streq(file, "sql_parser.l"))
    {
        printf(TCOL(RED, "PARSE ERORR:") " %s", message);
    }
    else
        printf(TCOL(RED,"(%s:%u) ") "\n%s\n",
                file, line, message);
    return EXCEPTION_WIPE;
}
