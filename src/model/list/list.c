/*************************************************************************
    > File Name: list.c
    > Author: Shukun Xie
    > Descriptions: implement the functions for list usage.
 ************************************************************************/

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/list/list.h"

boolean
checkList(const List *list)
{
    if (list == NIL)
        return TRUE;

    assert(list->length > 0);
    assert(list->head != NULL);
    assert(list->tail != NULL);
    assert(list->type == T_List || list->type == T_IntList);

    if (list->length == 1)
        assert(list->head == list->tail);
    if (list->length == 2)
        assert(list->head->next == list->tail);

    assert(list->tail->next == NULL);

    return TRUE;
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

    newListHead = (ListCell *)MALLOC(sizeof(ListCell));
    newListHead->next = NULL;

    newList = (List *)MALLOC(sizeof(List));
    newList->type = type;
    newList->length = 1;
    newList->head = newListHead;
    newList->tail = newListHead;

    assert(checkList(newList));

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


List *
singletonInt(int value)
{
	List *list;

	list = newList(T_List);
	list->head->data.int_value = value;

	return list;
}

List *
singleton(void *value)
{
	List *list;

	list = newList(T_List);
	list->head->data.ptr_value = value;

	return list;
}

void
newListTail(List *list)
{
    ListCell *newTail;
    
    newTail = (ListCell *)MALLOC(sizeof(ListCell));
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
    
    list->tail->data.ptr_value = value;
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
    
    list->tail->data.int_value = value;
    assert(checkList(list));
    return list;
}

void
newListHead(List *list)
{
    ListCell *newHead;

    newHead = (ListCell *)MALLOC(sizeof(ListCell));
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
    
    list->head->data.ptr_value = value;
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

    list->head->data.int_value = value;
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
	List *listCopy;

	listCopy = newList(list->type);

	return NULL; /* keep compiler quiet for now */
}

void
freeList(List *list)
{
    if (list == NIL)
        return;
    FREE(list);
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
        FREE(temp);
    }
    FREE(list);
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
	lista->length += listb->length;

    assert(checkList(lista));
    deepFreeList(listb);
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
