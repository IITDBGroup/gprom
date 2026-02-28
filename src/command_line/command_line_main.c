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
#include "../../include/parser/parse_internal_oracle.h"
#include "parser/parser.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "execution/executor.h"
#include "execution/exe_run_query.h"
#include "rewriter.h"
#include "utility/string_utils.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "metadata_lookup/metadata_lookup.h"

#define PROCESS_CONTEXT_NAME "PROCESS_CONTEXT"
static const char *commandHelp = "\n"
            "\\q(uit)\t\t\texit CLI\n"
            "\\h(elp)\t\t\tshow this helptext\n"
            "\\I(nput language)\tshow help for currently selected parser\n"
            "\\o(ptions)\t\tshow current parameter settings\n"
            "\\s(et) PAR VAL\t\tset parameter PAR to value VAL\n"
            "\\b(ackend) QUERY\tsend QUERY directly to the backend\n"
	        "\\i(nput file) FILE\tread & execute commands from FILE\n"
            "\\m(ultiline)\t\t\ttoggle multiline editing mode, if on, then semicolon ends a query"
        ;

static char *prompt = "GProM";
static char *userhomedir = NULL;
static char *gpromconf = NULL;
static char *gpromhist = NULL;
static boolean histExists = FALSE;
static boolean multiLine = FALSE;
static boolean multiLineDone = FALSE;

#define GPROM_ENV_CONFFILE "GPROM_CONF"
#define GPROM_ENV_HISTFILE "GPROM_HIST"
#define BUFFER_SIZE 10000
#define IS_UTILITY(cmd) (strStartsWith((cmd), "\\"))

static boolean isInteractiveSession;

static void process(char *sql, FILE *stream);
static void processString(char *sql);
static void processQueryFromFile(char *fName);
static void inputLoop(void);
static void utility(char *command);
static ExceptionHandler handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s);
static char *readALine(char **str);
static char *readMultiLine (char **str);
static boolean statementFinished(char *stmt);
static void addToHistory(char *res);
static void readHistory(void);
static void persistHistory(void);
static char *createPromptString (void);
static void readConf (void);

#define HELLO_MESSAGE "Welcome to the GProM " PACKAGE_VERSION " command line interface"

