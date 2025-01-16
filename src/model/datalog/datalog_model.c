/*-----------------------------------------------------------------------------
 *
 * datalog_model.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/graph/graph.h"
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "model/integrity_constraints/integrity_constraints.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"
#include "analysis_and_translate/analyze_dl.h"
#include "metadata_lookup/metadata_lookup.h"

#if HAVE_ORACLE_BACKEND
#include "ocilib.h"
#endif

#include <assert.h>

static List *makeUniqueVarNames(List *args, int *varId, boolean doNotOrigNames, Set *allnames);
static boolean findVarsVisitor(Node *node, List **context);
static List *getAtomVars(DLAtom *a);
static List *getAtomArgs(DLAtom *a);
static Node *unificationMutator(Node *node, HashMap *context);
static List *mergeRule(DLRule *super, List *replacements);
static char *getFirstSubstitutableIDBAtom(DLRule *r, DLProgram *p, Set *idbPreds, Set *aggPreds, Set *genProjPreds, char *ansPred, HashMap *predToRule, List *fds, boolean allowRuleNumberIncrease,boolean isAgg);
/* static boolean ruleHasPosSubstitutableIDBAtom(DLRule *r, Set *idbPreds, Set *aggPreds, char *ansPred); */
static boolean headVarsImplyBodyVars(DLProgram *p, DLRule *r, List *fds);
/* static List *ruleGetAggIDBAtoms(DLRule *r, Set *aggPreds); */
static boolean delPropsVisitor(Node *n, void *context);

DLAtom *
createDLAtom (char *rel, List *args, boolean negated)
{
    DLAtom *result = makeNode(DLAtom);

    result->rel = rel;
    result->args = args;
    result->negated = negated;

    return result;
}

DLAtom *
createDLAtomFromStrs(char *rel, boolean negated, char *vars, ...)
{
	List *vargs = NIL;
	char *vname;
	va_list args;

	vargs = appendToTailOfList(vargs, createDLVar(vars, DT_INT));

	va_start(args, vars);

	while((vname = va_arg(args, void*)) != NULL)
	{
		vargs = appendToTailOfList(vargs, createDLVar(vname, DT_INT));
	}

	va_end(args);

	return createDLAtom(rel, vargs, negated);
}

DLVar *
createDLVar (char *vName, DataType vType)
{
    DLVar *result = makeNode(DLVar);

    result->name = vName;
    result->dt = vType;

    return result;
}

boolean
isConstAtom (DLAtom *a)
{
    FOREACH(Node,arg,a->args)
    {
        if (!isA(arg, Constant))
            return FALSE;
    }

    return TRUE;
}

DLRule *
createDLRule (DLAtom *head, List *body)
{
    DLRule *result = makeNode(DLRule);

    result->head = head;
    result->body = body;

    return result;
}


DLProgram *
createDLProgram(List *dlRules, List *facts, char *ans, List *doms, List *func, List *sumOpts)
{
    DLProgram *result = makeNode(DLProgram);

    result->rules = dlRules;
    result->facts = facts;
    result->ans = ans;
    result->doms = doms;
    result->func = func;
    result->sumOpts = sumOpts;

    return result;
}

DLComparison *
createDLComparison (char *op, Node *lArg, Node *rArg)
{
    DLComparison *result = makeNode(DLComparison);

    result->opExpr = createOpExpr(op, LIST_MAKE(lArg, rArg));

    return result;
}

DLDomain *
createDLDomain (char *rel, char *attr, char *dom)
{
    DLDomain *result = makeNode(DLDomain);

    result->rel = rel;
    result->attr = attr;
    result->name = dom;

    return result;
}

char *
getHeadPredName(DLRule *r)
{
    DLAtom *h = r->head;
    return h->rel;
}

List *
getHeadAttrNames(DLRule *r)
{
	List *results = NIL;

	for(int i = 0; i < LIST_LENGTH(r->head->args); i++)
	{
		results = appendToTailOfList(results, IDB_ATTR_NAME(i));
	}

	return results;
}

