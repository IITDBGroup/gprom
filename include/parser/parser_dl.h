/*-----------------------------------------------------------------------------
 *
 * parser_dl.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PARSER_PARSER_DL_H_
#define INCLUDE_PARSER_PARSER_DL_H_

#include "model/node/nodetype.h"

extern Node *parseStreamdl (FILE *stream);
extern Node *parseFromStringdl (char *input);
extern const char *languageHelpDL (void);

#endif /* INCLUDE_PARSER_PARSER_DL_H_ */
