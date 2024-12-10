/*************************************************************************
    > File Name: list.c
    > Author: Shukun Xie
    > Descriptions: implement the functions for list usage.
 ************************************************************************/

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/expression/expression.h"
#include "model/set/set.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"


boolean
checkList(const List *list)
{
    int realLength = 0;

    if (list == NIL)
        return TRUE;

    ASSERT(list->length > 0);
    ASSERT(list->head != NULL);
    ASSERT(list->tail != NULL);
    ASSERT(list->type == T_List || list->type == T_IntList);

    for(ListCell *lc = list->head; lc != NULL; lc = lc->next)
        realLength++;

    ASSERT(realLength == list->length);

    if (list->length == 1)
        ASSERT(list->head == list->tail);
    if (list->length == 2)
        ASSERT(list->head->next == list->tail);

    ASSERT(list->tail->next == NULL);

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

    ASSERT(checkList(newList));

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
    ASSERT(isIntList(list));
    ListCell *head;

    head = getHeadOfList(list);
    return head ? head->data.int_value : -1;
}

int
popHeadOfListInt(List *list)
{
    ListCell *head;
    int result = -1;

    ASSERT(isIntList(list) && LIST_LENGTH(list) > 0);
    head = popHeadOfList(list);

    if (head != NULL)
    {
        result = head->data.int_value;
        FREE(head);
    }

    return result;
}

void *
getHeadOfListP (List *list)
{
    ASSERT(isPtrList(list));
    ListCell *head;

    head = getHeadOfList(list);
    return head ? head->data.ptr_value : NULL;
}

ListCell *
popHeadOfList(List *list)
{
    ListCell *result = LIST_LENGTH(list) > 0 ? list->head : NULL;

    if (LIST_LENGTH(list) > 0)
    {
        list->length--;
        list->head = result->next;
        if(list->head == NULL)
        {
            list->tail = NULL;
        }
    }

    return result;
}

void *
popHeadOfListP (List *list)
{
    ASSERT(isPtrList(list));
    ListCell *head;
    void *result = NULL;

    head = popHeadOfList(list);
    if (head != NULL)
    {
        result = head->data.ptr_value;
        FREE(head);
    }

    return result;
}


ListCell *
getTailOfList(List *list)
{
    return list ? list->tail : NULL;
}

void *
getTailOfListP (List *list)
{
    ListCell *tail;
    ASSERT(isPtrList(list));

    tail = getTailOfList(list);

    return tail ? tail->data.ptr_value : NULL;
}

int
getTailOfListInt(List *list)
{
    ASSERT(isIntList(list));

    ListCell *tail;

    tail = getTailOfList(list);
    return tail ? tail->data.int_value : -1;
}

ListCell *
popTailOfList(List *list)
{
    ListCell *result = LIST_LENGTH(list) > 0 ? list->tail : NULL;
	ListCell *prev = list->head;

	// find cell before tail
	for(; prev->next && prev->next != list->tail; prev = prev->next)
	{

	}

    if (LIST_LENGTH(list) > 0)
    {
        list->length--;
        if(LIST_LENGTH(list) == 1)
        {
            list->head = list->tail = NULL;
        }
		else
		{
			list->tail = prev;
			prev->next = NULL;
		}

    }

    return result;
}

void *
popTailOfListP (List *list)
{
    ASSERT(isPtrList(list));
    ListCell *tail;
    void *result = NULL;

    tail = popTailOfList(list);
    if (tail != NULL)
    {
        result = tail->data.ptr_value;
        FREE(tail);
    }

    return result;
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
    ASSERT(getListLength(list) >= n);

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

    list = newList(T_IntList);
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
    va_list args;
    void *p;

    va_start(args, elem);

    while((p = va_arg(args, void *)))
        result = appendToTailOfList(result, p);

    va_end(args);

    return result;
}

List *
listMakeInt(int elem, ...)
{
    List *result = singletonInt(elem);
    va_list args;
    int p;

    va_start(args, elem);

    while((p = va_arg(args, int)) != -1)
        result = appendToTailOfListInt(result, p);

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
    ASSERT(isPtrList(list));

    if (list == NIL || list->length == 0)
    {
        list = newList(T_List);
    }
    else
    {
        newListTail(list);
    }

    list->tail->data.ptr_value = value;

    ASSERT(checkList(list));
    return list;
}

List *
appendToTailOfListInt(List *list, int value)
{
    ASSERT(isIntList(list));

    if (list == NIL)
        list = newList(T_IntList);
    else
        newListTail(list);

    list->tail->data.int_value = value;
    ASSERT(checkList(list));
    return list;
}

