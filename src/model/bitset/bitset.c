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
	//List *stringList = NIL;
	StringInfo stringResult  = makeStringInfo();
	while(bitset->length--)
	{
		if(bitset->value&1<<bitset->length){       //00001000 & 00001000 = 1
			char *bit = "1";
			appendStringInfoString(stringResult, bit);
			//appendToTailOfList(stringList, bit);
		}else{      //00001000 & 10000000 = 0
			char *bit = "0";
			appendStringInfoString(stringResult, bit);
			//appendToTailOfList(stringList, bit);
		}
	}
	//DEBUG_LOG("The bin is: %s", stringResult->data);
	return stringResult->data;
}

BitSet*
newBitSet (int length, int value, NodeTag type){
	BitSet *newBitSet = makeNode(BitSet);
	newBitSet->type = type;
	newBitSet->value = value;
	newBitSet->length = length;
	return newBitSet;

}




/*
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







