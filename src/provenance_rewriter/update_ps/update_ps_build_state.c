
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
#include "provenance_rewriter/update_ps/rbtree.h"
#include "model/set/set.h"

#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "provenance_rewriter/update_ps/bloom.h"
#include "provenance_rewriter/update_ps/base64.h"

//#include "provenance_rewriter/update_ps/update_ps_main.h"
//#include "provenance_rewriter/update_ps/update_ps_incremental.h"
#include "provenance_rewriter/update_ps/update_ps_build_state.h"
#include "instrumentation/timing_instrumentation.h"

static void buildStateInternal(QueryOperator *op);
static void buildStateAggregationOp(QueryOperator *op);
static void buildStateDuplicateRemovalOp(QueryOperator *op);
static void buildStateLimitOp(QueryOperator *op);
static void buildStateOrderOp(QueryOperator *op);
static void buildStateFinalOp(QueryOperator *op);
static void buildStateJoinOp(QueryOperator *op);
static void parseJoinConds(Node *conds, Vector *condsVec);
static void buildBloomFilterFromSQL(char *sql, List *attributeDefs, HashMap *BFMap);
static Vector *buildGroupByValueVecFromRelation(Relation *rel, List *gbList);
// static QueryOperator *captureRewriteOp(ProvenanceComputation *pc, QueryOperator *op);
// static int compareTwoValues(Constant *a, Constant *b, DataType dt);
static void swapListCell(List *list, int pos1, int pos2);

static ProvenanceComputation *PC_BuildState = NULL;

QueryOperator *
buildState(QueryOperator *op)
{
	START_TIMER(BUILD_STATE_TIMER);
	PC_BuildState = (ProvenanceComputation *) copyObject(op);
	PC_BuildState->op.inputs = NIL;

    buildStateInternal(op);

	SET_STRING_PROP(op, PROP_HAS_DATA_STRUCTURE_BUILT, (Node *) createConstBool(TRUE));

	STOP_TIMER(BUILD_STATE_TIMER);

    return op;
}

boolean hasLimitOpAbove(QueryOperator *op)
{
	if (isA(op, LimitOperator)) {
		return TRUE;
	}

	if (!isA(op, ProjectionOperator) && !isA(op, OrderOperator)) {
		return FALSE;
	}

	FOREACH(QueryOperator, o, op->parents) {
		if (hasLimitOpAbove(o)) {
			return TRUE;
		}
	}

	return FALSE;
}

boolean hasOrderOpBelow(QueryOperator *op)
{
	// check if order op below limit and between these two ops, only can exists projection op
	if (isA(op, OrderOperator)) {
		return TRUE;
	}

	if (!isA(op, ProjectionOperator) && !isA(op, LimitOperator)) {
		return FALSE;
	}

	if (LIST_LENGTH(op->inputs) > 1) {
		return FALSE;
	}

	FOREACH(QueryOperator, o, op->inputs) {
		if (hasOrderOpBelow(o)) {
			return TRUE;
		}
	}

	return FALSE;
}

static void
buildStateInternal(QueryOperator *op)
{
    if (isA(op, TableAccessOperator)) {
        return ;
    }

    FOREACH(QueryOperator, q, op->inputs) {
        buildStateInternal(q);
    }

    if (isA(op, AggregationOperator)) {
        buildStateAggregationOp(op);
    } else if (isA(op, DuplicateRemoval)) {
        buildStateDuplicateRemovalOp(op);
    } else if (isA(op, LimitOperator)) {
        buildStateLimitOp(op);
    } else if (isA(op, OrderOperator )) {
		buildStateOrderOp(op);
	} else if (isA(op, ProvenanceComputation)) {
        buildStateFinalOp(op);
    } else if (isA(op, JoinOperator)) {
		boolean isJoinUsingBF = getBoolOption(OPTION_UPDATE_PS_JOIN_USING_BF);
		if (isJoinUsingBF) {
			buildStateJoinOp(op);
		}
	}
}

static void
buildStateJoinOp(QueryOperator *op)
{
	Node *conds = (Node *) copyObject(((JoinOperator *) op)->cond);

	Vector *condsVec = makeVector(VECTOR_NODE, T_Vector);
	// Vector *condsAttrVec = makeVector(VECTOR_NODE, T_Vector);
	parseJoinConds(conds, condsVec);

	// for each attribute find out which branch it comes from;
	Set *leftAttrs = NODESET();
	Set *rightAttrs = NODESET();

	HashMap *leftMapping = NEW_MAP(Constant, Node);
	HashMap *rightMapping = NEW_MAP(Constant, Node);

	// parse to get all attributes of conditions;
	FOREACH_VEC(Operator, op, condsVec){
		AttributeReference *ar1 = (AttributeReference *) getNthOfListP(op->args, 0);
		AttributeReference *ar2 = (AttributeReference *) getNthOfListP(op->args, 1);
		DEBUG_NODE_BEATIFY_LOG("ar1", ar1);
		DEBUG_NODE_BEATIFY_LOG("ar2", ar2);
		if (ar1->fromClauseItem == 0) {
			addToSet(leftAttrs, ar1);
			Vector *v = NULL;
			if (MAP_HAS_STRING_KEY(leftMapping, ar1->name)) {
				v = (Vector *) MAP_GET_STRING(leftMapping, ar1->name);
			} else {
				v = makeVector(VECTOR_STRING, T_Vector);
			}
			vecAppendString(v, strdup(ar2->name));
			MAP_ADD_STRING_KEY(leftMapping, strdup(ar1->name), v);
		} else if (ar1->fromClauseItem == 1) {
			addToSet(rightAttrs, ar1);
			Vector *v = NULL;
			if (MAP_HAS_STRING_KEY(rightMapping, ar1->name)) {
				v = (Vector *) MAP_GET_STRING(rightMapping, ar1->name);
			} else {
				v = makeVector(VECTOR_STRING, T_Vector);
			}
			vecAppendString(v, strdup(ar2->name));
			MAP_ADD_STRING_KEY(rightMapping, strdup(ar1->name), v);
		}

		if (ar2->fromClauseItem == 0) {
			addToSet(leftAttrs, ar2);
			Vector *v = NULL;

			if (MAP_HAS_STRING_KEY(leftMapping, ar2->name)) {
				v = (Vector *) MAP_GET_STRING(leftMapping, ar2->name);
			} else {
				v = makeVector(VECTOR_STRING, T_Vector);
			}
			vecAppendString(v, strdup(ar1->name));
			MAP_ADD_STRING_KEY(leftMapping, strdup(ar2->name), v);
		} else if (ar2->fromClauseItem == 1) {
			addToSet(rightAttrs, ar2);
			Vector *v = NULL;
			if (MAP_HAS_STRING_KEY(rightMapping, ar2->name)) {
				v = (Vector *) MAP_GET_STRING(rightMapping, ar2->name);
			} else {
				v = makeVector(VECTOR_STRING, T_Vector);
			}
			vecAppendString(v, strdup(ar1->name));
			MAP_ADD_STRING_KEY(rightMapping, strdup(ar2->name), v);
		}
	}
	DEBUG_NODE_BEATIFY_LOG("conds vec", condsVec);


	// create left projection;
	List *leftProjExpr = NIL;
	List *leftADs = NIL;
	int idx = 0;
	FOREACH(AttributeDef, ad, OP_LCHILD(op)->schema->attrDefs) {
		FOREACH_SET(AttributeReference, ar, leftAttrs) {
			if (streq(ad->attrName, ar->name)) {
				leftProjExpr = appendToTailOfList(leftProjExpr, createFullAttrReference(ad->attrName, 0, idx, 0, ad->dataType));
				leftADs = appendToTailOfList(leftADs, copyObject(ad));
			}
		}
		idx++;
	}
	QueryOperator *lChild = (QueryOperator *) copyObject(OP_LCHILD(op));
	ProjectionOperator *leftProj = createProjectionOp(leftProjExpr, lChild, NIL, getAttrDefNames(leftADs));
	lChild->parents = singleton(leftProj);


	// create right projection;
	List *rightProjExpr = NIL;
	List *rightADs = NIL;
	idx = 0;
	FOREACH(AttributeDef, ad, OP_RCHILD(op)->schema->attrDefs) {
		FOREACH_SET(AttributeReference, ar, rightAttrs) {
			if (streq(ad->attrName, ar->name)) {
				rightProjExpr = appendToTailOfList(rightProjExpr, createFullAttrReference(ad->attrName, 0, idx, 0, ad->dataType));
				rightADs = appendToTailOfList(rightADs, copyObject(ad));
			}
		}
		idx++;
	}
	QueryOperator *rChild = (QueryOperator *) copyObject(OP_RCHILD(op));
	ProjectionOperator *rightProj = createProjectionOp(rightProjExpr, rChild, NIL, getAttrDefNames(rightADs));
	rChild->parents = singleton(rightProj);
	DEBUG_NODE_BEATIFY_LOG("right join proj", rightProj);

	// execute query to get all values;
	char *lSQL = serializeQuery((QueryOperator *) leftProj);
	HashMap *leftBFs = NEW_MAP(Constant, Node);
	DEBUG_NODE_BEATIFY_LOG("WHAT IS AD", ((QueryOperator *) leftProj)->schema->attrDefs);
	buildBloomFilterFromSQL(lSQL, ((QueryOperator *) leftProj)->schema->attrDefs, leftBFs);

	char *rSQL = serializeQuery((QueryOperator *) rightProj);
	HashMap *rightBFs = NEW_MAP(Constant, Node);
	DEBUG_NODE_BEATIFY_LOG("WHAT IS AD", ((QueryOperator *) rightProj)->schema->attrDefs);
	buildBloomFilterFromSQL(rSQL, ((QueryOperator *) rightProj)->schema->attrDefs, rightBFs);

	HashMap *stateMap = NEW_MAP(Constant, Node);
	MAP_ADD_STRING_KEY(stateMap, JOIN_LEFT_BLOOM, leftBFs);
	MAP_ADD_STRING_KEY(stateMap, JOIN_RIGHT_BLOOM, rightBFs);
	MAP_ADD_STRING_KEY(stateMap, JOIN_LEFT_BLOOM_ATT_MAPPING, leftMapping);
	MAP_ADD_STRING_KEY(stateMap, JOIN_RIGHT_BLOOM_ATT_MAPPING, rightMapping);
	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_JOIN, stateMap);
	DEBUG_NODE_BEATIFY_LOG("JOIN MAPS BF: ", stateMap);
}

