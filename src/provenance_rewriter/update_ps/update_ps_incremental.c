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
#include "provenance_rewriter/update_ps/update_ps_build_state.h"

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
// static void buildState(Node *node);
// static void buildStateAggregation(QueryOperator *op);
// static void buildStateDuplicateRemoval(QueryOperator *op);
// static void buildStateLimit(QueryOperator *op);
// static void buildStateFinal(QueryOperator *op);
// OrderOperator
static HashMap *getDataChunkFromUpdateStatement(QueryOperator* op, TableAccessOperator *tableAccessOp);
static void getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfDelete(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfUpdate(QueryOperator* updateOp, DataChunk* dataChunkInsert, DataChunk *dataChunkDelete, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static Relation *getQueryResult(char* sql);
// static Constant *makeValue(DataType dataType, char* value);
// static void executeQueryWithoutResult(char* sql);
static DataChunk *filterDataChunk(DataChunk* dataChunk, Node* condition);
static QueryOperator *captureRewrite(QueryOperator *operator);
// static int compareTwoValues(Constant *a, Constant *b, DataType dt);
// static void swapListCell(List *list, int pos1, int pos2);
static BitSet *setFragmentToBitSet(int value, List *rangeList);
// static ConstRelOperator *getConstRelOpFromDataChunk(DataChunk *dataChunk);
static ConstRelMultiListsOperator *createConstRelMultiListsFromDataChunk(DataChunk *dc, boolean isLeftBranch, List *parentList);
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
// static Vector *columnChunkToVector(ColumnChunk *cc);
// static BitSet *computeIsNullBitSet(Node *expr, DataChunk *dc);
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


/*
 *  TODO:
 *	datachunk isNull: in multiple place: a. TableAccess, state+update: Join, Aggregation, Update: evaluate operators functions.
 */

// dummy result
static StringInfo strInfo;
static QueryOperator* updateStatement = NULL;
static psInfo *PS_INFO = NULL;
static ProvenanceComputation *PC = NULL;
static HashMap *limitAttrPoss;
static List *limitOrderBys;
//static HashMap *coarseGrainedRangeList = NULL;

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

char *
update_ps_incremental(QueryOperator* operator, QueryOperator *updateStmt)
{
	// get partition information;
	PC = (ProvenanceComputation *) copyObject(operator);
	PS_INFO = createPSInfo((Node *) getStringProperty(operator, PROP_PC_COARSE_GRAINED));


	DEBUG_NODE_BEATIFY_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR: \n", operator);
	INFO_OP_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR", operator);
	strInfo = makeStringInfo();

	INFO_LOG("Start update");
	DEBUG_NODE_BEATIFY_LOG("update stmt", updateStmt);
	updateStatement = updateStmt;
	updateByOperators((QueryOperator*) OP_LCHILD(operator));
	return strInfo->data;
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

	QueryOperator *childOp = OP_LCHILD(op);

	if (!HAS_STRING_PROP(childOp, PROP_DATA_CHUNK)) {
		return;
	}
	INFO_LOG("NO PS TO UPDATE");
	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(childOp, PROP_DATA_CHUNK);

	// get delta ps list;
	HashMap *dcPsMaps = dataChunk->fragmentsInfo;

	PSMap *psMap = (PSMap *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);

	FOREACH_HASH_KEY(Constant, c, psMap->fragCnts) {
		List *bitsetList = (List *) MAP_GET_STRING(dcPsMaps, STRING_VALUE(c));
		BitSet *bitset = NULL;
		if (MAP_HAS_STRING_KEY(psMap->provSketchs, STRING_VALUE(c))) {
			bitset = (BitSet *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
		} else {
			bitset = newBitSet(((BitSet *) getNthOfListP(bitsetList, 0))->length);
		}

		// get frag cnt map;
		HashMap *fragCnt = NULL;
		if (MAP_HAS_STRING_KEY(psMap->fragCnts, STRING_VALUE(c))) {
			fragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCnts, STRING_VALUE(c));
		} else {
			fragCnt = NEW_MAP(Constant, Constant);
		}

		// for insert;
		for (int i = 0; i < LIST_LENGTH(bitsetList); i++) {
			int updateType = getVecInt(dataChunk->updateIdentifier, i);
			if (updateType == -1) {
				continue;
			}

			BitSet *currBitset = (BitSet *) getNthOfListP(bitsetList, i);
			for (int bitIndex = 0; bitIndex < currBitset->length; bitIndex++) {
				if (isBitSet(currBitset, bitIndex)) {
					if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
						int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
						addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt + 1));
					} else {
						addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
					}
					setBit(bitset, bitIndex, TRUE);
				}
			}
		}

		// for delete;
		for (int i = 0; i < LIST_LENGTH(bitsetList); i++) {
			int updateType = getVecInt(dataChunk->updateIdentifier, i);
			if (updateType == 1) {
				continue;
			}

			BitSet *currBitset = (BitSet *) getNthOfListP(bitsetList, i);
			for (int bitIndex = 0; bitIndex < currBitset->length; bitIndex++) {
				if (isBitSet(currBitset, bitIndex)) {
					int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
					if (oriCnt <= 1) {
						removeMapElem(fragCnt, (Node *) createConstInt(bitIndex));
						setBit(bitset, bitIndex, FALSE);
					} else {
						addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt - 1));
					}
				}
			}
		}

		addToMap(psMap->fragCnts, (Node *) copyObject(c), (Node *) fragCnt);
		addToMap(psMap->provSketchs, (Node *) copyObject(c), (Node *) bitset);
	}

	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, (Node *) psMap);
	removeStringProperty(childOp, PROP_DATA_CHUNK);
}

static void
updateProjection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT PROJECTION OPERATOR", op);
	INFO_OP_LOG("CURRENT OPERATOR", op);
	QueryOperator *child = OP_LCHILD(op);

	if (!HAS_STRING_PROP(child, PROP_DATA_CHUNK)) {
		return;
	}


	HashMap *chunkMaps = (HashMap *) getStringProperty(child, PROP_DATA_CHUNK);
	DataChunk *dataChunkIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
	DataChunk *resultDCIns = NULL;
	if (dataChunkIns != NULL) {
		resultDCIns = initDataChunk();
		resultDCIns->attrNames = (List *) copyObject(op->schema->attrDefs);
		resultDCIns->numTuples = dataChunkIns->numTuples;
		resultDCIns->tupleFields = LIST_LENGTH(op->schema->attrDefs);
		resultDCIns->fragmentsInfo = (HashMap *) copyObject(dataChunkIns->fragmentsInfo);
		resultDCIns->updateIdentifier = (Vector *) copyObject(dataChunkIns->updateIdentifier);
		int pos = 0;
		FOREACH(AttributeDef, ad, op->schema->attrDefs) {
			addToMap(resultDCIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
			addToMap(resultDCIns->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
			pos++;
		}
	}

	DataChunk *dataChunkDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
	DataChunk *resultDCDel = NULL;
	if (dataChunkDel != NULL) {
		resultDCDel = initDataChunk();
		resultDCDel->attrNames = (List *) copyObject(op->schema->attrDefs);
		resultDCDel->numTuples = dataChunkDel->numTuples;
		resultDCDel->tupleFields = LIST_LENGTH(op->schema->attrDefs);
		resultDCDel->fragmentsInfo = dataChunkDel->fragmentsInfo;
		resultDCDel->updateIdentifier = dataChunkDel->updateIdentifier;
		int pos = 0;
		FOREACH(AttributeDef, ad, op->schema->attrDefs) {
			addToMap(resultDCDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
			addToMap(resultDCDel->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
			pos++;
		}
	}

	// List *attrDefs = op->schema->attrDefs;
	List *projExprs = ((ProjectionOperator *) op)->projExprs;

	// int pos = 0;
	FOREACH(Node, node, projExprs) {
		// resultDC->attrNames = appendToTailOfList(resultDC->attrNames, copyObject(ad));

		if (isA(node, Operator)) { // node is an expression, evaluate it;
			// calculate value of this operator;
			// ColumnChunk *evaluatedValue = (ColumnChunk *) evaluateExprOnDataChunk(node, dataChunk);

			// Vector *vector = columnChunkToVector(evaluatedValue);
			// vec
			// vecAppendNode(resultDC->tuples, (Node *) copyObject(vector));

			// compute isNull;
			// BitSet *bs = NULL;
			// bs = computeIsNullBitSet(node, dataChunk);
			// vecAppendNode(resultDC->isNull, (Node *) copyObject(bs));

		} else if (isA(node, AttributeReference)){ // node is an attribute, direct copy;
			// get value of this attribute reference;
			AttributeReference *ar = (AttributeReference *) node;
			int vecFromPos = ar->attrPosition;
			if (dataChunkIns != NULL) {
				vecAppendNode(resultDCIns->tuples, (Node *) getVecNode(dataChunkIns->tuples, vecFromPos));
			}
			if (dataChunkDel != NULL) {
				vecAppendNode(resultDCDel->tuples, (Node *) getVecNode(dataChunkDel->tuples, vecFromPos));
			}
		}
		// pos++;
	}

	// create chunk maps;
	HashMap *resChunkMaps = NEW_MAP(Constant, Node);
	// add ins chunk;
	if (resultDCIns != NULL) {
		addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) resultDCIns);
	}
	// add del chunk;
	if (resultDCDel != NULL) {
		addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) resultDCDel);
	}

	if (mapSize(resChunkMaps) > 0) {
		setStringProperty(op, PROP_DATA_CHUNK, (Node*) resChunkMaps);
	}

	// remove child data chunk;
	removeStringProperty(child, PROP_DATA_CHUNK);
}

static void
updateSelection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	// check if child operator has delta tuples;
	// TODO: another property to identify if it is only capture delta data;
	if (!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		return;
	}

	appendStringInfo(strInfo, "%s ", "UpdateSelection");
	HashMap *chunkMaps = (HashMap *) GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK);

	Node * selCond = ((SelectionOperator *) op)->cond;
	HashMap *resChunkMaps = NEW_MAP(Constant, Node);
	DataChunk *dataChunkIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
	if (dataChunkIns != NULL) {
		DataChunk *dcInsFilter = filterDataChunk(dataChunkIns, selCond);
		INFO_NODE_LOG("DDDD", dcInsFilter);

		if (dcInsFilter->numTuples > 0) {
			addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) dcInsFilter);
		}
	}
	DataChunk *dataChunkDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
	if (dataChunkDel != NULL) {
		DataChunk *dcDelFilter = filterDataChunk(dataChunkDel, selCond);
		if (dcDelFilter->numTuples > 0) {
			addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) dcDelFilter);
		}
	}
	if (mapSize(resChunkMaps) > 0) {
		setStringProperty(op, PROP_DATA_CHUNK, (Node *) resChunkMaps);
	}
	removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
}

static void
updateJoin(QueryOperator * op)
{
	// update two child operators;
	updateByOperators(OP_LCHILD(op));
    updateByOperators(OP_RCHILD(op));

	// both children operators don't contain delta tuples;
	if ((!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK))
	 && (!HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK))) {
		return;
	}

	QueryOperator *rewrOp = captureRewriteOp((ProvenanceComputation *) copyObject(PC), (QueryOperator *) copyObject(op));
	INFO_OP_LOG("rewr for join", rewrOp);

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOp), PROP_LEVEL_AGGREGATION_MARK), 0);
	DEBUG_NODE_BEATIFY_LOG("WHAT IS HASH MAP", psAttrAndLevel);
	ConstRelMultiListsOperator *leftDelta = NULL;
	int leftPSAttrCnt = 0;
	ConstRelMultiListsOperator *rightDelta = NULL;
	int rightPSAttrCnt = 0;

	DataChunk *resultDC = NULL;
	int deltaBranches = 0;
	// left branch is a delta inputs;
	if (HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		deltaBranches++;
		HashMap *chunkMaps = (HashMap *) getStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
		DataChunk *leftDCIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *leftDCDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
		Vector *chunks = makeVector(VECTOR_NODE, T_Vector);
		if (leftDCIns != NULL) {
			vecAppendNode(chunks, (Node *) leftDCIns);
		}

		if (leftDCDel != NULL) {
			vecAppendNode(chunks, (Node *) leftDCDel);
		}

		// leftDelta = createConstRelMultiListsFromDataChunk(chunks, TRUE, OP_LCHILD(cOp));

//j
		DataChunk *leftDataChunk = (DataChunk *) getStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
		leftPSAttrCnt = mapSize(leftDataChunk->fragmentsInfo);
		QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);
		leftDelta = createConstRelMultiListsFromDataChunk(leftDataChunk, TRUE, singleton(OP_LCHILD(cOp)));
