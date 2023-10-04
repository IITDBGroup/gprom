/*
 * Header files
 */
#include "common.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/bitset/bitset.h"
#include "model/set/vector.h"

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"

#include "sql_serializer/sql_serializer.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_postgres.h"
#include "instrumentation/timing_instrumentation.h"
#include "provenance_rewriter/update_ps/table_compress.h"
#include "provenance_rewriter/update_ps/update_ps_incremental.h"
#include "provenance_rewriter/update_ps/update_ps_build_state.h"
#include "provenance_rewriter/update_ps/rbtree.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "operator_optimizer/operator_optimizer.h"

#include "provenance_rewriter/update_ps/bloom.h"
#include "provenance_rewriter/update_ps/store_operator_data.h"
#include "provenance_rewriter/update_ps/fetch_operator_data.h"
/*
 * Macro
 */
#define DELETE_RULE_1 0 // DROP
#define DELETE_RULE_2 1 // DELETE(APPROXIMATE)
#define DELETE_RULE_3 3 // DELETE(ACCURATE)

#define INSERT_RULE_1 100 // INSERT(APPROXIMATE)
#define INSERT_RULE_2 101 // INSERT(ACCURATE)

#define UPDATE_RULE_1 200 // DELETE(APPROXIMATE) -> INSERT(APPROXIMATE)
#define UPDATE_RULE_2 201 // DELETE(APPROXIMATE) -> INSERT(ACCURATE)
#define UPDATE_RULE_3 202 // DELETE(ACCURATE) -> INSERT(APPROXIMATE)
#define UPDATE_RULE_4 203 // DELETE(ACCURATE) -> INSERT(ACCURATE)

/*
 * Function Declaration
 */
//
static QueryOperator* captureRewrite(ProvenanceComputation* pc);
//DELETE
static char* update_ps_delete(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo, int ruleNum);
static char* update_ps_delete_drop(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo);
static char* update_ps_delete_approximate(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo);
static char* update_ps_delete_accurate(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo);

//INSERT
static char* update_ps_insert(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo, int ruleNum);
static char* update_ps_insert_approximate(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo);
static char* update_ps_insert_accurate(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo);

//UPDATE
static char* update_ps_update(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo, int ruleNum);
static char* update_ps_del_ins_1(QueryOperator *query, QueryOperator *updateQuery, psInfo *PSInfo);

//OTHER AUXILIARY METHODS
static char* getUpdatedTable(QueryOperator *op);
static psAttrInfo* getUpdatedTablePSAttrInfo(psInfo *PSInfo, char *tableName);
static List* getAllTables(psInfo *PSInfo);
static char* createResultComponent(char *tableName, char *psAttr, char *ps);
static ProjectionOperator* createDummyProjTree(QueryOperator *updateQuery);
static void reversePSInfo(psInfo *PSInfo, char *updatedTable);
static QueryOperator* rewriteTableAccessOperator(TableAccessOperator *op, psInfo *PSInfo);
static boolean getTableAccessOps(Node *op, List **l);
static BitSet* bitOrResults(HashMap *old, HashMap *new, StringInfo *result);
void bitOrResultsPostgres(HashMap *old, HashMap *new, StringInfo *result);
int skipFragsBasedOnPS(char *updatedTable, psInfo *PSInfo, QueryOperator *updateQuery);
void compressTable(char *tablename, char *psAttr, List *ranges);
static boolean replaceSetBitsWithFastBitOr (Node *node, void *state);
static boolean replaceTableAccessWithCompressedTableAccess(Node *node, void *state);
void removeProvAttrsList(QueryOperator *op);
static void modifyUncertCapTree(QueryOperator *op);
static void preprocessJoin(QueryOperator *op);
static void setOperatorNumber(QueryOperator *op);
static int OPERATOR_NUM = 0;
//static void localTest();
/*
 * Function Implementation
 */
// #define JOIN_NUMBER_FROM_TOP "JOIN_NUMBER_FROM_TOP"

// static int joinNumber = 0;
// static void preprocessingJoinOpNumber(QueryOperator *op);
// static void preprocessingSetStartJoinOP(QueryOperator *op);
static void
preprocessJoin(QueryOperator *op)
{
	if (isA(op, TableAccessOperator)) {
		return;
	}

	if (isA(op, JoinOperator)) {
		setStringProperty(op, GROUP_JOIN_START, (Node *) createConstBool(TRUE));
		return;
	}

	FOREACH(QueryOperator, q, op->inputs) {
        preprocessJoin(q);
    }
}

static void
setOperatorNumber(QueryOperator *op)
{
	if (op == NULL) {
		return;
	}
	SET_STRING_PROP(op, PROP_OPERATOR_NUMBER, createConstInt(OPERATOR_NUM));
	OPERATOR_NUM++;
	FOREACH(QueryOperator, q, op->inputs) {
		setOperatorNumber(q);
	}
}

