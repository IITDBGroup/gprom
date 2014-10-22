/*-----------------------------------------------------------------------------
 *
 * hashmap.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

#include "model/set/hashmap.h"

static HashElem *getHashElem(HashMap *map, Node *key);


HashMap *
newHashMap(NodeTag keyType, NodeTag valueType, boolean (*eq) (void *, void *), void *(*cpy) (void *))
{
    HashMap *map = makeNode(HashMap);

    map->keyType = keyType;
    map->valueType = valueType;
    map->elem = NULL;
    map->eq = (eq == NULL) ? equal : eq;
    map->cpy = (cpy == NULL) ? copyObject : cpy;
    map->typelen = -1;

    return map;
}

static HashElem *
getHashElem(HashMap *map, Node *key)
{
    HashElem *result, *s;
    char *realKey = nodeToString(key);

    HASH_FIND_STR(map->elem, realKey, result);

    return result;
}

boolean
hasMapKey (HashMap *map, Node *key)
{
    return (getHashElem(map, key) != NULL);
}

Node *
getMap (HashMap *map, Node *key)
{
    HashElem *el = getHashElem(map, key);
    KeyValue *kv = el ? (KeyValue *) el->data : NULL;

    return (Node *) (kv ? kv->value : NULL);
}

KeyValue *
getMapEntry (HashMap *map, Node *key)
{
    HashElem *el = getHashElem(map, key);
    return (KeyValue *) (el ? el->data : NULL);
}

boolean
addToMap (HashMap *map, Node *key, Node *value)
{
    HashElem *entry = getHashElem(map, key);

    // entry does not exist, add it
    if (entry == NULL)
    {
        KeyValue *kv = createNodeKeyValue(key, value);
        entry = NEW(HashElem);
        entry->data = kv;

        entry->key = nodeToString(key);
        HASH_ADD_KEYPTR(hh, map->elem, entry->key, strlen(entry->key), entry);

        return TRUE;
    }
    // overwrite value of existing entry with same key
    else
    {
        KeyValue *kv = (KeyValue *) entry->data;
        kv->value = value;

        return FALSE;
    }
}

int
mapIncr(HashMap *map, Node *key)
{
    int v;
    Constant *val;

    if (hasMapKey(map,key))
    {
        val = (Constant *) getMap(map, key);
        v = (INT_VALUE(val)) + 1;
        INT_VALUE(val) = v;
    }
    else
    {
        v = 0;
        addToMap(map, key, (Node *) createConstInt(v));
    }

    return v;
}

void
removeAndFreeMapElem (HashMap *map, Node *key)
{
    HashElem *el = getHashElem(map, key);

    if (el != NULL)
    {
        HASH_DEL(map->elem, el);
        deepFree(el->data);
        FREE(el);
    }
}

void
removeMapElem (HashMap *map, Node *key)
{
    HashElem *el = getHashElem(map, key);

    if (el != NULL)
    {
        HASH_DEL(map->elem, el);
        FREE(el->data);
        FREE(el);
    }
}

int
mapSize (HashMap *map)
{
    if (map == NULL)
        return 0;
    return HASH_COUNT(map->elem);
}
