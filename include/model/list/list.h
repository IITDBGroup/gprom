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

extern boolean checkList(const List *list);

extern List *newList(NodeTag type);

extern List *singletonInt(int value);
extern List *singleton(void *value);

extern int getListLength(List *list);

extern ListCell *getHeadOfList(List *list);
extern int getHeadOfListInt (List *list);

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
