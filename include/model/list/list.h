/*************************************************************************
	> File Name: list.h
	> Author: Shukun Xie
    > Descriptions: Head file for the list.c
 ************************************************************************/

#ifndef LIST_H
#define LIST_H

#include "common.h"
#include "model/node/nodetype.h"

typedef struct ListCell
{
    union
    {
        void *ptr_value;
        int  int_value;
    } data;
    struct ListCell *next;
} ListCell;

typedef struct List
{
    NodeTag type;
    int     length;
    ListCell *head;
    ListCell *tail;
} List;

#define NIL ((List *)NULL)

#define FOREACH_NODE(_type_,_node_,_list_) \
	{ \
	    ListCell *__cell__; \
	    _type_ *_node_; \
	    for(__cell__ = (_list_)->head; __cell__ != NULL; __cell__ = __cell__->next) \
	    {
	       // _node_ = __cell__->data.ptr_value;

#define FOREACH_INT(_list_, _ival_) \
    { \
        ListCell *__cell__; \
        int _ival_; \
        for(__cell__ = (_list_)->head; __cell__ != NULL; __cell__ = __cell__->next) \
        { \
           _ival_ = __cell__->data.int_value;

#define ENDFOR }}

extern boolean checkList(const List *list);

extern List *newList(NodeTag type);

extern List *singletonInt(int value);
extern List *singleton(void *value);

extern int getListLength(List *list);

extern ListCell *getHeadOfList(List *list);
extern int getHeadOfListInt (List *list);
extern void *getHeadOfListP (List *list);

extern ListCell *getTailOfList(List *list);
extern int getTailOfListInt(List *list);

extern ListCell *getNthOfList(List *list, int n);

extern void newListTail(List *list);
extern List *appendToTailOfList(List *list, void *value);
extern List *appendToTailOfListInt(List *list, int value);

extern void newListHead(List *list);
extern List *appendToHeadOfList(List *list, void *value);
extern List *appendToHeadOfListInt (List *list, int value);

extern void reverseList(List *list);

extern void sortList(List *list);

extern List *copyList(List *list);

extern void freeList(List *list);
extern void deepFreeList(List *list);

extern boolean searchList(List *list, void *value);
extern boolean searchListInt(List *list, int value);

#endif /* LIST_H */
