/*-----------------------------------------------------------------------------
 *
 * sql_serializer_sqlite.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_SQLITE_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_SQLITE_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModelSQLite(Node *q);
extern char *serializeQuerySQLite(QueryOperator *q);
extern char *quoteIdentifierSQLite (char *ident);

#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_SQLITE_H_ */
