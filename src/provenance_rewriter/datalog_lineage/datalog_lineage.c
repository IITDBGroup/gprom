/*
 *------------------------------------------------------------------------------
 *
 * datalog_lineage.c - Rewrites to capture lineage for a Datalog program.
 *
 *     Given a relation of interest and Datalog program, these rewrites
 *     instrument the program to capture lineage for this query wrt. this
 *     relation.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-09
 *        SUBDIR: src/provenance_rewriter/datalog_lineage/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "analysis_and_translate/analyze_dl.h"
#include "configuration/option.h"
#include "exception/exception.h"
#include "log/logger.h"
#include "model/datalog/datalog_model.h"
#include "model/expression/expression.h"
#include "model/graph/graph.h"
#include "model/integrity_constraints/integrity_constraint_inference.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"

#define PROV_PRED(_p) CONCAT_STRINGS(backendifyIdentifier("prov_"), _p);

static Set *computePredsToRewrite(char *targetTable, DLProgram *p);

/**
 * @brief Rewrites a program to compute lineage.
 *
 * This rewrites the rules based on the approach from Widom et al. Optionally
 * the user can specify for which input relation provenance should be tracked.
 *
 * @param p the program to be rewritten
 * @return the rewritten program
 */

DLProgram *
rewriteDLForLinageCapture(DLProgram *p)
{
	ASSERT(IS_LINEAGE_PROV(p));
	char *targetTable;
	char *provPred;
	char *answerPred;
    Set *rewrPreds;
	List *rulesToRewrite = NIL;
	char *filterPred = DL_GET_STRING_PROP_DEFAULT(p, DL_PROV_LINEAGE_RESULT_FILTER_TABLE, NULL);
	DLProgram *rewrP = copyObject(p);
	List *provRules = NIL;

	DL_DEL_PROP(rewrP, DL_PROV_LINEAGE);

	ENSURE_BODY_PRED_TO_RULE_MAP(rewrP);
    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) rewrP, DL_MAP_BODYPRED_TO_RULES);

	DEBUG_NODE_BEATIFY_LOG("body predicate to rule map:\n", predToRules);

	// get target table if specified
	if(DL_HAS_PROP(p, DL_PROV_LINEAGE_TARGET_TABLE))
	{
		targetTable = DL_GET_STRING_PROP(p, DL_PROV_LINEAGE_TARGET_TABLE);
		answerPred = p->ans;
		provPred = PROV_PRED(targetTable);
		rewrP->ans = provPred;
		DEBUG_LOG("answer predicate: %s, target table for lineage is: %s, filter predicate: %s", answerPred, targetTable, filterPred ? filterPred : "");

		// create head predicate to body predicate mapping graph and find predicates that need to be rewritten
		createRelToRelGraph((Node *) p);
		rewrPreds = computePredsToRewrite(targetTable, p);

		//TODO filter based on answer predicate to avoid generating unncessary rules

		// infer FDs for idb predicates if semantic optimization is on
		if(getBoolOption(OPTION_DL_SEMANTIC_OPT))
		{
			List *fds = inferFDsForProgram(p);
			DL_SET_PROP(p, DL_PROG_FDS, fds);
		}

		FOREACH_SET(char,pred,rewrPreds)
		{
			List *rs = (List *) MAP_GET_STRING(predToRules, pred);

			DEBUG_LOG("handle predicate %s", pred);
			DEBUG_DL_LOG("rules for predicate are", rs);

			FOREACH(DLRule,r,rs)
			{
				char *filter = filterPred && streq(getHeadPredName(r), answerPred) ? filterPred : NULL;
				DLRule *captureRule;

				if(getBoolOption(OPTION_DL_SEMANTIC_OPT))
				{
					List *fds = (List *) DL_GET_PROP(p, DL_PROG_FDS);

					captureRule = optimizeDLRule(p, r, fds, pred, filter);
				}
				else
				{
					captureRule = createCaptureRuleForTable(r, pred, filter);
				}

				provRules = appendToTailOfList(
					provRules,
					captureRule);
			}
		}

	}
	// no target, just rewrite the whole program
	else //TODO not tested, probably needs to be revised
	{
		DEBUG_LOG("compute lineage for all edb predicates for answer predicate");

		FOREACH(DLNode,n,p->rules)
		{
			if(isA(n,DLRule))
			{
				rulesToRewrite = appendToTailOfList(rulesToRewrite, n);
			}
		}
	}

	rewrP->rules = CONCAT_LISTS(rewrP->rules, provRules);
	//TODO rewrite rules to propagate provenance for R
	// remove properties to allow for reanalysis
	delAllProps((DLNode *) rewrP);
    INFO_DL_LOG("rewritten program for lineage:\n\n", rewrP);

	return rewrP;
}

/**
 * @brief Determines for which predicates we have to rewrite rules to capture
 * provenance for the program wrt. to an EDB relation.
 *
 * @param targetTable The EDB relation for which we are capturing provenance
 * @param p The program for which we are capturing provenance
 * @return A set of Constant nodes storing the names of predicates whose rules need to be rewritten.
 */

static Set *
computePredsToRewrite(char *targetTable, DLProgram *p)
{
	Constant *target = createConstString(targetTable);

	Graph *bodyRelToHead = invertEdges((Graph *) getDLProp((DLNode *) p, DL_REL_TO_REL_GRAPH));
	Set *reach = reachableFrom(bodyRelToHead, (Node *) target);

	addToSet(reach, createConstString(targetTable));

	DEBUG_NODE_BEATIFY_LOG("predicates that are targets of rewritte:", reach);

	return makeStringSetFromConstSet(reach);
}



//TODO this does not support arithmetic expressions
DLRule *
createCaptureRule(DLRule *r, DLAtom *targetAtom, char *filterAnswerPred)
{
	List *body = copyObject(r->body);
	DLAtom *newHead = copyObject(targetAtom);
	DLRule *result;
	DLAtom *headOrFilterAtom;

	newHead->rel = PROV_PRED(newHead->rel);

	headOrFilterAtom = copyObject(r->head);

	if(filterAnswerPred)
	{
		headOrFilterAtom->rel = filterAnswerPred;
	}

	if(hasAggFunction((Node *) r->head->args))
	{
		List *newArgs = NIL;

		FOREACH(Node,arg,r->head->args)
		{
			if(isA(arg,DLVar))
			{
				newArgs = appendToTailOfList(newArgs, arg);
			}
			else //create fresh var for aggregation functions
			{
				DLVar *v = createUniqueVar((Node *) LIST_MAKE(newArgs,headOrFilterAtom), typeOf(arg));
				newArgs = appendToTailOfList(newArgs,v);
			}
		}

		headOrFilterAtom->args = newArgs;
	}

	body = appendToTailOfList(body, headOrFilterAtom);

	result = createDLRule(newHead, body);

	DEBUG_DL_LOG("Created capture rule: ", result);

	return result;
}

DLRule *
createCaptureRuleForTable(DLRule *r, char *table, char *filterAnswerPred)
{
	DLAtom *target = NULL;

	// determine target goal
	FOREACH(DLAtom,a,r->body)
	{
		if(streq(a->rel, table)) //TODO support multiple goals for self-joins?n
		{
			target = a;
		}
	}

	if(target == NULL)
	{
		THROW(SEVERITY_RECOVERABLE, "There is no atom for target table %s in rule %s",
			  table,
			  beatify(nodeToString(r)));
	}

	return createCaptureRule(r, target, filterAnswerPred);
}
