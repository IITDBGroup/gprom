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

static Node *createReenactmentAlgebra(List *updates);
static char *createReenactment(List *updates);
static void whatIfResult(List *updates, boolean ds);
static char* serializeCond(Node *node);

static ExceptionHandler handleCLIException(const char *message,
		const char *file, int line, ExceptionSeverity s);

int main(int argc, char* argv[]) {
	List *history;

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
						history = (List *) parseFromString(
								getStringOption("input.query"));

						ERROR_LOG("Total Number of updates: %d \n",
								getListLength(history) - 1);
						List *updates = SymbolicExeAlgo(history);
						ERROR_LOG("Number of dependent statements: %d \n",
								getListLength(updates) - 2);

						/*
						 DEBUG_LOG("What-if result for all updates:\n");
						 whatIfResult(history, FALSE);

						 DEBUG_LOG(
						 "What-if result for all updates with data slicing:\n");
						 whatIfResult(history, TRUE);

						 DEBUG_LOG(
						 "What-if result for just dependent updates:\n");
						 whatIfResult(updates, FALSE);
						 */
						DEBUG_LOG(
								"What-if result for just dependent updates with data slicing:\n");
						whatIfResult(updates, TRUE);

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

static Node *
createReenactmentAlgebra(List *updates)
{
	Provenancestmt *provStat;
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

	return qoModel;
}

static char *
createReenactment(List *updates)
{
	/* ProvenanceStmt *provStat; */
	/* char *sql = "TRUE"; */
	Node *qoModel;
	/* provStat = createProvenanceStmt((Node *) updates); */
	/* provStat->provType = PROV_NONE; */
	/* provStat->inputType = PROV_INPUT_REENACT; */
	/* DEBUG_NODE_BEATIFY_LOG("prov:\n", provStat); */
	/* provStat = (ProvenanceStmt *) analyzeParseModel((Node *) provStat); */
	/* DEBUG_NODE_BEATIFY_LOG("analysis:\n", provStat); */
	/* qoModel = translateParseOracle((Node *) provStat); */
	/* DEBUG_NODE_BEATIFY_LOG("qo model:\n", provStat); */
	/* qoModel = provRewriteQBModel(qoModel); */
	/* DEBUG_NODE_BEATIFY_LOG("after prov rewrite:\n", provStat); */
	/* qoModel = optimizeOperatorModel(qoModel); */
	/* DEBUG_NODE_BEATIFY_LOG("after opt:\n", provStat); */
	//qoModel = translateParse((Node *) provStat);
	qoModel = createReenactmentAlgebra(updates);
	sql = serializeOperatorModel(qoModel);
	return sql;
}

static void
whatIfResult(List *updates, boolean ds) {
	/* Node *originalUp; */
	/* originalUp = (Node *) popHeadOfListP(updates); */
	/* List *orList = copyList(updates); */
	/* popHeadOfListP(updates); */
	/* updates = appendToHeadOfList(updates, originalUp); */
	/* char *reen1 = createReenactment(orList); */
	/* int length = strlen(reen1) - 2; */
	/* reen1 = substr(reen1, 0, length); */
	/* char *reen2 = createReenactment(updates); */
	/* length = strlen(reen2) - 2; */
	/* reen2 = substr(reen2, 0, length); */
	Node *originalUp;
	originalUp = (Node *) popHeadOfListP(updates);
	List *orList = copyList(updates);
	popHeadOfListP(updates);
	updates = appendToHeadOfList(updates, originalUp);
	QueryOperator *reop1 = createReenactmentAlgebra(orList);
	QueryOperator *reop2 = createReenactmentAlgebra(updates);

	QueryOperator *

	QueryOperator *un;
	QueryOperator *diff1;
	QueryOperator *diff2;

	diff1 = createSetOp(SETOP_DIFFERENCE, LIST_MAKE(reop1, reop2), NIL, NIL);
	diff2 = createSetOp(SETOP_DIFFERENCE, LIST_MAKE(reop2, reop1), NIL, NIL);
	un = createSetOp(SETOP_UNION, LIST_MAKE(diff1, diff2), NIL, NIL);

	//TODO backconnect to parents

    ERROR_OP_LOG("diff query", un);

	if (!ds)
		ERROR_LOG("(%s\nMINUS \n%s)\nUNION ALL\n(%s\nMINUS \n%s);", reen1,
				reen2, reen2, reen1);
	else {
		Node *orUp = (Node *) getHeadOfListP(orList);
		Node *wifUp = (Node *) getHeadOfListP(updates);
		char *c1 = serializeCond(orUp);
		char *c2 = serializeCond(wifUp);
		ERROR_LOG(
				"(%s WHERE (%s) OR (%s)\nMINUS \n%s WHERE (%s) OR (%s))\nUNION ALL\n(%s WHERE (%s) OR (%s)\nMINUS \n%s WHERE (%s) OR (%s));",
				reen1, c1, c2, reen2, c1, c2, reen2, c1, c2, reen1, c1, c2);

	}
	deepFreeList(updates);
}

static char* serializeCond(Node *node) {
	char* condition = "TRUE";
		if (node->type == T_Update) {
			condition = serializeOperatorModel(((Update *) node)->cond);
		} else if (node->type == T_Delete) {
			condition = serializeOperatorModel(((Delete *) node)->cond);
		}
		return condition;
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
