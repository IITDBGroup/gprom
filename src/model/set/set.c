/*-----------------------------------------------------------------------------
 *
 * set.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

#include "uthash.h"


static SetElem *getSetElem(Set *set, void *key);

static boolean intCmp (void *a, void *b);
static void *intCpy (void *a);

static boolean longCmp (void *a, void *b);
static void *longCpy (void *a);

static boolean ptrCmp (void *a, void *b);
static void *ptrCpy (void *a);

static boolean strCmp (void *a, void *b);
static void *strCpy (void *a);

#define CREATE_SAME_TYPE_SET(s) newSet(s->setType, s->typelen, s->eq, s->cpy)


Set *
newSet(SetType set, int typelen, boolean (*eq) (void *, void *), void * (*cpy) (void *))
{
    Set *result = makeNode(Set);

    result->setType = set;
    result->elem = NULL;
    result->typelen = typelen;

    if (set == SET_TYPE_INT)
    {
        result->eq = intCmp;
        result->cpy = intCpy;
    }
    else if (set == SET_TYPE_LONG)
    {
        result->eq = longCmp;
        result->cpy = longCpy;
    }
    else if (set == SET_TYPE_POINTER)
    {
        result->eq = ptrCmp;
        result->cpy = ptrCpy;
    }
    else if (set == SET_TYPE_STRING)
    {
        result->eq = strCmp;
        result->cpy = strCpy;
    }
    // NODE
    else
    {
        result->eq = eq;
        result->cpy = cpy;
    }

    return result;
}

Set *
makeSet(SetType set, int typelen, boolean (*eq) (void *, void *), void * (*cpy) (void *), void *elem, ...)
{
    Set *result = newSet(set, typelen, eq, cpy);
    va_list args;
    void *arg;

    addToSet(result, elem);

    va_start(args, elem);

    while((arg = va_arg(args,void *)) != NULL)
        addToSet(result, arg);

    va_end(args);

    return result;
}

Set *
makeSetInt(int elem, ...)
{
    Set *result = INTSET();
    va_list args;
    int arg;

    addIntToSet(result, elem);

    va_start(args, elem);

    for(arg = elem; arg >= 0; arg = va_arg(args,int))
    {
        TRACE_LOG("add int %d to set", arg);
        addIntToSet(result, arg);
    }

    va_end(args);

    return result;
}

Set *
makeSetLong(gprom_long_t elem, ...)
{
    Set *result = LONGSET();
    va_list args;
    int arg;

    addLongToSet(result, elem);

    va_start(args, elem);

    for(arg = elem; arg >= 0; arg = va_arg(args,gprom_long_t))
    {
        TRACE_LOG("add int %d to set", arg);
        addLongToSet(result, arg);
    }

    va_end(args);

    return result;
}

Set *
makeStrSetFromList(List *strList)
{
    Set *result = STRSET();

    FOREACH(char,str,strList)
        addToSet(result, str);

    return result;
}

Set *
makeNodeSetFromList(List *list)
{
    Set *result = NODESET();

    FOREACH(Node,n,list)
        addToSet(result, n);

    return result;
}

List *
makeNodeListFromSet(Set *s)
{
	List *result = NIL;

	FOREACH_SET(Node,n,s)
	{
		result = appendToTailOfList(result, n);
	}

	return result;
}

boolean
hasSetElem (Set *set, void *_el)
{
    return getSetElem(set, _el) != NULL;
}

static SetElem *
getSetElem(Set *set, void *key)
{
    SetElem *result, *s;

    if (set->setType == SET_TYPE_POINTER)
        HASH_FIND_PTR(set->elem, &key, result);
    else if (set->setType == SET_TYPE_STRING)
        HASH_FIND_STR(set->elem, key, result);
    else
    {
		HASH_FIND_NODE(hh, set->elem, key, result);
        /* char *realKey = nodeToString(key); */
        /* HASH_FIND_STR(set->elem, realKey, result); */
    }

	if(maxLevel >= LOG_TRACE)
	{
		// do not loop through set unless log level if high enough
		for(s=set->elem; s != NULL; s=s->hh.next)
		{
			TRACE_LOG("key and value %p with hv %u keyptr %p", s->data, s->hh.hashv, s->hh.key);
		}
	}

    return result;
}