char*
update_ps(ProvenanceComputation *qbModel)
{
	DEBUG_NODE_BEATIFY_LOG("qbModel", qbModel);
	INFO_OP_LOG("qbModel", qbModel);
	/*
	 *	two children
	 * 	left: update statements (one or list of statements)
	 *	right: query
	 */

	/* Get left(update statements or null if from delta) and right children(query) */
	DLMorDDLOperator *leftChild = NULL;
	QueryOperator *rightChild = NULL;
	if (LIST_LENGTH(qbModel->op.inputs) > 1) {
		leftChild = (DLMorDDLOperator *) OP_LCHILD((QueryOperator *) qbModel);
		rightChild = OP_RCHILD((QueryOperator *) qbModel);
		// remove left child from provenance computation;
		((QueryOperator *) qbModel)->inputs = singleton(rightChild);
	// } else {
		// rightChild = OP_LCHILD((QueryOperator *) qbModel);
	}

	/* set each operator a number*/
	setOperatorNumber((QueryOperator *) qbModel);
	DEBUG_NODE_BEATIFY_LOG("after set number", qbModel);

	/* check stored info */
	char *queryName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);

	boolean isQStored = checkQueryInfoStored(queryName);

	/* Check if there is state for this algebra tree */
	if (!isQStored) {
		qbModel = (ProvenanceComputation *) buildState((QueryOperator *) qbModel);
		DEBUG_NODE_BEATIFY_LOG("operator with state before stored", qbModel);
		storeOperatorData((QueryOperator *) qbModel);
		setInfoStored(queryName);
	} else {
		START_TIMER("module - update provenance sketch - fetching stored data");
		fetchOperatorData((QueryOperator *) qbModel);
		STOP_TIMER("module - update provenance sketch - fetching stored data");
	}

	DEBUG_NODE_BEATIFY_LOG("operator with state data", qbModel);

	if (1 == 2) {
		return "END";
	}
	// if (!HAS_STRING_PROP((QueryOperator *) qbModel, PROP_HAS_DATA_STRUCTURE_BUILT)) {
	// 	qbModel = (ProvenanceComputation *) buildState((QueryOperator *) qbModel);

	// 	store_operator_data((QueryOperator *) qbModel);
	// 	SET_STRING_PROP((QueryOperator *) qbModel, PROP_HAS_DATA_STRUCTURE_BUILT, createConstBool(TRUE));
	// }

	DEBUG_NODE_BEATIFY_LOG("operator with state data", qbModel);

	// check option for group join and preprocess;
	if (getBoolOption(OPTION_UPDATE_PS_GROUP_JOIN)) {
		preprocessJoin((QueryOperator *) qbModel);
		preprocessJoin((QueryOperator *) qbModel);
	}

	int repetition = 1;
	repetition = getIntOption(OPTION_UPDATE_PS_REPETITION);
	INFO_LOG("execution repetition %d", repetition);
	/* Update provenance sketch */
	if (NULL == leftChild) { // delta from cached table not from statement
		START_TIMER(INCREMENTAL_UPDATE_TIMER);
		update_ps_incremental((QueryOperator *) qbModel, (QueryOperator *) leftChild);
		STOP_TIMER(INCREMENTAL_UPDATE_TIMER);
	} else {
		if (isA(leftChild->stmt, List)) {
			List *updateStmts = (List *) leftChild->stmt;
			START_TIMER(INCREMENTAL_UPDATE_TIMER);
			FOREACH(DLMorDDLOperator, stmt, updateStmts) {
				update_ps_incremental((QueryOperator *) qbModel, (QueryOperator *) stmt);
			}
			STOP_TIMER(INCREMENTAL_UPDATE_TIMER);
		} else {
			START_TIMER(INCREMENTAL_UPDATE_TIMER);
			update_ps_incremental((QueryOperator *) qbModel, (QueryOperator *) leftChild);
			STOP_TIMER(INCREMENTAL_UPDATE_TIMER);
		}
	}
	// AFTER INCREMENTAL UPDATE STEPS, GET NEW SKETCH;
	QueryOperator *topOperator = OP_LCHILD((QueryOperator *) qbModel);
	if (HAS_STRING_PROP(topOperator, PROP_DATA_CHUNK)) {
		HashMap *inputChunkMaps = (HashMap *) GET_STRING_PROP(topOperator, PROP_DATA_CHUNK);
		DataChunk *inputDCIns = (DataChunk *) MAP_GET_STRING(inputChunkMaps, PROP_DATA_CHUNK_INSERT);
		DataChunk *inputDCDel = (DataChunk *) MAP_GET_STRING(inputChunkMaps, PROP_DATA_CHUNK_DELETE);

		PSMap *storedPSMap = (PSMap *) GET_STRING_PROP((QueryOperator *) qbModel, PROP_DATA_STRUCTURE_STATE);

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
	}
	PSMap *psMap = (PSMap *) GET_STRING_PROP((QueryOperator *) qbModel, PROP_DATA_STRUCTURE_STATE);
	StringInfo allPSs = makeStringInfo();
	FOREACH_HASH_KEY(Constant, c, psMap->fragCount) {
		HashMap *fragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
		int provLens = INT_VALUE((Constant *) MAP_GET_STRING(psMap->provLens, STRING_VALUE(c)));
		BitSet *ps = newBitSet(provLens);
		FOREACH_HASH_KEY(Constant, c, fragCnt) {
			setBit(ps, INT_VALUE(c), TRUE);
		}
		MAP_ADD_STRING_KEY(psMap->provSketchs, STRING_VALUE(c), ps);

		appendStringInfo(allPSs, "%s:%s\n", STRING_VALUE(c), bitSetToString(ps));
	}


	return allPSs->data;



















	// return update_ps_incremental((QueryOperator *) qbModel);

	/* ################################## */
	/* ## CODES BELOW CAN BE DISCARDED ## */
	/* ################################## */

	//	return update_ps_incremental( qbModel);


	//initialize some parameters to get the ps, left chile(update statement) and right child(query);
	ProvenanceComputation *op = qbModel;
	Node *coarsePara = NULL;
	psInfo *psPara = NULL;

	coarsePara = (Node*) getStringProperty((QueryOperator*) op,
	PROP_PC_COARSE_GRAINED);
	psPara = createPSInfo(coarsePara);
	/*
	 * Check the compressed table exists,
	 * If not, build it.
	 */
	// check each table to build compressed table;
	List *allTables = getAllTables(psPara);

	for (int i = 0; i < LIST_LENGTH(allTables); i++) {
		char *tableName =
				(char*) ((Constant*) getNthOfListP(allTables, i))->value;
		List *psAttrInfoList = (List*) getMapString(psPara->tablePSAttrInfos,
				tableName);
		psAttrInfo *attInfo = (psAttrInfo*) getNthOfListP(psAttrInfoList, 0);
		INFO_LOG("TABLE NAME: %s\n", tableName);
		INFO_LOG("ATTRI NAME: %s\n", attInfo->attrName);
		DEBUG_NODE_BEATIFY_LOG("RANGE LIST\n", attInfo->rangeList);
		tableCompress(tableName, attInfo->attrName, attInfo->rangeList);

	}


	/*
	 * get the left and right childred respectively;
	 * left child is a update statement
	 * right child is a normal query
	 */
	QueryOperator *op1 = (QueryOperator*) qbModel;
	QueryOperator *lChild = (QueryOperator*) OP_LCHILD(op1);
	removeParent(lChild, op1);
	DEBUG_NODE_BEATIFY_LOG("LEFT CHILD\n", lChild);
	/*
	 * Update the compressed table based on UPDATE STATEMENT;
	 */
	// get update table name from update statement;
	char *updateTblName = NULL;
	DLMorDDLOperator *updateOp = (DLMorDDLOperator*) lChild;
	switch (nodeTag(updateOp->stmt)) {
	case T_Insert:
		updateTblName = ((Insert*) updateOp->stmt)->insertTableName;
		break;
	case T_Delete:
		updateTblName = ((Delete*) updateOp->stmt)->deleteTableName;
		break;
	case T_Update:
		updateTblName = ((Update*) updateOp->stmt)->updateTableName;
		break;
	default:
		break;
	}
	List *psAttrInfoList = (List*) getMapString(psPara->tablePSAttrInfos,
			updateTblName);
	psAttrInfo *attrInfo = (psAttrInfo*) getNthOfListP(psAttrInfoList, 0);


	updateCompressedTable(lChild, updateTblName, attrInfo);


	/*
	 * Get capture rewrite query
	 */
	QueryOperator* captureQuery = captureRewrite(qbModel);
//	DEBUG_NODE_BEATIFY_LOG("capture query:\n", captureQuery);
//	INFO_OP_LOG("before uncert rewrite:\n", captureQuery);


	/*
	 * For each table access operator
	 * Replace it with compressed table
	 * Set the property "PROP_HAS_UNCERT"
	 */

	replaceTableAccessWithCompressedTableAccess((Node*) captureQuery, NULL);
	replaceSetBitsWithFastBitOr((Node*) captureQuery, NULL);
	removeProvAttrsList((QueryOperator *) captureQuery);
	/*
	 * uncertainty rewrite of capture query;
	 */
	INFO_OP_LOG("before uncert rewrite:\n", captureQuery);
	QueryOperator* uncertCaptureRewriteOp = rewriteRange(captureQuery);
//	QueryOperator* uncertCaptureRewriteOp = rewriteUncert(captureQuery);
	INFO_OP_LOG("after uncert rewrite:\n", uncertCaptureRewriteOp);

	modifyUncertCapTree(uncertCaptureRewriteOp);

	/*
	 * Serialize uncertainy rewrite query
	 * Run to get the updated ps
	 */
	StringInfo updatePSQuery = makeStringInfo();
	appendStringInfo(updatePSQuery, "%s;", serializeQuery(uncertCaptureRewriteOp));

//	if (getBackend() == BACKEND_POSTGRES) {
//			postgresExecuteStatement(updatePSQuery->data);
//	}

	boolean stopHere = TRUE;
		if (stopHere) {
			return updatePSQuery->data;
	}

	QueryOperator *rChild = OP_RCHILD(op1);
	op1->inputs = singleton(rChild);

	removeParent(lChild, (QueryOperator*) op);

	DEBUG_NODE_BEATIFY_LOG(
			"\n#######################\n \t Query:\n#######################\n",
			rChild);
