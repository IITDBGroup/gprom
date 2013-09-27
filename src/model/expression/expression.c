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


