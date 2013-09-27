/*-----------------------------------------------------------------------------
 *
 * parser.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "parser/parser.h"
#include "sql_parser.tab.h"

List *
parserStmts (char *input)
{
    return NIL;
}

Node *
parseSingleQuery (char *input)
{
    yyparse();
    return NULL;
}
