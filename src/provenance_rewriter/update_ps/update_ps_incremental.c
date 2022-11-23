/*
 * header
 */

#include "configuration/option.h"
#include "common.h"
#include "log/logger.h"
#include "analysis_and_translate/translator.h"
#include "analysis_and_translate/translate_update.h"
#include "mem_manager/mem_mgr.h"
#include "metadata_lookup/metadata_lookup_postgres.h"
#include "model/bitset/bitset.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/query_block/query_block.h"
#include "model/relation/relation.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "sql_serializer/sql_serializer.h"
#include "utility/string_utils.h"

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "provenance_rewriter/update_ps/update_ps_incremental.h"

// update for each operator;
static void updateByOperators(QueryOperator * op);
static void updateProvenanceComputation(QueryOperator* op);
static void updateTableAccess(QueryOperator* op);
static void updateProjection(QueryOperator* op);
static void updateSelection(QueryOperator* op);
static void updateJoin(QueryOperator* op);
static void updateAggregation(QueryOperator* op);
static void updateDuplicateRemoval(QueryOperator* op);
static void updateSet(QueryOperator* op);
static void updateOrder(QueryOperator *op);
static void updateLimit(QueryOperator * op);
static void buildStateAggregation(QueryOperator *op);
static void buildStateDuplicateRemoval(QueryOperator *op);
static void buildStateLimit(QueryOperator *op);
// OrderOperator

static DataChunk *getDataChunkFromUpdateStatement(QueryOperator* op, TableAccessOperator *tableAccessOp);
static void getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfDelete(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfUpdate(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static Relation *getQueryResult(char* sql);
static Constant *makeValue(DataType dataType, char* value);
static void executeQueryWithoutResult(char* sql);
static DataChunk *filterDataChunk(DataChunk* dataChunk, Node* condition);
static void buildState(Node *node);
static QueryOperator *captureRewrite(QueryOperator *operator);
static int compareTwoValues(Constant *a, Constant *b, DataType dt);
static void swapListCell(List *list, int pos1, int pos2);
static BitSet *setFragmentToBitSet(int value, List *rangeList);
static ConstRelOperator *getConstRelOpFromDataChunk(DataChunk *dataChunk);
static ColumnChunk *makeColumnChunk(DataType dataType, size_t len);
static ColumnChunk *evaluateExprOnDataChunk(Node *expr, DataChunk *dc);
static ColumnChunk *evaluateOperatorOnDataChunk(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorPlus(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorMinus(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorMult(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorDiv(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorMod(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorAnd(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorOr(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorNot(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorEq(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorLt(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorLe(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorGt(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorGe(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorNeq(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorConcat(Operator *op, DataChunk *dc);
static ColumnChunk *evaluateOperatorLike(Operator *op, DataChunk *dc);
static ColumnChunk *getColumnChunkOfAttr(char *attrName, DataChunk *dataChunk);
static ColumnChunk *getColumnChunkOfConst(Constant *c, DataChunk *dc);
static ColumnChunk *castColumnChunk(ColumnChunk *cc, DataType fromType, DataType toType);
static Vector *columnChunkToVector(ColumnChunk *cc);
static BitSet *computeIsNullBitSet(Node *expr, DataChunk *dc);
static int limitCmp(const void **a, const void **b);
static char *constToString(Constant *c);
//static void setNumberToEachOperator(QueryOperator *operator);

#define SQL_PRE backendifyIdentifier("SELECT * FROM ")
#define VALUES_IDENT backendifyIdentifier(" VALUES ")
#define ADD_FUNC_PREFIX backendifyIdentifier("ADD_FUNC_PREFIX")
#define LEFT_BRANCH_OF_JOIN backendifyIdentifier("LEFT_BRANCH_OF_JOIN")
#define RIGHT_BRANCH_OF_JOIN backendifyIdentifier("RIGHT_BRANCH_OF_JOIN")
#define LEFT_UPDATE_IDENTIFIER backendifyIdentifier("LEFT_UPDATE_TYPE")
#define RIGHT_UPDATE_IDENTIFIER backendifyIdentifier("RIGHT_UPDATE_TYPE")
#define NULLVALUE backendifyIdentifier("null");
// #define INSERT 1;
// #define DELETE -1;

#define MIN_HEAP "MIN_HEAP"
#define MAX_HEAP "MAX_HEAP"

#define OPERATOR_NUMBER "OPERATOR_NUMBER"
#define PROV_BIT_STRING "PROV_BIT_STRING"
#define DATA_STRUCTURE_STATE "DATA_STRUCTURE_STATE"

#define LCHILD_POS(pos) (2 * pos + 1)
#define RCHILD_POS(pos) (2 * pos + 2)
#define PARENT_POS(pos) ((pos - 1) / 2)
/*
 *  TODO:
 *	1. datachunk isNull: in multiple place: a. TableAccess, state+update: Join, Aggregation, Update: evaluate operators functions.
 *  2. add Option for Vasudha's capture data only: if having this option, for operators not TableAccess, just pass.
 * 		2.1: if having this option, need to backup the original table when first time get data,
 * 		2.2: when update, for those table access for join, agg, use the backuped table + dalta data.
 *  3.
 */

// dummy result
static StringInfo strInfo;
static QueryOperator* updateStatement = NULL;
static psInfo *PS_INFO = NULL;
static ProvenanceComputation *PC = NULL;
static HashMap *limitAttrPoss;
static List *limitOrderBys;
//static HashMap *coarseGrainedRangeList = NULL;

List *
heapInsert(List *list, char *type, Node *ele)
{
	// append to tail of list;
	list = appendToTailOfList(list, ele);

	// heapify -- sift up;
	heapifyListSiftUp(list, LIST_LENGTH(list) - 1, type, ((Constant *) ele)->constType);

	return list;
}

List *
heapifyListSiftUp(List *list, int pos, char *type, DataType valDataType)
{
	while (pos > 0) {
		int parentPos = PARENT_POS(pos);

		// get current value;
		Constant *currVal = (Constant *) getNthOfListP(list, pos);

		// get parent value;
		Constant *parentVal = (Constant *) getNthOfListP(list, parentPos);

		// compare two value;
		int compTwoVal = compareTwoValues(currVal, parentVal, valDataType);

		if (compTwoVal < 0) {
			if (strcmp(type, MIN_HEAP) == 0) { // current < parent, for min heap, continue;
				swapListCell(list, pos, parentPos);
			} else { // terminate for max heap when current < parent;
				return list;
			}
		} else if (compTwoVal > 0) {
			if (strcmp(type, MAX_HEAP) == 0) { // current > parent, for max heap, continue;
				swapListCell(list, pos, parentPos);
			} else { // terminate for min heap when min heap;
				return list;
			}
		} else { // terminate for both min and max heap when current == parent;
			return list;
		}

		pos = parentPos;
	}

	return list;
}


List *
heapDelete(List *list, char *type, Node *ele)
{
	for (int i = 0; i < LIST_LENGTH(list); i++) {
		if (equal((Node *) getNthOfListP(list, i), ele)) {
			list = heapifyListSiftDown(list, i, type, ((Constant *) ele)->constType);
			break;
		}
	}
	return list;
}

List *
heapifyListSiftDown(List *list, int pos, char *type, DataType valDataType)
{
	while (LCHILD_POS(pos) < LIST_LENGTH(list)) {
		// get lChild;
		// List *lChild = (List *) getNthOfListP(list, LCHILD_POS(pos));
		Constant *lChildVal = (Constant *) getNthOfListP(list, LCHILD_POS(pos));

		if (RCHILD_POS(pos) < LIST_LENGTH(list)) { // has two children
			// List *rChild = (List *) getNthOfListP(list, RCHILD_POS(pos));
			Constant *rChildVal = (Constant *) getNthOfListP(list, RCHILD_POS(pos));

			int compTwoVal = compareTwoValues(lChildVal, rChildVal, valDataType);

			if (compTwoVal == 0) {
				// two children have the same value, either one is ok;
				swapListCell(list, pos, LCHILD_POS(pos));
				pos = LCHILD_POS(pos);

			} else if (compTwoVal < 0) {
				// lChild is smaller, MIN_HEAP: lChild, MAX_HEAP: rChild;
				if (strcmp(type, MIN_HEAP) == 0) {
					swapListCell(list, pos, LCHILD_POS(pos));
					pos = LCHILD_POS(pos);

				} else {
					swapListCell(list, pos, RCHILD_POS(pos));
					pos = RCHILD_POS(pos);
				}
			} else {
				// lChild is larger,
				if (strcmp(type, MIN_HEAP) == 0) {
					swapListCell(list, pos, RCHILD_POS(pos));
					pos = RCHILD_POS(pos);

				} else {
					swapListCell(list, pos, LCHILD_POS(pos));
					pos = LCHILD_POS(pos);
				}
			}
		} else { // only has left child;
			swapListCell(list, pos, LCHILD_POS(pos));
			pos = LCHILD_POS(pos);
		}
	}

	// re-heapify from "pos" since it is the last removed position;
	List *newList = NIL;
	for (int i = 0; i < LIST_LENGTH(list); i++) {
		// skip value at pos;
		if (i == pos) {
			continue;
		}

		newList = appendToTailOfList(newList, (List *) copyObject((List *) getNthOfListP(list, i)));

		if (i > pos) {
			heapifyListSiftUp(newList, i, type, valDataType);
		}
	}

	return newList;
}

static void
swapListCell(List *list, int pos1, int pos2)
{
	ListCell *lc1 = getNthOfList(list, pos1);
	ListCell *lc2 = getNthOfList(list, pos2);

	void *ptr = lc1->data.ptr_value;
	lc1->data.ptr_value = lc2->data.ptr_value;
	lc2->data.ptr_value = ptr;
}

static int
compareTwoValues(Constant *a, Constant *b, DataType dt)
{
	int result = 0;
	switch (dt) {
		case DT_INT:
			result = INT_VALUE(a) - INT_VALUE(b);
			break;
		case DT_LONG:
			result = (LONG_VALUE(a) - LONG_VALUE(b) < 0 ? -1 : 1);
			break;
		case DT_FLOAT:
			result = (FLOAT_VALUE(a) - FLOAT_VALUE(b) < 0 ? -1 : 1);
			break;
		case DT_STRING:
		case DT_VARCHAR2:
			result = strcmp(STRING_VALUE(a), STRING_VALUE(b));
			break;
		case DT_BOOL:
			result = BOOL_VALUE(a) - BOOL_VALUE(b);
			break;
		default:
			ERROR_LOG("data type %d is not supported", dt);
	}
	return result;
}

DataChunk*
initDataChunk()
{
	DataChunk* dc = makeNode(DataChunk);

	dc->attrNames = NIL;
	dc->updateIdentifier = makeVector(VECTOR_INT, T_Vector);
	dc->tuples = makeVector(VECTOR_NODE, T_Vector);
	dc->fragmentsInfo = NEW_MAP(Constant, Node);
	dc->numTuples = 0;
	dc->tupleFields = 0;
	dc->attriToPos = NEW_MAP(Constant, Constant);
	dc->posToDatatype = NEW_MAP(Constant, Constant);
	dc->isNull = makeVector(VECTOR_NODE, T_Vector);

	return dc;
}

GBHeaps *
makeGBHeaps()
{
	GBHeaps *gbHeaps = makeNode(GBHeaps);

	gbHeaps->fragCount = NEW_MAP(Constant, Constant);
	gbHeaps->heapLists = NEW_MAP(Constant, Node);
	gbHeaps->provSketchs = NEW_MAP(Constant, Node);
	gbHeaps->heapType = NULL;
	gbHeaps->valType = 0;

	return gbHeaps;
}

GBACSs *
makeGBACSs()
{
	GBACSs *acs = makeNode(GBACSs);

	acs->provSketchs = NEW_MAP(Constant, Node);
	acs->map = NEW_MAP(Constant, Node);
	acs->fragCount = NEW_MAP(Constant, Node);

	return acs;
}

LMTChunk *
makeLMTChunk()
{
	LMTChunk *lmtC = makeNode(LMTChunk);

	lmtC->attrToPos = NEW_MAP(Constant, Constant);
	lmtC->posToDatatype = NEW_MAP(Constant, Constant);
	lmtC->vals = NIL;
	lmtC->provToPos = NEW_MAP(Constant, Constant);
	lmtC->tupleFields = 0;
	lmtC->numTuples = 0;

	return lmtC;
}

char *
update_ps_incremental(QueryOperator* operator){
	// get partition information;
	PC = (ProvenanceComputation *) copyObject(operator);
	PS_INFO = createPSInfo((Node *) getStringProperty(operator, PROP_PC_COARSE_GRAINED));


	DEBUG_NODE_BEATIFY_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR: \n", operator);
	INFO_OP_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR", operator);
	strInfo = makeStringInfo();

	// left  child: update statement;
	// right child: query;
	updateStatement = (QueryOperator *) OP_LCHILD(operator);
	QueryOperator *rChild = OP_RCHILD(operator);

	operator->inputs = singleton(rChild);

	// build state for each aggregation operator
	buildState((Node *) operator);

	if (1 != 1) {
		return "FINISH";
	}
	updateByOperators((QueryOperator*) OP_LCHILD(operator));
	return strInfo->data;
}

static void
buildState(Node *node)
{
	// return when it is a table access operator;
	if (isA(node, TableAccessOperator)) {
		return ;
	}

	// post order traverse;
	FOREACH(Node, n, ((QueryOperator *) node)->inputs) {
		buildState(n);
	}

	INFO_LOG("NODE: %d", node->type);

	// build data structure for aggregation operator;
	if (isA(node, AggregationOperator)) {
		buildStateAggregation((QueryOperator *) node);
	} else if (isA(node, DuplicateRemoval)) {
		buildStateDuplicateRemoval((QueryOperator *) node);
	} else if (isA(node, LimitOperator)) {
		buildStateLimit((QueryOperator *) node);
	}
}

static void
buildStateLimit(QueryOperator *op)
{
	/* support ONLY for ORDER BY - LIMIT */
	/* only LIMIT without ORDER BY is not supported */
	DEBUG_NODE_BEATIFY_LOG("limit Op", op);

	QueryOperator *lchild = (QueryOperator *) copyObject(OP_LCHILD(op));
	/* if lchild is not a OrderOperator, not support currently */
	if(!isA(lchild, OrderOperator)) {
		return;
	}

	QueryOperator *rewrOp = captureRewrite(lchild);

	char *sql = serializeQuery(rewrOp);

	// get data;
	Relation *resultRel = resultRel = executeQuery(sql);

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOp), PROP_LEVEL_AGGREGATION_MARK), 0);
	FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
		int pos = listPosString(resultRel->schema, STRING_VALUE(c));
		List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
		l = appendToTailOfList(l, createConstInt(pos));
	}

	List *attrDefs = (List *) copyObject(lchild->schema->attrDefs);

	LMTChunk *limitC = makeLMTChunk();
	limitC->numTuples = LIST_LENGTH(resultRel->tuples);
	limitC->tupleFields = LIST_LENGTH(attrDefs);

	List *vals = NIL;
	int fields = LIST_LENGTH(attrDefs);

	for (int row = 0; row < LIST_LENGTH(resultRel->tuples); row++) {
		List *tuple = (List *) getNthOfListP(resultRel->tuples, row);
		List *val = NIL;
		for (int col = 0; col < fields; col++) {
			DataType dt = ((AttributeDef *) getNthOfListP(attrDefs, col))->dataType;
			if (row == 0) {
				addToMap(limitC->attrToPos, (Node *) createConstString((char *) getNthOfListP(resultRel->schema, col)), (Node *) createConstInt(col));
				addToMap(limitC->posToDatatype, (Node *) createConstInt(col), (Node *) createConstInt(dt));
			}
			Constant *c = makeValue(dt, (char *) getNthOfListP(tuple, col));
			val = appendToTailOfList(val, c);
		}

		// prove sketch;
		for (int col = fields; col < LIST_LENGTH(resultRel->schema); col++) {

			char *provName = (char *) getNthOfListP(resultRel->schema, col);
			char *prov = (char *) getNthOfListP(tuple, col);

			if (row == 0) {
				addToMap(limitC->provToPos, (Node *) createConstString(provName), (Node *) createConstInt(col));
			}

			List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, provName);
			int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
			int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));

			BitSet *provBit = NULL;
			if (provLevel < 2) {
				provBit = newBitSet(provNumFrag);
				setBit(provBit, atoi(prov), TRUE);
			} else {
				provBit = stringToBitset(prov);
			}

			val = appendToTailOfList(val, provBit);
		}

		vals = appendToTailOfList(vals, val);
	}
	limitC->vals = vals;

	// set data structure prop;
	SET_STRING_PROP(op, DATA_STRUCTURE_STATE, limitC);
}

static void
buildStateDuplicateRemoval(QueryOperator *op)
{
	// for duplicate remove: rewrite like group by no agg
	DuplicateRemoval *dupRem = (DuplicateRemoval *) op;

	// create gb list;
	List *gbList = NIL;
	int pos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		AttributeReference *ar = createFullAttrReference(ad->attrName, 0, pos, 0, ad->dataType);
		gbList = appendToTailOfList(gbList, ar);
		pos++;
	}
	List *attrNames = (List *) getAttrNames(dupRem->op.schema);

	AggregationOperator *aggOp = createAggregationOp(NIL, gbList, (QueryOperator *) copyObject(OP_LCHILD(op)), NIL, attrNames);
	SET_STRING_PROP((QueryOperator *) aggOp, PROP_COARSE_GRAINED_AGGREGATION_MARK_UPDATE_PS, createConstBool(TRUE));
	OP_LCHILD((QueryOperator *) aggOp)->parents = singleton(aggOp);

	DEBUG_NODE_BEATIFY_LOG("NEW AGG", aggOp);
	QueryOperator *rewrOp = captureRewrite((QueryOperator *) aggOp);
	DEBUG_NODE_BEATIFY_LOG("REWR AGG", rewrOp);

	char *sql = serializeQuery(rewrOp);

	Relation *resultRel = executeQuery(sql);

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOp), PROP_LEVEL_AGGREGATION_MARK), 0);
	FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
		int pos = listPosString(resultRel->schema, STRING_VALUE(c));
		List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
		l = appendToTailOfList(l, createConstInt(pos));
	}

	// make data structure;
	GBACSs *acs = makeGBACSs();
	List *gbAttrPos = NIL;
	FOREACH(AttributeReference, ar, gbList) {
		int pos = listPosString(resultRel->schema, ar->name);
		gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(pos));
	}

	int	groupCountPos = listPosString(resultRel->schema, backendifyIdentifier("count_per_group"));

	for (int i = 0; i < LIST_LENGTH(resultRel->tuples); i++) {
		List *tuple = (List *) getNthOfListP(resultRel->tuples, i);

		// build gb identifier;
		StringInfo gbVals = makeStringInfo();
		for (int j = 0; j < LIST_LENGTH(gbAttrPos); j++) {
			appendStringInfo(gbVals, "%s#", (char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, j))));
		}

		List *l = NIL;
		List *newL = NIL;

		if (MAP_HAS_STRING_KEY(acs->map, gbVals->data)) {
			l = (List *) MAP_GET_STRING(acs->map, gbVals->data);
		} else {
			l = appendToTailOfList(l, createConstInt(0));
		}

		int cnt = INT_VALUE((Constant *) getNthOfListP(l, 0));
		newL = appendToTailOfList(newL, createConstInt(cnt + 1));
		addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));

		// for provenance sketch;
		FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
			List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
			int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
			int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));
			int provPos = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 2));

			// get group provcesketch;
			BitSet *gbProvSketch = NULL;
			if (MAP_HAS_STRING_KEY(acs->provSketchs, STRING_VALUE(c))) {
				gbProvSketch = (BitSet *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
			} else {
				gbProvSketch = newBitSet(provNumFrag);
			}

			// get group fragment count map;
			HashMap *gbFragCount = NULL;
			if (MAP_HAS_STRING_KEY(acs->fragCount, STRING_VALUE(c))) {
				gbFragCount = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
			} else {
				gbFragCount = NEW_MAP(Constant, Constant);
			}

			char *prov = (char *) getNthOfListP(tuple, provPos);
			int groupCount = atoi((char *) getNthOfListP(tuple, groupCountPos));

			if (provLevel < 2) {
				int fragNo = atoi(prov);
				if (MAP_HAS_INT_KEY(gbFragCount, fragNo)) {
					int gCnt = INT_VALUE((Constant *) MAP_GET_INT(gbFragCount, fragNo));
					addToMap(gbFragCount, (Node *) createConstInt(fragNo), (Node *) createConstInt(gCnt + groupCount));
				} else {
					addToMap(gbFragCount, (Node *) createConstInt(fragNo), (Node *) createConstInt(groupCount));
				}

				setBit(gbProvSketch, fragNo, TRUE);
			} else {
				for (int psIdx = 0; psIdx < strlen(prov); psIdx++) {
					if (prov[psIdx] == '1') {
						if (MAP_HAS_INT_KEY(gbFragCount, psIdx)) {
							int gCnt = INT_VALUE((Constant *) MAP_GET_INT(gbFragCount, psIdx));
							gCnt += groupCount;
							addToMap(gbFragCount, (Node *) createConstInt(psIdx), (Node *) createConstInt(gCnt));
						} else {
							addToMap(gbFragCount, (Node *) createConstInt(psIdx), (Node *) createConstInt(groupCount));
						}
					}
				}

				gbProvSketch = bitOr(stringToBitset(prov), gbProvSketch);
			}

			addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) copyObject(gbProvSketch));
			addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) copyObject(gbFragCount));
		}
	}

	SET_STRING_PROP(op, DATA_STRUCTURE_STATE, (Node *) acs);
}

