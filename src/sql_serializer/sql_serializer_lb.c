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
#include "utility/string_utils.h"

static void datalogToStr(StringInfo str, Node *n, int indent);
static Node *fixExpression(Node *n);
static Node *fixExpressionMutator (Node *n, void *context);
static char *exprToLB (Node *e);
static void exprToLBInternal (StringInfo str, Node *e);
static void funcToLB(StringInfo str, FunctionCall *f);
static void opToLB (StringInfo str, Operator *o);
static char *dataTypeToLB (DataType d);
static char *constToLB(Constant *c);


char *
serializeOperatorModelLB(Node *q)
{
    StringInfo str = makeStringInfo();

    //TODO change operators and functions in expressions to LogiQL equivalent

    // add a rule to output query result in lb
    DLProgram *p = (DLProgram *) q;
	char *headPred = NULL;
	int i = 0;

	FOREACH(DLRule,a,p->rules)
	{
		if (i == 0)
			headPred = a->head->rel;

		i++;
	}

	appendStringInfo(str,"%s",CONCAT_STRINGS("_(X,Y) <- _", headPred, "(X,Y).\n"));
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

            // make IDB predicate local
            if (DL_HAS_PROP(a,DL_IS_IDB_REL) || !DL_HAS_PROP(a,DL_IS_EDB_REL))
            	appendStringInfo(str, "%s(", CONCAT_STRINGS("_", a->rel));
            else
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
            {
                DEBUG_NODE_BEATIFY_LOG("expr before transformation", n);
                // add casts where necessary
                n = addCastsToExpr(n, TRUE);
                DEBUG_NODE_BEATIFY_LOG("expr after adding casts", n);

                // translate operators and translate casts into logicblox function calls
                n = fixExpression(n);
                DEBUG_NODE_BEATIFY_LOG("expr after fixing to LB", n);

                // output expression
                char *result = exprToLB(n);
                DEBUG_LOG("expr: %s", result);
                // replace string concat and single quote to plus and double for lb

//                char *result = NULL;
//                result = replaceSubstr(exprToSQL(n), " || ", " + ");
//                result = replaceSubstr(result, "'", "\"");
                appendStringInfo(str, "%s", result);
            }
            else
                FATAL_LOG("should have never come here, datalog program should"
                        " not have nodes like this: %s",
                        beatify(nodeToString(n)));
        }
        break;
    }
}

static Node *
fixExpression(Node *n)
{
    n = fixExpressionMutator(n, NULL);
    return n;
}

static Node *
fixExpressionMutator (Node *n, void *context)
{
    if (n == NULL)
        return NULL;

    if(isA(n, Operator))
    {
        Operator *o = (Operator *) n;

        if (streq(o->name, "||"))
        {
            o->name = "+";
        }

        FOREACH_LC(arg,o->args)
        {
            Node *a = LC_P_VAL(arg);
            LC_P_VAL(arg) = fixExpressionMutator(a, context);
        }

        return (Node *) o;
    }

    if (isA(n, CastExpr))
    {
        CastExpr *c = (CastExpr *) n;
        FunctionCall *f;
        StringInfo fName;
        DataType inType = typeOf(c->expr);
        Node *arg;

        fName = makeStringInfo();
        appendStringInfo(fName, "%s:%s:convert", dataTypeToLB(inType), dataTypeToLB(c->resultDT));

        arg = fixExpressionMutator(c->expr, context);
        f = createFunctionCall(fName->data, singleton(arg));

        return (Node *) f;
    }

//    if(isA(n, FunctionCall))
//    {
//        FunctionCall *f = (FunctionCall *) n;
//
//    }

    return mutate(n, fixExpressionMutator, context);
}

static char *
exprToLB (Node *e)
{
    StringInfo str = makeStringInfo();

    exprToLBInternal(str, e);

    return str->data;
}

static void
exprToLBInternal (StringInfo str, Node *e)
{
    if (e == NULL)
        return;

    switch(e->type)
    {
        case T_DLVar:
            appendStringInfo(str, "%s", ((DLVar *) e)->name);
            break;
        case T_Constant:
            appendStringInfoString(str,constToLB((Constant *) e));
            break;
        case T_FunctionCall:
            funcToLB(str, (FunctionCall *) e);
            break;
        case T_Operator:
            opToLB(str, (Operator *) e);
            break;
        case T_List:
        {
            int i = 0;
            FOREACH(Node,arg,(List *) e)
            {
                appendStringInfoString(str, ((i++ == 0) ? "(" : ", "));
                exprToLBInternal(str, arg);
            }
            appendStringInfoString(str,")");
        }
        break;
        default:
            FATAL_LOG("not an LB supported expression node <%s>", nodeToString(e));
    }
}

static void
funcToLB(StringInfo str, FunctionCall *f)
{
    appendStringInfoString(str, f->functionname);

    appendStringInfoString(str, "[");

    int i = 0;
    FOREACH(Node,arg,f->args)
    {
        appendStringInfoString(str, ((i++ == 0) ? "" : ", "));
        exprToLBInternal(str, arg);
    }

    appendStringInfoString(str, "]");
}

static void
opToLB (StringInfo str, Operator *o)
{
    appendStringInfoString(str, "(");

    FOREACH(Node,arg,o->args)
    {
        exprToLBInternal(str,arg);
        if(arg_his_cell != o->args->tail)
            appendStringInfo(str, " %s ", o->name);
    }

    appendStringInfoString(str, ")");
}

static char *
dataTypeToLB (DataType d)
{
    switch (d)
    {
        case DT_INT:
            return "int";
        case DT_STRING:
            return "string";
        case DT_FLOAT:
             return "float";
        case DT_BOOL:
             return "bool";
        default:
            return "string"; //TODO
    }
}

static char *
constToLB(Constant *c)
{
    if (CONST_IS_NULL(c))
        return strdup("null");
    if (c->constType == DT_STRING)
    {
        return CONCAT_STRINGS("\"",STRING_VALUE(c),"\"");
    }
    if (c->constType == DT_BOOL)
    {
        if (BOOL_VALUE(c))
            return "true";
        else
            return "false";
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