static void
buildBloomFilterFromSQL(char *sql, List *attributeDefs, HashMap *BFMap)
{
	DEBUG_NODE_BEATIFY_LOG("CURR AD", attributeDefs);
	Relation *rel = executeQuery(sql);
	int tupleLen = rel->tuples->length;
	int colLen = LIST_LENGTH(rel->schema);

	for (int col = 0; col < colLen; col++) {
		Bloom *bf = createBloomFilter();
		AttributeDef *ad = (AttributeDef *) getNthOfListP(attributeDefs, col);
		switch (ad->dataType) {
			case DT_INT:
			{
				for (int row = 0; row < tupleLen; row++) {
					char *val = getVecString((Vector *) getVecNode(rel->tuples, row), col);
					if (!streq(val, "NULL")) {
						int value = atoi(val);
						bloom_add(bf, &value, sizeof(int));
					}
				}
			}
			break;
			case DT_LONG:
			{
				for (int row = 0; row < tupleLen; row++) {
					char *val = getVecString((Vector *) getVecNode(rel->tuples, row), col);
					if (!streq(val, "NULL")) {
						gprom_long_t value = atol(val);
						bloom_add(bf, &value, sizeof(gprom_long_t));
					}
				}
			}
			break;
			case DT_BOOL:
			{
				for (int row = 0; row < tupleLen; row++) {
					char *val = getVecString((Vector *) getVecNode(rel->tuples, row), col);
					if (!streq(val, "NULL")) {
						if (streq(val, "TRUE") || streq(val, "t") || streq(val, "true")) {
							int value = 1;
							bloom_add(bf, &value, sizeof(int));
						}
    					if (streq(val, "FALSE") || streq(val, "f") || streq(val, "false")){
							int value = 0;
							bloom_add(bf, &value, sizeof(int));
						}
					}
				}
			}
			break;
			case DT_FLOAT:
			{
				for (int row = 0; row < tupleLen; row++) {
					char *val = getVecString((Vector *) getVecNode(rel->tuples, row), col);
					if (!streq(val, "NULL")) {
						double value = atof(val);
						bloom_add(bf, &value, sizeof(double));
					}
				}
			}
			break;
			case DT_STRING:
			case DT_VARCHAR2:
			{
				for (int row = 0; row < tupleLen; row++) {
					char *val = getVecString((Vector *) getVecNode(rel->tuples, row), col);
					if (!streq(val, "NULL")) {
						bloom_add(bf, val, strlen(val));
					}
				}
			}
			break;
		}
		MAP_ADD_STRING_KEY(BFMap, strdup(ad->attrName), bf);
	}
}

/*
	a = b; Yes
	a = b + 1; No
	a = b + c; No
	a = 2(constant); No
*/
static void
parseJoinConds(Node *conds, Vector *condsVec)
{
	char *opName = ((Operator *) conds)->name;
	if (streq(opName, OPNAME_AND) || streq(opName, OPNAME_OR)) {
		parseJoinConds((Node *) getNthOfListP(((Operator *) conds)->args, 0), condsVec);
		parseJoinConds((Node *) getNthOfListP(((Operator *) conds)->args, 1), condsVec);
		return;
	}

	if (streq(opName, OPNAME_EQ)) {
		FOREACH(Node, n, ((Operator *) conds)->args) {
			if (!isA(n, AttributeReference)) {
				return;
			}
		}
		vecAppendNode(condsVec, conds);
	}
}