//	DEBUG_NODE_BEATIFY_LOG(
//			"\n#######################\n \t PS INFO:\n#######################\n",
//			psPara);
//	DEBUG_NODE_BEATIFY_LOG(
//			"\n#######################\n \t LEFT query:\n#######################\n",
//			lChild);
	INFO_OP_LOG(
			"\n#######################\n \t LEFT CHILD query:\n#######################\n",
			lChild);

	/*
	 * Currently, stop
	 */

	markTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

	//mark the number of table - used in provenance scratch
	markNumOfTableAccess((QueryOperator*) op);
	bottomUpPropagateLevelAggregation((QueryOperator*) op, psPara);

	QueryOperator *newQuery = rewritePI_CS(op);
	newQuery = addTopAggForCoarse(newQuery);
//	result = update_ps_insert(newQuery, lChild, psPara, INSERT_RULE_1);

	int operation = 0;
	if (isA(lChild, SelectionOperator)) {
		DEBUG_LOG("IT IS A DELETE");
		operation = 1;
	} else if (isA(lChild, SetOperator)) {
		DEBUG_LOG("IT IS A INSERT");
		operation = 2;
	} else {
		operation = 3;
	}
	int update_ps_options = getIntOption(OPTION_UPDATE_PS_OPTION);
	char *result = NULL;

	if (operation == 1) {
		if (update_ps_options == 1) {
			result = update_ps_delete(newQuery, lChild, psPara, DELETE_RULE_2);
		} else if (update_ps_options == 2) {
			result = update_ps_delete(newQuery, lChild, psPara, DELETE_RULE_3);
		}
	} else if (operation == 2) {
		if (update_ps_options == 1) {
			result = update_ps_insert(newQuery, lChild, psPara, INSERT_RULE_1);
		} else if (update_ps_options == 2) {
			result = update_ps_insert(newQuery, lChild, psPara, INSERT_RULE_2);
		}
	} else if (operation == 3) {
		result = update_ps_update(newQuery, lChild, psPara, UPDATE_RULE_1);
	}

	return result;
}

//static void
//localTest()
//{
//	// test data type of a real db like date, in GPROM.
//	List *attrDef = getAttributes("customer");
//	for(int i = 0; i < getListLength(attrDef); i++)
//	{
//		AttributeDef * ad = (AttributeDef*) getNthOfListP(attrDef, i);
//		INFO_LOG("attr name %s, attr type %d", ad->attrName, ad->dataType);
//	}
//}

static void
modifyUncertCapTree(QueryOperator *op)
{
	if(op == NULL)
		return;
	if(isA((Node *)op, AggregationOperator))
	{
		List *aggArgs = ((AggregationOperator *) op)->aggrs;
		FOREACH(Node, n, aggArgs)
		{
			FunctionCall *fc = (FunctionCall *)n;
			char *attrRefName = ((AttributeReference *)getNthOfListP(fc->args, 0))->name;
			if(strcmp(fc->functionname, MIN_FUNC_NAME) == 0)
			{
				if(strcmp(attrRefName, "prov") == 0
						|| strcmp(attrRefName, "lb_prov") == 0
						|| strcmp(attrRefName, "ub_prov") == 0)
				{
					if(getBackend() == BACKEND_POSTGRES)
						fc->functionname = strdup(POSTGRES_BIT_AND_FUN);
				}
			}
			if(strcmp(fc->functionname, MAX_FUNC_NAME) == 0) {
				if(strcmp(attrRefName, "prov") == 0
				|| strcmp(attrRefName, "lb_prov") == 0
				|| strcmp(attrRefName, "ub_prov") == 0)
				{
					if(getBackend() == BACKEND_POSTGRES)
						fc->functionname = strdup(POSTGRES_FAST_BITOR_FUN);
				}
			}
		}
	}

	FOREACH(QueryOperator, o, op->inputs)
		modifyUncertCapTree(o);
}


static boolean
replaceTableAccessWithCompressedTableAccess(Node *node, void *state)
{
	if(node == NULL)
		return TRUE;

	if (isA(node, TableAccessOperator))
	{

		TableAccessOperator *taOp = (TableAccessOperator *) node;
		StringInfo cmprdTaOp = makeStringInfo();

		appendStringInfo(cmprdTaOp, "compressedtable_%s", taOp->tableName);

		List *attrNames = getAttributeNames(cmprdTaOp->data);
		List *attrDataTypes = getAttributeDataTypes(cmprdTaOp->data);
		List *attrDefs = getAttributes(cmprdTaOp->data);

		TableAccessOperator *cmprTableAccessOp = createTableAccessOp(cmprdTaOp->data, NULL, cmprdTaOp->data, NIL, copyList(attrNames), copyList(attrDataTypes));
		cmprTableAccessOp->op.parents = taOp->op.parents;
		((QueryOperator *) getNthOfListP(cmprTableAccessOp->op.parents, 0))->inputs = singleton(cmprTableAccessOp);

		/*
		 *	adapt proj->projExprs with compressed table attributes;
		 */

		// put attribute of compressed table into hash map;
		HashMap *hmap = NEW_MAP(Constant, Constant);
		for (int i = 0; i < getListLength(attrNames); i++)
		{
			char *name = (char *) getNthOfListP(attrNames, i);
			addToMap(hmap, (Node *) createConstString(name), (Node *) createConstInt(i));
		}
//		INFO_NODE_LOG("HASH MAP\n", hmap);

		ProjectionOperator *projOp = (ProjectionOperator *) ((QueryOperator *) getNthOfListP(cmprTableAccessOp->op.parents, 0));

		// replace with new attribute reference;
		List *projExprs = projOp->projExprs;
		FOREACH(Node, node, projExprs)
		{
			AttributeReference *ar = (AttributeReference *) node;
//			INFO_LOG("ar %s", ar->name);
			Constant *c = (Constant *) getMapString(hmap, ar->name);
//			INFO_NODE_LOG("const", c);
			int pos = INT_VALUE(c);
			// AttributeReference *newAr = createFullAttrReference(strdup(ar->name), 0, pos, 0, (DataType) getNthOfListP(attrDataTypes, pos));
			AttributeReference *newAr = createFullAttrReference(strdup(ar->name), 0, pos, 0, ((AttributeDef *)getNthOfListP(attrDefs, pos))->dataType);
			projExprs = replaceNode(projExprs, ar, newAr);
		}

		// set property "PROP_HAS_UNCERT"
		setStringProperty((QueryOperator*) cmprTableAccessOp, PROP_HAS_RANGE, (Node*) createConstBool(TRUE));
		setStringProperty((QueryOperator*) cmprTableAccessOp, PROP_HAS_UNCERT, (Node*) createConstBool(TRUE));

		return TRUE;
	}

	return visit(node, replaceTableAccessWithCompressedTableAccess, state);
}

static boolean
replaceSetBitsWithFastBitOr (Node* node, void* state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, FunctionCall))
    {
        FunctionCall *f = (FunctionCall *) node;
        if(streq(f->functionname, POSTGRES_SET_BITS_FUN))
        {
            f->functionname = strdup(POSTGRES_FAST_BITOR_FUN);
        }

        return TRUE;
    }

    return visit(node, replaceSetBitsWithFastBitOr, state);
}

