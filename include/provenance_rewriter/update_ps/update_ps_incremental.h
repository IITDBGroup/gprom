

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_

#include "model/set/vector.h"
#include "model/bitset/bitset.h"
#include "model/query_operator/query_operator.h"

#define DATA_CHUNK_PROP "DATA_CHUNK"
#define INCREMENTAL_UPDATE_TIMER "module - update provenance sketch - incremental update"
#define INCREMENTAL_FETCHING_DATA_TIMER "module - update provenance sketch - incremental update fetching data"
#define JOIN_LEFT_BRANCH_IDENTIFIER backendifyIdentifier("JOIN_LEFT_TUPLE_IDENTIFIER")
#define JOIN_RIGHT_BRANCH_IDENTIFIER backendifyIdentifier("JOIN_RIGHT_TUPLE_IDENTIFIER")
extern char* update_ps_incremental(QueryOperator * qbModel, QueryOperator *updateStmt);

extern BitSet *setFragmentToBitSet(int value, List *rangeList);
extern int setFragmengtToInt(int value, Vector *rangeList);
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
	 *  	a HashMap(key: prov_r_a1, value: list of bitset) -> insert tuple:(1, 2, 4) its bitset: 10000,
	 * 			two problems: keep a pointer of current element,
	 *  		ps_attr -> bitset list;
	 * 		a HashMap(key: prov_r_a1, value: vector of bitset);
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
	HashMap *fragmentsIsInt;
	int    	numTuples;
	int    	tupleFields;
	HashMap *attriToPos;
	HashMap *posToDatatype;
	Vector  *isNull;
	boolean isAPSChunk;
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
/*
// heap
typedef struct GBHeaps
{
	NodeTag  type;
	DataType valType;
	HashMap  *heapLists; // key: gb value like "2", value: all values of same gb value like: (1,2,8)
	HashMap   *provSketchs; // key: prov_r_a, prov_s_b, value: '111', '101' TODO: Need modify the implementation to key: gbvalue, keys: hashMap(key, prov_r_a, prov_t_b, values: "101", "010")
	Constant *heapType;
	HashMap  *fragCount; // key: prov_attr_string, Value: HashMap(fragNumber, tuple count);

} GBHeaps;

// group by avg, count, sum : use same data structure GBACSs
typedef struct GBACSs
{
	NodeTag type;
	HashMap *map; // key: gb, value: for cnt: one value-- count// for sum : a list of two values -- sum, count of the group;// for avg: a list of three values: avg, sum, count;
	HashMap *provSketchs;
	HashMap *fragCount;
} GBACSs;

typedef struct LMTChunk
{
	NodeTag type;
	HashMap *attrToPos;
	HashMap *posToDatatype;
	List 	*vals;
	int 	tupleFields;
	int 	numTuples;
	HashMap *provToPos;
} LMTChunk;

typedef struct PSMap
{
	NodeTag type;
	HashMap *psMaps; // key: ps name string, value: hash map(key: fragno, value: count);
} PSMap;
*/
extern DataChunk *initDataChunk();
/*
extern GBHeaps *makeGBHeaps();
extern GBACSs *makeGBACSs();
extern LMTChunk *makeLMTChunk();
extern PSMap *makePSMap();
*/
/*
 * 	type: "MIN" or "MAX";
 * 	valueType: list cell value type;
 *  valuePos: each cell is a list, [value, ps];
 * 	direction: "1" to head or "-1" to end;
 * 	removeLast: "TRUE" for remove element from heap;
 */

/*
extern List *heapifyListSiftUp(List *list, int pos, char *type, DataType valDataType);
extern List *heapifyListSiftDown(List *list, int pos, char *type, DataType valDataType);
extern List *heapInsert(List *list, char* type, Node *ele);
extern List *heapDelete(List *list, char* type, Node *ele);
*/
//extern int getIntValueFromDataChunk(DataChunk *dc, int row, int col);
//extern float getFloatValueFromDataChunk(DataChunk *dc, int pos);
//extern gprom_long getLongValueFromDataChunk(DataChunk *dc, int pos);
//extern char *getStringValueFromDataChunk(DataChunk *dc, int pos);
//extern boolean *getBooleanValueFromDataChunk(DataChunk *dc, int pos);

//extern int* getIntArrayFromDataChunk(DataChunk *dc, int col);
//extern gprom_float* getFloatArray(DataChunk *dc, int col);
//extern int* getIntArray(DataChunk *dc, int col);
//extern int* getIntArray(DataChunk *dc, int col);
//extern int* getIntArray(DataChunk *dc, int col);


#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_ */
