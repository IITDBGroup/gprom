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
#include "model/set/set.h"
#include "sql_serializer/sql_serializer.h"
#include "utility/string_utils.h"
#include "instrumentation/timing_instrumentation.h"

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "provenance_rewriter/update_ps/update_ps_incremental.h"
#include "provenance_rewriter/update_ps/update_ps_build_state.h"
#include "provenance_rewriter/update_ps/rbtree.h"
#include "provenance_rewriter/update_ps/bloom.h"

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
static void updateJoinByGroupJoin(QueryOperator *op);
static void updateJoin2(QueryOperator *op);
static Vector *buildGroupByValueVecFromDataChunk(DataChunk *dc, List *gbList);
// chunk operation;
static HashMap *getDataChunkFromDeltaTable(TableAccessOperator * tableAccessOp);
static HashMap *getDataChunkFromUpdateStatement(QueryOperator* op, TableAccessOperator *tableAccessOp);
static void getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfDelete(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfUpdate(QueryOperator* updateOp, DataChunk* dataChunkInsert, DataChunk *dataChunkDelete, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static DataChunk *filterDataChunk(DataChunk* dataChunk, Node* condition);
static QueryOperator *captureRewrite(QueryOperator *operator);
static ConstRelMultiListsOperator *createConstRelMultiListsFromDataChunk(DataChunk *leftDC, DataChunk *rightDC, boolean isLeftBranch, List *parentList, List *provAttrDefs);
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
static ColumnChunk *getColumnChunkOfQuantComp(QuantifiedComparison *qc, DataChunk *dc);
static ColumnChunk *getColumnChunkOfCastExpr(CastExpr *expr, DataChunk *dc);
static ColumnChunk *getColumnChunkOfFunctionCall(FunctionCall *fc, DataChunk *dc);
static ColumnChunk *castColumnChunk(ColumnChunk *cc, DataType fromType, DataType toType);
static DataType evaluateTwoDatatypes(DataType dt1, DataType dt2);
// static Vector *columnChunkToVector(ColumnChunk *cc);
// static BitSet *computeIsNullBitSet(Node *expr, DataChunk *dc);
// static int limitCmp(const void **a, const void **b);
// static char *constToString(Constant *c);
#define SQL_PRE backendifyIdentifier("SELECT * FROM ")
#define VALUES_IDENT backendifyIdentifier(" VALUES ")
#define ADD_FUNC_PREFIX backendifyIdentifier("ADD_FUNC_PREFIX")
#define LEFT_BRANCH_OF_JOIN backendifyIdentifier("LEFT_BRANCH_OF_JOIN")
#define RIGHT_BRANCH_OF_JOIN backendifyIdentifier("RIGHT_BRANCH_OF_JOIN")
#define LEFT_UPDATE_IDENTIFIER backendifyIdentifier("LEFT_UPDATE_TYPE")
#define RIGHT_UPDATE_IDENTIFIER backendifyIdentifier("RIGHT_UPDATE_TYPE")
#define NULLVALUE backendifyIdentifier("null");

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

static boolean option_copy_join = FALSE;
// static HashMap *limitAttrPoss;
// static List *limitOrderBys;

DataChunk*
initDataChunk()
{
	DataChunk* dc = makeNode(DataChunk);

	dc->attrNames = NIL;
	dc->updateIdentifier = makeVector(VECTOR_INT, T_Vector);
	dc->tuples = makeVector(VECTOR_NODE, T_Vector);
	dc->fragmentsInfo = NEW_MAP(Constant, Node);
	dc->fragmentsIsInt = NEW_MAP(Constant, Constant);
	dc->numTuples = 0;
	dc->tupleFields = 0;
	dc->attriToPos = NEW_MAP(Constant, Constant);
	dc->posToDatatype = NEW_MAP(Constant, Constant);
	dc->isNull = makeVector(VECTOR_NODE, T_Vector);
	dc->isAPSChunk = FALSE;

	return dc;
}

char *
update_ps_incremental(QueryOperator* operator, QueryOperator *updateStmt)
{
	PC = (ProvenanceComputation *) copyObject(operator);
	PS_INFO = createPSInfo((Node *) getStringProperty(operator, PROP_PC_COARSE_GRAINED));

	DEBUG_NODE_BEATIFY_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR: \n", operator);
	INFO_OP_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR", operator);
	strInfo = makeStringInfo();

	INFO_LOG("Start update");
	DEBUG_NODE_BEATIFY_LOG("update stmt", updateStmt);
	updateStatement = updateStmt;


	option_copy_join = getBoolOption(OPTION_UPDATE_PS_COPY_DELTA_JOIN);
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
			updateProvenanceComputation(op);
			break;
		case T_ProjectionOperator:
			updateProjection(op);
			break;
		case T_SelectionOperator:
			updateSelection(op);
			break;
		case T_JoinOperator:
		{
			if (option_copy_join) {
				updateJoin2(op);
			} else {
				updateJoin(op);
			}
		}
			break;
		case T_AggregationOperator:
			updateAggregation(op);
			break;
		case T_TableAccessOperator:
			updateTableAccess(op);
			break;
		case T_DuplicateRemoval:
			updateDuplicateRemoval(op);
			break;
		case T_SetOperator:
			updateSet(op);
			break;
		case T_LimitOperator:
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
	updateByOperators(OP_LCHILD(op));

	QueryOperator *childOp = OP_LCHILD(op);

	if (!HAS_STRING_PROP(childOp, PROP_DATA_CHUNK)) {
		return;
	}

	HashMap *inputChunkMaps = (HashMap *) GET_STRING_PROP(childOp, PROP_DATA_CHUNK);
	DataChunk *inputDCIns = (DataChunk *) MAP_GET_STRING(inputChunkMaps, PROP_DATA_CHUNK_INSERT);
	DataChunk *inputDCDel = (DataChunk *) MAP_GET_STRING(inputChunkMaps, PROP_DATA_CHUNK_DELETE);

	PSMap *storedPSMap = (PSMap *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);

	if (inputDCIns != NULL && inputDCIns->isAPSChunk) {
		FOREACH_HASH_KEY(Constant, c, inputDCIns->fragmentsInfo) {
			HashMap *fragCnt = (HashMap *) MAP_GET_STRING(storedPSMap->fragCount, STRING_VALUE(c));
			boolean isPSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(inputDCIns->fragmentsIsInt, STRING_VALUE(c)));
			Vector *psVec = (Vector *) MAP_GET_STRING(inputDCIns->fragmentsInfo, STRING_VALUE(c));
			if (isPSInt) {
				int *psVals = (int *) VEC_TO_IA(psVec);
				for (int row = 0; row < psVec->length; row++) {
					if (MAP_HAS_INT_KEY(fragCnt, psVals[row])) {
						Constant *c = (Constant *) MAP_GET_INT(fragCnt, psVals[row]);
						incrConst(c);
					} else {
						addToMap(fragCnt, (Node *) createConstInt(psVals[row]), (Node *) createConstInt(1));
					}
				}
			} else {
				BitSet **bitSet = (BitSet **) VEC_TO_ARR(psVec, BitSet);
				for (int row = 0; row < psVec->length; row++) {
					char *psStr = (char *) bitSetToString(bitSet[row]);
					int len = strlen(psStr);
					for (int bitIdx = 0; bitIdx < len; bitIdx++) {
						if (psStr[bitIdx] == '1') {
							if (MAP_HAS_INT_KEY(fragCnt, bitIdx)) {
								Constant *c = (Constant *) MAP_GET_INT(fragCnt, bitIdx);
								incrConst(c);
							} else {
								addToMap(fragCnt, (Node *) createConstInt(bitIdx), (Node *) createConstInt(1));
							}
						}
					}
				}
			}
		}
	}

	if (inputDCDel != NULL && inputDCDel->isAPSChunk) {
		FOREACH_HASH_KEY(Constant, c, inputDCDel->fragmentsInfo) {
			HashMap *fragCnt = (HashMap *) MAP_GET_STRING(storedPSMap->fragCount, STRING_VALUE(c));
			boolean isPSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(inputDCDel->fragmentsIsInt, STRING_VALUE(c)));
			Vector *psVec = (Vector *) MAP_GET_STRING(inputDCDel->fragmentsInfo, STRING_VALUE(c));
			if (isPSInt) {
				int *psVals = (int *) VEC_TO_IA(psVec);
				for (int row = 0; row < psVec->length; row++) {
					Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVals[row]);
					if (INT_VALUE(cnt) > 1) {
						INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
					} else {
						removeMapElem(fragCnt, (Node *) createConstInt(psVals[row]));
					}
				}
			} else {
				BitSet **bitSet = (BitSet **) VEC_TO_ARR(psVec, BitSet);
				for (int row = 0; row < psVec->length; row++) {
					char *psStr = (char *) bitSetToString(bitSet[row]);
					int len = strlen(psStr);
					for (int bitIdx = 0; bitIdx < len; bitIdx++) {
						if (psStr[bitIdx] == '1') {
							Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, bitIdx);
							if (INT_VALUE(cnt) > 1) {
								INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
							} else {
								removeMapElem(fragCnt, (Node *) createConstInt(bitIdx));
							}
						}
					}
				}
			}
		}
	}

	DEBUG_NODE_BEATIFY_LOG("UPDATED PSMAP", storedPSMap);
}

static void
updateProjection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	QueryOperator *childOp = OP_LCHILD(op);

	if (!HAS_STRING_PROP(childOp, PROP_DATA_CHUNK)) {
		return;
	}

	HashMap *chunkMaps = (HashMap *) getStringProperty(childOp, PROP_DATA_CHUNK);
	DataChunk *dataChunkIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
	DataChunk *resultDCIns = NULL;
	if (dataChunkIns != NULL) {
		resultDCIns = initDataChunk();
		resultDCIns->attrNames = (List *) copyObject(op->schema->attrDefs);
		resultDCIns->numTuples = dataChunkIns->numTuples;
		resultDCIns->tupleFields = LIST_LENGTH(op->schema->attrDefs);
		if (dataChunkIns->isAPSChunk) {
			resultDCIns->fragmentsInfo = dataChunkIns->fragmentsInfo;
			resultDCIns->fragmentsIsInt = dataChunkIns->fragmentsIsInt;
			resultDCIns->isAPSChunk = TRUE;
		}
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
		if (dataChunkDel->isAPSChunk) {
			resultDCDel->fragmentsInfo = dataChunkDel->fragmentsInfo;
			resultDCDel->fragmentsIsInt = dataChunkDel->fragmentsIsInt;
			resultDCDel->isAPSChunk = TRUE;
		}
		resultDCDel->updateIdentifier = dataChunkDel->updateIdentifier;
		int pos = 0;
		FOREACH(AttributeDef, ad, op->schema->attrDefs) {
			addToMap(resultDCDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
			addToMap(resultDCDel->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
			pos++;
		}
	}

	/*
	 *  Iterate all elements in projection expression list;
	 *  Case 1: Element is an Attribute;
	 *  Case 2: Element is an Expression;
	 *  Case 3: Element is an Constant;
	 *  case 4: Element is an CaseExpr;
	 */
	List *projExprs = ((ProjectionOperator *) op)->projExprs;
	int exprIdx = 0;
	FOREACH(Node, node, projExprs) {
		DEBUG_NODE_BEATIFY_LOG("EVALUATE IN PROJ EXPR", node);
		if (isA(node, Operator)) {
			if (dataChunkIns != NULL) {
				ColumnChunk *evaluatedValueIns = evaluateExprOnDataChunk(node, dataChunkIns);
				vecAppendNode(resultDCIns->tuples, (Node *) evaluatedValueIns->data.v);
			}
			if (dataChunkDel != NULL) {
				ColumnChunk *evaluatedValueDel = evaluateExprOnDataChunk(node, dataChunkDel);
				vecAppendNode(resultDCDel->tuples, (Node *) evaluatedValueDel->data.v);
			}
		} else if (isA(node, AttributeReference)){
			AttributeReference *ar = (AttributeReference *) node;
			int vecFromPos = ar->attrPosition;
			if (dataChunkIns != NULL) {
				vecAppendNode(resultDCIns->tuples, (Node *) getVecNode(dataChunkIns->tuples, vecFromPos));
			}
			if (dataChunkDel != NULL) {
				vecAppendNode(resultDCDel->tuples, (Node *) getVecNode(dataChunkDel->tuples, vecFromPos));
			}
		} else if (isA(node, Constant)) {
			if (dataChunkIns != NULL) {
				ColumnChunk *cc = getColumnChunkOfConst((Constant *) node, dataChunkIns);
				vecAppendNode(resultDCIns->tuples, (Node *) cc->data.v);
			}
			if (dataChunkDel != NULL) {
				ColumnChunk *cc = getColumnChunkOfConst((Constant *) node, dataChunkDel);
				vecAppendNode(resultDCDel->tuples, (Node *) cc->data.v);
			}
		} else if (isA(node, CastExpr)) {
			CastExpr *ce = (CastExpr *) node;
			DataType dt = ce->resultDT;

			if (dataChunkIns != NULL) {
				ColumnChunk *cc = evaluateExprOnDataChunk(ce->expr, dataChunkIns);
				if (cc->dataType != dt) {
					INFO_LOG("before cast: %d", cc->dataType);
					cc = castColumnChunk(cc, cc->dataType, dt);
				}
				vecAppendNode(resultDCIns->tuples, (Node *) cc->data.v);
			}
			if (dataChunkDel != NULL) {
				ColumnChunk *cc = evaluateExprOnDataChunk(ce->expr, dataChunkDel);
				if (cc->dataType != dt) {
					INFO_LOG("before cast: %d", cc->dataType);
					cc = castColumnChunk(cc, cc->dataType, dt);
				}
				vecAppendNode(resultDCDel->tuples, (Node *) cc->data.v);
			}
		} else if (isA(node, FunctionCall)) {
			INFO_LOG("PROJ FC");
			if (dataChunkIns != NULL) {
				ColumnChunk *cc = evaluateExprOnDataChunk(node, dataChunkIns);
				vecAppendNode(resultDCIns->tuples, (Node *) cc->data.v);
			}

			if (dataChunkDel != NULL) {
				ColumnChunk *cc = evaluateExprOnDataChunk(node, dataChunkDel);
				vecAppendNode(resultDCDel->tuples, (Node *) cc->data.v);
			}
		} if (isA(node, CaseExpr)) {
			// DEBUG_NODE_BEATIFY_LOG("case when", node);

			List *caseWhens = ((CaseExpr *) node)->whenClauses;
			int caseNums = LIST_LENGTH(caseWhens);

			// Vector *allCaseWhenValues = makeVector(VECTOR_NODE, T_Vector);
			Vector *evalCasesIns = makeVector(VECTOR_NODE, T_Vector);
			Vector *evalThensIns = makeVector(VECTOR_NODE, T_Vector);
			Vector *evalCasesDel = makeVector(VECTOR_NODE, T_Vector);
			Vector *evalThensDel = makeVector(VECTOR_NODE, T_Vector);
			FOREACH(CaseWhen, cw, caseWhens) {
				if (dataChunkIns != NULL) {
					ColumnChunk *evalCases = evaluateExprOnDataChunk((Node *) cw->when, dataChunkIns);
					vecAppendNode(evalCasesIns, (Node *) evalCases->data.v);
					ColumnChunk *evalThens = evaluateExprOnDataChunk((Node *) cw->then, dataChunkIns);
					vecAppendNode(evalThensIns, (Node *) evalThens->data.v);
				}

				if (dataChunkDel != NULL) {
					ColumnChunk *evalCases = evaluateExprOnDataChunk((Node *) cw->when, dataChunkDel);
					vecAppendNode(evalCasesDel, (Node *) evalCases->data.v);
					ColumnChunk *evalThens = evaluateExprOnDataChunk((Node *) cw->then, dataChunkDel);
					vecAppendNode(evalThensDel, (Node *) evalThens->data.v);
				}
			}
			Vector *evalElsesIns = NULL;
			Vector *evalElsesDel = NULL;
			if (dataChunkIns != NULL) {
				ColumnChunk *evalElses = evaluateExprOnDataChunk(((CaseExpr *) node)->elseRes, dataChunkIns);
				// vecAppendNode(evalElsesIns, evalElses);
				evalElsesIns = evalElses->data.v;
			}

			if (dataChunkDel != NULL) {
				ColumnChunk *evalElses = evaluateExprOnDataChunk(((CaseExpr *) node)->elseRes, dataChunkDel);
				// vecAppendNode(evalElsesDel, evalElses);
				evalElsesDel = evalElses->data.v;
			}

			if (dataChunkIns != NULL) {
				DEBUG_NODE_BEATIFY_LOG("ins case", evalCasesIns);
				DEBUG_NODE_BEATIFY_LOG("ins then", evalThensIns);
				DEBUG_NODE_BEATIFY_LOG("ins else", evalElsesIns);
            }
			if (dataChunkDel != NULL) {
				DEBUG_NODE_BEATIFY_LOG("ins case", evalCasesDel);
				DEBUG_NODE_BEATIFY_LOG("ins then", evalThensDel);
				DEBUG_NODE_BEATIFY_LOG("ins else", evalElsesDel);
			}
			// based on case evaluation result to decide which one need to project;
			DataType resDT = ((AttributeDef *) getNthOfListP(op->schema->attrDefs, exprIdx))->dataType;
			if (dataChunkIns != NULL) {
				ColumnChunk *resCC = makeColumnChunk(resDT, dataChunkIns->numTuples);
				switch (resDT) {
					case DT_BOOL:
					case DT_INT:
					{
						int *val = VEC_TO_IA(resCC->data.v);
						// check which when satisfies
						for (int row = 0; row < dataChunkIns->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesIns, caseWhenId), row) == 1) {
									val[row] = getVecInt((Vector *) getVecNode(evalThensIns, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecInt(evalElsesIns, row);
							}
						}
					}
					break;
					case DT_LONG:
					{
						gprom_long_t *val = VEC_TO_LA(resCC->data.v);
						for (int row = 0; row < dataChunkIns->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesIns, caseWhenId), row) == 1) {
									val[row] = getVecLong((Vector *) getVecNode(evalThensIns, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecLong(evalElsesIns, row);
							}
						}
					}
					break;
					case DT_FLOAT:
					{
						double *val = VEC_TO_FA(resCC->data.v);
						for (int row = 0; row < dataChunkIns->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesIns, caseWhenId), row) == 1) {
									val[row] = getVecFloat((Vector *) getVecNode(evalThensIns, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecFloat(evalElsesIns, row);
							}
						}
					}
					break;
					case DT_STRING:
					case DT_VARCHAR2:
					{
						char **val = (char **) VEC_TO_ARR(resCC->data.v, char);
						for (int row = 0; row < dataChunkIns->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesIns, caseWhenId), row) == 1) {
									val[row] = getVecString((Vector *) getVecNode(evalThensIns, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecString(evalElsesIns, row);
							}
						}
					}
					break;
					default:
						FATAL_LOG("not supprot this type");
				}
				vecAppendNode(resultDCIns->tuples, (Node *) resCC->data.v);
			}
			if (dataChunkDel != NULL) {
				ColumnChunk *resCC = makeColumnChunk(resDT, dataChunkDel->numTuples);
				switch (resDT) {
					case DT_BOOL:
					case DT_INT:
					{
						int *val = VEC_TO_IA(resCC->data.v);
						// check which when satisfies
						for (int row = 0; row < dataChunkDel->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesDel, caseWhenId), row) == 1) {
									val[row] = getVecInt((Vector *) getVecNode(evalThensDel, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecInt(evalElsesDel, row);
							}
						}
					}
					break;
					case DT_LONG:
					{
						gprom_long_t *val = VEC_TO_LA(resCC->data.v);
						for (int row = 0; row < dataChunkDel->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesDel, caseWhenId), row) == 1) {
									val[row] = getVecLong((Vector *) getVecNode(evalThensDel, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecLong(evalElsesDel, row);
							}
						}
					}
					break;
					case DT_FLOAT:
					{
						double *val = VEC_TO_FA(resCC->data.v);
						// check which when satisfies
						for (int row = 0; row < dataChunkDel->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesDel, caseWhenId), row) == 1) {
									val[row] = getVecFloat((Vector *) getVecNode(evalThensDel, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecFloat(evalElsesDel, row);
							}
						}
					}
					break;
					case DT_STRING:
					case DT_VARCHAR2:
					{
						char **val = (char **) VEC_TO_ARR(resCC->data.v, char);
						// check which when satisfies
						for (int row = 0; row < dataChunkDel->numTuples; row++) {
							boolean findInWhen = FALSE;
							for (int caseWhenId = 0; caseWhenId < caseNums; caseWhenId++) {
								if (getVecInt((Vector *) getVecNode(evalCasesDel, caseWhenId), row) == 1) {
									val[row] = getVecString((Vector *) getVecNode(evalThensDel, caseWhenId), row);
									findInWhen = TRUE;
									break;
								}
							}

							if (!findInWhen) {
								val[row] = getVecString(evalElsesDel, row);
							}
						}
					}
					break;
					default:
						FATAL_LOG("not supprot this type");
				}
				vecAppendNode(resultDCDel->tuples, (Node *) resCC->data.v);
			}
		}
		exprIdx++;
	}

	// Create output data chunk;
	HashMap *resChunkMaps = NEW_MAP(Constant, Node);
	if (resultDCIns != NULL) {
		MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_INSERT, resultDCIns);
	}
	if (resultDCDel != NULL) {
		MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_DELETE, resultDCDel);
		// addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) resultDCDel);
	}

	if (mapSize(resChunkMaps) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, (Node *) resChunkMaps);
	}
	DEBUG_NODE_BEATIFY_LOG("projection output chunks", resChunkMaps);
	// remove child data chunk;
	removeStringProperty(childOp, PROP_DATA_CHUNK);
}

static void
updateSelection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	// check child's delta chunk;
	if (!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		return;
	}

	boolean updatePSSelPD = getBoolOption(OPTION_UPDATE_PS_SELECTION_PUSH_DOWN);
	if (updatePSSelPD) {
		// return only if this is a selection just above table access operator;
		if (isA(OP_LCHILD(op), TableAccessOperator)) {
			INFO_LOG("SELECTION PUSH DOWN, DIRECTY COPY DATACHUNK AND RETURN");
			DEBUG_NODE_BEATIFY_LOG("SELECTION OPERATOR COND:", ((SelectionOperator *) op)->cond);
			SET_STRING_PROP(op, PROP_DATA_CHUNK, GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK));
			removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
			DEBUG_NODE_BEATIFY_LOG("SELECTION DC", GET_STRING_PROP(op, PROP_DATA_CHUNK));
			return;
		}
	}

	HashMap *chunkMaps = (HashMap *) GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK);

	Node * selCond = ((SelectionOperator *) op)->cond;
	HashMap *resChunkMaps = NEW_MAP(Constant, Node);

	// insert chunk;
	DataChunk *dataChunkIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
	if (dataChunkIns != NULL) {
		DataChunk *dcInsFilter = filterDataChunk(dataChunkIns, selCond);

		if (dcInsFilter->numTuples > 0) {
			addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) dcInsFilter);
		}
	}

	// delete chunk;
	DataChunk *dataChunkDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
	if (dataChunkDel != NULL) {
		DataChunk *dcDelFilter = filterDataChunk(dataChunkDel, selCond);

		if (dcDelFilter->numTuples > 0) {
			addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) dcDelFilter);
		}
	}

	// check if this selection op will have chunks;
	if (mapSize(resChunkMaps) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, resChunkMaps);
	}

	// remove child's chunks;
	removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
	DEBUG_NODE_BEATIFY_LOG("selection operator output chunks", resChunkMaps);
}

static void
updateJoinByGroupJoin(QueryOperator *op) {
	// for test purpose, now we have two level join;
	QueryOperator *thisJoinRewrite = captureRewriteOp((ProvenanceComputation *) copyObject(PC), (QueryOperator *) copyObject(op));

	INFO_OP_LOG("join rewrite in grou join", thisJoinRewrite);

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(thisJoinRewrite), PROP_LEVEL_AGGREGATION_MARK), 0);

	QueryOperator *lChild = OP_LCHILD(op);
	QueryOperator *rChild = OP_RCHILD(op);
	QueryOperator *childJoin = NULL;
	if (isA(lChild, JoinOperator)) {
		childJoin = lChild;
	} else if (isA(rChild, JoinOperator)) {
		childJoin = rChild;
	}

	// prepare the ConstRelMultiListsOperator for join branchs;
	ConstRelMultiListsOperator *leftDelta = NULL;
	// ConstRelMultiListsOperator *rightDelta = NULL;


	// initialize result data chunks;
	DataChunk *resDCIns = initDataChunk();
	DataChunk *resDCDel = initDataChunk();

	// initialize chunks' tuples vectors based on the data type;
	int pos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		addToMap(resDCIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resDCDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));

		addToMap(resDCIns->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
		addToMap(resDCDel->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));

		switch (ad->dataType) {
			case DT_INT:
			case DT_BOOL:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
			}
				break;
			case DT_LONG:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
			}
				break;
			case DT_FLOAT:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		pos++;
	}

	// set fields like: tupleFields, attrNames
	resDCIns->tupleFields = pos;
	resDCDel->tupleFields = pos;
	resDCIns->attrNames = (List *) copyObject(op->schema->attrDefs);
	resDCDel->attrNames = (List *) copyObject(op->schema->attrDefs);
	QueryOperator *cOp = captureRewriteOp((ProvenanceComputation *) copyObject(PC), (QueryOperator *) copyObject(childJoin));
	// int deltaBranches = 0;
	if (HAS_STRING_PROP(OP_LCHILD(childJoin), PROP_DATA_CHUNK)) {
		HashMap *chunkMaps = (HashMap *) GET_STRING_PROP(OP_LCHILD(childJoin), PROP_DATA_CHUNK);
		DataChunk *leftDCIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *leftDCDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
		List *provAttrDefs = getProvenanceAttrDefs(OP_LCHILD(OP_LCHILD(cOp)));
		leftDelta = createConstRelMultiListsFromDataChunk(leftDCIns, leftDCDel, TRUE, singleton(childJoin),provAttrDefs);
	}

	DEBUG_NODE_BEATIFY_LOG("left delta", leftDelta);
	DEBUG_NODE_BEATIFY_LOG("GROUP JOIN TOP", thisJoinRewrite);
	INFO_OP_LOG("GROUP JOIN TOP", thisJoinRewrite);

	QueryOperator *join0 = OP_LCHILD(thisJoinRewrite);
	join0->provAttrs = NIL;
	QueryOperator *proj0 = OP_LCHILD(join0);
	proj0->provAttrs = NIL;
	QueryOperator *join1 = OP_LCHILD(proj0);
	join1->provAttrs = NIL;
	replaceNode(join1->inputs, OP_LCHILD(join1), leftDelta);

	DEBUG_NODE_BEATIFY_LOG("GROUP JOIN TOP after replace", thisJoinRewrite);
	INFO_OP_LOG("GROUP JOIN TOP after replace", thisJoinRewrite);

	char *sql = serializeQuery(thisJoinRewrite);

	INFO_LOG("sql sql %s", sql);
	postgresGetDataChunkGroupJoin(sql, resDCIns, resDCDel, -1, psAttrAndLevel);
	// Relation *rel = executeQuery(sql);
	// DEBUG_NODE_BEATIFY_LOG("vec", rel->tuples);
	HashMap *resChunkMaps = NEW_MAP(Constant, Node);
	if (resDCIns->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_INSERT, resDCIns);
	}

	if (resDCDel->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_DELETE, resDCDel);
	}

	if (mapSize(resChunkMaps) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, resChunkMaps);
	}

}

