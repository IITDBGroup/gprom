/*-----------------------------------------------------------------------------
 *
 * sort_helpers.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "model/expression/expression.h"
#include "utility/sort_helpers.h"

int
compareConstInt (const Constant *c1, const Constant *c2)
{
    int lV = INT_VALUE(c1);
    int rV = INT_VALUE(c2);

    return rV - lV;
}