List *
getRuleVars(DLRule *r)
{
    List *result = NIL;

    result = CONCAT_LISTS(result, getAtomVars(r->head));
    result = CONCAT_LISTS(result, getBodyVars(r));

    return result;
}

List *
getBodyArgs(DLRule *r)
{
    List *result = NIL;

    FOREACH(Node,a,r->body)
    {
        if (isA(a, DLAtom))
            result = CONCAT_LISTS(result,
                    getAtomArgs((DLAtom *) a));
    }

    return result;
}

List *
getBodyVars(DLRule *r)
{
    List *result = NIL;

    FOREACH(Node,a,r->body)
    {
        if (isA(a, DLAtom))
            result = CONCAT_LISTS(result,
                    getAtomVars((DLAtom *) a));
        else if (isA(a,DLComparison))
            result = CONCAT_LISTS(result,
                    getComparisonVars((DLComparison *) a));
    }

    return result;
}

List *
getBodyPredVars(DLRule *r)
{
    List *result = NIL;

    FOREACH(Node,a,r->body)
    {
        if (isA(a, DLAtom))
            result = CONCAT_LISTS(result,
                    getAtomVars((DLAtom *) a));
    }

    return result;
}


/**
 * @brief Return variables from head of rule r.
 * Ignore variables in expressions, e.g., for
 *
 * Q(X,Y+Z,sum(A)) :-
 *
 * we would return [X].
 *
 * @param r the rule
 * @return list of DLVar nodes
 */


List *
getHeadVars(DLRule *r)
{
	List *result = NIL;

	FOREACH(Node,harg,r->head->args)
	{
		if(isA(harg,DLVar))
		{
			result = appendToTailOfList(result, harg);
		}
	}

    return result;
}

List *
getHeadVarNames(DLRule *r)
{
	List *result = getHeadVars(r);

	return getVarNames(result);
}

List *
getVarNames (List *vars)
{
    List *result = NIL;

    FOREACH(DLVar,v,vars)
        result = appendToTailOfList(result, strdup(v->name));

    return result;
}

DLRule *
unifyRule(DLRule *r, List *headBinds)
{
    DLRule *result = copyObject(r);
    List *hVars = getHeadVars(r);
    HashMap *varToBind = NEW_MAP(Constant,Node);
    ASSERT(LIST_LENGTH(headBinds) == LIST_LENGTH(hVars));

    // create map varname to binding
    FORBOTH(Node,v,bind,hVars,headBinds)
    {
        DLVar *var = (DLVar *) v;
        MAP_ADD_STRING_KEY(varToBind,var->name,bind);
        DEBUG_LOG("Var %s bind to %s", var->name, exprToSQL(bind, NULL, FALSE));
    }

    result = (DLRule *) unificationMutator((Node *) result, varToBind);

    return result;
}

char *
getUnificationString(DLAtom *a)
{
    StringInfo result = makeStringInfo();
    int curId = 0;
    HashMap *varToNewVar = NEW_MAP(Constant,Constant);

    appendStringInfoString(result,strdup(a->rel));

    FOREACH(Node,arg,a->args)
    {
        char *stringArg = NULL;
        boolean isVar = isA(arg,DLVar);

        if (isVar)
        {
            DLVar *d = (DLVar *) arg;
            void *entry = MAP_GET_STRING(varToNewVar,d->name);

            if (entry == NULL)
            {
                stringArg = CONCAT_STRINGS("V", gprom_itoa(curId++));
                MAP_ADD_STRING_KEY(varToNewVar, d->name, createConstString(stringArg));
            }
            else
                stringArg = strdup(STRING_VALUE(entry));
        }
        else if (isA(arg,Constant))
            stringArg = exprToSQL(arg, NULL, FALSE);
        else
            FATAL_LOG("unexpected type: %u", arg->type);

        appendStringInfo(result,"_%s.%s", isVar ? "V" : "C", stringArg);
    }

    return result->data;
}

