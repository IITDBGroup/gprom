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
#include "utility/string_utils.h"

static void analyzeDLProgram (DLProgram *p);
static void analyzeRule (DLRule *r, Set *idbRels); // , Set *edbRels, Set *factRels);
static void analyzeProv (DLProgram *p, KeyValue *kv);
static List *analyzeAndExpandRPQ (RPQQuery *q, List **rpqRules);


Node *
analyzeDLModel (Node *stmt)
{
    //TODO implement checks, e.g., edb relation arity, variable data types, which relations edb which are idb
    if (isA(stmt, DLProgram))
        analyzeDLProgram((DLProgram *) stmt);

    if (!checkDLModel(stmt))
        FATAL_LOG("failed model check on:\n%s", datalogToOverviewString(stmt));

    DEBUG_NODE_BEATIFY_LOG("analyzed model is:", stmt);
    INFO_DL_LOG("analyzed model overview is:", stmt);

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
//    Set *edbRels = STRSET();
//    Set *factRels = STRSET();
    List *rules = NIL;
    List *facts = NIL;
    List *rpqRules = NIL;
    List *doms = NIL;

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
            DLAtom *f = (DLAtom *) r;
            checkFact(f);
//            addToSet(factRels, f->rel);
            facts = appendToTailOfList(facts,r);
        }
        // associate domain
        else if(isA(r,DLDomain))
        	doms = appendToTailOfList(doms,r);
        // provenance question
        else if(isA(r,KeyValue))
            analyzeProv(p, (KeyValue *) r);
        else
            FATAL_LOG("datalog programs can consists of rules, constants, an "
                    "answer relation specification, facts, associate domain specification, "
            		"and provenance computations");
    }

    // analyze all rules
    FOREACH(DLRule,r,rules)
        analyzeRule((DLRule *) r, idbRels); //, edbRels, factRels);

    p->rules = rules;
    p->facts = facts;
    p->doms = doms;

//    // check that answer relation exists
//    if (p->ans)
//    {
//        if (!hasSetElem(idbRels, p->ans))
//            FATAL_LOG("no rules found for specified answer relation"
//                    " %s in program:\n\n%s",
//                    p->ans, datalogToOverviewString((Node *) p));
//    }
}

static void
analyzeProv (DLProgram *p, KeyValue *kv)
{
    char *type;
    ASSERT(isA(kv->key, Constant));

    // get provenance type
    type = STRING_VALUE(kv->key);
    if (isPrefix(type,DL_PROV_WHY))
    {
        setDLProp((DLNode *) p,DL_PROV_WHY, kv->value);
    }
    //TODO check that is WHY prove
    if (isPrefix(type,DL_PROV_WHYNOT))
    {
        setDLProp((DLNode *) p,DL_PROV_WHYNOT, kv->value);
    }
    if (isPrefix(type,DL_PROV_FULL_GP))
    {
        DL_SET_BOOL_PROP(p,DL_PROV_FULL_GP);
    }

    // check if format is given
    if (!streq(type,DL_PROV_WHY) && !streq(type,DL_PROV_WHYNOT) && !streq(type,DL_PROV_FULL_GP))
    {
        if (isSuffix(type, DL_PROV_FORMAT_GP))
        {
            DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_GP);
        }
        else if (isSuffix(type, DL_PROV_FORMAT_GP_REDUCED))
        {
            DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_GP_REDUCED);
        }
        else if (isSuffix(type, DL_PROV_FORMAT_TUPLE_ONLY))
        {
            DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_TUPLE_ONLY);
        }
        else if (isSuffix(type, DL_PROV_FORMAT_TUPLE_RULE_TUPLE))
        {
            DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_TUPLE_RULE_TUPLE);
        }
        else if (isSuffix(type, DL_PROV_FORMAT_HEAD_RULE_EDB))
		{
			DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_HEAD_RULE_EDB);
		}
        else if (isSuffix(type, DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE))
        {
            DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE);
        }
        else if (isSuffix(type, DL_PROV_FORMAT_TUPLE_RULE_TUPLE_REDUCED))
		{
			DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_TUPLE_RULE_TUPLE_REDUCED);
		}
        else
        {
            FATAL_LOG("unkown provenance return format: %s", type);
        }
    }
    // otherwise default is full game provenance graph
    else
    {
        DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_GP);
    }
}

static void
analyzeRule (DLRule *r, Set *idbRels) // , Set *edbRels, Set *factRels)
{
    // check safety
    if (!checkDLRuleSafety(r))
        FATAL_LOG("rule is not safe: %s",
                        datalogToOverviewString((Node *) r));

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
//            // else edb, check that exists and has right arity
//            else
//            {
//                boolean isFactRel = hasSetElem (factRels,atom->rel);
//                boolean isEdbRel = catalogTableExists(atom->rel);
//                if (isEdbRel)
//                {
//                    addToSet(edbRels,atom->rel);
//                }
//                if(!(isFactRel || isEdbRel))
//                    FATAL_LOG("Atom uses predicate %s that is neither IDB nor EDB", atom->rel);
//            }
        }
    }
}

/*
 * Analyse and expand an RPQ expression. RPQ(path_expr,resul_type,edge_relation,result_relation).
 */
static List *
analyzeAndExpandRPQ (RPQQuery *q, List **rpqRules)
{
    DLProgram *rpqP = (DLProgram *) rpqQueryToDatalog(q);
    return rpqP->rules;
}
