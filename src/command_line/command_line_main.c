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
#include "execution/exe_run_query.h"
#include "rewriter.h"
#include "utility/string_utils.h"

static const char *commandHelp = "\n"
            "\\q(uit)\t\t\texit CLI\n"
            "\\h(elp)\t\t\tshow this helptext\n"
            "\\i(nput language)\tshow help for currently selected parser\n"
            "\\o(ptions)\t\tshow current parameter settings\n"
            "\\s(et) PAR VAL\t\tset parameter PAR to value VAL\n"
            "\\b(ackend) QUERY\tsend QUERY directly to the backend"
        ;

static char *prompt = "GProM";
static char *userhomedir = NULL;
static char *gpromconf = NULL;
static char *gpromhist = NULL;
static boolean histExists = FALSE;

#define GPROM_ENV_CONFFILE "GPROM_CONF"
#define GPROM_ENV_HISTFILE "GPROM_HIST"

#define IS_UTILITY(cmd) (strStartsWith((cmd), "\\"))
static boolean isInteractiveSession;

static void process(char *sql);
static void inputLoop(void);
static void utility(char *command);
static ExceptionHandler handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s);
static char *readALine(char **str);
static void readHistory(void);
static void persistHistory(void);
static char *createPromptString (void);
static void readConf (void);

int
main(int argc, char* argv[])
{
    char *result;
    boolean optionsInit;

    // initialize basic modules (memory manager, logger, options storage)
    optionsInit = initBasicModules();
    if (optionsInit)
        return EXIT_FAILURE;

    // register exception handler
    registerExceptionCallback(handleCLIException);

    // read configuration and history files
    readConf();

    // read options, determine session type, and setup plugins
    int returnVal = readOptions("gprom", "GProM command line application.", argc, argv);

    isInteractiveSession = !(getStringOption("languagehelp") != NULL
            || getBoolOption("help")
            || getStringOption("input.sql") != NULL
            || getStringOption("input.sqlFile") != NULL);

    setupPluginsFromOptions();

    // create prompt
    createPromptString();

    START_TIMER("TOTAL");

    // read from terminal
    if (returnVal != EXIT_SUCCESS)
    {

    }
    else if (getStringOption("languagehelp") != NULL)
    {
        char *lang = getStringOption("languagehelp");
        printf(TB_FG_BG(WHITE,BLACK,"%s - LANGUAGE OVERVIEW") ":\n%s",
                getParserPluginNameFromString(lang), getParserPluginLanguageHelp(lang));
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
        readHistory();
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
        persistHistory();
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

    // SEND COMMAND TO BACKEND
    if(strStartsWith(command,"\\b"))
    {
        char *cmd = command + 2;
        TRY
        {
            exeRunQuery(cmd);
        }
        END_TRY
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

    // throw error if in non-interactive mode, otherwise try to recover by wiping memcontext
    if (isInteractiveSession)
        return EXCEPTION_WIPE;
    else
        return EXCEPTION_DIE;
}

static void
readConf (void)
{
    TRY
    {
        NEW_AND_ACQUIRE_MEMCONTEXT("READ_CONF");
        //TODO Windows
        // find user home directory
        if ((userhomedir = getenv("HOME")) == NULL) {
#ifdef HAVE_PWD_H
            userhomedir = getpwuid(getuid())->pw_dir;
#endif
        }

        // get gprom configuration file if exists
        if ((gpromconf = getenv(GPROM_ENV_CONFFILE)) == NULL && userhomedir != NULL)
        {
            gpromconf = CONCAT_STRINGS(userhomedir,"/.gprom");
            DEBUG_LOG("config file: %s", gpromconf);
        }
        // if conf file does exist then read configuration
        if(gpromconf != NULL && access(gpromconf, F_OK ) != -1 ) {
            FILE *conff = fopen(gpromconf,"r");

            // read configuration
            if (conff != NULL)
            {
                char buf[1024];
                StringInfo line = makeStringInfo();

                while (fgets(buf, 1024, conff) != NULL)
                {
                    appendStringInfoString(line, buf);
                    if (buf[strlen(buf) - 1] == '\n' || feof(conff))
                    {
                        char *l;
                        l = strtrim(line->data);

                        DEBUG_LOG("line <%s>", l);

                        if ((!isPrefix(l, "#")) && isSubstr(l, "="))
                        {
                            List *keyV = splitString(l, "=");

                            if (LIST_LENGTH(keyV) != 2)
                                THROW(SEVERITY_RECOVERABLE, "expected only one =: %s", l);
                            char *key = getNthOfListP(keyV,0);
                            char *value = getNthOfListP(keyV,1);
                            DEBUG_LOG("read option <%s:%s>", key, value);
                            setOption(key, value);
                        }
                        // ignore comments and lines with only whitespace
                        else if (!isPrefix(l, "#") && !streq(l,""))
                        {
                            THROW(SEVERITY_RECOVERABLE, "error in configuration file %s, cannot parse line:\n <%s>", gpromconf, l);
                        }
                        resetStringInfo(line);
                    }
                }

                if (!feof(conff))
                {
                    THROW(SEVERITY_RECOVERABLE,"error reading configuration file %s: %s", gpromconf, strerror(errno));
                }
            }
            else
            {
                THROW(SEVERITY_RECOVERABLE,"error reading configuration file %s: %s", gpromconf, strerror(errno));
            }
        }
        else
            gpromconf = NULL;

        // find gprom history file if exists
        if ((gpromhist = getenv(GPROM_ENV_HISTFILE)) == NULL && userhomedir != NULL)
        {
            gpromhist = CONCAT_STRINGS(userhomedir,"/.gprom_hist");
        }
        if(gpromhist != NULL) {
            // check if hist file exists
            if (access(gpromhist, F_OK ) != -1)
            {
                histExists = TRUE;
            }
            // if not see whether we can create it
            else
            {
                FILE *hfile = fopen(gpromhist, "ab+");
                if (hfile == NULL)
                {
                    gpromhist = NULL;
                }
                else
                    fclose(hfile);
            }
        }
        FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    }
    ON_EXCEPTION
    {
        ERROR_LOG("error reading from configuration file");
        PROCESS_EXCEPTION_AND_DIE();
    }
    END_ON_EXCEPTION
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
    {
        if (!streq(res, "\\q"))
            add_history (res);
    }
    *str = res;
    return *str;
}

static void
readHistory()
{
    int err;
    TRY
    {
        if (histExists)
        {
            err = read_history(gpromhist);
            if (err != 0)
                FATAL_LOG("error loading history");
        }
    }
    END_TRY
}

static void
persistHistory()
{
    TRY
    {
        if (gpromhist != NULL)
        {
            int err;
            err = write_history(gpromhist);
            if (err != 0)
            {
                FATAL_LOG("error saving history %s", strerror(err));
            }
        }
    }
    END_TRY
}

#else

static char *
readALine(char **str)
{
    printf(TB_FG_BG(WHITE,BLACK,"%s"), prompt);
    printf(" ");
    return gets(*str);
}

static void
readHistory()
{

}

static void
persistHistory()
{

}

#endif
