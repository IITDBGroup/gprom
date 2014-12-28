/*-----------------------------------------------------------------------------
 *
 * gp_bottom_up_program.c
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
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/datalog/datalog_model.h"
#include "provenance_rewriter/game_provenance/gp_bottom_up_program.h"
#include "utility/string_utils.h"

#define CONCAT_MAP_LIST(map,key,newList) \
    do { \
        List *old = (List *) getMap(map,key); \
        old = CONCAT_LISTS(newList, old); \
        addToMap(map,key,(Node *) old); \
    } while (0)

#define NON_LINKED_POSTFIX "_nonlinked"

#define NORM_COPY(result,a) \
	do { \
	    result = getNormalizedAtom(copyObject((DLAtom *) a));   \
	    result->negated = FALSE; \
	    ((DLNode *) result)->properties = NULL; \
    } while (0)

#define AD_NORM_COPY(result,a) \
    do { \
        result = getNormalizedAtom(copyObject((DLAtom *) a));   \
        result->negated = FALSE; \
        DLNode *_p = (DLNode *) result; \
        _p->properties = NULL; \
        DL_COPY_PROP(a,_p,DL_WON); \
        DL_COPY_PROP(a,_p,DL_LOST); \
        DL_COPY_PROP(a,_p,DL_UNDER_NEG_WON); \
        DL_COPY_PROP(a,_p,DL_UNDER_NEG_LOST); \
    } while (0)

static DLProgram *createWhyGPprogram (DLProgram *p, DLAtom *why);
static DLProgram *createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot);
static DLProgram *createFullGPprogram (DLProgram *p);

static void enumerateRules (DLProgram *p);
static List *removeVars (List *vars, List *remVars);
static List *makeUniqueVarList (List *vars);
static void setIDBBody (DLRule *r);

static DLProgram *rewriteSolvedProgram (DLProgram *solvedProgram);
static DLRule *createMoveRule(Node *lExpr, Node *rExpr, char *bodyAtomName, List *bodyArgs);
static Node *createSkolemExpr (GPNodeType type, char *id, List *args);

static DLProgram *unifyProgram (DLProgram *p, DLAtom *question);
static void unifyOneWithRuleHeads(HashMap *pToR, HashMap *rToUn, DLAtom *curAtom);
static DLProgram *solveProgram (DLProgram *p, DLAtom *question, boolean neg);

DLProgram *
createBottomUpGPprogram (DLProgram *p)
{
    DEBUG_LOG("create GP bottom up program for:\n%s",
            beatify(nodeToString(p)));
    INFO_LOG("create GP bottom up program for:\n%s",
            datalogToOverviewString((Node *) p));
    // why provenance
    if(DL_HAS_PROP(p,DL_PROV_WHY))
    {
        DLAtom *why = (DLAtom *) getDLProp((DLNode *) p,DL_PROV_WHY);
        return createWhyGPprogram(p, why);
    }
    // why not
    else if(DL_HAS_PROP(p,DL_PROV_WHYNOT))
    {
        DLAtom *whyN = (DLAtom *) getDLProp((DLNode *) p,DL_PROV_WHYNOT);
        return createWhyNotGPprogram(p, whyN);
    }
    // full GP
    else if(DL_HAS_PROP(p,DL_PROV_FULL_GP))
        return createFullGPprogram(p);

    return p;
}

static DLProgram *
createWhyGPprogram (DLProgram *p, DLAtom *why)
{
    DLProgram *solvedProgram;
//    DLProgram *result;

    enumerateRules (p);
    solvedProgram = copyObject(p);
    solvedProgram = unifyProgram(solvedProgram, why);
    solvedProgram = solveProgram(solvedProgram, why, FALSE);

    p->n.properties = NULL;
    setDLProp((DLNode *) solvedProgram, DL_PROV_PROG, (Node *) p);

    solvedProgram = rewriteSolvedProgram(solvedProgram);
    DL_DEL_PROP(solvedProgram, DL_PROV_WHY);

    INFO_LOG("program for computing Why-prov: %s",
            datalogToOverviewString((Node *) solvedProgram));
//    FATAL_LOG("solvedProgram: %s", datalogToOverviewString((Node *) solvedProgram));

    return solvedProgram;
}

static DLProgram *
createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot)
{
    return p;
}

static DLProgram *
createFullGPprogram (DLProgram *p)
{
    return p;
}

/*
 * Rewrite program based on adornments created when statically solving the game for the program.
 * We use the solved program to created rules for each adorned version of a unified rule/goal atom
 */

