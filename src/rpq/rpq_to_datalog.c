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

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/datalog/datalog_model.h"
#include "model/rpq/rpq_model.h"
#include "rpq/rpq_to_datalog.h"

#define EDGE_REL_NAME "e"
#define NODE_REL_NAME "node"
#define MATCH_PRED "match"
#define RESULT_PRED "result"

#define MATCH_REL(rpq) (CONCAT_STRINGS(MATCH_PRED,"_",rpqToReversePolish(rpq)))
#define CHILD_MATCH_REL(rpq,pos) CONCAT_STRINGS(MATCH_PRED,"_", \
		    rpqToShortString(getNthOfListP(rpq->children,(pos))))
#define GET_MATCH_REL(rpq) (STRING_VALUE(MAP_GET_STRING(subexToPred,rpqToReversePolish(rpq))))
#define CHILD_GET_MATCH_REL(rpq,pos) (STRING_VALUE(MAP_GET_STRING(subexToPred,rpqToReversePolish(getNthOfListP(rpq->children,(pos))))))

static void rpqTranslate (Regex *rpq, HashMap *subexToRules, HashMap *subexToPred, Set *usedNames);
static void translateLabel(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred);
static void translateOptional(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred);
static void translatePlus(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred);
static void translateStar(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred);
static void translateOr(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred);
static void translateConc(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred);

static List *addNodeRules (List *rules);
static List *addResultRules (List *rules, char *rpqName);
static char *replaceCharsForPred(char *in);

Node *
rpqToDatalog(Regex *rpq)
{
    HashMap *subexToRules = NEW_MAP(Constant,List);
    HashMap *subexToPred = NEW_MAP(Constant,Constant);
    Set *usedNames = STRSET();
    DLProgram *dl;
    List *rules = NIL;

    INFO_LOG("translate %s", rpqToShortString(rpq));

    rpqTranslate(rpq, subexToRules, subexToPred, usedNames);

    /* create program */
    FOREACH_HASH(List,subRules,subexToRules)
        FOREACH(DLRule,r,subRules)
            rules = appendToTailOfList(rules,r);

    // add node rules
    rules = addNodeRules(rules);

    // add rule for result generation
    rules = addResultRules(rules, GET_MATCH_REL(rpq));

    dl = createDLProgram(rules, NIL, RESULT_PRED);

    return (Node *) dl;
}

