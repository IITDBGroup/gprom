/*-----------------------------------------------------------------------------
 *
 * translator_dl.c
 *
 *
 *		AUTHOR: lord_pretzel & seokki
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "analysis_and_translate/analyze_dl.h"
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "log/logger.h"

#include "analysis_and_translate/translator_dl.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/query_operator_dt_inference.h"
#include "model/query_operator/operator_property.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/game_provenance/gp_main.h"
#include "provenance_rewriter/summarization_rewrites/summarize_main.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"
#include "utility/string_utils.h"

static Node *translateProgram(DLProgram *p);
static QueryOperator *translateFact(DLAtom *f);
static QueryOperator *translateRule(DLRule *r);
static QueryOperator *translateSafeRule(DLRule *r);
static QueryOperator *translateUnSafeRule(DLRule *r);
static QueryOperator *translateUnSafeGoal(DLAtom *r, int goalPos);
static QueryOperator *translateSafeGoal(DLAtom *r, int goalPos, QueryOperator *posPart);
static QueryOperator *translateHeadAggregation(QueryOperator *joinedGoals, List *headArgs);

//static void analyzeProgramDTs (DLProgram *p, HashMap *predToRules);
static void analyzeFactDTs(DLAtom *f, HashMap *predToDTs);
static void analyzeRuleDTs(DLRule *r, HashMap *predToDTs, HashMap *predToRules);
static void setVarDTs(Node *expr, HashMap *varToDT);
static QueryOperator *joinGoalTranslations(DLRule *r, List *goalTrans);
static Node *createJoinCondOnCommonAttrs(QueryOperator *l, QueryOperator *r, List *leftOrigAttrs);
static List *getHeadProjectionExprs(DLAtom *head, QueryOperator *joinedGoals, List *bodyArgs);
static Node *replaceDLVarMutator(Node *node, HashMap *vToA);
static Node *createCondFromComparisons(List *comparisons, QueryOperator *in, HashMap *varDTmap);
static List *connectProgramTranslation(DLProgram *p, HashMap *predToTrans);
static boolean adaptProjectionAttrRef(QueryOperator *o, void *context);

static Node *replaceVarWithAttrRef(Node *node, List *context);

boolean provQ = FALSE;
static List *negBoolDone = NIL;


Node *
translateParseDL(Node *q)
{
    Node *result = NULL;
    DLProgram *p = (DLProgram *) q;
    char *ans = p->ans;
    boolean doSumm = DL_HAS_PROP(p, PROP_SUMMARIZATION_DOSUM) && !IS_GP_PROV(p);
	ProvQuestion qType = PROV_Q_WHY;
	HashMap *props = copyObject(p->n.properties);

//

	if(doSumm)
		qType = (ProvQuestion) INT_VALUE(DL_GET_PROP(p, PROP_SUMMARIZATION_QTYPE));

    // check if ans exists
    if(ans != NULL)
    	provQ = TRUE;

    INFO_LOG("translate DL model:\n\n%s", datalogToOverviewString(q));

//    if (!checkDLModel(q))
//        FATAL_LOG("failed model check on:\n%s", datalogToOverviewString(q));

    if (isA(q,DLProgram))
        result = translateProgram((DLProgram *) q);
    // what other node types can be here?
    else
        FATAL_LOG("currently only DLProgram node type translation supported");

    INFO_OP_LOG("translated DL model before casting:\n", result);

    // apply casts where necessary
    if (isA(result, List))
    {
        List *translation = (List *) result;
        FOREACH(QueryOperator,c,translation)
        {
            introduceCastsWhereNecessary(c);
            ASSERT(checkModel(c));
        }
    }
    else if (IS_OP(result))
    {
        introduceCastsWhereNecessary((QueryOperator *) result);
        ASSERT(checkModel((QueryOperator *) result));
    }
    INFO_OP_LOG("translated DL model:\n", result);

	if (doSumm)
	{
		DEBUG_LOG("add relational algebra summarization code");
		MAP_ADD_STRING_KEY(props, PROP_SUMMARIZATION_IS_DL, createConstBool(TRUE));
		result = rewriteSummaryOutput(result, props, qType);
		INFO_OP_LOG("translated DL model with summarization:\n", result);
	}

    return result;
}

QueryOperator *
translateQueryDL(Node *node)
{
    // dummy implementation, DL will not have query nodes
    return NULL;
}

#define APPEND_TO_MAP_VALUE_LIST(map,key,elem) \
    do { \
        if(MAP_HAS_STRING_KEY((map),(key))) \
        { \
            KeyValue *_kv = MAP_GET_STRING_ENTRY((map),(key)); \
            List *_valList = (List *) _kv->value; \
            _valList = appendToTailOfList(_valList, (elem)); \
            _kv->value = (Node *) _valList; \
        } \
        else \
        { \
            List *_valList = singleton((elem)); \
            MAP_ADD_STRING_KEY((map),(key),_valList); \
        } \
    } while(0)

static Node *
translateProgram(DLProgram *p)
{
    Node *answerRel;
    List *origHeadPred = NIL;
	List *translation = NIL; // list of sinks of a relational algebra graph
	HashMap *predToTrans = NEW_MAP(Constant,List);
	HashMap *predToRules = NEW_MAP(Constant,List);

	// if we want to compute the provenance then construct program
	// for creating the provenance and translate this one
	if (IS_GP_PROV(p))
	{
		DEBUG_LOG("user asked for provenance computation for:\n%s",
				  datalogToOverviewString((Node *) p));

		Node *gpComp = rewriteForGP((Node *) p);

		ASSERT(!IS_GP_PROV(gpComp));

		checkDLModel(gpComp);
		gpComp = translateParseDL(gpComp);

		return gpComp;
	}

	if (IS_LINEAGE_PROV(p))
	{
		Node *comp;
		DLProgram *pl = rewriteDLForLinageCapture(p);

		pl->rules = CONCAT_LISTS(pl->rules, pl->facts);
		pl = (DLProgram *) analyzeDLModel((Node *) pl);
		comp = translateParseDL((Node*) pl);

		return comp;
	}

	// determine pred -> rules
	FOREACH(DLRule,r,p->rules)
	{
		char *headPred = getHeadPredName(r);
		APPEND_TO_MAP_VALUE_LIST(predToRules,headPred,r);

		// store orig head pred
		if(DL_HAS_PROP(r,DL_ORIGINAL_RULE))
			origHeadPred = appendToTailOfList(origHeadPred, createConstString(headPred));

	}
	FOREACH(DLAtom,f,p->facts)
	{
		char *relName = f->rel;
		APPEND_TO_MAP_VALUE_LIST(predToRules,relName,f);
	}

	// analyze rules to determine data types
	analyzeProgramDTs(p, predToRules);

	// translate facts
	FOREACH(DLAtom,f,p->facts)
	{
		QueryOperator *tFact = translateFact(f);
		char *predName = f->rel;

		DEBUG_LOG("translate fact: %s", datalogToOverviewString((Node *) f));
		APPEND_TO_MAP_VALUE_LIST(predToTrans,predName,tFact);
	}

	// translate rules
	FOREACH(DLRule,r,p->rules)
	{
		QueryOperator *tRule = translateRule(r);
		char *headPred = getHeadPredName(r);

		DEBUG_LOG("translate rule: %s", datalogToOverviewString((Node *) r));

		if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
		{
			introduceCastsWhereNecessary((QueryOperator *) tRule);
			ASSERT(checkModel((QueryOperator *) tRule));
		}
		APPEND_TO_MAP_VALUE_LIST(predToTrans,headPred,tRule);
	}

	// for each predicate create a union between all translated rules
	// replace the lists with a reference to the top-most union op
	FOREACH_HASH_ENTRY(kv,predToTrans)
	{
		List *rTs = (List *) kv->value;
		QueryOperator *un = (QueryOperator *) popHeadOfListP(rTs);

		FOREACH(QueryOperator,o,rTs)
		{
			QueryOperator *prev = un;
			un = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(prev,o), NIL,
													 getQueryOperatorAttrNames(un));
			addParent(prev, un);
			addParent(o, un);
		}

		// if union is used, then add duplicate removal
		if (LIST_LENGTH(rTs) >= 1)
		{
			QueryOperator *old = un;
			un = (QueryOperator *) createDuplicateRemovalOp(NULL, (QueryOperator *) un, NIL,
															getQueryOperatorAttrNames((QueryOperator *) un));
			addParent(old, un);
		}

		kv->value = (Node *) un;
	}

	// exclude the edge rel and domain rules before connect
	Node *transMoveRel = NULL;
	List *domHeadPreds = NIL;

	if (!MY_LIST_EMPTY(p->sumOpts) && p->ans != NULL)
	{
		transMoveRel = MAP_GET_STRING(predToTrans, "move");
		removeMapStringElem(predToTrans, "move");

		// remove domain query translated for WHYNOT
		FOREACH(DLRule,r,p->rules)
			if(DL_HAS_PROP(r,DL_DOMAIN_RULE))
				domHeadPreds = appendToTailOfList(domHeadPreds,r->head->rel);

		FOREACH(char,c,domHeadPreds)
			removeMapStringElem(predToTrans, c);
	}

	// connect rules by replacing table access operators representing idb
	// relations with the corresponding algebra expression
	translation = connectProgramTranslation(p, predToTrans);

	// generate input for the summarization
	if (!MY_LIST_EMPTY(p->sumOpts) && p->ans != NULL)
	{
		Node *origInRule = NULL;
		Node *ruleFire = NULL;
		List *summInputs = NIL;

		// check whether the ans rel is sinlge or multiple
		if(!isSubstr(p->ans,"-"))
		{
			ruleFire = MAP_GET_STRING(predToTrans, p->ans);
			origInRule = MAP_GET_STRING(predToTrans, STRING_VALUE(getHeadOfListP(origHeadPred)));
			QueryOperator *origIntoFire = (QueryOperator *) ruleFire;
			origIntoFire->properties = copyObject(origInRule);
			summInputs = singleton(origIntoFire);
		}
		else
		{
			List *ansRels = splitString(p->ans,"-");
			int i = 0;

			FOREACH(char,c,ansRels)
			{
				ruleFire = MAP_GET_STRING(predToTrans, c);
				origInRule = MAP_GET_STRING(predToTrans, STRING_VALUE(getNthOfListP(origHeadPred,i)));
				QueryOperator *origIntoFire = (QueryOperator *) ruleFire;
				origIntoFire->properties = copyObject(origInRule);
				summInputs = appendToTailOfList(summInputs, origIntoFire);
				i++;
			}
		}

		summInputs = appendToTailOfList(summInputs, transMoveRel);
		return (Node *) summInputs;
	}

	if (p->ans == NULL)
		return (Node *) translation;

	if (!MAP_HAS_STRING_KEY(predToTrans, p->ans))
	{
		FATAL_LOG("answer relation %s does not exist in program:\n\n%s",
				  p->ans, datalogToOverviewString((Node *) p));
	}

	answerRel = MAP_GET_STRING(predToTrans, p->ans);
	if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
	{
		introduceCastsWhereNecessary((QueryOperator *) answerRel);

	}

    return answerRel;
}

void
analyzeProgramDTs (DLProgram *p, HashMap *predToRules)
{
    HashMap *predToDTs = NEW_MAP(Constant,List);

    // determine fact data types
    FOREACH(DLAtom,f,p->facts)
        analyzeFactDTs(f, predToDTs);

    // determine data types
    FOREACH(DLRule,r,p->rules)
        analyzeRuleDTs(r, predToDTs, predToRules);

    // set properties on goals for rule translation
    FOREACH(DLRule,r,p->rules)
    {
        FOREACH(DLNode,a,r->body)
        {
            if (isA(a,DLAtom))
            {
                DLAtom *atom = (DLAtom *) a;
                List *dts = (List *) MAP_GET_STRING(predToDTs,atom->rel);
                ASSERT(dts != NIL);

                setDLProp((DLNode *) a,DL_PRED_DTS,(Node *) dts);
            }
        }
    }

    // set DT properties for all variables in rules
    FOREACH(DLRule,r,p->rules)
    {
        HashMap *varToDT = NEW_MAP(Constant,Constant);

        FOREACH(DLNode,a,r->body)
        {
            if (isA(a,DLAtom))
            {
                DLAtom *atom = (DLAtom *) a;
                List *dts = (List *) MAP_GET_STRING(predToDTs,atom->rel);
                ASSERT(dts != NIL);

                // scan through arguments and their data types to determine datatypes of variables
                FORBOTH_LC(argLc,dtLc,atom->args,dts)
                {
                    Node *arg = LC_P_VAL(argLc);
                    DataType dt = (DataType) LC_INT_VAL(dtLc);

                    if (isA(arg, DLVar))
                    {
                        DLVar *v = (DLVar *) arg;
                        MAP_ADD_STRING_KEY(varToDT, v->name, createConstInt((int) dt));
                    }
                }
            }
        }

        setDLProp((DLNode *) r,DL_PRED_DTS,(Node *) varToDT);
    }

    DEBUG_NODE_BEATIFY_LOG("analyzed DTs for datalog program before translation", p);
}

static void
analyzeFactDTs (DLAtom *f, HashMap *predToDTs)
{
    List *dts = NIL;

    FOREACH(Node,arg,f->args)
        dts = appendToTailOfListInt(dts, typeOf(arg));

    setDLProp((DLNode *) f, DL_PRED_DTS, (Node *) dts);
    MAP_ADD_STRING_KEY(predToDTs, f->rel, dts);
}

static void
analyzeRuleDTs (DLRule *r, HashMap *predToDTs, HashMap *predToRules)
{
    HashMap *varToDT = NEW_MAP(Constant,Constant);

    // determine goal dts
    FOREACH(Node,n,r->body)
    {
        if (isA(n,DLAtom))
        {
            DLAtom *a = (DLAtom *) n;
            List *dts = NIL;

            if (MAP_HAS_STRING_KEY(predToDTs,a->rel))
                dts = (List *) MAP_GET_STRING(predToDTs, a->rel);
            else
            {
                // idb rel
                if (DL_HAS_PROP(a,DL_IS_IDB_REL))
                {
                    List *rules = (List *) MAP_GET_STRING(predToRules, a->rel);

                    // analyze rules
                    FOREACH(DLRule,pRule,rules)
                        analyzeRuleDTs(pRule, predToDTs, predToRules);

                    // now we should have dts available
                    dts = (List *) MAP_GET_STRING(predToDTs, a->rel);
                }
                // edb rels
                else
                {
                    // add DTs if not done already
                    dts = getAttributeDataTypes(a->rel);
                    MAP_ADD_STRING_KEY(predToDTs, strdup(a->rel), dts);
                }
            }
            ASSERT(dts != NIL);


            // set var -> dt mappings
            int i = 0;
            FOREACH(Node,arg,a->args)
            {
                if(isA(arg, DLVar))
                {
                    DLVar *v = (DLVar *) arg;
                    MAP_ADD_STRING_KEY(varToDT, v->name,
                            createConstInt(getNthOfListInt(dts, i)));
                }

                if(isA(arg, Operator))
                {
                	Operator *o = (Operator *) arg;
                	DLVar *v = (DLVar *) getHeadOfListP(o->args);
                    MAP_ADD_STRING_KEY(varToDT, v->name,
                            createConstInt(getNthOfListInt(dts, i)));
                }

                i++;
            }
        }
    }

    // determine head dts
    List *headDTs = NIL;
//    setVarDTs((Node *) r->head->args, varToDT);
    setVarDTs((Node *) r, varToDT);

    // deal with negated boolean arg for dts
    List *headArgs = NIL;
    FOREACH(Node,arg,r->head->args)
    {
    	if(isA(arg, Operator))
    	{
    		Operator *o = (Operator *) arg;
    		headArgs = appendToTailOfList(headArgs, (Node *) getHeadOfListP(o->args));
    	}
    	else
    		headArgs = appendToTailOfList(headArgs, arg);
    }

    FOREACH(Node,arg,headArgs)
        headDTs = appendToTailOfListInt(headDTs, typeOf(arg));

    setDLProp((DLNode *) r->head, DL_PRED_DTS, (Node *) headDTs);
    MAP_ADD_STRING_KEY(predToDTs, r->head->rel, headDTs);
}

static void
setVarDTs (Node *expr, HashMap *varToDT)
{
    List *vars = getDLVarsIgnoreProps (expr);
//    List *vars = getDLVars (expr);

//	if (getBoolOption(OPTION_WHYNOT_ADV))
//	{
//		FOREACH(Node,n,vars)
//		{
//			DLVar *v;
//
//			if(isA(n,DLVar))
//				v = (DLVar *) n;
//
//			if(isA(n,Operator))
//			{
//				Operator *o = (Operator *) n;
//				v = (DLVar *) getHeadOfListP(o->args);
//			}
//
//	    	v->dt = INT_VALUE(MAP_GET_STRING(varToDT,v->name));
//		}
//	}
//	else
//	{
	    FOREACH(DLVar,v,vars)
	    	v->dt = INT_VALUE(MAP_GET_STRING(varToDT,v->name));
//	}
}

static QueryOperator *
translateFact(DLAtom *f)
{
    QueryOperator *result;
    List *attrNames = NIL;
    List *dts = NIL;
    int pos = 0;

    FOREACH(Node,arg,f->args)
    {
        dts = appendToTailOfListInt(dts, typeOf(arg));
        attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("c", gprom_itoa(pos++)));
    }

    result = (QueryOperator *) createConstRelOp(copyObject(f->args), NIL, attrNames, dts);

    return result;
}

static QueryOperator *
translateRule(DLRule *r)
{
    boolean isSafe = FALSE; //TODO implement real safety check in model checker or analyzer

    FOREACH(DLNode,d,r->body) //TODO assume that if we have at least one positive goal then we are safe for now
    {
        if (isA(d,DLAtom) && !((DLAtom *) d)->negated)
            isSafe = TRUE;
    }

    if(isSafe)
        return translateSafeRule(r);
    else
    	return translateUnSafeRule(r);
}

/*
 * Create algebra expression for one datalog rule. This treats both idb and edb
 *  relations the same by representing them as relation accesses. A rule will be
 *  translated as follows:
 *
 *  1) natural joins between goal translations
 *  2) comparison atoms are translated into a selection on top of that
 *  3) the rule head will be a projection followed by duplicate removal (it's bag semantics, baby!)
 */
