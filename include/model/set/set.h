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

typedef enum SetType {
    SET_TYPE_INT,
    SET_TYPE_NODE,
    SET_TYPE_STRING,
    SET_TYPE_POINTER
} SetType;

typedef struct SetElem {
    void *data;
    UT_hash_handle hh;
} SetElem;

typedef struct Set {
    NodeTag type;
    SetType setType;
    int typelen;
    boolean (*eq) (void *, void*);
    SetElem *elem;
} Set;


#define NODESET() newSet(SET_TYPE_NODE, -1, equals);
#define INTSET() newSet(SET_TYPE_INT, sizeof(int), NULL);
extern Set *newSet(SetType set, int typelen, boolean (*eq) (void *, void *));

extern boolean hasSetElem (Set *set, void *elem);
extern boolean hasSetIntElem (Set *set, int elem);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);

extern int setSize (Set *set);

#endif /* SET_H_ */
