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


#include "common.h"
#include "log/termcolor.h"
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
#include "common.h"
#include "parser/parser.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "sql_serializer/sql_serializer.h"
#include "analysis_and_translate/translator_oracle.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "operator_optimizer/operator_optimizer.h"
#include "exception/exception.h"


static ExceptionHandler handleCLIException (const char *message, const char *file, int line, ExceptionSeverity s);

int main(int argc, char* argv[]) {
	Node *result;

// initialize components
	READ_OPTIONS_AND_INIT("testexprcplex", "Run expr to cplex.");
	// register exception handler
	 registerExceptionCallback(handleCLIException);

	// read from terminal
	if (getStringOption("input.query") == NULL) {
		FATAL_LOG("give -query");
	}
	// parse input string
	else {
	    TRY
	    {
		result = parseFromString(getStringOption("input.query"));

		//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
		DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));

		//ERROR_LOG("Result of CPLEX IS: %d \n", checkCplex((List *) result));

		List *updates = dependAlgo((List *) result);

		ERROR_LOG("Number of dependent statements: %d \n", updates->length - 2);
		//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(updates));
		//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(updates)));

		Node *originalUp;
		originalUp = popHeadOfListP(updates);

		ProvenanceStmt *provStat;
		char *sql;
		Node *qoModel;
		provStat = createProvenanceStmt((Node *) updates);
		provStat->provType = PROV_NONE;
		provStat->inputType = PROV_INPUT_REENACT;
		DEBUG_NODE_BEATIFY_LOG("prov:\n", provStat);
		provStat = (ProvenanceStmt *) analyzeParseModel((Node *) provStat);
		DEBUG_NODE_BEATIFY_LOG("analysis:\n", provStat);
		qoModel = translateParseOracle((Node *) provStat);
		DEBUG_NODE_BEATIFY_LOG("qo model:\n", provStat);
		qoModel = provRewriteQBModel(qoModel);
		DEBUG_NODE_BEATIFY_LOG("after prov rewrite:\n", provStat);
		qoModel = optimizeOperatorModel(qoModel);
		DEBUG_NODE_BEATIFY_LOG("after opt:\n", provStat);
		//qoModel = translateParse((Node *) provStat);
		sql = serializeOperatorModel(qoModel);
		ERROR_LOG("SERIALIZED SQL Reenactment For Whatif Query:\n%s", sql);

		//replace new update with original updates and create reenactment again
		popHeadOfListP(updates);
		updates = appendToHeadOfList(updates, originalUp);
		provStat = createProvenanceStmt((Node *) updates);
		provStat->provType = PROV_NONE;
		provStat->inputType = PROV_INPUT_REENACT;
		DEBUG_NODE_BEATIFY_LOG("prov:\n", provStat);
		provStat = (ProvenanceStmt *) analyzeParseModel((Node *) provStat);
		DEBUG_NODE_BEATIFY_LOG("analysis:\n", provStat);
		qoModel = translateParseOracle((Node *) provStat);
		DEBUG_NODE_BEATIFY_LOG("qo model:\n", provStat);
		qoModel = provRewriteQBModel(qoModel);
		DEBUG_NODE_BEATIFY_LOG("after prov rewrite:\n", provStat);
		sql = serializeOperatorModel(qoModel);
		ERROR_LOG("SERIALIZED SQL Reenactment For Original update:\n%s", sql);
	    }
	    ON_EXCEPTION
	    {
	        // if an exception is thrown then the query memory context has been
	        // destroyed and we can directly create an empty string in the callers
	        // context
	        DEBUG_LOG("allocated in memory context: %s", getCurMemContext()->contextName);
	    }
	    END_ON_EXCEPTION
	}

	shutdownApplication();

	return EXIT_SUCCESS;
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
    return EXCEPTION_DIE;
}
