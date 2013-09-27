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

#include <stdio.h>
#include <stdlib.h>
#include "command_line/command_line_main.h"
#include "configuration/option_parser.h"

int
main(int argc, char* argv[]) {
	mallocOptions();

	int rc=parseOption(argc,argv);

	if(rc<0)
		printError();
	else if(rc>0)
		printHelp();
	else
	{
		printSuccess();
		inputSQL();
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
	int i,size=options->optionRewrite->size;
	for(i=0;i<size;i++)
		printf("activated rewrite:%s\n",options->optionRewrite->rewriteMethods[i]->name);
	printf("===================================================================\n");
}

void
inputSQL()
{
	char* sql=(char*)malloc(sizeof(char)*999);
	while(TRUE)
	{
		printf("Please input a SQL or 'q' to exit the program\n");
		scanf("%s",sql);
		if(*sql=='q')
		{
			printf("Client Exit.\n");
			break;
		}
		printf("Rewrite SQL is:%s\n",sql);
	}
	free(sql);
}
