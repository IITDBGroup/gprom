/*-----------------------------------------------------------------------------
 *
 * sql_serializer_dl.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_DL_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_DL_H_


extern char *serializeOperatorModelDL(Node *q);
extern char *serializeQueryDL(QueryOperator *q);
extern char *quoteIdentifierDL (char *ident);


#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_DL_H_ */
