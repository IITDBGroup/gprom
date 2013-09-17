/*************************************************************************
    > File Name: list.c
    > Author: skxie
    > Descriptions: implement the functions for list usage.
    > 
 ************************************************************************/

#include "common.h"
#include "model/list/list.h"

int
getListLength(List *list) // should just return the list length. Empty lists should have length 0 not -1
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

// If you use a parameter to return a value of a function then it needs to be a pointer to the type you want to return because C is pass-by value. You
// want to return a pointer to a ListCell, so the parameter should be ListCell **element. However, since you do not have any other return value, there
// is no need to have the return value as an argument. Just change the return type to ListCell * and remove the parameter

void
getTailOfList(List *list, ListCell *tail)
{
	tail = NULL;
	if (list == NULL)
		return;
	tail = list->tail;
}

// If you use a parameter to return a value of a function then it needs to be a pointer to the type you want to return because C is pass-by value. You
// want to return a pointer to a ListCell, so the parameter should be ListCell **element. However, since you do not have any other return value, there
// is no need to have the return value as an argument. Just change the return type to ListCell * and remove the parameter

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
		if (counter == n) // simpler to not check anything here and just return the current "node" after traversing (need right condition in while)
		{
			element = node;
			return;
		}
		node = node->next;
	}
	// need a return here for case where n >= list->length, but better to check this before looping ;-)
	return NULL;
}

// we need convienience wrappers for appending and int or void * value to the end or start of a list 

void
appendToTailOfList(List *list, ListCell *node) 
{
  if (list == NULL) //create singleton instead
		return;
	if (list->head == NULL)
	  return; // create singleton instead
	ListCell * tail;
	if (list->tail != NULL)
		tail = list->tail;
	else // why if list is not empty tail should always point to the last cell in the list
	{
		ListCell *curr = list->head;
		while (curr->next != NULL)
		{
			curr = curr->next;
		}
		tail = curr;
	}
	tail->next = node;
	list->tail = node;
	list->length++;
}

void
appendToHeadOfList(List *list, ListCell *node)
{
  if (list == NULL) // no should return new list
		return; 
  if (list->head == NULL) // create singleton list from empty list
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
  if (lista == NULL && listb == NULL) // make sure they are of same type
        return;
    if (lista == NULL)
    {
      lista = listb; // wouldn't work if lista = NULL
        return;
    }
    if (listb == NULL)
    {
        return;
    }
    lista->tail->next = listb->head;
    lista->tail = listb->tail;
    lista->length += listb->length; // destroys both original list. listb should consequently be free'd
}

void 
searchList(List *list, )
{
    
}