static DLProgram *
rewriteSolvedProgram (DLProgram *solvedProgram)
{
    DLProgram *result = makeNode(DLProgram);
    List *newRules = NIL;
    List *unLinkedRules = NIL;
    List *helpRules = NIL;
    List *edbRules = NIL;
    List *moveRules = NIL;
    Set *adornedEDBAtoms = NODESET();
    HashMap *idbAdToRules = NEW_MAP(Node,Node);

    result->rules = copyObject(solvedProgram->rules);

    // collect rules and adornedheads we are interested in

    // create rules for Rule_i^adornment:
    //  - create head^adornment :- rule^adornment
    //  - create rule^adornment :- adornedBody
    FOREACH(DLRule,r,solvedProgram->rules)
    {
        boolean ruleWon = DL_HAS_PROP(r,DL_WON)
                           || DL_HAS_PROP(r,DL_UNDER_NEG_WON);
        boolean ruleNeg = DL_HAS_PROP(r,DL_UNDER_NEG_WON)
                           || DL_HAS_PROP(r,DL_UNDER_NEG_LOST);
        char *adHeadName = CONCAT_STRINGS("R", r->head->rel, "_",
                ruleWon ? "WON" : "LOST", "_" ,
                ruleNeg ? "-" : "+");
        char *adRuleName = CONCAT_STRINGS("r",
                itoa(INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))), "_",
                ruleWon ? "WON" : "LOST", "_" ,
                ruleNeg ? "-" : "+");
        List *newRuleArgs = NIL;

        // ************************************************************
        // create rule rule^adornment :- adornedBody
        DEBUG_LOG("create GP RULE rule for %s based on rule:\n%s", adRuleName,
                datalogToOverviewString((Node *) r));
        DLRule *ruleRule = copyObject(r);
        newRuleArgs = removeVars(makeUniqueVarList(getRuleVars(ruleRule)),
                ruleRule->head->args);
        newRuleArgs = CONCAT_LISTS(ruleRule->head->args, newRuleArgs);

        DEBUG_LOG("new args for rule head are:, %s",
                datalogToOverviewString((Node *) newRuleArgs));

        // not under negated rule
        if (!ruleNeg)
        {
            // won rule not under neg - replace Q(X) :- g_i,1(Y_1), ... g_i,n(Y_n) with Q(X,Y) :- ...
            if (ruleWon)
            {
                // add all vars to head
                ruleRule->head->rel = CONCAT_STRINGS(adRuleName,
                        NON_LINKED_POSTFIX);
                ruleRule->head->args = copyObject(newRuleArgs);
                setDLProp((DLNode *) ruleRule->head, DL_ORIG_ATOM,
                        (Node *) copyObject(r->head));

                // adapt goal nodes
                FOREACH(DLAtom,a,ruleRule->body) //TODO comparison atoms
                {
                    // if an edb atom
                    if (!DL_HAS_PROP(a, DL_IS_IDB_REL))
                    {
                        DLAtom *at;
                        AD_NORM_COPY(at,a);
                        addToSet(adornedEDBAtoms, at);
                    }
                    boolean ruleWon = DL_HAS_PROP(r,DL_WON)
                                       || DL_HAS_PROP(r,DL_UNDER_NEG_WON);
                    boolean ruleNeg = DL_HAS_PROP(r,DL_UNDER_NEG_WON)
                                       || DL_HAS_PROP(r,DL_UNDER_NEG_LOST);
                    char *adHeadName = CONCAT_STRINGS("R", a->rel, "_",
                            ruleWon ? "WON" : "LOST", "_" ,
                            ruleNeg ? "-" : "+",
                            NON_LINKED_POSTFIX);

                    setDLProp((DLNode *) a, DL_ORIG_ATOM, (Node *) copyObject(a));
                    a->rel = adHeadName;
                }
            }
            // lost rules
            else
            {
                //TODO
            }
        }
        // under negated rule
        else
        {
            // won rule under negation
            if (ruleWon)
            {
                //TODO
            }
            // lost rule under negation
            else
            {
                //TODO
            }
        }

//        setIDBBody(ruleRule);
        DEBUG_LOG("created new rule:\n%s", datalogToOverviewString((Node *) ruleRule));
        unLinkedRules = appendToTailOfList(unLinkedRules, ruleRule);

        // create rule head^adornment :- rule^adornment
        DEBUG_LOG("create GP HEAD rule for %s based on rule:\n%s", adHeadName, datalogToOverviewString((Node *) r));
        DLRule *headRule = makeNode(DLRule);
        DLAtom *adHead = copyObject(r->head);
        DLAtom *ruleAtom = makeNode(DLAtom);

        // create head head^adornment(X) if head(X) for original rule
        headRule->head = adHead;
//        adHead->rel = strdup(adHeadName);
        adHead->rel = CONCAT_STRINGS(strdup(adHeadName), NON_LINKED_POSTFIX);
        setDLProp((DLNode *) adHead, DL_ORIG_ATOM,
                   (Node *) copyObject(r->head));

        // create rule atom rule^adornment(X) if rule(X) where X are all vars in rule
        ruleAtom->rel = CONCAT_STRINGS(strdup(adRuleName), NON_LINKED_POSTFIX);
        ruleAtom->args = copyObject(newRuleArgs);
        headRule->body = singleton(ruleAtom);

        DLAtom *lookupAtom;
        AD_NORM_COPY(lookupAtom,headRule->head);
