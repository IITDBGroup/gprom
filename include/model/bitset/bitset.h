/*
 * bitset.h
 *
 *  Created on: Mar 8, 2019
 *      Author: liuziyu
 */

#ifndef INCLUDE_MODEL_BITSET_BITSET_H_
#define INCLUDE_MODEL_BITSET_BITSET_H_

#include "model/list/list.h"
#include "model/set/hashmap.h"

typedef struct BitSet
{
	NodeTag type;
    int     length;
    int 	value;
} BitSet;


//extern List* addBitset(int length, List *result);
//extern char* binDis(int length, int value);
extern char* bitSetToString (BitSet *bitset);
extern BitSet* newBitSet (int length, int value, NodeTag type);

#endif /* INCLUDE_MODEL_BITSET_BITSET_H_ */