DLProgram *
mergeSubqueries(DLProgram *p, boolean allowRuleNumberIncrease)
{
	DLProgram *result;
	Graph *relGraph;
	HashMap *predToRule;
	Set *idbRels;
	Set *aggRels;
	Set *genProjRels;
	/* Set *requiredAggRels = STRSET(); */
	Set *todo, *done;
	List *newRules = NIL;
	List *fds = (List *) DL_GET_PROP(p, DL_PROG_FDS);
	char *filterPred = DL_GET_STRING_PROP_DEFAULT(p, DL_PROV_LINEAGE_RESULT_FILTER_TABLE, NULL);

	DEBUG_LOG("Trying to merge subqueries.");

	ENSURE_DL_CHECKED(p);
	/* checkDLModel((Node *) p); */
	result = copyObject(p);

	idbRels = (Set *) getDLProp((DLNode *) result, DL_IDB_RELS);
	relGraph = (Graph *) getDLProp((DLNode *) result, DL_REL_TO_REL_GRAPH);
	predToRule = (HashMap *) getDLProp((DLNode *) result, DL_MAP_RELNAME_TO_RULES);
	aggRels = (Set *) getDLProp((DLNode *) result, DL_AGGR_RELS);
	genProjRels = (Set *) getDLProp((DLNode *) result, DL_GEN_PROJ_RELS);

	// need to preserve answer relation
	if(p->ans)
	{
		todo = MAKE_NODE_SET(createConstString(p->ans));
	}
	else
	{
		todo = sourceNodes(relGraph);
	}
	done = STRSET();

	// if this is a program that will be rewritten then keep the rules for relation restricting the provenance
	if(filterPred)
	{
		addToSet(todo,createConstString(filterPred));
	}

	// iterate until all non-negated IDB predicates have been replaced with the bodies of the rules that defines them
	while(!EMPTY_SET(todo))
	{
		char *cur = STRING_VALUE(popSet(todo));
		addToSet(done, cur);
		List *pRules = (List *) MAP_GET_STRING(predToRule, cur);

		DEBUG_LOG("Process predicate %s", cur);

		List *todoR = copyObject(pRules);

		// loop until we have exaustively replaced idb predicates in rules for
		// the predicate.
		while(!MY_LIST_EMPTY(todoR))
		{
			DLRule *curR = popHeadOfListP(todoR);
			/* List *aggPredNames = NIL; */
			boolean isAgg = hasSetElem(aggRels, getHeadPredName(curR));
			char *firstIDB = getFirstSubstitutableIDBAtom(curR,
														  p,
														  idbRels,
														  aggRels,
														  genProjRels,
														  p->ans,
														  predToRule,
														  fds,
														  allowRuleNumberIncrease,
														  isAgg);

			DEBUG_DL_LOG("Check for substitutable IDB atom in", curR);

			if(firstIDB != NULL)
			{
				List *iRules = (List *) MAP_GET_STRING(predToRule, firstIDB);
				List *addedRules;

				DEBUG_LOG("Do replace atom %s", firstIDB);

				addedRules = mergeRule(copyObject(curR), copyObject(iRules));
				/* newRules = appendAllToTail(newRules, copyObject(addedRules)); */
				todoR = appendAllToTail(todoR, copyObject(addedRules));
			}
			else
			{
				DEBUG_DL_LOG("No replacable atoms found for", curR);
				newRules = appendToTailOfList(newRules, curR);

				// need to add remaining IDB predicates to todo since we need them
				FOREACH(DLNode,n,curR->body)
				{
					if(isA(n,DLAtom))
					{
						DLAtom *a = (DLAtom *) n;

						if(hasSetElem(idbRels, a->rel) && !hasSetElem(done, a->rel))
						{
							DEBUG_LOG("Added predicate %s to todo", a->rel);
							addToSet(todo, createConstString(a->rel));
						}
					}
				}
			}
		}
	}


	result = copyObject(p);
	result->rules = newRules;

	// recreate analysis data structures for modified program
	createDLanalysisStructures(result, TRUE, TRUE, TRUE);

	INFO_DL_LOG("Program after merging subqueries:", result);

	return result;
}