//        DL_SET_BOOL_PROP((getDLProp((DLNode *) lookupAtom, DL_ORIG_ATOM)), DL_IS_IDB_REL);

        CONCAT_MAP_LIST(idbAdToRules,(Node *) lookupAtom, singleton(ruleRule));
        DEBUG_LOG("created new head rule:\n%s", datalogToOverviewString((Node *) headRule));
        helpRules = appendToTailOfList(helpRules, headRule);
    }

    DEBUG_LOG("------------- STEP 1 ---------------\n: created unlinked rules:\n%s\nand help rules:\n%s",
             datalogToOverviewString((Node *) unLinkedRules),
             datalogToOverviewString((Node *) helpRules));

    // create rules adorned edb atoms R^adornment
    //  - create R^adornment(X) :- R(X) if Won+
    //  - create R^adornment(X) :- not R(X) if Lost+
    //  - create R^adornment(X,TRUE) :- R(X), R^adornment(X,FALSE) :- not R(X) if Won-
    //  - create R^adornment(X,FALSE) :- R(X), R^adornment(X,TRUE) :- not R(X) if Lost-
    FOREACH_SET(DLAtom,edb,adornedEDBAtoms)
    {
        DLRule *atRule;
        DLAtom *atHead;
        DLAtom *atBody;
        DLRule *atNegRule = NULL;

        boolean ruleWon = DL_HAS_PROP(edb,DL_WON)
                                       || DL_HAS_PROP(edb,DL_UNDER_NEG_WON);
        boolean ruleNeg = DL_HAS_PROP(edb,DL_UNDER_NEG_WON)
                                       || DL_HAS_PROP(edb,DL_UNDER_NEG_LOST);
        char *adAtomName = CONCAT_STRINGS("R", edb->rel, "_",
                ruleWon ? "WON" : "LOST", "_" ,
                        ruleNeg ? "-" : "+",
                NON_LINKED_POSTFIX);

        atHead = copyObject(edb);
        atHead->rel = adAtomName;
        atBody = copyObject(edb);

        // is under negated
        if (ruleNeg)
        {
            //TODO
            atRule = createDLRule(atHead, singleton(atBody));
        }
        else
        {
            atRule = createDLRule(atHead, singleton(atBody));
            if (!ruleWon)
                atBody->negated = TRUE;
        }

        // add rules to new rules list
        setDLProp((DLNode *) atRule->head, DL_ORIG_ATOM, (Node *) edb);
        DLAtom *lookup;
        AD_NORM_COPY(lookup, atRule->head);
        CONCAT_MAP_LIST(idbAdToRules,(Node *) lookup,singleton(atRule));
        edbRules = appendToTailOfList(edbRules, atRule);
        DEBUG_LOG("new EDB rule generated:\n%s",
                datalogToOverviewString((Node *) atRule));

        if (ruleNeg)
        {
            edbRules = appendToTailOfList(edbRules, atNegRule);
            setDLProp((DLNode *) atNegRule->head, DL_ORIG_ATOM, (Node *) edb);
            DEBUG_LOG("new EDB rule generated:\n%s",
                    datalogToOverviewString((Node *) atNegRule));
        }
    }

    FOREACH(DLRule,r,unLinkedRules)
        setIDBBody(r);

    DEBUG_LOG("------------- STEP 2 ---------------\n: created unlinked rules:\n%s\nand help rules:\n%sand EDB rules:\n%s",
             datalogToOverviewString((Node *) unLinkedRules),
             datalogToOverviewString((Node *) helpRules),
             datalogToOverviewString((Node *) edbRules));


