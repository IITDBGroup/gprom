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


/*
 * Loop through list _list_ and access each element of type _type_ using name
 * _node_. _cell_ has to be an existing variable of type ListCell *.
 */
#define DUMMY_INT_FOR_COND(_name_) _name_##_stupid_int_
#define DUMMY_LC(_name_) _name_##_his_cell
#define INJECT_VAR(type,name) \
	for(int DUMMY_INT_FOR_COND(name) = 0; DUMMY_INT_FOR_COND(name) == 0; DUMMY_INT_FOR_COND(name)++) \
		for(type name = NULL; DUMMY_INT_FOR_COND(name) == 0;) \

#define FOREACH(_type_,_node_,_list_) \
    INJECT_VAR(ListCell*,DUMMY_LC(_node_)) \
        for(_type_ *_node_ = (_type_ *)(((DUMMY_LC(_node_) = \
        		(_list_)->head) != NULL) ? \
        				DUMMY_LC(_node_)->data.ptr_value : NULL); \
        		DUMMY_LC(_node_) != NULL; \
               _node_ = (_type_ *)(((DUMMY_LC(_node_) = \
                        DUMMY_LC(_node_)->next) != NULL) ? \
                        DUMMY_LC(_node_)->data.ptr_value : NULL))

/*
 * Loop through integer list _list_ and access each element using name _ival_.
 * _cell_ has to be an existing variable of type ListCell *.
 */
#define FOREACH_INT(_ival_,_list_) \
    INJECT_VAR(ListCell*,DUMMY_LC(_ival_)) \
	    for(int _ival_ = (((DUMMY_LC(_ival_) = (_list_)->head) != NULL)  ? \
                            DUMMY_LC(_ival_)->data.int_value : -1); \
                    DUMMY_LC(_ival_) != NULL; \
                    _ival_ = (((DUMMY_LC(_ival_) = DUMMY_LC(_ival_)->next) != NULL) ? \
                            DUMMY_LC(_ival_)->data.int_value: -1))

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