static char *
getFirstSubstitutableIDBAtom(DLRule *r, DLProgram *p, Set *idbPreds, Set *aggPreds, Set *genProjPreds, char *ansPred, HashMap *predToRule, List *fds, boolean allowRuleNumberIncrease, boolean isAgg)
{
	FOREACH(DLNode,n,r->body)
	{
		if(isA(n,DLAtom))
		{
			DLAtom *a = (DLAtom *) n;
			List *iRules = (List *) MAP_GET_STRING(predToRule, a->rel);

			// for non aggregation rules, we only merge if there is only one rule defining
			// the IDB body goal or if we explicitly have allowed increasing the number of rules
			// for aggregation rules we can only merge the rules of an IDB goal if this does not
			// change the number of tuples (when there is a single rule defining the IDB predicate
			// and this rule's head variables imply all of the body variables.
			if(!a->negated &&
			   hasSetElem(idbPreds, a->rel) &&
			   !strpeq(a->rel, ansPred) &&
			   !hasSetElem(aggPreds, a->rel) &&
			   !hasSetElem(genProjPreds, a->rel) &&
			   ((!isAgg
				   && (LIST_LENGTH(iRules) < 2 || allowRuleNumberIncrease))
			   || (isAgg
				   && LIST_LENGTH(iRules) == 1
				   && headVarsImplyBodyVars(p, getHeadOfListP(iRules), fds))))
			{
				return a->rel;
			}
		}
	}

	return NULL;
}

/* static boolean */
/* ruleHasPosSubstitutableIDBAtom(DLRule *r, Set *idbPreds, Set *aggPreds, char *ansPred) */
/* { */
/* 	return getFirstSubstitutableIDBAtom(r, idbPreds, aggPreds, ansPred) != NULL; */
/* } */

static boolean
headVarsImplyBodyVars(DLProgram *p, DLRule *r, List *fds)
{
	List *adaptedFDs = adaptFDsToRules(p, r, fds);
	boolean result = FALSE;
	FD *headImpliesBody = createFD(getHeadPredName(r),
								   makeStrSetFromList(getHeadVarNames(r)),
								   makeStrSetFromList(getVarNames(getBodyVars(r))));

	if(checkFDonAtoms(makeNodeSetFromList(r->body), adaptedFDs, headImpliesBody))
	{
		result = TRUE;
	}

	DEBUG_LOG("for rule %s head variables do %simply body variable",
			  datalogToOverviewString(r),
			  result ? "" : "NOT "
		);

	return result;
}


/* static List * */
/* ruleGetAggIDBAtoms(DLRule *r, Set *aggPreds) */
/* { */
/* 	List *result = NIL; */

/* 	FOREACH(DLNode,n,r->body) */
/* 	{ */
/* 		if(isA(n,DLAtom)) */
/* 		{ */
/* 			DLAtom *a = (DLAtom *) n; */

/* 			if(hasSetElem(aggPreds, a->rel)) */
/* 			{ */
/* 				result = appendToTailOfList(result, a->rel); */
/* 			} */

/* 		} */
/* 	} */

/* 	return result; */
/* } */

