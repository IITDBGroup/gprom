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
#if HAVE_LIBOCILIB && (HAVE_LIBOCI || HAVE_LIBOCCI)

int
main (int argc, char* argv[])
{
    Node *result;
    Node *qoModel;
    Node *rewriteQoModel;

    initMemManager();
    mallocOptions();
    parseOption(argc, argv);
    initLogger();
    initMetadataLookupPlugin();

    // read from terminal
    if (getOptions()->optionConnection->sql == NULL)
    {
        result = parseStream(stdin);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
    }
    // parse input string
    else
    {
        result = parseFromString(getOptions()->optionConnection->sql);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
    }

    qoModel = translateParse(result);
    ERROR_LOG("TRANSLATION RESULT FROM STRING IS:\n%s", beatify(nodeToString(qoModel)));
    ERROR_LOG("SIMPLIFIED OPERATOR TREE:\n%s", operatorToOverviewString(qoModel));

    QueryOperator *op = (QueryOperator *) getHeadOfListP(qoModel);
    rewriteQoModel = (Node *) rewritePI_CS((ProvenanceComputation *) op);
    ERROR_LOG("REWRITTEN PROVENANCE RESULT IS:\n%s", beatify(nodeToString(rewriteQoModel)));

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}



/* if OCI or OCILIB are not avaible replace with dummy test */
#else

int main()
{
    return EXIT_SUCCESS;
}

#endif