//    DEBUG_LOG("create head to unlinked rules mappings:\n%s",
//            beatify(nodeToString((Node *) idbAdToRules)));

    // ************************************************************
    // create rule rule_i^adornment(X) :- R_unlinked(X) rule_j^adornment_unlinked(X,Y)
    // for every pattern 1) rule_j^adornment_unlinked(X,Y) :- ..., R(X) ...
    //                   2) R_unlinked(X) -> rule_i^adornment_unlinked(X)
    // i.e., starting from the user question atom we check - one hop of a time - that all
    // intermediate tuples which we include in the game provenance are actually needed
    // to explain the user question. That is they are reachable from the user question atom
    Set *unHeadToRules = NODESET();
    FOREACH(DLRule,unRule,unLinkedRules)
    {
        // for each goal lookup all rules creating goal
        FOREACH(Node,a,unRule->body)
        {
            // ignore comparison atoms
            if (isA(a,DLAtom))
            {
                DLAtom *at = (DLAtom *) copyObject(a);
                DLAtom *lookup;
                AD_NORM_COPY(lookup,a);
                DL_DEL_PROP(at,DL_IS_IDB_REL);
                List *goalRules = (List *) getMap(idbAdToRules, (Node *) lookup);
                DEBUG_LOG("create link rules between %s and rule %s\nusing rules:\n%s",
                        datalogToOverviewString((Node *) lookup),
                        datalogToOverviewString((Node *) unRule),
                        datalogToOverviewString((Node *) goalRules));
                ASSERT(goalRules != NIL);

                // create unlink rule for each rule that has goal atom as head
                FOREACH(DLRule,gRule,goalRules)
                {
                    DLRule *linkRule = makeNode(DLRule);
                    DLAtom *goalGoal;
                    DLAtom *ruleGoal;
                    DLAtom *atCopy = copyObject(at);
                    int goalRuleId = DL_HAS_PROP(gRule,DL_RULE_ID) ?
                            INT_VALUE(DL_GET_PROP(gRule, DL_RULE_ID)) : -1;
                    int relNumArgs = LIST_LENGTH(at->args);
                    DLRule *dummyRule;

                    DEBUG_LOG("back link rule\n%s to\n%s",
                            datalogToOverviewString((Node *) gRule),
                            datalogToOverviewString((Node *) unRule));

                    /*
                     * unRule: rX  :- ... at ...;
                     * at :- rY_UN;
                     * gRule: rY_UN  :- ...;
                     *
                     * create rY :- rY_UN, rX;
                     */

                    // create goal for the rel that the rule has in its body and unify with link rule head
                    goalGoal = copyObject(gRule->head);

                    // create goal for the rule that goal in its body and unify vars with link rule head
                    ruleGoal = copyObject(unRule->head);
                    ruleGoal->rel = strRemPostfix(ruleGoal->rel,
                            strlen(NON_LINKED_POSTFIX));

                    // create unique variable names for both rule atoms
                    dummyRule = createDLRule(ruleGoal, singleton(atCopy));
                    makeVarNamesUnique(LIST_MAKE(goalGoal, dummyRule));

                    DEBUG_LOG("after making names unique:\n%s\n%s",
                            datalogToOverviewString((Node *) goalGoal),
                            datalogToOverviewString((Node *) dummyRule));

                    // head is the rule atom with args from goal for this rule
                    linkRule->head = copyObject(goalGoal);
                    linkRule->head->rel = strRemPostfix(linkRule->head->rel,
                            strlen(NON_LINKED_POSTFIX));
                    if (goalRuleId != -1)
                        setDLProp((DLNode *) linkRule, DL_RULE_ID,
                                (Node *) createConstInt(goalRuleId));

                    ruleGoal = (DLAtom *) applyVarMapAsLists((Node *) ruleGoal ,
                            copyObject(atCopy->args),
                            sublist(copyObject(linkRule->head->args), 0, relNumArgs - 1));
//                    ruleGoal = (DLAtom *) applyVarMapAsLists((Node *) ruleGoal,
//                            copyObject(ruleGoal->args),
//                            copyObject(linkRule->head->args));

                    addToSet(unHeadToRules, copyObject(gRule->head));
                    linkRule->body = LIST_MAKE(ruleGoal, goalGoal);

                    // is a "ruleRule"
                    if (DL_HAS_PROP(gRule, DL_RULE_ID))
                    {
                        DEBUG_LOG("created back link rule:\n%s", datalogToOverviewString((Node *) linkRule));
                        newRules = appendToTailOfList(newRules, linkRule);
                    }
                    // is a "edb rule"
                    else
                    {
                        DEBUG_LOG("created back link EDB rule:\n%s", datalogToOverviewString((Node *) linkRule));
                        helpRules = appendToTailOfList(helpRules, linkRule);
                    }
                }
            }
        }
    }

    // create dummy rules ri^adornment(X) := ri^adornment_unlinked(X) for unlinked rules that have no linked versions yet
    FOREACH(DLRule,unRule,unLinkedRules)
    {
        if (!hasSetElem(unHeadToRules,unRule->head))
        {
            DLRule *dummyRule = makeNode(DLRule);
            DLAtom *dummyBody;

            dummyRule->head = copyObject(unRule->head);
            dummyRule->head->rel = strRemPostfix(dummyRule->head->rel,
                    strlen(NON_LINKED_POSTFIX));
            dummyBody = copyObject(unRule->head);
            dummyRule->body = singleton(dummyBody);
            ((DLNode *) dummyRule)->properties =
                    copyObject(((DLNode *) unRule)->properties);

            DEBUG_LOG("created rule for still unlinked rule:\n %s",
                    datalogToOverviewString((Node *) dummyRule));
            newRules = appendToTailOfList(newRules, dummyRule);
        }
    }

    // remove unRules which have been transformed into newRules
//    FOREACH(DLRule,r,newRules)
//        unLinkedRules = REMOVE_FROM_LIST_PTR(unLinkedRules, r);

    DEBUG_LOG("------------- STEP 3 ---------------\ncreated unlinked rules:\n%s\nand linked rules:\n%s\nand help rules:\n%sand EDB rules:\n%s",
            datalogToOverviewString((Node *) unLinkedRules),
            datalogToOverviewString((Node *) newRules),
            datalogToOverviewString((Node *) helpRules),
            datalogToOverviewString((Node *) edbRules));

    // create rules for move relation
    // for each edb rule create two move entries
    FOREACH(DLRule,e,edbRules)
    {
        boolean ruleWon = DL_HAS_PROP(e->head,DL_WON)
                                       || DL_HAS_PROP(e->head,DL_UNDER_NEG_WON);
        // if is won then we
        if (ruleWon)
        {
            ASSERT(DL_HAS_PROP(e->head,DL_ORIG_ATOM));
            DLAtom *origAtom = (DLAtom *) DL_GET_PROP(e->head,DL_ORIG_ATOM);
            char *rel = CONCAT_STRINGS(strdup(origAtom->rel), "_WON");
            char *negRel = CONCAT_STRINGS(strdup(origAtom->rel), "_LOST");
            char *headName = strRemPostfix(strdup(e->head->rel),
                    strlen(NON_LINKED_POSTFIX));

            Node *lExpr = createSkolemExpr(GP_NODE_NEGREL, negRel, e->head->args);
            Node *rExpr = createSkolemExpr(GP_NODE_POSREL, rel, e->head->args);
            DLRule *moveRule = createMoveRule(lExpr, rExpr, headName, e->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);

            lExpr = createSkolemExpr(GP_NODE_POSREL, rel, e->head->args);
            rExpr = createSkolemExpr(GP_NODE_EDB, negRel, e->head->args);
            moveRule = createMoveRule(lExpr, rExpr, headName, e->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);

            DEBUG_LOG("NEW MOVE RULE: negR-atom -> R-atom -> R: %s", headName);
        }
//        else
//        {
//            Node *lExpr = createSkolemExpr(GP_NODE_POSREL, e->head->rel, e->head->args);
//            Node *rExpr = createSkolemExpr(GP_NODE_EDB, e->head->rel, e->head->args);
//
//            DLRule *moveRule = createMoveRule(lExpr, rExpr, e->head->rel, e->head->args);
//            moveRules = appendToTailOfList(moveRules, moveRule);
//        }

    }

    // for each rule_i do head -> rule_i, rule_i -> goal_i_j, goal_i_j -> posR/negR -> posR?
    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,DL_WON)
                                       || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
