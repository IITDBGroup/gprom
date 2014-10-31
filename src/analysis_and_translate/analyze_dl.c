/*-----------------------------------------------------------------------------
 *
 * analyze_dl.c
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
#include "analysis_and_translate/analyze_dl.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/datalog/datalog_model.h"


static void analyzeDLProgram (DLProgram *p);
static void analyzeRule (DLRule *r, Set *idbRels, Set *edbRels);
static void analyzeProv (DLProgram *p, KeyValue *kv);
static boolean checkHeadSafety (DLRule *r);

Node *
analyzeDLModel (Node *stmt)
{
    //TODO implement checks, e.g., edb relation arity, variable data types, which relations edb which are idb
    if (isA(stmt, DLProgram))
        analyzeDLProgram((DLProgram *) stmt);

    return stmt;
}

static void
analyzeDLProgram (DLProgram *p)
{
    Set *idbRels = STRSET();
    Set *edbRels = STRSET();
    HashMap *relToRule = NEW_MAP(Constant,List); // map idb relations to all rules that have this relation in their head
    List *rules = NIL;
    List *facts = NIL;
//    List *provComps = NIL;

    //TODO infer data types for idb predicates

    FOREACH(Node,r,p->rules)
    {
        // a rule
        if(isA(r,DLRule))
        {
            char *headPred = getHeadPredName((DLRule *) r);
            List *relRules = (List *) MAP_GET_STRING(relToRule,headPred);

            rules = appendToTailOfList(rules, r);
            addToSet(idbRels, headPred);
            relRules = appendToTailOfList(relRules, r);
            MAP_ADD_STRING_KEY(relToRule,headPred,relRules);
        }
        else if(isA(r,Constant))
        {
            p->ans = STRING_VALUE(r);
            //TODO check that it exists
        }
        // fact
        else if(isA(r,DLAtom))
            facts = appendToTailOfList(facts,r);
        // provenance question
        else if(isA(r,KeyValue))
            analyzeProv(p, (KeyValue *) r);
        else
            FATAL_LOG("datalog programs can consists of rules, constants, an "
                    "answer relation specification, facts, and provenance "
                    "computations");
        //TODO check that atom exists and is of right arity and that only constants are used in the fact
    }

    FOREACH(DLRule,r,rules)
        analyzeRule((DLRule *) r, idbRels, edbRels);

    p->rules = rules;
    p->facts = facts;

    // store some auxiliary results of analysis in properties
    setDLProp((DLNode *) p, DL_IDB_RELS, (Node *) idbRels);
    setDLProp((DLNode *) p, DL_EDB_RELS, (Node *) edbRels);
    setDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES, (Node *) relToRule);
}

static void
analyzeProv (DLProgram *p, KeyValue *kv)
{
    char *type;
    ASSERT(isA(kv->key, Constant));

    type = STRING_VALUE(kv->key);
    if (streq(type,DL_PROV_WHY))
    {
        setDLProp((DLNode *) p,DL_PROV_WHY, kv->value);
        //TODO check that is WHY prove
    }
    if (streq(type,DL_PROV_WHYNOT))
    {
        setDLProp((DLNode *) p,DL_PROV_WHYNOT, kv->value);
        //TODO check that is WHY prove
    }
    if (streq(type,DL_PROV_FULL_GP))
    {
        DL_SET_BOOL_PROP(p,DL_PROV_FULL_GP);
    }
}

static void
analyzeRule (DLRule *r, Set *idbRels, Set *edbRels)
{
//    HashMap *varToPredMapping;

    if (!checkHeadSafety(r))
        FATAL_LOG("head predicate is not safe: %s",
                datalogToOverviewString((Node *) r));

    // check that head predicate is not a edb relation

    // check body
    FOREACH(Node,a,r->body)
    {
        if (isA(a,DLAtom))
        {
            DLAtom *atom = (DLAtom *) a;
            // is idb relation
            if (hasSetElem(idbRels,atom->rel))
            {
                DL_SET_BOOL_PROP(atom,DL_IS_IDB_REL);
            }
            // else edb, check that exists and has right arity
            else
            {
                addToSet(edbRels,atom->rel);
                if(!catalogTableExists(atom->rel))
                    FATAL_LOG("EDB atom %s does not exist", atom->rel);
            }
        }
        if (isA(a, DLComparison))
        {

        }
    }
}

/*
 * Check whether all head variables occur in the body
 */
static boolean
checkHeadSafety (DLRule *r)
{
    DLAtom *h = r->head;
    Set *bodyVars = makeNodeSetFromList(getBodyPredVars(r));

    // foreach variable
    FOREACH(Node,n,h->args)
    {
        if (isA(n, DLVar))
        {
            if(!hasSetElem(bodyVars,n))
                return FALSE;
        }
        else
            FATAL_LOG("for now only variables accepted in the head");
    }

    return TRUE;
}
