/*-----------------------------------------------------------------------------
 *
 * parser_oracle.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_ORACLE_H_
#define PARSER_ORACLE_H_

#include "model/node/nodetype.h"

extern Node *parseStreamOracle (FILE *stream);
extern Node *parseFromStringOracle (char *input);
extern Node *parseExprFromStringOracle (char *input);
extern const char *languageHelpOracle(void);

#endif /* PARSER_ORACLE_H_ */
