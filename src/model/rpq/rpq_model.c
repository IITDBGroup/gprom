/*-----------------------------------------------------------------------------
 *
 * rpq_model.c
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
#include "model/rpq/rpq_model.h"
#include "parser/parser_rpq.h"

static void rpqToShortInternal (StringInfo str, Regex *rpq);
static void rpqToReversePolishInternal(StringInfo str, Regex *rpq);


Regex *
makeRegex(List *args, char *type)
{
    Regex *result = makeNode(Regex);

    result->children = args;
    result->opType = parseRegexOp(type);
    result->label = NULL;

    return result;
}

Regex *
makeRegexLabel(char *label)
{
    Regex *result = makeNode(Regex);

    result->children = NIL;
    result->opType = REGEX_LABEL;
    result->label = strdup(label);

    return result;
}

RegexOpType
parseRegexOp (char *label)
{
    if (streq(label,"+"))
        return REGEX_PLUS;
    if (streq(label,"*"))
        return REGEX_STAR;
    if (streq(label,"|"))
        return REGEX_OR;
    if (streq(label,"."))
        return REGEX_CONC;
    if (streq(label,"?"))
        return REGEX_OPTIONAL;

    FATAL_LOG("unkown regex type <%s>", label);
}

char *
rpqToShortString (Regex *rpq)
{
    StringInfo str = makeStringInfo();

    rpqToShortInternal(str, rpq);

    return str->data;
}

static void
rpqToShortInternal (StringInfo str, Regex *rpq)
{
    switch (rpq->opType)
    {
        case REGEX_LABEL:
            appendStringInfoString(str, rpq->label);
            break;
        case REGEX_OR:
            appendStringInfoString(str, "(");
            rpqToShortInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, "|");
            rpqToShortInternal(str, getNthOfListP(rpq->children, 1));
            appendStringInfoString(str, ")");
        break;
        case REGEX_PLUS:
            appendStringInfoString(str, "(");
            rpqToShortInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, ")+");
        break;
        case REGEX_STAR:
            appendStringInfoString(str, "(");
            rpqToShortInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, ")*");
        break;
        case REGEX_CONC:
            rpqToShortInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, ".");
            rpqToShortInternal(str, getNthOfListP(rpq->children, 1));
        break;
        case REGEX_OPTIONAL:
            appendStringInfoString(str, "(");
            rpqToShortInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, ")?");
        break;
    }
}

char *
rpqToReversePolish(Regex *rpq)
{
    StringInfo str = makeStringInfo();

    rpqToReversePolishInternal(str, rpq);

    return str->data;
}

static void
rpqToReversePolishInternal(StringInfo str, Regex *rpq)
{
    switch (rpq->opType)
    {
        case REGEX_LABEL:
            appendStringInfoString(str, rpq->label);
            break;
        case REGEX_OR:
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, "_");
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 1));
            appendStringInfoString(str, "_OR");
            break;
        case REGEX_PLUS:
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, "_PLUS");
        break;
        case REGEX_STAR:
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, "_STAR");
        break;
        case REGEX_CONC:
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, "_");
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 1));
            appendStringInfoString(str, "_CONC");
        break;
        case REGEX_OPTIONAL:
            rpqToReversePolishInternal(str, getNthOfListP(rpq->children, 0));
            appendStringInfoString(str, "_OPT");
        break;
    }
}

extern RPQQuery *
makeRPQQuery(char *query, char *rpqType, char *edgeRel, char *resultRel)
{
    RPQQuery *result = makeNode(RPQQuery);

    result->q = (Regex *) parseFromStringrpq(query);
    result->t = getRPQQueryType(rpqType);
    result->edgeRel = edgeRel;
    result->resultRel = resultRel;

    return result;
}

RPQQueryType
getRPQQueryType (char *t)
{
    if (streq(t,"RESULT"))
        return RPQ_QUERY_RESULT;
    if (streq(t,"PROV"))
        return RPQ_QUERY_PROV;
    if (streq(t,"SUBGRAPH"))
        return RPQ_QUERY_SUBGRAPH;
    FATAL_LOG("unkown rpq query type <%s>. Should be one of RESULT, PROV, SUBGRAPH", t);
    return RPQ_QUERY_RESULT;
}
