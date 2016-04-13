/*-----------------------------------------------------------------------------
 *
 * translator_dl.c
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
#include "configuration/option.h"
#include "log/logger.h"

#include "metadata_lookup/metadata_lookup.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/game_provenance/gp_main.h"

static Node *translateProgram(DLProgram *p);
static QueryOperator *translateFact(DLAtom *f);
static QueryOperator *translateRule(DLRule *r, boolean typeQ);
static QueryOperator *translateSafeRule(DLRule *r);
static QueryOperator *translateUnSafeRule(DLRule *r, boolean typeQ, boolean caseC);
static QueryOperator *translateGoal(DLAtom *r, int goalPos, boolean typeQ, boolean caseC);
static QueryOperator *translateSafeGoal(DLAtom *r, int goalPos, QueryOperator *posPart);
static void analyzeProgramDTs (DLProgram *p, HashMap *predToRules);
static void analyzeFactDTs (DLAtom *f, HashMap *predToDTs);
static void analyzeRuleDTs (DLRule *r, HashMap *predToDTs, HashMap *predToRules);
static void setVarDTs (Node *expr, HashMap *varToDT);
static QueryOperator *joinGoalTranslations (DLRule *r, List *goalTrans);
static Node *createJoinCondOnCommonAttrs (QueryOperator *l, QueryOperator *r, List *leftOrigAttrs);
static List *getHeadProjectionExprs (DLAtom *head, QueryOperator *joinedGoals, List *bodyArgs);
static Node *replaceDLVarMutator (Node *node, HashMap *vToA);
static Node *createCondFromComparisons (List *comparisons, QueryOperator *in, HashMap *varDTmap);
static List *connectProgramTranslation(DLProgram *p, HashMap *predToTrans);
static void adaptProjectionAttrRef (QueryOperator *o);

static Node *replaceVarWithAttrRef(Node *node, List *context);
boolean typeOfQuestion = FALSE;
boolean caseChecker = FALSE;

Node *
translateParseDL(Node *q)
{
    Node *result = NULL;

    INFO_LOG("translate DL model:\n\n%s", datalogToOverviewString(q));

//    if (!checkDLModel(q))
//        FATAL_LOG("failed model check on:\n%s", datalogToOverviewString(q));

    if (isA(q,DLProgram))
        result = translateProgram((DLProgram *) q);
    // what other node types can be here?
    else
        FATAL_LOG("currently only DLProgram node type translation supported");

    INFO_LOG("translated DL model:\n\n%s", operatorToOverviewString(result));
//    FATAL_LOG("");

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
    List *translation = NIL; // list of sinks of a relational algebra graph
//    List *singleRuleTrans = NIL;
    HashMap *predToTrans = NEW_MAP(Constant,List);
    HashMap *predToRules = NEW_MAP(Constant,List);
    Node *answerRel;
//    Set *edbRels, idbRels, factRels;

    // get names of EDB, IDB and fact predicates
//    idbRels = (Set *) DL_GET_PROP(p, DL_IDB_RELS);
//    edbRels = (Set *) DL_GET_PROP(p, DL_EDB_RELS);
//    factRels = (Set *) DL_GET_PROP(p, DL_FACT_RELS);

    // if we want to compute the provenance then construct program
    // for creating the provenance and translate this one
    if (IS_GP_PROV(p))
    {
        DEBUG_LOG("user asked for provenance computation for:\n%s",
                        datalogToOverviewString((Node *) p));

        // check type of question (e.g., WHY or WHYNOT question)
        if(DL_HAS_PROP(p,DL_PROV_WHY))
        	typeOfQuestion = TRUE;
        else if(DL_HAS_PROP(p,DL_PROV_WHYNOT))
        	typeOfQuestion = FALSE;

//        DEBUG_LOG("type of question: %d", typeOfQuestion);

        Node *gpComp = rewriteForGP((Node *) p);

        ASSERT(!IS_GP_PROV(gpComp));

        checkDLModel(gpComp);
        gpComp = translateParseDL(gpComp);

        return gpComp;
    }

    // determine pred -> rules
    FOREACH(DLRule,r,p->rules)
    {
        char *headPred = getHeadPredName(r);
        APPEND_TO_MAP_VALUE_LIST(predToRules,headPred,r);
//        // not first rule for this pred
//        if(MAP_HAS_STRING_KEY(predToRules,headPred))
//        {
//            KeyValue *kv = MAP_GET_STRING_ENTRY(predToRules,headPred);
//            List *pRules = (List *) kv->value;
//            pRules = appendToTailOfList(pRules, r);
//            kv->value = (Node *) pRules;
//        }
//        // first rule for this pred
//        else
//        {
//            List *pRules = singleton(r);
//            MAP_ADD_STRING_KEY(predToRules,headPred,pRules);
//        }
    }
    FOREACH(DLAtom,f,p->facts)
    {
        char *relName = f->rel;
        APPEND_TO_MAP_VALUE_LIST(predToRules,relName,f);
//        // not first rule for this pred
//        if(MAP_HAS_STRING_KEY(predToRules,relName))
//        {
//            KeyValue *kv = MAP_GET_STRING_ENTRY(predToRules,relName);
//            List *pRules = (List *) kv->value;
//            pRules = appendToTailOfList(pRules, f);
//            kv->value = (Node *) pRules;
//        }
//        // first rule for this pred
//        else
//        {
//            List *pRules = singleton(f);
//            MAP_ADD_STRING_KEY(predToRules,relName,pRules);
//        }
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
//        if(MAP_HAS_STRING_KEY(predToTrans,predName))
//        {
//            KeyValue *kv = MAP_GET_STRING_ENTRY(predToTrans,predName);
//            List *ruleTrans = (List *) kv->value;
//            ruleTrans = appendToTailOfList(ruleTrans, tFact);
//            kv->value = (Node *) ruleTrans;
//        }
//        // first rule for this pred
//        else
//        {
//            List *ruleTrans = singleton(tFact);
//            MAP_ADD_STRING_KEY(predToTrans,predName,ruleTrans);
//        }
    }

    // translate rules
    FOREACH(DLRule,r,p->rules)
    {
        QueryOperator *tRule = translateRule(r,typeOfQuestion);
//        QueryOperator *tRule = translateRule(r);
        char *headPred = getHeadPredName(r);

        DEBUG_LOG("translate rule: %s", datalogToOverviewString((Node *) r));

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
             ASSERT(checkModel((QueryOperator *) tRule));

        APPEND_TO_MAP_VALUE_LIST(predToTrans,headPred,tRule);
//        // not first rule for this pred
//        if(MAP_HAS_STRING_KEY(predToTrans,headPred))
//        {
//            KeyValue *kv = MAP_GET_STRING_ENTRY(predToTrans,headPred);
//            List *ruleTrans = (List *) kv->value;
//            ruleTrans = appendToTailOfList(ruleTrans, tRule);
//            kv->value = (Node *) ruleTrans;
//        }
//        // first rule for this pred
//        else
//        {
//            List *ruleTrans = singleton(tRule);
//            MAP_ADD_STRING_KEY(predToTrans,headPred,ruleTrans);
//        }
//        singleRuleTrans = appendToTailOfList(singleRuleTrans,
//                tRule);
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

    // connect rules by replacing table access operators representing idb
    // relations with the corresponding algebra expression
    translation = connectProgramTranslation(p, predToTrans);

    if (p->ans == NULL)
        return (Node *) translation;

    if (!MAP_HAS_STRING_KEY(predToTrans, p->ans))
    {
        FATAL_LOG("answer relation %s does not exist in program:\n\n%s",
                p->ans, datalogToOverviewString((Node *) p));
    }

    answerRel = MAP_GET_STRING(predToTrans, p->ans);
    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
         ASSERT(checkModel((QueryOperator *) answerRel));

    return answerRel;
}

static void
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

    DEBUG_LOG("analyzed DTs for datalog program before translation: %s", beatify(nodeToString((Node *) p)));
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
                if (isA(arg, DLVar))
                {
                    DLVar *v = (DLVar *) arg;
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

    FOREACH(Node,arg,r->head->args)
        headDTs = appendToTailOfListInt(headDTs, typeOf(arg));

    setDLProp((DLNode *) r->head, DL_PRED_DTS, (Node *) headDTs);
    MAP_ADD_STRING_KEY(predToDTs, r->head->rel, headDTs );
}

static void
setVarDTs (Node *expr, HashMap *varToDT)
{
    List *vars = getDLVarsIgnoreProps (expr);
//    List *vars = getDLVars (expr);
    FOREACH(DLVar,v,vars)
        v->dt = INT_VALUE(MAP_GET_STRING(varToDT,v->name));
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
        attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("c", itoa(pos++)));
    }

    result = (QueryOperator *) createConstRelOp(copyObject(f->args), NIL, attrNames, dts);

    return result;
}

static QueryOperator *
translateRule(DLRule *r, boolean typeQ)
{
    boolean isSafe = FALSE; //TODO implement real safety check in model checker or analyzer

    FOREACH(DLNode,d,r->body) //TODO assume that if we have at least one positive goal then we are safe for now
    {
        if (isA(d,DLAtom) && !((DLAtom *) d)->negated)
            isSafe = TRUE;
    }

    int getPos = 0;
    int varPos = 0;
    int constantPos = 0;

    FOREACH(DLAtom,da,r->body) //TODO need to implement safer way to categorize those num of attributes in relation not equal to in domain
    {
    	FOREACH(Node,at,da->args)
		{
    		if(isA(at,DLVar) && varPos >= constantPos)
    			varPos = getPos;

    		if(isA(at,Constant))
    			constantPos = getPos;

    		// if variable exists before and after constant in the negated goal, then different type of translation
    		if(isA(at,DLVar) && da->negated  && getPos == constantPos+1  && getPos == varPos-1 )
    				caseChecker = TRUE;

    		getPos++;
		}
    }

    if(isSafe)
        return translateSafeRule(r);
    else
//    	return translateUnSafeRule(r);
   		return translateUnSafeRule(r, typeQ, caseChecker);
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
translateUnSafeRule(DLRule *r, boolean typeQ, boolean caseC)
//translateUnSafeRule(DLRule *r)
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
            QueryOperator *tG = translateGoal((DLAtom *) a, goalPos++, typeQ, caseC);
//            QueryOperator *tG = translateGoal((DLAtom *) a, goalPos++);
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
    if (!LIST_EMPTY(conditions))
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
        headNames = appendToTailOfList(headNames,CONCAT_STRINGS("A", itoa(i)));
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
 */
