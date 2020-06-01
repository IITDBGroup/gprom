/*
 * update_ps_main.h
 *
 *  Created on: May 17, 2020
 *      Author: pengyuanli
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_MAIN_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_MAIN_H_

#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"

extern char* update_ps(ProvenanceComputation* qbModel);



/*
char* update_ps_update(Node *query, Node *updateQuery, psInfo* PSInfo, int ruleNum);
char* update_ps_insert(Node *query, int ruleNum, Node *updateQuery, psInfo* PSInfo);
*/
//char* update_ps_delete_drop(QueryOperator *query, QueryOperator *updateQuery, psInfo* PSInfo);

//char* update_ps_delete_approximate(QueryOperator *query, QueryOperator *updateQuery, psInfo* PSInfo);
///*
//char* update_ps_delete_accurate(Node *query, Node *updateQuery, psInfo* PSInfo);
//char* update_ps_insert_approximate(Node *query, Node *updateQuery, psInfo* PSInfo);
//char* update_ps_insert_accurate(Node *query, Node *updateQuery, psInfo* PSInfo);
//*/
//char* getUpdatedTable(QueryOperator* op);
//psAttrInfo* getUpdatedTablePSAttrInfo(psInfo* PSInfo, char* tableName);
//List* getAllTables(psInfo* PSInfo);
//char* createResultComponent(char* tableName, char* psAttr, char* ps);
#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_MAIN_H_ */
