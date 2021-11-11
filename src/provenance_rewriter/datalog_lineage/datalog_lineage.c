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
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"

#define PROV_PRED(_p) CONCAT_STRINGS("PROV_", _p);

static Set *computePredsToRewrite(char *targetTable, DLProgram *p);

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

	// if requested, first merge rules
	if(getBoolOption(OPTION_DL_MERGE_RULES))
	{
		p = mergeSubqueries(p, TRUE);
	}

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

		FOREACH_SET(char,pred,rewrPreds)
		{
			List *rs = (List *) MAP_GET_STRING(predToRules, pred);

			DEBUG_LOG("handle predicate %s", pred);
			DEBUG_DL_LOG("rules for predicate are", rs);

			FOREACH(DLRule,r,rs)
			{
				char *filter = filterPred && streq(getHeadPredName(r), answerPred) ? filterPred : NULL;
				provRules = appendToTailOfList(
					provRules,
					createCaptureRuleForTable(r, pred, filter));
			}
		}

		if(getBoolOption(OPTION_DL_SEMANTIC_OPT))
		{
			/* List *fds = (List *) DL_GET_PROP(p, DL_PROG_FDS); */

		}
	}
	// no target, just rewrite the whole program
	else
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
	DEBUG_DL_LOG("rewritten program for lineage:\n\n", rewrP);

	return rewrP;
}

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

DLRule *
createCaptureRule(DLRule *r, DLAtom *targetAtom, char *filterAnswerPred)
{
	List *body = copyObject(r->body);
	DLAtom *newHead = copyObject(targetAtom);
	DLRule *result;

	newHead->rel = PROV_PRED(newHead->rel);

	if(filterAnswerPred)
	{
		DLAtom *filterQ = copyObject(r->head);

		filterQ->rel = filterAnswerPred;
		body = appendToTailOfList(body, filterQ);
	}
	else
	{
		body = appendToTailOfList(body, copyObject(r->head));
	}

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
