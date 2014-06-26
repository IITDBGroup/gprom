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

static char *process(char *sql);


int
main(int argc, char* argv[]) {
    readOptionsAndIntialize("commandline", "GProM default command line that outputs rewritten input query to stdout.", argc, argv);
	inputSQL();
	shutdownApplication();

	return EXIT_SUCCESS;
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
