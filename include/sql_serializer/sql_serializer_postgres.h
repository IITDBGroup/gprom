/*-----------------------------------------------------------------------------
 *
 * sql_serializer_postgres.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_POSTGRES_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_POSTGRES_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModelPostgres(Node *q);
extern char *serializeQueryPostgres(QueryOperator *q);
extern char *quoteIdentifierPostgres (char *ident);


#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_POSTGRES_H_ */
