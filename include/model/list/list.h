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
#define LIST_LENGTH(l) ((l == NULL) ? 0 : ((List *) l)->length)
#define MY_LIST_EMPTY(l) (LIST_LENGTH(l) == 0)

/*
 * Loop through list _list_ and access each element of type _type_ using name
 * _node_. _cell_ has to be an existing variable of type ListCell *.
 */
#define DUMMY_INT_FOR_COND(_name_) _name_##_stupid_int_
#define DUMMY_LC(_name_) _name_##_his_cell
#define INJECT_VAR(type,name) \
	for(int DUMMY_INT_FOR_COND(name) = 0; DUMMY_INT_FOR_COND(name) == 0;) \
		for(type name = NULL; DUMMY_INT_FOR_COND(name) == 0; DUMMY_INT_FOR_COND(name)++) \

#define FOREACH_LC(lc,list) \
	for(ListCell *lc = getHeadOfList(list); lc != NULL; lc = lc->next)

#define FOREACH(_type_,_node_,_list_) \
    INJECT_VAR(ListCell*,DUMMY_LC(_node_)) \
    for(_type_ *_node_ = (_type_ *)(((DUMMY_LC(_node_) = \
    		getHeadOfList(_list_)) != NULL) ? \
                    DUMMY_LC(_node_)->data.ptr_value : NULL); \
            DUMMY_LC(_node_) != NULL; \
           _node_ = (_type_ *)(((DUMMY_LC(_node_) = \
                    DUMMY_LC(_node_)->next) != NULL) ? \
                    DUMMY_LC(_node_)->data.ptr_value : NULL))

#define FOREACH_GET_LC(_node_) (DUMMY_LC(_node_))
#define FOREACH_HAS_MORE(_node_) (DUMMY_LC(_node_)->next != NULL)
#define FOREACH_IS_FIRST(_node_,_list_) (DUMMY_LC(_node_) == _list_->head)

/*
 * Loop through integer list _list_ and access each element using name _ival_.
 * _cell_ has to be an existing variable of type ListCell *.
 */
#define FOREACH_INT(_ival_,_list_) \
    INJECT_VAR(ListCell*,DUMMY_LC(_ival_)) \
	for(int _ival_ = (((DUMMY_LC(_ival_) = getHeadOfList(_list_)) != NULL)  ? \
                        DUMMY_LC(_ival_)->data.int_value : -1); \
                DUMMY_LC(_ival_) != NULL; \
                _ival_ = (((DUMMY_LC(_ival_) = DUMMY_LC(_ival_)->next) != NULL) ? \
                        DUMMY_LC(_ival_)->data.int_value: -1))

/*
 * Loop through the cells of two lists simultaneously
 */
#define FORBOTH_LC(lc1,lc2,l1,l2) \
    for(ListCell *lc1 = getHeadOfList(l1), *lc2 = getHeadOfList(l2); lc1 != NULL && lc2 != NULL; \
            lc1 = lc1->next, lc2 = lc2->next)

/*
 * Loop through lists of elements with the same type simultaneously
 */
#define FORBOTH(_type_,_node1_,_node2_,_list1_,_list2_) \
	INJECT_VAR(ListCell*,DUMMY_LC(_node1_)) \
	INJECT_VAR(ListCell*,DUMMY_LC(_node2_)) \
    for(_type_ *_node1_ = (_type_ *)(((DUMMY_LC(_node1_) = \
    		getHeadOfList(_list1_)) != NULL) ? \
                    DUMMY_LC(_node1_)->data.ptr_value : NULL), \
        *_node2_ = (_type_ *)(((DUMMY_LC(_node2_) = \
        	getHeadOfList(_list2_)) != NULL) ? \
            		DUMMY_LC(_node2_)->data.ptr_value : NULL) \
        ; \
            DUMMY_LC(_node1_) != NULL && DUMMY_LC(_node2_) != NULL; \
           _node1_ = (_type_ *)(((DUMMY_LC(_node1_) = \
                    DUMMY_LC(_node1_)->next) != NULL) ? \
                    DUMMY_LC(_node1_)->data.ptr_value : NULL), \
           _node2_ = (_type_ *)(((DUMMY_LC(_node2_) = \
                    DUMMY_LC(_node2_)->next) != NULL) ? \
                    DUMMY_LC(_node2_)->data.ptr_value : NULL))


/*
 * Loop through two integer lists simultaneously
 */
#define FORBOTH_INT(_ival1_,_ival2_,_list1_,_list2_) \
    INJECT_VAR(ListCell*,DUMMY_LC(_ival1_)) \
    INJECT_VAR(ListCell*,DUMMY_LC(_ival2_)) \
    for(int _ival1_ = (((DUMMY_LC(_ival1_) = \
            (_list1_)->head) != NULL) ? \
                    DUMMY_LC(_ival1_)->data.int_value : -1), \
        _ival2_ = (((DUMMY_LC(_ival2_) = \
            (_list2_)->head) != NULL) ? \
                    DUMMY_LC(_ival2_)->data.int_value : -1) \
        ; \
            DUMMY_LC(_ival1_) != NULL && DUMMY_LC(_ival2_) != NULL; \
           _ival1_ = (((DUMMY_LC(_ival1_) = \
                    DUMMY_LC(_ival1_)->next) != NULL) ? \
                    DUMMY_LC(_ival1_)->data.int_value : -1), \
           _ival2_ = (((DUMMY_LC(_ival2_) = \
                    DUMMY_LC(_ival2_)->next) != NULL) ? \
                    DUMMY_LC(_ival2_)->data.int_value : -1))