//


		List *attrDefs = NIL;
		List *projExpr = NIL;

		int pos = 0;
		FOREACH(AttributeDef, ad, ((QueryOperator *) leftDelta)->schema->attrDefs) {
			attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 0, pos++, 0, ad->dataType));
		}

		List *defs = OP_LCHILD(cOp)->schema->attrDefs;
		for (int i = LIST_LENGTH(((QueryOperator *) leftDelta)->schema->attrDefs) - 1; i < LIST_LENGTH(defs); i++) {
			AttributeDef *ad = (AttributeDef *) getNthOfListP(defs, i);
			attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 1, pos++, 0, ad->dataType));
		}

		OP_LCHILD(cOp)->schema->attrDefs = attrDefs;
		cOp->schema->attrDefs = attrDefs;
		((ProjectionOperator *) cOp)->projExprs = projExpr;

		replaceNode(OP_LCHILD(cOp)->inputs, OP_LCHILD(OP_LCHILD(cOp)), leftDelta);

		DEBUG_NODE_BEATIFY_LOG(" create left branch ", leftDelta);

		DEBUG_NODE_BEATIFY_LOG("rewr join", cOp);
		INFO_OP_LOG("rewr join", cOp);

		char *sql = serializeQuery(cOp);
		INFO_LOG("SQL is : %s", sql);

		Relation *relation = executeQuery(sql);
		INFO_LOG("length: %d", LIST_LENGTH(relation->tuples));

		// if (LIST_LENGTH(relation->tuples) > 0) {
		// 	resultDC = initDataChunk();
		// 	int pos = 0;
		// 	FOREACH(AttributeDef, ad, rewrOp->schema->attrDefs) {
		// 		// get non ps attribute
		// 		if (!MAP_HAS_STRING_KEY(psAttrAndLevel, ad->attrName)) {
		// 			resultDC->attrNames = appendToTailOfList(resultDC->attrNames, copyObject(ad));
		// 			addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		// 			addToMap(resultDC->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
		// 		} else {
		// 			Vector *psVec = makeVector(VECTOR_NODE, T_Vector);
		// 			addToMap(resultDC->fragmentsInfo, (Node *) createConstString(ad->attrName), (Node *) psVec);
		// 		}
		// 		pos++;
		// 	}
		// }












		// createConstRelMultiListsOp();
	}

	// right branch is a delta inputs;
	if (HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		deltaBranches++;
		DataChunk *rightDataChunk = (DataChunk *) getStringProperty(OP_RCHILD(op), PROP_DATA_CHUNK);
		rightPSAttrCnt = mapSize(rightDataChunk->fragmentsInfo);
		QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);
		rightDelta = createConstRelMultiListsFromDataChunk(rightDataChunk, FALSE, singleton(OP_LCHILD(cOp)));

		OP_LCHILD(cOp)->schema->attrDefs = appendToTailOfList(OP_LCHILD(cOp)->schema->attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));
		((ProjectionOperator *) cOp)->projExprs = appendToTailOfList(((ProjectionOperator *) cOp)->projExprs, createFullAttrReference(JOIN_RIGHT_BRANCH_IDENTIFIER, 1, LIST_LENGTH(OP_LCHILD(cOp)->schema->attrDefs) - 1,0, DT_INT));
		cOp->schema->attrDefs = appendToTailOfList(cOp->schema->attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));


		replaceNode(OP_LCHILD(cOp)->inputs, OP_RCHILD(OP_LCHILD(cOp)), rightDelta);

		INFO_OP_LOG("rewr JJJ", cOp);
		char *sql = serializeQuery(cOp);
		INFO_LOG("WAHT IS SQL", sql);
		Relation *relation = executeQuery(sql);
		INFO_LOG("schema %s", stringListToConstList(relation->schema));

	}

	if (deltaBranches == 2) {
		INFO_OP_LOG("CONST_REL_LEFT", leftDelta);
		INFO_OP_LOG("CONST_REL_right", rightDelta);

		QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);

		List *attrDefs = NIL;
		List *projExpr = NIL;

		QueryOperator *joinOp = OP_LCHILD(cOp);
		int pos = 0;
		List *leftAttrDefs = OP_LCHILD(joinOp)->schema->attrDefs;

		for (int i = 0; i < LIST_LENGTH(joinOp->schema->attrDefs); i++) {
			AttributeDef *ad = getNthOfListP(joinOp->schema->attrDefs, i);
			attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 0, pos++, 0, ad->dataType));

			if (i >= LIST_LENGTH(leftAttrDefs) - 1) {
				break;
			}
		}
		attrDefs = appendToTailOfList(attrDefs, createAttributeDef(JOIN_LEFT_BRANCH_IDENTIFIER, DT_INT));
		projExpr = appendToTailOfList(projExpr, createFullAttrReference(JOIN_LEFT_BRANCH_IDENTIFIER, 0, pos++, 0, DT_INT));

		for (int i = LIST_LENGTH(leftAttrDefs); i < LIST_LENGTH(joinOp->schema->attrDefs); i++) {
			AttributeDef *ad = getNthOfListP(joinOp->schema->attrDefs, i);
			attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 1, pos++, 0, ad->dataType));
		}
		attrDefs = appendToTailOfList(attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));
		projExpr = appendToTailOfList(projExpr, createFullAttrReference(JOIN_RIGHT_BRANCH_IDENTIFIER, 1, pos++, 0, DT_INT));

		// modify join op;
		OP_LCHILD(cOp)->schema->attrDefs = attrDefs;

		// modify proj op;
		((ProjectionOperator *) cOp)->projExprs = projExpr;
		cOp->schema->attrDefs = attrDefs;

		replaceNode(OP_LCHILD(cOp)->inputs, OP_LCHILD(OP_LCHILD(cOp)), leftDelta);
		replaceNode(OP_LCHILD(cOp)->inputs, OP_RCHILD(OP_LCHILD(cOp)), rightDelta);

		leftDelta->op.parents = singleton(OP_LCHILD(cOp));
		rightDelta->op.parents = singleton(OP_LCHILD(cOp));


		INFO_OP_LOG("rewr Both", cOp);
		char *sql = serializeQuery(cOp);
		INFO_LOG("WAHT IS SQL", sql);
		Relation *relation = executeQuery(sql);
		INFO_LOG("schema %s", stringListToConstList(relation->schema));
	}

	if (!resultDC) {
		setStringProperty(op, PROP_DATA_CHUNK, (Node*) resultDC);
	}

	// remove children's data chunk;
	if (HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
	}
	if (HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		removeStringProperty(OP_RCHILD(op), PROP_DATA_CHUNK);
	}
}

static ConstRelMultiListsOperator *
createConstRelMultiListsFromDataChunk(DataChunk *chunks, boolean isLeftBranch, List *parentList)
{
	// build attr names and datatypes;
	if (((Vector *)chunks)->length == 0) {
		return NULL;
	}

	List *attrNames = NIL;
	List *attrTypes = NIL;
	List *values = NIL;
	boolean hasBuildInfo = FALSE;
	FOREACH_VEC(DataChunk, dc, chunks) {
		if (!hasBuildInfo) {
			attrNames = getAttrDefNames(dc->attrNames);
			attrTypes = getAttrDataTypes(dc->attrNames);
			hasBuildInfo = TRUE;
		}

		for (int col = 0; col < dc->tupleFields; col++) {
			DataType colType = INT_VALUE((Constant *) MAP_GET_INT(dc->posToDatatype, col));
			Vector *colVec = (Vector *) getVecNode(dc->tuples, col);
			for (int row = 0; row < dc->numTuples; row++) {
				Constant *val = NULL;
				switch (colType) {
					case DT_INT:
						val = createConstInt(getVecInt(colVec, row));
						break;
					case DT_LONG:
						val = createConstLong(getVecLong(colVec, row));
						break;
					case DT_FLOAT:
						val = createConstFloat(getVecFloat(colVec, row));
						break;
					case DT_STRING:
					case DT_VARCHAR2:
						val = createConstString(getVecString(colVec, row));
						break;
					case DT_BOOL:
						val = createConstBool(getVecInt(colVec, row) == 1 ? TRUE : FALSE);
						break;
					default:
						FATAL_LOG("data type is not supported");
				}
				if (col == 0) {
					List *l = singleton(val);
					values = appendToTailOfList(values, l);
				} else {
					List *l = (List *) getNthOfListP(values, row);
					l = appendToTailOfList(l, val);
				}
			}
		}
	}
	// List *attrNames = getAttrDefNames(dc->attrNames);
	// List *attrTypes = getAttrDataTypes(dc->attrNames);
	// List *values = NIL;

	// for (int col = 0; col < dc->tupleFields; col++) {
	// 	DataType colType = INT_VALUE((Constant *) MAP_GET_INT(dc->posToDatatype, col));
	// 	Vector *colVec = (Vector *) getVecNode(dc->tuples, col);
	// 	for (int row = 0; row < dc->numTuples; row++) {
	// 		Constant *val = NULL;
	// 		switch (colType) {
	// 			case DT_INT:
	// 				val = createConstInt(getVecInt(colVec, row));
	// 				break;
	// 			case DT_LONG:
	// 				val = createConstLong(getVecLong(colVec, row));
	// 				break;
	// 			case DT_FLOAT:
	// 				val = createConstFloat(getVecFloat(colVec, row));
	// 				break;
	// 			case DT_STRING:
	// 			case DT_VARCHAR2:
	// 				val = createConstString(getVecString(colVec, row));
	// 				break;
	// 			case DT_BOOL:
	// 				val = createConstBool(getVecInt(colVec, row) == 1 ? TRUE : FALSE);
	// 				break;
	// 			default:
	// 				FATAL_LOG("data type is not supported");
	// 		}
	// 		if (col == 0) {
	// 			List *l = singleton(val);
	// 			values = appendToTailOfList(values, l);
	// 		} else {
	// 			List *l = (List *) getNthOfListP(values, row);
	// 			l = appendToTailOfList(l, val);
	// 		}
	// 	}
	// }

	// // deal with ps;
	// FOREACH_HASH_KEY(Constant, c, dc->fragmentsInfo) {
	// 	attrNames = appendToTailOfList(attrNames, STRING_VALUE(c));
	//     attrTypes = appendToTailOfListInt(attrTypes, DT_STRING);

	// 	Vector *colVec = (Vector *) MAP_GET_STRING(dc->fragmentsInfo, STRING_VALUE(c));

	// 	for (int row = 0; row < colVec->length; row++) {
	// 		List *l = (List *) getNthOfListP(values, row);
	// 		l = appendToTailOfList(l, createConstString(bitSetToString((BitSet *) getVecNode(colVec, row))));
	// 	}
	// }

	// // add update identifier to attr name and datatypes;
	// if (isLeftBranch) {
	// 	attrNames = appendToTailOfList(attrNames, JOIN_LEFT_BRANCH_IDENTIFIER);
	// } else {
	// 	attrNames = appendToTailOfList(attrNames, JOIN_RIGHT_BRANCH_IDENTIFIER);
	// }
	// attrTypes = appendToTailOfListInt(attrTypes, DT_INT);

	// int *updIdenArr = VEC_TO_IA(dc->updateIdentifier);
	// for (int row = 0; row < dc->numTuples; row++) {
	// 	List *l = (List *) getNthOfListP(values, row);
	// 	l = appendToTailOfList(l, createConstInt(updIdenArr[row]));
	// }

	ConstRelMultiListsOperator *co = createConstRelMultiListsOp(values, parentList, attrNames, attrTypes);
	return co;
}


