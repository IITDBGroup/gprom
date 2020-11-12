/*-----------------------------------------------------------------------------
 *
 * schema_utility.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "model/query_operator/schema_utility.h"
#include "log/logger.h"

int
getAttributeNum (char *attrName, QueryOperator *op)
{
    int i = 0;

    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        if (!strcmp(a->attrName, attrName))
            return i;
        i++;
    }

    ERROR_LOG("Did not find attribute <%s>", attrName);
    return -1;
}
