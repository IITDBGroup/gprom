/*
 *------------------------------------------------------------------------------
 *
 * translator_dl_to_dl.c - translate datalog to datalog
 *
 *     This is a no-op except for that provenance ccomputations are rewritten.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2023-01-04
 *        SUBDIR: src/analysis_and_translate/
 *
 *-----------------------------------------------------------------------------
 */

#include "analysis_and_translate/analyze_dl.h"
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "log/logger.h"

#include "analysis_and_translate/translate_dl_to_dl.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/game_provenance/gp_main.h"
#include "provenance_rewriter/summarization_rewrites/summarize_main.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"

static DLProgram *rewriteDLProgram(DLProgram *p);

Node *
translateParseDLToDL(Node *q)
{
    DLProgram *p = (DLProgram *) q;
	DLProgram *result = NULL;

    INFO_LOG("translate DL model:\n\n%s", datalogToOverviewString(q));

    if (isA(q,DLProgram))
        result = rewriteDLProgram(p);
    // what other node types can be here?
    else
        FATAL_LOG("currently only DLProgram node type translation supported");

    INFO_OP_LOG("rewritten DL program:\n", result);

    return (Node *) result;
}

QueryOperator *
translateQueryDLToDL(Node *node)
{
	FATAL_LOG("we never want to use this translator to translate datalog into RA!");

	return NULL;
}

static DLProgram *
rewriteDLProgram(DLProgram *p)
{
	// if we want to compute the provenance then construct program
	// for creating the provenance and translate this one
	if (IS_GP_PROV(p))
	{
		DEBUG_LOG("user asked for provenance computation for:\n%s",
				  datalogToOverviewString((Node *) p));

		DLProgram *gpComp = (DLProgram *) rewriteForGP((Node *) p);

		ASSERT(!IS_GP_PROV(gpComp));

		checkDLModel((Node *) gpComp);

		return gpComp;
	}

	if (IS_LINEAGE_PROV(p))
	{
		DLProgram *pl = rewriteDLForLinageCapture(p);

		pl->rules = CONCAT_LISTS(pl->rules, pl->facts);
		pl = (DLProgram *) analyzeDLModel((Node *) pl);

		return pl;
	}

	return p;
}
