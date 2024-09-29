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
#include "provenance_rewriter/update_ps/base64.h"
#include "provenance_rewriter/update_ps/sketch_based_checking.h"

static void updateByOperators(QueryOperator * op);
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
static HashMap *getDataChunkFromDeltaTable(TableAccessOperator * tableAccessOp);
static Node *buildWhereClauseFromSketch(AttributeReference *psAttrRef, Vector *ranges);


static ProvenanceComputation *PC = NULL;
static QueryOperator* updateStatement = NULL;
static psInfo *PS_INFO = NULL;
static StringInfo strInfo;

char *
sketch_based_checking(QueryOperator* operator, QueryOperator *updateStmt)
{
	START_TIMER("module - update provenance sketch - pre");
	PC = (ProvenanceComputation *) copyObject(operator);
	PC->op.inputs = NIL;
	PS_INFO = createPSInfo((Node *) getStringProperty(operator, PROP_PC_COARSE_GRAINED));

	DEBUG_NODE_BEATIFY_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR: \n", operator);
	INFO_OP_LOG("CURRENT PROVENANCE COMPUTATION OPERATOR", operator);
	strInfo = makeStringInfo();

	INFO_LOG("Start update");
	DEBUG_NODE_BEATIFY_LOG("update stmt", updateStmt);
	updateStatement = updateStmt;


	STOP_TIMER("module - update provenance sketch - pre");

	START_TIMER(INCREMENTAL_UPDATE_ACTUAL_TIMER);
	updateByOperators((QueryOperator*) OP_LCHILD(operator));
	STOP_TIMER(INCREMENTAL_UPDATE_ACTUAL_TIMER);

	return strInfo->data;
}

static void
updateByOperators(QueryOperator * op)
{
	switch(op->type)
	{
		case T_ProvenanceComputation:
		{
			updateProvenanceComputation(op);
		}
			break;
		case T_ProjectionOperator:
		{
			updateProjection(op);
		}
			break;
		case T_SelectionOperator:
		{
			updateSelection(op);
		}
			break;
		case T_JoinOperator:
		{
			updateJoin(op);
		}
			break;
		case T_AggregationOperator:
		{
			START_TIMER(INCREMENTAL_AGGREGATION_OPERATOR);
			updateAggregation(op);
			STOP_TIMER(INCREMENTAL_AGGREGATION_OPERATOR);
		}
			break;
		case T_TableAccessOperator:
		{
			START_TIMER(INCREMENTAL_TABLE_OPERATOR);
			updateTableAccess(op);
			STOP_TIMER(INCREMENTAL_TABLE_OPERATOR);
		}
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
updateSet(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));
	return;
}
static void
updateProvenanceComputation(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	return;
}

static void
updateProjection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	return;

}

static void
updateSelection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	return;
}

static void
updateJoin(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));
	return;
}

static void
updateAggregation(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));
	return;
}

static void
updateDuplicateRemoval(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));
	return;
}

static void
updateOrder(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));
	return;
}

static void
updateLimit(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));
	return;
}

static void
updateTableAccess(QueryOperator * op)
{
	char* tableName = ((TableAccessOperator *) op)->tableName;
	boolean isUpdatedDirectFromDelta = getBoolOption(OPTION_UPDATE_PS_DIRECT_DELTA);
	char *updatedTableName = NULL;
	if (isUpdatedDirectFromDelta) {
		updatedTableName = getStringOption(OPTION_UPDATE_PS_UPDATED_TABLE);
	} else {
		return;
	}

	if (strcmp(updatedTableName, tableName) != 0) {
		return;
	}

	START_TIMER(INCREMENTAL_FETCHING_DATA_TIMER);
	// build a chumk map (insert chunk and delete chunk) based on update type;
	HashMap *chunkMap = NULL;
	// printf("is directed from delta %d\n",isUpdatedDirectFromDelta);
	if (isUpdatedDirectFromDelta) {
		chunkMap = getDataChunkFromDeltaTable((TableAccessOperator *) op);
	} else {
		chunkMap = getDataChunkFromDeltaTable((TableAccessOperator *) op);
	}
	if (mapSize(chunkMap) > 0) {
		setStringProperty(op, PROP_DATA_CHUNK, (Node *) chunkMap);
	}
	STOP_TIMER(INCREMENTAL_FETCHING_DATA_TIMER);

}