static List *
mergeRule(DLRule *super, List *replacements)
{
	List *results = NIL;
	char *pred = getHeadPredName(getHeadOfListP(replacements));

    INFO_LOG("Replace %s in %s with\n\n%s",
			  pred,
			  datalogToOverviewString(super),
			  datalogToOverviewString(replacements));

	// for every replacement we need to create a copy of the input rule
	FOREACH(DLRule,repl,replacements)
	{
		DLRule *newR = copyObject(super);
		List *newBody = NIL;
		List *subst = NIL;
		List *uniqueScope = NIL;

		DEBUG_DL_LOG("substitute", repl);

		// create one copy of repl for each atom with repl's head predicate
		FOREACH(DLNode,a,super->body)
		{
			if(isA(a,DLAtom))
			{
				DLAtom *at = (DLAtom *) a;
				if(streq(pred,at->rel) && !at->negated)
				{
					subst = appendToTailOfList(subst, copyObject(repl));
				}
			}
		}

		// make sure that the rule and all replacement rule copies have unique
		// variable names. Try to keep original names for readability
		uniqueScope = copyList(subst);
		uniqueScope = appendToHeadOfList(uniqueScope, newR);
		makeVarNamesUnique(uniqueScope, TRUE);

	    INFO_DL_LOG("after making super variable names unique", newR);
	    INFO_DL_LOG("after making variable name unique", subst);

		// replace atoms with rule bodies
		FOREACH(DLNode,a,newR->body)
		{
			if(isA(a,DLAtom))
			{
				DLAtom *at = (DLAtom *) a;
				if(streq(pred,at->rel) && !at->negated)
				{
					DLRule *unRule = popHeadOfListP(subst);

					// unify rule head with bindings from body atom
					unRule = unifyRule(unRule, at->args);

					// insert body atoms of unified rule
					newBody = appendAllToTail(newBody, unRule->body);
				}
				else
				{
					newBody = appendToTailOfList(newBody, copyObject(a));
				}
			}
			else
			{
				newBody = appendToTailOfList(newBody, copyObject(a));
			}
		}

		newR->body = newBody;
		INFO_DL_LOG("result of substitution", newR);
		results = appendToTailOfList(results, newR);
	}

	return results;
}


/*
 *  Takes an atom and replaces variables with standardized names (V1, V2, ...). This
 *  is used, e.g., when unifying a program with a user provenance questions to
 *  abstract from variable naming. For instance
 *
 *      WHY(Q(1))
 *
 *      r1: Q(X) :- R(X,Y), S(Y,X)
 *      r2: Q(X) :- T(X,Z), S(Z,X)
 *      r3: S(A,B) :- U(A,B)
 *
 *      for both rules r1 and r2 the first variable of atom S is not bound to a constant.
 *      We replace both occurances with S(V1,1) to ensure that we are not trying to
 *      derive two unified instances of r3 where only one is needed.
 */
DLAtom *
getNormalizedAtom(DLAtom *a)
{
    DLAtom *result = copyObject(a);
    int varId = 0;
	Set *allnames = STRSET();

    makeUniqueVarNames(result->args, &varId, FALSE, allnames);

    return result;
}

void
makeVarNamesUnique(List *nodes, boolean keepOrigNames)
{
    int varId = 0;
	Set *allnames = STRSET();

    FOREACH(DLNode,n,nodes)
    {
        if (isA(n, DLRule))
        {
            DLRule *r = (DLRule *) n;
            List *args = getRuleVars(r);
            makeUniqueVarNames(args, &varId, keepOrigNames, allnames);
        }
        if (isA(n, DLAtom))
        {
            DLAtom *a = (DLAtom *) n;
            makeUniqueVarNames(a->args, &varId, keepOrigNames, allnames);
        }
    }
}