static List *
addResultRules (List *rules, char *rpqName)
{
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;

    head = createDLAtom(strdup(RESULT_PRED),
            LIST_MAKE(createDLVar("X",DT_STRING),
            createDLVar("Y",DT_STRING),
            createDLVar("U",DT_STRING),
            createDLVar("L",DT_STRING),
            createDLVar("V",DT_STRING)),
            FALSE);
    edge = createDLAtom(strdup(rpqName),
            LIST_MAKE(createDLVar("X",DT_STRING),
            createDLVar("Y",DT_STRING),
            createDLVar("U",DT_STRING),
            createDLVar("L",DT_STRING),
            createDLVar("V",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules, r);

    return rules;
}

static List *
addNodeRules (List *rules)
{
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;

    head = createDLAtom(strdup(NODE_REL_NAME),
            singleton(createDLVar("X",DT_STRING)),
            FALSE);
    edge = createDLAtom(strdup(EDGE_REL_NAME),
            LIST_MAKE(createDLVar("X",DT_STRING),
            createDLVar("Y",DT_STRING),
            createDLVar("Z",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules, r);

    head = createDLAtom(strdup(NODE_REL_NAME),
            LIST_MAKE(createDLVar("X",DT_STRING)),
            FALSE);
    edge = createDLAtom(strdup(EDGE_REL_NAME),
            LIST_MAKE(createDLVar("Z",DT_STRING),
            createDLVar("Y",DT_STRING),
            createDLVar("X",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules, r);

    return rules;
}


static void
rpqTranslate (Regex *rpq, HashMap *subexToRules, HashMap *subexToPred, Set *usedNames)
{
    char *rpqSubex = rpqToReversePolish(rpq);
    StringInfo predName = makeStringInfo();
    char *origName = MATCH_REL(rpq);

    if (MAP_HAS_STRING_KEY(subexToPred, rpqSubex))
        return;

    origName = replaceCharsForPred(origName);

    appendStringInfoString(predName, origName);
    //appendStringInfoString(predName, origName);
    while(hasSetElem(usedNames,predName->data))
        appendStringInfoString(predName,"1");

    addToSet(usedNames,strdup(predName->data));
    MAP_ADD_STRING_KEY(subexToPred,rpqSubex,createConstString(strdup(predName->data)));
    DEBUG_LOG("pred name for %s is %s wiht orig %s", rpqToShortString(rpq), strdup(predName->data), origName);

    FOREACH(Regex,child,rpq->children)
        rpqTranslate(child, subexToRules, subexToPred, usedNames);

    switch(rpq->opType)
    {
        case REGEX_LABEL:
            translateLabel(rpq,subexToRules, subexToPred);
        break;
        case REGEX_OR:
            translateOr(rpq,subexToRules, subexToPred);
        break;
        case REGEX_PLUS:
            translatePlus(rpq,subexToRules, subexToPred);
        break;
        case REGEX_STAR:
            translateStar(rpq,subexToRules, subexToPred);
        break;
        case REGEX_CONC:
            translateConc(rpq,subexToRules, subexToPred);
        break;
        case REGEX_OPTIONAL:
            translateOptional(rpq,subexToRules, subexToPred);
        break;
    }
}

/**
 * for regex "l" return rule match_l
 */
static void
translateLabel(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;

    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("X",DT_STRING),
                    createConstString(rpq->label),
                    createDLVar("Y",DT_STRING)),
                    FALSE);
    edge = createDLAtom(strdup(EDGE_REL_NAME),
            LIST_MAKE(createDLVar("X",DT_STRING),
            createConstString(rpq->label),
            createDLVar("Y",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,singleton(r));
}

static void
translateOptional(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *childRel = CHILD_GET_MATCH_REL(rpq,0);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;
    List *rules = NIL;

    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("X",DT_STRING),
                    createNullConst(DT_STRING),
                    createNullConst(DT_STRING),
                    createNullConst(DT_STRING)),
                    FALSE);
    edge = createDLAtom(strdup(NODE_REL_NAME),
            singleton(createDLVar("X",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("Z",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("U",DT_STRING)),
                    FALSE);
    edge = createDLAtom(childRel,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("Z",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("U",DT_STRING)),
                    FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(subexToRules,relName,rules);
}

static void
translatePlus(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *childRel = CHILD_GET_MATCH_REL(rpq,0);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge1;
    DLAtom *edge2;
    List *rules = NIL;

    // match_A+(X,Y,A,L,B) :- match_A+(X,Y,A,L,B)
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    edge1 = createDLAtom(childRel,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    r = createDLRule(head, singleton(edge1));
    rules = appendToTailOfList(rules,r);

    // create a join between A+ and A and propagate both their path edges = 2 options
    edge1 = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Z",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L1",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    edge2 = createDLAtom(childRel,
            LIST_MAKE(createDLVar("Z",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("C",DT_STRING),
                    createDLVar("L2",DT_STRING),
                    createDLVar("D",DT_STRING)),
                    FALSE);
    // match_A+(X,Y,A,L1,B) :- match_A+(X,Z,A,L1,B), match_A(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L1",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    r = createDLRule(head, LIST_MAKE(edge1,edge2));
    rules = appendToTailOfList(rules,r);

    // match_A+(X,Y,C,L2,D) :- match_A+(X,Z,A,L1,B), match_A(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("C",DT_STRING),
                    createDLVar("L2",DT_STRING),
                    createDLVar("D",DT_STRING)),
                    FALSE);
    edge1 = copyObject(edge1);
    edge2 = copyObject(edge2);
    r = createDLRule(head, LIST_MAKE(edge1,edge2));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(subexToRules,relName,rules);
}

static void
translateStar(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *childRel = CHILD_GET_MATCH_REL(rpq,0);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge1;
    DLAtom *edge2;
    List *rules = NIL;

    // match_A*(X,Y,null,null,null) :- node(X)
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("X",DT_STRING),
                    createNullConst(DT_STRING),
                    createNullConst(DT_STRING),
                    createNullConst(DT_STRING)),
                    FALSE);
    edge1 = createDLAtom(strdup(NODE_REL_NAME),
            singleton(createDLVar("X",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge1));
    rules = appendToTailOfList(rules,r);

    // create a join between A* and A and propagate both their path edges = 2 options
    edge1 = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Z",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L1",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    edge2 = createDLAtom(childRel,
            LIST_MAKE(createDLVar("Z",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("C",DT_STRING),
                    createDLVar("L2",DT_STRING),
                    createDLVar("D",DT_STRING)),
                    FALSE);
    // match_A*(X,Y,A,L1,B) :- match_A*(X,Z,A,L1,B), match_A(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L1",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    r = createDLRule(head, LIST_MAKE(edge1,edge2));
    rules = appendToTailOfList(rules,r);

    // match_A*(X,Y,C,L2,D) :- match_A*(X,Z,A,L1,B), match_A(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("C",DT_STRING),
                    createDLVar("L2",DT_STRING),
                    createDLVar("D",DT_STRING)),
                    FALSE);
    edge1 = copyObject(edge1);
    edge2 = copyObject(edge2);
    r = createDLRule(head, LIST_MAKE(edge1,edge2));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(subexToRules,relName,rules);
}

static void
translateOr(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);
    Regex *rChild = getNthOfListP(rpq->children,1);
    char *rChildRel = GET_MATCH_REL(rChild);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;
    List *rules = NIL;

    // match_A|B(X,Y,A,L1,B) :- match_A(X,Y,A,L1,B).s
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    edge = createDLAtom(lChildRel,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // match_A.B(X,Y,C,L2,D) :- match_B(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    edge = createDLAtom(rChildRel,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(subexToRules,relName,rules);
}

static void
translateConc(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);
    Regex *rChild = getNthOfListP(rpq->children,1);
    char *rChildRel = GET_MATCH_REL(rChild);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge1;
    DLAtom *edge2;
    List *rules = NIL;

    // create a join between A . B
    edge1 = createDLAtom(lChildRel,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Z",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L1",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    edge2 = createDLAtom(rChildRel,
            LIST_MAKE(createDLVar("Z",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("C",DT_STRING),
                    createDLVar("L2",DT_STRING),
                    createDLVar("D",DT_STRING)),
                    FALSE);

    // match_A.B(X,Y,A,L1,B) :- match_A(X,Z,A,L1,B), match_B(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("A",DT_STRING),
                    createDLVar("L1",DT_STRING),
                    createDLVar("B",DT_STRING)),
                    FALSE);
    r = createDLRule(head, LIST_MAKE(edge1,edge2));
    rules = appendToTailOfList(rules,r);

    // match_A.B(X,Y,C,L2,D) :- match_A(X,Z,A,L1,B), match_B(Z,Y,C,L2,D).
    head = createDLAtom(relName,
            LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING),
                    createDLVar("C",DT_STRING),
                    createDLVar("L2",DT_STRING),
                    createDLVar("D",DT_STRING)),
                    FALSE);
    edge1 = copyObject(edge1);
    edge2 = copyObject(edge2);
    r = createDLRule(head, LIST_MAKE(edge1,edge2));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(subexToRules,relName,rules);
}

static char *
replaceCharsForPred(char *in)
{
    char *res = in;
    while(*in++ != '\0')
    {
        if (*in == '(')
            *in = '_';
        else if (*in == ')')
            *in = '_';
        else if (*in == '|')
            *in = '_';
        else if (*in == '*')
            *in = '_';
        else if (*in == '+')
            *in = '_';
        else if (*in == '?')
            *in = '_';
        else if (*in == '.')
            *in = '_';
    }

    return res;
}