static QueryOperator *
translateSafeRule(DLRule *r)
{
    ProjectionOperator *headP;
    DuplicateRemoval *dupRem;
    QueryOperator *joinedGoals;
    SelectionOperator *sel = NULL;
    List *goalTrans = NIL;
    List *posGoals = NIL;
    List *conditions = NIL;
    QueryOperator *posPart = NULL;
    int goalPos;
    HashMap *varDTmap;

    DEBUG_LOG("translate rules: %s", datalogToOverviewString((Node *) r));

    varDTmap = (HashMap *) getDLProp((DLNode *) r, DL_PRED_DTS);

    // translate positive goals
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
    if (!LIST_EMPTY(conditions))
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
    int i = 0;

//    FOREACH(Node,p,projExprs)
    for (; i < LIST_LENGTH(projExprs); i++)
        headNames = appendToTailOfList(headNames,CONCAT_STRINGS("A", itoa(i)));
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

static List *
getHeadProjectionExprs (DLAtom *head, QueryOperator *joinedGoals, List *bodyArgs)
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
            if (isA(bA, DLVar))
            {
                DLVar *v = (DLVar *) bA;
                AttributeDef *d = (AttributeDef *) a;
                MAP_ADD_STRING_KEY(vToA, v->name,(Node *) LIST_MAKE(
                                    createConstString(d->attrName),
                                    createConstInt(pos),
                                    createConstInt(d->dataType)));
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
    //        if (isA(bA, DLVar))
    //        {
                DLVar *v = (DLVar *) bA;
                AttributeDef *d = (AttributeDef *) a;
                MAP_ADD_STRING_KEY(vToA, v->name,(Node *) LIST_MAKE(
                                    createConstString(d->attrName),
                                    createConstInt(pos),
                                    createConstInt(d->dataType)));
    //        }

            pos++;
        }
    }

    FOREACH(Node,a,headArgs)
    {
        Node *newA = replaceDLVarMutator(a, vToA);
        projExprs = appendToTailOfList(projExprs, newA);
    }

    return projExprs;
}

