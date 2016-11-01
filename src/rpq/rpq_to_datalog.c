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

//#define EDGE_REL_NAME "e"
#define NODE_REL_NAME "node_for_rpq"
#define MATCH_PRED "match"
//#define RESULT_PRED "result"

#define MATCH_REL(rpq) (CONCAT_STRINGS(MATCH_PRED,"_",rpqToReversePolish(rpq)))
#define CHILD_MATCH_REL(rpq,pos) CONCAT_STRINGS(MATCH_PRED,"_", \
		    rpqToShortString(getNthOfListP(rpq->children,(pos))))
#define GET_MATCH_REL(rpq) (STRING_VALUE(MAP_GET_STRING((c->subexToPred),rpqToReversePolish(rpq))))
#define CHILD_GET_MATCH_REL(rpq,pos) (STRING_VALUE(MAP_GET_STRING(c->subexToPred,rpqToReversePolish(getNthOfListP(rpq->children,(pos))))))



typedef struct RpqToDatalogContext {
    HashMap *subexToRules;
    HashMap *subexToPred;
    Set *usedNames;
    RPQQueryType queryType;
    char *edgeRel;
    char *resultRel;
} RpqToDatalogContext;

#define MAKE_RPQ_CONTEXT(varname) \
    do { \
        varname = NEW(RpqToDatalogContext); \
        varname->subexToRules =  NEW_MAP(Constant,List); \
        varname->subexToPred = NEW_MAP(Constant,Constant); \
        varname->usedNames = STRSET(); \
    } while(0)


static void rpqTranslate (Regex *rpq, RpqToDatalogContext *c);
static void translateLabel(Regex *rpq, RpqToDatalogContext *c);
static void translateOptional(Regex *rpq, RpqToDatalogContext *c);
static void translatePlus(Regex *rpq, RpqToDatalogContext *c);
static void translateStar(Regex *rpq, RpqToDatalogContext *c);
static void translateOr(Regex *rpq, RpqToDatalogContext *c);
static void translateConc(Regex *rpq, RpqToDatalogContext *c);

static List *addNodeRules (List *rules, RpqToDatalogContext *c);
static List *addResultRules (List *rules, char *rpqName, RpqToDatalogContext *c);
static char *replaceCharsForPred(char *in);

Node *
rpqQueryToDatalog(RPQQuery *q)
{
    return rpqToDatalog(q->q, q->t, q->edgeRel, q->resultRel);
}

Node *
rpqToDatalog(Regex *rpq, RPQQueryType type, char *edgeRel, char *outRel)
{
    RpqToDatalogContext *c;
    DLProgram *dl;
    List *rules = NIL;

    MAKE_RPQ_CONTEXT(c);
    c->queryType = type;
    c->edgeRel = edgeRel;
    c->resultRel = outRel;

    INFO_LOG("translate %s", rpqToShortString(rpq));

    rpqTranslate(rpq, c);

    /* create program */
    FOREACH_HASH(List,subRules,c->subexToRules)
        FOREACH(DLRule,r,subRules)
            rules = appendToTailOfList(rules,r);

    // add node rules
    rules = addNodeRules(rules, c);

    // add rule for result generation
    rules = addResultRules(rules, GET_MATCH_REL(rpq), c);

    dl = createDLProgram(rules, NIL, strdup(c->resultRel), NULL);

    return (Node *) dl;
}

