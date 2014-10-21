/*-----------------------------------------------------------------------------
 *
 * parse_internal_dl.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PARSER_PARSE_INTERNAL_DL_H_
#define INCLUDE_PARSER_PARSE_INTERNAL_DL_H_


/* for storing results */
extern Node *dlParseResult;

/* interface to the lexer */
extern int dllineno; /* from lexer */
int dllex(void);
void dlerror(char *s);
extern FILE *dlin;
extern void dlSetupStringInput(char *input);


#endif /* INCLUDE_PARSER_PARSE_INTERNAL_DL_H_ */
