

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_

#include "model/set/vector.h"
#include "model/bitset/bitset.h"
#include "model/query_operator/query_operator.h"

/*
updateIdentifier:
	a Vector
		storing chars "+"/"-" indicates "insert" or "delete"; 1 / -1;

tuples:
 	a Vector
  		each vector node of tuples is a vector storing the tuples' data;
  		each vector node should be a vector which is a column of table values of same data type;
 (1, 2) (3, 4) (5, 6)
 (1, 3, 5);
 (2, 4, 6);

need vecLong/ vecFloat/ vecDouble to be more efficient to access data;



fragmentsInfo:
	a HashMap(key: prov_r_a1, value: list of bitset) -> insert tuple:(1, 2, 4) its bitset: 10000,
 		two problems: keep a pointer of current element,
  		ps_attr -> bitset list;
 		a HashMap(key: prov_r_a1, value: vector of bitset);

numTuples:
  	integer
  		numbers of tuples;
  		this value is equivalent to size of "list cell" in "tuples" list;

tupleFields:
  	integer
  		attribute numbers of each tuple;
  		this value is equivalent to size of "tuples" list
attriToPos:
  	a HashMap
  		Key: attribute name, Value: column index in "tuples" list;
posToDatatype:
  	a HashMap
  		Key: index of "tuple" list, Value: datatype of this column;
*/
typedef struct DataChunk
{
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

#define DATA_CHUNK_PROP "DATA_CHUNK"
#define INCREMENTAL_UPDATE_TIMER "module - update provenance sketch - incremental update all steps"
#define INCREMENTAL_FETCHING_DATA_TIMER "module - update provenance sketch - incremental update fetching data"
#define INCREMENTAL_JOIN_TIMER "module - update provenance sketch - incremental update join fetching data"
#define INCREMENTAL_FETCHING_DATA_BUILD_QUERY_TIMER "module - update provenance sketch - incremental update fetching data - build query"

// define left and right update for join;
#define JOIN_LEFT_BRANCH_DELTA_TABLE backendifyIdentifier("JOIN_LEFT_DELTA_TABLE")
#define JOIN_LEFT_BRANCH_IDENTIFIER backendifyIdentifier("JOIN_LEFT_TUPLE_IDENTIFIER")
#define JOIN_RIGHT_BRANCH_DELTA_TABLE backendifyIdentifier("JOIN_RIGHT_DELTA_TABLE")
#define JOIN_RIGHT_BRANCH_IDENTIFIER backendifyIdentifier("JOIN_RIGHT_TUPLE_IDENTIFIER")

#define GROUP_JOIN_START "GROUP_JOIN_START"


// functions;
extern char* update_ps_incremental(QueryOperator * qbModel, QueryOperator *updateStmt);
extern BitSet *setFragmentToBitSet(int value, List *rangeList);
extern int setFragmengtToInt(int value, Vector *rangeList);
extern DataChunk *initDataChunk();

#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_ */
