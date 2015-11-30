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

static void rpqToShortInternal (StringInfo str, Regex *rpq);


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
