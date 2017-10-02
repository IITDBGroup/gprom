/*-----------------------------------------------------------------------------
 *
 * translator_sqlite.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_SQLITE_H_
#define INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_SQLITE_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern Node *translateParseSqlite(Node *q);
extern QueryOperator *translateQuerySqlite(Node *node);


#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_SQLITE_H_ */
