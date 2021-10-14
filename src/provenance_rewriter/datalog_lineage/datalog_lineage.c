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
#include "exception/exception.h"
#include "model/datalog/datalog_model.h"
#include "model/node/nodetype.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"


DLProgram *
rewriteDLForLinageCapture(DLProgram *p)
{
	ASSERT(IS_LINEAGE_PROV(p));
	char *targetTable;
    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);

	// get target table if specified
	if(DL_HAS_PROP(p, DL_PROV_LINEAGE_TARGET_TABLE))
	{
		targetTable = DL_GET_STRING_PROP(p, DL_PROV_LINEAGE_TARGET_TABLE);
	}

	//TODO rewrite rules to propagate provenance for R



	return p;
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
