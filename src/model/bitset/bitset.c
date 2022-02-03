/*
 * bitset.c
 *
 *  Created on: Mar 8, 2019
 *      Author: liuziyu
 */
#include "model/bitset/bitset.h"
#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include <string.h>

#define LONGSIZE 8 * sizeof(unsigned long)
#define TRUE_CHAR '1'
#define FALSE_CHAR '0'

static void growBitset(BitSet *b, unsigned int newLen);



char*
bitSetToString (BitSet *bitset)
{
	StringInfo stringResult  = makeStringInfo();
	unsigned int length = bitset->length;
    unsigned int longpos = -1;
    unsigned int pos = -1;
	unsigned long mask = 1UL;

	while(++pos < length)
	{
	    if (!(pos % 64)) {
	        longpos++;
	        mask = 1;
	    }
	    appendStringInfoChar(stringResult,
	            (bitset->value[longpos] & mask ? TRUE_CHAR : FALSE_CHAR));
		mask <<= 1;
	}

	return stringResult->data;
}

BitSet *
stringToBitset (char *v)
{
    BitSet *res = newBitSet(strlen(v));
    unsigned int longpos = -1;
    unsigned long mask = 1UL;

    for(int i = 0; i < res->length; i++)
    {
        if (!(i % LONG_BITS))
        {
            longpos++;
            mask = 1;
        }
        if (v[i] == '1')
            res->value[longpos] |= mask;
        else
            res->value[longpos] &= ~mask;
        mask <<= 1;
    }

    return res;
}

/*
BitSet*
singleton(int pos)
{
}*/
//001000

boolean
isBitSet(BitSet *bitset, unsigned int pos){
    unsigned int longpos = pos / 64;
	unsigned int bitpos = pos % 64;

	if(bitset->length <= pos)
	{
		DEBUG_LOG("OUT OF RANGE");
		return FALSE;
	}
	if(bitset->value[longpos] & (1UL << bitpos))
		return TRUE;
	else
		return FALSE;
}

void
setBit(BitSet *bitset, unsigned int pos, boolean val)
{
    unsigned int longpos = pos / 64;
    unsigned int bitpos = pos % 64;

    // have outgrown the bitset
    if (longpos + 1 > bitset->numWords) {
        growBitset(bitset, longpos + 1);
    }
    // set bit to 1 using bitor with 0...010...0
    if (val) {
        bitset->value[longpos] |= 1UL << bitpos;
    }
    // set bit to 0 using bitand with 1...101...1
    else {
        bitset->value[longpos] &= ~(1UL << bitpos);
    }
}

static void
growBitset(BitSet *b, unsigned int newLen)
{
    unsigned int powTwoLen = b->numWords;
    unsigned long *newVal;

    while(powTwoLen < newLen)
        powTwoLen *= 2;
    newVal = CALLOC(sizeof(unsigned long), powTwoLen);
    memcpy(newVal, b->value, b->numWords * sizeof(unsigned long));
}

BitSet*
newBitSet(unsigned int length)
{
	BitSet *newBitSet = makeNode(BitSet);

	newBitSet->numWords = ((length - 1) / LONG_BITS) + 1;
	newBitSet->value = CALLOC(sizeof(unsigned long), newBitSet->numWords);
	newBitSet->length = length;

	return newBitSet;
}

BitSet *
newSingletonBitSet(int pos)
{
	BitSet *b = newBitSet(pos + 1);
	setBit(b, pos, TRUE);

	return b;
}

BitSet *
longToBitSet(unsigned long bits)
{
	BitSet *b = newBitSet(sizeof(unsigned long) * 8);
	b->value[0] = bits;
	return b;
}