static QueryOperator *
translateUnSafeRule(DLRule *r)
{
    ProjectionOperator *headP;
    DuplicateRemoval *dupRem;
    QueryOperator *joinedGoals;
    SelectionOperator *sel = NULL;
    List *goalTrans = NIL;
    List *conditions = NIL;
    int goalPos = 0;
    HashMap *varDTmap;

    DEBUG_LOG("translate rules: %s", datalogToOverviewString((Node *) r));

    varDTmap = (HashMap *) getDLProp((DLNode *) r, DL_PRED_DTS);

    // translate goals
    FOREACH(Node,a,r->body)
    {
        if (isA(a,DLAtom))
        {
            QueryOperator *tG = translateUnSafeGoal((DLAtom *) a, goalPos++);
            goalTrans = appendToTailOfList(goalTrans, tG);
            DEBUG_LOG("translated body goal is: %s", operatorToOverviewString((Node *) tG));
        }
        else if (isA(a,DLComparison))
            conditions = appendToTailOfList(conditions, a);
        else
            FATAL_LOG("datalog rule should only contain atoms and comparison ops");
    }

    // add joins to connect goal translations
    joinedGoals = joinGoalTranslations(r, goalTrans);
    DEBUG_LOG("joined goals are: %s", operatorToOverviewString((Node *) joinedGoals));

    // create selection from comparison expression in the rule
    if (!MY_LIST_EMPTY(conditions))
    {
        Node *cond = createCondFromComparisons(conditions, joinedGoals, varDTmap);
        sel = createSelectionOp(cond, joinedGoals, NIL, NULL);
        addParent(joinedGoals, (QueryOperator *) sel);
    }

    // create projection to simulate head
    List *projExprs = NIL;
    List *headNames = NIL;

    projExprs = getHeadProjectionExprs(r->head, joinedGoals, getBodyArgs(r));
    int i = 0;

//    FOREACH(Node,p,projExprs)
    for (; i < LIST_LENGTH(projExprs); i++)
        headNames = appendToTailOfList(headNames,CONCAT_STRINGS("A", gprom_itoa(i)));
//
//    List *headVars = getVarNames(getHeadVars(r));
//    List *headArgs = r->head->args;
//
//    FOREACH(char,hV,headVars)
//    {
//        AttributeReference *a;
//        int pos = getAttrPos(joinedGoals, hV);
//        DataType dt = getAttrDefByPos(joinedGoals, pos)->dataType;
//
//        a = createFullAttrReference(hV,0,pos,INVALID_ATTR, dt);
//        projExprs = appendToTailOfList(projExprs, a);
//    }

    headP = createProjectionOp(projExprs,
            sel ? (QueryOperator *) sel : joinedGoals,
            NIL,
            headNames);
    addParent(sel ? (QueryOperator *) sel : joinedGoals, (QueryOperator *) headP);

    // add duplicate removal operator
    dupRem = createDuplicateRemovalOp(NULL, (QueryOperator *) headP, NIL,
            getQueryOperatorAttrNames((QueryOperator *) headP));
    addParent((QueryOperator *) headP, (QueryOperator *) dupRem);

    DEBUG_LOG("translated rule:\n%s\n\ninto\n\n%s",
            datalogToOverviewString((Node *) r),
            operatorToOverviewString((Node *) dupRem));

    return (QueryOperator *) dupRem;
}



/*
 * Create algebra expression for one datalog rule. This treats both idb and edb
 *  relations the same by representing them as relation accesses. A rule will be
 *  translated as follows:
 *
 *  1) natural joins between goal translations
 *  2) comparison atoms are translated into a selection on top of that
 *  3) the rule head will be a projection followed by duplicate removal (it's bag semantics, baby!)
 *  4) if aggregation functions are menionted in one of the head expressions, then we grouping on the other expressions (no dup rem required)
 */
