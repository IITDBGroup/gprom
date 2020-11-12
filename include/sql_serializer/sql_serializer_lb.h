/*-----------------------------------------------------------------------------
 *
 * sql_serializer_dl.h
 *		
 *
 *		AUTHOR: seokki
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_LB_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_LB_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModelLB(Node *q);
extern char *serializeQueryLB(QueryOperator *q);
extern char *quoteIdentifierLB (char *ident);


#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_LB_H_ */
