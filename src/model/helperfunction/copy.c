/*************************************
*         copy.c
*    Author: Hao Guo
*    copy function for tree nodes
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



/*use the Macros(the varibles are 'newnode' and 'from')*/

/*copy a simple scalar field(int, bool, float, etc)*/
#define COPY_SCALAR_FIELD(fldname)  \
      (newnode->fldname = from->fldname)

/*copy a field that is a pointer to Node or Node tree*/
#define COPY_NODE_FIELD(fldname)  \
      (newnode->fldname = (copyObject(from->fldname))

/*copy a field that is a pointer to C string or NULL*/
#define COPY_STRING_FIELD(fldname)  \

/*deep copy for List operation*/
#define COPY_NODE_CELL(new, old)   \
  (new) = (ListCell *) malloc(sizeof(ListCell));  \
  lfirst(new) = copyObject(lfirst(old));

static List *copyList(List *from)
{
    List *new;
    ListCell *cur_old;
    ListCell *pre_new;
   
    Assert(list_length(from) >= 1);
    new = makeNode(List);
    new->length = from->length;

    COPY_NODE_CELL(new->head, from->head);
    pre_new = new->head;
    cur_old = lnext(from->head);

    while(cur_old)
       {
          COPY_NODE_CELL(pre_new->next, cur_old);
          pre_new = pre_new->next;
          cur_old = cur_old->next;
       }
       pre_new->next = NULL;
       new->tail = pre_new;
       return new;

}



/*copyObject copy of a Node tree or list and all substructure copied too */
/*this is a deep copy & with recursive*/
extern void *copyNode(void *from);

void *copyObject(void *from)
{
    void *retval;
    
    if(from == NULL)
        return NULL;
   
    switch(nodeTag(from))
     {
        /*different type nodes*/
        
        /*list nodes*/
        case T_List:
             retval = copyList(from);
             break;
        //T_QueryOperator
        //T_SelectionOperator
        //T_ProjectionOperator
        //T_JoinOperator
        //T_AggregationOperator
        //T_ProvenanceComputation
        //T_QueryBlock model
        
        default:
           retval = copyNode(from);
           break;
     }

     return retval;

}








  
