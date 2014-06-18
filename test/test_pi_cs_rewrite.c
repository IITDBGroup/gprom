/*
 *test_pi_cs_rewrite .c
 *
 *      Author: Pankaj Purandare
 */

#include "common.h"

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parse_internal.h"
#include "parser/parser.h"
#include "../src/parser/sql_parser.tab.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "analysis_and_translate/translator.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"

/* if OCI is not available then add dummy versions */
#if HAVE_A_BACKEND

int
main (int argc, char* argv[])
{
    Node *result;
    Node *qoModel;
    Node *rewriteQoModel;

    initMemManager();
    mallocOptions();
    if(parseOption(argc, argv) != 0)
    {
        printOptionParseError(stdout);
        printOptionsHelp(stdout, "testpicsrewrite", "Run all stages on input and output rewritten SQL, but only use pi-cs rewritter.");
        return EXIT_FAILURE;
    }
    initLogger();
    initMetadataLookupPlugins();
    chooseMetadataLookupPluginFromString(getStringOption("backend"));
    initMetadataLookupPlugin();

    // read from terminal
    if (getStringOption("input.sql") == NULL)
    {
        result = parseStream(stdin);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
    }
    // parse input string
    else
    {
        result = parseFromString(getStringOption("input.sql"));

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
    }

    qoModel = translateParse(result);
    ERROR_LOG("TRANSLATION RESULT FROM STRING IS:\n%s", beatify(nodeToString(qoModel)));
    ERROR_LOG("SIMPLIFIED OPERATOR TREE:\n%s", operatorToOverviewString(qoModel));

    QueryOperator *op = (QueryOperator *) getHeadOfListP((List *) qoModel);
    rewriteQoModel = (Node *) rewritePI_CS((ProvenanceComputation *) op);
    ERROR_LOG("REWRITTEN PROVENANCE RESULT IS:\n%s", beatify(nodeToString(rewriteQoModel)));
    ERROR_LOG("REWRITTEN PROVENANCE RESULT IS:\n%s", operatorToOverviewString(rewriteQoModel));

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}



/* if no backend db plugin is available replace with dummy test */
#else

int main()
{
    return EXIT_SUCCESS;
}

#endif


