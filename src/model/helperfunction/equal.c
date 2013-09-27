/*************************************
 *         equal.c
 *    Author: Hao Guo
 *    One-line description
 *
 *
 *
 **************************************/

#include <string.h>

#include "common.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

/* equal functions for specific node types */
static boolean equalFunctionCall(FunctionCall *a, FunctionCall *b);
static boolean equalAttributeReference (AttributeReference *a,
        AttributeReference *b);
static boolean equalList(List *a, List *b);


/*use these macros to compare fields */

/*compare one simple scalar field(int, boolean, float, etc)*/
#define COMPARE_SCALAR_FIELD(fldname)  \
		do{  \
			if (a->fldname != b->fldname)  \
			return FALSE;  \
		} while (0)

/*compare a field pointer to Node tree*/
#define COMPARE_NODE_FIELD(fldname)  \
		do{  \
			if(!equal(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/*compare a field that is a pointer to a C string or maybe NULL*/
#define COMPARE_STRING_FIELD(fldname)  \
		do{  \
			if(!equalstr(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/*compare a string field that maybe NULL*/
#define equalstr(a, b)  \
		(((a) != NULL && (b) != NULL) ? (strcmp(a, b) == 0) : (a) == (b))

/* */
static boolean
equalAttributeReference (AttributeReference *a,
        AttributeReference *b)
{
    COMPARE_STRING_FIELD(name);

    return TRUE;
}

/* */
static boolean
equalFunctionCall(FunctionCall *a, FunctionCall *b)
{
    COMPARE_NODE_FIELD(functionname);
    COMPARE_NODE_FIELD(args);

    return TRUE;
}

/*equal list fun */
static boolean
equalList(List *a, List *b)
{
    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length

    switch(a->type)
    {
        case T_List:
        {
            FORBOTH(Node,l,r,a,b)
            {
                if (!equal(a,b))
                    return FALSE;
            }
        }
        break;
        case T_IntList:
        {
            FORBOTH_INT(i,j,a,b)
            {
                if (i != j)
                    return FALSE;
            }
        }
        break;
        default:
            return FALSE;
    }

    return TRUE;
}


/*equalfun returns  whether two nodes are equal*/
boolean
equal(void *a, void *b)
{
    boolean retval;
    if (a == b)
        return TRUE;

    if (a == NULL || b == NULL)
        return FALSE;

    if (nodeTag(a) !=nodeTag(b))
        return FALSE;

    switch(nodeTag(a))
    {
        case T_List:
        case T_IntList:
            retval = equalList(a,b);
            break;
        case T_FunctionCall:
            retval = equalFunctionCall(a,b);
            break;
        case T_AttributeReference:
            retval = equalAttributeReference(a,b);
            break;
            /*something different cases this, and we have*/
            /*different types of T_Node       */
        default:
            retval = FALSE;
            break;
    }

    return retval;
}
