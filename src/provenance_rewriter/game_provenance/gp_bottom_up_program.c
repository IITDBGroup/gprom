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

#include "provenance_rewriter/game_provenance/gp_bottom_up_program.h"

static DLProgram *createWhyGPprogram (DLProgram *p, DLAtom *why);
static DLProgram *createWhyNotGPprogram (DLProgram *p, DLAtom *whyNot);
static DLProgram *createFullGPprogram (DLProgram *p);
static DLProgram *unifyProgram (DLProgram *p, DLAtom *question);
static DLProgram *solveProgram (DLProgram *p, DLAtom *question);

DLProgram *
createGPBottomUpprogram (DLProgram *p)
{
    //TODO switch provenance types
    return p;
}

static DLProgram *
createWhyGPprogram (DLProgram *p, DLAtom *why)
{
    DLProgram *solvedProgram;
    DLProgram *result;

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
    HashMap *predToRules = getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    List *newRules;


}

/*
 * Determine which nodes in the game template (annotated program) will
 * be lost and which ones will be won based on the use query.
 */

static DLProgram *
solveProgram (DLProgram *p, DLAtom *question)
{
    HashMap *predToRules = getDLProp((DLNode *) p, DL_MAP_RELNAME_TO_RULES);
    char *targetPred;



}