static QueryOperator *
translateSafeRule(DLRule *r)
{
    ProjectionOperator *headP;
    DuplicateRemoval *dupRem;
    QueryOperator *joinedGoals;
	QueryOperator *ruleRoot;
    SelectionOperator *sel = NULL;
    List *goalTrans = NIL;
    List *posGoals = NIL;
    List *conditions = NIL;
    QueryOperator *posPart = NULL;
    int goalPos;
    HashMap *varDTmap;
	boolean hasAgg;

    DEBUG_LOG("translate rules: %s", datalogToOverviewString((Node *) r));

    varDTmap = (HashMap *) getDLProp((DLNode *) r, DL_PRED_DTS);

    // translate positive goals and comparisons
    goalPos = 0;
    FOREACH(Node,a,r->body)
    {
        if (isA(a,DLAtom))
        {
            if (!((DLAtom *) a)->negated)
            {
                QueryOperator *tG = translateSafeGoal((DLAtom *) a, goalPos, NULL);
                posGoals = appendToTailOfList(posGoals, tG);
                DEBUG_LOG("translated positive body goal is: %s", operatorToOverviewString((Node *) tG));
            }
            goalPos++;
        }
        else if (isA(a,DLComparison))
            conditions = appendToTailOfList(conditions, a);
    }

    posPart = joinGoalTranslations(r, posGoals);
    joinedGoals = posPart;

    // create selection from comparison expression in the rule
    if (!MY_LIST_EMPTY(conditions))
    {
        Node *cond = createCondFromComparisons(conditions, joinedGoals, varDTmap);
        sel = createSelectionOp(cond, joinedGoals, NIL, NULL);
        addParent(joinedGoals, (QueryOperator *) sel);
        joinedGoals = (QueryOperator *) sel;
    }

    goalTrans = singleton(joinedGoals);

    // translate negated goals
    goalPos = 0;
    FOREACH(Node,a,r->body)
    {
        if (isA(a,DLAtom))
        {
            if (((DLAtom *) a)->negated)
            {
                QueryOperator *tG = translateSafeGoal((DLAtom *) a, goalPos, joinedGoals);
                goalTrans = appendToTailOfList(goalTrans, tG);
                DEBUG_LOG("translated negative body goal is: %s", operatorToOverviewString((Node *) tG));
            }
            goalPos++;
        }
    }

    // add joins to connect goal translations
    joinedGoals = joinGoalTranslations(r, goalTrans);
    DEBUG_LOG("joined goals are: %s", operatorToOverviewString((Node *) joinedGoals));


    // create projection to simulate head
    List *projExprs = NIL;
    List *headNames = NIL;

	projExprs = getHeadProjectionExprs(r->head, joinedGoals, getBodyArgs(r));

    // check whether agg functions exist
    // check if there exist aggr functions
	hasAgg = hasAggFunction((Node *) r->head->args);

    // translate agg functions
    if(hasAgg)
    {
		ruleRoot = translateHeadAggregation(joinedGoals, projExprs);
    }
	// no aggregation, create projection and duplicate removal
    else
    {
		for(int i = 0; i < LIST_LENGTH(projExprs); i++)
			headNames = appendToTailOfList(headNames,CONCAT_STRINGS("A", gprom_itoa(i)));

		headP = createProjectionOp(projExprs,
								   joinedGoals,
								   NIL,
								   headNames);
		addParent(joinedGoals, (QueryOperator *) headP);

		// add duplicate removal operator
		dupRem = createDuplicateRemovalOp(NULL, (QueryOperator *) headP, NIL,
										  getQueryOperatorAttrNames((QueryOperator *) headP));
		addParent((QueryOperator *) headP, (QueryOperator *) dupRem);

		ruleRoot = (QueryOperator *) dupRem;
    }

    DEBUG_LOG("translated rule:\n%s\n\ninto\n\n%s",
            datalogToOverviewString((Node *) r),
            operatorToOverviewString((Node *) ruleRoot));

    return (QueryOperator *) ruleRoot;
}


static QueryOperator *
translateHeadAggregation(QueryOperator *joinedGoals, List *headProj)
{
    	List *groupByClause = NIL;
        List *attrNames = NIL;
		List *headNames = NIL;
        List *groupbyNames = NIL;
        List *funcNames = NIL;
		List *aggExprs = NIL;
		AggregationOperator *ao;
		QueryOperator *finalProjection;
		/* List *newProjExprs = NIL; */
        int aggri = -1;
        int groupbyi = -1;

        // gather group by expressions and aggregation function expression and anmes
        FOREACH(Node,e,headProj) //TODO support expressions in head mixed with agg in the future, e.g., sum(X+1) * 2, Y + Z
        {
        	if(isA(e,AttributeReference))
        	{
				char *groupbyName = CONCAT_STRINGS("GROUP_", gprom_itoa(groupbyi + 1));

        		groupByClause = appendToTailOfList(groupByClause, copyObject(e));
        		groupbyNames = appendToTailOfList(groupbyNames, groupbyName);
				attrNames = appendToTailOfList(attrNames, groupbyName);
				groupbyi++;
        	}
			else if (isAggFunction(e))
			{
				char *aggrName = CONCAT_STRINGS("AGGR_", gprom_itoa(aggri + 1));

				aggExprs = appendToTailOfList(aggExprs, e);
				funcNames = appendToTailOfList(funcNames, aggrName);
				attrNames = appendToTailOfList(attrNames, aggrName);
				aggri++;
			}
			else
			{
				THROW(SEVERITY_RECOVERABLE,
					  "currently only aggregation function calls applied to variables and variables (GROUP-BY) are allowed in head predicates of rules that use aggregation:\n%s",
					  beatify(nodeToString(headProj)));
			}
        }

        // gather aggr function expressions
    	/* FOREACH(Node,e,projExprs) */
        /* { */
        /* 	if(isA(e,FunctionCall)) */
        /* 	{ */
        /* 		FunctionCall *f = (FunctionCall *) e; */

        /* 		if(f->isAgg) */
        /* 		{ */
        /* 			aggrFuncs = appendToTailOfList(aggrFuncs, copyObject(f)); */

        /* 			char *aggrName = CONCAT_STRINGS("AGGR_", gprom_itoa(aggri + 1)); */
        /* 			funcNames = appendToTailOfList(funcNames, aggrName); */
        /* 		} */
        /* 	} */
        /* } */

    	// create aggregation operator
		ao = createAggregationOp(aggExprs, groupByClause, joinedGoals, NIL, CONCAT_LISTS(funcNames,groupbyNames));
		OP_LCHILD(ao)->parents = singleton(ao);
//		addParent(joinedGoals, (QueryOperator *) ao);

		DEBUG_NODE_BEATIFY_LOG("agg operator", ao);

		// create new projExprs for projection operator
		/* AttributeReference *a; */
		/* groupbyi = 0; */
		/* aggri = 0; */

		for(int i = 0; i < LIST_LENGTH(attrNames); i++)
			headNames = appendToTailOfList(headNames,CONCAT_STRINGS("A", gprom_itoa(i)));

		finalProjection = createProjOnAttrsByName((QueryOperator *) ao,
												  attrNames,
												  headNames);
		addChildOperator(finalProjection, (QueryOperator *) ao);

		DEBUG_NODE_BEATIFY_LOG("agg operator", ao);

		/* FOREACH(char,aname,attrNames) */
		/* { */

		/* } */

		/* // To re-order the attributes, create expressions with group by first */
		/* FOREACH(AttributeDef,n,ao->op.schema->attrDefs) */
		/* { */
		/* 	if(isPrefix(n->attrName,"GROUP_")) */
		/* 	{ */
		/* 		a = createFullAttrReference(strdup(n->attrName), 0, groupbyi, -1, n->dataType); */
		/* 		newProjExprs = appendToTailOfList(newProjExprs, a); */
		/* 	} */
		/* 	groupbyi++; */
		/* } */

		/* // Then, create aggr function expressions */
		/* FOREACH(AttributeDef,n,ao->op.schema->attrDefs) */
		/* { */
		/* 	if(isPrefix(n->attrName,"AGGR_")) */
		/* 	{ */
		/* 		a = createFullAttrReference(strdup(n->attrName), 0, aggri, -1, n->dataType); */
		/* 		newProjExprs = appendToTailOfList(newProjExprs, a); */
		/* 	} */
		/* 	aggri++; */
		/* } */

		/* // Re-order attribute names according to the expressions */
		/* headNames = CONCAT_LISTS(copyList(groupbyNames),copyList(funcNames)); */

		return finalProjection;
}

static List *
getHeadProjectionExprs(DLAtom *head, QueryOperator *joinedGoals, List *bodyArgs)
{
    List *headArgs = head->args;
    List *projExprs = NIL;
    HashMap *vToA = NEW_MAP(Constant,List);
    int pos = 0;

    //TODO assume same length = have constants there, assume not same length = have removed constants
    if (LIST_LENGTH(bodyArgs) == LIST_LENGTH(joinedGoals->schema->attrDefs))
    {
        FORBOTH(Node,bA,a,bodyArgs,joinedGoals->schema->attrDefs)
        {
            if (isA(bA, DLVar) || isA(bA, Operator))
            {
                DLVar *v = NULL;

                if(isA(bA, DLVar))
                	v = (DLVar *) bA;

                if(isA(bA, Operator))
                {
                	Operator *o = (Operator *) bA;
                	Node *n = (Node *) getHeadOfListP(o->args);

                	if(isA(n, DLVar))
                		v = (DLVar *) n;
                }

                if(v != NULL)
                {
                    AttributeDef *d = (AttributeDef *) a;

                    if(streq(v->name,d->attrName))
                    {
    					MAP_ADD_STRING_KEY(vToA, v->name,(Node *) LIST_MAKE(
    										createConstString(d->attrName),
    										createConstInt(pos),
    										createConstInt(d->dataType)));
                    }
                }
            }

            pos++;
        }
    }
    else
    {
        FORBOTH(Node,bA,a,bodyArgs,joinedGoals->schema->attrDefs)
        {
            while(FOREACH_HAS_MORE(bA) && !isA(bA, DLVar))
            {
                DUMMY_LC(bA) = DUMMY_LC(bA)->next;
                bA = (Node *) LC_P_VAL(DUMMY_LC(bA));
            }

            ASSERT(isA(bA, DLVar));

			DLVar *v = (DLVar *) bA;
			AttributeDef *d = (AttributeDef *) a;
			MAP_ADD_STRING_KEY(vToA, v->name,(Node *) LIST_MAKE(
								   createConstString(d->attrName),
								   createConstInt(pos),
								   createConstInt(d->dataType)));

            pos++;
        }
    }

	// translate head args
    FOREACH(Node,a,headArgs)
    {
    	boolean isNegBool = FALSE;

    	if(isA(a,Operator) && typeOf(a) == DT_BOOL)
    		isNegBool = TRUE;

        Node *newA = replaceDLVarMutator(a, vToA);

		//TODO check what this is used for and remove if not needed
        if(isNegBool)
        {
        	AttributeReference *ar = (AttributeReference *) newA;

        	if(!searchListString(negBoolDone,ar->name))
        		newA = (Node *) createOpExpr(OPNAME_not, LIST_MAKE(newA));

        	/*
        	 *  only at first appearance, keep it as an operator expression for neg-bool
        	 *  otherwise, the boolean value will keep switch by CASE WHEN clause
        	 */

    		Operator *o = (Operator *) a;
    		DLVar *dv = (DLVar *) getHeadOfListP(o->args);

    		if(!searchListString(negBoolDone,dv->name))
    			negBoolDone = appendToTailOfList(negBoolDone, dv->name);
        }

        projExprs = appendToTailOfList(projExprs, newA);
    }

    return projExprs;
}