// delta use COPY
static void
updateJoin2(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));

	// check children's delta
	if ((!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK))
	 && (!HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK))) {
		return;
	}

	QueryOperator *rewrOp = captureRewriteOp((ProvenanceComputation *) copyObject(PC), (QueryOperator *) copyObject(op));

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOp), PROP_LEVEL_AGGREGATION_MARK), 0);

	// build result data chunks;
	DataChunk *resDCIns = initDataChunk();
	DataChunk *resDCDel = initDataChunk();
	int pos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		addToMap(resDCIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resDCDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));

		addToMap(resDCIns->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
		addToMap(resDCDel->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));

		switch (ad->dataType) {
			case DT_INT:
			case DT_BOOL:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
			}
				break;
			case DT_LONG:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
			}
				break;
			case DT_FLOAT:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		pos++;
	}

	resDCIns->tupleFields = pos;
	resDCDel->tupleFields = pos;
	resDCIns->attrNames = (List *) copyObject(op->schema->attrDefs);
	resDCDel->attrNames = (List *) copyObject(op->schema->attrDefs);

	int deltaBranches = 0;
	TableAccessOperator *leftDelta = NULL;
	TableAccessOperator *rightDelta = NULL;

	boolean hasLeftDelta = FALSE;
	boolean hasRightDelta = FALSE;
	boolean hasBloomOptionApplied = getBoolOption(OPTION_UPDATE_PS_JOIN_USING_BF);
	boolean isLeftFailPassBF = TRUE;
	boolean isRightFailPassBF = TRUE;
	if (HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		deltaBranches++;
		// hasLeftDelta = TRUE;
		HashMap *leftDC = (HashMap *) getStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
		List *deltaInfos = NIL;
		if (hasBloomOptionApplied) {
			HashMap *bloomStates = (HashMap *) getStringProperty(op, PROP_DATA_STRUCTURE_JOIN);

			HashMap *rightBloom = (HashMap *) MAP_GET_STRING(bloomStates, JOIN_RIGHT_BLOOM);

			HashMap *leftMapping = (HashMap *) MAP_GET_STRING(bloomStates, JOIN_LEFT_BLOOM_ATT_MAPPING);

			Vector *DCInsCheck = NULL;
			if (MAP_HAS_STRING_KEY(leftDC, PROP_DATA_CHUNK_INSERT)) {
				DataChunk *dcIns = (DataChunk *) MAP_GET_STRING(leftDC, PROP_DATA_CHUNK_INSERT);
				int tupleLen = dcIns->numTuples;
				DCInsCheck = makeVectorOfSize(VECTOR_INT, T_Vector, tupleLen);
				DCInsCheck->length = tupleLen;
				int *checkValue = VEC_TO_IA(DCInsCheck);
				// check each equal join attri in left mapping;
				FOREACH_HASH_KEY(Constant, c, leftMapping) {
					Vector *mapping = (Vector *) MAP_GET_STRING(leftMapping, STRING_VALUE(c));

					int pos = INT_VALUE((Constant *) MAP_GET_STRING(dcIns->attriToPos, STRING_VALUE(c)));
					DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(dcIns->posToDatatype, pos));

					Vector *vec = (Vector *) getVecNode(dcIns->tuples, pos);

					FOREACH_VEC(char, attName, mapping) {
						INFO_LOG("mapping attr: %s", attName);
						Bloom *bloom = (Bloom *) MAP_GET_STRING(rightBloom, attName);

						switch (dt) {
							case DT_BOOL:
							case DT_INT:
							{
								int *value = VEC_TO_IA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(int));
								}
							}
							break;
							case DT_LONG:
							{
								gprom_long_t *value = VEC_TO_LA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(gprom_long_t));
								}
							}
							break;
							case DT_FLOAT:
							{
								double *value = VEC_TO_FA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(double));
								}
							}
							break;
							case DT_VARCHAR2:
							case DT_STRING:
							{
								char **value = VEC_TO_ARR(vec, char);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, value[row], sizeof(strlen(value[row])));
								}
							}
							break;
						}
					}
				}
			}
			Vector *DCDelCheck = NULL;
			if (MAP_HAS_STRING_KEY(leftDC, PROP_DATA_CHUNK_DELETE)) {
				DataChunk *dcDel = (DataChunk *) MAP_GET_STRING(leftDC, PROP_DATA_CHUNK_DELETE);
				int tupleLen = dcDel->numTuples;
				INFO_LOG("tuple number: %d", tupleLen);

				DCDelCheck = makeVectorOfSize(VECTOR_INT, T_Vector, tupleLen);
				DCDelCheck->length = tupleLen;
				int *checkValue = VEC_TO_IA(DCDelCheck);

				FOREACH_HASH_KEY(Constant, c, leftMapping) {
					Vector *mapping = (Vector *) MAP_GET_STRING(leftMapping, STRING_VALUE(c));

					int pos = INT_VALUE((Constant *) MAP_GET_STRING(dcDel->attriToPos, STRING_VALUE(c)));
					DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(dcDel->posToDatatype, pos));

					Vector *vec = (Vector *) getVecNode(dcDel->tuples, pos);

					FOREACH_VEC(char, attName, mapping) {
						INFO_LOG("mapping attr: %s", attName);
						Bloom *bloom = (Bloom *) MAP_GET_STRING(rightBloom, attName);

						switch (dt) {
							case DT_BOOL:
							case DT_INT:
							{
								int *value = VEC_TO_IA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(int));
								}
							}
							break;
							case DT_LONG:
							{
								gprom_long_t *value = VEC_TO_LA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(gprom_long_t));
								}
							}
							break;
							case DT_FLOAT:
							{
								double *value = VEC_TO_FA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(double));
								}
							}
							break;
							case DT_VARCHAR2:
							case DT_STRING:
							{
								char **value = VEC_TO_ARR(vec, char);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, value[row], sizeof(strlen(value[row])));
								}
							}
							break;
						}
					}
				}
			}

			DEBUG_NODE_BEATIFY_LOG("INSCheck", DCInsCheck);
			DEBUG_NODE_BEATIFY_LOG("DELCheck", DCDelCheck);

			deltaInfos = postgresCopyDeltaToDBWithBF(leftDC, JOIN_LEFT_BRANCH_DELTA_TABLE, JOIN_LEFT_BRANCH_IDENTIFIER, DCInsCheck, DCDelCheck);

			if (deltaInfos) {
				isLeftFailPassBF = FALSE;
			}
		} else {
			deltaInfos = postgresCopyDeltaToDB(leftDC, JOIN_LEFT_BRANCH_DELTA_TABLE, JOIN_LEFT_BRANCH_IDENTIFIER);
		}

		if (deltaInfos) {
			hasLeftDelta = TRUE;
			QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);

			leftDelta = createTableAccessOp(strdup(JOIN_LEFT_BRANCH_DELTA_TABLE), NULL, strdup(JOIN_LEFT_BRANCH_DELTA_TABLE), singleton(OP_LCHILD(cOp)), getAttrDefNames(deltaInfos), getAttrDataTypes(deltaInfos));
			INFO_OP_LOG("LEFT DELTA ", leftDelta);
			INFO_OP_LOG("copy ccc", cOp);
			// List *provAttrDefs = getProvenanceAttrDefs(OP_LCHILD(OP_LCHILD(cOp)));

			// create a project
			int pos = 0;
			List *attrDefs = (List *) copyObject(deltaInfos);
			List *projExpr = NIL;
			// append left child
			FOREACH(AttributeDef, ad, deltaInfos) {
				projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 0, pos++, 0, ad->dataType));
			}
			List *allDefs = OP_LCHILD(cOp)->schema->attrDefs;
			// start from a postion to append all attrdef of join op, which will append all right child attrdefs, note the start position(-1, since there we create a identifier)
			int startIdx = LIST_LENGTH(((QueryOperator *) leftDelta)->schema->attrDefs) - 1;
			int idx = 0;
			FOREACH(AttributeDef, ad, allDefs) {
				if (idx++ < startIdx) {
					continue;
				}
				attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
				projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 1, pos++, 0, ad->dataType));
			}

			// replace with new attrDefs
			OP_LCHILD(cOp)->schema->attrDefs = attrDefs;
			cOp->schema->attrDefs = attrDefs;
			((ProjectionOperator *) cOp)->projExprs = projExpr;
			OP_LCHILD(cOp)->provAttrs = NIL;
			cOp->provAttrs = NIL;

			List *inputs = NIL;
			inputs = appendToTailOfList(inputs, leftDelta);
			// ((QueryOperator *) leftDelta)->parents = singleton(OP_LCHILD(cOp));
			inputs = appendToTailOfList(inputs, OP_RCHILD(OP_LCHILD(cOp)));
			OP_LCHILD(cOp)->inputs = inputs;

			// replaceNode(OP_LCHILD(cOp)->inputs, OP_LCHILD(OP_LCHILD(cOp)), leftDelta);
			INFO_OP_LOG("new Join OP", cOp);
			// FOREACH(AttributeDef, ad, )

			char *sql = serializeQuery(cOp);
			INFO_LOG("left delta join %s", sql);
			postgresGetDataChunkJoin(sql, resDCIns, resDCDel, -1, psAttrAndLevel);
		}
	}

	if (HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		deltaBranches++;

		// hasRightDelta = TRUE;

		HashMap *rightDC = (HashMap *) getStringProperty(OP_RCHILD(op), PROP_DATA_CHUNK);
		List *deltaInfos = NIL;
		if (hasBloomOptionApplied) {
			HashMap *bloomStates = (HashMap *) getStringProperty(op, PROP_DATA_STRUCTURE_JOIN);

			HashMap *leftBloom = (HashMap *) MAP_GET_STRING(bloomStates, JOIN_LEFT_BLOOM);

			HashMap *rightMapping = (HashMap *) MAP_GET_STRING(bloomStates, JOIN_RIGHT_BLOOM_ATT_MAPPING);

			Vector *DCInsCheck = NULL;
			if (MAP_HAS_STRING_KEY(rightDC, PROP_DATA_CHUNK_INSERT)) {
				DataChunk *dcIns = (DataChunk *) MAP_GET_STRING(rightDC, PROP_DATA_CHUNK_INSERT);

				int tupleLen = dcIns->numTuples;

				DCInsCheck = makeVectorOfSize(VECTOR_INT, T_Vector, tupleLen);
				DCInsCheck->length = tupleLen;

				int *checkValue = VEC_TO_IA(DCInsCheck);

				FOREACH_HASH_KEY(Constant, c, rightMapping) {
					Vector *mapping = (Vector *) MAP_GET_STRING(rightMapping, STRING_VALUE(c));

					int pos = INT_VALUE((Constant *) MAP_GET_STRING(dcIns->attriToPos, STRING_VALUE(c)));
					DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(dcIns->posToDatatype, pos));

					Vector *vec = (Vector *) getVecNode(dcIns->tuples, pos);

					FOREACH_VEC(char, attName, mapping) {
						Bloom *bloom = (Bloom *) MAP_GET_STRING(leftBloom, attName);
						switch (dt) {
							case DT_BOOL:
							case DT_INT:
							{
								int *value = VEC_TO_IA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(int));
								}
							}
							break;
							case DT_LONG:
							{
								gprom_long_t *value = VEC_TO_LA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(gprom_long_t));
								}
							}
							break;
							case DT_FLOAT:
							{
								double *value = VEC_TO_FA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(double));
								}
							}
							break;
							case DT_VARCHAR2:
							case DT_STRING:
							{
								char **value = VEC_TO_ARR(vec, char);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, value[row], sizeof(strlen(value[row])));
								}
							}
							break;
						}
					}
				}
			}
			Vector *DCDelCheck = NULL;
			if (MAP_HAS_STRING_KEY(rightDC, PROP_DATA_CHUNK_DELETE)) {
				DataChunk *dcDel = (DataChunk *) MAP_GET_STRING(rightDC, PROP_DATA_CHUNK_DELETE);

				int tupleLen = dcDel->numTuples;

				DCDelCheck = makeVectorOfSize(VECTOR_INT, T_Vector, tupleLen);
				DCDelCheck->length = tupleLen;

				int *checkValue = VEC_TO_IA(DCDelCheck);

				FOREACH_HASH_KEY(Constant, c, rightMapping) {
					Vector *mapping = (Vector *) MAP_GET_STRING(rightMapping, STRING_VALUE(c));

					int pos = INT_VALUE((Constant *) MAP_GET_STRING(dcDel->attriToPos, STRING_VALUE(c)));
					DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(dcDel->posToDatatype, pos));

					Vector *vec = (Vector *) getVecNode(dcDel->tuples, pos);

					FOREACH_VEC(char, attName, mapping) {
						Bloom *bloom = (Bloom *) MAP_GET_STRING(leftBloom, attName);
						switch(dt) {
							case DT_BOOL:
							case DT_INT:
							{
								int *value = VEC_TO_IA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(int));
								}
							}
							break;
							case DT_LONG:
							{
								gprom_long_t *value = VEC_TO_LA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(gprom_long_t));
								}
							}
							break;
							case DT_FLOAT:
							{
								double *value = VEC_TO_FA(vec);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, &value[row], sizeof(double));
								}
							}
							break;
							case DT_VARCHAR2:
							case DT_STRING:
							{
								char **value = VEC_TO_ARR(vec, char);
								for (int row = 0; row < tupleLen; row++) {
									checkValue[row] = bloom_check(bloom, value[row], sizeof(strlen(value[row])));
								}
							}
							break;

						}
					}
				}
			}

			deltaInfos = postgresCopyDeltaToDBWithBF(rightDC, JOIN_RIGHT_BRANCH_DELTA_TABLE, JOIN_RIGHT_BRANCH_IDENTIFIER, DCInsCheck, DCDelCheck);

			if (deltaInfos) {
				isRightFailPassBF = FALSE;
			}
		} else {
			deltaInfos = postgresCopyDeltaToDB(rightDC, JOIN_RIGHT_BRANCH_DELTA_TABLE, JOIN_RIGHT_BRANCH_IDENTIFIER);
		}

		if (deltaInfos) {
			hasRightDelta = TRUE;

			DEBUG_NODE_BEATIFY_LOG("deltaInfos", deltaInfos);

			QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);
			INFO_OP_LOG("before cOp", cOp);

			rightDelta = createTableAccessOp(strdup(JOIN_RIGHT_BRANCH_DELTA_TABLE), NULL, strdup(JOIN_RIGHT_BRANCH_DELTA_TABLE), singleton(OP_LCHILD(cOp)), getAttrDefNames(deltaInfos), getAttrDataTypes(deltaInfos));

			OP_LCHILD(cOp)->schema->attrDefs = appendToTailOfList(OP_LCHILD(cOp)->schema->attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));

			((ProjectionOperator *) cOp)->projExprs = appendToTailOfList(((ProjectionOperator *) cOp)->projExprs, createFullAttrReference(JOIN_RIGHT_BRANCH_IDENTIFIER, 1, LIST_LENGTH(OP_LCHILD(cOp)->schema->attrDefs) - 1, 0, DT_INT));

			cOp->schema->attrDefs = appendToTailOfList(cOp->schema->attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));

			cOp->provAttrs = NIL;
			OP_LCHILD(cOp)->provAttrs = NIL;

			List *inputs = NIL;
			inputs = appendToTailOfList(inputs, OP_LCHILD(OP_LCHILD(cOp)));
			inputs = appendToTailOfList(inputs, rightDelta);
			OP_LCHILD(cOp)->inputs = inputs;
			// OP_LCHILD(cOp)->inputs = replaceNode(OP_LCHILD(cOp)->inputs, OP_RCHILD(OP_LCHILD(cOp)), rightDelta);
			INFO_OP_LOG("create join op", cOp);
			char *sql = serializeQuery(cOp);
			INFO_LOG("create right join %s", sql);
			postgresGetDataChunkJoin(sql, resDCIns, resDCDel, 1, psAttrAndLevel);
		}
	}

	// if two branch both contains delta tuples:
	// 1. if applied BF, both branch must have delta table created.
	// 2. BF not applied, only make sure both branches have delta tuples;
	INFO_LOG("hasBloom :%d, isLeftFail: %d, isRightFail: %d, deltaBranchs: %d\n", hasBloomOptionApplied, isLeftFailPassBF, isRightFailPassBF, deltaBranches);
	if ((hasBloomOptionApplied && !isLeftFailPassBF && !isRightFailPassBF)
	|| (!hasBloomOptionApplied && deltaBranches == 2)){
	// if (deltaBranches == 2) {
		QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);
		List *attrDefs = NIL;
		List *projExpr = NIL;
		int pos = 0;
		FOREACH(AttributeDef, ad, ((QueryOperator *) leftDelta)->schema->attrDefs) {
			attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 0, pos++, 0, ad->dataType));
		}
		FOREACH(AttributeDef, ad, ((QueryOperator *) rightDelta)->schema->attrDefs) {
			attrDefs = appendToTailOfList(attrDefs, copyObject(ad));
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(ad->attrName, 1, pos++, 0, ad->dataType));
		}

		OP_LCHILD(cOp)->schema->attrDefs = attrDefs;
		((ProjectionOperator *) cOp)->projExprs = projExpr;
		cOp->schema->attrDefs = attrDefs;
		cOp->provAttrs = NIL;
		OP_LCHILD(cOp)->provAttrs = NIL;

		List* inputs = NIL;
		appendToTailOfList(inputs, leftDelta);
		appendToTailOfList(inputs, rightDelta);
		OP_LCHILD(cOp)->inputs = inputs;

		char *sql = serializeQuery(cOp);
		postgresGetDataChunkJoin(sql, resDCIns, resDCDel, 0, psAttrAndLevel);
	}

	// clear temporal delta tables
	if (hasLeftDelta) {
		postgresDropTemporalDeltaTable(JOIN_LEFT_BRANCH_DELTA_TABLE);
		// StringInfo dropTable = makeStringInfo();
		// appendStringInfo(dropTable, "drop table if exists %s", JOIN_LEFT_BRANCH_DELTA_TABLE);
		// executeQuery(dropTable->data);
		// executeQuery(CONCAT_STRINGS("DROP TABLE IF EXISTS ", JOIN_LEFT_BRANCH_DELTA_TABLE));

	}

	if (hasRightDelta) {
		postgresDropTemporalDeltaTable(JOIN_RIGHT_BRANCH_DELTA_TABLE);
	}

	// build final data chunks;
	HashMap *resChunks = NEW_MAP(Constant, Node);
	if (resDCIns->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunks, PROP_DATA_CHUNK_INSERT, resDCIns);
	}
	if (resDCDel->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunks, PROP_DATA_CHUNK_DELETE, resDCDel);
	}

	if (mapSize(resChunks) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, (Node *) resChunks);
	}

	// remove children's data chunk;
	if (HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
	}
	if (HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		removeStringProperty(OP_RCHILD(op), PROP_DATA_CHUNK);
	}

	DEBUG_NODE_BEATIFY_LOG("insChunk", resDCIns);
	DEBUG_NODE_BEATIFY_LOG("delChunk", resDCDel);
}


static void
updateJoin(QueryOperator * op)
{
	// update two child operators;
	updateByOperators(OP_LCHILD(op));
    updateByOperators(OP_RCHILD(op));

	boolean groupJoinOption = getBoolOption(OPTION_UPDATE_PS_GROUP_JOIN);
	if (groupJoinOption) {
		int joinNumber = INT_VALUE((Constant *) GET_STRING_PROP(op, "JOIN_NUMBER_FROM_TOP"));
		if (joinNumber == 0) {
			updateJoinByGroupJoin(op);
			return;
		}
		// return;
	}

	// both children operators don't contain delta tuples;
	if ((!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK))
	 && (!HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK))) {
		return;
	}

	INFO_OP_LOG("join operator before rewrite", op);
	QueryOperator *rewrOp = captureRewriteOp((ProvenanceComputation *) copyObject(PC), (QueryOperator *) copyObject(op));
	INFO_OP_LOG("join operator after rewrite", rewrOp);

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOp), PROP_LEVEL_AGGREGATION_MARK), 0);

	DEBUG_NODE_BEATIFY_LOG("WHAT IS HASH MAP", psAttrAndLevel);

	// prepare the ConstRelMultiListsOperator for join branchs;
	ConstRelMultiListsOperator *leftDelta = NULL;
	ConstRelMultiListsOperator *rightDelta = NULL;

	// initialize result data chunks;
	DataChunk *resDCIns = initDataChunk();
	DataChunk *resDCDel = initDataChunk();

	// initialize chunks' tuples vectors based on the data type;
	int pos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		addToMap(resDCIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resDCDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));

		addToMap(resDCIns->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
		addToMap(resDCDel->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));

		switch (ad->dataType) {
			case DT_INT:
			case DT_BOOL:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
			}
				break;
			case DT_LONG:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
			}
				break;
			case DT_FLOAT:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
			}
				break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
			}
				break;
			default:
				FATAL_LOG("not supported");
		}
		pos++;
	}

	// set fields like: tupleFields, attrNames
	resDCIns->tupleFields = pos;
	resDCDel->tupleFields = pos;
	resDCIns->attrNames = (List *) copyObject(op->schema->attrDefs);
	resDCDel->attrNames = (List *) copyObject(op->schema->attrDefs);

	int deltaBranches = 0;
	if (HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		deltaBranches++;

		QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);
		// get data chunks from left branch;
		HashMap *chunkMaps = (HashMap *) getStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
		DataChunk *leftDCIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *leftDCDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);

		List *provAttrDefs = getProvenanceAttrDefs(OP_LCHILD(OP_LCHILD(cOp)));
		leftDelta = createConstRelMultiListsFromDataChunk(leftDCIns, leftDCDel, TRUE, singleton(OP_LCHILD(cOp)), provAttrDefs);

		DEBUG_NODE_BEATIFY_LOG("what is left data", leftDelta);

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
		cOp->provAttrs = NIL;
		OP_LCHILD(cOp)->provAttrs = NIL;

		replaceNode(OP_LCHILD(cOp)->inputs, OP_LCHILD(OP_LCHILD(cOp)), leftDelta);

		DEBUG_NODE_BEATIFY_LOG(" create left branch ", leftDelta);

		DEBUG_NODE_BEATIFY_LOG("rewr join", cOp);
		INFO_OP_LOG("rewr join", cOp);

		char *sql = serializeQuery(cOp);
		postgresGetDataChunkJoin(sql, resDCIns, resDCDel, -1, psAttrAndLevel);
		INFO_LOG("SQL is : %s", sql);
	}

	// right branch is a delta inputs;
	if (HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		deltaBranches++;

		QueryOperator *cOp = (QueryOperator *) copyObject(rewrOp);

		// get data chunks from right branch;
		HashMap *chunkMaps = (HashMap *) getStringProperty(OP_RCHILD(op), PROP_DATA_CHUNK);
		DataChunk *rightDCIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *rightDCDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);

		List *provAttrDefs = getProvenanceAttrDefs(OP_RCHILD(OP_LCHILD(cOp)));
		rightDelta = createConstRelMultiListsFromDataChunk(rightDCIns, rightDCDel, FALSE, singleton(OP_LCHILD(cOp)), provAttrDefs);

		OP_LCHILD(cOp)->schema->attrDefs = appendToTailOfList(OP_LCHILD(cOp)->schema->attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));
		((ProjectionOperator *) cOp)->projExprs = appendToTailOfList(((ProjectionOperator *) cOp)->projExprs, createFullAttrReference(JOIN_RIGHT_BRANCH_IDENTIFIER, 1, LIST_LENGTH(OP_LCHILD(cOp)->schema->attrDefs) - 1,0, DT_INT));
		cOp->schema->attrDefs = appendToTailOfList(cOp->schema->attrDefs, createAttributeDef(JOIN_RIGHT_BRANCH_IDENTIFIER, DT_INT));
		cOp->provAttrs = NIL;
		OP_LCHILD(cOp)->provAttrs = NIL;

		replaceNode(OP_LCHILD(cOp)->inputs, OP_RCHILD(OP_LCHILD(cOp)), rightDelta);
		char *sql = serializeQuery(cOp);
		postgresGetDataChunkJoin(sql, resDCIns, resDCDel, 1, psAttrAndLevel);
		INFO_LOG("SQL is : %s", sql);
	}

	// both join branches have delta tuples;
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
		postgresGetDataChunkJoin(sql, resDCIns, resDCDel, 0, psAttrAndLevel);
		INFO_LOG("WAHT IS SQL", sql);
		// Relation *relation = executeQuery(sql);
		// INFO_LOG("schema %s", stringListToConstList(relation->schema));
	}

	HashMap *resChunks = NEW_MAP(Constant, Node);
	if (resDCIns->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunks, PROP_DATA_CHUNK_INSERT, resDCIns);
	}
	if (resDCDel->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunks, PROP_DATA_CHUNK_DELETE, resDCDel);
	}
	if (mapSize(resChunks) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, (Node *) resChunks);
	}

	DEBUG_NODE_BEATIFY_LOG("JOIN CHUNKS", resChunks);
	// remove children's data chunk;
	if (HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
	}
	if (HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		removeStringProperty(OP_RCHILD(op), PROP_DATA_CHUNK);
	}
}

