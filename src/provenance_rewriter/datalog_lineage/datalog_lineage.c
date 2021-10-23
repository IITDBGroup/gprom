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

#include "analysis_and_translate/analyze_dl.h"
#include "common.h"
#include "exception/exception.h"
#include "model/datalog/datalog_model.h"
#include "model/graph/graph.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
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

	/* List *provRules = NIL; */
	DLProgram *rewrP = copyObject(p);
    /* HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES); */

	// get target table if specified
	if(DL_HAS_PROP(p, DL_PROV_LINEAGE_TARGET_TABLE))
	{
		targetTable = DL_GET_STRING_PROP(p, DL_PROV_LINEAGE_TARGET_TABLE);
		answerPred = p->ans;
		provPred = PROV_PRED(targetTable);
		rewrP->ans = provPred;
		DEBUG_LOG("answer predicate: %s, target table for lineage is: %s", answerPred, targetTable);

		// create head predicate to body predicate mapping graph and find predicates that need to be rewritten
		createRelToRelGraph((Node *) p);
		rewrPreds = computePredsToRewrite(targetTable, p);

		FOREACH_SET(char,pred,rewrPreds)
		{

		}
	}
	// no target, just rewrite the whole program
	else
	{
		FOREACH(DLNode,n,p->rules)
		{
			if(isA(n,DLRule))
			{
				rulesToRewrite = appendToTailOfList(rulesToRewrite, n);
			}
		}
	}


	//TODO rewrite rules to propagate provenance for R

	return rewrP;
}

static Set *
computePredsToRewrite(char *targetTable, DLProgram *p)
{
	Constant *target = createConstString(targetTable);

	Graph *bodyRelToHead = invertEdges((Graph *) getDLProp((DLNode *) p, DL_REL_TO_REL_GRAPH));
	Set *reach = reachableFrom(bodyRelToHead, (Node *) target);

	DEBUG_NODE_BEATIFY_LOG("other predicates that need to be rewritten:", reach);

	return makeStringSetFromConstSet(reach);
}

DLRule *
createCaptureRule(DLRule *r, DLAtom *targetAtom)
{
	List *body = copyObject(r->body);

	body = appendToTailOfList(body, copyObject(r->head));

	return createDLRule(copyObject(targetAtom), body);
}

DLRule *
createCaptureRuleForTable(DLRule *r, char *table)
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

	return createCaptureRule(r, target);
}
