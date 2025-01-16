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
#include "sql_serializer/sql_serializer.h"
#include "sql_serializer/sql_serializer_common.h"

extern char *serializeOperatorModelPostgres(Node *q);
extern char *serializeQueryPostgres(QueryOperator *q);
extern char *quoteIdentifierPostgres (char *ident);

extern void postgresSerializeJoinOperator(StringInfo from, QueryOperator* fromRoot, JoinOperator* j,
        int* curFromItem, int* attrOffset, FromAttrsContext *fac, SerializeClausesAPI *api);
extern List *postgresSerializeProjectionAndAggregation(QueryBlockMatch *m, StringInfo select,
        StringInfo having, StringInfo groupBy, FromAttrsContext *fac, boolean materialize, SerializeClausesAPI *api);
extern void postgresSerializeConstRel(StringInfo from, ConstRelOperator* t, FromAttrsContext *fac,
        int* curFromItem,  SerializeClausesAPI *api);
extern void postgresSerializeTableAccess(StringInfo from, TableAccessOperator* t, int* curFromItem,
		FromAttrsContext *fac, int* attrOffset, SerializeClausesAPI *api);
extern List *postgresSerializeSetOperator(QueryOperator *q, StringInfo str, FromAttrsContext *fac, SerializeClausesAPI *api);



#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_POSTGRES_H_ */