static Vector *
buildGroupByValueVecFromRelation(Relation *rel, List *gbList)
{
	Vector *gbValsVec = makeVector(VECTOR_NODE, T_Vector);
	int numTuples = rel->tuples->length;
	if (gbList == NULL) {
		char *dummyGBVal = strdup("##");
		for (int row = 0; row < numTuples; row++) {
			StringInfo gb = NEW(StringInfoData);
			gb->cursor = 0;
			gb->len = 2;
			gb->maxlen = 2;
			gb->data = MALLOC(3);
			memcpy(gb->data, dummyGBVal, 2);
			gb->data[2] = '\0';
			vecAppendNode(gbValsVec, (Node *) gb);
		}
	} else {
		Vector *gbAttrPos = makeVector(VECTOR_INT, T_Vector);
		Vector *gbAttrType = makeVector(VECTOR_INT, T_Vector);
		Vector *gbAttrTypeStringPos = makeVector(VECTOR_INT, T_Vector);

		boolean noStringTypeExists = TRUE;
		size_t totalSizeIfNoStringType = 0;
		FOREACH(AttributeReference, ar, gbList) {
			int pos = listPosString(rel->schema, backendifyIdentifier(ar->name));
			vecAppendInt(gbAttrPos, pos);
			vecAppendInt(gbAttrType, (int) ar->attrType);
			switch (ar->attrType) {
				case DT_BOOL:
				case DT_INT:
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
			for (int row = 0; row < numTuples; row++) {
				char *gb = (char *) MALLOC(totalSizeIfNoStringType + 1);
				Vector * tuple = (Vector *) getVecNode(rel->tuples, row);
				size_t preSize = 0;
				for (int col = 0; col < gbAttrPos->length; col++) {
					int pos = (int) getVecInt(gbAttrPos, col);
					DataType dt = (DataType) getVecInt(gbAttrType, col);
					char *value = (char *) getVecString(tuple, pos);

					switch(dt) {
						case DT_INT:
						{
							int val = atoi(value);
							memcpy(gb + preSize, &val, sizeof(int));
							preSize += sizeof(int);
						}
						break;
						case DT_LONG:
						{
							gprom_long_t val = atol(value);
							memcpy(gb + preSize, &val, sizeof(gprom_long_t));
							preSize += sizeof(gprom_long_t);
						}
						break;
						case DT_FLOAT:
						{
							double val = atof(value);
							memcpy(gb + preSize, &val, sizeof(double));
							preSize += sizeof(double);
						}
						break;
						case DT_BOOL:
						{
							int val = 0;
							if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true")) {
								val = 1;
							}
							memcpy(gb + preSize, &val, sizeof(int));
							preSize += sizeof(int);
						}
						break;
						case DT_VARCHAR2:
						case DT_STRING:
						{

						}
						break;
					}
				}
				gb[preSize] = '\0';
				int flen;
				char *base64Str = (char *) base64(gb, (int) preSize, &flen);
				StringInfo gbv = NEW(StringInfoData);
				gbv->cursor = 0;
				gbv->maxlen = flen;
				gbv->len = flen;
				gbv->data = base64Str;
				vecAppendNode(gbValsVec, (Node *) gbv);
			}
		} else {
			for (int row = 0; row < numTuples; row++) {
				// first check each row size for string type;
				Vector * tuple = (Vector *) getVecNode(rel->tuples, row);

				size_t totSizeWithStringType = totalSizeIfNoStringType;
				for (int strCol = 0; strCol < gbAttrTypeStringPos->length; strCol++) {
					int pos = getVecInt(gbAttrTypeStringPos, strCol);
					char *strVal = (char *) getVecString(tuple, pos);
					totSizeWithStringType += strlen(strVal);
				}

				char *gb = (char *) MALLOC(totSizeWithStringType + 1);
				// memcpy content to StringInfo->data;
				size_t preSize = 0;
				for (int col = 0; col < gbAttrPos->length; col++) {
					int pos = (int) getVecInt(gbAttrPos, col);
					DataType dt = (DataType) getVecInt(gbAttrType, col);
					char *value = (char *) getVecString(tuple, pos);

					switch(dt) {
						case DT_INT:
						{
							int val = atoi(value);
							memcpy(gb + preSize, (char *) &val, sizeof(int));
							preSize += sizeof(int);
						}
						break;
						case DT_LONG:
						{
							gprom_long_t val = atol(value);
							memcpy(gb + preSize, (char *) &val, sizeof(gprom_long_t));
							preSize += sizeof(gprom_long_t);
						}
						break;
						case DT_FLOAT:
						{
							double val = atof(value);
							memcpy(gb + preSize, (char *) &val, sizeof(double));
							preSize += sizeof(double);
						}
						break;
						case DT_BOOL:
						{
							int val = 0;
							if (streq(value, "TRUE") || streq(value, "t") || streq(value, "true")) {
								val = 1;
							}
							memcpy(gb + preSize, (char *) &val, sizeof(int));
							preSize += sizeof(int);
						}
						break;
						case DT_VARCHAR2:
						case DT_STRING:
						{
							size_t lens = strlen(value);
							memcpy(gb + preSize, value, lens);
							preSize += lens;
						}
						break;
					}
				}
				gb[preSize] = '\0';

				int flen;
				char *base64Str = (char *) base64(gb, preSize, &flen);
				StringInfo gbv = NEW(StringInfoData);
				gbv->cursor = 0;
				gbv->len = flen;
				gbv->maxlen = flen;
				gbv->data = base64Str;
				vecAppendNode(gbValsVec, (Node *) gbv);
			}
		}
	}
	// INFO_LOG("gb len %d", gbValsVec->length);

	// HashMap *map = NEW_MAP(Constant, Constant);
	// for (int i = 0; i < gbValsVec->length; i++) {
	// 	StringInfo info = (StringInfo) getVecNode(gbValsVec, i);
	// 	if (MAP_HAS_STRING_KEY(map, info->data)) {
	// 		char *data = info->data;
	// 		for (int i = 0; i < 20; i++) {
	// 			INFO_LOG("pos: %d, char: '%c', ascii: %d", i, data[i], data[i]);
	// 		}
	// 		INFO_LOG("val %d", *((int*) data));
	// 		INFO_LOG("HASH THIS KEY FOR POS %d", i);
	// 		Constant *cnt = (Constant *) MAP_GET_STRING(map, info->data);
	// 		INFO_LOG("previous pos %d", INT_VALUE(cnt));
	// 	} else {

	// 		addToMap(map, (Node *) createConstString(info->data), (Node *) createConstInt(i));
	// 	}
	// }

	// INFO_LOG("WHAT IS MAP SIZE %d", mapSize(map));
	// DEBUG_NODE_BEATIFY_LOG("WHAT IS MAP", map);
	return gbValsVec;
}

