/*-----------------------------------------------------------------------------
 *
 * sql_serializer_dl.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "sql_serializer/sql_serializer_oracle.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/datalog/datalog_model.h"
#include "model/list/list.h"
#include "model/set/set.h"

static void datalogToStr(StringInfo str, Node *n, int indent);
static char *constToLB(Constant *c);


char *
serializeOperatorModelLB(Node *q)
{
    StringInfo str = makeStringInfo();
    //TODO change operators and functions in expressions to LogiQL equivalent
    datalogToStr(str, q, 0);

    return str->data;
}

//TODO 1) sanitize constants (remove '' from strings), they have to be lower-case), sanitize rel names (lowercase), translate expressions (e.g., string concat to skolems)

static void
datalogToStr(StringInfo str, Node *n, int indent)
{
    if (n == NULL)
        return;

    switch(n->type)
    {
        case T_DLAtom:
        {
            DLAtom *a = (DLAtom *) n;
            int i = 1;
            int len = LIST_LENGTH(a->args);

            if (a->negated)
                appendStringInfoString(str, "! ");
            appendStringInfo(str, "%s(", a->rel);
            FOREACH(Node,arg,a->args)
            {
                datalogToStr(str, arg, indent);
                if (i++ < len)
                    appendStringInfoString(str, ",");
            }
            appendStringInfoString(str, ")");
        }
        break;
        case T_DLRule:
        {
            DLRule *r = (DLRule *) n;
            int i = 1;
            int len = LIST_LENGTH(r->body);

            indentString(str,indent);

            datalogToStr(str, (Node *) r->head, indent);
            appendStringInfoString(str, " <- ");
            FOREACH(Node,a,r->body)
            {
                datalogToStr(str, a, indent);
                if (i++ < len)
                    appendStringInfoString(str, ",");
            }

            appendStringInfoString(str, ".\n");
        }
        break;
        case T_DLComparison:
        {
            DLComparison *c = (DLComparison *) n;

            datalogToStr(str,getNthOfListP(c->opExpr->args, 0), indent);
            appendStringInfo(str, " %s ", c->opExpr->name);
            datalogToStr(str,getNthOfListP(c->opExpr->args, 1), indent);
//            appendStringInfo(str, "%s", exprToSQL((Node *) c->opExpr));
        }
        break;
        case T_DLVar:
        {
            DLVar *v = (DLVar *) n;

            appendStringInfo(str, "%s", v->name);
        }
        break;
        case T_DLProgram:
        {
            DLProgram *p = (DLProgram *) n;

            FOREACH(Node,f,p->facts)
            {
                datalogToStr(str,(Node *) f, 0);
            }
            FOREACH(Node,r,p->rules)
            {
                datalogToStr(str,(Node *) r, 0);
            }
        }
        break;
        case T_Constant:
            appendStringInfo(str, "%s",
                    constToLB((Constant *) n));
        break;
        // provenance
        case T_List:
        {
            List *l = (List *) n;
            FOREACH(Node,el,l)
            datalogToStr(str,el, indent + 4);
        }
        break;
        default:
        {
            if (IS_EXPR(n))
                appendStringInfo(str, "%s", exprToSQL(n));
            else
                FATAL_LOG("should have never come here, datalog program should"
                        " not have nodes like this: %s",
                        beatify(nodeToString(n)));
        }
        break;
    }
}

static char *
constToLB(Constant *c)
{
    if (CONST_IS_NULL(c))
        return strdup("null");
    if (c->constType == DT_STRING)
    {
        return STRING_VALUE(c);
    }
    else
        return CONST_TO_STRING(c);
}

char *
serializeQueryLB(QueryOperator *q)
{
    FATAL_LOG("should never have ended up here");
    return NULL;
}


char *
quoteIdentifierLB (char *ident)
{
    return ident; //TODO
}
