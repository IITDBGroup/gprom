/*-------------------------------------------------------------------------
 *
 * option.c
 *    The basic functions for options.
 *
 *        mallocOptions, freeOptions manage the memory for options.
 *        getOptions let other parts of program to have access to options.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "log/logger.h"
#include "exception/exception.h"

// we have to use actual free here
#ifndef MALLOC_REDEFINED
#undef free
#undef malloc
#endif

// stopper used to indicate end of options
#define STOPPER_STRING "STOPPER"

//Options* options;
HashMap *optionPos; // optionname -> position of option in list
HashMap *cmdOptionPos;
HashMap *backendInfo;
HashMap *frontendInfo;

typedef union OptionValue {
    char **string;
    int *i;
    boolean *b;
    double *f;
} OptionValue;

typedef union OptionDefault {
    char *string;
    int i;
    boolean b;
    double f;
} OptionDefault;

typedef struct OptionInfo {
    char *option;
    char *cmdLine;
    char *description;
    OptionType valueType;
    OptionValue value;
    OptionDefault def;
} OptionInfo;

typedef struct FrontendInfo {
    char *frontendName;
    char *analyzer;
    char *parser;
    char *translator;
} FrontendInfo;

typedef struct BackendInfo {
    BackendType typ;
    char *backendName;
    char *analyzer;
    char *parser;
    char *metadata;
    char *sqlserializer;
    char *translator;
} BackendInfo;

// show help only
boolean opt_show_help = FALSE;
char *opt_test = NULL;
boolean opt_listtests = FALSE;
char *opt_language_help = NULL;

// connection options
char *connection_host = NULL;
char *connection_db = NULL;
char *connection_user = NULL;
char *connection_passwd = NULL;
int connection_port = 0;

// backend specific options
char *oracle_audit_log_table = NULL;
boolean oracle_use_service_name = FALSE;

char *odbc_driver = NULL;

// logging options
int logLevel = 0;
boolean logActive = FALSE;
boolean opt_log_operator_colorize = TRUE;
boolean opt_log_operator_verbose = FALSE;
int opt_log_operator_verbose_props = 0;

// input options
char *sql = NULL;
char *sqlFile = NULL;

// database backend
char *backend = NULL;
char *frontend = NULL;
char *plugin_metadata = NULL;
char *plugin_parser = NULL;
char *plugin_sqlcodegen = NULL;
char *plugin_analyzer = NULL;
char *plugin_translator = NULL;
char *plugin_sql_serializer = NULL;
char *plugin_executor = NULL;
char *plugin_cbo = NULL;

// instrumentation options
boolean opt_inputdb = FALSE;
boolean opt_timing = FALSE;
boolean opt_memmeasure = FALSE;
boolean opt_graphviz_output = FALSE;
boolean opt_graphviz_detail = FALSE;
boolean opt_show_query_runtime = FALSE;
char *time_query_format = NULL;
int query_repeat_count = 1;
boolean opt_show_query_result = TRUE;

// rewrite options
boolean opt_aggressive_model_checking = FALSE;
boolean opt_update_only_conditions = FALSE;
boolean opt_treeify_opterator_model = FALSE;
boolean opt_treeify_all = FALSE;
boolean opt_only_updated_use_history = FALSE;
boolean opt_pi_cs_composable = FALSE;
boolean opt_pi_cs_rewrite_agg_window = FALSE;
boolean opt_optimize_operator_model = FALSE;
boolean opt_translate_update_with_case = FALSE;
//boolean   = FALSE;

// cost based optimization option
boolean cost_based_optimizer = FALSE;
boolean cost_based_close_option_removedp_by_set = FALSE;
int cost_max_considered_plans = 200;
int cost_sim_ann_const = 10;
int cost_sim_ann_cooldown_rate = 5;
int cost_based_num_heuristic_opt_iterations = 1;

// optimization options
boolean opt_optimization_push_selections = FALSE;
boolean opt_optimization_merge_ops = FALSE;
boolean opt_optimization_factor_attrs = FALSE;
boolean opt_optimization_materialize_unsafe_proj = FALSE;
boolean opt_optimization_merge_unsafe_proj = FALSE;
boolean opt_optimization_remove_redundant_projections = TRUE;
boolean opt_optimization_remove_redundant_duplicate_operator = TRUE;
boolean opt_optimization_pulling_up_provenance_proj = FALSE;
boolean opt_optimization_push_selections_through_joins = FALSE;
boolean opt_optimization_selection_move_around = FALSE;
boolean opt_optimization_remove_unnecessary_columns = FALSE;
boolean opt_optimization_remove_unnecessary_window_operators = FALSE;
boolean opt_optimization_pull_up_duplicate_remove_operators = FALSE;

// optimization options for group by operator
boolean opt_optimization_push_down_aggregation_through_join = FALSE;

// sanity check options
boolean opt_operator_model_unique_schema_attribues = FALSE;
boolean opt_operator_model_parent_child_links = FALSE;
boolean opt_operator_model_schema_consistency = FALSE;
boolean opt_operator_model_attr_reference_consistency = FALSE;
boolean opt_operator_model_data_structure_consistency = FALSE;

// temporal database options
boolean temporal_use_coalesce =	 TRUE;
boolean temporal_use_normalization = TRUE;
boolean temporal_use_normalization_window = FALSE;
boolean temporal_agg_combine_with_norm = TRUE;

// lateral rewrite for nesting operator
boolean opt_lateral_rewrite = FALSE;
boolean opt_unnest_rewrite = FALSE;
boolean opt_agg_reduction_model_rewrite = FALSE;

// use provenance scratch
int max_number_paritions_for_uses = 0;
int bit_vector_size = 32;
boolean ps_binary_search = FALSE;
boolean ps_binary_search_case_when = FALSE;
boolean ps_settings = FALSE;
boolean ps_set_bits = FALSE;
boolean ps_use_brin_op = FALSE;
boolean ps_analyze = TRUE;
boolean ps_use_nest = FALSE;
boolean ps_post_to_oracle = FALSE;
char *ps_store_table = NULL;

// Uncertainty rewriter options
boolean range_optimize_join = TRUE;
boolean range_optimize_agg = TRUE;
int range_compression_rate = 1;

// struct that encapsulates option state
struct option_state {
    HashMap *optionPos; // optionname -> position of option in list
    HashMap *cmdOptionPos;
    HashMap *backendInfo;
    HashMap *frontendInfo;
    OptionInfo opts[];
};

// dl rewrite options
boolean opt_whynot_adv = FALSE;
boolean opt_dl_min_with_fds = FALSE;
boolean opt_merge_dl = FALSE;
boolean opt_load_fds = FALSE;

// functions
#define wrapOptionInt(value) { .i = (int *) value }
#define wrapOptionBool(value) { .b = (boolean *) value }
#define wrapOptionString(value) { .string = (char **) value }
#define wrapOptionFloat(value) { .f = (float *) value }

#define defOptionInt(value) { .i = value }
#define defOptionBool(value) { .b = value }
#define defOptionString(value) { .string = value }
#define defOptionFloat(value) { .f = value }

static void initOptions(void);
static void setDefault(OptionInfo *o);
static OptionValue *getValue (char *name);
static OptionInfo *getInfo (char *name);
static char *valGetString(OptionValue *def, OptionType type);
static char *defGetString(OptionDefault *def, OptionType type);

#define aRewriteOption(_name,_opt,_desc,_var,_def) \
        { \
            _name, \
            _opt, \
            _desc, \
            OPTION_BOOL, \
            wrapOptionBool(&_var), \
            defOptionBool(_def) \
        }

#define anOptimizationOption(_name,_opt,_desc,_var,_def) \
        { \
            _name, \
            _opt, \
            _desc, \
            OPTION_BOOL, \
            wrapOptionBool(&_var), \
            defOptionBool(_def) \
        }

#define anSanityCheckOption(_name,_opt,_desc,_var,_def) \
        { \
            _name, \
            _opt, \
            _desc, \
            OPTION_BOOL, \
            wrapOptionBool(&_var), \
            defOptionBool(_def) \
        }

#define anTemporaldbOption(_name,_opt,_desc,_var,_def) \
        { \
            _name, \
            _opt, \
            _desc, \
            OPTION_BOOL, \
            wrapOptionBool(&_var), \
            defOptionBool(_def) \
        }

#define anUncertaintyOption(_name,_opt,_desc,_var,_def) \
        { \
            _name, \
            _opt, \
            _desc, \
            OPTION_BOOL, \
            wrapOptionBool(&_var), \
            defOptionBool(_def) \
        }


#define OPT_POS(name) INT_VALUE(MAP_GET_STRING(optionPos,name))

// array storing information for all supported options
OptionInfo opts[] =
{
        // show help only and quit
        {
                OPTION_SHOW_HELP,
                "-help",
                "Show this help text.",
                OPTION_BOOL,
                wrapOptionString(&opt_show_help),
                defOptionBool(FALSE)
        },
        // choose test
        {
                OPTION_TEST_NAME,
                "-test",
                "choose the test to run (ignored by all binaries except test_main)",
                OPTION_STRING,
                wrapOptionString(&opt_test),
                defOptionString(NULL)
        },
        // list tests
        {
                OPTION_LIST_TESTS,
                "-listtests",
                "list available tests (only used by test_main)",
                OPTION_BOOL,
                wrapOptionBool(&opt_listtests),
                defOptionBool(FALSE)
        },
        // show help only and quit
        {
                OPTION_SHOW_LANGUAGE_HELP,
                "-languagehelp",
                "Show supported provenance requests for a supported frontend language.",
                OPTION_STRING,
                wrapOptionString(&opt_language_help),
                defOptionString(NULL)
        },
        // database backend connection options
        {
                OPTION_CONN_HOST,
                "-host",
                "Host IP address for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_host),
                defOptionString("")
        },
        {
                OPTION_CONN_DB,
                "-db",
                "Database name for backend DB connection (SID or SERVICE_NAME for Oracle backends).",
                OPTION_STRING,
                wrapOptionString(&connection_db),
                defOptionString("")
        },
        {
                OPTION_CONN_USER,
                "-user",
                "User for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_user),
                defOptionString("")
        },
        {
                OPTION_CONN_PASSWD,
                "-passwd",
                "Password for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_passwd),
                defOptionString("")
        },
        {
                OPTION_CONN_PORT,
                "-port",
                "TCP/IP port for backend DB connection.",
                OPTION_INT,
                wrapOptionInt(&connection_port),
                defOptionInt(1521)
        },
        // backend specific options
        {
                OPTION_ORACLE_AUDITTABLE,
                "-Boracle.audittable",
                "Table storing the audit log (usually fga_log$ or unified_audit_trail)",
                OPTION_STRING,
                wrapOptionString(&oracle_audit_log_table),
                defOptionString("UNIFIED_AUDIT_TRAIL")
        },
        {
                OPTION_ORACLE_USE_SERVICE,
                "-Boracle.servicename",
                "if this option then the db connection parameter is interpreted as a service name instead of an SID",
                OPTION_BOOL,
                wrapOptionString(&oracle_use_service_name),
                defOptionBool(FALSE)
        },
        {
                OPTION_ODBC_DRIVER,
                "-Bodbc.driver",
                "Name of the ODBC driver to use.",
                OPTION_STRING,
                wrapOptionString(&odbc_driver),
                defOptionString("")
        },
        // logging options
        {
                OPTION_LOG_LEVEL,
                "-loglevel",
                "Log level determining log output: TRACE=5, DEBUG=4, INFO=3, WARN=2, ERROR=1, FATAL=0",
                OPTION_INT,
                wrapOptionInt(&logLevel),
                defOptionInt(1)
        },
        {
                OPTION_LOG_ACTIVE,
                "-log",
                "Activate/Deactivate logging",
                OPTION_BOOL,
                wrapOptionBool(&logActive),
                defOptionBool(TRUE)
        },
		{
                OPTION_LOG_OPERATOR_COLORIZED,
                "-Loperator_colorize",
                "Colorize relational algebra operator overviews",
                OPTION_BOOL,
                wrapOptionBool(&opt_log_operator_colorize),
                defOptionBool(TRUE)
        },
		{
                OPTION_LOG_OPERATOR_VERBOSE,
                "-Loperator_verbose",
                "Relational algebra operator overviews are verbose",
                OPTION_BOOL,
                wrapOptionBool(&opt_log_operator_verbose),
                defOptionBool(FALSE)
        },
        {
                OPTION_LOG_OPERATOR_VERBOSE_PROPS,
                "-Loperator_verbose_props",
                "Relational algebra operator overviews print properties (requires -Loperator_verbose): KEYS&VALUES=2, KEYS=1, NONE=0",
                OPTION_INT,
                wrapOptionInt(&opt_log_operator_verbose_props),
                defOptionInt(0)
        },
        // input options
        {
                OPTION_INPUT_SQL,
                "-sql",
                "input query",
                OPTION_STRING,
                wrapOptionString(&sql),
                defOptionString(NULL)
        },
        {
                OPTION_INPUT_QUERY,
                "-query",
                "input query",
                OPTION_STRING,
                wrapOptionString(&sql),
                defOptionString(NULL)
        },
        {
                 OPTION_INPUT_QUERY_FILE,
                 "-queryFile",
                 "input query file name",
                 OPTION_STRING,
                 wrapOptionString(&sqlFile),
                 defOptionString(NULL)
        },
        {
                OPTION_INPUT_SQL_FILE,
                "-sqlfile",
                "input SQL file name",
                OPTION_STRING,
                wrapOptionString(&sqlFile),
                defOptionString(NULL)
        },
        {
                OPTION_INPUTDB,
                "-inputdb",
                "output samples of input database relations",
                OPTION_BOOL,
                wrapOptionBool(&opt_inputdb),
                defOptionBool(FALSE)
        },
        // backend, frontend and plugin selection
        {
                OPTION_BACKEND,
                "-backend",
                "select backend database type: postgres, oracle, sqlite, duckdb - this determines analyzer, parser, metadata-lookup, sql-code generator, and translator plugins",
                OPTION_STRING,
                wrapOptionString(&backend),
                defOptionString(NULL)
        },
        {
                OPTION_FRONTEND,
                "-frontend",
                "select frontend language: oracle, dl - this determines analyzer, parser, and translator plugins",
                OPTION_STRING,
                wrapOptionString(&frontend),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_METADATA,
                "-Pmetadata",
                "select metadatalookup plugin: oracle, postgres, sqlite, duckdb",
                OPTION_STRING,
                wrapOptionString(&plugin_metadata),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_PARSER,
                "-Pparser",
                "select parser plugin: oracle, dl",
                OPTION_STRING,
                wrapOptionString(&plugin_parser),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_SQLCODEGEN,
                "-Psqlcodegen",
                "select SQL code generator plugin: oracle, postgres, sqlite, duckdb",
                OPTION_STRING,
                wrapOptionString(&plugin_sql_serializer),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_ANALYZER,
                "-Panalyzer",
                "select parser result model analyzer: oracle, dl",
                OPTION_STRING,
                wrapOptionString(&plugin_analyzer),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_TRANSLATOR,
                "-Ptranslator",
                "select parser result to relational algebra translator: oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_translator),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_SQLSERIALIZER,
                "-Psqlserializer",
                "select SQL code generator plugin: oracle, postgres, sqlite, duckdb, dl, lb",
                OPTION_STRING,
                wrapOptionString(&plugin_sql_serializer),
                defOptionString(NULL)
        },
        {
                OPTION_PLUGIN_EXECUTOR,
                "-Pexecutor",
                "select Executor plugin: sql (output rewritten SQL code), "
                        "gp (output Game provenance), run (execute the "
                        "rewritten query and return its result)",
                OPTION_STRING,
                wrapOptionString(&plugin_executor),
                defOptionString("run")
        },
        {
                OPTION_PLUGIN_CBO,
                "-Pcbo",
                "select Cost-Based Optimizer plugin: exhaustive (enumerate all options), "
                        "balance (stop optimization after optimization time exceeds estimated runtime of best plan), "
                        "sim_ann (simmulated annealing)",
                OPTION_STRING,
                wrapOptionString(&plugin_cbo),
                defOptionString(NULL)
        },
        // boolean instrumentation options
		{
                OPTION_TIMING,
                "-timing",
                "measure and output execution time of modules.",
                OPTION_BOOL,
                wrapOptionBool(&opt_timing),
                defOptionBool(FALSE)
        },
        {
                OPTION_MEMMEASURE,
                "-memdebug",
                "measure and output memory allocation stats.",
                OPTION_BOOL,
                wrapOptionBool(&opt_memmeasure),
                defOptionBool(FALSE)
        },
        aRewriteOption(OPTION_GRAPHVIZ,
                "-show_graphviz",
                "output created relational algebra graphs as graphviz scripts.",
                opt_graphviz_output,
                FALSE),
        aRewriteOption(OPTION_GRAPHVIZ_DETAILS,
                "-graphviz_details",
                "show operator parameters in graphviz scripts.",
                opt_graphviz_detail ,
                FALSE),
        aRewriteOption(OPTION_TIME_QUERIES,
                "-time_queries",
                "measure query runtimes (only makes a difference for executor <run>).",
                opt_show_query_runtime,
                FALSE),
        {
                OPTION_TIME_QUERY_OUTPUT_FORMAT,
                "-time_query_format",
                "format used for printing query timing results. "
                        "The format is printf compatible and should contain "
                        "exactly on %f element (additional formating such as %12f is ok)",
                OPTION_STRING,
                wrapOptionString(&time_query_format),
                defOptionString(NULL)
        },
        {
                OPTION_REPEAT_QUERY,
                "-repeat_query_count",
                "execute query this many times (useful for timing).",
                OPTION_INT,
                wrapOptionInt(&query_repeat_count),
                defOptionInt(1)
        },
        aRewriteOption(OPTION_SHOW_QUERY_RESULT,
                "-show_result",
                "show query result (only makes a difference for executor <run>).",
                opt_show_query_result,
                TRUE),
        // boolean rewrite options
        aRewriteOption(OPTION_AGGRESSIVE_MODEL_CHECKING,
                "-aggressive_model_checking",
                "do aggressive validity checking of AGM models.",
                opt_aggressive_model_checking,
                FALSE),
        aRewriteOption(OPTION_UPDATE_ONLY_USE_CONDS,
                "-prov_reenact_only_updated_rows_use_conditions",
                "Use disjunctions of update conditions to filter out tuples from "
                "transaction provenance that are not updated by the transaction.",
                opt_update_only_conditions,
                TRUE),
        aRewriteOption(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN,
                "-prov_reenact_only_updated_rows_use_hist_join",
                "Use a join between the version at commit time with the table version"
                " at transaction start to prefilter rows that were not updated by the transaction.",
                opt_only_updated_use_history,
                FALSE),
        aRewriteOption(OPTION_TREEIFY_OPERATOR_MODEL,
                "-treeify-algebra-graphs",
                "Turn AGM graph into a tree before passing it off to the provenance rewriter.",
                opt_treeify_opterator_model,
                TRUE),
        aRewriteOption(OPTION_ALWAYS_TREEIFY,
                "-treeify-all",
                "Turn AGM graph into a tree passing it to serializer.",
                opt_treeify_all,
                FALSE),
        aRewriteOption(OPTION_PI_CS_USE_COMPOSABLE,
                "-prov_use_composable",
                "Use composable version of PI-CS provenance that adds additional columns which"
                " enumerate duplicates introduced by provenance.",
                opt_pi_cs_composable,
                FALSE),
        aRewriteOption(OPTION_PI_CS_COMPOSABLE_REWRITE_AGG_WINDOW,
                "-prov_instrument_agg_window",
                "When composable version of PI-CS provenance is use then rewrite aggregations using window functions.",
                opt_pi_cs_rewrite_agg_window,
                TRUE),
        aRewriteOption(OPTION_TRANSLATE_UPDATE_WITH_CASE,
                "-prov_reenact_update_with_case",
                "Create reenactment query for UPDATE statements using CASE instead of UNION.",
                opt_translate_update_with_case,
                TRUE),
		aRewriteOption(OPTION_LATERAL_REWRITE,
				"-lateral_rewrite",
				"Activate automatic rewrite of nested subqueries into LATERAL queries.",
				opt_lateral_rewrite,
				FALSE),
		aRewriteOption(OPTION_UNNEST_REWRITE,
						"-unnest_rewrite",
						"Activate unnest & de-correlation rewrites.",
						opt_unnest_rewrite,
						FALSE),
		aRewriteOption(OPTION_AGG_REDUCTION_MODEL_REWRITE,
				"-agg_reduction_model_rewrite",
				"Activate aggregation reduction model rewrite",
				opt_agg_reduction_model_rewrite,
				FALSE),
        // Optimization Options
        {
                OPTION_OPTIMIZE_OPERATOR_MODEL,
                "-heuristic_opt",
                "Activate heuristic relational algebra optimization",
                OPTION_BOOL,
                wrapOptionBool(&opt_optimize_operator_model),
                defOptionBool(FALSE)
        },
        {
				OPTION_COST_BASED_OPTIMIZER,
				"-cbo",
				"Activate cost based optimizer",
				OPTION_BOOL,
				wrapOptionBool(&cost_based_optimizer),
				defOptionBool(FALSE)
		 },
         {
        		OPTION_COST_BASED_CLOSE_OPTION_REMOVEDP_BY_SET,
                "-cbo_choice_point_remove_duplicate_removal",
                "Close cost based remove duplicate remove op by set option",
                OPTION_BOOL,
                wrapOptionBool(&cost_based_close_option_removedp_by_set),
                defOptionBool(FALSE)
         },
         {
                 OPTION_COST_BASED_MAX_PLANS,
                 "-cbo_max_considered_plans",
                 "Maximal number of plans considered by cost based optimizer",
                 OPTION_INT,
                 wrapOptionInt(&cost_max_considered_plans),
                 defOptionInt(200)
         },
         {
                 OPTION_COST_BASED_SIMANN_CONST,
                 "-cbo_sim_ann_const",
                 "Cost based optimzation: set the constant used by simulated  annealing to calculate ap, e.g., c = 10, 20, 50 or 100",
                 OPTION_INT,
                 wrapOptionInt(&cost_sim_ann_const),
                 defOptionInt(10)
         },
         {
                 OPTION_COST_BASED_SIMANN_COOLDOWN_RATE,
                 "-cbo_sim_ann_cooldown_rate",
                 "Cost based optimization: Set the cool down rate used by simulated annealing between 0.1 and 0.9, 1 means 0.1",
                 OPTION_INT,
                 wrapOptionInt(&cost_sim_ann_cooldown_rate),
                 defOptionFloat(5)
         },
         {
        		 OPTION_COST_BASED_NUM_HEURISTIC_OPT_ITERATIONS,
                 "-cbo_num_heuristic_opt_iterations",
                 "Cost base number of heuristic optimization iterations",
                 OPTION_INT,
                 wrapOptionInt(&cost_based_num_heuristic_opt_iterations),
                 defOptionInt(1)
         },
         {
        		 OPTION_MAX_NUMBER_PARTITIONS_FOR_USE,
                 "-cmax_number_paritions_for_uses",
                 "max number of partitions can be used in any clause",
                 OPTION_INT,
                 wrapOptionInt(&max_number_paritions_for_uses),
                 defOptionInt(0)
         },
		 {
				 OPTION_BIT_VECTOR_SIZE,
				 "-ps_bit_vector_size",
				 "bit vector length used in bit or",
				 OPTION_INT,
				 wrapOptionInt(&bit_vector_size),
				 defOptionInt(32)
		 },
		 {
				 OPTION_PS_STORE_TABLE,
				 "-ps_store_table",
				 "the table name used to store the ps information",
				 OPTION_STRING,
				 wrapOptionString(&ps_store_table),
				 defOptionString(NULL)
		 },
		 {
				 OPTION_PS_BINARY_SEARCH,
				 "-ps_binary_search",
				 "Activate binary search instead of case when",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_binary_search),
				 defOptionBool(FALSE)
		 },
		 {
				 OPTION_PS_BINARY_SEARCH_CASE_WHEN,
				 "-ps_binary_search_case_when",
				 "Activate binary search case when",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_binary_search_case_when),
				 defOptionBool(FALSE)
		 },
		 {
				 OPTION_PS_SETTINGS,
				 "-ps_settings",
				 "Activate settings about provenance sketch",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_settings),
				 defOptionBool(FALSE)
		 },
		 {
				 OPTION_PS_SET_BITS,
				 "-ps_set_bits",
				 "Activate set_bits about provenance sketch",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_set_bits),
				 defOptionBool(FALSE)
		 },
		 {
				 OPTION_PS_ANALYZE,
				 "-ps_analyze",
				 "Activate ps_analyze about provenance sketch",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_analyze),
				 defOptionBool(TRUE)
		 },
		 {
				 OPTION_PS_USE_NEST,
				 "-ps_use_nest",
				 "Activate ps_use_nest about provenance sketch",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_use_nest),
				 defOptionBool(FALSE)
		 },
		 {
				 OPTION_PS_POST_TO_ORACLE,
				 "-ps_post_to_oracle",
				 "Activate using postgres generate oracle sql",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_post_to_oracle),
				 defOptionBool(FALSE)
		 },
		 {
				 OPTION_PS_USE_BRIN_OP,
				 "-ps_use_brin_op",
				 "Activate use_brin_op about provenance sketch",
				 OPTION_BOOL,
				 wrapOptionBool(&ps_use_brin_op),
				 defOptionBool(FALSE)
		 },
        // AGM (Query operator model) individual optimizations
        anOptimizationOption(OPTIMIZATION_SELECTION_PUSHING,
                "-Opush_selections",
                "Optimization: Activate selection move-around",
                opt_optimization_push_selections,
                FALSE
        ),
        anOptimizationOption(OPTIMIZATION_MERGE_OPERATORS,
                "-Omerge_ops",
                "Optimization: try to merge adjacent selection and projection operators",
                opt_optimization_merge_ops,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR,
                "-Ofactor_attrs",
                "Optimization: try to factor attribute references in projection"
                " expressions to open up new operator merging opportunities",
                opt_optimization_factor_attrs,
                FALSE
        ),
        anOptimizationOption(OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ,
                "-Omaterialize_unsafe_proj",
                "Optimization: add materialization hint for projections that "
                "if merged with adjacent projection would cause expontential "
                "expression size blowup",
				opt_optimization_materialize_unsafe_proj,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_MERGE_UNSAFE_PROJECTIONS,
                "-Omerge_unsafe_proj",
                "Optimization: merge projections even "
                "if this may cause an expontential blowup in "
                "expression size",
				opt_optimization_merge_unsafe_proj,
                FALSE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_REDUNDANT_PROJECTIONS,
                "-Oremove_redundant_projections",
                "Optimization: try to remove redundant projections",
                opt_optimization_remove_redundant_projections,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR,
                "-Oremove_redundant_duplicate_removals",
                "Optimization: try to remove redundant duplicate removal operators",
                opt_optimization_remove_redundant_duplicate_operator,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_UNNECESSARY_WINDOW_OPERATORS,
                "-Oremove_redundant_window_operators",
                "Optimization: try to remove redundant window operators",
                opt_optimization_remove_unnecessary_window_operators,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_UNNECESSARY_COLUMNS,
                "-Oremove_unnecessary_columns",
                "Optimization: try to remove unnecessary columns that are not used by the query",
                opt_optimization_remove_unnecessary_columns,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_PULL_UP_DUPLICATE_REMOVE_OPERATORS,
        		"-Opullup_duplicate_removals",
        		"Optimization: try to pull up duplicate remove operators",
        		opt_optimization_pull_up_duplicate_remove_operators,
        		TRUE
        ),
        anOptimizationOption(OPTIMIZATION_PULLING_UP_PROVENANCE_PROJ,
                "-Opullup_prov_projections",
                "Optimization: try to pull up provenance projection",
                opt_optimization_pulling_up_provenance_proj,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_SELECTION_PUSHING_THROUGH_JOINS,
                "-Opush_selections_through_joins",
                "Optimization: try to push selections through joins",
                opt_optimization_push_selections_through_joins,
                FALSE
        ),
        anOptimizationOption(OPTIMIZATION_SELECTION_MOVE_AROUND,
                "-Oselection_move_around",
                "Optimization: try to move selection operators around to push them down including side-way information passing",
                opt_optimization_selection_move_around,
                TRUE
        ),
		anOptimizationOption(OPTIMIZATION_PUSH_DOWN_AGGREGATION_THROUGH_JOIN,
				"-Opush_down_aggregation_through_join",
				"Optimization: try to push down aggregation through join",
				opt_optimization_push_down_aggregation_through_join,
				TRUE
		),
        // temporal database options for coalesce and normalization
        anTemporaldbOption(TEMPORAL_USE_COALSECE,
                "-temporal_use_coalesce",
                "Temporaldb: Activate coalesce",
				temporal_use_coalesce,
                TRUE
        ),
		anTemporaldbOption(TEMPORAL_USE_NORMALIZATION,
                "-temporal_use_normalization",
                "Temporaldb: Activate normalization",
				temporal_use_normalization,
                TRUE
        ),
		anTemporaldbOption(TEMPORAL_USE_NORMALIZATION_WINDOW,
                "-temporal_use_normalization_window",
                "Temporaldb: Activate normalization using window",
				temporal_use_normalization_window,
                FALSE
        ),
        anTemporaldbOption(TEMPORAL_AGG_WITH_NORM,
                "-temporal_agg_combine_with_norm",
                "Temporaldb: rewrite and aggregation by applying a rewrite that combines aggregation with normalization",
                temporal_agg_combine_with_norm,
                TRUE
        ),
        // sanity model checking options
        anSanityCheckOption(CHECK_OM_UNIQUE_ATTR_NAMES,
                "-Cunique_attr_names",
                "Model Check: check that attribute names are unique for each operator's schema",
                opt_operator_model_unique_schema_attribues,
                TRUE
        ),
        anSanityCheckOption(CHECK_OM_PARENT_CHILD_LINKS,
                "-Cparent_child_links",
                "Model Check: check that an query operator graph is correctly "
                "connected. For example, if X is a child of Y then Y should"
                " be a parent of X.",
                opt_operator_model_parent_child_links ,
                TRUE
        ),
        anSanityCheckOption(CHECK_OM_SCHEMA_CONSISTENCY,
                "-Cschema_consistency",
                "Model Check: Perform operator type specific sanity checks"
                " on the schema of an operator. For example, the number of"
                " attributes in a projection's schema should be equal to the"
                " number of projection expressions.",
                opt_operator_model_schema_consistency,
                TRUE
        ),
        anSanityCheckOption(CHECK_OM_ATTR_REF,
                "-Cattr_reference_consistency",
                "Model Check: check that attribute references used in "
                "expressions are consistent. For instance, they have to "
                "refer to existing inputs and attributes.",
                opt_operator_model_attr_reference_consistency,
                TRUE
        ),
        // dl rewrite options
		{
				OPTION_WHYNOT_ADV,
				"-whynot_adv",
				"advanced way to create firing rules for whynot.",
				OPTION_BOOL,
				wrapOptionBool(&opt_whynot_adv),
				defOptionBool(FALSE)
		},
		{
			OPTION_DL_SEMANTIC_OPT,
			"-Osemantic_opt",
			"Use functional dependencies to minimizing a provenance capture datalog query.",
			OPTION_BOOL,
			wrapOptionBool(&opt_dl_min_with_fds),
			defOptionBool(FALSE)
		},
		{
			OPTION_DL_MERGE_RULES,
			"-Oflatten_dl",
			"Merge Datalog rules by substituting idb predicates in bodies with the rules that define them.",
			OPTION_BOOL,
			wrapOptionBool(&opt_merge_dl),
			defOptionBool(FALSE)
		},
		{
			OPTION_DL_FETCH_PK_FDS_FROM_DB,
			"-dl_load_fds",
			"Merge Datalog rules by substituting idb predicates in bodies with the rules that define them.",
			OPTION_BOOL,
			wrapOptionBool(&opt_load_fds),
			defOptionBool(FALSE)
		},
        anSanityCheckOption(CHECK_OM_DATA_STRUCTURE_CONSISTENCY,
                "-Cdata_structure_consistency",
                "Model Check: check that nodes in a query operator graph are not sharing "
                "datastructures incorrectly.",
                opt_operator_model_data_structure_consistency,
                TRUE
        ),
        // Unercainty options
        anUncertaintyOption(RANGE_OPTIMIZE_JOIN,
                "-range_optimize_join",
                "Range rewriter: Optimized join rewriting.",
                range_optimize_join,
                TRUE
        ),
        anUncertaintyOption(RANGE_OPTIMIZE_AGG,
                "-range_optimize_agg",
                "Range rewriter: Optimized aggregation rewriting.",
                range_optimize_agg,
                TRUE
        ),
        {
                 RANGE_COMPRESSION_RATE,
                 "-range_compression_rate",
                 "Range rewriter: Set rate of compression for possible, number indicates iterations where 1=split by half and 2=split by quarter...",
                 OPTION_INT,
                 wrapOptionInt(&range_compression_rate),
                 defOptionInt(1)
         },
        // stopper to indicate end of array
        {
                STOPPER_STRING,
                NULL,
                NULL,
                OPTION_STRING,
                wrapOptionString(NULL),
                defOptionString("")
        }
};

// backend plugins information
BackendInfo backends[]  = {
        {
            BACKEND_ORACLE,
            "oracle",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "oracle",   // metadata
            "oracle",   // sqlserializer
            "oracle"   // translator
        },
        {
            BACKEND_POSTGRES,
            "postgres",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "postgres",   // metadata
            "postgres",    // sqlserializer
            "oracle"   // translator
        },
        {
            BACKEND_SQLITE,
            "sqlite",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "sqlite",   // metadata
            "sqlite",    // sqlserializer
            "oracle"   // translator
        },
        {
            BACKEND_DUCKDB,
            "duckdb",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "duckdb",   // metadata
            "sqlite",    // sqlserializer
            "oracle"   // translator
        },
        {
            BACKEND_MONETDB,
            "monetdb",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "monetdb",   // metadata
            "sqlite",    // sqlserializer
            "oracle"   // translator
        },
		{
			BACKEND_MSSQL,
            "mssql",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "mssql",   // metadata
            "postgres",    // sqlserializer
            "oracle"   // translator
		},
        {
            BACKEND_ORACLE, STOPPER_STRING, NULL, NULL, NULL, NULL, NULL
        }
};

// frontend plugins information
FrontendInfo frontends[]  = {
        {
            "oracle",   // name
            "oracle",   // analyzer
            "oracle",   // parser
            "oracle"   // translator
        },
        {
            "dl",   // name
            "dl",   // analyzer
            "dl",   // parser
            "dl"   // translator
        },
        {
            STOPPER_STRING, NULL, NULL, NULL
        }
};

static void
initOptions(void)   //TODO make this threadsafe
{
    // create hashmap option -> position in option info array for lookup
    optionPos = NEW_MAP(Constant,Constant);
    cmdOptionPos = NEW_MAP(Constant,Constant);
    backendInfo = NEW_MAP(Constant,HashMap);
    frontendInfo = NEW_MAP(Constant,HashMap);

    // add options to hashmap and set all options to default values
    for(int i = 0; strcmp(opts[i].option,STOPPER_STRING) != 0; i++)
    {
        OptionInfo *o = &(opts[i]);
        setDefault(o);
        ASSERT(!MAP_HAS_STRING_KEY(optionPos, o->option)); // no two option with same identifier
        ASSERT(!MAP_HAS_STRING_KEY(cmdOptionPos, o->cmdLine));

        MAP_ADD_STRING_KEY(optionPos, o->option, createConstInt(i));
        if (o->cmdLine == NULL)
            MAP_ADD_STRING_KEY(cmdOptionPos, o->option, createConstInt(i));
        else
            MAP_ADD_STRING_KEY(cmdOptionPos, o->cmdLine, createConstInt(i));
    }

    // create backend infos
    for(int i = 0; strcmp(backends[i].backendName,STOPPER_STRING) != 0; i++)
    {
        HashMap *newMap = NEW_MAP(Constant, Constant);
        char *name = backends[i].backendName;

        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_ANALYZER, backends[i].analyzer);
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_PARSER, backends[i].parser);
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_METADATA, backends[i].metadata);
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_SQLSERIALIZER, backends[i].sqlserializer);
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_SQLCODEGEN, backends[i].sqlserializer); //for backward compatibility for now
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_TRANSLATOR, backends[i].translator);


        MAP_ADD_STRING_KEY(backendInfo, name, newMap);
    }

    // create frontend infos
    for(int i = 0; strcmp(frontends[i].frontendName,STOPPER_STRING) != 0; i++)
    {
        HashMap *newMap = NEW_MAP(Constant, Constant);
        char *name = frontends[i].frontendName;

        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_ANALYZER, frontends[i].analyzer);
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_PARSER, frontends[i].parser);
        MAP_ADD_STRING_KEY_AND_VAL(newMap, OPTION_PLUGIN_TRANSLATOR, frontends[i].translator);

        MAP_ADD_STRING_KEY(frontendInfo, name, newMap);
    }
}

static void
setDefault(OptionInfo *o)
{
    switch(o->valueType)
    {
        case OPTION_INT:
            *(o->value.i) = o->def.i;
            break;
        case OPTION_FLOAT:
            *(o->value.f) = o->def.f;
            break;
        case OPTION_STRING:
            {
                if (o->def.string == NULL)
                    *(o->value.string) = NULL;
                else
                {
                    char *newS = malloc(sizeof(char) * (strlen(o->def.string) + 1));
                    strcpy(newS, o->def.string);
                    *(o->value.string) = newS;
                }
            }
            break;
        case OPTION_BOOL:
            *(o->value.b) = o->def.b;
            break;
    }
}

void
mallocOptions()
{
	initOptions();
}

void
freeOptions()
{
    //TODO
}




boolean
isRewriteOptionActivated(char *name)
{
    return getBoolOption(name);
}

static OptionValue *
getValue (char *name)
{
    OptionInfo *i = NULL;
    ASSERT(hasOption(name));

    i = &(opts[OPT_POS(name)]);

    return &(i->value);
}

static OptionInfo *
getInfo (char *name)
{
    ASSERT(hasOption(name));

    return &(opts[OPT_POS(name)]);
}

char *
getOptionAsString (char *name)
{
    OptionInfo *v;

    ASSERT(hasOption(name));
    v = getInfo(name);

    return valGetString(&v->value, v->valueType);
}

char *
getStringOption (char *name)
{
    ASSERT(hasOption(name));

    return *(getValue(name)->string);
}

int
getIntOption (char *name)
{
    ASSERT(hasOption(name));

    return *(getValue(name)->i);
}

boolean
getBoolOption (char *name)
{
    ASSERT(hasOption(name));

    return *(getValue(name)->b);
}

double
getFloatOption (char *name)
{
    ASSERT(hasOption(name));

    return *(getValue(name)->f);
}

void
setOption (char *name, char *value)
{
    OptionInfo *v;
    ASSERT(hasOption(name));
    v = getInfo(name);

    switch(v->valueType)
    {
        case OPTION_BOOL:
            setBoolOption(name, parseBool(value));
            break;
        case OPTION_STRING:
            setStringOption(name, value);
            break;
        case OPTION_INT:
            setIntOption(name, parseInt(value));
            break;
        case OPTION_FLOAT:
            break;
    }
}


void
setStringOption (char *name, char *value)
{
    OptionValue *v;
    ASSERT(hasOption(name));
    v = getValue(name);

    *(v->string) = value;
}

void
setIntOption(char *name, int value)
{
    OptionValue *v;
    ASSERT(hasOption(name));
    v = getValue(name);

    *(v->i) = value;
}

void
setBoolOption(char *name, boolean value)
{
    OptionValue *v;
    ASSERT(hasOption(name));
    v = getValue(name);

    *(v->b) = value;
}

void
setFloatOption(char *name, double value)
{
    OptionValue *v;
    ASSERT(hasOption(name));
    v = getValue(name);

    *(v->f) = value;
}

void
setOptionsFromMap(HashMap *opts)
{
    FOREACH_HASH_ENTRY(k,opts)
    {
        char *name = STRING_VALUE(k->key);
        char *value = STRING_VALUE(k->value);

        setOption(name, value);
    }
}

boolean
hasOption(char *name)
{
    return MAP_HAS_STRING_KEY(optionPos,name);
}

boolean
hasCommandOption(char *name)
{
    return MAP_HAS_STRING_KEY(cmdOptionPos,name);
}

char *
commandOptionGetOption(char *cmd)
{
    OptionInfo *i = &(opts[INT_VALUE(MAP_GET_STRING(cmdOptionPos, cmd))]);

    return i->option;
}

OptionType
getOptionType(char *name)
{
    ASSERT(hasOption(name));
    OptionInfo *i = getInfo(name);

    return i->valueType;
}

boolean
optionSet(char *name)
{
    return TRUE;
}

#define PRINT_VERSION_STRING "gprom version %s\n"

void
printVersion(FILE *stream)
{
    fprintf(stream, PRINT_VERSION_STRING, PACKAGE_VERSION);
    fflush(stream);
}

void
printOptionsHelp(FILE *stream, char *progName, char *description, boolean showValues)
{
    fprintf(stream, "%s:\n\t%s\n\nthe following options are supported:\n\n",
            progName, description);

    FOREACH_HASH_KEY(Constant,k,optionPos)
    {
        char *name = STRING_VALUE(k);
        OptionInfo *v = getInfo(name);

        if (showValues)
        {
            fprintf(stream, "%-50s\t%-30sDEFAULT VALUE: %s\tACTUAL VALUE: %s\n\t%s\n\n",
                    v->cmdLine ? v->cmdLine : "-activate/-deactivate ",
                    v->cmdLine ? "" : v->option,
                    defGetString(&v->def, v->valueType),
                    valGetString(&v->value, v->valueType),
                    v->description);
        }
        else
        {
            fprintf(stream, "%s%s\tDEFAULT VALUE: %s\n\t%s\n",
                    v->cmdLine ? v->cmdLine : "-activate/-deactivate ",
                    v->cmdLine ? "" : v->option,
                    defGetString(&v->def, v->valueType),
                    v->description);
        }
    }
    fflush(stream);
}

void
printCurrentOptions(FILE *stream)
{
    FOREACH_HASH_KEY(Constant,k,optionPos)
    {
        char *name = STRING_VALUE(k);
        OptionInfo *v = getInfo(name);
        fprintf(stream, "%50s\tVALUE: %s\n",
                v->option,
                valGetString(&v->value, v->valueType));
    }
}

char *
internalOptionsToString(boolean showValues)
{
    StringInfo result = makeStringInfo();
    char *str;

    FOREACH_HASH_KEY(Constant,k,optionPos)
      {
          char *name = STRING_VALUE(k);
          OptionInfo *v = getInfo(name);

          if (showValues)
          {
              appendStringInfo(result, "%s=%s DEFAULT VALUE: %s\n\t%s\n\n",
                      v->option,
                      valGetString(&v->value, v->valueType),
                      defGetString(&v->def, v->valueType),
                      v->description);
          }
          else
          {
              appendStringInfo(result, "%s\tDEFAULT VALUE: %s\n\t%s\n",
                      v->option,
                      defGetString(&v->def, v->valueType),
                      v->description);
          }
      }

    str = result->data;
    return str;
}

char *
optionsToStringOnePerLine(void)
{
    StringInfo result = makeStringInfo();
    char *str;

    FOREACH_HASH_KEY(Constant,k,optionPos)
    {
        char *name = STRING_VALUE(k);
        OptionInfo *v = getInfo(name);
        appendStringInfo(result, "%50s\tVALUE: %s\n",
                v->option,
                valGetString(&v->value, v->valueType));
    }

    str = result->data;
//    free(result);
    return str;
}

HashMap *
optionsToHashMap(void)
{
    HashMap *result = NEW_MAP(Constant,Constant);

    FOREACH_HASH_KEY(Constant,k,optionPos)
    {
        char *name = STRING_VALUE(k);
        OptionInfo *v = getInfo(name);
        MAP_ADD_STRING_KEY(result,name, createConstString(valGetString(&v->value, v->valueType)));
    }

    return result;
}

char *
getBackendPlugin(char *be, char *pluginOpt)
{
    HashMap *bInfo = (HashMap *) MAP_GET_STRING(backendInfo, be);

	if(bInfo == NULL)
	{
		FATAL_LOG("backend %s not defined", be);
	}

    return STRING_VALUE(MAP_GET_STRING(bInfo, pluginOpt));
}

BackendType
getBackend(void)
{
    if (backend == NULL)
        return BACKEND_ORACLE;
    for(int i = 0; strcmp(backends[i].backendName,STOPPER_STRING) != 0; i++)
    {
        if (streq(backends[i].backendName, backend))
            return backends[i].typ;
    }

    return BACKEND_ORACLE;
}

char *
getFrontendPlugin(char *fe, char *pluginOpt)
{
    HashMap *fInfo = (HashMap *) MAP_GET_STRING(frontendInfo, fe);

    return STRING_VALUE(MAP_GET_STRING(fInfo, pluginOpt));
}

static char *
valGetString(OptionValue *def, OptionType type)
{
    switch(type)
    {
        case OPTION_INT:
            return gprom_itoa(*(def->i));
        case OPTION_STRING:
            return *def->string ? *def->string : "NULL";
        case OPTION_BOOL:
            return (*def->b ? "TRUE" : "FALSE");
        case OPTION_FLOAT:
        {
            char *buf = malloc(sizeof(char) * 50);
            snprintf(buf,50,"%f", *def->f);
            return buf;
        }
    }
    FATAL_LOG("should never end up here");
    return NULL; //keep compiler quit
}


static char *
defGetString(OptionDefault *def, OptionType type)
{
    switch(type)
    {
        case OPTION_INT:
            return gprom_itoa(def->i);
        case OPTION_STRING:
            return def->string ? def->string : "NULL";
        case OPTION_BOOL:
            return (def->b ? "TRUE" : "FALSE");
        case OPTION_FLOAT:
        {
            char *buf = malloc(sizeof(char) * 50);
            snprintf(buf,50,"%f", def->f);
            return buf;
        }
    }

    return NULL; //keep compiler quit
}
