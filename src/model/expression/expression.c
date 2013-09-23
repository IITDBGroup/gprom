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
