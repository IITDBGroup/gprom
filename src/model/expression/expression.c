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
createFunctionCall(char *fName, List *args);
{
    FunctionCall *result = makeNode(FunctionCall);
    
    if(fName != NULL)
    {
        result->fName = (char *) MALLOC(strlen(name) + 1);
        strcpy(result->fName, fName);
    }
    else if(args != NULL)
    {
        result->args = (char *) MALLOC(strlen(name) + 1);
        strcpy(result->args, args);
    }
    else
        result->fName = NULL;
        result->args = NULL;

    return result; 
}




