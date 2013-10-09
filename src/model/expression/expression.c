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
#include "model/expression/expression.h"



AttributeReference *
createAttributeReference (char *name)
{
    AttributeReference *result = makeNode(AttributeReference);

    if (name != NULL)
    {
        result->name = (char *) MALLOC(strlen(name) + 1);
        strcpy(result->name, name);
    }
    else 
        result->name = NULL;
    result->fromClauseItem = INVALID_FROM_ITEM;
    result->attrPosition = INVALID_ATTR;

    return result;
}


FunctionCall *
createFunctionCall(char *fName, List *args)
{
    FunctionCall *result = makeNode(FunctionCall);
    
    if(fName != NULL)
    {
        result->functionname = (char *) MALLOC(strlen(fName) + 1);
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
        result->name = (char *) MALLOC(strlen(name) + 1);
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
createConstFloat (float value)
{
    Constant *result = makeNode(Constant);
    double *v = NEW(double);

    *v = value;
    result->constType = DT_FLOAT;
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
    if (isA(expr, AttributeReference)) // need to figure out type
    {
        return DT_STRING;
    }
    return DT_STRING;
}
