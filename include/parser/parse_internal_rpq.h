/*-----------------------------------------------------------------------------
 *
 * parse_internal_dl.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PARSER_PARSE_INTERNAL_RPQ_H_
#define INCLUDE_PARSER_PARSE_INTERNAL_RPQ_H_


/* for storing results */
extern Node *rpqParseResult;

/* interface to the lexer */
extern int rpqlineno; /* from lexer */
int rpqlex(void);
void rpqerror(char *s);
extern FILE *rpqin;
extern void rpqSetupStringInput(char *input);


#endif /* INCLUDE_PARSER_PARSE_INTERNAL_DL_H_ */
