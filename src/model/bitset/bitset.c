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

char*
bitSetToString (BitSet *bitset)
{
	StringInfo stringResult  = makeStringInfo();
	unsigned int length = bitset->length;
	while(length--)
	{
		if(*bitset->value&1<<length){       //00001000 & 00001000 = 1
			char *bit = "1";
			appendStringInfoString(stringResult, bit);
		}else{      //00001000 & 10000000 = 0
			char *bit = "0";
			appendStringInfoString(stringResult, bit);
		}
	}
	return stringResult->data;
}
/*
BitSet*
singleton(int pos)
{
}*/
//001000

boolean
isBitSet(BitSet *bitset, unsigned int pos){
	unsigned int length = bitset->length;
	if(length <= pos){
		DEBUG_LOG("OUT OF RANGE");
		return FALSE;
	}
	if(*bitset->value&1<<(length - pos - 1)){
		return TRUE;
	}else{
		return FALSE;
	}


}
void setBit(BitSet *bitset, unsigned int pos, boolean val) {
	if(pos >= bitset->length || isBitSet(bitset, pos) == val) {
		return;
	}
	if (isBitSet(bitset, pos)) {
		*bitset->value = *bitset->value - (1 << (bitset->length - pos - 1));
	} else {
		*bitset->value = *bitset->value + (1 << (bitset->length - pos - 1));

	}
}

BitSet*
newBitSet(unsigned int length, unsigned long *value, NodeTag type) {
	BitSet *newBitSet = makeNode(BitSet);
	newBitSet->type = type;
	newBitSet->value = value;
	newBitSet->length = length;
	return newBitSet;
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
bitOr(BitSet *b1, BitSet *b2){
	unsigned int length = b1->length > b2->length ? b1->length : b2->length;
	unsigned long *value = MALLOC(sizeof(unsigned long));
	*value = *b1->value|*b2->value;
	return newBitSet(length, value ,T_BitSet);
}

BitSet*
bitAnd(BitSet *b1, BitSet *b2){
	unsigned int length = b1->length > b2->length ? b1->length : b2->length;
	unsigned long value = *b1->value&*b2->value;
	return newBitSet(length, &value ,T_BitSet);
}

BitSet*
bitNot(BitSet *b){
	unsigned int length = b->length;
	unsigned long value = (1<<length) - 1 - *b->value;
	return newBitSet(length, &value ,T_BitSet);
}

boolean
bitsetEquals(BitSet *b1, BitSet *b2){
	if (b1 == NULL || b2 == NULL){
		return FALSE;
	}
	if (b1->length == b2->length && *b1->value == *b2->value){
		return TRUE;
	}
	return FALSE;
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







