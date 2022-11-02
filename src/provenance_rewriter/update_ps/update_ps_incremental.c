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
static void updateSet(QueryOperator* op) ;
// OrderOperator
// LimitOperator

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
//static void setNumberToEachOperator(QueryOperator *operator);

#define SQL_PRE backendifyIdentifier("SELECT * FROM ")
#define VALUES_IDENT backendifyIdentifier(" VALUES ")
#define ADD_FUNC_PREFIX backendifyIdentifier("ADD_FUNC_PREFIX")
#define LEFT_BRANCH_OF_JOIN backendifyIdentifier("LEFT_BRANCH_OF_JOIN")
#define RIGHT_BRANCH_OF_JOIN backendifyIdentifier("RIGHT_BRANCH_OF_JOIN")
#define LEFT_UPDATE_IDENTIFIER backendifyIdentifier("LEFT_UPDATE_TYPE")
#define RIGHT_UPDATE_IDENTIFIER backendifyIdentifier("RIGHT_UPDATE_TYPE")

#define MIN_HEAP "MIN_HEAP"
#define MAX_HEAP "MAX_HEAP"
#define OPERATOR_NUMBER "OPERATOR_NUMBER"
#define PROV_BIT_STRING "PROV_BIT_STRING"
#define DATA_STRUCTURE_STATE "DATA_STRUCTURE_STATE"

#define LCHILD_POS(pos) (2 * pos + 1)
#define RCHILD_POS(pos) (2 * pos + 2)
#define PARENT_POS(pos) ((pos - 1) / 2)
/*
 *  TODO
 *  1. check if the inut is null
 * 		YES: nothing to update in this branch
 * 		NO : update
 */

// dummy result
static StringInfo strInfo;
static QueryOperator* updateStatement = NULL;
static psInfo *PS_INFO = NULL;
static ProvenanceComputation *PC = NULL;
//static HashMap *coarseGrainedRangeList = NULL;

void
heapInsert(List *list, char *type, Node *ele)
{
	list = appendToTailOfList(list, ele);
	Constant *value = (Constant *) getHeadOfListP((List *) ele);
	heapifyListSiftUp(list, LIST_LENGTH(list) - 1, type, value->constType, 0);
}

void
heapifyListSiftUp(List *list, int pos, char *type, DataType valDataType, int valuePos)
{
	while (pos > 0) {
		int parentPos = PARENT_POS(pos);

		List *curr = (List *) getNthOfListP(list, pos);
		Constant *currVal = (Constant *) getNthOfListP(curr, 0);

		List *parent = (List *) getNthOfListP(list, parentPos);
		Constant *parentVal = (Constant *) getNthOfListP(parent, 0);

		int compTwoVal = compareTwoValues(currVal, parentVal, valDataType);

		if (compTwoVal < 0) {
			if (strcmp(type, MIN_HEAP) == 0) { // current < parent, for min heap, continue;
				swapListCell(list, pos, parentPos);
			} else { // terminate for max heap when current < parent;
				return;
			}
		} else if (compTwoVal > 0) {
			if (strcmp(type, MAX_HEAP) == 0) { // current > parent, for max heap, continue;
				swapListCell(list, pos, parentPos);
			} else { // terminate for min heap when min heap;
				return;
			}
		} else { // terminate for both min and max heap when current == parent;
			return;
		}

		pos = parentPos;
	}
}


void
heapDelete(List *list, char *type, Node *ele)
{
	for (int i = 0; i < LIST_LENGTH(list); i++) {
		if (equal((Node *) getNthOfListP(list, i), ele)) {
			heapifyListSiftDown(list, i, type, ((Constant *) getHeadOfListP((List *) ele))->constType, 0);
			break;
		}
	}
}

