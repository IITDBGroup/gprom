/*-----------------------------------------------------------------------------
 *
 * parser_dl.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "parser/parser.h"
#include "parser/parse_internal_rpq.h"
#include "parser/parser_rpq.h"
#include "rpq_parser.tab.h"
#include "instrumentation/timing_instrumentation.h"

static Node *parseInternalrpq (void);

Node *
parseStreamrpq (FILE *stream)
{
    rpqin = stream;

    return parseInternalrpq();
}

Node *
parseFromStringrpq (char *input)
{
    INFO_LOG("parse RQP expression:\n%s", input);
    rpqSetupStringInput(input);

    return parseInternalrpq();
}

static Node *
parseInternalrpq (void) //TODO make copyObject work first
{
    Node *result;
    START_TIMER("module - parser");

    NEW_AND_ACQUIRE_MEMCONTEXT("PARSER_CONTEXT");

    // parse
    int rc = rpqparse();
    if (rc)
    {
        FATAL_LOG("parse error!");
        return NULL;
    }

    STOP_TIMER("module - parser");

    if(rpqParseResult != NULL)
    DEBUG_NODE_BEATIFY_LOG("RPQ model generated by parser is:",rpqParseResult);

    // create copy of parse result in parent context
    FREE_MEM_CONTEXT_AND_RETURN_COPY(Node,rpqParseResult);
}

