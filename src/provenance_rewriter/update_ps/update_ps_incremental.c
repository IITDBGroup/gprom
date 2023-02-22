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

static DataChunk *getDataChunkFromUpdateStatement(QueryOperator* op, TableAccessOperator *tableAccessOp);
static void getDataChunkOfInsert(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfDelete(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static void getDataChunkOfUpdate(QueryOperator* updateOp, DataChunk* dataChunk, TableAccessOperator *tableAccessOp, psAttrInfo *attrInfo);
static Relation *getQueryResult(char* sql);
// static Constant *makeValue(DataType dataType, char* value);
static void executeQueryWithoutResult(char* sql);
static DataChunk *filterDataChunk(DataChunk* dataChunk, Node* condition);
static QueryOperator *captureRewrite(QueryOperator *operator);
// static int compareTwoValues(Constant *a, Constant *b, DataType dt);
// static void swapListCell(List *list, int pos1, int pos2);
static BitSet *setFragmentToBitSet(int value, List *rangeList);
// static ConstRelOperator *getConstRelOpFromDataChunk(DataChunk *dataChunk);
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

	DataChunk *dataChunk = NULL;
	dataChunk = (DataChunk *) getStringProperty(child, PROP_DATA_CHUNK);

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

	setStringProperty(op, PROP_DATA_CHUNK, (Node*) resultDC);

	// remove child data chunk;
	removeStringProperty(child, PROP_DATA_CHUNK);
}

static void
updateSelection(QueryOperator* op)
{
	updateByOperators(OP_LCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT SELECTION OPERATOR", op);

	// check if child operator has delta tuples;
	QueryOperator *child = OP_LCHILD(op);

	// TODO: another property to identify if it is only capture delta data;
	if (!HAS_STRING_PROP(child, PROP_DATA_CHUNK)) {
		return;
	}

	appendStringInfo(strInfo, "%s ", "UpdateSelection");
	DataChunk * dataChunk = (DataChunk *) getStringProperty(child, PROP_DATA_CHUNK);

	Node * selCond = ((SelectionOperator *) op)->cond;

	DataChunk* selDataChunk = filterDataChunk(dataChunk, selCond);

	if (selDataChunk->numTuples == 0) {
		removeStringProperty(child, PROP_DATA_CHUNK);
		return;
	}

	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR SELECTION OPERATOR: ", selDataChunk);
	setStringProperty(op, PROP_DATA_CHUNK, (Node*) selDataChunk);

	// remove child's data chunk;
	removeStringProperty(child, PROP_DATA_CHUNK);
}

// TODO: IN ORDER NOT TO WRITE BACK DATA TO DB, JUST CREATE A CONSTRELOPERATOR WITH A LIST OF INPUT DATA
static void
updateJoin(QueryOperator* op)
{
	INFO_OP_LOG("CURRENT JOIN", op);
	// update two children first;
	updateByOperators(OP_LCHILD(op));
	updateByOperators(OP_RCHILD(op));

	DEBUG_NODE_BEATIFY_LOG("CURRENT JOIN OPERATOR", op);

	QueryOperator *lChild = OP_LCHILD(op);
	QueryOperator *rChild = OP_RCHILD(op);

	if ((!HAS_STRING_PROP(lChild, PROP_DATA_CHUNK))
	 && (!HAS_STRING_PROP(rChild, PROP_DATA_CHUNK))) {
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
	if (HAS_STRING_PROP(lChild, PROP_DATA_CHUNK)) {
		// increase delta branch of join;
		branchWithDeltaCnt++;

		DataChunk *dataChunk = (DataChunk *) getStringProperty(lChild, PROP_DATA_CHUNK);
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
	if (HAS_STRING_PROP(rChild, PROP_DATA_CHUNK)) {
		// increase delta branch of join;
		branchWithDeltaCnt++;

		DataChunk *dataChunk = (DataChunk *) getStringProperty(lChild, PROP_DATA_CHUNK);
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
		DataChunk *lDC = (DataChunk *) getStringProperty(lChild, PROP_DATA_CHUNK);
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
		DataChunk *rDC = (DataChunk *) getStringProperty(rChild, PROP_DATA_CHUNK);
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

	setStringProperty(op, PROP_DATA_CHUNK, (Node *) resultDC);
	DEBUG_NODE_BEATIFY_LOG("DATACHUNK BUILT FOR JOIN OPERATOR");

	// remove children's data chunk;
	removeStringProperty(lChild, PROP_DATA_CHUNK);
	removeStringProperty(rChild, PROP_DATA_CHUNK);
}

static void
updateAggregation(QueryOperator *op)
{
	updateByOperators(OP_LCHILD(op));

	// check if child has data chunk;
	QueryOperator *lChild = OP_LCHILD(op);
	if (!HAS_STRING_PROP(lChild, PROP_DATA_CHUNK)) {
		return;
	}

	DataChunk *dataChunk = (DataChunk *) GET_STRING_PROP(lChild, PROP_DATA_CHUNK);

	// init result data chunk;
	DataChunk *resultDC = initDataChunk();
	resultDC->attrNames = (List *) copyObject(op->schema->attrDefs);
	resultDC->tupleFields = LIST_LENGTH(resultDC->attrNames);
	// set fields for result data chunk;
	int pos = 0;
	FOREACH (AttributeDef, ad, resultDC->attrNames) {
		addToMap(resultDC->attriToPos, (Node *) createConstString(ad->attrName), (Node *) createConstInt(pos));
		addToMap(resultDC->posToDatatype, (Node *) createConstInt(pos), (Node *) createConstInt(ad->dataType));
		pos++;
		vecAppendNode(resultDC->tuples, (Node *) makeVector(VECTOR_NODE, T_Vector));
	}

	// get group by and agg function list;
	List *aggGBList = (List *) copyObject(((AggregationOperator *) op)->groupBy);
	List *aggFCList = (List *) copyObject(((AggregationOperator *) op)->aggrs);

	// match function and group by names to schema names;
	// 	since in GProM rewrite min(a) as min_a, sum(b) as sum_b group by c, d -> in aggop schema (aggr_0, aggr_1, group_0, group_1)
	//	and in upper level, it will translate aggr_0, aggr_1, group_0, group_1 to min_a, sum_b, c, d;
	HashMap *mapFCsToSchemas = NEW_MAP(Constant, Constant);
	pos = 0;
	FOREACH (FunctionCall, fc, aggFCList) {
		AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
		Constant *nameInFC = createConstString(CONCAT_STRINGS(strdup(fc->functionname), "_", strdup(ar->name)));
		Constant *nameInSchema = createConstString(((AttributeDef *) getNthOfListP(resultDC->attrNames, pos))->attrName);
		addToMap(mapFCsToSchemas, (Node *) nameInFC, (Node *) nameInSchema);
		pos++;
	}
	FOREACH (AttributeReference, ar, aggGBList) {
		Constant *nameInFC = createConstString(CONCAT_STRINGS(strdup(ar->name)));
		Constant *nameInSchema = createConstString(((AttributeDef *) getNthOfListP(resultDC->attrNames, pos))->attrName);
		addToMap(mapFCsToSchemas, (Node *) nameInFC, (Node *) nameInSchema);
		pos++;
	}

	// get stored data structure state;
	HashMap *dataStructure = (HashMap *) GET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE);

	// go through all tuple in data chunk;
	int length = dataChunk->numTuples;
	// update state and get value for return data chunk;
	// first update: insert (+1);
	// then update : delete (-1);

	// update insert;
	for (int i = 0; i < length; i++) {
		if (getVecInt(dataChunk->updateIdentifier, i) == -1) {
			continue;
		}
		// 1. get group by value of this tuple;
		StringInfo gbValues = makeStringInfo();
		for (int gbIndex = 0; gbIndex < LIST_LENGTH(aggGBList); gbIndex++) {
			// locate the datatype of current attribute;
			AttributeReference *ar = (AttributeReference *) getNthOfListP(aggGBList, gbIndex);
			int attPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			DataType dt = INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, attPos));

			// get this group by attribute value;
			Constant *value = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, attPos), i);
			switch(dt) {
				case DT_INT:
					appendStringInfo(gbValues, "%s#", gprom_itoa(INT_VALUE(value)));
					break;
				case DT_LONG:
					appendStringInfo(gbValues, "%s#", gprom_ltoa(INT_VALUE(value)));
					break;
				case DT_FLOAT:
					appendStringInfo(gbValues, "%s#", gprom_ftoa(INT_VALUE(value)));
					break;
				case DT_BOOL:
					// for postgresql;
					appendStringInfo(gbValues, "%s#", (BOOL_VALUE(value) == 1 ? 't' : 'f'));
					break;
				case DT_VARCHAR2:
				case DT_STRING:
					appendStringInfo(gbValues, "%s#", STRING_VALUE(value));
					break;
				default:
					FATAL_LOG("data type %d is not supported", dt);
			}
		}

		// get update type: here it must be 1 since it is an insert;
		// int updateType = 1;

		// set return update type:
		// 0: delete, insert: the data structure store previous group;
		// 1: insert only; no previous group value in the data structure;
		int resUpdateType = -1;

		// identifier for ps
		boolean hasBuildGroupPS = FALSE;

		// 2. iterate over all agg function to update data structure and get data chunk;
		for (int aggIndex = 0; aggIndex < LIST_LENGTH(aggFCList); aggIndex++) {
			// get the name stored in the data structure;
			FunctionCall *fc = (FunctionCall *) getNthOfListP(aggFCList, aggIndex);
			AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
			Constant *nameInDS = createConstString(CONCAT_STRINGS(strdup(fc->functionname), "_", strdup(ar->name)));

			// get inserted value;
			int insertPosFromDataChunk = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			Constant *insertV = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, insertPosFromDataChunk), i);


			if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0
			 || strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
				// get heaps;
				GBHeaps *gbHeap = (GBHeaps *) MAP_GET_STRING(dataStructure, STRING_VALUE(nameInDS));

				// get vector position of return datachunk;
				Constant *nameInResDC = (Constant *) MAP_GET_STRING(mapFCsToSchemas, STRING_VALUE(nameInDS));
				int posInResDC = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, STRING_VALUE(nameInResDC)));

				if (MAP_HAS_STRING_KEY(gbHeap->heapLists, gbValues->data)) {
					// get heap list of this group;
					List *heap = (List *) MAP_GET_STRING(gbHeap->heapLists, gbValues->data);

					if (heap == NIL || LIST_LENGTH(heap) == 0) {
						resUpdateType = 1;
						heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) insertV);
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) insertV);
					} else {
						resUpdateType = 0;
						Constant *oldV = (Constant *) getNthOfListP(heap, 0);
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) oldV);
						heap = heapInsert(heap, STRING_VALUE(gbHeap->heapType), (Node *) insertV);
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) getNthOfListP(heap, 0));
					}
					addToMap(gbHeap->heapLists, (Node *) createConstString(gbValues->data), (Node *) heap);
				} else { // new group heap
					resUpdateType = 1;

					// create new heap list;
					List *heap = NIL;
					heap = appendToTailOfList(heap, insertV);
					addToMap(gbHeap->heapLists, (Node *) createConstString(gbValues->data), (Node *) copyObject(heap));
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) insertV);
				}
				// addToMap(dataStructure, (Node*) nameInDS, (Node *) gbHeap);

				// dealing with ps;
				FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
					// delta tuples ps list;
					List *psList = (List *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
					// delta tuple bitset;
					BitSet *bitset = (BitSet *) getNthOfListP(psList, i);

					List *resPsList = NIL;
					if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, STRING_VALUE(c))) {
						resPsList = (List *) MAP_GET_STRING(resultDC->fragmentsInfo, STRING_VALUE(c));
					}

					BitSet *resBitset = newBitSet(bitset->length);
					if (resUpdateType == 0) {
						// append old ps from GBHeaps
						HashMap *psMapInGBHeaps = NULL;
						if (MAP_HAS_STRING_KEY(gbHeap->provSketchs, STRING_VALUE(c))) {
							psMapInGBHeaps = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));
						} else {
							psMapInGBHeaps = NEW_MAP(Constant, Node);
						}
						BitSet *bitsetInHeap = (BitSet *) MAP_GET_STRING(psMapInGBHeaps, gbValues->data);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, (BitSet *) copyObject(bitsetInHeap));
						}

						// update frag->count;
						HashMap *gbFragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbHeap->fragCount, STRING_VALUE(c))) {
							gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));
						} else {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						HashMap *fragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbFragCnt, gbValues->data)) {
							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbValues->data);
						} else {
							fragCnt = NEW_MAP(Constant, Constant);
						}

						char *bitStr = bitSetToString(bitset);

						for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
							if (bitStr[bitIndex] == '1') {
								int oriCnt = 0;
								if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
									oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
									// setBit(resBitset, bitIndex, TRUE);
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt + 1));
								} else {
									// setBit(resBitset, bitIndex, TRUE);
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
								}
							}
						}

						resBitset = bitOr(bitset, bitsetInHeap);

						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(resBitset));
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}

						addToMap(psMapInGBHeaps, (Node *) createConstString(gbValues->data), (Node *) copyObject(resBitset));
						addToMap(gbHeap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGBHeaps);
						addToMap(gbFragCnt, (Node *) createConstString(gbValues->data), (Node *) fragCnt);
						addToMap(gbHeap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						// addToMap(dataStructure, (Node *) createConstString(nameInDS), (Node *) gbHeap);
					} else if (resUpdateType == 1) {
						resBitset = (BitSet *) copyObject(bitset);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, resBitset);
							// add ps list to result data chunk;
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}

						// add this group ps to gb heaps;
						HashMap *psMapInGBHeap = NULL;
						if (MAP_HAS_STRING_KEY(gbHeap->provSketchs, STRING_VALUE(c))) {
							psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));
						} else {
							psMapInGBHeap = NEW_MAP(Constant, Node);
						}


						addToMap(psMapInGBHeap, (Node *) createConstString(gbValues->data), (Node *) copyObject(resBitset));
						addToMap(gbHeap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGBHeap);

						HashMap *gbFragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbHeap->fragCount, STRING_VALUE(c))) {
							gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));
						} else {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						HashMap *fragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbFragCnt, gbValues->data)) {
							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbValues->data);
						} else {
							NEW_MAP(Constant, Constant);
						}
						// since this is a new group;
						char *bitStr = bitSetToString(resBitset);
						for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
							if (bitStr[bitIndex] == '1') {
								addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
							}
						}

						addToMap(gbFragCnt, (Node *) createConstString(gbValues->data), (Node *) fragCnt);
						addToMap(gbHeap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);


					}
				}

				if (!hasBuildGroupPS) {
					hasBuildGroupPS = TRUE;
				}
				// }
			} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0
				|| strcmp(fc->functionname, AVG_FUNC_NAME) == 0
				|| strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
				// get acs;
				GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructure, STRING_VALUE(nameInDS));

				// get vector position of return data chunk;
				Constant *nameInResDC = (Constant *) MAP_GET_STRING(mapFCsToSchemas, STRING_VALUE(nameInDS));
				int posInResDC = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, STRING_VALUE(nameInResDC)));

				if (MAP_HAS_STRING_KEY(acs->map, gbValues->data)) {
					resUpdateType = 0;
					List *oldList = (List *) MAP_GET_STRING(acs->map, gbValues->data);
					List *newList = NIL;

					if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
						double avg = FLOAT_VALUE((Constant *) getNthOfListP(oldList, 0));
						double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldList, 1));
						int cnt = INT_VALUE((Constant *) getNthOfListP(oldList, 2));
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(avg));

						switch (insertV->constType) {
							case DT_INT:
								sum += INT_VALUE(insertV);
								break;
							case DT_FLOAT:
								sum += FLOAT_VALUE(insertV);
								break;
							case DT_LONG:
								sum += LONG_VALUE(insertV);
							default:
								FATAL_LOG("data type %d is not supported for avg", insertV->constType);
						}

						cnt += 1;
						avg = sum / cnt;
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(avg));
						newList = appendToTailOfList(newList, createConstFloat(avg));
						newList = appendToTailOfList(newList, createConstFloat(sum));
						newList = appendToTailOfList(newList, createConstInt(cnt));
					} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
						double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldList, 0));
						int cnt = INT_VALUE((Constant *) getNthOfListP(oldList, 1));
						double newSum = (double) 0;
						switch (insertV->constType) {
							case DT_INT:
								newSum = sum + INT_VALUE(insertV);
								break;
							case DT_FLOAT:
								newSum = sum + FLOAT_VALUE(insertV);
								break;
							case DT_LONG:
								newSum = sum + LONG_VALUE(insertV);
								break;
							default:
								FATAL_LOG("data type %d is not supported for sum", insertV->constType);
						}

						DataType resDT = ((AttributeDef *) getNthOfListP(resultDC->attrNames, posInResDC))->dataType;
						if (DT_INT == resDT) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt((int) sum));
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt((int) newSum));
						} else if (DT_LONG == resDT) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstLong((gprom_long_t) sum));
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstLong((gprom_long_t) newSum));
						} else if (DT_FLOAT == resDT) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(sum));
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(newSum));
						}

						newList = appendToTailOfList(newList, createConstFloat(newSum));
						newList = appendToTailOfList(newList, createConstInt(cnt + 1));
					} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
						int cnt = INT_VALUE((Constant *) getNthOfListP(oldList, 0));
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt(cnt));
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt(cnt + 1));
						newList = appendToTailOfList(newList, createConstInt(cnt + 1));
					}
					addToMap(acs->map, (Node *) createConstString(gbValues->data), (Node *) copyObject(newList));
				} else {
					// new group acs;
					resUpdateType = 1;

					List *newList = NIL;

					double sum_avg = (double) 0;

					// based on insertV type to increase avg_sum;
					switch (insertV->constType) {
						case DT_INT:
							sum_avg += (double) INT_VALUE(insertV);
							break;
						case DT_FLOAT:
							sum_avg += (double) FLOAT_VALUE(insertV);
							break;
						case DT_LONG:
							sum_avg += (double) LONG_VALUE(insertV);
							break;
						default:
							FATAL_LOG("data type is not supported here");
					}

					if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
						newList = appendToTailOfList(newList, createConstFloat(sum_avg));
						newList = appendToTailOfList(newList, createConstFloat(sum_avg));
						newList = appendToTailOfList(newList, createConstInt(1));
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(sum_avg));
					} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
						newList = appendToTailOfList(newList, createConstFloat(sum_avg));
						newList = appendToTailOfList(newList, createConstInt(1));
						DataType resDT = ((AttributeDef *) getNthOfListP(resultDC->attrNames, posInResDC))->dataType;
						if (DT_INT == resDT) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt((int) sum_avg));
						} else if (DT_LONG == resDT) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstLong((gprom_long_t) sum_avg));
						} else if (DT_FLOAT == resDT) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(sum_avg));
						}
					} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
						newList = appendToTailOfList(newList, createConstInt(1));
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt(1));
					}
					addToMap(acs->map, (Node *) createConstString(gbValues->data), (Node *) newList);
				}

				// dealing with ps;
				FOREACH_HASH_KEY (Constant, c, dataChunk->fragmentsInfo) {
					List * psList = (List *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
					// delta bitset;
					BitSet *bitSet = (BitSet *) getNthOfListP(psList, i);

					List *resPsList = NIL;
					if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, STRING_VALUE(c))) {
						resPsList = (List *) MAP_GET_STRING(resultDC->fragmentsInfo, STRING_VALUE(c));
					}
					BitSet *resBitset = newBitSet(bitSet->length);
					if (resUpdateType == 0) {
						HashMap *psMapInACS = NULL;

						if (MAP_HAS_STRING_KEY(acs->provSketchs, STRING_VALUE(c))) {
							psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
						} else {
							psMapInACS = NEW_MAP(Constant, Node);
						}

						// old bitset;
						BitSet *bitSetInACS = (BitSet *) MAP_GET_STRING(psMapInACS, gbValues->data);

						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, (BitSet *) copyObject(bitSetInACS));
						}

						// get gb fragcnt
						HashMap *gbFragCnt = NULL;
						if (MAP_HAS_STRING_KEY(acs->fragCount, STRING_VALUE(c))) {
							gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
						} else {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						HashMap *fragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbFragCnt, gbValues->data)) {
							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbValues->data);
						} else {
							fragCnt = NEW_MAP(Constant, Constant);
						}

						char *bitStr = bitSetToString(bitSet);
						for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
							if (bitStr[bitIndex] == '1') {
								if (MAP_HAS_INT_KEY(fragCnt, bitIndex)) {
									int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt + 1));
								} else {
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
								}
							}
						}

						resBitset = bitOr(bitSet, bitSetInACS);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(resBitset));
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}

						addToMap(psMapInACS, (Node *) createConstString(gbValues->data), (Node *) copyObject(resBitset));
						addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) psMapInACS);

						addToMap(gbFragCnt, (Node *) createConstString(gbValues->data), (Node *) fragCnt);
						addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);

					} else if (resUpdateType == 1) {
						resBitset = (BitSet *) copyObject(bitSet);

						// append to ps list in result data chunk;
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, resBitset);
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}

						HashMap *psMapInACS = NULL;
						if (MAP_HAS_STRING_KEY(acs->provSketchs, STRING_VALUE(c))) {
							psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
						} else {
							psMapInACS = NEW_MAP(Constant, Node);
						}

						addToMap(psMapInACS, (Node *) createConstString(gbValues->data), (Node *) copyObject(resBitset));

						HashMap *gbFragCnt = NULL;
						if (MAP_HAS_STRING_KEY(acs->fragCount, STRING_VALUE(c))) {
							gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
						} else {
							gbFragCnt = NEW_MAP(Constant, Node);
						}

						HashMap *fragCnt = NULL;
						if (MAP_HAS_STRING_KEY(gbFragCnt, gbValues->data)) {
							fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbValues->data);
						} else {
							fragCnt = NEW_MAP(Constant, Constant);
						}

						char *bitStr = bitSetToString(resBitset);
						for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
							if (bitStr[bitIndex] == '1') {
								addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(1));
							}
						}

						addToMap(gbFragCnt, (Node *) createConstString(gbValues->data), (Node *) fragCnt);
						addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
					}
				}
				if (!hasBuildGroupPS) {
					hasBuildGroupPS = TRUE;
				}
			}
		}

		// get group by attributes;
		FOREACH (AttributeReference, ar, aggGBList) {
			// get inserted value;
			int posInFromDataChunk = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			Constant *insertV = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, posInFromDataChunk), i);
			Constant *nameInResDC = (Constant *) MAP_GET_STRING(mapFCsToSchemas, ar->name);
			int posInResDataChunk = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, STRING_VALUE(nameInResDC)));
			if (resUpdateType == 0) {
				// -, +, append two constants to this vector with same value;
				vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDataChunk), (Node *) copyObject(insertV));
				vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDataChunk), (Node *) copyObject(insertV));
			} else if (resUpdateType == 1) {
				vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDataChunk), (Node *) copyObject(insertV));
			}
		}

		// dealing with resUpdateIdentifier;
		if (resUpdateType == 0) {
			vecAppendInt(resultDC->updateIdentifier, -1);
			vecAppendInt(resultDC->updateIdentifier, 1);
		} else if (resUpdateType == 1) {
			vecAppendInt(resultDC->updateIdentifier, 1);
		}
	}

	// update delete;
	for (int i = 0; i < length; i++) {
		if (getVecInt(dataChunk->updateIdentifier, i) == 1) {
			continue;
		}

		// 1. get group by value;
		StringInfo gbValues = makeStringInfo();
		for (int gbIndex = 0; gbIndex < LIST_LENGTH(aggGBList); gbIndex++) {
			// locate currrent attr datatype
			AttributeReference *ar = (AttributeReference *) getNthOfListP(aggGBList, gbIndex);
			int attPos = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			DataType dt = INT_VALUE((Constant *) MAP_GET_INT(dataChunk->posToDatatype, attPos));

			// get this group by value;
			Constant *value = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, attPos), i);

			switch (dt) {
				case DT_INT:
					appendStringInfo(gbValues, "%s#", gprom_itoa(INT_VALUE(value)));
					break;
				case DT_LONG:
					appendStringInfo(gbValues, "%s#", gprom_ltoa(INT_VALUE(value)));
					break;
				case DT_FLOAT:
					appendStringInfo(gbValues, "%s#", gprom_ftoa(INT_VALUE(value)));
					break;
				case DT_BOOL:
					// for postgresql;
					appendStringInfo(gbValues, "%s#", (BOOL_VALUE(value) == 1 ? 't' : 'f'));
					break;
				case DT_VARCHAR2:
				case DT_STRING:
					appendStringInfo(gbValues, "%s#", STRING_VALUE(value));
					break;
				default:
					FATAL_LOG("data type %d is not supported", dt);
			}
		}

		// set return update type;
		// 0: delete insert: find previous group in data structure;
		// -1: delete only; no data afterwards;
		int resUpdateType = 1;
		boolean hasBuildGroupPS = FALSE;
		// identifier for ps;
		for (int aggIndex = 0; aggIndex < LIST_LENGTH(aggFCList); aggIndex++) {
			// get store data structure;
			FunctionCall *fc = (FunctionCall *) getNthOfListP(aggFCList, aggIndex);
			AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
			Constant *nameInDS = createConstString(CONCAT_STRINGS(strdup(fc->functionname), "_", strdup(ar->name)));

			// get inserted value;
			int delPosFromDataChunk = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			Constant *delV = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, delPosFromDataChunk), i);

			if (strcmp(fc->functionname, MIN_FUNC_NAME) == 0
			|| strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
				GBHeaps *gbHeap = (GBHeaps *) MAP_GET_STRING(dataStructure, STRING_VALUE(nameInDS));

				// vec of return;
				Constant * nameInResDC = (Constant *) MAP_GET_STRING(mapFCsToSchemas, STRING_VALUE(nameInDS));
				int posInResDC = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, STRING_VALUE(nameInResDC)));

				List *heap = (List *) MAP_GET_STRING(gbHeap->heapLists, gbValues->data);

				if (LIST_LENGTH(heap) == 1) {
					resUpdateType = -1;
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) getNthOfListP(heap, 0));
					removeMapStringElem(gbHeap->heapLists, gbValues->data);
				} else if (LIST_LENGTH(heap) > 1) {
					resUpdateType = 0;
					Constant *oldV = (Constant *) getNthOfListP(heap, 0);
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) oldV);
					heap = heapDelete(heap, STRING_VALUE(gbHeap->heapType), (Node *) delV);
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) getNthOfListP(heap, 0));
				}
				addToMap(gbHeap->heapLists, (Node *) createConstString(gbValues->data), (Node *) heap);

				// dealing with ps;
				FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
					List *psList = (List *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
					BitSet *bitset = (BitSet *) getNthOfListP(psList, i);

					List *resPsList = NIL;
					if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, STRING_VALUE(c))) {
						resPsList = (List *) MAP_GET_STRING(resultDC->fragmentsInfo, STRING_VALUE(c));
					}

					// BitSet *resBitset = newBitSet(bitset->length);
					if (resUpdateType == -1) {
						HashMap *psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));

						BitSet *bitsetInHeap = (BitSet *) MAP_GET_STRING(psMapInGBHeap, gbValues->data);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, (BitSet *) copyObject(bitsetInHeap));
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}

						// -1 means this is the last of the heap, remove ps, fg-cnt
						removeMapStringElem(psMapInGBHeap, gbValues->data);
						addToMap(gbHeap->provSketchs, (Node *) copyObject(c), (Node *) psMapInGBHeap);

						// remove fgcnt;
						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));
						removeMapStringElem(gbFragCnt, gbValues->data);
						addToMap(gbHeap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
					} else if (resUpdateType == 0) {
						HashMap *psMapInGBHeap = (HashMap *) MAP_GET_STRING(gbHeap->provSketchs, STRING_VALUE(c));

						BitSet *bitsetInHeap = (BitSet *) MAP_GET_STRING(psMapInGBHeap, gbValues->data);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(bitsetInHeap));
						}

						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(gbHeap->fragCount, STRING_VALUE(c));
						HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbValues->data);

						char *bitStr = bitSetToString(bitset);
						for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
							if (bitStr[bitIndex] == 1) {
								int oriCnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));
								if (oriCnt == 1) {
									setBit(bitsetInHeap, bitIndex, FALSE);
									removeMapElem(fragCnt, (Node *) createConstInt(bitIndex));
								} else if (oriCnt > 1) {
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(oriCnt - 1));
								}
							}
						}

						addToMap(gbFragCnt, (Node *) createConstString(gbValues->data), (Node *) fragCnt);
						addToMap(gbHeap->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(bitsetInHeap));
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}
					}
				}

				if (!hasBuildGroupPS) {
					hasBuildGroupPS = TRUE;
				}
			} else if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0
				|| strcmp(fc->functionname, SUM_FUNC_NAME) == 0
				|| strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
				// get acs;
				GBACSs *acs = (GBACSs *) MAP_GET_STRING(dataStructure, STRING_VALUE(nameInDS));

				// get vector pos of return dc;
				Constant *nameInResDC = (Constant *) MAP_GET_STRING(mapFCsToSchemas, STRING_VALUE(nameInDS));
				int posInResDC = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, STRING_VALUE(nameInResDC)));

				List *oldList = (List *) MAP_GET_STRING(acs->map, gbValues->data);
				List *newList = NIL;

				if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
					double avg = FLOAT_VALUE((Constant *) getNthOfListP(oldList, 0));
					double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldList, 1));
					int cnt = INT_VALUE((Constant *) getNthOfListP(oldList, 2));
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(avg));

					if (cnt == 1) {
						removeMapStringElem(acs->map, gbValues->data);
						resUpdateType = -1;
					} else if (cnt > 1) {
						resUpdateType = 0;
						switch (delV->constType) {
							case DT_INT:
								sum -= (double) INT_VALUE(delV);
								break;
							case DT_LONG:
								sum -= (double) LONG_VALUE(delV);
								break;
							case DT_FLOAT:
								sum -= FLOAT_VALUE(delV);
								break;
							default:
								FATAL_LOG("data type is not supported");
						}

						cnt -= 1;
						avg = sum / cnt;
						newList = appendToTailOfList(newList, createConstFloat(avg));
						newList = appendToTailOfList(newList, createConstFloat(sum));
						newList = appendToTailOfList(newList, createConstInt(cnt));
						addToMap(acs->map, (Node *) createConstString(gbValues->data), (Node *) newList);
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(avg));
					}
				} else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
					double sum = FLOAT_VALUE((Constant *) getNthOfListP(oldList, 0));
					int cnt = INT_VALUE((Constant *) getNthOfListP(oldList, 1)) - 1;
					double newSum = (double) 0;
					switch(delV->constType) {
						case DT_INT:
							newSum = sum - INT_VALUE(delV);
							break;
						case DT_FLOAT:
							newSum = sum - FLOAT_VALUE(delV);
							break;
						case DT_LONG:
							newSum = sum - LONG_VALUE(delV);
							break;
						default:
							FATAL_LOG("data type %d is not supported for sum", delV->constType);
					}
					newList = appendToTailOfList(newList, createConstFloat(newSum));
					newList = appendToTailOfList(newList, createConstInt(cnt));

					// based on return type to cast sum to correct type;
					DataType resDT = ((AttributeDef *) getNthOfListP(resultDC->attrNames, posInResDC))->dataType;

					if (cnt <= 0) {
						resUpdateType = -1;
						removeMapStringElem(acs->map, gbValues->data);
					} else {
						resUpdateType = 0;
						addToMap(acs->map, (Node *) createConstString(gbValues->data), (Node *) newList);
					}

					if (DT_INT == resDT) {
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt((int) sum));
						if (cnt > 0) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt((int) newSum));
						}
					} else if (DT_LONG == resDT) {
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstLong((gprom_long_t) sum));
						if (cnt > 0) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstLong((gprom_long_t) newSum));
						}
					} else if (DT_FLOAT == resDT) {
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(sum));
						if (cnt > 0) {
							vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstFloat(newSum));
						}
					}
				} else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
					int cnt = INT_VALUE(getNthOfListP(oldList, 0));
					vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt(cnt));
					if (cnt > 1) {
						resUpdateType = 0;
						vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) createConstInt(cnt - 1));
						newList = appendToTailOfList(newList, createConstInt(cnt - 1));
						addToMap(acs->map, (Node *) createConstString(gbValues->data), (Node *) newList);
					} else {
						resUpdateType = -1;
						removeMapStringElem(acs->map, gbValues->data);
					}
				}
				// dealing with ps;
				FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
					List *psList = (List *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
					BitSet *bitset = (BitSet *) getNthOfListP(psList, i);

					List *resPsList = NIL;
					if (MAP_HAS_STRING_KEY(resultDC->fragmentsInfo, STRING_VALUE(c))) {
						resPsList = (List *) MAP_GET_STRING(resultDC->fragmentsInfo, STRING_VALUE(c));
					}

					BitSet *resBitSet = newBitSet(bitset->length);

					if (resUpdateType == -1) {
						HashMap *psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c));
						resBitSet = (BitSet *) MAP_GET_STRING(psMapInACS, gbValues->data);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(resBitSet));
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}

						removeMapStringElem(psMapInACS, gbValues->data);
						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
						removeMapStringElem(gbFragCnt, gbValues->data);
						addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);
						addToMap(acs->provSketchs, (Node *) copyObject(c), (Node *) psMapInACS)	;
					} else if (resUpdateType == 0) {
						HashMap *psMapInACS = (HashMap *) MAP_GET_STRING(acs->provSketchs, STRING_VALUE(c)) ;
						resBitSet = (BitSet *) MAP_GET_STRING(psMapInACS, gbValues->data);
						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(resBitSet));
						}

						HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(acs->fragCount, STRING_VALUE(c));
						HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, gbValues->data);

						char *bitStr = bitSetToString(bitset);
						for (int bitIndex = 0; bitIndex < strlen(bitStr); bitIndex++) {
							if (bitStr[bitIndex] == '1') {
								int cnt = INT_VALUE((Constant *) MAP_GET_INT(fragCnt, bitIndex));

								if (cnt == 1) {
									setBit(resBitSet, bitIndex, FALSE);
									removeMapElem(fragCnt, (Node *) createConstInt(bitIndex));
								} else if (cnt > 1) {
									addToMap(fragCnt, (Node *) createConstInt(bitIndex), (Node *) createConstInt(cnt - 1));
								}
							}
						}

						addToMap(gbFragCnt, (Node *) createConstString(gbValues->data), (Node *) fragCnt);
						addToMap(acs->fragCount, (Node *) copyObject(c), (Node *) gbFragCnt);

						if (!hasBuildGroupPS) {
							resPsList = appendToTailOfList(resPsList, copyObject(resBitSet));
							addToMap(resultDC->fragmentsInfo, (Node *) copyObject(c), (Node *) resPsList);
						}
					}
				}

				if (!hasBuildGroupPS) {
					hasBuildGroupPS = TRUE;
				}
			}
		}

		// gb attri;
		FOREACH (AttributeReference, ar, aggGBList) {
			int posInFromDataChunk = INT_VALUE((Constant *) MAP_GET_STRING(dataChunk->attriToPos, ar->name));
			Constant *delV = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, posInFromDataChunk), i);
			Constant *nameInResDC = (Constant *) MAP_GET_STRING(mapFCsToSchemas, ar->name);
			int posInResDC = INT_VALUE((Constant *) MAP_GET_STRING(resultDC->attriToPos, STRING_VALUE(nameInResDC)));
			if (resUpdateType == -1) {
				vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) copyObject(delV));
			} else if (resUpdateType == 0) {
				vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) copyObject(delV));
				vecAppendNode((Vector *) getVecNode(resultDC->tuples, posInResDC), (Node *) copyObject(delV));
			}
		}

		// dealing with res update indntifier;
		if (resUpdateType == -1) {
			vecAppendInt(resultDC->updateIdentifier, -1);
		} else if (resUpdateType == 0) {
			vecAppendInt(resultDC->updateIdentifier, -1);
			vecAppendInt(resultDC->updateIdentifier, 1);
		}
	}

	removeStringProperty(OP_LCHILD(op), PROP_DATA_CHUNK);
	setStringProperty(op, PROP_DATA_CHUNK, (Node *) resultDC);
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

	// TODO
	// For batch update, there might be multiple updates for this table
	// we might use a list to store all the updates,

	DataChunk* dataChunk = getDataChunkFromUpdateStatement(updateStatement, (TableAccessOperator *) op);
	setStringProperty(op, PROP_DATA_CHUNK, (Node *) dataChunk);

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
	for(int col = 0; col < LIST_LENGTH(relation->schema); col++)
	{
		int dataType = INT_VALUE((Constant*) getMapInt(dataChunk->posToDatatype, col));
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

	// fill tuples:
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

	ColumnChunk *filterResult = evaluateExprOnDataChunk(condition, dataChunk);

	// new a result data chunk AND set fields except numTuples;
	DataChunk* resultDC = initDataChunk();

	// identifier to indicate first meeting value in result data chunk;
	boolean initField = FALSE;

	// result is True or False stored in a bitset;
	BitSet *bits = filterResult->data.bs;
	char *bitString = bitSetToString(bits);
	int length = strlen(bitString);

	for (int i = 0; i < length; i++) {
		if (bitString[i] == '1') {
			if (!initField) {
				resultDC->attrNames = (List*) copyList(dataChunk->attrNames);
				resultDC->attriToPos = (HashMap*) copyObject(dataChunk->attriToPos);
				resultDC->posToDatatype = (HashMap*) copyObject(dataChunk->posToDatatype);
				resultDC->tupleFields = dataChunk->tupleFields;
				for (int col = 0; col < resultDC->tupleFields; col++) {
					Vector *colVec = makeVector(VECTOR_NODE, T_Vector);
					vecAppendNode(resultDC->tuples, (Node *) colVec);
				}
				initField = TRUE;
			}

			// append data;
			for (int col = 0; col < resultDC->tupleFields; col++) {
				Vector *dstVec = (Vector *) getVecNode(resultDC->tuples, col);
				Vector *srcVec = (Vector *) getVecNode(dataChunk->tuples, col);
				vecAppendNode(dstVec, (Node *) copyObject(getVecNode(srcVec, i)));
			}

			// append update type;
			vecAppendInt(resultDC->updateIdentifier, getVecInt(dataChunk->updateIdentifier, i));

			// append provenance sketch;
			FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
				List *srcList = (List *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
				List *dstList = NIL;
				if (resultDC->numTuples > 0) {
					dstList = (List *) MAP_GET_STRING(resultDC->fragmentsInfo, STRING_VALUE(c));
				}
				dstList = appendToTailOfList(dstList, copyObject((BitSet *) getNthOfListP(srcList, i)));
				addToMap(resultDC->fragmentsInfo, (Node *) c, (Node *) dstList);
			}
			resultDC->numTuples += 1;
		}
	}

	return resultDC;

	/*
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
	*/
}