static ConstRelMultiListsOperator *
createConstRelMultiListsFromDataChunk(DataChunk *dataChunkIns, DataChunk *dataChunkDel, boolean isLeftBranch, List *parentList, List *provAttrDefs)
{
	if (dataChunkIns == NULL && dataChunkDel == NULL) {
		return NULL;
	}

	// all the chunk has same attribute names and data types;
	// build attr names and datatypes;
	List *attrNames = NIL;
	List *attrTypes = NIL;
	boolean hasBuiltInfo = FALSE;

	int dcInsLen = 0;
	int dcDelLen = 0;
	int attrLen = 0;
	HashMap *psIsInt = NULL;
    boolean isAPSChunk = FALSE;
	if (dataChunkIns != NULL) {
		dcInsLen = dataChunkIns->numTuples;
		attrNames = getAttrDefNames(dataChunkIns->attrNames);
		attrTypes = getAttrDataTypes(dataChunkIns->attrNames);
		attrLen = dataChunkIns->tupleFields;
		if (dataChunkIns->isAPSChunk) {
			psIsInt = (HashMap *) dataChunkIns->fragmentsIsInt;
            isAPSChunk = TRUE;
		}
		hasBuiltInfo = TRUE;
	}
	if (dataChunkDel != NULL) {
		dcDelLen = dataChunkDel->numTuples;
		if (!hasBuiltInfo) {
			attrNames = getAttrDefNames(dataChunkDel->attrNames);
			attrTypes = getAttrDataTypes(dataChunkDel->attrNames);
			attrLen = dataChunkDel->tupleFields;
            if (dataChunkDel->isAPSChunk) {
                psIsInt = (HashMap *) dataChunkDel->fragmentsIsInt;
                isAPSChunk = TRUE;
            }
            hasBuiltInfo = TRUE;
		}
	}

	Vector *values = makeVector(VECTOR_NODE, T_Vector);
	for (int col = 0; col < attrLen; col++) {
		Vector *insVec = NULL;
		Vector *delVec = NULL;
		size_t insBytes = 0;
		size_t delBytes = 0;
		if (dcInsLen > 0) {
			insVec = (Vector *) getVecNode(dataChunkIns->tuples, col);
			insBytes = dcInsLen * getVecElemSize(insVec);
		}
		if (dcDelLen > 0) {
			delVec = (Vector *) getVecNode(dataChunkDel->tuples, col);
			delBytes = dcDelLen * getVecElemSize(delVec);
		}

		Vector *resVec = NULL;
		DataType colType = (DataType) getNthOfListInt(attrTypes, col);
		switch (colType) {
			case DT_INT:
			case DT_BOOL:
				resVec = makeVectorOfSize(VECTOR_INT, T_Vector, dcInsLen + dcDelLen);
				break;
			case DT_LONG:
				resVec = makeVectorOfSize(VECTOR_LONG, T_Vector, dcInsLen + dcDelLen);
				break;
			case DT_FLOAT:
				resVec = makeVectorOfSize(VECTOR_FLOAT, T_Vector, dcInsLen + dcDelLen);
				break;
			case DT_STRING:
			case DT_VARCHAR2:
				resVec = makeVectorOfSize(VECTOR_STRING, T_Vector, dcInsLen + dcDelLen);
				break;
			default:
				FATAL_LOG("not supported");
		}

		if (insBytes > 0) {
			memcpy(resVec->data, insVec->data, insBytes);
		}
		if (delBytes > 0) {
			memcpy(resVec->data + insBytes, delVec->data, delBytes);
		}
		resVec->length = dcInsLen + dcDelLen;
		vecAppendNode(values, (Node *) resVec);
	}

	// deal with provenance sketch;
    if (isAPSChunk) {
		FOREACH_HASH_KEY(Constant, c, psIsInt) {
			attrNames = appendToTailOfList(attrNames, STRING_VALUE(c));

			boolean isIntPS = BOOL_VALUE(MAP_GET_STRING(psIsInt, STRING_VALUE(c)));
			Vector *psVector = NULL;
			if (isIntPS) {
				attrTypes = appendToTailOfListInt(attrTypes, DT_INT);
				psVector = makeVectorOfSize(VECTOR_INT, T_Vector, dcInsLen + dcDelLen);

				if (dcInsLen > 0) {
					memcpy(psVector->data, ((Vector *) MAP_GET_STRING(dataChunkIns->fragmentsInfo, STRING_VALUE(c)))->data, (size_t) (dcInsLen * sizeof(int)));
				}
				if (dcDelLen > 0) {
					memcpy(psVector->data + (dcInsLen * sizeof(int)), ((Vector *) MAP_GET_STRING(dataChunkDel->fragmentsInfo, STRING_VALUE(c)))->data, (size_t) (dcDelLen * sizeof(int)));
				}

				psVector->length = dcInsLen + dcDelLen;
			} else {
				attrTypes = appendToTailOfListInt(attrTypes, DT_STRING);
				psVector = makeVector(VECTOR_NODE, T_Vector);
				if (dcInsLen > 0) {
					Vector *fromVec = (Vector *) MAP_GET_STRING(dataChunkIns->fragmentsInfo, STRING_VALUE(c));
					FOREACH_VEC(BitSet, bs, fromVec) {
						vecAppendString(psVector, bitSetToString(bs));
					}
				}
				if (dcDelLen > 0) {
					Vector *fromVec = (Vector *) MAP_GET_STRING(dataChunkDel->fragmentsInfo, STRING_VALUE(c));
					FOREACH_VEC(BitSet, bs, fromVec) {
						vecAppendString(psVector, bitSetToString(bs));
					}
				}
			}
			vecAppendNode(values, (Node *) psVector);
		}
    }

	// for update type;
	if (isLeftBranch) {
		attrNames = appendToTailOfList(attrNames, JOIN_LEFT_BRANCH_IDENTIFIER);
	} else {
		attrNames = appendToTailOfList(attrNames, JOIN_RIGHT_BRANCH_IDENTIFIER);
	}
	attrTypes = appendToTailOfListInt(attrTypes, DT_INT);
	Vector *updIden = makeVectorOfSize(VECTOR_INT, T_Vector, dcInsLen + dcDelLen);
	/* cannot use memset() for this int vec, */
	int *idens = VEC_TO_IA(updIden);
	for (int i = 0; i < dcInsLen; i++) {
		idens[i] = 1;
	}
	for (int i = 0; i < dcDelLen; i++) {
		idens[i + dcInsLen] = -1;
	}
	updIden->length = dcInsLen + dcDelLen;
	vecAppendNode(values, (Node *) updIden);

	ConstRelMultiListsOperator *coOp = createConstRelMultiListsOp(values, parentList, attrNames, attrTypes);
	return coOp;
}

