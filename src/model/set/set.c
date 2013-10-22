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
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "uthash.h"

Set *
newSet(NodeTag type)
{
    Set *result = makeNode(Set);

    result->elemType = type;
    result->elem = NULL;

    return result;
}

boolean
hasSetElem (Set *set, void *elem)
{
    SetElem *result;

    assert(IsA(elem, set->elemType));

    HASH_FIND_NODE(set->elem, &elem, result);

    return result != NULL;
}

boolean
hasSetIntElem (Set *set, int elem)
{
    SetElem *result;
    HASH_FIND_INT(set->elem, &elem, result);
    return result != NULL;
}

boolean
addToSet (Set *set, void *elem)
{
    SetElem *setEl;

    if (hasSetElem(set, elem))
        return FALSE;

    setEl = NEW(setEl);
    setEl->data = elem;

    HASH_ADD_PTR(set->elem, data, setEl);

    return TRUE;
}

boolean
addIntToSet (Set *set, int elem)
{
    SetElem *setEl;

    if (hasIntSetElem(set, elem))
        return FALSE;

    setEl = NEW(setEl);
    setEl->data = NEW(int);
    *((int *) setEl->data) = elem;

    HASH_ADD_INT(set->elem, data, setEl);

    return TRUE;
}

void
removeSetElem (Set *set, void *elem)
{
    SetElem *e;

    HASH_FIND_NODE(set->elem, &elem, e);

    if (e != NULL)
    {
        HASH_DEL(set->elem, e);
        if (set->elemType != T_Invalid) // && set->type != T_IntSet)
            deepFree(e->data);
        else
            FREE(e->data);
        FREE(e);
    }
}

void
removeSetIntElem (Set *set, int elem)
{
    SetElem *e;

    HASH_FIND_INT(set->elem, &elem, e);
    if (e != NULL)
    {
        HASH_DEL(set->elem, e);
        FREE(e);
    }
}

Set *
unionSets (Set *left, Set *right)
{
    Set *result = NEW(Set);
    result->elem = NULL;

    for(SetElem *s = left->elem; s != NULL; s = s->hh.next)
        addToSet(result, copy(s->data));
    for(SetElem *s = right->elem; s != NULL; s = s->hh.next)
        if (!hasSetElem(result, s->data))
            addToSet(result, copy(s->data));
    return result;
}

Set *
intersectSets (Set *left, Set *right)
{
    Set *result = NEW(Set);
    for(SetElem *s = left->elem; s != NULL; s = s->hh.next)
        if (hasSetElem(right, s->data))
            addToSet(result, s->data);

    return result;
}

int
setSize (Set *set)
{
    if (set == NULL)
        return 0;
    return HASH_COUNT(set->elem);
}
