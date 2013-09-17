/*************************************************************************
    > File Name: list.c
    > Author: skxie
    > Descriptions: implement the functions for list usage.
    > 
 ************************************************************************/

#include "model/list/list.h"

int
getListLength(List *list)
{
	if (list == NULL)
		return -1;
	if (list->length != -1)
		return list->length;
	ListCell * node = list->head;
	int len = 0;
	while (node != NULL)
	{
		node = node->next;
		len++;
	}
	list->length = len;
	return list->length;
}

void
getHeadOfList(List *list, ListCell *head)
{
	head = NULL;
	if (list == NULL)
		return;
	head = list->head;
}

void
getTailOfList(List *list, ListCell *tail)
{
	tail = NULL;
	if (list == NULL)
		return;
	tail = list->tail;
}

void
getNthOfList(List *list, int n, ListCell *element)
{
	element = NULL;
	if (list == NULL)
		return;
	int counter = 0;
	ListCell * node = list->head;
	while (node != NULL && counter < n)
	{
		counter++;
		if (counter == n)
		{
			element = node;
			return;
		}
		node = node->next;
	}
}

void
appendToTailOfList(List *list, ListCell *node)
{
	if (list == NULL)
		return;
	if (list->head == NULL)
		return;
	ListCell * tail;
	if (list->tail != NULL)
		tail = list->tail;
	else
	{
		ListCell *curr = list->head;
		while (curr->next != NULL)
		{
			curr = curr->next;
		}
		tail = curr;
	}
	tail->next = node;
	tail = tail->next;
	list->tail = tail;
	list->length++;
}

void
appendToHeadOfList(List *list, ListCell *node)
{
	if (list == NULL)
		return;
	if (list->head == NULL)
		return;
	ListCell * head = list->head;
	node->next = head;
	head = node;
	list->head = head;
	list->length++;
}

void
reverseList(List *list)
{
	if (list == NULL)
		return;
	if (list->head == NULL || list->length <= 1)
		return;
    ListCell *tail  = list->head;
    ListCell *prev  = list->head;
    ListCell *curr  = list->head->next;
    tail->next      = NULL;
    while (curr != NULL)
    {
        ListCell *temp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = temp;
    }
    list->head = prev;
    list->tail = tail;
}

void
sortList(List *list)
{
    
}

void
copyList(List *lista, List *listb)
{
    
}

void
freeList(List *list)
{
    
}

void
deepFreeList(List *list)
{

}

void 
concatenateTwoLists(List *lista, List*listb)
{
    if (lista == NULL && listb == NULL)
        return;
    if (lista == NULL)
    {
        lista = listb;
        return;
    }
    if (listb == NULL)
    {
        return;
    }
    lista->tail->next = listb->head;
    lista->tail = listb->tail;
    lista->length += listb->length;
}

void 
searchList(List *list, )
{
    
}