// map over list elements with anamorphic expression
#define MAP_LIST(_list_,_expr_)											\
	do {																\
		List *_result_ = newList(T_List);								\
		for(ListCell *_lc_ = _list_->head; _lc_ != NULL; lc = lc->next)	\
		{																\
			void *it = _lc_->data.ptr_value;							\
			_lc_->data.ptr_value = (_expr_);							\
		}																\
    } while(0)

/*
 * Get pointer of integer value of a list cell
 */
#define LC_P_VAL(lc) (((ListCell *) lc)->data.ptr_value)
#define LC_STRING_VAL(lc) ((char *) ((ListCell *) lc)->data.ptr_value)
#define LC_INT_VAL(lc) (((ListCell *) lc)->data.int_value)
#define LC_NEXT(lc) (((ListCell *) lc)->next)
#define LC_ADVANCE(lc) \
    do {    \
        lc = lc->next;  \
    } while(0)
/*
 * Create a integer list starting from _start to _end increasing _step
 */
#define CREATE_INT_SEQ(_result, _start, _end, _step) \
    do { \
    	_result = NIL; \
        for(int _my_var = (_start), _my_end = (_end), _my_step = (_step); _my_var <= _my_end; _my_var += _my_step) \
            _result = appendToTailOfListInt(_result, _my_var); \
    } while(0)

#define SHIFT_INT_LIST(_list,_shift) \
    do { \
        FOREACH_LC(_lc,_list) \
            LC_INT_VAL(_lc) += _shift; \
    } while (0)

extern boolean checkList(const List *list);

/* list creation */
extern List *newList(NodeTag type);

extern List *singletonInt(int value);
extern List *singleton(void *value);
#define LIST_MAKE(...) listMake(__VA_ARGS__, NULL)
#define LIST_MAKE_INT(...) listMakeInt(__VA_ARGS__, -1)
extern List *listMake(void *elem, ...);
extern List *listMakeInt(int elem, ...);

/* length of list */
extern int getListLength(List *list);

/* get certain elements */
extern ListCell *getHeadOfList(List *list);
extern int getHeadOfListInt(List *list);
extern int popHeadOfListInt(List *list);
extern void *getHeadOfListP(List *list);
extern void *popHeadOfListP(List *list);
extern ListCell *popHeadOfList(List *list);

extern ListCell *getTailOfList(List *list);
extern void *getTailOfListP (List *list);
extern int getTailOfListInt(List *list);
extern void *popTailOfListP(List *list);
extern ListCell *popTailOfList(List *list);


extern void *getNthOfListP(List *list, int n);
extern int getNthOfListInt(List *list, int n);
extern ListCell *getNthOfList(List *list, int n);

/* append to head or tail of list */
extern void newListTail(List *list);
extern List *appendToTailOfList(List *list, void *value);
extern List *appendToTailOfListInt(List *list, int value);
extern List *appendAllToTail(List *l, List *new);

extern void newListHead(List *list);
extern List *appendToHeadOfList(List *list, void *value);
extern List *appendToHeadOfListInt (List *list, int value);

/* sort and reverse list */
extern void reverseList(List *list);
extern List *sortList(List *list, int (*sm) (const void **, const void **));
extern List *unique(List *list, int (*cmp) (const void **, const void **));

/* copy and free lists */
extern List *copyList(List *list);
extern List *deepCopyStringList (List *list);
extern void freeList(List *list);
extern void deepFreeList(List *list);
extern void deepFreeStringList(List *list);
extern List *stringListToConstList(List *list);
extern List *constStringListToStringList(List *list);

/* compare lists */
extern boolean equalStringList (List *a, List *b);

/* search for elements in list */
extern boolean searchList(List *list, void *value);
extern boolean searchListInt(List *list, int value);
extern boolean searchListNode(List *list, Node *value);
extern boolean searchListString(List *list, char *value);
extern boolean genericSearchList(List *list, boolean (*eq) (void *, void *), void *value);

extern int listPosString (List *list, char *value);
extern int genericListPos (List *list, boolean (*eq) (void *, void *), void *value);
extern int listPosInt (List *list, int val);
#define LIST_POS_NODE(_l,_val) genericListPos(_l,equal,_val);

/* replace list elements */
extern List *replaceNode(List *list, void *n1, void *n2);

/* remove element from list */
extern List *removeListElementsFromAnotherList(List *l1, List *l2);
extern List *genericRemoveFromList (List *list, boolean (*eq) (void *, void *), void *value);
extern List *removeFromListInt(List *l, int el);
extern List *removeFromTail(List *X);
extern List *removeFromHead(List *X);
extern List *removeListElemAtPos (List *list, int pos);
#define REMOVE_FROM_LIST_PTR(list,ptr) genericRemoveFromList (list, ptrEqual, ptr)
#define REMOVE_FROM_LIST_NODE(list,node) genericRemoveFromList (list, equal, node)

/* concatenate lists and get sublists */
extern List *concatTwoLists (List *listA, List *listB);
extern List *concatLists (List *a, ...);
#define CONCAT_LISTS(...) concatLists(__VA_ARGS__, NULL)
extern List *sublist(List *l, int from, int to);
extern List *genericSublist(List *l, boolean (*pred) (void *, void *), void *context);

/* higher-order functions */
extern List *mapList(List *, void * (*f) (void *));
extern List *mapToIntList(List *, int (*f) (void *));

/* serialize to string */
extern char *stringListToString (List *node);
extern uint64_t hashStringList(uint64_t cur, List *node);


#endif /* LIST_H */
