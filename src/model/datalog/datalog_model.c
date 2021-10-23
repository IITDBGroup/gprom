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
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/graph/graph.h"
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "analysis_and_translate/analyze_dl.h"
#include "ocilib.h"
#include <assert.h>

static List *makeUniqueVarNames (List *args, int *varId, boolean doNotOrigNames);
static boolean findVarsVisitor (Node *node, List **context);
static List *getAtomVars(DLAtom *a);
static List *getAtomArgs(DLAtom *a);
static List *getComparisonVars(DLComparison *a);
static Node *unificationMutator (Node *node, HashMap *context);
static List *mergeRule(DLRule *super, List *replacements);
static char *getFirstIDBAtom(DLRule *r, Set *idbPreds);
static boolean ruleHasPosIDBAtom(DLRule *r, Set *idbPreds);

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
createDLProgram (List *dlRules, List *facts, char *ans, List *doms, List *func, List *sumOpts)
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
getRuleVars (DLRule *r)
{
    List *result = NIL;

    result = CONCAT_LISTS(result, getAtomVars(r->head));
    result = CONCAT_LISTS(result, getBodyVars(r));

    return result;
}

List *
getBodyArgs (DLRule *r)
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
getBodyVars (DLRule *r)
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
getBodyPredVars (DLRule *r)
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


List *
getHeadVars (DLRule *r)
{
    return getAtomVars(r->head);
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
        DEBUG_LOG("Var %s bind to %s", var->name, exprToSQL(bind, NULL));
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
            stringArg = exprToSQL(arg, NULL);
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
	Set *todo;
	List *newRules = NIL;

	ENSURE_REL_TO_REL_GRAPH(p);
	checkDLModel((Node *) p);
	result = copyObject(p);

	idbRels = (Set *) getDLProp((DLNode *) result, DL_IDB_RELS);
	relGraph = (Graph *) getDLProp((DLNode *) result, DL_REL_TO_REL_GRAPH);
	predToRule = (HashMap *) getDLProp((DLNode *) result, DL_MAP_RELNAME_TO_RULES);
    todo = sourceNodes(relGraph);

	// iterate until all non-negated IDB predicates have been replaced with the bodies of the rules that defines them
	while(!EMPTY_SET(todo))
	{
		char *cur = STRING_VALUE(popSet(todo));
		List *pRules = (List *) MAP_GET_STRING(predToRule, cur);

		DEBUG_LOG("Process predicate %s", cur);

		FOREACH(DLRule,r,pRules)
		{
			List *todoR = LIST_MAKE(copyObject(r));

			while(!LIST_EMPTY(todoR))
			{
				DLRule *curR = popHeadOfListP(todoR);
				DEBUG_DL_LOG("Substitute first IDB atom in", curR);

				if(ruleHasPosIDBAtom(curR, idbRels))
				{
					char *firstIDB = getFirstIDBAtom(curR, idbRels);
					List *iRules = (List *) MAP_GET_STRING(predToRule, firstIDB);

					DEBUG_LOG("Replace atom %s", firstIDB);

					if(LIST_LENGTH(iRules) < 2 || allowRuleNumberIncrease)
					{
						List *newRules;
						newRules = mergeRule(curR, copyObject(iRules));
						todoR = appendAllToTail(todoR, newRules);
					}
				}
				else
				{
					newRules = appendToTailOfList(newRules, curR);
				}
			}
		}
	}

	result = copyObject(p);
	result->rules = newRules;

	DEBUG_DL_LOG("After merging subqueries we get", result);
	return result;
}

static char *
getFirstIDBAtom(DLRule *r, Set *idbPreds)
{
	FOREACH(DLNode,n,r->body)
	{
		if(isA(n,DLAtom))
		{
			DLAtom *a = (DLAtom *) n;

			if(!a->negated && hasSetElem(idbPreds, a->rel))
			{
				return a->rel;
			}
		}
	}

	return NULL;
}

static boolean
ruleHasPosIDBAtom(DLRule *r, Set *idbPreds)
{
	return getFirstIDBAtom(r, idbPreds) != NULL;
}

static List *
mergeRule(DLRule *super, List *replacements)
{
	List *results = NIL;
	char *pred = getHeadPredName(getHeadOfListP(replacements));

	DEBUG_LOG("Replace %s in %s with\n\n%s",
			  pred,
			  datalogToOverviewString(super),
			  datalogToOverviewString(replacements));

	// for every replacement we need to create a copy of the input rule
	FOREACH(DLRule,repl,replacements)
	{
		DLRule *newR = copyObject(super);
		List *newBody = NIL;
		List *subst = NIL;

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

		// make sure that the rule and all replacement rule copies have unique variable names
		subst = appendToHeadOfList(subst, newR);
		makeVarNamesUnique(subst);
		DEBUG_DL_LOG("after making variable name unique", subst);
		popHeadOfListP(subst);


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

    makeUniqueVarNames(result->args, &varId, FALSE);

    return result;
}

void
makeVarNamesUnique(List *nodes)
{
    int varId = 0;

    FOREACH(DLNode,n,nodes)
    {
        if (isA(n, DLRule))
        {
            DLRule *r = (DLRule *) n;
            List *args = getRuleVars(r);
            makeUniqueVarNames(args, &varId, TRUE);
        }
        if (isA(n, DLAtom))
        {
            DLAtom *a = (DLAtom *) n;
            makeUniqueVarNames(a->args, &varId, TRUE);
        }
    }
}

static List *
makeUniqueVarNames (List *args, int *varId, boolean doNotOrigNames)
{
    HashMap *varToNewVar = NEW_MAP(Constant,Constant);
    Set *names = STRSET();

    FOREACH(Node,arg,args)
        if (isA(arg,DLVar))
            addToSet(names, ((DLVar *) arg)->name);

    FOREACH(Node,arg, args)
    {
        char *stringArg = NULL;

        if (isA(arg,DLVar))
        {
            DLVar *d = (DLVar *) arg;
            void *entry = MAP_GET_STRING(varToNewVar,d->name);

            if (entry == NULL)
            {
                // skip varnames that already exist
                if (doNotOrigNames)
                    while(hasSetElem(names, stringArg = CONCAT_STRINGS("V", gprom_itoa((*varId)++))))
                        ;
                else
                    stringArg = CONCAT_STRINGS("V", gprom_itoa((*varId)++));

                MAP_ADD_STRING_KEY(varToNewVar, d->name, createConstString(stringArg));
            }
            else
                stringArg = strdup(STRING_VALUE(entry));

            d->name = stringArg;
        }
    }

    return args;
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
getExprVars(Node *expr)
{
    List *result = NIL;

    findVarsVisitor(expr, &result);

    return result;
}

static boolean
findVarsVisitor (Node *node, List **context)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, DLVar))
    {
        *context = appendToTailOfList(*context, node);
    }

    return visit(node, findVarsVisitor, context);
}

static List *
getAtomVars(DLAtom *a)
{
    List *result = NIL;

    FOREACH(Node,arg,a->args)
    {
        if(isA(arg, DLVar))
            result = appendToTailOfList(result, arg);
    }

    return result;
}

static List *
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
