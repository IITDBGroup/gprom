/*-----------------------------------------------------------------------------
 *
 * parser_internal_postgres.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_INTERNAL_POSTGRES_H_
#define PARSER_INTERNAL_POSTGRES_H_

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
extern Node *postgresParseResult;

/* interface to the lexer */
extern int postgreslineno; /* from lexer */
int postgreslex(void);
void postgreserror(char *s);
extern FILE *postgresin;
extern void postgresSetupStringInput(char *input);
//extern yy_buffer_state;
//typedef struct yy_buffer_state YY_BUFFER_STATE;
//extern YY_BUFFER_STATE yy_scan_buffer(char *, size_t);


#endif /* PARSER_INTERNAL_POSTGRES_H_ */