static void
buildStateAggregation(QueryOperator *op)
{
	AggregationOperator *aggOp = (AggregationOperator *) op;
	DEBUG_NODE_BEATIFY_LOG("CURRENT AGG", aggOp);
	INFO_OP_LOG("CURRENT AGG", aggOp);

	// get aggregation operator args and groupbys;
	List *aggrFCs = (List *) copyObject(aggOp->aggrs);
	List *aggrGBs = (List *) copyObject(aggOp->groupBy);

	// split this aggOp into two cagetories: min/max and avg/count/sum;
	List *min_max = NIL;
	List *min_max_attNames = NIL;

	List *avg_sum_count = NIL;
	List *avg_sum_count_attNames = NIL;

	DEBUG_NODE_BEATIFY_LOG("aggrs list", aggrFCs);
	for (int i = 0; i < LIST_LENGTH(aggrFCs); i++) {
		FunctionCall * fc = (FunctionCall *) getNthOfListP(aggrFCs, i);
		DEBUG_NODE_BEATIFY_LOG("function call:\n", fc);

		char * fcName = fc->functionname;
		AttributeDef *ad = copyObject(getNthOfListP(((QueryOperator *) aggOp)->schema->attrDefs, i));

		// find all min/max function calls;
		if (strcmp(fcName, MIN_FUNC_NAME) == 0 || strcmp(fcName, MAX_FUNC_NAME) == 0) {
			min_max = appendToTailOfList(min_max, copyObject((FunctionCall *) fc));
			min_max_attNames = appendToTailOfList(min_max_attNames, ad);
		}

		// find all avg/count/sum function calls;
		if (strcmp(fcName, AVG_FUNC_NAME) == 0 || strcmp(fcName, SUM_FUNC_NAME) == 0
		 || strcmp(fcName, COUNT_FUNC_NAME) == 0) {
			// add current function call;
			avg_sum_count = appendToTailOfList(avg_sum_count, copyObject((FunctionCall *) fc));
			avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, strdup(ad->attrName));

			// add function calls sum and count for avg;
			if (strcmp(fcName, AVG_FUNC_NAME) == 0) {
				// add function call sum;
				FunctionCall *fc_sum = createFunctionCall(SUM_FUNC_NAME, copyList(fc->args));
				avg_sum_count = appendToTailOfList(avg_sum_count, fc_sum);

				StringInfo sum_fc_name = makeStringInfo();
				appendStringInfo(sum_fc_name, "%s_%s_%s", ADD_FUNC_PREFIX, SUM_FUNC_NAME, backendifyIdentifier(ad->attrName));
				avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, sum_fc_name->data);

				// add function call count; not need this anymore since there is a "count_per_group";
				// FunctionCall *fc_cnt = createFunctionCall(COUNT_FUNC_NAME, copyList(fc->args));
				// avg_sum_count = appendToTailOfList(avg_sum_count, fc_cnt);

				// StringInfo cnt_fc_name = makeStringInfo();
				// appendStringInfo(cnt_fc_name, "%s_%s_%s", ADD_FUNC_PREFIX, COUNT_FUNC_NAME, ad->attrName);
				// avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, cnt_fc_name->data);
			}
		}
	}

	DEBUG_NODE_BEATIFY_LOG("min max fc list", min_max);
	DEBUG_NODE_BEATIFY_LOG("avg, sum, count fc list", avg_sum_count);

	// get min_max rewrite;
	// for min and max function: run capture-rewritted child;
	// insert into the min/max heap based on group by attribute(s);
	if (LIST_LENGTH(min_max) != 0) {
		// get child operator;
		QueryOperator *child = OP_LCHILD(aggOp);

		// get capture rewrite
		QueryOperator *rewrOp = captureRewrite((QueryOperator *) copyObject(child));
		DEBUG_NODE_BEATIFY_LOG("REWR OP", rewrOp);
		INFO_OP_LOG("REWR OP", rewrOp);
		// searialize operator;
		char *sql = serializeQuery(rewrOp);

		// get tuples;
		Relation *resultRel = NULL;
		resultRel = executeQuery(sql);
		// INFO_LOG("schema name: %s", stringListToString(resultRel->schema));

		// get prov attr name;
		QueryOperator *auxOP = captureRewrite((QueryOperator *) copyObject(aggOp));
		HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(auxOP), PROP_LEVEL_AGGREGATION_MARK), 0);

		// append to each map value a "prov_attr_pos_in_result" indicating this pos in "resultRel"
		// DEBUG_NODE_BEATIFY_LOG("BEFORE APPEND", psAttrAndLevel);
		FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
			int pos = listPosString(resultRel->schema, STRING_VALUE(c));
			List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
			l = appendToTailOfList(l, createConstInt(pos));
		}
		// DEBUG_NODE_BEATIFY_LOG("AFTER APPEND", psAttrAndLevel);

		for (int i = 0; i < LIST_LENGTH(min_max); i++) {

			// get this function call;
			FunctionCall *fc = (FunctionCall *) getNthOfListP(min_max, i);
			AttributeReference *attrRef = (AttributeReference *) getNthOfListP(fc->args, 0);

			// set this min/max heap a name like: min_a, max_b
			Constant *heapName = createConstString(CONCAT_STRINGS(fc->functionname, "_", attrRef->name));

			// make a GBHeaps for this function call;
			GBHeaps *gbHeaps = makeGBHeaps();

			// set value type:
			gbHeaps->valType = attrRef->attrType;

			// set heap type: MIN or MAX;
			if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
				gbHeaps->heapType = createConstString(MIN_HEAP);
			} else {
				gbHeaps->heapType = createConstString(MAX_HEAP);
			}

			// get function call attribute position in result "relResult"
			int fcAttrPos = listPosString(resultRel->schema, backendifyIdentifier(attrRef->name));
			// INFO_LOG("fc attr pos %d", fcAttrPos);
			// get group by attributes positions in result "resultRel";
			// get the pos and its type; so if iterate, +2;
			List *gbAttrPos = NIL;
			FOREACH(AttributeReference, ar, aggrGBs) {
				int pos = listPosString(resultRel->schema, backendifyIdentifier(ar->name));
				gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(pos));
				// gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(ar->attrType));
			}

			// iterate over result "resultRel" to build the heap;
			for (int ii = 0; ii < LIST_LENGTH(resultRel->tuples); ii++) {
				List *tuple = (List *) getNthOfListP(resultRel->tuples, ii);
				// INFO_LOG("tuple values: %s", stringListToString(tuple));
				// get group by value of this tuple; TODO: isNULL
				StringInfo gbVals = makeStringInfo();
				for (int j = 0; j < LIST_LENGTH(gbAttrPos); j++) {
					appendStringInfo(gbVals, "%s#", (char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, j))));
				}

				// check if gbHeaps has this group by value;
				List *heapList = NIL;
				if (MAP_HAS_STRING_KEY(gbHeaps->heapLists, gbVals->data)) {
					heapList = (List *) MAP_GET_STRING(gbHeaps->heapLists, gbVals->data);
				}

				// get min/max attribute value;
				char *val = (char *) getNthOfListP(tuple, fcAttrPos);
				// INFO_LOG("val str %s", val);
				Constant *value = makeValue(attrRef->attrType, val);
				// DEBUG_NODE_BEATIFY_LOG("CONSTANT", value);
				// DEBUG_NODE_BEATIFY_LOG("HEAPLIST", heapList);
				// insert this value to heap and heapify
				heapList = heapInsert(heapList, STRING_VALUE(gbHeaps->heapType), (Node *) value);

				// DEBUG_NODE_BEATIFY_LOG("HEAPLIST", heapList);
				// heap list done, add to map;
				addToMap(gbHeaps->heapLists, (Node *) createConstString(gbVals->data), (Node *) copyObject(heapList));

				// BELOW: DEALING WITH PROVENANCE SKETCHS;

				// get all provenance sketch attrs;
				FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
					// get provenance sketch info:
					List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
					int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
					int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));
					int provPos = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 2));

					// get group provenance sketch;
					BitSet *gbProvSketch = NULL;
					if (MAP_HAS_STRING_KEY(gbHeaps->provSketchs, STRING_VALUE(c))) {
						gbProvSketch = (BitSet *) MAP_GET_STRING(gbHeaps->provSketchs, STRING_VALUE(c));
					} else {
						gbProvSketch = newBitSet(provNumFrag);
					}

					// get group fragment count map;
					HashMap *gbFragCount = NULL;
					if (MAP_HAS_STRING_KEY(gbHeaps->fragCount, STRING_VALUE(c))) {
						gbFragCount = (HashMap *) MAP_GET_STRING(gbHeaps->fragCount, STRING_VALUE(c));
					} else {
						gbFragCount = NEW_MAP(Constant, Constant);
					}

					// get provenance sketch value:
					char *prov = (char *) getNthOfListP(tuple, provPos);

					// dealing with group provenance sketch and fragment-count;
					if (provLevel < 2) { // prov is an integer;
						// get fragment number;
						int whichFrag = atoi(prov);
						if (MAP_HAS_INT_KEY(gbFragCount, whichFrag)) {
							int cnt = INT_VALUE((Constant *) MAP_GET_INT(gbFragCount, whichFrag)) + 1;
							addToMap(gbFragCount, (Node *) createConstInt(whichFrag), (Node *) createConstInt(cnt));
						} else {
							addToMap(gbFragCount, (Node *) createConstInt(whichFrag), (Node *) createConstInt(1));
						}

						// dealing with group provenance sketch;
						setBit(gbProvSketch, whichFrag, TRUE);
					} else { // prov is a string;
						for (int k = 0; k < strlen(prov); k++) {
							if (prov[k] == '1') {
								// dealing with frag-count;
								if (MAP_HAS_INT_KEY(gbFragCount, k)) {
									int cnt = INT_VALUE(MAP_GET_INT(gbFragCount, k)) + 1;
									addToMap(gbFragCount, (Node *) createConstInt(k), (Node *) createConstInt(cnt));
								} else {
									addToMap(gbFragCount, (Node *) createConstInt(k), (Node *) createConstInt(1));
								}
							}
						}

						// BitOr prov bits and gbProvSketch bits;
						gbProvSketch = bitOr(stringToBitset(prov), gbProvSketch);
					}

					addToMap(gbHeaps->fragCount, (Node *) copyObject(c), (Node *) copyObject(gbFragCount));
					addToMap(gbHeaps->provSketchs, (Node *) copyObject(c), (Node *) copyObject(gbProvSketch));
				}
			}

			HashMap *dataStructures = NULL;
			if (HAS_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE)) {
				dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE);
			} else {
				dataStructures = NEW_MAP(Constant, Node);
			}
			addToMap(dataStructures, (Node *) heapName, (Node *) gbHeaps);
			SET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE, (Node *) dataStructures);
		}
	}

	// get avg/sum/count rewrite;
	if (LIST_LENGTH(avg_sum_count) != 0) {
		INFO_LOG("rewrite new agg");

		// get current aggregation input;
		QueryOperator *child = copyObject(OP_LCHILD(aggOp));
		INFO_OP_LOG("child", child);

		// append group by list to attrNames;
		FOREACH(AttributeReference, a, aggOp->groupBy) {
			avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, a->name);
		}

		// create a nwe aggregation operator;
		AggregationOperator *newAgg = createAggregationOp(avg_sum_count, (List *) copyList(aggOp->groupBy), child, NIL, avg_sum_count_attNames);
		child->parents = singleton(newAgg);

		// set a property "AGG_UPDATE_PS"
		SET_STRING_PROP((QueryOperator *) newAgg, PROP_COARSE_GRAINED_AGGREGATION_MARK_UPDATE_PS, createConstBool(TRUE));

		INFO_OP_LOG("NEW AGG BUILT", (QueryOperator *) newAgg);
		// DEBUG_NODE_BEATIFY_LOG("new AGG", newAgg);

		// rewrite capture;
		// ProvenanceComputation * newPC = copyObject(pc);
		// newPC->op.inputs = singleton(newAgg);
		// newAgg->op.parents = singleton(newPC);
		// DEBUG_NODE_BEATIFY_LOG("NEW PC:", newPC);

		QueryOperator * rewriteOP = captureRewrite((QueryOperator *) newAgg);
		// DEBUG_NODE_BEATIFY_LOG("avg_sum_count agg", rewriteOP);
		INFO_OP_LOG("rewrite op avg_sum_cont: ", rewriteOP);

		// char *sql = serializeOperatorModel((Node *) rewriteOP);
		char *sql = serializeQuery(rewriteOP);
		INFO_LOG("sql %s", sql);

		Relation *resultRel = NULL;
		resultRel = executeQuery(sql);

		// get the level of aggregation
		HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewriteOP), PROP_LEVEL_AGGREGATION_MARK), 0);
		// DEBUG_NODE_BEATIFY_LOG("LEVEL AND NUM FRAG", psAttrAndLevel);

		// HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(auxOP), PROP_LEVEL_AGGREGATION_MARK), 0);

		// append to each map value a "prov_attr_pos_in_result" indicating this pos in "resultRel"
		// DEBUG_NODE_BEATIFY_LOG("BEFORE APPEND", psAttrAndLevel);
		FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
			int pos = listPosString(resultRel->schema, STRING_VALUE(c));
			List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
			l = appendToTailOfList(l, createConstInt(pos));
		}
		// DEBUG_NODE_BEATIFY_LOG("AFTER APPEND", psAttrAndLevel);

		for (int i = 0; i < LIST_LENGTH(aggrFCs); i++) {
			FunctionCall *fc = (FunctionCall *) getNthOfListP(aggrFCs, i);

			// only focus on avg, count, sum;
			if (strcmp(fc->functionname, AVG_FUNC_NAME) != 0
			 && strcmp(fc->functionname, SUM_FUNC_NAME) != 0
			 && strcmp(fc->functionname, COUNT_FUNC_NAME) != 0) {
				continue;
			}

			// get function call attribute reference;
			AttributeReference *attrRef = (AttributeReference *) getNthOfListP(fc->args, 0);

			// set a name to this function call like : sum_a, avg_b, count_c used in outer map;
			Constant *ACSsName = createConstString(CONCAT_STRINGS(fc->functionname, "_", attrRef->name));
			// DEBUG_NODE_BEATIFY_LOG("ACSsName", ACSsName);

			// make a GBACSs;
			GBACSs *acs = makeGBACSs();

			// get function call attribute pos in result "resultRel"
			// DEBUG_NODE_BEATIFY_LOG("agg schema", aggOp->op.schema);
			List *attrDefs = (List *) copyObject(aggOp->op.schema->attrDefs);

			List *namesAndPoss = NIL;

			// append fc call name in resultRel;
			char *aggFCName = backendifyIdentifier(((AttributeDef *) getNthOfListP(attrDefs, i))->attrName);
			// namesAndPoss = appendToTailOfList(namesAndPoss, createConstString(((AttributeDef *) getNthOfListP(attrDefs, i))->attrName));

			// append pos in resultRel;
			int pos = listPosString(resultRel->schema, backendifyIdentifier(aggFCName));
			namesAndPoss = appendToTailOfList(namesAndPoss, createConstInt(pos));

			// if it is a avg, there should be sum in the result;
			if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
				StringInfo sum_fc_name = makeStringInfo();
				appendStringInfo(sum_fc_name, "%s_%s_%s", ADD_FUNC_PREFIX, SUM_FUNC_NAME, backendifyIdentifier(aggFCName));
				pos = listPosString(resultRel->schema, sum_fc_name->data);

				// appendToTailOfList(namesAndPoss, createConstString(sum_fc_name->data));
				appendToTailOfList(namesAndPoss, createConstInt(pos));
			}

			// DEBUG_NODE_BEATIFY_LOG("namesANDposs", namesAndPoss);

			// get group by attributes positions in result "resultRel";
			List *gbAttrPos = NIL;
			FOREACH(AttributeReference, ar, aggrGBs) {
				int pos = listPosString(resultRel->schema, backendifyIdentifier(ar->name));
				gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(pos));
				// gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(ar->attrType));
			}

			// for (int j = 0; j < LIST_LENGTH(resultRel->schema); j++) {
			// 	INFO_LOG("result name: %s", (char *) getNthOfListP(resultRel->schema, j));
			// }

			// get group count for avg and sum
			int groupCountPos = -1;
			if (strcmp(fc->functionname, COUNT_FUNC_NAME) != 0) {
				groupCountPos = listPosString(resultRel->schema, backendifyIdentifier("count_per_group"));
			}
			DEBUG_NODE_BEATIFY_LOG("gb list", gbAttrPos);

			for (int j = 0; j < LIST_LENGTH(resultRel->tuples); j++) {
				List *tuple = (List *) getNthOfListP(resultRel->tuples, j);

				// get the group by values;
				StringInfo gbVals = makeStringInfo();
				for (int k = 0; k < LIST_LENGTH(gbAttrPos); k++) {
					appendStringInfo(gbVals, "%s#", (char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, k))));
				}

				// value list;
				List *l = NIL;
				List *newL = NIL;
				if (MAP_HAS_STRING_KEY(acs->map, gbVals->data)) {
					l = (List *) MAP_GET_STRING(acs->map, gbVals->data);
				} else {
					if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
						l = appendToTailOfList(l, createConstFloat((double) 0));
						l = appendToTailOfList(l, createConstFloat((double) 0));
						l = appendToTailOfList(l, createConstInt(0));
					} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
						l = appendToTailOfList(l, createConstFloat((double) 0));
						l = appendToTailOfList(l, createConstInt(0));
					} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
						l = appendToTailOfList(l, createConstInt(0));
					}
				}

				if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
					// get avg, sum and cont;
					double avg = FLOAT_VALUE((Constant *) getNthOfListP(l, 0));
					double sum = FLOAT_VALUE((Constant *) getNthOfListP(l, 1));
					int count = INT_VALUE((Constant *) getNthOfListP(l, 2));

					// compute new avg, sum, count;
					int newSumPos = INT_VALUE((Constant *) getNthOfListP(namesAndPoss, 1));
					double newSum = atof((char *) getNthOfListP(tuple, newSumPos));
					sum += newSum;
					count += atoi((char *) getNthOfListP(tuple, groupCountPos));
					avg = sum / count;

					// make a new list;

					// l = listMake(createConstFloat(avg), createConstFloat(sum), createConstInt(count));
					newL = appendToTailOfList(newL, createConstFloat(avg));
					newL = appendToTailOfList(newL, createConstFloat(sum));
					newL = appendToTailOfList(newL, createConstInt(count));
				} else if (strcmp(fc->functionname, SUM_FUNC_NAME)) {
					// get previous sum and count;
					double sum = FLOAT_VALUE((Constant *) getNthOfListP(l, 0));
					int count = INT_VALUE((Constant *) getNthOfListP(l, 1));

					// compute new sum and count;
					int newSumPos = INT_VALUE((Constant *) getNthOfListP(namesAndPoss, 0));
					sum += atof((char *) getNthOfListP(tuple, newSumPos));
					count += atoi((char *) getNthOfListP(tuple, groupCountPos));

					// make a new list;
					// l = listMake(createConstFloat(sum), createConstInt(count));

					newL = appendToTailOfList(newL, createConstFloat(sum));
					newL = appendToTailOfList(newL, createConstInt(count));
				} else if (strcmp(fc->functionname, COUNT_FUNC_NAME)) {
					// previous count;
					int count = INT_VALUE((Constant *) getNthOfListP(l, 0));

					// get new count;
					count += atoi((char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(namesAndPoss, 0))));

					// make a new list;
					// l = singleton(createConstInt(count));
					newL = appendToTailOfList(newL, createConstInt(count));
				}

				// add this key-value to map;
				addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));


				// BELOW DEALING WITH PROVENANCE SKETCH;
				FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
					List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
					int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
					int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));
					int provPos = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 2));

					// get group provenance sketch;
					BitSet *gbProvSketch = NULL;
					if (MAP_HAS_STRING_KEY(acs->provSketchs, STRING_VALUE(c))) {
						gbProvSketch = (BitSet *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
					} else {
						gbProvSketch = newBitSet(provNumFrag);
					}

					// get group fragment count map;
					HashMap *gbFragCount = NULL;
					if (MAP_HAS_STRING_KEY(acs->fragCount, STRING_VALUE(c))) {
						gbFragCount = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
					} else {
						gbFragCount = NEW_MAP(Constant, Constant);
					}

					// get prov string or int;
					char *prov = (char *) getNthOfListP(tuple, provPos);
					int groupCount = atoi((char *) getNthOfListP(tuple, groupCountPos));
					if (provLevel < 2) {
						int whichFrag = atoi(prov);
						if (MAP_HAS_INT_KEY(gbFragCount, whichFrag)) {
							int cnt = INT_VALUE((Constant *) MAP_GET_INT(gbFragCount, whichFrag));
							cnt += groupCount;
							addToMap(gbFragCount, (Node *) createConstInt(whichFrag), (Node *) createConstInt(cnt));
						} else {
							addToMap(gbFragCount, (Node *) createConstInt(whichFrag), (Node *) createConstInt(groupCount));
						}

						// set whichFrag pos to '1'
						setBit(gbProvSketch, whichFrag, TRUE);
					} else {
						for (int provIdx = 0; provIdx < strlen(prov); provIdx++) {
							if (prov[provIdx] == '1') {
								if (MAP_HAS_INT_KEY(gbFragCount, provIdx)) {
									int cnt = INT_VALUE((Constant *) MAP_GET_INT(gbFragCount, provIdx));
									cnt += groupCount;
									addToMap(gbFragCount, (Node *) createConstInt(provIdx), (Node *) createConstInt(cnt));
								} else {
									addToMap(gbFragCount, (Node *) createConstInt(provIdx), (Node *) createConstInt(groupCount));
								}
							}
						}

						// BITOR prov and this string;
						gbProvSketch = bitOr(stringToBitset(prov), gbProvSketch);
					}

					addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) copyObject(gbProvSketch));
					addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) copyObject(gbFragCount));
				}
			}

			HashMap *dataStructures = NULL;
			if (HAS_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE)) {
				dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE);
			} else {
				dataStructures = NEW_MAP(Constant, Node);
			}

			addToMap(dataStructures, (Node *) ACSsName, (Node *) acs);
			SET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE, (Node *) dataStructures);
		}
	}
	DEBUG_NODE_BEATIFY_LOG("STATE", (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE));
}