void
heapifyListSiftDown(List *list, int pos, char *type, DataType valDataType, int valuePos)
{
	while (LCHILD_POS(pos) < LIST_LENGTH(list)) {
		// get lChild;
		List *lChild = (List *) getNthOfListP(list, LCHILD_POS(pos));
		Constant *lChildVal = (Constant *) getNthOfListP(lChild, 0);

		if (RCHILD_POS(pos) < LIST_LENGTH(list)) { // has two children
			List *rChild = (List *) getNthOfListP(list, RCHILD_POS(pos));
			Constant *rChildVal = (Constant *) getNthOfListP(rChild, 0);

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
			heapifyListSiftUp(newList, i, type, valDataType, valuePos);
		}
	}

	list = newList;
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
	return dc;
}

char *
update_ps_incremental(QueryOperator* operator){

	// get partition information;
	PC = (ProvenanceComputation *) copyObject(operator);
	PS_INFO = createPSInfo((Node *) getStringProperty(operator, PROP_PC_COARSE_GRAINED));


	DEBUG_NODE_BEATIFY_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR: \n", operator);
	strInfo = makeStringInfo();

	// left  child: update statement;
	// right child: query;
	updateStatement = (QueryOperator *) OP_LCHILD(operator);
	QueryOperator *rChild = OP_RCHILD(operator);

	operator->inputs = singleton(rChild);

////	DEBUG_NODE_BEATIFY_LOG("root operator:", operator);
////	INFO_OP_LOG("root operator:", operator);
////
////	QueryOperator *qOp = captureRewrite(rChild);
////	DEBUG_NODE_BEATIFY_LOG("QOP", qOp);
////	INFO_OP_LOG("QOP", qOp);
//
//	char *sqlll = serializeQuery(qOp);
//	INFO_LOG("SQLLL %s", sqlll);

	// build state for each aggregation operator
//	if(2 != 2) {
//		return "FINISH";
		buildState((Node *) operator);
//	}

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
		AggregationOperator *aggOp = (AggregationOperator *) node;
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
				avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, ad->attrName);

				// add function calls sum and count for avg;
				if (strcmp(fcName, AVG_FUNC_NAME) == 0) {
					// add function call sum;
					FunctionCall *fc_sum = createFunctionCall(SUM_FUNC_NAME, copyList(fc->args));
					avg_sum_count = appendToTailOfList(avg_sum_count, fc_sum);

					StringInfo sum_fc_name = makeStringInfo();
					appendStringInfo(sum_fc_name, "%s_%s_%s", ADD_FUNC_PREFIX, SUM_FUNC_NAME, ad->attrName);
					avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, sum_fc_name->data);

					// add function call count;
					FunctionCall *fc_cnt = createFunctionCall(COUNT_FUNC_NAME, copyList(fc->args));
					avg_sum_count = appendToTailOfList(avg_sum_count, fc_cnt);

					StringInfo cnt_fc_name = makeStringInfo();
					appendStringInfo(cnt_fc_name, "%s_%s_%s", ADD_FUNC_PREFIX, COUNT_FUNC_NAME, ad->attrName);
					avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, cnt_fc_name->data);
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

			// get prov attr name;
			QueryOperator *auxOP = captureRewrite((QueryOperator *) copyObject(aggOp));
			HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(auxOP), PROP_LEVEL_AGGREGATION_MARK), 0);

			// append to each map value a "prov attr pos in result"
			DEBUG_NODE_BEATIFY_LOG("BEFORE APPEND", psAttrAndLevel);
			FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
				int pos = listPosString(resultRel->schema, STRING_VALUE(c));
				List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
				l = appendToTailOfList(l, createConstInt(pos));
			}
			DEBUG_NODE_BEATIFY_LOG("AFTER APPEND", psAttrAndLevel);

			// for each min or max function call, build a heap for this attribute;
			// heap: gb-> frag
			//     :      list(value, frag);
			for (int i = 0; i < LIST_LENGTH(min_max); i++) {
				// heap for this min/max attr;
				HashMap *heap = NEW_MAP(Node, Node);

				FunctionCall *fc = (FunctionCall *) getNthOfListP(min_max, i);
				AttributeReference *attrRef = (AttributeReference *) getNthOfListP(fc->args, 0);

				char *heapType = NULL;
				if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
					heapType = MIN_HEAP;
				} else {
					heapType = MAX_HEAP;
				}

				// get function call attribute position in result;
				int fcAttrPos = listPosString(resultRel->schema, attrRef->name);
				INFO_LOG("fc attr: %d", fcAttrPos);

				// get group by attribute(s) position in result with the datatype;
				// pos + datatype
				List *gbPosList = NIL;
				FOREACH(AttributeReference, ar, aggrGBs) {
					int pos = listPosString(resultRel->schema, ar->name);
					gbPosList = appendToTailOfList(gbPosList, createConstInt(pos));
					gbPosList = appendToTailOfList(gbPosList, createConstInt(ar->attrType));
				}

				DEBUG_NODE_BEATIFY_LOG("list", gbPosList);

				for (int row = 0; row < LIST_LENGTH(resultRel->tuples); row++) {
					List *tuple = (List *) getNthOfListP(resultRel->tuples, row);

					// get group by
					StringInfo gbs = makeStringInfo();
					for (int idx = 0; idx < LIST_LENGTH(gbPosList); idx += 2) {
						appendStringInfo(gbs, "%s#", (char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(gbPosList, idx))));
					}
					INFO_LOG("gbs %s", gbs->data);
					// get group by map;
					// gbMap:
					//        "group by value" -> list(value, map(prov -> bits))
					//        "provs_bit"          -> map ( prov_name -> bits);
					HashMap *gbMap = NULL;
					HashMap *groupBitSetMap = NULL;
					if (MAP_HAS_STRING_KEY(heap, gbs->data)) {
						gbMap = (HashMap *) MAP_GET_STRING(heap, gbs->data);
						groupBitSetMap = (HashMap *) MAP_GET_STRING(gbMap, PROV_BIT_STRING);
					}

					// create heapNode;
					List *heapNode = NIL;

					// get attr value;
					char *val = (char *) getNthOfListP(tuple, fcAttrPos);
					Constant *value = makeValue(attrRef->attrType, val);

					heapNode = appendToTailOfList(heapNode, value);

					// get prov infos;
					// create prov value map for this "tuple";
					HashMap *provs_bit = NEW_MAP(Constant, Node);
					FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
						List * levelNumPos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));

						int level = INT_VALUE((Constant *) getNthOfListP(levelNumPos, 0));
						int numFrag = INT_VALUE((Constant *) getNthOfListP(levelNumPos, 1));
						int pos = INT_VALUE((Constant *) getNthOfListP(levelNumPos, 1));

						// create this tuple bitset;
						char *bits = (char *) getNthOfListP(tuple, pos);
						BitSet *bitSet = NULL;
						if (level < 2) { // ps is an integer;
							bitSet = newBitSet(numFrag);
							setBit(bitSet, atoi(bits), TRUE);
						} else { // bitset is a string;
							bitSet = stringToBitset(bits);
						}

						addToMap(provs_bit, (Node *) c, (Node *) bitSet);

						// bit or with this group's bits;
						BitSet *gBitSet = NULL;
						if (MAP_HAS_STRING_KEY(groupBitSetMap, STRING_VALUE(c))) {
							gBitSet = (BitSet *) MAP_GET_STRING(groupBitSetMap, STRING_VALUE(c));
							gBitSet = bitOr(gBitSet, bitSet);
						} else {
							gBitSet = (BitSet *) copyObject(bitSet);
							addToMap(groupBitSetMap, (Node *) c, (Node *) gBitSet);
						}
					}

					// append this row's prov info;
					heapNode = appendToTailOfList(heapNode, provs_bit);

					if (MAP_HAS_STRING_KEY(heap, gbs->data)) {
						List *heapList = (List *) MAP_GET_STRING(heap, gbs->data);
						heapInsert(heapList, heapType, (Node *) heapNode);
					} else {
						List *heapList = NIL;
						appendToTailOfList (heapList, heapNode);
						addToMap(heap, (Node *) createConstString(gbs->data), (Node *) heapNode);
					}
				}

				// add this heap to aggOp's state map;
				HashMap *opState = NULL;
				KeyValue *kv = createNodeKeyValue((Node *) createConstString(fc->functionname), (Node *) createConstString(attrRef->name));
				if (HAS_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE)) {
					opState = (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE);
					addToMap(opState, (Node *)kv, (Node *) heap);
				} else {
					opState = NEW_MAP(Node, Node);
					addToMap(opState, (Node *)kv, (Node *) heap);
					SET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE, opState);
				}
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
//			ProvenanceComputation * newPC = copyObject(pc);
//			newPC->op.inputs = singleton(newAgg);
//			newAgg->op.parents = singleton(newPC);
//			DEBUG_NODE_BEATIFY_LOG("NEW PC:", newPC);

			QueryOperator * rewriteOP = captureRewrite((QueryOperator *) newAgg);
			DEBUG_NODE_BEATIFY_LOG("avg_sum_count agg", rewriteOP);
			INFO_OP_LOG("rewrite op avg_sum_cont: ", rewriteOP);