static
QueryOperator* captureRewrite(ProvenanceComputation* op){
	DEBUG_NODE_BEATIFY_LOG("Provenance Computation* \n", op);
	// remove left child which is the update statemnt;
	QueryOperator *rChild = OP_RCHILD((QueryOperator*) op);
	((QueryOperator*) op)->inputs = singleton(rChild);
	DEBUG_NODE_BEATIFY_LOG("query operator after remove update statment:\n", op);
	QueryOperator* result = NULL;
	Node *coarsePara = NULL;
	psInfo *psPara = NULL;

	coarsePara = (Node*) getStringProperty((QueryOperator*) op,
	PROP_PC_COARSE_GRAINED);
	psPara = createPSInfo(coarsePara);
	DEBUG_LOG("coarse grained fragment parameters: %s",
							nodeToString((Node* ) psPara));

	markTableAccessAndAggregationUpdatePS((QueryOperator*) op, (Node*) psPara);
//	markTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

	//mark the number of table - used in provenance scratch
	markNumOfTableAccess((QueryOperator*) op);
	DEBUG_LOG("finish markNumOfTableAccess!");
	bottomUpPropagateLevelAggregation((QueryOperator*) op, psPara);
	DEBUG_LOG("finish bottomUpPropagateLevelAggregation!");
	INFO_OP_LOG("before rewrite pics", op);
	result = rewritePI_CS(op);
	result = addTopAggForCoarse(result);
//	result = addTopAggForCoarseUpdatePS(result);
	return result;
}

void removeProvAttrsList(QueryOperator *op)
{
	if(op == NULL)
		return;

	op->provAttrs = NIL;

	FOREACH(QueryOperator, o, op->inputs)
		removeProvAttrsList(o);
}

/*
 * DELETE OPERATION UPDATING
 */
static char*
update_ps_delete(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum)
{
	char *result = NULL;
	switch (ruleNum) {
	case DELETE_RULE_1:
		result = update_ps_delete_drop(query, updateQuery, PSInfo);
		break;

	case DELETE_RULE_2:
		result = update_ps_delete_approximate(query, updateQuery, PSInfo);
		break;
	case DELETE_RULE_3:
		result = update_ps_delete_accurate(query, updateQuery, PSInfo);
		break;
	}
	return result;
}

static char*
update_ps_delete_drop(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo)
{

	//GET UPDATED TABLE
	char *updatedTableName = getUpdatedTable((QueryOperator*) updateQuery);
	if (!updatedTableName)
		return NULL;

	//GET UPDATE TABLE PS
	psAttrInfo *updatedTablePSInfo = getUpdatedTablePSAttrInfo(PSInfo,
			updatedTableName);
	if (!updatedTablePSInfo)
		return NULL; // the updated table does not relate to the query

	//get all the tables in the query;
	List *tableList = getAllTables(PSInfo);

	StringInfo result = makeStringInfo();
	appendStringInfo(result, "%s", "{");
	for (int i = 0; i < LIST_LENGTH(tableList); i++) {

		char *tableName =
				(char*) ((Constant*) getNthOfListP(tableList, i))->value;
		List *psAttrInfoList = (List*) getMapString(PSInfo->tablePSAttrInfos,
				tableName);

		/*
		 *  One table could have multiple partition attributes.
		 *  Iterate to get all partition attributes and the provenance sketchs.
		 */
		for (int j = 0; j < LIST_LENGTH(psAttrInfoList); j++) {
			psAttrInfo *info = getNthOfListP(psAttrInfoList, j);

			appendStringInfo(result, "%s",
					createResultComponent(tableName, info->attrName, "NULL"));

		}
	}
	appendStringInfo(result, "%s", "}");

	return result->data;

}

static char*
update_ps_delete_approximate(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo)
{
	char *updatedTableName = getUpdatedTable((QueryOperator*) updateQuery);
	if (!updatedTableName)
		return NULL;

	List *tableList = getAllTables(PSInfo);

	StringInfo result = makeStringInfo();
	appendStringInfo(result, "%s", "{");
	//iteratoion to get all the participation tables' ps info
	for (int i = 0; i < LIST_LENGTH(tableList); i++) {

		char *tableName =
				(char*) ((Constant*) getNthOfListP(tableList, i))->value;
		List *psAttrInfoList = (List*) getMapString(PSInfo->tablePSAttrInfos,
				tableName);

		/*
		 *  One table could have multiple partition attributes.
		 *  Iterate to get all partition attributes and the provenance sketchs.
		 */
		for (int j = 0; j < LIST_LENGTH(psAttrInfoList); j++) {
			psAttrInfo *info = getNthOfListP(psAttrInfoList, j);

			appendStringInfo(result, "%s",
					createResultComponent(tableName, info->attrName,
							bitSetToString(info->BitVector)));

		}
	}
	appendStringInfo(result, "%s", "}");

	return result->data;
}

static char*
update_ps_delete_accurate(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo)
{
	/*
	 *	Capture PS of ((PS_r - Delta_r) join PS_s)
	 */

	/*
	 * TODO: (optimization) check if the updated table is related to the sql, for example: updated r but the query run on s and t;
	 *
	 */

	//GET ALL THE TABLE ACCESS OPERATOR TO REWRITE
	List *taList = NIL;
	getTableAccessOps((Node*) query, &taList);

	char *updatedTable = getUpdatedTable(updateQuery);
	for (int i = 0; i < LIST_LENGTH(taList); i++) {
		TableAccessOperator *taOp = (TableAccessOperator*) getNthOfListP(taList,
				i);

		List *parent = ((QueryOperator*) taOp)->parents;
		QueryOperator *rewriteOp = rewriteTableAccessOperator(taOp, PSInfo);
		((QueryOperator*) getHeadOfListP(parent))->inputs = replaceNode(
				((QueryOperator*) getHeadOfListP(parent))->inputs, taOp,
				rewriteOp);

		if (streq(taOp->tableName, updatedTable)) {
			// in this contidional case: construct PS_r - Delta_r;
			Node *notCond = copyObject(
					((SelectionOperator*) updateQuery)->cond);
			List *conds = LIST_MAKE(notCond,
					((SelectionOperator* )rewriteOp)->cond);
			((SelectionOperator*) rewriteOp)->cond = andExprList(conds);
			DEBUG_NODE_BEATIFY_LOG("WHAT IS THE SEL CO:",
					((SelectionOperator* ) rewriteOp)->cond);

		}
	}

	//GET CAPTURE SQL
//	DEB
	DEBUG_LOG("BEGIN SERIALIZE");
	char *capSql = serializeOperatorModel((Node*) query);

	DEBUG_LOG("END SERIALIZE");
	//CAPTURE NEW PS
	List *attrNames = getAttrNames(query->schema);
	DEBUG_LOG("BEGIN CAP");
	HashMap *psMap = getPS(capSql, attrNames);
	DEBUG_NODE_BEATIFY_LOG("WHAT IS THE NWE PS", psMap);

	//GET RESULT AND RETURN
	StringInfo result = makeStringInfo();
	List *keys = getKeys(PSInfo->tablePSAttrInfos);
	for (int i = 0; i < LIST_LENGTH(keys); i++) {
		char *tableName = (char*) ((Constant*) getNthOfListP(keys, i))->value;
		List *psAttrInfos = (List*) getMapString(PSInfo->tablePSAttrInfos,
				tableName);

		for (int j = 0; j < LIST_LENGTH(psAttrInfos); j++) {
			psAttrInfo *info = (psAttrInfo*) getNthOfListP(psAttrInfos, j);
			char *attrName = info->attrName;
			StringInfo str = makeStringInfo();
			appendStringInfo(str, "\"%s_%s_%s", "prov", tableName, attrName);

			List *keys2 = getKeys(psMap);
			for (int k = 0; k < LIST_LENGTH(keys2); k++) {
				char *ss = ((Constant*) getNthOfListP(keys2, k))->value;
				if (strncmp(str->data, ss, strlen(str->data)) == 0) {

					Constant *newPSValue = (Constant*) getMapString(psMap, ss);
					if (getBackend() == BACKEND_POSTGRES) {
						appendStringInfo(result, "%s",
								createResultComponent(tableName, attrName,
										(char*) newPSValue->value));
						continue;
					}
					int bitSetLength = info->BitVector->length;
					BitSet *bitSet = newBitSet(bitSetLength);
					int bitSetIntValue = *((int*) newPSValue->value);

					int index = info->BitVector->length - 1;

					//set bit for each position;
					while (bitSetIntValue > 0) {
						if (bitSetIntValue % 2 == 1) {
							setBit(bitSet, index, 1);
						}
						index--;
						bitSetIntValue /= 2;
					}
					char *bits = bitSetToString(bitSet);

					//reverse the bits;
					for (index = 0; index < bitSetLength / 2; index++) {
						bits[index] ^= bits[bitSetLength - index - 1];
						bits[bitSetLength - index - 1] ^= bits[index];
						bits[index] ^= bits[bitSetLength - index - 1];
					}

					bitSet = stringToBitset(bits);

					appendStringInfo(result, "%s",
							createResultComponent(tableName, attrName,
									bitSetToString(bitSet)));

				}
			}
		}
	}

	return result->data;
}

