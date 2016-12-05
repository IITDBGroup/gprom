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

#include "metadata_lookup/metadata_lookup.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/datalog/datalog_model.h"
#include "provenance_rewriter/game_provenance/gp_bottom_up_program.h"
#include "utility/string_utils.h"

//#define ADD_ARGS "L"
#define NON_LINKED_POSTFIX "_nonlinked"
#define NON_LINKED_POSTFIX_CHKPOS "_nonlinked_chkpos"

#define CONCAT_MAP_LIST(map,key,newList) \
    do { \
        List *old = (List *) getMap(map,key); \
        old = CONCAT_LISTS(newList, old); \
        addToMap(map,key,(Node *) old); \
    } while (0)

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
boolean searchVars (List *vars, List *searVars);
static List *makeUniqueVarList (List *vars);
static void setIDBBody (DLRule *r);

static DLProgram *rewriteSolvedProgram (DLProgram *solvedProgram);
static List*createTupleRuleTupleGraphMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules);
static List*createTupleOnlyGraphMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules);
static List*createGPMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules);
static List*createGPReducedMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules);
static DLRule *createMoveRule(Node *lExpr, Node *rExpr, char *bodyAtomName, List *bodyArgs);
static Node *createSkolemExpr (GPNodeType type, char *id, List *args);

static DLProgram *unifyProgram (DLProgram *p, DLAtom *question);
static void unifyOneWithRuleHeads(HashMap *pToR, HashMap *rToUn, DLAtom *curAtom);
static DLProgram *solveProgram (DLProgram *p, DLAtom *question, boolean neg);

//char *idbHeadPred = NULL;
List *programRules = NIL;
List *domainRules = NIL;


DLProgram *
createBottomUpGPprogram (DLProgram *p)
{
    DEBUG_NODE_BEATIFY_LOG("create GP bottom up program for:",p);
    INFO_DL_LOG("create GP bottom up program for:",p);
    // why provenance
    if(DL_HAS_PROP(p,DL_PROV_WHY))
    {
        DLAtom *why = (DLAtom *) getDLProp((DLNode *) p,DL_PROV_WHY);
        DLProgram *program = createWhyGPprogram(p, why);

        programRules = NIL;
        domainRules = NIL;

        return program;

    }
    // why not
    else if(DL_HAS_PROP(p,DL_PROV_WHYNOT))
    {
        DLAtom *whyN = (DLAtom *) getDLProp((DLNode *) p,DL_PROV_WHYNOT);
        DLProgram *program = createWhyNotGPprogram(p, whyN);

        programRules = NIL;
        domainRules = NIL;

        return program;
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

	if (LIST_LENGTH(p->doms) != 0)
	{
		List *domHead = NIL;

		FOREACH(DLDomain,d,p->doms)
			domHead = appendToTailOfList(domHead,d->name);


		FOREACH(DLRule,r,p->rules)
		{
			if (searchListString(domHead,r->head->rel))
				domainRules = appendToTailOfList(domainRules, (List *) r);
			else
				programRules = appendToTailOfList(programRules, (List *) r);
		}
		p->rules = programRules;

		INFO_DL_LOG("create new GP bottom up program for:", p);
		DEBUG_LOG("Associated Domain:\n%s", datalogToOverviewString((Node *) domainRules));
	}

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

/* Implementing  WHYNOT*/
static DLProgram *
createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot)
{
	DLProgram *solvedProgram;

	if (LIST_LENGTH(p->doms) != 0)
	{
		List *domHead = NIL;
		FOREACH(DLDomain,d,p->doms)
		domHead = appendToTailOfList(domHead,d->name);

		FOREACH(DLRule,r,p->rules)
		{
			if (searchListString(domHead,r->head->rel))
				domainRules = appendToTailOfList(domainRules, (List *) r);
			else
				programRules = appendToTailOfList(programRules, (List *) r);
		}
		p->rules = programRules;

		INFO_DL_LOG("create new GP bottom up program for:", p);
		DEBUG_LOG("Associated Domain:\n%s", datalogToOverviewString((Node *) domainRules));
	}

	enumerateRules (p);
	solvedProgram = copyObject(p);
	solvedProgram = unifyProgram(solvedProgram, whyNot);
	solvedProgram = solveProgram(solvedProgram, whyNot, TRUE);

	p->n.properties = NULL;
	setDLProp((DLNode *) solvedProgram, DL_PROV_PROG, (Node *) p);

	solvedProgram = rewriteSolvedProgram(solvedProgram);
	DL_DEL_PROP(solvedProgram, DL_PROV_WHYNOT);

	INFO_LOG("program for computing WhyNot-prov: %s",
		 datalogToOverviewString((Node *) solvedProgram));

	return solvedProgram;
}

static DLProgram *
createFullGPprogram (DLProgram *p)
{
    return p;
}

static List*createTupleRuleTupleReducedGraphMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules)
{
    List *moveRules = NIL;
    int checkPos = 0;
    char *bName = NULL;
    List *newBoolArgs = NIL;
    List *collectRuleId = NIL;
    DLVar* createBoolArgs;
    int ruleIdPos = 0;

    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,
                DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

        // Collecting all the original variables for later use
        int argPos = -1;
        List *ruleArgs = NIL;
        int rNumGoals = LIST_LENGTH(r->body);

        FOREACH(DLAtom,a,r->body)
        {
            argPos++;
            if (!ruleWon && (argPos + 1) == rNumGoals
                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
                            == getMatched && rNumGoals > 1)
            {
                ruleArgs = copyObject(ruleArgs);
            }
            else
            {
                FOREACH(DLNode,arg,a->args)
                {
                    if (!searchListNode(ruleArgs, (Node *) arg))
                        ruleArgs = appendToTailOfList(ruleArgs,
                                copyObject(arg));
                }
            }
        }
        DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

        char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *ruleRel = CONCAT_STRINGS(
                CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
                !ruleWon ? "_WON" : "_LOST");
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
                strlen(NON_LINKED_POSTFIX));
        Node *lExpr;
        Node *rExpr;
        DLRule *moveRule;
        int goalPos = -1;
        boolean goalChk;
        List *newRuleHeadArgs = NIL;
        List *boolArgs = removeVars(r->head->args, ruleArgs);

        // head atom -> rule hyper edge
        lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                               copyObject(origAtom->args));
        rExpr = createSkolemExpr(GP_NODE_RULEHYPER, ruleRel,
                copyObject(
                        removeVars(r->head->args,
                                removeVars(r->head->args, ruleArgs))));
        moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                r->head->args);
        moveRules = appendToTailOfList(moveRules, moveRule);

        // rule -> goal atoms
        lExpr = copyObject(rExpr);

        // create a list for collecting rule id
        int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
                STRING_VALUE(createConstBool(ruleWon)));
        int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
        collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

        // reset the position to check if the rule id is changed
        if (LIST_LENGTH(collectRuleId) > 1)
            if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
                    != getNthOfListInt(collectRuleId, ruleIdPos))
                checkPos = 0;

        if (!ruleWon)
        {
            // generate boolean args with variables to reduce redundant move rules
            newBoolArgs = NIL;
            for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
                    checkLoop++)
            {
                bName = CONCAT_STRINGS("BL", itoa(checkLoop));
                createBoolArgs = createDLVar(bName, DT_BOOL);
                newBoolArgs = appendToTailOfList(newBoolArgs,
                        copyObject(createBoolArgs));
            }

            if (checkPos < LIST_LENGTH(boolArgs))
                goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
        }

        if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
                || ruleWon)
        {
            if (!ruleWon)
            {
                // update the boolean arguments
                List *replaceBoolArgs = copyObject(newBoolArgs);
                replaceBoolArgs = replaceNode(replaceBoolArgs,
                        getNthOfListP(replaceBoolArgs, checkPos),
                        getNthOfListP(boolArgs, checkPos));

                newRuleHeadArgs = removeVars(r->head->args,
                        removeVars(r->head->args, ruleArgs));
                for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
                    newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
                            getNthOfListP(replaceBoolArgs, k));
            }

            FOREACH(DLAtom,a,r->body)
            {
                goalPos++;
//                int numHeadArgs = LIST_LENGTH(r->head->args);
                boolean goalWon = FALSE;
                lExpr = copyObject(lExpr);

                // Not include the additional atom for filtering out
                if (!ruleWon && (goalPos + 1) == rNumGoals
                        && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
                {
                    goalWon = TRUE;
                }
                else
                {
                    if ((!ruleWon && checkPos == j) || ruleWon)
                    {
                        ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
                        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
                                DL_ORIG_ATOM);

                        char *atomRel = CONCAT_STRINGS("r", strdup(origAtom->rel),
                                ruleWon ? "_WON" : "_LOST");
                        char *negAtomRel = CONCAT_STRINGS("r", strdup(origAtom->rel),
                                !ruleWon ? "_WON" : "_LOST");

                        if (!ruleWon)
                        {
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//                            if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//                                    == getMatched)
//                            {
//                                if (goalPos != rNumGoals - 1)
//                                    goalWon = BOOL_VALUE(
//                                            getNthOfListP(r->head->args,
//                                                    numHeadArgs - (rNumGoals - 1) + goalPos));
//                            }
//                            else
//                                goalWon = BOOL_VALUE(
//                                        getNthOfListP(r->head->args,
//                                                numHeadArgs - rNumGoals + goalPos));
                        }

                        // head atom -> negR
                        if (a->negated)
                        {
                            if (!goalWon)
                            {
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        negAtomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                        // head atom -> posR
                        else
                        {
                            if (!goalWon || ruleWon)
                            {
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        atomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                    }
                }
                DEBUG_LOG("created new move rule for tuple -> tuple");
                j++;
            }
            checkPos++;
        }
        ruleIdPos++;
    }


    return moveRules;
}

static List*createHeadRuleEdbGraphMoveRules(int getMatched, List* negedbRules, List* edbRules, List* unLinkedRules)
{
	List *moveRules = NIL;
	int checkPos = 0;
	char *bName = NULL;
	List *newBoolArgs = NIL;
	List *collectRuleId = NIL;
	DLVar* createBoolArgs;
	int ruleIdPos = 0;

	FOREACH(DLRule,r,unLinkedRules)
	{
		boolean ruleWon = DL_HAS_PROP(r->head,
				DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
		ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
		DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

		// Collecting all the original variables for later use
		int argPos = -1;
		List *ruleArgs = NIL;
		int rNumGoals = LIST_LENGTH(r->body);

		FOREACH(DLAtom,a,r->body)
		{
			argPos++;
			if (!ruleWon && (argPos + 1) == rNumGoals
					&& INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
			{
				ruleArgs = copyObject(ruleArgs);
			}
			else
			{
				FOREACH(DLNode,arg,a->args)
				{
					if (!searchListNode(ruleArgs, (Node *) arg))
						ruleArgs = appendToTailOfList(ruleArgs,
								copyObject(arg));
				}
			}
		}
		DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

		char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
				ruleWon ? "_WON" : "_LOST");
		char *ruleRel = CONCAT_STRINGS(
				CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
				ruleWon ? "_WON" : "_LOST");
//		i = INT_VALUE(DL_GET_PROP(r,DL_RULE_ID));
		int j = 0;
		char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
				strlen(NON_LINKED_POSTFIX));

		// head -> rule_i
		if (!ruleWon)
		{
			Node *lExpr;
			Node *rExpr;
			DLRule *moveRule;
			lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
					copyObject(origAtom->args));
			rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel,
					copyObject(
							removeVars(r->head->args,
									removeVars(r->head->args, ruleArgs))));
			moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
					r->head->args);
			moveRules = appendToTailOfList(moveRules, moveRule);
		}
		else
		{
			Node *lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
					copyObject(origAtom->args));
			Node *rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel,
					copyObject(r->head->args));
			DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
					r->head->args);
			moveRules = appendToTailOfList(moveRules, moveRule);
		}

		// rule_i -> goal_i_j -> posR/negR -> posR
		int goalPos = -1;

		// remove over generated move rules
		boolean goalChk;
		List *newRuleHeadArgs = NIL;
		List *boolArgs = removeVars(r->head->args, ruleArgs);

		// create a list for collecting rule id
		int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
		char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
				STRING_VALUE(createConstBool(ruleWon)));
		int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
		collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

		// reset the position to check if the rule id is changed
		if (LIST_LENGTH(collectRuleId) > 1)
			if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
					!= getNthOfListInt(collectRuleId, ruleIdPos))
				checkPos = 0;

		if (!ruleWon)
		{
			// generate boolean args with variables to reduce redundant move rules
			newBoolArgs = NIL;
			for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
					checkLoop++)
			{
				bName = CONCAT_STRINGS("BL", itoa(checkLoop));
				createBoolArgs = createDLVar(bName, DT_BOOL);
				newBoolArgs = appendToTailOfList(newBoolArgs,
						copyObject(createBoolArgs));
			}

			if (checkPos < LIST_LENGTH(boolArgs))
				goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
		}

		if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
				|| ruleWon)
		{
			if (!ruleWon)
			{
				// update the boolean arguments
				List *replaceBoolArgs = copyObject(newBoolArgs);
				replaceBoolArgs = replaceNode(replaceBoolArgs,
						getNthOfListP(replaceBoolArgs, checkPos),
						getNthOfListP(boolArgs, checkPos));

				newRuleHeadArgs = removeVars(r->head->args,
						removeVars(r->head->args, ruleArgs));
				for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
					newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
							getNthOfListP(replaceBoolArgs, k));
			}

			FOREACH(DLAtom,a,r->body)
			{
				goalPos++;
				//              int unruleNumGoals = LIST_LENGTH(r->body);
//				int numHeadArgs = LIST_LENGTH(r->head->args);
				boolean goalWon = FALSE;
				boolean relWon = FALSE;

				// Not include the additional atom for filtering out
				if (!ruleWon && (goalPos + 1) == rNumGoals
						&& INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
				{
					goalWon = TRUE;
				}
				else
				{
					if ((!ruleWon && checkPos == j) || ruleWon)
					{
						ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
						DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
								DL_ORIG_ATOM);

//						char *goalRel = CONCAT_STRINGS(itoa(i), "_", itoa(j),
//								ruleWon ? "_WON" : "_LOST");

						// is goal won?
						if (!ruleWon)
						{
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//							if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//									== getMatched)
//							{
//								if (goalPos != rNumGoals - 1)
//									goalWon = BOOL_VALUE(
//											getNthOfListP(r->head->args,
//													numHeadArgs - (rNumGoals - 1) + goalPos));
//							}
//							else
//								goalWon = BOOL_VALUE(
//										getNthOfListP(r->head->args,
//												numHeadArgs - rNumGoals + goalPos));
						}
						else
							goalWon = TRUE;

						DEBUG_LOG("goal is %s and goal is negated %s", goalWon ? "TRUE" : "FALSE", a->negated ? "TRUE" : "FALSE");

						// is tuple won?
						if ((goalWon && (!a->negated)) || ((!goalWon) && a->negated))
						{
							relWon = TRUE;
						}
						else
						{
							relWon = FALSE;
						}

						char *atomRel = CONCAT_STRINGS("r",strdup(origAtom->rel),
								relWon ? "_WON" : "_LOST");

						// -> posR
						if (a->negated)
						{
							if (!goalWon || ruleWon)
							{
								Node *lExpr = createSkolemExpr(GP_NODE_RULE,
										ruleRel,
										copyObject(
												removeVars(r->head->args,
														removeVars(
																r->head->args,
																ruleArgs))));
//								Node *rExpr = createSkolemExpr(GP_NODE_GOAL,
//										goalRel, copyObject(a->args));
//								DLRule *moveRule;
//								if (ruleWon)
//									moveRule = createMoveRule(lExpr, rExpr,
//											linkedHeadName, r->head->args);
//								else
//									moveRule = createMoveRule(lExpr, rExpr,
//											linkedHeadName,
//											copyObject(newRuleHeadArgs));
//
//								moveRules = appendToTailOfList(moveRules,
//										moveRule);
//
//								lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel,
//										copyObject(a->args));
								Node *rExpr = createSkolemExpr(GP_NODE_EDB,
										atomRel, copyObject(a->args));
								DLRule *moveRule;
								if (ruleWon)
									moveRule = createMoveRule(lExpr, rExpr,
											linkedHeadName, r->head->args);
								else
									moveRule = createMoveRule(lExpr, rExpr,
											linkedHeadName,
											copyObject(newRuleHeadArgs));

								moveRules = appendToTailOfList(moveRules,
										moveRule);
							}
						}
						// -> posR
						else
						{
							if (!goalWon || ruleWon)
							{
								Node *lExpr;
								if (!ruleWon)
									lExpr =
											createSkolemExpr(GP_NODE_RULE,
													ruleRel,
													copyObject(
															removeVars(
																	r->head->args,
																	removeVars(
																			r->head->args,
																			ruleArgs))));
								else
									lExpr = createSkolemExpr(GP_NODE_RULE,
											ruleRel, copyObject(r->head->args));

								char *Rel = CONCAT_STRINGS("r",strdup(origAtom->rel),
												ruleWon ? "_WON" : "_LOST");
								Node *rExpr = createSkolemExpr(GP_NODE_EDB,
										Rel, copyObject(a->args));

								DLRule *moveRule;
								if (ruleWon)
									moveRule = createMoveRule(lExpr, rExpr,
											linkedHeadName, r->head->args);
								else
									moveRule = createMoveRule(lExpr, rExpr,
											linkedHeadName,
											copyObject(newRuleHeadArgs));

								moveRules = appendToTailOfList(moveRules,
										moveRule);

//                                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel,
//                                        copyObject(a->args));
//
//                                rExpr = createSkolemExpr(GP_NODE_POSREL,
//                                        atomRel, copyObject(a->args));
//                                if (ruleWon)
//                                    moveRule = createMoveRule(lExpr, rExpr,
//                                            linkedHeadName, r->head->args);
//                                else
//                                    moveRule = createMoveRule(lExpr, rExpr,
//                                            linkedHeadName,
//                                            copyObject(newRuleHeadArgs));
//
//                                moveRules = appendToTailOfList(moveRules,
//                                        moveRule);
							}
						}
					}
				}
				DEBUG_LOG("created new move rule for head -> rule");
				j++;
			}
			checkPos++;
		}
		ruleIdPos++;
	}

	return moveRules;
}

static List*createTupleRuleGoalTupleGraphMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules)
{
    List *moveRules = NIL;
    int checkPos = 0;
    char *bName = NULL;
    List *newBoolArgs = NIL;
    List *collectRuleId = NIL;
    DLVar* createBoolArgs;
    int ruleIdPos = 0;

    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,
                DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

        // Collecting all the original variables for later use
        int argPos = -1;
        List *ruleArgs = NIL;
        int rNumGoals = LIST_LENGTH(r->body);

        FOREACH(DLAtom,a,r->body)
        {
            argPos++;
            if (!ruleWon && (argPos + 1) == rNumGoals
                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
            {
                ruleArgs = copyObject(ruleArgs);
            }
            else
            {
                FOREACH(DLNode,arg,a->args)
                {
                    if (!searchListNode(ruleArgs, (Node *) arg))
                        ruleArgs = appendToTailOfList(ruleArgs,
                                copyObject(arg));
                }
            }
        }
        DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

        char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *ruleRel = CONCAT_STRINGS(
                CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
                ruleWon ? "_WON" : "_LOST");
        int i = INT_VALUE(DL_GET_PROP(r,DL_RULE_ID));
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
                strlen(NON_LINKED_POSTFIX));

        // head -> rule_i
        if (!ruleWon)
        {
            Node *lExpr;
            Node *rExpr;
            DLRule *moveRule;
            lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                    copyObject(origAtom->args));
            rExpr = createSkolemExpr(GP_NODE_RULEHYPER, ruleRel,
                    copyObject(
                            removeVars(r->head->args,
                                    removeVars(r->head->args, ruleArgs))));
            moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                    r->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);
        }
        else
        {
            Node *lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                    copyObject(origAtom->args));
            Node *rExpr = createSkolemExpr(GP_NODE_RULEHYPER, ruleRel,
                    copyObject(r->head->args));
            DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                    r->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);
        }

        // rule_i -> goal_i_j -> posR/negR -> posR
        int goalPos = -1;

        // remove over generated move rules
        boolean goalChk;
        List *newRuleHeadArgs = NIL;
        List *boolArgs = removeVars(r->head->args, ruleArgs);

        // create a list for collecting rule id
        int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
                STRING_VALUE(createConstBool(ruleWon)));
        int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
        collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

        // reset the position to check if the rule id is changed
        if (LIST_LENGTH(collectRuleId) > 1)
            if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
                    != getNthOfListInt(collectRuleId, ruleIdPos))
                checkPos = 0;

        if (!ruleWon)
        {
            // generate boolean args with variables to reduce redundant move rules
            newBoolArgs = NIL;
            for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
                    checkLoop++)
            {
                bName = CONCAT_STRINGS("BL", itoa(checkLoop));
                createBoolArgs = createDLVar(bName, DT_BOOL);
                newBoolArgs = appendToTailOfList(newBoolArgs,
                        copyObject(createBoolArgs));
            }

            if (checkPos < LIST_LENGTH(boolArgs))
                goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
        }

        if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
                || ruleWon)
        {
            if (!ruleWon)
            {
                // update the boolean arguments
                List *replaceBoolArgs = copyObject(newBoolArgs);
                replaceBoolArgs = replaceNode(replaceBoolArgs,
                        getNthOfListP(replaceBoolArgs, checkPos),
                        getNthOfListP(boolArgs, checkPos));

                newRuleHeadArgs = removeVars(r->head->args,
                        removeVars(r->head->args, ruleArgs));
                for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
                    newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
                            getNthOfListP(replaceBoolArgs, k));
            }

            FOREACH(DLAtom,a,r->body)
            {
                goalPos++;
                //              int unruleNumGoals = LIST_LENGTH(r->body);
//                int numHeadArgs = LIST_LENGTH(r->head->args);
                boolean goalWon = FALSE;
                boolean relWon = FALSE;

                // Not include the additional atom for filtering out
                if (!ruleWon && (goalPos + 1) == rNumGoals
                        && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
                {
                    goalWon = TRUE;
                }
                else
                {
                    if ((!ruleWon && checkPos == j) || ruleWon)
                    {
                        ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
                        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
                                DL_ORIG_ATOM);

                        char *goalRel = CONCAT_STRINGS(itoa(i), "_", itoa(j),
                                ruleWon ? "_WON" : "_LOST");

                        // is goal won?
                        if (!ruleWon)
                        {
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//                            if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//                                    == getMatched)
//                            {
//                                if (goalPos != rNumGoals - 1)
//                                    goalWon = BOOL_VALUE(
//                                            getNthOfListP(r->head->args,
//                                                    numHeadArgs - (rNumGoals - 1) + goalPos));
//                            }
//                            else
//                                goalWon = BOOL_VALUE(
//                                        getNthOfListP(r->head->args,
//                                                numHeadArgs - rNumGoals + goalPos));
                        }
                        else
                            goalWon = TRUE;

                        DEBUG_LOG("goal is %s and goal is negated %s", goalWon ? "TRUE" : "FALSE", a->negated ? "TRUE" : "FALSE");

                        // is tuple won?
                        if ((goalWon && (!a->negated)) || ((!goalWon) && a->negated))
                        {
                            relWon = TRUE;
                        }
                        else
                        {
                            relWon = FALSE;
                        }

                        char *atomRel = CONCAT_STRINGS("r",strdup(origAtom->rel),
                                relWon ? "_WON" : "_LOST");

                        // -> posR
                        if (a->negated)
                        {
                            if (!goalWon || ruleWon)
                            {
                                Node *lExpr = createSkolemExpr(GP_NODE_RULEHYPER,
                                        ruleRel,
                                        copyObject(
                                                removeVars(r->head->args,
                                                        removeVars(
                                                                r->head->args,
                                                                ruleArgs))));
                                Node *rExpr = createSkolemExpr(GP_NODE_GOALHYPER,
                                        goalRel, copyObject(a->args));
                                DLRule *moveRule;
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_GOALHYPER, goalRel,
                                        copyObject(a->args));
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        atomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                        // -> posR
                        else
                        {
                            if (!goalWon || ruleWon)
                            {
                                Node *lExpr;
                                if (!ruleWon)
                                    lExpr =
                                            createSkolemExpr(GP_NODE_RULEHYPER,
                                                    ruleRel,
                                                    copyObject(
                                                            removeVars(
                                                                    r->head->args,
                                                                    removeVars(
                                                                            r->head->args,
                                                                            ruleArgs))));
                                else
                                    lExpr = createSkolemExpr(GP_NODE_RULEHYPER,
                                            ruleRel, copyObject(r->head->args));

                                Node *rExpr = createSkolemExpr(GP_NODE_GOALHYPER,
                                        goalRel, copyObject(a->args));

                                DLRule *moveRule;
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_GOALHYPER, goalRel,
                                        copyObject(a->args));

                                char *Rel = CONCAT_STRINGS("r",strdup(origAtom->rel),
                                				ruleWon ? "_WON" : "_LOST");
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                		Rel, copyObject(a->args));

                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                    }
                }
                DEBUG_LOG("created new move rule for head -> rule");
                j++;
            }
            checkPos++;
        }
        ruleIdPos++;
    }

    return moveRules;
}

static List*createTupleRuleTupleGraphMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules)
{
    List *moveRules = NIL;
    int checkPos = 0;
    char *bName = NULL;
    List *newBoolArgs = NIL;
    List *collectRuleId = NIL;
    DLVar* createBoolArgs;
    int ruleIdPos = 0;

    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,
                DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

        // Collecting all the original variables for later use
        int argPos = -1;
        List *ruleArgs = NIL;
        int rNumGoals = LIST_LENGTH(r->body);

        FOREACH(DLAtom,a,r->body)
        {
            argPos++;
            if (!ruleWon && (argPos + 1) == rNumGoals
                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
                            == getMatched && rNumGoals > 1)
            {
                ruleArgs = copyObject(ruleArgs);
            }
            else
            {
                FOREACH(DLNode,arg,a->args)
                {
                    if (!searchListNode(ruleArgs, (Node *) arg))
                        ruleArgs = appendToTailOfList(ruleArgs,
                                copyObject(arg));
                }
            }
        }
        DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

        char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *ruleRel = CONCAT_STRINGS(
                CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
                !ruleWon ? "_WON" : "_LOST");
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
                strlen(NON_LINKED_POSTFIX));
        Node *lExpr;
        Node *rExpr;
        DLRule *moveRule;
        int goalPos = -1;
        boolean goalChk;
        List *newRuleHeadArgs = NIL;
        List *boolArgs = removeVars(r->head->args, ruleArgs);

        // head atom -> rule hyper edge
        lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                               copyObject(origAtom->args));
        rExpr = createSkolemExpr(GP_NODE_RULEHYPER, ruleRel,
                copyObject(
                        removeVars(r->head->args,
                                removeVars(r->head->args, ruleArgs))));
        moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                r->head->args);
        moveRules = appendToTailOfList(moveRules, moveRule);

        // rule -> goal atoms
        lExpr = copyObject(rExpr);

        // create a list for collecting rule id
        int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
                STRING_VALUE(createConstBool(ruleWon)));
        int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
        collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

        // reset the position to check if the rule id is changed
        if (LIST_LENGTH(collectRuleId) > 1)
            if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
                    != getNthOfListInt(collectRuleId, ruleIdPos))
                checkPos = 0;

        if (!ruleWon)
        {
            // generate boolean args with variables to reduce redundant move rules
            newBoolArgs = NIL;
            for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
                    checkLoop++)
            {
                bName = CONCAT_STRINGS("BL", itoa(checkLoop));
                createBoolArgs = createDLVar(bName, DT_BOOL);
                newBoolArgs = appendToTailOfList(newBoolArgs,
                        copyObject(createBoolArgs));
            }

            if (checkPos < LIST_LENGTH(boolArgs))
                goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
        }

        if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
                || ruleWon)
        {
            if (!ruleWon)
            {
                // update the boolean arguments
                List *replaceBoolArgs = copyObject(newBoolArgs);
                replaceBoolArgs = replaceNode(replaceBoolArgs,
                        getNthOfListP(replaceBoolArgs, checkPos),
                        getNthOfListP(boolArgs, checkPos));

                newRuleHeadArgs = removeVars(r->head->args,
                        removeVars(r->head->args, ruleArgs));
                for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
                    newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
                            getNthOfListP(replaceBoolArgs, k));
            }

            FOREACH(DLAtom,a,r->body)
            {
                goalPos++;
//                int numHeadArgs = LIST_LENGTH(r->head->args);
                boolean goalWon = FALSE;
                lExpr = copyObject(lExpr);

                // Not include the additional atom for filtering out
                if (!ruleWon && (goalPos + 1) == rNumGoals
                        && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
                {
                    goalWon = TRUE;
                }
                else
                {
                    if ((!ruleWon && checkPos == j) || ruleWon)
                    {
                        ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
                        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
                                DL_ORIG_ATOM);

                        char *atomRel = CONCAT_STRINGS("r", strdup(origAtom->rel),
                                ruleWon ? "_WON" : "_LOST");
                        char *negAtomRel = CONCAT_STRINGS("r", strdup(origAtom->rel),
                                !ruleWon ? "_WON" : "_LOST");

                        if (!ruleWon)
                        {
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//                            if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//                                    == getMatched)
//                            {
//                                if (goalPos != rNumGoals - 1)
//                                    goalWon = BOOL_VALUE(
//                                            getNthOfListP(r->head->args,
//                                                    numHeadArgs - (rNumGoals - 1) + goalPos));
//                            }
//                            else
//                                goalWon = BOOL_VALUE(
//                                        getNthOfListP(r->head->args,
//                                                numHeadArgs - rNumGoals + goalPos));
                        }

                        // head atom -> negR
                        if (a->negated)
                        {
                            if (!goalWon)
                            {
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        negAtomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                        // head atom -> posR
                        else
                        {
                            if (!goalWon || ruleWon)
                            {
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        atomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                    }
                }
                DEBUG_LOG("created new move rule for tuple -> tuple");
                j++;
            }
            checkPos++;
        }
        ruleIdPos++;
    }


    return moveRules;
}

static List*
createTupleOnlyGraphMoveRules(int getMatched, List* negedbRules,
        List* edbRules, List* unLinkedRules)
{
    List *moveRules = NIL;
    int checkPos = 0;
    char *bName = NULL;
    List *newBoolArgs = NIL;
    List *collectRuleId = NIL;
    DLVar* createBoolArgs;
    int ruleIdPos = 0;

    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,
                DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

        // Collecting all the original variables for later use
        int argPos = -1;
        List *ruleArgs = NIL;
        int rNumGoals = LIST_LENGTH(r->body);

        FOREACH(DLAtom,a,r->body)
        {
            argPos++;
            if (!ruleWon && (argPos + 1) == rNumGoals
                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
                            == getMatched && rNumGoals > 1)
            {
                ruleArgs = copyObject(ruleArgs);
            }
            else
            {
                FOREACH(DLNode,arg,a->args)
                {
                    if (!searchListNode(ruleArgs, (Node *) arg))
                        ruleArgs = appendToTailOfList(ruleArgs,
                                copyObject(arg));
                }
            }
        }
        DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

        char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
                strlen(NON_LINKED_POSTFIX));
        Node *lExpr;
        Node *rExpr;
        DLRule *moveRule;
        int goalPos = -1;
        boolean goalChk;
        List *newRuleHeadArgs = NIL;
        List *boolArgs = removeVars(r->head->args, ruleArgs);

        // head atom
        lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                               copyObject(origAtom->args));

        // create a list for collecting rule id
        int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
                STRING_VALUE(createConstBool(ruleWon)));
        int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
        collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

        // reset the position to check if the rule id is changed
        if (LIST_LENGTH(collectRuleId) > 1)
            if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
                    != getNthOfListInt(collectRuleId, ruleIdPos))
                checkPos = 0;

        if (!ruleWon)
        {
            // generate boolean args with variables to reduce redundant move rules
            newBoolArgs = NIL;
            for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
                    checkLoop++)
            {
                bName = CONCAT_STRINGS("BL", itoa(checkLoop));
                createBoolArgs = createDLVar(bName, DT_BOOL);
                newBoolArgs = appendToTailOfList(newBoolArgs,
                        copyObject(createBoolArgs));
            }

            if (checkPos < LIST_LENGTH(boolArgs))
                goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
        }

        if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
                || ruleWon)
        {
            if (!ruleWon)
            {
                // update the boolean arguments
                List *replaceBoolArgs = copyObject(newBoolArgs);
                replaceBoolArgs = replaceNode(replaceBoolArgs,
                        getNthOfListP(replaceBoolArgs, checkPos),
                        getNthOfListP(boolArgs, checkPos));

                newRuleHeadArgs = removeVars(r->head->args,
                        removeVars(r->head->args, ruleArgs));
                for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
                    newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
                            getNthOfListP(replaceBoolArgs, k));
            }

            FOREACH(DLAtom,a,r->body)
            {
                goalPos++;
//                int numHeadArgs = LIST_LENGTH(r->head->args);
                boolean goalWon = FALSE;
                lExpr = copyObject(lExpr);

                // Not include the additional atom for filtering out
                if (!ruleWon && (goalPos + 1) == rNumGoals
                        && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
                {
                    goalWon = TRUE;
                }
                else
                {
                    if ((!ruleWon && checkPos == j) || ruleWon)
                    {
                        ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
                        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
                                DL_ORIG_ATOM);

                        char *atomRel = CONCAT_STRINGS("r",strdup(origAtom->rel),
                                ruleWon ? "_WON" : "_LOST");
                        char *negAtomRel = CONCAT_STRINGS("r",strdup(origAtom->rel),
                                !ruleWon ? "_WON" : "_LOST");

                        if (!ruleWon)
                        {
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//                            if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//                                    == getMatched)
//                            {
//                                if (goalPos != rNumGoals - 1)
//                                    goalWon = BOOL_VALUE(
//                                            getNthOfListP(r->head->args,
//                                                    numHeadArgs - (rNumGoals - 1) + goalPos));
//                            }
//                            else
//                                goalWon = BOOL_VALUE(
//                                        getNthOfListP(r->head->args,
//                                                numHeadArgs - rNumGoals + goalPos));
                        }

                        // head atom -> negR
                        if (a->negated)
                        {
                            if (!goalWon)
                            {
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        negAtomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                        // head atom -> posR
                        else
                        {
                            if (!goalWon || ruleWon)
                            {
                                rExpr = createSkolemExpr(GP_NODE_EDB,
                                        atomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                    }
                }
                DEBUG_LOG("created new move rule for tuple -> tuple");
                j++;
            }
            checkPos++;
        }
        ruleIdPos++;
    }

    return moveRules;
}

static List*createGPReducedMoveRules(int getMatched, List* negedbRules, List* edbRules,
        List* unLinkedRules)
{
    List *moveRules = NIL;
    int checkPos = 0;
    char *bName = NULL;
    List *newBoolArgs = NIL;
    List *collectRuleId = NIL;
    DLVar* createBoolArgs;
    int ruleIdPos = 0;

    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,
                DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

        // Collecting all the original variables for later use
        int argPos = -1;
        List *ruleArgs = NIL;
        int rNumGoals = LIST_LENGTH(r->body);

        FOREACH(DLAtom,a,r->body)
        {
            argPos++;
            if (!ruleWon && (argPos + 1) == rNumGoals
                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
            {
                ruleArgs = copyObject(ruleArgs);
            }
            else
            {
                FOREACH(DLNode,arg,a->args)
                {
                    if (!searchListNode(ruleArgs, (Node *) arg))
                        ruleArgs = appendToTailOfList(ruleArgs,
                                copyObject(arg));
				}
            }
        }
        DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

        char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *ruleRel = CONCAT_STRINGS(
                CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
                ruleWon ? "_WON" : "_LOST");
        int i = INT_VALUE(DL_GET_PROP(r,DL_RULE_ID));
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
                strlen(NON_LINKED_POSTFIX));

        // head -> rule_i
        if (!ruleWon)
        {
            Node *lExpr;
            Node *rExpr;
            DLRule *moveRule;
            lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                    copyObject(origAtom->args));
            rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel,
                    copyObject(
                            removeVars(r->head->args,
                                    removeVars(r->head->args, ruleArgs))));
            moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                    r->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);
        }
        else
        {
            Node *lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                    copyObject(origAtom->args));
            Node *rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel,
                    copyObject(r->head->args));
            DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                    r->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);
        }

        // rule_i -> goal_i_j -> posR/negR -> posR
        int goalPos = -1;

        // remove over generated move rules
        boolean goalChk;
        List *newRuleHeadArgs = NIL;
        List *boolArgs = removeVars(r->head->args, ruleArgs);

        // create a list for collecting rule id
        int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
                STRING_VALUE(createConstBool(ruleWon)));
        int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
        collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

        // reset the position to check if the rule id is changed
        if (LIST_LENGTH(collectRuleId) > 1)
            if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
                    != getNthOfListInt(collectRuleId, ruleIdPos))
                checkPos = 0;

        if (!ruleWon)
        {
            // generate boolean args with variables to reduce redundant move rules
            newBoolArgs = NIL;
            for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
                    checkLoop++)
            {
                bName = CONCAT_STRINGS("BL", itoa(checkLoop));
                createBoolArgs = createDLVar(bName, DT_BOOL);
                newBoolArgs = appendToTailOfList(newBoolArgs,
                        copyObject(createBoolArgs));
            }

            if (checkPos < LIST_LENGTH(boolArgs))
                goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
        }

        if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
                || ruleWon)
        {
            if (!ruleWon)
            {
                // update the boolean arguments
                List *replaceBoolArgs = copyObject(newBoolArgs);
                replaceBoolArgs = replaceNode(replaceBoolArgs,
                        getNthOfListP(replaceBoolArgs, checkPos),
                        getNthOfListP(boolArgs, checkPos));

                newRuleHeadArgs = removeVars(r->head->args,
                        removeVars(r->head->args, ruleArgs));
                for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
                    newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
                            getNthOfListP(replaceBoolArgs, k));
            }

            FOREACH(DLAtom,a,r->body)
            {
                goalPos++;
                //              int unruleNumGoals = LIST_LENGTH(r->body);
//                int numHeadArgs = LIST_LENGTH(r->head->args);
                boolean goalWon = FALSE;
                boolean relWon = FALSE;

                // Not include the additional atom for filtering out
                if (!ruleWon && (goalPos + 1) == rNumGoals
                        && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched && rNumGoals > 1)
                {
                    goalWon = TRUE;
                }
                else
                {
                    if ((!ruleWon && checkPos == j) || ruleWon)
                    {
                        ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
                        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
                                DL_ORIG_ATOM);

                        char *goalRel = CONCAT_STRINGS(itoa(i), "_", itoa(j),
                                ruleWon ? "_WON" : "_LOST");

                        // is goal won?
                        if (!ruleWon)
                        {
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//                            if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//                                    == getMatched)
//                            {
//                                if (goalPos != rNumGoals - 1)
//                                    goalWon = BOOL_VALUE(
//                                            getNthOfListP(r->head->args,
//                                                    numHeadArgs - (rNumGoals - 1) + goalPos));
//                            }
//                            else
//                                goalWon = BOOL_VALUE(
//                                        getNthOfListP(r->head->args,
//                                                numHeadArgs - rNumGoals + goalPos));
                        }
                        else
                            goalWon = TRUE;

                        DEBUG_LOG("goal is %s and goal is negated %s", goalWon ? "TRUE" : "FALSE", a->negated ? "TRUE" : "FALSE");

                        // is tuple won?
                        if ((goalWon && (!a->negated)) || ((!goalWon) && a->negated))
                        {
                            relWon = TRUE;
                        }
                        else
                        {
                            relWon = FALSE;
                        }

                        char *atomRel = CONCAT_STRINGS(strdup(origAtom->rel),
                                relWon ? "_WON" : "_LOST");

                        // -> posR
                        if (a->negated)
                        {
                            if (!goalWon || ruleWon)
                            {
                                Node *lExpr = createSkolemExpr(GP_NODE_RULE,
                                        ruleRel,
                                        copyObject(
                                                removeVars(r->head->args,
                                                        removeVars(
                                                                r->head->args,
                                                                ruleArgs))));
                                Node *rExpr = createSkolemExpr(GP_NODE_GOAL,
                                        goalRel, copyObject(a->args));
                                DLRule *moveRule;
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel,
                                        copyObject(a->args));

                                // check the goal predicate contains the head predicate
//                                if(idbHeadPred != NULL && strstr(a->rel,idbHeadPred) != NULL)
                                if(DL_HAS_PROP(a, DL_IS_IDB_REL))
                                	rExpr = createSkolemExpr(GP_NODE_POSREL,
                                	        atomRel, copyObject(a->args));
                                else
                                	rExpr = createSkolemExpr(GP_NODE_TUPLE,
                                			atomRel, copyObject(a->args));

                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                        // -> posR
                        else
                        {
                            if (!goalWon || ruleWon)
                            {
                                Node *lExpr;
                                if (!ruleWon)
                                    lExpr =
                                            createSkolemExpr(GP_NODE_RULE,
                                                    ruleRel,
                                                    copyObject(
                                                            removeVars(
                                                                    r->head->args,
                                                                    removeVars(
                                                                            r->head->args,
                                                                            ruleArgs))));
                                else
                                    lExpr = createSkolemExpr(GP_NODE_RULE,
                                            ruleRel, copyObject(r->head->args));

                                Node *rExpr = createSkolemExpr(GP_NODE_GOAL,
                                        goalRel, copyObject(a->args));

                                DLRule *moveRule;
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel,
                                        copyObject(a->args));

                                char *Rel = CONCAT_STRINGS(strdup(origAtom->rel),
                                				ruleWon ? "_WON" : "_LOST");

                                // check the goal predicate contains the head predicate
//                                if(idbHeadPred != NULL && strstr(a->rel,idbHeadPred) != NULL)
                                if(DL_HAS_PROP(a, DL_IS_IDB_REL))
                                	rExpr = createSkolemExpr(GP_NODE_POSREL,
                                			Rel, copyObject(a->args));
                                else
                                	rExpr = createSkolemExpr(GP_NODE_TUPLE,
                                			Rel, copyObject(a->args));

                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                    }
                }
                DEBUG_LOG("created new move rule for head -> rule");
                j++;
            }
            checkPos++;
        }
        ruleIdPos++;
    }

    return moveRules;
}

