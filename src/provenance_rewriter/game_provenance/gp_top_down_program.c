/*-----------------------------------------------------------------------------
 *
 * gp_top_down_program.c
 *			Create a datalog program to construct the GP top-down
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "model/datalog/datalog_model.h"
#include "provenance_rewriter/game_provenance/gp_top_down_program.h"

static DLProgram *createWhyGPprogram (DLProgram *p, DLAtom *why);
static DLProgram *createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot);
static DLProgram *createFullGPprogram (DLProgram *p);
static DLProgram *unifyProgram (DLProgram *p, DLAtom *question);
static DLProgram *solveProgram (DLProgram *p, DLAtom *question);

DLProgram *
createTopDownGPprogram (DLProgram *p)
{
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

    solvedProgram = copyObject(p);
    unifyProgram(solvedProgram, why);
    solveProgram(solvedProgram, why);

    return p;
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
 *
 */

static DLProgram *
unifyProgram (DLProgram *p, DLAtom *question)
{
//    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
//    List *newRules;

    return p;
}

/*
 * Determine which nodes in the game template (annotated program) will
 * be lost and which ones will be won based on the use query.
 */

static DLProgram *
solveProgram (DLProgram *p, DLAtom *question)
{
//    HashMap *predToRules = (HashMap *) getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
//    char *targetPred;


    return p;
}
