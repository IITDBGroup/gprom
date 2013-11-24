/*-----------------------------------------------------------------------------
 *
 * expr_to_sql.c
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
#include "model/expression/expression.h"


/* function declarations */
static void exprToSQLString(StringInfo str, Node *expr);
static void attributeReferenceToSQL (StringInfo str, AttributeReference *node);
static void constantToSQL (StringInfo str, Constant *node);
static void functionCallToSQL (StringInfo str, FunctionCall *node);
static void operatorToSQL (StringInfo str, Operator *node);

static void
attributeReferenceToSQL (StringInfo str, AttributeReference *node)
{
    appendStringInfoString(str, node->name);
}

static void
constantToSQL (StringInfo str, Constant *node)
{
    switch(node->constType)
    {
        case DT_INT:
            appendStringInfo(str, "%u", *((int *) node->value));
            break;
        case DT_FLOAT:
            appendStringInfo(str, "%f", *((double *) node->value));
            break;
        case DT_STRING:
            appendStringInfo(str, "'%s'", (char *) node->value);
            break;
        case DT_BOOL:
            appendStringInfo(str, "%s", *((boolean *) node->value) == TRUE ? "TRUE" : "FALSE");
            break;
    }
}

static void
functionCallToSQL (StringInfo str, FunctionCall *node)
{
    appendStringInfoString(str, node->functionname);

    int i = 0;
    FOREACH(Node,arg,node->args)
    {
        appendStringInfoString(str, ((i++ == 0) ? "(" : ", "));
        exprToSQLString(str, arg);
    }

    appendStringInfoString(str,")");
}

static void
operatorToSQL (StringInfo str, Operator *node)
{
    appendStringInfoString(str, "(");
    exprToSQLString(str,getNthOfListP(node->args,0));

    appendStringInfo(str, " %s ", node->name);

    exprToSQLString(str,getNthOfListP(node->args,1));
    appendStringInfoString(str, ")");
}


static void
exprToSQLString(StringInfo str, Node *expr)
{
    if (expr == NULL)
        return;

    switch(expr->type)
    {
        case T_AttributeReference:
            attributeReferenceToSQL(str, (AttributeReference *) expr);
            break;
        case T_Constant:
            constantToSQL(str, (Constant *) expr);
            break;
        case T_FunctionCall:
            functionCallToSQL(str, (FunctionCall *) expr);
            break;
        case T_Operator:
            operatorToSQL(str, (Operator *) expr);
            break;
        case T_List:
        {
            int i = 0;
            FOREACH(Node,arg,(List *) expr)
            {
                appendStringInfoString(str, ((i++ == 0) ? "(" : ", "));
                exprToSQLString(str, arg);
            }
            appendStringInfoString(str,")");
        }
        break;
        default:
            FATAL_LOG("not an expression node <%s>", nodeToString(expr));
    }
}

char *
exprToSQL (Node *expr)
{
    StringInfo str = makeStringInfo();
    char *result;

    exprToSQLString(str, expr);

    result = str->data;
    FREE(str);

    return result;
}
