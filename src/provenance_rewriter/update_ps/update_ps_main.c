/*-----------------------------------------------------------------------------
 *
 * update_ps_main.c
 *
 *
 *      AUTHOR: Pengyuan Li
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

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

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"

#include "sql_serializer/sql_serializer.h"
#include "metadata_lookup/metadata_lookup.h"

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
static char* update_ps_delete(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum);
static char* update_ps_delete_drop(QueryOperator *query,
		QueryOperator *updateQuery, psInfo *PSInfo);
static char* update_ps_delete_approximate(QueryOperator *query,
		QueryOperator *updateQuery, psInfo *PSInfo);
static char* update_ps_delete_accurate(QueryOperator *query,
		QueryOperator *updateQuery, psInfo *PSInfo);

static char* update_ps_insert(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum);
static char* update_ps_insert_approximate(QueryOperator *query,
		QueryOperator *updateQuery, psInfo *PSInfo);

static char* getUpdatedTable(QueryOperator *op);
static psAttrInfo* getUpdatedTablePSAttrInfo(psInfo *PSInfo, char *tableName);
static List* getAllTables(psInfo *PSInfo);
static char* createResultComponent(char *tableName, char *psAttr, char *ps);
static ProjectionOperator* createDummyProjTree(QueryOperator *updateQuery);
static void reversePSInfo(psInfo *PSInfo, char *updatedTable);
static QueryOperator* rewriteTableAccessOperator(TableAccessOperator *op,
		psInfo *PSInfo);
static boolean getTableAccessOps(Node *op, List **l);
static BitSet* bitOrResults(HashMap *old, HashMap *new, StringInfo* result);

/*
 * Function Implementation
 */
char*
update_ps(ProvenanceComputation *qbModel) {

	//initialize some parameters to get the ps, left chile(update statement) and right child(query);
	ProvenanceComputation *op = qbModel;
	Node *coarsePara = NULL;
	psInfo *psPara = NULL;

	coarsePara = (Node*) getStringProperty((QueryOperator*) op,
	PROP_PC_COARSE_GRAINED);
	psPara = createPSInfo(coarsePara);

	DEBUG_LOG("use coarse grained fragment parameters: %s",
			nodeToString((Node* ) psPara));

	/*
	 * get the left and right childred respectively;
	 * left child is a update statement
	 * right child is a normal query
	 */

	QueryOperator *op1 = (QueryOperator*) op;
	QueryOperator *rChild = OP_RCHILD(op1);
	QueryOperator *lChild = (QueryOperator*) OP_LCHILD(op1);
	op1->inputs = singleton(rChild);

//	QueryOperator *query = rewritePI_CS(op);
//	removeParent(rChild, (QueryOperator*) op);
	removeParent(lChild, (QueryOperator*) op);

	DEBUG_NODE_BEATIFY_LOG(
			"\n#######################\n \t Query:\n#######################\n",
			rChild);
//	DEBUG_NODE_BEATIFY_LOG(
//			"\n#######################\n \t PS INFO:\n#######################\n",
//			psPara);
	DEBUG_NODE_BEATIFY_LOG(
			"\n#######################\n \t Update query:\n#######################\n",
			lChild);

	char *result;

//	result = update_ps_delete(rChild, lChild, psPara, DELETE_RULE_1);
	result = update_ps_delete(rChild, lChild, psPara, DELETE_RULE_2);
//	result = update_ps_delete(rChild, lChild, psPara, DELETE_RULE_3);

	markTableAccessAndAggregation((QueryOperator*) op, (Node*) psPara);

	//mark the number of table - used in provenance scratch
	markNumOfTableAccess((QueryOperator*) op);
	bottomUpPropagateLevelAggregation((QueryOperator*) op, psPara);

	DEBUG_LOG("START HERE");
	QueryOperator *newQuery = rewritePI_CS(op);
	INFO_OP_LOG("NEW QUERY: ", newQuery);
	DEBUG_LOG("FINISH 1");
	newQuery = addTopAggForCoarse(newQuery);
	INFO_OP_LOG("NEW QUERY IS: ", newQuery);
	DEBUG_LOG("FINISH 2");
	DEBUG_LOG("xxxxxxxxxxxxxxxx%s", result);
	result = update_ps_insert(newQuery, lChild, psPara, INSERT_RULE_1);

	return result;
}

static char*
update_ps_delete(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum) {
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
		psInfo *PSInfo) {

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
		psInfo *PSInfo) {
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
		psInfo *PSInfo) {

	return NULL;
}

static char*
update_ps_insert(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo, int ruleNum) {
	char *result = NULL;
	switch (ruleNum) {
	case INSERT_RULE_1:
		result = update_ps_insert_approximate(query, updateQuery, PSInfo);
		break;
	case INSERT_RULE_2:
//		result = NULL;
		break;
	}
	return result;
}

static char*
update_ps_insert_approximate(QueryOperator *query, QueryOperator *updateQuery,
		psInfo *PSInfo) {
	char *updatedTable = getUpdatedTable(updateQuery);

	ProjectionOperator *proOpDummy = createDummyProjTree(updateQuery);


	psInfo *reservedPS = (psInfo*) copyObject(PSInfo);

	reversePSInfo(PSInfo, updatedTable);

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

	char *capSql = serializeOperatorModel((Node*) query);

	//RUN
	List *attrNames = getAttrNames(query->schema);
	DEBUG_LOG("PS Attr Names : %s", stringListToString(attrNames));
	HashMap *psMap = getPS(capSql, attrNames);


	StringInfo result = makeStringInfo();
	appendStringInfo(result, "%s", "{");
	bitOrResults(reservedPS->tablePSAttrInfos, psMap, &result);
	appendStringInfo(result, "%s", "}");


	return result->data;
}

