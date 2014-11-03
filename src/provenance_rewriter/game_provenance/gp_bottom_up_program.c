/*-----------------------------------------------------------------------------
 *
 * gp_bottom_up_program.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/datalog/datalog_model.h"
#include "provenance_rewriter/game_provenance/gp_bottom_up_program.h"

#define CONCAT_MAP_LIST(map,key,newList) \
    do { \
        List *old = (List *) getMap(map,key); \
        old = CONCAT_LISTS(old,newList); \
        addToMap(map,key,(Node *) old); \
    } while (0)

static DLProgram *createWhyGPprogram (DLProgram *p, DLAtom *why);
static DLProgram *createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot);
static DLProgram *createFullGPprogram (DLProgram *p);

static void enumerateRules (DLProgram *p);
static DLProgram *unifyProgram (DLProgram *p, DLAtom *question);
static void unifyOneRule(HashMap *pToR, HashMap *rToUn, DLAtom *curAtom);
static DLProgram *solveProgram (DLProgram *p, DLAtom *question, boolean neg);

DLProgram *
createBottomUpGPprogram (DLProgram *p)
{
    DEBUG_LOG("create GP bottom up program for:\n%s",
            beatify(nodeToString(p)));
    INFO_LOG("create GP bottom up program for:\n%s",
            datalogToOverviewString((Node *) p));
    // why provenance
    if(DL_HAS_PROP(p,DL_PROV_WHY))
    {
        DLAtom *why = (DLAtom *) getDLProp((DLNode *) p,DL_PROV_WHY);
        return createWhyGPprogram(p, why);
    }
    // why not
    else if(DL_HAS_PROP(p,DL_PROV_WHYNOT))
    {
        DLAtom *whyN = (DLAtom *) getDLProp((DLNode *) p,DL_PROV_WHYNOT);
        return createWhyNotGPprogram(p, whyN);
    }
    // full GP
    else if(DL_HAS_PROP(p,DL_PROV_FULL_GP))
        return createFullGPprogram(p);

    return p;
}

static DLProgram *
createWhyGPprogram (DLProgram *p, DLAtom *why)
{
    DLProgram *solvedProgram;
//    DLProgram *result;

    enumerateRules (p);
    solvedProgram = copyObject(p);
    unifyProgram(solvedProgram, why);
    solveProgram(solvedProgram, why, FALSE);

    p->n.properties = NULL;

    return p;
}

static DLProgram *
createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot)
{
    return p;
}

static DLProgram *
createFullGPprogram (DLProgram *p)
{
    return p;
}

static void
enumerateRules (DLProgram *p)
{
    int i = 0;

    FOREACH(DLRule,r,p->rules)
        setDLProp((DLNode *) r, DL_RULE_ID, (Node *) createConstInt(i++));
}

/*
 * Given a target atom for provenance computation Why(R(X)) unify rules in the
 *  program according to constants in X. This may result in multiple copies of
 *  each rule and relation node. E.g., if rules are
 *      Q(X) :- R(X,Y)
 *      Q(X) :- R(Y,X)
 *      R(X,Y) :- S(X,Y)
 *
 *  and the user question is Why(Q(1)), then we unify rules with X <- 1
 *  recursively which rules in two instances of the rule for R:
 *      Q(1) :- R(1,Y)
 *      Q(1) :- R(Y,1)
 *      R(1,Y) :- S(1,Y)
 *      R(Y,1) :- S(Y,1)
 *  We store which original rule a unified rule corresponds using properties
 */

