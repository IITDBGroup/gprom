/*-----------------------------------------------------------------------------
 *
 * rewriter.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "rewriter.h"

#include "common.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "analysis_and_translate/translator.h"
#include "sql_serializer/sql_serializer.h"

char *
rewriteQuery(char *input)
{
    char *result;
    Node *parse;
    QueryOperator *operatorTree;
    QueryOperator *rewrittenTree;

    parse = parseFromString(input);
    operatorTree = translateParse(parse);
    rewrittenTree = provRewriteQuery(operatorTree);
    result = serializeQuery(rewrittenTree);

    return result;
}