static List*
createGPMoveRules(int getMatched, List* negedbRules,
        List* edbRules, List* unLinkedRules)
{
    List *moveRules = NIL;
    // create rules for move relation
    // for edb-help rule
    FOREACH(DLRule,e,negedbRules)
    {
        boolean ruleWon = DL_HAS_PROP(e->head,
                DL_WON) || DL_HAS_PROP(e->head,DL_UNDER_NEG_WON);

        ASSERT(DL_HAS_PROP(e->head,DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(e->head, DL_ORIG_ATOM);
        char *rel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *negRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_LOST" : "_WON");
        char *headName = strRemPostfix(strdup(e->head->rel),
                strlen(NON_LINKED_POSTFIX));

        if (!ruleWon)
        {

            Node *lExpr = createSkolemExpr(GP_NODE_NEGREL, negRel,
                    e->head->args);
            Node *rExpr = createSkolemExpr(GP_NODE_POSREL, rel, e->head->args);
            DLRule *moveRule = createMoveRule(lExpr, rExpr, headName,
                    e->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);
        }
    }
    // for each edb rule create two move entries
    FOREACH(DLRule,e,edbRules)
    {
        boolean ruleWon = DL_HAS_PROP(e->head,
                DL_WON) || DL_HAS_PROP(e->head,DL_UNDER_NEG_WON);

        ASSERT(DL_HAS_PROP(e->head,DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(e->head, DL_ORIG_ATOM);

        char *rel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *negRel = CONCAT_STRINGS("r", strdup(origAtom->rel),
                ruleWon ? "_LOST" : "_WON");
        char *headName = strRemPostfix(strdup(e->head->rel),
                strlen(NON_LINKED_POSTFIX));

        // if is won then we
        if (ruleWon)
        {

            //            Node *lExpr = createSkolemExpr(GP_NODE_NEGREL, negRel, e->head->args);
            //            Node *rExpr = createSkolemExpr(GP_NODE_POSREL, rel, e->head->args);
            //            DLRule *moveRule = createMoveRule(lExpr, rExpr, headName, e->head->args);
            //            moveRules = appendToTailOfList(moveRules, moveRule);

            Node *lExpr = createSkolemExpr(GP_NODE_POSREL, rel, e->head->args);
            Node *rExpr = createSkolemExpr(GP_NODE_EDB, negRel, e->head->args);
            DLRule *moveRule = createMoveRule(lExpr, rExpr, headName,
                    e->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);

            DEBUG_LOG("NEW MOVE RULE: negR-atom -> R-atom -> R: %s", headName);

        }
    }
    // for each rule_i do head -> rule_i, rule_i -> goal_i_j, goal_i_j -> posR/negR -> posR?
    int checkPos = 0;
    // To filter out unnecessary move rules (rule_i -> goal_i_j, goal_i_j -> posR/negR)int ruleId = 0;
    // To check rule id is changedint ruleIdPos = 0;
    // position in the list of rule idint ruleIdCheck = 0;
    char *bName = NULL;
    List *newBoolArgs = NIL;
    List *collectRuleId = NIL;
    DLVar* createBoolArgs;
    int ruleIdPos = 0;
    boolean filterNegHead = FALSE;

    FOREACH(DLRule,r,unLinkedRules)
    {
        boolean ruleWon = DL_HAS_PROP(r->head,
                DL_WON) || DL_HAS_PROP(r->head,DL_UNDER_NEG_WON);
        //        boolean ruleNeg = DL_HAS_PROP(r->head,DL_UNDER_NEG_WON)
        //                                       || DL_HAS_PROP(r->head,DL_UNDER_NEG_LOST);
        ASSERT(DL_HAS_PROP(r->head, DL_ORIG_ATOM));
        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(r->head, DL_ORIG_ATOM);

        // Collecting all the original variables for later use
        int argPos = -1;
        List *ruleArgs = NIL;
        int rNumGoals = LIST_LENGTH(r->body);

        FOREACH(DLAtom,a,r->body)
        {
            argPos++;
            if (!ruleWon && (argPos + 1) == rNumGoals
                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
                            == getMatched && rNumGoals > 1)
            {
                ruleArgs = copyObject(ruleArgs);
            }
            else
            {
                FOREACH(DLNode,arg,a->args)
                {
                    if (!searchListNode(ruleArgs, (Node *) arg))
                        ruleArgs = appendToTailOfList(ruleArgs,
                                copyObject(arg));
                }
            }

            if(ruleWon)
            	if(a->negated && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched)
            		filterNegHead = TRUE;
        }
        //        DEBUG_LOG("List Length:%d", LIST_LENGTH(r->body));
        DEBUG_LOG("args for rule:%s", exprToSQL((Node * ) ruleArgs));

        char *negHeadRel = CONCAT_STRINGS(strdup(origAtom->rel), "_WON");
        char *headRel = CONCAT_STRINGS(strdup(origAtom->rel),
                ruleWon ? "_WON" : "_LOST");
        char *ruleRel = CONCAT_STRINGS(
                CONST_TO_STRING(DL_GET_PROP(r,DL_RULE_ID)),
                !ruleWon ? "_WON" : "_LOST");
        int i = INT_VALUE(DL_GET_PROP(r,DL_RULE_ID));
        int j = 0;
        char *linkedHeadName = strRemPostfix(strdup(r->head->rel),
                strlen(NON_LINKED_POSTFIX));

        // head -> rule_i
        if (!ruleWon)
        {

            Node *lExpr;
            Node *rExpr;
            DLRule *moveRule;

//            if ((((DLAtom *) r->body)->negated
//                    && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
//                            == getMatched) || !((DLAtom *) r->body)->negated)
            if(!filterNegHead)
            {
                lExpr = createSkolemExpr(GP_NODE_NEGREL, negHeadRel,
                        copyObject(origAtom->args));
                rExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                        copyObject(origAtom->args));
                moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                        r->head->args);
                moveRules = appendToTailOfList(moveRules, moveRule);
            }

            lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                    copyObject(origAtom->args));
            rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel,
                    copyObject(
                            removeVars(r->head->args,
                                    removeVars(r->head->args, ruleArgs))));
            moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                    r->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);

        }
        else
        {

            Node *lExpr = createSkolemExpr(GP_NODE_POSREL, headRel,
                    copyObject(origAtom->args));
            Node *rExpr = createSkolemExpr(GP_NODE_RULE, ruleRel,
                    copyObject(r->head->args));
            DLRule *moveRule = createMoveRule(lExpr, rExpr, linkedHeadName,
                    r->head->args);
            moveRules = appendToTailOfList(moveRules, moveRule);
        }

        // rule_i -> goal_i_j -> posR/negR -> posR
        int goalPos = -1;

        // remove over generated move rules
        boolean goalChk;
        List *newRuleHeadArgs = NIL;
        List *boolArgs = removeVars(r->head->args, ruleArgs);

        // create a list for collecting rule id
        int ruleId = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));
        char *newRuleId = CONCAT_STRINGS(itoa(ruleId),
                STRING_VALUE(createConstBool(ruleWon)));
        int ruleIdCheck = INT_VALUE(createConstString(newRuleId));
        collectRuleId = appendToTailOfListInt(collectRuleId, ruleIdCheck);

        // reset the position to check if the rule id is changed
        if (LIST_LENGTH(collectRuleId) > 1)
            if (getNthOfListInt(collectRuleId, ruleIdPos - 1)
                    != getNthOfListInt(collectRuleId, ruleIdPos))
                checkPos = 0;

        if (!ruleWon)
        {
            // generate boolean args with variables to reduce redundant move rules
            newBoolArgs = NIL;
            for (int checkLoop = 0; checkLoop < LIST_LENGTH(boolArgs);
                    checkLoop++)
            {
                bName = CONCAT_STRINGS("BL", itoa(checkLoop));
                createBoolArgs = createDLVar(bName, DT_BOOL);
                newBoolArgs = appendToTailOfList(newBoolArgs,
                        copyObject(createBoolArgs));
            }

            if (checkPos < LIST_LENGTH(boolArgs))
                goalChk = BOOL_VALUE(getNthOfListP(boolArgs, checkPos));
        }

        if ((!ruleWon && !goalChk && checkPos < LIST_LENGTH(boolArgs))
                || ruleWon)
        {
            if (!ruleWon)
            {
                // update the boolean arguments
                List *replaceBoolArgs = copyObject(newBoolArgs);
                replaceBoolArgs = replaceNode(replaceBoolArgs,
                        getNthOfListP(replaceBoolArgs, checkPos),
                        getNthOfListP(boolArgs, checkPos));

                newRuleHeadArgs = removeVars(r->head->args,
                        removeVars(r->head->args, ruleArgs));
                for (int k = 0; k < LIST_LENGTH(replaceBoolArgs); k++)
                    newRuleHeadArgs = appendToTailOfList(newRuleHeadArgs,
                            getNthOfListP(replaceBoolArgs, k));
            }

            FOREACH(DLAtom,a,r->body)
            {
                goalPos++;
                //				int unruleNumGoals = LIST_LENGTH(r->body);
//                int numHeadArgs = LIST_LENGTH(r->head->args);
                boolean goalWon = FALSE;

                // Not include the additional atom for filtering out
                if (!ruleWon && (goalPos + 1) == rNumGoals
                        && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))
                                == getMatched && rNumGoals > 1)
                {
                    goalWon = TRUE;
                }
                else
                {
                    if ((!ruleWon && checkPos == j) || ruleWon)
                    {
                        ASSERT(DL_HAS_PROP(a,DL_ORIG_ATOM));
                        DLAtom *origAtom = (DLAtom *) DL_GET_PROP(a,
                                DL_ORIG_ATOM);

                        char *goalRel = CONCAT_STRINGS(itoa(i), "_", itoa(j),
                                ruleWon ? "_WON" : "_LOST");
                        char *atomRel = CONCAT_STRINGS(strdup(origAtom->rel),
                                ruleWon ? "_WON" : "_LOST");
                        char *negAtomRel = CONCAT_STRINGS(strdup(origAtom->rel),
                                !ruleWon ? "_WON" : "_LOST");
                        //            char *rel = CONCAT_STRINGS(strdup(origAtom->rel), ruleWon ? "_WON" : "_LOST");
                        //            char *negRel = CONCAT_STRINGS(strdup(origAtom->rel), ruleWon ? "_LOST" : "_WON");

                        if (!ruleWon)
                        {
                            DEBUG_LOG("Only Boolean Args:%s", exprToSQL((Node * ) removeVars(r->head->args,ruleArgs)));
                            goalWon = BOOL_VALUE(getNthOfListP(removeVars(r->head->args,ruleArgs),goalPos));

//                        	if (INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched)
//                            {
//                                if (goalPos != rNumGoals - 1)
//                                    goalWon = BOOL_VALUE(
//                                            getNthOfListP(r->head->args,
//                                                    numHeadArgs - (rNumGoals - 1) + goalPos));
//                            }
//                            else
//                                goalWon = BOOL_VALUE(getNthOfListP(r->head->args, numHeadArgs - rNumGoals + goalPos));
                        }

                        // -> posR
                        if (a->negated)
                        {
                            if (!goalWon)
                            {
                                Node *lExpr = createSkolemExpr(GP_NODE_RULE,
                                        ruleRel,
                                        copyObject(
                                                removeVars(r->head->args,
                                                        removeVars(
                                                                r->head->args,
                                                                ruleArgs))));
                                Node *rExpr = createSkolemExpr(GP_NODE_GOAL,
                                        goalRel, copyObject(a->args));
                                DLRule *moveRule;
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel,
                                        copyObject(a->args));
                                rExpr = createSkolemExpr(GP_NODE_POSREL,
                                        negAtomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

//                                if (!ruleWon)
//                                {
//                                    lExpr = createSkolemExpr(GP_NODE_POSREL,
//                                            negAtomRel, copyObject(a->args));
//                                    rExpr = createSkolemExpr(GP_NODE_EDB,
//                                            atomRel, copyObject(a->args));
//                                    if (ruleWon)
//                                        moveRule = createMoveRule(lExpr, rExpr,
//                                                linkedHeadName, r->head->args);
//                                    else
//                                        moveRule = createMoveRule(lExpr, rExpr,
//                                                linkedHeadName,
//                                                copyObject(newRuleHeadArgs));
//
//                                    moveRules = appendToTailOfList(moveRules,
//                                            moveRule);
//                                }
                            }
                        }
                        // -> negR -> posR
                        else
                        {
                            if (!goalWon || ruleWon)
                            {
                                Node *lExpr;
                                if (!ruleWon)
                                    lExpr =
                                            createSkolemExpr(GP_NODE_RULE,
                                                    ruleRel,
                                                    copyObject(
                                                            removeVars(
                                                                    r->head->args,
                                                                    removeVars(
                                                                            r->head->args,
                                                                            ruleArgs))));
                                else
                                    lExpr = createSkolemExpr(GP_NODE_RULE,
                                            ruleRel, copyObject(r->head->args));

                                Node *rExpr = createSkolemExpr(GP_NODE_GOAL,
                                        goalRel, copyObject(a->args));
                                DLRule *moveRule;
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_GOAL, goalRel,
                                        copyObject(a->args));
                                rExpr = createSkolemExpr(GP_NODE_NEGREL,
                                        negAtomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);

                                lExpr = createSkolemExpr(GP_NODE_NEGREL,
                                        negAtomRel, copyObject(a->args));
                                rExpr = createSkolemExpr(GP_NODE_POSREL,
                                        atomRel, copyObject(a->args));
                                if (ruleWon)
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName, r->head->args);
                                else
                                    moveRule = createMoveRule(lExpr, rExpr,
                                            linkedHeadName,
                                            copyObject(newRuleHeadArgs));

                                moveRules = appendToTailOfList(moveRules,
                                        moveRule);
                            }
                        }
                    }
                }
                DEBUG_LOG("created new move rule for head -> rule");
                j++;
            }
            checkPos++;
        }
        ruleIdPos++;
    }

    return moveRules;
}