static void
buildStateAggregationOp(QueryOperator *op)
{
	AggregationOperator *aggOp = (AggregationOperator *) op;
	DEBUG_NODE_BEATIFY_LOG("CURRENT AGG", aggOp);
	INFO_OP_LOG("CURRENT AGG", aggOp);

	// get aggregation operator args and groupbys;
	List *aggrFCs = (List *) copyObject(aggOp->aggrs);
	List *aggrGBs = (List *) copyObject(aggOp->groupBy);
	boolean noGB = (LIST_LENGTH(aggrGBs) == 0 ? TRUE : FALSE);

	// split this aggOp into two categories: min/max and avg/count/sum;
	List *min_max = NIL;
	List *min_max_attNames = NIL;

	List *avg_sum_count = NIL;
	List *avg_sum_count_attNames = NIL;

	DEBUG_NODE_BEATIFY_LOG("aggrs list", aggrFCs);
	for (int i = 0; i < LIST_LENGTH(aggrFCs); i++) {
		// build count(1) as count_per_group
		if (i == 0) {
			FunctionCall *fc_cnt_per_group = NULL;
			// fc_cnt_per_group = createFunctionCall(COUNT_FUNC_NAME, singleton((AttributeReference *) copyObject(getNthOfListP(aggrGBs, 0))));
			fc_cnt_per_group = createFunctionCall(COUNT_FUNC_NAME, singleton(createConstInt(1)));
			fc_cnt_per_group->isAgg = TRUE;

			// append to agg list;
			avg_sum_count = appendToTailOfList(avg_sum_count, fc_cnt_per_group);
			avg_sum_count_attNames = appendToTailOfList(avg_sum_count_attNames, backendifyIdentifier("count_per_group"));
		}
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
				fc_sum->isAgg = TRUE;
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
	// indicate if building ps info for the operator;
	boolean hasBuildPSMaps = FALSE;
	PSMap *groupPSMap = makePSMap();

	// get min_max rewrite;
	// for min and max function: run capture-rewritted child;
	// insert into the min/max heap based on group by attribute(s);
	if (LIST_LENGTH(min_max) != 0) {
		// get child operator;
		QueryOperator *child = OP_LCHILD(aggOp);

		// get capture rewrite
		QueryOperator *rewrOp = captureRewriteOp(PC_BuildState, (QueryOperator *) copyObject(child));
		DEBUG_NODE_BEATIFY_LOG("REWR OP", rewrOp);
		INFO_OP_LOG("REWR OP", rewrOp);
		// searialize operator;
		char *sql = serializeQuery(rewrOp);
		INFO_LOG("MIN_MAX sql %s", sql);
		// get tuples;
		Relation *resultRel = NULL;
		resultRel = executeQuery(sql);
		INFO_LOG("schema name: %s", stringListToString(resultRel->schema));


		// get prov attr name;
		QueryOperator *auxOP = captureRewriteOp(PC_BuildState, (QueryOperator *) copyObject(aggOp));
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
			} else if (strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
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
			for (int ii = 0; ii < resultRel->tuples->length; ii++) {
				Vector *tuple = (Vector *) getVecNode(resultRel->tuples, ii);
				// INFO_LOG("tuple values: %s", stringListToString(tuple));
				// get group by value of this tuple; TODO: isNULL
				StringInfo gbVals = makeStringInfo();
				if (noGB) {
					appendStringInfo(gbVals, "##");
				} else {
					for (int j = 0; j < LIST_LENGTH(gbAttrPos); j++) {
						appendStringInfo(gbVals, "%s#", (char *) getVecString(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, j))));
						// appendStringInfo(gbVals, "%s#", (char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, j))));
					}
				}

				// check if gbHeaps has this group by value;
				// C1.
				// List *heapList = NIL;
				RBTRoot *heap = NULL;
				if (MAP_HAS_STRING_KEY(gbHeaps->heapLists, gbVals->data)) {
					// C1.
					// heapList = (List *) MAP_GET_STRING(gbHeaps->heapLists, gbVals->data);
					heap = (RBTRoot *) MAP_GET_STRING(gbHeaps->heapLists, gbVals->data);
				} else {
					heap = makeRBT(RBT_MIN_HEAP, FALSE);
				}

				// get min/max attribute value;
				// char *val = (char *) getNthOfListP(tuple, fcAttrPos);
				char *val = getVecString(tuple, fcAttrPos);
				// INFO_LOG("val str %s", val);
				Constant *value = makeValue(attrRef->attrType, val);
				// DEBUG_NODE_BEATIFY_LOG("CONSTANT", value);
				// DEBUG_NODE_BEATIFY_LOG("HEAPLIST", heapList);
				// insert this value to heap and heapify

				// C1;
				// heapList = heapInsert(heapList, STRING_VALUE(gbHeaps->heapType), (Node *) value);
				RBTInsert(heap, (Node *) value, NULL);

				// DEBUG_NODE_BEATIFY_LOG("HEAPLIST", heapList);
				// heap list done, add to map;
				//C1.
				// addToMap(gbHeaps->heapLists, (Node *) createConstString(gbVals->data), (Node *) copyObject(heapList));
				addToMap(gbHeaps->heapLists, (Node *) createConstString(gbVals->data), (Node *) heap);

				// BELOW: DEALING WITH PROVENANCE SKETCHS;

				// get all provenance sketch attrs;
				if (!hasBuildPSMaps) {
					FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
						List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
						int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
						int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));
						int provPos = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 2));
						INFO_LOG("MINMAX PROV LEVEL %d", provLevel);
						HashMap *psMapInGP = NULL;
						if (MAP_HAS_STRING_KEY(groupPSMap->provSketchs, STRING_VALUE(c))) {
							psMapInGP = (HashMap *) MAP_GET_STRING(groupPSMap->provSketchs, STRING_VALUE(c));
						} else {
							psMapInGP = NEW_MAP(Constant, Node);
						}

						HashMap *gbFragCnt = NULL;
						if (MAP_HAS_STRING_KEY(groupPSMap->fragCount, STRING_VALUE(c))) {
							gbFragCnt = (HashMap *) MAP_GET_STRING(groupPSMap->fragCount, STRING_VALUE(c));
						} else {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						HashMap *fragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbFragCnt, gbVals->data)) {
							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVals->data);
						} else {
							fragCnt = NEW_MAP(Constant, Constant);
						}

						char *prov = getVecString(tuple, provPos);
						if (provLevel < 2) {
							int whichFrag = atoi(prov);
							if (MAP_HAS_INT_KEY(fragCnt, whichFrag)) {
								Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, whichFrag);
								incrConst(cnt);
							} else {
								addToMap(fragCnt, (Node *) createConstInt(whichFrag), (Node *) createConstInt(1));
							}
							addToMap(psMapInGP, (Node *) createConstString(gbVals->data), (Node *) createConstInt(whichFrag));
						} else {
							BitSet *gbProvSketch = NULL;
							if (MAP_HAS_STRING_KEY(psMapInGP, gbVals->data)) {
								gbProvSketch = (BitSet *) MAP_GET_STRING(psMapInGP, gbVals->data);
							} else {
								gbProvSketch = newBitSet(provNumFrag);
							}
							for (int bitIndex = 0; bitIndex < provNumFrag; bitIndex++) {
								if (prov[bitIndex] == '1') {
									if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
										Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, bitIndex);
										incrConst(cnt);
									} else {
										addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
										setBit(gbProvSketch, bitIndex, TRUE);
									}
								}
							}

							addToMap(psMapInGP, (Node *) createConstString(gbVals->data), (Node *) gbProvSketch);
						}

						addToMap(gbFragCnt, (Node *) createConstString(gbVals->data), (Node *) fragCnt);
						addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						addToMap(groupPSMap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGP);
						if (!MAP_HAS_STRING_KEY(groupPSMap->isIntSketch, STRING_VALUE(c))) {
							if (provLevel < 2) {
								addToMap(groupPSMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							} else {
								addToMap(groupPSMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							}
							addToMap(groupPSMap->provLens, (Node *) copyObject(c), (Node *) createConstInt(provNumFrag));
						}
					}
				}
			}

			hasBuildPSMaps = TRUE;
			HashMap *dataStructures = NULL;
			if (HAS_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE)) {
				dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE);
			} else {
				dataStructures = NEW_MAP(Constant, Node);
			}
			addToMap(dataStructures, (Node *) heapName, (Node *) gbHeaps);
			addToMap(dataStructures, (Node *) createConstString(PROP_PROV_SKETCH_AGG), (Node *) groupPSMap);
			SET_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE, (Node *) dataStructures);
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
		DEBUG_NODE_BEATIFY_LOG("before avg_um_count", newAgg);
		INFO_OP_LOG("before rewrite op avg_sum_cont: ", newAgg);
		QueryOperator * rewriteOP = captureRewriteOp(PC_BuildState, (QueryOperator *) newAgg);
		DEBUG_NODE_BEATIFY_LOG("avg_sum_count agg", rewriteOP);
		INFO_OP_LOG("rewrite op avg_sum_cont TTTTTTTTTTTTTTTTTTTTT: ", rewriteOP);
		char *sql = serializeQuery(rewriteOP);
		Relation *resultRel = NULL;
		resultRel = executeQuery(sql);
		INFO_LOG("avg sql %s", sql);
		// get the level of aggregation
		HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewriteOP), PROP_LEVEL_AGGREGATION_MARK), 0);
		FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
			int pos = listPosString(resultRel->schema, STRING_VALUE(c));
			List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
			l = appendToTailOfList(l, createConstInt(pos));
		}

		// build group by value vectors version 2;
		Vector * groupByVealusVector = buildGroupByValueVecFromRelation(resultRel, aggrGBs);

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
			DEBUG_NODE_BEATIFY_LOG("what is name and pos", namesAndPoss);
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
			// version 2: do not need this list anymore;
			// List *gbAttrPos = NIL;
			// FOREACH(AttributeReference, ar, aggrGBs) {
			// 	int pos = listPosString(resultRel->schema, backendifyIdentifier(ar->name));
			// 	gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(pos));
			// 	// gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(ar->attrType));
			// }

			// get group count for avg and sum
			int groupCountPos = -1;
			// if (strcmp(fc->functionname, COUNT_FUNC_NAME) != 0) {
			groupCountPos = listPosString(resultRel->schema, backendifyIdentifier("count_per_group"));
			// }
			// DEBUG_NODE_BEATIFY_LOG("gb list", gbAttrPos);
			DEBUG_NODE_BEATIFY_LOG("what is name and pos", namesAndPoss);
			int tupleLen = resultRel->tuples->length;
			for (int j = 0; j < tupleLen; j++) {

				Vector *tuple = (Vector *) getVecNode(resultRel->tuples, j);

				// get the group by values;
				// StringInfo gbVals = makeStringInfo();
				// if (noGB) {
				// 	appendStringInfoString(gbVals, "##");
				// } else {
				// 	for (int k = 0; k < LIST_LENGTH(gbAttrPos); k++) {
				// 		// appendStringInfo(gbVals, "%s#", (char *) getNthOfListP(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, k))));
				// 		// TODO: For float in group by, TPCH-10.
				// 		appendStringInfo(gbVals, "%s#", getVecString(tuple, INT_VALUE((Constant *) getNthOfListP(gbAttrPos, k))));
				// 	}
				// }

				StringInfo gbVals = (StringInfo ) getVecNode(groupByVealusVector, j);

				Vector *l = NULL;
				Vector *newL = makeVector(VECTOR_NODE, T_Vector);

				if (MAP_HAS_STRING_KEY(acs->map, gbVals->data)) {
					l = (Vector *) MAP_GET_STRING(acs->map, gbVals->data);
				} else {
					l = makeVector(VECTOR_NODE, T_Vector);
					if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
						vecAppendNode(l, (Node *) createConstFloat((double) 0));
						vecAppendNode(l, (Node *) createConstFloat((double) 0));
						vecAppendNode(l, (Node *) createConstLong(0));
					} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
						vecAppendNode(l, (Node *) createConstFloat((double) 0));
						vecAppendNode(l, (Node *) createConstLong(0));
					} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
						vecAppendNode(l, (Node *) createConstLong(0));
					}
				}
				DEBUG_NODE_BEATIFY_LOG("what is list", l);
				if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
					// get avg, sum and cont;
					double avg = FLOAT_VALUE((Constant *) getVecNode(l, 0));
					double sum = FLOAT_VALUE((Constant *) getVecNode(l, 1));
					gprom_long_t count = LONG_VALUE((Constant *) getVecNode(l, 2));

					// compute new avg, sum, count;
					int newSumPos = INT_VALUE((Constant *) getNthOfListP(namesAndPoss, 1));
					// double newSum = atof((char *) getNthOfListP(tuple, newSumPos));
					double newSum = atof(getVecString(tuple, newSumPos));
					sum += newSum;
					// count += atoi((char *) getNthOfListP(tuple, groupCountPos));
					count += atol(getVecString(tuple, groupCountPos));
					avg = sum / count;

					// make a new list;

					vecAppendNode(newL, (Node *) createConstFloat(avg));
					vecAppendNode(newL, (Node *) createConstFloat(sum));
					vecAppendNode(newL, (Node *) createConstLong(count));
				} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
					// get previous sum and count;
					double sum = FLOAT_VALUE((Constant *) getVecNode(l, 0));
					gprom_long_t count = LONG_VALUE((Constant *) getVecNode(l, 1));

					// compute new sum and count;
					// TODO: is it necssary to check the type??
					int newSumPos = INT_VALUE((Constant *) getNthOfListP(namesAndPoss, 0));
					// sum += atof((char *) getNthOfListP(tuple, newSumPos));
					sum += atof(getVecString(tuple, newSumPos));
					count += atol(getVecString(tuple, groupCountPos));

					vecAppendNode(newL, (Node *) createConstFloat(sum));
					vecAppendNode(newL, (Node *) createConstLong(count));
				} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
					// previous count;
					gprom_long_t count = INT_VALUE((Constant *) getVecNode(l, 0));

					// get new count;
					count += atol(getVecString(tuple, INT_VALUE((Constant *) getNthOfListP(namesAndPoss, 0))));

					// make a new list;
					// l = singleton(createConstInt(count));
					vecAppendNode(newL, (Node *) createConstLong(count));
				}

				// add this key-value to map;
				addToMap(acs->map, (Node *) createConstString(gbVals->data), (Node *) newL);


				// BELOW DEALING WITH PROVENANCE SKETCH;
				if (!hasBuildPSMaps) {
					FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
						List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
						int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
						int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));
						int provPos = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 2));
						INFO_LOG("avg provlevel %d", provLevel);
						// addToMap(groupPSMap->provLens, (Node *) copyObject(c), (Node *) createConstInt(provNumFrag));
						HashMap *psMapInGP = NULL;
						if (MAP_HAS_STRING_KEY(groupPSMap->provSketchs, STRING_VALUE(c))) {
							psMapInGP = (HashMap *) MAP_GET_STRING(groupPSMap->provSketchs, STRING_VALUE(c));
						} else {
							psMapInGP = NEW_MAP(Constant, Node);
						}

						HashMap *gbFragCnt = NULL;
						if (MAP_HAS_STRING_KEY(groupPSMap->fragCount, STRING_VALUE(c))) {
							gbFragCnt = (HashMap *) MAP_GET_STRING(groupPSMap->fragCount, STRING_VALUE(c));
						} else {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						HashMap *fragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbFragCnt, gbVals->data)) {
							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVals->data);
						} else {
							fragCnt = NEW_MAP(Constant, Constant);
						}

						char *prov = getVecString(tuple, provPos);
						int groupCnt = (int) atol(getVecString(tuple, groupCountPos));
						if (provLevel < 2) {
							int whichFrag = atoi(prov);
							INFO_LOG("WHICH FRAG %d", whichFrag);
							if (MAP_HAS_INT_KEY(fragCnt, whichFrag)) {
								int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, whichFrag));
								oriCnt += groupCnt;
								addToMap(fragCnt, (Node *) createConstInt(whichFrag), (Node *) createConstInt(oriCnt));
							} else {
								addToMap(fragCnt, (Node *) createConstInt(whichFrag), (Node *) createConstInt(groupCnt));
							}
							/*
							IMPORTANT NOTES(IN):
							previous if the provLevel < 2, we store a integer, but this is not sufficient to include all frags even if the ps attr is safe enough because: if we partition on a, and group by b, c, then maybe same group have multiple 'a' values(result from different fragments), so we still need to materialized ps in a bit-vector format.
							*/
							BitSet *gbProvSketch = NULL;
							if (MAP_HAS_STRING_KEY(psMapInGP, gbVals->data)) {
								gbProvSketch = (BitSet *) MAP_GET_STRING(psMapInGP, gbVals->data);
							} else {
								gbProvSketch = newBitSet(provNumFrag);
								addToMap(psMapInGP, (Node *) createConstString(gbVals->data), (Node *) gbProvSketch);
							}
							setBit(gbProvSketch, whichFrag, TRUE);
						} else {
							BitSet *gbProvSketch = NULL;
							if (MAP_HAS_STRING_KEY(psMapInGP, gbVals->data)) {
								gbProvSketch = (BitSet *) MAP_GET_STRING(psMapInGP, gbVals->data);
							} else {
								gbProvSketch = newBitSet(provNumFrag);
								addToMap(psMapInGP, (Node *) createConstString(gbVals->data), (Node *) gbProvSketch);
							}

							for (int bitIndex = 0; bitIndex < provNumFrag; bitIndex++) {
								if (prov[bitIndex] == '1') {
									if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
										int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
										oriCnt += groupCnt;
										addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt));
									}  else {
										addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(groupCnt));
										setBit(gbProvSketch, bitIndex, TRUE);
									}
								}
							}

							// addToMap(psMapInGP, (Node *) createConstString(gbVals->data), (Node *) gbProvSketch);
						}

						// build fields for PSMap of this
						addToMap(gbFragCnt, (Node *) createConstString(gbVals->data), (Node *) fragCnt);
						addToMap(groupPSMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						// addToMap(psMapInACS, (Node *) createConstString(gbVals->data), (Node *) gbProvSketch);
						addToMap(groupPSMap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGP);
						if (!MAP_HAS_STRING_KEY(groupPSMap->isIntSketch, STRING_VALUE(c))) {
							// if (provLevel < 2) {
								// addToMap(groupPSMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
							// } else {
								// addToMap(groupPSMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							// }
							addToMap(groupPSMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
							addToMap(groupPSMap->provLens, (Node *) copyObject(c), (Node *) createConstInt(provNumFrag));
						}

					}
					// hasBuildPSMaps = TRUE;
				}
			}

			HashMap *dataStructures = NULL;
			if (HAS_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE)) {
				dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE);
			} else {
				dataStructures = NEW_MAP(Constant, Node);
			}

			addToMap(dataStructures, (Node *) ACSsName, (Node *) acs);
			addToMap(dataStructures, (Node *) createConstString(PROP_PROV_SKETCH_AGG), (Node *) groupPSMap);
			SET_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE, (Node *) dataStructures);

			hasBuildPSMaps = TRUE;
		}
	}

	DEBUG_NODE_BEATIFY_LOG("AGG STATE DATA STRUCTURE", (HashMap *) GET_STRING_PROP((QueryOperator *) aggOp, PROP_DATA_STRUCTURE_STATE));
}

