/*-----------------------------------------------------------------------------
 *
 * parser.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "model/query_block/query_block.h"
#include "model/list/list.h"

List *parserStmts (char *input);
Node *parseSingleQuery (char *input);

#endif /* PARSER_H_ */