/*
 * Rewrite program based on adornments created when statically solving the game for the program.
 * We use the solved program to created rules for each adorned version of a unified rule/goal atom
 */
//List *listTopHeadArgs = NIL;

static DLProgram *
rewriteSolvedProgram (DLProgram *solvedProgram)
{
    DLProgram *result = makeNode(DLProgram);
    List *newRules = NIL;
    List *unLinkedRules = NIL;
    List *unLinkedHelpRules = NIL;
    List *helpRules = NIL;
    List *negedbRules = NIL;
    List *edbRules = NIL;
    List *moveRules = NIL;
    List *origArgs = NIL;
    List *newRuleArg = NIL;
    List *origProg = NIL;
	Set *adornedEDBAtoms = NODESET();
	Set *adornedEDBHelpAtoms = NODESET();
    HashMap *idbAdToRules = NEW_MAP(Node,Node);
    char *fmt = STRING_VALUE(DL_GET_PROP((DLNode *) solvedProgram, DL_PROV_FORMAT));
    result->rules = copyObject(solvedProgram->rules);
    DLVar *createArgs;
    DLRule *ruleRule;
	char *vName = NULL;

    // collect rules and adornedheads we are interested in

    // create rules for Rule_i^adornment:
    //  - create head^adornment :- rule^adornment
    //  - create rule^adornment :- adornedBody
	int getFirstRule = 0;
	int getMatched = 0;

	// New Implementation Test
//    DLRule *origRule;
//    List *newGoalArgs = NIL;
//	List *headPred = NIL;
//	List *bodyPred = NIL;

	// store orig program to analyze later on
	if(solvedProgram->doms != NIL)
	{
		origProg = solvedProgram->rules;
		FOREACH(DLRule,a,origProg)
			DL_SET_BOOL_PROP(a,DL_ORIGINAL_RULE);
	}

	FOREACH(DLRule,r,solvedProgram->rules)
    {
//		// collect predicates
//		headPred = appendToTailOfList(headPred, copyObject(r->head));
//
//		if (getFirstRule == 0)
//			FOREACH(DLAtom,p,r->body)
//				bodyPred = appendToTailOfList(bodyPred, copyObject(p));


	    int numGoals = 0;
//		List *addArg = NIL;
//        List *addBoolConst = NIL;
    	boolean ruleWon = DL_HAS_PROP(r,DL_WON)
                           || DL_HAS_PROP(r,DL_UNDER_NEG_WON);
        /*
    	boolean ruleNeg = DL_HAS_PROP(r,DL_UNDER_NEG_WON)
                           || DL_HAS_PROP(r,DL_UNDER_NEG_LOST);
		*/

   		if (getFirstRule == 0)
   			getMatched = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));

        char *adHeadName = CONCAT_STRINGS("R", r->head->rel, "_",
        		ruleWon ? "WON" : "LOST");
        		/*
        		ruleWon ? "WON" : "LOST", "_" ,
                ruleNeg ? "-" : "+");
                */
        char *adRuleName = CONCAT_STRINGS("r",
                itoa(INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))), "_",
				ruleWon ? "WON" : "WL");
//				ruleWon ? "WON" : "LOST");

        //New Implementation Test
//        char *posRuleName = CONCAT_STRINGS("r",
//        		itoa(INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))), "_",
//				"WON");
        // ************************************************************
        // create rule rule^adornment :- adornedBody
        DEBUG_LOG("create GP RULE rule for %s based on rule:\n%s", adRuleName,
                datalogToOverviewString((Node *) r));
        ruleRule = copyObject(r);
//        origRule = copyObject(r);

        origArgs = removeVars(makeUniqueVarList(getRuleVars(ruleRule)), ruleRule->head->args);
        origArgs = CONCAT_LISTS(ruleRule->head->args, origArgs);
        newRuleArg = copyObject(origArgs);
        // add args for boolean
        if (!ruleWon) {

        	int j = 0;

        	FOREACH(DLNode,n,ruleRule->body) //TODO for(int i = 0; i < LIST_LENGTH(ruleRule->body); i++), but, e.g.,  Y=3
        	{
        	    if (isA(n,DLAtom))
        	    {
        	        vName = CONCAT_STRINGS("BL", itoa(j++));
                    createArgs = createDLVar(vName, DT_BOOL);

                    numGoals++; // For calculation of length of only new args
                    newRuleArg = appendToTailOfList(newRuleArg, copyObject(createArgs));
        	    }
        	}

        }
//        DEBUG_LOG("new args for rule head are: %s", datalogToOverviewString((Node *) newRuleArgs));

        // not under negated rule
        //if (!ruleNeg)
        //{
            // won rule not under neg - replace Q(X) :- g_i,1(Y_1), ... g_i,n(Y_n) with Q(X,Y) :- ...

/*
            if (ruleWon)
            {
                // add all vars to head
                ruleRule->head->rel = CONCAT_STRINGS(adRuleName, NON_LINKED_POSTFIX);
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

//                    boolean ruleNeg = DL_HAS_PROP(r,DL_UNDER_NEG_WON)
//                                       || DL_HAS_PROP(r,DL_UNDER_NEG_LOST);

                    char *adHeadName = CONCAT_STRINGS("R", a->rel, "_",
                            ruleWon ? "WON" : "LOST",
                            NON_LINKED_POSTFIX);

                    setDLProp((DLNode *) a, DL_ORIG_ATOM, (Node *) copyObject(a));
                    a->rel = adHeadName;
                }
            }
            // lost rules
            else
            {
*/
            	// add all vars to head
                ruleRule->head->rel = CONCAT_STRINGS(adRuleName, NON_LINKED_POSTFIX);
                ruleRule->head->args = copyObject(newRuleArg);
                setDLProp((DLNode *) ruleRule->head, DL_ORIG_ATOM, (Node *) copyObject(r->head));

                // adapt goal nodes
                FOREACH(DLAtom,a,ruleRule->body) //TODO comparison atoms
                {
                	// if an edb atom
                    if (!DL_HAS_PROP(a, DL_IS_IDB_REL))
                    {
                    	DLAtom *at;
                        AD_NORM_COPY(at,a);
//                        if(ruleWon)
                        	addToSet(adornedEDBAtoms, at);
//                        else
//                            addToSet(adornedEDBHelpAtoms, at);
                    }

                    boolean ruleWon = DL_HAS_PROP(r,DL_WON)
                    						|| DL_HAS_PROP(r,DL_UNDER_NEG_WON);

//                    char *adHeadName = CONCAT_STRINGS("R", a->rel, "_",
//                    						ruleWon ? "WON" : "WL",
//                        	            	NON_LINKED_POSTFIX);

                    char *adHeadName = CONCAT_STRINGS("R", a->rel, "_",
                    						ruleWon ? "WON" : "WL",
                        	            	NON_LINKED_POSTFIX);

                    setDLProp((DLNode *) a, DL_ORIG_ATOM, (Node *) copyObject(a));
                    a->rel = adHeadName;

//                    if(!ruleWon)
//                    {
//    					vName = CONCAT_STRINGS("BL", itoa(j++));
//    					createArgs = createDLVar(vName, DT_BOOL);
//
//    					newGoalArgs = copyObject(a->args);
//    					newGoalArgs = appendToTailOfList(newGoalArgs, copyObject(createArgs));
//    					a->args = copyObject(newGoalArgs);
//                    }
                }


//            }
        //}
        // under negated rule
        /*
        else
        {
            // won rule under negation
            if (ruleWon)
            {

            }
            // lost rule under negation
            else
            {

            }
        }
        */

        DEBUG_LOG("created new rule:\n%s", datalogToOverviewString((Node *) ruleRule));
        // create rule head^adornment :- rule^adornment
        DEBUG_LOG("create GP HEAD rule for %s based on rule:\n%s", adHeadName, datalogToOverviewString((Node *) r));

        DLRule *headRule = makeNode(DLRule);
        DLAtom *adHead = copyObject(r->head);
        DLAtom *ruleAtom = makeNode(DLAtom);

        // create head head^adornment(X) if head(X) for original rule
        headRule->head = adHead;
//        adHead->rel = strdup(adHeadName);

        char *adNegHeadName = CONCAT_STRINGS("R", r->head->rel, "_", ruleWon ? "WON" : "LOST");
        adHead->rel = CONCAT_STRINGS(strdup(adNegHeadName), NON_LINKED_POSTFIX);
        setDLProp((DLNode *) adHead, DL_ORIG_ATOM, (Node *) copyObject(r->head));

        // create rule atom rule^adornment(X) if rule(X) where X are all vars in rule
        if (ruleWon)
        {
			ruleAtom->rel = CONCAT_STRINGS(strdup(adRuleName), NON_LINKED_POSTFIX);
			ruleAtom->args = copyObject(newRuleArg);
			headRule->body = singleton(ruleAtom);

			DLAtom *lookupAtom;
			AD_NORM_COPY(lookupAtom,headRule->head);
			CONCAT_MAP_LIST(idbAdToRules,(Node *) lookupAtom, singleton(ruleRule));

			helpRules = appendToTailOfList(helpRules, headRule);
//            unLinkedRules = appendToTailOfList(unLinkedRules, ruleRule);
        }
        else
        {
            DLAtom *posHeadAtom = copyObject(r->head);

           	char *PosHeadNm = CONCAT_STRINGS("R", r->head->rel, "_", "WON");
           	posHeadAtom->negated = TRUE;
            posHeadAtom->rel = CONCAT_STRINGS(strdup(PosHeadNm), NON_LINKED_POSTFIX);
            headRule->body = singleton(posHeadAtom);

            DLAtom *lookupAtom;
            AD_NORM_COPY(lookupAtom,headRule->head);
            CONCAT_MAP_LIST(idbAdToRules,(Node *) lookupAtom, singleton(ruleRule));

            helpRules = appendToTailOfList(helpRules, headRule);

        	DLRule *PosHeadRule = makeNode(DLRule);
            DLAtom *adPosHead = copyObject(r->head);

            PosHeadRule->head = adPosHead;
        	adPosHead->rel = CONCAT_STRINGS(strdup(PosHeadNm), NON_LINKED_POSTFIX);
            setDLProp((DLNode *) adPosHead, DL_ORIG_ATOM, (Node *) copyObject(r->head));

           	ruleAtom->rel = CONCAT_STRINGS(strdup(adRuleName),
           			INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) != getMatched ? NON_LINKED_POSTFIX : NON_LINKED_POSTFIX_CHKPOS);

           	//New Implementation Test
//           	ruleAtom->rel = CONCAT_STRINGS(strdup(posRuleName),
//           			INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) != getMatched ? NON_LINKED_POSTFIX : NON_LINKED_POSTFIX_CHKPOS);

    	    newRuleArg = copyObject(origArgs);
//          	addArg = NIL;

        	FOREACH(DLNode,n,ruleRule->body) //TODO for(int i = 0; i < LIST_LENGTH(ruleRule->body); i++), but, e.g.,  Y=3
        	{
        	    if (isA(n,DLAtom))
        	    {
//                    addArg = appendToTailOfList(addArg, createConstBool(TRUE));
                    newRuleArg = appendToTailOfList(newRuleArg, createConstBool(TRUE));
        	    }
        	}

          	ruleAtom->args = newRuleArg;
//          	ruleAtom->negated = TRUE;
          	PosHeadRule->body = singleton(ruleAtom);

            AD_NORM_COPY(lookupAtom,PosHeadRule->head);
            CONCAT_MAP_LIST(idbAdToRules,(Node *) lookupAtom, singleton(ruleRule));

            helpRules = appendToTailOfList(helpRules, PosHeadRule);

            // New Implementation Test
//            unLinkedRules = appendToTailOfList(unLinkedRules, ruleRule);
//            origRule->head->rel = ruleAtom->rel;
//            origRule->head->args = copyObject(origArgs);
//
//            // adapt goal nodes
//			FOREACH(DLAtom,a,origRule->body) //TODO comparison atoms
//			{
////				boolean ruleWon = DL_HAS_PROP(r,DL_WON)
////										|| DL_HAS_PROP(r,DL_UNDER_NEG_WON);
//
//				char *adGoalName = CONCAT_STRINGS("R", a->rel, "_",
////										ruleWon ? "WON" : "WL",
//										"WON",
//										NON_LINKED_POSTFIX);
//
//				setDLProp((DLNode *) a, DL_ORIG_ATOM, (Node *) copyObject(a));
//				a->rel = adGoalName;
//
//				List *goalArgs = copyObject(a->args);
//				goalArgs = appendToTailOfList(goalArgs, createConstBool(TRUE));
//				a->args = copyObject(goalArgs);
//			}
//
//            unLinkedHelpRules = appendToTailOfList(unLinkedHelpRules, origRule);
        }

         //New Implementation Test