static void
buildStateDuplicateRemovalOp(QueryOperator *op)
{
    // for duplicate remove: rewrite like group by no agg
	DuplicateRemoval *dupRem = (DuplicateRemoval *) op;

	// create gb list: attributes are all from distinct attribute list;
	List *gbList = NIL;
	int pos = 0;
	FOREACH(AttributeDef, ad, op->schema->attrDefs) {
		AttributeReference *ar = createFullAttrReference(ad->attrName, 0, pos, 0, ad->dataType);
		gbList = appendToTailOfList(gbList, ar);
		pos++;
	}

	List *aggrList = singleton(createFunctionCall(COUNT_FUNC_NAME, singleton(createConstInt(1))));
	List *attrNames = (List *) getAttrNames(dupRem->op.schema);
	appendToHeadOfList(attrNames, backendifyIdentifier("count_per_group"));

	// create a new aggregation operator with group-by and fc list;
	AggregationOperator *aggOp = createAggregationOp(aggrList, gbList, (QueryOperator *) copyObject(OP_LCHILD(op)), NIL, attrNames);
	SET_STRING_PROP((QueryOperator *) aggOp, PROP_COARSE_GRAINED_AGGREGATION_MARK_UPDATE_PS, createConstBool(TRUE));

	QueryOperator *child = (QueryOperator *) copyObject(op);
	child->parents = singleton(aggOp);

	DEBUG_NODE_BEATIFY_LOG("BEFORE DUP REWRITE", aggOp);
	QueryOperator *rewrOp = captureRewriteOp(PC_BuildState, (QueryOperator *) aggOp);

	if (LIST_LENGTH(rewrOp->provAttrs) == 0) {
		return;
	}
	DEBUG_NODE_BEATIFY_LOG("DUPLICATE REWRITE", rewrOp);

	char *sql = serializeQuery(rewrOp);
	INFO_LOG("DUPLICATE SQL: %s", sql);

	Relation *resultRel = executeQuery(sql);

	int tupleLen = resultRel->tuples->length;

	if (tupleLen < 1) {
		return;
	}


	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(OP_LCHILD(rewrOp), PROP_LEVEL_AGGREGATION_MARK), 0);
	boolean isFoundPS = TRUE;
	FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
		int pos = listPosString(resultRel->schema, STRING_VALUE(c));
		if (pos == -1) {
			isFoundPS = FALSE;
			break;
		}
		List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
		l = appendToTailOfList(l, createConstInt(pos));
	}

	HashMap *dataStructures = NEW_MAP(Constant, Node);
	// make data structure;
	GBACSs *acs = makeGBACSs();
	Vector *gbAttrPos = makeVector(VECTOR_INT, T_Vector);
	int gbAttrCnt = 0;
	FOREACH(AttributeReference, ar, gbList) {
		pos = listPosString(resultRel->schema, ar->name);
		// gbAttrPos = appendToTailOfList(gbAttrPos, createConstInt(pos));
		vecAppendInt(gbAttrPos, pos);
		gbAttrCnt++;
	}

	int	groupCountPos = listPosString(resultRel->schema, backendifyIdentifier("count_per_group"));

	// deal with tuples
	// Vector *gbValsVec = makeVector(VECTOR_STRING, T_Vector);
	Vector *gbCntVec = makeVector(VECTOR_LONG, T_Vector);
	Vector *gbValsVec = buildGroupByValueVecFromRelation(resultRel, gbList);
	for (int row = 0; row < tupleLen; row++) {
		Vector *tuple = (Vector *) getVecNode(resultRel->tuples, row);

		gprom_long_t groupCnt = atol(getVecString(tuple, groupCountPos));
		vecAppendLong(gbCntVec, groupCnt);

		Vector *cntV = NULL;
		char *gbval = getVecString(gbValsVec, row);
		if (MAP_HAS_STRING_KEY(acs->map, gbval)) {
			cntV = (Vector *) MAP_GET_STRING(acs->map, gbval);
			Constant *cnt = (Constant *) getVecNode(cntV, 0);
			LONG_VALUE(cnt) = LONG_VALUE(cnt) + groupCnt;
		} else {
			cntV = makeVector(VECTOR_NODE, T_Vector);
			vecAppendNode(cntV, (Node *) createConstLong(groupCnt));
			addToMap(acs->map, (Node *) createConstString(gbval), (Node *) cntV);
		}
	}
	addToMap(dataStructures, (Node *) createConstString(PROP_DATA_STRUCTURE_DUP_DATA), (Node *) acs);

	// deal with ps;
	if (isFoundPS) {
		PSMap *psMap = makePSMap();
		char **gbVals = (char **) VEC_TO_ARR(gbValsVec, char);
		gprom_long_t *gbCnts = (gprom_long_t *) VEC_TO_LA(gbCntVec);

		FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
			List *prov_level_num_pos = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
			int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 0));
			int provNumFrag = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 1));
			int provPos = INT_VALUE((Constant *) getNthOfListP(prov_level_num_pos, 2));

			addToMap(psMap->provLens, (Node *) createConstString(STRING_VALUE(c)), (Node*) createConstInt(provNumFrag));
			// if (provLevel < 2) {
				// addToMap(psMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
			// } else {
			addToMap(psMap->isIntSketch, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
			// }

			HashMap *gbFragCnt = NULL;
			gbFragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
			if (gbFragCnt == NULL) {
				gbFragCnt = NEW_MAP(Constant, Node);
				addToMap(psMap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
			}

			HashMap *psMapInGP = NULL;
			psMapInGP = (HashMap *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
			if (psMapInGP == NULL) {
				psMapInGP = NEW_MAP(Constant, Node);
				addToMap(psMap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGP);
			}

			for (int row = 0; row < tupleLen; row++) {
				char *psVal = (char *) getVecString((Vector *) getVecNode(resultRel->tuples, row), provPos);
				HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbVals[row]);
				if (fragCnt == NULL) {
					fragCnt = NEW_MAP(Constant, Constant);
					addToMap(gbFragCnt, (Node *) createConstString(gbVals[row]), (Node *) fragCnt);
				}
				if (provLevel < 2) {
					int whichFrag = atoi(psVal);
					int cnt = (int) gbCnts[row];
					addToMap(fragCnt, (Node *) createConstInt(whichFrag), (Node *) createConstInt(cnt));
					BitSet *gbProvSketch = NULL;

					if (MAP_HAS_STRING_KEY(psMapInGP, gbVals[row])) {
						gbProvSketch = (BitSet *) MAP_GET_STRING(psMapInGP, gbVals[row]);
					} else {
						gbProvSketch = newBitSet(provNumFrag);
						addToMap(psMapInGP, (Node *) createConstString(gbVals[row]), (Node *) gbProvSketch);
					}
					setBit(gbProvSketch, whichFrag, TRUE);
				} else {
					// BitSet *gbProvSketch = (BitSet *) stringToBitset(psVal);
					BitSet *gbProvSketch = NULL;
					if (MAP_HAS_STRING_KEY(psMapInGP, gbVals[row])) {
						gbProvSketch = (BitSet *) MAP_GET_STRING(psMapInGP, gbVals[row]);
					} else {
						gbProvSketch = newBitSet(provNumFrag);
						addToMap(psMapInGP, (Node *) createConstString(gbVals[row]), (Node *) gbProvSketch);
					}
					int bitLen = strlen(psVal);
					for (int bitIdx = 0; bitIdx < bitLen; bitIdx++) {
						if (psVal[bitIdx] == '1') {
							if (MAP_HAS_INT_KEY(fragCnt, bitIdx)) {
								Constant *ori = (Constant *) MAP_GET_INT(fragCnt, bitIdx);
								INT_VALUE(ori) = INT_VALUE(ori) + (int) gbCnts[row];
							} else {
								addToMap(fragCnt, (Node *) createConstInt(bitIdx), (Node *) createConstInt((int) gbCnts[row]));
								setBit(gbProvSketch, bitIdx, TRUE);
							}
						}
					}
					// addToMap(psMapInGP, (Node *) createConstString(gbVals[row]), (Node *) gbProvSketch);
				}
			}
		}
		addToMap(dataStructures, (Node *) createConstString(PROP_PROV_SKETCH_DUP), (Node *) psMap);
	}

	DEBUG_NODE_BEATIFY_LOG("acs duplicate", acs);
	DEBUG_NODE_BEATIFY_LOG("duplicate ds", dataStructures);
	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, (Node *) dataStructures);
}

