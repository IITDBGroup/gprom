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

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(char *s);


#endif /* PARSE_INTERNAL_H_ */