List *
appendAllToTail(List *l, List *new)
{
    if(MY_LIST_EMPTY(l))
    {
        return new;
    }
    if(MY_LIST_EMPTY(new))
    {
        return l;
    }

    return CONCAT_LISTS(l,new);
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
    ASSERT(isPtrList(list));

    if (list == NIL)
        list = newList(T_List);
    else
        newListHead(list);

    list->head->data.ptr_value = value;
    ASSERT(checkList(list));
    return list;
}

List *
appendToHeadOfListInt (List *list, int value)
{
    ASSERT(isIntList(list));

    if (list == NIL)
        list = newList(T_IntList);
    else
        newListHead(list);

    list->head->data.int_value = value;
    ASSERT(checkList(list));
    return list;
}

List *
replaceNode(List *list, void *n1, void *n2)
{
    FOREACH_LC(lc,list)
    {
        if (lc->data.ptr_value == n1)
            lc->data.ptr_value = n2;
    }

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
    ASSERT(checkList(list));
}

List *
sortList(List *list, int (*sm) (const void **, const void **))
{
    int numE = LIST_LENGTH(list);
    ListCell *lc;
    List *result = NIL;
    if (list == NIL)
        return NIL;

    lc = list->head;
    // create array for quicksort
    void **arr = CALLOC(sizeof(void *), numE);
    for(int i = 0; i < numE; i++, lc = lc->next)
        arr[i] = LC_P_VAL(lc);

    // using stdlib quicksort
    qsort(arr, numE, sizeof(void*), (int (*)(const void *, const void *)) sm);

    // result list construction
    for(int i = 0; i < numE; i++)
        result = appendToTailOfList(result, arr[i]);

    return result;
}

