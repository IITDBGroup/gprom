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
#include "configuration/option.h"

#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "analysis_and_translate/translator.h"
#include "sql_serializer/sql_serializer.h"
#include "operator_optimizer/operator_optimizer.h"
#include "instrumentation/timing_instrumentation.h"

static char *rewriteParserOutput (Node *parse, boolean applyOptimizations);

char *
rewriteQuery(char *input)
{
    Node *parse;
    char *result;

    parse = parseFromString(input);
    DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

    result = rewriteParserOutput(parse, isRewriteOptionActivated("optimize_operator_model"));
    INFO_LOG("Rewritten SQL text from <%s>\n\n is <%s>", input, result);

    OUT_TIMERS();

    return result;
}

char *
rewriteQueryFromStream (FILE *stream) {
    Node *parse;
    char *result;

    parse = parseStream(stream);
    DEBUG_LOG("parser returned:\n\n%s", nodeToString(parse));

    result = rewriteParserOutput(parse, isRewriteOptionActivated("optimize_operator_model"));
    INFO_LOG("Rewritten SQL text is <%s>", result);

    OUT_TIMERS();

    return result;
}

char *
rewriteQueryWithOptimization(char *input)
{
    Node *parse;
    char *result;

    parse = parseFromString(input);
    DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

    result = rewriteParserOutput(parse, TRUE);
    INFO_LOG("Rewritten SQL text from <%s>\n\n is <%s>", input, result);

    return result;
}

static char *
rewriteParserOutput (Node *parse, boolean applyOptimizations)
{
    StringInfo result = makeStringInfo();
    char *rewrittenSQL = NULL;
    Node *oModel;
    Node *rewrittenTree;

    START_TIMER("translation");
    oModel = translateParse(parse);
    DEBUG_LOG("translator returned:\n\n<%s>", nodeToString(oModel));
    INFO_LOG("as overview:\n\n%s", operatorToOverviewString(oModel));
    STOP_TIMER("translation");

    START_TIMER("rewrite");
    rewrittenTree = provRewriteQBModel(oModel);
    DEBUG_LOG("provenance rewriter returned:\n\n<%s>", beatify(nodeToString(rewrittenTree)));
    INFO_LOG("as overview:\n\n%s", operatorToOverviewString(rewrittenTree));
    STOP_TIMER("rewrite");

    ASSERT_BARRIER(
        if (isA(rewrittenTree, List))
            FOREACH(QueryOperator,o,(List *) rewrittenTree)
                ASSERT(checkModel(o));
        else
            ASSERT(checkModel((QueryOperator *) rewrittenTree));
    )

    if(applyOptimizations)
    {
        START_TIMER("OptimizeModel");
        if(isA(rewrittenTree, List))
        {
            FOREACH_LC(lc, (List *) rewrittenTree)
            {
                QueryOperator *o = (QueryOperator *) LC_P_VAL(lc);

                o = mergeAdjacentOperators(o);
                ASSERT(checkModel(o));
                DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) o));
                o = pushDownSelectionOperatorOnProv(o);
                ASSERT(checkModel(o));
                DEBUG_LOG("selections pushed down\n\n%s", operatorToOverviewString((Node *) o));
                o = mergeAdjacentOperators(o);
                ASSERT(checkModel(o));
                DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) o));
                LC_P_VAL(lc) = o;
            }
        }
        else
        {
            rewrittenTree = (Node *) mergeAdjacentOperators((QueryOperator *) rewrittenTree);
            ASSERT(checkModel((QueryOperator *) rewrittenTree));
            rewrittenTree = (Node *) pushDownSelectionOperatorOnProv((QueryOperator *) rewrittenTree);
            ASSERT(checkModel((QueryOperator *) rewrittenTree));
            rewrittenTree = (Node *) mergeAdjacentOperators((QueryOperator *) rewrittenTree);
            ASSERT(checkModel((QueryOperator *) rewrittenTree));
        }
        INFO_LOG("after merging operators:\n\n%s", operatorToOverviewString(rewrittenTree));
        STOP_TIMER("OptimizeModel");
    }

    START_TIMER("SQLcodeGen");
    appendStringInfo(result, "%s\n", serializeOperatorModel(rewrittenTree));
    STOP_TIMER("SQLcodeGen");

    rewrittenSQL = result->data;
    FREE(result);

    return rewrittenSQL;
}