//        boolean ruleNeg = DL_HAS_PROP(r->head,DL_UNDER_NEG_WON)
//                                       || DL_HAS_PROP(r->head,DL_UNDER_NEG_LOST);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);
        char *headRel = CONCAT_STRINGS(
                strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *ruleRel = CONCAT_STRINGS(
                CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
                !ruleWon ? "_WON" : "_LOST");
        int i = INT_VALUE(DL_GET_PROP(r,DL_RULE_ID));
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel), strlen(NON_LINKED_POSTFIX));

        // head -> rule_i
        Node *lExpr = createSkolemExpr(GP_NODE_POSREL, headRel, copyObject(origAtom->args));
        Node *rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel, copyObject(r->head->args));
        DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName, r->head->args);
        moveRules = appendToTailOfList(moveRules, moveRule);

        // rule_i -> goal_i_j -> posR/negR -> posR
        FOREACH(DLAtom,a,r->body)
        {
            ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
            DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,DL_ORIG_ATOM);
            char *goalRel = CONCAT_STRINGS(itoa(i), "_", itoa(j),
                    ruleWon ? "_WON" : "_LOST");
            char *atomRel = CONCAT_STRINGS(
                    strdup(origAtom->rel),
                    ruleWon ? "_WON" : "_LOST");
            char *negAtomRel = CONCAT_STRINGS(
                    strdup(origAtom->rel),
                    !ruleWon ? "_WON" : "_LOST");

            // -> posR
            if (a->negated)
            {
                Node *lExpr = createSkolemExpr(GP_NODE_RULE, ruleRel, r->head->args);
                Node *rExpr = createSkolemExpr(GP_NODE_GOAL, goalRel, copyObject(a->args));
                DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName, r->head->args);
                moveRules = appendToTailOfList(moveRules, moveRule);

                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel, copyObject(a->args));
                rExpr = createSkolemExpr(GP_NODE_POSREL, atomRel, copyObject(a->args));
                moveRule = createMoveRule(lExpr, rExpr, linkedHeadName, r->head->args);
                moveRules = appendToTailOfList(moveRules, moveRule);
            }
            // -> negR -> posR
            else
            {
                Node *lExpr = createSkolemExpr(GP_NODE_RULE, ruleRel, r->head->args);
                Node *rExpr = createSkolemExpr(GP_NODE_GOAL, goalRel, copyObject(a->args));
                DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName, r->head->args);
                moveRules = appendToTailOfList(moveRules, moveRule);

                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel, copyObject(a->args));
                rExpr = createSkolemExpr(GP_NODE_NEGREL, negAtomRel, copyObject(a->args));
                moveRule = createMoveRule(lExpr, rExpr, linkedHeadName, r->head->args);
                moveRules = appendToTailOfList(moveRules, moveRule);

                lExpr = createSkolemExpr(GP_NODE_NEGREL, negAtomRel, copyObject(a->args));
                rExpr = createSkolemExpr(GP_NODE_POSREL, atomRel, copyObject(a->args));
                moveRule = createMoveRule(lExpr, rExpr, linkedHeadName, r->head->args);
                moveRules = appendToTailOfList(moveRules, moveRule);
            }

            j++;
        }

        DEBUG_LOG("created new move rule for head -> rule");
    }

    FOREACH(DLRule,r,newRules)
        setIDBBody(r);

    FOREACH(DLRule,r,moveRules)
        setIDBBody(r);

    FOREACH(DLRule,r,helpRules)
        setIDBBody(r);

    DEBUG_LOG("------------- STEP 3 ---------------\ncreated unlinked rules:\n%s\nand linked rules:\n%s\nand help rules:\n%s\nand EDB rules:\n%s\nand move rules:\n%s",
            datalogToOverviewString((Node *) unLinkedRules),
            datalogToOverviewString((Node *) newRules),
            datalogToOverviewString((Node *) helpRules),
            datalogToOverviewString((Node *) edbRules),
            datalogToOverviewString((Node *) moveRules));

    solvedProgram->ans = "move";
    solvedProgram->rules = CONCAT_LISTS(moveRules, newRules, edbRules, helpRules, unLinkedRules);

    INFO_LOG("gp program is:\n%s", datalogToOverviewString((Node *) solvedProgram));

    return solvedProgram;
}

