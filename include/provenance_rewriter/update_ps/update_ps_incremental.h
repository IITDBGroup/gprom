

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_

#include "model/set/vector.h"
#include "model/query_operator/query_operator.h"

#define DATA_CHUNK_PROP "DATA_CHUNK"

extern char* update_ps_incremental(QueryOperator * qbModel);

typedef struct DataChunk
{
	/*
	 *  updateIdentifier:
	 *  	a Vector
	 * 			storing chars "+"/"-" indicates "insert" or "delete";
	 *
	 *  tuples:
	 *  	a Vector
	 *  		each vector node of tuples is a vector storing the tuples' data;
	 *  		each vector node should be a vector which is a column of table values of same data type;
	 *
	 *  fragmentsInfo:
	 *  	a HashMap
	 *  		ps_attr -> bitset
	 *
	 *  numTuples:
	 *  	integer
	 *  		numbers of tuples;
	 *  		this value is equivalent to size of "list cell" in "tuples" list;
	 *
	 *  tupleFields:
	 *  	integer
	 *  		attribute numbers of each tuple;
	 *  		this value is equivalent to size of "tuples" list
	 *  attriToPos:
	 *  	a HashMap
	 *  		Key: attribute name, Value: column index in "tuples" list;
	 *  posToDatatype:
	 *  	a HashMap
	 *  		Key: index of "tuple" list, Value: datatype of this column;
	 */
	NodeTag type;
	List    *attrNames; // attrDefs
	Vector 	*updateIdentifier;
	Vector  *tuples;
	HashMap	*fragmentsInfo;
	int    	numTuples;
	int    	tupleFields;
	HashMap *attriToPos;
	HashMap *posToDatatype;

} DataChunk;

extern DataChunk* initDataChunk();

/*
 * 	type: "MIN" or "MAX";
 * 	valueType: list cell value type;
 *  valuePos: each cell is a list, [value, ps];
 * 	direction: "1" to head or "-1" to end;
 * 	removeLast: "TRUE" for remove element from heap;
 */
extern void heapifyListSiftUp(List *list, int pos, char *type, DataType valDataType, int valuePos);
extern void heapifyListSiftDown(List *list, int pos, char *type, DataType valDataType, int valuePos);
extern void heapInsert(List *list, char* type, Node *ele);
extern void heapDelete(List *list, char* type, Node *ele);


#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_ */
