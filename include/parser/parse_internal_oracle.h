/*-----------------------------------------------------------------------------
 *
 * parse_internal.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSE_INTERNAL_H_
#define PARSE_INTERNAL_H_

#include "model/node/nodetype.h"

/* for storing results */
extern Node *oracleParseResult;

/* interface to the lexer */
extern int oraclelineno; /* from lexer */
int oraclelex(void);
void oracleerror(char *s);
extern FILE *oraclein;
extern void oracleSetupStringInput(char *input);


#endif /* PARSE_INTERNAL_H_ */