static List *
makeUniqueVarNames(List *args, int *varId, boolean doNotOrigNames, Set *allnames)
{
    HashMap *varToNewVar = NEW_MAP(Constant,Constant);
    Set *names = STRSET();

    FOREACH(Node,arg,args)
	{
		List *vars = getExprVars(arg);

		FOREACH(DLVar,v,vars)
		{
            addToSet(names, v->name);
		}
	}

    FOREACH(Node,arg, args)
    {
		List *vars = getExprVars(arg);

		FOREACH(DLVar,v,vars)
		{
			char *stringArg = NULL;
			void *entry = MAP_GET_STRING(varToNewVar,v->name);

            if (entry == NULL)
            {
                // skip varnames that already exist
                if (doNotOrigNames)
				{
					if (!hasSetElem(allnames, v->name))
					{
						stringArg = strdup(v->name);
					}
					else {
						while(hasSetElem(allnames, stringArg = CONCAT_STRINGS("V", gprom_itoa((*varId)++))))
							;
					}
				}
                else
                    stringArg = CONCAT_STRINGS("V", gprom_itoa((*varId)++));

                MAP_ADD_STRING_KEY(varToNewVar, v->name, createConstString(stringArg));
            }
            else
                stringArg = strdup(STRING_VALUE(entry));

			addToSet(allnames, stringArg);
            v->name = stringArg;
		}
	}

    return args;
}

DLVar *
createUniqueVar(Node *n, DataType dt)
{
	List *varList = getExprVars(n);
	Set *vars = STRSET();
	int i = 0;
	char *varName;

	FOREACH(DLVar,v,varList)
	{
		addToSet(vars, v->name);
	}

	do {
		varName = CONCAT_STRINGS("V", gprom_itoa(i++));
	} while (hasSetElem(vars, varName));

	return createDLVar(varName, dt);
}

Node *
applyVarMap(Node *input, HashMap *h)
{
    return unificationMutator(input, h);
}

boolean
argListsUnifyable (List *argsL, List *argsR)
{
    HashMap *varToRepl = NEW_MAP(Constant,Node);

    // check whether args are unifyable by type (const/var)
    FORBOTH(Node,l,r,argsL,argsR)
    {
        // both are constant then they have to be the same
        if (isA(l, Constant) && isA(r, Constant))
        {
            if (!equal(l,r))
                return FALSE;
        }
        // original one is a var, make sure that we do not have set it to a conflicting value already
        else if (isA(l, DLVar))
        {
            DLVar *lV = (DLVar *) l;
            if (MAP_HAS_STRING_KEY(varToRepl, lV->name))
            {
                if (!equal(MAP_GET_STRING(varToRepl, lV->name), r))
                    return FALSE;
            }
            else
                MAP_ADD_STRING_KEY(varToRepl, lV->name, copyObject(r));
        }
        // left one is a constant and right one is a var. unification failed
        else
            return FALSE;
    }

    return TRUE;
}

Node *
applyVarMapAsLists(Node *input, List *vars, List *replacements)
{
    HashMap *h = NEW_MAP(Constant,Node);

    FORBOTH(Node,l,r,vars,replacements)
    {
        if (isA(l, DLVar))
        {
            DLVar *v = (DLVar *) l;

            if(v->dt != DT_BOOL)
            	MAP_ADD_STRING_KEY(h,v->name,r);
        }
    }

    return applyVarMap(input, h);
}

static Node *
unificationMutator(Node *node, HashMap *context)
{
    if (node == NULL)
        return NULL;

    // replace vars with bindings (if bound)
    if (isA(node, DLVar))
    {
        DLVar *oVar = (DLVar *) node;
        if (MAP_HAS_STRING_KEY(context, oVar->name))
        {
            Node *result = MAP_GET_STRING(context, oVar->name);

            return (Node *) copyObject(result);
        }
    }

    return mutate(node, unificationMutator, context);
}

static List *
getAtomArgs(DLAtom *a)
{
    return copyObject(a->args);
}

List *
getHeadExprVars (DLRule *r)
{
    return getAtomExprVars(r->head);
}

List *
getAtomExprVars (DLAtom *a)
{
    return getExprVars((Node *) a);
}

List *
getAtomTopLevelVars(DLAtom *a)
{
	List *result = NIL;

	FOREACH(DLNode,arg,a->args)
	{
		if(isA(arg,DLVar))
		{
			result = appendToTailOfList(result, arg);
		}
	}

	return result;
}