static void
updateAggregation(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));

	// check if child has data chunk;
	if (!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		return;
	}

	// get input data chunk;
	HashMap *chunkMaps = (HashMap *) GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK);

	//init return data chunk; here we must init ins and del chunks.
	// res ins chunk;
	DataChunk *resultDCInsert = initDataChunk();
	resultDCInsert->attrNames = (List *) copyObject(op->schema->attrDefs);
	resultDCInsert->tupleFields = LIST_LENGTH(op->schema->attrDefs);
	// res del chunk;
	DataChunk *resultDCDelete = initDataChunk();
	resultDCDelete->attrNames = (List *) copyObject(op->schema->attrDefs);
	resultDCDelete->tupleFields = LIST_LENGTH(op->schema->attrDefs);
	// init tuples vectors;
	int attrPos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		addToMap(resultDCInsert->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrPos));
		addToMap(resultDCInsert->posToDatatype, (Node *) createConstInt(attrPos), (Node *) createConstInt(ad->dataType));

		addToMap(resultDCDelete->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrPos));
		addToMap(resultDCDelete->posToDatatype, (Node *) createConstInt(attrPos), (Node *) createConstInt(ad->dataType));
		// init this attr col vector;
		switch (ad->dataType) {
			case DT_INT:
				vecAppendNode(resultDCInsert->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resultDCDelete->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				break;
			case DT_LONG:
				vecAppendNode(resultDCInsert->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(resultDCDelete->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				break;
			case DT_FLOAT:
				vecAppendNode(resultDCInsert->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(resultDCDelete->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				break;
			case DT_BOOL:
				vecAppendNode(resultDCInsert->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resultDCDelete->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				break;
			case DT_STRING:
			case DT_VARCHAR2:
				vecAppendNode(resultDCInsert->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(resultDCDelete->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				break;
			default:
				FATAL_LOG("data type is not supported");
		}
		attrPos++;
	}

	// get gb list and aggr list;
	List *aggGBList = ((AggregationOperator *) op)->groupBy;
	List *aggFCList = ((AggregationOperator *) op)->aggrs;

	// match function call name and gb name to output schema's name
	// This is because, for avg(b) as avg_b gprom will write avg(b) to AGGR_0, in functiona call it is avg(b) but schema is AGGR_0
	// And in upper level, they will use AGGR_0 to output avg_b;
	HashMap *mapFCsToSchemas = NEW_MAP(Constant, Constant);
	attrPos = 0;
	FOREACH(FunctionCall, fc, aggFCList) {
		AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
		Constant *nameInFC = createConstString(CONCAT_STRINGS(strdup(fc->functionname), "_", strdup(ar->name)));
		Constant *nameInSchema = createConstString(((AttributeDef *) getNthOfListP(resultDCInsert->attrNames, attrPos))->attrName);
		addToMap(mapFCsToSchemas, (Node *) nameInFC, (Node *) nameInSchema);
		attrPos++;
	}
	FOREACH(AttributeReference, ar, aggGBList) {
		Constant *nameInFC = createConstString(ar->name);
		Constant *nameInSchema = createConstString(((AttributeDef *) getNthOfListP(resultDCInsert->attrNames, attrPos))->attrName);
		addToMap(mapFCsToSchemas, (Node *) nameInFC, (Node *) nameInSchema);
		attrPos++;
	}

	DEBUG_NODE_BEATIFY_LOG("name maps:", mapFCsToSchemas);
	// get all gb values;
	int gbAttrCnt = LIST_LENGTH(aggGBList);
	Vector *gbPoss = makeVectorOfSize(VECTOR_INT, T_Vector, gbAttrCnt);
	// gbPoss->length = gbAttrCnt;
	Vector *gbType = makeVectorOfSize(VECTOR_INT, T_Vector, gbAttrCnt);
	// gbType->length = gbAttrCnt;
	Vector *gbName = makeVectorOfSize(VECTOR_STRING, T_Vector, gbAttrCnt);
	// gbName->length = gbAttrCnt;

	// identifier to show whether build the pos, type and names of groub by attributes;
	boolean hasBuildGBAttrsPosTypeVec = FALSE;

	Vector *gbValsInsert = NULL;
	// Vector *gbRealValuesInsert = NULL;
	DataChunk *dataChunkInsert = NULL;
	if (MAP_HAS_STRING_KEY(chunkMaps, PROP_DATA_CHUNK_INSERT)) {
		dataChunkInsert = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
		gbValsInsert = makeVectorOfSize(VECTOR_STRING, T_Vector, dataChunkInsert->numTuples);
		gbValsInsert->length = dataChunkInsert->numTuples;

		// build gb pos and type only once;
		FOREACH(AttributeReference, ar, aggGBList) {
			int pos = INT_VALUE(MAP_GET_STRING(dataChunkInsert->attriToPos, ar->name));
			DataType type = INT_VALUE(MAP_GET_INT(dataChunkInsert->posToDatatype, pos));
			vecAppendInt(gbPoss, pos);
			vecAppendInt(gbType, type);
			vecAppendString(gbName, strdup(ar->name));
		}
		hasBuildGBAttrsPosTypeVec = TRUE;

		char ** gbValsArr = (char **) VEC_TO_ARR(gbValsInsert, char);
		for (int gbIndex = 0; gbIndex < gbAttrCnt; gbIndex++) {
			DataType type = getVecInt(gbType, gbIndex);
			int pos = getVecInt(gbPoss, gbIndex);
			switch (type) {
				case DT_INT:
				{
					int *valArr = VEC_TO_IA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#");
					}
				}
					break;
				case DT_LONG:
				{
					gprom_long_t *valArr = VEC_TO_LA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_ltoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ltoa(valArr[row]), "#");
					}
				}
					break;
				case DT_FLOAT:
				{
					double *valArr = VEC_TO_FA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_ftoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ftoa(valArr[row]), "#");
					}
				}
					break;
				case DT_BOOL:
				{
					int *valArr = VEC_TO_IA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#");
					}
				}
					break;
				case DT_STRING:
				case DT_VARCHAR2:
				{
					char **valArr = (char **) VEC_TO_ARR((Vector *) getVecNode(dataChunkInsert->tuples, pos), char);
					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(valArr[row], "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], valArr[row], "#");
					}

				}
					break;
				default:
					FATAL_LOG("data type is not supproted");
			}
		}
		ASSERT(gbValsInsert->length == dataChunkInsert->numTuples);
	}

	Vector *gbValsDelete = NULL;
	DataChunk *dataChunkDelete = NULL;
	if (MAP_HAS_STRING_KEY(chunkMaps, PROP_DATA_CHUNK_DELETE)) {
		dataChunkDelete = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
		gbValsDelete = makeVectorOfSize(VECTOR_STRING, T_Vector, dataChunkDelete->numTuples);
		gbValsDelete->length = dataChunkDelete->numTuples;

		if (!hasBuildGBAttrsPosTypeVec) {
			FOREACH(AttributeReference, ar, aggGBList) {
				int pos = INT_VALUE(MAP_GET_STRING(dataChunkDelete->attriToPos, ar->name));
				DataType type = INT_VALUE(MAP_GET_INT(dataChunkDelete->posToDatatype, pos));
				vecAppendInt(gbPoss, pos);
				vecAppendInt(gbType, type);
				vecAppendString(gbName, strdup(ar->name));
			}
			hasBuildGBAttrsPosTypeVec = TRUE;
		}

		char ** gbValsArr = (char **) VEC_TO_ARR(gbValsDelete, char);
		for (int gbIndex = 0; gbIndex < gbAttrCnt; gbIndex++) {
			DataType type = getVecInt(gbType, gbIndex);
			int pos = getVecInt(gbPoss, gbIndex);
			switch (type) {
				case DT_INT:
				{
					int *valArr = VEC_TO_IA((Vector *) getVecNode(dataChunkDelete->tuples, pos));
					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
						if (gbIndex == 0)
						{ gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#"); }
						else
						{ gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#"); }
					}
				}
					break;
				case DT_LONG:
				{
					gprom_long_t *valArr = VEC_TO_LA(getVecNode(dataChunkDelete->tuples, pos));
					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_ltoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ltoa(valArr[row]), "#");
					}
				}
					break;
				case DT_FLOAT:
				{
					double *valArr = VEC_TO_FA(getVecNode(dataChunkDelete->tuples, pos));
					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_ftoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ftoa(valArr[row]), "#");
					}
				}
					break;
				case DT_BOOL:
				{
					int *valArr = VEC_TO_IA(getVecNode(dataChunkDelete->tuples, pos));
					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#");
					}
				}
					break;
				case DT_STRING:
				case DT_VARCHAR2:
				{
					char **valArr = (char **) VEC_TO_ARR((Vector *) getVecNode(dataChunkDelete->tuples, pos), char);
					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
						if (gbIndex == 0)
							gbValsArr[row] = CONCAT_STRINGS(valArr[row], "#");
						else
							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], valArr[row], "#");
					}

				}
					break;
				default:
					FATAL_LOG("data type is not supproted");
			}
		}
		ASSERT(gbValsDelete->length == dataChunkDelete->numTuples);
	}
	DEBUG_NODE_BEATIFY_LOG("insert gbs", gbValsInsert);
	DEBUG_NODE_BEATIFY_LOG("delete gbs", gbValsDelete);

	// fetch stored pre-built data structures for all aggs;
	HashMap *dataStructures = (HashMap *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);

	boolean hasFinishPS = FALSE;
	boolean hasFinishGB = FALSE;

	// four vectors to indicate the pos value of each update tuple which is used later to add gb attr values in result chunks;
	Vector *resDCGBsInsFromIns = makeVector(VECTOR_INT, T_Vector);
	Vector *resDCGBsDelFromIns = makeVector(VECTOR_INT, T_Vector);
	Vector *resDCGBsInsFromDel = makeVector(VECTOR_INT, T_Vector);
	Vector *resDCGBsDelFromDel = makeVector(VECTOR_INT, T_Vector);


	FOREACH(FunctionCall, fc, aggFCList) {
		AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
		char *nameInDS = CONCAT_STRINGS(fc->functionname, "_", ar->name);
		if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0
		|| strcmp(fc->functionname, SUM_FUNC_NAME) == 0
		|| strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
			// get stored data structure for this aggregation function call;
			GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructures, nameInDS);

			if (dataChunkInsert != NULL) {
				// get input vector infos;
				int inputVecPos = INT_VALUE(MAP_GET_STRING(dataChunkInsert->attriToPos, ar->name));
				DataType inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, inputVecPos));
				// get name in output schema from map;
				Constant *nameInOutChunk = (Constant *) MAP_GET_STRING(mapFCsToSchemas, nameInDS);
				// get output vector infos;
				int outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCInsert->attriToPos, STRING_VALUE(nameInOutChunk)));
				DataType outputVecType = INT_VALUE((Constant *) MAP_GET_INT(resultDCInsert->posToDatatype, outputVecPos));

				// fetch input vector;
				Vector *inputVec = (Vector *) getVecNode(dataChunkInsert->tuples, inputVecPos);
				DEBUG_NODE_BEATIFY_LOG("input agg vec", inputVec);
				// fetch output vectors: both for delete(old values) and insert(newly updated values);
				Vector *outputVecInsert = (Vector *) getVecNode(resultDCInsert->tuples, outputVecPos);
				Vector *outputVecDelete = (Vector *) getVecNode(resultDCDelete->tuples, outputVecPos);

				// get an char* array of group by values;
				char **gbValArr = (char **) VEC_TO_ARR(gbValsInsert, char);

				for (int row = 0; row < gbValsInsert->length; row++) {
					int resUpdType = 0;
					char *gbVal = gbValArr[row];
					if (MAP_HAS_STRING_KEY(acs->map, gbVal)) {
						resUpdType = 0;
						List *oldL = (List *) MAP_GET_STRING(acs->map, gbVal);
						List *newL = NIL;
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							double avg = FLOAT_VALUE((Constant *) getNthOfListP(oldL, 0));
							double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldL, 1));
							gprom_long_t cnt = LONG_VALUE((Constant *) getNthOfListP(oldL, 2));

							vecAppendFloat(outputVecDelete, avg);
							switch (inputVecType) {
								case DT_INT:
									sum += getVecInt(inputVec, row);
									break;
								case DT_FLOAT:
									sum += getVecFloat(inputVec, row);
									break;
								case DT_LONG:
									sum += getVecLong(inputVec, row);
									break;
								default:
									FATAL_LOG("not supported");
							}
							cnt += 1;
							avg = sum / cnt;
							vecAppendFloat(outputVecInsert, avg);
							newL = appendToTailOfList(newL, createConstFloat(avg));
							newL = appendToTailOfList(newL, createConstFloat(sum));
							newL = appendToTailOfList(newL, createConstLong(cnt));
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldL, 0));
							gprom_long_t cnt = LONG_VALUE((Constant *) getNthOfListP(oldL, 1));
							double newSum = (double) 0;
							switch (inputVecType) {
								case DT_INT:
									newSum = sum + getVecInt(inputVec, row);
									break;
								case DT_FLOAT:
									newSum = sum + getVecFloat(inputVec, row);
									break;
								case DT_LONG:
									newSum = sum + getVecLong(inputVec, row);
									break;
								default:
									FATAL_LOG("not supported");
							}
							switch (outputVecType) {
								case DT_INT:
								{
									vecAppendInt(outputVecDelete, (int) sum);
									vecAppendInt(outputVecInsert, (int) newSum);
								}
									break;
								case DT_LONG:
								{
									vecAppendLong(outputVecDelete, (gprom_long_t) sum);
									vecAppendLong(outputVecInsert, (gprom_long_t) newSum);
								}
									break;
								case DT_FLOAT:
								{
									vecAppendFloat(outputVecDelete, sum);
									vecAppendFloat(outputVecInsert, newSum);
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
							newL = appendToTailOfList(newL, createConstFloat(newSum));
							newL = appendToTailOfList(newL, createConstLong(cnt + 1));
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							gprom_long_t cnt = LONG_VALUE((Constant *) getNthOfListP(oldL, 0));
							switch (outputVecType) {
								case DT_INT:
									vecAppendInt(outputVecDelete, (int) cnt);
									vecAppendInt(outputVecInsert, (int) (cnt + 1));
									break;
								case DT_LONG:
									vecAppendLong(outputVecDelete, cnt);
									vecAppendLong(outputVecInsert, cnt + 1);
									break;
								case DT_FLOAT:
									vecAppendFloat(outputVecDelete, (double) cnt);
									vecAppendFloat(outputVecInsert, (double) (cnt + 1));
								default:
									FATAL_LOG("not supported");
							}
							newL = appendToTailOfList(newL, createConstLong(cnt + 1));
						}
						addToMap(acs->map, (Node *) createConstString(gbVal), (Node *) newL);
						DEBUG_NODE_BEATIFY_LOG("oldList", oldL);
						DEBUG_NODE_BEATIFY_LOG("oldList", newL);
					} else {
						resUpdType = 1;
						// no previous group; insert and new group;
						List *newL = NIL;
						double sum_avg = (double) 0;
						switch (inputVecType) {
							case DT_INT:
								sum_avg += getVecInt(inputVec, row);
								break;
							case DT_LONG:
								sum_avg += getVecLong(inputVec, row);
								break;
							case DT_FLOAT:
								sum_avg += getVecFloat(inputVec, row);
								break;
							default:
								FATAL_LOG("not supproted");
						}

						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							newL = appendToTailOfList(newL, createConstFloat(sum_avg));
							newL = appendToTailOfList(newL, createConstFloat(sum_avg));
							newL = appendToTailOfList(newL, createConstLong(1));
							vecAppendFloat(outputVecInsert, sum_avg);
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0){
							newL = appendToTailOfList(newL, createConstFloat(sum_avg));
							newL = appendToTailOfList(newL, createConstLong(1));
							switch (outputVecType) {
								case DT_INT:
									vecAppendInt(outputVecInsert, (int) sum_avg);
									break;
								case DT_LONG:
									vecAppendLong(outputVecInsert, (gprom_long_t) sum_avg);
									break;
								case DT_FLOAT:
									vecAppendFloat(outputVecInsert, sum_avg);
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							newL = appendToTailOfList(newL, createConstLong(1));
							switch (outputVecType) {
								case DT_INT:
									vecAppendInt(outputVecInsert, 1);
									break;
								case DT_LONG:
									vecAppendLong(outputVecInsert, (gprom_long_t) 1);
									break;
								case DT_FLOAT:
									vecAppendFloat(outputVecInsert, (double) 1);
								default:
									FATAL_LOG("not supported");
							}
							// vecAppendInt(outputVecInsert, 1);
						}
						addToMap(acs->map, (Node *) createConstString(gbVal), (Node *) newL);
						// DEBUG_NODE_BEATIFY_LOG("oldList", oldL);
						DEBUG_NODE_BEATIFY_LOG("newList", newL);
					}
					if (!hasFinishGB) {
						for (int gbAttIdx = 0; gbAttIdx < gbAttrCnt; gbAttIdx++) {
							int fromPos = getVecInt(gbPoss, gbAttIdx);
							DataType gbDataType = getVecInt(gbType, gbAttIdx);
							char *toName = STRING_VALUE((Constant *) MAP_GET_STRING(mapFCsToSchemas, getVecString(gbName, gbAttIdx)));
							int toPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCInsert->attriToPos, toName));

							Vector *fromVecc = (Vector *) getVecNode(dataChunkInsert->tuples, fromPos);
							Vector *resVecDel = (Vector *) getVecNode(resultDCDelete->tuples, toPos);
							Vector *resVecIns = (Vector *) getVecNode(resultDCInsert->tuples, toPos);
							Vector *updIdenIns = (Vector *) resultDCInsert->updateIdentifier;
							Vector *updIdenDel = (Vector *) resultDCDelete->updateIdentifier;

							switch(gbDataType) {
								case DT_INT:
								case DT_BOOL:
								{
									int val = getVecInt(fromVecc, row);
									vecAppendInt(resVecIns, val);
									vecAppendInt(updIdenIns, 1);
									if (resUpdType == 0) {
										vecAppendInt(resVecDel, val);
										vecAppendInt(updIdenDel, -1);
									}
								}
									break;
								case DT_LONG:
								{
									gprom_long_t val = getVecLong(fromVecc, row);
									vecAppendLong(resVecIns, val);
									vecAppendInt(updIdenIns, 1);
									if (resUpdType == 0) {
										vecAppendLong(resVecDel, val);
										vecAppendInt(updIdenDel, -1);
									}
								}
									break;
								case DT_FLOAT:
								{
									double val = getVecFloat(fromVecc, row);
									vecAppendFloat(resVecIns, val);
									vecAppendInt(updIdenIns, 1);
									if (resUpdType == 0) {
										vecAppendFloat(resVecDel, val);
										vecAppendInt(updIdenDel, -1);
									}
								}
									break;
								case DT_STRING:
								case DT_VARCHAR2:
								{
									char *val = getVecString(fromVecc, row);
									vecAppendString(resVecIns, val);
									vecAppendInt(updIdenIns, 1);
									if (resUpdType == 0) {
										vecAppendString(resVecDel, val);
										vecAppendInt(updIdenDel, -1);
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						}
					}

					// dealing with ps;
					// if (dataChunkInsert non ps data) continue;

					FOREACH_HASH_KEY(Constant, c, dataChunkInsert->fragmentsInfo) {
						Vector *inputBSVec = (Vector *) MAP_GET_STRING(dataChunkInsert->fragmentsInfo, STRING_VALUE(c));
						BitSet *inputBS = (BitSet *) getVecNode(inputBSVec, row);
						if (resUpdType == 1) {
							// BitSet *resBS = (BitSet *) copyObject(inputBS);
							if (!hasFinishPS) {
								Vector *psVecIns = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));

								if (psVecIns == NULL) {
									psVecIns = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecIns, (Node *) copyObject(inputBS));
								addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecIns);
							}
							HashMap *psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
							if (psMapInACS == NULL) {
								psMapInACS = NEW_MAP(Constant, Node);
							}

							addToMap(psMapInACS, (Node *) createConstString(gbVal), (Node *) copyObject(inputBS));
							addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) psMapInACS);

							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
							if (NULL == gbFragCnt) {
								gbFragCnt = NEW_MAP(Constant, Node);
							}

							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
							if (fragCnt == NULL) {
								fragCnt = NEW_MAP(Constant, Constant);
							}

							char *bsStr = bitSetToString(inputBS);
							for (int bitIndex = 0; bitIndex < strlen(bsStr); bitIndex++) {
								if (bsStr[bitIndex] == '1') {
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
								}
							}

							addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
							addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						} else if (resUpdType == 0) {
							HashMap *psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
							if (NULL == psMapInACS) {
								psMapInACS = NEW_MAP(Constant, Node);
							}

							// old bitset
							BitSet *bsInACS = (BitSet *) MAP_GET_STRING(psMapInACS, gbVal);
							// BitSet *resBS = (BitSet *) copyObject(bsInACS);
							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
							if (gbFragCnt == NULL) {
								gbFragCnt = NEW_MAP(Constant, Node);
							}

							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
							if (fragCnt == NULL) {
								fragCnt = NEW_MAP(Constant, Constant);
							}

							char *bsStr = bitSetToString(inputBS);
							for (int bitIndex = 0; bitIndex < strlen(bsStr); bitIndex++) {
								if (bsStr[bitIndex] == '1') {
									if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
										int cnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
										addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(cnt + 1));
									} else {
										addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
									}
								}
							}
							BitSet *resBS = bitOr(inputBS, bsInACS);

							if (!hasFinishPS) {
								Vector *psVecIns = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
								if (psVecIns == NULL) {
									psVecIns = makeVector(VECTOR_NODE, T_Vector);
								}

								Vector *psVecDel = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
								if (psVecDel == NULL) {
									psVecDel = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecDel, (Node *) copyObject(bsInACS));
								vecAppendNode(psVecIns, (Node *) copyObject(resBS));
								addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecIns);
								addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecDel);
							}

							addToMap(psMapInACS, (Node *) createConstString(gbVal), (Node *) copyObject(resBS));
							addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) psMapInACS);
							addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
							addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						}
					}
				}
				// addToMap(dataStructures, (Node *) createConstString(nameInDS), (Node *) acs);
			}

			if (dataChunkDelete != NULL) {
				int inputVecPos = INT_VALUE(MAP_GET_STRING(dataChunkDelete->attriToPos, ar->name));
				DataType inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkDelete->posToDatatype, inputVecPos));
				Constant *nameInOutChunk = (Constant *) MAP_GET_STRING(mapFCsToSchemas, nameInDS);
				int outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCDelete->attriToPos, STRING_VALUE(nameInOutChunk)));
				DataType outputVecType = INT_VALUE((Constant *) MAP_GET_INT(resultDCDelete->posToDatatype, outputVecPos));

				Vector *inputVec = (Vector *) getVecNode(dataChunkDelete->tuples, inputVecPos);
				Vector *outputVecInsert = (Vector *) getVecNode(resultDCInsert->tuples, outputVecPos);
				Vector *outputVecDelete = (Vector *) getVecNode(resultDCDelete->tuples, outputVecPos);
				char **gbValArr = (char **) VEC_TO_ARR(gbValsDelete, char);

				for (int row = 0; row < gbValsDelete->length; row++) {
					int resUpdType = 0;
					char *gbVal = gbValArr[row];
					List *oldL = (List *) MAP_GET_STRING(acs->map, gbVal);
					List *newL = NIL;
					if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
						double avg = FLOAT_VALUE((Constant *) getNthOfListP(oldL, 0));
						double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldL, 1));
						gprom_long_t cnt = LONG_VALUE((Constant *) getNthOfListP(oldL, 2));
						vecAppendFloat(outputVecDelete, avg);

						if (cnt <= 1) {
							removeMapStringElem(acs->map, gbVal);
							resUpdType = -1;
						} else {
							resUpdType = 0;

							switch (inputVecType) {
								case DT_INT:
									sum -= getVecInt(inputVec, row);
									break;
								case DT_FLOAT:
									sum -= getVecFloat(inputVec, row);
									break;
								case DT_LONG:
									sum -= getVecLong(inputVec, row);
									break;
								default:
									FATAL_LOG("not supported");
							}
							cnt -= 1;
							avg = sum / cnt;
							vecAppendFloat(outputVecInsert, avg);
							newL = appendToTailOfList(newL, createConstFloat(avg));
							newL = appendToTailOfList(newL, createConstFloat(sum));
							newL = appendToTailOfList(newL, createConstLong(cnt));
							addToMap(acs->map, (Node *) createConstString(gbVal), (Node *) newL);
						}
						// addToMap()
					} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
						double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldL, 0));
						gprom_long_t cnt = LONG_VALUE((Constant *) getNthOfListP(oldL, 1));
						double newSum = (double) 0;
						switch (inputVecType) {
							case DT_INT:
							{
								newSum = sum - getVecInt(inputVec, row);
							}
								break;
							case DT_LONG:
							{
								newSum = sum - getVecLong(inputVec, row);
							}
								break;
							case DT_FLOAT:
							{
								newSum = sum - getVecFloat(inputVec, row);
							}
								break;
							default:
								FATAL_LOG("type not supported");
						}
						INFO_LOG("gbVal: %s", gbVal);
						INFO_LOG("SUM %f, new SUM: %f", sum, newSum);
						switch(outputVecType) {
							case DT_INT:
							{
								vecAppendInt(outputVecDelete, (int) sum);
								if (cnt > 1)
								{
									vecAppendInt(outputVecInsert, (int) newSum);
								}
							}
								break;
							case DT_LONG:
							{
								vecAppendLong(outputVecDelete, (gprom_long_t) sum);
								if (cnt > 1)
								{
									vecAppendLong(outputVecInsert, (gprom_long_t) newSum);
								}
							}
								break;
							case DT_FLOAT:
							{
								vecAppendFloat(outputVecDelete, sum);
								if (cnt > 1)
								{
									vecAppendFloat(outputVecInsert, newSum);
								}
							}
								break;
							default:
								FATAL_LOG("not supported");
						}
						if (cnt <= 1) {
							resUpdType = -1;
							removeMapStringElem(acs->map, gbVal);
						} else {
							resUpdType = 0;
							cnt -= 1;
							newL = appendToTailOfList(newL, createConstFloat(newSum));
							newL = appendToTailOfList(newL, createConstLong(cnt));
							addToMap(acs->map, (Node *) createConstString(gbVal), (Node *) newL);
						}
					} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
						gprom_long_t cnt = LONG_VALUE((Constant *) getNthOfListP(oldL, 0));
						// vecAppendInt(outputVecDelete, cnt);
						if (cnt <= 1) {
							resUpdType = -1;
							removeMapStringElem(acs->map, gbVal);
						} else {
							resUpdType = 0;
							newL = appendToTailOfList(newL, createConstLong(cnt - 1));
							addToMap(acs->map, (Node *) createConstString(gbVal), (Node *) newL);
						}

						switch (outputVecType)
						{
							case DT_INT:
							{
								vecAppendInt(outputVecDelete, (int) cnt);
								if (resUpdType == 0) {
									vecAppendInt(outputVecInsert, (int) (cnt - 1));
								}
							}
								break;
							case DT_LONG:
							{
								vecAppendLong(outputVecDelete, cnt);
								if (resUpdType == 0) {
									vecAppendLong(outputVecInsert, (int) (cnt - 1));
								}
							}
							case DT_FLOAT:
							{
								vecAppendFloat(outputVecDelete, (double) cnt);
								if (resUpdType == 0) {
									vecAppendFloat(outputVecInsert, (double) (cnt - 1));
								}
							}
							default:
								break;
						}
					}
					DEBUG_NODE_BEATIFY_LOG("oldList", oldL);
					DEBUG_NODE_BEATIFY_LOG("newList", newL);
					if (!hasFinishGB) {
						for (int gbAttIdx = 0; gbAttIdx < gbAttrCnt; gbAttIdx++) {
							int fromPos = getVecInt(gbPoss, gbAttIdx);
							DataType gbDataType = getVecInt(gbType, gbAttIdx);
							char *toName = STRING_VALUE((Constant *) MAP_GET_STRING(mapFCsToSchemas, getVecString(gbName, gbAttIdx)));
							int toPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCDelete->attriToPos, toName));

							Vector *fromVecc = (Vector *) getVecNode(dataChunkDelete->tuples, fromPos);

							Vector *resVecDel = (Vector *) getVecNode(resultDCDelete->tuples, toPos);
							Vector *resVecIns = (Vector *) getVecNode(resultDCInsert->tuples, toPos);
							Vector *updIdenIns = (Vector *) resultDCInsert->updateIdentifier;
							Vector *updIdenDel = (Vector *) resultDCDelete->updateIdentifier;

							switch(gbDataType) {
								case DT_INT:
								case DT_BOOL:
								{
									int val = getVecInt(fromVecc, row);
									vecAppendInt(resVecDel, val);
									vecAppendInt(updIdenDel, -1);
									if (resUpdType == 0) {
										vecAppendInt(resVecIns, val);
										vecAppendInt(updIdenIns, 1);
									}
								}
									break;
								case DT_LONG:
								{
									gprom_long_t val = getVecLong(fromVecc, row);
									vecAppendLong(resVecDel, val);
									vecAppendInt(updIdenDel, -1);
									if (resUpdType == 0) {
										vecAppendLong(resVecIns, val);
										vecAppendInt(updIdenIns, 1);
									}
								}
									break;
								case DT_FLOAT:
								{
									double val = getVecFloat(fromVecc, row);
									vecAppendFloat(resVecDel, val);
									vecAppendInt(updIdenDel, -1);
									if (resUpdType == 0) {
										vecAppendFloat(resVecIns, val);
										vecAppendInt(updIdenIns, 1);
									}
								}
									break;
								case DT_STRING:
								case DT_VARCHAR2:
								{
									char *val = getVecString(fromVecc, row);
									vecAppendString(resVecDel, val);
									vecAppendInt(updIdenDel, -1);
									if (resUpdType == 0) {
										vecAppendString(resVecIns, val);
										vecAppendInt(updIdenIns, 1);
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						}
					}
					// deal with PS
					// if(non pS check )continue;
					FOREACH_HASH_KEY(Constant, c, dataChunkDelete->fragmentsInfo) {
						Vector *inputBSVec = (Vector *) MAP_GET_STRING(dataChunkDelete->fragmentsInfo, STRING_VALUE(c));
						BitSet *inputBS = (BitSet *) getVecNode(inputBSVec, row);

						if (resUpdType == -1) {
							HashMap *psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
							if (psMapInACS == NULL) {
								FATAL_LOG("ERROR PS MAP");
							}
							BitSet *bitset = (BitSet *) MAP_GET_STRING(psMapInACS, gbVal);
							if (bitset == NULL) {
								FATAL_LOG("ERROR PS");
							}

							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
							if (!hasFinishPS) {
								Vector *psVecDel = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
								if (psVecDel == NULL) {
									psVecDel = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecDel, (Node *) copyObject(bitset));
								addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecDel);
							}
							removeMapStringElem(psMapInACS, gbVal);
							removeMapStringElem(gbFragCnt, gbVal);
							addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) psMapInACS);
						} else if (resUpdType == 0) {
							HashMap *psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);


							BitSet *oriBS = (BitSet *) MAP_GET_STRING(psMapInACS, gbVal);
							BitSet *resBS = (BitSet *) copyObject(oriBS);

							char *bsStr = bitSetToString(inputBS);
							for (int bitIndex = 0; bitIndex < strlen(bsStr); bitIndex++) {
								if (bsStr[bitIndex] == '1') {
									int cnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
									if (cnt <= 1) {
										setBit(resBS, bitIndex, FALSE);
										removeMapElem(fragCnt, (Node *) createConstInt(bitIndex));
									} else {
										addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(cnt - 1));
									}
								}
							}

							if (!hasFinishPS) {
								Vector *psVecDel = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
								if (psVecDel == NULL) {
									psVecDel = makeVector(VECTOR_NODE, T_Vector);
								}

								Vector *psVecIns = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
								if (psVecIns == NULL) {
									psVecIns = makeVector(VECTOR_NODE, T_Vector);
								}

								vecAppendNode(psVecDel, (Node *) copyObject(oriBS));
								vecAppendNode(psVecIns, (Node *) copyObject(resBS));

								addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecDel);
								addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecIns);
							}
							addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
							addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							addToMap(psMapInACS, (Node *) createConstString(gbVal), (Node *) copyObject(resBS));
							addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) psMapInACS);
						}
					}
				}
			}
			addToMap(dataStructures, (Node *) createConstString(nameInDS), (Node *) acs);
			// get input vec and output vec;
		} else if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0
		|| strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
			// get heap;
			GBHeaps *gbHeap = (GBHeaps *) MAP_GET_STRING(dataStructures, nameInDS);

			int inputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunkInsert->attriToPos, ar->name));
			DataType inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, inputVecPos));
			Constant *nameInOutChunk = (Constant *) MAP_GET_STRING(mapFCsToSchemas, CONCAT_STRINGS(fc->functionname, "_", ar->name));

			int outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunkInsert->attriToPos, STRING_VALUE(nameInOutChunk)));
			// DataType outputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, outputVecPos));


			if (dataChunkInsert != NULL) {
				Vector *inputVec = (Vector *) getVecNode(dataChunkInsert->tuples, inputVecPos);
				Vector *outputVecInsert = (Vector *) getVecNode(resultDCInsert->tuples, outputVecPos);
				Vector *outputVecDelete = (Vector *) getVecNode(resultDCDelete->tuples, outputVecPos);

				char **gbValArr = (char **) VEC_TO_ARR(gbValsInsert, char);

				for (int row = 0; row < gbValsInsert->length; row++) {
					int resUpdType = 0;
					char *gbVal = gbValArr[row];
					if (MAP_HAS_STRING_KEY(gbHeap->heapLists, gbVal)) {
						List *heap = (List *) MAP_GET_STRING(gbHeap->heapLists, gbVal);
						if (heap == NIL || LIST_LENGTH(heap) == 0) {
							resUpdType = 1;
							switch (inputVecType) {
								case DT_INT:
								{
									int insertV = getVecInt(inputVec, row);
									vecAppendInt(outputVecInsert, insertV);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstInt(insertV));
								}
									break;
								case DT_LONG:
								{
									gprom_long_t insertV = getVecLong(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstLong(insertV));
									vecAppendLong(outputVecInsert, insertV);
								}
									break;
								case DT_FLOAT:
								{
									double insertV = getVecFloat(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstFloat(insertV));
									vecAppendFloat(outputVecInsert, insertV);
								}
									break;
								case DT_STRING:
								case DT_VARCHAR2:
								{
									char *insertV = getVecString(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstString(insertV));
									vecAppendString(outputVecInsert, strdup(insertV));
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else {
							resUpdType = 0;
							Constant *oldV = (Constant *) getNthOfListP(heap, 0);
							switch (inputVecType)
							{
								case DT_INT:
								{
									vecAppendInt(outputVecDelete, INT_VALUE(oldV));
									int insertV = getVecInt(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstInt(insertV));
									vecAppendInt(outputVecInsert, INT_VALUE((Constant *) getNthOfListP(heap, 0)));
								}
									break;
								case DT_FLOAT:
								{
									vecAppendFloat(outputVecDelete, FLOAT_VALUE(oldV));
									double insertV = getVecFloat(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstFloat(insertV));
									vecAppendFloat(outputVecInsert, FLOAT_VALUE((Constant *) getNthOfListP(heap, 0)));

								}
									break;
								case DT_LONG:
								{
									vecAppendLong(outputVecDelete, LONG_VALUE(oldV));
									gprom_long_t insertV = getVecLong(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstLong(insertV));
									vecAppendLong(outputVecInsert, LONG_VALUE((Constant *) getNthOfListP(heap, 0)));

								}
									break;
								case DT_STRING:
								case DT_VARCHAR2:
								{
									vecAppendString(outputVecDelete, STRING_VALUE(oldV));
									char* insertV = getVecString(inputVec, row);
									heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstString(insertV));
									vecAppendString(outputVecInsert, strdup(STRING_VALUE((Constant *) getNthOfListP(heap, 0))));

								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						}
						addToMap(gbHeap->heapLists, (Node *) createConstString(gbVal), (Node *) heap);
					} else {
						resUpdType = 1;
						List *heap = NIL;
						switch (inputVecType) {
							case DT_INT:
							{
								int insertV = getVecInt(inputVec, row);
								vecAppendInt(outputVecInsert, insertV);
								heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstInt(insertV));
							}
								break;
							case DT_LONG:
							{
								gprom_long_t insertV = getVecLong(inputVec, row);
								heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstLong(insertV));
								vecAppendLong(outputVecInsert, insertV);
							}
								break;
							case DT_FLOAT:
							{
								double insertV = getVecFloat(inputVec, row);
								heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstFloat(insertV));
								vecAppendFloat(outputVecInsert, insertV);
							}
								break;
							case DT_STRING:
							case DT_VARCHAR2:
							{
								char *insertV = getVecString(inputVec, row);
								heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstString(insertV));
								vecAppendString(outputVecInsert, strdup(insertV));
							}
								break;
							default:
								FATAL_LOG("not supported");
						}
						addToMap(gbHeap->heapLists, (Node *) createConstString(gbVal), (Node *) heap);
					}
					if (!hasFinishGB) {
						if (resUpdType == 0) {
							// vecAppendInt(resDCDelGBs, row);
							vecAppendInt(resDCGBsDelFromIns, row);
							vecAppendInt(resDCGBsInsFromIns, row);
							// vecAppendInt(resDCInsGBs, row);
						} else if (resUpdType == 1) {
							// vecAppendInt(resDCInsGBs, row);
							vecAppendInt(resDCGBsInsFromIns, row);
						}
					}

					// deal with ps
					// if (noon pso branch) continue;
					FOREACH_HASH_KEY(Constant, c, dataChunkInsert->fragmentsInfo) {
						Vector *inputBSVec = (Vector *) MAP_GET_STRING(dataChunkInsert->fragmentsInfo, STRING_VALUE(c));
						BitSet *inputBS = (BitSet *) getVecNode(inputBSVec, row);
						if (resUpdType == 1) {
							if (!hasFinishPS) {
								Vector *psVecIns = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
								if (psVecIns == NULL) {
									psVecIns = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecIns, (Node *) copyObject(inputBS));
								addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecIns);
							}
							HashMap *psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));

							if (psMapInGBHeap == NULL) {
								psMapInGBHeap = NEW_MAP(Constant, Node);
							}

							addToMap(psMapInGBHeap, (Node *) createConstString(gbVal), (Node *) copyObject(inputBS));
							addToMap(gbHeap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGBHeap);

							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));

							if (gbFragCnt == NULL) {
								gbFragCnt = NEW_MAP(Constant, Node);
							}

							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
							if (fragCnt == NULL) {
								fragCnt = NEW_MAP(Constant, Constant);
							}

							char *bsStr = bitSetToString(inputBS);
							for (int bitIndex = 0; bitIndex < strlen(bsStr); bitIndex++) {
								if (bsStr[bitIndex] == '1') {
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
								}
							}
							addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
							addToMap(gbHeap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						} else if (resUpdType == 0) {
							HashMap *psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));

							BitSet *oldBS = (BitSet *) MAP_GET_STRING(psMapInGBHeap, gbVal);

							if (!hasFinishPS) {
								Vector *psVecDel = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
								if (psVecDel == NULL) {
									psVecDel = makeVector(VECTOR_NODE, T_Vector);
								}

								vecAppendNode(psVecDel, (Node *) copyObject(oldBS));
								addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecDel);
							}

							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));
							if (gbFragCnt == NULL) {
								gbFragCnt = NEW_MAP(Constant, Node);
							}

							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
							if (fragCnt == NULL) {
								fragCnt = NEW_MAP(Constant, Constant);
							}

							char* bitStr = bitSetToString(inputBS);
							for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
								if (bitStr[bitIndex] == '1') {
									int cnt = 0;
									if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
										cnt = INT_VALUE(MAP_GET_INT(fragCnt, bitIndex));
									}
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(cnt + 1));
								}
							}

							BitSet *resBS = bitOr(inputBS, oldBS);
							if (!hasFinishPS) {
								Vector *psVecIns = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
								if (psVecIns == NULL) {
									psVecIns = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecIns, copyObject(resBS));
								addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecIns);
							}
						}
					}
				}
			}

			if (dataChunkDelete != NULL) {
				Vector *inputVec = (Vector *) getVecNode(dataChunkDelete->tuples, inputVecPos);
				Vector *outputVecInsert = (Vector *) getVecNode(resultDCInsert->tuples, outputVecPos);
				Vector *outputVecDelete = (Vector *) getVecNode(resultDCDelete->tuples, outputVecPos);

				char **gbValArr = (char **) VEC_TO_ARR(gbValsDelete, char);
				for (int row = 0; row < gbValsDelete->length; row++) {
					int resUpdType = 0;
					char *gbVal = gbValArr[row];

					List *heap = (List *) MAP_GET_STRING(gbHeap->heapLists, gbVal);
					if (LIST_LENGTH(heap) == 1) {
						resUpdType = -1;
						Constant *delV = getNthOfListP(heap, 0);
						switch (inputVecType) {
							case DT_INT:
								vecAppendInt(outputVecDelete, INT_VALUE(delV));
								break;
							case DT_LONG:
								vecAppendInt(outputVecDelete, LONG_VALUE(delV));
								break;
							case DT_FLOAT:
								vecAppendFloat(outputVecDelete, FLOAT_VALUE(delV));
								break;
							case DT_STRING:
							case DT_VARCHAR2:
								vecAppendString(outputVecDelete, STRING_VALUE(delV));
								break;
							default:
								FATAL_LOG("not supported");
						}
						removeMapStringElem(gbHeap->heapLists, gbVal);
					} else {
						resUpdType = 0;
						Constant *oldV = (Constant *) getNthOfListP(heap, 0);
						switch (inputVecType) {
							case DT_INT:
							{
								int delV = getVecInt(inputVec, row);
								heap = heapDelete(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstInt(delV));
								vecAppendInt(outputVecDelete, INT_VALUE(oldV));
								vecAppendInt(outputVecInsert, INT_VALUE((Constant *) getNthOfListP(heap, 0)));
							}
								break;
							case DT_FLOAT:
							{
								double delV = getVecFloat(inputVec, row);
								heap = heapDelete(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstFloat(delV));
								vecAppendFloat(outputVecDelete, FLOAT_VALUE(oldV));
								vecAppendFloat(outputVecInsert, FLOAT_VALUE((Constant *) getNthOfListP(heap, 0)));
							}
								break;
							case DT_LONG:
							{
								gprom_long_t delV = getVecLong(inputVec, row);
								heap = heapDelete(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstLong(delV));
								vecAppendFloat(outputVecDelete, LONG_VALUE(oldV));
								vecAppendFloat(outputVecInsert, LONG_VALUE((Constant *) getNthOfListP(heap, 0)));
							}
								break;
							case DT_STRING:
							case DT_VARCHAR2:
							{
								char *delV = getVecString(inputVec, row);
								heap = heapDelete(heap, STRING_VALUE(gbHeap->heapType), (Node *) createConstString(delV));
								vecAppendString(outputVecDelete, STRING_VALUE(oldV));
								vecAppendString(outputVecInsert, STRING_VALUE((Constant *) getNthOfListP(heap, 0)));
							}
								break;
							default:
								FATAL_LOG("not supported");
						}
						addToMap(gbHeap->heapLists, (Node *) gbVal, (Node *) heap);
					}
					if (!hasFinishGB) {
						if (resUpdType == 0) {
							// vecAppendInt(resDCDelGBs, row);
							// vecAppendInt(resDCInsGBs, row);
							vecAppendInt(resDCGBsDelFromDel, row);
							vecAppendInt(resDCGBsInsFromDel, row);
						} else if (resUpdType == -1) {
							// vecAppendInt(resDCDelGBs, row);
							vecAppendInt(resDCGBsDelFromDel, row);
						}
					}
					// deal with ps;
					// if(non ps ) continue;
					FOREACH_HASH_KEY(Constant, c, dataChunkDelete->fragmentsInfo) {
						Vector *inputBSVec = (Vector *) MAP_GET_STRING(dataChunkDelete->fragmentsInfo, STRING_VALUE(c));
						BitSet *inputBS = (BitSet *) getVecNode(inputBSVec, row);
						if (resUpdType == -1) {
							if (!hasFinishPS) {
								Vector *psVecDel = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
								if (psVecDel == NULL) {
									psVecDel = makeVector(VECTOR_NODE, T_Vector);
								}

								vecAppendNode(psVecDel, (Node *) copyObject(inputBS));
								addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecDel);
							}

							HashMap *psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));
							removeMapStringElem(psMapInGBHeap, gbVal);
							if (mapSize(psMapInGBHeap) > 0) {
								addToMap(gbHeap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGBHeap);
							}

							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));
							removeMapStringElem(gbFragCnt, gbVal);
							if (mapSize(gbFragCnt) > 0) {
								addToMap(gbHeap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							}
						} else if (resUpdType == 0) {
							HashMap *psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));
							BitSet *oldBS = (BitSet *) MAP_GET_STRING(psMapInGBHeap, gbVal);
							if (!hasFinishPS) {
								Vector *psVecDel = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
								if (psVecDel == NULL) {
									psVecDel = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecDel, (Node *) copyObject(oldBS));
								addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecDel);
							}

							HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));
							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);

							char *bitStr = bitSetToString(inputBS);
							for (int index = 0; index < strlen(bitStr); index++) {
								if (bitStr[index] == '1') {
									int cnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, index));
									if (cnt <= 1) {
										setBit(oldBS, index, FALSE);
										removeMapElem(fragCnt, (Node *) createConstInt(index));
									} else {
										addToMap(fragCnt, (Node *) createConstInt(index), (Node *) createConstInt(cnt - 1));
									}
								}
							}

							addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
							addToMap(gbHeap->provSketchs, (Node *) copyObject(c), (Node *) gbFragCnt);
							if (!hasFinishPS) {
								Vector *psVecIns = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
								if (psVecIns == NULL) {
									psVecIns = makeVector(VECTOR_NODE, T_Vector);
								}
								vecAppendNode(psVecIns, (Node *) copyObject(oldBS));
								addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) psVecIns);
							}
						}
					}
				}
			}

			addToMap(dataStructures, (Node *) createConstString(nameInDS), (Node *) gbHeap);
		}
		hasFinishPS = TRUE;
		hasFinishGB = TRUE;
	}

	// fill group by attrs and update identifier;
	// Vector *updIdenIns = resultDCInsert->updateIdentifier;
	// Vector *updIdenDel = resultDCDelete->updateIdentifier;

	INFO_OP_LOG("AGG OP", op);
	DEBUG_NODE_BEATIFY_LOG("gb names:", gbName);
	DEBUG_NODE_BEATIFY_LOG("gb POS:", gbPoss);
	DEBUG_NODE_BEATIFY_LOG("gb TYPE:", gbType);
	DEBUG_NODE_BEATIFY_LOG("gb list", aggGBList);
	DEBUG_NODE_BEATIFY_LOG("args list:", aggFCList);
	DEBUG_NODE_BEATIFY_LOG("data chunk:", chunkMaps);

	INFO_LOG("gbAttrCnt: %d", gbAttrCnt);
	// set data chunk numTuple;
	resultDCInsert->numTuples = resultDCInsert->updateIdentifier->length;
	resultDCDelete->numTuples = resultDCDelete->updateIdentifier->length;

	HashMap *resChunkMaps = NEW_MAP(Constant, Node);

	if (resultDCInsert->updateIdentifier->length > 0) {
		addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) resultDCInsert);
	}
	if (resultDCDelete->updateIdentifier->length > 0) {
		addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) resultDCDelete);
	}

	if (mapSize(resChunkMaps) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, resChunkMaps);
	}
	removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
	DEBUG_NODE_BEATIFY_LOG("AGG CHUNK", resChunkMaps);
}

