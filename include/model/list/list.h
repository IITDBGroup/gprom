/*************************************************************************
	> File Name: list.h
	> Author: Shukun Xie
    > Descriptions: Head file for the list.c
 ************************************************************************/

typedef struct ListCell
{
    union
    {
        void *ptr_value;
        int  int_value;
    } data;
    ListCell *next;
} ListCell;

typedef struct List
{
    NodeTag type;
    int     length;
    ListCell *head;
    ListCell *tail;
} List;

#define NIL ((List *)NULL)

extern void checkList(const List *list);

extern List *newList(NodeTag type);

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

extern bool searchList(List *list, void *value);
extern bool searchListInt(List *list, int value);