static void
buildStateOrderOp(QueryOperator *op)
{
	// if a query only has order by, no limit operator, we ca just skip this order by;
	// because only order by, it just sort all the tuples from the lower level;
	// if (!isA(getHeadOfListP(op->parents), LimitOperator)) {
	// 	return;
	// }
	boolean hasLimitAbove = hasLimitOpAbove(op);
	if (!hasLimitAbove) {
		return;
	}
	DEBUG_NODE_BEATIFY_LOG("build state order op", op);
	int orderBySafeNum = getIntOption(OPTION_UPDATE_PS_ORDER_SAFE_NUM);
	INFO_LOG("WHAT IS SAFE NUM IN ORDER BY %d", orderBySafeNum);

	INFO_OP_LOG("order by operator ", op);
	QueryOperator *lchild = (QueryOperator *) copyObject(OP_LCHILD(op));
	// QueryOperator *rewrOp = captureRewriteOp(PC_BuildState, lchild);
	QueryOperator *rewrOp = NULL;
	if (orderBySafeNum == 0) {
		rewrOp = captureRewriteOp(PC_BuildState, lchild);
	} else {
		LimitOperator *limParent = (LimitOperator *) copyObject(getNthOfListP(op->parents, 0));
		limParent->limitExpr = (Node *) createConstInt(orderBySafeNum);
		rewrOp = captureRewriteOp(PC_BuildState, (QueryOperator *) limParent);
	}
	// QueryOperator *rewrOp = captureRewriteOp(PC_BuildState, (QueryOperator *) copyObject(op));

	DEBUG_NODE_BEATIFY_LOG("rewrite order by", rewrOp);
	INFO_OP_LOG("rewrite order by operator ", rewrOp);

	char *sql = serializeQuery(rewrOp);
	INFO_LOG("order by sql %s", sql);
	Relation *resultRel = executeQuery(sql);

	// build rbt;
	RBTRoot *orderByRBT = makeRBT(RBT_ORDER_BY, TRUE);
	addToMap(orderByRBT->metadata, (Node *) createConstString(ORDER_BY_ATTR_NUMS), (Node *) createConstInt(LIST_LENGTH(((OrderOperator *) op)->orderExprs)));
	Vector *orderByASCs = makeVector(VECTOR_INT, T_Vector);
	HashMap *indexToDT = NEW_MAP(Constant, Constant);

	// get attr datatype;
	int attIdx = 0;
	FOREACH(AttributeDef, ad, OP_LCHILD(op)->schema->attrDefs) {
		addToMap(indexToDT, (Node *) createConstInt(attIdx++), (Node *) createConstInt(ad->dataType));
	}
	Set *orderAttrIndexSet = INTSET();
	Set *orderAttrNameSet = STRSET();
	FOREACH(OrderExpr, oe, ((OrderOperator *) op)->orderExprs) {
		if (oe->order == SORT_ASC) {
			vecAppendInt(orderByASCs, 1);
		} else {
			vecAppendInt(orderByASCs, -1);
		}

		int pos = listPosString(resultRel->schema, ((AttributeReference *) oe->expr)->name);
		addIntToSet(orderAttrIndexSet, pos);
		addToSet(orderAttrNameSet, strdup(((AttributeReference *) oe->expr)->name));
	}

	addToMap(orderByRBT->metadata, (Node *) createConstString(ORDER_BY_ASCS), (Node *) orderByASCs);
	addToMap(orderByRBT->metadata, (Node *) createConstString(ORDER_BY_ATTRS), (Node *) orderAttrNameSet);
	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(rewrOp, PROP_LEVEL_AGGREGATION_MARK), 0);

	HashMap *isPSInt = NEW_MAP(Constant, Constant);
	HashMap *PSLens = NEW_MAP(Constant, Constant);
	FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
		List *provs = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
		int provLevel = INT_VALUE((Constant *) getNthOfListP(provs, 0));
		int provLen = INT_VALUE((Constant *) getNthOfListP(provs, 1));
		if (provLevel < 1) {
			addToMap(isPSInt, (Node *) copyObject(c), (Node *) createConstBool(TRUE));
		} else {
			addToMap(isPSInt, (Node *) copyObject(c), (Node *) createConstBool(FALSE));
		}
		addToMap(PSLens, (Node *) copyObject(c), (Node *) createConstInt(provLen));
	}
	addToMap(orderByRBT->metadata, (Node *) createConstString(ORDER_BY_IS_PS_INT), (Node *) isPSInt);
	addToMap(orderByRBT->metadata, (Node *) createConstString(ORDER_BY_PS_LENS), (Node *) PSLens);

	HashMap *psAttrIndex = NEW_MAP(Constant, Constant);
	HashMap *psAttrLevel = NEW_MAP(Constant, Constant);
	for (int index = 0; index < LIST_LENGTH(resultRel->schema); index++) {
		char *attr = (char *) getNthOfListP(resultRel->schema, index);
		if (MAP_HAS_STRING_KEY(psAttrAndLevel, attr)) {
			MAP_ADD_INT_KEY(psAttrIndex, index, createConstString(strdup(attr)));
			List *prov_level_num = (List *) MAP_GET_STRING(psAttrAndLevel, attr);
			int provLevel = INT_VALUE((Constant *) getNthOfListP(prov_level_num, 0));
			MAP_ADD_STRING_KEY(psAttrLevel, strdup(attr), createConstInt(provLevel));
		}
	}

	// dealing with tuples;
	int tupleLens = resultRel->tuples->length;
	int attrLens = LIST_LENGTH(resultRel->schema);
	Vector *allTuples = makeVector(VECTOR_NODE, T_Vector);
	INFO_LOG("ORDER TUPLES: %d", tupleLens);
	for (int row = 0; row < tupleLens; row++) {
		HashMap *tuple = NEW_MAP(Constant, Node);
		// 0: key, 1: val, 2: ps
		addToMap(tuple, (Node *) createConstInt(0), (Node *) makeVector(VECTOR_NODE, T_Vector));
		addToMap(tuple, (Node *) createConstInt(1), (Node *) makeVector(VECTOR_NODE, T_Vector));
		addToMap(tuple, (Node *) createConstInt(2), (Node *) NEW_MAP(Constant, Node));
		vecAppendNode(allTuples, (Node *) tuple);
	}
	for (int col = 0; col < attrLens; col++) {
		if (MAP_HAS_INT_KEY(psAttrIndex, col)) {
			Constant *psName = (Constant *) MAP_GET_INT(psAttrIndex, col);
			int level = INT_VALUE(MAP_GET_STRING(psAttrLevel, STRING_VALUE(psName)));
			INFO_LOG("ORDER BY LEVEL %d", level);
			// TODO: NOTE: except agg other operator, if prov level is > 0 use setbit ->"x10"
			if (level < 1) {
				for (int row = 0; row < tupleLens; row++) {
					int ps = atoi(getVecString((Vector *) getVecNode(resultRel->tuples, row), col));
					HashMap *tuple = (HashMap *) getVecNode(allTuples, row);
					addToMap((HashMap *) MAP_GET_INT(tuple, 2), (Node *) psName, (Node *) createConstInt(ps));
				}
			} else {
				for (int row = 0; row < tupleLens; row++) {
					BitSet *ps = stringToBitset(getVecString((Vector *) getVecNode(resultRel->tuples, row), col));
					HashMap *tuple = (HashMap *) getVecNode(allTuples, row);
					addToMap((HashMap *) MAP_GET_INT(tuple, 2), (Node *) psName, (Node *) ps);
				}
			}
			continue;
		}

		DataType dt = INT_VALUE((Constant *) MAP_GET_INT(indexToDT, col));
		// check if this attr is an order by attr;
		if (hasSetIntElem(orderAttrIndexSet, col)) {
			for (int row = 0; row < tupleLens; row++) {
				Constant *c = makeValue(dt, getVecString((Vector *) getVecNode(resultRel->tuples, row), col));
				HashMap *tuple = (HashMap *) getVecNode(allTuples, row);
				// append to key;
				vecAppendNode((Vector *) MAP_GET_INT(tuple, 0), (Node *) c);
				// append to val;
				vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
			}
			continue;
		}

		// all other attr: non-ps, non-orderby attributes;
		for (int row = 0; row < tupleLens; row++) {
			Constant *c = makeValue(dt, getVecString((Vector *) getVecNode(resultRel->tuples, row), col));
			HashMap *tuple = (HashMap *) getVecNode(allTuples, row);
			vecAppendNode((Vector *) MAP_GET_INT(tuple, 1), (Node *) c);
		}
	}

	// insert all tuples into rb tree;

	for (int row = 0; row < tupleLens; row++) {
		HashMap *tuple = (HashMap *) getVecNode(allTuples, row);
		Vector *key = (Vector *) MAP_GET_INT(tuple, 0);
		Vector *val = (Vector *) MAP_GET_INT(tuple, 1);
		HashMap *ps = (HashMap *) MAP_GET_INT(tuple, 2);
		vecAppendNode(val, (Node *) ps);
		// DEBUG_NODE_BEATIFY_LOG("KEY INSERT", key);
		// DEBUG_NODE_BEATIFY_LOG("VAL INSERT", val);

		RBTInsert(orderByRBT, (Node *) key, (Node *) val);
	}

	HashMap *dataStructures = NEW_MAP(Constant, Node);
	addToMap(dataStructures, (Node *) createConstString(PROP_DATA_STRUCTURE_ORDER_BY), (Node *) orderByRBT);

	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, dataStructures);
	// show RBT
	// DEBUG_NODE_BEATIFY_LOG("RBTROOT", orderbyRBT);
	// Vector *tree = RBTInorderTraverse(orderByRBT);
	// INFO_LOG("ele size %d", tree->length);
	// DEBUG_NODE_BEATIFY_LOG("RBT ", tree);
}

