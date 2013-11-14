/*-----------------------------------------------------------------------------
 *
 * sql_serializer.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SQL_SERIALIZER_H_
#define SQL_SERIALIZER_H_

#include "model/query_operator/query_operator.h"

extern char *serializeOperatorModel(Node *q);
extern char *serializeQuery(QueryOperator *q);

#endif /* SQL_SERIALIZER_H_ */