static DLProgram *
unifyProgram (DLProgram *p, DLAtom *question)
{
    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    HashMap *predToUnRules = NEW_MAP(DLAtom, List);
    HashMap *newPredToRules = NEW_MAP(Constant,List);
//    List *predRules;
    List *newRules = NIL;
    DLProgram *newP = makeNode(DLProgram);

//    predRules = (List *) MAP_GET_STRING(predToRules, question->rel);

    // unify rule bodies starting with constants provided by the user query
    // e.g., Why(Q(1))
    unifyOneRule(predToRules, predToUnRules, question);

    // build new pred name to rules map
    FOREACH_HASH_ENTRY(kv,predToUnRules)
    {
        DLAtom *head = (DLAtom *) kv->key;
        List *rules = (List *) kv->value;
        char *predName = head->rel;

        CONCAT_MAP_LIST(newPredToRules,(Node *) createConstString(predName),rules);
        newRules = CONCAT_LISTS(newRules, rules);
    }

    // build new program based on generated rules
    newP->ans = strdup(p->ans);
    newP->facts = p->facts;
    newP->rules = newRules;
    newP->n.properties = copyObject(p->n.properties);

    setDLProp((DLNode *) newP, DL_MAP_RELNAME_TO_RULES, (Node *) newPredToRules);
    setDLProp((DLNode *) newP, DL_MAP_UN_PREDS_TO_RULES, (Node *) predToUnRules);

    DEBUG_LOG("unified program is:\n", beatify(nodeToString(newP)));
    INFO_LOG("unified program is:\n", datalogToOverviewString((Node *) newP));

    return p;
}

static void
unifyOneRule(HashMap *pToR, HashMap *rToUn, DLAtom *curAtom)
{
    List *vals = curAtom->args;
//    char *unRel = curAtom->rel;
    List *unRules = NIL;
    List *origRules;

    // get originalRules and if they exist unified rules
    origRules = (List *) MAP_GET_STRING(pToR, curAtom->rel);
    unRules = (List *) getMap(rToUn, (Node *) curAtom);

    DEBUG_LOG("unify head %s with rules:\n%s",
            datalogToOverviewString((Node *) curAtom),
            datalogToOverviewString((Node *) origRules));

    // if rules for unified head already exist, then return
    if (unRules)
        return;

    // otherwise process each rule to unify it, then process body atoms
    // recursively
    FOREACH(DLRule,r,origRules)
    {
        DLRule *un;
        int ruleId;

        ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        un = unifyRule(r,vals);
        unRules = appendToTailOfList(unRules, un);
        setDLProp((DLNode *) r,DL_ORIGINAL_RULE, (Node *) createConstInt(ruleId));

        DEBUG_LOG("unified rule with head %s\nRule: %s\nUnified Rules: %s",
                datalogToOverviewString((Node *) curAtom),
                datalogToOverviewString((Node *) r),
                datalogToOverviewString((Node *) un));
    }

    // store mapping of this head atom to all unified rules for it
    addToMap(rToUn, (Node *) curAtom, (Node *) unRules);

    FOREACH(DLRule,un,unRules)
    {
        FOREACH(DLAtom,a,un->body)
        {
            if (DL_HAS_PROP(a,DL_IS_IDB_REL))
            {
                boolean hasConst = FALSE;
                FOREACH(Node,arg,a->args)
                    if (isA(arg,Constant))
                        hasConst = TRUE;
                if (hasConst)
                    unifyOneRule(pToR,rToUn,a);
            }
        }
    }
}


/*
 * Determine which nodes in the game template (annotated program) will
 * be lost and which ones will be won based on the user query. Although, the
 * user question determines which nodes are won and lost simply based on their distance to
 *
 */

static DLProgram *
solveProgram (DLProgram *p, DLAtom *question, boolean neg)
{
    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    HashMap *unPredToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_UN_PREDS_TO_RULES);
    boolean unified = unPredToRules == NULL;
//    char *targetPred;
    // rules have been unified
    if (unified)
    {
        Set *won = NODESET();  // keep track of which nodes have been marked as won
        Set *lost = NODESET(); // or lost status
        List *todoStack = NIL;
        List *unHeads;

        // initialize stack with rule heads for user provenance question
        todoStack = copyList((List *)
                MAP_GET_STRING(predToRules,question->rel));

        // user question defines whether the starting predicate is lost or won
        FOREACH(DLRule,r,todoStack)
        {
            if (neg)
                addToSet(lost, r->head);
            else
                addToSet(won, r->head); //done
            addDLProp((DLNode *) r, neg ? DL_LOST : DL_WON,
                    (Node *) createConstBool(TRUE));
        }

        // recursivly set WON/LOST status
        while(todoStack)
        {

        }
    }
    // non-unified rules
    else
    {

    }

    return p;
}
