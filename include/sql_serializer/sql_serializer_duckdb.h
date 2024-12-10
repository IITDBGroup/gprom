/*-----------------------------------------------------------------------------
 *
 * sql_serializer_duckdb.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_DUCKDB_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_DUCKDB_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModelDuckDB(Node *q);
extern char *serializeQueryDuckDB(QueryOperator *q);
extern char *quoteIdentifierDuckDB (char *ident);


#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_DUCKDB_H_ */