static void
updateDuplicateRemoval(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	INFO_LOG("UPDATE DUREM");
	QueryOperator *lchild = OP_LCHILD(op);
	if (!HAS_STRING_PROP(lchild, PROP_DATA_CHUNK)) {
		return;
	}
	List* dupAttrs = (List *) copyObject(op->schema->attrDefs);
	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(lchild, PROP_DATA_CHUNK);


	DataChunk *dc = initDataChunk();
	int len = LIST_LENGTH(dupAttrs);
	dc->attrNames = (List *) copyObject(dupAttrs);
	dc->tupleFields = len;
	for (int i = 0; i < len; i++) {
		AttributeDef *ad = (AttributeDef *) getNthOfListP(dupAttrs, i);
		addToMap(dc->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(i));
		addToMap(dc->posToDatatype, (Node *) createConstInt(i), (Node *) createConstInt(ad->dataType));
	}

	GBACSs *acs = (GBACSs *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);

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
				// FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
				// 	// dealing with ps; just copy to dst datachunk;
				// }

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
			// FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
				// dealing with ps; just copy to dst datachunk;
			//}
		} else {
			// there must be such group;
			for (int index = 0; index < LIST_LENGTH(values); index++) {
				vecAppendNode((Vector *) getVecNode(dc->tuples, index), (Node *) copyObject(getNthOfListP(values, index)));
			}
			vecAppendInt(dc->updateIdentifier, -1);
			(dc->numTuples)++;
			// FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
				// dealing with ps; just copy to dst datachunk;
			// }

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
				// FOREACH_HASH_KEY(char, ps, dataChunk->fragmentsInfo) {
				// 	// dealing with ps; just copy to dst datachunk;
				// }
				addToMap(acs->map, (Node *) createConstString(info->data), (Node *) createConstInt(count - 1));
			}
		}


	}

	// set the refreshed data structure;
	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, (Node *) acs);

	// set datachunk to operator;
	SET_STRING_PROP(op, PROP_DATA_CHUNK, (Node *) dc);

	// remove child's datachunk;
	removeStringProperty(lchild, PROP_DATA_CHUNK);
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

	if (!HAS_STRING_PROP(lchild, PROP_DATA_CHUNK)) {
		return;
	}

	// for order operator, we do nothing, just copy the data chunk and pass to next level;
	DataChunk *dc = (DataChunk *) copyObject((DataChunk *) GET_STRING_PROP(lchild, PROP_DATA_CHUNK));
	SET_STRING_PROP(op, PROP_DATA_CHUNK, dc);

	removeStringProperty(lchild, PROP_DATA_CHUNK);
}

