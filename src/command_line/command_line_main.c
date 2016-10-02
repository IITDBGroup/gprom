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
#include "utility/string_utils.h"

static const char *commandHelp = "\n"
            "\\q(uit)\t\t\texit CLI\n"
            "\\h(elp)\t\t\tshow this helptext\n"
            "\\i(nput language)\tshow help for currently selected parser\n"
            "\\o(ptions)\t\tshow current parameter settings\n"
            "\\s(et) PAR VAL\t\tset parameter PAR to value VAL\n"
        ;

static char *prompt = "GProM";
#define IS_UTILITY(cmd) (strStartsWith((cmd), "\\"))

static void process(char *sql);
static void inputLoop(void);
static void utility(char *command);
static ExceptionHandler handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s);
static char *readALine(char **str);
static char *createPromptString (void);

int
main(int argc, char* argv[])
{
    char *result;
    boolean optionsInit;

    optionsInit = readOptionsAndIntialize("gprom", "GProM command line application.", argc, argv);
    if (optionsInit)
        return EXIT_FAILURE;

    registerExceptionCallback(handleCLIException);
    createPromptString();

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
        printf("======================================================================\n\n");
        inputLoop();
    }

    STOP_TIMER("TOTAL");
    OUT_TIMERS();

    shutdownApplication();

    return EXIT_SUCCESS;
}

static char *
createPromptString (void)
{
    prompt = CONCAT_STRINGS(getParserPluginName(), " - ", getConnectionDescription(), "$");
    return prompt;
}

static void
inputLoop(void)
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

		DEBUG_LOG("process command:\n<%s>", sql);

		// deal with utility commands
		if (IS_UTILITY(sql))
		{
		    utility(sql);
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
 * Process utility commands
 */
static void
utility(char *command)
{
    // QUIT
    if(strStartsWith(command,"\\q"))
    {
        //printf(TB_FG_BG(WHITE,BLACK,"%s"),"\n\nExit GProM.\n");
        printf("\n");
        FREE(command);
        exit(EXIT_SUCCESS);
    }

    // SHOW CLI HELP
    if(strStartsWith(command,"\\h"))
    {
        printf(TB_FG_BG(WHITE,BLACK,"GPROM CLI COMMANDS") "%s",commandHelp);
        printf("\n");
        return;
    }

    // SHOW CURRENT PARSER LANGUAGE HELP
    if(strStartsWith(command,"\\i"))
    {
        printf(TB_FG_BG(WHITE,BLACK,"%s - LANGUAGE OVERVIEW") ":\n%s",
                getParserPluginName(), getParserLanguageHelp());
        printf("\n");
        return;
    }

    // SHOW PARAMETERS
    if(strStartsWith(command,"\\o"))
    {
        printf(TB_FG_BG(WHITE,BLACK,"PARAMETERS ARE SET AS FOLLOWS") "\n%s",
                optionsToStringOnePerLine());
        printf("\n");
        return;
    }

    // SET PARAMETER
    if(strStartsWith(command,"\\s"))
    {
        List *args = splitString(command, " \t");
        char *par;
        char *val;
        char *previousVal;

        // has 2 args?
        if (LIST_LENGTH(args) != 3)
        {
            printf("\n\\s(et) PAR VAL - requires two parameters, the first PAR is an option name and the second one VAL is a value.\n\n");
            return;
        }

        par = getNthOfListP(args, 1);
        val = getNthOfListP(args, 2);

        // option exists?
        if (!hasOption(par))
        {
            printf("\n<%s> is not a valid option.\n\n", par);
            return;
        }

        // get previous value and set new one
        previousVal = getOptionAsString(par);
        setOption(par, val);

        // let rewriter figure out if anything is triggered by this option change
        reactToOptionsChange(par);
        createPromptString();

        printf("Changed option <%s> from <%s> to <%s>\n\n", par, previousVal, val);
        return;
    }

    printf("Unkown command <%s>\n\n", command);
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