//        if(LIST_LENGTH(unLinkedHelpRules) != 0)
//        {
//            // adapt goal nodes
//    		FOREACH(DLAtom,a,origRule->body) //TODO comparison atoms
//    		{
//    			// if an edb atom
//    			if (!DL_HAS_PROP(a, DL_IS_IDB_REL))
//    			{
//    				DLAtom *at;
//    				AD_NORM_COPY(at,a);
//    				addToSet(adornedEDBHelpAtoms, at);
//    			}
//
//    			// check the args to make TRUE -> WON and FALSE -> LOST
//    			DLAtom *posHead = copyObject(a);
//    			DLAtom *posBody = copyObject(a);
//
//    			List *newBoolArgs = copyObject(a->args);
//    			newBoolArgs = removeFromTail(newBoolArgs);
//    			posBody->args = copyObject(newBoolArgs);
//
//    			DLRule *posRule = createDLRule(posHead, singleton(posBody));
//
//    			DLAtom *lookup;
//    			AD_NORM_COPY(lookup, posRule->head);
//    			CONCAT_MAP_LIST(idbAdToRules,(Node *) lookup, singleton(posRule));
//
//    			negedbRules = appendToTailOfList(negedbRules, posRule);
//
//
//    			DLAtom *negHead = copyObject(a);
//    			DLAtom *negBody = copyObject(a);
//
//    			char *negHeadName = a->rel;
//    			negHeadName = replaceSubstr(negHeadName,"WON","LOST");
//
//    			negHead->rel = negHeadName;
//    			negHead->args = copyObject(newBoolArgs);
//    			negHead->args = appendToTailOfList(negHead->args,createConstBool(FALSE));
//
//    			negBody->rel = negHeadName;
//    			negBody->args = copyObject(newBoolArgs);
//
//    			DLRule *negRule = createDLRule(negHead, singleton(negBody));
//
//    			AD_NORM_COPY(lookup, negRule->head);
//    			CONCAT_MAP_LIST(idbAdToRules,(Node *) lookup, singleton(negRule));
//
//    			negedbRules = appendToTailOfList(negedbRules, negRule);
//    		}
//        }

//        DLAtom *lookupAtom;
//        AD_NORM_COPY(lookupAtom,headRule->head);
////        DL_SET_BOOL_PROP((getDLProp((DLNode *) lookupAtom, DL_ORIG_ATOM)), DL_IS_IDB_REL);
//        CONCAT_MAP_LIST(idbAdToRules,(Node *) lookupAtom, singleton(ruleRule));


        DEBUG_LOG("created new head rule:\n%s", datalogToOverviewString((Node *) helpRules));

//        setIDBBody(ruleRule);
        if (ruleWon)
            unLinkedRules = appendToTailOfList(unLinkedRules, ruleRule);
        else
        {
        	//calculation of number of loop
        	int numLoop = 1;
        	for (int l = 1; l <= numGoals; l++) {
        		numLoop = numLoop * 2;
        	}
        	DEBUG_LOG("Number of loop:%d", numLoop);

        	//generating possible rules with args
        	boolean *numArgs = MALLOC(sizeof(boolean) * numGoals);
    	    newRuleArg = copyObject(origArgs);

    	    DEBUG_LOG("Args:%s", datalogToOverviewString((Node *) newRuleArg));
    	    DEBUG_LOG("Length of addArgs:%d", numGoals);

			//give boolean value for the args added
//			DLVar boolArgs;
        	for (int j = 0; j < numLoop; j++) {
        	    int curBitVec = j;
        		int pos, k;
//        		List *searchBoolArgs = NIL;
//                List *boolArgs = NIL;

        		// represent all the possible bit flip for boolean args
				for (pos = 0; pos < numGoals; pos++)
				{
					k = curBitVec >> pos;

					if (k & 1)
						numArgs[pos] = TRUE;
					else
						numArgs[pos] = FALSE;

	        		//add into the list
//					boolArgs = appendToTailOfList(boolArgs, createConstBool(numArgs[pos]));
					newRuleArg = appendToTailOfList(newRuleArg, createConstBool(numArgs[pos]));
//					searchBoolArgs = appendToTailOfListInt(searchBoolArgs, numArgs[pos]);
  				}

        		//add into the list
        		DEBUG_LOG("Rule Args:%s", exprToSQL((Node *) newRuleArg));

        		//create unlinked rules
				ruleRule = copyObject(r);
        		char *adNegRuleName = CONCAT_STRINGS("r", itoa(INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID))), "_WL");

				if (!searchListNode(newRuleArg, (Node *) createConstBool(FALSE)) && INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)) == getMatched)
					ruleRule->head->rel = CONCAT_STRINGS(adNegRuleName, NON_LINKED_POSTFIX_CHKPOS);
				else
					ruleRule->head->rel = CONCAT_STRINGS(adNegRuleName, NON_LINKED_POSTFIX);

				ruleRule->head->args = newRuleArg;
				setDLProp((DLNode *) ruleRule->head, DL_ORIG_ATOM, (Node *) copyObject(r->head));

				// adapt goal nodes
				int listPos = 0;
				int getPos = 0;

				FOREACH(DLAtom,a,ruleRule->body) //TODO comparison atoms
				{
					getPos = listPos + LIST_LENGTH(origArgs);

					// if an edb atom
					if (!DL_HAS_PROP(a, DL_IS_IDB_REL))
					{
						DLAtom *at;
						AD_NORM_COPY(at,a);
//						if (searchListInt(searchBoolArgs, 0))
//						if (searchListNode(newRuleArg, (Node *) createConstBool(FALSE)) || getFirstRule != 0)
						if (searchListNode(newRuleArg, (Node *) createConstBool(FALSE)))
							addToSet(adornedEDBHelpAtoms, at);
						else
							addToSet(adornedEDBAtoms, at);
					}

					// check the args to make TRUE -> WON and FALSE -> LOST
					char *adBodyName = NULL;
					boolean goalChk = BOOL_VALUE(getNthOfListP(newRuleArg, getPos));

//					adBodyName = CONCAT_STRINGS("R", a->rel, "_", (getNthOfListInt(searchBoolArgs, listPos) == 1) ? "WON" : "LOST", NON_LINKED_POSTFIX);
					adBodyName = CONCAT_STRINGS("R", a->rel, "_", goalChk ? "WON" : "LOST", NON_LINKED_POSTFIX);

					listPos++;

//					if (searchListNode(newRuleArg, (Node *) createConstBool(FALSE)) || getFirstRule != 0)
					if (searchListNode(newRuleArg, (Node *) createConstBool(FALSE)))
						setDLProp((DLNode *) a, DL_ORIG_ATOM, (Node *) copyObject(a));

					a->rel = adBodyName;
				}

//				if (searchListNode(newRuleArg, (Node *) createConstBool(FALSE)) || getFirstRule != 0)
				if (searchListNode(newRuleArg, (Node *) createConstBool(FALSE)))
					unLinkedRules = appendToTailOfList(unLinkedRules, ruleRule);
				else
					unLinkedHelpRules = appendToTailOfList(unLinkedHelpRules, ruleRule);

				ruleRule = copyObject(r);
				newRuleArg = copyObject(origArgs);
        	}

        }

        getFirstRule++;
    }

//	// check if the rule for ANS is multi-level to use for move rules later for simplified models
//	// TODO: apply to all simplified models as currently only for REDUCED GP
//	FORBOTH(DLAtom,h,b,headPred,bodyPred)
//	{
//		char *hp = (char *) h->rel;
//		char *bp = (char *) b->rel;
//
//		if(streq(hp,bp))
//			idbHeadPred = hp;
//	}


    DEBUG_LOG("------------- STEP 1 ---------------\n: created unlinked rules:\n%s\nand unliked help rules rules:\n%s\nand help rules:\n%s",
             datalogToOverviewString((Node *) unLinkedRules),
			 datalogToOverviewString((Node *) unLinkedHelpRules),
             datalogToOverviewString((Node *) helpRules));

    // create rules adorned edb atoms R^adornment
    //  - create R^adornment(X) :- R(X) if Won+
    //  - create R^adornment(X) :- not R(X) if Lost+
    //  - create R^adornment(X,TRUE) :- R(X), R^adornment(X,FALSE) :- not R(X) if Won-
    //  - create R^adornment(X,FALSE) :- R(X), R^adornment(X,TRUE) :- not R(X) if Lost-

    FOREACH_SET(DLAtom,edbhelp,adornedEDBHelpAtoms)
    {
        DLRule *atRule;
        DLAtom *atHead;
        DLAtom *atBody;

  	  	// edb negation
       	if (!DL_HAS_PROP(edbhelp, DL_IS_IDB_REL)) // if an edb atom
       	{
       		DLAtom *at;
       		AD_NORM_COPY(at,edbhelp);
       		addToSet(adornedEDBAtoms, at);
       	}

       	char *adNegAtomName = CONCAT_STRINGS("R", edbhelp->rel, "_", "LOST", NON_LINKED_POSTFIX);
       	atHead = copyObject(edbhelp);
       	atHead->rel = adNegAtomName;

       	atBody = copyObject(edbhelp);
       	char *adNegAtomBody = CONCAT_STRINGS("R", edbhelp->rel, "_", "WON", NON_LINKED_POSTFIX);
       	atBody->negated = TRUE;
       	atBody->rel = adNegAtomBody;

       	atRule = createDLRule(atHead, singleton(atBody));
       	setDLProp((DLNode *) atRule->head, DL_ORIG_ATOM, (Node *) edbhelp);
       	setDLProp((DLNode *) atBody, DL_ORIG_ATOM, (Node *) edbhelp);

       	DLAtom *lookup;
       	AD_NORM_COPY(lookup, atRule->head);
       	CONCAT_MAP_LIST(idbAdToRules,(Node *) lookup, singleton(atRule));

        negedbRules = appendToTailOfList(negedbRules, atRule);
    }
   	DEBUG_LOG("new EDB help rule generated:\n%s", datalogToOverviewString((Node *) negedbRules));

    FOREACH_SET(DLAtom,edb,adornedEDBAtoms)
    {
        DLRule *atRule;
        DLAtom *atHead;
        DLAtom *atBody;
        //DLRule *atNegRule = NULL;
//
//        boolean ruleWon = DL_HAS_PROP(edb,DL_WON)
//        						|| DL_HAS_PROP(edb,DL_UNDER_NEG_WON);
        /*
        boolean ruleNeg = DL_HAS_PROP(edb,DL_UNDER_NEG_WON);
                                       || DL_HAS_PROP(edb,DL_UNDER_NEG_LOST);
        */

        // positive case
    	atRule = makeNode(DLRule);
    	char *adAtomName = CONCAT_STRINGS("R", edb->rel, "_", "WON", NON_LINKED_POSTFIX);

        atHead = copyObject(edb);
        atHead->rel = adAtomName;
        atBody = copyObject(edb);

        //DEBUG_LOG("checkStart:\n%s", datalogToOverviewString((Node *) atRule));
        // is under negated
        //if (ruleNeg)
        //{
            //TODO
        //    atRule = createDLRule(atHead, singleton(atBody));
        //}
        //else
        //{
        atRule = createDLRule(atHead, singleton(atBody));
//        if (!ruleWon)
//        	atBody->negated = FALSE;
//        }
//        DEBUG_LOG("checkEnd:\n%s", datalogToOverviewString((Node *) atRule));


        // add rules to new rules list
        setDLProp((DLNode *) atRule->head, DL_ORIG_ATOM, (Node *) edb);

        DLAtom *lookup;
        AD_NORM_COPY(lookup, atRule->head);
        CONCAT_MAP_LIST(idbAdToRules,(Node *) lookup, singleton(atRule));

       	edbRules = appendToTailOfList(edbRules, atRule);
    }
   	DEBUG_LOG("new EDB rule generated:\n%s", datalogToOverviewString((Node *) edbRules));

    // mark IDB predicates in body //TODO necessary here?
    FOREACH(DLRule,r,unLinkedRules)
        setIDBBody(r);

    FOREACH(DLRule,r,unLinkedHelpRules)
        setIDBBody(r);

    FOREACH(DLRule,r,negedbRules)
        setIDBBody(r);


    DEBUG_LOG("------------- STEP 2 ---------------\n: created unlinked rules:\n%s\nand unlinked help rules:\n%s\nand help rules:\n%s\nand EDB help rules:\n%s\nand EDB rules:\n%s",
             datalogToOverviewString((Node *) unLinkedRules),
			 datalogToOverviewString((Node *) unLinkedHelpRules),
             datalogToOverviewString((Node *) helpRules),
			 datalogToOverviewString((Node *) negedbRules),
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

					ruleGoal = (DLAtom *) applyVarMapAsLists((Node *) ruleGoal,
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
    	boolean ruleWon = DL_HAS_PROP(unRule->head,DL_WON)
    	                                   || DL_HAS_PROP(unRule->head,DL_UNDER_NEG_WON);

        if (!hasSetElem(unHeadToRules,unRule->head))
        {
        	if((!ruleWon && INT_VALUE(getDLProp((DLNode *) unRule,DL_RULE_ID)) == getMatched) || ruleWon)
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
    }

    // double check if the user question is correct
    DLAtom *adAtom;
    getFirstRule = 0;
    getMatched = 0;
//    List *unlinkedHeads = NIL;

    FOREACH(DLRule,r,solvedProgram->rules)
    {
    	if (getFirstRule == 0)
    	{
    		getMatched = INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID));

			DEBUG_LOG("match number:%d", getMatched);
			DEBUG_LOG("rule number:%d", INT_VALUE(getDLProp((DLNode *) r,DL_RULE_ID)));

			adAtom = copyObject(r->head);
			char *adNegHead = CONCAT_STRINGS("R", r->head->rel, "_", "LOST", NON_LINKED_POSTFIX);

			adAtom->rel = CONCAT_STRINGS(strdup(adNegHead));
			adAtom->args = copyObject(r->head->args);

//			unlinkedHeads = appendToTailOfList(unlinkedHeads,copyObject(adAtom));
    	}

    	getFirstRule++;
    }

    // add neg head predicate in the body
