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
#include "parser/parser.h"
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
static void whatIfResult(List *updates, boolean ds);
static Node *getCond(Node *node);

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
						int depUp = getListLength(updates) - 2;
						ERROR_LOG("Number of dependent statements: %d \n",
								depUp);

						/*
						 DEBUG_LOG("What-if result for all updates:\n");
						 whatIfResult(history, FALSE);

						 DEBUG_LOG(
						 "What-if result for all updates with data slicing:\n");
						 whatIfResult(history, TRUE);

						 if(depUp>0)
						 {
						 DEBUG_LOG(
						 "What-if result for just dependent updates:\n");
						 whatIfResult(updates, FALSE);
						 }
						 */
						if (depUp > 0) {
							DEBUG_LOG(
									"What-if result for just dependent updates with data slicing:\n");
							whatIfResult(updates, TRUE);
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

static Node *
createReenactmentAlgebra(List *updates) {
	ProvenanceStmt *provStat;
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

static void whatIfResult(List *updates, boolean ds) {
	Node *originalUp;
	originalUp = (Node *) popHeadOfListP(updates);
	List *orList = copyList(updates);
	popHeadOfListP(updates);
	updates = appendToHeadOfList(updates, originalUp);
	QueryOperator *reop1 = (QueryOperator *) createReenactmentAlgebra(orList);
	QueryOperator *reop2 = (QueryOperator *) createReenactmentAlgebra(updates);

	QueryOperator *uni;
	QueryOperator *diff1;
	QueryOperator *diff2;
	char *sql;

	if (!ds) {
		diff1 = (QueryOperator *) createSetOperator(SETOP_DIFFERENCE,
				LIST_MAKE(reop1, reop2), NIL,
				NIL);
		diff2 = (QueryOperator *) createSetOperator(SETOP_DIFFERENCE,
				LIST_MAKE(reop2, reop1), NIL,
				NIL);
		uni = (QueryOperator *) createSetOperator(SETOP_UNION,
				LIST_MAKE(diff1, diff2), NIL, NIL);

		addChildOperator(uni, diff1);
		addChildOperator(uni, diff2);
		addParent(diff1, uni);
		addParent(diff2, uni);
		//ERROR_OP_LOG("The result of what-if query is:\n", uni);
		sql = serializeQuery(uni);
		ERROR_LOG("%s", sql);

	} else {
		Node *oUp = (Node *) getHeadOfListP(orList);
		Node *wUp = (Node *) getHeadOfListP(updates);

		Node *cond1, *cond2;
		cond1 = copyObject(getCond(oUp));
		cond2 = copyObject(getCond(wUp));
		SelectionOperator *so1 = NULL, *so2 = NULL;

		if (cond1 != NULL && cond2 != NULL) {
			Node *orCond;
			orCond = (Node *) createOpExpr("OR", LIST_MAKE(cond1, cond2));
			so1 = createSelectionOp(orCond, reop1, NIL, NIL);
			so2 = createSelectionOp(copyObject(orCond), reop2, NIL, NIL);
		}
		if (cond1 != NULL && cond2 == NULL) {
			so1 = createSelectionOp(copyObject(cond1), reop1, NIL, NIL);
			so2 = createSelectionOp(copyObject(cond1), reop2, NIL, NIL);
		}

		if (cond1 == NULL && cond2 != NULL) {
			so1 = createSelectionOp(copyObject(cond2), reop1, NIL, NIL);
			so2 = createSelectionOp(copyObject(cond2), reop2, NIL, NIL);
		}
		addParent(reop1, (QueryOperator *) so1);
		addChildOperator((QueryOperator *) so1, reop1);
		addParent(reop2, (QueryOperator *) so2);
		addChildOperator((QueryOperator *) so2, reop2);

		diff1 = (QueryOperator *) createSetOperator(SETOP_DIFFERENCE,
				LIST_MAKE(so1, so2), NIL,
				NIL);
		diff2 = (QueryOperator *) createSetOperator(SETOP_DIFFERENCE,
				LIST_MAKE(so2, so1), NIL,
				NIL);
		uni = (QueryOperator *) createSetOperator(SETOP_UNION,
				LIST_MAKE(diff1, diff2), NIL, NIL);

		addChildOperator(uni, diff1);
		addChildOperator(uni, diff2);
		addParent(diff1, uni);
		addParent(diff2, uni);
		//ERROR_OP_LOG("The result of what-if query with data sclicing is:\n", uni);
		sql = serializeQuery(uni);
		ERROR_LOG("%s", sql);
	}

	deepFreeList(updates);
}

static Node *getCond(Node *node) {
	if (node->type == T_Update) {
		return ((Update *) node)->cond;
	} else if (node->type == T_Delete) {
		return ((Delete *) node)->cond;
	}
	return NULL;
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
