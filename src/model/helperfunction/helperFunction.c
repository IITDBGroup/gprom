/*************************************************************************
 	
     File Name: helperFunction.c
 	
	
 	
     Descriptions: implement basic function for tostring(), deepcopy(), deepequal().
 	
	
 	
 ************************************************************************/

#include "common.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"



/*********************/
/*deepequal function*/
/********************/
#define COMPARE_SCALAR_FIFLD(fldname)
 {
     if(a->fldname!=b->fldname) 
      return false;
 }
/*compare a simple scalar field(int, float, bool, etc)*/

#define COMPARE_NODE_FIELD(fldname)
 {
     if(!equal(a->fldname, b->fldname)) 
      return false;
 }
/*compare a field that is a pointer to some kind of Node or Node tree*/

#define COMPARE_BITMAPSET_FIELD(fldname)
 {
     if(!bms_equal(a->fldname, b->fldname)) 
      return false;
 }
/*compare a field that is a pointer to a Bitmapset*/

#define COMPARE_STRING_FIELD(fldname)
 {
     if(!equalstr(a->fldname, b->fldname)) 
      return false;
 }
/*compare a field that is a pointer to a C string, or maybe NULL*/




/*function for equalList*/

static bool equalList(List *a, List *b)
{
     ListCell *item_a;
     ListCell *item_b;

     COMPARE_SCALAR_FIELD(type);
     COMPARE_SCALAR_FIELD(length);

     switch (a->type)
    {
       case T_List:
            forboth(item_a, a, item_b, b)
            {
                 if(!equal(lfirst(item_a)), lfirst(item_b)))
                    return false;
            }
            break;
       default:
          return false;
    }
/*if we done, should able to run both lists*/

Assert(item_a == NULL);
Assert(item_b == NULL);
 
return true;  
}

static bool equalValue(Value *a, Value *b)
{
      COMPARE_SCALAR_FIELD(type);

      switch(a->type)
       {
         case T_Integer:
            COMPARE_SCALAR_FIELD();
            break;
         default:
            break;
       }

       return true;

}

 

/*equal returns whether two nodes are equal*/

bool equal(void *a, void *b)
{
   bool retval;
   if (a == b)
      return true;

   if(a == NULL || b == NULL)
      return false;

   switch(nodeTag(a))
     {
         case T_Const:
         retval = equalConst(a,b);
         break;

         case T_List:
         retval = equalList(a,b);
         break;

         default:
         retval = provNodesEquals(a,b);
         break;
     }
     
     return retval;

}

/*********************/
/*deepcopy function*/
/********************/

#define COPY_NODE_CELL(new, old)
(new) =(ListCell *)malloc(sizeof(ListCell));
lfirst(new) = copyObject(lfirst(old));

#define COPY_NODE_FIELD(fldname)

static List *copyList(List *from)
{
      List *new;
      ListCell *curr_old;
      ListCell *prev_new;
      
      Assert(list_length(from) >= 1 );
      new = makeNode(List);
      new->length = from->length;
      
      COPY_NODE_CELL(new->head, from->head);
      prev_new = new->head;
      curr_old = lnext(from->head);

      while(curr_old)
      {
             COPY_NODE_CELL(prev_new->next, curr_old);
             prev_new = prev_new->next;
	     curr_old = curr_old->next;
      }

      prev_new->next = NULL;
      new->tail = prev_new;

      return new;

}

void * copyObject(void *from)
{
     void *retval;
     
     if(from == NULL)
     return NULL;
     
     switch(nodeTag(from))
         {
            case T_List:
            retval = copyList(from);
            break;



         }

}

/********************/
/*toString function*/
/*******************/
typedef struct StringInfoData
{
   char *data;
   int len;
   int maxlen;
   int cursor;

}StringInfoData;

typedef StringInfoData *StringInfo;

extern void outNode(StringInfo str, void *obj);
extern void initStringInfo(StringInfo str);

char *nodeToString(void *obj)
{
      StringInfoData str;
      initStringInfo(&str);
      outNode(&str, obj);
      return str.data;
}
