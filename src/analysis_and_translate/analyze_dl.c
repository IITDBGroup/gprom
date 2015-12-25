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
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "model/rpq/rpq_model.h"
#include "rpq/rpq_to_datalog.h"

static void analyzeDLProgram (DLProgram *p);
static void analyzeRule (DLRule *r, Set *idbRels, Set *edbRels);
static void analyzeProv (DLProgram *p, KeyValue *kv);
static List * analyzeAndExpandRPQ (RPQQuery *q, List **rpqRules);
static boolean checkFact (DLAtom *f);


Node *
analyzeDLModel (Node *stmt)
{
    //TODO implement checks, e.g., edb relation arity, variable data types, which relations edb which are idb
    if (isA(stmt, DLProgram))
        analyzeDLProgram((DLProgram *) stmt);

    if (!checkDLModel(stmt))
        FATAL_LOG("failed model check on:\n%s", datalogToOverviewString(stmt));

    DEBUG_LOG("analyzed model is \n%s", nodeToString(stmt));
    INFO_LOG("analyzed model overview is \n%s", datalogToOverviewString(stmt));

    return stmt;
}

void
createRelToRuleMap (Node *stmt)
{
    if (!isA(stmt,DLProgram))
        return;

    HashMap *relToRule = NEW_MAP(Constant,List); // map idb relations to all rules that have this relation in their head
    DLProgram *p = (DLProgram *) stmt;

    FOREACH(Node,r,p->rules)
    {
        // a rule
        if(isA(r,DLRule))
        {
            char *headPred = getHeadPredName((DLRule *) r);
            List *relRules = (List *) MAP_GET_STRING(relToRule,headPred);

            relRules = appendToTailOfList(relRules, r);
            MAP_ADD_STRING_KEY(relToRule,headPred,relRules);
        }
    }

    setDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES, (Node *) relToRule);
}

static void
analyzeDLProgram (DLProgram *p)
{
    Set *idbRels = STRSET();
    Set *edbRels = STRSET();
//    HashMap *relToRule = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    List *rules = NIL;
    List *facts = NIL;
    List *rpqRules = NIL;

    // expand RPQ queries
    FOREACH(Node,r,p->rules)
    {
        if(isA(r,RPQQuery))
        {
            List *newRules;
            newRules = analyzeAndExpandRPQ ((RPQQuery *) r, &rpqRules);
            rpqRules = CONCAT_LISTS(rpqRules, newRules);
        }
        else
        {
            rpqRules = appendToTailOfList(rpqRules, r);
        }
    }
    p->rules = CONCAT_LISTS(rpqRules);


    createRelToRuleMap((Node *) p);
    //TODO infer data types for idb predicates

    FOREACH(Node,r,p->rules)
    {
        // a rule
        if(isA(r,DLRule))
        {
            char *headPred = getHeadPredName((DLRule *) r);

            rules = appendToTailOfList(rules, r);
            addToSet(idbRels, headPred);
        }
        // answer relation specification
        else if(isA(r,Constant))
        {
            p->ans = STRING_VALUE(r);
        }
        // fact
        else if(isA(r,DLAtom))
        {
            checkFact((DLAtom *) r);
            facts = appendToTailOfList(facts,r);
        }
        // provenance question
        else if(isA(r,KeyValue))
            analyzeProv(p, (KeyValue *) r);
        else
            FATAL_LOG("datalog programs can consists of rules, constants, an "
                    "answer relation specification, facts, and provenance "
                    "computations");
        //TODO check that atom exists and is of right arity and that only constants are used in the fact
    }

    // analyze all rules
    FOREACH(DLRule,r,rules)
        analyzeRule((DLRule *) r, idbRels, edbRels);

    p->rules = rules;
    p->facts = facts;

    // check that answer relation exists
    if (p->ans)
    {
        if (!hasSetElem(idbRels, p->ans))
            FATAL_LOG("no rules found for specified answer relation"
                    " %s in program:\n\n%s",
                    p->ans, datalogToOverviewString((Node *) p));
    }

    // store some auxiliary results of analysis in properties
    setDLProp((DLNode *) p, DL_IDB_RELS, (Node *) idbRels);
    setDLProp((DLNode *) p, DL_EDB_RELS, (Node *) edbRels);
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

//    // check safety
    if (!checkDLRuleSafety(r))
        FATAL_LOG("rule is not safe: %s",
                        datalogToOverviewString((Node *) r));

    //    if (!checkHeadSafety(r))
//        FATAL_LOG("head predicate is not safe: %s",
//                datalogToOverviewString((Node *) r));

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
            // check DTs
        }
    }
}

/*
 * Check that facts only use constants
 */
static boolean
checkFact (DLAtom *f)
{
    FOREACH(Node,arg,f->args)
    {
        if (!isConstExpr(arg))
            FATAL_LOG("datalog facts can only contain constant expressions: %s",
                    datalogToOverviewString((Node *) f));
    }
    return TRUE;
}

static List *
analyzeAndExpandRPQ (RPQQuery *q, List **rpqRules)
{
    DLProgram *rpqP = (DLProgram *) rpqQueryToDatalog(q);
    return rpqP->rules;
}
