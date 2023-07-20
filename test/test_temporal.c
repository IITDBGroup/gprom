/*-----------------------------------------------------------------------------
 *
 * test_temporal.c
 *
 *
 *		AUTHOR: felix
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/translator.h"
#include "temporal_queries/temporal_rewriter.h"
#include "metadata_lookup/metadata_lookup.h"
#include "sql_serializer/sql_serializer.h"
#include "rewriter.h"

static rc testNormalization(void);

rc
testTemporal()
{
    RUN_TEST(testNormalization(), "test normalization");

    return PASS;
}

static rc
testNormalization(void)
{
    /* QueryOperator *tableAccess = (QueryOperator *)provRewriteQuery((QueryOperator *)translateParse(parseFromString("sequenced temporal (select * from op with time (P_START, P_END));"))); */
    // QueryOperator *normalized = addTemporalNormalization(tableAccess, copyObject(tableAccess), NIL);

    /* INFO_LOG("TEMPORAL OPERATOR TREE IS: ", beatify(nodeToString(tableAccess))); */

    /* char *serialized = serializeOperatorModel((Node *)tableAccess); */
    /* INFO_LOG("SERIALIZED TEMPORAL QUERY IS: %s\n", serialized); */

    /* Relation *relation = executeQuery(serializeOperatorModel((Node *)tableAccess)); */
    /* INFO_LOG("TUPLES OF RESULT ARE: ", beatify(nodeToString(relation->tuples))); */

    return PASS;
}