int
main(int argc, char* argv[])
{
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
    int returnVal = readOptions("gprom", HELLO_MESSAGE, argc, argv);

    isInteractiveSession = !(getStringOption(OPTION_SHOW_LANGUAGE_HELP) != NULL
            || getBoolOption(OPTION_SHOW_HELP)
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
    else if (getStringOption(OPTION_SHOW_LANGUAGE_HELP) != NULL)
    {
        char *lang = getStringOption(OPTION_SHOW_LANGUAGE_HELP);
        printf(TB_FG_BG(WHITE,BLACK,"%s - LANGUAGE OVERVIEW") ":\n%s",
                getParserPluginNameFromString(lang), getParserPluginLanguageHelp(lang));
    }
    else if (getStringOption("input.sql") != NULL)
    {
        processString(getStringOption("input.sql"));
    }
    else if (getStringOption("input.sqlFile") != NULL)
    {
        char *fName = getStringOption("input.sqlFile");
		processQueryFromFile(fName);
    }
    // parse input string
    else
    {
        readHistory();
        printf(HELLO_MESSAGE "\n");
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
    char* sql=(char*) CALLOC(BUFFER_SIZE,1);
    while(TRUE)
    {
        char *returnVal;

        if (multiLine)
            returnVal = readMultiLine(&sql);
        else
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
        processString(sql);
    }
    FREE(sql);
}

/*
 * Parse -> Translate -> Provenance Rewrite -> Serialize -> Execute
 */
static void
process(char *sql, FILE *stream)
{
    // process query
    TRY
    {
        NEW_AND_ACQUIRE_MEMCONTEXT(PROCESS_CONTEXT_NAME);
        processInput(sql, stream);
    }
    ON_EXCEPTION
    {
        printf("\nError occured\n");
        fflush(stdout);
    }
    END_ON_EXCEPTION
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
}

static void
processString(char *sql)
{
	process(sql, NULL);
}

/*
 * process for queries stored in a file
 */
static void
processQueryFromFile(char *fName)
{
	FILE *file = fopen(fName, "r");

	if (file == NULL)
		FATAL_LOG("could not open file %s with error %s", fName, strerror(errno));

	process(NULL, file);

	fclose(file);
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
    	//if in self-turning model, store the cached provenance sketches into table
    	if(getStringOption(OPTION_PS_STORE_TABLE) != NULL)
    		storePS();

        //printf(TB_FG_BG(WHITE,BLACK,"%s"),"\n\nExit GProM.\n");
        printf("\n");
        persistHistory();
        FREE(command);
        exit(EXIT_SUCCESS);
    }

    // SHOW CLI HELP
    if(strStartsWith(command,"\\h") || strStartsWith(command,"\\?"))
    {
        printf(TB_FG_BG(WHITE,BLACK,"GPROM CLI COMMANDS") "%s",commandHelp);
        printf("\n");
        return;
    }

	// EXECUTE COMMANDS FROM FILE
    if(strStartsWith(command,"\\i"))
    {
        char *fName;

		if(strlen(command) <= 3)
		{
			printf("need to provide a filename to \\I\n");
			return;
		}

		fName = substr(command, 3, strlen(command) - 1);

		processQueryFromFile(fName);
        return;
    }

    // SHOW CURRENT PARSER LANGUAGE HELP
    if(strStartsWith(command,"\\I"))
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

    // TOGGLE MULTILINE EDITING
    if (strStartsWith(command, "\\m"))
    {
        multiLine = ! multiLine;
        if (multiLine)
        {
            printf("Activated multi line commands (use ; to end command)\n");
        }
        else
            printf("Activated  single line commands\n");

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
    fflush(stdout);

    // throw error if in non-interactive mode, otherwise try to recover by wiping memcontext
    if (isInteractiveSession)
        return EXCEPTION_ABORT;
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



/********************************************************************************/
#ifdef HAVE_READLINE

static void
addToHistory(char *res)
{
    /* If the line has any text in it, save it to the history. */
    if (res && *res)
    {
        if (!streq(res, "\\q"))
            add_history (res);
    }
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
            {
                ERROR_LOG("error loading history, errno = %d", err);
            }
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

static char *
readALine(char **str)
{
    char *res;
    StringInfo promptstr = makeStringInfo();

    appendStringInfo(promptstr, TB_FG_BG(WHITE,BLACK,"%s") " ", prompt);

    /* Get a line from the user. */
    res = readline(promptstr->data);
    TRACE_LOG(res);
    /* If the line has any text in it, save it to the history. */
    addToHistory(res);
    *str = res;

    return *str;
}

static char *
readMultiLine (char **str)
{
    multiLineDone = FALSE;
    StringInfo result = makeStringInfo();
    char *line;
    int lineNum = 0;

    // online show prompt for first line
    while(!multiLineDone)
    {
        if (lineNum == 0)
            line = readline(prompt);
        else
            line = readline("> ");
        appendStringInfoString(result, line);

        if (statementFinished(line) || IS_UTILITY(line))
            multiLineDone = TRUE;

        if (!multiLineDone)
            appendStringInfoString(result, "\n");

        lineNum++;
    }

    // add to history if the line is not empty
    addToHistory(result->data);
    *str = result->data;
    return *str;
}

static boolean
statementFinished(char *stmt)
{
    char *trimLine = strtrim(stmt);
    char lastChar = trimLine[strlen(trimLine) - 1];
    TRACE_LOG("line <%s> , trimmed <%s>, last char <%c>", stmt, trimLine, lastChar);
    if (lastChar == ';') //TODO accept whitepsace
        return TRUE;
    return FALSE;
}

#else

static char *readLine(char *buffer, size_t buflen, FILE *fp);

static char *
readALine(char **str)
{
    printf(TB_FG_BG(WHITE,BLACK,"%s"), prompt);
    printf(" ");
    addToHistory(NULL);
    return readLine(*str, BUFFER_SIZE, stdin);
}

static char *
readLine(char *buffer, size_t buflen, FILE *fp)
{
    if (fgets(buffer, buflen, fp) != 0)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
            buffer[len-1] = '\0';
        return buffer;
    }
    return 0;
}

static char *
readMultiLine (char **str)
{
    if (statementFinished(NULL))
        return readALine(str);
    multiLineDone = TRUE;
    return NULL;
}

static void
addToHistory(char *res)
{

}

static void
readHistory()
{

}

static boolean
statementFinished(char *stmt)
{
    return TRUE;
}

static void
persistHistory()
{

}

#endif
