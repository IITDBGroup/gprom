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
#include "configuration/option_parser.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/expression/expression.h"

#include "rewriter.h"

#include "metadata_lookup/metadata_lookup.h"

static void init();
static void cleanup();
static char *process(char *sql);


int
main(int argc, char* argv[]) {
	mallocOptions();

	int rc=parseOption(argc,argv);

	if(rc<0)
	{
		printError();
		return EXIT_FAILURE;
	}
	else if(rc>0)
	{
		printHelp();
		return EXIT_FAILURE;
	}
	else
	{
		printSuccess();
		init();
		inputSQL();
		cleanup();
	}

	freeOptions();
	return EXIT_SUCCESS;
}

void
printError()
{
	printf("Parameters Errors\n");
}

void
printHelp()
{
	printf("================================HELP===============================\n");
	printf("-host, host address of the machine where oracle server runs on\n");
	printf("-db, database name of oracle server\n");
	printf("-port, port number of oracle server\n");
	printf("-user, username of account\n");
	printf("-passwd, password of account\n");
	printf("-log, set to use log output, default is off\n");
	printf("-loglevel, level of log for output, default is 0\n");
	printf("-debugmemory, set to use debug implementation of memory management, default is off\n");
	printf("-activate, set to activate certain types of alternative rewrites\n");
	printf("-sql, SQL you want to input\n");
	printf("===================================================================\n");
}


void
printSuccess()
{
	Options* options=getOptions();
	printf("=====================Options Parsed successfully===================\n");
	printf("host:%s\n",options->optionConnection->host);
	printf("db:%s\n",options->optionConnection->db);
	printf("port:%d\n",options->optionConnection->port);
	printf("user:%s\n",options->optionConnection->user);
	printf("passwd:%s\n",options->optionConnection->passwd);
	printf("log:%d\n",options->optionDebug->log);
	printf("loglevel:%d\n",options->optionDebug->loglevel);
	printf("debugmemory:%d\n",options->optionDebug->debugMemory);
	FOREACH(KeyValue,o,options->optionRewrite)
	{
	    printf("rewrite option %s set to %s\n",exprToSQL(o->key), exprToSQL(o->value));
	}
	printf("SQL:%s\n",options->optionConnection->sql);
	printf("===================================================================\n");
}

static void
init()
{
    initMemManager();
    initMetadataLookupPlugin();
}

static void
cleanup()
{
    destroyMemManager();
}

void
inputSQL()
{
	char* sql=(char*) CALLOC(999,1);
	while(TRUE)
	{
	    char *rewritten;
	    int returnVal;

		printf("Please input a SQL or 'q' to exit the program\n");
		returnVal = scanf("%s",sql);
		if(*sql=='q')
		{
			printf("Client Exit.\n");
			break;
		}

		NEW_AND_ACQUIRE_MEMCONTEXT("PROCESS_CONTEXT");
		rewritten = process(sql);
		FREE_AND_RELEASE_CUR_MEM_CONTEXT();

		printf("Rewrite SQL is:%s\n",rewritten);
	}
	FREE(sql);
}

/*
 * Parse -> Translate -> Provenance Rewrite -> Serialize
 */
static char *
process(char *sql)
{
    return rewriteQuery(sql);
}