//			char *sql = serializeOperatorModel((Node *) rewriteOP);
			char *sql = serializeQuery(rewriteOP);
			INFO_LOG("sql %s", sql);

			Relation *relation = NULL;
			relation = executeQuery(sql);

			// get the level of aggregation
			HashMap *aggLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewriteOP), PROP_LEVEL_AGGREGATION_MARK), 0);
			DEBUG_NODE_BEATIFY_LOG("LEVEL AND NUM FRAG", aggLevel);
		}

		DEBUG_NODE_BEATIFY_LOG("STATE", (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, DATA_STRUCTURE_STATE));
	}
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

//	DEBUG_NODE_BEATIFY_LOG("BEFORE REQEIRE", newPC);
//	INFO_OP_LOG("BEFORE REQEIRE", newPC);

	// rewrite query;
	result = rewritePI_CS(newPC);
//	DEBUG_NODE_BEATIFY_LOG("AFTER REQEIRE", newPC);
//	INFO_OP_LOG("AFTER REQEIRE", newPC);
	return result;
}

static void
updateByOperators(QueryOperator * op)
{
	switch(op->type)
	{
//		case T_DuplicateRemoval:
//			INFO_LOG("update duplicate remove");
//			updateDuplicateRemoval(op);
//			break;
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

	int pos = 0;
	FOREACH(AttributeDef, ad, attrDefs) {
		resultDC->attrNames = appendToTailOfList(resultDC->attrNames, (AttributeDef *) copyObject(ad));

		addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resultDC->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));

		int oldPos = INT_VALUE((Constant *) getMapString(dataChunk->attriToPos, ad->attrName));
		Vector *vector = (Vector *) getVecNode(dataChunk->tuples, oldPos);
		vecAppendNode(resultDC->tuples, (Node *) copyObject(vector));

		pos++;
	}


	// TODO: check if projExpr contains function call, operator,
	// mod(a, b): function;
	// +,-,*,/,||, : operators
