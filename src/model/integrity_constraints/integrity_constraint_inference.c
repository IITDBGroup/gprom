/*
 *------------------------------------------------------------------------------
 *
 * integrity_constraint_inference.c - Inferring interity constraints for DL queries
 *
 *     Methods that infer integrity constraints for Datalog rules based on base table ICs.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2022-02-18
 *        SUBDIR: src/model/integrity_constraints/
 *
 *-----------------------------------------------------------------------------
 */


#include "analysis_and_translate/analyze_dl.h"
#include "model/datalog/datalog_model.h"
#include "model/list/list.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"

List *
inferFDsForProgram(DLProgram *progam)
{
	return NIL;
}


List *
inferFDsForRule(DLRule *r, List *fds)
{
	List *adaptedFDs = adaptFDsToRules(r, fds);
	List *result = NIL;
	
	// filter adapted FDs that will hold on the rule head.
	// deal with aggregation rules
	if(hasAggFunction((Node *) r))
	{
		
	}
	else
	{
		FOREACH(FD,f,adaptedFDs)
		{
			
		}
	}

	// replace variable names with head attribute names

	return result;
}

