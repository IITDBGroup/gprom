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
#include "metadata_lookup/metadata_lookup.h"

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
    Set *edbRels = STRSET();
    Set *factRels = STRSET();
    Set *edbOrFactRels = STRSET();

    // check facts
    FOREACH(DLAtom,f,p->facts)
    {
        if (!checkFact(f))
            return FALSE;
        addToSet(factRels, f->rel);
        if (!MAP_HAS_STRING_KEY(relArities, f->rel))
            MAP_ADD_STRING_KEY(relArities, strdup(f->rel),
                    createConstInt(LIST_LENGTH(f->args)));
    }

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

    // Gather edb rules and check their consistency
    FOREACH(DLRule,r,p->rules)
    {
        FOREACH(DLNode,a,r->body)
        {
            if (isA(a, DLAtom))
            {
                DLAtom *atom = (DLAtom *) a;
                char *relName = atom->rel;
                if (catalogTableExists(relName))
                {
                    addToSet(edbRels, relName);
                    if (LIST_LENGTH(getAttributes(relName)) != LIST_LENGTH(atom->args))
                        FATAL_LOG("Body atom %s uses edb relation %s but does "
                                "not have that same arity as this relation.",
                                datalogToOverviewString((Node *) atom), relName);
                }
                else if (!(hasSetElem(idbRels,relName) || hasSetElem(factRels,relName)))
                {
                    FATAL_LOG("Predicate of body atom %s is neither IDB nor EDB"
                            " (used in facts of present in the database)",
                            datalogToOverviewString((Node *) atom));
                }

            }
        }
    }

    // check for overlap among EDB and IDB
    edbOrFactRels = unionSets(edbRels,factRels);
    // check that idb and edb do not overlap
    if (overlapsSet(idbRels,edbOrFactRels))
    {
        FATAL_LOG("relation has to be either EDB or IDB:\n%s\n\n%s",
                nodeToString(idbRels), nodeToString(edbRels));
        return FALSE;
    }

    // third loop to check consistency of idb rels and arities
    Set *checkIdb = copyObject(idbRels);
    FOREACH(DLRule,r,p->rules)
    {
        int arity;

        removeSetElem(checkIdb,r->head->rel);
        arity = INT_VALUE(MAP_GET_STRING(relArities, r->head->rel));
        if (arity != LIST_LENGTH(r->head->args))
        {
            FATAL_LOG("arity of rel %s was supposed to be %u in rule:\n%s",
                    r->head->rel, arity, datalogToOverviewString((Node *) r));
            return FALSE;
        }

        FOREACH(DLNode,a,r->body)
        {
            if (isA(a, DLAtom))
            {
                DLAtom *at = (DLAtom *) a;

                // set predicate type for atom
                DL_DEL_PROP(at,DL_IS_IDB_REL);
                DL_DEL_PROP(at,DL_IS_EDB_REL);
                DL_DEL_PROP(at,DL_IS_FACT_REL);

                if (hasSetElem(idbRels,at->rel))
                    DL_SET_BOOL_PROP(at,DL_IS_IDB_REL);
                if (hasSetElem(edbRels,at->rel))
                    DL_SET_BOOL_PROP(at,DL_IS_EDB_REL);
                if (hasSetElem(factRels,at->rel))
                    DL_SET_BOOL_PROP(at,DL_IS_FACT_REL);

                DEBUG_LOG("fixed atom: %s", nodeToString((Node *) at));
//                if (DL_HAS_PROP(at, DL_IS_IDB_REL)
//                        && !hasSetElem(idbRels, at->rel))
//                {
//                    FATAL_LOG("atom marked as IDB is not using IDB predicate:\n%s",
//                            at->rel, datalogToOverviewString((Node *) r));
//                    return FALSE;
//                }

                // check arity
                arity = INT_VALUE(MAP_GET_STRING(relArities, at->rel));
                if (arity != LIST_LENGTH(at->args))
                {
                    FATAL_LOG("arity of rel %s was supposed to be %u in rule:\n%s",
                            at->rel, arity, datalogToOverviewString((Node *) r));
                    return FALSE;
                }
            }
        }
    }

    if (!EMPTY_SET(checkIdb))
        FATAL_LOG("The following IDB predicate(s) used in rule bodies do not "
                "occur in the head of any rule: %s", checkIdb);

    // answer relation appears as head of a rule
    if (p->ans && !hasSetElem(idbRels, p->ans))
    {
        FATAL_LOG("answer relation %s does not appear in the head of any rule"
                " in program:\n%s",
                p->ans, datalogToOverviewString((Node *) p));
        return FALSE;
    }

    // store some auxiliary results of analysis in properties
    setDLProp((DLNode *) p, DL_IDB_RELS, (Node *) idbRels);
    setDLProp((DLNode *) p, DL_EDB_RELS, (Node *) edbRels);
    setDLProp((DLNode *) p, DL_FACT_RELS, (Node *) factRels);

    return TRUE;
}