boolean
hasSetIntElem (Set *set, int _el)
{
    SetElem *result = NULL, *s;

    HASH_FIND(hh,set->elem, &_el, sizeof(int), result);

	if(maxLevel >= LOG_TRACE)
	{
		// do not loop through set unless log level if high enough
		for(s=set->elem; s != NULL; s=s->hh.next)
		{
			TRACE_LOG("key and value %d with hv %u keyptr %d", *((int *) s->data), s->hh.hashv, *((int *) s->hh.key));
		}
	}

    return result != NULL;
}

boolean
hasSetLongElem (Set *set, gprom_long_t _el)
{
    SetElem *result = NULL, *s;

    HASH_FIND(hh,set->elem, &_el, sizeof(gprom_long_t), result);

	if(maxLevel >= LOG_TRACE)
	{
		// do not loop through set unless log level if high enough
		for(s=set->elem; s != NULL; s=s->hh.next)
		{
			TRACE_LOG("key and value %d with hv %u keyptr %d", *((gprom_long_t *) s->data), s->hh.hashv, *((gprom_long_t *) s->hh.key));
		}
	}

    return result != NULL;
}

boolean
addToSet (Set *set, void *elem)
{
    SetElem *setEl;

    if (hasSetElem(set, elem))
        return FALSE;

    setEl = NEW(SetElem);
    setEl->data = elem;

    if (set->setType == SET_TYPE_POINTER)
        HASH_ADD_PTR(set->elem, data, setEl);
    else if (set->setType == SET_TYPE_STRING)
        HASH_ADD_KEYPTR(hh, set->elem, setEl->data, strlen((char *) elem), setEl);
    // Node: store nodeToString as key
    else
    {
        /* setEl->key = nodeToString(elem); */
        /* HASH_ADD_KEYPTR(hh, set->elem, setEl->key, strlen(setEl->key), setEl); */
		HASH_ADD_NODE(hh, set->elem, setEl->data, setEl);
    }

    return TRUE;
}

boolean
addIntToSet (Set *set, int elem)
{
    SetElem *setEl;

    if (hasSetIntElem(set, elem))
        return FALSE;

    setEl = NEW(SetElem);
    setEl->data = NEW(int);
    *((int *) setEl->data) = elem;

    HASH_ADD_KEYPTR(hh, set->elem, setEl->data, sizeof(int), setEl);

    return TRUE;
}

boolean
addLongToSet (Set *set, gprom_long_t elem)
{
    SetElem *setEl;

    if (hasSetLongElem(set, elem))
        return FALSE;

    setEl = NEW(SetElem);
    setEl->data = NEW(gprom_long_t);
    *((gprom_long_t *) setEl->data) = elem;

    HASH_ADD_KEYPTR(hh, set->elem, setEl->data, sizeof(gprom_long_t), setEl);

    return TRUE;
}

void
removeAndFreeSetElem (Set *set, void *elem)
{
    SetElem *e;

    e = getSetElem(set,elem);

    if (e != NULL)
    {
        HASH_DEL(set->elem, e);
        if (set->setType == SET_TYPE_NODE)
        {
            deepFree(e->data);
            //FREE(e->key);
        }
        else if (set->setType == SET_TYPE_STRING)
            FREE(e->data);

        FREE(e);
    }
}

void
removeSetElem (Set *set, void *elem)
{
    SetElem *e;

    e = getSetElem(set,elem);

    if (e != NULL)
    {
        HASH_DEL(set->elem, e);
        /* if (set->setType == SET_TYPE_NODE) */
        /*     FREE(e->key); */
        FREE(e);
    }
}

void
removeSetIntElem (Set *set, int elem)
{
    SetElem *e;

    HASH_FIND(hh, set->elem, &elem, sizeof(int), e);
    if (e != NULL)
    {
        HASH_DEL(set->elem, e);
        FREE(e);
    }
}

void
removeSetLongElem (Set *set, gprom_long_t elem)
{
    SetElem *e;

    HASH_FIND(hh, set->elem, &elem, sizeof(gprom_long_t), e);
    if (e != NULL)
    {
        HASH_DEL(set->elem, e);
        FREE(e);
    }
}

