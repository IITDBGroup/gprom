/*-----------------------------------------------------------------------------
 *
 * analyze_sqlite.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef ANALYSE_SQLITE_H_
#define ANALYSE_SQLITE_H_

#include "model/node/nodetype.h"
#include "model/list/list.h"

extern Node *analyzeSqliteModel (Node *stmt);
extern void analyzeSqliteQueryBlockStmt (Node *stmt, List *parentFroms);
extern boolean hasSqliteNestedSubqueries (Node *node);
extern boolean findSqliteNestedSubqueries (Node *node, List **state);

#endif /* ANALYSE_SQLITE_H_ */