List *
getExprVars(Node *expr)
{
    List *result = NIL;

    findVarsVisitor(expr, &result);

    return result;
}

static boolean
findVarsVisitor(Node *node, List **context)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, DLVar))
    {
        *context = appendToTailOfList(*context, node);
    }

    return visit(node, findVarsVisitor, context);
}

size_t
getIDBPredArity(DLProgram *p, char *pred)
{
	ENSURE_DL_CHECKED(p);
	HashMap *relToRules = (HashMap *) DL_GET_PROP(p, DL_MAP_RELNAME_TO_RULES);
	ASSERT(MAP_HAS_STRING_KEY(relToRules, pred));
	DLRule *r = getHeadOfListP((List *) MAP_GET_STRING(relToRules, pred));

	return LIST_LENGTH(r->head->args);
}

boolean
isIDB(DLProgram *p, char *pred)
{
	ENSURE_DL_CHECKED(p);
	Set *idbPreds = (Set *) DL_GET_PROP(p,DL_IDB_RELS);
	/* HashMap *relToRules = (HashMap *) DL_GET_PROP(p, DL_MAP_RELNAME_TO_RULES); */
	return hasSetElem(idbPreds, pred);
}

boolean
isEDB(DLProgram *p, char *pred)
{
	return !isIDB(p, pred);
}


List *
predGetAttrNames(DLProgram *p, char *pred)
{
	ENSURE_DL_CHECKED(p);
	if(isIDB(p, pred))
	{
		List *attrs = NIL;
		int arity = getIDBPredArity(p, pred);
		for(int i = 0; i < arity; i++)
		{
			attrs = appendToTailOfList(attrs, IDB_ATTR_NAME(i));
		}

		return attrs;
	}
	else
	{
		return getAttributeNames(pred);
	}
}

List *
getComparisonAtoms(DLRule *r)
{
	List *result = NIL;

	FOREACH(DLNode,n,r->body)
	{
		if(isA(n,DLComparison))
		{
			result = appendToTailOfList(result, n);
		}
	}

	return result;
}

List *
getGoalsForPred(DLRule *r, char *p)
{
	List *results = NIL;

	FOREACH(DLNode,n,r->body)
	{
		if(isA(n,DLAtom))
		{
			DLAtom *a = (DLAtom *) n;
			if(streq(p,a->rel))
			{
				results = appendToTailOfList(results, a);
			}
		}
	}

	return results;
}

static List *
getAtomVars(DLAtom *a)
{
    List *result = NIL;

    FOREACH(Node,arg,a->args)
    {
		result = CONCAT_LISTS(result, getExprVars(arg));
    }

    return result;
}

List *
getComparisonVars(DLComparison *a)
{
    List *result = NIL;

    FOREACH(Node,arg,a->opExpr->args)
    {
        if (isA(arg, DLVar))
            result = appendToTailOfList(result, arg);
    }

    return result;
}

Node *
getDLProp(DLNode *n, char *key)
{
    if (n->properties == NULL)
        return NULL;

    return MAP_GET_STRING(n->properties, key);
}

void
setDLProp(DLNode *n, char *key, Node *value)
{
    if (n->properties == NULL)
        n->properties = NEW_MAP(Node,Node);

    if (value == NULL)
        return;

    MAP_ADD_STRING_KEY(n->properties, key, value);
}

void
delDLProp(DLNode *n, char *key)
{
    if (n->properties != NULL)
        removeMapElem(n->properties, (Node *) createConstString(key));
}

void
delAllProps(DLNode *n)
{
	delPropsVisitor((Node *) n, NULL);
}

static boolean
delPropsVisitor(Node *n, void *context)
{
	if (n == NULL)
		return TRUE;

	if(IS_DL_NODE(n))
	{
		DLNode *dl = (DLNode *) n;
		dl->properties = NEW_MAP(Constant,Node);
	}

	return visit(n, delPropsVisitor, context);
}
