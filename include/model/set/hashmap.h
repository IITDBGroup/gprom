/*-----------------------------------------------------------------------------
 *
 * hashmap.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include "common.h"
#include "uthash.h"
#include "model/node/nodetype.h"

typedef struct HashElem {
    void *data;
    void *key;
    UT_hash_handle hh;
} HashElem;

typedef struct HashMap {
    NodeTag type;
    NodeTag keyType;
    NodeTag valueType;
    int typelen;
    boolean (*eq) (void *, void*);
    void * (*cpy) (void *);
    HashElem *elem;
} HashMap;

// create new empty hashmap1
#define NEW_MAP(keyType,valueType) newHashMap(T_ ## keyType, T_ ## valueType, NULL, NULL)
extern HashMap *newHashMap(NodeTag keyType, NodeTag valueType, boolean (*eq) (void *, void *), void *(*cpy) (void *));

// accesssing map elements
extern boolean hasMapKey (HashMap *map, Node *key);
#define MAP_HAS_STRING_KEY(map,key) hasMapKey(map, (Node *) createConstString(key))
#define MAP_HAS_INT_KEY(map,key) hasMapKey(map, (Node *) createConstInt(key))

extern Node *getMap (HashMap *map, Node *key);
#define MAP_GET_STRING(map,key) getMap(map, (Node *) createConstString(key))
#define MAP_GET_INT(map,key) getMap(map, (Node *) createConstInt(key))
extern KeyValue *getMapEntry (HashMap *map, Node *key);

// add elements to map
extern boolean addToMap (HashMap *map, Node *key, Node *value);
#define ADD_TO_MAP(map, keyvalue) \
	do { \
	    KeyValue *_kv = (keyvalue); \
		addToMap((HashMap *) map, (Node *) _kv->key, (Node *) _kv->value); \
	} while(0)
#define MAP_ADD_STRING_KEY(map, key, value) addToMap((HashMap *) map, (Node *) createConstString(key), (Node *) value)
#define MAP_ADD_INT_KEY(map, key, value) addToMap((HashMap *) map, (Node *) createConstInt(key), (Node *) value)

// remove elements from map
extern void removeAndFreeMapElem (HashMap *map, Node *key);
extern void removeMapElem (HashMap *map, Node *key);

// map size
extern int mapSize (HashMap *map);

// iterate over map values or entries
#define DUMMY_INT_FOR_COND_HASH(_name_) _name_##_stupid_int_
#define DUMMY_HASHEL(_name_) _name_##_his_el
#define INJECT_VAR_HASH(type,name) \
    for(int DUMMY_INT_FOR_COND_HASH(name) = 0; DUMMY_INT_FOR_COND_HASH(name) == 0;) \
        for(type name = NULL; DUMMY_INT_FOR_COND_HASH(name) == 0; DUMMY_INT_FOR_COND_HASH(name)++)

#define FOREACH_HASH(_type_,_elem_,_map) \
		INJECT_VAR_HASH(HashElem*,DUMMY_HASHEL(_elem_)) \
		for(_type_ *_elem_ = (_type_ *)(((DUMMY_HASHEL(_elem_) = \
				_map->elem) != NULL) ? \
						((KeyValue *) DUMMY_HASHEL(_elem_)->data)->value : NULL); \
						DUMMY_HASHEL(_elem_) != NULL; \
						_elem_ = (_type_ *)(((DUMMY_HASHEL(_elem_) = \
								DUMMY_HASHEL(_elem_)->hh.next) != NULL) ? \
										((KeyValue *) DUMMY_HASHEL(_elem_)->data)->value : NULL))

#define FOREACH_HASH_KEY(_type_,_elem_,_map) \
        INJECT_VAR_HASH(HashElem*,DUMMY_HASHEL(_elem_)) \
        for(_type_ *_elem_ = (_type_ *)(((DUMMY_HASHEL(_elem_) = \
                _map->elem) != NULL) ? \
                        ((KeyValue *) DUMMY_HASHEL(_elem_)->data)->key : NULL); \
                        DUMMY_HASHEL(_elem_) != NULL; \
                        _elem_ = (_type_ *)(((DUMMY_HASHEL(_elem_) = \
                                DUMMY_HASHEL(_elem_)->hh.next) != NULL) ? \
                                        ((KeyValue *) DUMMY_HASHEL(_elem_)->data)->key : NULL))


#define FOREACH_HASH_ENTRY(_elem_,_map) \
        INJECT_VAR_HASH(HashElem*,DUMMY_HASHEL(_elem_)) \
        for(KeyValue *_elem_ = (KeyValue *)(((DUMMY_HASHEL(_elem_) = \
                _map->elem) != NULL) ? \
                        ((KeyValue *) DUMMY_HASHEL(_elem_)->data) : NULL); \
                        DUMMY_HASHEL(_elem_) != NULL; \
                        _elem_ = (KeyValue *)(((DUMMY_HASHEL(_elem_) = \
                                DUMMY_HASHEL(_elem_)->hh.next) != NULL) ? \
                                        DUMMY_HASHEL(_elem_)->data : NULL))


#endif /* HASHMAP_H_ */
