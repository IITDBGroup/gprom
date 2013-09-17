#ifndef LIST_H
#define LIST_H

#include "nodetype.h"

typedef struct ListCell ListCell;
typedef struct List{
NodeTag type;
int length;
ListCell *head;
ListCell *tail;

}List;

struct ListCell{

union

   {
      void *ptr_value;
      int  value;

   }data;

ListCell *next;

};

#define NIL ((List*) NULL)
#ifdef /*GNUC*/

static inline ListCell *
list_head(List *l)
{
return l?l->head :NULL;
}

static inline ListCell *
list_tail(List *l)
{
return l?l->tail : NULL;
}

static inline int
list_length(List *l)
{
return l?l->length : 0;
}

#else

extern ListCell *list_head(List *l);
extern ListCell *list_tail(List *l);
extern int *list_length(List *l);

#endif   /*GNUC*/
