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
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/summarization_rewrites/summarize_main.h"
#include "model/rpq/rpq_model.h"
#include "rpq/rpq_to_datalog.h"
#include "utility/string_utils.h"

static void analyzeDLProgram (DLProgram *p);
static void analyzeSummerizationBasics (DLProgram *p);
static void analyzeSummarizationAdvanced (DLProgram *p);
static void analyzeRule (DLRule *r, Set *idbRels, DLProgram *p); // , Set *edbRels, Set *factRels);
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

    // extract summarization options
    analyzeSummerizationBasics(p);

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
        analyzeRule((DLRule *) r, idbRels, p); //, edbRels, factRels);

    p->rules = rules;
    p->facts = facts;
    p->doms = doms;

    /* for summarization, check whether
     * 1) the length of failure pattern is equal to the number of rule body atom
     * 2) the failure pattern is assigned with why question
     */
	analyzeSummarizationAdvanced(p);
    
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
analyzeSummerizationBasics (DLProgram *p)
{
    ProvQuestion qType = PROV_Q_WHY;

    // only continue if summarization was requested
    if (p->sumOpts == NIL)
        return;

	DEBUG_LOG("user asked for summarization");
	
    // either why or why-not
    FOREACH(Node,n,p->rules)
    {
        if(isA(n,KeyValue))
        {
            KeyValue *kv = (KeyValue *) n;
            char *qTypeStr = STRING_VALUE(kv->key);

            if(isPrefix(qTypeStr,"WHYNOT_"))
                qType = PROV_Q_WHYNOT;
            else
                qType = PROV_Q_WHY;
        }
    }

	// set the summarization type
    DL_SET_PROP(p, PROP_SUMMARIZATION_QTYPE, createConstInt(qType));

    // mark as summarization
    DL_SET_BOOL_PROP(p, PROP_SUMMARIZATION_DOSUM);

    // turn summarization options into properties
    FOREACH(Node,n,p->sumOpts)
    {
        ASSERT(isA(n,KeyValue));
        KeyValue *kv = (KeyValue *) n;
        ASSERT(isA(kv->key, Constant));
        Constant *key = (Constant *) kv->key;
        DL_SET_PROP(p, STRING_VALUE(key), kv->value);
    }

    // keep track of (var,rel) and (negidb,edb)
    HashMap *varRelPair = NEW_MAP(Constant,Constant);
    HashMap *headEdbPair = NEW_MAP(Constant,List);
    List *negAtoms = NIL;

    FOREACH(Node,n,p->rules)
    {
        if(isA(n,DLRule))
        {
            DLRule *r = (DLRule *) n;
            List *edbList = NIL;

            FOREACH(Node,b,r->body)
            {
                if(isA(b,DLAtom))
                {
                    DLAtom *a = (DLAtom *) b;

                    // keep track of which negated atom needs domains from which edb atom
                    if(a->negated)
                        negAtoms = appendToTailOfList(negAtoms,a->rel);
                    else
                        edbList = appendToTailOfList(edbList,a->rel);

                    // keep track of which variable belongs to which edb
                    FOREACH(Node,n,a->args)
                    {
                        if(isA(n,DLVar))
                        {
                            DLVar *v = (DLVar *) n;
                            MAP_ADD_STRING_KEY_AND_VAL(varRelPair,v->name,a->rel);
                        }
                    }
                }
            }

            char *headPred = getHeadPredName(r);
            MAP_ADD_STRING_KEY(headEdbPair,headPred,edbList);
        }
    }

    // store edb information for negated atoms and why-not questions
    if(!LIST_EMPTY(negAtoms))
    {
        FOREACH(char,c,negAtoms)
        {
            if(!MAP_HAS_STRING_KEY(headEdbPair,c))
                MAP_ADD_STRING_KEY_AND_VAL(varRelPair,c,c);
            else
            {
                List *edbs = (List *) MAP_GET_STRING(headEdbPair,c);

                FOREACH(char,e,edbs)
                MAP_ADD_STRING_KEY_AND_VAL(varRelPair,e,e);
            }
        }
    }

    if(LIST_EMPTY(negAtoms) || qType == PROV_Q_WHYNOT)
    {
        FOREACH_HASH(List,edbs,headEdbPair)
            FOREACH(char,e,edbs)
                MAP_ADD_STRING_KEY_AND_VAL(varRelPair,e,e);
    }

    // store into the list of the summarization options
    DL_SET_PROP(p, PROP_SUMMARIZATION_VARREL, varRelPair);

}