static QueryOperator*
captureRewrite(QueryOperator *operator)
{
	QueryOperator* result = NULL;
	Node *coarsePara2 = NULL;
	psInfo *psPara2 = NULL;

	ProvenanceComputation *newPC = (ProvenanceComputation *) copyObject(PC);
	newPC->op.inputs = singleton(operator);
	operator->parents = singleton(newPC);

	// get psInfo;
	coarsePara2 = (Node*) getStringProperty((QueryOperator*) newPC, PROP_PC_COARSE_GRAINED);
	psPara2 = createPSInfo(coarsePara2);

	// mark table;
	markTableAccessAndAggregation((QueryOperator*) newPC, (Node*) psPara2);

	// mark the number of table - used in provenance scratch;
	markNumOfTableAccess((QueryOperator*) newPC);

	// bottom up propagate;
	bottomUpPropagateLevelAggregation((QueryOperator*) newPC, psPara2);

	// DEBUG_NODE_BEATIFY_LOG("BEFORE REQEIRE", newPC);
	// INFO_OP_LOG("BEFORE REQEIRE", newPC);

	// rewrite query;
	result = rewritePI_CS(newPC);
	// DEBUG_NODE_BEATIFY_LOG("AFTER REQEIRE", newPC);
	// INFO_OP_LOG("AFTER REQEIRE", newPC);
	return result;
}

static void
updateByOperators(QueryOperator * op)
{
	switch(op->type)
	{
		case T_ProvenanceComputation:
			INFO_LOG("update provenance computation");
			updateProvenanceComputation(op);
			break;
		case T_ProjectionOperator:
			INFO_LOG("update projection");
			updateProjection(op);
			break;
		case T_SelectionOperator:
			INFO_LOG("update selection");
			updateSelection(op);
			break;
		case T_JoinOperator:
			INFO_LOG("update join");
			updateJoin(op);
			break;
		case T_AggregationOperator:
			INFO_LOG("update aggregation");
			updateAggregation(op);
			break;
		case T_TableAccessOperator:
			INFO_LOG("update table access");
			updateTableAccess(op);
			break;
		case T_DuplicateRemoval:
			INFO_LOG("update duplicate removal");
			updateDuplicateRemoval(op);
			break;
		case T_SetOperator:
			INFO_LOG("update set");
			updateSet(op);
			break;
		case T_LimitOperator:
			INFO_LOG("update limit");
			updateLimit(op);
			break;
		case T_OrderOperator:
			updateOrder(op);
			break;
		default:
			FATAL_LOG("update for %s not implemented", NodeTagToString(op->type));
			break;
	}
}

static void
updateProvenanceComputation(QueryOperator* op)
{
	appendStringInfo(strInfo, "%s ", "UpdtateProvenanceComputation");
}

static void
updateProjection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT PROJECTION OPERATOR", op);
	INFO_OP_LOG("CURRENT OPERATOR", op);
	QueryOperator *child = OP_LCHILD(op);

	if (!HAS_STRING_PROP(child, DATA_CHUNK_PROP))
	{
		return;
	}

	DataChunk *dataChunk = NULL;
	dataChunk = (DataChunk *) getStringProperty(child, DATA_CHUNK_PROP);

	DataChunk *resultDC = initDataChunk();

	resultDC->numTuples = dataChunk->numTuples;
	resultDC->fragmentsInfo = (HashMap *) copyObject(dataChunk->fragmentsInfo);
	resultDC->updateIdentifier = (Vector *) copyObject(dataChunk->updateIdentifier);
	resultDC->tupleFields = LIST_LENGTH(op->schema->attrDefs);

	List *attrDefs = op->schema->attrDefs;
	List *projExprs = ((ProjectionOperator *) op)->projExprs;

	int pos = 0;
	FOREACH(Node, node, projExprs) {
		// get projection attribute def;
		AttributeDef *ad = (AttributeDef *) getNthOfListP(attrDefs, pos);
		resultDC->attrNames = appendToTailOfList(resultDC->attrNames, copyObject(ad));

		// add to map the name, datatype, pos;
		addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resultDC->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));

		if (isA(node, Operator)) { // node is an expression, evaluate it;
			// calculate value of this operator;
			ColumnChunk *evaluatedValue = (ColumnChunk *) evaluateExprOnDataChunk(node, dataChunk);

			Vector *vector = columnChunkToVector(evaluatedValue);

			vecAppendNode(resultDC->tuples, (Node *) copyObject(vector));

			// compute isNull;
			BitSet *bs = NULL;
			bs = computeIsNullBitSet(node, dataChunk);
			// vecAppendNode(resultDC->isNull, (Node *) copyObject(bs));

		} else if (isA(node, AttributeReference)){ // node is an attribute, direct copy;
			// get value of this attribute reference;
			AttributeReference *af = (AttributeReference *) node;
			int attPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, af->name));
			Vector *vector = (Vector *) getVecNode(dataChunk->tuples, attPos);
			vecAppendNode(resultDC->tuples, (Node *) copyObject(vector));

			// get isNull;
			// vecAppendNode(resultDC->isNull, (Node *) copyObject(getVecNode(dataChunk->isNull, pos)));
		}
		pos++;
	}

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR PROJECTION OPERATOR: ", resultDC);

	setStringProperty(op, DATA_CHUNK_PROP, (Node*) resultDC);

	// remove child data chunk;
	removeStringProperty(child, DATA_CHUNK_PROP);
}

static void
updateSelection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT SELECTION OPERATOR", op);

	// check if child operator has delta tuples;
	QueryOperator *child = OP_LCHILD(op);

	// TODO: another property to identify if it is only capture delta data;
	if (!HAS_STRING_PROP(child, DATA_CHUNK_PROP)) {
		return;
	}

	appendStringInfo(strInfo, "%s ", "UpdateSelection");
	DataChunk * dataChunk = (DataChunk *) getStringProperty(child, DATA_CHUNK_PROP);

	Node * selCond = ((SelectionOperator *) op)->cond;

	DataChunk* selDataChunk = filterDataChunk(dataChunk, selCond);

	if (selDataChunk == NULL) {
		removeStringProperty(child, DATA_CHUNK_PROP);
		return;
	}

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR SELECTION OPERATOR: ", selDataChunk);
	setStringProperty(op, DATA_CHUNK_PROP, (Node*) selDataChunk);

	// remove child's data chunk;
	removeStringProperty(child, DATA_CHUNK_PROP);
}