/*
boolean
isBitSet2(BitSet *bitset, unsigned int pos){
	size_t wordpos = pos % LONGSIZE;
	unsigned long w = bitset->value[wordpos];
	unsigned int remainder = pos / LONGSIZE;

	if(bitset->length <= pos){
		DEBUG_LOG("OUT OF RANGE");
		return FALSE;
	}
	if(w & (1 << remainder)) {
		return TRUE;
	}else{
		return FALSE;
	}
}

void
setBit2(BitSet *bitset, unsigned int pos, boolean val)
{
	size_t wordpos = pos % LONGSIZE;
	unsigned long w;
	unsigned int remainder = pos / LONGSIZE;

    if (isBitSet(bitset, pos) == val)
    {
		return;
	}
	if(wordpos >= bitset->numWords)
	{
		unsigned long origWords = bitset->value;
		bitset->value = MALLOC(sizeof(unsigned long) * (wordpos + 1));
		bitset->value[wordpos] = 0;
		bitset->numWords = wordpos + 1;
	}
	if (pos >= bitset->length)
		bitset->length = pos + 1;
	if(val)
	{
		bitset->value[wordpos] |= (1 << remainder);
	}
	else
	{
		bitset->value[wordpos] &= ~(1 << remainder);
	}
}

BitSet*
newBitSet2 () {
	BitSet *newBitSet = makeNode(BitSet);
	newBitSet->value = MALLOC(sizeof(unsigned long));
	newBitSet->length = LONGSIZE;
	return newBitSet;
}*/

BitSet*
bitOr(BitSet *b1, BitSet *b2)
{
	BitSet *longer = b1->length > b2->length ? b1 : b2;
	BitSet *shorter = b1->length > b2->length ? b2 : b1;
	BitSet *result = copyBitSet(longer);

	for(int i = 0; i < shorter->numWords; i++)
	    result->value[i] |= shorter->value[i];

	return result;
}

BitSet*
bitAnd(BitSet *b1, BitSet *b2)
{
    BitSet *longer = b1->length > b2->length ? b1 : b2;
    BitSet *shorter = b1->length > b2->length ? b2 : b1;
    BitSet *result = copyBitSet(longer);

    for(int i = 0; i < shorter->numWords; i++)
        result->value[i] &= shorter->value[i];

    return result;
}

BitSet*
bitNot(BitSet *b)
{
	BitSet *result = copyBitSet(b);
	unsigned int lastWordBits = b->length % 64;
	unsigned int mask = (1 << (lastWordBits)) - 1; // 2^n - 1 to get n 1 bits

	for(int i = 0; i < result->numWords; i++)
	    result->value[i] = ~(result->value[i]);
	// if length is not a multiple of LONG_BITS then we have addtiional 1's that should not be there
	// replace them with zeros
	result->value[result->numWords - 1] &= mask;

	return result;
}

boolean
bitsetEquals(BitSet *b1, BitSet *b2)
{
	if (b1 == NULL || b2 == NULL){
		return FALSE;
	}
	if (b1->length == b2->length)
	{
		for(int i = 0; i < b1->numWords; i++)
			if (b1->value[i] != b2->value[i])
				return FALSE;
		return TRUE;
	}
	return FALSE;
}

BitSet *
copyBitSet(BitSet *in)
{
    BitSet *n = newBitSet(in->length);
    memcpy(n->value, in->value, sizeof(unsigned long) * in->numWords);
    return n;
}





/*
boolean
doubleLength(BitSet *bitset) {
	if (bitset->length <= 8) {
		bitset->length = bitset->length * 2;
		return TRUE;
	} else {
	return FALSE;
	}
}





char*
binDis(int length, int value)
{
	//List *stringList = NIL;
	StringInfo stringResult  = makeStringInfo();
	while(length--)
	{
		if(value&1<<length){
			char *bit = "1";
			appendStringInfoString(stringResult, bit);
			//appendToTailOfList(stringList, bit);
		}else{
			char *bit = "0";
			appendStringInfoString(stringResult, bit);
			//appendToTailOfList(stringList, bit);
		}
	}
	//DEBUG_LOG("The bin is: %s", stringResult->data);
	return stringResult->data;
}*/
