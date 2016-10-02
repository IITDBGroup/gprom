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

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

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

static const char *commandHelp = "\n"
            "\\q\t\texit CLI\n"
            "\\h\t\tshow this helptext\n"
            "\\i\t\tshow help for currently selected parser\n"
            "\\o\t\tshow current parameter settings\n"
        ;

static char *prompt = "GProM";

static void process(char *sql);
static void inputSQL(void);
static ExceptionHandler handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s);
static char *readALine(char **str);

int
main(int argc, char* argv[])
{
    char *result;
    boolean optionsInit;

    optionsInit = readOptionsAndIntialize("gprom", "GProM command line application.", argc, argv);
    if (optionsInit)
        return EXIT_FAILURE;

    registerExceptionCallback(handleCLIException);
//
//    prompt = CONCAT_STRINGS("\\[",TERM_ALL_ON(BOLD,WHITE,BLACK),"\\]",
//            getParserPluginName(), " - ", getConnectionDescription(), "$",
//            "\\[",TERM_RESET,"\\]");
    prompt = CONCAT_STRINGS(getParserPluginName(), " - ", getConnectionDescription(), "$");

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
        printf("Please input a SQL command, '\\q' to exit the program, or '\\h' for help\n");
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

	    returnVal = readALine(&sql);

		// deal with failure
		if (returnVal == NULL && ferror(stdin))
		{
		    printf("\n\nError reading from stdin\n");
		    exit(EXIT_FAILURE);
		}

		DEBUG_LOG("process SQL code: ", sql);

		// deal with utility commands
		if(strStartsWith(sql,"\\q"))
		{
			//printf(TB_FG_BG(WHITE,BLACK,"%s"),"\n\nExit GProM.\n");
			printf("\n");
			break;
		}

		if(strStartsWith(sql,"\\h"))
        {
		    printf(TB_FG_BG(WHITE,BLACK,"GPROM CLI COMMANDS") "%s",commandHelp);
		    printf("\n");
		    continue;
        }

        if(strStartsWith(sql,"\\i"))
        {
            printf(TB_FG_BG(WHITE,BLACK,"%s - LANGUAGE OVERVIEW") ":\n%s",
                    getParserPluginName(), getParserLanguageHelp());
            printf("\n");
            continue;
        }

        if(strStartsWith(sql,"\\o"))
        {
            printf(TB_FG_BG(WHITE,BLACK,"PARAMETERS ARE SET AS FOLLOWS") "\n%s",
                    optionsToStringOnePerLine());
            printf("\n");
            continue;
        }


		// process query
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

#ifdef HAVE_READLINE

static char *
readALine(char **str)
{
    char *res;

    /* Get a line from the user. */
    res = readline (prompt);
    DEBUG_LOG(res);
    /* If the line has any text in it, save it to the history. */
    if (res && *res)
      add_history (res);

    *str = res;
    return *str;
}

#else

static char *
readALine(char **str)
{
    printf(TB_FG_BG(WHITE,BLACK,"%s"), prompt);
    printf(" ");
    return gets(*str);
}

#endif
