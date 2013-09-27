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

//typedef struct ParseContext
//{
//    Node *node;
//} ParseContext;
//
//typedef struct ScanContext
//{
//    Node *why;
//} ScanContext;

/* for storing results */
extern Node *bisonParseResult;

/* interface to the lexer */
extern int yylineno; /* from lexer */
int yylex(void);
void yyerror(char *s);


#endif /* PARSE_INTERNAL_H_ */