static void
setIDBBody (DLRule *r)
{
    FOREACH(DLAtom,a,r->body)
    {
        if (isA(a, DLAtom))
            DL_SET_BOOL_PROP(a, DL_IS_IDB_REL);
    }
}

static List *
removeVars (List *vars, List *remVars)
{
    Set *rVars = NODESET();
    List *result = NIL;

    FOREACH(Node,r,remVars)
        addToSet(rVars,r);

    FOREACH(Node,v,vars)
        if(!hasSetElem(rVars,v))
            result = appendToTailOfList(result, v);

    return result;
}

static List *
makeUniqueVarList (List *vars)
{
    Set *varSet = NODESET();
    List *result = NIL;

    FOREACH(DLVar,v,vars)
        addToSet(varSet,v);

    FOREACH_SET(DLVar,v,varSet)
        result = appendToTailOfList(result, v);

    return result;
}

static DLRule *
createMoveRule(Node *lExpr, Node *rExpr, char *bodyAtomName, List *bodyArgs)
{
    DLRule *moveRule = makeNode(DLRule);
    DLAtom *moveHead = makeNode(DLAtom);
    DLAtom *moveBody = makeNode(DLAtom);

    moveHead->rel = strdup("move");
    moveHead->args = LIST_MAKE(lExpr, rExpr);

    moveBody->rel = strdup(bodyAtomName);
    moveBody->args = copyObject(bodyArgs);

    moveRule->head = moveHead;
    moveRule->body = singleton(moveBody);

    DEBUG_LOG("new move rule:\n%s", datalogToOverviewString((Node *) moveRule));

    return moveRule;
}


static Node *
createSkolemExpr (GPNodeType type, char *id, List *args)
{
    Node *result;
    int i = 0;
    List *concatArgs = NIL;

    // start with type id, id, '('
    switch(type)
    {
        case GP_NODE_RULE:
            concatArgs = appendToTailOfList(concatArgs,
                        createConstString("RULE_"));
            break;
        case GP_NODE_GOAL:
            concatArgs = appendToTailOfList(concatArgs,
                        createConstString("GOAL_"));
            break;
        case GP_NODE_POSREL:
            concatArgs = appendToTailOfList(concatArgs,
                        createConstString("REL_"));
            break;
        case GP_NODE_NEGREL:
            concatArgs = appendToTailOfList(concatArgs,
                        createConstString("notREL_"));
            break;
        case GP_NODE_EDB:
            concatArgs = appendToTailOfList(concatArgs,
                        createConstString("EDB_"));
            break;
    }
    concatArgs = appendToTailOfList(concatArgs,
                createConstString(id));

    concatArgs = appendToTailOfList(concatArgs,
                createConstString("("));

    // add args
    FOREACH(Node,arg,args)
    {
        if (i++ != 0)
            concatArgs = appendToTailOfList(concatArgs,
                            createConstString(","));
        concatArgs = appendToTailOfList(concatArgs, copyObject(arg));
    }

    // end with ')'
    concatArgs = appendToTailOfList(concatArgs,
            createConstString(")"));


    // create expression to concatenate parts of the skolem string
    result = popHeadOfListP(concatArgs);
    while(!LIST_EMPTY(concatArgs))
        result = (Node *) createOpExpr("||", LIST_MAKE(result,
                popHeadOfListP(concatArgs)));

    DEBUG_LOG("result expression is: %s", exprToSQL(result));

    return (Node *) result;
}

static void
enumerateRules (DLProgram *p)
{
    int i = 0;

    FOREACH(DLRule,r,p->rules)
        setDLProp((DLNode *) r, DL_RULE_ID, (Node *) createConstInt(i++));
}

/*
 * Given a target atom for provenance computation Why(R(X)) unify rules in the
 *  program according to constants in X. This may result in multiple copies of
 *  each rule and relation node. E.g., if rules are
 *      Q(X) :- R(X,Y)
 *      Q(X) :- R(Y,X)
 *      R(X,Y) :- S(X,Y)
 *
 *  and the user question is Why(Q(1)), then we unify rules with X <- 1
 *  recursively which rules in two instances of the rule for R:
 *      Q(1) :- R(1,Y)
 *      Q(1) :- R(Y,1)
 *      R(1,Y) :- S(1,Y)
 *      R(Y,1) :- S(Y,1)
 *  We store which original rule a unified rule corresponds using properties
 */