static void
buildStateLimitOp(QueryOperator *op)
{
    /* support ONLY for ORDER BY - LIMIT */
	/* only LIMIT without ORDER BY is not supported */
	// DEBUG_NODE_BEATIFY_LOG("limit Op", op);
	QueryOperator *lchild = (QueryOperator *) copyObject(OP_LCHILD(op));
	/* if lchild is not a OrderOperator, not support currently */
	if(!isA(lchild, OrderOperator)) {
		return;
	}

	/*
	QueryOperator *rewrOp = captureRewriteOp(PC_BuildState, lchild);

	char *sql = serializeQuery(rewrOp);

	// get data;
	Relation *resultRel = executeQuery(sql);

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

	for (int row = 0; row < resultRel->tuples->length; row++) {
		Vector *tuple = (Vector *) getVecNode(resultRel->tuples, row);
		List *val = NIL;
		for (int col = 0; col < fields; col++) {
			DataType dt = ((AttributeDef *) getNthOfListP(attrDefs, col))->dataType;
			if (row == 0) {
				addToMap(limitC->attrToPos, (Node *) createConstString((char *) getNthOfListP(resultRel->schema, col)), (Node *) createConstInt(col));
				addToMap(limitC->posToDatatype, (Node *) createConstInt(col), (Node *) createConstInt(dt));
			}
			Constant *c = makeValue(dt, getVecString(tuple, col));
			val = appendToTailOfList(val, c);
		}

		// prove sketch;
		for (int col = fields; col < LIST_LENGTH(resultRel->schema); col++) {

			char *provName = (char *) getNthOfListP(resultRel->schema, col);
			char *prov = getVecString(tuple, col);

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
	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, limitC);
	*/
}