static Node *
replaceDLVarMutator (Node *node, HashMap *vToA)
{
    if (node == NULL)
        return node;

    if (isA(node, DLVar))
    {
        AttributeReference *a;
        char *hV = ((DLVar *) node)->name;
        List *l = (List *) MAP_GET_STRING(vToA, hV);
        char *name = STRING_VALUE(getNthOfListP(l,0));
        int pos = INT_VALUE(getNthOfListP(l,1));
        DataType dt = (DataType) INT_VALUE(getNthOfListP(l,2));

        a = createFullAttrReference(name,0,pos,INVALID_ATTR, dt);
        return (Node *) a;
    }

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
                    op = createOpExpr("=", LIST_MAKE(lAr,rAr));

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
//        op = createOpExpr("=", LIST_MAKE(lA,rA));
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
            if (streq(d->opExpr->name,"="))
                newC = (Node *) createIsNullExpr(leftIn);
            else if (streq(d->opExpr->name,"!="))
                newC = (Node *) createOpExpr("NOT", singleton(createIsNullExpr(leftIn)));
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
        DEBUG_LOG("create table for goal %s(%s)%s%s%s", table->tableName, \
                stringListToString(getQueryOperatorAttrNames((QueryOperator *) table)), \
                DL_HAS_PROP(dl,DL_IS_IDB_REL) ? " IDB " : "", \
                DL_HAS_PROP(dl,DL_IS_FACT_REL) ? " FACT " : "", \
                DL_HAS_PROP(dl,DL_IS_EDB_REL) ? " EDB " : ""); \
    } while(0)

