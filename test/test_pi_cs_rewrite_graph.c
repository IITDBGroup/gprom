/*
 * test_pi_cs_rewrite_graph.c
 *
 *  Created on: Jan 28, 2015
 *      Author: jingyuzhu
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parser.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/translator.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "rewriter.h"

int
main(int argc, char* argv[]){

	Node *result;
	Node *qoModel;
	Node *rewriteQoModel;



    READ_OPTIONS_AND_INIT("testpicsrewritegraph", "Run all stages on input and output rewritten SQL.");

    // read from terminal
    if (getStringOption("input.sql") == NULL){

    }else{
    	result = parseFromString(getStringOption("input.sql"));
    	qoModel = translateParse(result);
    	QueryOperator *op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    	rewriteQoModel = (Node *) rewritePI_CS((ProvenanceComputation *) op);
    }

}
