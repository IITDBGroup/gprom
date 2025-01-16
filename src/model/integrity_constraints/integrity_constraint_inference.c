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


#include "mem_manager/mem_mgr.h"
#include "analysis_and_translate/analyze_dl.h"
#include "model/datalog/datalog_model.h"
#include "model/graph/graph.h"
#include "model/integrity_constraints/integrity_constraints.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/datalog/datalog_model_checker.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"
#include "model/integrity_constraints/integrity_constraint_inference.h"

static boolean allBodyAtomsDone(DLRule *r, Set *doneAtoms, Graph *g);

List *
inferFDsForProgram(DLProgram *program)
{
	Set *doneAtoms;
	List *newFDs = copyObject((List *) DL_GET_PROP(program, DL_PROG_FDS));
	Set *todoRules = NODESET();
	HashMap *predToRule;
	HashMap *bodyPredToRule;
	Graph *relG;

	// check the DL model
	checkDLModel((Node *) program);
	ENSURE_REL_TO_REL_GRAPH(program);
	ENSURE_BODY_PRED_TO_RULE_MAP(program);
	createRelToRuleMap((Node *) program);

	doneAtoms = copyObject(DL_GET_PROP(program, DL_EDB_RELS));
	predToRule = copyObject(DL_GET_PROP(program, DL_MAP_RELNAME_TO_RULES));
	bodyPredToRule = (HashMap *) DL_GET_PROP(program, DL_MAP_BODYPRED_TO_RULES);
	relG = (Graph *) copyObject(getDLProp((DLNode *) program, DL_REL_TO_REL_GRAPH));

	FOREACH(DLRule,r,program->rules)
	{
		if(allBodyAtomsDone(r, doneAtoms, relG))
		{
			addToSet(todoRules, r);
		}
	}

	while(!EMPTY_SET(todoRules))
	{
		DLRule *r = popSet(todoRules);
		char *headPred = getHeadPredName(r);
		List *predRules = (List *) getMapString(predToRule, headPred);

		newFDs = CONCAT_LISTS(newFDs, inferFDsForRule(program, r, newFDs));

		predRules = genericRemoveFromList(predRules, equal, r);
		MAP_ADD_STRING_KEY(predToRule, strdup(headPred), predRules);

		// we are done with this predicate. check whether we can now process additional rules
		if(MY_LIST_EMPTY(predRules))
		{
			List *posRules = (List *) getMapString(bodyPredToRule, headPred);
			addToSet(doneAtoms, headPred);
			FOREACH(DLRule,pr,posRules)
			{
				if(allBodyAtomsDone(pr, doneAtoms, relG))
				{
					addToSet(todoRules, pr);
				}
			}
		}
	}

	return newFDs;
}

static boolean
allBodyAtomsDone(DLRule *r, Set *doneAtoms, Graph *g)
{
    Set *preds = directlyReachableFrom(
		g,
		(Node *) createConstString(getHeadPredName(r)));

	FOREACH_SET(Constant,p,preds)
	{
		if(!hasSetElem(doneAtoms, STRING_VALUE(p)))
		{
			return FALSE;
		}
	}

	return TRUE;
}


List *
inferFDsForRule(DLProgram *p, DLRule *r, List *fds)
{
	List *adaptedFDs = adaptFDsToRules(p, r, fds);
    Set *headVars = makeNodeSetFromList(getHeadVars(r));
	char *headPred = getHeadPredName(r);
	HashMap *headVarToAttr = NEW_MAP(Constant,Constant);
	List *result = NIL;
	List *headAttrs = getHeadAttrNames(r);

	MAP_SET_NODE(headVars, createConstString(((DLVar *) it)->name));
	headVars = makeStringSetFromConstSet(headVars);

	// map head vars to attribute names
	FORBOTH(Node, arg, attr, r->head->args, headAttrs)
	{
		if(isA(arg,DLVar))
		{
			DLVar *v = (DLVar *) arg;
			char *a = (char *) attr;

		    MAP_ADD_STRING_KEY_AND_VAL(headVarToAttr, v->name, a);
		}
	}

	// filter adapted FDs that will hold on the rule head.
	FOREACH(FD,f,adaptedFDs)
	{
		Set *lhs = f->lhs;

		// For every FD A -> B, if A <= headVars(r) add A -> attributeClosure(A) INTERSECT headVars(r)
		// This is necessary to deal with things like A -> B, B -> C when Q(A,C) is the head
		if(containsSet(lhs, headVars))
		{
			Set *newLhs = copyObject(lhs);
			Set *rhsClos = attributeClosure(adaptedFDs, newLhs, NULL);
			Set *newRhs =  setDifference(intersectSets(rhsClos, headVars), newLhs);

			//map variables back to head pred attribute names
			MAP_SET_STR(newRhs, (strdup(STRING_VALUE(MAP_GET_STRING(headVarToAttr, it)))));
			MAP_SET_STR(newLhs, (strdup(STRING_VALUE(MAP_GET_STRING(headVarToAttr, it)))));

			result = appendToTailOfList(
				result,
				createFD(strdup(headPred), newLhs, newRhs));
		}
	}

	//  add PK for group-by
	if(hasAggFunction((Node *) r))
	{
		Set *lhs, *rhs;
		FD *f;

		lhs = copyObject(headVars);
		MAP_SET_STR(lhs, (strdup(STRING_VALUE(MAP_GET_STRING(headVarToAttr, it)))));
		rhs = makeStrSetFromList(headAttrs);

		f = createFD(strdup(headPred), lhs, rhs);
		result = appendToTailOfList(result, f);
	}

	return result;
}
