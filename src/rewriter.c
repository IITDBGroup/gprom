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
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "analysis_and_translate/translator.h"
#include "sql_serializer/sql_serializer.h"

static char *rewriteParserOutput (Node *parse);

char *
rewriteQuery(char *input)
{
    Node *parse;
    char *result;

    parse = parseFromString(input);
    DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

    result = rewriteParserOutput(parse);
    INFO_LOG("Rewritten SQL text from <%s>\n\n is <%s>", input, result);

    return result;
}

char *
rewriteQueryFromStream (FILE *stream) {
    Node *parse;
    char *result;

    parse = parseStream(stream);
    DEBUG_LOG("parser returned:\n\n%s", nodeToString(parse));

    result = rewriteParserOutput(parse);
    INFO_LOG("Rewritten SQL text is <%s>", result);

    return result;
}

static char *
rewriteParserOutput (Node *parse)
{
    StringInfo result = makeStringInfo();
    char *rewrittenSQL = NULL;
    Node *oModel;
    Node *rewrittenTree;

    oModel = translateParse(parse);
    DEBUG_LOG("translator returned:\n\n<%s>", nodeToString(oModel));

    rewrittenTree = provRewriteQBModel(oModel);
    DEBUG_LOG("provenance rewriter returned:\n\n<%s>", beatify(nodeToString(rewrittenTree)));
    DEBUG_LOG("as overview:\n\n%s", operatorToOverviewString(rewrittenTree));

    appendStringInfo(result, "%s\n", serializeOperatorModel(rewrittenTree));

    rewrittenSQL = result->data;
    FREE(result);

    return rewrittenSQL;
}