//    List *unRules = NIL;
//    DLRule *connRule;

    FOREACH(DLRule,u,unLinkedRules)
	{
		boolean ruleWon = DL_HAS_PROP(u->head,DL_WON)
                                   || DL_HAS_PROP(u->head,DL_UNDER_NEG_WON);

		if (!ruleWon && INT_VALUE(getDLProp((DLNode *) u,DL_RULE_ID)) == getMatched)
//		if (!ruleWon)
		{
//			connRule = copyObject(u);

			setDLProp((DLNode *) adAtom, DL_ORIG_ATOM, (Node *) copyObject(adAtom));
			u->body = appendToTailOfList(u->body, copyObject(adAtom));
//			connRule->body = appendToTailOfList(connRule->body,
//								copyObject(getNthOfListP(unlinkedHeads, INT_VALUE(getDLProp((DLNode *) u,DL_RULE_ID)))));

//			unRules = appendToTailOfList(unRules,copyObject(connRule));
		}
	}
//    unLinkedRules = copyObject(unRules);


	// filter out incorrect linked rules
	List *removeRules = NIL;
	List *nRuleBodyArgs;
	DEBUG_LOG("newRules before removing:\n%s", datalogToOverviewString((Node *) newRules));

	FOREACH(DLRule,nRule,newRules)
	{
		boolean ruleWon = DL_HAS_PROP(nRule->head,DL_WON)
                                   || DL_HAS_PROP(nRule->head,DL_UNDER_NEG_WON);

		if (!ruleWon && INT_VALUE(getDLProp((DLNode *) nRule,DL_RULE_ID)) != getMatched)
		{
			int getFirstGoal = 0;
			FOREACH(DLAtom,r,nRule->body)
			{
				if (getFirstGoal == 0)
				{
					nRuleBodyArgs = NIL;
					for (int i = 0; i < LIST_LENGTH(r->args); i++)
						nRuleBodyArgs = appendToTailOfList(nRuleBodyArgs, getNthOfListP(r->args, i));
				}
				getFirstGoal++;
			}

			if (searchListNode(nRuleBodyArgs, (Node *) createConstBool(FALSE)) || searchListNode(nRuleBodyArgs, (Node *) createConstBool(TRUE)))
				if (INT_VALUE(getDLProp((DLNode *) nRule,DL_RULE_ID)) != getMatched && BOOL_VALUE(getTailOfListP(nRuleBodyArgs)))
					removeRules = appendToTailOfList(removeRules,nRule);
		}
	}
	newRules = removeVars(newRules,removeRules);
	DEBUG_LOG("newRules after removing:\n%s", datalogToOverviewString((Node *) newRules));

	FOREACH(DLRule,r,unLinkedRules)
		setIDBBody(r);

    // remove unRules which have been transformed into newRules