// TODO: IN ORDER NOT TO WRITE BACK DATA TO DB, JUST CREATE A CONSTRELOPERATOR WITH A LIST OF INPUT DATA
static void
updateJoin(QueryOperator* op)
{
	INFO_OP_LOG("CURRENT JOIN", op);
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT JOIN OPERATOR", op);

	// TODO: for future Vasudha's suggestion, we only capture delta tuples, can terminate here;

	QueryOperator *lChild = OP_LCHILD(op);
	QueryOperator *rChild = OP_RCHILD(op);

	if ((!HAS_STRING_PROP(lChild, DATA_CHUNK_PROP))
	 && (!HAS_STRING_PROP(rChild, DATA_CHUNK_PROP))) {
		return;
	}

	HashMap *psMap = NEW_MAP(Constant, Constant);
	FOREACH_HASH_KEY(Node, key, PS_INFO->tablePSAttrInfos) {
		List *psAttrInfoList = (List *) getMapString(PS_INFO->tablePSAttrInfos, STRING_VALUE((Constant *) key));
		psAttrInfo *attrInfo = (psAttrInfo *) getHeadOfListP(psAttrInfoList);
		char * newPSName = CONCAT_STRINGS(backendifyIdentifier("prov"), "_", backendifyIdentifier(STRING_VALUE((Constant *) key)), "_", backendifyIdentifier(attrInfo->attrName));
		addToMap(psMap, (Node *) createConstString(newPSName), (Node *) createConstInt(LIST_LENGTH(attrInfo->rangeList) - 1));
	}

	DataChunk *resultDC = initDataChunk();
	int branchWithDeltaCnt = 0;
	// left delta join right;
	if (HAS_STRING_PROP(lChild, DATA_CHUNK_PROP)) {
		// increase delta branch of join;
		branchWithDeltaCnt++;

		DataChunk *dataChunk = (DataChunk *) getStringProperty(lChild, DATA_CHUNK_PROP);
		DEBUG_NODE_BEATIFY_LOG("DATA_CHUNK LEFT", dataChunk);

		// create a temp table for left delta;

		// create attribute defs;
		// 1. copy tuple attrdefs from datachunk;
		List *attrDefs = (List *) copyObject(dataChunk->attrNames);

		// 2. create ps attr defs;
		FOREACH_HASH_KEY(Node, key, dataChunk->fragmentsInfo) {
			AttributeDef *ad = createAttributeDef(STRING_VALUE((Constant *) key), DT_STRING);
			attrDefs = appendToTailOfList(attrDefs, ad);
		}

		// 3. create update type attr defs;
		AttributeDef *ad_update = createAttributeDef(LEFT_UPDATE_IDENTIFIER, DT_INT);
		attrDefs = appendToTailOfList(attrDefs, ad_update);


		CreateTable* leftBranchTable = createCreateTable(LEFT_BRANCH_OF_JOIN, attrDefs);
		QueryOperator *createTableOp = translateCreateTable(leftBranchTable);
		char *leftTableSQL = serializeQuery(createTableOp);
		INFO_LOG("CREATED TABLE\n %s", leftTableSQL);

		// write back datachunk and join;
		INFO_LOG("create table\n", leftTableSQL);
		// executeQueryIgnoreResult(leftTableSQL); // TODO: this line has ERROR;
		// executeQueryWithoutResult(leftTableSQL);

		// insert values to LEFT_BRANCH_OF_JOIN;
		// create bulk insert for all data in datachunk;
		List * insertQuery = NIL;

		for (int row = 0; row < dataChunk->numTuples; row++) {
			// StringInfo insSQL = makeStringInfo();
			List *tup = NIL;

			// append tuple value;
			for (int col = 0; col < dataChunk->tupleFields; col++) {
				tup = appendToTailOfList(tup, (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, col), row));
			}

			// append ps bits;
			for (int col = dataChunk->tupleFields; col < LIST_LENGTH(attrDefs) - 1; col++) {
				AttributeDef *ad = (AttributeDef *) getNthOfListP(attrDefs, col);
				List *bitsetList = (List *) getMapString(dataChunk->fragmentsInfo, ad->attrName);
				BitSet *bitset = (BitSet *) getNthOfListP(bitsetList, row);
				tup = appendToTailOfList(tup, createConstString(bitSetToString(bitset)));
			}

			// append update type;
			tup = appendToTailOfList(tup, createConstInt(getVecInt(dataChunk->updateIdentifier, row)));

			insertQuery = appendToTailOfList(insertQuery, tup);
		}

		DEBUG_NODE_BEATIFY_LOG("ATTRDEFS", attrDefs);
		Insert *insert = createInsert(LEFT_BRANCH_OF_JOIN, (Node *) insertQuery, getAttrDefNames(attrDefs));
		DLMorDDLOperator* dlm = createDMLDDLOp((Node *) insert);
		char *insSQL = serializeQuery((QueryOperator *) dlm);
		INFO_LOG("INS SQL \n %s", insSQL);
		executeQueryWithoutResult(insSQL);

		// build proj-tableaccess for left branch;
		// create left delta
		TableAccessOperator *leftTaOp = createTableAccessOp(strdup(LEFT_BRANCH_OF_JOIN), NULL, strdup(LEFT_BRANCH_OF_JOIN), NIL, getAttrDefNames(attrDefs), getAttrDataTypes(attrDefs));
		INFO_OP_LOG("LEFT TA", leftTaOp);

		// create left delta projec
		List *leftProjList = NIL;
		List *leftProjExprs = NIL;

		int attrPos = 0;
		FOREACH(AttributeDef, ad, ((QueryOperator *) leftTaOp)->schema->attrDefs) {
			leftProjList = appendToTailOfList(leftProjList, createAttributeDef(ad->attrName, ad->dataType));
			leftProjExprs = appendToTailOfList(leftProjExprs, createFullAttrReference(ad->attrName, 0, attrPos++, 0, ad->dataType));
		}

		ProjectionOperator *leftProjOp = createProjectionOp(leftProjExprs, (QueryOperator *) leftTaOp, NIL, getAttrDefNames(leftProjList));
		leftTaOp->op.parents = singleton(leftProjOp);
		DEBUG_NODE_BEATIFY_LOG("LEFT PROJECTION", leftProjOp);

		// rewrite join, return proj;
		QueryOperator *rewrOP = captureRewrite((QueryOperator *) copyObject(op));
		DEBUG_NODE_BEATIFY_LOG("REWRITE JOIN", rewrOP);
		INFO_OP_LOG("Rewr JOIN", rewrOP);

		// get psattr and level;
		HashMap *psAttrAndLevelMap = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOP), PROP_LEVEL_AGGREGATION_MARK), 0);
		DEBUG_NODE_BEATIFY_LOG("MAP", psAttrAndLevelMap);

		// get join;
		QueryOperator *childOp = OP_LCHILD(rewrOP);
		INFO_OP_LOG("BEFORE REPLACE", childOp);

		// replace left branch with delta table access's proj;
		// replaceNode(childOp->inputs, OP_LCHILD(childOp), leftProjOp);
		// leftProjOp->op.parents = singleton(rewrOP);
		// DEBUG_NODE_BEATIFY_LOG("AFTER REPLACE", childOp);
		// INFO_OP_LOG("AFTER REPLACE", childOp);


		// replace lchild with new proj;
		replaceNode(childOp->inputs, OP_LCHILD(childOp), leftProjOp);
		leftProjOp->op.parents = singleton(childOp);
		// DEBUG_NODE_BEATIFY_LOG("NEW JOIN", childOp);

		// modify prov attr datatype for left branch from DT_INT to DT_STRING;
		FOREACH(AttributeDef, ad, childOp->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, ad->attrName)) {
				ad->dataType = DT_STRING;
			}
		}

		// add update type indentifier to join Schema;
		childOp->schema->attrDefs = appendToTailOfList(childOp->schema->attrDefs, createAttributeDef(LEFT_UPDATE_IDENTIFIER, DT_INT));

		// modify proj prov attr datatype
		FOREACH(AttributeDef, ad, rewrOP->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, ad->attrName)) {
				ad->dataType = DT_STRING;
			}
		}

		// add update type identifier to proj schama;
		rewrOP->schema->attrDefs = appendToTailOfList(rewrOP->schema->attrDefs, createAttributeDef(LEFT_UPDATE_IDENTIFIER, DT_INT));

		// modify proj expr list prov attr data type for left child;
		FOREACH(AttributeReference, af, ((ProjectionOperator *) rewrOP)->projExprs) {
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, af->name)) {
				af->attrType = DT_STRING;
			}
		}

		// add update type attr to proj exor;
		((ProjectionOperator *) rewrOP)->projExprs = appendToTailOfList(((ProjectionOperator *) rewrOP)->projExprs, createFullAttrReference(LEFT_UPDATE_IDENTIFIER, 0, LIST_LENGTH(childOp->schema->attrDefs) - 1, 0, DT_INT));


		DEBUG_NODE_BEATIFY_LOG("new PROJ", rewrOP);
		INFO_OP_LOG("NEW PROJ", rewrOP);
		char *sql = serializeQuery(rewrOP);
		INFO_LOG("SQL %s", sql);

		Relation * resultRel = executeQuery(sql);
		resultDC->numTuples = LIST_LENGTH(resultRel->tuples);
		resultDC->tupleFields = LIST_LENGTH(resultRel->schema) - mapSize(psAttrAndLevelMap);

		// build datachunk map for pos and dt;
		attrPos = 0;
		FOREACH(AttributeDef, ad, rewrOP->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(psAttrAndLevelMap, ad->attrName)) {
				addToMap(resultDC->fragmentsInfo, (Node *) createConstString(ad->attrName), (Node *) newList(T_List));
			} else {
				addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrPos));
				addToMap(resultDC->posToDatatype, (Node *) createConstInt(attrPos), (Node *) createConstInt(ad->dataType));
				attrPos++;
				vecAppendNode(resultDC->tuples, (Node*) makeVector(VECTOR_NODE, T_Vector));
			}
		}

		for (int row = 0; row < LIST_LENGTH(resultRel->tuples); row++) {
			List *tuple = (List *) getNthOfListP(resultRel->tuples, row);

			for (int col = 0; col < LIST_LENGTH(tuple); col++) {
				char *name = (char *) getNthOfListP(resultRel->schema, col);
				char *val = (char *) getNthOfListP(tuple, col);
				if (MAP_HAS_STRING_KEY(psAttrAndLevelMap, name)) {
					List *bitsetList = (List *) getMapString(resultDC->fragmentsInfo, name);
					BitSet *bitset = NULL;
					if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, name)) { // from left delta, it is already a string;
						bitset = stringToBitset(val);
					} else { // from right table, if level < 2, it is a integer, should transform to bitset;
						int level = INT_VALUE((Constant*) getNthOfListP((List *) getMapString(psAttrAndLevelMap, name), 0));
						if (level < 2) { // it is a integer;
							int sketchLen = 0;
							if (MAP_HAS_STRING_KEY(psMap, substr(name, 0, strlen(name) - 1))) {
								sketchLen = INT_VALUE((Constant *) getMapString(psMap, substr(name, 0, strlen(name) - 1)));
							} else if (MAP_HAS_STRING_KEY(psMap, substr(name, 0, strlen(name) - 2))) {
								sketchLen = INT_VALUE((Constant *) getMapString(psMap, substr(name, 0, strlen(name) - 2)));
							}
							bitset = newBitSet(sketchLen);
							setBit(bitset, atoi(val), TRUE);
						} else {
							bitset = stringToBitset(val);
						}
					}
					bitsetList = appendToTailOfList(bitsetList, bitset);
				} else if (strcmp(name, LEFT_UPDATE_IDENTIFIER) != 0){
					int pos = INT_VALUE((Constant *) getMapString(resultDC->attriToPos, name));
					DataType dataType = INT_VALUE(getMapInt(resultDC->posToDatatype, pos));
					Constant *value = makeValue(dataType, (char *) getNthOfListP(tuple, col));
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, pos), (Node *) value);
				} else {
					vecAppendInt(resultDC->updateIdentifier, atoi(val));
				}
			}
		}
	}

	// right delta join left;
	if (HAS_STRING_PROP(rChild, DATA_CHUNK_PROP)) {
		// increase delta branch of join;
		branchWithDeltaCnt++;

		DataChunk *dataChunk = (DataChunk *) getStringProperty(lChild, DATA_CHUNK_PROP);
		DEBUG_NODE_BEATIFY_LOG("DATA CHUNK RIGHT", dataChunk);

		// create a temp table for left delta;

		// create attribute defs;
		// 1. copy tuple attrdefs from datachunk;
		List *attrDefs = (List *) copyObject(dataChunk->attrNames);

		// 2. create ps attr defs;
		FOREACH_HASH_KEY(Node, key, dataChunk->fragmentsInfo) {
			AttributeDef *ad = createAttributeDef(STRING_VALUE((Constant *) key), DT_STRING);
			attrDefs = appendToTailOfList(attrDefs, ad);
		}

		// 3. create update type attr defs;
		AttributeDef *ad_update = createAttributeDef(RIGHT_UPDATE_IDENTIFIER, DT_INT);
		attrDefs = appendToTailOfList(attrDefs, ad_update);


		CreateTable* rightBranchTable = createCreateTable(RIGHT_BRANCH_OF_JOIN, attrDefs);
		QueryOperator *createTableOp = translateCreateTable(rightBranchTable);
		char *rightTableSQL = serializeQuery(createTableOp);
		INFO_LOG("CREATED TABLE\n %s", rightTableSQL);

		// write back datachunk and join;
		INFO_LOG("create table\n", rightTableSQL);
		// executeQueryIgnoreResult(leftTableSQL); // TODO: this line has ERROR;
		// executeQueryWithoutResult(leftTableSQL);

		// insert values to RIGHT_BRANCH_OF_JOIN;
		// create bulk insert for all data in datachunk;
		List * insertQuery = NIL;

		for (int row = 0; row < dataChunk->numTuples; row++) {
			// StringInfo insSQL = makeStringInfo();
			List *tup = NIL;

			// append tuple value;
			for (int col = 0; col < dataChunk->tupleFields; col++) {
				tup = appendToTailOfList(tup, (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, col), row));
			}

			// append ps bits;
			for (int col = dataChunk->tupleFields; col < LIST_LENGTH(attrDefs) - 1; col++) {
				AttributeDef *ad = (AttributeDef *) getNthOfListP(attrDefs, col);
				List *bitsetList = (List *) getMapString(dataChunk->fragmentsInfo, ad->attrName);
				BitSet *bitset = (BitSet *) getNthOfListP(bitsetList, row);
				tup = appendToTailOfList(tup, createConstString(bitSetToString(bitset)));
			}

			// append update type;
			tup = appendToTailOfList(tup, createConstInt(getVecInt(dataChunk->updateIdentifier, row)));

			insertQuery = appendToTailOfList(insertQuery, tup);
		}

		// DEBUG_NODE_BEATIFY_LOG("ATTRDEFS", attrDefs);
		Insert *insert = createInsert(RIGHT_BRANCH_OF_JOIN, (Node *) insertQuery, getAttrDefNames(attrDefs));
		DLMorDDLOperator* dlm = createDMLDDLOp((Node *) insert);
		char *insSQL = serializeQuery((QueryOperator *) dlm);
		INFO_LOG("INS SQL \n %s", insSQL);
		executeQueryWithoutResult(insSQL);

		// build proj-tableaccess for left branch;
		// create right delta
		TableAccessOperator *rightTaOp = createTableAccessOp(strdup(RIGHT_BRANCH_OF_JOIN), NULL, strdup(RIGHT_BRANCH_OF_JOIN), NIL, getAttrDefNames(attrDefs), getAttrDataTypes(attrDefs));
		INFO_OP_LOG("LEFT TA", rightTaOp);

		// create right delta projec
		List *rightProjList = NIL;
		List *rightProjExprs = NIL;

		int attrPos = 0;
		FOREACH(AttributeDef, ad, ((QueryOperator *) rightTaOp)->schema->attrDefs) {
			rightProjList = appendToTailOfList(rightProjList, createAttributeDef(ad->attrName, ad->dataType));
			rightProjExprs = appendToTailOfList(rightProjExprs, createFullAttrReference(ad->attrName, 0, attrPos++, 0, ad->dataType));
		}

		ProjectionOperator *rightProjOp = createProjectionOp(rightProjExprs, (QueryOperator *) rightTaOp, NIL, getAttrDefNames(rightProjList));
		rightTaOp->op.parents = singleton(rightProjOp);
		DEBUG_NODE_BEATIFY_LOG("RIGHT PROJECTION", rightProjOp);

		// rewrite join, return proj;
		QueryOperator *rewrOP = captureRewrite((QueryOperator *) copyObject(op));
		DEBUG_NODE_BEATIFY_LOG("REWRITE JOIN", rewrOP);
		INFO_OP_LOG("Rewr JOIN", rewrOP);

		// get psattr and level;
		HashMap *psAttrAndLevelMap = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOP), PROP_LEVEL_AGGREGATION_MARK), 0);
		DEBUG_NODE_BEATIFY_LOG("MAP", psAttrAndLevelMap);

		// get join;
		QueryOperator *childOp = OP_LCHILD(rewrOP);
		INFO_OP_LOG("BEFORE REPLACE", childOp);

		// replace left branch with delta table access's proj;
		// replaceNode(childOp->inputs, OP_LCHILD(childOp), rightProjOp);
		// rightProjOp->op.parents = singleton(rewrOP);
		// DEBUG_NODE_BEATIFY_LOG("AFTER REPLACE", childOp);
		// INFO_OP_LOG("AFTER REPLACE", childOp);


		// replace lchild with new proj;
		replaceNode(childOp->inputs, OP_RCHILD(childOp), rightProjOp);
		rightProjOp->op.parents = singleton(childOp);
		DEBUG_NODE_BEATIFY_LOG("NEW JOIN", childOp);
		//

		// modify prov attr datatype for left branch from DT_INT to DT_STRING;
		FOREACH(AttributeDef, ad, childOp->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, ad->attrName)) {
				ad->dataType = DT_STRING;
			}
		}

		// add update type indentifier to join Schema;
		childOp->schema->attrDefs = appendToTailOfList(childOp->schema->attrDefs, createAttributeDef(RIGHT_UPDATE_IDENTIFIER, DT_INT));

		// modify proj prov attr datatype
		FOREACH(AttributeDef, ad, rewrOP->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, ad->attrName)) {
				ad->dataType = DT_STRING;
			}
		}

		// add update type identifier to proj schama;
		rewrOP->schema->attrDefs = appendToTailOfList(rewrOP->schema->attrDefs, createAttributeDef(RIGHT_UPDATE_IDENTIFIER, DT_INT));

		// modify proj expr list prov attr data type for left child;
		FOREACH(AttributeReference, af, ((ProjectionOperator *) rewrOP)->projExprs) {
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, af->name)) {
				af->attrType = DT_STRING;
			}
		}

		// add update type attr to proj exor;
		((ProjectionOperator *) rewrOP)->projExprs = appendToTailOfList(((ProjectionOperator *) rewrOP)->projExprs, createFullAttrReference(RIGHT_UPDATE_IDENTIFIER, 0, LIST_LENGTH(childOp->schema->attrDefs) - 1, 0, DT_INT));

		DEBUG_NODE_BEATIFY_LOG("new PROJ", rewrOP);
		INFO_OP_LOG("NEW PROJ", rewrOP);
		char *sql = serializeQuery(rewrOP);
		INFO_LOG("SQL %s", sql);

		Relation * resultRel = executeQuery(sql);


		// build datachunk map for pos and dt;

		if (branchWithDeltaCnt == 1) { // if > 1, it already build in left branch
			resultDC->numTuples = LIST_LENGTH(resultRel->tuples);
			resultDC->tupleFields = LIST_LENGTH(resultRel->schema) - mapSize(psAttrAndLevelMap);

			attrPos = 0;
			FOREACH(AttributeDef, ad, rewrOP->schema->attrDefs) {
				if (MAP_HAS_STRING_KEY(psAttrAndLevelMap, ad->attrName)) {
					addToMap(resultDC->fragmentsInfo, (Node *) createConstString(ad->attrName), (Node *) newList(T_List));
				} else {
					addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrPos));
					addToMap(resultDC->posToDatatype, (Node *) createConstInt(attrPos), (Node *) createConstInt(ad->dataType));
					attrPos++;
					vecAppendNode(resultDC->tuples, (Node*) makeVector(VECTOR_NODE, T_Vector));
				}
			}
		} else {
			resultDC->numTuples += LIST_LENGTH(resultRel->tuples);
		}

		// fill data
		for (int row = 0; row < LIST_LENGTH(resultRel->tuples); row++) {
			List *tuple = (List *) getNthOfListP(resultRel->tuples, row);

			for (int col = 0; col < LIST_LENGTH(tuple); col++) {
				char *name = (char *) getNthOfListP(resultRel->schema, col);
				char *val = (char *) getNthOfListP(tuple, col);
				if (MAP_HAS_STRING_KEY(psAttrAndLevelMap, name)) { // fill prov bit set
					List *bitsetList = (List *) getMapString(resultDC->fragmentsInfo, name);
					BitSet *bitset = NULL;
					if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, name)) { // from left delta, it is already a string;
						bitset = stringToBitset(val);
					} else { // from right table, if level < 2, it is a integer, should transform to bitset;
						int level = INT_VALUE((Constant*) getNthOfListP((List *) getMapString(psAttrAndLevelMap, name), 0));
						if (level < 2) { // it is a integer;
							int sketchLen = 0;
							if (MAP_HAS_STRING_KEY(psMap, substr(name, 0, strlen(name) - 1))) {
								sketchLen = INT_VALUE((Constant *) getMapString(psMap, substr(name, 0, strlen(name) - 1)));
							} else if (MAP_HAS_STRING_KEY(psMap, substr(name, 0, strlen(name) - 2))) {
								sketchLen = INT_VALUE((Constant *) getMapString(psMap, substr(name, 0, strlen(name) - 2)));
							}
							bitset = newBitSet(sketchLen);
							setBit(bitset, atoi(val), TRUE);
						} else {
							bitset = stringToBitset(val);
						}
					}
					bitsetList = appendToTailOfList(bitsetList, bitset);
				} else if (strcmp(name, LEFT_UPDATE_IDENTIFIER) != 0){ // fill data tuples;
					int pos = INT_VALUE((Constant *) getMapString(resultDC->attriToPos, name));
					DataType dataType = INT_VALUE(getMapInt(resultDC->posToDatatype, pos));
					Constant *value = makeValue(dataType, (char *) getNthOfListP(tuple, col));
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, pos), (Node *) value);
				} else { // fill update type;
					vecAppendInt(resultDC->updateIdentifier, atoi(val));
				}
			}
		}
	}

	if (branchWithDeltaCnt == 2) {
		// create left branch;
		DataChunk *lDC = (DataChunk *) getStringProperty(lChild, DATA_CHUNK_PROP);
		List *leftAttrDefs = (List *) copyObject(lDC->attrNames);
		TableAccessOperator *leftTaOp = createTableAccessOp(strdup(LEFT_BRANCH_OF_JOIN), NULL, strdup(LEFT_BRANCH_OF_JOIN), NIL, getAttrDefNames(leftAttrDefs), getAttrDataTypes(leftAttrDefs));
		INFO_OP_LOG("LEFT TA", leftTaOp);

		// create left delta projec
		List *leftProjList = NIL;
		List *leftProjExprs = NIL;

		int attrPos = 0;
		FOREACH(AttributeDef, ad, ((QueryOperator *) leftTaOp)->schema->attrDefs) {
			leftProjList = appendToTailOfList(leftProjList, createAttributeDef(ad->attrName, ad->dataType));
			leftProjExprs = appendToTailOfList(leftProjExprs, createFullAttrReference(ad->attrName, 0, attrPos++, 0, ad->dataType));
		}

		ProjectionOperator *leftProjOp = createProjectionOp(leftProjExprs, (QueryOperator *) leftTaOp, NIL, getAttrDefNames(leftProjList));
		leftTaOp->op.parents = singleton(leftProjOp);


		// create right branch;
		DataChunk *rDC = (DataChunk *) getStringProperty(rChild, DATA_CHUNK_PROP);
		List *rightAttrDefs = (List *) copyObject(rDC->attrNames);
		TableAccessOperator *rightTaOp = createTableAccessOp(strdup(RIGHT_BRANCH_OF_JOIN), NULL, strdup(RIGHT_BRANCH_OF_JOIN), NIL, getAttrDefNames(rightAttrDefs), getAttrDataTypes(rightAttrDefs));

		// create right delta projec
		List *rightProjList = NIL;
		List *rightProjExprs = NIL;

		attrPos = 0;
		FOREACH(AttributeDef, ad, ((QueryOperator *) rightTaOp)->schema->attrDefs) {
			rightProjList = appendToTailOfList(rightProjList, createAttributeDef(ad->attrName, ad->dataType));
			rightProjExprs = appendToTailOfList(rightProjExprs, createFullAttrReference(ad->attrName, 0, attrPos++, 0, ad->dataType));
		}

		ProjectionOperator *rightProjOp = createProjectionOp(rightProjExprs, (QueryOperator *) rightTaOp, NIL, getAttrDefNames(rightProjList));
		rightTaOp->op.parents = singleton(rightProjOp);
		DEBUG_NODE_BEATIFY_LOG("RIGHT PROJECTION", rightProjOp);

		QueryOperator *rewrOP = captureRewrite((QueryOperator *) copyObject(op));

		// get join;
		QueryOperator *childOp = OP_LCHILD(rewrOP);
		INFO_OP_LOG("BEFORE REPLACE", childOp);

		// replace LCHILD with leftProjOp, RCHILD with right ProjOp;
		replaceNode(childOp->inputs, OP_LCHILD(childOp), leftProjOp);
		replaceNode(childOp->inputs, OP_RCHILD(childOp), rightProjOp);

		// set parents to left/right proj Op;
		leftProjOp->op.parents = singleton(childOp);
		rightProjOp->op.parents = singleton(childOp);


		// modify prov attr datatype for left branch from DT_INT to DT_STRING;
		FOREACH(AttributeDef, ad, childOp->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, ad->attrName)) {
				ad->dataType = DT_STRING;
			}
		}

		// add update type indentifier to join Schema;
		childOp->schema->attrDefs = appendToTailOfList(childOp->schema->attrDefs, createAttributeDef(LEFT_UPDATE_IDENTIFIER, DT_INT));
		childOp->schema->attrDefs = appendToTailOfList(childOp->schema->attrDefs, createAttributeDef(RIGHT_UPDATE_IDENTIFIER, DT_INT));

		// modify proj prov attr datatype
		FOREACH(AttributeDef, ad, rewrOP->schema->attrDefs) {
			if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, ad->attrName)) {
				ad->dataType = DT_STRING;
			}
		}

		// add update type identifier to proj schama;
		rewrOP->schema->attrDefs = appendToTailOfList(rewrOP->schema->attrDefs, createAttributeDef(LEFT_UPDATE_IDENTIFIER, DT_INT));
		rewrOP->schema->attrDefs = appendToTailOfList(rewrOP->schema->attrDefs, createAttributeDef(RIGHT_UPDATE_IDENTIFIER, DT_INT));

		// modify proj expr list prov attr data type for left child;
		FOREACH(AttributeReference, af, ((ProjectionOperator *) rewrOP)->projExprs) {
			if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, af->name)) {
				af->attrType = DT_STRING;
			}
		}

		// add update type attr to proj exor;
		((ProjectionOperator *) rewrOP)->projExprs = appendToTailOfList(((ProjectionOperator *) rewrOP)->projExprs, createFullAttrReference(LEFT_UPDATE_IDENTIFIER, 0, LIST_LENGTH(childOp->schema->attrDefs) - 2, 0, DT_INT));
		((ProjectionOperator *) rewrOP)->projExprs = appendToTailOfList(((ProjectionOperator *) rewrOP)->projExprs, createFullAttrReference(RIGHT_UPDATE_IDENTIFIER, 0, LIST_LENGTH(childOp->schema->attrDefs) - 1, 0, DT_INT));

		char *sql = serializeQuery(rewrOP);
		INFO_LOG("SQL %s", sql);

		Relation * resultRel = executeQuery(sql);

		resultDC->numTuples += LIST_LENGTH(resultRel->tuples);

		// first get to update identifiers;
		int leftUpdIdent = 0;
		int rightUpdIdent = 0;
		for (int col = 0; col < LIST_LENGTH(resultRel->schema); col++) {
			if (strcmp(LEFT_UPDATE_IDENTIFIER, (char *) getNthOfListP(resultRel->schema, col)) == 0) {
				leftUpdIdent = col;
			}
			if (strcmp(RIGHT_UPDATE_IDENTIFIER, (char *) getNthOfListP(resultRel->schema, col)) == 0) {
				rightUpdIdent = col;
			}
		}


		for (int row = 0; row < LIST_LENGTH(resultRel->tuples); row++) {
			List *tuple = getNthOfListP(resultRel->tuples, row);
			int updIdent1 = atoi((char *) getNthOfListP(tuple, leftUpdIdent));
			int updIdent2 = atoi((char *) getNthOfListP(tuple, rightUpdIdent));

			if (updIdent1 != updIdent2) {
				continue;
			}

			// append update type;
			vecAppendInt(resultDC->updateIdentifier, updIdent1);

			for (int col = 0; col < LIST_LENGTH(tuple); col++) {
				char *name = (char *) getNthOfListP(resultRel->schema, col);
				char *val = (char *) getNthOfListP(tuple, col);
				if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, name)) {
					List *bitsetList = (List *) getMapString(resultDC->fragmentsInfo, name);
					bitsetList = appendToTailOfList(bitsetList, stringToBitset(val));
				} else if (strcmp(name, LEFT_UPDATE_IDENTIFIER) != 0
						|| strcmp(name, RIGHT_UPDATE_IDENTIFIER) != 0) {
					int pos = INT_VALUE((Constant *) getMapString(resultDC->attriToPos, name));
					DataType dataType = INT_VALUE(getMapInt(resultDC->posToDatatype, pos));
					Constant *value = makeValue(dataType, (char *) getNthOfListP(tuple, col));
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, pos), (Node *) value);
				}
			}
		}
	}

	setStringProperty(op, DATA_CHUNK_PROP, (Node *) resultDC);
	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR JOIN OPERATOR");

	// remove children's data chunk;
	removeStringProperty(lChild, DATA_CHUNK_PROP);
	removeStringProperty(rChild, DATA_CHUNK_PROP);
}


