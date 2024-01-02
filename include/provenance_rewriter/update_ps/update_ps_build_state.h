

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_BUILD_STATE_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_BUILD_STATE_H_

#include "model/bitset/bitset.h"
#include "model/set/vector.h"
#include "model/set/hashmap.h"
#include "model/query_operator/query_operator.h"
#include "model/expression/expression.h"
#include "model/list/list.h"

/* macros for heap */
#define LCHILD_POS(pos) (2 * pos + 1)
#define RCHILD_POS(pos) (2 * pos + 2)
#define PARENT_POS(pos) ((pos - 1) / 2)

#define MIN_HEAP "MIN_HEAP"
#define MAX_HEAP "MAX_HEAP"

#define ORDER_BY_ATTR_NUMS "ORDER_BY_ATTR_NUMS"
#define ORDER_BY_ASCS "ORDER_BY_ASCs"
#define ORDER_BY_ATTRS "ORDER_BY_ATTRS"
#define ORDER_BY_IS_PS_INT "ORDER_BY_IS_PS_INT"
#define ORDER_BY_PS_LENS "ORDER_BY_PS_LENS"
#define JOIN_LEFT_BLOOM "JOIN_LEFT_BLOOM"
#define JOIN_RIGHT_BLOOM "JOIN_RIGHT_BLOOM"
#define JOIN_LEFT_BLOOM_ATT_MAPPING "JOIN_LEFT_BLOOM_ATT_MAPPING"
#define JOIN_RIGHT_BLOOM_ATT_MAPPING "JOIN_RIGHT_BLOOM_ATT_MAPPING"

/* macros for agg functions rewrite */
#define ADD_FUNC_PREFIX backendifyIdentifier("ADD_FUNC_PREFIX")

/* macros for timer */
#define BUILD_STATE_TIMER "module - update provenance sketch - build operators state"

typedef struct GBHeaps
{
	NodeTag  type;
	/*
		data value type(int, float,...)
	*/
	DataType valType;
	/*
		key  : group by value: like "2"
		value: RBT to store values;
	*/
	HashMap  *heapLists;
	/*
		key  : prov_attr_string, like prov_r_a1,prov_s_d1
		value: hashmap:
				key  : group by value,
				value: bitset for this group by
	*/
	HashMap   *provSketchs;
	/*
		MIN_HEAP or MAX_HEAP
	*/
	Constant *heapType;
	/*
		key  : prov_attr_string, like prov_lineitem_key1;
		value: hashmap:
				key  : group by value,
				value: tuple count
	*/
	HashMap  *fragCount;

} GBHeaps;

/*
	GBACSs: group by avg, cnt and sum
 	avg, count, sum : use same data structure GBACSs
*/
typedef struct GBACSs
{
	NodeTag type;
	/*
		key: group by value
		value: list:
		for cnt: one value   : cnt
		for sum: two values  : sum, cnt
		for avg: three values: avg, sum, cnt
	*/
	HashMap *map;


	/*
		key  : prov_attr_string, like prov_r_a1,prov_s_d1
		value: hashmap:
				key  : group by value,
				value: bitset for this group by
	*/
	HashMap *provSketchs;


	/*
		key  : prov_attr_string, like prov_lineitem_key1;
		value: hashmap:
				key  : group by value,
				value: hashMap
						key: fragment info
						value: count;
	*/
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
	/*
		key  : ps_attr_string, like prov_lineitem_key1, prov_r_a1
		value: hashMap:
				key  : fragment info like 1, 2
				value: tuple count
	*/
	HashMap *fragCount; // key: prov_r_a, value: hash map(key: fragno, value: count);
	HashMap *provSketchs; // key prov_r_a, prov_s_b; value :vector(bitset or int);
	HashMap *isIntSketch; // key:prov_r_a, value: boolean;
	HashMap *provLens; // key:prov_r_a, value: length(int);
} PSMap;

extern QueryOperator *buildState(QueryOperator *op);
extern GBHeaps *makeGBHeaps();
extern GBACSs *makeGBACSs();
extern LMTChunk *makeLMTChunk();
extern PSMap *makePSMap();

extern QueryOperator *captureRewriteOp(ProvenanceComputation *pc, QueryOperator *op);
extern int compareTwoValues(Constant *a, Constant *b, DataType dt);
extern Constant *makeValue(DataType dataType, char* value);
extern List *heapifyListSiftUp(List *list, int pos, char *type, DataType valDataType);
extern List *heapifyListSiftDown(List *list, int pos, char *type, DataType valDataType);
extern List *heapInsert(List *list, char* type, Node *ele);
extern List *heapDelete(List *list, char* type, Node *ele);
extern boolean hasLimitOpAbove(QueryOperator *op);
extern boolean hasOrderOpBelow(QueryOperator *op);
#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_BUILD_STATE_H_ */