static Node *
replaceDLVarMutator(Node *node, HashMap *vToA)
{
    if (node == NULL)
        return node;

    if(isA(node, DLVar))
	{
		DLVar *v = (DLVar *) node;
		AttributeReference *a;
		List *l = (List *) MAP_GET_STRING(vToA, v->name);
		char *name = STRING_VALUE(getNthOfListP(l,0));
		int pos = INT_VALUE(getNthOfListP(l,1));
		DataType dt = (DataType) INT_VALUE(getNthOfListP(l,2));

		a = createFullAttrReference(name,0,pos,INVALID_ATTR, dt);

		return (Node *) a;
	}

    /* if (isA(node, DLVar) || isA(node, Operator)) */
    /* { */
    /*     AttributeReference *a; */
    /*     char *hV = NULL; */

    /*     if(isA(node, DLVar)) */
    /*     	hV = ((DLVar *) node)->name; */

    /*     if(isA(node, Operator)) */
    /*     { */
    /*     	Operator *o = (Operator *) node; */
	/* 		Node *n = (Node *) getHeadOfListP(o->args); */

	/* 		if(isA(n, DLVar)) */
	/* 			hV = ((DLVar *) n)->name; */
    /*     } */

    /*     if(hV != NULL) */
    /*     { */
    /*         List *l = (List *) MAP_GET_STRING(vToA, hV); */
    /*         char *name = STRING_VALUE(getNthOfListP(l,0)); */
    /*         int pos = INT_VALUE(getNthOfListP(l,1)); */
    /*         DataType dt = (DataType) INT_VALUE(getNthOfListP(l,2)); */

    /*         a = createFullAttrReference(name,0,pos,INVALID_ATTR, dt); */
    /*         return (Node *) a; */
    /*     } */
    /* } */

    return mutate(node, replaceDLVarMutator, vToA);
}

static QueryOperator *
joinGoalTranslations (DLRule *r, List *goalTrans)
{
    QueryOperator *result = (QueryOperator *) popHeadOfListP(goalTrans);
    Set *allNames = STRSET();
    List *origAttrs = getQueryOperatorAttrNames(result);

    // find attribute names
    FOREACH(QueryOperator,g,goalTrans)
		FOREACH(char,a,getQueryOperatorAttrNames(g))
			addToSet(allNames,a);

    FOREACH(QueryOperator,g,goalTrans)
    {
        JoinOperator *j;
        Node *cond;
        List *attrNames = CONCAT_LISTS(getQueryOperatorAttrNames(result),
                getQueryOperatorAttrNames(g));
        makeNamesUnique(attrNames, allNames);

        // TODO: if many variables(attrs) exist in the rule,
        //		 then creating unique var name by adding another digit of number with incremental is not enough
        /* if (LIST_LENGTH(attrNames) > 10) */
        /* 	makeNamesUnique(attrNames, allNames); */

        cond = createJoinCondOnCommonAttrs(result,g, origAttrs);

        j = createJoinOp(cond ? JOIN_INNER : JOIN_CROSS, cond, LIST_MAKE(result,g), NIL, attrNames);
        addParent(result, (QueryOperator *) j);
        addParent(g, (QueryOperator *) j);

        result =  (QueryOperator *) j;
        origAttrs = CONCAT_LISTS(origAttrs, getQueryOperatorAttrNames(g));
    }

    return result;
}

static Node *
createJoinCondOnCommonAttrs (QueryOperator *l, QueryOperator *r, List *leftOrigAttrs)
{
    Node *result = NULL;
    List *leftAttrs =  getQueryOperatorAttrNames(l);
    List *rightAttrs = getQueryOperatorAttrNames(r);
    Set *lAttrs = makeStrSetFromList(leftOrigAttrs);
    Set *rAttrs = makeStrSetFromList(rightAttrs);
    Set *commonAttrs = intersectSets(lAttrs,rAttrs);
    int lPos;
    int rPos;

    DEBUG_LOG("common attrs are:\n%s", nodeToString(commonAttrs));

    rPos = 0;
    FOREACH(char,rA,rightAttrs)
    {
        // attribute in right
        if (hasSetElem(commonAttrs,rA))
        {
            lPos = 0;
            FORBOTH(char,lA,origL,leftAttrs,leftOrigAttrs)
            {
                // found same attribute
                if (streq(origL,rA))
                {
                    Operator *op;
                    AttributeReference *lAr;
                    AttributeReference *rAr;
                    char *lName = strdup(lA);
                    char *rName = strdup(rA);

                    lAr = createFullAttrReference(lName, 0, lPos, INVALID_ATTR,
                            getAttrDefByPos(l,lPos)->dataType);
                    rAr = createFullAttrReference(rName, 1, rPos, INVALID_ATTR,
                            getAttrDefByPos(r,rPos)->dataType);
                    op = createOpExpr(OPNAME_EQ, LIST_MAKE(lAr,rAr));

                    if (result == NULL)
                        result = (Node *) op;
                    else
                        result = AND_EXPRS(result, op);
                }
                lPos++;
            }
        }
        rPos++;
    }
//
//    FOREACH_SET(char,a,commonAttrs)
//    {
//        Operator *op;
//        AttributeReference *lA;
//        AttributeReference *rA;
//        int lPos = getAttrPos(l,a);
//        int rPos = getAttrPos(r,a);
//        char *lName;
//        char *rName;
//
//        lA = createFullAttrReference(lName, 0, lPos, INVALID_ATTR,
//                getAttrDefByPos(l,lPos)->dataType);
//        rA = createFullAttrReference(rName, 1, rPos, INVALID_ATTR,
//                getAttrDefByPos(r,rPos)->dataType);
//        op = createOpExpr(OPNAME_EQ, LIST_MAKE(lA,rA));
//
//        if (result == NULL)
//            result = (Node *) op;
//        else
//            result = AND_EXPRS(result, op);
//    }

    return result;
}



/*
 *
 */
static Node *
createCondFromComparisons (List *comparisons, QueryOperator *in, HashMap *varDTmap)
{
    Node *result = NULL;
    List *attrNames = getQueryOperatorAttrNames(in);
    List *vars = getDLVarsIgnoreProps((Node *) comparisons);
//    List *vars = getDLVars((Node *) comparisons);

    // set correct data types
    FOREACH(DLVar,v,vars)
    {
        DataType dt = (DataType) INT_VALUE(MAP_GET_STRING(varDTmap, v->name));
        v->dt = dt;
    }

    // create condition as conjunction of all conditions and replace variable with attribute references
    FOREACH(DLComparison,d,comparisons)
    {
        Node *newC = replaceVarWithAttrRef(copyObject(d->opExpr), attrNames);
        Node *leftIn = getNthOfListP(d->opExpr->args,0);
        Node *rightIn = getNthOfListP(d->opExpr->args,1);

        if (isA(rightIn,Constant) && (CONST_IS_NULL(rightIn)))
        {
            if (streq(d->opExpr->name,OPNAME_EQ))
                newC = (Node *) createIsNullExpr(leftIn);
            else if (streq(d->opExpr->name,"!="))
                newC = (Node *) createOpExpr(OPNAME_NOT, singleton(createIsNullExpr(leftIn)));
        }

        if (result == NULL)
            result = newC;
        else
            result = AND_EXPRS(result,newC);
    }


    return result;
}

static Node *
replaceVarWithAttrRef(Node *node, List *context)
{
    if (node == NULL)
        return NULL;

    if (isA(node, DLVar))
    {
        DLVar *v = (DLVar *) node;
        int pos = listPosString(context, v->name);
        AttributeReference *a = createFullAttrReference(strdup(v->name),
                0,
                pos,
                INVALID_ATTR,
                v->dt);
        return (Node *) a;
    }

    return mutate(node, replaceVarWithAttrRef, context);
}

/*
 * Translate a datalog rule goal for relation R into
 *  1) a relation access R if it is a positive goal
 *  2) a (domain^n - R) for a negative goal
 *  In both cases we add a projection to rename attributes into the variable
 *  names of the goal. For example, R(X,Y) over relation R with attributes A1
 *  and A2 would be translated into PROJECTION[A1->X,B1->Y](R).
 */

#define COPY_PROPS_TO_TABLEACCESS(table,dl) \
    do { \
        if (DL_HAS_PROP(dl,DL_IS_IDB_REL)) \
            SET_BOOL_STRING_PROP(table, DL_IS_IDB_REL); \
        if (DL_HAS_PROP(dl,DL_IS_EDB_REL)) \
            SET_BOOL_STRING_PROP(table, DL_IS_EDB_REL); \
        if (DL_HAS_PROP(dl,DL_IS_FACT_REL)) \
            SET_BOOL_STRING_PROP(table, DL_IS_FACT_REL); \
		if (DL_HAS_PROP(dl,DL_IS_DOMAIN_REL)) \
			SET_BOOL_STRING_PROP(table, DL_IS_DOMAIN_REL); \
        DEBUG_LOG("create table for goal %s(%s)%s%s%s%s", table->tableName, \
                stringListToString(getQueryOperatorAttrNames((QueryOperator *) table)), \
                DL_HAS_PROP(dl,DL_IS_IDB_REL) ? " IDB " : "", \
                DL_HAS_PROP(dl,DL_IS_FACT_REL) ? " FACT " : "", \
           		DL_HAS_PROP(dl,DL_IS_FACT_REL) ? " EDB " : "", \
                DL_HAS_PROP(dl,DL_IS_DOMAIN_REL) ? " DOMAIN " : ""); \
    } while(0)


//List *edbAttr = NIL;

static QueryOperator *
translateUnSafeGoal(DLAtom *r, int goalPos)
{
    ProjectionOperator *renameOnSetDiff;
    QueryOperator *pInput;
    TableAccessOperator *rel;
    List *attrNames = NIL;
    List *dts = NIL;
    boolean isIDB = DL_HAS_PROP(r,DL_IS_IDB_REL);
    boolean isEDB = DL_HAS_PROP(r,DL_IS_EDB_REL);
    boolean isFact = DL_HAS_PROP(r,DL_IS_FACT_REL);
    boolean isDom = DL_HAS_PROP(r,DL_IS_DOMAIN_REL);
	boolean typeTransConst = FALSE;
	boolean typeTransVar = FALSE;

    DEBUG_LOG("goal is marked as %s%s%s%s",
            isIDB ? " IDB " : "",
            isFact ? " FACT " : "",
       		isEDB ? " EDB " : "",
            isDom ? "DOMAIN" : "");

    // for idb rels just fake attributes (and for now DTs)
    if (isIDB || (isFact && !isEDB))
    {
		for(int i = 0; i < LIST_LENGTH(r->args); i++)
			attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("A", gprom_itoa(i)));
    }
    // is edb, get information from db
    else
        attrNames = getAttributeNames(r->rel);

    dts = (List *) getDLProp((DLNode *) r, DL_PRED_DTS);

	// create table access op
	rel = createTableAccessOp(r->rel, NULL, "REL", NIL, attrNames, dts);
	COPY_PROPS_TO_TABLEACCESS(rel,r);

