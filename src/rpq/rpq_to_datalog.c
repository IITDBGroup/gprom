/*-----------------------------------------------------------------------------
 *
 * rpq_to_datalog.c:
 *
 *		This modules translates regular path queries into datalog queries over
 *		an edge relation.
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/datalog/datalog_model.h"
#include "model/rpq/rpq_model.h"
#include "rpq/rpq_to_datalog.h"

static void rpqTranslate (Regex *rpq, HashMap *subexToPred);
static void translateLabel(Regex *rpq, HashMap *subexToPred);
static void translateOptional(Regex *rpq, HashMap *subexToPred);
static void translatePlus(Regex *rpq, HashMap *subexToPred);
static void translateStar(Regex *rpq, HashMap *subexToPred);
static void translateOr(Regex *rpq, HashMap *subexToPred);
static void translateConc(Regex *rpq, HashMap *subexToPred);

Node *
rpqToDatalog(Regex *rpq)
{
    HashMap *subexToPred = NEW_MAP(Constant,DLNode);
    DLProgram *dl;
    List *rules = NIL;

    rpqTranslate(rpq, subexToPred);

    /* create program */
    FOREACH_HASH(List,subRules,subexToPred)
        FOREACH(DLRule,r,subRules)
            rules = appendToTailOfList(rules,r);

    // add rule for result generation


    dl = createDLProgram(rules, NIL, "result");

    return (Node *) dl;
}

static void
rpqTranslate (Regex *rpq, HashMap *subexToPred)
{
    char *rpqSubex = rpqToShortString(rpq);

    if (MAP_HAS_STRING_KEY(subexToPred, rpqSubex))
        return;

    switch(rpq->opType)
    {
        case REGEX_LABEL:
            translateLabel(rpq,subexToPred);
        break;
        case REGEX_OR:
            translateOr(rpq,subexToPred);
        break;
        case REGEX_PLUS:
            translatePlus(rpq,subexToPred);
        break;
        case REGEX_STAR:
            translateStar(rpq,subexToPred);
        break;
        case REGEX_CONC:
            translateConc(rpq,subexToPred);
        break;
        case REGEX_OPTIONAL:
            translateOptional(rpq,subexToPred);
        break;

    }
}

static void
translateLabel(Regex *rpq, HashMap *subexToPred)
{
    char *rpqSubex = rpqToShortString(rpq);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;

    r = createDLRule();

    // add rule
    MAP_ADD_STRING(subexToPred,rpqSubex,singleton(r));
}

static void
translateOptional(Regex *rpq, HashMap *subexToPred)
{

}

static void
translatePlus(Regex *rpq, HashMap *subexToPred)
{

}

static void
translateStar(Regex *rpq, HashMap *subexToPred)
{

}

static void
translateOr(Regex *rpq, HashMap *subexToPred)
{

}

static void
translateConc(Regex *rpq, HashMap *subexToPred)
{

}
