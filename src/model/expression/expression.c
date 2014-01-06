/*-----------------------------------------------------------------------------
 *
 * expression.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include <string.h>

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

AttributeReference *
createAttributeReference (char *name)
{
    AttributeReference *result = makeNode(AttributeReference);

    result->name = strdup(name);
    result->fromClauseItem = INVALID_FROM_ITEM;
    result->attrPosition = INVALID_ATTR;
    result->outerLevelsUp = INVALID_ATTR;

    return result;
}

AttributeReference *
createFullAttrReference (char *name, int fromClause, int attrPos,
        int outerLevelsUp)
{
    AttributeReference *result = makeNode(AttributeReference);

    result->name = strdup(name);
    result->fromClauseItem = fromClause;
    result->attrPosition = attrPos;
    result->outerLevelsUp = outerLevelsUp;

    return result;
}

Node *
andExprs (Node *expr, ...)
{
    Node *result = NULL;
    Node *curArg = NULL;
    va_list args;

    va_start(args, expr);

    while((curArg = va_arg(args,Node*)))
    {
        if (result == NULL)
            result = copyObject(curArg);
        else
            result = (Node *) createOpExpr("AND", LIST_MAKE(result, copyObject(curArg)));
    }

    va_end(args);

    return result;
}

FunctionCall *
createFunctionCall(char *fName, List *args)
{
    FunctionCall *result = makeNode(FunctionCall);
    
    if(fName != NULL)
    {
        result->functionname = (char *) CALLOC(1,strlen(fName) + 1);
        strcpy(result->functionname, fName);
    }
    else
        result->functionname = NULL;

    result->args = args; //should we copy?
    result->isAgg = FALSE;

    return result; 
}

Operator *
createOpExpr(char *name, List *args)
{
    Operator *result = makeNode(Operator);

    if(name != NULL)
    {
        result->name = (char *) CALLOC(1, strlen(name) + 1);
        strcpy(result->name, name);
    }
    else
        result->name = NULL;

    result->args = args;

    return result;
}

Constant *
createConstInt (int value)
{
    Constant *result = makeNode(Constant);
    int *v = NEW(int);

    *v = value;
    result->constType = DT_INT;
    result->value = v;

    return result;
}

Constant *
createConstString (char *value)
{
    Constant *result = makeNode(Constant);

    result->constType = DT_STRING;
    result->value = strdup(value);

    return result;
}

Constant *
createConstFloat (double value)
{
    Constant *result = makeNode(Constant);
    double *v = NEW(double);

    *v = value;
    result->constType = DT_FLOAT;
    result->value = v;

    return result;
}

Constant *
createConstBool (boolean value)
{
    Constant *result = makeNode(Constant);
    boolean *v = NEW(boolean);

    *v = value;
    result->constType = DT_BOOL;
    result->value = v;

    return result;
}

DataType
typeOf (Node *expr)
{
    switch(expr->type)
    {
        case T_Constant:
        {
            Constant *c = (Constant *) expr;
            return c->constType;
        }
        //TODO use metadata lookup
        case T_FunctionCall:
        case T_Operator:
            return DT_STRING;
        default:
             ERROR_LOG("unkown expression type for node: %s", nodeToString(expr));
             break;
    }
    return DT_STRING;
}

DataType
typeOfInOpModel (Node *expr, List *inputOperators)
{
    switch(expr->type)
    {
        case T_AttributeReference: //TODO need to figure out type
            return DT_STRING;
        case T_FunctionCall:
        case T_Operator:
            return DT_STRING; //TODO metadata lookup plugin
        case T_Constant:
            return typeOf(expr);
        default:
            ERROR_LOG("unkown expression type for node: %s", nodeToString(expr));
            break;
    }
    return DT_STRING;
}
