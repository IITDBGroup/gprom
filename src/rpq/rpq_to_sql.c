/*-----------------------------------------------------------------------------
 *
 * rpq_to_sql.c
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
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/rpq/rpq_model.h"
#include "rpq/rpq_to_sql.h"
#include "utility/string_utils.h"


#define EDGE_REL_NAME "e"
#define NODE_REL_NAME "node"
#define MATCH_PRED "match"
#define RESULT_PRED "result"

#define SQL_TEMPLATE_LABEL "$1 AS (SELECT head, tail, head AS ehead, label, tail AS etail FROM " EDGE_REL_NAME " WHERE label = '$2')"
#define SQL_TEMPLATE_OPTIONAL "$1 AS (SELECT * FROM $2 UNION ALL SELECT head, tail, NULL, NULL, NULL FROM dual)"
#define SQL_TEMPLATE_PLUS "$1(head,tail,ehead,label,etail) AS (SELECT CONNECT_BY_ROOT(head) head, tail, ehead,label,etail " \
                          "FROM $2 CONNECT BY NOCYCLE PRIOR tail = head)"
#define SQL_TEMPLATE_STAR "$1(head,tail,ehead,label,etail) AS ((SELECT CONNECT_BY_ROOT(head) AS head, tail, ehead,label,etail " \
		                  "FROM $2 CONNECT BY NOCYCLE PRIOR tail = head) UNION ALL (SELECT n AS head, n AS tail, NULL, NULL, NULL FROM " NODE_REL_NAME "))"
#define SQL_TEMPLATE_CONC "$1_join AS (SELECT l.head, r.tail, l.ehead AS lehead, l.label AS llabel, l.etail AS letail, r.ehead AS rehead, r.label AS rlabel, r.etail AS retail FROM $2 l, $3 r WHERE l.tail = r.head),\n" \
                          "$1 AS ((SELECT head, tail, lehead AS ehead, llabel AS label, letail AS etail FROM $1_join) " \
                          "UNION ALL (SELECT head, tail, rehead AS ehead, rlabel AS label, retail AS etail FROM $1_join))"
#define SQL_TEMPLATE_OR "$1 AS (SELECT * FROM $2 UNION ALL SELECT * FROM $3)"
#define SQL_TEMPLATE_RESULT "WITH node AS (SELECT head AS n FROM "  EDGE_REL_NAME  \
                            " UNION ALL SELECT tail AS n FROM "  EDGE_REL_NAME  ")," \
		                    "$1 \nSELECT DISTINCT * FROM $2;\n"

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

static char *replaceCharsForPred(char *in);
static char *getUniqueName(char *in, Set *usedNames);

char *
rpqToSQL(Regex *rpq)
{
    HashMap *subexToRules = NEW_MAP(Constant,Constant);
    HashMap *subexToPred = NEW_MAP(Constant,Constant);
    StringInfo ctes = makeStringInfo();
    Set *usedNames = STRSET();
    char *result;

    rpqTranslate(rpq, subexToRules, subexToPred, usedNames);

    // create CTEs (WITH views)
    FOREACH_HASH(Constant,c,subexToRules)
    {
        appendStringInfo(ctes, "\n%s,", STRING_VALUE(c));
    }
    removeTailingStringInfo(ctes,1);

    // create wrapper query
    result = specializeTemplate(SQL_TEMPLATE_RESULT, LIST_MAKE(ctes->data, GET_MATCH_REL(rpq)));

    return result;
}

static void
rpqTranslate (Regex *rpq, HashMap *subexToRules, HashMap *subexToPred, Set *usedNames)
{
    char *rpqSubex = rpqToReversePolish(rpq);
//    StringInfo predName = makeStringInfo();
    char *origName = MATCH_REL(rpq);

    if (MAP_HAS_STRING_KEY(subexToPred, rpqSubex))
        return;

    origName = getUniqueName(origName, usedNames);

//            replaceCharsForPred(origName);
//
//    appendStringInfoString(predName, origName);
//    //appendStringInfoString(predName, origName);
//    while(hasSetElem(usedNames,predName->data))
//        appendStringInfoString(predName,"1");
//    addToSet(usedNames,strdup(predName->data));
    MAP_ADD_STRING_KEY(subexToPred,rpqSubex,createConstString(strdup(origName)));
    DEBUG_LOG("pred name for %s is %s wiht orig %s", rpqToShortString(rpq), strdup(origName), origName);

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

static void
translateLabel(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *sql;

    sql = specializeTemplate(SQL_TEMPLATE_LABEL,LIST_MAKE(relName,strdup(rpq->label)));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,createConstString(sql));
}

static void
translateOptional(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *sql;
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);

    sql = specializeTemplate(SQL_TEMPLATE_OPTIONAL,LIST_MAKE(relName,lChildRel));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,createConstString(sql));
}

static void
translatePlus(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *sql;
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);

    sql = specializeTemplate(SQL_TEMPLATE_PLUS,LIST_MAKE(relName,lChildRel));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,createConstString(sql));
}

static void
translateStar(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *sql;
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);

    sql = specializeTemplate(SQL_TEMPLATE_STAR,LIST_MAKE(relName,lChildRel));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,createConstString(sql));
}

static void
translateOr(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *sql;
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);
    Regex *rChild = getNthOfListP(rpq->children,1);
    char *rChildRel = GET_MATCH_REL(rChild);

    sql = specializeTemplate(SQL_TEMPLATE_OR,LIST_MAKE(relName,lChildRel, rChildRel));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,createConstString(sql));
}

static void
translateConc(Regex *rpq, HashMap *subexToRules, HashMap *subexToPred)
{
    char *relName = GET_MATCH_REL(rpq);
    char *sql;
    Regex *lChild = getNthOfListP(rpq->children,0);
    char *lChildRel = GET_MATCH_REL(lChild);
    Regex *rChild = getNthOfListP(rpq->children,1);
    char *rChildRel = GET_MATCH_REL(rChild);

    sql = specializeTemplate(SQL_TEMPLATE_CONC,LIST_MAKE(relName,lChildRel, rChildRel));

    // add rule
    MAP_ADD_STRING_KEY(subexToRules,relName,createConstString(sql));
}

static char *
getUniqueName(char *in, Set *usedNames)
{
    char *resultStr = replaceCharsForPred(in);
    StringInfo result = makeStringInfo();
    int count = 0;

    if (strlen(resultStr) > 30)
        resultStr = substr(resultStr,0,29);
    appendStringInfoString(result, resultStr);
    DEBUG_LOG("%s with len %u", resultStr, strlen(resultStr));
    //appendStringInfoString(predName, origName);
    while(hasSetElem(usedNames,result->data))
    {
        char *countStr = itoa(count++);
        int countLen = strlen(countStr);
        int pos = MIN(STRINGLEN(result),30-countLen);
        replaceStringInfo(result,pos,countStr);
    }

    addToSet(usedNames,strdup(result->data));
    return result->data;
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

