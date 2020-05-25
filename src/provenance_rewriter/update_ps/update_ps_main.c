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

#include "common.h"
#include "configuration/option.h"
#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/bitset/bitset.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define DELETE_RULE_1 0 // DROP
#define DELETE_RULE_2 1 // DELETE(APPROXIMATE)
#define DELETE_RULE_3 3 // DELETE(ACCURATE)

#define INSERT_RULE_1 100 // INSERT(APPROXIMATE)
#define INSERT_RULE_2 101 // INSERT(ACCURATE)

#define UPDATE_RULE_1 200 // DELETE(APPROXIMATE) -> INSERT(APPROXIMATE)
#define UPDATE_RULE_2 201 // DELETE(APPROXIMATE) -> INSERT(ACCURATE)
#define UPDATE_RULE_3 202 // DELETE(ACCURATE) -> INSERT(APPROXIMATE)
#define UPDATE_RULE_4 203 // DELETE(ACCURATE) -> INSERT(ACCURATE)

char* update_ps(ProvenanceComputation *qbModel) {

	DEBUG_NODE_BEATIFY_LOG(
			"\n ############################\n \t START UPDATE THE PROVENANCE SKETCH\n ############################\n",
			qbModel);

	//initialize some parameters to get the ps, left chile(update statement) and right child(query);
	ProvenanceComputation* op = qbModel;
	Node* coarsePara = NULL;
	psInfo* psPara = NULL;
	ProvenanceComputation *originalOp = copyObject(op);

	if (isRewriteOptionActivated(OPTION_PS_USE_NEST)){
		op = originalOp;
	}

	coarsePara = (Node*) getStringProperty((QueryOperator*) op, PROP_PC_COARSE_GRAINED);
	psPara = createPSInfo(coarsePara);

	DEBUG_LOG("use coarse grained fragment parameters: %s",nodeToString((Node* ) psPara));

	//get the left and right childred respectively;
	// left child is a update statement
	// right child is a normal query
	QueryOperator *op1 = (QueryOperator*) op;
	QueryOperator *rChild = OP_RCHILD(op1);
	QueryOperator *lChild = (QueryOperator*) OP_LCHILD(op1);
	op1->inputs = singleton(rChild);







	QueryOperator* query = rewritePI_CS(op);
	removeParent(query, (QueryOperator*) op);



	DEBUG_NODE_BEATIFY_LOG( "\n#######################\n \t Query:\n#######################\n", query);
	DEBUG_NODE_BEATIFY_LOG( "\n#######################\n \t PS INFO:\n#######################\n", psPara);
	DEBUG_NODE_BEATIFY_LOG( "\n#######################\n \t Update query:\n#######################\n", lChild);
	char* result;

	result = update_ps_delete(query, lChild, psPara, DELETE_RULE_1);

/*
	if(lChild->type == T_Update) { //isA(lChild, ??)

		result = update_ps_update(query, lChild, psPara, UPDATE_RULE_1);
	} else if(lChild->type == T_Insert) {

		result = update_ps_insert(query, lChild, psPara, INSERT_RULE_1);
	} else if(lChild->type == T_Delete) {
		result = update_ps_delete(query, lChild, psPara, DELETE_RULE_1);
	}
	*/
	return result;

}

char* update_ps_delete(QueryOperator *query, QueryOperator *updateQuery, psInfo* PSInfo, int ruleNum) {
	char *result = NULL;
	switch (ruleNum) {
	case DELETE_RULE_1:
		result = update_ps_delete_drop(query, updateQuery, PSInfo);
		break;

	case DELETE_RULE_2:
//		result = update_ps_delete_approximate(query, updateQuery, PSInfo);
		break;
	case DELETE_RULE_3:
//		result = update_ps_delete_accurate(query, updateQuery, PSInfo);
		break;
	}
	return result;
}
/*
char* update_ps_update(Node *query, Node *updateQuery, psInfo* PSInfo, int ruleNum) {
	// step 1. delete




	// step 2. insert


	return NULL;
}
char* update_ps_insert(Node *query, int ruleNum, Node *updateQuery, psInfo* PSInfo) {
	char *result;

	switch (ruleNum) {
	case INSERT_RULE_1:
		result = update_ps_insert_approximate(query, updateQuery, PSInfo);
		break;
	case INSERT_RULE_2:
		result = update_ps_insert_accurate(query, updateQuery, PSInfo);
	}

	return result;
}
*/