static boolean
checkDLRule (DLRule *r)
{
//    Set *headV = makeStrSetFromList(getVarNames(getHeadVars(r)));
//    Set *allV = makeStrSetFromList(getVarNames(getBodyVars(r)));
//
//    // check that head vars appear in body
//    FOREACH_SET(char,v,headV)
//        if(!hasSetElem(allV,v))
//        {
//            FATAL_LOG("did not find head var %s in body of rule:\n%s", v,
//                    datalogToOverviewString((Node *) r));
//        }



    return checkDLRuleSafety(r);
}

boolean
checkDLRuleSafety (DLRule *r)
{
    Set *posVars = STRSET();

    // gather variables used in positive relation atoms
    FOREACH(DLNode,d,r->body)
    {
         if (isA(d,DLAtom) && !((DLAtom *) d)->negated)
         {
             DLAtom *a = (DLAtom *) d;
             FOREACH(Node,n,a->args)
             {
                 if(isA(n,DLVar))
                 {
                     DLVar *v = (DLVar *) n;
                     addToSet(posVars,v->name);
                 }
             }
         }

    }

    // check that variables in negated atoms are also occur in positive atoms
    FOREACH(DLNode,d,r->body)
    {
        if (isA(d,DLAtom) && ((DLAtom *) d)->negated)
        {
            DLAtom *a = (DLAtom *) d;
            FOREACH(Node,n,a->args)
            {
                if(isA(n,DLVar))
                {
                    DLVar *v = (DLVar *) n;
                    if(!hasSetElem(posVars, v->name))
                    {
                        ERROR_LOG("Unsafe rule. Variable %s is only used in "
                                "negated subgoal in rule:\n%s",
                                v->name,
                                datalogToOverviewString((Node *) r));
                        return FALSE;
                    }
                }
            }
        }
    }

    // check that vars used in comparison atoms are also occur in positive atoms
    FOREACH(DLNode,d,r->body)
    {
        if (isA(d,DLComparison))
        {
            DLComparison *a = (DLComparison *) d;
            List *vars = getDLVarsIgnoreProps((Node *) a->opExpr);
            FOREACH(DLVar,v,vars)
            {
                if(!hasSetElem(posVars, v->name))
                {
                    ERROR_LOG("Unsafe rule. Variable %s is used in comparison atom "
                            "but not in positive subgoal in rule:\n%s",
                            v->name,
                            datalogToOverviewString((Node *) r));
                    return FALSE;
                }
            }
        }
    }

    // check that head is safe
    DLAtom *a = (DLAtom *) r->head;
    FOREACH(Node,n,a->args)
    {
        List *vars = getDLVarsIgnoreProps(n);
        FOREACH(DLVar,v,vars)
        {
            if(!hasSetElem(posVars, v->name))
            {
                ERROR_LOG("Unsafe rule. Variable %s is used in head atom "
                        "but not in positive subgoal in rule:\n%s",
                        v->name,
                        datalogToOverviewString((Node *) r));
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*
 * Check that facts only use constants
 */
boolean
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