//    char *atomRel = NULL;

    // is negated goal?
    if (r->negated)
    {
    	// check if constant exists
    	FOREACH(Node,t,r->args)
		{
    		if(isA(t,Constant))
    			typeTransConst = TRUE;

    		if(isA(t,DLVar))
    			typeTransVar = TRUE;
		}

        SetOperator *setDiff;
        QueryOperator *dom;
        ProjectionOperator *rename;

        if(typeTransConst && typeTransVar) // if both constants and variables exist
    	{
    		// add selection if constants are used in the goal
    		// e.g., R(X,1) with attributes A0,A1 are translated into SELECTION[A1=1](R)
    		QueryOperator *sdom;
    		ProjectionOperator *pdom;

    		int s = 0;
    		List *selExprForRel = NIL;
    		List *varAttrNames = NIL;

    		sdom = (QueryOperator *) rel;

    		FOREACH(Node,arg,r->args)
    		{
    			if (isA(arg,Constant))
    			{
    				Node *comp;
    				AttributeDef *a = getAttrDefByPos(sdom,s);
    				comp = (Node *) createOpExpr(OPNAME_EQ,
    						LIST_MAKE(createFullAttrReference(strdup(a->attrName),
    								0, s, INVALID_ATTR, a->dataType),
    						copyObject(arg)));

    				ASSERT(a->dataType == ((Constant *) arg)->constType);

    				selExprForRel = appendToTailOfList(selExprForRel, comp);
    			}
    			else
    			{
    				AttributeDef *a = getAttrDefByPos(sdom,s);
    				varAttrNames = appendToTailOfList(varAttrNames, strdup(a->attrName));
    			}

    			s++;
    		}

    		// create a selection if necessary (equate constants, )
    		if (selExprForRel != NIL)
    		{
    			SelectionOperator *sel;
    			Node *cond = andExprList(selExprForRel);

    			sel = createSelectionOp(cond, sdom, NIL, NULL);
    			addParent(sdom, (QueryOperator *) sel);
    			sdom = (QueryOperator *) sel;
    		}

    	    // add projection
    	    pdom = (ProjectionOperator *) createProjOnAttrsByName(sdom, varAttrNames, NIL);
    	    addChildOperator((QueryOperator *) pdom, sdom);

    	    int numAttrs = 0;
//        	QueryOperator *dTransRule;

        	// get the number of variables in the goal
			FOREACH(Node,arg,r->args)
				if (!isA(arg,Constant))
					numAttrs++;

//        	// use associate domains if exist
//            if (associateDomain && LIST_LENGTH(dRules) > 0)
//            {
//    			atomRel = r->rel;
//    			atomRel = replaceSubstr(atomRel, "R", "");
//    			atomRel = replaceSubstr(atomRel, "_WON", "");
//    			atomRel = replaceSubstr(atomRel, "_nonlinked", "");
//
////    			HashMap *analyzeAtom = NEW_MAP(Constant,List);
//
//				List *origHead = NIL;
//				FOREACH(DLRule,h,origProg)
//					origHead = appendToTailOfList(origHead,h->head->rel);
//
//				// collect actual attribute name from table
////			    int varPosition = 0;
//				if(!searchListString(origHead,atomRel))
//				{
//					edbAttr = getAttributeNames(atomRel);
//
//					// store attr name and rel name as key and value pairs
//					FOREACH(Node,arg,r->args)
//					{
//						if (!isA(arg,Constant))
//						{
//							char *atomAttr = (char *) getNthOfListP(ordAttr,varPosition);
//							ADD_TO_MAP(analyzeAtom,createStringKeyValue(atomAttr,edbRel));
//						}
//						varPosition++;
//					}
//				}
//				else
//				{
//					int progPos = 0;
//					FOREACH(DLRule,h,origProg)
//					{
//						progPos++;
//
//						if(strcmp(h->head->rel,edbRel) == 0)
//						{
//							FOREACH(DLAtom,a,h->body)
//							{
////								boolean edbAtom = DL_HAS_PROP(a,DL_IS_EDB_REL);
//
//								for(int i = 0; i < LIST_LENGTH(h->head->args); i++)
//								{
//									Node *headArg = (Node *) getNthOfListP(h->head->args,i);
//
//									if(progPos == 1)
//									{
//										if(!isA(headArg,Constant) && searchListString(a->args,(char *) getNthOfListP(h->head->args,i)))
//										{
//											edbRel = a->rel;
//											varPosition = i;
//
//											char *atomAttr = (char *) getNthOfListP(ordAttr,varPosition);
//											ADD_TO_MAP(analyzeAtom,createStringKeyValue(atomAttr,edbRel));
//										}
//									}
//									else
//									{
//										edbRel = a->rel;
//										char *atomAttr = (char *) getNthOfListP(ordAttr,varPosition);
//										ADD_TO_MAP(analyzeAtom,createStringKeyValue(atomAttr,edbRel));
//									}
//								}
//							}
//						}
//					}
//				}
//
//    			// put domain rules in the order
//    			List *ordDomList = NIL;
//    			for(int i = 0; i < LIST_LENGTH(ordAttr); i++)
//    			{
//    				FOREACH(DLDomain,d,dlDom)
//    					if(strcmp(d->attr,(char *) getNthOfListP(ordAttr,i)) == 0)
//    						ordDomList = appendToTailOfList(ordDomList,d);
//    			}
//
//    			// match key value pairs with associate domain
//    			List *transList = NIL;
////    			List *domTrans = NIL;
//
//    			QueryOperator *dQuery;
//    			List *domainAttrs = NIL;
//
////				if (LIST_LENGTH(ordDomList) < mapSize(analyzeAtom))
////					FATAL_LOG("No associate domain that is required has not been assigned.");
//
//    			FOREACH(DLDomain,d,ordDomList)
//				{
//					if(MAP_HAS_STRING_KEY(analyzeAtom,d->attr) &&
//							strcmp(exprToSQL(MAP_GET_STRING(analyzeAtom,d->attr)),CONCAT_STRINGS("'",d->rel,"'")) == 0)
////					if(MAP_HAS_STRING_KEY(analyzeAtom,d->attr))
//					{
//		    			List *unionList = NIL;
//						for(int i = 0; i < LIST_LENGTH(dRules); i++)
//						{
//							DLAtom *headAtom = ((DLRule *) getNthOfListP(dRules,i))->head;
//							char *dHead = headAtom->rel;
//
////							if(strcmp(dHead,d->name) == 0 && !searchList(domTrans,getNthOfListP(dRules,i)))
//							if(strcmp(dHead,d->name) == 0)
//							{
//								dTransRule = translateRule((DLRule *) getNthOfListP(dRules,i));
//
//								// if another head atom exist in the list of domain rules, then store it for union later
//								int numOfUnion = 0;
//								FOREACH(DLRule,d,dRules)
//									if(strcmp(d->head->rel,dHead) == 0)
//										numOfUnion++;
//
//								if(numOfUnion - 1 > 0)
//									unionList = appendToTailOfList(unionList,dTransRule);
//								else
//									transList = appendToTailOfList(transList,dTransRule);
//
////								// check domain rule has been translated
////								domTrans = appendToTailOfList(domTrans,getNthOfListP(dRules,i));
//							}
//						}
//
//						if(LIST_LENGTH(unionList) != 0)
//						{
//							dQuery = (QueryOperator *) getNthOfListP(unionList,0);
//
//							for(int i = 1; i < LIST_LENGTH(unionList); i++)
//							{
//								QueryOperator *uDom = (QueryOperator *) getNthOfListP(unionList,i);
//								QueryOperator *oldUd = dQuery;
//
//								dQuery = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(dQuery,uDom), NIL, getQueryOperatorAttrNames(uDom));
//
//								addParent(uDom, dQuery);
//								addParent(oldUd, dQuery);
//							}
//
//							transList = appendToTailOfList(transList,dQuery);
//						}
//					}
//				}
//
//				if(transList == NIL)
//					FATAL_LOG("No translated rules for associate domains.");
//				else
//				{
//					// compute Domain X Domain X ... X Domain number of attributes of goal relation R times
//					// then return (Domain X Domain X ... X Domain) - R
//
//					dom = (QueryOperator *) getNthOfListP(transList,0);
//					domainAttrs = singleton("D");
//
//					for(int i = 1; i < LIST_LENGTH(transList); i++)
//					{
//						char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i));
//						QueryOperator *aDom = (QueryOperator *) getNthOfListP(transList,i);
//						QueryOperator *oldD = dom;
//
//						domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
//						dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
//								LIST_MAKE(dom, aDom), NULL,
//								domainAttrs);
//
//						addParent(aDom, dom);
//						addParent(oldD, dom);
//					}
//				}
//			}
//			else // if associate domain does not exist, then use previous method
//            {


        	    // compute Domain X Domain X ... X Domain number of attributes of goal relation R times
                // then return (Domain X Domain X ... X Domain) - R
                dom = (QueryOperator *) createTableAccessOp("_DOMAIN", NULL,
                        "DummyDom", NIL, LIST_MAKE("D"), singletonInt(DT_STRING));
                List *domainAttrs = singleton("D");

                for(int i = 1; i < numAttrs; i++)
                {
                    char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i));
                    QueryOperator *aDom = (QueryOperator *) createTableAccessOp(
                            "_DOMAIN", NULL, "DummyDom", NIL,
                            LIST_MAKE("D"), singletonInt(DT_STRING));

                    QueryOperator *oldD = dom;
                    domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
                    dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
                            LIST_MAKE(dom, aDom), NULL,
                            domainAttrs);

                    addParent(aDom, dom);
                    addParent(oldD, dom);
                }
//            }

            rename = (ProjectionOperator *) createProjOnAllAttrs(dom);
            //TODO check whether this works