//    FOREACH(DLRule,r,newRules)
//        unLinkedRules = REMOVE_FROM_LIST_PTR(unLinkedRules, r);

    DEBUG_LOG("------------- STEP 3 ---------------\ncreated unlinked rules:\n%s\nand unlinked help rules:\n%s\nand linked rules:\n%s\nand help rules:\n%s\nand EDB help rules:\n%s\nand EDB rules:\n%s",
            datalogToOverviewString((Node *) unLinkedRules),
			datalogToOverviewString((Node *) unLinkedHelpRules),
            datalogToOverviewString((Node *) newRules),
            datalogToOverviewString((Node *) helpRules),
			datalogToOverviewString((Node *) negedbRules),
			datalogToOverviewString((Node *) edbRules));

    /* ************************************************************ */
    // create rules for move relation
    if (streq(fmt,DL_PROV_FORMAT_GP))
    {
        moveRules = createGPMoveRules(getMatched, negedbRules, edbRules,
                unLinkedRules);
    }
    else if (streq(fmt, DL_PROV_FORMAT_TUPLE_ONLY))
    {
        moveRules = createTupleOnlyGraphMoveRules(getMatched, negedbRules, edbRules,
                        unLinkedRules);
    }
    else if (streq(fmt, DL_PROV_FORMAT_TUPLE_RULE_TUPLE))
    {
        moveRules = createTupleRuleTupleGraphMoveRules(getMatched, negedbRules, edbRules,
                        unLinkedRules);
    }
    else if (streq(fmt, DL_PROV_FORMAT_GP_REDUCED))
    {
        moveRules = createGPReducedMoveRules(getMatched, negedbRules, edbRules,
                        unLinkedRules);
    }
    else if (streq(fmt, DL_PROV_FORMAT_HEAD_RULE_EDB))
    {
        moveRules = createHeadRuleEdbGraphMoveRules(getMatched, negedbRules, edbRules,
                        unLinkedRules);
    }
    else if (streq(fmt, DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE))
    {
        moveRules = createTupleRuleGoalTupleGraphMoveRules(getMatched, negedbRules, edbRules,
                        unLinkedRules);
    }
    else if (streq(fmt, DL_PROV_FORMAT_TUPLE_RULE_TUPLE_REDUCED))
	{
		moveRules = createTupleRuleTupleReducedGraphMoveRules(getMatched, negedbRules, edbRules,
						unLinkedRules);
	}

    /* ************************************************************ */
    // mark goals as IDB
    FOREACH(DLRule,r,newRules)
        setIDBBody(r);

    FOREACH(DLRule,r,moveRules)
        setIDBBody(r);

    FOREACH(DLRule,r,helpRules)
        setIDBBody(r);

    FOREACH(DLRule,r,negedbRules)
        setIDBBody(r);


    DEBUG_LOG("------------- STEP 4 ---------------\ncreated unlinked rules:\n%s\nand unlinked help rules:\n%s\nand linked rules:\n%s\nand help rules:\n%s\nand EDB help rules:\n%s\nand EDB rules:\n%s\nand move rules:\n%s",
            datalogToOverviewString((Node *) unLinkedRules),
			datalogToOverviewString((Node *) unLinkedHelpRules),
            datalogToOverviewString((Node *) newRules),
            datalogToOverviewString((Node *) helpRules),
			datalogToOverviewString((Node *) negedbRules),
			datalogToOverviewString((Node *) edbRules),
            datalogToOverviewString((Node *) moveRules));


    /* ************************************************************ */
    // check domain rules are assigned by the user
	List *associateDomainRule;

    if (solvedProgram->doms != NIL)
	{
    	associateDomainRule = NIL;
    	DLRule *newDomRule;

   		List *edbAttr;
   		char *atomRel = NULL;

    	FOREACH(DLRule,r,negedbRules)
		{
    		edbAttr = NIL;
    		DLRule *eachNegedbRule = r;
//    		boolean argConst = FALSE;
			boolean argVar = FALSE;

    		FOREACH(DLAtom,a,r->body)
			{
    			if(a->negated)
    			{
    				atomRel = a->rel;
    				atomRel = replaceSubstr(atomRel, "R", "");
    				atomRel = replaceSubstr(atomRel, "_WON", "");
    				atomRel = replaceSubstr(atomRel, "_nonlinked", "");

    				if(strlen(atomRel) == 0)
    					atomRel = strdup("R");

					edbAttr = getAttributeNames(atomRel);
	    			HashMap *analyzeAtom = NEW_MAP(Constant,List);

					// check if constant exists
					FOREACH(Node,arg,a->args)
					{
//						if(isA(arg,Constant))
//							argConst = TRUE;

						if(isA(arg,DLVar))
							argVar = TRUE;
					}

					int varPosition = 0;
					char *atomAttr = NULL;
//					int numOfAttr = LIST_LENGTH(a->args);
//					int bodyLeng = LIST_LENGTH(eachNegedbRule->body);
					List *domHeadList = NIL;

	    			FOREACH(Node,arg,a->args)
					{
	    				if(argVar)
						{
	    					if (!isA(arg,Constant))
							{
								atomAttr = (char *) getNthOfListP(edbAttr,varPosition);

				    			FOREACH(DLDomain,d,solvedProgram->doms)
				    			{
									if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,atomRel) == 0)
									{
										char *key = (char *) CONCAT_STRINGS(atomAttr,".",atomRel);
										char *value = d->name;
//										char *value = (char *) CONCAT_STRINGS(d->name,"(",((DLVar *) arg)->name,")");
										ADD_TO_MAP(analyzeAtom,createStringKeyValue(key,value));

										DLAtom *domAtom = makeNode(DLAtom);
										domAtom->rel = value;
										domAtom->args = singleton(arg);

										if(!searchListNode(domHeadList, (Node *) domAtom))
											domHeadList = appendToTailOfList(domHeadList, domAtom);
				    				}
				    			}

				    			FOREACH(DLRule,r,domainRules)
								{
				    				FOREACH_HASH(Constant,c,analyzeAtom)
									{
				    					if(strcmp(r->head->rel,STRING_VALUE(c)) == 0)
				    					{
//						    				newDomRule = unifyRule(r,singleton(arg));
				    						newDomRule = r;

			    							if(!searchListNode(associateDomainRule,(Node *) newDomRule))
												associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
				    					}
									}
								}
							}
						}
//	    				else if(argConst && !argVar)
//	    				{
//	    					atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
//
//							FOREACH(DLDomain,d,solvedProgram->doms)
//							{
//								if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,atomRel) == 0)
//								{
//									char *key = (char *) CONCAT_STRINGS(atomAttr,".",atomRel);
//									char *value = d->name;
//									ADD_TO_MAP(analyzeAtom,createStringKeyValue(key,value));
//
//									DLAtom *domAtom = makeNode(DLAtom);
//									domAtom->rel = value;
//									domAtom->args = singleton(arg);
//
////									char *varName = CONCAT_STRINGS("V", itoa(varPosition));
////									DLVar *createVar = createDLVar(varName, DT_STRING);
////									domAtom->args = singleton(createVar);
//
//									eachNegedbRule->body = appendToTailOfList(eachNegedbRule->body, domAtom);
//								}
//							}
//
//							if(associateDomainRule == NIL)
//							{
//				    			FOREACH(DLRule,r,domainRules)
//								{
//				    				FOREACH_HASH(Constant,c,analyzeAtom)
//									{
//				    					if(strcmp(r->head->rel,STRING_VALUE(c)) == 0)
//				    					{
//				    						newDomRule = r;
//
//			    							if(!searchListNode(associateDomainRule,(Node *) newDomRule))
//												associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
//				    					}
//									}
//								}
//							}
//	    				}
	    				varPosition++;
					}
	    			reverseList(domHeadList);
					for(int i = 0; i < LIST_LENGTH(domHeadList); i++)
					{
						DLAtom *domAtom = (DLAtom *) getNthOfListP(domHeadList,i);
						eachNegedbRule->body = appendToHeadOfList(eachNegedbRule->body, domAtom);
					}

//	    			if(mapSize(analyzeAtom) == 0)
//						FATAL_LOG("domain is not defined: %s", CONCAT_STRINGS(atomRel,".",atomAttr));

//	    			FOREACH(Node,arg,a->args)
//					{
//	    				if(argVar)
//						{
//	    					if (!isA(arg,Constant))
//							{
//								char *atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
//	//							ADD_TO_MAP(analyzeAtom,createStringKeyValue(atomAttr,atomRel));
//
//								FOREACH(DLDomain,d,solvedProgram->doms)
//								{
//	//								if(MAP_HAS_STRING_KEY(analyzeAtom,d->attr) &&
//	//										strcmp(exprToSQL(MAP_GET_STRING(analyzeAtom,d->attr)),CONCAT_STRINGS("'",d->rel,"'")) == 0)
//									if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,atomRel) == 0)
//									{
//										FOREACH(DLRule,dr,domainRules)
//										{
//											char *dHead = dr->head->rel;
//
//											if(strcmp(dHead,d->name) == 0)
//											{
//												newDomRule = dr;
//												List *headArgs = newDomRule->head->args;
//												newDomRule->head->args = singleton(arg);
//
//												FOREACH(DLVar,h,headArgs)
//													FOREACH(DLAtom,b,newDomRule->body)
//														FOREACH(DLVar,ba,b->args)
//															if(strcmp(h->name,ba->name) == 0)
//																ba->name = ((DLVar *) arg)->name;
//
//												if(!searchListNode(eachNegedbRule->body,(Node *) newDomRule->head))
//													eachNegedbRule->body = appendToTailOfList(eachNegedbRule->body, newDomRule->head);
//
//												if(!searchListNode(associateDomainRule,(Node *) newDomRule))
//													associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
//											}
//										}
//									}
//								}
//							}
//						}
//	    				else if(argConst && !argVar)
//	    				{
//	    					char *atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
////							ADD_TO_MAP(analyzeAtom,createStringKeyValue(atomAttr,atomRel));
//
//							FOREACH(DLDomain,d,solvedProgram->doms)
//							{
////								if(MAP_HAS_STRING_KEY(analyzeAtom,d->attr) &&
////										strcmp(exprToSQL(MAP_GET_STRING(analyzeAtom,d->attr)),CONCAT_STRINGS("'",d->rel,"'")) == 0)
//								if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,atomRel) == 0)
//								{
//									FOREACH(DLRule,dr,domainRules)
//									{
//										char *dHead = dr->head->rel;
//
//										if(strcmp(dHead,d->name) == 0 && LIST_LENGTH(eachNegedbRule->body) < bodyLeng + numOfAttr)
//										{
//											newDomRule = dr;
//											eachNegedbRule->body = appendToTailOfList(eachNegedbRule->body, newDomRule->head);
//
//											if(!searchListNode(associateDomainRule,(Node *) newDomRule))
//												associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
//										}
//									}
//								}
//							}
//	    				}
//						varPosition++;
//					}

    			}
			}

    		r = eachNegedbRule;
		}


    	FOREACH(DLRule,h,helpRules)
		{
			DLRule *eachNegheadRule = h;
//			boolean argConst = FALSE;
			boolean argVar = FALSE;

			FOREACH(DLAtom,a,h->body)
			{
				if(a->negated)
				{
					char *bodyAtomRel = a->rel;
					bodyAtomRel = replaceSubstr(bodyAtomRel, "R", "");
					bodyAtomRel = replaceSubstr(bodyAtomRel, "_WON", "");
					bodyAtomRel = replaceSubstr(bodyAtomRel, "_nonlinked", "");

	    			HashMap *analyzeAtom = NEW_MAP(Constant,List);

					// check if constant exists
					FOREACH(Node,arg,a->args)
					{
//						if(isA(arg,Constant))
//							argConst = TRUE;

						if(isA(arg,DLVar))
							argVar = TRUE;
					}

					int varPosition = 0;
					char *atomAttr = NULL;
					List *domHeadList = NIL;

					FOREACH(Node,arg,a->args)
					{
						if(argVar)
						{
							FOREACH(DLRule,h,origProg)
								if(strcmp(h->head->rel,bodyAtomRel) == 0)
									FOREACH(DLAtom,a,h->body)
										FOREACH(DLVar,v,h->head->args)
											if(!isA(v,Constant) && searchListNode(a->args,(Node *) v))
												bodyAtomRel = a->rel;

							if (!isA(arg,Constant))
							{
								atomAttr = (char *) getNthOfListP(edbAttr,varPosition);

								FOREACH(DLDomain,d,solvedProgram->doms)
								{
									if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,bodyAtomRel) == 0)
									{
										char *key = (char *) CONCAT_STRINGS(atomAttr,".",bodyAtomRel);
										char *value = d->name;
//										char *value = (char *) CONCAT_STRINGS(d->name,"(",((DLVar *) arg)->name,")");
										ADD_TO_MAP(analyzeAtom,createStringKeyValue(key,value));

										DLAtom *domAtom = makeNode(DLAtom);
										domAtom->rel = value;
										domAtom->args = singleton(arg);

										if(!searchListNode(domHeadList, (Node *) domAtom))
											domHeadList = appendToTailOfList(domHeadList, domAtom);
									}
								}

								FOREACH(DLRule,r,domainRules)
								{
									FOREACH_HASH(Constant,c,analyzeAtom)
									{
										if(strcmp(r->head->rel,STRING_VALUE(c)) == 0)
										{
//											newDomRule = unifyRule(r,singleton(arg));
											newDomRule = r;

											if(!searchListNode(associateDomainRule,(Node *) newDomRule))
												associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
										}
									}
								}
							}
						}
//						else if(argConst && !argVar)
//						{
//							FOREACH(DLRule,h,origProg)
//								if(strcmp(h->head->rel,bodyAtomRel) == 0)
//									FOREACH(DLAtom,a,h->body)
//	//										if(DL_HAS_PROP(a,DL_IS_EDB_REL))
//											bodyAtomRel = a->rel;
//
////							if(strcmp(bodyAtomRel,atomRel) == 0)
////							{
////								eachNegheadRule->body = appendToTailOfList(eachNegheadRule->body,newDomRule->head);
////
////								if(!searchListNode(associateDomainRule,(Node *) newDomRule))
////									associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
////							}
//
//							atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
//
//							int i = 0;
//							FOREACH(DLDomain,d,solvedProgram->doms)
//							{
//								if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,bodyAtomRel) == 0)
//								{
//									char *key = (char *) CONCAT_STRINGS(atomAttr,".",bodyAtomRel);
//									char *value = d->name;
//									ADD_TO_MAP(analyzeAtom,createStringKeyValue(key,value));
//
//									DLAtom *domAtom = makeNode(DLAtom);
//									domAtom->rel = value;
//									domAtom->args = singleton(arg);
//
////									char *varName = CONCAT_STRINGS("V", itoa(varPosition));
////									DLVar *createVar = createDLVar(varName, DT_STRING);
////									domAtom->args = singleton(createVar);
//
//									eachNegheadRule->body = appendToTailOfList(eachNegheadRule->body, domAtom);
//								}
//								i++;
//							}
//
//							if(associateDomainRule == NIL)
//							{
//								FOREACH(DLRule,r,domainRules)
//								{
//									FOREACH_HASH(Constant,c,analyzeAtom)
//									{
//										if(strcmp(r->head->rel,STRING_VALUE(c)) == 0)
//										{
//											newDomRule = r;
//
//											if(!searchListNode(associateDomainRule,(Node *) newDomRule))
//												associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
//										}
//									}
//								}
//							}
//						}
						varPosition++;
					}

					reverseList(domHeadList);
					for(int i = 0; i < LIST_LENGTH(domHeadList); i++)
					{
						DLAtom *domAtom = (DLAtom *) getNthOfListP(domHeadList,i);
						eachNegheadRule->body = appendToHeadOfList(eachNegheadRule->body, domAtom);
					}

//	    			if(mapSize(analyzeAtom) == 0)
//						FATAL_LOG("domain is not defined: %s", CONCAT_STRINGS(atomRel,".",atomAttr));

//					FOREACH(Node,arg,a->args)
//					{
//						if(argVar)
//						{
//							FOREACH(DLRule,h,origProg)
//								if(strcmp(h->head->rel,bodyAtomRel) == 0)
//									FOREACH(DLAtom,a,h->body)
//										FOREACH(DLVar,v,h->head->args)
//											if(!isA(v,Constant) && searchListNode(a->args,(Node *) v))
//												bodyAtomRel = a->rel;
//
//							if (!isA(arg,Constant))
//							{
//								char *atomAttr = (char *) getNthOfListP(edbAttr,varPosition);
//	//							ADD_TO_MAP(analyzeAtom,createStringKeyValue(atomAttr,atomRel));
//
//								FOREACH(DLDomain,d,solvedProgram->doms)
//								{
//	//								if(MAP_HAS_STRING_KEY(analyzeAtom,d->attr) &&
//	//										strcmp(exprToSQL(MAP_GET_STRING(analyzeAtom,d->attr)),CONCAT_STRINGS("'",d->rel,"'")) == 0)
//									if(strcmp(d->attr,atomAttr) == 0 && strcmp(d->rel,bodyAtomRel) == 0)
//									{
//										FOREACH(DLRule,dr,domainRules)
//										{
//											char *dHead = dr->head->rel;
//
//											if(strcmp(dHead,d->name) == 0)
//											{
//												DLRule *newDrule = dr;
//												List *headArgs = newDrule->head->args;
//												newDrule->head->args = singleton(arg);
//
//												FOREACH(DLVar,h,headArgs)
//													FOREACH(DLAtom,b,newDrule->body)
//														FOREACH(DLVar,ba,b->args)
//															if(strcmp(h->name,ba->name) == 0)
//																ba->name = ((DLVar *) arg)->name;
//
//												if(!searchListNode(eachNegheadRule->body,(Node *) newDrule->head))
//													eachNegheadRule->body = appendToTailOfList(eachNegheadRule->body,newDrule->head);
//
//												if(!searchListNode(associateDomainRule,(Node *) newDrule))
//													associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDrule);
//											}
//										}
//									}
//								}
//							}
//						}
//						else if(argConst && !argVar)
//						{
//							FOREACH(DLRule,h,origProg)
//								if(strcmp(h->head->rel,bodyAtomRel) == 0)
//									FOREACH(DLAtom,a,h->body)
////										if(DL_HAS_PROP(a,DL_IS_EDB_REL))
//											bodyAtomRel = a->rel;
//
//							if(strcmp(bodyAtomRel,atomRel) == 0)
//							{
//								eachNegheadRule->body = appendToTailOfList(eachNegheadRule->body,newDomRule->head);
//
//								if(!searchListNode(associateDomainRule,(Node *) newDomRule))
//									associateDomainRule = appendToTailOfList(associateDomainRule,(List *) newDomRule);
//							}
//						}
//						varPosition++;
//					}

				}
			}

			h = eachNegheadRule;
		}

    	FOREACH(DLRule,r,negedbRules)
    		setIDBBody(r);

    	FOREACH(DLRule,r,helpRules)
    		setIDBBody(r);


    	DEBUG_LOG("------------- STEP 5 ---------------\ncreated unlinked rules:\n%s\nand unlinked help rules:\n%s\nand linked rules:\n%s\nand help rules:\n%s\nand EDB help rules:\n%s\nand EDB rules:\n%s\nand move rules:\n%s\nand domain rules:\n%s",
    	            datalogToOverviewString((Node *) unLinkedRules),
    				datalogToOverviewString((Node *) unLinkedHelpRules),
    	            datalogToOverviewString((Node *) newRules),
    	            datalogToOverviewString((Node *) helpRules),
    				datalogToOverviewString((Node *) negedbRules),
    				datalogToOverviewString((Node *) edbRules),
    	            datalogToOverviewString((Node *) moveRules),
					datalogToOverviewString((Node *) associateDomainRule));
	}

    solvedProgram->ans = "move";

    boolean ruleWon = TRUE;
    FOREACH(DLRule,r,solvedProgram->rules)
    {
    	ruleWon = ruleWon && (DL_HAS_PROP(r,DL_WON)
    	                           || DL_HAS_PROP(r,DL_UNDER_NEG_WON));
    }

    if (solvedProgram->doms == NIL)
    {
        if (ruleWon)
            solvedProgram->rules = CONCAT_LISTS(moveRules, edbRules, helpRules, unLinkedRules, newRules);
        else
        	solvedProgram->rules = CONCAT_LISTS(moveRules, negedbRules, edbRules, helpRules, unLinkedRules, unLinkedHelpRules, newRules);
    }
    else
    {
        if (ruleWon)
            solvedProgram->rules = CONCAT_LISTS(associateDomainRule, moveRules, edbRules, helpRules, unLinkedRules, newRules);
        else
        	solvedProgram->rules = CONCAT_LISTS(associateDomainRule, moveRules, negedbRules, edbRules, helpRules, unLinkedRules, unLinkedHelpRules, newRules);
    }


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

boolean
searchVars (List *vars, List *searVars)
{
    Set *sVars = NODESET();
    List *result = NIL;

    FOREACH(Node,r,vars)
        addToSet(sVars,r);

    FOREACH(Node,v,searVars)
        if(hasSetElem(sVars,v))
            result = appendToTailOfList(result, v);

    if (LIST_LENGTH(result) == 0)
    	return FALSE;
    else
    	return TRUE;
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
        case GP_NODE_TUPLE:
			concatArgs = appendToTailOfList(concatArgs,
						createConstString("TUPLE_"));
			break;
        case GP_NODE_RULEHYPER:
            concatArgs = appendToTailOfList(concatArgs,
                    createConstString("RULEHYPEREDGE_"));
            break;
        case GP_NODE_GOALHYPER:
                    concatArgs = appendToTailOfList(concatArgs,
                            createConstString("GOALHYPEREDGE_"));
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

//   	INFO_DL_LOG("enumerated program:", p);
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

    DEBUG_NODE_BEATIFY_LOG("predToUnRules:", predToUnRules);

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
    newP->doms = p->doms;
    newP->ans = strdup(p->ans);
    newP->facts = p->facts;
    newP->rules = newRules;
    newP->n.properties = copyObject(p->n.properties);

    setDLProp((DLNode *) newP, DL_MAP_RELNAME_TO_RULES, (Node *) newPredToRules);
    setDLProp((DLNode *) newP, DL_MAP_UN_PREDS_TO_RULES, (Node *) predToUnRules);

    DEBUG_NODE_BEATIFY_LOG("unified program is:", newP);
    INFO_DL_LOG("unified program is:", newP);

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

    if(LIST_LENGTH(domainRules) != 0)
    	DEBUG_LOG("Associated Domain:\n%s", datalogToOverviewString((Node *) domainRules));

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

        // recursively set WON/LOST status for all
        while(!LIST_EMPTY(todoStack))
        {
            DLRule *r = (DLRule *) popHeadOfListP(todoStack);
            boolean ruleWon = DL_HAS_PROP(r,DL_WON)
                    				|| DL_HAS_PROP(r,DL_UNDER_NEG_WON);
//            boolean ruleNeg = DL_HAS_PROP(r,DL_UNDER_NEG_WON)
//                    				|| DL_HAS_PROP(r,DL_UNDER_NEG_LOST);

            DEBUG_LOG("process rule:\n%s",
                    datalogToOverviewString((Node *) r));

            // process each atom
            FOREACH(DLAtom,a,r->body)
            {
                DLAtom *adHead;
                boolean newWon = ruleWon && !a->negated;
                char *newProp = newWon ? DL_WON : DL_LOST;

//                boolean newNeg = ruleNeg || !ruleWon;
//                char *newProp = newNeg ?
//                        (newWon ? DL_UNDER_NEG_WON : DL_UNDER_NEG_LOST)
//                        : (newWon ? DL_WON : DL_LOST);

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

    DEBUG_NODE_BEATIFY_LOG("adorned (solved) program is:", p);
    INFO_DL_LOG("adorned (solved) program is:", p);
    DEBUG_LOG("Associated Domain:\n%s", datalogToOverviewString((Node *) domainRules));

    return p;
}
