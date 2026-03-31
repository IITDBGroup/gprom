/*-----------------------------------------------------------------------------
 *
 * parser_oracle.c
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
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parser.h"
#include "parser/parse_internal_oracle.h"
#include "parser/parser_oracle.h"
#include "oracle_parser.tab.h"
#include "instrumentation/timing_instrumentation.h"

static const char *languageHelp = "We support a subset of SQL mostly focusing on the query part. Limited forms of DML and DDL commands are supported, but only in the context of reenactment and not as standalone commands. In addition to standard SQL's query features, GProM introduces the following new constructs that are treated as queries (e.g., can be used as subqueries):\n"
        "\n"
        "PROVENANCE [AS OF as_of] [WITH prov_options] OF prov_target [TRANSLATE AS JSON];:\n"
        "    - capture the provenance of prov_target applying prov_options and optionally translate the result into PROV-JSON\n"
        "\n"
        "REENACT [AS OF as_of] [WITH prov_options] ( ddl_dml_stmt_list );:\n"
        "   - reenact the statements from ddl_dml_stmt_list which have to be either DML or DDL statements\n"
        "\n"
        "prov_target:\n"
        "   TRANSACTION 'xid'   - capture provenance of a past transaction with identifier xid (currently only Oracle supported)\n"
        "   | ( query )         - capture provenance of query, where query is an SQL query\n"
        "\n"
        "prov_options:\n"
        "   TABLE table_name    - for reenactment track provenance of table_name\n"
        "   | ONLY UPDATED      - during reenactment show only tuples affected by at least one update\n"
        "   | SHOW INTERMEDIATE - for reenactment show provenance after each intermediate step (update) \n"
        "   | TUPLE VERSIONS    - show tuple ID + version instead of full tuples in provenance\n"
        "   | [NO] STATEMENT ANNOTATIONS     - [do not] show boolean attributes that record which update affected which tuple\n"
        "   | PROVENANCE        - when using REENACT also capture provenance (default is no provenance capture)\n"
        "   | RESULTTIDS        - when using -prov_use_composable show ids of original result tuples (default is not to)"
        "   | SEMIRING COMBINER [ semiring | ADD (expr) MULT (expr) ] - combine provenance using a specific semiring (N: natural number semiring, NX: provenance polynomials, TROPICAL, VITERBI, FUZZY) or specify an aggregation expression for addition and scalar expression for muliplication. Typically, when using this you would want to select a single attribute storing annotations as the provenance for each table accessed by a query."
        "   | COARSE GRAINED coarse_grained_spec - capture a provenance sketch for the query"
        "\n"
        "as_of:\n"
        "   SCN scn             - reenact statements over the database version as of SCN (Oracle's internal version identifier)\n"
        "   | TIMESTAMP time    - reenact statements over the database version as of time\n"
        "\n"
        "Insider a query that is a prov_target, options that control provenance capture can be set on a per FROM clause item basis. That is, each from clause item can be followed by an optional capture_option\n"
        "\n"
                                  "coarse_grained_sepc:\n"
                                  "   HASH (hash_list)\n"
                                  "   | PAGE (page_list)\n"
                                  "   | RANGESA (rangesa)\n"
                                  "   | RANGESB (rangesb)\n"
                                  "   | FRAGMENT (fragments)\n"
                                  "\n"
        "capture_option:\n"
        "   BASERELATION                                    - treat from clause item  as a base relation, i.e., do not capture provenance of the computation within the from clause item \n"
        "   HAS PROVENANCE ( attr_list )                    - use attr_list as provenance for this from clause item \n"
        "   USE PROVENANCE ( attr_list )                    - duplicate attr_list as provenance for this from clause item \n"
        "   SHOW INTERMEDIATE PROVENANCE [ ( attr_list ) ]  - propagate the result tuples (optionally projected on attr_list) of this from clause item in addition to any provenance that is captured already\n"
        ;

static Node *parseInternalOracle (void);

Node *
parseStreamOracle (FILE *stream)
{
    oraclein = stream;

    return parseInternalOracle();
}

Node *
parseFromStringOracle (char *input)
{
    INFO_LOG("parse SQL:\n%s", input);
    oracleSetupStringInput(input);

    return parseInternalOracle();
}

Node *
parseExprFromStringOracle (char *input)
{
	INFO_LOG("parse expr:\n%s", input);
	char *wrappedInput = CONCAT_STRINGS("[", input, "]");
	oracleSetupStringInput(wrappedInput);

	return getHeadOfListP((List *) parseInternalOracle());
}

const char *
languageHelpOracle (void)
{
    return languageHelp;
}

static Node *
parseInternalOracle (void) //TODO make copyObject work first
{
    START_TIMER("module - parser");

    NEW_AND_ACQUIRE_MEMCONTEXT("PARSER_CONTEXT");

    // parse
    int rc = oracleparse();
    if (rc)
    {
        ERROR_LOG("parse error!");
        return NULL;
    }

    STOP_TIMER("module - parser");

    DEBUG_NODE_BEATIFY_LOG("query block model generated by parser is:",
            oracleParseResult);

    // create copy of parse result in parent context
    FREE_MEM_CONTEXT_AND_RETURN_COPY(Node,oracleParseResult);
}
