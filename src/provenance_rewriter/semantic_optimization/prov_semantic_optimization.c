/*
 *------------------------------------------------------------------------------
 *
 * prov_semantic_optimization.c - Semantic query optimization with constraints
 * for provenance computation.
 *
 *     Implements faster alternatives to chase-based methods for semantic query
 *     optimization for the special case of provenance capture queries.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-01
 *        SUBDIR: src/provenance_rewriter/semantic_optimization/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/graph/graph.h"
#include "model/datalog/datalog_model.h"
#include "model/integrity_constraints/integrity_constraints.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"
#include "src/parser/oracle_parser.tab.h"
#include "analysis_and_translate/analyze_dl.h"

typedef struct RewriteSearchState {
	Set *in;
	Set *todo;
	Graph *jg;
	List *fds;
	DLRule *r;
} RewriteSearchState;

#define COPY_STATE(_in,_out) \
	do { \
		_out = NEW(RewriteSearchState); \
		_out->in = copyObject(_in->in); \
		_out->todo = copyObject(_in->todo); \
		_out->jg = copyObject(_in->jg); \
		_out->fds = _in->fds;  \
		_out->r = _in->r; \
	} while(0)

#define LOG_STATE(_st) DEBUG_NODE_BEATIFY_LOG("RewriteSearchState: \n", _st->in, _st->todo, _st->jg)

static Set *varNamesForAtom(DLAtom *atom);
static Set *topLevelVarNamesForAtom(DLAtom *atom);
static Set *varNamesForAtoms(Set *atoms);
static Set *varListToNameSet(List *vars);
static Set *computeSeeds(DLRule *r, List *fds, DLAtom *target);
static boolean existsNonReachableAtom(RewriteSearchState *state);
static boolean removeOneEdgeBasedOnFDs(Set *dreach, RewriteSearchState *state);
static Set *computeFrontier(Graph *g, Set *nodes);
static int getMaxId(HashMap *m);

List *
optimizeDLRule(DLProgram *p, DLRule *r, List *inFDs, DLAtom *target, char *filterPred)
{
	Set *seeds;
	Graph *joinG;
	List *todo = NIL;
	DLRule *min;
	List *fds;
	Set *headVars = makeStrSetFromList(getHeadVarNames(r));
	Graph *ig = GET_INV_REL_TO_REL_GRAPH(p);
	HashMap *ruleids = createRuleIds(p);
	List *result  = NIL;
	int maxId = getMaxId(ruleids) + 1;

	START_TIMER("semantic optimization");

	joinG = createJoinGraph(r);
	fds = adaptFDsToRules(p, r, inFDs);

	DEBUG_LOG("adapted FDs: %s", icToString((Node *) fds));

	ASSERT(target);
	DEBUG_NODE_BEATIFY_LOG("target atom is ", target);

	// determine seeds and setup todo list
	seeds = computeSeeds(r, fds, target);

	DEBUG_NODE_BEATIFY_LOG("seeds are: ", seeds);

	// no seeds, head of the query provides all variables we need
	if(!seeds) //TODO
	{
		min =  createDLRule(copyObject(r->head), NIL);
		addToMap(ruleids, (Node *) min, (Node *) createConstInt(maxId++));
		result = createCaptureRule(min, target, filterPred, ig, ruleids); //TODO
		STOP_TIMER("semantic optimization");
		return result;
	}

	Set *bodyAts = makeNodeSetFromList(r->body);

	FOREACH_SET(Set,seed,seeds)
	{
		RewriteSearchState *state = NEW(RewriteSearchState);

		state->in = seed;
		state->todo = setDifference(copyObject(bodyAts), seed);
		state->jg = copyObject(joinG);
		state->fds = fds;
		state->r = r;

		todo = appendToTailOfList(todo, state);
	}

	// determine minimal rewritings for all seeds
	Set *results = NODESET();

	while(!MY_LIST_EMPTY(todo))
	{
		RewriteSearchState *cur = (RewriteSearchState *) popHeadOfListP(todo);
		LOG_STATE(cur);

		// no more atoms to check -> we have a result
		if(EMPTY_SET(cur->todo))
		{
			DEBUG_NODE_BEATIFY_LOG("new result: ", cur->in);
			addToSet(results, cur->in);
		}
		// if there are non-reachable atoms, then remove them and put ourselves back on the todo list
		else if(existsNonReachableAtom(cur))
		{
			DEBUG_NODE_BEATIFY_LOG("after removing unreachable we get: ", cur->in);
			todo = appendToTailOfList(todo, cur);
		}
		// no unreachable atoms
		else
		{
			Set *frontier = computeFrontier(cur->jg, cur->in);

			// check whether we can remove and edge based on FDs and cur.in
			if(removeOneEdgeBasedOnFDs(frontier, cur))
			{
				LOG_STATE(cur);
				todo = appendToTailOfList(todo, cur);
			}
			// no edge removable, need to branch on all atoms in cur.check that are directly reachable from atoms in cur.in
			else
			{
				DEBUG_NODE_BEATIFY_LOG("need to expand based on all directly reachable atoms: ", frontier);
				FOREACH_SET(DLAtom,a,frontier)
				{
					if(hasSetElem(cur->todo, a))
					{
						RewriteSearchState *newstate = NEW(RewriteSearchState);

						COPY_STATE(cur, newstate);
						addToSet(newstate->in, a);
						removeSetElem(newstate->todo, a);

						todo = appendToTailOfList(todo, newstate);
					}
				}
			}
		}

	}

	// construct rule from results
	//TODO for now just return the first one
	Set *bodyAtoms = popSet(results);
	List *body = NIL;
	List *comparisonAtoms = getComparisonAtoms(r);
	Set *minBodyVars = STRSET();
	Set *notImpliedMinBodyVars = STRSET();
	boolean fallbackToOriginalRule = FALSE;

	/* Some comparison atoms may have to be kept we have to distinguish 5 cases
	 *
	 * 1) comparisons that only refer variables from atoms we are keeping and
	 * these variable are implied by the head of the query => such comparison
	 * are not required for correctness since there is a single possible value
	 * for these variables for a given result tuple, but they may improve
	 * performance, e.g., by using an index in some cases so we will keep them
	 * (KEEP)
	 *
	 * 2) comparisons that only refer variables from atoms we are keeping which
	 * are not implied by the head variables of the query => these comparisons
	 * may filter tuples from the provenance so we have to keep them (KEEP).
	 *
	 * 3) comparisons using only variables from atoms we have removed => these
	 * can be savely removed because if we remove atoms then we know their
	 * variables are implied by the head variables, so they do not filter.
	 * (REMOVE)
	 *
	 * 4) comparisons using some variables from atoms that are removed and some
	 * that we are keeping with variables we are keeping are implied by the
	 * query's head variables. => using the same argument as in 1) and given
	 * that we only remove atoms whose variables are implied by the head
	 * variables, it is safe to remove these comparisons. (REMOVE)
	 *
	 * 5) comparisons using some variables from atoms that are removed and some
	 * that we are keeping with variables we are keeping are not implied by the
	 * query's head variables. => This case represents a problem, we have to
	 * apply the comparison predicate, but have already removed some atoms that
	 * bind the variables. Ideally, we should check this when removing atoms,
	 * but for now we just return the unoptimized rule as a fallback. (DO NOT
	 * OPTIMIZE)
	 */
	FOREACH_SET(DLAtom,a,bodyAtoms)
	{
		minBodyVars = unionSets(minBodyVars,
								makeStrSetFromList(getVarNames(getAtomExprVars(a))));
	}

	// determine which new body variables are not implied by the head
	notImpliedMinBodyVars = setDifference(minBodyVars,
		attributeClosureOnAtoms(bodyAtoms, headVars, fds));

	// handle each comparison predicate and check which
	FOREACH(DLComparison,c,comparisonAtoms)
	{
		Set *cVars = makeStrSetFromList(getVarNames(getComparisonVars(c)));

		// all new body variables
		if(containsSet(cVars,minBodyVars))
		{
			addToSet(bodyAtoms, c);
		}
		// some new body variables and some variables from removed atoms
		else if (overlapsSet(cVars, minBodyVars))
		{
			// uses non-implied variables from the new body atoms -> we made a
			// mistake and have to fall back to using the original rule!
			if(overlapsSet(cVars, notImpliedMinBodyVars))
			{
				fallbackToOriginalRule = TRUE;
				break;
			}
			// otherwise the rule can be removed
		}
		// otherwise the comparison only uses variables from removed atoms and
		// can be removed too
	}

	DEBUG_NODE_BEATIFY_LOG("minimized body is: ", bodyAtoms);

	// if we created an incorrect optimization, then use the original rule
	if(fallbackToOriginalRule)
	{
		min = r;
	}
	// otherwise build the optimized rule
	else
	{
		FOREACH_SET(DLAtom,a,bodyAtoms)
		{
			body = appendToTailOfList(body, a);
		}

		min = createDLRule(copyObject(r->head), body);
		addToMap(ruleids, (Node *) min, (Node *) createConstInt(maxId++));
	}

	// create a lineage capture rule
    result = createCaptureRule(min, target, filterPred, ig, ruleids);

    STOP_TIMER("semantic optimization");

	return result;
}