Set *
unionSets (Set *left, Set *right)
{
    Set *result;
    SetElem *s;

    ASSERT(left->setType == right->setType);
    ASSERT(left->cpy && right->cpy);

    result = CREATE_SAME_TYPE_SET(left);

    if (result->setType == SET_TYPE_INT)
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            addIntToSet(result, *((int *) s->data));
        for(s = right->elem; s != NULL; s = s->hh.next)
            if (!hasSetIntElem(result, *((int *) s->data)))
            {
                addIntToSet(result, *((int *) s->data));
            }
    }
    else
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            addToSet(result, left->cpy(s->data));
        for(s = right->elem; s != NULL; s = s->hh.next)
            if (!hasSetElem(result, s->data))
            {
                addToSet(result, right->cpy(s->data));
            }
    }

    TRACE_LOG("union result set %s", nodeToString(result));

    return result;
}

Set *
intersectSets (Set *left, Set *right)
{
    Set *result;
    SetElem *s;

    ASSERT(left->setType == right->setType);
    ASSERT(left->cpy && right->cpy);

    result = CREATE_SAME_TYPE_SET(left);

    if (result->setType == SET_TYPE_INT)
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            if (hasSetIntElem(right, *((int *) s->data)))
                addIntToSet(result, *((int *) s->data));
    }
    else
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            if (hasSetElem(right, s->data))
                addToSet(result, left->cpy(s->data));
    }

    TRACE_LOG("intersect result set %s", nodeToString(result));

    return result;
}

Set *
setDifference(Set *left, Set *right)
{
    Set *result;
    SetElem *s;

    ASSERT(left->setType == right->setType);
    ASSERT(left->cpy && right->cpy);

    result = CREATE_SAME_TYPE_SET(left);

    if (result->setType == SET_TYPE_INT)
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            if (!hasSetIntElem(right, *((int *) s->data)))
                addIntToSet(result, *((int *) s->data));
    }
    else
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            if (!hasSetElem(right, s->data))
                addToSet(result, left->cpy(s->data));
    }

    TRACE_LOG("different result set %s", nodeToString(result));

    return result;
}


boolean
overlapsSet(Set *left, Set *right)
{
    SetElem *s;
    if (left->setType != right->setType)
        return FALSE;

    if (left->setType == SET_TYPE_INT)
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            if (hasSetIntElem(right, *((int *) s->data)))
                return TRUE;
    }
    else
    {
        for(s = left->elem; s != NULL; s = s->hh.next)
            if (hasSetElem(right, s->data))
                return TRUE;
    }

    return FALSE;
}

boolean
containsSet(Set *left, Set *right)
{

    boolean containedElem = FALSE;

    if (left->setType != right->setType)
        return FALSE;

    FOREACH_SET(char, s1, left)
    {
    	containedElem = FALSE;
    	if (hasSetElem(right, s1))
    		containedElem = TRUE;

    	if (!containedElem)
    		break;
    }
	return containedElem;
}

void *
popSet(Set *set)
{
    ASSERT(!EMPTY_SET(set));
	SetElem *e = set->elem;

	if(set->setType == SET_TYPE_STRING)
	{
		char *el = strdup(e->data);
		removeSetElem(set, el);
		return el;
	}
	else
	{
		Node *el = copyObject(e->data);
		removeSetElem(set, el);
		return el;
	}
}

int
setSize (Set *set)
{
    if (set == NULL)
        return 0;
    return HASH_COUNT(set->elem);
}

static boolean
intCmp (void *a, void *b)
{
    int c = *((int *) a);
    int d = *((int *) b);

    return c == d;
}

static void *
intCpy (void *a)
{
    int *result = NEW(int);
    *result = *((int *) a);

    return (void *) result;
}

static boolean
longCmp (void *a, void *b)
{
    gprom_long_t c = *((gprom_long_t *) a);
    gprom_long_t d = *((gprom_long_t *) b);

    return c == d;
}

static void *
longCpy (void *a)
{
    gprom_long_t *result = NEW(gprom_long_t);
    *result = *((gprom_long_t *) a);

    return (void *) result;
}

static boolean
ptrCmp (void *a, void *b)
{
    return a == b;
}

static void *
ptrCpy (void *a)
{
    return a;
}

static boolean
strCmp (void *a, void *b)
{
    return strcmp(a,b) == 0;
}

static void *
strCpy (void *a)
{
    char *in = (char *) a;

    return strdup(in);
}
