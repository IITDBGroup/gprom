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


typedef struct Set {
    NodeTag type;
    NodeTag elemType;
    SetElem *elem;
} Set;

typedef struct SetElem {
    void *data;
    UT_hash_handle hh;
} SetElem;

extern Set *newSet(NodeTag type);

extern boolean hasSetElem (Set *set, void *elem);
extern boolean hasSetIntElem (Set *set, int elem);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);


} SetElem;

extern Set *newSet();

extern boolean hasSetElem (Set *set, void *elem);
extern boolean hasSetIntElem (Set *set, int elem);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);


} SetElem;

extern Set *newSet();

extern boolean hasSetElem (Set *set, void *elem);
extern boolean hasSetIntElem (Set *set, int elem);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);


} SetElem;

extern Set *newSet();

extern boolean hasSetElem (Set *set, void *elem);
extern boolean hasSetIntElem (Set *set, int elem);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);


} SetElem;

extern Set *newSet();

extern boolean hasSetElem (Set *set, void *elem);
extern boolean hasSetIntElem (Set *set, int elem);

extern boolean addToSet (Set *set, void *elem);
extern boolean addIntToSet (Set *set, int elem);

extern void removeSetElem (Set *set, void *elem);
extern void removeSetIntElem (Set *set, int elem);

extern Set *unionSets (Set *left, Set *right);
extern Set *intersectSets (Set *left, Set *right);

#endif /* SET_H_ */
