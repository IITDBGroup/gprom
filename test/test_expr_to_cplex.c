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
#include "exception/exception.h"
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
#include "utility/string_utils.h"

static char *createReenactment(List *updates);
static void createWhatIfMin(List *updates);
static ExceptionHandler handleCLIException(const char *message,
		const char *file, int line, ExceptionSeverity s);

int main(int argc, char* argv[]) {
	List *result;

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
						result = (List *) parseFromString(
								getStringOption("input.query"));

						//ERROR_LOG("Result of CPLEX IS: %d \n", checkCplex((List *) result));

						//List *updates = dependAlgo(result);
						List *updates = SymbolicExeAlgo(result);
						int depcount = getListLength(updates) - 2;

						ERROR_LOG("total Number of updates: %d \n",
								getListLength(result) - 1);
						ERROR_LOG("Number of dependent statements: %d \n",
								depcount);

						if (depcount > 0) {
							//Compute what-if for dependent updates
							createWhatIfMin(updates);
							/*
							 //Compute what-if for all updates
							 createWhatIfMin(result);
							 */
						}
					}ON_EXCEPTION
					{
						// if an exception is thrown then the query memory context has been
						// destroyed and we can directly create an empty string in the callers
						// context
						DEBUG_LOG("allocated in memory context: %s",
								getCurMemContext()->contextName);
					}
					END_ON_EXCEPTION

	}

	shutdownApplication();

	return EXIT_SUCCESS;
}

static char *createReenactment(List *updates) {
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
	return sql;
}

static void createWhatIfMin(List *updates) {
	Node *originalUp;
	originalUp = (Node *) popHeadOfListP(updates);
	List *orList = copyList(updates);
	popHeadOfListP(updates);
	updates = appendToHeadOfList(updates, originalUp);
	ERROR_LOG("What-if Result using minus for %d updates :\n",
			getListLength(updates));
	char *reen1 = createReenactment(orList);
	int length = strlen(reen1) - 2;
	reen1 = substr(reen1, 0, length);
	char *reen2 = createReenactment(updates);
	length = strlen(reen2) - 2;
	reen2 = substr(reen2, 0, length);
	ERROR_LOG("(%s\nMINUS \n%s)\nUNION ALL\n(%s\nMINUS \n%s);", reen1, reen2,
			reen2, reen1);
	deepFreeList(updates);
	deepFreeList(orList);
}
/*
 * Function that handles exceptions
 */
static ExceptionHandler handleCLIException(const char *message,
		const char *file, int line, ExceptionSeverity s) {
	if (streq(file, "sql_parser.l")) {
		printf(TCOL(RED, "PARSE ERORR:") " %s", message);
	} else
		printf(TCOL(RED,"(%s:%u) ") "\n%s\n", file, line, message);

	// throw error if in non-interactive mode, otherwise try to recover by wiping memcontext
	return EXCEPTION_DIE;
}

