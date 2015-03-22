/*-----------------------------------------------------------------------------
 *
 * set.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SET_H_
#define SET_H_

#include "common.h"
#include "uthash.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

typedef enum SetType {
    SET_TYPE_INT,
    SET_TYPE_NODE,
    SET_TYPE_STRING,
    SET_TYPE_POINTER
} SetType;

typedef struct SetElem {
    void *data;
    void *key;
    UT_hash_handle hh;
} SetElem;

typedef struct Set {
    NodeTag type;
    SetType setType;
    int typelen;
    boolean (*eq) (void *, void*);
    void * (*cpy) (void *);
    SetElem *elem;
} Set;

// create new empty set
#define PSET() newSet(SET_TYPE_POINTER, sizeof(void*), NULL, NULL)
#define STRSET() newSet(SET_TYPE_STRING, -1, NULL, NULL)
#define NODESET() newSet(SET_TYPE_NODE, sizeof(Node), equal, copyObject)
#define INTSET() newSet(SET_TYPE_INT, sizeof(int), NULL, NULL)
extern Set *newSet(SetType set, int typelen, boolean (*eq) (void *, void *), void *(*cpy) (void *));

// create new set with content
extern Set *makeSet(SetType set, int typelen, boolean (*eq) (void *, void *), void *(*cpy) (void *), void *elem, ...);
extern Set *makeSetInt(int elem, ...);
#define MAKE_NODE_SET(...) makeSet(SET_TYPE_NODE, -1, equal, copyObject, __VA_ARGS__, NULL)
#define MAKE_SET_PTR(...) makeSet(SET_TYPE_POINTER, sizeof(void *), NULL, NULL, __VA_ARGS__, NULL)
#define MAKE_INT_SET(...) makeSetInt(__VA_ARGS__, -1)
#define MAKE_STR_SET(...) makeSet(SET_TYPE_STRING, -1, NULL, NULL, __VA_ARGS__, NULL)

// turn lists into sets
extern Set *makeStrSetFromList(List *strList);
extern Set *makeNodeSetFromList(List *list);

// iterate through sets
#define DUMMY_INT_FOR_COND_SET(_name_) _name_##_stupid_int_
#define DUMMY_SETEL(_name_) _name_##_his_el
#define INJECT_VAR_SET(type,name) \
    for(int DUMMY_INT_FOR_COND_SET(name) = 0; DUMMY_INT_FOR_COND_SET(name) == 0;) \
        for(type name = NULL; DUMMY_INT_FOR_COND_SET(name) == 0; DUMMY_INT_FOR_COND_SET(name)++) \

#define FOREACH_SET(_type_,_elem_,_set) \
        INJECT_VAR_SET(SetElem*,DUMMY_SETEL(_elem_)) \
        for(_type_ *_elem_ = (_type_ *)(((DUMMY_SETEL(_elem_) = \
                _set->elem) != NULL) ? \
                		DUMMY_SETEL(_elem_)->data : NULL); \
                		DUMMY_SETEL(_elem_) != NULL; \
                		_elem_ = (_type_ *)(((DUMMY_SETEL(_elem_) = \
            		   DUMMY_SETEL(_elem_)->hh.next) != NULL) ? \
            				   DUMMY_SETEL(_elem_)->data : NULL))

#define FOREACH_SET_INT(_elem_,_set) \
        INJECT_VAR_SET(SetElem*,DUMMY_SETEL(_elem_)) \
        for(int _elem_ = (((DUMMY_SETEL(_elem_) = \
                _set->elem) != NULL) ? \
                        *((int *) DUMMY_SETEL(_elem_)->data) : -1); \
                        DUMMY_SETEL(_elem_) != NULL; \
                        _elem_ = (((DUMMY_SETEL(_elem_) = \
                       DUMMY_SETEL(_elem_)->hh.next) != NULL) ? \
                               *((int *) DUMMY_SETEL(_elem_)->data) : -1))

#define FOREACH_SET_HAS_NEXT(_elem_) (DUMMY_SETEL(_elem_)->hh.next != NULL)

extern boolean hasSetElem (Set *set, void *_el);
extern boolean hasSetIntElem (Set *set, int _el);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeAndFreeSetElem (Set *set, void *elem);
extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);
extern Set *setDifference(Set *left, Set *right);

extern boolean overlapsSet(Set *left, Set *right);

extern int setSize (Set *set);
#define EMPTY_SET(set) (setSize(set) == 0)

#endif /* SET_H_ */
