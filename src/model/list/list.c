/*************************************************************************
    > File Name: list.c
    > Author: Shukun Xie
    > Descriptions: implement the functions for list usage.
 ************************************************************************/

#include "common.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"

//#define isPtrList(list)     ((list) == NIL || isA(list, T_List))
//#define isIntList(list)     ((list) == NIL || isA(list, T_IntList))

void
checkList(const List *list)
{
    if (list == NIL)
        return;

    assert(list->length > 0);
    assert(list->head != NULL);
    assert(list->tail != NULL);
    assert(list->type == T_List || list->type == T_IntList);

    if (list->length == 1)
        assert(list->head == list->tail);
    if (list->length == 2)
        assert(list->head->next == list->tail);

    assert(list->tail->next == NULL);
}

boolean
isPtrList(List *list)
{
	if (list == NIL)
		return TRUE;
	if (list->type == T_List)
		return TRUE;
	return FALSE;
}

boolean
isIntList(List *list)
{
	if (list == NIL)
		return TRUE;
	if (list->type == T_IntList)
		return TRUE;
	return FALSE;
}

List *
newList(NodeTag type)
{
    List *newList;
    ListCell *newListHead;

    newListHead = (ListCell *)malloc(sizeof(ListCell));
    newListHead->next = NULL;

    newList = (List *)malloc(sizeof(List));
    newList->type = type;
    newList->length = 1;
    newList->head = newListHead;
    newList->tail = newListHead;

    checkList(newList);

    return newList;
}

int
getListLength(List *list) 
{
	if (list == NIL)
		return 0;
	if (list->length != -1)
		return list->length;

	ListCell *node; 
    
    node = list->head;
	list->length = 0;

	if (node != NULL)
	{
		do
			list->length++;
		while ((node = node->next) != NULL);
	}

	return list->length;
}

ListCell *
getHeadOfList(List *list)
{
    return  list ? list->head : NULL;
}

int
getHeadOfListInt (List *list)
{
	assert(isIntList(list));
    ListCell *head;
    
    head = getHeadOfList(list);
    return head ? head->data.int_value : -1;
}

// If you use a parameter to return a value of a function then it needs to be a pointer to the type you want to return because C is pass-by value. You
// want to return a pointer to a ListCell, so the parameter should be ListCell **element. However, since you do not have any other return value, there
// is no need to have the return value as an argument. Just change the return type to ListCell * and remove the parameter

ListCell *
getTailOfList(List *list)
{
    return list ? list->tail : NULL;
}

int
getTailOfListInt(List *list)
{
    assert(isIntList(list));
    
    ListCell *tail;
    
    tail = getTailOfList(list);
    return tail ? tail->data.int_value : -1;
}

// If you use a parameter to return a value of a function then it needs to be a pointer to the type you want to return because C is pass-by value. You
// want to return a pointer to a ListCell, so the parameter should be ListCell **element. However, since you do not have any other return value, there
// is no need to have the return value as an argument. Just change the return type to ListCell * and remove the parameter

ListCell *
getNthOfList(List *list, int n)
{
	if (list == NIL)
		return NULL;
	assert(getListLength(list) >= n);
    
    int counter = 0;
	ListCell * node;
    
    node = list->head;
	while (node != NULL && counter < n)
	{
		counter++;
		node = node->next;
	}
    
    return node ? node : NULL;
}

// we need convienience wrappers for appending and int or void * value to the end or start of a list 

void
newListTail(List *list)
{
    ListCell *newTail;
    
    newTail = (ListCell *)malloc(sizeof(List));
    newTail->next = NULL;
    
    list->tail->next = newTail;
    list->tail = newTail;
    list->length++;
}

List *
appendToTailOfList(List *list, void *value)
{
    assert(isPtrList(list));
	
    if (list == NIL) 
		list = newList(T_List);
	else
        newListTail(list);
    
    list->tail.data = value;
    assert(checkList(list));
    return list;
}

List *
appendToTailOfListInt(List *list, int value)
{
    assert(isIntList(list));
    
    if (list == NIL)
        list = newList(T_IntList);
    else
        newListTail(list);
    
    list->tail.data = value;
    assert(checkList(list));
    return list;
}

void
newListHead(List *list)
{
    ListCell *newHead;

    newHead = (ListCell *)malloc(sizeof(ListCell));
    newHead->next = list->head;
    list->head = newHead;
    list->length++;
}

List *
appendToHeadOfList(List *list, void *value)
{
    assert(isPtrList(list));

    if (list == NIL)
        list = newList(T_List);
    else
        newListHead(list);
    
    list->head.data = value;
    assert(checkList(list));
    return list;
}

List *
appendToHeadOfListInt (List *list, int value)
{
    assert(isIntList(list));

    if (list == NIL)
        list = newList(T_IntList);
    else
        newListHead(list);

    list->head.data = value;
    assert(checkList(list));
    return list;
}

void
reverseList(List *list)
{
	if (list == NULL || getListLength(list) == 0)
		return;

	ListCell *tail  = list->head;
	ListCell *prev  = list->head;
	ListCell *curr  = list->head->next;

	tail->next = NULL;
	while (curr != NULL)
	{
		ListCell *temp = curr->next;
		curr->next = prev;
		prev = curr;
		curr = temp;
	}
	list->head = prev;
	list->tail = tail;
    assert(checkList(list));
}

void
sortList(List *list)
{

}

List *
copyList(List *list)
{

}

void
freeList(List *list)
{
    if (list == NIL)
        return;
    free(list);
}

void
deepFreeList(List *list)
{
    if (list == NIL)
        return;
    
    ListCell *node;

    node = list->head;
    while (node != NULL)
    {
        ListCell *temp = node;
        node = node->next;
        free(temp);
    }
    free(list);
}

List *
concatTwoLists(List *lista, List*listb)
{
	if (lista == NIL)
		return  listb;
	if (listb == NIL)
		return lista;

    assert(lista->type == listb->type);

	lista->tail->next = listb->head;
	lista->tail = listb->tail;
	lista->length += listb->length; // destroys both original list. listb should consequently be free'd

    assert(checkList(lista));
    freeList(listb);
    return lista;
}

boolean
searchList(List *list, void *value)
{
    assert(isPtrList(list));

    if (list == NIL)
        return FALSE;

    ListCell *lc;

    lc = list->head;
    while (lc != NULL)
    {
        if (lc->data.ptr_value == value)
            return TRUE;
        lc = lc->next;
    }

    return FALSE;
}

boolean
searchListInt(List *list, int value)
{
    assert(isIntList(list));

    if (list == NIL)
		return FALSE;

	ListCell *lc;

    lc = list->head;
    while (lc != NULL)
	{
		if (lc->data.int_value == value)
			return TRUE;
        lc = lc->next;
    }

	return FALSE;
}
