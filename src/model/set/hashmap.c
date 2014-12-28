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

// static variables to speed up lookup for string and int keys and avoid the memory consumption of creating a new Constant node for each lookup
// without having to implement a different backend hashmap

static inline HashElem *getHashElem(HashMap *map, Node *key);


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

static inline HashElem *
getHashElem(HashMap *map, Node *key)
{
    HashElem *result = NULL;
//    char *realKey = nodeToString(key);
//    printf("hash elem:%p\n", key);
    HASH_FIND_NODE(hh,map->elem, key, result);

    return result;
}

boolean
hasMapKey (HashMap *map, Node *key)
{
    return (getHashElem(map, key) != NULL);
}

boolean
hasMapStringKey (HashMap *map, char *key)
{
    return (getMapString(map, key) != NULL);
}

boolean
hasMapIntKey (HashMap *map, int key)
{
    return (getMapInt(map, key) != NULL);
}

boolean
hasMapLongKey (HashMap *map, long key)
{
    return (getMapLong(map, key) != NULL);
}


Node *
getMap (HashMap *map, Node *key)
{
    if (key == NULL)
        return NULL;
    HashElem *el = getHashElem(map, key);
    KeyValue *kv = el ? (KeyValue *) el->data : NULL;

    return (Node *) (kv ? kv->value : NULL);
}

Node *
getMapString (HashMap *map, char *key)
{
    static Constant *stringDummy = NULL;
    if (key == NULL)
        return NULL;
    if (stringDummy == NULL)
        stringDummy = createConstString("");
    stringDummy->value = key;
    return getMap(map, (Node *) stringDummy);
}

Node *
getMapInt (HashMap *map, int key)
{
    int *v;
    static Constant *intDummy = NULL;
    if (intDummy == NULL)
        intDummy = createConstInt(0);
    v = (int *) intDummy->value;
    *v = key;
    return getMap(map, (Node *) intDummy);
}

Node *
getMapLong (HashMap *map, long key)
{
    static Constant *longDummy = NULL;
    long *v;
    if (longDummy == NULL)
        longDummy = createConstLong(0);
    v = (long *) longDummy->value;
    *v = key;
    return getMap(map, (Node *) longDummy);
}


KeyValue *
getMapEntry (HashMap *map, Node *key)
{
    HashElem *el = getHashElem(map, key);
    return (KeyValue *) (el ? el->data : NULL);
}

List *
getKeys(HashMap *map)
{
    List *result = NIL;

    FOREACH_HASH_KEY(Node,n,map)
        result = appendToTailOfList(result, n);

    return result;
}

List *
getEntries(HashMap *map)
{
    List *result = NIL;

    FOREACH_HASH_ENTRY(k,map)
        result = appendToTailOfList(result, k);

    return result;
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
        entry->key = key;

        HASH_ADD_NODE(hh, map->elem, entry->key, entry);
//        HashElem *el, *temp;
//        HASH_OUT_DEBUG(hh, map->elem,el,temp);
//        if(!getHashElem(map,entry->key))
//        {
//            printf("\n\nfailed lookup for key hashed to %u for \n%s", (unsigned) hashValue(entry->key), beatify(nodeToString(entry->key)));
//            ASSERT(FALSE);
//        }
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
        addToMap(map, copyObject(key), (Node *) createConstInt(v));
    }

    return v;
}

int
mapIncrString(HashMap *map, char *key)
{
    static Constant *stringDummy = NULL;
    if (stringDummy == NULL)
        stringDummy = createConstString("");
    stringDummy->value = key;
    return mapIncr(map, (Node *) stringDummy);
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
