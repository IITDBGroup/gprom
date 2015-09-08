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
static QueryOperator *translateRule(DLRule *r);
static QueryOperator *translateGoal(DLAtom *r, int goalPos);
static void analyzeProgramDTs (DLProgram *p, HashMap *predToRules);
static void analyzeRuleDTs (DLRule *r, HashMap *predToDTs, HashMap *predToRules);
static void setVarDTs (Node *expr, HashMap *varToDT);
static QueryOperator *joinGoalTranslations (DLRule *r, List *goalTrans);
static Node *createJoinCondOnCommonAttrs (QueryOperator *l, QueryOperator *r, List *leftOrigAttrs);
static List *getHeadProjectionExprs (DLAtom *head, QueryOperator *joinedGoals, List *bodyArgs);
static Node *replaceDLVarMutator (Node *node, HashMap *vToA);
static Node *createCondFromComparisons (List *comparisons, QueryOperator *in, HashMap *varDTmap);
static void makeNamesUnique (List *names, Set *allNames);
static List *connectProgramTranslation(DLProgram *p, HashMap *predToTrans);
static void adaptProjectionAttrRef (QueryOperator *o);

static Node *replaceVarWithAttrRef(Node *node, List *context);

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

static Node *
translateProgram(DLProgram *p)
{
    List *translation = NIL; // list of sinks of a relational algebra graph
    List *singleRuleTrans = NIL;
    HashMap *predToTrans = NEW_MAP(Constant,List);
    HashMap *predToRules = NEW_MAP(Constant,List);
    Node *answerRel;

    // if we want to compute the provenance then construct program
    // for creating the provenance and translate this one
    if (IS_GP_PROV(p))
    {
        Node *gpComp = rewriteForGP((Node *) p);

        DEBUG_LOG("user asked for provenance computation for:\n%s",
                datalogToOverviewString((Node *) p));

        return translateParseDL(gpComp);
    }

    // determine pred -> rules
    FOREACH(DLRule,r,p->rules)
    {
        char *headPred = getHeadPredName(r);

        // not first rule for this pred
        if(MAP_HAS_STRING_KEY(predToRules,headPred))
        {
            KeyValue *kv = MAP_GET_STRING_ENTRY(predToRules,headPred);
            List *pRules = (List *) kv->value;
            pRules = appendToTailOfList(pRules, r);
            kv->value = (Node *) pRules;
        }
        // first rule for this pred
        else
        {
            List *pRules = singleton(r);
            MAP_ADD_STRING_KEY(predToRules,headPred,pRules);
        }
    }

    // analyze rules to determine data types
    analyzeProgramDTs(p, predToRules);

    // translate rules
    FOREACH(DLRule,r,p->rules)
    {
        QueryOperator *tRule = translateRule(r);
        char *headPred = getHeadPredName(r);

        DEBUG_LOG("translate rule: %s", datalogToOverviewString((Node *) r));

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
             ASSERT(checkModel((QueryOperator *) tRule));

        // not first rule for this pred
        if(MAP_HAS_STRING_KEY(predToTrans,headPred))
        {
            KeyValue *kv = MAP_GET_STRING_ENTRY(predToTrans,headPred);
            List *ruleTrans = (List *) kv->value;
            ruleTrans = appendToTailOfList(ruleTrans, tRule);
            kv->value = (Node *) ruleTrans;
        }
        // first rule for this pred
        else
        {
            List *ruleTrans = singleton(tRule);
            MAP_ADD_STRING_KEY(predToTrans,headPred,ruleTrans);
        }
        singleRuleTrans = appendToTailOfList(singleRuleTrans,
                tRule);
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
    // relations with the corresponding alegebra expression
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

    // determine data types recursively
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
translateRule(DLRule *r)
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
            QueryOperator *tG = translateGoal((DLAtom *) a, goalPos++);
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

static List *
getHeadProjectionExprs (DLAtom *head, QueryOperator *joinedGoals, List *bodyArgs)
{
    List *headArgs = head->args;
    List *projExprs = NIL;
    HashMap *vToA = NEW_MAP(Constant,Constant);
    int pos = 0;

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

static void
makeNamesUnique (List *names, Set *allNames)
{
    HashMap *nCount = NEW_MAP(Constant,Constant);

    FOREACH_LC(lc,names)
    {
        char *oldName = LC_P_VAL(lc);
        char *newName = oldName;
        int count = MAP_INCR_STRING_KEY(nCount,oldName);

        if (count != 0)
        {
            // create a new unique name
            newName = CONCAT_STRINGS(oldName, itoa(count));
            while(hasSetElem(allNames, newName))
                newName = CONCAT_STRINGS(oldName, itoa(++count));
        }

        LC_P_VAL(lc) = newName;
    }
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

static QueryOperator *
translateGoal(DLAtom *r, int goalPos)
{
    ProjectionOperator *rename;
    QueryOperator *pInput;
    TableAccessOperator *rel;
    List *attrNames = NIL;
    List *dts = NIL;

    // for idb rels just fake attributes (and for now DTs)
    if (DL_HAS_PROP(r,DL_IS_IDB_REL))
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
    if (DL_HAS_PROP(r,DL_IS_IDB_REL))
        SET_BOOL_STRING_PROP(rel, DL_IS_IDB_REL);

    // is negated goal?
    if (r->negated)
    {
        SetOperator *setDiff;
        ProjectionOperator *rename;
        QueryOperator *dom;
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
					LIST_MAKE(aDomAttrName), singletonInt(DT_STRING));
//            		LIST_MAKE("D"), singletonInt(DT_STRING));
            QueryOperator *oldD = dom;
            domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),aDomAttrName);
//            domainAttrs = appendToTailOfList(deepCopyStringList(domainAttrs),"D");
            dom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL,
                    LIST_MAKE(dom, aDom), NULL,
                    domainAttrs);
            addParent(aDom, dom);
            addParent(oldD, dom);
        }

        rename = (ProjectionOperator *) createProjOnAllAttrs(dom);
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

            if (a->dataType != ((Constant *) arg)->constType) // check for unmatched dataType
            	a->dataType = ((Constant *) arg)->constType;  // , but need discussion if it is correct to check

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
            DEBUG_LOG("check Table %s that is %s", r->tableName, isIDB ? " idb": " edb");
            ASSERT(!isIDB || MAP_HAS_STRING_KEY(predToTrans,r->tableName));

            if(isIDB && MAP_HAS_STRING_KEY(predToTrans,r->tableName))
            {
                QueryOperator *idbImpl = (QueryOperator *) MAP_GET_STRING(predToTrans,r->tableName);
                switchSubtreeWithExisting((QueryOperator *) r,idbImpl);
                DEBUG_LOG("replaced idb Table %s with\n:%s", r->tableName,
                        operatorToOverviewString((Node *) idbImpl));
            }
            else if (isIDB)
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

