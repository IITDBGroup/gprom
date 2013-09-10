/*
 ============================================================================
 Name        : CommandLine.c
 Author      : Zephyr
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "CommandLine/CommandLine.h"
#include "Configuration/OptionParser.h"

int main(int argc, char* argv[]) {
	mallocOptions();
	int rc=parseOption(argc,argv);

	if(rc<0)
		printError();
	else if(rc>0)
		printHelp();
	else
		printSuccess();

	freeOptions();
	return EXIT_SUCCESS;
}

void printError()
{
	printf("Parameters Errors\n");
}

void printHelp()
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
	printf("-alternative, set to activate certain types of alternative rewrites, default is off\n");
}


void printSuccess()
{
	Options* options=getOptions();
	printf("Options Parsed successfully\n");
	printf("host:%s\n",options->optionConnection->host);
	printf("db:%s\n",options->optionConnection->db);
	printf("port:%d\n",options->optionConnection->port);
	printf("user:%s\n",options->optionConnection->user);
	printf("passwd:%s\n",options->optionConnection->passwd);
	printf("log:%d\n",options->optionDebug->log);
	printf("loglevel:%d\n",options->optionDebug->loglevel);
	printf("debugmemory:%d\n",options->optionDebug->debugMemory);
	printf("alternative:%d\n",options->optionRewrite->alternative);
}

