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
#include "parser/parse_internal.h"
#include "sql_parser.tab.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

List *
parseStmts (char *input)
{
    return NIL;
}

Node *
parseSingleQuery (char *input)
{
    MemContext *parseContext = NEW_MEM_CONTEXT("PARSER_CONTEXT");
    Node *result;
    AQUIRE_MEM_CONTEXT(parseContext);

    // parse
    int rc = yyparse();
    if (rc)
    {
        ERROR_LOG("parse error!");
        return NULL;
    }

    // create copy of parse result in parent context
    RELEASE_MEM_CONTEXT();
    result = copyObject(bisonParseResult);

    // clean up
    FREE_MEM_CONTEXT(parseContext);
    return result;
}