//            //check if cast is needed
//            FOREACH(Node,c,pdom->projExprs)
//            {
//            	if(typeOf(c) != DT_STRING && typeOf(c) != DT_BOOL)
//            	{
//                	castChecker = TRUE;
//            		castForPos = TRUE; // check if CAST occurred in general
//            	}
//            }
//
//            // if TRUE, then cast to DT_STRING
//            if(castChecker)
//            {
//        		goalRel = r->rel; //TODO Implement to find certain part of translation for CAST
////        		goalVars = r->args;
//
//                List *newDataType = NIL;
//    			FORBOTH(Node,p,r,pdom->projExprs,rename->projExprs)
//						newDataType = appendToTailOfList(newDataType,getHeadOfListP(createCasts(p,r)));
//
//    			pdom->projExprs = copyObject(newDataType);
//            }

            // rename attribute names
    //        List *newAttrNames = NIL;
    //        int posAttr =0;
    //
    //        FOREACH(Node,arg,r->args)
    //		{
    //			if (!isA(arg,Constant))
    //			{
    //				newAttrNames = appendToTailOfList(newAttrNames, CONCAT_STRINGS("A", gprom_itoa(posAttr)));
    //			}
    //			posAttr++;
    //		}

            int i = 0;
            FOREACH(AttributeDef,a,rename->op.schema->attrDefs)
            {
                char *name = strdup(getNthOfListP(varAttrNames, i++));
                a->attrName = name;
            }
            addChildOperator((QueryOperator *) rename, dom);
            dom = (QueryOperator *) rename;

            setDiff = createSetOperator(SETOP_DIFFERENCE, LIST_MAKE(dom, (QueryOperator *) pdom),
                    NULL, deepCopyStringList(varAttrNames));
            addParent(dom, (QueryOperator *) setDiff);
            addParent((QueryOperator *) pdom, (QueryOperator *) setDiff);

            pInput = (QueryOperator *) setDiff;
    	}
    	else // if only constants or only variables exist
   		{
    		// if only constant exsits, then filter out upfront
			ProjectionOperator *pdom = NULL;
    		if(typeTransConst && !typeTransVar)
    		{
    			// add selection if constants are used in the goal
				// e.g., R(X,1) with attributes A0,A1 are translated into SELECTION[A1=1](R)
				QueryOperator *sdom;

				int s = 0;
				List *selExprForRel = NIL;
				sdom = (QueryOperator *) rel;

				FOREACH(Node,arg,r->args)
				{
					if (isA(arg,Constant))
					{
						Node *comp;
						AttributeDef *a = getAttrDefByPos(sdom,s);
						comp = (Node *) createOpExpr(OPNAME_EQ,
								LIST_MAKE(createFullAttrReference(strdup(a->attrName),
										0, s, INVALID_ATTR, a->dataType),
								copyObject(arg)));

						ASSERT(a->dataType == ((Constant *) arg)->constType);

						selExprForRel = appendToTailOfList(selExprForRel, comp);
					}
//					else
//					{
//						AttributeDef *a = getAttrDefByPos(sdom,s);
//						varAttrNames = appendToTailOfList(varAttrNames, strdup(a->attrName));
//					}

					s++;
				}

				// create a selection if necessary (equate constants, )
				if (selExprForRel != NIL)
				{
					SelectionOperator *sel;
					Node *cond = andExprList(selExprForRel);

					sel = createSelectionOp(cond, sdom, NIL, NULL);
					addParent(sdom, (QueryOperator *) sel);
					sdom = (QueryOperator *) sel;
				}

				// add projection
				pdom = (ProjectionOperator *) createProjOnAllAttrs(sdom);
				addChildOperator((QueryOperator *) pdom, sdom);
    		}
//            SetOperator *setDiff;
//            ProjectionOperator *rename;
//            QueryOperator *dom;

            int numAttrs = getNumAttrs((QueryOperator *) rel);
//        	QueryOperator *dTransRule;
//			List *transList = NIL;
//
//            if(typeTransConst && !typeTransVar && associateDomain && LIST_LENGTH(dRules) > 0)
//            {
////            	if (associateDomain && LIST_LENGTH(dRules) > 0) // use associate domains if exist
////            	{
//            		char *bodyAtomRel = r->rel;
//					bodyAtomRel = replaceSubstr(bodyAtomRel, "R", "");
//					bodyAtomRel = replaceSubstr(bodyAtomRel, "_WON", "");
//					bodyAtomRel = replaceSubstr(bodyAtomRel, "_nonlinked", "");
//
//					List *origHead = NIL;
//					FOREACH(DLRule,h,origProg)
//						origHead = appendToTailOfList(origHead,h->head->rel);
//
//					if(searchListString(origHead,bodyAtomRel))
//					{
//						FOREACH(DLRule,h,origProg)
//							if(strcmp(h->head->rel,bodyAtomRel) == 0)
//								FOREACH(DLAtom,a,h->body)
//										bodyAtomRel = a->rel;
//
//					}
//					edbAttr = getAttributeNames(bodyAtomRel);
//
//	    			HashMap *analyzeAtom = NEW_MAP(Constant,List);
//					char *atomAttr = NULL;
//
//	    			// store the head predicate name for which domain rules are necessary
//					if(LIST_LENGTH(r->args) == 1)
//					{
//						for(int varPosition = 0; varPosition < LIST_LENGTH(r->args); varPosition++)
//						{
//		    				atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
//
//							FOREACH(DLDomain,d,dlDom)
//							{
//								if(strcmp(d->rel,bodyAtomRel) == 0)
//								{
//									char *key = (char *) CONCAT_STRINGS(d->attr,".",d->rel);
//									char *value = d->name;
//									ADD_TO_MAP(analyzeAtom,createStringKeyValue(key,value));
//								}
//							}
//						}
//					}
//					else
//					{
//						for(int varPosition = 0; varPosition < LIST_LENGTH(r->args); varPosition++)
//						{
//		    				atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
//
//							FOREACH(DLDomain,d,dlDom)
//							{
//								if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,bodyAtomRel) == 0)
//								{
//									char *key = (char *) CONCAT_STRINGS(d->attr,".",d->rel);
//									char *value = d->name;
//									ADD_TO_MAP(analyzeAtom,createStringKeyValue(key,value));
//								}
//							}
//						}
//					}
//
//					QueryOperator *dQuery;
//					List *domainAttrs = NIL;
//
//					// translate domain rules
//    				FOREACH_HASH(Constant,c,analyzeAtom)
//					{
//    					List *unionList = NIL;
//
//    					for(int i = 0; i < LIST_LENGTH(dRules); i++)
//    	    			{
//    						char *dHead = ((DLRule *) getNthOfListP(dRules,i))->head->rel;
//
//    						if(strcmp(dHead,STRING_VALUE(c)) == 0)
//    						{
//    							dTransRule = translateRule((DLRule *) getNthOfListP(dRules,i));
//
//    							// if another head atom exist in the list of domain rules, then store it for union later
//    							int numOfUnion = 0;
//    							FOREACH(DLRule,d,dRules)
//    								if(strcmp(d->head->rel,dHead) == 0)
//    									numOfUnion++;
//
//    							if(numOfUnion - 1 > 0)
//    								unionList = appendToTailOfList(unionList,dTransRule);
//    							else
//    								transList = appendToTailOfList(transList,dTransRule);
//    						}
//    	    			}
//
//    					if(unionList != NIL)
//    					{
//    						dQuery = (QueryOperator *) getNthOfListP(unionList,0);
//
//    						for(int i = 1; i < LIST_LENGTH(unionList); i++)
//    						{
//    							QueryOperator *uDom = (QueryOperator *) getNthOfListP(unionList,i);
//    							QueryOperator *oldUd = dQuery;
//
//    							dQuery = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(dQuery,uDom), NIL, getQueryOperatorAttrNames(uDom));
//
//    							addParent(uDom, dQuery);
//    							addParent(oldUd, dQuery);
//    						}
//
//    						transList = appendToTailOfList(transList,dQuery);
//    					}
//					}
//
//					if(transList == NIL)
//						FATAL_LOG("No translated rules for associate domains.");
//					else
//					{
//						// compute Domain X Domain X ... X Domain number of attributes of goal relation R times
//						// then return (Domain X Domain X ... X Domain) - R
//						dom = (QueryOperator *) getNthOfListP(transList,0);
//						domainAttrs = singleton("D");
//
//						for(int i = 1; i < LIST_LENGTH(transList); i++)
//						{
//							char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i));
//							QueryOperator *aDom = (QueryOperator *) getNthOfListP(transList,i);
//							QueryOperator *oldD = dom;
//
//							domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
//							dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
//									LIST_MAKE(dom, aDom), NULL,
//									domainAttrs);
//
//							addParent(aDom, dom);
//							addParent(oldD, dom);
//						}
//					}
////            	}
//            }
//			else // if associate domain does not exist, then use previous method
//			{
//			   // compute Domain X Domain X ... X Domain number of attributes of goal relation R times
//			   // then return (Domain X Domain X ... X Domain) - R
//			   dom = (QueryOperator *) createTableAccessOp("_DOMAIN", NULL,
//					   "DummyDom", NIL, LIST_MAKE("D"), singletonInt(DT_STRING));
//			   List *domainAttrs = singleton("D");
//
//			   for(int i = 1; i < numAttrs; i++)
//			   {
//				   char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i));
//				   QueryOperator *aDom = (QueryOperator *) createTableAccessOp(
//						   "_DOMAIN", NULL, "DummyDom", NIL,
//						   LIST_MAKE("D"), singletonInt(DT_STRING));
//
//				   QueryOperator *oldD = dom;
//				   domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
//				   dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
//						   LIST_MAKE(dom, aDom), NULL,
//						   domainAttrs);
//
//				   addParent(aDom, dom);
//				   addParent(oldD, dom);
//			   }
//			}

            if(typeTransConst && !typeTransVar)
            {
        		List *constAttrs = singleton("D");
        		for(int i = 1; i < numAttrs; i++)
        		{
        			char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i));
        			constAttrs = appendToTailOfList(deepCopyStringList(constAttrs),aDomAttrName);
        		}

                dom = (QueryOperator *) createConstRelOp(r->args, NIL, constAttrs, NIL);
            }
            else
            {
			   // compute Domain X Domain X ... X Domain number of attributes of goal relation R times
			   // then return (Domain X Domain X ... X Domain) - R
			   dom = (QueryOperator *) createTableAccessOp("_DOMAIN", NULL,
					   "DummyDom", NIL, LIST_MAKE("D"), singletonInt(DT_STRING));
			   List *domainAttrs = singleton("D");

			   for(int i = 1; i < numAttrs; i++)
			   {
				   char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i));
				   QueryOperator *aDom = (QueryOperator *) createTableAccessOp(
						   "_DOMAIN", NULL, "DummyDom", NIL,
						   LIST_MAKE("D"), singletonInt(DT_STRING));

				   QueryOperator *oldD = dom;
				   domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
				   dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
						   LIST_MAKE(dom, aDom), NULL,
						   domainAttrs);

				   addParent(aDom, dom);
				   addParent(oldD, dom);
			   }
            }
            rename = (ProjectionOperator *) createProjOnAllAttrs(dom);

            // rename attribute names
    //        List *newAttrNames = NIL;
    //        int posAttr =0;
    //
    //        FOREACH(Node,arg,r->args)
    //		{
    //			if (!isA(arg,Constant))
    //			{
    //				newAttrNames = appendToTailOfList(newAttrNames, CONCAT_STRINGS("A", gprom_itoa(posAttr)));
    //			}
    //			posAttr++;
    //		}

            int i = 0;
            FOREACH(AttributeDef,a,rename->op.schema->attrDefs)
            {
                char *name = strdup(getNthOfListP(attrNames, i++));
                a->attrName = name;
            }
            addChildOperator((QueryOperator *) rename, dom);
            dom = (QueryOperator *) rename;

            if(typeTransConst && !typeTransVar)
            {
                setDiff = createSetOperator(SETOP_DIFFERENCE, LIST_MAKE(dom, pdom),
                        NULL, deepCopyStringList(attrNames));
                addParent(dom, (QueryOperator *) setDiff);
                addParent((QueryOperator *) pdom, (QueryOperator *) setDiff);
            }
            else
            {
                setDiff = createSetOperator(SETOP_DIFFERENCE, LIST_MAKE(dom, rel),
                        NULL, deepCopyStringList(attrNames));
                addParent(dom, (QueryOperator *) setDiff);
                addParent((QueryOperator *) rel, (QueryOperator *) setDiff);
            }

            pInput = (QueryOperator *) setDiff;
   		}
    }
    else
	{
        pInput = (QueryOperator *) rel;
	}

    // do not generate selection if constants exist in the only negated body args
