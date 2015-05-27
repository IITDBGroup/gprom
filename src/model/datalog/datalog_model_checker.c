/*-----------------------------------------------------------------------------
 *
 * datalog_model_checker.c
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
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"

static boolean checkDLProgram (DLProgram *p);
static boolean checkDLRule (DLRule *r);

boolean
checkDLModel (Node *dlModel)
{
    // check for specific DL node types
    if (isA(dlModel, DLProgram))
        return checkDLProgram ((DLProgram *) dlModel);
    if (isA(dlModel, DLRule))
        return checkDLRule ((DLRule *) dlModel);

    // no known DL node type
    FATAL_LOG("Expected a datalog model elements, but was\n%s",
            beatify(nodeToString(dlModel)));
    return FALSE;
}

static boolean
checkDLProgram (DLProgram *p)
{
    HashMap *relArities = NEW_MAP(Constant,Constant);
    Set *idbRels = STRSET();

    // check internal consistency of each rule
    FOREACH(DLRule,r,p->rules)
    {
        // check rule itself
        if (!checkDLRule(r))
            return FALSE;

        // collect idb relations and arities
        addToSet(idbRels, strdup(r->head->rel));
        if (!MAP_HAS_STRING_KEY(relArities, r->head->rel))
            MAP_ADD_STRING_KEY(relArities, strdup(r->head->rel),
                    createConstInt(LIST_LENGTH(r->head->args)));

        FOREACH(DLNode,a,r->body)
        {
            if (isA(a, DLAtom))
            {
                DLAtom *at = (DLAtom *) a;

                if (!MAP_HAS_STRING_KEY(relArities, at->rel))
                    MAP_ADD_STRING_KEY(relArities, strdup(at->rel),
                            createConstInt(LIST_LENGTH(at->args)));
            }
        }
    }

    // second loop to check consistency of idb rels and arities
    FOREACH(DLRule,r,p->rules)
    {
        int arity;

        arity = INT_VALUE(MAP_GET_STRING(relArities, r->head->rel));
        if (arity != LIST_LENGTH(r->head->args))
        {
            FATAL_LOG("arity of rel %s was supposed to be %u in rul:\n%s",
                    r->head->rel, arity, datalogToOverviewString((Node *) r));
            return FALSE;
        }

        FOREACH(DLNode,a,r->body)
        {
            if (isA(a, DLAtom))
            {
                DLAtom *at = (DLAtom *) a;

                // idb check
                if (DL_HAS_PROP(at, DL_IS_IDB_REL)
                        && !hasSetElem(idbRels, at->rel))
                {
                    FATAL_LOG("arity of rel %s was supposed to be %u in rul:\n%s",
                            at->rel, arity, datalogToOverviewString((Node *) r));
                    return FALSE;
                }

                // check arity
                arity = INT_VALUE(MAP_GET_STRING(relArities, at->rel));
                if (arity != LIST_LENGTH(at->args))
                {
                    FATAL_LOG("arity of rel %s was supposed to be %u in rul:\n%s",
                            at->rel, arity, datalogToOverviewString((Node *) r));
                    return FALSE;
                }
            }
        }
    }

    // answer relation appears as head of a rule
    if (p->ans && !hasSetElem(idbRels, p->ans))
    {
        FATAL_LOG("answer relation %s does not appear in the head of any rule"
                " in program:\n%s",
                p->ans, datalogToOverviewString((Node *) p));
        return FALSE;
    }

    //TODO check that answer relation appears in head

    //TODO check that idb relations appear at least in the head of one rule

    return TRUE;
}

static boolean
checkDLRule (DLRule *r)
{
    Set *headV = makeStrSetFromList(getVarNames(getHeadVars(r)));
    Set *allV = makeStrSetFromList(getVarNames(getBodyVars(r)));

    // check that head vars appear in body
    FOREACH_SET(char,v,headV)
        if(!hasSetElem(allV,v))
        {
            FATAL_LOG("did not find head var %s in body of rule:\n%s", v,
                    datalogToOverviewString((Node *) r));
        }


    return TRUE;
}
