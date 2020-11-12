/*-----------------------------------------------------------------------------
 *
 * sql_serializer_oracle.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_ORACLE_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_ORACLE_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModelOracle(Node *q);
extern char *serializeQueryOracle(QueryOperator *q);
extern char *quoteIdentifierOracle (char *ident);

#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_ORACLE_H_ */