static void
updateAggregation(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT AGGREGATION OPERATOR", op);

	// check if child has data chunk;
	QueryOperator *child = OP_LCHILD(op);
	if (!HAS_STRING_PROP(child, DATA_CHUNK_PROP))
	{
		return;
	}

	// get child's data chunk;
	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(child, DATA_CHUNK_PROP);

	// init return data chunk;
	List *attrDefs = (List *) copyObject(op->schema->attrDefs);
	DataChunk *resultDC = initDataChunk();

	// set some fields for result data chunk;
	resultDC->attrNames = copyObject(attrDefs);
	int pos = 0;
	FOREACH(AttributeDef, ad, resultDC->attrNames) {
		addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resultDC->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
		pos++;

		vecAppendNode(resultDC->tuples, (Node *) makeVector(VECTOR_NODE, T_Vector));
	}
	resultDC->tupleFields = LIST_LENGTH(resultDC->attrNames);



	// get lists of group by, agg functions;
	List *aggrFCs = (List *) copyObject(((AggregationOperator *) op)->aggrs);
	List *aggrGBs = (List *) copyObject(((AggregationOperator *) op)->groupBy);

	// get stored data structures;
	HashMap *dataStructures = (HashMap *) GET_STRING_PROP(op, DATA_STRUCTURE_STATE);

	int length = dataChunk->numTuples;
	for (int i = 0; i < length; i++) {
		// get gb values;
		StringInfo gbVals = makeStringInfo();
		for (int j = 0; j < LIST_LENGTH(aggrGBs); j++) {
			// get value of this gb attribute;
			AttributeReference *ar = (AttributeReference *) getNthOfListP(aggrGBs, j);

			// find position and datatype of this attribute value;
			int attrPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			DataType dt = INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, attrPos));

			// get the constaint value;
			Constant *value = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, attrPos), i);

			switch(dt) {
				case DT_INT:
					appendStringInfo(gbVals, "%s#", gprom_itoa(INT_VALUE(value)));
					break;
				case DT_LONG:
					appendStringInfo(gbVals, "%s#", gprom_ltoa(INT_VALUE(value)));
					break;
				case DT_FLOAT:
					appendStringInfo(gbVals, "%s#", gprom_ftoa(INT_VALUE(value)));
					break;
				case DT_BOOL:
					// this is for postgresql
					appendStringInfo(gbVals, "%s#", (BOOL_VALUE(value) == 1 ? 't' : 'f'));
					break;
				case DT_STRING:
				case DT_VARCHAR2:
					appendStringInfo(gbVals, "%s#", STRING_VALUE(value));
					break;
				default:
					FATAL_LOG("data type %d is not support", dt);
					break;
			}
		}

		// get update type;
		int updateType = getVecInt(dataChunk->updateIdentifier, i);

		// identifierType:
		// 0: - +
		// 1: -
		// 2: +
		int identifierType = -1;

		// iterate over all aggregation functions to fetch values: 1. old aggregation values, 2, new aggregation values;
		for (int j = 0; j < LIST_LENGTH(aggrFCs); j++) {
			FunctionCall *fc = (FunctionCall *) getNthOfListP(aggrFCs, j);
			AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);

			// get fc name in data structure;
			Constant *nameInDS = createConstString(CONCAT_STRINGS(fc->functionname, "_", ar->name));

			// get value pos in result data chunk;
			AttributeDef *ad = (AttributeDef *) getNthOfListP(attrDefs, j);
			int resPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, ad->attrName));
			DataType resDT = INT_VALUE((Constant *) MAP_GET_INT(resultDC->posToDatatype, resPos));

			// get result vector for this position;
			Vector *v = (Vector *) getVecNode(resultDC->tuples, resPos);

			// get value pos in input data chunk;
			int inpPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));

			// get insertd value;
			Constant *insertV = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, inpPos), i);

			///////////////////////////////////////////////////////////////////////////////////
			//																				 //
			// HERE SHOULD CONSIDER A EXTERME CASE:                                          //
			//           IN SOME STAGE: ALL THE VALUE OF DATA STRUCTURE ARE GONE(EMPTY)      //
			//           FOR FOLLOWING TUPLES, SHOULD REBUILD THE DATA STRUCTURE             //
			//                                                                               //
			///////////////////////////////////////////////////////////////////////////////////

			if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0
			 || strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {

				GBHeaps *heap = (GBHeaps *) MAP_GET_STRING(dataStructures, STRING_VALUE(nameInDS));
				if (updateType == 1) {
					if (MAP_HAS_STRING_KEY(heap->heapLists, gbVals->data)) { // has previous value of this group;
						// 1. old value;
						// get heap list;
						List *heapList = (List *) MAP_GET_STRING(heap->heapLists, gbVals->data);
						if (LIST_LENGTH(heapList) > 0) {
							identifierType = 0;
							// get old value;
							Constant *oldV = (Constant *) getNthOfListP(heapList, 0);

							// add to result data chunk;
							vecAppendNode(v, (Node *) copyObject(oldV));
						} else {
							identifierType = 2;
						}

						// 2. add insert value to heap, and re-heapify;
						heapList = heapInsert(heapList, STRING_VALUE(heap->heapType), (Node *) insertV);

						// 3. get new min/max and add to result vector;
						Constant *newV = (Constant *) getNthOfListP(heapList, 0);
						vecAppendNode(v, (Node *) copyObject(newV));

					} else { // no previous value, this is a new group;
						identifierType = 2;

						// new heapList;
						List *heapList = NIL;

						// add value;
						heapList = heapInsert(heapList, STRING_VALUE(heap->heapType), (Node *) insertV);

						Constant *newV = (Constant *) getNthOfListP(heapList, 0);
						vecAppendNode(v, (Node *) copyObject(newV));

						addToMap(heap->heapLists, (Node *) createConstString(gbVals->data), (Node *) copyObject(heapList));
					}
				} else {
					// in this case, in theory, the heap must contain this value;
					List *heapList = (List *) MAP_GET_STRING(heap->heapLists, gbVals->data);
					INFO_LOG("gb values %s", gbVals->data);
					DEBUG_NODE_BEATIFY_LOG("heapListsssss", heapList);
					Constant *oldV = (Constant *) getNthOfListP(heapList, 0);

					vecAppendNode(v, (Node *) copyObject(oldV));

					if (LIST_LENGTH(heapList) < 2) {
						removeMapStringElem(heap->heapLists, gbVals->data);
						identifierType = 1;
					} else {
						heapList = heapDelete(heapList, STRING_VALUE(heap->heapType), (Node *) oldV);

						Constant *newV = (Constant *) getNthOfListP(heapList, 0);
						vecAppendNode(v, (Node *) copyObject(newV));
						identifierType = 0;
					}
				}
			} else {
				GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructures, STRING_VALUE(nameInDS));
				DEBUG_NODE_BEATIFY_LOG("acs", acs);

				if (updateType == 1) {
					List *acsList = NIL;
					List *newL = NIL;
					if (MAP_HAS_STRING_KEY(acs->map, gbVals->data)) {
						acsList = (List *) MAP_GET_STRING(acs->map, gbVals->data);
						if (identifierType != -1) {
							identifierType = 0;
						}

						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							double avg = FLOAT_VALUE((Constant *) getNthOfListP(acsList, 0));
							double sum = FLOAT_VALUE((Constant *) getNthOfListP(acsList, 1));
							int count = INT_VALUE((Constant *) getNthOfListP(acsList, 2)) + 1;

							// append old value;
							vecAppendNode(v, (Node *) createConstFloat(avg));

							switch(insertV->constType) {
								case DT_INT:
									sum += INT_VALUE(insertV);
									break;
								case DT_LONG:
									sum += LONG_VALUE(insertV);
									break;
								case DT_FLOAT:
									sum += FLOAT_VALUE(insertV);
									break;
								default:
									FATAL_LOG("data type %d is not supported for avg", insertV->constType);
							}

							avg = sum / count;

							// new Value;
							vecAppendNode(v, (Node *) createConstFloat(avg));

							// update acslist;
							newL = appendToTailOfList(newL, createConstFloat(avg));
							newL = appendToTailOfList(newL, createConstFloat(sum));
							newL = appendToTailOfList(newL, createConstInt(count));
							addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							double oSum = FLOAT_VALUE((Constant *) getNthOfListP(acsList, 0));
							double nSum = oSum;
							int count = INT_VALUE((Constant *) getNthOfListP(acsList, 1)) + 1;

							switch(insertV->constType) {
								case DT_INT:
									nSum += INT_VALUE(insertV);
									break;
								case DT_LONG:
									nSum += LONG_VALUE(insertV);
									break;
								case DT_FLOAT:
									nSum += FLOAT_VALUE(insertV);
									break;
								default:
									FATAL_LOG("data type %d is not supported for avg", insertV->constType);
							}

							newL = appendToTailOfList(newL, createConstFloat(nSum));
							newL = appendToTailOfList(newL, createConstInt(count));
							addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));

							// compare sum type with result dc type
							// since we use double when building the datastructure;
							Constant *oldV = NULL;
							Constant *newV = NULL;
							if (resDT == DT_INT) {
								oldV = createConstInt((int) oSum);
								newV = createConstInt((int) nSum);
							} else if (resDT == DT_LONG) {
								oldV = createConstLong((gprom_long_t) oSum);
								newV = createConstLong((gprom_long_t) nSum);
							} else if (resDT == DT_FLOAT) {
								oldV = createConstFloat(oSum);
								newV = createConstFloat(nSum);
							}

							vecAppendNode(v, (Node *) oldV);
							vecAppendNode(v, (Node *) newV);
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME)) {
							int count = INT_VALUE((Constant *) getNthOfListP(acsList, 0));
							vecAppendNode(v, (Node *) createConstInt(count));
							vecAppendNode(v, (Node *) createConstInt(count + 1));
							addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) singleton(createConstInt(count + 1)));
						}
					} else { // new group
						if (identifierType != -1) {
							identifierType = 2;
						}

						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							double avg = (double) 0;
							double sum = (double) 0;

							switch(insertV->constType) {
								case DT_INT: {
									int insV = INT_VALUE(insertV);
									avg = (double) insV;
									sum = (double) insV;
								}
									break;
								case DT_LONG: {
									gprom_long_t insV = LONG_VALUE(insertV);
									avg = (double) insV;
									sum = (double) insV;
								}
									break;
								case DT_FLOAT: {
									sum = FLOAT_VALUE(insertV);
									avg = FLOAT_VALUE(insertV);
								}
									break;
								default:
									FATAL_LOG("datatype %d is not supported for avg()", insertV->constType);
							}

							newL = appendToTailOfList(newL, createConstFloat(avg));
							newL = appendToTailOfList(newL, createConstFloat(sum));
							newL = appendToTailOfList(newL, createConstInt(1));
							addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));

							Constant *newV = NULL;
							if (resDT == DT_INT) {
								newV = createConstInt((int) avg);
							} else if (resDT == DT_LONG) {
								newV = createConstLong((gprom_long_t) avg);
							} else if (resDT == DT_FLOAT) {
								newV = createConstFloat(avg);
							}
							vecAppendNode(v, (Node *) newV);
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							double sum = (double) 0;

							switch(insertV->constType) {
								case DT_INT: {
									int insV = INT_VALUE(insertV);
									sum = (double) insV;
								}
									break;
								case DT_LONG: {
									gprom_long_t insV = LONG_VALUE(insertV);
									sum = (double) insV;
								}
									break;
								case DT_FLOAT: {
									sum = FLOAT_VALUE(insertV);
								}
									break;
								default:
									FATAL_LOG("datatype %d is not supported for sum()", insertV->constType);
							}

							newL = appendToTailOfList(newL, createConstFloat(sum));
							newL = appendToTailOfList(newL, createConstInt(1));
							addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));

							Constant *newV = NULL;
							if (resDT == DT_INT) {
								newV = createConstInt((int) sum);
							} else if (resDT == DT_LONG) {
								newV = createConstLong((gprom_long_t) sum);
							} else if (resDT == DT_FLOAT) {
								newV = createConstFloat(sum);
							}
							vecAppendNode(v, (Node *) newV);
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							newL = appendToTailOfList(newL, createConstInt(1));
							addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) copyObject(newL));
							vecAppendNode(v, (Node *) createConstInt(1));
						}
					}
				} else {
				}
			}
		}
	}

	// remove data chunk from child operator;
	removeStringProperty(child, DATA_CHUNK_PROP);
	appendStringInfo(strInfo, "%s ", "UpdateAggregation");
}


static void
updateDuplicateRemoval(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	INFO_LOG("UPDATE DUREM");
	QueryOperator *lchild = OP_LCHILD(op);
	if (!HAS_STRING_PROP(lchild, DATA_CHUNK_PROP)) {
		return;
	}
	List* dupAttrs = (List *) copyObject(op->schema->attrDefs);
	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(lchild, DATA_CHUNK_PROP);


	DataChunk *dc = initDataChunk();
	int len = LIST_LENGTH(dupAttrs);
	dc->attrNames = (List *) copyObject(dupAttrs);
	dc->tupleFields = len;
	for (int i = 0; i < len; i++) {
		AttributeDef *ad = (AttributeDef *) getNthOfListP(dupAttrs, i);
		addToMap(dc->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(i));
		addToMap(dc->posToDatatype, (Node *) createConstInt(i), (Node *) createConstInt(ad->dataType));
	}

	GBACSs *acs = (GBACSs *) GET_STRING_PROP(op, DATA_STRUCTURE_STATE);

	len = dataChunk->numTuples;
	for (int row = 0; row < len; row++) {
		StringInfo info = makeStringInfo();
		List *values = NIL;
		FOREACH(AttributeDef, ad, dupAttrs) {
			int pos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ad->attrName));
			Constant *value = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, pos), row);
			values = appendToTailOfList(values, value);
			appendStringInfo(info, "%s#", constToString(value));
		}

		int updType = getVecInt(dataChunk->updateIdentifier, row);

		if (updType == 1) {
			if (MAP_HAS_STRING_KEY(acs->map, info->data)) {
				// get old chunk;
				for (int index = 0; index < LIST_LENGTH(values); index++) {
					vecAppendNode((Vector *) getVecNode(dc->tuples, index), (Node *) copyObject(getNthOfListP(values, index)));
				}
				vecAppendInt(dc->updateIdentifier, -1);
				(dc->numTuples)++;
				FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
					// dealing with ps; just copy to dst datachunk;
				}

				List *l = (List *) MAP_GET_STRING(acs->map, info->data);
				int count = INT_VALUE((Constant *) getNthOfListP(l, 0));
				addToMap(acs->map, (Node *) createConstString(info->data), (Node *) createConstInt(count + 1));
			} else {
				addToMap(acs->map, (Node *) createConstString(info->data), (Node *) createConstInt(1));
			}

			// get new chunk;
			for (int index = 0; index < LIST_LENGTH(values); index++) {
				vecAppendNode((Vector *) getVecNode(dc->tuples, index), (Node *) copyObject(getNthOfListP(values, index)));
			}
			vecAppendInt(dc->updateIdentifier, 1);
			(dc->numTuples)++;
			FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
				// dealing with ps; just copy to dst datachunk;
			}
		} else {
			// there must be such group;
			for (int index = 0; index < LIST_LENGTH(values); index++) {
				vecAppendNode((Vector *) getVecNode(dc->tuples, index), (Node *) copyObject(getNthOfListP(values, index)));
			}
			vecAppendInt(dc->updateIdentifier, -1);
			(dc->numTuples)++;
			FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
				// dealing with ps; just copy to dst datachunk;
			}

			List *l = (List *) MAP_GET_STRING(acs->map, info->data);
			int count = INT_VALUE((Constant *) getNthOfListP(l, 0));
			if (count < 2) {
				removeMapStringElem(acs->map, info->data);
			} else {
				for (int index = 0; index < LIST_LENGTH(values); index++) {
					vecAppendNode((Vector *) getVecNode(dc->tuples, index), (Node *) copyObject(getNthOfListP(values, index)));
				}
				vecAppendInt(dc->updateIdentifier, 1);
				(dc->numTuples)++;
				FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
					// dealing with ps; just copy to dst datachunk;
				}
				addToMap(acs->map, (Node *) createConstString(info->data), (Node *) createConstInt(count - 1));
			}
		}


	}

	// set the refreshed data structure;
	SET_STRING_PROP(op, DATA_STRUCTURE_STATE, (Node *) acs);

	// set datachunk to operator;
	SET_STRING_PROP(op, DATA_CHUNK_PROP, (Node *) dc);

	// remove child's datachunk;
	removeStringProperty(lchild, DATA_CHUNK_PROP);
	appendStringInfo(strInfo, "%s ", "UpdateDuplicatiRemoval");
}


