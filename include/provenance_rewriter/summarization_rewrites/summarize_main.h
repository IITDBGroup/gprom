/*-----------------------------------------------------------------------------
 *
 * summarize_main.h
 *			  
 *		
 *		AUTHOR: seokki
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/query_operator/query_operator.h"

extern Node *rewriteSummaryOutput (Node *rewrittenTree, List *summOpts);
//extern Node *rewriteProvJoinOutput (Node *rewrittenTree);