static List *
addResultRules (List *rules, char *rpqName, RpqToDatalogContext *c)
{
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;
    DLComparison *noNull;

    switch(c->queryType)
    {
        case RPQ_QUERY_RESULT:
        {
            head = createDLAtom(strdup(c->resultRel),
                    LIST_MAKE(
                    createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING)),
                    FALSE);
            edge = createDLAtom(strdup(rpqName),
                    LIST_MAKE(createDLVar("X",DT_STRING),
                    createDLVar("Y",DT_STRING)),
                    FALSE);
            r = createDLRule(head, singleton(edge));
            rules = appendToTailOfList(rules, r);
        }
        break;
        case RPQ_QUERY_SUBGRAPH:
        {
            head = createDLAtom(strdup(c->resultRel),
                    LIST_MAKE(
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
            noNull = createDLComparison("!=",
                    (Node *) createDLVar("L", DT_STRING),
                    (Node *) createNullConst(DT_STRING));
            r = createDLRule(head, LIST_MAKE(edge,noNull));
            rules = appendToTailOfList(rules, r);
        }
        break;
        case RPQ_QUERY_PROV:
        {
            head = createDLAtom(strdup(c->resultRel),
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
            noNull = createDLComparison("!=",
                    (Node *) createDLVar("L", DT_STRING),
                    (Node *) createNullConst(DT_STRING));
            r = createDLRule(head, LIST_MAKE(edge,noNull));
            rules = appendToTailOfList(rules, r);
        }
        break;
    }

    return rules;
}

static List *
addNodeRules (List *rules, RpqToDatalogContext *c)
{
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;

    head = createDLAtom(strdup(NODE_REL_NAME),
            singleton(createDLVar("X",DT_STRING)),
            FALSE);
    edge = createDLAtom(strdup(c->edgeRel),
            LIST_MAKE(createDLVar("X",DT_STRING),
            createDLVar("Y",DT_STRING),
            createDLVar("Z",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules, r);

    head = createDLAtom(strdup(NODE_REL_NAME),
            LIST_MAKE(createDLVar("X",DT_STRING)),
            FALSE);
    edge = createDLAtom(strdup(c->edgeRel),
            LIST_MAKE(createDLVar("Z",DT_STRING),
            createDLVar("Y",DT_STRING),
            createDLVar("X",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules, r);

    return rules;
}


static void
rpqTranslate (Regex *rpq, RpqToDatalogContext *c)
{
    char *rpqSubex = rpqToReversePolish(rpq);
    StringInfo predName = makeStringInfo();
    char *origName = MATCH_REL(rpq);

    if (MAP_HAS_STRING_KEY(c->subexToPred, rpqSubex))
        return;

    origName = replaceCharsForPred(origName);

    appendStringInfoString(predName, origName);
    //appendStringInfoString(predName, origName);
    while(hasSetElem(c->usedNames,predName->data))
        appendStringInfoString(predName,"1");

    addToSet(c->usedNames,strdup(predName->data));
    MAP_ADD_STRING_KEY(c->subexToPred,rpqSubex,createConstString(strdup(predName->data)));
    DEBUG_LOG("pred name for %s is %s wiht orig %s", rpqToShortString(rpq), strdup(predName->data), origName);

    FOREACH(Regex,child,rpq->children)
        rpqTranslate(child, c);

    switch(rpq->opType)
    {
        case REGEX_LABEL:
            translateLabel(rpq,c);
        break;
        case REGEX_OR:
            translateOr(rpq,c);
        break;
        case REGEX_PLUS:
            translatePlus(rpq,c);
        break;
        case REGEX_STAR:
            translateStar(rpq,c);
        break;
        case REGEX_CONC:
            translateConc(rpq,c);
        break;
        case REGEX_OPTIONAL:
            translateOptional(rpq,c);
        break;
    }
}

/**
 * for regex "l" return rule match_l
 */
static void
translateLabel(Regex *rpq, RpqToDatalogContext *c)
{
    char *relName = GET_MATCH_REL(rpq);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;

    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
    }
    else
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING),
                        createDLVar("X",DT_STRING),
                        createConstString(rpq->label),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
    }
    edge = createDLAtom(strdup(c->edgeRel),
            LIST_MAKE(createDLVar("X",DT_STRING),
            createConstString(rpq->label),
            createDLVar("Y",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));

    // add rule
    MAP_ADD_STRING_KEY(c->subexToRules,relName,singleton(r));
}

static void
translateOptional(Regex *rpq, RpqToDatalogContext *c)
{
    char *relName = GET_MATCH_REL(rpq);
    char *childRel = CHILD_GET_MATCH_REL(rpq,0);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge;
    List *rules = NIL;

    // match_a?(X,X) :- node(X)
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("X",DT_STRING)),
                        FALSE);
    }
    // match_a?(X,X,null,null,null) :- node(X)
    else
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("X",DT_STRING),
                        createNullConst(DT_STRING),
                        createNullConst(DT_STRING),
                        createNullConst(DT_STRING)),
                        FALSE);
    }
    edge = createDLAtom(strdup(NODE_REL_NAME),
            singleton(createDLVar("X",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // match_a?(X,Y) :- match_A(X,Y)
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        edge = createDLAtom(childRel,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
    }
    // match_a?(X,Y,Z,L,U) :- match_A(X,Y,Z,L,U)
    else
    {
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
    }
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(c->subexToRules,relName,rules);
}

static void
translatePlus(Regex *rpq, RpqToDatalogContext *c)
{
    char *relName = GET_MATCH_REL(rpq);
    char *childRel = CHILD_GET_MATCH_REL(rpq,0);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge1;
    DLAtom *edge2;
    List *rules = NIL;

    // match_A+(X,Y) :- match_A(X,Y)
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        edge1 = createDLAtom(childRel,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
    }
    // match_A+(X,Y,A,L,B) :- match_A(X,Y,A,L,B)
    else
    {
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
    }
    r = createDLRule(head, singleton(edge1));
    rules = appendToTailOfList(rules,r);

    if (c->queryType == RPQ_QUERY_RESULT)
    {
        // match_A+(X,Y) :- match_A+(X,Z), match_A(Z,Y).
        edge1 = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Z",DT_STRING)),
                        FALSE);
        edge2 = createDLAtom(childRel,
                LIST_MAKE(createDLVar("Z",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);

        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);

        r = createDLRule(head, LIST_MAKE(edge1,edge2));
        rules = appendToTailOfList(rules,r);
    }
    else
    {
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
    }

    // add rules
    MAP_ADD_STRING_KEY(c->subexToRules,relName,rules);
}

static void
translateStar(Regex *rpq, RpqToDatalogContext *c)
{
    char *relName = GET_MATCH_REL(rpq);
    char *childRel = CHILD_GET_MATCH_REL(rpq,0);
    DLRule *r;
    DLAtom *head;
    DLAtom *edge1;
    DLAtom *edge2;
    List *rules = NIL;

    // match_A*(X,Y) :- node(X)
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("X",DT_STRING)),
                        FALSE);

    }
    // match_A*(X,Y,null,null,null) :- node(X)
    else
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("X",DT_STRING),
                        createNullConst(DT_STRING),
                        createNullConst(DT_STRING),
                        createNullConst(DT_STRING)),
                        FALSE);
    }
    edge1 = createDLAtom(strdup(NODE_REL_NAME),
            singleton(createDLVar("X",DT_STRING)),
            FALSE);
    r = createDLRule(head, singleton(edge1));
    rules = appendToTailOfList(rules,r);

    // match_A*(X,Y) :- match_A*(X,Z), match_A(Z,Y).
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        edge1 = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Z",DT_STRING)),
                        FALSE);
        edge2 = createDLAtom(childRel,
                LIST_MAKE(createDLVar("Z",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        r = createDLRule(head, LIST_MAKE(edge1,edge2));
        rules = appendToTailOfList(rules,r);
    }
    else
    {
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
    }

    // add rules
    MAP_ADD_STRING_KEY(c->subexToRules,relName,rules);
}

static void
translateOr(Regex *rpq, RpqToDatalogContext *c)
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

    // match_A|B(X,Y) :- match_A(X,Y)
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        edge = createDLAtom(lChildRel,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
    }
    // match_A|B(X,Y,A,L1,B) :- match_A(X,Y,A,L1,B).s
    else
    {
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
    }
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // match_A|B(X,Y) :- match_B(X,Y)
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        edge = createDLAtom(rChildRel,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
    }
    // match_A.B(X,Y,C,L2,D) :- match_B(Z,Y,C,L2,D).
    else
    {
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
    }
    r = createDLRule(head, singleton(edge));
    rules = appendToTailOfList(rules,r);

    // add rules
    MAP_ADD_STRING_KEY(c->subexToRules,relName,rules);
}

static void
translateConc(Regex *rpq, RpqToDatalogContext *c)
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

    // match_A.B(X,Y) :- match_A(X,Z), match_B(Z,Y).
    if (c->queryType == RPQ_QUERY_RESULT)
    {
        // create a join between A . B
        edge1 = createDLAtom(lChildRel,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Z",DT_STRING)),
                        FALSE);
        edge2 = createDLAtom(rChildRel,
                LIST_MAKE(createDLVar("Z",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);
        head = createDLAtom(relName,
                LIST_MAKE(createDLVar("X",DT_STRING),
                        createDLVar("Y",DT_STRING)),
                        FALSE);

        r = createDLRule(head, LIST_MAKE(edge1,edge2));
        rules = appendToTailOfList(rules,r);
    }
    else
    {
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
    }

    // add rules
    MAP_ADD_STRING_KEY(c->subexToRules,relName,rules);
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
