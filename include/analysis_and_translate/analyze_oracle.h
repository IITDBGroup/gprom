/*-----------------------------------------------------------------------------
 *
 * analyze_oracle.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef ANALYSE_ORACLE_H_
#define ANALYSE_ORACLE_H_

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"

extern Node *analyzeOracleModel(Node *stmt);
extern void analyzeQueryBlockStmt(Node *stmt, List *parentFroms, HashMap *ctes);
extern boolean hasNestedSubqueries(Node *node);
extern boolean findNestedSubqueries(Node *node, List **state);
extern List *splitAttrOnDot(char *dotName);
extern char *lastAttrNamePart(char *attrName);
extern boolean findFunctionCall(Node *node, List **state);

#endif /* ANALYSE_ORACLE_H_ */