static HashMap *
getDataChunkFromDeltaTable(TableAccessOperator * tableAccessOp)
{
	QueryOperator *rewr = captureRewriteOp(PC, (QueryOperator *) copyObject(tableAccessOp));
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

	// ps attr col pos;
	int psAttrCol = -1;
	AttributeReference *psAttrRef = NULL;
	int attrIdx = 0;
	FOREACH(AttributeDef, ad, schema->attrDefs) {
		addToMap(dcIns->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrIdx));
		addToMap(dcDel->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(attrIdx));

		addToMap(dcIns->posToDatatype, (Node *) createConstInt(attrIdx), (Node *) createConstInt(ad->dataType));
		addToMap(dcDel->posToDatatype, (Node *) createConstInt(attrIdx), (Node *) createConstInt(ad->dataType));

		if (psName != NULL && psAttrCol == -1 && strcmp(ad->attrName, attrInfo->attrName) == 0) {
			psAttrCol = attrIdx;
			psAttrRef = createFullAttrReference(ad->attrName, 0, attrIdx, 0, ad->dataType);
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

	boolean isSketchBasedFilter = getBoolOption(OPTION_UPDATE_PS_SKETCH_BASED_FILTER);
	if (isSketchBasedFilter) {
		if (psAttrCol != -1) {
			Node *whereConditions = buildWhereClauseFromSketch(psAttrRef, ranges);
			if (whereConditions != NULL) {
				if (selOp == NULL) {
					selOp = createSelectionOp(whereConditions, (QueryOperator *) taOp, NIL, attrNames);
					taOp->op.parents = singleton(selOp);
				} else {
					selOp->cond = andExprList(LIST_MAKE(selOp->cond, whereConditions));
				}
			}
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
	// STOP_TIMER(INCREMENTAL_FETCHING_DATA_BUILD_QUERY_TIMER);
	DEBUG_NODE_BEATIFY_LOG("delta query: ", projOp);
	char *query = serializeQuery((QueryOperator *) projOp);
	INFO_LOG("sql: %s", sql);
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

static Node *
buildWhereClauseFromSketch(AttributeReference *psAttrRef, Vector *ranges)
{
	// read sketch;
	char *sketchPath = getStringOption(OPTION_UPDATE_PS_SKETCH_BASED_SKETCH);
	FILE *sketchFile = fopen(sketchPath, "r");
    if (!sketchFile) {
        return NULL;
    }
	StringInfo sketch = makeStringInfo();
	fseek(sketchFile, 0, SEEK_SET);
	char ch;
	while ((ch = fgetc(sketchFile)) != EOF) {
	  	if (ch == '1' || ch == '0') {
			appendStringInfoChar(sketch, ch);
		}
    }
	fclose(sketchFile);
	INFO_LOG("SKETCH FROM FILE: \n sketch: %s, len of sketch: %d", sketch->data, strlen(sketch->data));
	INFO_LOG("RANGE LENGTH: %d", ranges->length);

	// Build conds based on PS and Ranges;
	List *condsList = NIL;
	int startPos = -1;
	int bitCount = 0;
	int sketchLen = strlen(sketch->data);
	for (int idx = 0; idx < sketchLen; idx++) {
		if (sketch->data[idx] == '1') {
			if (startPos == -1) {
				startPos = idx;
				bitCount = 1;
			} else {
				bitCount += 1;
			}
		} else {
			if (bitCount > 0) {
				int leftBoundary = getVecInt(ranges, startPos);
				Operator *lOp = createOpExpr(">=", LIST_MAKE((Node *) psAttrRef, (Node *) createConstInt(leftBoundary)));
				int rightBoundary = getVecInt(ranges, startPos + bitCount);
				Operator *rOp = createOpExpr("<", LIST_MAKE((Node *) psAttrRef, (Node *) createConstInt(rightBoundary)));
				Node *andExp = andExprList(LIST_MAKE(lOp, rOp));
				condsList = appendToTailOfList(condsList, andExp);

				bitCount = 0;

			}
			startPos = -1;
		}

		// check when at last position;
		if (idx == sketchLen - 1) {
			if (bitCount > 0) {
				//left;
				int leftBoundary = getVecInt(ranges, startPos);
				Operator *lOp = createOpExpr(">=", LIST_MAKE((Node *) psAttrRef, (Node *) createConstInt(leftBoundary)));
				//right;
				int rightBoundary = getVecInt(ranges, startPos + bitCount);
				Operator *rOp = createOpExpr("<", LIST_MAKE((Node *) psAttrRef, (Node *) createConstInt(rightBoundary)));

				Node *andExp = andExprList(LIST_MAKE(lOp, rOp));
				condsList = appendToTailOfList(condsList, andExp);
			}
		}
	}

	INFO_LOG("cond list length: %d", LIST_LENGTH(condsList));
	DEBUG_NODE_BEATIFY_LOG("CONDS: ", condsList);

	Node *whereConditions = NULL;
	if (LIST_LENGTH(condsList) == 1) {
		return getNthOfListP(condsList, 0);
	} else if (LIST_LENGTH(condsList) > 1) {
		whereConditions = orExprList(condsList);
	}

	return whereConditions;
}



