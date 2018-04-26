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
static void whatIfResultMin(List *updates1, List *updates2);
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

						//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
						//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));

						//ERROR_LOG("Result of CPLEX IS: %d \n", checkCplex((List *) result));

						//List *updates = dependAlgo(result);
						List *updates = SymbolicExeAlgo(result);

						ERROR_LOG("total Number of updates: %d \n",
								getListLength(result) - 1);
						ERROR_LOG("Number of dependent statements: %d \n",
								getListLength(updates) - 2);
						//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(updates));
						//DEBUG_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(updates)));

						Node *originalUp;
						//Compute what-if for dependent updates
						originalUp = (Node *) popHeadOfListP(updates);
						List *orList = copyList(updates);
						popHeadOfListP(updates);
						updates = appendToHeadOfList(updates, originalUp);
						DEBUG_LOG(
								"What-if Result for dependent update using minus:\n");
						whatIfResultMin(orList, updates);
						deepFreeList(updates);
						deepFreeList(orList);

						//Compute what-if for all updates
						originalUp = (Node *) popHeadOfListP(result);
						List *orgList = copyList(result);
						popHeadOfListP(result);
						result = appendToHeadOfList(result, originalUp);
						DEBUG_LOG(
								"What-if Result for dependent update using minus:\n");
						whatIfResultMin(orgList, result);
						deepFreeList(result);
						deepFreeList(orgList);
						/*
						 Node *originalUp;

						 ProvenanceStmt *provStat;
						 char *sql;
						 Node *qoModel;
						 originalUp = (Node *) popHeadOfListP(result);

						 provStat = createProvenanceStmt((Node *) result);
						 provStat->provType = PROV_NONE;
						 provStat->inputType = PROV_INPUT_REENACT;
						 DEBUG_NODE_BEATIFY_LOG("prov:\n", provStat);
						 provStat = (ProvenanceStmt *) analyzeParseModel(
						 (Node *) provStat);
						 DEBUG_NODE_BEATIFY_LOG("analysis:\n", provStat);
						 qoModel = translateParseOracle((Node *) provStat);
						 DEBUG_NODE_BEATIFY_LOG("qo model:\n", provStat);
						 qoModel = provRewriteQBModel(qoModel);
						 DEBUG_NODE_BEATIFY_LOG("after prov rewrite:\n",
						 provStat);
						 //qoModel = translateParse((Node *) provStat);
						 qoModel = optimizeOperatorModel(qoModel);
						 DEBUG_NODE_BEATIFY_LOG("after opt:\n", provStat);
						 sql = serializeOperatorModel(qoModel);
						 ERROR_LOG(
						 "All SERIALIZED SQL Reenactment For Whatif Query:\n%s",
						 sql);

						 //replace new update with original updates and create reenactment again
						 popHeadOfListP(result);
						 result = appendToHeadOfList(result, originalUp);
						 provStat = createProvenanceStmt((Node *) result);
						 provStat->provType = PROV_NONE;
						 provStat->inputType = PROV_INPUT_REENACT;
						 DEBUG_NODE_BEATIFY_LOG("prov:\n", provStat);
						 provStat = (ProvenanceStmt *) analyzeParseModel(
						 (Node *) provStat);
						 DEBUG_NODE_BEATIFY_LOG("analysis:\n", provStat);
						 qoModel = translateParseOracle((Node *) provStat);
						 DEBUG_NODE_BEATIFY_LOG("qo model:\n", provStat);
						 qoModel = provRewriteQBModel(qoModel);
						 DEBUG_NODE_BEATIFY_LOG("after prov rewrite:\n",
						 provStat);
						 qoModel = optimizeOperatorModel(qoModel);
						 DEBUG_NODE_BEATIFY_LOG("after opt:\n", provStat);
						 sql = serializeOperatorModel(qoModel);
						 ERROR_LOG(
						 "All SERIALIZED SQL Reenactment For Original update:\n%s",
						 sql);
						 */
						////////////////////////////
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

static void whatIfResultMin(List *updates1, List *updates2) {
	char *reen1 = createReenactment(updates1);
	int length = strlen(reen1) - 2;
	reen1 = substr(reen1, 0, length);
	char *reen2 = createReenactment(updates2);
	length = strlen(reen2) - 2;
	reen2 = substr(reen2, 0, length);
	ERROR_LOG("(%s\nMINUS \n%s)\nUNION ALL\n(%s\nMINUS \n%s);", reen1,
			reen2, reen2, reen1);
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