/*
 * INSERT OPRATION UPDATING
 */

static char*
update_ps_insert(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum)
{
	char *result = NULL;
	switch (ruleNum) {
	case INSERT_RULE_1:
		result = update_ps_insert_approximate(query, updateQuery, PSInfo);
		break;
	case INSERT_RULE_2:
		result = update_ps_insert_accurate(query, updateQuery, PSInfo);
		break;
	}
	return result;
}
static char*
update_ps_insert_accurate(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo)
{
	//delta tuple join whole table;

	char *updatedTable = getUpdatedTable(updateQuery);
	DEBUG_LOG("the update tables is:--> %s", updatedTable);
	ProjectionOperator *proOpDummy = createDummyProjTree(updateQuery);

	//test apply min-max can affect the time;
	psInfo *reservedPS = (psInfo*) copyObject(PSInfo);
//	int skipped = skipFragsBasedOnPS(updatedTable, PSInfo, updateQuery);
	int skipped = -1;

	DEBUG_LOG("SKIPPED: %d\n", skipped);
	DEBUG_LOG("BIT SIZE%d\n", getIntOption(OPTION_BIT_VECTOR_SIZE));
	//Skip all the fragments, then just return the original PS
	if (skipped == getIntOption(OPTION_BIT_VECTOR_SIZE)) {
		//TODO returned;
		List *tableList = getAllTables(PSInfo);

		StringInfo result = makeStringInfo();
		appendStringInfo(result, "%s", "{");
		//iteratoion to get all the participation tables' ps info
		for (int i = 0; i < LIST_LENGTH(tableList); i++) {

			char *tableName =
					(char*) ((Constant*) getNthOfListP(tableList, i))->value;
			List *psAttrInfoList = (List*) getMapString(
					reservedPS->tablePSAttrInfos, tableName);

			/*
			 *  One table could have multiple partition attributes.
			 *  Iterate to get all partition attributes and the provenance sketchs.
			 */
			for (int j = 0; j < LIST_LENGTH(psAttrInfoList); j++) {
				psAttrInfo *info = getNthOfListP(psAttrInfoList, j);

				appendStringInfo(result, "%s",
						createResultComponent(tableName, info->attrName,
								bitSetToString(info->BitVector)));

			}
		}
		appendStringInfo(result, "%s", "}");
		//			STOP_TIMER("UPDATEPS_ins_app");
		return result->data;
	}

	//end test
	DEBUG_NODE_BEATIFY_LOG("WHAT IS THE DUMMY PROJ", proOpDummy);
	List *taList = NIL;
	getTableAccessOps((Node*) query, &taList);

	for (int i = 0; i < LIST_LENGTH(taList); i++) {
		TableAccessOperator *taOp = (TableAccessOperator*) getNthOfListP(taList,
				i);
		if (streq(taOp->tableName, updatedTable)) {

			List *parent = ((QueryOperator*) taOp)->parents;
			((QueryOperator*) getHeadOfListP(parent))->inputs = replaceNode(
					((QueryOperator*) getHeadOfListP(parent))->inputs, taOp,
					proOpDummy);

			proOpDummy->op.parents = singleton(parent);
		}
	}

	INFO_OP_LOG("new construct query", query);
	DEBUG_LOG("BEGIN CAP");
	char *capSql = serializeOperatorModel((Node*) query);
	DEBUG_LOG("WHAT IS THE CALSQL %s", capSql);

	List *attrNames = getAttrNames(query->schema);
	DEBUG_LOG("PS Attr Names : %s", stringListToString(attrNames));
	HashMap *psMap = getPS(capSql, attrNames);

	DEBUG_NODE_BEATIFY_LOG("WHAT IS THE NEW PS:", psMap);
	// TODO Here we can optimize by check the result of psMap, if all the ps is 0, which means that there are no new fragment to be added.
	if (getBackend() == BACKEND_POSTGRES) {
		StringInfo results = makeStringInfo();
		appendStringInfo(results, "%s", "{");
//		bitOrResultsPostgres(HashMap *old, HashMap *new, StringInfo *result)
		bitOrResultsPostgres(PSInfo->tablePSAttrInfos, psMap, &results);
		appendStringInfo(results, "%s", "}");
		return results->data;
	}
	StringInfo result = makeStringInfo();
	appendStringInfo(result, "%s", "{");

	//min-max optimized
	bitOrResults(reservedPS->tablePSAttrInfos, psMap, &result);

	//bitOrResults(PSInfo->tablePSAttrInfos, psMap, &result);
	appendStringInfo(result, "%s", "}");

	return result->data;

//	return NULL;
}
static char*
update_ps_insert_approximate(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo)
{

	START_TIMER("UPDATEPS_ins_app");
	char *updatedTable = getUpdatedTable(updateQuery);

	//check the Dalta tuple's ps, and directly set it to '1' if it is '0';

	List *psAttrInfos = (List*) getMapString(PSInfo->tablePSAttrInfos,
			updatedTable);
	char *psAttr = NULL;
//	int index = -1;
	psAttrInfo *updatedTableInfo = NULL;
	for (int i = 0; i < LIST_LENGTH(psAttrInfos); i++) {
		psAttrInfo *info = (psAttrInfo*) getNthOfListP(psAttrInfos, i);
		psAttr = info->attrName;
		updatedTableInfo = info;
	}

	int psAttrValue = -1;
	FOREACH(QueryOperator, o, updateQuery->inputs)
	{
		if (isA(o, ConstRelOperator)) {
			List *attrList = o->schema->attrDefs;
			for (int i = 0; i < LIST_LENGTH(attrList); i++) {
				AttributeDef *def = (AttributeDef*) getNthOfListP(attrList, i);
				if (streq(def->attrName, psAttr)) {
					List *values = ((ConstRelOperator*) o)->values;
					DEBUG_NODE_BEATIFY_LOG("what is nodessss:",
							(Constant* ) getNthOfListP(values, i));
					psAttrValue =
							*((int*) ((Constant*) getNthOfListP(values, i))->value);

				}
			}
			break;
		}
	}
//	DEBUG_NODE_BEATIFY_LOG("previous psInfo:", PSInfo);
	List *ranges = updatedTableInfo->rangeList;
	for (int i = 1; i < LIST_LENGTH(ranges); i++) {
		Constant *previous = (Constant*) getNthOfListP(ranges, i - 1);
		Constant *current = (Constant*) getNthOfListP(ranges, i);

		int pValue = *((int*) previous->value);
		int cValue = *((int*) current->value);

		if (psAttrValue >= pValue && psAttrValue < cValue) {
//			DEBUG_LOG("the modify position: %d", i - 1);
			setBit(updatedTableInfo->BitVector, i - 1, 1);
		}

	}

	ProjectionOperator *proOpDummy = createDummyProjTree(updateQuery);

	psInfo *reservedPS = (psInfo*) copyObject(PSInfo);

	/*
	 * Here replace reversePSInfo(PSInfo, updateTable);
	 */
//	reversePSInfo(PSInfo, updatedTable);
	int skipped = skipFragsBasedOnPS(updatedTable, PSInfo, updateQuery);

	DEBUG_LOG("SKIPPED: %d\n", skipped);
	DEBUG_LOG("BIT SIZE%d\n", getIntOption(OPTION_BIT_VECTOR_SIZE));
	//Skip all the fragments, then just return the original PS
	if (skipped == getIntOption(OPTION_BIT_VECTOR_SIZE)) {
		//TODO returned;
		List *tableList = getAllTables(PSInfo);

		StringInfo result = makeStringInfo();
		appendStringInfo(result, "%s", "{");
		//iteratoion to get all the participation tables' ps info
		for (int i = 0; i < LIST_LENGTH(tableList); i++) {

			char *tableName =
					(char*) ((Constant*) getNthOfListP(tableList, i))->value;
			List *psAttrInfoList = (List*) getMapString(
					reservedPS->tablePSAttrInfos, tableName);

			for (int j = 0; j < LIST_LENGTH(psAttrInfoList); j++) {
				psAttrInfo *info = getNthOfListP(psAttrInfoList, j);

				appendStringInfo(result, "%s",
						createResultComponent(tableName, info->attrName,
								bitSetToString(info->BitVector)));

			}
		}
		appendStringInfo(result, "%s", "}");
//			STOP_TIMER("UPDATEPS_ins_app");
		return result->data;
	}

	List *taList = NIL;
	getTableAccessOps((Node*) query, &taList);

	//For each TableAccessOperator, rewrite it with the 1 as 0 and 0 as 1 or make a dummy table only when it is the updated table;
	for (int i = 0; i < LIST_LENGTH(taList); i++) {
		TableAccessOperator *taOp = (TableAccessOperator*) getNthOfListP(taList,
				i);
		if (!streq(taOp->tableName, updatedTable)) {
			//rewrite current tableaccess; with specific ranges;
			List *parent = ((QueryOperator*) taOp)->parents;
			QueryOperator *rewriteOp = rewriteTableAccessOperator(
					(TableAccessOperator*) taOp, PSInfo);
			((QueryOperator*) getHeadOfListP(parent))->inputs = replaceNode(
					((QueryOperator*) getHeadOfListP(parent))->inputs, taOp,
					rewriteOp);

		} else {
			List *parent = ((QueryOperator*) taOp)->parents;
			((QueryOperator*) getHeadOfListP(parent))->inputs = replaceNode(
					((QueryOperator*) getHeadOfListP(parent))->inputs, taOp,
					proOpDummy);

			proOpDummy->op.parents = singleton(parent);
		}
	}
	STOP_TIMER("UPDATEPS_ins_app");
	DEBUG_LOG("BEGIN SERIALIZE");
	char *capSql = serializeOperatorModel((Node*) query);

	//RUN
	List *attrNames = getAttrNames(query->schema);
	DEBUG_LOG("PS Attr Names : %s", stringListToString(attrNames));

	DEBUG_LOG("BEGIN CAP");
	HashMap *psMap = getPS(capSql, attrNames);
	DEBUG_LOG("END CAP");
	// TODO Here we can optimize by check the result of psMap, if all the ps is 0, which means that there are no new fragment to be added.

	StringInfo result = makeStringInfo();
	appendStringInfo(result, "%s", "{");
	if (getBackend() == BACKEND_POSTGRES) {
		bitOrResultsPostgres(reservedPS->tablePSAttrInfos, psMap, &result);
		appendStringInfo(result, "%s", "}");
		return result->data;
	}
	bitOrResults(reservedPS->tablePSAttrInfos, psMap, &result);
	appendStringInfo(result, "%s", "}");

	return result->data;
}

