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

#include <stdio.h>
#include "model/query_block/query_block.h"
#include "model/list/list.h"

Node *parseStream (FILE *file);
Node *parseFromString (char *input);

#endif /* PARSER_H_ */