static DLProgram *
unifyProgram (DLProgram *p, DLAtom *question)
{
    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    HashMap *predToUnRules = NEW_MAP(DLAtom, List);
    HashMap *newPredToRules = NEW_MAP(Constant,List);
//    List *predRules;
    List *newRules = NIL;
    DLProgram *newP = makeNode(DLProgram);

//    predRules = (List *) MAP_GET_STRING(predToRules, question->rel);
    DEBUG_LOG("using %s to unify program:\n%s",
            datalogToOverviewString((Node *) question),
            datalogToOverviewString((Node *) p));

    // unify rule bodies starting with constants provided by the user query
    // e.g., Why(Q(1))
    unifyOneWithRuleHeads(predToRules, predToUnRules, question);

    DEBUG_LOG("predToUnRules:\n%s", beatify(nodeToString(predToUnRules)));

    // build new pred name to rules map
    FOREACH_HASH_ENTRY(kv,predToUnRules)
    {
        DLAtom *head = (DLAtom *) kv->key;
        List *rules = (List *) kv->value;
        char *predName = head->rel;

        CONCAT_MAP_LIST(newPredToRules,(Node *) createConstString(predName),
                copyObject(rules));
        if (newRules)
            newRules = CONCAT_LISTS(newRules, copyObject(rules));
        else
            newRules = copyObject(rules);
    }

    DEBUG_LOG("new rules are:\n%s",
            datalogToOverviewString((Node *) newRules));

    // build new program based on generated rules
    newP->ans = strdup(p->ans);
    newP->facts = p->facts;
    newP->rules = newRules;
    newP->n.properties = copyObject(p->n.properties);

    setDLProp((DLNode *) newP, DL_MAP_RELNAME_TO_RULES, (Node *) newPredToRules);
    setDLProp((DLNode *) newP, DL_MAP_UN_PREDS_TO_RULES, (Node *) predToUnRules);

    DEBUG_LOG("unified program is:\n%s", beatify(nodeToString(newP)));
    INFO_LOG("unified program is:\n%s", datalogToOverviewString((Node *) newP));

    return newP;
}

static void
unifyOneWithRuleHeads(HashMap *pToR, HashMap *rToUn, DLAtom *curAtom)
{
    List *vals; //= curAtom->args;
//    char *unRel = curAtom->rel;
    List *unRules = NIL;
    List *origRules;
    DLAtom *lookupAtom;

    NORM_COPY(lookupAtom, curAtom);
    vals = lookupAtom->args;
    setDLProp((DLNode *) curAtom, DL_NORM_ATOM, copyObject(lookupAtom));

    // get originalRules and if they exist unified rules
    origRules = (List *) MAP_GET_STRING(pToR, curAtom->rel);
    unRules = (List *) getMap(rToUn, (Node *) lookupAtom);

    DEBUG_LOG("unify head %s with rules:\n%s",
            datalogToOverviewString((Node *) curAtom),
            datalogToOverviewString((Node *) origRules));

    // if rules for unified head already exist, then return
    if (unRules)
        return;

    // otherwise process each rule to unify it, then process body atoms
    // recursively
    FOREACH(DLRule,r,origRules)
    {
        DLRule *un;
        int ruleId;

        ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        un = unifyRule(r,vals);
        unRules = appendToTailOfList(unRules, un);
        setDLProp((DLNode *) r,DL_ORIGINAL_RULE, (Node *) createConstInt(ruleId));

        DEBUG_LOG("unified rule with head %s\nRule: %s\nUnified Rules: %s",
                datalogToOverviewString((Node *) curAtom),
                datalogToOverviewString((Node *) r),
                datalogToOverviewString((Node *) un));
    }

    // store mapping of this head atom to all unified rules for it
    addToMap(rToUn, (Node *) lookupAtom, (Node *) unRules);

    FOREACH(DLRule,un,unRules)
    {
        FOREACH(DLAtom,a,un->body)
        {
            if (DL_HAS_PROP(a,DL_IS_IDB_REL))
            {
//                boolean hasConst = FALSE;
//                FOREACH(Node,arg,a->args)
//                    if (isA(arg,Constant))
//                        hasConst = TRUE;
//                if (hasConst)
                unifyOneWithRuleHeads(pToR,rToUn,a);
            }
        }
    }
}


/*
 * Determine which nodes in the game template (annotated program) will
 * be lost and which ones will be won based on the user query. Or to be
 * precise for which nodes we are interested in only lost ones and which ones we
 * are interested in only won one. This differs based on whether a won or lost node
 * is below a negation. Under a negation we are still only interested in won or
 * lost nodes, but may have to compute also the lost/won parts for joining rules.
 * This is because a failed rule instantiation will have all combinations of
 * failed goals in its provenance. However, to construct it we may need to join
 * such failed goals with successful goals. For instance,
 *
 *      r1: Q(X) :- R(X,Y), not S(X,Y).
 *
 *      WhyNot(Q(1))
 *
 *  We know that r1(1,Y) is failed and need to enumerate all ways of how this has
 *  failed. However, for a concrete failed instantiation r1(1,y) one of the following holds:
 *
 *      1) R(1,y) has failed and not S(1,y) has failed
 *      2) R(1,y) has failed and not S(1,y) was successful
 *      3) R(1,y) was successful and not S(1,Y) has failed
 *
 *  In each case only the failed goals would be in the game provenance. To find all combinations,
 *  we need to join both all successful and all failed instantiations of R(1,y) and not S(1,y).
 *  If the question would have been Why(Q(1)) then r1 is successful and we are only interested in
 *  all ways of how goals R and S succeed.
 *
 *  In this step we create adorned versions of rules with 4 types of adornments. In terms of the
 *  program each adorned version of an atom is treated as a separate relation. Thus,
 *  we may create up to 4 adorned versions of each rule. The adornments are:
 *
 *      1) positive Won (DL_WON)
 *      2) positive Lost (DL_LOST)
 *      3) negative Won (DL_UNDER_NEG_WON)
 *      4) negative Lost (DL_UNDER_NEG_LOST)
 */