static Vector *
buildGroupByValueVecFromDataChunk(DataChunk *dc, List *gbList)
{
	Vector *gbValsVec = makeVector(VECTOR_STRING, T_Vector);
	int numTuples = dc->numTuples;
	if (gbList == NULL) {
		char *dummyGBVal = strdup("##");
		for (int row = 0; row < numTuples; row++) {
			vecAppendString(gbValsVec, dummyGBVal);
		}
	} else {
		Vector *gbAttrPos = makeVector(VECTOR_INT, T_Vector);
		Vector *gbAttrType = makeVector(VECTOR_INT, T_Vector);
		Vector *gbAttrTypeStringPos = makeVector(VECTOR_INT, T_Vector);
		boolean noStringTypeExists = TRUE;
		size_t totalSizeIfNoStringType = 0;
		FOREACH(AttributeReference, ar, gbList) {
			int pos = INT_VALUE((Constant *) MAP_GET_STRING(dc->attriToPos, ar->name));
			DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(dc->posToDatatype, pos));
			vecAppendInt(gbAttrPos, pos);
			vecAppendInt(gbAttrType, (int) dt);

			switch (dt) {
				case DT_INT:
				case DT_BOOL:
					totalSizeIfNoStringType += sizeof(int);
					break;
				case DT_FLOAT:
					totalSizeIfNoStringType += sizeof(double);
					break;
				case DT_LONG:
					totalSizeIfNoStringType += sizeof(gprom_long_t);
					break;
				case DT_VARCHAR2:
				case DT_STRING:
					{
						vecAppendInt(gbAttrTypeStringPos, pos);
						totalSizeIfNoStringType += 0;
						noStringTypeExists = FALSE;
					}
					break;
			}
		}
		if (noStringTypeExists) {
			// column fashion,
			// assign space for each gbval;
			for (int row = 0; row < dc->numTuples; row++) {
				char *gbVals = MALLOC(totalSizeIfNoStringType + 1);
				vecAppendString(gbValsVec, gbVals);
			}
			// memcpy actural content;
			size_t preSize = 0;

			for (int col = 0; col < gbAttrPos->length; col++) {
				int pos = getVecInt(gbAttrPos, col);
				DataType dt = (DataType) getVecInt(gbAttrType, col);
				boolean isLastCol = (col == gbAttrPos->length - 1);
				Vector *valVec = (Vector *) getVecNode(dc->tuples, pos);
				switch (dt) {
					case DT_INT:
					case DT_BOOL:
					{
						for (int row = 0; row < numTuples; row++) {
							int val = getVecInt(valVec, row);
							char *gbval = (char *) getVecString(gbValsVec, row);
							memcpy(gbval + preSize, &val, sizeof(int));
							if (isLastCol) {
								gbval[totalSizeIfNoStringType] = '\0';
							}
						}
						preSize += sizeof(int);

					}
					break;
					case DT_LONG:
					{
						for (int row = 0; row < numTuples; row++) {
							gprom_long_t val = getVecLong(valVec, row);
							char *gbval = (char *) getVecString(gbValsVec, row);
							memcpy(gbval + preSize, &val, sizeof(gprom_long_t));
							if (isLastCol) {
								gbval[totalSizeIfNoStringType] = '\0';
							}
						}
						preSize += sizeof(gprom_long_t);
					}
					break;
					case DT_FLOAT:
					{
						for (int row = 0; row < numTuples; row++) {
							double val = getVecFloat(valVec, row);
							char *gbval = (char *) getVecString(gbValsVec, row);
							memcpy(gbval + preSize, &val, sizeof(double));
							if (isLastCol) {
								gbval[totalSizeIfNoStringType] = '\0';
							}
						}
						preSize += sizeof(double);
					}
					break;
					case DT_STRING:
					case DT_VARCHAR2:
					{

					}
					break;
				}
			}
		} else {
			size_t preSizes[numTuples];
			size_t totalSizes[numTuples];
			// check each str len;
			for (int col = 0; col < gbAttrTypeStringPos->length; col++) {
				int pos = getVecInt(gbAttrTypeStringPos, col);
				for (int row = 0; row < numTuples; row++) {
					if (col == 0) {
						totalSizes[row] = 0;
					}
					char *strVal = getVecString((Vector *) getVecNode(dc->tuples, pos), row);
					totalSizes[row] += strlen(strVal);
				}
			}
			// assign each gb val a space;
			// set preSize = 0;
			for (int row = 0; row < numTuples; row++) {
				preSizes[row] = 0;
				totalSizes[row] += totalSizeIfNoStringType;
				char *gbVal = MALLOC(totalSizes[row] + 1);
				vecAppendString(gbValsVec, gbVal);
			}

			// memcpy content;
			for (int col = 0; col < gbAttrPos->length; col++) {
				int pos = getVecInt(gbAttrPos, col);
				DataType dt = (DataType) getVecInt(gbAttrType, col);
				boolean isLastCol = (col == gbAttrPos->length - 1);

				Vector *valVec = (Vector *) getVecNode(dc->tuples, pos);
				switch (dt) {
					case DT_INT:
					case DT_BOOL:
					{
						for (int row = 0; row < numTuples; row++) {
							int val = getVecInt(valVec, row);
							char *gbval = getVecString(gbValsVec, row);
							memcpy(gbval + preSizes[row], &val, sizeof(int));
							if (isLastCol) {
								gbval[totalSizes[row]] = '\0';
							}
							preSizes[row] += sizeof(int);
						}
					}
					break;
					case DT_LONG:
					{
						for (int row = 0; row < numTuples; row++) {
							gprom_long_t val = getVecLong(valVec, row);
							char *gbval = getVecString(gbValsVec, row);
							memcpy(gbval + preSizes[row], &val, sizeof(gprom_long_t));
							if (isLastCol) {
								gbval[totalSizes[row]] = '\0';
							}
							preSizes[row] += sizeof(gprom_long_t);
						}
					}
					break;
					case DT_FLOAT:
					{
						for (int row = 0; row < numTuples; row++) {
							double val = getVecFloat(valVec, row);
							char *gbval = getVecString(gbValsVec, row);
							memcpy(gbval + preSizes[row], &val, sizeof(double));
							if (isLastCol) {
								gbval[totalSizes[row]] = '\0';
							}
							preSizes[row] += sizeof(double);
						}
					}
					break;
					case DT_VARCHAR2:
					case DT_STRING:
					{
						for (int row = 0; row < numTuples; row++) {
							char *val = getVecString(valVec, row);
							char *gbval = getVecString(gbValsVec, row);
							size_t len = strlen(val);
							memcpy(gbval + preSizes[row], val, len);
							if (isLastCol) {
								gbval[totalSizes[row]] = '\0';
							}
							preSizes[row] += len;
						}
					}
					break;
				}
			}
		}
	}
	return gbValsVec;
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
			case DT_BOOL:
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
	boolean noGB = LIST_LENGTH(aggGBList) == 0 ? TRUE : FALSE;

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
		gbValsInsert = buildGroupByValueVecFromDataChunk(dataChunkInsert, aggGBList);
	}
	if (MAP_HAS_STRING_KEY(chunkMaps, PROP_DATA_CHUNK_INSERT)) {
		dataChunkInsert = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
		// gbValsInsert = makeVectorOfSize(VECTOR_STRING, T_Vector, dataChunkInsert->numTuples);
		// gbValsInsert->length = dataChunkInsert->numTuples;

		// build gb pos and type only once;
		FOREACH(AttributeReference, ar, aggGBList) {
			int pos = INT_VALUE(MAP_GET_STRING(dataChunkInsert->attriToPos, ar->name));
			DataType type = INT_VALUE(MAP_GET_INT(dataChunkInsert->posToDatatype, pos));
			vecAppendInt(gbPoss, pos);
			vecAppendInt(gbType, type);
			vecAppendString(gbName, strdup(ar->name));
		}
		hasBuildGBAttrsPosTypeVec = TRUE;
	}
	// 	char ** gbValsArr = (char **) VEC_TO_ARR(gbValsInsert, char);
	// 	if (noGB) {
	// 		for (int row = 0; row < dataChunkInsert->numTuples; row++) {
	// 			gbValsArr[row] = strdup("##");
	// 		}
	// 	} else {
	// 		for (int gbIndex = 0; gbIndex < gbAttrCnt; gbIndex++) {
	// 			DataType type = getVecInt(gbType, gbIndex);
	// 			int pos = getVecInt(gbPoss, gbIndex);
	// 			switch (type) {
	// 				case DT_INT:
	// 				{
	// 					int *valArr = VEC_TO_IA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
	// 					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_LONG:
	// 				{
	// 					gprom_long_t *valArr = VEC_TO_LA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
	// 					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(gprom_ltoa(valArr[row]), "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ltoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_FLOAT:
	// 				{
	// 					double *valArr = VEC_TO_FA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
	// 					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
	// 						// version 2: add some processing to gb float
	// 						char *thisVal = gprom_ftoa(valArr[row]);
	// 						int valLen = strlen(thisVal);
	// 						int trim0Pos = valLen - 1;
	// 						for (int zeroPos = valLen - 1; zeroPos >= 0; zeroPos--) {
	// 							if (thisVal[zeroPos] != '0') {
	// 								if (thisVal[zeroPos] == '.') {
	// 									// in case get 2. we need 2.0
	// 									trim0Pos = zeroPos + 1;
	// 								} else {
	// 									trim0Pos = zeroPos;
	// 								}
	// 								break;
	// 							}
	// 						}

	// 						char actualStr[1000];
	// 						strncpy(actualStr, thisVal, trim0Pos + 1);
	// 						actualStr[trim0Pos + 1] = '\0';

	// 						if (gbIndex == 0) {
	// 						 	gbValsArr[row] = CONCAT_STRINGS(actualStr, "#");
	// 						} else {
	// 						 	gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], actualStr, "#");
	// 						}
	// 						// end of process version 2

	// 						// version 1
	// 						// if (gbIndex == 0)
	// 						// 	gbValsArr[row] = CONCAT_STRINGS(gprom_ftoa(valArr[row]), "#");
	// 						// else
	// 						// 	gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ftoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_BOOL:
	// 				{
	// 					int *valArr = VEC_TO_IA((Vector *) getVecNode(dataChunkInsert->tuples, pos));
	// 					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_STRING:
	// 				case DT_VARCHAR2:
	// 				{
	// 					char **valArr = (char **) VEC_TO_ARR((Vector *) getVecNode(dataChunkInsert->tuples, pos), char);
	// 					for (int row = 0; row < dataChunkInsert->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(valArr[row], "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], valArr[row], "#");
	// 					}

	// 				}
	// 					break;
	// 				default:
	// 					FATAL_LOG("data type is not supproted");
	// 			}
	// 		}
	// 	}
	// 	// ASSERT(gbValsInsert->length == dataChunkInsert->numTuples);
	// }

	Vector *gbValsDelete = NULL;
	DataChunk *dataChunkDelete = NULL;
	if (MAP_HAS_STRING_KEY(chunkMaps, PROP_DATA_CHUNK_DELETE)) {
		dataChunkDelete = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
		gbValsDelete = buildGroupByValueVecFromDataChunk(dataChunkDelete, aggGBList);
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//>>>>>>>> remove below code>>>>>>>>>>>>>>>>>>>
	if (MAP_HAS_STRING_KEY(chunkMaps, PROP_DATA_CHUNK_DELETE)) {
		dataChunkDelete = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);
		// gbValsDelete = makeVectorOfSize(VECTOR_STRING, T_Vector, dataChunkDelete->numTuples);
		// gbValsDelete->length = dataChunkDelete->numTuples;

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
	}

	// 	char ** gbValsArr = (char **) VEC_TO_ARR(gbValsDelete, char);
	// 	if (noGB) {
	// 		for (int row = 0; row < dataChunkDelete->numTuples; row++) {
	// 			gbValsArr[row] = strdup("##");
	// 		}
	// 	} else {
	// 		for (int gbIndex = 0; gbIndex < gbAttrCnt; gbIndex++) {
	// 			DataType type = getVecInt(gbType, gbIndex);
	// 			int pos = getVecInt(gbPoss, gbIndex);
	// 			switch (type) {
	// 				case DT_INT:
	// 				{
	// 					int *valArr = VEC_TO_IA((Vector *) getVecNode(dataChunkDelete->tuples, pos));
	// 					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 						{ gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#"); }
	// 						else
	// 						{ gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#"); }
	// 					}
	// 				}
	// 					break;
	// 				case DT_LONG:
	// 				{
	// 					gprom_long_t *valArr = VEC_TO_LA(getVecNode(dataChunkDelete->tuples, pos));
	// 					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(gprom_ltoa(valArr[row]), "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ltoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_FLOAT:
	// 				{
	// 					double *valArr = VEC_TO_FA(getVecNode(dataChunkDelete->tuples, pos));
	// 					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
	// 						// version 2: add some processing to gb float
	// 						char *thisVal = gprom_ftoa(valArr[row]);
	// 						int valLen = strlen(thisVal);
	// 						int trim0Pos = valLen - 1;
	// 						for (int zeroPos = valLen - 1; zeroPos >= 0; zeroPos--) {
	// 							if (thisVal[zeroPos] != '0') {
	// 								if (thisVal[zeroPos] == '.') {
	// 									// in case get 2. we need 2.0
	// 									trim0Pos = zeroPos + 1;
	// 								} else {
	// 									trim0Pos = zeroPos;
	// 								}
	// 								break;
	// 							}
	// 						}

	// 						char actualStr[1000];
	// 						strncpy(actualStr, thisVal, trim0Pos + 1);
	// 						actualStr[trim0Pos + 1] = '\0';
	// 						INFO_LOG("actural str %s", actualStr);
	// 						if (gbIndex == 0) {
	// 						 	gbValsArr[row] = CONCAT_STRINGS(actualStr, "#");
	// 						} else {
	// 						 	gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], actualStr, "#");
	// 						}
	// 						// end of process version 2

	// 						// version 1
	// 						// if (gbIndex == 0)
	// 						// 	gbValsArr[row] = CONCAT_STRINGS(gprom_ftoa(valArr[row]), "#");
	// 						// else
	// 						// 	gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_ftoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_BOOL:
	// 				{
	// 					int *valArr = VEC_TO_IA(getVecNode(dataChunkDelete->tuples, pos));
	// 					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(gprom_itoa(valArr[row]), "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], gprom_itoa(valArr[row]), "#");
	// 					}
	// 				}
	// 					break;
	// 				case DT_STRING:
	// 				case DT_VARCHAR2:
	// 				{
	// 					char **valArr = (char **) VEC_TO_ARR((Vector *) getVecNode(dataChunkDelete->tuples, pos), char);
	// 					for (int row = 0; row < dataChunkDelete->numTuples; row++) {
	// 						if (gbIndex == 0)
	// 							gbValsArr[row] = CONCAT_STRINGS(valArr[row], "#");
	// 						else
	// 							gbValsArr[row] = CONCAT_STRINGS(gbValsArr[row], valArr[row], "#");
	// 					}

	// 				}
	// 					break;
	// 				default:
	// 					FATAL_LOG("data type is not supproted");
	// 			}
	// 		}
	// 	}
	// 	ASSERT(gbValsDelete->length == dataChunkDelete->numTuples);
	// }
	//<<<<<<<<<<<<<<<<<<remove above code <<<<<<<<<<<<<<
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	DEBUG_NODE_BEATIFY_LOG("insert gbs", gbValsInsert);
	DEBUG_NODE_BEATIFY_LOG("delete gbs", gbValsDelete);

	// fetch stored pre-built data structures for all aggs;
	HashMap *dataStructures = (HashMap *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);

	// cast group by values to primitive array;
	char **gbValsInsertArr = NULL;
	int insertLength = 0;
	if (gbValsInsert != NULL) {
		gbValsInsertArr = (char **) VEC_TO_ARR(gbValsInsert, char);
		insertLength = dataChunkInsert->numTuples;
	}

	char **gbValsDeleteArr = NULL;
	int deleteLength = 0;
	if (gbValsDelete != NULL) {
		gbValsDeleteArr = (char **) VEC_TO_ARR(gbValsDelete, char);
		deleteLength = dataChunkDelete->numTuples;
	}

	boolean hasFinishGBDelete = FALSE;
	boolean hasFinishPSDelete = FALSE;
	boolean hasFinishGBInsert = FALSE;
	boolean hasFinishPSInsert = FALSE;
	FOREACH(FunctionCall, fc, aggFCList) {
		AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
		char *nameInDS = CONCAT_STRINGS(fc->functionname, "_", ar->name);
		if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0
		|| strcmp(fc->functionname, SUM_FUNC_NAME) == 0
		|| strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
			GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructures, nameInDS);

			// is a ps chunk;
			boolean isAPSChunk = FALSE;

			// input vec pos and type;
			int inputVecPos = 0;
			DataType inputVecType = DT_INT;

			// output vec pos and type;
			Constant *nameInOutChunk = (Constant *) MAP_GET_STRING(mapFCsToSchemas, nameInDS);
			int outputVecPos = 0;
			DataType outputVecType = DT_INT;

			if (dataChunkInsert != NULL) {
				isAPSChunk = dataChunkInsert->isAPSChunk;
				inputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunkInsert->attriToPos, ar->name));
				inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, inputVecPos));
				outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCInsert->attriToPos, STRING_VALUE(nameInOutChunk)));
				outputVecType = INT_VALUE((Constant *) MAP_GET_INT(resultDCInsert->posToDatatype, outputVecPos));
			} else if (dataChunkDelete != NULL) {
				isAPSChunk = dataChunkDelete->isAPSChunk;
				inputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunkDelete->attriToPos, ar->name));
				inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkDelete->posToDatatype, inputVecPos));
				outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCDelete->attriToPos, STRING_VALUE(nameInOutChunk)));
				outputVecType = INT_VALUE((Constant *) MAP_GET_INT(resultDCDelete->posToDatatype, outputVecPos));
			}

			Vector *outputVecInsert = (Vector *) getVecNode(resultDCInsert->tuples, outputVecPos);
			Vector *outputVecDelete = (Vector *) getVecNode(resultDCDelete->tuples, outputVecPos);
			// update for insert chunk;

			if (dataChunkInsert != NULL) {
				Vector *inputVec = (Vector *) getVecNode(dataChunkInsert->tuples, inputVecPos) ;
				Vector *updateTypeForEachTuple = makeVector(VECTOR_INT, T_Vector);

				switch (inputVecType) {
					case DT_INT:
					{
						int *inputVecVals = (int *) VEC_TO_IA(inputVec);
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
								if (oldL != NULL) {
									// -1, + 1-> 0;
									vecAppendInt(updateTypeForEachTuple, 0);

									// get old stored values;
									Constant *avg = (Constant *) getVecNode(oldL, 0);
									Constant *sum = (Constant *) getVecNode(oldL, 1);
									Constant *cnt = (Constant *) getVecNode(oldL, 2);

									// append old avg value;
									vecAppendFloat(outputVecDelete, FLOAT_VALUE(avg));

									// calculate new values;
									// FLOAT_VALUE(sum) += inputVecVals[row];
									FLOAT_VALUE(sum) = FLOAT_VALUE(sum) + (double) inputVecVals[row];
									// (*((double *) sum->value)) += inputVecVals[row];
									// (*((gprom_long_t *) cnt->value)) += 1;
									incrConst(cnt);
									FLOAT_VALUE(avg) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);
									// (*((double *) avg->value)) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);

									// append new avg value;
									vecAppendFloat(outputVecInsert, FLOAT_VALUE(avg));
								} else {
									// +1 -> 1;
									vecAppendInt(updateTypeForEachTuple, 1);

									// initialize vec and insert values;
									oldL = makeVector(VECTOR_NODE, T_Vector);
									int value = inputVecVals[row];
									vecAppendNode(oldL, (Node *) createConstFloat((double) value));
									vecAppendNode(oldL, (Node *) createConstFloat((double) value));
									vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

									// append new value;
									vecAppendFloat(outputVecInsert, (double) value);
									addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
								}
							}
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							switch(outputVecType) {
								case DT_INT:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											// +1 -> 1;
											vecAppendInt(updateTypeForEachTuple, 1);
											// init vector;
											oldL = makeVector(VECTOR_NODE, T_Vector);
											int value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											// append value;
											vecAppendInt(outputVecInsert, value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);
											vecAppendInt(outputVecDelete, (int) FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;
											vecAppendInt(outputVecInsert, (int) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_LONG:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											int value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendLong(outputVecInsert, (gprom_long_t) value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);
											vecAppendLong(outputVecDelete, (gprom_long_t) FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;
											vecAppendLong(outputVecInsert, (gprom_long_t) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_FLOAT:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											int value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendFloat(outputVecInsert, (double) value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);
											// gprom_long_t cnt = LONG_VALUE((Constant *) popVecNode(oldL));
											// double sum = FLOAT_VALUE((Constant *) popVecNode(oldL));

											vecAppendFloat(outputVecDelete, FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;

											vecAppendFloat(outputVecInsert, FLOAT_VALUE(sum));
										}
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
								if (oldL == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									oldL = makeVector(VECTOR_NODE, T_Vector);
									vecAppendNode(oldL, (Node *) createConstLong(1));
									vecAppendLong(outputVecInsert, (gprom_long_t) 1);
									addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									Constant *cnt = (Constant *) getVecNode(oldL, 0);
									vecAppendLong(outputVecDelete, LONG_VALUE(cnt));
									(*((gprom_long_t *) cnt->value)) += 1;
									vecAppendLong(outputVecInsert, LONG_VALUE(cnt));
								}
							}
						}
					}
						break;
					case DT_LONG:
					{
						gprom_long_t *inputVecVals = (gprom_long_t *) VEC_TO_LA(inputVec);
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
								if (oldL != NULL) {
									vecAppendInt(updateTypeForEachTuple, 0);

									// get old stored values;
									Constant *avg = (Constant *) getVecNode(oldL, 0);
									Constant *sum = (Constant *) getVecNode(oldL, 1);
									Constant *cnt = (Constant *) getVecNode(oldL, 2);

									// append old avg value;
									vecAppendFloat(outputVecDelete, FLOAT_VALUE(avg));

									// calculate new values;
									(*((double *) sum->value)) += inputVecVals[row];
									(*((gprom_long_t *) cnt->value)) += 1;
									(*((double *) avg->value)) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);

									// append new avg value;
									vecAppendFloat(outputVecInsert, FLOAT_VALUE(avg));
								} else {
									// +1 -> 1;
									vecAppendInt(updateTypeForEachTuple, 1);

									// initialize vec and insert values;
									oldL = makeVector(VECTOR_NODE, T_Vector);
									gprom_long_t value = inputVecVals[row];
									vecAppendNode(oldL, (Node *) createConstFloat((double) value));
									vecAppendNode(oldL, (Node *) createConstFloat((double) value));
									vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

									// append new value;
									vecAppendFloat(outputVecInsert, (double) value);
									addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
								}
							}
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							switch(outputVecType) {
								case DT_INT:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											gprom_long_t value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendInt(outputVecInsert, (int) value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);

											vecAppendInt(outputVecDelete, (int) FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;

											vecAppendInt(outputVecInsert, (int) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_LONG:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											gprom_long_t value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendLong(outputVecInsert, value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);

											vecAppendLong(outputVecDelete, (gprom_long_t) FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;

											vecAppendLong(outputVecInsert, (gprom_long_t) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_FLOAT:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											gprom_long_t value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendFloat(outputVecInsert, (double) value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);

											vecAppendFloat(outputVecDelete, FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;
											vecAppendFloat(outputVecInsert, FLOAT_VALUE(sum));
										}
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
								if (oldL == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									oldL = makeVector(VECTOR_NODE, T_Vector);
									vecAppendLong(oldL, (gprom_long_t) 1);
									vecAppendLong(outputVecInsert, (gprom_long_t) 1);
									addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									Constant *cnt = (Constant *) getVecNode(oldL, 0);
									vecAppendLong(outputVecDelete, LONG_VALUE(cnt));
									(*((gprom_long_t *) cnt->value)) += 1;
									vecAppendLong(outputVecInsert, LONG_VALUE(cnt));
								}
							}
						}
					}
						break;
					case DT_FLOAT:
					{
						double *inputVecVals = (double *) VEC_TO_FA(inputVec);
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
								if (oldL != NULL) {
									// -1, + 1-> 0;
									vecAppendInt(updateTypeForEachTuple, 0);

									// get old stored values;
									Constant *avg = (Constant *) getVecNode(oldL, 0);
									Constant *sum = (Constant *) getVecNode(oldL, 1);
									Constant *cnt = (Constant *) getVecNode(oldL, 2);

									// append old avg value;
									vecAppendFloat(outputVecDelete, FLOAT_VALUE(avg));

									// calculate new values;
									(*((double *) sum->value)) += inputVecVals[row];
									(*((gprom_long_t *) cnt->value)) += 1;
									(*((double *) avg->value)) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);

									// store new values;

									// append new avg value;
									vecAppendFloat(outputVecInsert, FLOAT_VALUE(avg));
								} else {
									// +1 -> 1;
									vecAppendInt(updateTypeForEachTuple, 1);

									// initialize vec and insert values;
									oldL = makeVector(VECTOR_NODE, T_Vector);
									double value = inputVecVals[row];
									vecAppendNode(oldL, (Node *) createConstFloat((double) value));
									vecAppendNode(oldL, (Node *) createConstFloat((double) value));
									vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

									// append new value;
									vecAppendFloat(outputVecInsert, value);
									addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
								}
							}
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							switch(outputVecType) {
								case DT_INT:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											double value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendInt(outputVecInsert, (int) value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);

											vecAppendInt(outputVecDelete, (int) FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;

											vecAppendInt(outputVecInsert, (int) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_LONG:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											double value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendLong(outputVecInsert, (gprom_long_t) value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);

											vecAppendLong(outputVecDelete, (gprom_long_t) FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;

											vecAppendLong(outputVecInsert, (gprom_long_t) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_FLOAT:
								{
									for (int row = 0; row < insertLength; row++) {
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
										if (oldL == NULL) {
											vecAppendInt(updateTypeForEachTuple, 1);
											oldL = makeVector(VECTOR_NODE, T_Vector);
											double value = inputVecVals[row];
											vecAppendNode(oldL, (Node *) createConstFloat((double) value));
											vecAppendNode(oldL, (Node *) createConstLong((gprom_long_t) 1));

											vecAppendFloat(outputVecInsert, value);
											addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											// get old values;
											Constant *sum = (Constant *) getVecNode(oldL, 0);
											Constant *cnt = (Constant *) getVecNode(oldL, 1);

											vecAppendFloat(outputVecDelete, FLOAT_VALUE(sum));

											(*((double *) sum->value)) += inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) += 1;

											vecAppendFloat(outputVecInsert,  FLOAT_VALUE(sum));
										}
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbValsInsertArr[row]);
								if (oldL == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									oldL = makeVector(VECTOR_NODE, T_Vector);
									vecAppendLong(oldL, (gprom_long_t) 1);
									vecAppendLong(outputVecInsert, (gprom_long_t) 1);
									addToMap(acs->map, (Node *) createConstString(gbValsInsertArr[row]), (Node *) oldL);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									Constant *cnt = (Constant *) getVecNode(oldL, 0);
									vecAppendLong(outputVecDelete, LONG_VALUE(cnt));
									(*((gprom_long_t *) cnt->value)) += 1;
									vecAppendLong(outputVecInsert, LONG_VALUE(cnt));
								}
							}
						}
					}
						break;
					default:
						FATAL_LOG("not supported");
				}

				// dealing with group by values;
				if (!hasFinishGBInsert && !noGB) {
					for (int gbAttrIndex = 0; gbAttrIndex < gbAttrCnt; gbAttrIndex++) {
						int fromPos = getVecInt(gbPoss, gbAttrIndex);
						DataType gbDataType = (DataType) getVecInt(gbType, gbAttrIndex);
						char *gbAttrName = (char *) getVecString(gbName, gbAttrIndex);
						char *toName = STRING_VALUE((Constant *) MAP_GET_STRING(mapFCsToSchemas, gbAttrName));
						int toPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCInsert->attriToPos, toName));

						Vector *fromVec = (Vector *) getVecNode(dataChunkInsert->tuples, fromPos);
						Vector *resVecIns = (Vector *) getVecNode(resultDCInsert->tuples, toPos);
						Vector *resVecDel = (Vector *) getVecNode(resultDCDelete->tuples, toPos);

						// Vector *updateIdentIns = (Vector *) resultDCInsert->updateIdentifier;
						// Vector *updateIdentDel = (Vector *) resultDCDelete->updateIdentifier;
						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);

						switch (gbDataType) {
							case DT_INT:
							case DT_BOOL:
							{
								int *values = (int *) VEC_TO_IA(fromVec);
								for (int row = 0; row < insertLength; row++) {
									int val = values[row];
									vecAppendInt(resVecIns, val);
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendInt(resVecDel, val);
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
								break;
							case DT_LONG:
							{
								gprom_long_t *values = (gprom_long_t *) VEC_TO_LA(fromVec);
								for (int row = 0; row < insertLength; row++) {
									gprom_long_t val = values[row];
									vecAppendLong(resVecIns, val);
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendLong(resVecDel, val);
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
								break;
							case DT_FLOAT:
							{
								double *values = (double *) VEC_TO_FA(fromVec);
								for (int row = 0; row < insertLength; row++) {
									double val = values[row];
									vecAppendFloat(resVecIns, val);
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendFloat(resVecDel, val);
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
								break;
							case DT_VARCHAR2:
							case DT_STRING:
							{
								char **values = VEC_TO_ARR(fromVec, char);
								for (int row = 0; row < insertLength; row++) {
									char *val = values[row];
									vecAppendString(resVecIns, strdup(val));
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendString(resVecDel, strdup(val));
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
								break;
							default:
								FATAL_LOG("not supported");
						}
					}
					hasFinishGBInsert = TRUE;
				}

				// dealing with ps;

				if (isAPSChunk && !hasFinishPSInsert) {
					PSMap *groupPSMap = (PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_AGG);
					FOREACH_HASH_KEY(Constant, c, dataChunkInsert->fragmentsInfo) {
						// get two input ps vectors;
						boolean inputPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(dataChunkInsert->fragmentsIsInt, STRING_VALUE(c)));
						Vector *inputPS = (Vector *) MAP_GET_STRING(dataChunkInsert->fragmentsInfo, STRING_VALUE(c));

						boolean storedPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(groupPSMap->isIntSketch, STRING_VALUE(c)));
						HashMap *storedPS = (HashMap *) MAP_GET_STRING(groupPSMap->provSketchs, STRING_VALUE(c));
						if (storedPS == NULL) {
							storedPS = NEW_MAP(Constant, Node);
						}
						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(groupPSMap->fragCount, STRING_VALUE(c));
						if (gbFragCnt == NULL) {
							gbFragCnt = NEW_MAP(Constant, Node);
						}
						// output psVector;
						Vector *outputPSInsert = NULL;
						Vector *outputPSDelete = NULL;

						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);
						if (storedPSIsInt) {
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							int *intputPSArr = (int *) VEC_TO_IA(inputPS);
							outputPSInsert = makeVector(VECTOR_INT, T_Vector);
							outputPSDelete = makeVector(VECTOR_INT, T_Vector);
							// in this case input data ps must be integer;
							for (int row = 0; row < insertLength; row++) {
								int type = updateTypeForEachTupleArr[row];
								char *gbVal = gbValsInsertArr[row];
								HashMap *fragCnt = NULL;

								int psVal = intputPSArr[row];
								if (type == 1) {
									vecAppendInt(outputPSInsert, psVal);

									addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) createConstInt(psVal));
									fragCnt = NEW_MAP(Constant, Constant);
									addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));
									addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
								} else if (type == 0) {
									vecAppendInt(outputPSDelete, psVal);
									vecAppendInt(outputPSInsert, psVal);
									fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
									Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
									// (*((int *) cnt->value))++;
									incrConst(cnt);
									//?? is it necessary to addToMap(fragCnt to gbFragCnt);
								}
							}
							addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);

						} else {
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));

							// two cases:
							//		input ps is int or bitset;
							int provSketchLen = INT_VALUE((Constant *) MAP_GET_STRING(groupPSMap->provLens, STRING_VALUE(c)));
							outputPSInsert = makeVector(VECTOR_NODE, T_Vector);
							outputPSDelete = makeVector(VECTOR_NODE, T_Vector);

							if (inputPSIsInt) {
								int *inputPSArr = (int *) VEC_TO_IA(inputPS);

								for (int row = 0; row < insertLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = gbValsInsertArr[row];
									HashMap *fragCnt = NULL;
									int psVal = inputPSArr[row];
									if (type == 1) {
										BitSet *bitSet = newBitSet(provSketchLen);
										setBit(bitSet, psVal, TRUE);
										vecAppendNode(outputPSInsert, (Node *) copyObject(bitSet));
										addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitSet));

										fragCnt = NEW_MAP(Constant, Constant);
										addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));
										addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
									} else if (type == 0) {
										BitSet *oldBitset = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
										vecAppendNode(outputPSDelete, (Node *) copyObject(oldBitset));

										setBit(oldBitset, psVal, TRUE);
										fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
										if (cnt == NULL) {
											addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));
										} else {
											// (*((int *) cnt->value))++;
											incrConst(cnt);
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitset));
										// since oldBitSet already in storedPS, no need to add back;
									}
								}
								addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							} else {
								BitSet **inputPSArr = (BitSet **) VEC_TO_ARR(inputPS, BitSet);
								for (int row = 0; row < insertLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = gbValsInsertArr[row];
									HashMap *fragCnt = NULL;
									if (type == 1) {
										BitSet *bitset = (BitSet *) inputPSArr[row];
										vecAppendNode(outputPSInsert, (Node *) copyObject(bitset));
										addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitset));
										char *provStr = bitSetToString(bitset);
										fragCnt = NEW_MAP(Constant, Constant);
										for (int bitIndex = 0; bitIndex < provSketchLen; bitIndex++) {
											if (provStr[bitIndex] == '1') {
												addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
											}
										}
										addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
									} else if (type == 0) {
										BitSet *bitSet = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
										vecAppendNode(outputPSDelete, (Node *) copyObject(bitSet));
										fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										BitSet *inputBitset = (BitSet *) inputPSArr[row];
										char *provStr = bitSetToString(inputBitset);
										for (int bitIndex = 0; bitIndex < provSketchLen; bitIndex++) {
											if (provStr[bitIndex] == '1') {
												Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, bitIndex);
												if (cnt == NULL) {
													addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
												} else {
													// (*((int *) cnt->value))++;
													incrConst(cnt);
												}
												setBit(bitSet, bitIndex, TRUE);
											}
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(bitSet));
										addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
										addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitSet));
									}
								}
								addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							}
						}


						addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSInsert);
						addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSDelete);
						addToMap(groupPSMap->provSketchs, (Node *) copyObject(c), (Node *) storedPS);
					}
					hasFinishPSInsert = TRUE;
					resultDCInsert->isAPSChunk = TRUE;
					resultDCDelete->isAPSChunk = TRUE;
				}
			}

			// update for delete chunk;
			// boolean hasFinishGBDelete = FALSE;
			// boolean hasFinishPSDelete = FALSE;
			if (dataChunkDelete != NULL) {

				Vector *inputVec = (Vector *) getVecNode(dataChunkDelete->tuples, inputVecPos) ;
				Vector *updateTypeForEachTuple = makeVector(VECTOR_INT, T_Vector);

				switch (inputVecType) {
					case DT_INT:
					{
						int *inputVecVals = (int *) VEC_TO_IA(inputVec);
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								char *gbVal = (char *) gbValsDeleteArr[row];
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
								DEBUG_NODE_BEATIFY_LOG("oldL", oldL);
								Constant *avg = (Constant *) getVecNode(oldL, 0);
								Constant *sum = (Constant *) getVecNode(oldL, 1);
								Constant *cnt = (Constant *) getVecNode(oldL, 2);
								vecAppendFloat(outputVecDelete, FLOAT_VALUE(avg));
								if (LONG_VALUE(cnt) <= 1) {
									vecAppendInt(updateTypeForEachTuple, -1);
									removeMapStringElem(acs->map, gbVal);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									DEBUG_NODE_BEATIFY_LOG("before modify", sum);
									(*((double *) sum->value)) -= inputVecVals[row];
									DEBUG_NODE_BEATIFY_LOG("after modify", sum);
									(*((gprom_long_t *) cnt->value)) -= 1;
									(*((double *) avg->value)) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);
									vecAppendFloat(outputVecInsert, FLOAT_VALUE(avg));
								}
								DEBUG_NODE_BEATIFY_LOG("newL", oldL);
							}
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							switch(outputVecType) {
								case DT_INT:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);

										vecAppendInt(outputVecDelete, (int) FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendInt(outputVecInsert, (int) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_LONG:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);
										vecAppendLong(outputVecDelete, (gprom_long_t) FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendLong(outputVecInsert, (gprom_long_t) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_FLOAT:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);
										vecAppendFloat(outputVecDelete, FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendFloat(outputVecInsert, FLOAT_VALUE(sum));
										}
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							INFO_LOG("deleteLength: %d", deleteLength);
							for (int row = 0; row < deleteLength; row++) {
								char *gbVal = (char *) gbValsDeleteArr[row];
								INFO_LOG("gb values: %s", gbVal);
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
								Constant *cnt = (Constant *) getVecNode(oldL, 0);

								vecAppendLong(outputVecDelete, LONG_VALUE(cnt));
								if (LONG_VALUE(cnt) <= 1) {
									vecAppendInt(updateTypeForEachTuple, -1);
									removeMapStringElem(acs->map, gbVal);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									LONG_VALUE(cnt) = LONG_VALUE(cnt) - 1;
									// (*((gprom_long_t *) cnt->value)) -= 1;
									vecAppendLong(outputVecInsert, LONG_VALUE(cnt));
								}
							}
						}
					}
						break;
					case DT_LONG:
					{
						gprom_long_t *inputVecVals = (gprom_long_t *) VEC_TO_LA(inputVec);
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								char *gbVal = (char *) gbValsDeleteArr[row];
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
								Constant *avg = (Constant *) getVecNode(oldL, 0);
								Constant *sum = (Constant *) getVecNode(oldL, 1);
								Constant *cnt = (Constant *) getVecNode(oldL, 2);

								vecAppendFloat(outputVecDelete, FLOAT_VALUE(avg));
								if (LONG_VALUE(cnt) <= 1) {
									vecAppendInt(updateTypeForEachTuple, -1);
									removeMapStringElem(acs->map, gbVal);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									(*((double *) sum->value)) -= inputVecVals[row];
									(*((gprom_long_t *) cnt->value)) -= 1;
									(*((double *) avg->value)) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);
									vecAppendFloat(outputVecInsert, FLOAT_VALUE(avg));
								}
							}
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) ==0) {
							switch (outputVecType) {
								case DT_INT:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);

										vecAppendInt(outputVecDelete, (int) FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendInt(outputVecInsert, (int) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_LONG:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);
										vecAppendLong(outputVecDelete, (gprom_long_t) FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendLong(outputVecInsert, (gprom_long_t) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_FLOAT:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);
										vecAppendFloat(outputVecDelete, FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendFloat(outputVecInsert, FLOAT_VALUE(sum));
										}
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								char *gbVal = (char *) gbValsDeleteArr[row];
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
								Constant *cnt = (Constant *) getVecNode(oldL, 0);

								vecAppendLong(outputVecDelete, LONG_VALUE(cnt));
								if (LONG_VALUE(cnt) <= 1) {
									vecAppendInt(updateTypeForEachTuple, -1);
									removeMapStringElem(acs->map, gbVal);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									(*((gprom_long_t *) cnt->value)) -= 1;
									vecAppendLong(outputVecInsert, LONG_VALUE(cnt));
								}
							}
						}
					}
						break;
					case DT_FLOAT:
					{
						double *inputVecVals = (double *) VEC_TO_FA(inputVec);
						if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								char *gbVal = (char *) gbValsDeleteArr[row];
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
								Constant *avg = (Constant *) getVecNode(oldL, 0);
								Constant *sum = (Constant *) getVecNode(oldL, 1);
								Constant *cnt = (Constant *) getVecNode(oldL, 2);

								vecAppendFloat(outputVecDelete, FLOAT_VALUE(avg));
								if (LONG_VALUE(cnt) <= 1) {
									vecAppendInt(updateTypeForEachTuple, -1);
									removeMapStringElem(acs->map, gbVal);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									(*((double *) sum->value)) -= inputVecVals[row];
									(*((gprom_long_t *) cnt->value)) -= 1;
									(*((double *) avg->value)) = FLOAT_VALUE(sum) / LONG_VALUE(cnt);
									vecAppendFloat(outputVecInsert, FLOAT_VALUE(avg));
								}
							}
						} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
							switch (outputVecType) {
								case DT_INT:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);

										vecAppendInt(outputVecDelete, (int) FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendInt(outputVecInsert, (int) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_LONG:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);
										vecAppendLong(outputVecDelete, (gprom_long_t) FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendLong(outputVecInsert, (gprom_long_t) FLOAT_VALUE(sum));
										}
									}
								}
									break;
								case DT_FLOAT:
								{
									for (int row = 0; row < deleteLength; row++) {
										char *gbVal = (char *) gbValsDeleteArr[row];
										Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
										Constant *sum = (Constant *) getVecNode(oldL, 0);
										Constant *cnt = (Constant *) getVecNode(oldL, 1);
										vecAppendFloat(outputVecDelete, FLOAT_VALUE(sum));
										if (LONG_VALUE(cnt) <= 1) {
											vecAppendInt(updateTypeForEachTuple, -1);
											removeMapStringElem(acs->map, gbVal);
										} else {
											vecAppendInt(updateTypeForEachTuple, 0);
											(*((double *) sum->value)) -= inputVecVals[row];
											(*((gprom_long_t *) cnt->value)) -= 1;
											vecAppendFloat(outputVecInsert, FLOAT_VALUE(sum));
										}
									}
								}
									break;
								default:
									FATAL_LOG("not supported");
							}
						} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								char *gbVal = (char *) gbValsDeleteArr[row];
								Vector *oldL = (Vector *) MAP_GET_STRING(acs->map, gbVal);
								Constant *cnt = (Constant *) getVecNode(oldL, 0);

								vecAppendLong(outputVecDelete, LONG_VALUE(cnt));
								if (LONG_VALUE(cnt) <= 1) {
									vecAppendInt(updateTypeForEachTuple, -1);
									removeMapStringElem(acs->map, gbVal);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									(*((gprom_long_t *) cnt->value)) -= 1;
									vecAppendLong(outputVecInsert, LONG_VALUE(cnt));
								}
							}
						}
					}
						break;
					default:
						FATAL_LOG("not supported");
				}

				// deal with gb and update identifier;
				if (!hasFinishGBDelete && !noGB) {
					INFO_LOG("BUILD DELETE CHUNK GB VALUES");
					for (int gbAttrIndex = 0; gbAttrIndex < gbAttrCnt; gbAttrIndex++) {
						int fromPos = getVecInt(gbPoss, gbAttrIndex);
						DataType gbDataType = (DataType) getVecInt(gbType, gbAttrIndex);
						char *gbAttrName = (char *) getVecString(gbName, gbAttrIndex);
						char *toName = STRING_VALUE((Constant *) MAP_GET_STRING(mapFCsToSchemas, gbAttrName));
						int toPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCDelete->attriToPos, toName));

						Vector *fromVec = (Vector *) getVecNode(dataChunkDelete->tuples, fromPos);

						Vector *resVecIns = (Vector *) getVecNode(resultDCInsert->tuples, toPos);
						Vector *resVecDel = (Vector *) getVecNode(resultDCDelete->tuples, toPos);

						// Vector *updateIdentIns = (Vector *) resultDCInsert->updateIdentifier;
						// Vector *updateIdentDel = (Vector *) resultDCDelete->updateIdentifier;
						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);
						switch (gbDataType) {
							case DT_INT:
							case DT_BOOL:
							{
								int *values = (int *) VEC_TO_IA(fromVec);
								for (int row = 0; row < deleteLength; row++) {
									int val = values[row];
									vecAppendInt(resVecDel, val);
									// vecAppendInt(updateIdentDel, -1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendInt(resVecIns, val);
										// vecAppendInt(updateIdentIns, 1);
									}
								}
							}
								break;
							case DT_LONG:
							{
								gprom_long_t *values = (gprom_long_t *) VEC_TO_LA(fromVec);
								for (int row = 0; row < deleteLength; row++) {
									gprom_long_t val = values[row];
									vecAppendLong(resVecDel, val);
									// vecAppendInt(updateIdentDel, -1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendLong(resVecIns, val);
										// vecAppendInt(updateIdentIns, 1);
									}
								}
							}
								break;
							case DT_FLOAT:
							{
								double *values = (double *) VEC_TO_FA(fromVec);
								for (int row = 0; row < deleteLength; row++) {
									double val = values[row];
									vecAppendFloat(resVecDel, val);
									// vecAppendInt(updateIdentDel, -1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendFloat(resVecIns, val);
										// vecAppendInt(updateIdentIns, 1);
									}
								}
							}
								break;
							case DT_VARCHAR2:
							case DT_STRING:
							{
								char **values = VEC_TO_ARR(fromVec, char);
								for (int row = 0; row < deleteLength; row++) {
									char *val = values[row];
									vecAppendString(resVecDel, strdup(val));
									// vecAppendInt(updateIdentDel, -1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendString(resVecIns, strdup(val));
										// vecAppendInt(updateIdentIns, 1);
									}
								}
							}
								break;
							default:
								FATAL_LOG("not supported");
						}
					}

					hasFinishGBDelete = TRUE;
				}

				// dealing with ps;
				if (isAPSChunk && !hasFinishPSDelete) {
					INFO_LOG("BUILD DELETE CHUNK PS INFO");
					PSMap *groupPSMap = (PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_AGG);

					FOREACH_HASH_KEY(Constant, c, dataChunkDelete->fragmentsInfo) {
						boolean inputPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(dataChunkDelete->fragmentsIsInt, STRING_VALUE(c)));
						Vector *inputPS = (Vector *) MAP_GET_STRING(dataChunkDelete->fragmentsInfo, STRING_VALUE(c));

						boolean storedPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(groupPSMap->isIntSketch, STRING_VALUE(c)));
						HashMap *storedPs = (HashMap *) MAP_GET_STRING(groupPSMap->provSketchs, STRING_VALUE(c));

						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(groupPSMap->fragCount, STRING_VALUE(c));

						Vector *outputPSInsert = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
						Vector *outputPSDelete = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));
						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);

						if (storedPSIsInt) {
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							// int *inputPSArr = (int *) VEC_TO_IA(inputPS);
							if (outputPSDelete == NULL) {
								outputPSDelete = makeVector(VECTOR_INT, T_Vector);
							}
							if (outputPSInsert == NULL) {
								outputPSInsert = makeVector(VECTOR_INT, T_Vector);
							}

							for (int row = 0; row < deleteLength; row++) {
								int type = updateTypeForEachTupleArr[row];
								char *gbVal = gbValsDeleteArr[row];
								int psVal = INT_VALUE((Constant *) MAP_GET_STRING(storedPs, gbVal));
								vecAppendInt(outputPSDelete, psVal);
								if (type == -1) {
									removeMapStringElem(gbFragCnt, gbVal);
									removeMapStringElem(storedPs, gbVal);
								} else if (type == 0) {
									vecAppendInt(outputPSInsert, psVal);
									HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
									Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
									// (*((int *) cnt->value)) -= 1;
									INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
								}
							}
						} else {
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							if (outputPSDelete == NULL) {
								outputPSDelete = makeVector(VECTOR_NODE, T_Vector);
							}
							if (outputPSInsert == NULL) {
								outputPSInsert = makeVector(VECTOR_NODE, T_Vector);
							}
							if (inputPSIsInt) {
								int *inputPSArr = (int *) VEC_TO_IA(inputPS);
								for (int row = 0; row < deleteLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = gbValsDeleteArr[row];
									int psVal = inputPSArr[row];
									BitSet *oldBitset = (BitSet *) MAP_GET_STRING(storedPs, gbVal);
									vecAppendNode(outputPSDelete, (Node *) copyObject(oldBitset));
									if (type == -1) {
										removeMapStringElem(storedPs, gbVal);
										removeMapStringElem(gbFragCnt, gbVal);
									} else if (type == 0) {
										HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
										if(INT_VALUE(cnt) <= 1) {
											// this case; this frag cnt ->0
											// maybe other frag has cnt;
											removeMapElem(fragCnt, (Node *) createConstInt(psVal));
											setBit(oldBitset, psVal, FALSE);
										} else {
											// (*((int *) cnt->value)) -= 1;
											INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
										}

										vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitset));
									}
								}
							} else {
								BitSet **inputPSArr = VEC_TO_ARR(inputPS, BitSet);
								int provSketchLen = INT_VALUE((Constant *) MAP_GET_STRING(groupPSMap->provLens, STRING_VALUE(c)));
								for (int row = 0; row < deleteLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = (char *) gbValsDeleteArr[row];
									BitSet *oldBitset = (BitSet *) MAP_GET_STRING(storedPs, gbVal);
									vecAppendNode(outputPSDelete, (Node *) copyObject(oldBitset));
									if (type == -1) {
										removeMapStringElem(storedPs, gbVal);
										removeMapStringElem(gbFragCnt, gbVal);
									} else if (type == 0) {
										HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										char *provStr = bitSetToString((BitSet *) inputPSArr[row]);
										for (int bitI = 0; bitI < provSketchLen; bitI++) {
											if (provStr[bitI] == '1') {
												Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, bitI);
												if (INT_VALUE(cnt) < 2) {
													removeMapElem(fragCnt, (Node *) createConstInt(bitI));
													setBit(oldBitset, bitI, FALSE);
												} else {
													// (*((int *) cnt->value)) -= 1;
													INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
												}
											}
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitset));
									}
								}
							}
						}

						addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSDelete);
						addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSInsert);
					}
					hasFinishPSDelete = TRUE;
					resultDCInsert->isAPSChunk = TRUE;
					resultDCDelete->isAPSChunk = TRUE;
				}
			}
		} else if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0
		|| strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
			// get heap;
			GBHeaps *gbHeap = (GBHeaps *) MAP_GET_STRING(dataStructures, nameInDS);

			// is a psChunk;
			boolean isAPSChunk = FALSE;

			// input vec pos and type;
			int inputVecPos = 0;
			DataType inputVecType = DT_INT;

			// output vec pos and type;
			Constant *nameInOutChunk = (Constant *) MAP_GET_STRING(mapFCsToSchemas, nameInDS);
			int outputVecPos = 0;
			// DataType outputVecType = DT_INT;

			if (dataChunkInsert != NULL) {
				isAPSChunk = dataChunkInsert->isAPSChunk;
				inputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunkInsert->attriToPos, ar->name));
				inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkInsert->posToDatatype, inputVecPos));

				outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCInsert->attriToPos, STRING_VALUE(nameInOutChunk)));
				// outputVecType = INT_VALUE((Constant *) MAP_GET_INT(resultDCInsert->posToDatatype, outputVecPos));
			} else if (dataChunkDelete != NULL) {
				isAPSChunk = dataChunkDelete->isAPSChunk;
				inputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunkDelete->attriToPos, ar->name));
				inputVecType = INT_VALUE((Constant *) MAP_GET_INT(dataChunkDelete->posToDatatype, inputVecPos));
				outputVecPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCDelete->attriToPos, STRING_VALUE(nameInOutChunk)));
				// outputVecType = INT_VALUE((Constant *) MAP_GET_INT(resultDCDelete->posToDatatype, outputVecPos));
			}

			Vector *outputVecInsert = (Vector *) getVecNode(resultDCInsert->tuples, outputVecPos);
			Vector *outputVecDelete = (Vector *) getVecNode(resultDCDelete->tuples, outputVecPos);


			if (dataChunkInsert != NULL) {
				Vector *inputVec = (Vector *) getVecNode(dataChunkInsert->tuples, inputVecPos);
				Vector *updateTypeForEachTuple = makeVector(VECTOR_INT, T_Vector);

				switch(inputVecType) {
					case DT_INT:
					{
						int *inputVecVals = (int *) VEC_TO_IA(inputVec);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									vecAppendInt(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendInt(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										int oldMin = INT_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendInt(outputVecDelete, oldMin);
										RBTInsert(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
										int newMin = INT_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendInt(outputVecInsert, newMin);
									}
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									vecAppendInt(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendInt(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										int oldMax = INT_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendInt(outputVecDelete, oldMax);
										RBTInsert(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
										int newMax = INT_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendInt(outputVecInsert, newMax);
									}
								}
							}
						}
					}
					break;
					case DT_BOOL:
					{
						int *inputVecVals = (int *) VEC_TO_IA(inputVec) ;
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									vecAppendInt(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendInt(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										int oldMin = BOOL_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendInt(outputVecDelete, oldMin);
										RBTInsert(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
										int newMin = BOOL_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendInt(outputVecInsert, newMin);
									}
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0){
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									vecAppendInt(outputVecInsert, inputVecVals[row]);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									RBTInsert(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendInt(outputVecInsert, inputVecVals[row]);
										tree = makeRBT(RBT_MIN_HEAP, FALSE);
										RBTInsert(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										int oldMax = BOOL_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendInt(outputVecDelete, oldMax);
										RBTInsert(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
										int newMax = BOOL_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendInt(outputVecInsert, newMax);
									}
								}
							}
						}
					}
					break;
					case DT_LONG:
					{
						gprom_long_t *inputVecVals = (gprom_long_t *) VEC_TO_LA(inputVec) ;
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									vecAppendLong(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendLong(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										gprom_long_t oldMin = LONG_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendLong(outputVecDelete, oldMin);
										RBTInsert(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
										gprom_long_t newMin = LONG_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendLong(outputVecInsert, newMin);
									}
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0){
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									vecAppendLong(outputVecInsert, inputVecVals[row]);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									RBTInsert(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendLong(outputVecInsert, inputVecVals[row]);
										tree = makeRBT(RBT_MIN_HEAP, FALSE);
										RBTInsert(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										gprom_long_t oldMax = LONG_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendLong(outputVecDelete, oldMax);
										RBTInsert(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
										gprom_long_t newMax = LONG_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendLong(outputVecInsert, newMax);
									}
								}
							}
						}
					}
					break;
					case DT_FLOAT:
					{
						double *inputVecVals = (double *) VEC_TO_FA(inputVec);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MAX_HEAP, FALSE);
									vecAppendFloat(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendFloat(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										double oldMin = FLOAT_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendFloat(outputVecDelete, oldMin);
										RBTInsert(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
										double newMin = FLOAT_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendFloat(outputVecInsert, newMin);
									}
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MAX_HEAP, FALSE);
									vecAppendFloat(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);

								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendFloat(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										double oldMax = FLOAT_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendFloat(outputVecDelete, oldMax);
										RBTInsert(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
										double newMax = FLOAT_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendFloat(outputVecInsert, newMax);
									}
								}
							}
						}
					}
					break;
					case DT_STRING:
					case DT_VARCHAR2:
					{
						char ** inputVecVals = (char **) VEC_TO_ARR(inputVec, char);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									vecAppendString(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstString(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendString(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstString(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										char *oldMin = STRING_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendString(outputVecDelete, strdup(oldMin));
										RBTInsert(tree, (Node *) createConstString(inputVecVals[row]), NULL);
										char *newMin = STRING_VALUE((Constant *) RBTGetMin(tree)->key);
										vecAppendString(outputVecInsert, strdup(newMin));
									}
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < insertLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsInsertArr[row]);
								if (tree == NULL) {
									vecAppendInt(updateTypeForEachTuple, 1);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									tree = makeRBT(RBT_MIN_HEAP, FALSE);
									vecAppendString(outputVecInsert, inputVecVals[row]);
									RBTInsert(tree, (Node *) createConstString(inputVecVals[row]), NULL);
									addToMap(gbHeap->heapLists, (Node *) createConstString(gbValsInsertArr[row]), (Node *) tree);
								} else {
									if (tree->size == 0) {
										vecAppendInt(updateTypeForEachTuple, 1);
										vecAppendString(outputVecInsert, inputVecVals[row]);
										RBTInsert(tree, (Node *) createConstString(inputVecVals[row]), NULL);
									} else {
										vecAppendInt(updateTypeForEachTuple, 0);
										char *oldMax = STRING_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendString(outputVecDelete, strdup(oldMax));
										RBTInsert(tree, (Node *) createConstString(inputVecVals[row]), NULL);
										char *newMax = STRING_VALUE((Constant *) RBTGetMax(tree)->key);
										vecAppendString(outputVecInsert, strdup(newMax));
									}
								}

							}
						}
					}
					break;
				}

				if (!hasFinishGBInsert && !noGB) {
					for (int gbAttrIndex = 0; gbAttrIndex < gbAttrCnt; gbAttrIndex++) {
						int fromPos = getVecInt(gbPoss, gbAttrIndex);
						DataType gbDataType = (DataType) getVecInt(gbType, gbAttrIndex);
						char *gbAttrName = (char *) getVecString(gbName, gbAttrIndex);

						char *toName = STRING_VALUE((Constant *) MAP_GET_STRING(mapFCsToSchemas, gbAttrName));
						int toPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCInsert->attriToPos, toName));

						Vector *fromVec = (Vector *) getVecNode(dataChunkInsert->tuples, fromPos);
						Vector *resVecIns = (Vector *) getVecNode(resultDCInsert->tuples, toPos);
						Vector *resVecDel = (Vector *) getVecNode(resultDCDelete->tuples, toPos);

						// Vector *updateIdentIns = (Vector *) resultDCInsert->updateIdentifier;
						// Vector *updateIdentDel = (Vector *) resultDCDelete->updateIdentifier;
						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);
						switch(gbDataType) {
							case DT_BOOL:
							case DT_INT:
							{
								int *values = (int *) VEC_TO_IA(fromVec);
								for (int row = 0; row < insertLength; row++) {
									int val = values[row];
									vecAppendInt(resVecIns, val);
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendInt(resVecDel, val);
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
							break;
							case DT_LONG:
							{
								gprom_long_t *values = (gprom_long_t *) VEC_TO_LA(fromVec);
								for (int row = 0; row < insertLength; row++) {
									gprom_long_t val = values[row];
									vecAppendLong(resVecIns, val);
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendLong(resVecDel, val);
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
							break;
							case DT_FLOAT:
							{
								double *values = (double *) VEC_TO_FA(fromVec);
								for (int row = 0; row < insertLength; row++) {
									double val = values[row];
									vecAppendFloat(resVecIns, val);
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendFloat(resVecDel, val);
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
							break;
							case DT_STRING:
							case DT_VARCHAR2:
							{
								char **values = (char **) VEC_TO_ARR(fromVec, char);
								for (int row = 0; row < insertLength; row++) {
									char *val = values[row];
									vecAppendString(resVecIns, strdup(val));
									// vecAppendInt(updateIdentIns, 1);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendString(resVecDel, strdup(val));
										// vecAppendInt(updateIdentDel, -1);
									}
								}
							}
							break;
						}
					}
					hasFinishGBInsert = TRUE;
				}

				if (isAPSChunk && !hasFinishPSInsert) {
					PSMap *groupPSMap = (PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_AGG);

					// if (groupPSMap == NULL) {
						// groupPSMap = makePSMap();
					// }

					FOREACH_HASH_KEY(Constant, c, dataChunkInsert->fragmentsInfo) {
						boolean inputPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(dataChunkInsert->fragmentsIsInt, STRING_VALUE(c)));

						Vector *inputPS = (Vector *) MAP_GET_STRING(dataChunkInsert->fragmentsInfo, STRING_VALUE(c));

						boolean storedPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(groupPSMap->isIntSketch, STRING_VALUE(c)));

						HashMap *storedPS = (HashMap *) MAP_GET_STRING(groupPSMap->provSketchs, STRING_VALUE(c));
						if (storedPS == NULL) {
							storedPS = NEW_MAP(Constant, Node);
						}

						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(groupPSMap->fragCount, STRING_VALUE(c));
						if (gbFragCnt == NULL) {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						Vector *outputPSInsert = NULL;
						Vector *outputPSDelete = NULL;

						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);
						if (storedPSIsInt) {
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));

							outputPSDelete = makeVector(VECTOR_INT, T_Vector);
							outputPSInsert = makeVector(VECTOR_INT, T_Vector);

							int *inputPSArr = (int *) VEC_TO_IA(inputPS);

							for (int row = 0; row < insertLength; row++) {
								int type = updateTypeForEachTupleArr[row];
								char *gbVal = gbValsInsertArr[row];

								HashMap *fragCnt = NULL;
								int psVal = inputPSArr[row];
								if (type == 1) {
									vecAppendInt(outputPSInsert, psVal);
									addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) createConstInt(psVal));
									fragCnt = NEW_MAP(Constant, Constant);
									addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));

									addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
								} else if (type == 0) {
									vecAppendInt(outputPSDelete, psVal);
									vecAppendInt(outputPSInsert, psVal);
									fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
									Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
									incrConst(cnt);
								}
							}
							addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						} else {
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));

							int provSketchLen = INT_VALUE((Constant *) MAP_GET_STRING(groupPSMap->provLens, STRING_VALUE(c)));
							outputPSDelete = makeVector(VECTOR_NODE, T_Vector);
							outputPSInsert = makeVector(VECTOR_NODE, T_Vector);

							if (inputPSIsInt) {
								int *inputPSArr = (int *) VEC_TO_IA(inputPS);
								for (int row = 0; row < insertLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = gbValsInsertArr[row];
									HashMap *fragCnt = NULL;
									int psVal = inputPSArr[row];
									if (type == 1) {
										BitSet *bitSet = newBitSet(provSketchLen);
										setBit(bitSet, psVal, TRUE);
										vecAppendNode(outputPSInsert, (Node *) copyObject(bitSet));
										addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) bitSet);
										fragCnt = NEW_MAP(Constant, Constant);
										addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));

										addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
									} else if (type == 0) {
										BitSet *oldBitset = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
										vecAppendNode(outputPSDelete, (Node *) copyObject(oldBitset));

										setBit(oldBitset, psVal, TRUE);

										fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
										if (cnt == NULL) {
											addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));
										} else {
											incrConst(cnt);
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitset));
									}
								}
								addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							} else {
								BitSet **inputPSArr = (BitSet **) VEC_TO_ARR(inputPS, BitSet);

								for (int row = 0; row < insertLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = gbValsInsertArr[row];
									HashMap *fragCnt = NULL;
									if (type == 1) {
										BitSet *bitset = (BitSet *) inputPSArr[row];
										vecAppendNode(outputPSInsert, (Node *) copyObject(bitset));
										addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitset));
										char *provStr = bitSetToString(bitset);
										fragCnt = NEW_MAP(Constant, Constant);
										for (int bitIndex = 0; bitIndex < provSketchLen; bitIndex++) {
											if (provStr[bitIndex] == '1') {
												addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
											}
										}
										addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
									} else {
										BitSet *bitSet = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
										vecAppendNode(outputPSDelete, (Node *) copyObject(bitSet));
										fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										BitSet *inputBitset = (BitSet *) inputPSArr[row];
										char *provStr = bitSetToString(inputBitset);
										for (int bitIndex = 0; bitIndex < provSketchLen; bitIndex++) {
											if (provStr[bitIndex] == '1') {
												Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, bitIndex);
												if (cnt == NULL) {
													addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
												} else {
													incrConst(cnt);
												}
												setBit(bitSet, bitIndex, TRUE);
											}
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(bitSet));
										addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
										addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitSet));
									}
								}
								addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
							}
						}
						addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSInsert);
						addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSDelete);
						addToMap(groupPSMap->provSketchs, (Node *) copyObject(c), (Node *) storedPS);
						addToMap(dataStructures, (Node *) createConstString(PROP_PROV_SKETCH_AGG), (Node *) groupPSMap);
					}
					hasFinishPSInsert = TRUE;
					resultDCInsert->isAPSChunk = TRUE;
					resultDCDelete->isAPSChunk = TRUE;

				}
			}

			// for deletion, there must exist a rbt for a group, if not build is wrong.
			if (dataChunkDelete != NULL) {
				Vector *inputVec = (Vector *) getVecNode(dataChunkDelete->tuples, inputVecPos);
				Vector *updateTypeForEachTuple = makeVector(VECTOR_INT, T_Vector);
				switch(inputVecType) {
					case DT_INT:
					{
						int *inputVecVals = (int *) VEC_TO_IA(inputVec);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								DEBUG_NODE_BEATIFY_LOG("TREE BEFORE UPDATE", tree);
								int oldMin = INT_VALUE((Constant *) RBTGetMin(tree)->key);
								vecAppendInt(outputVecDelete, oldMin);
								RBTDelete(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
									INFO_LOG("group: %s, oldMin: %d, deleteval: %d, newmin: %d", gbValsDeleteArr[row], oldMin, inputVecVals[row], 999999);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									int newMin = INT_VALUE((Constant *) RBTGetMin(tree)->key);
									INFO_LOG("group: %s, oldMin: %d, deleteval: %d, newmin: %d", gbValsDeleteArr[row], oldMin, inputVecVals[row], newMin);
									vecAppendInt(outputVecInsert, newMin);
								}

								DEBUG_NODE_BEATIFY_LOG("TREE", tree);
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								int oldMax = INT_VALUE((Constant *) RBTGetMax(tree)->key);
								vecAppendInt(outputVecDelete, oldMax);
								RBTDelete(tree, (Node *) createConstInt(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									int newMax = INT_VALUE((Constant *) RBTGetMax(tree)->key);
									vecAppendInt(outputVecDelete, newMax);
								}
							}
						}
					}
					break;
					case DT_BOOL:
					{
						int *inputVecVals = (int *) VEC_TO_IA(inputVec);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								int oldMin = BOOL_VALUE((Constant *) RBTGetMin(tree)->key);
								vecAppendInt(outputVecDelete, oldMin);
								RBTDelete(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									int newMin = BOOL_VALUE((Constant *) RBTGetMin(tree)->key);
									vecAppendInt(outputVecInsert, newMin);
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								int oldMax = BOOL_VALUE((Constant *) RBTGetMax(tree)->key);
								vecAppendInt(outputVecDelete, oldMax);
								RBTDelete(tree, (Node *) createConstBool(inputVecVals[row] != 0), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									int newMax = BOOL_VALUE((Constant *) RBTGetMax(tree)->key);
									vecAppendInt(outputVecInsert, newMax);
								}
							}
						}
					}
					break;
					case DT_LONG:
					{
						gprom_long_t *inputVecVals = (gprom_long_t *) VEC_TO_LA(inputVec);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								gprom_long_t oldMin = LONG_VALUE((Constant *) RBTGetMin(tree)->key);
								vecAppendLong(outputVecDelete, oldMin);
								RBTDelete(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									gprom_long_t newMin = LONG_VALUE((Constant *) RBTGetMin(tree)->key);
									vecAppendLong(outputVecInsert, newMin);
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								gprom_long_t oldMax = LONG_VALUE((Constant *) RBTGetMax(tree)->key);
								vecAppendLong(outputVecDelete, oldMax);
								RBTDelete(tree, (Node *) createConstLong(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									gprom_long_t newMax = LONG_VALUE((Constant *) RBTGetMax(tree)->key);
									vecAppendLong(outputVecInsert, newMax);
								}
							}
						}
					}
					break;
					case DT_FLOAT:
					{
						double *inputVecVals = (double *) VEC_TO_FA(inputVec);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								double oldMin = FLOAT_VALUE((Constant *) RBTGetMin(tree)->key);
								vecAppendFloat(outputVecDelete, oldMin);
								RBTDelete(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									double newMin = FLOAT_VALUE((Constant *) RBTGetMin(tree)->key);
									vecAppendFloat(outputVecInsert, newMin);
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								double oldMax = FLOAT_VALUE((Constant *) RBTGetMax(tree)->key);
								vecAppendFloat(outputVecDelete, oldMax);
								RBTDelete(tree, (Node *) createConstFloat(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									double newMax = FLOAT_VALUE((Constant *) RBTGetMax(tree)->key);
									vecAppendFloat(outputVecInsert, newMax);
								}
							}
						}
					}
					break;
					case DT_STRING:
					case DT_VARCHAR2:
					{
						char **inputVecVals = (char **) VEC_TO_ARR(inputVec, char);
						if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								char *oldMin = STRING_VALUE((Constant *) RBTGetMin(tree)->key);
								vecAppendString(outputVecDelete, strdup(oldMin));
								RBTDelete(tree, (Node *) createConstString(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									char *newMin = STRING_VALUE((Constant *) RBTGetMin(tree)->key);
									vecAppendString(outputVecInsert, strdup(newMin));
								}
							}
						} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
							for (int row = 0; row < deleteLength; row++) {
								RBTRoot *tree = (RBTRoot *) MAP_GET_STRING(gbHeap->heapLists, gbValsDeleteArr[row]);
								char *oldMax = STRING_VALUE((Constant *) RBTGetMax(tree)->key);
								vecAppendString(outputVecDelete, strdup(oldMax));
								RBTDelete(tree, (Node *) createConstString(inputVecVals[row]), NULL);
								if (tree->size == 0) {
									vecAppendInt(updateTypeForEachTuple, -1);
								} else {
									vecAppendInt(updateTypeForEachTuple, 0);
									char *newMax = STRING_VALUE((Constant *) RBTGetMax(tree)->key);
									vecAppendString(outputVecInsert, strdup(newMax));
								}
							}
						}
					}
					break;
				}


				// deal with gb attrs;
				if (!hasFinishGBDelete && !noGB) {
					for (int gbAttrIndex = 0; gbAttrIndex < gbAttrCnt; gbAttrIndex++) {
						int fromPos = getVecInt(gbPoss, gbAttrIndex);
						DataType gbDataType = (DataType) getVecInt(gbType, gbAttrIndex);
						char *gbAttrName = (char *) getVecString(gbName, gbAttrIndex);

						char *toName = STRING_VALUE((Constant *) MAP_GET_STRING(mapFCsToSchemas, gbAttrName));
						int toPos = INT_VALUE((Constant *) MAP_GET_STRING(resultDCDelete->attriToPos, toName));

						Vector *fromVec = (Vector *) getVecNode(dataChunkDelete->tuples, fromPos);

						Vector *resVecIns = (Vector *) getVecNode(resultDCInsert->tuples, toPos);
						Vector *resVecDel = (Vector *) getVecNode(resultDCDelete->tuples, toPos);

						// Vector *updateIdentIns = (Vector *) resultDCInsert->updateIdentifier;
						// Vector *updateIdentDel = (Vector *) resultDCDelete->updateIdentifier;
						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);
						switch(gbDataType) {
							case DT_BOOL:
							case DT_INT:
							{
								int *values = (int *) VEC_TO_IA(fromVec);
								for (int row = 0; row < deleteLength; row++) {
									int val = values[row];
									vecAppendInt(resVecDel, val);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendInt(resVecIns, val);
									}
								}
							}
							break;
							case DT_LONG:
							{
								gprom_long_t *values = (gprom_long_t *) VEC_TO_LA(fromVec);
								for (int row = 0; row < deleteLength; row++) {
									gprom_long_t val = values[row];
									vecAppendLong(resVecDel, val);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendLong(resVecIns, val);
									}
								}
							}
							break;
							case DT_FLOAT:
							{
								double *values = (double *) VEC_TO_FA(fromVec);
								for (int row = 0; row < deleteLength; row++) {
									double val = values[row];
									vecAppendFloat(resVecDel, val);
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendFloat(resVecIns, val);
									}
								}
							}
							break;
							case DT_STRING:
							case DT_VARCHAR2:
							{
								char **values = VEC_TO_ARR(fromVec, char);
								for (int row = 0; row < deleteLength; row++) {
									char *val = values[row];
									vecAppendString(resVecDel, strdup(val));
									if (updateTypeForEachTupleArr[row] == 0) {
										vecAppendString(resVecIns, strdup(val));
									}
								}
							}
							break;
							default:
								FATAL_LOG("not supported");

						}
					}
					hasFinishGBDelete = TRUE;
				}

				// dealing with ps;
				if (isAPSChunk && !hasFinishPSDelete) {
					PSMap *groupPSMap = (PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_AGG);
					// if (groupPSMap == NULL) {
						// groupPSMap = NEW_MAP(Constant, Node);
					// }

					FOREACH_HASH_KEY(Constant, c, dataChunkDelete->fragmentsInfo) {
						boolean inputPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(dataChunkDelete->fragmentsIsInt, STRING_VALUE(c)));
						Vector *inputPS = (Vector *) MAP_GET_STRING(dataChunkDelete->fragmentsInfo, STRING_VALUE(c));

						boolean storedPSIsInt = BOOL_VALUE((Constant *) MAP_GET_STRING(groupPSMap->isIntSketch,STRING_VALUE(c)));
						HashMap *storedPs = (HashMap *) MAP_GET_STRING(groupPSMap->provSketchs, STRING_VALUE(c));

						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(groupPSMap->fragCount, STRING_VALUE(c));

						Vector *outputPSInsert = (Vector *) MAP_GET_STRING(resultDCInsert->fragmentsInfo, STRING_VALUE(c));
						Vector *outputPSDelete = (Vector *) MAP_GET_STRING(resultDCDelete->fragmentsInfo, STRING_VALUE(c));

						int *updateTypeForEachTupleArr = (int *) VEC_TO_IA(updateTypeForEachTuple);

						if (storedPSIsInt) {
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));

							if (outputPSDelete == NULL) {
								outputPSDelete = makeVector(VECTOR_INT, T_Vector);
							}
							if (outputPSInsert == NULL) {
								outputPSInsert = makeVector(VECTOR_INT, T_Vector);
							}

							for (int row = 0; row < deleteLength; row++) {
								int type = updateTypeForEachTupleArr[row];
								char *gbVal = gbValsDeleteArr[row];
								int psVal = INT_VALUE((Constant *) MAP_GET_STRING(storedPs, gbVal));
								vecAppendInt(outputPSDelete, psVal);
								if (type == -1) {
									removeMapStringElem(gbFragCnt, gbVal);
									removeMapStringElem(storedPs, gbVal);
								} else if (type == 0) {
									vecAppendInt(outputPSInsert, psVal);
									HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
									Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
									// incrConst(cnt);
									INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
								}
							}
						} else {
							addToMap(resultDCInsert->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							addToMap(resultDCDelete->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							if (outputPSDelete == NULL) {
								outputPSDelete = makeVector(VECTOR_NODE, T_Vector);
							}
							if (outputPSInsert == NULL) {
								outputPSInsert = makeVector(VECTOR_NODE, T_Vector);
							}

							if (inputPSIsInt) {
								int *inputPSArr = (int *) VEC_TO_IA(inputPS);
								for (int row = 0; row < deleteLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = gbValsDeleteArr[row];
									int psVal = inputPSArr[row];
									BitSet *oldBitset = (BitSet *) MAP_GET_STRING(storedPs, gbVal);
									vecAppendNode(outputPSDelete, (Node *) copyObject(oldBitset));
									if (type == -1) {
										removeMapStringElem(storedPs, gbVal);
										removeMapStringElem(gbFragCnt, gbVal);
									} else {
										HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
										if (INT_VALUE(cnt) < 2) {
											removeMapElem(fragCnt, (Node *) createConstInt(psVal));
											setBit(oldBitset, psVal, FALSE);
										} else {
											// (*((int *) cnt->value)) -= 1;
											INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitset));
									}

									// TODO: MOVE BELOW TO TYPE = 0 ;
									// vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitset));
								}
							} else {
								BitSet **inputPSArr = (BitSet **) VEC_TO_ARR(inputPS, BitSet);
								int provSketchLen = INT_VALUE((Constant *) MAP_GET_STRING(groupPSMap->provLens, STRING_VALUE(c)));
								for (int row = 0; row < deleteLength; row++) {
									int type = updateTypeForEachTupleArr[row];
									char *gbVal = (char *) gbValsDeleteArr[row];
									BitSet *oldBitSet = (BitSet *) MAP_GET_STRING(storedPs, gbVal);
									vecAppendNode(outputPSDelete, (Node *) copyObject(oldBitSet));
									if (type == -1) {
										removeMapStringElem(storedPs, gbVal);
										removeMapStringElem(gbFragCnt, gbVal);
									} else {
										HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
										char *provStr = bitSetToString((BitSet *) inputPSArr[row]);
										for (int bitIndex = 0; bitIndex < provSketchLen; bitIndex++) {
											if (provStr[bitIndex] == '1') {
												Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, bitIndex);
												if (INT_VALUE(cnt) < 2) {
													removeMapElem(fragCnt, (Node *) createConstInt(bitIndex));
													setBit(oldBitSet, bitIndex, FALSE);
												} else {
													// (*((int *) cnt->value)) -= 1;
													INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
												}
											}
										}
										vecAppendNode(outputPSInsert, (Node *) copyObject(oldBitSet));
									}
								}
							}
						}

						addToMap(resultDCDelete->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSDelete);
						addToMap(resultDCInsert->fragmentsInfo, (Node *) copyObject(c), (Node *) outputPSInsert);
					}
					hasFinishPSDelete = TRUE;
					resultDCInsert->isAPSChunk = TRUE;
					resultDCDelete->isAPSChunk = TRUE;
				}
			}
		}
		// hasFinishPSInsert = TRUE;
		// hasFinishGBInsert = TRUE;
	}

	// fill group by attrs and update identifier;
	// Vector *updIdenIns = resultDCInsert->updateIdentifier;
	// Vector *updIdenDel = resultDCDelete->updateIdentifier;
	DEBUG_NODE_BEATIFY_LOG("AGG STATE AFTER UPDATE", dataStructures);
	// INFO_OP_LOG("AGG OP", op);
	// DEBUG_NODE_BEATIFY_LOG("gb names:", gbName);
	// DEBUG_NODE_BEATIFY_LOG("gb POS:", gbPoss);
	// DEBUG_NODE_BEATIFY_LOG("gb TYPE:", gbType);
	// DEBUG_NODE_BEATIFY_LOG("gb list", aggGBList);
	// DEBUG_NODE_BEATIFY_LOG("args list:", aggFCList);
	// DEBUG_NODE_BEATIFY_LOG("data chunk:", chunkMaps);

	// INFO_LOG("gbAttrCnt: %d", gbAttrCnt);
	// set data chunk numTuple;
	resultDCInsert->numTuples = ((Vector *) getVecNode(resultDCInsert->tuples, 0))->length;
	for (int row = 0; row < resultDCInsert->numTuples; row++) {
		vecAppendInt(resultDCInsert->updateIdentifier, 1);
	}
	resultDCDelete->numTuples = ((Vector *) getVecNode(resultDCDelete->tuples, 0))->length;
	for (int row = 0; row < resultDCDelete->numTuples; row++) {
		vecAppendInt(resultDCDelete->updateIdentifier, -1);
	}

	if (dataChunkInsert != NULL) {
		resultDCInsert->isAPSChunk = dataChunkInsert->isAPSChunk;
		resultDCDelete->isAPSChunk = dataChunkInsert->isAPSChunk;
	} else if (dataChunkDelete != NULL) {
		resultDCInsert->isAPSChunk = dataChunkDelete->isAPSChunk;
		resultDCDelete->isAPSChunk = dataChunkDelete->isAPSChunk;
	}

	HashMap *resChunkMaps = NEW_MAP(Constant, Node);

	if (resultDCInsert->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_INSERT, resultDCInsert);
	}
	if (resultDCDelete->numTuples > 0) {
		MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_DELETE, resultDCDelete);
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
	INFO_LOG("UPDATE DUPLICATE REMOVAL");
	updateByOperators(OP_LCHILD(op));

	QueryOperator *lchild = OP_LCHILD(op);
	if (!HAS_STRING_PROP(lchild, PROP_DATA_CHUNK)) {
		return;
	} else {
		INFO_LOG("LCHILD IS NOT NULL");
	}

	HashMap *chunkMaps = (HashMap *) GET_STRING_PROP(lchild, PROP_DATA_CHUNK);
	DataChunk* dcIns = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_INSERT);
	DataChunk* dcDel = (DataChunk *) MAP_GET_STRING(chunkMaps, PROP_DATA_CHUNK_DELETE);

	List* dupAttrs = op->schema->attrDefs;
	int tups = LIST_LENGTH(dupAttrs);

	DataChunk* resDCIns = initDataChunk();
	resDCIns->attrNames = (List *) copyObject(dupAttrs);
	resDCIns->tupleFields = tups;

	DataChunk* resDCDel = initDataChunk();
	resDCDel->attrNames = (List *) copyObject(dupAttrs);
	resDCDel->tupleFields = tups;

	for (int i = 0; i < tups; i++) {
		AttributeDef *ad = (AttributeDef *) getNthOfListP(dupAttrs, i);
		addToMap(resDCIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(i));
		addToMap(resDCIns->posToDatatype, (Node *) createConstInt(i), (Node *) createConstInt(ad->dataType));

		addToMap(resDCDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(i));
		addToMap(resDCDel->posToDatatype, (Node *) createConstInt(i), (Node *) createConstInt(ad->dataType));

		switch (ad->dataType) {
			case DT_INT:
			case DT_BOOL:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
			}
			break;
			case DT_LONG:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
			}
			break;
			case DT_FLOAT:
			{
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
			}
			break;
			case DT_STRING:
			case DT_VARCHAR2:
			{

				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
			}
			break;
			default:
				FATAL_LOG("data type is not supported");
		}
	}

	Vector *gbValsIns = NULL;
	Vector *gbValsDel = NULL;
	List *gbList = NIL;
	int gbpos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		AttributeReference *ar = createFullAttrReference(ad->attrName, 0, gbpos, 0, ad->dataType);
		gbList = appendToTailOfList(gbList, ar);
		gbpos++;
	}
	if (dcIns != NULL) {
		gbValsIns = buildGroupByValueVecFromDataChunk(dcIns, gbList);
	}

	if (dcDel != NULL) {
		gbValsDel = buildGroupByValueVecFromDataChunk(dcDel, gbList);
	}
	HashMap *dataStructures = (HashMap *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);
	int insLen = 0;
	int delLen = 0;
	if (dcIns != NULL) {
		char ** gbInsVals = (char **) VEC_TO_ARR(gbValsIns, char);
		GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructures, PROP_DATA_STRUCTURE_DUP_DATA);
		Vector *updType = makeVector(VECTOR_INT, T_Vector);

		// update acs;
		for (int row = 0; row < dcIns->numTuples; row++) {
			if (MAP_HAS_STRING_KEY(acs->map, gbInsVals[row])) {
				insLen++;
				delLen++;
				vecAppendInt(updType, 0);
				Vector *cntV = (Vector *) MAP_GET_STRING(acs->map, gbInsVals[row]);
				Constant *cnt = (Constant *) getVecNode(cntV, 0);
				incrConst(cnt);
			} else {
				insLen++;
				vecAppendInt(updType, 1);
				Vector *cntV = makeVector(VECTOR_NODE, T_Vector);
				vecAppendNode(cntV, (Node *) createConstLong(1));
				addToMap(acs->map, (Node *) createConstString(gbInsVals[row]), (Node *) cntV);
			}
		}

		// get return data;
		int *updTypeArr = (int *) VEC_TO_IA(updType);
		for (int col = 0; col < tups; col++) {
			DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(resDCIns->posToDatatype, col));
			switch (dt) {
				case DT_INT:
				case DT_BOOL:
				{
					int *fromVecVals = (int *) VEC_TO_IA((Vector *) getVecNode(dcIns->tuples, col));
					for (int row = 0; row < dcIns->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendInt((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
							vecAppendInt((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);

						} else {
							vecAppendInt((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						}
					}
				}
				break;
				case DT_LONG:
				{
					gprom_long_t *fromVecVals = (gprom_long_t *) VEC_TO_LA((Vector *) getVecNode(dcIns->tuples, col));
					for (int row = 0; row < dcIns->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendLong((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
							vecAppendLong((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						} else {
							vecAppendLong((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						}
					}
				}
				break;
				case DT_FLOAT:
				{
					double *fromVecVals = (double *) VEC_TO_FA((Vector *) getVecNode(dcIns->tuples, col));
					for (int row = 0; row < dcIns->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendFloat((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
							vecAppendFloat((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						} else {
							vecAppendFloat((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						}
					}
				}
				break;
				case DT_VARCHAR2:
				case DT_STRING:
				{
					char ** fromVecVals = (char **) VEC_TO_ARR((Vector *) getVecNode(dcIns->tuples, col), char);
					for (int row = 0; row < dcIns->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendString((Vector *) getVecNode(resDCDel->tuples, col), strdup(fromVecVals[row]));
							vecAppendString((Vector *) getVecNode(resDCIns->tuples, col), strdup(fromVecVals[row]));
						} else {
							vecAppendString((Vector *) getVecNode(resDCIns->tuples, col), strdup(fromVecVals[row]));
						}
					}
				}
				break;
				default:
					FATAL_LOG("not supported");
			}
		}

		// update ps and get ps for each input;
		if (dcIns->isAPSChunk == TRUE) {
			resDCIns->isAPSChunk = TRUE;
			resDCDel->isAPSChunk = TRUE;
			PSMap *psMap = (PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_DUP);
			if (psMap == NULL) {
				psMap = makePSMap();
			}

			FOREACH_HASH_KEY(Constant, c, dcIns->fragmentsInfo) {
				boolean isIntPS = BOOL_VALUE((Constant *) MAP_GET_STRING(dcIns->fragmentsIsInt, STRING_VALUE(c)));



				// Vector *inputPS = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, STRING_VALUE(c));

				boolean isStorePSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(psMap->isIntSketch, STRING_VALUE(c)));

				HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
				if (gbFragCnt == NULL) {
					gbFragCnt = NEW_MAP(Constant, Node);
					addToMap(psMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
				}

				Vector *outPSIns = NULL;
				Vector *outPSDel = NULL;

				HashMap *storedPS = (HashMap *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
				if (storedPS == NULL) {
					storedPS = NEW_MAP(Constant, Node);
					addToMap(psMap->provSketchs, (Node *) copyObject(c), (Node *) storedPS);
				}
				Vector *inputPS = (Vector *) MAP_GET_STRING(dcIns->fragmentsInfo, STRING_VALUE(c));
				if (isStorePSInt) {
					outPSIns = makeVector(VECTOR_INT, T_Vector);
					outPSDel = makeVector(VECTOR_INT, T_Vector);
					addToMap(resDCIns->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
					addToMap(resDCDel->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));

					int *psVals = (int *) VEC_TO_IA(inputPS);

					for (int row = 0; row < dcIns->numTuples; row++) {
						int type = updTypeArr[row];
						char *gbVal = gbInsVals[row];
						HashMap *fragCnt = NULL;
						if (type == 1) {
							vecAppendInt(outPSIns, psVals[row]);
							addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) createConstInt(psVals[row]));
							fragCnt = NEW_MAP(Constant, Constant);
							addToMap(fragCnt, (Node *) createConstInt(psVals[row]), (Node *) createConstInt(1));
							addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);


						} else {
							vecAppendInt(outPSDel, psVals[row]);
							vecAppendInt(outPSIns, psVals[row]);

							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
							Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVals[row]);
							incrConst(cnt);
						}
					}
					// addToMap(psMap->fragCount, (Node))
				} else {
					outPSIns = makeVector(VECTOR_NODE, T_Vector);
					outPSDel = makeVector(VECTOR_NODE, T_Vector);
					addToMap(resDCIns->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
					addToMap(resDCDel->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));

					int sketchLen = INT_VALUE((Constant *) MAP_GET_STRING(psMap->provLens, STRING_VALUE(c)));

					if (isIntPS) {
						int *psVals = (int *) VEC_TO_IA(inputPS);
						for (int row = 0; row < dcIns->numTuples; row++) {
							int type = updTypeArr[row];
							int psVal = psVals[row];
							HashMap *fragCnt =NULL;
							char *gbVal = gbInsVals[row];
							if (type == 1) {
								BitSet *bitSet = newBitSet(sketchLen);
								setBit(bitSet, psVal, TRUE);
								vecAppendNode(outPSIns, (Node *) copyObject(bitSet));
								addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitSet));
								fragCnt = NEW_MAP(Constant, Constant);
								addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));
								addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);

							} else {
								BitSet *oldBitSet = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
								vecAppendNode(outPSDel, (Node *) copyObject(oldBitSet));

								setBit(oldBitSet, psVal, TRUE);
								vecAppendNode(outPSIns, (Node *) copyObject(oldBitSet));
								fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
								Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
								if (cnt == NULL) {
									addToMap(fragCnt, (Node *) createConstInt(psVal), (Node *) createConstInt(1));
								} else {
									incrConst(cnt);
								}
							}
						}
					} else {
						BitSet **psVals = (BitSet **) VEC_TO_ARR(inputPS, BitSet);
						for (int row = 0; row < dcIns->numTuples; row++) {
							int type = updTypeArr[row];
							char *gbVal = gbInsVals[row];
							HashMap *fragCnt = NULL;
							if (type == 1) {
								BitSet *bitSet = (BitSet *) psVals[row];
								vecAppendNode(outPSIns, (Node *) bitSet);
								addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitSet));

								fragCnt = NEW_MAP(Constant, Constant);
								char *psStr = bitSetToString(bitSet);
								for (int idx = 0; idx < sketchLen; idx++) {
									if (psStr[idx] == '1') {
										addToMap(fragCnt, (Node *) createConstInt(idx), (Node *) createConstInt(1));
									}
								}
								addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
							} else {
								BitSet *bitSet = (BitSet *) MAP_GET_STRING(storedPS, gbVal) ;
								vecAppendNode(outPSDel, (Node *) copyObject(bitSet));
								fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
								BitSet *inBS = (BitSet *) psVals[row];
								char *psStr = (char *) bitSetToString(inBS);
								for (int idx = 0; idx < sketchLen; idx++) {
									if (psStr[idx] == '1') {
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, idx);
										if (cnt == NULL) {
											addToMap(fragCnt, (Node *) createConstInt(idx), (Node *) createConstInt(1));
										} else {
											incrConst(cnt);
										}
										setBit(bitSet, idx, TRUE);
									}
								}

								vecAppendNode(outPSIns, (Node *) copyObject(bitSet));
								addToMap(gbFragCnt, (Node *) createConstString(gbVal), (Node *) fragCnt);
								addToMap(storedPS, (Node *) createConstString(gbVal), (Node *) copyObject(bitSet));
							}
						}
					}
				}
				addToMap(resDCIns->fragmentsInfo, (Node *) copyObject(c), (Node *) outPSIns);
				addToMap(resDCDel->fragmentsInfo, (Node *) copyObject(c), (Node *) outPSDel);
			}

		}

	}

	if (dcDel != NULL) {
		char ** gbDelVals = (char **) VEC_TO_ARR(gbValsDel, char);
		GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructures, PROP_DATA_STRUCTURE_DUP_DATA);
		Vector *updType = makeVector(VECTOR_INT, T_Vector);

		// update acs;
		for (int row = 0; row < dcDel->numTuples; row++) {

			if (MAP_HAS_STRING_KEY(acs->map, gbDelVals[row])) {
				Vector *cntV = (Vector *) MAP_GET_STRING(acs->map, gbDelVals[row]);
				Constant *cnt = (Constant *) getVecNode(cntV, 0);
				if (LONG_VALUE(cnt) <= 1) {
					delLen++;
					vecAppendInt(updType, -1);
					removeMapStringElem(acs->map, gbDelVals[row]);
				} else {
					insLen++;
					delLen++;
					vecAppendInt(updType, 0);
					LONG_VALUE(cnt) = LONG_VALUE(cnt) - 1;
				}
			}
		}

		// get return data;
		int *updTypeArr = (int *) VEC_TO_IA(updType);
		for (int col = 0; col < tups; col++) {
			DataType dt = (DataType) INT_VALUE((Constant *) MAP_GET_INT(resDCDel->posToDatatype, col));
			switch (dt) {
				case DT_INT:
				case DT_BOOL:
				{
					int *fromVecVals = (int *) VEC_TO_IA((Vector *) getVecNode(dcDel->tuples, col));
					for (int row = 0; row < dcDel->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendInt((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
							vecAppendInt((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						} else {
							vecAppendInt((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
						}
					}
				}
				break;
				case DT_LONG:
				{
					gprom_long_t *fromVecVals = (gprom_long_t *) VEC_TO_LA((Vector *) getVecNode(dcDel->tuples, col));
					for (int row = 0; row < dcDel->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendLong((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
							vecAppendLong((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						} else {
							vecAppendLong((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
						}
					}
				}
				break;
				case DT_FLOAT:
				{
					double *fromVecVals = (double *) VEC_TO_FA((Vector *) getVecNode(dcDel->tuples, col));
					for (int row = 0; row < dcDel->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendFloat((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);
							vecAppendFloat((Vector *) getVecNode(resDCIns->tuples, col), fromVecVals[row]);
						} else {
							vecAppendFloat((Vector *) getVecNode(resDCDel->tuples, col), fromVecVals[row]);

						}
					}
				}
				break;
				case DT_VARCHAR2:
				case DT_STRING:
				{
					char ** fromVecVals = (char **) VEC_TO_ARR((Vector *) getVecNode(dcDel->tuples, col), char);
					for (int row = 0; row < dcDel->numTuples; row++) {
						if (updTypeArr[row] == 0) {
							vecAppendString((Vector *) getVecNode(resDCDel->tuples, col), strdup(fromVecVals[row]));
							vecAppendString((Vector *) getVecNode(resDCIns->tuples, col), strdup(fromVecVals[row]));
						} else {
							vecAppendString((Vector *) getVecNode(resDCDel->tuples, col), strdup(fromVecVals[row]));
						}
					}
				}
				break;
				default:
					FATAL_LOG("not supported");
			}
		}

		// update ps and return ps;
		if (dcDel->isAPSChunk == TRUE) {
			resDCIns->isAPSChunk = TRUE;
			resDCDel->isAPSChunk = TRUE;
			PSMap *psMap = (PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_DUP);
			if (psMap == NULL) {
				psMap = makePSMap();
			}

			FOREACH_HASH_KEY(Constant, c, dcDel->fragmentsInfo) {
				boolean isIntPS = BOOL_VALUE((Constant *) MAP_GET_STRING(dcDel->fragmentsIsInt, STRING_VALUE(c)));

				Vector *inputPS = (Vector *) MAP_GET_STRING(dcDel->fragmentsInfo, STRING_VALUE(c));

				boolean isStoreInt = BOOL_VALUE((Constant *) MAP_GET_STRING(psMap->isIntSketch, STRING_VALUE(c)));

				HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
				if (gbFragCnt == NULL) {
					gbFragCnt = NEW_MAP(Constant, Node);
					addToMap(psMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
				}

				HashMap *storedPS = (HashMap *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
				if (storedPS == NULL) {
					storedPS = NEW_MAP(Constant, Node);
					addToMap(psMap->provLens, (Node *) copyObject(c), (Node *) storedPS);
				}

				Vector *outPSIns = (Vector *) MAP_GET_STRING(resDCIns->fragmentsInfo, STRING_VALUE(c));
				Vector *outPSDel = (Vector *) MAP_GET_STRING(resDCDel->fragmentsInfo, STRING_VALUE(c));
				if (isStoreInt) {
					addToMap(resDCIns->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
					addToMap(resDCDel->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
					if (outPSIns == NULL) {
						outPSIns = makeVector(VECTOR_INT, T_Vector);
					}
					if (outPSDel == NULL) {
						outPSDel = makeVector(VECTOR_INT, T_Vector);
					}

					for (int row = 0; row < dcDel->numTuples; row++) {
						int type = updTypeArr[row];
						char *gbVal = gbDelVals[row];
						int psVal = INT_VALUE((Constant *) MAP_GET_STRING(storedPS, gbVal));
						vecAppendInt(outPSDel, psVal);

						if (type == -1) {
							removeMapStringElem(gbFragCnt, gbVal);
							removeMapStringElem(storedPS, gbVal);
						} else {
							vecAppendInt(outPSIns, psVal);
							HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
							Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
							INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
						}
					}
				} else {
					addToMap(resDCIns->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
					addToMap(resDCDel->fragmentsIsInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
					if (outPSIns == NULL) {
						outPSIns = makeVector(VECTOR_NODE, T_Vector);
					}
					if (outPSDel == NULL) {
						outPSDel = makeVector(VECTOR_NODE, T_Vector);
					}

					if (isIntPS) {
						int *psVals = (int *) VEC_TO_IA(inputPS);
						for (int row = 0; row < dcDel->numTuples; row++) {
							int type = updTypeArr[row];
							char *gbVal = gbDelVals[row];
							int psVal = psVals[row];
							BitSet *oldBitSet = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
							vecAppendNode(outPSDel, (Node *) copyObject(oldBitSet));

							if (type == -1) {
								removeMapStringElem(storedPS, gbVal);
								removeMapStringElem(gbFragCnt, gbVal);
							} else {
								HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
								Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, psVal);
								if (INT_VALUE(cnt) < 2) {
									removeMapElem(fragCnt, (Node *) createConstInt(psVal));
									setBit(oldBitSet, psVal, FALSE);
								} else {
									INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
								}
								vecAppendNode(outPSIns, (Node *) copyObject(oldBitSet));
							}
						}
					} else {
						BitSet ** psVals = (BitSet **) VEC_TO_ARR(inputPS, char);
						int sketchLen = INT_VALUE((Constant *) MAP_GET_STRING(psMap->provLens, STRING_VALUE(c)));
						for (int row = 0; row < dcDel->numTuples; row++) {
							int type = updTypeArr[row];
							char *gbVal = gbDelVals[row];
							BitSet *oldBitSet = (BitSet *) MAP_GET_STRING(storedPS, gbVal);
							vecAppendNode(outPSDel, (Node *) copyObject(oldBitSet));
							if (type == -1) {
								removeMapStringElem(storedPS, gbVal);
								removeMapStringElem(gbFragCnt, gbVal);
							} else {
								HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVal);
								char *psStr = (char *) bitSetToString((BitSet *) psVals[row]);
								for (int idx = 0; idx < sketchLen; idx++) {
									if (psStr[idx] == '1') {
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, idx);
										if (INT_VALUE(cnt) < 2) {
											removeMapElem(fragCnt, (Node *) createConstInt(idx));
											setBit(oldBitSet, idx, FALSE);
										} else {
											INT_VALUE(cnt) = INT_VALUE(cnt) -1;
										}
									}
								}
								vecAppendNode(outPSIns, (Node *) copyObject(oldBitSet));
							}
						}
					}
				}

				addToMap(resDCIns->fragmentsInfo, (Node *) copyObject(c), (Node *) outPSIns);
				addToMap(resDCDel->fragmentsInfo, (Node *) copyObject(c), (Node *) outPSDel);
			}
		}
	}

	resDCDel->numTuples = delLen;
	resDCIns->numTuples = insLen;

	HashMap *resChunkMaps = NEW_MAP(Constant, Node);
	if (insLen > 0) {
		addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) resDCIns);
	}

	if (delLen > 0) {
		addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) resDCDel);

	}

	if (mapSize(resChunkMaps) > 0) {
		SET_STRING_PROP(op, PROP_DATA_CHUNK, resChunkMaps);
	}
	// remove child's datachunk;
	DEBUG_NODE_BEATIFY_LOG("dup return chunks", resChunkMaps);
	removeStringProperty(lchild, PROP_DATA_CHUNK);
	appendStringInfo(strInfo, "%s ", "UpdateDuplicatiRemoval");
}


static void
updateSet(QueryOperator* op)
{
	/*
		In GProM:
		SETOP_UNION: UNION ALL; so union in GProM is DUPLICATEREMOVE <-- SETOP_UNION;
        SETOP_INTERSECTION: INTERCEPT, so intersect all is
        SETOP_DIFFERENCE:
	*/
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));

	if (!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)
	&& !HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		return;
	}

	HashMap *resChunkMaps = NEW_MAP(Constant, Node);
	if (!HAS_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK)) {
		resChunkMaps = (HashMap *) GET_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK);
	} else if (!HAS_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK)) {
		resChunkMaps = (HashMap *) GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK);
	} else {
		HashMap *leftMaps = (HashMap *) GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_CHUNK);
		DataChunk *leftDCIns = (DataChunk *) MAP_GET_STRING(leftMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *leftDCDel = (DataChunk *) MAP_GET_STRING(leftMaps, PROP_DATA_CHUNK_DELETE);

		HashMap *rightMaps = (HashMap *) GET_STRING_PROP(OP_RCHILD(op), PROP_DATA_CHUNK);
		DataChunk *rightDCIns = (DataChunk *) MAP_GET_STRING(rightMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *rightDCDel = (DataChunk *) MAP_GET_STRING(rightMaps, PROP_DATA_CHUNK_DELETE);
		DataChunk *resDCIns = NULL;
		if (leftDCIns != NULL && rightDCIns != NULL) {
			// resDCIns = mergeDataChunks(leftDCIns, rightDCIns);
		} else if (leftDCIns != NULL) {
			resDCIns = leftDCIns;
		} else if (rightDCIns != NULL) {
			resDCIns = rightDCIns;
		}

		DataChunk *resDCDel = NULL;
		if (leftDCDel != NULL && rightDCDel != NULL) {
			// resDCDel = mergeDataChunks(leftDCDel, rightDCDel);
		} else if (leftDCDel != NULL) {
			resDCDel = leftDCDel;
		} else if (rightDCDel != NULL) {
			resDCDel = rightDCDel;
		}

		if (resDCIns != NULL) {
			// addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) resDCIns);
			MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_INSERT, resDCIns);
		}
		if (resDCDel != NULL) {
			// addToMap(resChunkMaps, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) resDCDel);
			MAP_ADD_STRING_KEY(resChunkMaps, PROP_DATA_CHUNK_DELETE, resDCDel);
		}
	}

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
	HashMap *resChunkMaps = (HashMap *) GET_STRING_PROP(lchild, PROP_DATA_CHUNK);
	SET_STRING_PROP(op, PROP_DATA_CHUNK, resChunkMaps);

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

	if (!isA(OP_LCHILD(op), OrderOperator)) {
		// special case;
		return;
	}

	// get the rbtree from child(order operator);
	HashMap *dataStructures = (HashMap *) GET_STRING_PROP(OP_LCHILD(op), PROP_DATA_STRUCTURE_STATE);
	RBTRoot *rbtree = NULL;
	rbtree = (RBTRoot *) MAP_GET_STRING(dataStructures, PROP_DATA_STRUCTURE_ORDER_BY);

	int limitNum = INT_VALUE((Constant *) ((LimitOperator *) op)->limitExpr);

	// get old top k;
	Vector *deleteChunkTuples = RBTGetTopK(rbtree, limitNum);
	DEBUG_NODE_BEATIFY_LOG("DELETE TUPLES", deleteChunkTuples);
	HashMap *inputDataChunks = (HashMap *) GET_STRING_PROP(lchild, PROP_DATA_CHUNK);
	DataChunk *inputDCIns = (DataChunk *) MAP_GET_STRING(inputDataChunks, PROP_DATA_CHUNK_INSERT);
	DataChunk *inputDCDel = (DataChunk *) MAP_GET_STRING(inputDataChunks, PROP_DATA_CHUNK_DELETE);

	Vector *vecInsToTree = NULL;
	Vector *vecDelToTree = NULL;
	boolean isAPSChunk = FALSE;
	int attrLens = 0;
	if (inputDCIns != NULL) {
		vecInsToTree = makeVector(VECTOR_NODE, T_Vector);
		isAPSChunk = inputDCIns->isAPSChunk;
		for (int i = 0; i < inputDCIns->numTuples; i++) {
			HashMap *tuple = NEW_MAP(Constant, Node);
			// 0: key, 1: val, 2: ps
			addToMap(tuple, (Node *) createConstInt(0), (Node *) makeVector(VECTOR_NODE, T_Vector));
			addToMap(tuple, (Node *) createConstInt(1), (Node *) makeVector(VECTOR_NODE, T_Vector));
			addToMap(tuple, (Node *) createConstInt(2), (Node *) NEW_MAP(Constant, Node));
			vecAppendNode(vecInsToTree, (Node *) tuple);
		}
		attrLens = inputDCIns->tupleFields;
	}

	if (inputDCDel != NULL) {
		vecDelToTree = makeVector(VECTOR_NODE, T_Vector);
		isAPSChunk = inputDCDel->isAPSChunk;
		for (int i = 0; i < inputDCDel->numTuples; i++) {
			HashMap *tuple = NEW_MAP(Constant, Node);
			// 0: key, 1: val, 2: ps
			addToMap(tuple, (Node *) createConstInt(0), (Node *) makeVector(VECTOR_NODE, T_Vector));
			addToMap(tuple, (Node *) createConstInt(1), (Node *) makeVector(VECTOR_NODE, T_Vector));
			addToMap(tuple, (Node *) createConstInt(2), (Node *) NEW_MAP(Constant, Node));
			vecAppendNode(vecDelToTree, (Node *) tuple);
		}
		attrLens = inputDCDel->tupleFields;
	}
	Set *orderByAttrs = (Set *) MAP_GET_STRING(rbtree->metadata, ORDER_BY_ATTRS);

	for (int col = 0; col < attrLens; col++) {
		AttributeDef *ad = NULL;
		if (inputDCIns != NULL) {
			ad = (AttributeDef *) getNthOfListP(inputDCIns->attrNames, col);
		} else {
			ad = (AttributeDef *) getNthOfListP(inputDCDel->attrNames, col);
		}
		DataType dt = ad->dataType;
		// if it is a order by att, add to both key and value;
		if (hasSetElem(orderByAttrs, (void *) ad->attrName)) {
			switch(dt) {
				case DT_BOOL:
				{
					if (inputDCIns != NULL) {
						int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCIns->tuples, col));
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							Constant *c = createConstInt(vals[row] != 0);
							HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

					if (inputDCDel != NULL) {
						int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCDel->tuples, col));
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							Constant *c = createConstInt(vals[row] != 0);
							HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}
				}
				break;
				case DT_INT:
				{
					if (inputDCIns != NULL) {
						int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCIns->tuples, col));
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							Constant *c = createConstInt(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

					if (inputDCDel != NULL) {
						int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCDel->tuples, col));
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							Constant *c = createConstInt(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

				}
				break;
				case DT_LONG:
				{
					if (inputDCIns != NULL) {
						gprom_long_t *vals = VEC_TO_LA((Vector *) getVecNode(inputDCIns->tuples, col));
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							Constant *c = createConstLong(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

					if (inputDCDel != NULL) {
						gprom_long_t *vals = VEC_TO_LA((Vector *) getVecNode(inputDCDel->tuples, col));
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							Constant *c = createConstLong(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}
				}
				break;
				case DT_FLOAT:
				{
					if (inputDCIns != NULL) {
						double *vals = VEC_TO_FA((Vector *) getVecNode(inputDCIns->tuples, col));
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							Constant *c = createConstFloat(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

					if (inputDCDel != NULL) {
						double *vals = VEC_TO_FA((Vector *) getVecNode(inputDCDel->tuples, col));
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							Constant *c = createConstFloat(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}
				}
				break;
				case DT_STRING:
				case DT_VARCHAR2:
				{
					if (inputDCIns != NULL) {
						char** vals = (char **) VEC_TO_ARR((Vector *) getVecNode(inputDCIns->tuples, col), char);
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							Constant *c = createConstString(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

					if (inputDCDel != NULL) {
						char** vals = (char **) VEC_TO_ARR((Vector *) getVecNode(inputDCDel->tuples, col), char);
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							Constant *c = createConstString(vals[row]);
							HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
							vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
						}
					}

				}
				break;
			}
			continue;
		}
		// this attr is no an order by attribute;
		switch(dt){
			case DT_BOOL:
			{
				if (inputDCIns != NULL) {
					int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCIns->tuples, col));
					for (int row = 0; row < inputDCIns->numTuples; row++) {
						Constant *c = createConstBool(vals[row] != 0);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree,row), 1), (Node *) c);
					}
				}

				if (inputDCDel != NULL) {
					int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCDel->tuples, col));
					for (int row = 0; row < inputDCDel->numTuples; row++) {
						Constant *c = createConstBool(vals[row] != 0);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree,row), 1), (Node *) c);
					}
				}
			}
			break;
			case DT_INT:
			{
				if (inputDCIns != NULL) {
					int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCIns->tuples, col));
					for (int row = 0; row < inputDCIns->numTuples; row++) {
						Constant *c = createConstInt(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 1), (Node *) c);
					}
				}

				if (inputDCDel != NULL) {
					int *vals = VEC_TO_IA((Vector *) getVecNode(inputDCDel->tuples, col));
					for (int row = 0; row < inputDCDel->numTuples; row++) {
						Constant *c = createConstInt(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 1), (Node *) c);
					}
				}

			}
			break;
			case DT_LONG:
			{
				if (inputDCIns != NULL) {
					gprom_long_t *vals = VEC_TO_LA((Vector *) getVecNode(inputDCIns->tuples, col));
					for (int row = 0; row < inputDCIns->numTuples; row++) {
						Constant *c = createConstLong(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 1), (Node *) c);
					}
				}

				if (inputDCDel != NULL) {
					gprom_long_t *vals = VEC_TO_LA((Vector *) getVecNode(inputDCDel->tuples, col));
					for (int row = 0; row < inputDCDel->numTuples; row++) {
						Constant *c = createConstLong(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 1), (Node *) c);
					}
				}
			}
			break;
			case DT_FLOAT:
			{
				if (inputDCIns != NULL) {
					double *vals = VEC_TO_FA((Vector *) getVecNode(inputDCIns->tuples, col));
					for (int row = 0; row < inputDCIns->numTuples; row++) {
						Constant *c = createConstFloat(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 1), (Node *) c);
					}
				}

				if (inputDCDel != NULL) {
					double *vals = VEC_TO_FA((Vector *) getVecNode(inputDCDel->tuples, col));
					for (int row = 0; row < inputDCDel->numTuples; row++) {
						Constant *c = createConstFloat(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 1), (Node *) c);
					}
				}
			}
			break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				if (inputDCIns != NULL) {
					char** vals = (char **) VEC_TO_ARR((Vector *) getVecNode(inputDCIns->tuples, col), char);
					for (int row = 0; row < inputDCIns->numTuples; row++) {
						Constant *c = createConstString(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 1), (Node *) c);
					}
				}

				if (inputDCDel != NULL) {
					char** vals = (char **) VEC_TO_ARR((Vector *) getVecNode(inputDCDel->tuples, col), char);
					for (int row = 0; row < inputDCDel->numTuples; row++) {
						Constant *c = createConstString(vals[row]);
						vecAppendNode((Vector *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 1), (Node *) c);
					}
				}

			}
			break;
		}
	}

	// HashMap *inputPSIsInt = NULL;
	HashMap *orderByIsPSInt = (HashMap *) MAP_GET_STRING(rbtree->metadata, ORDER_BY_IS_PS_INT);
	HashMap *orderByPSLens = (HashMap *) MAP_GET_STRING(rbtree->metadata, ORDER_BY_PS_LENS);
	// dealing with ps;
	if (isAPSChunk) {
		if (inputDCIns != NULL) {
			// inputPSIsInt = (HashMap *) copyObject(inputDCIns->fragmentsIsInt);
			FOREACH_HASH_KEY(Constant, c, inputDCIns->fragmentsInfo) {
				boolean isOutputInt = BOOL_VALUE((Constant *) MAP_GET_STRING(orderByIsPSInt, STRING_VALUE(c)));
				int psLens = INT_VALUE((Constant *) MAP_GET_STRING(orderByPSLens, STRING_VALUE(c)));
				Vector *ps = (Vector *) MAP_GET_STRING(inputDCIns->fragmentsInfo, STRING_VALUE(c));
				boolean isPSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(inputDCIns->fragmentsIsInt, STRING_VALUE(c)));
				if (isPSInt) {
					int* psVals = VEC_TO_IA(ps);
					if (isOutputInt) {
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							addToMap((HashMap *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 2), (Node *) c, (Node *) createConstInt(psVals[row]));
						}
					} else {
						for (int row = 0; row < inputDCIns->numTuples; row++) {
							BitSet *bitSet = newBitSet(psLens);
							setBit(bitSet, psVals[row], TRUE);
							addToMap((HashMap *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 2), (Node *) c, (Node *) bitSet);
						}
					}
				} else {
					BitSet **psVals = VEC_TO_ARR(ps, BitSet);
					for (int row = 0; row < inputDCIns->numTuples; row++) {
						addToMap((HashMap *) MAP_GET_INT((HashMap *) getVecNode(vecInsToTree, row), 2), (Node *) c, (Node *) psVals[row]);
					}
				}
			}
		}

		if (inputDCDel != NULL) {
			// if (inputPSIsInt == NULL) {
				// inputPSIsInt = (HashMap *) copyObject(inputDCDel->fragmentsIsInt);
			// }
			FOREACH_HASH_KEY(Constant, c, inputDCDel->fragmentsInfo) {
				boolean isOutputInt = BOOL_VALUE((Constant *) MAP_GET_STRING(orderByIsPSInt, STRING_VALUE(c)));
				int psLens = INT_VALUE((Constant *) MAP_GET_STRING(orderByPSLens, STRING_VALUE(c)));
				Vector *ps = (Vector *) MAP_GET_STRING(inputDCDel->fragmentsInfo, STRING_VALUE(c));
				boolean isPSInt = BOOL_VALUE((Constant *) MAP_GET_STRING(inputDCDel->fragmentsIsInt, STRING_VALUE(c)));
				if (isPSInt) {
					int* psVals = VEC_TO_IA(ps);
					if (isOutputInt) {
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							addToMap((HashMap *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 2), (Node *) c, (Node *) createConstInt(psVals[row]));
						}
					} else {
						for (int row = 0; row < inputDCDel->numTuples; row++) {
							BitSet *bitset = newBitSet(psLens);
							setBit(bitset, psVals[row], TRUE);
							addToMap((HashMap *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 2), (Node *) c, (Node *) bitset);
						}
					}
				} else {
					BitSet **psVals = VEC_TO_ARR(ps, BitSet);
					for (int row = 0; row < inputDCDel->numTuples; row++) {
						addToMap((HashMap *) MAP_GET_INT((HashMap *) getVecNode(vecDelToTree, row), 2), (Node *) c, (Node *) psVals[row]);
					}
				}
			}
		}
	}

	// insert inserted values to rbtree
	if (inputDCIns != NULL) {
		if (isAPSChunk) {
			for (int row = 0; row < vecInsToTree->length; row++) {
				HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
				Vector *key = (Vector *) MAP_GET_INT(tuple, 0);
				Vector *val = (Vector *) MAP_GET_INT(tuple, 1);
				HashMap *ps = (HashMap *) MAP_GET_INT(tuple, 2);
				vecAppendNode(val, (Node *) ps);
				RBTInsert(rbtree, (Node *) key, (Node *) val);
			}
		} else {
			for (int row = 0; row < vecInsToTree->length; row++) {
				HashMap *tuple = (HashMap *) getVecNode(vecInsToTree, row);
				Vector *key = (Vector *) MAP_GET_INT(tuple, 0);
				Vector *val = (Vector *) MAP_GET_INT(tuple, 1);
				RBTInsert(rbtree, (Node *) key, (Node *) val);
			}
		}
	}

	// insert deleted values to rbtree;
	if (inputDCDel != NULL) {
		if (isAPSChunk) {
			for (int row = 0; row < vecDelToTree->length; row++) {
				HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
				Vector *key = (Vector *) MAP_GET_INT(tuple, 0);
				Vector *val = (Vector *) MAP_GET_INT(tuple, 1);
				HashMap *ps = (HashMap *) MAP_GET_INT(tuple, 2);
				vecAppendNode(val, (Node *) ps);
				RBTDelete(rbtree, (Node *) key, (Node *) val);
			}
		} else {
			for (int row = 0; row < vecDelToTree->length; row++) {
				HashMap *tuple = (HashMap *) getVecNode(vecDelToTree, row);
				Vector *key = (Vector *) MAP_GET_INT(tuple, 0);
				Vector *val = (Vector *) MAP_GET_INT(tuple, 1);
				RBTDelete(rbtree, (Node *) key, (Node *) val);
			}
		}
	}

	// get new top k;
	Vector *insertChunkTuples = RBTGetTopK(rbtree, limitNum);
	DEBUG_NODE_BEATIFY_LOG("LIMIT INSERT TUPLES:", insertChunkTuples);

	DataChunk *resDCIns = initDataChunk();
	DataChunk *resDCDel = initDataChunk();
	resDCIns->attrNames = (List *) copyObject(op->schema->attrDefs);
	resDCIns->tupleFields = LIST_LENGTH(op->schema->attrDefs);
	resDCIns->isAPSChunk = isAPSChunk;
	resDCIns->fragmentsIsInt = (HashMap *) MAP_GET_STRING(rbtree->metadata, ORDER_BY_IS_PS_INT);


	resDCDel->attrNames = (List *) copyObject(op->schema->attrDefs);
	resDCDel->tupleFields = LIST_LENGTH(op->schema->attrDefs);
	resDCDel->isAPSChunk = isAPSChunk;
	resDCDel->fragmentsIsInt = (HashMap *) MAP_GET_STRING(rbtree->metadata, ORDER_BY_IS_PS_INT);
	int attrPos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		addToMap(resDCIns->attriToPos, (Node *) createConstString(strdup(ad->attrName)), (Node *) createConstInt(attrPos));
		addToMap(resDCIns->posToDatatype, (Node *) createConstInt(attrPos), (Node *) createConstInt(ad->dataType));

		addToMap(resDCDel->attriToPos, (Node *) createConstString(strdup(ad->attrName)), (Node *) createConstInt(attrPos));
		addToMap(resDCDel->posToDatatype, (Node *) createConstInt(attrPos), (Node *) createConstInt(ad->dataType));

		switch (ad->dataType) {
			case DT_INT:
			case DT_BOOL:
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_INT, T_Vector));
				break;
			case DT_LONG:
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_LONG, T_Vector));
				break;
			case DT_FLOAT:
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_FLOAT, T_Vector));
				break;
			case DT_STRING:
			case DT_VARCHAR2:
				vecAppendNode(resDCIns->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				vecAppendNode(resDCDel->tuples, (Node *) makeVector(VECTOR_STRING, T_Vector));
				break;
			default:
				FATAL_LOG("data type is not supported");
		}
		attrPos++;
	}

	// fill result insert data chunk
	if (insertChunkTuples != NULL && insertChunkTuples->length > 0) {
		resDCIns->numTuples = insertChunkTuples->length;
		resDCIns->isAPSChunk = isAPSChunk;
		int attrIdx = 0;
		int totalTups = insertChunkTuples->length;

		//deal with tuple values;
		FOREACH(AttributeDef, ad, op->schema->attrDefs) {
			switch(ad->dataType) {
				case DT_INT:
				{
					Vector *v = (Vector *) getVecNode(resDCIns->tuples, attrIdx);
					for (int row = 0; row < totalTups; row++) {
						vecAppendInt(v, INT_VALUE((Constant *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_BOOL:
				{
					Vector *v = (Vector *) getVecNode(resDCIns->tuples, attrIdx);
					for (int row = 0; row < totalTups; row++) {
						vecAppendInt(v, BOOL_VALUE((Constant *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_LONG:
				{
					Vector *v = (Vector *) getVecNode(resDCIns->tuples, attrIdx);
					for (int row = 0; row < totalTups; row++) {
						vecAppendLong(v, LONG_VALUE((Constant *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_FLOAT:
				{
					Vector *v = (Vector *) getVecNode(resDCIns->tuples, attrIdx);
					for (int row = 0; row < totalTups; row++) {
						vecAppendFloat(v, FLOAT_VALUE((Constant *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_STRING:
				case DT_VARCHAR2:
				{
					Vector *v = (Vector *) getVecNode(resDCIns->tuples, attrIdx);
					for (int row = 0; row < totalTups; row++) {
						vecAppendString(v, STRING_VALUE((Constant *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), attrIdx)));
					}
				}
				break;
			}
			attrIdx++;
		}

		// deal with ps;
		if (isAPSChunk) {
			int psMapPos = ((Vector *) getVecNode(insertChunkTuples, 0))->length - 1;
			HashMap *map = (HashMap *) getVecNode((Vector *) getVecNode(insertChunkTuples, 0), psMapPos);

			FOREACH_HASH_KEY(Constant, c, map) {
				char *psName = STRING_VALUE(c);
				boolean isPSInt = BOOL_VALUE(MAP_GET_STRING(resDCIns->fragmentsIsInt, STRING_VALUE(c)));
				Vector *ps = NULL;
				if (isPSInt) {
					ps = makeVector(VECTOR_INT, T_Vector);
					for (int row = 0; row < totalTups; row++) {
						vecAppendInt(ps, INT_VALUE((Constant*) MAP_GET_STRING((HashMap *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), psMapPos), psName)));
					}

				} else {
					ps = makeVector(VECTOR_NODE, T_Vector);
					for (int row = 0; row < totalTups; row++) {

						vecAppendNode(ps, (Node *) MAP_GET_STRING((HashMap *) getVecNode((Vector *) getVecNode(insertChunkTuples, row), psMapPos), psName));
					}
				}
				addToMap(resDCIns->fragmentsInfo, (Node *) c, (Node *) ps);
			}
		}

		resDCIns->numTuples = totalTups;

		// deal with updateIdentifier;
		Vector *updIde = resDCIns->updateIdentifier;
		for (int row = 0; row < totalTups; row++) {
			vecAppendInt(updIde, 1);
		}

	}


	// fill result deleted data chunk;
	if (deleteChunkTuples != NULL && deleteChunkTuples->length > 0) {
		resDCDel->numTuples = deleteChunkTuples->length;
		resDCDel->isAPSChunk = isAPSChunk;
		int attrIdx = 0;
		int totalTup = deleteChunkTuples->length;

		FOREACH(AttributeDef, ad, op->schema->attrDefs) {
			switch(ad->dataType) {
				case DT_INT:
				{
					Vector *v = (Vector *) getVecNode(resDCDel->tuples, attrIdx);
					for (int row = 0; row < totalTup; row++) {
						vecAppendInt(v, INT_VALUE((Constant *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_BOOL:
				{
					Vector *v = (Vector *) getVecNode(resDCDel->tuples, attrIdx);
					for (int row = 0; row < totalTup; row++) {
						vecAppendInt(v, BOOL_VALUE((Constant *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_LONG:
				{
					Vector *v = (Vector *) getVecNode(resDCDel->tuples, attrIdx);
					for (int row = 0; row < totalTup; row++) {
						vecAppendLong(v, LONG_VALUE((Constant *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_FLOAT:
				{
					Vector *v = (Vector *) getVecNode(resDCDel->tuples, attrIdx);
					for (int row = 0; row < totalTup; row++) {
						vecAppendFloat(v, FLOAT_VALUE((Constant *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), attrIdx)));
					}
				}
				break;
				case DT_STRING:
				case DT_VARCHAR2:
				{
					Vector *v = (Vector *) getVecNode(resDCDel->tuples, attrIdx);
					for (int row = 0; row < totalTup; row++) {
						vecAppendString(v, STRING_VALUE((Constant *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), attrIdx)));
					}
				}
				break;
			}
			attrIdx++;
		}

		// ps
		if (isAPSChunk) {
			int psMapPos = ((Vector *) getVecNode(deleteChunkTuples, 0))->length - 1;
			HashMap *map = (HashMap *) getVecNode((Vector *) getVecNode(deleteChunkTuples, 0), psMapPos);
			FOREACH_HASH_KEY(Constant, c, map) {
				char *psName = STRING_VALUE(c);
				boolean isPSInt = BOOL_VALUE(MAP_GET_STRING(resDCDel->fragmentsIsInt, STRING_VALUE(c)));
				Vector *ps = NULL;
				if (isPSInt) {
					ps = makeVector(VECTOR_INT, T_Vector);
					for (int row = 0; row < totalTup; row++) {
						vecAppendInt(ps, INT_VALUE((Constant *) MAP_GET_STRING((HashMap *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), psMapPos), psName)));
					}
				} else {
					ps = makeVector(VECTOR_NODE, T_Vector);
					for (int row = 0; row < totalTup; row++) {
						vecAppendNode(ps, (Node *) MAP_GET_STRING((HashMap *) getVecNode((Vector *) getVecNode(deleteChunkTuples, row), psMapPos), psName));
					}
				}

				addToMap(resDCDel->fragmentsInfo, (Node *) c, (Node *) ps);
			}
		}

		resDCDel->numTuples = totalTup;
		Vector *updIde = resDCDel->updateIdentifier;
		for (int row = 0; row < totalTup; row++) {
			vecAppendInt(updIde, -1);
		}
	}

	HashMap *resDataStructures = NEW_MAP(Constant, Node);
	if (resDCIns->numTuples > 0) {
		addToMap(resDataStructures, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) resDCIns);
	}

	if (resDCDel->numTuples > 0) {
		addToMap(resDataStructures, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) resDCDel);
	}

	setStringProperty(op, PROP_DATA_CHUNK, (Node *) resDataStructures);
	DEBUG_NODE_BEATIFY_LOG("LIMIT OUTPUT CHUNKS", resDataStructures);
}

// static int
// limitCmp(const void **a, const void **b) {
// 	List *la = (List *) *a;
// 	List *lb = (List *) *b;

// 	int res = 0;
// 	FOREACH(OrderExpr, oe, limitOrderBys) {
// 		AttributeReference *ar = (AttributeReference *) oe->expr;
// 		int pos = INT_VALUE((Constant *) MAP_GET_STRING(limitAttrPoss, ar->name));

// 		int cmp = compareTwoValues((Constant *) getNthOfListP(la, pos), (Constant *) getNthOfListP(lb, pos), ar->attrType);
// 		if (cmp) {
// 			if (oe->order == SORT_DESC) {
// 				res -= cmp;
// 			} else {
// 				res = cmp;
// 			}
// 			break;
// 		}
// 	}

// 	return res;
// }

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

	boolean isUpdatedDirectFromDelta = getBoolOption(OPTION_UPDATE_PS_DIRECT_DELTA);
	char *updatedTableName = NULL;
	if (isUpdatedDirectFromDelta) {
		updatedTableName = getStringOption(OPTION_UPDATE_PS_UPDATED_TABLE);
	} else {
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
	}
	if (strcmp(updatedTableName, tableName) != 0) {
		return;
	}
	// build a chumk map (insert chunk and delete chunk) based on update type;
	START_TIMER(INCREMENTAL_FETCHING_DATA_TIMER);
	HashMap *chunkMap = NULL;
	if (isUpdatedDirectFromDelta) {
		chunkMap = getDataChunkFromDeltaTable((TableAccessOperator *) op);
	} else {
		chunkMap = getDataChunkFromUpdateStatement(updateStatement, (TableAccessOperator *) op);
	}
	if (mapSize(chunkMap) > 0) {
		setStringProperty(op, PROP_DATA_CHUNK, (Node *) chunkMap);
	}
	STOP_TIMER(INCREMENTAL_FETCHING_DATA_TIMER);

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FRO TABLEACCESS OPERATOR", chunkMap);
}

static HashMap *
getDataChunkFromDeltaTable(TableAccessOperator * tableAccessOp)
{

	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));
	List *provAttrDefs = getProvenanceAttrDefs(rewr);
	char *psName = NULL;
	if (provAttrDefs != NULL) {
		psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;
	}

	List *psAttrInfoList = (List *) MAP_GET_STRING(PS_INFO->tablePSAttrInfos, tableAccessOp->tableName);
	psAttrInfo *attrInfo = (psAttrInfo *) getHeadOfListP(psAttrInfoList);

	// init datachunk and fields;
	DataChunk *dcIns = initDataChunk();
	DataChunk *dcDel = initDataChunk();
	Schema *schema = ((QueryOperator *) tableAccessOp)->schema;
	dcIns->attrNames = (List *) copyObject(schema->attrDefs);
	dcDel->attrNames = (List *) copyObject(schema->attrDefs);
	dcIns->tupleFields = LIST_LENGTH(((QueryOperator *) tableAccessOp)->schema->attrDefs);
	dcDel->tupleFields = LIST_LENGTH(((QueryOperator *) tableAccessOp)->schema->attrDefs);
	DEBUG_NODE_BEATIFY_LOG("SCHEMA", schema);
	INFO_LOG("SIZE OF ATTRIS: %d", LIST_LENGTH(schema->attrDefs));
	INFO_LOG("SIZE OF ATTRIS: %d", LIST_LENGTH(((QueryOperator *) tableAccessOp)->schema->attrDefs));

	INFO_LOG("what is size %d, %d", dcIns->tupleFields, dcDel->tupleFields);

	// ps attr col pos;
	int psAttrCol = -1;

	int attrIdx = 0;
	FOREACH(AttributeDef, ad, schema->attrDefs) {
		addToMap(dcIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrIdx));
		addToMap(dcDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrIdx));

		addToMap(dcIns->posToDatatype, (Node *) createConstInt(attrIdx), (Node *) createConstInt(ad->dataType));
		addToMap(dcDel->posToDatatype, (Node *) createConstInt(attrIdx), (Node *) createConstInt(ad->dataType));

		if (psName != NULL && psAttrCol == -1 && strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = attrIdx;
		}

		attrIdx++;
	}

	Vector *ranges = NULL;
	if (psAttrCol != -1) {
		ranges = makeVector(VECTOR_INT, T_Vector);
		FOREACH(Constant, c, attrInfo->rangeList) {
			vecAppendInt(ranges, INT_VALUE(c));
		}
	}

	// create TableAccess, Selection, Projection to get delta tuples;

	// get delta Table name;
	char *deltaTableName = getStringOption(OPTION_UPDATE_PS_DELTA_TABLE);

	// get names, and types and append the identifier name and type
	List *attrNames = getAttrNames(schema);
	attrNames = appendToTailOfList(attrNames, strdup(getStringOption(OPTION_UPDATE_PS_DELTA_TABLE_UPDIDENT)));
	List *dataTypes = getDataTypes(schema);
	dataTypes = appendToTailOfListInt(dataTypes, DT_INT);

	// create taop;
	TableAccessOperator *taOp = NULL;
	taOp = createTableAccessOp(strdup(deltaTableName), NULL, strdup(deltaTableName), NIL, attrNames, dataTypes);

	// check selection push down and create selection operator;
	SelectionOperator *selOp = NULL;

	boolean isSelectionPD = getBoolOption(OPTION_UPDATE_PS_SELECTION_PUSH_DOWN);
	if (isSelectionPD) {
		// check parent op is a selection;
		QueryOperator *parent = OP_FIRST_PARENT(tableAccessOp);
		if (isA(parent, SelectionOperator)) {
			Node *conds = (Node *) copyObject(((SelectionOperator *) parent)->cond);
			selOp = createSelectionOp(conds, (QueryOperator *) taOp, NIL, attrNames);
			taOp->op.parents = singleton(selOp);
		}
	}

	// create projection operator;
	ProjectionOperator *projOp = NULL;
	List *projExpr = NIL;
	attrIdx = 0;
	FOREACH(AttributeDef, ad, ((QueryOperator *) taOp)->schema->attrDefs) {
		AttributeReference *ar = createFullAttrReference(strdup(ad->attrName), 0, attrIdx++, 0, ad->dataType);
		projExpr = appendToTailOfList(projExpr, ar);
	}
	DEBUG_NODE_BEATIFY_LOG("proj list", projExpr);
	if (selOp == NULL) {
		projOp = createProjectionOp(projExpr, (QueryOperator *) taOp, NIL, attrNames);
		taOp->op.parents = singleton(projOp);
	} else {
		projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, attrNames);
		selOp->op.parents = singleton(projOp);
	}

	DEBUG_NODE_BEATIFY_LOG("delta query: ", projOp);
	char *query = serializeQuery((QueryOperator *) projOp);
	postgresGetDataChunkFromDeltaTable(query, dcIns, dcDel, psAttrCol, ranges, psName);

	HashMap *chunkMap = NEW_MAP(Constant, Node);
	if (dcIns && dcIns->numTuples > 0) {
		MAP_ADD_STRING_KEY(chunkMap, PROP_DATA_CHUNK_INSERT, dcIns);
	}

	if (dcDel && dcDel->numTuples > 0) {
		MAP_ADD_STRING_KEY(chunkMap, PROP_DATA_CHUNK_DELETE, dcDel);
	}
	DEBUG_NODE_BEATIFY_LOG("DC FOR TABLE", chunkMap);
	return chunkMap;
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

	if (dataChunkInsert && dataChunkInsert->numTuples > 0) {
		// addToMap(chunkMap, (Node *) createConstString(PROP_DATA_CHUNK_INSERT), (Node *) dataChunkInsert);
		MAP_ADD_STRING_KEY(chunkMap, PROP_DATA_CHUNK_INSERT, dataChunkInsert);
	}

	if (dataChunkDelete && dataChunkDelete->numTuples > 0) {
		// addToMap(chunkMap, (Node *) createConstString(PROP_DATA_CHUNK_DELETE), (Node *) dataChunkDelete);
		MAP_ADD_STRING_KEY(chunkMap, PROP_DATA_CHUNK_DELETE, dataChunkDelete);
	}

	return chunkMap;
}

static void
getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
{
	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));
	List *provAttrDefs = getProvenanceAttrDefs(rewr);
	char *psName = NULL;
	if (provAttrDefs != NIL) {
		psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;
	}

	DEBUG_NODE_BEATIFY_LOG("INSERT", updateOp);
	Insert *insert = (Insert *) ((DLMorDDLOperator *) updateOp)->stmt;

	Schema *schema = createSchema(insert->insertTableName, insert->schema);

	// TableAccessOperator *taOp = NULL;
	// SelectionOperator *selOp = NULL;
	// ProjectionOperator *projOp = NULL;
	// List *projExpr = NIL;
	// int attrIdx = 0;
	// FOREACH(AttributeDef, ad, schema->attrDefs) {
	// 	AttributeReference *ar = createFullAttrReference(ad->attrName, 0, attrIdx++, 0, ad->dataType);
	// 	projExpr = appendToTailOfList(projExpr, ar);
	// }

	// taOp = createTableAccessOp(insert->insertTableName, NULL, insert->insertTableName, NIL, getAttrNames(schema), getDataTypes(schema));

	// boolean updatePSSelPD = getBoolOption(OPTION_UPDATE_PS_SELECTION_PUSH_DOWN);
	// if (updatePSSelPD) {
	// 	QueryOperator *parent = (QueryOperator *) getNthOfListP(((QueryOperator *) tableAccessOp)->parents, 0);

	// 	if (isA(parent, SelectionOperator)) {
	// 		Node *parentCond = (Node *) copyObject(((SelectionOperator *) parent)->cond);
	// 		selOp = createSelectionOp(parentCond, (QueryOperator *) taOp, NIL, getAttrNames(schema));
	// 		taOp->op.parents = singleton(selOp);
	// 	}
	// }

	// if (selOp != NULL) {
	// 	projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, getAttrNames(schema));
	// 	selOp->op.parents = singleton(selOp);
	// } else {
	// 	projOp = createProjectionOp(projExpr, (QueryOperator *) taOp, NIL, getAttrNames(schema));
	// 	taOp->op.parents = singleton(projOp);
	// }

	// char *query = serializeQuery((QueryOperator *) projOp);

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
	dataChunk->tupleFields = LIST_LENGTH(schema->attrDefs);

	Vector *ranges = NULL;
	if (psAttrCol != -1) {
		ranges = makeVector(VECTOR_INT, T_Vector);
		FOREACH(Constant, c, attrInfo->rangeList) {
			vecAppendInt(ranges, INT_VALUE(c));
		}
	}

	// insert into xx select xx from xxx;
	if (isA(insert->query, QueryBlock)) {
		// char *sql = serializeQuery((QueryOperator *) ((Insert *) updateOp)->query);

		char *sql = serializeOperatorModel((Node *) ((Insert *) updateOp)->query);
		INFO_LOG("insert serialize: %s", sql);
		return;
	} else { // only insert one row;
		INFO_LOG("single row");
		dataChunk->numTuples = 1;
		List *tupleRow = (List *) insert->query;
		for (int col = 0; col < dataChunk->tupleFields; col++) {
			Constant *value = (Constant *) getNthOfListP(tupleRow, col);
			DEBUG_NODE_BEATIFY_LOG("insert VALUE:", value);
			DataType colType = INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, col));
			Vector *colVec = NULL;
			switch (colType) {
				case DT_INT:
				{
					colVec = makeVector(VECTOR_INT, T_Vector);
					vecAppendInt(colVec, INT_VALUE(value));
				}
				break;
				case DT_LONG:
				{

					colVec = makeVector(VECTOR_LONG, T_Vector);
					vecAppendLong(colVec, LONG_VALUE(value));
				}
				break;
				case DT_FLOAT:
				{

					colVec = makeVector(VECTOR_FLOAT, T_Vector);
					vecAppendFloat(colVec, FLOAT_VALUE(value));
				}
				break;
				case DT_BOOL:
				{

					colVec = makeVector(VECTOR_INT, T_Vector);
					vecAppendInt(colVec, BOOL_VALUE(value));
				}
				break;
				case DT_STRING:
				case DT_VARCHAR2:
				{

					colVec = makeVector(VECTOR_STRING, T_Vector);
					vecAppendString(colVec, STRING_VALUE(value));
				}
				break;
				default:
					FATAL_LOG("not support this data type currently");
			}
			vecAppendNode(dataChunk->tuples, (Node*) colVec);

			if (psAttrCol != -1 && psAttrCol == col) {
				dataChunk->isAPSChunk = TRUE;
				int bitSet = setFragmengtToInt(INT_VALUE(value), ranges);
				Vector *psVec = makeVector(VECTOR_INT, T_Vector);
				vecAppendInt(psVec, bitSet);
				addToMap(dataChunk->fragmentsInfo, (Node *) createConstString(psName), (Node *) psVec);
				addToMap(dataChunk->fragmentsIsInt, (Node *) createConstString(psName), (Node *) createConstBool(TRUE));
			}
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

	DEBUG_NODE_BEATIFY_LOG("what is prov attr defs", provAttrDefs);
	char *psName = NULL;
	if (provAttrDefs != NIL) {
		psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;
		// DEBUG_NODE_BEATIFY_LOG("Attribute def: ", (AttributeDef *) getHeadOfListP(provAttrDefs));
	}

	Delete * delete = (Delete *) ((DLMorDDLOperator *) updateOp)->stmt;
	// translate delete to selection projection
	// build TableAccess <- Selection <- projection

	Schema* schema = createSchema(delete->deleteTableName, delete->schema);

	// boolean directedFromDelta = getBoolOption(OPTION_UPDATE_PS_DIRECT_DELTA);
	boolean updatePSSelPD = getBoolOption(OPTION_UPDATE_PS_SELECTION_PUSH_DOWN);
	// create tableaccess;
	TableAccessOperator* taOp = NULL;
	SelectionOperator *selOp = NULL;
	ProjectionOperator *projOp = NULL;
	List *projExpr = NIL;
	int attrIdx = 0;
	FOREACH(AttributeDef, ad, schema->attrDefs) {
		AttributeReference *ar = createFullAttrReference(ad->attrName, 0, attrIdx++, 0, ad->dataType);
		projExpr = appendToTailOfList(projExpr, ar);
	}

	// create deltaTable;
	taOp = createTableAccessOp(delete->deleteTableName, NULL, delete->deleteTableName, NIL, getAttrNames(schema), getDataTypes(schema));

	if (updatePSSelPD) {
		QueryOperator *parent = (QueryOperator *) getNthOfListP(((QueryOperator *) tableAccessOp)->parents, 0);
		if (isA(parent, SelectionOperator)) {
			Node *parentCond = (Node *) copyObject(((SelectionOperator *) parent)->cond);
			// new selection condition is and (sel->cond, del->cond)
			selOp = createSelectionOp((Node *) createOpExpr(OPNAME_AND, LIST_MAKE(delete->cond, parentCond)), (QueryOperator *) taOp, NIL, getAttrNames(schema));
			taOp->op.parents = singleton(selOp);
		}
	} else {
		selOp = createSelectionOp(delete->cond, (QueryOperator *) taOp, NIL, getAttrNames(schema));
		taOp->op.parents = singleton(selOp);
	}

	if (selOp != NULL) {
		projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, getAttrNames(schema));
		selOp->op.parents = singleton(selOp);
	} else {
		projOp = createProjectionOp(projExpr, (QueryOperator *) taOp, NIL, getAttrNames(schema));
		taOp->op.parents = singleton(projOp);
	}

	DEBUG_NODE_BEATIFY_LOG("BUILD PROJECTION: ", projOp);

	// serialize query;
	char* query = serializeQuery((QueryOperator*) projOp);

	// fill data chunk;
	int psAttrCol = -1;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++)
	{
		AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunk->posToDatatype, (Node*) createConstInt(i), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunk->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(i));

		// get ps attr col pos;
		if (psName != NULL && psAttrCol == -1 && strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = i;
		}
	}

	// INFO_LOG("this table's ps name %s", psName);
	// INFO_LOG("ps col %d", psAttrCol);
	dataChunk->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunk->tupleFields = LIST_LENGTH(schema->attrDefs);
	// DEBUG_NODE_BEATIFY_LOG("EMPTY CHUNK", dataChunk);
	Vector *ranges = NULL;
	if (psAttrCol != -1) {
		ranges = makeVector(VECTOR_INT, T_Vector);
		FOREACH(Constant, c, attrInfo->rangeList) {
			vecAppendInt(ranges, INT_VALUE(c));
		}
	}

	postgresGetDataChunkFromStmtInsDel(query, dataChunk, psAttrCol, (psAttrCol == -1 ? NULL : ranges), psName, -1);

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR DELETE", dataChunk);
}

static void
getDataChunkOfUpdate(QueryOperator *updateOp, DataChunk *dataChunkInsert, DataChunk *dataChunkDelete, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo)
{
	QueryOperator *rewr = captureRewrite((QueryOperator *) copyObject(tableAccessOp));

	List *provAttrDefs = getProvenanceAttrDefs(rewr);

	char *psName = NULL;
	if (provAttrDefs != NIL) {
		psName = ((AttributeDef *) getHeadOfListP(provAttrDefs))->attrName;
	}

	// get update statement;
	Update *update = (Update *) ((DLMorDDLOperator *) updateOp)->stmt;

	// create table access operator;
	Schema *schema = createSchema(update->updateTableName, update->schema);

	TableAccessOperator *taOp = NULL;
	SelectionOperator *selOp = NULL;

	taOp = createTableAccessOp(update->updateTableName, NULL, update->updateTableName, NIL, getAttrNames(schema), getDataTypes(schema));


	boolean updatePSSelPD = getBoolOption(OPTION_UPDATE_PS_SELECTION_PUSH_DOWN);
	if (updatePSSelPD) {
		QueryOperator *parent = (QueryOperator *) getNthOfListP(((QueryOperator *) tableAccessOp)->parents, 0);
		if (isA(parent, SelectionOperator)) {
			Node *parentCond = (Node *) copyObject(((SelectionOperator *) parent)->cond);
			Node *updCond = update->cond;
			Node *newCond = NULL;
			if (updCond != NULL) {
				newCond = (Node *) createOpExpr(OPNAME_AND, LIST_MAKE(parentCond, updCond));
			} else {
				newCond = parentCond;
			}
			selOp = createSelectionOp(newCond, (QueryOperator *) taOp, NIL, getAttrNames(schema));
			taOp->op.parents = singleton(selOp);
		}
	} else {
		if (update->cond != NULL) {
			selOp = createSelectionOp(update->cond, (QueryOperator *) taOp, NIL, getAttrNames(schema));
			taOp->op.parents = singleton(selOp);
		}
	}

	DEBUG_NODE_BEATIFY_LOG("NEW SEL IN UPD: ", selOp);
	INFO_OP_LOG("NEW SEL IN UPD:", selOp);

	// create projection operator;
	// select attr as old_attr, attr' as new_attr, ... from xxxx;
	// key     -> list[ele1, ele2]
	// attname -> list[old_att, new_att]
	HashMap* map = NEW_MAP(Constant, Node);

	// att name to old name;
	for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++) {
		AttributeDef *ad = (AttributeDef *) getNthOfListP(schema->attrDefs, i);
		AttributeReference *af = createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType);
		// List *l = NIL;
		Vector *v = makeVector(VECTOR_NODE, T_Vector);
		// l = appendToTailOfList(l, af);
		vecAppendNode(v, (Node *) af);
		addToMap(map, (Node*) createConstString(ad->attrName), (Node *) v);
	}

	DEBUG_NODE_BEATIFY_LOG("OLD MAP \n", map);
	// att name to new name;
	for (int i = 0; i < LIST_LENGTH(update->selectClause); i++) {
		// op->list[0]: old attribute name;
		// op->list[1]: new attribute name;

		Operator *op = (Operator *) getNthOfListP(update->selectClause, i);
		DEBUG_NODE_BEATIFY_LOG("OP in upd", op);
		char* oldName = ((AttributeReference *) getNthOfListP(op->args, 0))->name;

		Vector *v = (Vector *) MAP_GET_STRING(map, oldName);
		// l = appendToTailOfList(l, (Node *) getNthOfListP(op->args, 1));
		vecAppendNode(v, (Node *) getNthOfListP(op->args, 1));

		addToMap(map, (Node *) createConstString(oldName), (Node *) v);
	}
	DEBUG_NODE_BEATIFY_LOG("NEW MAP \n", map);

	// create projection operator;
	ProjectionOperator *projOp = NULL;
	List* projExpr = NIL;
	List* attrNames = NIL;
	INFO_LOG("1");
	// for (int i = 0; i < LIST_LENGTH(schema->attrDefs); i++) {
	FOREACH(AttributeDef, ad, schema->attrDefs) {
		// AttributeDef *ad = (AttributeDef *) getNthOfListP(schema->attrDefs, i);

		StringInfo oldName = makeStringInfo();
		appendStringInfo(oldName, "old_%s", ad->attrName);
		StringInfo newName = makeStringInfo();
		appendStringInfo(newName, "new_%s", ad->attrName);

		attrNames = appendToTailOfList(attrNames, oldName->data);
		attrNames = appendToTailOfList(attrNames, newName->data);


		// List *l = (List *) getMapString(map, ad->attrName);
		Vector *v = (Vector *) MAP_GET_STRING(map, ad->attrName);
		DEBUG_NODE_BEATIFY_LOG("list\n", v);
		// projExpr = appendToTailOfList(projExpr, getNthOfListP(l, 0));
		projExpr = appendToTailOfList(projExpr, getVecNode(v, 0));

		if (v->length == 1) {
			// projExpr = appendToTailOfList(projExpr, getNthOfListP(l, 0));
			projExpr = appendToTailOfList(projExpr, getVecNode(v, 0));
		} else {
			// projExpr = appendToTailOfList(projExpr, getNthOfListP(l, 1));
			projExpr = appendToTailOfList(projExpr, getVecNode(v, 1));
		}
	}

	if (selOp != NULL) {
		projOp = createProjectionOp(projExpr, (QueryOperator *) selOp, NIL, attrNames);
		selOp->op.parents = singleton(projOp);
	} else {
		projOp = createProjectionOp(projExpr, (QueryOperator *) taOp, NIL, attrNames);
		taOp->op.parents = singleton(projOp);

	}
	DEBUG_NODE_BEATIFY_LOG("proj op\n", projOp);
	INFO_OP_LOG("proj op", projOp);

	// serialize query;
	char * sql = serializeQuery((QueryOperator*) projOp);
	INFO_LOG("SQL: %s", sql);

	// fill data chunk;
	int psAttrCol = -1;
	int attrIdx = 0;
	FOREACH(AttributeDef, ad, schema->attrDefs) {
		// AttributeDef* ad = (AttributeDef*) getNthOfListP(schema->attrDefs, i);
		addToMap(dataChunkInsert->posToDatatype, (Node*) createConstInt(attrIdx), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunkInsert->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(attrIdx));
		addToMap(dataChunkDelete->posToDatatype, (Node*) createConstInt(attrIdx), (Node*) createConstInt(ad->dataType));
		addToMap(dataChunkDelete->attriToPos, (Node*) createConstString(ad->attrName), (Node*) createConstInt(attrIdx));

		// INFO_LOG("adname: %s, psName: %s", ad->attrName, psName);
		if (psName != NULL && strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = attrIdx;
		}
		attrIdx++;
	}

	dataChunkInsert->attrNames = (List *) copyObject(schema->attrDefs);
	dataChunkDelete->attrNames = (List *) copyObject(schema->attrDefs);

	Vector *ranges = NULL;
	if (psAttrCol != -1) {
		ranges = makeVector(VECTOR_INT, T_Vector);
		FOREACH(Constant, c, attrInfo->rangeList) {
			vecAppendInt(ranges, INT_VALUE(c));
		}
	}

	postgresGetDataChunkFromStmtUpd(sql, dataChunkInsert, dataChunkDelete, psAttrCol, ranges, psName);


	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR UPDATE", dataChunkInsert);
	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR UPDATE", dataChunkDelete);
}

static DataChunk*
filterDataChunk(DataChunk* dataChunk, Node* condition)
{

	DEBUG_NODE_BEATIFY_LOG("DISPLAY DATACHUNK: ", dataChunk);

	ColumnChunk *filterResult = evaluateExprOnDataChunk(condition, dataChunk);

	DEBUG_NODE_BEATIFY_LOG("output columnchunk", filterResult);
	// new a result data chunk AND set fields except numTuples;
	DataChunk* resultDC = initDataChunk();
	resultDC->attrNames = (List*) copyList(dataChunk->attrNames);
	resultDC->attriToPos = (HashMap*) copyObject(dataChunk->attriToPos);
	resultDC->posToDatatype = (HashMap*) copyObject(dataChunk->posToDatatype);
	resultDC->tupleFields = dataChunk->tupleFields;

	// result is True or False stored in a bitset
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

		if (col == 0) {
			resultDC->updateIdentifier = undateIden;
		}
	}
	resultDC->numTuples = resultDC->updateIdentifier->length;
	if (dataChunk->isAPSChunk) {
		FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
			boolean isPSInt = BOOL_VALUE(MAP_GET_STRING(dataChunk->fragmentsIsInt, STRING_VALUE(c)));
			Vector *fromPSVec = (Vector *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
			Vector *toPSVec = NULL;
			if (isPSInt) {
				toPSVec = makeVector(VECTOR_INT, T_Vector);
				int *psVals = (int *) VEC_TO_IA(fromPSVec);
				for (int row = 0; row < fromPSVec->length; row++) {
					if (trueOrFalse[row] == 1) {
						vecAppendInt(toPSVec, psVals[row]);
					}
				}
			} else {
				BitSet ** psVals = VEC_TO_ARR(fromPSVec, BitSet);
				toPSVec = makeVector(VECTOR_NODE, T_Vector);
				for (int row = 0; row < fromPSVec->length; row++) {
					if (trueOrFalse[row] == 1) {
						vecAppendNode(toPSVec, (Node *) psVals[row]);
					}
				}
			}
			addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) toPSVec);
		}
		resultDC->isAPSChunk = TRUE;
		resultDC->fragmentsIsInt = dataChunk->fragmentsIsInt;
	}
	DEBUG_NODE_BEATIFY_LOG("output chunk in selection", resultDC);
	return resultDC;
}

/*list rangelist to vector*/
int
setFragmengtToInt(int value, Vector *rangeList)
{
	int *ranges = VEC_TO_IA(rangeList);
	int rangeLen = rangeList->length;

	// check if value is beyond the ranges;
	if (value < ranges[0]) {
		return 0;
	} else if (value >= ranges[rangeLen - 2]) {
		return rangeLen - 2;
	}

	// binary search;
	int start = 0;
	int end = rangeLen - 2;

	while (start + 1 < end) {
		int mid = start + (end - start) / 2;

		int leftVal = ranges[mid];
		int rightVal = ranges[mid + 1];

		if (leftVal <= value && value < rightVal) {
			return mid;
		} else if (value < leftVal) {
			end = mid;
		} else {
			start = mid;
		}
	}

	if (ranges[start] <= value && value < ranges[start + 1]) {
		return start;
	}
	return end;
}

BitSet *
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

// static Relation*
// getQueryResult(char* sql)
// {
// 	Relation* relation = NULL;
// 	// TODO: generic executeQuery(sql);
// 	// get query result
// 	if (getBackend() == BACKEND_POSTGRES) {
// 		relation = postgresExecuteQuery(sql);
// 	} else if (getBackend() == BACKEND_ORACLE) {

// 	}

// 	return relation;
// }

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
		case T_QuantifiedComparison:
			return getColumnChunkOfQuantComp((QuantifiedComparison *) expr, dc);
		case T_CastExpr:
			return getColumnChunkOfCastExpr((CastExpr *) expr, dc);
		case T_FunctionCall:
			return getColumnChunkOfFunctionCall((FunctionCall *) expr, dc);
		default:
			FATAL_LOG("cannot evaluate this expr");
	}

	return 0;
}

static ColumnChunk *
getColumnChunkOfCastExpr(CastExpr *expr, DataChunk *dc)
{
	ColumnChunk *cc = evaluateExprOnDataChunk(expr->expr, dc);
	if (cc->dataType == expr->resultDT) {
		return cc;
	}

	cc = castColumnChunk(cc, cc->dataType, expr->resultDT);
	return cc;
}

static ColumnChunk *
getColumnChunkOfFunctionCall(FunctionCall *fc, DataChunk *dc)
{
	// ColumnChunk *cc = makeColumnChunk(DT_STRING, len);
	if (strcmp(backendifyIdentifier(fc->functionname), backendifyIdentifier("date_part")) == 0){
		Constant *c = (Constant *) getNthOfListP(fc->args, 0);
		AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 1);
		int exprIdx = INT_VALUE((Constant *) MAP_GET_STRING(dc->attriToPos, ar->name));
		if (strcmp(backendifyIdentifier(STRING_VALUE(c)), backendifyIdentifier("year")) == 0) {
			// find year start position;
			// assume year is 4 digit and must exist;
			char *sample = sample = getVecString((Vector *) getVecNode(dc->tuples, exprIdx), 0);
			int sampleLen = strlen(sample);
			Vector *dashPos = makeVector(VECTOR_INT, T_Vector);
			for (int i = 0; i < sampleLen; i++) {
				if (sample[i] == '-') {
					vecAppendInt(dashPos, i);
				}
			}
			int pre = -1;
			int start = -1;
			for(int i = 0; i < dashPos->length; i++) {
				int pos = getVecInt(dashPos, i);
				if (pos - pre == 5) {
					start = pre + 1;
					break;
				} else {
					pre = pos;
				}
			}

			if (start == -1) {
				start = pre + 1;
			}

			int len = dc->numTuples;
			ColumnChunk *cc = makeColumnChunk(DT_STRING, len);
			char **ccVals = (char **) VEC_TO_ARR((Vector *) cc->data.v, char);
			char ** vals = (char **) VEC_TO_ARR((Vector *) getVecNode(dc->tuples, exprIdx), char);
			for (int i = 0; i < len; i++) {
				char *res = MALLOC(5);
				strncpy(res, vals[i] + start, 4);
				res[4] = '\0';
				// vecAppendString((Vector *) cc->data.v, res);
				ccVals[i] = res;
			}
			return cc;
		}
	}
	return NULL;
}

/*
	this function is specific for where condition that attr in (n1, n2, n3... nn);
	QuantifiedCOmparison *qc
	{
		opName: = (always = )
		checkExpr: experision(single attr or Math(attr1, attr2, ..., attrn))
		exprList: a list of values
	}
 */
static ColumnChunk *
getColumnChunkOfQuantComp(QuantifiedComparison *qc, DataChunk *dc)
{
	// ColumnChunk *left =  evaluateExprOnDataChunk(qc->checkExpr, dc);
	// exprList put in a set;
	return NULL;
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

static DataType
evaluateTwoDatatypes(DataType dt1, DataType dt2)
{
	// support for +,-,*,/
	if (dt1 == DT_FLOAT || dt2 == DT_FLOAT) {
		return DT_FLOAT;
	} else if (dt1 == DT_LONG || dt2 == DT_LONG) {
		return DT_LONG;
	}
	return DT_INT;
}


static ColumnChunk *
evaluateOperatorPlus(Operator *op, DataChunk *dc)
{
	List *inputs = op->args;
	ColumnChunk *left = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 0), dc);
	ColumnChunk *right = evaluateExprOnDataChunk((Node *) getNthOfListP(inputs, 1), dc);

	// ASSERT(left->length == right->length);

	int len = left->length;

	// DataType resultDT = typeOf((Node *) op);
	DataType resultDT = evaluateTwoDatatypes((DataType) left->dataType, (DataType) right->dataType);
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

	// DataType resultDT = typeOf((Node *) op);
	DataType resultDT = evaluateTwoDatatypes((DataType) left->dataType, (DataType) right->dataType);
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

	// DataType resultDT = typeOf((Node *) op);
	DataType resultDT = evaluateTwoDatatypes((DataType) left->dataType, (DataType) right->dataType);
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

	// DataType resultDT = typeOf((Node *) op);
	DataType resultDT = evaluateTwoDatatypes((DataType) left->dataType, (DataType) right->dataType);
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
	// DEBUG_NODE_BEATIFY_LOG("LEFT CHUNK", left);
	// DEBUG_NODE_BEATIFY_LOG("right chunk", right);
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
				resV[i] = (leftV[i] == rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] == rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] == rightV[i] ? 1 : 0);
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
				resV[i] = (leftV[i] <= rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_FLOAT: {
			double *leftV = VEC_TO_FA(castLeft->data.v);
			double *rightV = VEC_TO_FA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] <= rightV[i] ? 1 : 0);
			}
		}
		break;
		case DT_LONG: {
			gprom_long_t *leftV = VEC_TO_LA(castLeft->data.v);
			gprom_long_t *rightV = VEC_TO_LA(castRight->data.v);
			for (int i = 0; i < len; i++) {
				resV[i] = (leftV[i] <= rightV[i] ? 1 : 0);
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
	DEBUG_NODE_BEATIFY_LOG("LEFT OF LIKE", left);
	DEBUG_NODE_BEATIFY_LOG("RIGHT OF LIKE", right);

	// ASSERT(left->length == right->length);

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

	// ASSERT(castLeft->dataType == DT_STRING && castRight->dataType == DT_STRING);

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
	// INFO_LOG("pattern : %d, modMode: %d", matchMode, modMode);
	// now only support one %;
	if (matchMode != 2 || (matchMode == 2 && modMode > 2)) {
		FATAL_LOG("NOT SUPPORT THIS TYPE OF 'like'");
	}

	// StringInfo modPtn = makeStringInfo();
	char *likeStr = rightV[0];
	int *resV = VEC_TO_IA(resultCC->data.v);
	if (modMode == 1) {
		int strLen = strlen(likeStr);
		// for (int i = 1; i < strLen; i++) {
		// 	appendStringInfoChar(modPtn, likeStr[i]);
		// }

		for (int row = 0; row < length; row++) {
			int str2Len = strlen(leftV[row]);
			if (str2Len >= strLen - 1) {
				int idx = strLen - 1;
				int idx2 = str2Len - 1;
				boolean findRes = FALSE;
				while (idx > 0) {
					if (leftV[row][idx2] != likeStr[idx]) {
						resV[row] = 0;
						findRes = TRUE;
						break;
					}
					idx--;
					idx2--;
				}
				if (!findRes) {
					resV[row] = 1;
				}
			} else {
				resV[row] = 0;
			}
		}
	}
	if (modMode == 2) {
		int strLen = strlen(likeStr);
		// for (int i = 0; i < strLen - 1; i++) {
		// 	appendStringInfoChar(modPtn, likeStr[i]);
		// }
		for (int row = 0; row < length; row++) {
			if (strlen(leftV[row]) >= strLen - 1) {
				boolean findRes = FALSE;
				for (int i = 0; i < strLen - 1; i++) {
					if (leftV[row][i] != likeStr[i]) {
						resV[row] = 0;
						findRes = TRUE;
						break;
					}
				}
				if (!findRes) {
					resV[row] = 1;
				}
			} else {
				resV[row] = 0;
			}
		}
	}
	/*
	// used for later
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
	// strncmp()
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
*/
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
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = vals[i];
				}
			} else if (DT_STRING == fromType || DT_VARCHAR2 == fromType) {
				char** vals = VEC_TO_ARR(cc->data.v, char);
				for (int i = 0; i < length; i++) {
					resV[i] = atoi(vals[i]);
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
			} else if (DT_STRING == fromType || DT_VARCHAR2 == fromType){
				char ** vals = (char **) VEC_TO_ARR(cc->data.v, char);
				for (int i = 0; i < length; i++) {
					resV[i] = (double) atof(vals[i]);
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
			} else if (DT_STRING == fromType || DT_VARCHAR2 == fromType) {
				char ** vals = (char **) VEC_TO_ARR(cc->data.v, char);
				for (int i = 0; i < length; i++) {
					resV[i] = (gprom_long_t) atol(vals[i]);
				}
			} else {
				FATAL_LOG("not supported");
			}
		}
		break;
		case DT_BOOL: {
			int *resV = VEC_TO_IA(resultCC->data.v);
			if (DT_STRING == fromType || DT_VARCHAR2 == fromType) {
				char ** vals = (char **) VEC_TO_ARR(cc->data.v, char);
				for (int i = 0; i < length; i++) {
					if (strcmp(vals[i], "t") == 0 || strcmp(vals[i], "true") == 0 || strcmp(vals[i], "TRUE") == 0) {
						resV[i] = 1;
					} else {
						resV[i] = 0;
					}
				}
			} else if (DT_INT == fromType) {
				int *vals = VEC_TO_IA(cc->data.v);
				for (int i = 0; i < length; i++) {
					resV[i] = (vals[i] == 0 ? 0 : 1);
				}
			} else {
				FATAL_LOG("not supported");
			}
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

// static char *
// constToString(Constant *c)
// {
// 	StringInfo info = makeStringInfo();

// 	switch(c->constType) {
// 		case DT_INT:
// 			appendStringInfo(info, "%s", gprom_itoa(INT_VALUE(c)));
// 			break;
// 		case DT_FLOAT:
// 			appendStringInfo(info, "%s", gprom_ftoa(FLOAT_VALUE(c)));
// 			break;
// 		case DT_LONG:
// 			appendStringInfo(info, "%s", gprom_ltoa(LONG_VALUE(c)));
// 			break;
// 		case DT_STRING:
// 			appendStringInfo(info, "%s", STRING_VALUE(c));
// 			break;
// 		case DT_BOOL:
// 			appendStringInfo(info, "%s", BOOL_VALUE(c) == 1 ? "t" : "f");
// 			break;
// 		default:
// 			FATAL_LOG("datatype %d is not supported", c->constType);
// 	}

// 	return info->data;
// }