static int
getMaxId(HashMap *m)
{
	int result = -1;
	FOREACH_HASH(Constant, i, m)
	{
		result = MAX(result, INT_VALUE(i));
	}

	return result;
}


/**
 * @brief Compute Frontier of nodes to look at
 *
 *
 * @param g goal graph for datalog rule
 * @param nodes nodes to start from
 * @return set of goals in the frontier
 */

static Set *
computeFrontier(Graph *g, Set *nodes)
{
	Set *result = NODESET();

	FOREACH_SET(Node,n,nodes)
	{
		Set *dreach = directlyReachableFrom(g, n);

		FOREACH_SET(Node,d,dreach)
		{
			if (!hasSetElem(nodes, d))
			{
				addToSet(result, d);
			}
		}
	}

	return result;
}

static boolean
removeOneEdgeBasedOnFDs(Set *dreach, RewriteSearchState *state)
{
	Set *inVars = varNamesForAtoms(state->in);
	Set *headVars = varListToNameSet(getHeadVars(state->r));
	FD *test;

	test = createFD(NULL,headVars, NULL);

	FOREACH_SET(DLAtom,a,dreach)
	{
		// is on todo list
		if(hasSetElem(state->todo, a))
		{
			// FD to be tested vars(head(r)) -> inVars INTERSECT vars(a)
			test->rhs = intersectSets(copyObject(inVars), varListToNameSet(getAtomExprVars(a)));

			// if join attributes are implied by the head attributes, then
			if (checkFDonAtoms(state->in, state->fds, test))
			{
				FOREACH_HASH_ENTRY(kv,state->jg->edges)
				{
					// remove edges to a for nodes from state->in
					if(hasSetElem(state->in, kv->key))
					{
						removeSetElem((Set *) kv->value, a);
					}
					else if (equal(kv->key,a))
					{
						kv->value = (Node *) setDifference((Set *) kv->value, state->in);
					}
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

static boolean
existsNonReachableAtom(RewriteSearchState *state)
{
	boolean ex = FALSE;
	Set *newCheck = copyObject(state->todo);

	FOREACH_SET(DLAtom,a,state->todo)
	{
		boolean notReach = TRUE;
		// if a is not reachable from any node in state->in, then we can remove it
		FOREACH_SET(DLAtom,i,state->in)
		{
			if(isReachable(state->jg, (Node *) i, (Node *) a))
			{
				notReach = FALSE;
			}
		}

		if(notReach)
		{
			ex = TRUE;
			removeSetElem(newCheck,(Node *)  a);
			deleteNode(state->jg, (Node *) a);
		}
	}

	state->todo = newCheck;

	return ex;
}

static Set *
computeSeeds(DLRule *r, List *fds, DLAtom *target)
{
	Set *seeds = NODESET();
	Set *targetVars = varNamesForAtom(target);
	Set *headVars = topLevelVarNamesForAtom(r->head);
	List *candidates = NIL;
//	Set *headAndTargetVars = intersectSets(copyObject(headVars), copyObject(targetVars));

	// if head contains all the variables then no computation is needed, return null
	if(containsSet(targetVars, headVars))
	{
		return NULL;
	}

	// the target is a seed
	addToSet(seeds, MAKE_NODE_SET(target));

	//TODO if head variables INTERSECT target variables do not apply target variables then we need to return {g}??? Check with Murali why this holds (if this is not a bug)


	// create candidates starting from single atoms
	List *atoms = copyObject(r->body);
	atoms = REMOVE_FROM_LIST_NODE(candidates, target);
	FOREACH(DLAtom,a,atoms)
	{
		candidates = appendToTailOfList(candidates, MAKE_NODE_SET(copyObject(a)));
	}


	// expand candidates until they cover the target goals variables
	while(!MY_LIST_EMPTY(candidates))
	{
		Set *c = (Set *) popHeadOfListP(candidates);
		Set *vars = varNamesForAtoms(c);

		if(containsSet(vars, targetVars))
		{
			if(!hasSetElem(seeds, c))
			{
				addToSet(seeds, c);
			}
		}
		// candidate does not have enough vars, need to add more atoms
		else
		{
			List *extensionCandidates = removeListElementsFromAnotherList(copyList(atoms), makeNodeListFromSet(c));

			FOREACH(DLAtom,a,extensionCandidates)
			{
				Set *newC = copyObject(c);
				addToSet(newC, a);
				candidates = appendToTailOfList(candidates, newC);
			}
		}
	}

	return seeds;
}

static Set *
varNamesForAtom(DLAtom *atom)
{
	return varListToNameSet(getAtomExprVars(atom));
}

static Set *
topLevelVarNamesForAtom(DLAtom *atom)
{
	return varListToNameSet(getAtomTopLevelVars(atom));
}

static Set *
varNamesForAtoms(Set *atoms)
{
	Set *result = STRSET();

	FOREACH_SET(DLAtom,a,atoms)
	{
		result = unionSets(result, varListToNameSet(getAtomExprVars(a)));
	}

	return result;
}

static Set *
varListToNameSet(List *vars)
{
	Set *result = STRSET();

	FOREACH(DLVar,v,vars)
	{
		addToSet(result, strdup(v->name));
	}

	return result;
}

List *
adaptFDsToRules(DLProgram *p, DLRule *r, List *fds)
{
	List *result = NIL;
	HashMap *predToGoals = NEW_MAP(Constant,List);

	// map pred to goals for preds
	FOREACH(DLNode,n,r->body)
	{
		if(isA(n,DLAtom))
		{
			DLAtom *a = (DLAtom *) n;
		    MAP_ADD_STRING_KEY_TO_VALUE_LIST(predToGoals, a->rel, a, TRUE);
		}
	}

	FOREACH(FD,f,fds)
	{
		List *goals = (List *) MAP_GET_STRING(predToGoals, f->table);
		List *attrNames = predGetAttrNames(p, f->table);

		FOREACH(DLAtom,g,goals)
		{
			Set *newLHS = STRSET();
			Set *newRHS = STRSET();
			FD *newF;

			FOREACH_SET(char,a,f->lhs)
			{
				int pos = listPosString(attrNames, a);
				DLNode *n;

				if(pos < 0)
				{
					THROW(SEVERITY_RECOVERABLE,
						  "did not find attribute %s of FD %s in schema of relation %s",
						  a,
						  icToString((Node *) f),
						  stringListToString(attrNames)
						);
				}

				n = getNthOfListP(g->args, pos);
				if(isA(n,DLVar))
				{
					DLVar *v = (DLVar *) n;
					addToSet(newLHS, strdup(v->name));
				}
			}

			FOREACH_SET(char,a,f->rhs)
			{
				int pos = listPosString(attrNames, a);
				DLNode *n = getNthOfListP(g->args, pos);
				if(isA(n,DLVar))
				{
					DLVar *v = (DLVar *) n;
					addToSet(newRHS, strdup(v->name));
				}
			}

			newF = createFD(strdup(f->table), newLHS, newRHS);

			DEBUG_LOG("FD adapted to rule: %s", icToString((Node *) newF));

			result = appendToTailOfList(result, newF);
		}
	}

	return result;
}

boolean
checkFDonAtoms(Set *atoms, List *fds, FD *fd) //FIXME does ignore on which relations an FD holds which is not correct. Fix that
{
	Set *closure;

	closure = attributeClosureOnAtoms(atoms, fd->lhs, fds);

    return containsSet(fd->rhs, closure);
}

Set *
attributeClosureOnAtoms(Set *atoms, Set *attrs, List *fds)
{
	Set *aattrs = STRSET();
	Set *closure;
	Set *rels = STRSET();
	List *aFds;

	FOREACH_SET(DLAtom,a,atoms)
	{
		Set *vars = attrListToSet(getVarNames(getExprVars((Node *) a)));
		aattrs = unionSets(aattrs, vars);
		addToSet(rels, a->rel);
	}

	aFds = getFDsForAttributesOnRels(fds, aattrs, rels);
	closure = attributeClosure(aFds, attrs, NULL);

	return closure;
}



Graph *
createJoinGraph(DLRule *r)
{
	Set *nodes = NODESET();
	Graph *g;
	HashMap *atomVars = NEW_MAP(DLVar,Set);
	List *edges = NIL;

	FOREACH(DLNode,g,r->body)
	{
		if(isA(g,DLAtom))
		{
			DLAtom *a = (DLAtom *) g;

			addToSet(nodes, a);
			FOREACH(DLNode,arg,a->args)
			{
				if(isA(arg,DLVar))
				{
					if(hasMapKey(atomVars, (Node *) arg))
					{
						Set *ats = (Set *) getMap(atomVars, (Node *) arg);
						addToSet(ats, a);
					}
					else {
						addToMap(atomVars,
								 (Node *) arg,
								 (Node *) MAKE_NODE_SET(a));
					}
				}
			}
		}
	}

	// create edges between atoms that share variables
	FOREACH(DLNode,g,r->body)
	{
		if(isA(g,DLAtom))
		{
			DLAtom *a = (DLAtom *) g;

			FOREACH(Node,arg,a->args)
			{
				if(isA(arg,DLVar))
				{
					Set *atomsForVar = (Set *) getMap(atomVars, (Node *) arg);
					FOREACH_SET(DLAtom,end,atomsForVar)
					{
						if(!equal(a, end))
						{
							edges = appendToTailOfList(
								edges,
								createNodeKeyValue((Node *) a, (Node *) end));
						}
					}
				}
			}
		}
	}

	g = createGraph(nodes, edges);
    DEBUG_NODE_BEATIFY_LOG("generated join graph", g);
	return g;
}

DLRule *
minimizeDLRule(DLRule *r)
{
	DLRule *prev = r;
	DLRule *result = copyObject(r);
	boolean changed = TRUE;
	//TODO implement this Set *headVars = varListToNameSet(getHeadVars(r));

	while(changed)
	{



		if(equal(prev, result))
		{
			changed = FALSE;
		}
	}

	return result;
}
