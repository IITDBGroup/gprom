/*-----------------------------------------------------------------------------
 *
 * parser_rpq.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PARSER_PARSER_RPQ_H_
#define INCLUDE_PARSER_PARSER_RPQ_H_

#include "common.h"

extern Node *parseStreamrpq (FILE *stream);
extern Node *parseFromStringrpq (char *input);

#endif /* INCLUDE_PARSER_PARSER_RPQ_H_ */
