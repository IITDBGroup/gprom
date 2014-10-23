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
static void analyzeRule (DLRule *r, Set *idbRels);
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
    HashMap *relToRule; // map idb relations to all rules that have this relation in their head
    List *rules = NIL;
    List *facts = NIL;

    FOREACH(Node,r,p->rules)
    {
        // a rule
        if(isA(r,DLRule))
        {
            rules = appendToTailOfList(rules, r);
            addToSet(idbRels, getHeadPredName((DLRule *) r));
        }
        if(isA(r,Constant))
        {
            p->ans = STRING_VALUE(r);
            //TODO check that it exists
        }
        // fact
        if(isA(r,DLAtom))
            facts = appendToTailOfList(facts,r);
        //TODO check that atom exists and is of right arity and that only constants are used in the fact
    }

    FOREACH(DLRule,r,rules)
        analyzeRule((DLRule *) r, idbRels);

    p->rules = rules;
    p->facts = facts;
}

static void
analyzeRule (DLRule *r, Set *idbRels)
{
    HashMap *varToPredMapping;

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