/*
 * UPDATE OPERATION UPDATING
 */

static char*
update_ps_update(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum)
{
	char *result = NULL;
	switch (ruleNum) {
	case UPDATE_RULE_1:
		result = update_ps_del_ins_1(query, updateQuery, PSInfo);
		break;

	case UPDATE_RULE_2:
		break;
	case UPDATE_RULE_3:
		break;
	case UPDATE_RULE_4:
		break;
	}
	return result;
}

static char*
update_ps_del_ins_1(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo)
{
	/*
	 * DELETE: APPROXIMATE
	 * INSERT: APPROXIMATE
	 */

	// CHECK IF THE UPDATED TABLE IS RELATED TO TABLES IN THE QUERIES
	char *updatedTable = getUpdatedTable(updateQuery);
	List *allParticipatedTables = NIL;
	getTableAccessOps((Node*) query, &allParticipatedTables);
	boolean isFind = FALSE;
	FOREACH(TableAccessOperator, op, allParticipatedTables)
	{
		if (streq(op->tableName, updatedTable)) {
			isFind = TRUE;
			break;
		}
	}
	if (!isFind) {
		return NULL;
	}

	//DELETE: KEEP THE PS
	//INSERT:

	List *projExprs = ((ProjectionOperator*) updateQuery)->projExprs;
	List *conds = NIL;
	for (int i = 0; i < LIST_LENGTH(projExprs); i++) {
		Node *curr = (Node*) getNthOfListP(projExprs, i);
		if (isA(curr, CaseExpr)) {
			List *whenClause = ((CaseExpr*) curr)->whenClauses;
			for (int j = 0; j < LIST_LENGTH(whenClause); j++) {
				Node *node = getNthOfListP(whenClause, j);
				if (isA(node, CaseWhen)) {
					DEBUG_LOG("make condds");
					Operator *op = createOpExpr("=",
							LIST_MAKE(copyObject(((CaseExpr* )curr)->elseRes),
									copyObject(((CaseWhen* ) node)->then)));
					conds = appendToTailOfList(conds, op);
				}
			}
		}
	}

	//TODO HERE CAN OPTIMIZE, CHECK IF THE NEW INSERT TUPLES BELONGS TO PS_r BASED ON PS ATTRIBUTE, IF YES, DELTA_r JOIN X(PS_S) ELSE DELTA_r JOIN S.

	psInfo *reservedPS = (psInfo*) copyObject(PSInfo);
	reversePSInfo(PSInfo, updatedTable);

	for (int i = 0; i < LIST_LENGTH(allParticipatedTables); i++) {

		TableAccessOperator *taOp = (TableAccessOperator*) getNthOfListP(
				allParticipatedTables, i);
		if (streq(updatedTable, taOp->tableName)) {
			List *parent = ((QueryOperator*) taOp)->parents;
			Node *selAndConds = andExprList(conds);
			SelectionOperator *selOp = createSelectionOp(selAndConds,
					(QueryOperator*) taOp, parent,
					getAttrNames(taOp->op.schema));
			((QueryOperator*) getHeadOfListP(parent))->inputs = replaceNode(
					((QueryOperator*) getHeadOfListP(parent))->inputs, taOp,
					selOp);
		}

	}

	char *capSql = serializeOperatorModel((Node*) query);

	DEBUG_LOG("%s", capSql);
	List *attrNames = getAttrNames(query->schema);
	DEBUG_LOG("PS Attr Names : %s", stringListToString(attrNames));
	HashMap *psMap = getPS(capSql, attrNames);

	DEBUG_NODE_BEATIFY_LOG("WHAT IS NEW PS", psMap);
	StringInfo result = makeStringInfo();
	appendStringInfo(result, "%s", "{");
	bitOrResults(reservedPS->tablePSAttrInfos, psMap, &result);
	appendStringInfo(result, "%s", "}");

	return result->data;

	return sql;
}

