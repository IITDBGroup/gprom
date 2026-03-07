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
#include "model/set/set.h"

#include "model/set/hashmap.h"

#define HASHMAP_MEM_CONTEXT_NAME "HASHMAP-CONTEXT"

// memory context to allocate static variables used for lookup
static MemContext *hashContext = NULL;
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

HashMap *
newHashMapOfSameType(HashMap *template)
{
    return newHashMap(template->keyType,
                      template->valueType,
                      template->eq,
                      template->cpy);
}

static inline HashElem *
getHashElem(HashMap *map, Node *key)
{
    HashElem *result = NULL;
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
hasMapLongKey (HashMap *map, gprom_long_t key)
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
    {
        if (hashContext == NULL)
            hashContext = NEW_MEM_CONTEXT(HASHMAP_MEM_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(hashContext);
        stringDummy = createConstString("");
        RELEASE_MEM_CONTEXT();
    }
    stringDummy->value = key;
    return getMap(map, (Node *) stringDummy);
}

Node *
getMapInt (HashMap *map, int key)
{
    int *v;
    static Constant *intDummy = NULL;
    if (intDummy == NULL)
    {
        if (hashContext == NULL)
            hashContext = NEW_MEM_CONTEXT(HASHMAP_MEM_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(hashContext);
        intDummy = createConstInt(0);
        RELEASE_MEM_CONTEXT();
    }

    v = (int *) intDummy->value;
    *v = key;
    return getMap(map, (Node *) intDummy);
}

Node *
getMapLong (HashMap *map, gprom_long_t key)
{
    static Constant *longDummy = NULL;
    gprom_long_t *v;
    if (longDummy == NULL)
    {
        if (hashContext == NULL)
            hashContext = NEW_MEM_CONTEXT(HASHMAP_MEM_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(hashContext);
        longDummy = createConstLong(0);
        RELEASE_MEM_CONTEXT();
    }

    v = (gprom_long_t *) longDummy->value;
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

Set *
getKeySet(HashMap *map)
{
    Set *result = NODESET();

    FOREACH_HASH_KEY(Node,n,map)
    {
        addToSet(result, copyObject(n));
    }

    return result;
}

Set *
getStringKeySet(HashMap *map)
{
    Set *result = STRSET();

    ASSERT(map->keyType == T_Constant);

    FOREACH_HASH_KEY(Constant,n,map)
    {
        addToSet(result, strdup(STRING_VALUE(n)));
    }

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

List *
getValues(HashMap *map)
{
    List *result = NIL;

    FOREACH_HASH_ENTRY(k, map)
    {
        result = appendToTailOfList(result, k->value);
    }

    return result;
}

boolean
addToMap(HashMap *map, Node *key, Node *value)
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

void
addToMapValueList(HashMap *map, Node *key, Node *item, boolean dupl)
{
	if(!hasMapKey(map, key))
	{
		List *l = singleton(item);
		addToMap(map, key, (Node *) l);
	}
	else
	{
		List *l = (List *) getMap(map, key);
		if(!(dupl && searchListNode(l, item)))
		{
			appendToTailOfList(l, item);
		}
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
    {
        if (hashContext == NULL)
            hashContext = NEW_MEM_CONTEXT(HASHMAP_MEM_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(hashContext);
        stringDummy = createConstString("");
        RELEASE_MEM_CONTEXT();
    }
    stringDummy->value = key;
    return mapIncr(map, (Node *) stringDummy);
}

int
mapIncrPointer(HashMap *map, void *key)
{
    static Constant *longDummy = NULL;
    if (longDummy == NULL)
    {
        if (hashContext == NULL)
            hashContext = NEW_MEM_CONTEXT(HASHMAP_MEM_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(hashContext);
        longDummy = createConstLong(0L);
        RELEASE_MEM_CONTEXT();
    }
	LONG_VALUE(longDummy) = (gprom_long_t) key;
	return mapIncr(map, (Node *) longDummy);
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

void
removeMapStringElem (HashMap *map, char *key)
{
    static Constant *stringDummy = NULL;
    if (key == NULL)
        return;
    if (stringDummy == NULL)
    {
        if (hashContext == NULL)
            hashContext = NEW_MEM_CONTEXT(HASHMAP_MEM_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(hashContext);
        stringDummy = createConstString("");
        RELEASE_MEM_CONTEXT();
    }
    stringDummy->value = key;
    return removeMapElem(map, (Node *) stringDummy);
}

int
mapSize (HashMap *map)
{
    if (map == NULL)
        return 0;
    return HASH_COUNT(map->elem);
}

void
unionIntoMap(HashMap *res, HashMap *new)
{
	FOREACH_HASH_ENTRY(kv,new)
	{
		if(!hasMapKey(res, kv->key))
		{
			addToMap(res, copyObject(kv->key), copyObject(kv->value));
		}
	}
}

HashMap *
unionMaps(HashMap *left, HashMap *right)
{
    HashMap *result = newHashMapOfSameType(left);

    FOREACH_HASH_ENTRY(kv, left)
    {
        ADD_TO_MAP(result, copyObject(kv));
    }

    FOREACH_HASH_ENTRY(kv, right)
    {
        ADD_TO_MAP(result, copyObject(kv));
    }

    return result;
}

void
diffMap(HashMap *res, HashMap *new)
{
	FOREACH_HASH_ENTRY(kv,new)
	{
		if(hasMapKey(res, kv->key))
		{
			removeMapElem(res, kv->key);
		}
	}
}

HashMap *
invertKeyValues(HashMap *map)
{
    HashMap *result = newHashMap(map->keyType,
                                 map->valueType,
                                 map->eq,
                                 map->cpy
                                 );

    FOREACH_HASH_ENTRY(kv, map)
    {
        KeyValue *newkv = createNodeKeyValue(copyObject(kv->value),
                                             copyObject(kv->key));
        ADD_TO_MAP(result, newkv);
    }

    return result;
}