static void
updateSet(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));
	appendStringInfo(strInfo, "%s ", "UpdateSet");
}


static void
updateOrder(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));

	QueryOperator *lchild = OP_LCHILD(op);

	if (!HAS_STRING_PROP(lchild, DATA_CHUNK_PROP)) {
		return;
	}

	// for order operator, we do nothing, just copy the data chunk and pass to next level;
	DataChunk *dc = (DataChunk *) copyObject((DataChunk *) GET_STRING_PROP(lchild, DATA_CHUNK_PROP));
	SET_STRING_PROP(op, DATA_CHUNK_PROP, dc);

	removeStringProperty(lchild, DATA_CHUNK_PROP);
}

static void
updateLimit(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));

	INFO_LOG("LIMIT OPERATOR");
	QueryOperator *lchild = OP_LCHILD(op);

	if (!HAS_STRING_PROP(lchild, DATA_CHUNK_PROP)) {
		return;
	}

	// if no data strcutre, indicate no order by lower;
	if (!HAS_STRING_PROP(op, DATA_STRUCTURE_STATE)) {
		return;
	}

	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(lchild, DATA_CHUNK_PROP);
	DataChunk *dc = initDataChunk;
	dc->tupleFields = dataChunk->tupleFields;
	dc->attrNames = (List *) copyObject(dataChunk->attrNames);
	dc->attriToPos = (HashMap *) copyObject(dataChunk->attriToPos);
	dc->posToDatatype = (HashMap *) copyObject(dataChunk->posToDatatype);
	LMTChunk *lmtc = (LMTChunk *) GET_STRING_PROP(op, DATA_CHUNK_PROP);

	/*
	 *	HERE, CURRENTL PAUSE:
	 *  IF WE USE LIST AS DONE NOW, TIME WILL BE HIGHT, SINCE sortList() -> qsort() WILL TAKE O(n lg n);
	 *  TRANSFER TO RED-BLACK TREE:
	*/

	// sort the data limit list;
	limitAttrPoss = (HashMap *) copyObject(lmtc->attrToPos);
	limitOrderBys = (List *) copyObject(((OrderOperator *) lchild)->orderExprs);

	// add value from datachunk;

	lmtc->vals = sortList(lmtc->vals, (int (*)(const void **, const void **)) limitCmp);

	// find the top-k;
}

static int
limitCmp(const void **a, const void **b) {
	List *la = (List *) *a;
	List *lb = (List *) *b;

	int res = 0;
	FOREACH(OrderExpr, oe, limitOrderBys) {
		AttributeReference *ar = (AttributeReference *) oe->expr;
		int pos = INT_VALUE((Constant *) MAP_GET_STRING(limitAttrPoss, ar->name));

		int cmp = compareTwoValues((Constant *) getNthOfListP(la, pos), (Constant *) getNthOfListP(lb, pos), ar->attrType);
		if (cmp) {
			if (oe->order == SORT_DESC) {
				res -= cmp;
			} else {
				res = cmp;
			}
			break;
		}
	}

	return res;
}

static void
updateTableAccess(QueryOperator * op)
{
	DEBUG_NODE_BEATIFY_LOG("CURRENT TABLEACCESS OPERATOR", op);
	/*
	 *  Check whether this table is updated?
	 *		Yes-> use the delta table;
	 * 		No -> nothing to do;
	 */
	char* tableName = ((TableAccessOperator *) op)->tableName;
	char* updatedTableName = NULL;

	Node* stmt = ((DLMorDDLOperator *) updateStatement)->stmt;
	switch(stmt->type) {
		case T_Update:
			updatedTableName = ((Update *) stmt)->updateTableName;
			break;
		case T_Insert:
			updatedTableName = ((Insert *) stmt)->insertTableName;
			break;
		case T_Delete:
			updatedTableName = ((Delete *) stmt)->deleteTableName;
			break;
		default:
			FATAL_LOG("error: cannot be here!");
			break;
	}

	if (strcmp(updatedTableName, tableName) != 0) {
		return;
	}

	// TODO
	// For batch update, there might be multiple updates for this table
	// we might use a list to store all the updates,

	DataChunk* dataChunk = getDataChunkFromUpdateStatement(updateStatement, (TableAccessOperator *) op);
	setStringProperty(op, DATA_CHUNK_PROP, (Node *) dataChunk);

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FRO TABLEACCESS OPERATOR", dataChunk);
}

static DataChunk*
getDataChunkFromUpdateStatement(QueryOperator* op, TableAccessOperator *tableAccessOp)
{
	/*
	 *  TODO: if multiple updates, we should pass a list of "DLMorDDLOperator"
	 *  here, we first consider only one update statement
	 */

	/*
	 *  for a update statement:
	 * 		case 1: "insert": insert one tuple
	 * 		case 2: "delete": delete multiple tuples: run the query to get result;
	 * 		case 3: "update": delete multiple tuples, then insert multiple tuples;
	 */
	List *psAttrInfoList = (List *) getMapString(PS_INFO->tablePSAttrInfos, tableAccessOp->tableName);
	psAttrInfo *attrInfo = (psAttrInfo *) getHeadOfListP(psAttrInfoList);
	DataChunk* dataChunk = initDataChunk();
	switch(nodeTag(((DLMorDDLOperator*) op)->stmt))
	{
		case T_Update:
			getDataChunkOfUpdate(op, dataChunk, tableAccessOp, attrInfo);
			break;
		case T_Insert:
			getDataChunkOfInsert(op, dataChunk, tableAccessOp, attrInfo);
			break;
		case T_Delete:
			getDataChunkOfDelete(op, dataChunk, tableAccessOp, attrInfo);
			break;
		default:
			FATAL_LOG("update should be an insert, delete or update? ");
	}
	return dataChunk;

}

static void
getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
{
	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));
	List *provAttrDefs = getProvenanceAttrDefs(rewr);
	char *psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;
	DEBUG_NODE_BEATIFY_LOG("WHAT IS OP", rewr);
	DEBUG_NODE_BEATIFY_LOG("WHAT IS PROV ATTR", provAttrDefs);
	DEBUG_NODE_BEATIFY_LOG("PS_INFO", attrInfo);
	INFO_LOG("ps name %s", psName);

	DEBUG_NODE_BEATIFY_LOG("INSERT", updateOp);
	Insert *insert = (Insert *) ((DLMorDDLOperator *) updateOp)->stmt;

	Schema *schema = createSchema(insert->insertTableName, insert->schema);
	//
	// fill data chunk;
	//
	int psAttrCol = -1;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++)
	{
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunk->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunk->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));

		// get ps attr col pos
		if (strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = i;
		}
	}

	dataChunk->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunk->numTuples = 1;
	dataChunk->tupleFields = LIST_LENGTH(schema->attrDefs);

	for (int index = 0; index < dataChunk->tupleFields; index++)
	{
		vecAppendNode(dataChunk->tuples, (Node*) makeVector(VECTOR_NODE, T_Vector));
	}

	List *tupleRow = (List *) insert->query;

	for (int col = 0; col < LIST_LENGTH(tupleRow); col++)
	{
		// DataType dataType = INT_VALUE((Constant*)getMapInt(dataChunk->posToDatatype, col));
		Constant *value = (Constant *) getNthOfListP(tupleRow, col);
		vecAppendNode((Vector *) getVecNode(dataChunk->tuples, col), (Node *) value);

		// set bit vector;
		if (col == psAttrCol) {
			BitSet *bitset = setFragmentToBitSet(INT_VALUE(value), attrInfo->rangeList);
			List *psList = NIL;
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, psName)) {
				psList = (List *) getMapString(dataChunk->fragmentsInfo, psName);
				psList = appendToTailOfList(psList, bitset);
				// addToMap(dataChunk->fragmentsInfo, createConstString(psName), (Node *) psList);
			} else {
				psList = singleton(bitset);
				// addToMap(dataChunk->fragmentsInfo, createConstStrign(psName), (Node *) psList);
			}
			addToMap(dataChunk->fragmentsInfo, (Node *) createConstString(psName), (Node *) psList);
		}
	}

	vecAppendInt(dataChunk->updateIdentifier, 1);

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR INSERT", dataChunk);
}

static void
getDataChunkOfDelete(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
{
	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));
	List *provAttrDefs = getProvenanceAttrDefs(rewr);
	char *psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;

	Delete * delete = (Delete *) ((DLMorDDLOperator *) updateOp)->stmt;
	DEBUG_NODE_BEATIFY_LOG("DELETE STMT: ", delete);
	// translate delete to selection projection
	// build TableAccess <- Selection <- projection

	Schema* schema = createSchema(delete->deleteTableName, delete->schema);

	// tableaccess;
	TableAccessOperator* taOp = createTableAccessOp(delete->deleteTableName, NULL, delete->deleteTableName, NIL, getAttrNames(schema), getDataTypes(schema));

	// selection;
	// TODO: append one (1 = 1) cond to condition incase there is no condition;
	SelectionOperator* selOp = createSelectionOp(delete->cond, (QueryOperator*) taOp, NIL, getAttrNames(schema));
	taOp->op.parents = singleton(selOp);

	// projection;
	List* projExprs = NIL;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++)
	{
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		AttributeReference * af = createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType);
		projExprs = appendToTailOfList(projExprs, af);
	}

	ProjectionOperator* projOp = createProjectionOp(projExprs, (QueryOperator*)selOp, NIL, getAttrNames(schema));
	selOp->op.parents = singleton(projOp);
	DEBUG_NODE_BEATIFY_LOG("BUILD PROJECTION: ", projOp);

	// serialize query;
	char* query = serializeQuery((QueryOperator*) projOp);
	INFO_LOG("SQL: %s", query);

	Relation *relation = getQueryResult(query);

	// check the result relation size;
	// if size == 0; return;
	if (LIST_LENGTH(relation->tuples) == 0)
	{
		return;
	}

	List* relSchema = relation->schema;
	for(int i = 0; i < LIST_LENGTH(relSchema); i++)
	{
		DEBUG_NODE_BEATIFY_LOG(getNthOfListP(relSchema, i));
	}

	// fill data chunk;
	int psAttrCol = -1;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++)
	{
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunk->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunk->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));

		// get ps attr col pos;
		if (strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = i;
		}
	}

	dataChunk->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunk->numTuples = LIST_LENGTH(relation->tuples);
	dataChunk->tupleFields = LIST_LENGTH(relation->schema);

	// fill tuples
	for(int col = 0; col < LIST_LENGTH(relation->tuples); col++)
	{
		int dataType = INT_VALUE((Constant*)getMapInt(dataChunk->posToDatatype, col));
		INFO_LOG("data type: %d", dataType);
		Vector *colValues = makeVector(VECTOR_NODE, T_Vector);
		for(int row = 0; row < dataChunk->numTuples; row++)
		{
			// char *val = (char *) getNthOfListP((List *) getNthOfListP(relation->tuples, row), col);
			Constant* value = makeValue(dataType, (char *) getNthOfListP((List *)getNthOfListP(relation->tuples, row), col));
			vecAppendNode(colValues, (Node*) value);
		}

		vecAppendNode(dataChunk->tuples, (Node*) colValues);
	}

	// fill identifier of update "-1"
	for (int row = 0; row < dataChunk->numTuples; row++)
	{
		vecAppendInt(dataChunk->updateIdentifier, -1);
	}

	// fill fragment information;
	List *psFragmentList = NIL;
	for (int row = 0; row < dataChunk->numTuples; row++) {

		Constant* value = makeValue(DT_INT, (char *) getNthOfListP((List *)getNthOfListP(relation->tuples, row), psAttrCol));

		BitSet *bitset = setFragmentToBitSet(INT_VALUE(value), attrInfo->rangeList);
		psFragmentList = appendToTailOfList(psFragmentList, bitset);
	}
	addToMap(dataChunk->fragmentsInfo, (Node *) createConstString(psName), (Node *) psFragmentList);

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR DELETE", dataChunk);
}

static void
getDataChunkOfUpdate(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
{
	DEBUG_NODE_BEATIFY_LOG("UPDATE", updateOp);
	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));
	DEBUG_NODE_BEATIFY_LOG("REWR OPP", rewr);
	List *provAttrDefs = getProvenanceAttrDefs(rewr);
	DEBUG_NODE_BEATIFY_LOG("PROV ATTR DEFS LIST", provAttrDefs);
	DEBUG_NODE_BEATIFY_LOG("PS_INFO", attrInfo);
	DEBUG_NODE_BEATIFY_LOG("WHAT ", getHeadOfListP(provAttrDefs));
	char *psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;
	INFO_LOG("PS NAME %s", psName);
	// get update statement;
	Update *update = (Update *) ((DLMorDDLOperator *) updateOp)->stmt;

	// create table access operator;
	Schema *schema = createSchema(update->updateTableName, update->schema);
	TableAccessOperator *taOp = createTableAccessOp(update->updateTableName, NULL, update->updateTableName, NIL, getAttrNames(schema), getDataTypes(schema));

	// create selection operator, and set table access operator's parent to it;
	SelectionOperator *selOp = createSelectionOp((Node *) copyObject(update->cond), (QueryOperator *) taOp, NIL, getAttrNames(schema));

	taOp->op.parents = singleton(selOp);
	DEBUG_NODE_BEATIFY_LOG("sel op", selOp);

	// create projection operator;
	// select attr as old_attr, attr' as new_attr, ... from xxxx;

	// key     -> list[ele1, ele2]
	// attname -> list[old_att, new_att]
	HashMap* map = NEW_MAP(Constant, Node);

	// att name to old name;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++) {
		AttributeDef *ad = (AttributeDef *) getNthOfListP(schema->attrDefs, i);
		AttributeReference *af = createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType);
		List *l = NIL;
		l = appendToTailOfList(l, af);
		addToMap(map, (Node*) createConstString(ad->attrName), (Node *) l);
	}

	DEBUG_NODE_BEATIFY_LOG("OLD MAP \n", map);
	// att name to new name;
	for (int i = 0; i < LIST_LENGTH(update->selectClause); i++) {
		// op->list[0]: old attribute name;
		// op->list[1]: new attribute name;
		Operator *op = (Operator *) getNthOfListP(update->selectClause, i);

		char* oldName = ((AttributeReference *) getNthOfListP(op->args, 0))->name;

		List * l = (List *) getMapString(map, oldName);
		l = appendToTailOfList(l, (Node *) getNthOfListP(op->args, 1));

		addToMap(map, (Node *) createConstString(oldName), (Node *) l);
	}
	DEBUG_NODE_BEATIFY_LOG("NEW MAP \n", map);

	// create projection operator;
	List* projExpr = NIL;
	List* attrNames = NIL;
	INFO_LOG("1");
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++) {
		AttributeDef *ad = (AttributeDef *) getNthOfListP(schema->attrDefs, i);

		StringInfo oldName = makeStringInfo();
		appendStringInfo(oldName, "old_%s", ad->attrName);
		StringInfo newName = makeStringInfo();
		appendStringInfo(newName, "new_%s", ad->attrName);

		attrNames = appendToTailOfList(attrNames, oldName->data);
		attrNames = appendToTailOfList(attrNames, newName->data);


		List *l = (List *) getMapString(map, ad->attrName);
		DEBUG_NODE_BEATIFY_LOG("list\n", l);
		projExpr = appendToTailOfList(projExpr, getNthOfListP(l, 0));

		if (LIST_LENGTH(l) == 1) {
			projExpr = appendToTailOfList(projExpr, getNthOfListP(l, 0));
		} else {
			projExpr = appendToTailOfList(projExpr, getNthOfListP(l, 1));
		}
	}

	ProjectionOperator *projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, attrNames);
	selOp->op.parents = singleton(projOp);

	DEBUG_NODE_BEATIFY_LOG("proj op\n", projOp);

	// serialize query;
	char * sql = serializeQuery((QueryOperator*) projOp);
	// INFO_LOG("SQL: %s", sql);
	Relation* relation = getQueryResult(sql);

	// fill data chunk;
	int psAttrCol = -1;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++) {
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunk->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunk->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));

		INFO_LOG("adname: %s, psName: %s", ad->attrName, psName);
		if (strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = i;
		}
	}

	// DEBUG_NODE_BEATIFY_LOG("pos->dt: ", dataChunk->posToDatatype);
	// DEBUG_NODE_BEATIFY_LOG("att->pos: ", dataChunk->attriToPos);
	dataChunk->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunk->numTuples = LIST_LENGTH(relation->tuples) * 2;
	dataChunk->tupleFields = LIST_LENGTH(relation->schema) / 2;

	// initialize each row vector;
	for (int index = 0; index < dataChunk->tupleFields; index++) {
		vecAppendNode(dataChunk->tuples, (Node*) makeVector(VECTOR_NODE, T_Vector));
	}
	INFO_LOG("psAttrCol %d", psAttrCol);

	// fill tuples
	List *psAttrList = NIL;
	for (int index = 0; index < LIST_LENGTH(relation->tuples); index++) {
		List *row = (List *) getNthOfListP(relation->tuples, index);

		for (int col = 0; col < LIST_LENGTH(row); col++) {
			int acturalCol = col / 2;
			int dataType = INT_VALUE((Constant *) getMapInt(dataChunk->posToDatatype, acturalCol));
			Constant *value = makeValue(dataType, (char *) getNthOfListP(row, col));
			vecAppendNode((Vector *) getVecNode(dataChunk->tuples, acturalCol), (Node *) value);

			if (acturalCol == psAttrCol) {
				BitSet *bitset = setFragmentToBitSet(INT_VALUE(value), attrInfo->rangeList);
				psAttrList = appendToTailOfList(psAttrList, bitset);
			}
		}

		// check the ps attribute, get the value, and set fragment info;
		vecAppendInt(dataChunk->updateIdentifier, -1);
		vecAppendInt(dataChunk->updateIdentifier, 1);
	}

	addToMap(dataChunk->fragmentsInfo, (Node *) createConstString(psName), (Node *) psAttrList);

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR UPDATE", dataChunk);
}