/*
 * OTHER METHODS
 */

int
skipFragsBasedOnPS(char *updatedTable, psInfo *PSInfo,
		QueryOperator *updateQuery)
{
	int bitVectorSize = getIntOption(OPTION_BIT_VECTOR_SIZE);
	DEBUG_LOG(" the bit vector size: %d", bitVectorSize);
//	List joinAttrList;

	ConstRelOperator *cro;
	FOREACH(QueryOperator, o, updateQuery->inputs)
	{
		if (isA(o, ConstRelOperator)) {
			cro = (ConstRelOperator*) o;

			break;
		}
	}

	int joinAttrValue;
	int everyFragSize = 10000000 / bitVectorSize;
	int minMaxSteps;

	psAttrInfo *info;
	char *bitList;
	if (streq(updatedTable, "RR") || streq(updatedTable, "rr")) {
		joinAttrValue = *((int*) ((Constant*) getNthOfListP(cro->values,
		LIST_LENGTH(cro->values) - 1))->value);
		minMaxSteps = everyFragSize / 100;
		List *infos = (List*) getMapString(PSInfo->tablePSAttrInfos, "ss");
		info = (psAttrInfo*) getHeadOfListP(infos);
		bitList = bitSetToString(info->BitVector);
	} else {
		joinAttrValue = *((int*) ((Constant*) getNthOfListP(cro->values,
		LIST_LENGTH(cro->values) - 2))->value);
		minMaxSteps = everyFragSize / 1000;
		List *infos = (List*) getMapString(PSInfo->tablePSAttrInfos, "rr");
		info = (psAttrInfo*) getHeadOfListP(infos);
		bitList = bitSetToString(info->BitVector);
	}
//
//	DEBUG_LOG("WHAT IS THE JOIN VALUE: %d", joinAttrValue);
//	DEBUG_LOG("everyFragSize %d", everyFragSize);
//	DEBUG_LOG("minMaxSteps %d", minMaxSteps);
	DEBUG_LOG("bitsetstring %s, %d", bitList, strlen(bitList));
	List *joinAttrList = NIL;
	// rr g: 100000; ss t: every 100 in a group;

	for (int i = 1; i <= bitVectorSize; i++) {
		//min
		Constant *min = createConstInt((i - 1) * minMaxSteps + 1);
		Constant *max = createConstInt(i * minMaxSteps);
		joinAttrList = appendToTailOfList(joinAttrList, min);
		joinAttrList = appendToTailOfList(joinAttrList, max);
	}

	//compare the value to skip
	int skipped = bitVectorSize;
	for (int i = 0; i < bitVectorSize; i++) {
		if (bitList[i] == '1') {
			setBit(info->BitVector, i, 0);
		} else {
			int min =
					*((int*) ((Constant*) getNthOfListP(joinAttrList, 2 * i))->value);
			int max = *((int*) ((Constant*) getNthOfListP(joinAttrList,
					2 * i + 1))->value);

			if (joinAttrValue >= min && joinAttrValue <= max) {
				setBit(info->BitVector, i, 1);
				skipped--;
			}
		}
	}

	DEBUG_LOG("the skipped bitstring:%s", bitSetToString(info->BitVector));
	return skipped;

}

static BitSet*
bitOrResults(HashMap *old, HashMap *new, StringInfo *result)
{

	List *keys = getKeys(old);

	for (int i = 0; i < LIST_LENGTH(keys); i++) {
		char *tableName = (char*) ((Constant*) getNthOfListP(keys, i))->value;

		List *psAttrInfos = (List*) getMapString(old, tableName);
		for (int j = 0; j < LIST_LENGTH(psAttrInfos); j++) {
			psAttrInfo *info = (psAttrInfo*) getNthOfListP(psAttrInfos, j);
			char *attrName = info->attrName;

			StringInfo str = makeStringInfo();
			appendStringInfo(str, "\"%s_%s_%s", "prov", tableName, attrName);

			List *keys2 = getKeys(new);
			for (int k = 0; k < LIST_LENGTH(keys2); k++) {
				char *ss = ((Constant*) getNthOfListP(keys2, k))->value;
				if (strncmp(str->data, ss, strlen(str->data)) == 0) {
					Constant *newPSValue = (Constant*) getMapString(new, ss);
					int bitSetLength = info->BitVector->length;
					BitSet *bitSet = newBitSet(bitSetLength);
					DEBUG_NODE_BEATIFY_LOG("what is the generated bit set:",
							bitSet);
					unsigned long bitSetIntValue =
							*((unsigned long*) newPSValue->value);
					DEBUG_LOG("what is the unsigned value: %ld\n",
							bitSetIntValue);

					int index = 0;
					while (bitSetIntValue > 0) {
						if (bitSetIntValue % 2 == 1) {
							setBit(bitSet, index, 1);
						} else {
							setBit(bitSet, index, 0);
						}
						index++;
						bitSetIntValue /= 2;

					}
					while (index < bitSetLength) {
						setBit(bitSet, index++, 0);
					}

					info->BitVector = bitOr(info->BitVector, bitSet);
					appendStringInfo(*result, "%s",
							createResultComponent(tableName, attrName,
									bitSetToString(info->BitVector)));

				}
			}

		}
	}
	return NULL;
}

static char*
getUpdatedTable(QueryOperator *op)
{

	FOREACH(QueryOperator, operator, op->inputs)
	{
		if (isA(operator, TableAccessOperator)) {
			return ((TableAccessOperator*) operator)->tableName;
		}
	}

	return NULL;

}

static psAttrInfo*
getUpdatedTablePSAttrInfo(psInfo *PSInfo, char *tableName)
{

	char *tmp = tableName;

	while (*tmp) {
		*tmp = tolower(*tmp);
		tmp++;
	}

	if (hasMapStringKey(PSInfo->tablePSAttrInfos, tableName)) {
		return (psAttrInfo*) getMapString(PSInfo->tablePSAttrInfos, tableName);
	}

	return NULL;
}

static List*
getAllTables(psInfo *PSInfo)
{
	return getKeys(PSInfo->tablePSAttrInfos);
}

static char*
createResultComponent(char *tableName, char *psAttr, char *ps)
{

	StringInfo result = makeStringInfo();

	appendStringInfo(result, "(%s[%s:%s])", tableName, psAttr, ps);

	return result->data;
}