static void
updateLimit(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));

	INFO_LOG("LIMIT OPERATOR");
	QueryOperator *lchild = OP_LCHILD(op);

	if (!HAS_STRING_PROP(lchild, PROP_DATA_CHUNK)) {
		return;
	}

	// if no data strcutre, indicate no order by lower;
	if (!HAS_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE)) {
		return;
	}

	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(lchild, PROP_DATA_CHUNK);
	DataChunk *dc = initDataChunk();
	dc->tupleFields = dataChunk->tupleFields;
	dc->attrNames = (List *) copyObject(dataChunk->attrNames);
	dc->attriToPos = (HashMap *) copyObject(dataChunk->attriToPos);
	dc->posToDatatype = (HashMap *) copyObject(dataChunk->posToDatatype);
	LMTChunk *lmtc = (LMTChunk *) GET_STRING_PROP(op, PROP_DATA_CHUNK);

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
	DEBUG_NODE_BEATIFY_LOG("what is stmt", updateStatement);
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

	// build a chumk map (insert chunk and delete chunk) based on update type;
	HashMap* chunkMap = getDataChunkFromUpdateStatement(updateStatement, (TableAccessOperator *) op);
	if (mapSize(chunkMap) > 0) {
		setStringProperty(op, PROP_DATA_CHUNK, (Node *) chunkMap);
	}

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FRO TABLEACCESS OPERATOR", chunkMap);
}