static void
analyzeSummarizationAdvanced (DLProgram *p)
{
	//TODO check this analyze code from main function
	int fpLeng = 0;
	boolean isFpattern = FALSE;
	HashMap *hm = p->n.properties;
	List *measureElemList = NIL;

	// check failure pattern assigned and if so then capture the length of the pattern
	FOREACH(KeyValue,kv,p->sumOpts)
	{
		char *key = STRING_VALUE(kv->key);

		if(streq(key,"fpattern"))
		{
			isFpattern = TRUE;
			fpLeng = LIST_LENGTH((List *) kv->value);
		}

		// for user defined score,
		// currenlty support 3 elements for measure: precision, recall, and informativeness
		char *measureElem = NULL;

		if(isPrefix(key,"sc_"))
		{
			measureElem = (char *) MALLOC(strlen(key) + 1);
			strcpy(measureElem,key);

			measureElem = strEndTok(measureElem,"_");
			measureElemList = appendToTailOfList(measureElemList,measureElem);

			if(!(streq(measureElem,"PRECISION") || streq(measureElem,"RECALL") || streq(measureElem,"INFORMATIVENESS")))
				FATAL_LOG("incorrect element for the measure! must be one of three: precision, recall, and informativeness!");
		}

//			if(isSubstr(key,"score"))
//			{
//				measureElem = replaceSubstr(measureElem,"score_","");
//				measureElemList = splitString(measureElem,"_");
//
//				FOREACH(char,c,measureElemList)
//					if(!(streq(c,"PRECISION") || streq(c,"RECALL") || streq(c,"INFORMATIVENESS")))
//						FATAL_LOG("incorrect element for the measure! must be one of three: precision, recall, and informativeness!");
//			}

		// the elements in the thresholds must be same as in the score
		if(isPrefix(key,"th_"))
		{
			measureElem = (char *) MALLOC(strlen(key) + 1);
			strcpy(measureElem,key);

			measureElem = strEndTok(measureElem,"_");

//				if(measureElemList != NIL)
//				{
//					if(!searchListString(measureElemList, measureElem))
//						FATAL_LOG("the element for thresholds must be consistent with those in the defined score");
//				}
//				else
//				{
			if(!(streq(measureElem,"PRECISION") || streq(measureElem,"RECALL") || streq(measureElem,"INFORMATIVENESS")))
				FATAL_LOG("incorrect element for the measure! must be one of three: precision, recall, and informativeness!");
//				}
		}
	}

	// failure pattern assigned with why question
	if(isFpattern && MAP_HAS_STRING_KEY(hm,"WHY_PROV"))
		FATAL_LOG("no failure can happen with WHY question");

//        // no failure pattern assigned with whynot question
//        if(!isFpattern && MAP_HAS_STRING_KEY(hm,"WHYNOT_PROV"))
//        	FATAL_LOG("failure pattern must be assigned with WHYNOT question for summarization");

	// the length of failure pattern must be equal to the number of body atoms
	char *ansPred = NULL;
	int bodyLeng = 0;

	if(MAP_HAS_STRING_KEY(hm,"WHYNOT_PROV") && isFpattern)
	{
		DLAtom *ansAtom = (DLAtom *) getMapString(hm,"WHYNOT_PROV");
		ansPred = (char *) ansAtom->rel;

		FOREACH(Node,r,p->rules)
		{
			if(isA(r,DLRule))
			{
				DLRule *eachR = (DLRule *) r;
				char *headPred = getHeadPredName(eachR);

				if(streq(headPred,ansPred))
					bodyLeng = LIST_LENGTH(eachR->body);
			}
		}

		if(fpLeng > bodyLeng)
			FATAL_LOG("the failure pattern is longer than the rule body");

		if(fpLeng < bodyLeng)
			FATAL_LOG("the failure pattern is less than the rule body");
	}
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
        else if (isSuffix(type, DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE_REDUCED))
        {
            DL_SET_STRING_PROP(p, DL_PROV_FORMAT, DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE_REDUCED);
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
analyzeRule (DLRule *r, Set *idbRels, DLProgram *p) // , Set *edbRels, Set *factRels)
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

        if (isA(a,DLComparison))
        {
        	p->comp = appendToTailOfList(p->comp, a);

        	INFO_DL_LOG("comparison expression:", p->comp);
			DEBUG_LOG("comparison expression:\n%s", datalogToOverviewString(p->comp));
        }
    }

    // check head if agg functions exist
    FOREACH(Node,ha,r->head->args)
    	if (isA(ha,FunctionCall))
    		p->func = appendToTailOfList(p->func, ha);
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


//static void
//summarizationPlan (Node *parse)
//{
//    // store options for the summarization and question type
//    if (isA(parse, List) && isA(getHeadOfListP((List *) parse), ProvenanceStmt))
//    {
//        ProvenanceStmt *ps = (ProvenanceStmt *) getHeadOfListP((List *) parse);
//
//    	if (!LIST_EMPTY(ps->sumOpts))
//    		FOREACH(Node,n,ps->sumOpts)
//    			summOpts = appendToTailOfList(summOpts,n);
//
//    	qType = "WHY";
//    }
//    else // summarization options for DL input
//    {
//    	DLProgram *p = (DLProgram *) parse;
//
//    	// either why or why-not
//    	FOREACH(Node,n,p->rules)
//    	{
//    		if(isA(n,KeyValue))
//    		{
//    			KeyValue *kv = (KeyValue *) n;
//    			qType = STRING_VALUE(kv->key);
//
//    			if(isPrefix(qType,"WHYNOT_"))
//    				qType = "WHYNOT";
//    			else
//    				qType = "WHY";
//    		}
//    	}
//
//    	if (p->sumOpts != NIL)
//    	{
//    		FOREACH(Node,n,p->sumOpts)
//				summOpts = appendToTailOfList(summOpts,n);
//
//    		// keep track of (var,rel) and (negidb,edb)
//    		HashMap *varRelPair = NEW_MAP(Constant,Constant);
//    		HashMap *headEdbPair = NEW_MAP(Constant,List);
//    		List *negAtoms = NIL;
//
//    		FOREACH(Node,n,p->rules)
//    		{
//    			if(isA(n,DLRule))
//    			{
//    				DLRule *r = (DLRule *) n;
//            		List *edbList = NIL;
//
//    				FOREACH(Node,b,r->body)
//    				{
//    					if(isA(b,DLAtom))
//    					{
//    						DLAtom *a = (DLAtom *) b;
//
//    						// keep track of which negated atom needs domains from which edb atom
//    						if(a->negated)
//    							negAtoms = appendToTailOfList(negAtoms,a->rel);
//    						else
//           						edbList = appendToTailOfList(edbList,a->rel);
//
//    						// keep track of which variable belongs to which edb
//    						FOREACH(Node,n,a->args)
//    						{
//    							if(isA(n,DLVar))
//    							{
//    								DLVar *v = (DLVar *) n;
//    								MAP_ADD_STRING_KEY_AND_VAL(varRelPair,v->name,a->rel);
//    							}
//    						}
//    					}
//    				}
//
//        			char *headPred = getHeadPredName(r);
//    				MAP_ADD_STRING_KEY(headEdbPair,headPred,edbList);
//    			}
//    		}
//
//    		// store edb information for negated atoms and why-not questions
//    		if(!LIST_EMPTY(negAtoms))
//    		{
//        		FOREACH(char,c,negAtoms)
//        		{
//        			if(!MAP_HAS_STRING_KEY(headEdbPair,c))
//        				MAP_ADD_STRING_KEY_AND_VAL(varRelPair,c,c);
//        			else
//        			{
//        				List *edbs = (List *) MAP_GET_STRING(headEdbPair,c);
//
//        				FOREACH(char,e,edbs)
//        					MAP_ADD_STRING_KEY_AND_VAL(varRelPair,e,e);
//        			}
//        		}
//    		}
//
//    		if(LIST_EMPTY(negAtoms) || streq(qType,"WHYNOT"))
//    		{
//				FOREACH_HASH(List,edbs,headEdbPair)
//					FOREACH(char,e,edbs)
//						MAP_ADD_STRING_KEY_AND_VAL(varRelPair,e,e);
//    		}
//
//    		// store into the list of the summarization options
//    		summOpts = appendToTailOfList(summOpts, (Node *) varRelPair);
//    	}
//    }
//}
