/*-----------------------------------------------------------------------------
 *
 * sql_serializer_oracle.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_SPARK_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_SPARK_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModelSpark(Node *q);
extern char *serializeQuerySpark(QueryOperator *q);
extern char *quoteIdentifierSpark (char *ident);

#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_ORACLE_H_ */
