/*
 * bitset.h
 *
 *  Created on: Mar 8, 2019
 *      Author: liuziyu
 */

#ifndef BITSET_H
#define BITSET_H

#include "model/list/list.h"
#include "model/set/hashmap.h"

#define BITS_OF(typ) sizeof(typ) * 8
#define LONG_BITS BITS_OF(unsigned long)

typedef struct BitSet
{
	NodeTag type;
	unsigned int numWords;
    unsigned int length;
    unsigned long *value;
} BitSet;


//extern List* addBitset(int length, List *result);
//extern char* binDis(int length, int value);
extern char* bitSetToString (BitSet *bitset);
extern BitSet *stringToBitset (char *v);
extern BitSet *newBitSet (unsigned int length);
extern BitSet *newSingletonBitSet(int pos);
extern boolean isBitSet(BitSet *bitset, unsigned int pos);
extern void setBit(BitSet *bitset, unsigned int pos, boolean val);

extern BitSet *bitOr(BitSet *b1, BitSet *b2);
extern BitSet *bitAnd(BitSet *b1, BitSet *b2);
extern BitSet *bitNot(BitSet *b);

extern boolean bitsetEquals(BitSet *b1, BitSet *b2);
extern BitSet *copyBitSet(BitSet *in);
//extern boolean doubleLength(BitSet *bitset);


extern BitSet* newBitSet2 ();
extern void setBit2(BitSet *bitset, unsigned int pos, boolean val);
extern boolean isBitSet2(BitSet *bitset, unsigned int pos);

#endif /* INCLUDE_MODEL_BITSET_BITSET_H_ */