//	List *projExprs = ((ProjectionOperator *) op)->projExprs;
//	boolean has_OP_FC = FALSE;
//	for (int i = 0; i < LIST_LENGTH(projExprs); i++)
//	{
//		Node *expr = getNthOfListP(projExprs, i);
//		if (isA(expr, Operator) || isA(expr, FunctionCall))
//		{
//			DEBUG_NODE_BEATIFY_LOG("fc or op:", expr);
//			has_OP_FC = TRUE;
////			Node * value = evaluateExpr(expr);
//		}
//	}

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



// TODO: join should use the capture to get tuple with their sketch;
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
//		executeQueryIgnoreResult(leftTableSQL); // TODO: this line has ERROR;
//		executeQueryWithoutResult(leftTableSQL);

		// insert values to LEFT_BRANCH_OF_JOIN;
		// create bulk insert for all data in datachunk;
		List * insertQuery = NIL;

		for (int row = 0; row < dataChunk->numTuples; row++) {
//			StringInfo insSQL = makeStringInfo();
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
//		replaceNode(childOp->inputs, OP_LCHILD(childOp), leftProjOp);
//		leftProjOp->op.parents = singleton(rewrOP);
//		DEBUG_NODE_BEATIFY_LOG("AFTER REPLACE", childOp);
//		INFO_OP_LOG("AFTER REPLACE", childOp);


		// replace lchild with new proj;
		replaceNode(childOp->inputs, OP_LCHILD(childOp), leftProjOp);
		leftProjOp->op.parents = singleton(childOp);
		DEBUG_NODE_BEATIFY_LOG("NEW JOIN", childOp);
//		//

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
				if(MAP_HAS_STRING_KEY(psAttrAndLevelMap, name)) {
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
//		executeQueryIgnoreResult(leftTableSQL); // TODO: this line has ERROR;
//		executeQueryWithoutResult(leftTableSQL);

		// insert values to RIGHT_BRANCH_OF_JOIN;
		// create bulk insert for all data in datachunk;
		List * insertQuery = NIL;

		for (int row = 0; row < dataChunk->numTuples; row++) {
//			StringInfo insSQL = makeStringInfo();
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
//		replaceNode(childOp->inputs, OP_LCHILD(childOp), rightProjOp);
//		rightProjOp->op.parents = singleton(rewrOP);
//		DEBUG_NODE_BEATIFY_LOG("AFTER REPLACE", childOp);
//		INFO_OP_LOG("AFTER REPLACE", childOp);


//		// replace lchild with new proj;
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

		if(branchWithDeltaCnt == 1) { // if > 1, it already build in left branch
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
				if(MAP_HAS_STRING_KEY(psAttrAndLevelMap, name)) { // fill prov bit set
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


	appendStringInfo(strInfo, "%s ", "UpdateAggregation");

	removeStringProperty(child, DATA_CHUNK_PROP);
}


static void
updateDuplicateRemoval(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
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

	if(strcmp(updatedTableName, tableName) != 0) {
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
//		DataType dataType = INT_VALUE((Constant*)getMapInt(dataChunk->posToDatatype, col));
		Constant *value = (Constant *) getNthOfListP(tupleRow, col);
		vecAppendNode((Vector *) getVecNode(dataChunk->tuples, col), (Node *) value);

		// set bit vector;
		if (col == psAttrCol) {
			BitSet *bitset = setFragmentToBitSet(INT_VALUE(value), attrInfo->rangeList);
			List *psList = NIL;
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, psName)) {
				psList = (List *) getMapString(dataChunk->fragmentsInfo, psName);
				psList = appendToTailOfList(psList, bitset);
//				addToMap(dataChunk->fragmentsInfo, createConstString(psName), (Node *) psList);
			} else {
				psList = singleton(bitset);
//				addToMap(dataChunk->fragmentsInfo, createConstStrign(psName), (Node *) psList);
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

//	DEBUG_NODE_BEATIFY_LOG("pos->dt: ", dataChunk->posToDatatype);
//	DEBUG_NODE_BEATIFY_LOG("att->pos: ", dataChunk->attriToPos);
	dataChunk->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunk->numTuples = LIST_LENGTH(relation->tuples);
	dataChunk->tupleFields = LIST_LENGTH(relation->schema);

	// fill tuples
	for(int col = 0; col < dataChunk->tupleFields; col++)
	{
		int dataType = INT_VALUE((Constant*)getMapInt(dataChunk->posToDatatype, col));
		INFO_LOG("data type: %d", dataType);
		Vector* colValues = makeVector(VECTOR_NODE, T_Vector);
		for(int row = 0; row < dataChunk->numTuples; row++)
		{
			Constant* value = makeValue(dataType, (char *) getNthOfListP((List *)getNthOfListP(relation->tuples, row), col));
			vecAppendNode(colValues, (Node*) value);
		}

		vecAppendNode(dataChunk->tuples, (Node*) colValues);
	}

	// fill identifier of update "-1"
	for(int row = 0; row < dataChunk->numTuples; row++)
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

//		check the ps attribute, get the value, and set fragment info;
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
	int choice = 1;
	char *sql = NULL;
	if (choice == 1) {
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
			if(row != dataChunk->numTuples - 1) {
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
//		List *values = NIL;
//		List *attrNames = (List *) getAttrDefNames(dataChunk->attrNames);
//		List *dataTypes = NIL;
//		List *updateType = NIL;
//
//		for (int col = 0; col < dataChunk->tupleFields; col++) {
//			List *colValues = NIL;
//
//			// get value from each vector cell;
//			Vector *vec = (Vector *) getVecNode(dataChunk->tuples, col);
//			for (int index = 0; index < VEC_LENGTH(vec); index++) {
//				colValues = appendToTailOfList(colValues, getVecNode(vec, index));
//			}
//
//			values = appendToTailOfList(values, colValues);
//			dataTypes = appendToTailOfListInt(dataTypes, INT_VALUE((Constant *) getMapInt(dataChunk->posToDatatype, col)));
//		}
//
//		// build list for sketch string;
//		FOREACH_HASH_KEY(Node, key, dataChunk->fragmentsInfo) {
//			List *bitsetList = NIL;
//			List *bitsets = (List *) getMapString(dataChunk->fragmentsInfo, STRING_VALUE((Constant *) key));
//			FOREACH(BitSet, bs, bitsets) {
//				bitsetList = appendToTailOfList(bitsetList, createConstString(bitSetToString(bs)));
//			}
//			values = appendToTailOfList(values, bitsetList);
////			attrNames = appendToTailOfList(attrNames, createAttributeDef(STRING_VALUE((Constant *) key), DT_STRING));
//			attrNames = appendToTailOfList(attrNames, STRING_VALUE((Constant *) key));
//			dataTypes = appendToTailOfListInt(dataTypes, DT_STRING);
//		}
//
//		// build list for update type;
//		for (int index = 0; index < VEC_LENGTH(dataChunk->updateIdentifier); index++) {
//			updateType = appendToTailOfList(updateType, createConstInt(getVecInt(dataChunk->updateIdentifier, index)));
//		}
//		values = appendToTailOfList(values, updateType);
//		attrNames = appendToTailOfList(attrNames, backendifyIdentifier("udpate_type"));
//		dataTypes = appendToTailOfListInt(dataTypes, DT_INT);
//
//		// create a constant real operator;
//		ConstRelOperator *constRelOp = createConstRelOp(values, NIL, deepCopyStringList(attrNames), dataTypes);
//		DEBUG_NODE_BEATIFY_LOG("CONST REL OP", constRelOp);
//
//		// create a selection operator;
//		SelectionOperator *selOp = createSelectionOp(condition, (QueryOperator *) constRelOp, NIL, attrNames);
//		constRelOp->op.parents = singleton(selOp);
//		DEBUG_NODE_BEATIFY_LOG("SEL OP", selOp);
//
//		// create a projection operator;
//		List *projExpr = NIL;
//		Schema *selSchema = selOp->op.schema;
//		for (int index = 0; index < LIST_LENGTH(selSchema->attrDefs); index++) {
//			AttributeDef *ad = (AttributeDef *) getNthOfListP(selSchema->attrDefs, index);
//			AttributeReference *af = createFullAttrReference(ad->attrName, 0, index, 0, ad->dataType);
//			projExpr = appendToTailOfList(projExpr, af);
//		}
//		ProjectionOperator *projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, getAttrNames(selOp->op.schema));
//		selOp->op.parents = singleton(projOp);
//		DEBUG_NODE_BEATIFY_LOG("PROJ", projOp);
//
//		// serialization query;
//		sql = serializeQuery((QueryOperator *) projOp);
//		INFO_LOG("SQL %s", sql);
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
//		attrNames = appendToTailOfList(attrNames, createAttributeDef(STRING_VALUE((Constant *) key), DT_STRING));
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
	} else if(getBackend() == BACKEND_ORACLE) {

	}

	return relation;
}

static void
executeQueryWithoutResult(char* sql)
{
	if (getBackend() == BACKEND_POSTGRES) {
		postgresExecuteStatement(sql);
	} else if(getBackend() == BACKEND_ORACLE) {

	}
}
