

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_

#include "model/set/vector.h"
#include "model/bitset/bitset.h"
#include "model/query_operator/query_operator.h"

#define DATA_CHUNK_PROP "DATA_CHUNK"

extern char* update_ps_incremental(QueryOperator * qbModel);

typedef struct DataChunk
{
	/*
	 *  updateIdentifier:
	 *  	a Vector
	 * 			storing chars "+"/"-" indicates "insert" or "delete"; 1 / -1;
	 *
	 *  tuples:
	 *  	a Vector
	 *  		each vector node of tuples is a vector storing the tuples' data;
	 *  		each vector node should be a vector which is a column of table values of same data type;
	 * (1, 2) (3, 4) (5, 6)
	 * (1, 3, 5);
	 * (2, 4, 6);
	 *
	 * need vecLong/ vecFloat/ vecDouble to be more efficient to access data;
	 *
	 *
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


// for evaluation > <, return a bit vector identify YES or NO;
typedef struct ColumnChunk
{
	NodeTag type;
	boolean isBit;
	union
	{
		BitSet *bs;
		Vector *v;
	} data;
	int length;
	DataType dataType;
} ColumnChunk;


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
extern int getIntValueFromDataChunk(DataChunk *dc, int row, int col);
//extern float getFloatValueFromDataChunk(DataChunk *dc, int pos);
//extern gprom_long getLongValueFromDataChunk(DataChunk *dc, int pos);
//extern char *getStringValueFromDataChunk(DataChunk *dc, int pos);
//extern boolean *getBooleanValueFromDataChunk(DataChunk *dc, int pos);

extern int* getIntArrayFromDataChunk(DataChunk *dc, int col);
//extern gprom_float* getFloatArray(DataChunk *dc, int col);
//extern int* getIntArray(DataChunk *dc, int col);
//extern int* getIntArray(DataChunk *dc, int col);
//extern int* getIntArray(DataChunk *dc, int col);


#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_ */
