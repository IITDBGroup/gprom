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

BitSet*
newBitSet (unsigned int length, unsigned long *value, NodeTag type){
	BitSet *newBitSet = makeNode(BitSet);
	newBitSet->type = type;
	newBitSet->value = value;
	newBitSet->length = length;
	return newBitSet;

}

BitSet*
bitOr(BitSet *b1, BitSet *b2){
	unsigned int length = b1->length > b2->length ? b1->length : b2->length;
	unsigned long value = *b1->value|*b2->value;
	return newBitSet(length, &value ,T_BitSet);
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







