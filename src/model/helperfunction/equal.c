/*************************************
*         equal.c
*    Author: Hao Guo
*    One-line description
*    
*   
*
**************************************/





#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"



/*use the Macros*/

/*compare one simple scalar field(int, bool, float, etc)*/
#define COMPARE_SCALAR_FIELD(fldname)  \
      do{  \
           if (a->fldname != b->fldname)  \
                return false;  \
        } while (0) 

/*compare a field pointer to Node tree*/
#define COMPARE_NODE_FIELD(fldname)  \
      do{  \
           if(!equal(a->fldname, b->fldname))  \
              return false;  \
        } while (0)

/*compare a field that is a pointer to a C string or maybe NULL*/
#define COMPARE_STRING_FIELD(fldname)  \
      do{  \
           if(!equal(a->fldname, b->fldname))  \
              return false;  \
        } while (0)

/*compare a string field that maybe NULL*/
#define equalstr(a, b)  \
(((a) != NULL && (b) != NULL) ? (strcmp(a, b) == 0) : (a) ==(b))


static bool equalFunctionCall(FunctionCall *a, FunctionCall *b)
{
    COMPARE_NODE_FIELD(functionname);
    COMPARE_NODE_FIELD(args);
    COMPARE_SCALAR_FIELD(agg_star);
    COMPARE_SCALAR_FIELD(agg_distinct);
    COMPARE_SCALAR_FIELD(location);

    return ture;
}

/*equal list fun */
static bool equalList(List *a, List *b)
{
   ListCell *item_a;
   ListCell *item_b;
   COMPARE_SCALAR_FIELD(type);
   COMPARE_SCALAR_FIELD(length);

   switch(a->type)
     {
       case T_List:
           {
                if(!equal(lfirst(item_a), lfirst(item_b)))
                    return false;
           }
           break;

       default:
           return false;
     }

/*if it works, the elements of both lists should run out*/

Assert(item_a == NULL);
Assert(item_b == NULL);

return true;
}


/*equalfun returns  whether two nodes are equal*/
extern bool nodesEquals(void *a, void *b);
bool equal(void *a, void *b)
{
       bool retval;
       if (a == b)
            return true;
       
       if (a == NULL || b == NULL)
            return false;
        
       if (nodeTag(a) !=nodeTag(b))
            return false;

       switch(nodeTag(a))
         {
               //case T_Integer:
                 //COMPARE_SCALAR_FIELD();
                   // break;
               /*something different cases this, and we have*/
               /*different types of T_Node       */
               default:
                   retval = nodesEquals(a,b);
                   break;
         }
         
         return retval;
}