//    if(!(typeTransConst && typeTransVar && r->negated))
   	if(!(typeTransConst && r->negated))
    {
        // add selection if constants are used in the goal
        // e.g., R(X,1) with attributes A0,A1 are translated into SELECTION[A1=1](R)
        List *selExpr = NIL;
        int i = 0;

        FOREACH(Node,arg,r->args)
        {
            if (isA(arg,Constant))
            {
                Node *comp;
                AttributeDef *a = getAttrDefByPos(pInput,i);
                comp = (Node *) createOpExpr(OPNAME_EQ,
                        LIST_MAKE(createFullAttrReference(strdup(a->attrName),
                                0, i, INVALID_ATTR, a->dataType),
                        copyObject(arg)));

                ASSERT(a->dataType == ((Constant *) arg)->constType);

                selExpr = appendToTailOfList(selExpr, comp);
            }
            i++;
        }

        // add selection to equate attributes if variables are repeated
        int j = 0;
        i = 0;
        FOREACH(Node,arg,r->args)
        {
            j = 0;
            if (isA(arg,DLVar))
            {
                char *name = ((DLVar *) arg)->name;
                DEBUG_LOG("name %s", name);
                FOREACH(Node,argI,r->args)
                {
                    // found matching names
                    if (isA(argI, DLVar) && j > i)
                    {
                        char *IName = ((DLVar *) argI)->name;
                        DEBUG_LOG("name %s and %s", name, IName);
                        if (streq(name,IName))
                        {
                            Node *comp;
                            AttributeDef *aI = getAttrDefByPos(pInput,i);
                            AttributeDef *aJ = getAttrDefByPos(pInput,j);
                            comp = (Node *) createOpExpr(OPNAME_EQ,
                                    LIST_MAKE(createFullAttrReference(strdup(aI->attrName),
                                            0, i, INVALID_ATTR, aI->dataType),
                                            createFullAttrReference(strdup(aJ->attrName),
                                                    0, j, INVALID_ATTR, aJ->dataType))
                                    );

                            selExpr = appendToTailOfList(selExpr, comp);
                        }
                    }
                    j++;
                }
            }
            i++;
        }

        // create a selection if necessary (equate constants, )
        if (selExpr != NIL)
        {
            SelectionOperator *sel;
            Node *cond = andExprList(selExpr);

            sel = createSelectionOp(cond, pInput, NIL, NULL);
            addParent(pInput, (QueryOperator *) sel);
            pInput = (QueryOperator *) sel;
        }
    }

    // add projection
    renameOnSetDiff = (ProjectionOperator *) createProjOnAllAttrs(pInput);
    addChildOperator((QueryOperator *) renameOnSetDiff, pInput);

    // change attribute names
    Set *nameSet = STRSET();
    List *finalNames = NIL;
    int argPos = 0;

    if(!(typeTransConst && typeTransVar && r->negated))
    {
        FORBOTH(Node,var,attr,r->args,renameOnSetDiff->op.schema->attrDefs)
        {
            char *n = NULL;
            AttributeDef *d = (AttributeDef *) attr;

            if(isA(var,DLVar))
            {
                DLVar *v = (DLVar *) var;
                n = v->name;
                d->attrName = strdup(n);
            }
            else if (isA(var, Constant))
            {
                n = CONCAT_STRINGS("C_", gprom_itoa(goalPos), "_", gprom_itoa(argPos++));
                d->attrName = strdup(n);
            }
            else
                FATAL_LOG("we should not end up here");

            addToSet(nameSet, strdup(n));
            finalNames = appendToTailOfList(finalNames, strdup(n));
        }

        //TODO make attribute names unique
        makeNamesUnique(finalNames, nameSet);
        FORBOTH(void,name,attr,finalNames,renameOnSetDiff->op.schema->attrDefs)
        {
            char *n = (char *) name;
            AttributeDef *a = (AttributeDef *) attr;

            if(!streq(a->attrName, name))
                a->attrName = strdup(n);
        }
    }
    else
    {
    	List *removeConstant = NIL;

    	FOREACH(Node,var,r->args)
    		if(isA(var,DLVar))
    			removeConstant = appendToTailOfList(removeConstant,var);

        FORBOTH(Node,var,attr,removeConstant,renameOnSetDiff->op.schema->attrDefs)
        {
            char *n = NULL;
            AttributeDef *d = (AttributeDef *) attr;

            if(isA(var,DLVar))
            {
                DLVar *v = (DLVar *) var;
                n = v->name;
                d->attrName = strdup(n);
            }
            else if (isA(var, Constant))
            {
                n = CONCAT_STRINGS("C_", gprom_itoa(goalPos), "_", gprom_itoa(argPos++));
                d->attrName = strdup(n);
            }
            else
                FATAL_LOG("we should not end up here");

            addToSet(nameSet, strdup(n));
            finalNames = appendToTailOfList(finalNames, strdup(n));
        }

        //TODO make attribute names unique
        makeNamesUnique(finalNames, nameSet);
        FORBOTH(void,name,attr,finalNames,renameOnSetDiff->op.schema->attrDefs)
        {
            char *n = (char *) name;
            AttributeDef *a = (AttributeDef *) attr;

            if(!streq(a->attrName, name))
                a->attrName = strdup(n);
        }
    }


    DEBUG_LOG("translated goal %s:\n%s",
            datalogToOverviewString((Node *) r),
            operatorToOverviewString((Node *) renameOnSetDiff));

    return (QueryOperator *) renameOnSetDiff;
}



/*
 * Like translate goal, but for safe rules we can first determine variable bindings from the positive part of a rule before applying MINUS.
 */

static QueryOperator *
translateSafeGoal(DLAtom *r, int goalPos, QueryOperator *posPart)
{
    ProjectionOperator *rename;
    QueryOperator *pInput;
    TableAccessOperator *rel;
    List *attrNames = NIL;
    List *dts = NIL;
    boolean isIDB = DL_HAS_PROP(r,DL_IS_IDB_REL);
    boolean isEDB = DL_HAS_PROP(r,DL_IS_EDB_REL);
    boolean isFact = DL_HAS_PROP(r,DL_IS_FACT_REL);
    boolean isDom = DL_HAS_PROP(r,DL_IS_DOMAIN_REL);

    DEBUG_LOG("goal is marked as %s%s%s",
            isIDB ? " IDB " : "",
            isFact ? " FACT " : "",
            isEDB ? " EDB " : "",
            isDom ? "DOMAIN" : "");

    // for idb rels just fake attributes (and for now DTs)
    if (isIDB || (isFact && !isEDB))
    {
        for(int i = 0; i < LIST_LENGTH(r->args); i++)
        	attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("A", gprom_itoa(i)));
    }
    // is edb, get information from db
    else
        attrNames = getAttributeNames(r->rel);

    dts = (List *) getDLProp((DLNode *) r, DL_PRED_DTS);

    /*
     * compare dts with actual dataType from the relation
     * if different, then use the correct one
     * TODO: check from the parser, i.e., while we parse, the dataType is set to DT_STRING for all
     */
    List *correctDts = NIL;
    FOREACH(Node,n,r->args)
    {
    	correctDts = appendToTailOfListInt(correctDts,typeOf(n));
//    	if(isA(n,DLVar))
//    	{
//    		DLVar *v = (DLVar *) n;
//    		correctDts = appendToTailOfListInt(correctDts,v->dt);
//    	}
//
//    	if(isA(n,Operator))
//    	{
//    		Operator *o = (Operator *) n;
//    		DLVar *v = (DLVar *) getHeadOfListP(o->args);
//			correctDts = appendToTailOfListInt(correctDts,v->dt);
//    	}
//
//    	if(isA(n,Constant))
//    	{
//    		Constant *c = (Constant *) n;
//			correctDts = appendToTailOfListInt(correctDts,c->constType);
//    	}
    }

    FORBOTH(Node,d,cd,dts,correctDts)
    {
    	if(d != cd)
    	{
    		dts = (List *) correctDts;
    		break;
    	}
    }

    // create table access op
    rel = createTableAccessOp(r->rel, NULL, "REL", NIL, attrNames, dts);
    COPY_PROPS_TO_TABLEACCESS(rel,r);

    // is negated goal?
    if (r->negated)
    {
        SetOperator *setDiff;
        ProjectionOperator *rename = NULL;
        QueryOperator *dom;
        List *varNames = NIL;
        List *projArgs = NIL;
        List *projNames = NIL;
//        int numAttrs = getNumAttrs((QueryOperator *) rel);
        // compute Domain X Domain X ... X Domain number of attributes of goal relation R times
        // then return (Domain X Domain X ... X Domain) - R
//        dom = (QueryOperator *) createTableAccessOp("_DOMAIN", NULL,
//        		"DummyDom", NIL, LIST_MAKE("D"), singletonInt(DT_STRING));
//        List *domainAttrs = singleton("D");
//
//        for(int i = 1; i < numAttrs; i++)
//        {
//        	char *aDomAttrName = CONCAT_STRINGS("D", gprom_itoa(i++));
//            QueryOperator *aDom = (QueryOperator *) createTableAccessOp(
//                    "_DOMAIN", NULL, "DummyDom", NIL,
//					LIST_MAKE("D"), singletonInt(DT_STRING));
//
//            QueryOperator *oldD = dom;
//            domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
//            dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
//                    LIST_MAKE(dom, aDom), NULL,
//                    domainAttrs);
//
//            addParent(aDom, dom);
//            addParent(oldD, dom);
//        }

        // create projection arguments and varnames for projection over joined positive subgoals
        int attrPos = 0;

        FOREACH(Node,n,r->args)
        {
            if(isA(n,DLVar))
            {
                DLVar *v = (DLVar*) n;
                char *name = v->name;
                int origPos = getAttrPos(posPart, name);

                varNames = appendToTailOfList(varNames, strdup(name));
                projArgs = appendToTailOfList(projArgs,
                        createFullAttrReference(strdup(name), 0, origPos, INVALID_ATTR, v->dt));
                //TODO search for real attribute position of attribute named after this variable in schema of pos

                // create unique variable name if same name exists
                if (!searchListString(projNames,name))
                	projNames = appendToTailOfList(projNames, strdup(name));
                else
                {
                	char *newProjName = CONCAT_STRINGS(name, gprom_itoa(attrPos));
                	projNames = appendToTailOfList(projNames, strdup(newProjName));
                }
            }
            else
            {
                projArgs = appendToTailOfList(projArgs, copyObject(n));
                projNames = appendToTailOfList(projNames, CONCAT_STRINGS("C_", gprom_itoa(attrPos)));
            }
            attrPos++;
        }

        DEBUG_LOG("var names of neg goal: %s", stringListToString(varNames));

        // check if variables existed
//        if (varNames != NULL)
//        {
//            // project onto variables
//            rename = (ProjectionOperator *) createProjOnAttrsByName(posPart, varNames);
////            int i = 0;
////            FOREACH(AttributeDef,a,rename->op.schema->attrDefs)
////            {
////                char *name = strdup(getNthOfListP(varNames, i++));
////                a->attrName = name;
////            }
//
//            // change attribute names
//			Set *nameSet = STRSET();
//			List *finalNames = NIL;
//
//			FORBOTH(Node,var,attr,r->args,rename->op.schema->attrDefs)
//			{
//				char *n = NULL;
//				AttributeDef *d = (AttributeDef *) attr;
//
//				if(isA(var,DLVar))
//				{
//					DLVar *v = (DLVar *) var;
//					n = v->name;
//					d->attrName = strdup(n);
//				}
////				else
////				    n = strdup("C");
//
//				addToSet(nameSet, strdup(n));
//				finalNames = appendToTailOfList(finalNames, strdup(n));
//			}
//
//			//TODO make attribute names unique
//			makeNamesUnique(finalNames, nameSet);
//			FORBOTH(void,name,attr,finalNames,rename->op.schema->attrDefs)
//			{
//				char *n = (char *) name;
//				AttributeDef *a = (AttributeDef *) attr;
//
//				if(!streq(a->attrName, name))
//					a->attrName = strdup(n);
//			}
//
//    		DEBUG_LOG("proj: %s", operatorToOverviewString((Node *) rename));
//    		addChildOperator((QueryOperator *) rename, posPart);
//
//            // add constants to projection
//            dom = (QueryOperator *) rename;
//            rename = (ProjectionOperator *) createProjectionOp(projArgs, dom, NIL, projNames);
//            addParent(dom, (QueryOperator *) rename);
//    //        addChildOperator((QueryOperator *) rename, dom);
//        }
//        else
//        {
            // add constants to projection
        	rename = (ProjectionOperator *) createProjectionOp(projArgs, (QueryOperator *) rename, NIL, projNames);
        	DEBUG_LOG("proj: %s", operatorToOverviewString((Node *) rename));

    		addChildOperator((QueryOperator *) rename, posPart);
//        }


//		// if CAST occurred, then apply CAST to the negated part of positive translation
//		// TODO consider more cases (currently only tested with certain query in TPCH)
//		if(castForPos)
//		{
//			List *newDataType = NIL;
//			ProjectionOperator *newRename = (ProjectionOperator *) createProjOnAllAttrs((QueryOperator *) rename);
//
//			FOREACH(Node,r,newRename->projExprs)
//				if(typeOf(r) != DT_STRING && typeOf(r) != DT_BOOL)
//					newDataType = appendToTailOfList(newDataType,createCastExpr(r,DT_STRING));
//
//			newRename->projExprs = copyObject(newDataType);
//			addChildOperator((QueryOperator *) newRename, (QueryOperator *) rename);
//			dom = (QueryOperator *) newRename;
//		}
//		else
			dom = (QueryOperator *) rename;


        // create set diff
        setDiff = createSetOperator(SETOP_DIFFERENCE, LIST_MAKE(dom, rel),
                NULL, deepCopyStringList(projNames));
        addParent(dom, (QueryOperator *) setDiff);
        addParent((QueryOperator *) rel, (QueryOperator *) setDiff);

        pInput = (QueryOperator *) setDiff;
    }
    else
        pInput = (QueryOperator *) rel;

    // add selection if constants are used in the goal
    // e.g., R(X,1) with attributes A0,A1 are translated into SELECTION[A1=1](R)
    List *selExpr = NIL;
    int i = 0;

    FOREACH(Node,arg,r->args)
    {
        if (isA(arg,Constant))
        {
            Node *comp;
            AttributeDef *a = getAttrDefByPos(pInput,i);
            comp = (Node *) createOpExpr(OPNAME_EQ,
                    LIST_MAKE(createFullAttrReference(strdup(a->attrName),
                            0, i, INVALID_ATTR, a->dataType),
                    copyObject(arg)));

            // commented out as CAST later handles the data types
//            ASSERT(a->dataType == ((Constant *) arg)->constType);

            selExpr = appendToTailOfList(selExpr, comp);
        }
        i++;
    }

    // add selection to equate attributes if variables are repeated
    int j = 0;
    i = 0;
    FOREACH(Node,arg,r->args)
    {
        j = 0;
        if (isA(arg,DLVar))
        {
            char *name = ((DLVar *) arg)->name;
            DEBUG_LOG("name %s", name);
            FOREACH(Node,argI,r->args)
            {
                // found matching names
                if (isA(argI, DLVar) && j > i)
                {
                    char *IName = ((DLVar *) argI)->name;
                    DEBUG_LOG("name %s and %s", name, IName);
                    if (streq(name,IName))
                    {
                        Node *comp;
                        AttributeDef *aI = getAttrDefByPos(pInput,i);
                        AttributeDef *aJ = getAttrDefByPos(pInput,j);
                        comp = (Node *) createOpExpr(OPNAME_EQ,
                                LIST_MAKE(createFullAttrReference(strdup(aI->attrName),
                                        0, i, INVALID_ATTR, aI->dataType),
                                        createFullAttrReference(strdup(aJ->attrName),
                                                0, j, INVALID_ATTR, aJ->dataType))
                                );

                        selExpr = appendToTailOfList(selExpr, comp);
                    }
                }
                j++;
            }
        }
        i++;
    }

    // create a selection if necessary (equate constants, )
    if (selExpr != NIL)
    {
        SelectionOperator *sel;
        Node *cond = andExprList(selExpr);

        sel = createSelectionOp(cond, pInput, NIL, NULL);
        addParent(pInput, (QueryOperator *) sel);
        pInput = (QueryOperator *) sel;
    }

    // add projection
    rename = (ProjectionOperator *) createProjOnAllAttrs(pInput);

    //TODO casts removed
    // if CAST occurred, then apply CAST for EDB