static HashMap*
getDataChunkFromUpdateStatement(QueryOperator* op, TableAccessOperator *tableAccessOp)
{
	/*
	 *  for a update statement:
	 * 		case 1: "insert": insert one tuple
	 * 		case 2: "delete": delete multiple tuples: run the query to get result;
	 * 		case 3: "update": delete multiple tuples, then insert multiple tuples;
	 */
	List *psAttrInfoList = (List *) getMapString(PS_INFO->tablePSAttrInfos, tableAccessOp->tableName);
	psAttrInfo *attrInfo = (psAttrInfo *) getHeadOfListP(psAttrInfoList);
	DataChunk* dataChunkInsert = NULL;
	DataChunk* dataChunkDelete = NULL;
	switch(nodeTag(((DLMorDDLOperator*) op)->stmt))
	{
		case T_Update:
		{
			dataChunkInsert = initDataChunk();
			dataChunkDelete = initDataChunk();
			getDataChunkOfUpdate(op, dataChunkInsert, dataChunkDelete, tableAccessOp, attrInfo);
		}
			break;
		case T_Insert:
		{
			dataChunkInsert = initDataChunk();
			getDataChunkOfInsert(op, dataChunkInsert, tableAccessOp, attrInfo);
		}
			break;
		case T_Delete:
		{
			dataChunkDelete = initDataChunk();
			getDataChunkOfDelete(op, dataChunkDelete, tableAccessOp, attrInfo);
		}
			break;
		default:
			FATAL_LOG("update should be an insert, delete or update? ");
	}

	HashMap *chunkMap = NEW_MAP(Constant, Node);

	if (dataChunkInsert != NULL && dataChunkInsert->numTuples > 0) {
		addToMap(chunkMap, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) dataChunkInsert);
	}

	if (dataChunkDelete != NULL && dataChunkDelete->numTuples > 0) {
		addToMap(chunkMap, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) dataChunkDelete);
	}

	return chunkMap;
}

