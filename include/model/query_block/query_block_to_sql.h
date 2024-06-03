/*
 *------------------------------------------------------------------------------
 *
 * query_block_to_sql.h - Serialized query block model to SQL.
 *
 *     Serialize back the query block model to SQL text.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-04-04
 *        SUBDIR: include/model/query_block/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _QUERY_BLOCK_TO_SQL_H_
#define _QUERY_BLOCK_TO_SQL_H_

#include "model/node/nodetype.h"

extern char *parseBackQueryBlock(Node *query);

#endif /* _QUERY_BLOCK_TO_SQL_H_ */
