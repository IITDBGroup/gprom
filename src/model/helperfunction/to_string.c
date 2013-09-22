/*************************************
*         to_string.c
*    Author: Hao Guo
* Implement a function that given a query
* tree or expression tree that returns a    
* string representing the tree. 
*
**************************************/

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

/*define macros*/

/*label for the node type*/
#define booltostr(a) \
 ((a) ? "ture" : "false")

#define WRITE_NODE_TYPE(nodelabel)  \
   appendStringInfoString(str, nodelabel)


/*int field*/
#define WRITE_INT_FIELD(fldname)  \
   appendStringInfo(str, ":" CppAsString(fldname) "%d", node->fldname)

/*long-int field*/
#define WRITE_LONG_FIELD(fldname)  \
   appendStringInfo(str, ":" CppAsString(fldname) "%ld", node->fldname)

/*char field*/
#define WRITE_CHAR_FIELD(fldname)  \
   appendStringInfo(str, ":" CppAsString(fldname) "%c", node->fldname)

/*enum-type field as integer*/
#define WRITE_ENUM_FIELD(fldname, enumtype)  \
   appendStringInfo(str, ":" CppAsString(fldname) "%d", (int)node->fldname)

/*float field*/
#define WRITE_FLOAT_FIELD(fldname, format)  \
   appendStringInfo(str, ":" CppAsString(fldname) " " format, node->fldname)

/*bool field*/
#define WRITE_BOOL_FIELD(fldname)  \
   appendStringInfo(str, ":" CppAsString(fldname) "%s", booltostr(node->fldname))

/*node field*/
#define WRITE_NODE_FIELD(fldname)  \
   appendStringInfo((str, ":" CppAsString(fldname) " "), outNode(str, node->fldname))



/*outNode from node append it to string*/
static void outList(StringInfo str, List *node)
{
     ListCell *lc;
     appendStringInfoChar(str, '(');
     
     if(isA(node, IntList))
        appendStringInfoChar(str, 'i');
    
      foreach(lc, node)
         {
            if(isA(node, List))
             {
               outNode(str, lfirst(lc));
               if(lnext(lc))
                  appendStringInfoChar(str, ' ');
             }
            else if(isA(node, IntList))
                  appendStringInfo(str, "%d", lfirst_int(lc));
            
         }

       appendStringInfoChar(str, ')');

}

typedef struct Value
{
   NodeTag type;

   union ValUnion
     {
        long   ival; /*integer*/
        char   *str;   /*string*/

     }val;
}Value;

static void outValue(StringInfo str, Value *value)
{
        switch (value->type)
           {
              case T_Integer:
                   appendStringInfo(str, "%ld", value->val.ival);
                   break;

              case T_NULL:
                   appendStringInfoString(str, "NULL"); 
                   break;

              default :
                   break;                               
           }
}

void outNode(StringInfo str, void *obj)
{
   if(obj == NULL)
       appendStringInfo(str, "<>");
   else if(isA(obj, List) || isA(obj, IntList))
       outList(str, obj);
   else if(isA(obj, Integer) || isA(obj, Float) || isA(obj, String))
       {
        outValue(str, obj);       
       }
   else 
    {
        appendStringInfoChar(str, '{');
        switch (nodeTag(obj))
            {
            
              //different case 
              //query operator model nodes 
              //T_QueryOperator,
              //T_SelectionOperator,
              //T_ProjectionOperator,
              //T_JoinOperator,
              //T_AggregationOperator,
              //T_ProvenanceComputation
             
           default :
                //outNode(str, obj);
                break;
            }
         appendStringInfoChar(str, '}');
    }
}

/*nodeToString return the node as string*/

char *nodeToString(void *obj)
{
     StringInfoData str;
     initStringInfo(&str);
     outNode(&str, obj);
     return str.data;
}