static void
getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
{
	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));
	List *provAttrDefs = getProvenanceAttrDefs(rewr);
	char *psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;

	DEBUG_NODE_BEATIFY_LOG("INSERT", updateOp);
	Insert *insert = (Insert *) ((DLMorDDLOperator *) updateOp)->stmt;

	Schema *schema = createSchema(insert->insertTableName, insert->schema);

	// fill data chunk;
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
		DataType colType = INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, index));
		switch (colType) {
			case DT_INT:
				vecAppendNode(dataChunk->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				break;
			case DT_LONG:
				vecAppendNode(dataChunk->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				break;
			case DT_FLOAT:
				vecAppendNode(dataChunk->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				break;
			case DT_BOOL:
				vecAppendNode(dataChunk->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				break;
			case DT_STRING:
			case DT_VARCHAR2:
				vecAppendNode(dataChunk->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				break;
			default:
				FATAL_LOG("not support this data type currently");
		}
		// vecAppendNode(dataChunk->tuples, (Node*) makeVector(VECTOR_NODE, T_Vector));
	}

	List *tupleRow = (List *) insert->query;

	for (int col = 0; col < LIST_LENGTH(tupleRow); col++)
	{
		DataType dataType = INT_VALUE((Constant*)getMapInt(dataChunk->posToDatatype, col));
		Constant *value = (Constant *) getNthOfListP(tupleRow, col);
		Vector *colVec = (Vector *) getVecNode(dataChunk->tuples, col);
		switch (dataType) {
			case DT_INT:
				vecAppendInt(colVec, INT_VALUE(value));
				break;
			case DT_LONG:
				vecAppendLong(colVec, LONG_VALUE(value));
				break;
			case DT_FLOAT:
				vecAppendFloat(colVec, FLOAT_VALUE(value));
				break;
			case DT_BOOL:
				vecAppendInt(colVec, BOOL_VALUE(value));
				break;
			case DT_STRING:
			case DT_VARCHAR2:
				vecAppendNode(colVec, (Node *) STRING_VALUE(value));
				break;
			default:
				FATAL_LOG("not support this data type currently");
		}

		// set bit vector;
		if (col == psAttrCol) {
			BitSet *bitset = setFragmentToBitSet(INT_VALUE(value), attrInfo->rangeList);
			Vector *psVec = makeVector(VECTOR_NODE, T_Vector);
			if (MAP_HAS_STRING_KEY(dataChunk->fragmentsInfo, psName)) {
				psVec = (Vector *) MAP_GET_STRING(dataChunk->fragmentsInfo, psName);
				vecAppendNode(psVec, (Node *) bitset);
			} else {
				vecAppendNode(psVec, (Node *) bitset);
			}
			addToMap(dataChunk->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVec);
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

	Relation *relation = getQueryResult(query);

	// check the result relation size;
	// if size == 0; return;
	if (LIST_LENGTH(relation->tuples) == 0)
	{
		return;
	}

	// fill data chunk;
	int psAttrCol = -1;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++)
	{
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunk->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunk->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));

		// get ps attr col pos;
		if (psAttrCol == -1 && strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = i;
		}
	}

	dataChunk->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunk->numTuples = relation->tuples->length;
	dataChunk->tupleFields = LIST_LENGTH(relation->schema);

	// fill tuples
	for(int col = 0; col < LIST_LENGTH(relation->schema); col++)
	{
		DataType dataType = INT_VALUE((Constant*) MAP_GET_INT(dataChunk->posToDatatype, col));
		Vector *colVec = NULL;

		// Vector *colValues = makeVector(VECTOR_NODE, T_Vector);
		for(int row = 0; row < dataChunk->numTuples; row++)
		{
			// char *value = (char *) getNthOfListP((List *) getNthOfListP(relation->tuples, row), col);
			char *value = getVecString((Vector *) getVecNode(relation->tuples, row), col);
			switch (dataType) {
				case DT_INT:
				{
					if (row == 0) {
						colVec = makeVector(VECTOR_INT, T_Vector);
					}

					vecAppendInt(colVec, atoi(value));
				}
					break;
				case DT_LONG:
				{
					if (row == 0) {
						colVec = makeVector(VECTOR_LONG, T_Vector);
					}
					vecAppendInt(colVec, atol(value));
				}
					break;
				case DT_FLOAT:
				{
					if (row == 0) {
						colVec = makeVector(VECTOR_FLOAT, T_Vector);
					}
					vecAppendInt(colVec, atof(value));
				}
					break;
				case DT_BOOL:
				{
					if (row == 0) {
						colVec = makeVector(VECTOR_INT, T_Vector);
					}
					if (streq(value,"TRUE") || streq(value,"t") || streq(value,"true"))
						vecAppendInt(colVec, 1);
    				if (streq(value,"FALSE") || streq(value,"f") || streq(value,"false"))
						vecAppendInt(colVec, 0);
				}
					break;
				case DT_STRING:
				case DT_VARCHAR2:
				{
					if (row == 0) {
						colVec = makeVector(VECTOR_STRING, T_Vector);
					}
					vecAppendNode(colVec, (Node *) strdup(value));
				}
					break;
				default:
					FATAL_LOG("not support this data type currently");
			}
		}

		vecAppendNode(dataChunk->tuples, (Node*) colVec);
	}

	// fill identifier of update "-1"
	for (int row = 0; row < dataChunk->numTuples; row++)
	{
		vecAppendInt(dataChunk->updateIdentifier, -1);
	}
	// fill fragment information;
	Vector *psVec = makeVector(VECTOR_NODE, T_Vector);
	for (int row = 0; row < dataChunk->numTuples; row++) {
		int value = atoi(getVecString((Vector *) getVecNode(relation->tuples, row), psAttrCol));
		// Constant* value = makeValue(DT_INT, getVecString((Vector *)getVecNode(relation->tuples, row), psAttrCol));
		BitSet *bitset = setFragmentToBitSet(value, attrInfo->rangeList);
		vecAppendNode(psVec, (Node *) bitset);
	}

	addToMap(dataChunk->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVec);
	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR DELETE", dataChunk);
}

static void
getDataChunkOfUpdate(QueryOperator *updateOp, DataChunk *dataChunkInsert, DataChunk *dataChunkDelete, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
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
	if (relation->tuples->length == 0) {
		return;
	}

	// fill data chunk;
	int psAttrCol = -1;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++) {
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunkInsert->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunkInsert->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));
		addToMap(dataChunkDelete->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunkDelete->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));

		INFO_LOG("adname: %s, psName: %s", ad->attrName, psName);
		if (strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = i;
		}
	}

	dataChunkInsert->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunkInsert->numTuples = relation->tuples->length;
	dataChunkInsert->tupleFields = LIST_LENGTH(relation->schema) / 2;
	dataChunkDelete->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunkDelete->numTuples = relation->tuples->length;
	dataChunkDelete->tupleFields = LIST_LENGTH(relation->schema) / 2;

	// initialize each col vector;
	for (int col = 0; col < dataChunkInsert->tupleFields; col++) {
		// vecAppendNode(dataChunk->tuples, (Node*) makeVector(VECTOR_NODE, T_Vector));
		DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, col));
		switch (dataType)
		{
			case DT_INT:
			{
				vecAppendNode(dataChunkInsert->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(dataChunkDelete->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
			}
				break;
			case DT_LONG:
			{
				vecAppendNode(dataChunkInsert->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(dataChunkDelete->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
			}
				break;
			case DT_FLOAT:
			{
				vecAppendNode(dataChunkInsert->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(dataChunkDelete->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				vecAppendNode(dataChunkInsert->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(dataChunkDelete->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
			}
				break;
			case DT_BOOL:
			{
				vecAppendNode(dataChunkInsert->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(dataChunkDelete->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
			}
				break;
			default:
				break;
		}
	}
	INFO_LOG("psAttrCol %d", psAttrCol);

	// fill tuples:
	Vector *psVecInsert = makeVector(VECTOR_NODE, T_Vector);
	Vector *psVecDelete = makeVector(VECTOR_NODE, T_Vector);

	int tupleCols = dataChunkInsert->tupleFields * 2;
	for (int col = 0; col < tupleCols; col++) {
		int acturalPos = col / 2;

		Vector *colVec = NULL;
		if (col % 2 == 1) {
			colVec = (Vector *) getVecNode(dataChunkInsert->tuples, acturalPos);
		} else {
			colVec = (Vector *) getVecNode(dataChunkDelete->tuples, acturalPos);
		}
		DataType dataType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, acturalPos));
		for (int row = 0; row < relation->tuples->length; row++) {
			char *val = getVecString((Vector *) getVecNode(relation->tuples, row), col);
			switch (dataType) {
				case DT_INT:
					vecAppendInt(colVec, atoi(val));
					break;
				case DT_LONG:
					vecAppendLong(colVec, atol(val));
					break;
				case DT_FLOAT:
					vecAppendFloat(colVec, atof(val));
					break;
				case DT_STRING:
				case DT_VARCHAR2:
					vecAppendNode(colVec, (Node *) strdup(val));
					break;
				case DT_BOOL:
					if (streq(val, "TRUE") || streq(val, "t") || streq(val, "true"))
						vecAppendInt(colVec, 1);
    				if (streq(val, "FALSE") || streq(val, "f") || streq(val, "false"))
						vecAppendInt(colVec, 0);
				default:
					FATAL_LOG("data type is not supported");
			}

			if (acturalPos == psAttrCol) {
				BitSet *bitset = setFragmentToBitSet(atoi(val), attrInfo->rangeList);
				if (col % 2 == 1) {
					vecAppendNode(psVecInsert, (Node *) bitset);
				} else {
					vecAppendNode(psVecInsert, (Node *) bitset);
				}
			}
		}
	}

	addToMap(dataChunkInsert->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVecInsert);
	addToMap(dataChunkDelete->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVecDelete);

	for (int row = 0; row < dataChunkInsert->numTuples; row++) {
		vecAppendInt(dataChunkDelete->updateIdentifier, -1);
		vecAppendInt(dataChunkInsert->updateIdentifier, 1);
	}

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR UPDATE", dataChunkInsert);
}

static DataChunk*
filterDataChunk(DataChunk* dataChunk, Node* condition)
{

	// TODO: use ConstrelOperator;
	DEBUG_NODE_BEATIFY_LOG("DISPLAY DATACHUNK: ", dataChunk);

	ColumnChunk *filterResult = evaluateExprOnDataChunk(condition, dataChunk);

	// new a result data chunk AND set fields except numTuples;
	DataChunk* resultDC = initDataChunk();
	resultDC->attrNames = (List*) copyList(dataChunk->attrNames);
	resultDC->attriToPos = (HashMap*) copyObject(dataChunk->attriToPos);
	resultDC->posToDatatype = (HashMap*) copyObject(dataChunk->posToDatatype);
	resultDC->tupleFields = dataChunk->tupleFields;

	// result is True or False stored in a bitset;
	Vector *isTrue = filterResult->data.v;
	int *trueOrFalse = VEC_TO_IA(isTrue);
	Vector *undateIden = makeVector(VECTOR_INT, T_Vector);

	for (int col = 0; col < dataChunk->tupleFields; col++) {
		Vector *fromVec = (Vector *) getVecNode(dataChunk->tuples, col);
		DataType type = INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, col));
		Vector *toVec = NULL;
		switch (type) {
			case DT_INT:
			case DT_BOOL:
			{
				int *val = VEC_TO_IA(fromVec);
				toVec = makeVector(VECTOR_INT, T_Vector);
				for (int i = 0; i < fromVec->length; i++) {
					if (trueOrFalse[i] == 1) {
						vecAppendInt(toVec, val[i]);
						if (col == 0) {
							vecAppendInt(undateIden, getVecInt(dataChunk->updateIdentifier, i));
						}
					}
				}
			}
				break;
			case DT_LONG:
			{
				gprom_long_t *val = VEC_TO_LA(fromVec);
				toVec = makeVector(VECTOR_LONG, T_Vector);
				for (int i = 0; i < fromVec->length; i++) {
					if (trueOrFalse[i] == 1) {
						vecAppendLong(toVec, val[i]);
						if (col == 0) {
							vecAppendInt(undateIden, getVecInt(dataChunk->updateIdentifier, i));
						}
					}
				}
			}
				break;
			case DT_FLOAT:
			{
				double *val = VEC_TO_FA(fromVec);
				toVec = makeVector(VECTOR_FLOAT, T_Vector);
				for (int i = 0; i < fromVec->length; i++) {
					if (trueOrFalse[i] == 1) {
						vecAppendFloat(toVec, val[i]);
						if (col == 0) {
							vecAppendInt(undateIden, getVecInt(dataChunk->updateIdentifier, i));
						}
					}
				}
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				char **val = (char **) VEC_TO_ARR(fromVec, char);
				toVec = makeVector(VECTOR_STRING, T_Vector);
				for (int i = 0; i < fromVec->length; i++) {
					if (trueOrFalse[i] == 1) {
						vecAppendString(toVec, val[i]);
						if (col == 0) {
							vecAppendInt(undateIden, getVecInt(dataChunk->updateIdentifier, i));
						}
					}
				}
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		vecAppendNode(resultDC->tuples, (Node *) toVec);
	}
	resultDC->numTuples = resultDC->updateIdentifier->length;
	FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
		Vector *fromPSVec = (Vector *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
		Vector *toPSVec = makeVector(VECTOR_NODE, T_Vector);
		for (int row = 0; row < fromPSVec->length; row++) {
			if (trueOrFalse[row] == 1) {
				vecAppendNode(toPSVec, (Node *) getVecNode(fromPSVec, row));
			}
		}

		addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) toPSVec);
	}
	return resultDC;
}

static BitSet *
setFragmentToBitSet(int value, List *rangeList)
{
	BitSet *result = newBitSet(LIST_LENGTH(rangeList) - 1);

	// check if the value is beyond current range;
	if (value < INT_VALUE((Constant *) getNthOfListP(rangeList, 0))) {
		setBit(result, 0, TRUE);
		return result;
	} else if (value >= INT_VALUE((Constant *) getNthOfListP(rangeList, LIST_LENGTH(rangeList) - 1))) {
		setBit(result, LIST_LENGTH(rangeList) - 2, TRUE);
		return result;
	}

	//binary search;
	int start = 0;
	int end = LIST_LENGTH(rangeList) - 2;

	while (start + 1 < end) {
		int mid = start + (end - start) / 2;

		int leftVal = INT_VALUE((Constant *) getNthOfListP(rangeList, mid));
		int rightVal = INT_VALUE((Constant *) getNthOfListP(rangeList, mid + 1));

		if (leftVal <= value && value < rightVal) {
			setBit(result, mid, TRUE);
			return result;
		} else if (value < leftVal) {
			end = mid;
		} else {
			start = mid;
		}
	}

	// compare start;
	int startVal1 = INT_VALUE((Constant *) getNthOfListP(rangeList, start));
	int startVal2 = INT_VALUE((Constant *) getNthOfListP(rangeList, start + 1));

	if (startVal1 <= value && value < startVal2) {
		setBit(result, start, TRUE);
		return result;
	}

	// compare end;
	int endVal1 = INT_VALUE((Constant *) getNthOfListP(rangeList, end));
	int endVal2 = INT_VALUE((Constant *) getNthOfListP(rangeList, end + 1));

	if (endVal1 <= value && value < endVal2) {
		setBit(result, end, TRUE);
		return result;
	}

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

/*
static void
executeQueryWithoutResult(char* sql)
{
	if (getBackend() == BACKEND_POSTGRES) {
		postgresExecuteStatement(sql);
	} else if (getBackend() == BACKEND_ORACLE) {

	}
}*/

/*
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
*/

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
			char **ccValue = (char **) VEC_TO_ARR(cc->data.v, char);
			for (int i = 0; i < length; i++) {
				ccValue[i] = STRING_VALUE(c);
			}
		}
		break;
		case DT_BOOL: {
			// BitSet *bs = newBitSet(length);
			// for (int i = 0; i < length; i++) {
			// 	if (TRUE == BOOL_VALUE(c)) {
			// 		setBit(bs, i, TRUE);
			// 	}
			// }
			// cc->data.bs = copyObject(bs);
			int *ccVal = VEC_TO_IA(cc->data.v);
			for (int i = 0; i < length; i++) {
				ccVal[i] = BOOL_VALUE(c);
			}
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
	cc->data.v = vec;
	return cc;
	/*
	if (dataType == DT_BOOL) {
		BitSet *bs = newBitSet(cc->length);
		int *val = VEC_TO_IA(vec);
		for (int i = 0; i < cc->length; i++) {
			if (1 == val[i]) {
				setBit(bs, i, TRUE);
			}
		}
		cc->data.bs = bs;
	}
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
			cc->data.bs = bs;
		}
		break;
		default:
			FATAL_LOG("datatype %d is not supported", dataType);
			break;
	}
	return cc;
	*/
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
			// columnChunk->isBit = TRUE;
			// columnChunk->data.bs = newBitSet(len);
			columnChunk->data.v = makeVectorOfSize(VECTOR_INT, T_Vector, len);
			columnChunk->data.v->length = len;
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

	// ASSERT(left->length == right->length);

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

	// ASSERT(left->length == right->length);

	// left->data.bs = bitAnd(left->data.bs, right->data.bs);
	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, left->length);
	int *leftV = VEC_TO_IA(left->data.v);
	int *rightV = VEC_TO_IA(right->data.v);
	int *resV = VEC_TO_IA(resultCC->data.v);
	int len = left->length;
	for (int i = 0; i < len; i++) {
		resV[i] = (leftV[i] & rightV[i]);
	}
	return resultCC;
}

static ColumnChunk *
evaluateOperatorOr(Operator *op, DataChunk *dc)
{
	// both left and right operand are bitset;
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	// ASSERT(left->length == right->length);

	// left->data.bs = bitOr(left->data.bs, right->data.bs);
	ColumnChunk *resultCC = makeColumnChunk(DT_BOOL, left->length);
	int *leftV = VEC_TO_IA(left->data.v);
	int *rightV = VEC_TO_IA(right->data.v);
	int *resV = VEC_TO_IA(resultCC->data.v);
	int len = left->length;
	for (int i = 0; i < len; i++) {
		resV[i] = (leftV[i] | rightV[i]);
	}
	return resultCC;
}

static ColumnChunk *
evaluateOperatorNot(Operator *op, DataChunk *dc)
{
	// both left and right operand are bitset;
	// only have one "NOT(EXPR)"
	List *inputs = op->args;
	ColumnChunk *cc = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);

	int *val = VEC_TO_IA(cc->data.v);
	for (int i = 0; i < cc->length; i++) {
		val[i] = (1 - val[i]);
	}
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
	int *resV = VEC_TO_IA(resultCC->data.v);
	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] > rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] > rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] > rightV[i] ? 1 : 0);
			}

		}
		break;
		case DT_STRING: {
			char **leftV = (char **) VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = (char **) VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				resV[i] = (streq(leftV[i], rightV[i]) ? 1 : 0);
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
	int *resV = VEC_TO_IA(resultCC->data.v);
	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] < rightV[i] ? 1 : 0;
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] < rightV[i] ? 1 : 0;
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = leftV[i] < rightV[i] ? 1 : 0;
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) < 0) {
					resV[i] = 1;
				} else {
					resV[i] = 0;
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
	int *resV = VEC_TO_IA(resultCC->data.v);
	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] < rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] < rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] < rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) <= 0) {
					resV[i] = 1;
				} else {
					resV[i] = 0;
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
	int *resV = VEC_TO_IA(resultCC->data.v);
	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] > rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] > rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] > rightV[i] ? 1 : 0);
			}

		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) > 0) {
					resV[i] = 1;
				} else {
					resV[i] = 0;
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

	// ASSERT(left->length == right->length);

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
	int *resV = VEC_TO_IA(resultCC->data.v);
	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] >= rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] >= rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] >= rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) >= 0) {
					resV[i] = 1;
				} else {
					resV[i] = 0;
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

	int *resV = VEC_TO_IA(resultCC->data.v);
	switch(resultDT) {
		case DT_INT: {
			int *leftV = VEC_TO_IA(castLeft->data.v);
			int *rightV = VEC_TO_IA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] != rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] != rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] != rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_STRING: {
			char **leftV = VEC_TO_ARR(castLeft->data.v, char);
			char **rightV = VEC_TO_ARR(castRight->data.v, char);
			for (int i = 0; i < len; i++) {
				if (strcmp(leftV[i], rightV[i]) != 0) {
					resV[i] = 1;
				} else {
					resV[i] = 0;
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

	char **leftV = (char **) VEC_TO_ARR(castLeft->data.v, char);
	char **rightV = (char **) VEC_TO_ARR(castRight->data.v, char);
	char **resultV = (char **) VEC_TO_ARR(resultCC->data.v, char);

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
					resV[i] = strdup(gprom_itoa(vals[i]));
				}
			} else if (DT_FLOAT == fromType) {
				double *vals = VEC_TO_FA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = strdup(gprom_ftoa(vals[i]));
				}
			} else if (DT_LONG == fromType) {
				gprom_long_t *vals = VEC_TO_LA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = strdup(gprom_ltoa(vals[i]));
				}
			} else if (DT_BOOL == fromType) {
				int *val = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = strdup((val[i] == '1' ? "t" : "f"));
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
				// BitSet *bs = cc->data.bs;
				// char *bStr = bitSetToString(bs);
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = vals[i];
				}
			} else {
				FATAL_LOG("not supported");
				// TODO:
			}
		}
		break;
		case DT_FLOAT: {
			double *resV = VEC_TO_FA(resultCC->data.v);

			if (DT_INT == fromType || DT_BOOL == fromType) {
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (double) vals[i];
				}
			} else if (DT_LONG == fromType) {
				gprom_long_t *vals = VEC_TO_LA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (double) vals[i];
				}
			} else {
				FATAL_LOG("not supported");
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *resV = VEC_TO_LA(resultCC->data.v);
			if (DT_INT == fromType || DT_BOOL == fromType) {
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (gprom_long_t) vals[i];
				}
			} else if (DT_FLOAT == fromType) {
				double *vals = VEC_TO_FA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (gprom_long_t) vals[i];
				}
			} else {
				FATAL_LOG("not supported");
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

/*
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
*/

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
