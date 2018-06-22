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

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"

NEW_ENUM_WITH_TO_STRING(ProvQuestion,
        PROV_Q_WHY,
        PROV_Q_WHYNOT
);

extern Node *rewriteSummaryOutput (Node *rewrittenTree, HashMap *summOpts, ProvQuestion qType);

