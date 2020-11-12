/*-----------------------------------------------------------------------------
 *
 * test_parser.c
 *
 *
 *      AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
//#include "parser/parse_internal.h"
#include "parser/parser.h"
//#include "metadata_lookup/metadata_lookup.h"
//#include "../src/parser/sql_parser.tab.h"
#include "rewriter.h"
#include "symbolic_eval/expr_to_constraint.h"
#include "symbolic_eval/whatif_algo.h"

int main(int argc, char* argv[]) {
	Node *result;
//    int retVal;

// initialize components
	READ_OPTIONS_AND_INIT("testexprcplex", "Run expr to cplex.");

	// read from terminal
	if (getStringOption("input.query") == NULL) {
		FATAL_LOG("give -query");
	}
	// parse input string
	else {
		result = parseFromString(getStringOption("input.query"));

		DEBUG_LOG("Address of returned node is <%p>", result);
		ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
		ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s",
				beatify(nodeToString(result)));

		List *updates = dependAlgo((List *) result);

		DEBUG_LOG("Address of returned node is <%p>", updates);
		ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(updates));
		ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s",
				beatify(nodeToString(updates)));

	}

	shutdownApplication();

	return EXIT_SUCCESS;
}