char* update_ps_delete_drop(QueryOperator *query, QueryOperator *updateQuery, psInfo* PSInfo) {
	DEBUG_LOG( "\n#######################\n \t RULE: DROP PS\n#######################\n");

	//GET UPDATED TABLE
	char* updatedTableName = getUpdatedTable((QueryOperator*) updateQuery);
	if(!updatedTableName) return NULL;
	DEBUG_LOG("\n#######################\n \t TABLE NAME: %s\n#######################\n", updatedTableName);

	//GET UPDATE TABLE PS
	psAttrInfo* updatedTablePSInfo = getUpdatedTablePSAttrInfo(PSInfo, updatedTableName);
	if(!updatedTablePSInfo) return NULL; // the updated table does not relate to the query
	DEBUG_NODE_BEATIFY_LOG( "\n#######################\n \t Updated Table PS INFO:\n#######################\n", updatedTablePSInfo);

	List* tableList = getAllTables(PSInfo);

	DEBUG_LOG("\n#######################\n \t List Length %d:\n#######################\n", LIST_LENGTH(tableList));

	int resultSize = 100;
	char* result = (char*) MALLOC(sizeof(char[resultSize]));
	int index = 0;

	index += sprintf(result + index, "%s", "( ");
	for(int i = 0; i < LIST_LENGTH(tableList); i++) {
		char* key = (char*) ((Constant*)getNthOfListP(tableList, i))->value;


		if(index + strlen(key) > resultSize) {
			resultSize *= 2;
			result = (char*) realloc(result, sizeof(char[resultSize]));
		}
		index += sprintf(result + index, "%s", " [table name: ");
		index += sprintf(result + index, "%s", key);
		index += sprintf(result + index, "%s", ", after updating: \"DROPED\"] ");
	}
	index += sprintf(result + index, "%s", ")");
	result[index] = '\0';
	DEBUG_LOG("\n#######################\n \t the updated PS\n \t\t %s\n#######################\n", result);
	return STRING_VALUE(result);
}

/*
char* update_ps_delete_approximate(Node *query, Node *updateQuery, psInfo* PSInfo) {
	char* updatedTableName = getUpdatedTable((QueryOperator*)updateQuery);
	if(!updatedTableName) return NULL;

	//get the ps attr of table "updateTable"
	psAttrInfo* updatedTableAttrInfo;

	updatedTableAttrInfo = getUpdatedTablePSAttrInfo(PSInfo, updatedTableName);

	if(!updatedTableAttrInfo) return NULL;

	// get the ps bitvector of updated table

	BitSet* updateTableBitVec = updatedTableAttrInfo->BitVector;
	List* rangesList = updatedTableAttrInfo->rangeList;







	return NULL;
}

char* update_ps_delete_accurate(Node *query, Node *updateQuery, psInfo* PSInfo) {
	return NULL;
}
char* update_ps_insert_approximate(Node *query, Node *updateQuery, psInfo* PSInfo) {
	return NULL;
}
char* update_ps_insert_accurate(Node *query, Node *updateQuery, psInfo* PSInfo) {
	return NULL;
}
*/
char* getUpdatedTable(QueryOperator* op) {

	FOREACH(QueryOperator, operator, op->inputs) {
		if(isA(operator, TableAccessOperator)) {
			return ((TableAccessOperator*) operator)->tableName;
		}
	}

	return NULL;

}

psAttrInfo* getUpdatedTablePSAttrInfo(psInfo* PSInfo, char* tableName) {

	char* tmp = tableName;

	while(*tmp) {
		*tmp = tolower(*tmp);
		tmp++;
	}
	if(hasMapStringKey(PSInfo->tablePSAttrInfos, tableName)) {
		return (psAttrInfo*)getMapString(PSInfo->tablePSAttrInfos, tableName);
	}
	DEBUG_LOG( "\n#######################\n \t NO SUCH PSINFO\n#######################\n");
	return NULL;
}

List* getAllTables(psInfo * PSInfo) {
	return getKeys(PSInfo->tablePSAttrInfos);
}
