/*-----------------------------------------------------------------------------
 *
 * parse_internal_dl.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PARSER_PARSE_INTERNAL_JP_H_
#define INCLUDE_PARSER_PARSE_INTERNAL_JP_H_


/* for storing results */
extern Node *jpParseResult;

/* interface to the lexer */
extern int jplineno; /* from lexer */
int jplex(void);
void jperror(char *s);
extern FILE *jpin;
extern void jpSetupStringInput(char *input);


#endif /* INCLUDE_PARSER_PARSE_INTERNAL_DL_H_ */
