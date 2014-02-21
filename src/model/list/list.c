/*************************************************************************
    > File Name: list.c
    > Author: Shukun Xie
    > Descriptions: implement the functions for list usage.
 ************************************************************************/

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"

boolean
checkList(const List *list)
{
    int realLength = 0;

    if (list == NIL)
        return TRUE;

    assert(list->length > 0);
    assert(list->head != NULL);
    assert(list->tail != NULL);
    assert(list->type == T_List || list->type == T_IntList);

    for(ListCell *lc = list->head; lc != NULL; lc = lc->next)
        realLength++;

    assert(realLength == list->length);

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

    newListHead = (ListCell *) NEW(ListCell);
    newListHead->next = NULL;

    newList = (List *) NEW(List);
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

void *
getHeadOfListP (List *list)
{
    assert(isPtrList(list));
    ListCell *head;

    head = getHeadOfList(list);
    return head ? head->data.ptr_value : NULL;
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

void *
getNthOfListP(List *list, int n)
{
    return getNthOfList(list,n)->data.ptr_value;
}

int
getNthOfListInt(List *list, int n)
{
    return getNthOfList(list,n)->data.int_value;
}


ListCell *
getNthOfList(List *list, int n)
{
	if (list == NIL)
		return NULL;
	assert(getListLength(list) >= n);
    
	ListCell * node;
    
    node = list->head;
	while (node != NULL && n--)
	{
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

List *
listMake(void *elem, ...)
{
    List *result = singleton(elem);
    int n = 0;
    va_list args;
    void *p;

    va_start(args, elem);

    while((p = va_arg(args, void *)))
        result = appendToTailOfList(result, p);

    va_end(args);

    return result;
}

void
newListTail(List *list)
{
    ListCell *newTail;
    
    newTail = (ListCell *) NEW(ListCell);
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

    newHead = (ListCell *) NEW(ListCell);
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

List *
replaceNode(List *list, void *n1, void *n2)
{
	//if (!(list->type == ((Node*)n1)->type && list->type == ((Node*)n2)->type))
		//return list;

	ListCell *dummy = (ListCell *) NEW(ListCell);
	dummy->next = list->head;

	while (dummy->next != NULL) {
		if (dummy->next->data.ptr_value == n1) {
			dummy->next->data.ptr_value = n2;
			break;
		}
	}

	ListCell *head = dummy->next;
	FREE(dummy);

	list->head = head;
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
	List *listCopy = NIL;

	if (list == NULL)
	    return NULL;

	FOREACH_LC(lc, list)
	{
	    if (list->type == T_List)
	        listCopy = appendToTailOfList(listCopy, LC_P_VAL(lc));
	    else
	        listCopy = appendToTailOfListInt(listCopy, LC_INT_VAL(lc));
	}

	return listCopy; /* keep compiler quiet for now */
}

List *
deepCopyStringList (List *list)
{
    List *listCopy = NIL;

    FOREACH(char,c, list)
        listCopy = appendToTailOfList(listCopy, strdup(c));

    return listCopy;
}

void
freeList(List *list)
{
    if (list == NIL)
        return;

    for(ListCell *lc = list->head->next, *prev = list->head; lc != NULL;
            prev = lc, lc = lc->next)
        FREE(prev);

    FREE(list);
}

void
deepFreeList(List *list)
{
    if (list == NIL)
        return;
    
    for(ListCell *lc = list->head->next, *prev = list->head; lc != NULL;
            prev = lc, lc = lc->next)
    {
        deepFree(LC_P_VAL(lc));
        FREE(prev);
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
    FREE(listb);
    return lista;
}

boolean
searchList(List *list, void *value)
{
    assert(isPtrList(list));

    FOREACH(void,item,list)
    {
        if (item == value)
            return TRUE;
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

boolean
searchListString(List *list, char *value)
{
    return listPosString(list, value) != -1;
}

boolean
searchListNode(List *list, Node *value)
{
    assert(isPtrList(list));

    FOREACH(Node,item,list)
    {
        if (equal(item,value))
            return TRUE;
    }

    return FALSE;
}

boolean
genericSearchList(List *list, boolean (*eq) (void *, void *), void *value)
{
    return genericListPos(list, eq, value) != -1;
}


boolean
genericListPos (List *list, boolean (*eq) (void *, void *), void *value)
{
    assert(isPtrList(list));
    int pos = 0;

    FOREACH(void,item,list)
    {
        if (eq(item, value))
            return pos;
        pos++;
    }

    return -1;
}


int
listPosString (List *list, char *value)
{
    assert(isPtrList(list));
    int pos = 0;

    FOREACH(char,item,list)
    {
        if (strcmp(item,value) == 0)
            return pos;
        pos++;
    }

    return -1;
}

List *
genericRemoveFromList (List *list, boolean (*eq) (void *, void *), void *value)
{
    assert(isPtrList(list));
    List *result = NULL;
    ListCell *prev = getHeadOfList(list);

    FOREACH_LC(lc,list)
    {
        void *ptrVal = LC_P_VAL(lc);

        if(!eq(ptrVal, value))
            result = appendToTailOfList(result, ptrVal);
    }

    freeList(list);

    return result;
}