static QueryOperator *
translateGoal(DLAtom *r, int goalPos, boolean typeQ, boolean caseC)
//translateGoal(DLAtom *r, int goalPos)
{
    ProjectionOperator *renameOnSetDiff;
    QueryOperator *pInput;
    TableAccessOperator *rel;
    List *attrNames = NIL;
    List *dts = NIL;
    boolean isIDB = DL_HAS_PROP(r,DL_IS_IDB_REL);
    boolean isEDB = DL_HAS_PROP(r,DL_IS_EDB_REL);
    boolean isFact = DL_HAS_PROP(r,DL_IS_FACT_REL);

    DEBUG_LOG("goal is marked as %s%s%s",
            isIDB ? " IDB " : "",
            isFact ? " FACT " : "",
            isEDB ? "EDB" : "");

    // for idb rels just fake attributes (and for now DTs)
    if (isIDB || (isFact && !isEDB))
    {
		for(int i = 0; i < LIST_LENGTH(r->args); i++)
			attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("A", itoa(i)));
    }
    // is edb, get information from db
    else
        attrNames = getAttributeNames(r->rel);

    dts = (List *) getDLProp((DLNode *) r, DL_PRED_DTS);

	// create table access op
	rel = createTableAccessOp(r->rel, NULL, "REL", NIL, attrNames, dts);
	COPY_PROPS_TO_TABLEACCESS(rel,r);

    // is negated goal?
    ProjectionOperator *rename;
	ProjectionOperator *pdom;

    if (r->negated)
    {

        SetOperator *setDiff;
        QueryOperator *dom;

    	if (typeQ && caseC) // if question is WHY and variable exists after constant
    	{
    		// add selection if constants are used in the goal
    		// e.g., R(X,1) with attributes A0,A1 are translated into SELECTION[A1=1](R)
    		QueryOperator *sdom;

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
    				comp = (Node *) createOpExpr("=",
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
    	    pdom = (ProjectionOperator *) createProjOnAttrsByName(sdom, varAttrNames);
    	    addChildOperator((QueryOperator *) pdom, sdom);


            // get the number of variables in the goal
            int numAttrs = 0;
            FOREACH(Node,arg,r->args)
            	if (!isA(arg,Constant))
            		numAttrs++;

            // compute Domain X Domain X ... X Domain number of attributes of goal relation R times
            // then return (Domain X Domain X ... X Domain) - R
            dom = (QueryOperator *) createTableAccessOp("_DOMAIN", NULL,
                    "DummyDom", NIL, LIST_MAKE("D"), singletonInt(DT_STRING));
            List *domainAttrs = singleton("D");

            for(int i = 1; i < numAttrs; i++)
            {
                char *aDomAttrName = CONCAT_STRINGS("D", itoa(i++));
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

            rename = (ProjectionOperator *) createProjOnAllAttrs(dom);

////			// cast data type if typeQ is true
////            List *newDataType = NIL;
//			FORBOTH(AttributeDef,p,r,pdom->op.schema->attrDefs,rename->op.schema->attrDefs)
//            {
////				newDataType = copyObject(createCasts(p,r));
//				if(p->dataType != r->dataType)
//				{
//					if(p->dataType == DT_STRING)
//						r->dataType = DT_STRING;
//					if(r->dataType == DT_STRING)
//						p->dataType = DT_STRING;
//				}
//            }
////			pdom->op.schema->attrDefs = copyObject(newDataType);

            // rename attribute names
    //        List *newAttrNames = NIL;
    //        int posAttr =0;
    //
    //        FOREACH(Node,arg,r->args)
    //		{
    //			if (!isA(arg,Constant))
    //			{
    //				newAttrNames = appendToTailOfList(newAttrNames, CONCAT_STRINGS("A", itoa(posAttr)));
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
    	else
   		{
//            SetOperator *setDiff;
//            ProjectionOperator *rename;
//            QueryOperator *dom;

            int numAttrs = getNumAttrs((QueryOperator *) rel);
            // compute Domain X Domain X ... X Domain number of attributes of goal relation R times
            // then return (Domain X Domain X ... X Domain) - R
            dom = (QueryOperator *) createTableAccessOp("_DOMAIN", NULL,
                    "DummyDom", NIL, LIST_MAKE("D"), singletonInt(DT_STRING));
            List *domainAttrs = singleton("D");

            for(int i = 1; i < numAttrs; i++)
            {
                char *aDomAttrName = CONCAT_STRINGS("D", itoa(i++));
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

            rename = (ProjectionOperator *) createProjOnAllAttrs(dom);

            // rename attribute names
    //        List *newAttrNames = NIL;
    //        int posAttr =0;
    //
    //        FOREACH(Node,arg,r->args)
    //		{
    //			if (!isA(arg,Constant))
    //			{
    //				newAttrNames = appendToTailOfList(newAttrNames, CONCAT_STRINGS("A", itoa(posAttr)));
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

            setDiff = createSetOperator(SETOP_DIFFERENCE, LIST_MAKE(dom, rel),
                    NULL, deepCopyStringList(attrNames));
            addParent(dom, (QueryOperator *) setDiff);
            addParent((QueryOperator *) rel, (QueryOperator *) setDiff);

            pInput = (QueryOperator *) setDiff;
   		}
    }
    else
        pInput = (QueryOperator *) rel;

    // do not generate selection if WHY and negated goal
    if (!(typeQ && caseC && r->negated))
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
                comp = (Node *) createOpExpr("=",
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
                            comp = (Node *) createOpExpr("=",
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

    if (!(typeQ && caseC && r->negated))
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
                n = CONCAT_STRINGS("C_", itoa(goalPos), "_", itoa(argPos++));
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
                n = CONCAT_STRINGS("C_", itoa(goalPos), "_", itoa(argPos++));
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

    DEBUG_LOG("goal is marked as %s%s%s",
            isIDB ? " IDB " : "",
            isFact ? " FACT " : "",
            isEDB ? "EDB" : "");

    // for idb rels just fake attributes (and for now DTs)
    if (isIDB || (isFact && !isEDB))
    {
        for(int i = 0; i < LIST_LENGTH(r->args); i++)
        	attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("A", itoa(i)));
    }
    // is edb, get information from db
    else
        attrNames = getAttributeNames(r->rel);

    dts = (List *) getDLProp((DLNode *) r, DL_PRED_DTS);

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
//        	char *aDomAttrName = CONCAT_STRINGS("D", itoa(i++));
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
                	char *newProjName = CONCAT_STRINGS(name, itoa(attrPos));
                	projNames = appendToTailOfList(projNames, strdup(newProjName));
                }
            }
            else
            {
                projArgs = appendToTailOfList(projArgs, copyObject(n));
                projNames = appendToTailOfList(projNames, CONCAT_STRINGS("C_", itoa(attrPos)));
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
            comp = (Node *) createOpExpr("=",
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
                        comp = (Node *) createOpExpr("=",
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
    addChildOperator((QueryOperator *) rename, pInput);

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
            n = CONCAT_STRINGS("C_", itoa(goalPos), "_", itoa(argPos++));
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

            DEBUG_LOG("check Table %s that is %s%s%s", r->tableName, isIDB ? " idb": "",
                    isEDB ? " edb": "", isFact ? " fact": "");
            ASSERT(!isIDB || MAP_HAS_STRING_KEY(predToTrans,r->tableName));

            // is idb or fact (which is not edb)
            if((isIDB || (isFact && !isEDB)) && MAP_HAS_STRING_KEY(predToTrans,r->tableName))
            {
                QueryOperator *idbImpl = (QueryOperator *) MAP_GET_STRING(predToTrans,r->tableName);
                switchSubtreeWithExisting((QueryOperator *) r,idbImpl);
                DEBUG_LOG("replaced idb Table %s with\n:%s", r->tableName,
                        operatorToOverviewString((Node *) idbImpl));
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
                FATAL_LOG("Do not find entry for %s", r->tableName);
        }

        qoRoots = appendToTailOfList(qoRoots, root);
        adaptProjectionAttrRef(root);
    }

    return qoRoots;
}

static void
adaptProjectionAttrRef (QueryOperator *o)
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

    FOREACH(QueryOperator,c,o->inputs)
        adaptProjectionAttrRef(c);
}