static BitSet*
bitOrResults(HashMap *old, HashMap *new, StringInfo* result) {

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
			for(int k = 0; k < LIST_LENGTH(keys2); k++) {
				char* ss = ((Constant*) getNthOfListP(keys2, k))->value;
				if(strncmp(str->data, ss, strlen(str->data)) == 0) {
					Constant* newPSValue = (Constant*) getMapString(new, ss);
					int bitSetLength = info->BitVector->length;
					BitSet* bitSet = newBitSet(bitSetLength);
					int bitSetIntValue = *((int*)newPSValue->value);

					int index = info->BitVector->length - 1;

					//set bit for each position;
					while(bitSetIntValue > 0) {
						if(bitSetIntValue % 2 == 1) {
							setBit(bitSet, index, 1);
						}
						index--;
						bitSetIntValue /= 2;
					}
					char* bits = bitSetToString(bitSet);

					//reverse the bits;
					for(index = 0; index < bitSetLength / 2; index++) {
						bits[index] ^= bits[bitSetLength - index - 1];
						bits[bitSetLength - index - 1] ^= bits[index];
						bits[index] ^= bits[bitSetLength - index - 1];
					}


					bitSet = stringToBitset(bits);

					info->BitVector = bitOr(info->BitVector, bitSet);
					appendStringInfo(*result, "%s", createResultComponent(tableName, attrName, bitSetToString(info->BitVector)));

				}
			}

		}
	}
	return NULL;
}

static char*
getUpdatedTable(QueryOperator *op) {

	FOREACH(QueryOperator, operator, op->inputs)
	{
		if (isA(operator, TableAccessOperator)) {
			return ((TableAccessOperator*) operator)->tableName;
		}
	}

	return NULL;

}

static psAttrInfo*
getUpdatedTablePSAttrInfo(psInfo *PSInfo, char *tableName) {

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
getAllTables(psInfo *PSInfo) {
	return getKeys(PSInfo->tablePSAttrInfos);
}

static char*
createResultComponent(char *tableName, char *psAttr, char *ps) {

	StringInfo result = makeStringInfo();

	appendStringInfo(result, "(%s[%s:%s])", tableName, psAttr, ps);

	return result->data;
}

static ProjectionOperator*
createDummyProjTree(QueryOperator *updateQuery) {

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
	TableAccessOperator *taOp = createTableAccessOp("DUAL", NULL, "DUAL", NIL,
			singleton("DUMMY"), singletonInt(DT_STRING));
	ProjectionOperator *projOp = createProjectionOp(attrValues,
			(QueryOperator*) taOp, NIL, attrNames);
	projOp->op.schema->attrDefs = attrDefs;
	return projOp;
}

static void reversePSInfo(psInfo *PSInfo, char *updatedTable) {
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

				DEBUG_NODE_BEATIFY_LOG("what is in the ranger",
						getNthOfListP(info->rangeList, 0));

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

static boolean getTableAccessOps(Node *op, List **l) {
	if (op == NULL)
		return TRUE;

	if (isA(op, TableAccessOperator)) {
		DEBUG_LOG("TABLEACCESSOPERATOR: %s",
				((TableAccessOperator* )op)->tableName);
		*l = appendToTailOfList(*l, op);
	}

	return visit((Node*) op, getTableAccessOps, l);
}

static QueryOperator*
rewriteTableAccessOperator(TableAccessOperator *op, psInfo *PSInfo) {

	List *attrInfos = (List*) getMapString(PSInfo->tablePSAttrInfos,
			op->tableName);

	List *selAndList = NIL;
	for (int i = 0; i < LIST_LENGTH(attrInfos); i++) {
		psAttrInfo *info = (psAttrInfo*) getNthOfListP(attrInfos, i);

		List *rangeList = info->rangeList;
		char *bitSet = bitSetToString(info->BitVector);

		AttributeReference *psAttrRef = createAttrsRefByName(
				(QueryOperator*) op, info->attrName);
		for (int j = 0; j < strlen(bitSet); j++) {
			if (bitSet[j] == '1') {
				Operator *lOp = createOpExpr(">=",
						LIST_MAKE(copyObject(psAttrRef),
								copyObject(getNthOfListP(rangeList, j))));
				Operator *rOp = createOpExpr("<",
						LIST_MAKE(copyObject(psAttrRef),
								copyObject(getNthOfListP(rangeList, j + 1))));
				Node *andExp = andExprList(LIST_MAKE(lOp, rOp));
				selAndList = appendToTailOfList(selAndList, andExp);
			}
		}

	}
	Node *selOrExp = orExprList(selAndList);
	SelectionOperator *selOp = createSelectionOp((Node*) selOrExp,
			(QueryOperator*) op, op->op.parents,
			getQueryOperatorAttrNames((QueryOperator*) op));
	INFO_OP_LOG("selecction operator", selOp);
	op->op.parents = singleton(selOp);
	return (QueryOperator*) selOp;
}