//    if(castForPos && isEDB)
//    {
//    	List *newDataType = NIL;
//		FOREACH(Node,r,rename->projExprs)
//		{
//			if(typeOf(r) != DT_STRING && typeOf(r) != DT_BOOL) //TODO check the datatype, if not string, then cast
//				newDataType = appendToTailOfList(newDataType,createCastExpr(r,DT_STRING));
//			else
//				newDataType = appendToTailOfList(newDataType,copyObject(r));
//		}
//
//		rename->projExprs = copyObject(newDataType);
//    }

    addChildOperator((QueryOperator *) rename, pInput);


//    // cast if checker is true e.g., the datatype is DL_INT and rel name and variables are same
//    char *posGoalRel = r->rel;
////    List *posGoalVars = removeListElementsFromAnotherList(goalVars,r->args);
//
//    if(castForPos && strcmp(goalRel,posGoalRel) == 0)
//    {
//        List *newDataType = NIL;
//        FOREACH(Node,r,rename->projExprs)
//        {
//        	if(typeOf(r) != DT_STRING && typeOf(r) != DT_BOOL) //TODO check the datatype, if not string, then cast
//        		newDataType = appendToTailOfList(newDataType,createCastExpr(r,DT_STRING));
//        	else
//        		newDataType = appendToTailOfList(newDataType,copyObject(r));
//        }
//
//    	rename->projExprs = copyObject(newDataType);
//    	DEBUG_LOG("cast on rename:%s", operatorToOverviewString((Node *) rename));
//    }


    // change attribute names
    Set *nameSet = STRSET();
    List *finalNames = NIL;
    int argPos = 0;

    FORBOTH(Node,var,attr,r->args,rename->op.schema->attrDefs)
    {
        char *n = NULL;
        AttributeDef *d = (AttributeDef *) attr;

        if(isA(var,DLVar))
        {
            DLVar *v = (DLVar *) var;
            n = v->name;
            d->attrName = strdup(n);
        }
        else if (isA(var, Constant))
        {
            n = CONCAT_STRINGS("C_", gprom_itoa(goalPos), "_", gprom_itoa(argPos++));
            d->attrName = strdup(n);
        }
        else if (isA(var, Operator))
        {
        	Operator *o = (Operator *) var;
        	DLVar *v = (DLVar *) getHeadOfListP(o->args);
        	n = v->name;
        	d->attrName = strdup(n);
        }
        else
            FATAL_LOG("we should not end up here");

        addToSet(nameSet, strdup(n));
        finalNames = appendToTailOfList(finalNames, strdup(n));
    }

    //TODO make attribute names unique
    makeNamesUnique(finalNames, nameSet);
    FORBOTH(void,name,attr,finalNames,rename->op.schema->attrDefs)
    {
        char *n = (char *) name;
        AttributeDef *a = (AttributeDef *) attr;

        if(!streq(a->attrName, name))
            a->attrName = strdup(n);
    }

    DEBUG_LOG("translated goal %s:\n%s",
            datalogToOverviewString((Node *) r),
            operatorToOverviewString((Node *) rename));

    return (QueryOperator *) rename;
}

static List *
connectProgramTranslation(DLProgram *p, HashMap *predToTrans)
{
    List *qoRoots = NIL;

    // for each translated rule find table access operators corresponding to idb predicates
    // and replace them with their algebra expressions
    FOREACH_HASH(QueryOperator,root,predToTrans)
    {
        List *rels = NIL;

        findTableAccessVisitor((Node *) root, &rels);

        // for each rel check whether it is idb
        FOREACH(TableAccessOperator,r,rels)
        {
            boolean isIDB = HAS_STRING_PROP(r,DL_IS_IDB_REL);
            boolean isEDB = HAS_STRING_PROP(r,DL_IS_EDB_REL);
            boolean isFact = HAS_STRING_PROP(r,DL_IS_FACT_REL);
            boolean isDom = HAS_STRING_PROP(r,DL_IS_DOMAIN_REL);

            DEBUG_LOG("check Table %s that is %s%s%s", r->tableName, isIDB ? " idb": "",
                    isEDB ? " edb": "", isFact ? " fact": "");

            // exception on DOMAIN due to the replacement in the summarization step
            if(!isDom)
            	ASSERT(!isIDB || MAP_HAS_STRING_KEY(predToTrans,r->tableName));

            // is idb or fact (which is not edb)
            if((isIDB || (isFact && !isEDB)) && MAP_HAS_STRING_KEY(predToTrans,r->tableName))
            {
                QueryOperator *idbImpl = (QueryOperator *) MAP_GET_STRING(predToTrans,r->tableName);
                switchSubtreeWithExisting((QueryOperator *) r,idbImpl);
                DEBUG_LOG("replaced idb Table %s with\n:%s", r->tableName,
                        operatorToOverviewString((Node *) idbImpl));

                // remove after switch subtree if no provenance question exists
                if(!provQ)
					removeMapStringElem(predToTrans,r->tableName);
            }
            // is fact which is edb
            else if (isFact && isEDB && MAP_HAS_STRING_KEY(predToTrans,r->tableName))
            {
                QueryOperator *idbImpl = (QueryOperator *)
                        MAP_GET_STRING(predToTrans,r->tableName);
                QueryOperator *un;

                un = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(r,idbImpl), NIL,
                        getQueryOperatorAttrNames((QueryOperator *) r));
                addParent(idbImpl, un);
                addParent((QueryOperator *) r, un);

                switchSubtreeWithExisting((QueryOperator *) r,un);
                DEBUG_LOG("replaced idb Table %s with\n:%s", r->tableName,
                        operatorToOverviewString((Node *) un));
            }
            else if (isIDB || isFact)
            	// exception on DOMAIN due to the replacement in the summarization step
            	if(!isDom)
            		FATAL_LOG("Do not find entry for %s", r->tableName);
        }

        qoRoots = appendToTailOfList(qoRoots, root);
        visitQOGraph(root, TRAVERSAL_PRE, adaptProjectionAttrRef, NULL);
    }

    return qoRoots;
}

static boolean
adaptProjectionAttrRef (QueryOperator *o, void *context)
{
    if(isA(o,ProjectionOperator))
    {
        ProjectionOperator *p = (ProjectionOperator *) o;

        FOREACH(Node,pro,p->projExprs)
        {
            List *attrRefs = getAttrReferences (pro);

            FOREACH(AttributeReference,a,attrRefs)
            {
                AttributeDef *def = getAttrDefByPos(OP_LCHILD(o), a->attrPosition);
                char *cName = def->attrName;
                if (!streq(a->name,cName))
                {
                    a->name = strdup(cName);
                    a->attrType = def->dataType;
                }
            }
        }
    }

//    FOREACH(QueryOperator,c,o->inputs)
//        adaptProjectionAttrRef(c);

    return TRUE;
}