static DLProgram *
solveProgram (DLProgram *p, DLAtom *question, boolean neg)
{
//    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    HashMap *unPredToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_UN_PREDS_TO_RULES);
    HashMap *solvPredToRules = NEW_MAP(Node,Node); // store adorned versions
    boolean unified = (unPredToRules != NULL);
    Set *doneAd = NODESET();
    Set *edb = (Set *) getDLProp((DLNode *) p, DL_EDB_RELS);
    List *adornedRules = NIL;
    DLAtom *lookupQ;

    NORM_COPY(lookupQ,question);

    DEBUG_LOG("normalized user atom is: %s",
            datalogToOverviewString((Node *) lookupQ));

    // rules have been unified
    if (unified)
    {
        List *todoStack = NIL;

        // initialize stack with copy of rules for user provenance question
        todoStack = copyObject((List *) getMap(unPredToRules,(Node *) lookupQ));

        DEBUG_LOG("rules mapping to user question:\n%s",
                datalogToOverviewString((Node *) todoStack));

        // user question defines whether the starting predicate is lost or won
        FOREACH(DLRule,r,todoStack)
        {
            char *state = neg ? DL_LOST : DL_WON;
            DLAtom *adornedHead;

            NORM_COPY(adornedHead, r->head);

            DEBUG_LOG("head atom %s matching normalized user question %s",
                    datalogToOverviewString((Node *) adornedHead),
                    datalogToOverviewString((Node *) lookupQ));

            setDLProp((DLNode *) r, state,
                    (Node *) createConstBool(TRUE));
            setDLProp((DLNode *) r->head, state,
                    (Node *) createConstBool(TRUE));
            setDLProp((DLNode *) adornedHead, state,
                    (Node *) createConstBool(TRUE));

            if (!hasSetElem(doneAd, adornedHead))
                addToSet(doneAd, copyObject(adornedHead));

            // store entry with adorned head as key and adorned rule as body
            CONCAT_MAP_LIST(solvPredToRules,(Node *) adornedHead,
                            singleton(r));
        }

        // recursivly set WON/LOST status for all
        while(!LIST_EMPTY(todoStack))
        {
            DLRule *r = (DLRule *) popHeadOfListP(todoStack);
            boolean ruleWon = DL_HAS_PROP(r,DL_WON)
                    || DL_HAS_PROP(r,DL_UNDER_NEG_WON);
            boolean ruleNeg = DL_HAS_PROP(r,DL_UNDER_NEG_WON)
                    || DL_HAS_PROP(r,DL_UNDER_NEG_LOST);

            DEBUG_LOG("process rule:\n%s",
                    datalogToOverviewString((Node *) r));

            // process each atom
            FOREACH(DLAtom,a,r->body)
            {
                DLAtom *adHead;
                boolean newWon = ruleWon && !a->negated;
                boolean newNeg = ruleNeg || !ruleWon;
                char *newProp = newNeg ?
                        (newWon ? DL_UNDER_NEG_WON : DL_UNDER_NEG_LOST)
                        : (newWon ? DL_WON : DL_LOST);

                NORM_COPY(adHead, a);
                DEBUG_LOG("process atom:\n%s",
                        datalogToOverviewString((Node *) a));

                if (!hasSetElem(edb,a->rel))
                {
                    DL_SET_BOOL_PROP(adHead, newProp);

                    // do not process rules for adorned head twice
                    if (!hasSetElem(doneAd, adHead))
                    {
                        DLAtom *lookup;
                        NORM_COPY(lookup,a);
                        List *newRules = copyObject(getMap(unPredToRules,
                                (Node *) lookup));

                        FOREACH(DLRule,r,newRules)
                        {
                            DLRule *cpy = copyObject(r);
                            DL_SET_BOOL_PROP(cpy, newProp);
                            DL_SET_BOOL_PROP(cpy->head, newProp);

                            // store entry with adorned head as key and list of adorned rules as body
                            CONCAT_MAP_LIST(solvPredToRules,(Node *) copyObject(adHead),
                                            singleton(cpy));

                            // put rule onto todo stack
                            todoStack = appendToTailOfList(todoStack,cpy);
                        }

                        addToSet(doneAd, copyObject(adHead));
                    }
                }

                DL_SET_BOOL_PROP(a, newProp);
            }

            adornedRules = appendToTailOfList(adornedRules, r);
        }
    }
    // non-unified rules
    else
    {
        FATAL_LOG("not unified is not supported yet.");
    }

    // create adorned program
    p->rules = adornedRules;
    setDLProp((DLNode *) p, DL_MAP_ADORNED_PREDS_TO_RULES, (Node *) solvPredToRules);

    DEBUG_LOG("adorned (solved) program is:\n%s", beatify(nodeToString(p)));
    INFO_LOG("adorned (solved) program is:\n%s", datalogToOverviewString((Node *) p));

    return p;
}
