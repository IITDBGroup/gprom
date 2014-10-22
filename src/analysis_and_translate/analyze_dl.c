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
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/datalog/datalog_model.h"


static void analyzeDLProgram (DLProgram *p);
static void analyzeRule (DLRule *r);
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
    Set *idbRels;
    Set *edbRels;
    HashMap *relToRule; // map idb relations to all rules that have this relation in their head

}

static void
analyzeRule (DLRule *r)
{
    HashMap *varToPredMapping;

    // check that head predicate is not a edb relation

    // add head predicate to idb (if not already exists)

    // check edb datatypes and arities

    // analyze comparison atoms and set


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