static void
buildStateFinalOp(QueryOperator *op)
{
	/* this is the final state keep a map of fragNo -> count
		and in this step, all ps are bit vector. not int anymore.

	*/
	INFO_LOG("ALL PS");
	QueryOperator *child = (QueryOperator *) copyObject(OP_LCHILD(op));
	INFO_OP_LOG("final op", child);
	QueryOperator *rewrOp = captureRewriteOp(PC_BuildState, child);
	DEBUG_NODE_BEATIFY_LOG("rewrite final", rewrOp);
	INFO_OP_LOG("rewrite final", rewrOp);
	// boolean psAuxIsAgg = isA(OP_LCHILD(op), AggregationOperator);
	char *sql = serializeQuery(rewrOp);
	INFO_LOG("FINAL SQL %s", sql);
	/* get Relation */
	Relation *resultRel = NULL;
	resultRel = executeQuery(sql);
	PSMap *psMap = makePSMap();

	HashMap *psAttrAndLevel = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(rewrOp, PROP_LEVEL_AGGREGATION_MARK), 0);
	// DEBUG_NODE_BEATIFY_LOG("ps and attr level", psAttrAndLevel);
	FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
		int pos = listPosString(resultRel->schema, STRING_VALUE(c));
		List * l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
		l = appendToTailOfList(l, createConstInt(pos));
	}

	int len = resultRel->tuples->length;
	FOREACH_HASH_KEY(Constant, c, psAttrAndLevel) {
		int pos = listPosString(resultRel->schema, STRING_VALUE(c));
		List *l = (List *) MAP_GET_STRING(psAttrAndLevel, STRING_VALUE(c));
		int level = INT_VALUE((Constant *) getNthOfListP(l, 0));
		int fragNo = INT_VALUE((Constant *) getNthOfListP(l, 1));
		// INFO_LOG("LEVEL IN LAST LEVEL %d", level);

		BitSet *bitset = NULL;
		if (MAP_HAS_STRING_KEY(psMap->provSketchs, STRING_VALUE(c))) {
			bitset = (BitSet *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
		} else {
			bitset = newBitSet(fragNo);
		}

		HashMap *fragCnt = NULL;
		if (MAP_HAS_STRING_KEY(psMap->fragCount, STRING_VALUE(c))) {
			fragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
		} else {
			fragCnt = NEW_MAP(Constant, Constant);
		}

		for (int i = 0; i < len; i++) {
			Vector *tuple = (Vector *) getVecNode(resultRel->tuples, i);
			char *prov = getVecString(tuple, pos);
			INFO_LOG("PS STR %s", prov);
			if (level < 1) {
				int whichFrag = atoi(prov);
				if (MAP_HAS_INT_KEY(fragCnt, whichFrag)) {
					Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, whichFrag);
					incrConst(cnt);
				} else {
					addToMap(fragCnt, (Node *) createConstInt(whichFrag), (Node *) createConstInt(1));
				}
			} else {
				// noneed to check, it is a bitvector string;
				for (int bitIndex = 0; bitIndex < strlen(prov); bitIndex++) {
					if (prov[bitIndex] == '1') {
						if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
							int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
							addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt + 1));
						} else {
							addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
						}
						// setBit(bitset, bitIndex, TRUE);
					}
				}
			}
		}
		FOREACH_HASH_KEY(Constant, c, fragCnt) {
			setBit(bitset, INT_VALUE(c), TRUE);
		}
        MAP_ADD_STRING_KEY(psMap->provSketchs, STRING_VALUE(c), copyObject(bitset));
        MAP_ADD_STRING_KEY(psMap->fragCount, STRING_VALUE(c), fragCnt);
		MAP_ADD_STRING_KEY(psMap->provLens, STRING_VALUE(c), createConstInt(fragNo));
		// addToMap(psMap->provSketchs, (Node *) copyObject(c), (Node *) copyObject(bitset));
		// addToMap(psMap->fragCount, (Node *) copyObject(c), (Node *) fragCnt);
	}
	SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, psMap);
}

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

int
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

QueryOperator *
captureRewriteOp(ProvenanceComputation *pc, QueryOperator *op)
{
	QueryOperator* result = NULL;
	Node *coarsePara = NULL;
	psInfo *psPara = NULL;

	ProvenanceComputation *newPC = (ProvenanceComputation *) copyObject(pc);
	newPC->op.inputs = singleton(op);
	op->parents = singleton(newPC);

	// get psInfo;
	coarsePara = (Node*) getStringProperty((QueryOperator*) newPC, PROP_PC_COARSE_GRAINED);
	psPara = createPSInfo(coarsePara);

	// mark table;
	markTableAccessAndAggregation((QueryOperator*) newPC, (Node*) psPara);

	// mark the number of table - used in provenance scratch;
	markNumOfTableAccess((QueryOperator*) newPC);

	// bottom up propagate;
	bottomUpPropagateLevelAggregation((QueryOperator*) newPC, psPara);

	// rewrite query;
	result = rewritePI_CS(newPC);

	return result;
}

Constant *
makeValue(DataType dataType, char* value)
{

	Constant* c = NULL;
	switch (dataType) {
		case DT_INT:
			c = createConstInt(atoi(value));
			break;
		case DT_LONG:
			c = createConstLong((gprom_long_t) atol(value));
			break;
		case DT_STRING:
			c = createConstString(value);
			break;
		case DT_FLOAT:
		{
			double d = atof((const char *) value);
			c = createConstFloat(d);
		}
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

GBHeaps *
makeGBHeaps()
{
	GBHeaps *gbHeaps = makeNode(GBHeaps);

	gbHeaps->fragCount = NEW_MAP(Constant, Node);
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

PSMap *
makePSMap()
{
	PSMap *map = makeNode(PSMap);

	map->fragCount = NEW_MAP(Constant, Node);
	map->provSketchs = NEW_MAP(Constant, Node);
	map->isIntSketch = NEW_MAP(Constant, Constant);
	map->provLens = NEW_MAP(Constant, Constant);

	return map;
}