static ProjectionOperator*
createDummyProjTree(QueryOperator *updateQuery)
{

	List *attrNames;
	List *attrValues;
	List *attrDefs;
	FOREACH(QueryOperator, o, updateQuery->inputs)
	{
		if (isA(o, ConstRelOperator)) {
			ConstRelOperator *cro = (ConstRelOperator*) o;
			attrNames = cro->op.schema->attrDefs;
			attrValues = cro->values;
			attrDefs = cro->op.schema->attrDefs;
			// TODO get attr data type
			break;
		}
	}

	ProjectionOperator *projOp = NULL;
	if (getBackend() == BACKEND_ORACLE) {
		TableAccessOperator *taOp = createTableAccessOp("DUAL", NULL, "DUAL",
		NIL, singleton("DUMMY"), singletonInt(DT_STRING));
		projOp = createProjectionOp(attrValues, (QueryOperator*) taOp, NIL,
				attrNames);
		projOp->op.schema->attrDefs = attrDefs;
	}

	if (getBackend() == BACKEND_POSTGRES) {
		char *tableName = ((TableAccessOperator*) getNthOfListP(
				updateQuery->inputs, 0))->tableName;

		List *dataTypes =
				getDataTypes(
						((TableAccessOperator*) getNthOfListP(
								updateQuery->inputs, 0))->op.schema);
		List *attNames =
				getAttrNames(
						((TableAccessOperator*) getNthOfListP(
								updateQuery->inputs, 0))->op.schema);
		TableAccessOperator *taOp = createTableAccessOp(tableName, NULL,
				tableName, NIL, attNames, dataTypes);

//		SelectionOperator* selOp = createSelectionOp();
		projOp = createProjectionOp(attrValues, (QueryOperator*) taOp, NIL,
				attrNames);
		projOp->op.schema->attrDefs = attrDefs;

		Constant *limitExpr = createConstInt(1);

		LimitOperator *limitOp = createLimitOp((Node*) limitExpr, NULL,
				(QueryOperator*) projOp, NIL);
		projOp->op.parents = singleton(limitOp);
		return (ProjectionOperator*) limitOp;
//		projOp->op.schema->attrDefs = attrDefs;
//		((TableAccessOperator*)getNthOfListP(projOp->op.inputs, 0))->op.parents = NIL;
//		projOp->op.inputs = NIL;
	}
	INFO_OP_LOG("wht is the proj", projOp);
	return projOp;
}

static void
reversePSInfo(psInfo *PSInfo, char *updatedTable)
{
	HashMap *allAttrInfos = PSInfo->tablePSAttrInfos;

	List *keys = getKeys(allAttrInfos);

	for (int i = 0; i < LIST_LENGTH(keys); i++) {
		char *tableName = (char*) ((Constant*) getNthOfListP(keys, i))->value;

		if (!streq(tableName, updatedTable)) {
			List *psAttrInfoList = (List*) getMapString(allAttrInfos,
					tableName);

			for (int j = 0; j < LIST_LENGTH(psAttrInfoList); j++) {

				psAttrInfo *info = (psAttrInfo*) getNthOfListP(psAttrInfoList,
						j);

				char *bitset = bitSetToString(info->BitVector);
				for (int index = 0; index < strlen(bitset); index++) {
					if (bitset[index] == '0') {
						setBit(info->BitVector, index, 1);
					} else {
						setBit(info->BitVector, index, 0);
					}
				}
			}
		}
	}

}

static boolean
getTableAccessOps(Node *op, List **l)
{
	if (op == NULL)
		return TRUE;

	if (isA(op, TableAccessOperator)) {
		*l = appendToTailOfList(*l, op);
	}

	return visit((Node*) op, getTableAccessOps, l);
}

static QueryOperator*
rewriteTableAccessOperator(TableAccessOperator *op, psInfo *PSInfo)
{

	List *attrInfos = (List*) getMapString(PSInfo->tablePSAttrInfos,
			op->tableName);

	List *selAndList = NIL;
	for (int i = 0; i < LIST_LENGTH(attrInfos); i++) {
		psAttrInfo *info = (psAttrInfo*) getNthOfListP(attrInfos, i);

		List *rangeList = info->rangeList;
		char *bitSet = bitSetToString(info->BitVector);

		AttributeReference *psAttrRef = createAttrsRefByName(
				(QueryOperator*) op, info->attrName);

		boolean findCond = FALSE;
		int start = -1;
		int end = -1;
		int index = 0;
		for (int j = 0; j < strlen(bitSet); j++) {
			if (bitSet[j] == '1') {
				index++;
				if (findCond == FALSE) {
					findCond = TRUE;
					start = j;
					end = j;
					continue;
				}
				if (bitSet[j - 1] == '1') {
					end = j;

				} else {

					Operator *lOp = createOpExpr(">=",
							LIST_MAKE(copyObject(psAttrRef),
									copyObject(
											getNthOfListP(rangeList, start))));
					Operator *rOp = createOpExpr("<",
							LIST_MAKE(copyObject(psAttrRef),
									copyObject(
											getNthOfListP(rangeList,
													end + 1))));
					Node *andExp = andExprList(LIST_MAKE(lOp, rOp));
					selAndList = appendToTailOfList(selAndList, andExp);

					start = j;
					end = j;
				}

			}

			if (j == strlen(bitSet) - 1) {
				DEBUG_LOG("BEGIN and END %d, %d", start, end);
				if (findCond) {
					Operator *lOp = createOpExpr(">=",
							LIST_MAKE(copyObject(psAttrRef),
									copyObject(
											getNthOfListP(rangeList, start))));
					Operator *rOp = createOpExpr("<",
							LIST_MAKE(copyObject(psAttrRef),
									copyObject(
											getNthOfListP(rangeList,
													end + 1))));
					Node *andExp = andExprList(LIST_MAKE(lOp, rOp));
					selAndList = appendToTailOfList(selAndList, andExp);

				}
			}
		}
		DEBUG_LOG("the number of ranges %d", index);
	}

	//TODO check the adn list is 1 or longer
	DEBUG_LOG("selAndList: %d", LIST_LENGTH(selAndList));
	Node *selOrExp = NULL;
	if (LIST_LENGTH(selAndList) == 0) {
		Operator *op = createOpExpr(">=",
				LIST_MAKE(createConstInt(1), createConstInt(2)));
		selOrExp = (Node*) op;
	} else if (LIST_LENGTH(selAndList) == 1) {
//		selOrExp = (Node*) getHeadOfList(selAndList);
		selOrExp = (Node*) getNthOfListP(selAndList, 0);
	} else {
		selOrExp = orExprList(selAndList);
	}
//	Node *selOrExp = orExprList(selAndList);
	SelectionOperator *selOp = createSelectionOp((Node*) selOrExp,
			(QueryOperator*) op, op->op.parents,
			getQueryOperatorAttrNames((QueryOperator*) op));
//	INFO_OP_LOG("selecction operator", selOp);
	op->op.parents = singleton(selOp);
	return (QueryOperator*) selOp;
}

void
bitOrResultsPostgres(HashMap *old, HashMap *new, StringInfo *result)
{
	DEBUG_NODE_BEATIFY_LOG("previous hashMap", old);
	DEBUG_NODE_BEATIFY_LOG("new hashMap", new);
	List *keys = getKeys(old);

	for (int i = 0; i < LIST_LENGTH(keys); i++) {
		char *tableName = (char*) ((Constant*) getNthOfListP(keys, i))->value;

		List *psAttrInfos = (List*) getMapString(old, tableName);
		for (int j = 0; j < LIST_LENGTH(psAttrInfos); j++) {
			psAttrInfo *info = (psAttrInfo*) getNthOfListP(psAttrInfos, j);
			char *attrName = info->attrName;

			StringInfo str = makeStringInfo();
			appendStringInfo(str, "\"%s_%s_%s", "prov", tableName, attrName);

			List *keys2 = getKeys(new);
			for (int k = 0; k < LIST_LENGTH(keys2); k++) {
				char *ss = ((Constant*) getNthOfListP(keys2, k))->value;
				if (strncmp(str->data, ss, strlen(str->data)) == 0) {
					BitSet *oldVector = info->BitVector;
					DEBUG_LOG("begin");
					BitSet *newVector = (BitSet*) stringToBitset(
							(char*) ((Constant*) getMapString(new, ss))->value);
					oldVector = bitOr(oldVector, newVector);

					appendStringInfo(*result, "%s",
							createResultComponent(tableName, attrName,
									bitSetToString(oldVector)));
				}
			}
		}
	}
}

void
compressTable(char *tablename, char *psAttr, List *ranges)
{
	// check the required table's exsitance
	// build compress queiry
	// execute to get the cdb
}