static DataChunk*
filterDataChunk(DataChunk* dataChunk, Node* condition)
{
	// TODO: use ConstrelOperator;
	DEBUG_NODE_BEATIFY_LOG("DISPLAY DATACHUNK: ", dataChunk);
	int choice = 2;
	char *sql = NULL;
	if (choice == 1) {
		// if this is locally used, it is fine to build this string.
		StringInfo strInfoSQL = makeStringInfo();
		appendStringInfo(strInfoSQL, "%s( %s", SQL_PRE, VALUES_IDENT);


		// ps attr list;
		List *provAttrsList = NIL;
		FOREACH_HASH_KEY(Node, key, dataChunk->fragmentsInfo) {
			provAttrsList = appendToTailOfList(provAttrsList, (Constant *) key);
		}
		DEBUG_NODE_BEATIFY_LOG("WHAT IS PROVATTRLIST", provAttrsList);

		// append data (tuple, sketch, update type);
		for (int row = 0; row < dataChunk->numTuples; row++) {
			StringInfo tmp = makeStringInfo();
			appendStringInfo(tmp, "(");

			// append tuple data;
			for (int col = 0; col < dataChunk->tupleFields; col++) {
				appendStringInfo(tmp, "%s,", exprToSQL((Node *) getVecNode((Vector *)getVecNode(dataChunk->tuples, col), row), NULL, FALSE));
			}

			// append prov bit vector;
			for (int col = 0; col < LIST_LENGTH(provAttrsList); col++) {
				List *bitsetList = (List *) getMapString(dataChunk->fragmentsInfo, STRING_VALUE((Constant *) getNthOfListP(provAttrsList, col)));
				BitSet *bitset = (BitSet *) getNthOfListP(bitsetList, row);
				appendStringInfo(tmp, "%s,", backendifyIdentifier(bitSetToString(bitset)));
			}

			// append update type identifier;
			appendStringInfo(tmp, gprom_itoa(getVecInt((Vector *) dataChunk->updateIdentifier, row)));

			appendStringInfo(tmp, ")");

			// append this tuple to sql string;
			appendStringInfo(strInfoSQL, tmp->data);
			if (row != dataChunk->numTuples - 1) {
				appendStringInfo(strInfoSQL, ",\n");
			}
		}

		appendStringInfo(strInfoSQL, ") tmp (");

		// append tuple attribute names
		for (int col = 0; col < dataChunk->tupleFields; col++) {
			appendStringInfo(strInfoSQL, "%s,", ((AttributeDef *) getNthOfListP(dataChunk->attrNames, col))->attrName);
		}

		// append prov attribute names;
		for (int col = 0; col < LIST_LENGTH(provAttrsList); col++) {
			appendStringInfo(strInfoSQL, "%s,", STRING_VALUE((Constant *) getNthOfListP(provAttrsList, col)));
		}

		// append update type name;
		appendStringInfo(strInfoSQL, "update_type");
		appendStringInfo(strInfoSQL, ") where ");

		// USE CONSTREL INSTEAD OF STRING;


		char* conditions = exprToSQL(condition, NULL, FALSE);
		appendStringInfo(strInfoSQL, conditions);

		INFO_LOG("THE FILTER SQL%s", strInfoSQL->data);
		sql = strInfoSQL->data;
	} else {
		ConstRelOperator *constRelOp = getConstRelOpFromDataChunk((DataChunk *) copyObject(dataChunk));
		// create a selection operator;
		SelectionOperator *selOp = createSelectionOp(condition, (QueryOperator *) constRelOp, NIL, getAttrDefNames(constRelOp->op.schema->attrDefs));
		constRelOp->op.parents = singleton(selOp);
		DEBUG_NODE_BEATIFY_LOG("SEL OP", selOp);

		// create a projection operator;
		List *projExpr = NIL;
		Schema *selSchema = selOp->op.schema;
		for (int index = 0; index < LIST_LENGTH(selSchema->attrDefs); index++) {
			AttributeDef *ad = (AttributeDef *) getNthOfListP(selSchema->attrDefs, index);
			AttributeReference *af = createFullAttrReference(ad->attrName, 0, index, 0, ad->dataType);
			projExpr = appendToTailOfList(projExpr, af);
		}

		ProjectionOperator *projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, getAttrNames(selOp->op.schema));
		selOp->op.parents = singleton(projOp);
		DEBUG_NODE_BEATIFY_LOG("PROJ", projOp);

		// serialization query;
		sql = serializeQuery((QueryOperator *) projOp);
		INFO_LOG("SQL %s", sql);
	}

	Relation* filterResult = getQueryResult(sql);

	// TODO: optimize
	// if no data here; we need return NULL; or set a flag to indicate this data chunk is empty;
	INFO_LOG("data size: %d", LIST_LENGTH(filterResult->tuples));
	if (LIST_LENGTH(filterResult->tuples) == 0) {
		return NULL;
	}

	// init result datachunk;
	DataChunk* resultDC = initDataChunk();
	resultDC->attrNames = (List*) copyList(dataChunk->attrNames);
	resultDC->attriToPos = (HashMap*) copyObject(dataChunk->attriToPos);
	resultDC->posToDatatype = (HashMap*) copyObject(dataChunk->posToDatatype);
	resultDC->tupleFields = dataChunk->tupleFields;
	resultDC->numTuples = LIST_LENGTH(filterResult->tuples);

	// fill data;
	int resultTupNum = LIST_LENGTH((List *) filterResult->tuples);
	for (int col = 0; col < resultDC->tupleFields; col++) {
		int dataType = INT_VALUE((Constant*)getMapInt(resultDC->posToDatatype, col));

		// create vector for this column;
		Vector* colValues = makeVector(VECTOR_NODE, T_Vector);

		for (int row = 0; row < resultTupNum; row++) {
			Constant* value = makeValue(dataType, (char *) getNthOfListP((List *) getNthOfListP(filterResult->tuples, row), col));
			vecAppendNode(colValues, (Node*) value);
		}

		vecAppendNode(resultDC->tuples, (Node*) colValues);
	}

	// fill sketch; postition is from that dataChunk->tupleFiles to length - 2;
	for (int col = resultDC->tupleFields; col < LIST_LENGTH(filterResult->schema) - 1; col++) {
		// get ps attr name;
		char *psAttrName = (char *) getNthOfListP(filterResult->schema, col);

		// init bit vector list for this attr;
		List *psBitVecList = NIL;

		for (int row = 0; row < resultTupNum; row++) {
			char *bitString = (char *) getNthOfListP((List *) getNthOfListP(filterResult->tuples, row), col);
			BitSet *bitset = stringToBitset(bitString);
			psBitVecList = appendToTailOfList(psBitVecList, bitset);
		}

		addToMap(resultDC->fragmentsInfo, (Node *) createConstString(psAttrName), (Node *) psBitVecList);
	}

	// fill update type; postion is last in result;
	for (int row = 0; row < resultTupNum; row++) {
		int col = LIST_LENGTH(filterResult->schema) - 1;
		int updateType = atoi((char *) getNthOfListP((List *) getNthOfListP(filterResult->tuples, row), col));
		vecAppendInt(resultDC->updateIdentifier, updateType);
	}

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR SELECTION/HAVING FILTER", resultDC);

	return resultDC;
}


static Constant *
makeValue(DataType dataType, char* value)
{

	Constant* c = NULL;
	switch (dataType) {
		case DT_INT:
			c = createConstInt(atoi(value));
			break;
		case DT_LONG:
			c = createConstLong(atol(value));
			break;
		case DT_STRING:
			c = createConstString(value);
			break;
		case DT_FLOAT:
			c = createConstFloat(atof(value));
			break;
		case DT_BOOL:
			c = createConstBoolFromString(value);
			break;
		case DT_VARCHAR2:
			c = createConstString(value);
			break;
		default:
			FATAL_LOG("not a supported type to cast\n", value);
	}

	return c;
}

static ConstRelOperator *
getConstRelOpFromDataChunk(DataChunk *dataChunk)
{
	List *values = NIL;
	List *attrNames = (List *) getAttrDefNames(dataChunk->attrNames);
	List *dataTypes = NIL;
	List *updateType = NIL;

	for (int col = 0; col < dataChunk->tupleFields; col++) {
		List *colValues = NIL;

		// get value from each vector cell;
		Vector *vec = (Vector *) getVecNode(dataChunk->tuples, col);
		for (int index = 0; index < VEC_LENGTH(vec); index++) {
			colValues = appendToTailOfList(colValues, getVecNode(vec, index));
		}

		values = appendToTailOfList(values, colValues);
		dataTypes = appendToTailOfListInt(dataTypes, INT_VALUE((Constant *) getMapInt(dataChunk->posToDatatype, col)));
	}

	// build list for sketch string;
	FOREACH_HASH_KEY(Node, key, dataChunk->fragmentsInfo) {
		List *bitsetList = NIL;
		List *bitsets = (List *) getMapString(dataChunk->fragmentsInfo, STRING_VALUE((Constant *) key));
		FOREACH(BitSet, bs, bitsets) {
			bitsetList = appendToTailOfList(bitsetList, createConstString(bitSetToString(bs)));
		}
		values = appendToTailOfList(values, bitsetList);
		// attrNames = appendToTailOfList(attrNames, createAttributeDef(STRING_VALUE((Constant *) key), DT_STRING));
		attrNames = appendToTailOfList(attrNames, STRING_VALUE((Constant *) key));
		dataTypes = appendToTailOfListInt(dataTypes, DT_STRING);
	}

	// build list for update type;
	for (int index = 0; index < VEC_LENGTH(dataChunk->updateIdentifier); index++) {
		updateType = appendToTailOfList(updateType, createConstInt(getVecInt(dataChunk->updateIdentifier, index)));
	}
	values = appendToTailOfList(values, updateType);
	attrNames = appendToTailOfList(attrNames, backendifyIdentifier("udpate_type"));
	dataTypes = appendToTailOfListInt(dataTypes, DT_INT);

	// create a constant real operator;
	ConstRelOperator *constRelOp = createConstRelOp(values, NIL, deepCopyStringList(attrNames), dataTypes);

	return constRelOp;
}

static BitSet *
setFragmentToBitSet(int value, List *rangeList)
{
	int fragNo = -1;

	//binary search;
	int start = 0;
	int end = LIST_LENGTH(rangeList) - 2;

	while (start + 1 < end) {
		int mid = start + (end - start) / 2;

		int leftVal = INT_VALUE((Constant *) getNthOfListP(rangeList, mid));
		int rightVal = INT_VALUE(getNthOfListP(rangeList, mid + 1));

		if (leftVal <= value && value < rightVal) {
			fragNo = mid;
			break;
		} else if (leftVal > value) {
			start = mid;
		} else {
			end = mid;
		}
	}

	// compare start;
	int startVal1 = INT_VALUE((Constant *) getNthOfListP(rangeList, start));
	int startVal2 = INT_VALUE((Constant *) getNthOfListP(rangeList, start + 1));

	if (startVal1 <= value && value < startVal2) {
		fragNo = start;
	}

	// compare end;
	int endVal1 = INT_VALUE((Constant *) getNthOfListP(rangeList, end));
	int endVal2 = INT_VALUE((Constant *) getNthOfListP(rangeList, end + 1));

	if (fragNo == -1 && endVal1 <= value && value < endVal2) {
		fragNo = end;
	}

	// dealing with value < min || val >= max;
	// if value < min -> set fragment to first;
	// if value >= max -> set fragment to last;
	if (fragNo == -1) {
		if (value < startVal1) {
			fragNo = 0;
		} else if (value >= endVal2) {
			fragNo = LIST_LENGTH(rangeList) - 1;
		}
	}

	// create a bitset
	BitSet *result = newBitSet(LIST_LENGTH(rangeList) - 1);
	setBit(result, fragNo, TRUE);

	return result;
}

static Relation*
getQueryResult(char* sql)
{
	Relation* relation = NULL;
	// TODO: generic executeQuery(sql);
	// get query result
	if (getBackend() == BACKEND_POSTGRES) {
		relation = postgresExecuteQuery(sql);
	} else if (getBackend() == BACKEND_ORACLE) {

	}

	return relation;
}

static void
executeQueryWithoutResult(char* sql)
{
	if (getBackend() == BACKEND_POSTGRES) {
		postgresExecuteStatement(sql);
	} else if (getBackend() == BACKEND_ORACLE) {

	}
}

static BitSet *
computeIsNullBitSet(Node *expr, DataChunk *dc)
{
	BitSet *result;
	switch(expr->type) {
		case T_Operator:{
			// List *inputs = ((Operator *) expr)->args;
			// BitSet *left = computeIsNullBitSet((Node *) getNthOfListP(inputs, 0), dc);
			// BitSet *right = computeIsNullBitSet((Node *) getNthOfListP(inputs, 1), dc);
			// result = bitOr(left, right);
			return newBitSet(10);// just for pass;
		}
		break;
		case T_AttributeReference: {
			int pos = INT_VALUE((Constant *) MAP_GET_STRING(dc->attriToPos, ((AttributeReference *) expr)->name));
			BitSet *bs = (BitSet *) getVecNode(dc->isNull, pos);
			result = (BitSet *) copyObject(bs);
		}
		break;
		case T_Constant: {
			result = newBitSet(dc->numTuples);
		}
		break;
		default:
			FATAL_LOG("cannot evaluate isNull for this operator");
	}

	return result;
}

static ColumnChunk *
evaluateExprOnDataChunk(Node *expr, DataChunk *dc)
{
	switch (expr->type) {
		case T_Operator:
			return evaluateOperatorOnDataChunk((Operator *) expr, dc);
		case T_AttributeReference:
			return getColumnChunkOfAttr(((AttributeReference *) expr)->name, dc);
		case T_Constant:
			return getColumnChunkOfConst((Constant *) expr, dc);
		default:
			FATAL_LOG("cannot evaluate this expr");
	}

	return 0;
}

static ColumnChunk *
getColumnChunkOfConst(Constant *c, DataChunk *dc)
{
	// column chunk length;
	int length = dc->numTuples;

	// create a new column chunk;
	ColumnChunk *cc = makeColumnChunk(c->constType, (size_t) length);


	switch (c->constType) {
		case DT_INT: {
			int *ccValue = VEC_TO_IA(cc->data.v);
			for (int i = 0; i < length; i++) {
				ccValue[i] = INT_VALUE(c);
			}
		}
		break;
		case DT_FLOAT: {
			double *ccValue = VEC_TO_FA(cc->data.v);
			for (int i = 0; i < length; i++) {
				ccValue[i] = FLOAT_VALUE(c);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *ccValue = VEC_TO_LA(cc->data.v);
			for (int i = 0; i < length; i++) {
				ccValue[i] = LONG_VALUE(c);
			}
		}
		break;
		case DT_STRING:
		case DT_VARCHAR2: {
			char **ccValue = VEC_TO_ARR(cc->data.v, char);
			for (int i = 0; i < length; i++) {
				ccValue[i] = STRING_VALUE(c);
			}
		}
		break;
		case DT_BOOL: {
			BitSet *bs = newBitSet(length);
			for (int i = 0; i < length; i++) {
				if (TRUE == BOOL_VALUE(c)) {
					setBit(bs, i, TRUE);
				}
			}
			cc->data.bs = copyObject(bs);
		}
		break;
		default:
			FATAL_LOG("data type %d is not supported", c->constType);
	}

	return cc;

}

static ColumnChunk *
getColumnChunkOfAttr(char *attrName, DataChunk *dataChunk)
{
	// get column vector of this attribute;
	int pos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, attrName));
	Vector *vec = (Vector *) getVecNode(dataChunk->tuples, pos);

	// create the result column chunk of this datatype;
	DataType dataType = (DataType) INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, pos));
	ColumnChunk *cc = makeColumnChunk(dataType, dataChunk->numTuples);

	// get original column values from data chunk;
	Constant **consts = VEC_TO_ARR(vec, Constant);

	switch (dataType) {
		case DT_INT: {
			int *ccValues = VEC_TO_IA(cc->data.v);
			for (int i = 0; i < cc->length; i++) {
				ccValues[i] = INT_VALUE(consts[i]);
			}
		}
		break;
		case DT_FLOAT: {
			double *ccValues = VEC_TO_FA(cc->data.v);
			for (int i = 0; i < cc->length; i++) {
				ccValues[i] = FLOAT_VALUE(consts[i]);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *ccValues = VEC_TO_LA(cc->data.v);
			for (int i = 0; i < cc->length; i++) {
				ccValues[i] = LONG_VALUE(consts[i]);
			}
		}
		break;
		case DT_STRING:
		case DT_VARCHAR2: {
			char **ccValues = (char **) VEC_TO_ARR(cc->data.v, char);
			for (int i = 0; i < cc->length; i++) {
				ccValues[i] = STRING_VALUE(consts[i]);
			}
		}
		break;
		case DT_BOOL: {
			BitSet *bs = newBitSet(cc->length);
			for (int i = 0; i < cc->length; i++) {
				if (TRUE == BOOL_VALUE(consts[i])) {
					setBit(bs, i, TRUE);
				}
			}
			cc->data.bs = (BitSet *) copyObject;
		}
		break;
		default:
			FATAL_LOG("datatype %d is not supported", dataType);
			break;
	}
	return cc;
}

static ColumnChunk *
evaluateOperatorOnDataChunk(Operator *op, DataChunk *dc)
{
	ColumnChunk *result = NULL;
	if (streq(op->name, OPNAME_ADD)) { // arithmetic operators;
		// "a + b"
		return evaluateOperatorPlus(op, dc);
	} else if (streq(op->name, OPNAME_MINUS)) {
		// "a - b"
		return evaluateOperatorMinus(op, dc);
	} else if (streq(op->name, OPNAME_DIV)) {
		// "a / b"
		return evaluateOperatorDiv(op, dc);
	} else if (streq(op->name, OPNAME_MULT)) {
		// "a * b"
		return evaluateOperatorMult(op, dc);
	} else if (streq(op->name, OPNAME_MOD)) {
		// "a % b"
		return evaluateOperatorMod(op, dc);
	} else if (streq(op->name, OPNAME_AND)) { // logic operators;
		// "a AND b"
		return evaluateOperatorAnd(op, dc);
	} else if (streq(op->name, OPNAME_OR)) {
		// "a OR b"
		return evaluateOperatorOr(op, dc);
	} else if (streq(op->name, OPNAME_NOT)) {
		// "NOT a"
		return evaluateOperatorNot(op, dc);
	} else if (streq(op->name, OPNAME_not)) {
		// "not a"
		return evaluateOperatorNot(op, dc);
	} else if (streq(op->name, OPNAME_EQ)) {
		// "a = b"
		return evaluateOperatorEq(op, dc);
	} else if (streq(op->name, OPNAME_LT)) {
		// "a < b"
		return evaluateOperatorLt(op, dc);
	} else if (streq(op->name, OPNAME_LE)) {
		// "a <= b"
		return evaluateOperatorLe(op, dc);
	} else if (streq(op->name, OPNAME_GT)) {
		// "a > b"
		return evaluateOperatorGt(op, dc);
	} else if (streq(op->name, OPNAME_GE)) {
		// "a >= b"
		return evaluateOperatorGe(op, dc);
	} else if (streq(op->name, OPNAME_NEQ)) {
		// "a <> b"
		return evaluateOperatorNeq(op, dc);
	} else if (streq(op->name, OPNAME_NEQ_BANG)) {
		// "a != b"
		return evaluateOperatorNeq(op, dc);
	} else if (streq(op->name, OPNAME_NEQ_HAT)) {
		// "^=" do not work for postgres; try ORACLE;
		return evaluateOperatorNeq(op, dc);
	} else if (streq(op->name, OPNAME_STRING_CONCAT)) { // string operators;
		// "a || b";
		return evaluateOperatorConcat(op, dc);
	} else if (streq(op->name, OPNAME_CONCAT)) {
		// "concat(a, b)"
		return evaluateOperatorConcat(op, dc);
	} else if (streq(op->name, OPNAME_LIKE)) {
		// "a like b"
		return evaluateOperatorLike(op, dc);
	}

	return result;
}

