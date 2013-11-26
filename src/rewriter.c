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

char *
rewriteQuery(char *input)
{
    StringInfo result = makeStringInfo();
    char *rewrittenSQL = NULL;
    Node *parse;
    Node *oModel;
    Node *rewrittenTree;


    parse = parseFromString(input);
    DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

    oModel = translateParse(parse);
    DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(oModel));

    if (isA(oModel, List))
    {
        List *stmtList = (List *) oModel;
        stmtList = provRewriteQueryList(stmtList);
        DEBUG_LOG("provenance rewriter returned:\n\n<%s>", nodeToString(stmtList));
        FOREACH(QueryOperator,o,stmtList)
            appendStringInfo(result, "%s\n", nodeToString(serializeQuery((QueryOperator *) oModel)));
    }
    else
    {
        oModel = (Node *) provRewriteQuery((QueryOperator *) oModel);
        DEBUG_LOG("provenance rewriter returned:\n\n<%s>", nodeToString(oModel));
        appendStringInfo(result, "%s\n", nodeToString(serializeQuery((QueryOperator *) oModel)));
    }

    rewrittenSQL = result->data;
    FREE(result);
    INFO_LOG("Rewritten SQL text from <%s>\n\n is <%s>", input, rewrittenSQL);

    return rewrittenSQL;
}