/*
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
*/

/*
static ConstRelOperator *
getConstRelOpFromDataChunk(DataChunk *dataChunk)
{
	List *values = NIL;
	List *attrNames = (List *) getAttrDefNames(dataChunk->attrNames);
	List *dataTypes = NIL;
	List *updateType = NIL;

	int lenRow = dataChunk->numTuples;
	int lenCol = dataChunk->tupleFields;

	// append value;
	for (int row = 0; row < lenRow; row++) {
		List *valList = NIL;
		for (int col = 0; col < lenCol; col++) {
			Constant *val = (Constant *) getVecNode((Vector *) getVecNode(dataChunk->tuples, col), row);
			valList = appendToTailOfList(valList, val);
		}

		values = appendToTailOfList(values, valList);
	}

	// append ps string;
	FOREACH_HASH_KEY(Constant, c, dataChunk->fragmentsInfo) {
		List *bitsetList = (List *) MAP_GET_STRING(dataChunk->fragmentsInfo, STRING_VALUE(c));
		for (int row = 0; row < LIST_LENGTH(bitsetList); row++) {
			List *l = NIL;
			l = (List *) getNthOfListP(values, row);

		}
	}





	// for (int col = 0; col < dataChunk->tupleFields; col++) {
	// 	List *colValues = NIL;

	// 	// get value from each vector cell;
	// 	Vector *vec = (Vector *) getVecNode(dataChunk->tuples, col);
	// 	for (int index = 0; index < VEC_LENGTH(vec); index++) {
	// 		colValues = appendToTailOfList(colValues, getVecNode(vec, index));
	// 	}

	// 	values = appendToTailOfList(values, colValues);
	// 	dataTypes = appendToTailOfListInt(dataTypes, INT_VALUE((Constant *) getMapInt(dataChunk->posToDatatype, col)));
	// }

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
	// ConstRelMultiListsOperator *constRel = createConstRelOp(values, NIL, deepCopyStringList(attrNames), dataTypes);
	ConstRelMultiListsOperator *constRel = createConstRelMultiListsOp(values, NIL, deepCopyStringList(attrNames), dataTypes);

	return (ConstRelOperator *) constRel;
}
*/

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
			fragNo = LIST_LENGTH(rangeList) - 2;
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