static ColumnChunk *
makeColumnChunk(DataType dataType, size_t len)
{
	ColumnChunk *columnChunk = makeNode(ColumnChunk);
	columnChunk->dataType = dataType;
	columnChunk->length = len;
	columnChunk->isBit = FALSE;

	switch (dataType) {
		case DT_INT: {
			columnChunk->data.v = makeVectorOfSize(VECTOR_INT, T_Vector, len);
			columnChunk->data.v->length = len;
		}
		break;
		case DT_FLOAT: {
			columnChunk->data.v = makeVectorOfSize(VECTOR_FLOAT, T_Vector, len);
			columnChunk->data.v->length = len;
		}
		break;
		case DT_LONG: {
			columnChunk->data.v = makeVectorOfSize(VECTOR_LONG, T_Vector, len);
			columnChunk->data.v->length = len;
		}
		break;
		case DT_STRING:
		case DT_VARCHAR2: {
			columnChunk->data.v = makeVectorOfSize(VECTOR_STRING, T_Vector, len);
			columnChunk->data.v->length = len;
		}
		break;
		case DT_BOOL: {
			columnChunk->isBit = TRUE;
			columnChunk->data.bs = newBitSet(len);
		}
		break;
	}

	return columnChunk;
}

static ColumnChunk *
evaluateOperatorPlus(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int len = left->length;

	DataType resultDT = typeOf((Node *) op);
	ColumnChunk *resultCC = makeColumnChunk(resultDT, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	// future, if have isNULL bitset, get bitset to identify null
	// two options: 1. accurately get if is null
	//				2. just calculate the value for +/-/*/ but we do not care the result.
	switch (resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			int *resV = VEC_TO_IA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] + rightV[i];
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			gprom_long_t *resV = VEC_TO_LA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] + rightV[i];
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			double *resV = VEC_TO_FA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] + rightV[i];
			}
		}
		break;
		case DT_STRING: // not applicable for "-"
		case DT_VARCHAR2:// not applicable for "-"
		case DT_BOOL:// not applicable for "-"
		default:
			FATAL_LOG("operator '+' is not supported for data type %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorMinus(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int len = left->length;

	DataType resultDT = typeOf((Node *) op);
	ColumnChunk *resultCC = makeColumnChunk(resultDT, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch (resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			int *resV = VEC_TO_IA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] - rightV[i];
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			gprom_long_t *resV = VEC_TO_LA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] - rightV[i];
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			double *resV = VEC_TO_FA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] - rightV[i];
			}
		}
		break;
		case DT_STRING: // not applicable for "-"
		case DT_VARCHAR2:// not applicable for "-"
		case DT_BOOL:// not applicable for "-"
		default:
			FATAL_LOG("operator '-' is not supported for data type %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorMult(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int len = left->length;

	DataType resultDT = typeOf((Node *) op);
	ColumnChunk *resultCC = makeColumnChunk(resultDT, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch (resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			int *resV = VEC_TO_IA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] * rightV[i];
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			gprom_long_t *resV = VEC_TO_LA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] * rightV[i];
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			double *resV = VEC_TO_FA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] * rightV[i];
			}
		}
		break;
		case DT_STRING: // not applicable for "*"
		case DT_VARCHAR2:// not applicable for "*"
		case DT_BOOL:// not applicable for "*"
		default:
			FATAL_LOG("operator '*' is not supported for data type %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorDiv(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int len = left->length;

	DataType resultDT = typeOf((Node *) op);
	ColumnChunk *resultCC = makeColumnChunk(resultDT, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch (resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			int *resV = VEC_TO_IA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				if (rightV[i] == 0) {
					FATAL_LOG("division by zero");
				}

				resV[i] = leftV[i] / rightV[i];
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			gprom_long_t *resV = VEC_TO_LA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				if (rightV[i] == (gprom_long_t) 0) {
					FATAL_LOG("division by zero");
				}

				resV[i] = leftV[i] / rightV[i];
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			double *resV = VEC_TO_FA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				if (rightV[i] == (double) 0) {
					FATAL_LOG("division by zero");
				}

				resV[i] = leftV[i] / rightV[i];
			}
		}
		break;
		case DT_STRING: // not applicable for "*"
		case DT_VARCHAR2:// not applicable for "*"
		case DT_BOOL:// not applicable for "*"
		default:
			FATAL_LOG("operator '/' is not supported for data type %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorMod(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int len = left->length;

	DataType resultDT = typeOf((Node *) op);
	ColumnChunk *resultCC = makeColumnChunk(resultDT, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch (resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			int *resV = VEC_TO_IA(resultCC->data.v);
			for (int i = 0; i < len; i++) {
				if (rightV[i] == 0) {
					FATAL_LOG("division by zero");
				}

				resV[i] = leftV[i] % rightV[i];
			}
		}
		break;
		case DT_LONG:
		case DT_FLOAT:
		case DT_STRING: // not applicable for "%"
		case DT_VARCHAR2:// not applicable for "%"
		case DT_BOOL:// not applicable for "%"
		default:
			FATAL_LOG("operator '%' is not supported for data type %d", resultDT);
	}
	return resultCC;
}

static ColumnChunk *
evaluateOperatorAnd(Operator *op, DataChunk *dc)
{
	// both left and right operand are bitset;
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	left->data.bs = bitAnd(left->data.bs, right->data.bs);

	return left;
}

static ColumnChunk *
evaluateOperatorOr(Operator *op, DataChunk *dc)
{
	// both left and right operand are bitset;
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	left->data.bs = bitOr(left->data.bs, right->data.bs);

	return left;
}

static ColumnChunk *
evaluateOperatorNot(Operator *op, DataChunk *dc)
{
	// both left and right operand are bitset;
	// only have one "NOT(EXPR)"
	List *inputs = op->args;
	ColumnChunk *cc = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);

	cc->data.bs = bitNot(cc->data.bs);
	return cc;
}

static ColumnChunk *
evaluateOperatorEq(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	DataType resultDT = lcaType(left->dataType, right->dataType);
	int len = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] == rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] == rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] == rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}

		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) == 0) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_BOOL:
		default:
			FATAL_LOG("operator '=' is not supported for datatype %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorLt(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	DataType resultDT = lcaType(left->dataType, right->dataType);
	int len = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] < rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] < rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] < rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) < 0) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_BOOL:
		default:
			FATAL_LOG("operator '<' is not supported for datatype %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorLe(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	DataType resultDT = lcaType(left->dataType, right->dataType);
	int len = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] <= rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] <= rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] <= rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) <= 0) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_BOOL:
		default:
			FATAL_LOG("operator '<=' is not supported for datatype %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorGt(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	DataType resultDT = lcaType(left->dataType, right->dataType);
	int len = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] > rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] > rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] > rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}

		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) > 0) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_BOOL:
		default:
			FATAL_LOG("operator '>' is not supported for datatype %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorGe(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	DataType resultDT = lcaType(left->dataType, right->dataType);
	int len = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] >= rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] >= rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] >= rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) >= 0) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_BOOL:
		default:
			FATAL_LOG("operator '>' is not supported for datatype %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorNeq(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	DataType resultDT = lcaType(left->dataType, right->dataType);
	int len = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, len);

	ColumnChunk *castLeft;
	if (left->dataType != resultDT) {
		castLeft = castColumnChunk(left, left->dataType, resultDT);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight;
	if (right->dataType != resultDT) {
		castRight = castColumnChunk(right, right->dataType, resultDT);
	} else {
		castRight = right;
	}

	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] != rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] != rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				if (leftV[i] != rightV[i]) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) != 0) {
					setBit(resultCC->data.bs, i, TRUE);
				}
			}
		}
		break;
		case DT_BOOL:
		default:
			FATAL_LOG("operator '!=' is not supported for datatype %d", resultDT);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorConcat(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk(getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk(getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int length = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_STRING, length);

	ColumnChunk *castLeft = NULL;
	if (DT_STRING != left->dataType && DT_VARCHAR2 != left->dataType) {
		castLeft = castColumnChunk(left, left->dataType, DT_STRING);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight = NULL;
	if (DT_STRING != right->dataType && DT_VARCHAR2 != right->dataType) {
		castRight = castColumnChunk(right, right->dataType, DT_STRING);
	} else {
		castRight = right;
	}

	char **leftV = VEC_TO_ARR(castLeft->data.v, char);
	char **rightV = VEC_TO_ARR(castRight->data.v, char);
	char **resultV = VEC_TO_ARR(resultCC->data.v, char);

	for (int i = 0; i < length; i++) {
		resultV[i] = CONCAT_STRINGS(leftV[i], rightV[i]);
	}

	return resultCC;
}

static ColumnChunk *
evaluateOperatorLike(Operator *op, DataChunk *dc)
{
	/*
	 * Now support two wildcards:
	 *	1. %;
	 * 	2. _;
	 */
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk(getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk(getNthOfListP(inputs, 1), dc);

	ASSERT(left->length == right->length);

	int length = left->length;

	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, length);

	ColumnChunk *castLeft = NULL;
	if (DT_STRING != left->dataType && DT_VARCHAR2 != left->dataType) {
		castLeft = castColumnChunk(left, left->dataType, DT_STRING);
	} else {
		castLeft = left;
	}

	ColumnChunk *castRight = NULL;
	if (DT_STRING != right->dataType && DT_VARCHAR2 != right->dataType) {
		castRight = castColumnChunk(right, right->dataType, DT_STRING);
	} else {
		castRight = right;
	}

	ASSERT(castLeft->dataType == DT_STRING && castRight->dataType == DT_STRING);

	char **leftV = VEC_TO_ARR(castLeft->data.v, char);
	char **rightV = VEC_TO_ARR(castRight->data.v, char);

	StringInfo pattern = makeStringInfo();
	appendStringInfo(pattern, "%s", rightV[0]);

	// matchMod: 0. pure string, 1. only "_", 2. only "%", 3. "_" and "%"
	int matchMode = 0;
	StringInfo regExPtn= makeStringInfo(); // only use for matchMode is 3;

	// modMode: 1. "%aaa", 2. "aaa%", 3. "%aaa%", 4. arbitrary: "%";
	// Suppose the pattern contains at least one normal character, and cannot contains only "%"s;
	int modMode = 0;

	for (int i = 0; i < strlen(pattern->data); i++) {
		if (pattern->data[i] == '_') {
			if (matchMode == 0) {
				matchMode = 1;
			} else if (matchMode == 2) {
				matchMode = 3;
			}

			appendStringInfo(regExPtn, ".");
		} else if (pattern->data[i] == '%') {
			if (matchMode == 0) {
				matchMode = 2;
			} else if (matchMode == 1) {
				matchMode = 3;
			}

			appendStringInfo(regExPtn, ".*");

			// dealing with "%" mode;
			if (i == 0) {
				modMode = 1;
			} else if (i == strlen(pattern->data) - 1) {
				if (modMode == 1) {
					modMode = 3;
				} else if(modMode == 0) {
					modMode = 2;
				}
			} else {
				modMode = 4;
			}
		}
	}

	StringInfo modPtn = makeStringInfo();
	if (matchMode == 2) {
		if (modMode == 1) {
			appendStringInfo(modPtn, "%s", strRemPrefix(pattern->data, 1));
		} else if (modMode == 2) {
			appendStringInfo(modPtn, "%s", strRemPostfix(pattern->data, strlen(pattern->data) - 1));
		} else if (modMode == 3) {
			char *tmp = strRemPrefix(pattern->data, 1);
			appendStringInfo(modPtn, "%s", strRemPostfix(tmp, strlen(tmp) - 1));
		}
	}

	for (int i = 0; i < length; i++) {
		boolean res = TRUE;;

		if (matchMode == 0) {
			res = ((strlen(leftV[i]) == strlen(rightV[i])) && (streq(leftV[i], rightV[i])));
		} else if (matchMode == 1) {
			if (strlen(leftV[i]) == strlen(rightV[i])) {
				for (int j = 0; j < strlen(pattern->data); j++) {
					if (pattern->data[j] == '_') {
						continue;
					}

					if (!streq(leftV[i], rightV[i])) {
						res = FALSE;
						break;
					}
				}
			} else {
				res = FALSE;
			}
		} else if (matchMode == 2) {
			if (modMode == 1) {
				if (!isSuffix(leftV[i], modPtn->data)) {
					res = FALSE;
				}
			} else if (modMode == 2) {
				if (!isPrefix(leftV[i], modPtn->data)) {
					res = FALSE;
				}
			} else if (modMode == 3) {
				if (!isSubstr(leftV[i], modPtn->data)) {
					res = FALSE;
				}
			} else { // more than 2 '%', use regExMatch();
				if (!regExMatch(regExPtn->data, leftV[i])) {
					res = FALSE;
				}
			}
		} else if (matchMode == 3) {
			if (!regExMatch(regExPtn->data, leftV[i])) {
				res = FALSE;
			}
		}

		if (res) {
			setBit(resultCC->data.bs, i, res);
		}
	}

	return resultCC;
}

static ColumnChunk *
castColumnChunk(ColumnChunk *cc, DataType fromType, DataType toType)
{
	// so far, this supports  int, float, long, bool -> char*
	if (fromType == toType
	|| (fromType == DT_STRING && toType == DT_VARCHAR2)
	|| (fromType == DT_VARCHAR2 && toType == DT_STRING)) {
		return (ColumnChunk *) copyObject(cc);
	}

	ColumnChunk *resultCC = makeColumnChunk(toType, cc->length);
	int length = cc->length;

	switch (toType) {
		case DT_STRING:
		case DT_VARCHAR2: {
			char **resV = VEC_TO_ARR(resultCC->data.v, char);

			if (DT_INT == fromType) {
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = gprom_itoa(vals[i]);
				}
			} else if (DT_FLOAT == fromType) {
				double *vals = VEC_TO_FA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = gprom_ftoa(vals[i]);
				}
			} else if (DT_LONG == fromType) {
				gprom_long_t *vals = VEC_TO_LA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = gprom_ltoa(vals[i]);
				}
			} else if (DT_BOOL == fromType) {
				BitSet *bs = cc->data.bs;
				char *bStr = bitSetToString(bs);
				for (int i = 0; i < length; i++) {
					char c = bStr[i];
					if (getBackend() == BACKEND_POSTGRES) {
						resV[i] = strdup(c == '1' ? "t" : "f");
					} else if (getBackend() == BACKEND_ORACLE) {
						// since oracle does not support boolean datatype
						// here we use the same as postgresql;
						resV[i] = strdup(c == '1' ? "t" : "f");
					}
				}
			}
		}
		break;
		case DT_INT: {
			int *resV = VEC_TO_IA(resultCC->data.v);

			if (DT_FLOAT == fromType) {
				double *vals = VEC_TO_FA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (int) vals[i];
				}
			} else if (DT_LONG == fromType) {
				gprom_long_t *vals = VEC_TO_LA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (int) vals[i];
				}
			} else if (DT_BOOL == fromType) {
				BitSet *bs = cc->data.bs;
				char *bStr = bitSetToString(bs);

				for (int i = 0; i < length; i++) {
					char c = bStr[i];
					resV[i] = (c == '1' ? 1 : 0);
				}
			} else if (DT_STRING == fromType) {
				// TODO:
			}
		}
		break;
		case DT_FLOAT: {
			double *resV = VEC_TO_FA(resultCC->data.v);

			if (DT_INT == fromType) {
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (double) vals[i];
				}
			} else if (DT_LONG == fromType) {
				gprom_long_t *vals = VEC_TO_LA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (double) vals[i];
				}
			} else if (DT_BOOL == fromType) {
				BitSet *bs = cc->data.bs;
				char *bStr = bitSetToString(bs);

				for (int i = 0; i < length; i++) {
					char c = bStr[i];
					resV[i] = (c == '1' ? (double) 1 : (double) 0);
				}
			} else if (DT_STRING == fromType) {
				// TODO:
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *resV = VEC_TO_LA(resultCC->data.v);
			if (DT_INT == fromType) {
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (gprom_long_t) vals[i];
				}
			} else if (DT_FLOAT == fromType) {
				double *vals = VEC_TO_FA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (gprom_long_t) vals[i];
				}
			} else if (DT_BOOL == fromType) {
				BitSet *bs = cc->data.bs;
				char *bStr = bitSetToString(bs);

				for (int i = 0; i < length; i++) {
					char c = bStr[i];
					resV[i] = (c == '1' ? (gprom_long_t) 1 : (gprom_long_t) 0);
				}
			} else if (DT_STRING == fromType) {
				// TODO:
			}
		}
		break;
		case DT_BOOL: {
			// TODO:
		}
		default:
			FATAL_LOG("cast from %d to %d is not supported now", fromType, toType);
	}

	return resultCC;
}

static Vector *
columnChunkToVector(ColumnChunk *cc) {
	int length = cc->length;
	Vector *v = makeVectorOfSize(VECTOR_NODE, T_Vector, length);

	switch (cc->dataType) {
		case DT_INT: {
			int *ccVals = VEC_TO_IA(cc->data.v);
			for (int i = 0; i < length; i++) {
				vecAppendNode(v, (Node *) createConstInt(ccVals[i]));
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *ccVals = VEC_TO_LA(cc->data.v);
			for (int i = 0; i < length; i++) {
				vecAppendNode(v, (Node *) createConstLong(ccVals[i]));
			}
		}
		break;
		case DT_FLOAT: {
			double *ccVals = VEC_TO_FA(cc->data.v);
			for (int i = 0; i < length; i++) {
				vecAppendNode(v, (Node *) createConstFloat(ccVals[i]));
			}
		}
		break;
		case DT_STRING:
		case DT_VARCHAR2: {
			char **ccVals = VEC_TO_ARR(cc->data.v, char);
			for (int i = 0; i < length; i++) {
				vecAppendNode(v, (Node *) createConstString(ccVals[i]));
			}
		}
		break;
		case DT_BOOL: {
			BitSet *bs = cc->data.bs;
			char * bstr = bitSetToString(bs);
			for (int i = 0; i < length; i++) {
				char c = bstr[i];
				vecAppendNode(v, (Node *) createConstBool((c == '1' ? TRUE : FALSE)));
			}
		}
		break;
		default:
			FATAL_LOG("data type %d is not support", cc->dataType);
	}
	return v;
}

static char *
constToString(Constant *c)
{
	StringInfo info = makeStringInfo();

	switch(c->constType) {
		case DT_INT:
			appendStringInfo(info, "%s", gprom_itoa(INT_VALUE(c)));
			break;
		case DT_FLOAT:
			appendStringInfo(info, "%s", gprom_ftoa(FLOAT_VALUE(c)));
			break;
		case DT_LONG:
			appendStringInfo(info, "%s", gprom_ltoa(LONG_VALUE(c)));
			break;
		case DT_STRING:
			appendStringInfo(info, "%s", STRING_VALUE(c));
			break;
		case DT_BOOL:
			appendStringInfo(info, "%s", BOOL_VALUE(c) == 1 ? "t" : "f");
			break;
		default:
			FATAL_LOG("datatype %d is not supported", c->constType);
	}

	return info->data;
}