extern List *
unique(List *list, int (*cmp) (const void **, const void **))
{
    List *sorted = sortList(list, cmp);
    List *result = NIL;
    const void *prev = NULL;

    FOREACH(const void,n,sorted)
    {
	    TRACE_LOG("compare prev: %s, cur: %s, cmp = %u",
				  nodeToString((Node *) prev),
				  nodeToString((Node *) n),
				  cmp(&prev, &n));
        if(prev == NULL || cmp(&prev, &n) != 0)
        {
            result = appendToTailOfList(result, (void *) n);
        }
        prev = n;
    }

    return result;
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

void
deepFreeStringList(List *list)
{
    if (list == NIL)
        return;

    for(ListCell *lc = list->head->next, *prev = list->head; lc != NULL;
                prev = lc, lc = lc->next)
    {
        FREE(LC_P_VAL(lc));
        FREE(prev);
    }

    FREE(list);
}

List *
stringListToConstList(List *list)
{
    List *result = NIL;

    FOREACH(char,el,list)
        result = appendToTailOfList(result, createConstString(el));

    return result;
}

List *
constStringListToStringList(List *list)
{
    List *result = NIL;

    FOREACH(Constant,el,list)
        result = appendToTailOfList(result, STRING_VALUE(el));

    return result;
}

List *
concatTwoLists(List *lista, List*listb)
{
    if (lista == listb)
    {
        listb = copyList(listb);
    }

    if (lista == NIL)
    {
        return  listb;
    }
    if (listb == NIL)
    {
        return lista;
    }

    ASSERT(lista->type == listb->type);

    lista->tail->next = listb->head;
    lista->tail = listb->tail;
    lista->length += listb->length;

    ASSERT(checkList(lista));
    FREE(listb);
    return lista;
}

List *
concatLists (List *a, ...)
{
    va_list args;
    List *result = a;
    List *cur;

    va_start(args, a);

    while((cur = va_arg(args, List *)))
         result = concatTwoLists(result, cur);

    va_end(args);

    ASSERT(checkList(result));

    return result;
}

/**
 * @brief Return a sublist of a list
 *
 * This creates a shallow copy. Like in python, negative numbers are interpreted
 * as starting from the end of the list, e.g., -1 is the last element.
 *
 * @param l the list to return a sublist from
 * @param from starting position (inclusive)
 * @param to stopping position (inclusive)
 * @return l[from,to]
 */


List *
sublist(List *l, int from, int to)
{
    List *result = makeNode(List);
    ListCell *lc = l->head;

	// python style counting from end of list
	if(from < 0)
	{
		from = LIST_LENGTH(l) + from;
	}
	if(to < 0)
	{
		to = LIST_LENGTH(l) + to;
	}

    ASSERT(from >= 0 && to < LIST_LENGTH(l) && to >= from);

    // skip from start
    for(int i = 0; i < from; i++, lc = lc ->next);

    // find new head and skip until to
    result->head = lc;
    for(int i = from + 1; i <= to; i++, lc = lc->next);

    result->tail = lc;
    lc->next = NULL;
    result->type = l->type;
    result->length = to - from + 1;

    ASSERT(checkList(result));

    return result;
}

List *
genericSublist(List *l, boolean (*pred) (void *, void *), void *context)
{
    List *result = NIL;

    FOREACH(void,n,l)
    {
        if (pred(n, context))
            result = appendToTailOfList(result, n);
    }

    return result;
}

List *
mapList(List *l, void * (*f) (void *))
{
    List *result = NIL;
    FOREACH(void,el,l)
        result = appendToTailOfList(result,f(el));

    return result;
}

List *
mapToIntList(List *l, int (*f) (void *))
{
	List *result = NIL;

	FOREACH(void,el,l)
		result = appendToTailOfListInt(result, f(el));

	return result;
}

boolean
searchList(List *list, void *value)
{
    ASSERT(isPtrList(list));

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
    ASSERT(isIntList(list));

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
    ASSERT(isPtrList(list));

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


int
genericListPos (List *list, boolean (*eq) (void *, void *), void *value)
{
    ASSERT(isPtrList(list));
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
listPosInt(List *list, int val)
{
   ASSERT(isIntList(list));
   int pos = 0;

   FOREACH_INT(item,list)
   {
	   if (item == val)
		   return pos;
	   pos++;
   }

    return -1;
}

int
listPosString (List *list, char *value)
{
    ASSERT(isPtrList(list));
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
    ASSERT(isPtrList(list));
    List *result = NULL;

    FOREACH_LC(lc,list)
    {
        void *ptrVal = LC_P_VAL(lc);

        if(!eq(ptrVal, value))
            result = appendToTailOfList(result, ptrVal);
    }

    freeList(list);

    return result;
}

//List *
//removeFromTail(List *X)
//{
//    int len = LIST_LENGTH(X);
//    int c = 0;
//
//    List *result = NIL;
//
//    if(len == 1)
//        result = NIL;
//    else
//    {
//        FOREACH_INT(lc, X)
//        {
//            result = appendToTailOfListInt(result, lc);
//            c++;
//            if(c + 1 == len)
//                break;
//        }
//    }
//    return result;
//}

List *
removeFromListInt(List *l, int el)
{
	int pos;

	if(LIST_LENGTH(l) == 0)
		return l;

	pos = listPosInt(l, el);

	if(pos != -1)
	{
		l = removeListElemAtPos(l, pos);
	}

	return l;
}

List *
removeFromTail(List *X)
{
    //DEBUG_LOG("remove from list %s", nodeToString(X));
    List *result = NIL;
    ListCell *el;
    int l = LIST_LENGTH(X);
    boolean isInt = isIntList(X);

    if (l <= 1)
        return NIL;

    el = X->head;
    //DEBUG_LOG("remove from list el %s", nodeToString(el->data));
    for(int i=0; i<l-1; i++)
    {
        if (isInt)
            result = appendToTailOfListInt(result, LC_INT_VAL(el));
        else
            result = appendToTailOfList(result, LC_P_VAL(el));

        el = el->next;
    }

    return result;
}

List *
removeFromHead(List *X)
{
    List *result;
    ListCell *head;

    if (LIST_LENGTH(X) <= 1)
        return NIL;

    result = copyList(X);
    head = result->head;
    result->head = head->next;
    result->length--;

    return result;
}

List *
removeListElemAtPos (List *list, int pos)
{
    ListCell *lc, *prev;
    ASSERT(LIST_LENGTH(list) > pos && pos >= 0);

    if (LIST_LENGTH(list) == 1)
        return NIL;

    for(lc = list->head, prev = NULL; lc != NULL; prev = lc, lc = lc->next)
    {
        if (pos-- <= 0)
        {
            if (lc == list->head)
            {
                list->head = lc->next;
            }
            else
            {
                prev->next = lc->next;
            }
            list->length--;
            if (lc == list->tail)
                list->tail = prev;
            break;
        }
    }

    return list;
}

//remove all the elements of list l1 from list l2, l1 is a sublist of l2
List *
removeListElementsFromAnotherList(List *l1, List *l2)
{
    List *result = NIL;
    boolean flag = FALSE;

    FOREACH_LC(lc2, l2)
    {
        flag = FALSE;
        void *value = LC_P_VAL(lc2);

        FOREACH_LC(lc1, l1)
        {
            void *ptrVal = LC_P_VAL(lc1);
            if(equal(ptrVal, value))
            {
                flag = TRUE;
                break;
            }
        }
        if(flag == FALSE)
        {
            result = appendToTailOfList(result, value);
        }
    }

    return result;
}
