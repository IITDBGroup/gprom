#ifndef OPTION_H
#define OPTION_H

#include "common.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "utility/enum_magic.h"

NEW_ENUM_WITH_TO_STRING(
    OptionType,
    OPTION_BOOL,
    OPTION_STRING,
    OPTION_INT,
    OPTION_FLOAT
);
//typedef enum OptionType {

/* help options */
#define OPTION_SHOW_HELP "help"
#define OPTION_SHOW_LANGUAGE_HELP "languagehelp"

/* connection options */
#define OPTION_CONN_USER "connection.user"
#define OPTION_CONN_PASSWD "connection.passwd"
#define OPTION_CONN_DB "connection.db"
#define OPTION_CONN_PORT "connection.port"
#define OPTION_CONN_HOST "connection.host"
#define OPTION_ODBC_DRIVER "connection.odbcdriver"

/* backend specific options */
#define OPTION_ORACLE_AUDITTABLE "backendOpts.oracle.logtable"
#define OPTION_ORACLE_USE_SERVICE "backendOpts.oracle.use_service"

/* test options */
#define OPTION_TEST_NAME "test"
#define OPTION_LIST_TESTS "listtests"

/* input options */
#define OPTION_INPUT_SQL "input.sql"
#define OPTION_INPUT_SQL_FILE "input.sqlFile"
#define OPTION_INPUT_QUERY "input.query"
#define OPTION_INPUT_QUERY_FILE "input.queryFile"
#define OPTION_INPUTDB "inputdb"

/* logging */
#define OPTION_LOG_ACTIVE "log.active"
#define OPTION_LOG_LEVEL "log.level"
#define OPTION_LOG_OPERATOR_COLORIZED "log.operator_colorized"
#define OPTION_LOG_OPERATOR_VERBOSE "log.operator_verbose"
#define OPTION_LOG_OPERATOR_VERBOSE_PROPS "log.operator_verbose_props"

/* plugin options */
#define OPTION_BACKEND "backend"
#define OPTION_FRONTEND "frontend"
#define OPTION_PLUGIN_ANALYZER "plugin.analyzer"
#define OPTION_PLUGIN_METADATA "plugin.metadata"
#define OPTION_PLUGIN_PARSER "plugin.parser"
#define OPTION_PLUGIN_SQLCODEGEN "plugin.sqlcodegen"
#define OPTION_PLUGIN_SQLSERIALIZER "plugin.sqlserializer"
#define OPTION_PLUGIN_TRANSLATOR "plugin.translator"
#define OPTION_PLUGIN_EXECUTOR "plugin.executor"
#define OPTION_PLUGIN_CBO "plugin.cbo"

/* debug option methods */
#define OPTION_TIMING "timing"
#define OPTION_MEMMEASURE "memdebug"
#define OPTION_GRAPHVIZ "graphviz"
#define OPTION_GRAPHVIZ_DETAILS "graphviz_details"
#define OPTION_AGGRESSIVE_MODEL_CHECKING "aggressive_model_checking"
#define OPTION_TIME_QUERIES "time_queries"
#define OPTION_TIME_QUERY_OUTPUT_FORMAT "time_query_format"
#define OPTION_REPEAT_QUERY "repeat_query_count"
#define OPTION_SHOW_QUERY_RESULT "show_query_result"

/* provennace and some optimization options */
#define OPTION_UPDATE_ONLY_USE_CONDS "only_updated_use_conditions"
#define OPTION_UPDATE_ONLY_USE_HISTORY_JOIN "only_updated_use_history"
#define OPTION_TREEIFY_OPERATOR_MODEL "treefiy_prov_rewrite_input"
#define OPTION_ALWAYS_TREEIFY "always_treeify"
#define OPTION_PI_CS_USE_COMPOSABLE "pi_cs_use_composable"
#define OPTION_PI_CS_COMPOSABLE_REWRITE_AGG_WINDOW "pi_cs_rewrite_agg_window"
#define OPTION_OPTIMIZE_OPERATOR_MODEL "optimize_operator_model"
#define OPTION_TRANSLATE_UPDATE_WITH_CASE "translate_update_with_case"
#define OPTION_COST_BASED_OPTIMIZER "cost_based_optimizer"
#define OPTION_COST_BASED_MAX_PLANS "cost_based_optimizer_max_plans"
#define OPTION_COST_BASED_SIMANN_CONST "cost_based_sim_ann_c"
#define OPTION_COST_BASED_SIMANN_COOLDOWN_RATE "cost_based_sim_ann_cooldown_rate"
#define OPTION_COST_BASED_NUM_HEURISTIC_OPT_ITERATIONS "cost_based_num_heuristic_opt_iterations"
#define OPTION_COST_BASED_CLOSE_OPTION_REMOVEDP_BY_SET "cost_based_close_option_removedp_by_set"
//#define OPTION_

/* optimization options */
#define OPTIMIZATION_SELECTION_PUSHING "optimization.push_selections"
#define OPTIMIZATION_MERGE_OPERATORS "optimization.merge_operators"
#define OPTIMIZATION_MERGE_UNSAFE_PROJECTIONS "optimization.merge_unsafe_projections"
#define OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR "optimization.factor_proj_attr_in_expr"
#define OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ "optimization.materialize_merge_unsafe_proj"
#define OPTIMIZATION_REMOVE_REDUNDANT_PROJECTIONS "optimization.remove_redundant_projections"
#define OPTIMIZATION_PULLING_UP_PROVENANCE_PROJ "optimization.pulling_up_provenance_proj"
#define OPTIMIZATION_SELECTION_PUSHING_THROUGH_JOINS "optimization.push_selections_through_joins"
#define OPTIMIZATION_SELECTION_MOVE_AROUND "optimization.selection_move_around"
#define OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR "optimization.remove_redundant_duplicate_operator"
#define OPTIMIZATION_REMOVE_UNNECESSARY_COLUMNS "optimization.remove_unnecessary_columns"
#define OPTIMIZATION_REMOVE_UNNECESSARY_WINDOW_OPERATORS "optimization.remove_unnecessary_window_operators"
#define OPTIMIZATION_PULL_UP_DUPLICATE_REMOVE_OPERATORS "optimization.pull_up_deplicate_remove_operators"
/* define optimization options for group by*/
#define OPTIMIZATION_PUSH_DOWN_AGGREGATION_THROUGH_JOIN "optimization.push_down_aggregation_through_join"


/* model checking options */
#define CHECK_OM_UNIQUE_ATTR_NAMES "check.unique_attrs"
#define CHECK_OM_PARENT_CHILD_LINKS "check.parent_child_links"
#define CHECK_OM_SCHEMA_CONSISTENCY "check.schema_consistency"
#define CHECK_OM_DATA_STRUCTURE_CONSISTENCY "check.data_structure_consistency"
#define CHECK_OM_ATTR_REF "check.attr_ref_consistency"

/* temporal database options */
#define TEMPORAL_USE_COALSECE "temporal_use_coalesce"
#define TEMPORAL_USE_NORMALIZATION "temporal_use_normalization"
#define TEMPORAL_USE_NORMALIZATION_WINDOW "temporal_use_normalization_window"
#define TEMPORAL_AGG_WITH_NORM "temporal_combine_agg_and_norm"

/* lateral rewrite for nesting operator */
#define OPTION_LATERAL_REWRITE "lateral_rewrite"
#define OPTION_UNNEST_REWRITE "unnest_rewrite"
#define OPTION_AGG_REDUCTION_MODEL_REWRITE "agg_reduction_model_rewrite"


/* use provenance scratch */
#define OPTION_MAX_NUMBER_PARTITIONS_FOR_USE "number_max_paritions_for_use"
#define OPTION_BIT_VECTOR_SIZE "bit_vector_size"
#define OPTION_PS_BINARY_SEARCH "ps_bianry_search"
#define OPTION_PS_BINARY_SEARCH_CASE_WHEN "ps_bianry_search_case_when"
#define OPTION_PS_SETTINGS "ps_settings"
#define OPTION_PS_SET_BITS "set_bits"
#define OPTION_PS_USE_BRIN_OP "us_brin_op"
#define OPTION_PS_ANALYZE "ps_analyze"
#define OPTION_PS_USE_NEST "ps_use_nest"
#define OPTION_PS_POST_TO_ORACLE "ps_post_to_oracle"
#define OPTION_PS_STORE_TABLE "ps_store_table"

/* Uncertainty rewriter options */
#define RANGE_OPTIMIZE_JOIN "range_optimize_join"
#define RANGE_OPTIMIZE_AGG "range_optimize_agg"
#define RANGE_COMPRESSION_RATE "range_compression_rate"

/* dl rewrite options */
#define OPTION_WHYNOT_ADV "whynot_adv"
#define OPTION_DL_SEMANTIC_OPT "dl_semantic_opt"
#define OPTION_DL_MERGE_RULES "dl_merge_rules"
#define OPTION_DL_FETCH_PK_FDS_FROM_DB "dl_load_fds"

// backend types
NEW_ENUM_WITH_TO_STRING(
    BackendType,
    BACKEND_ORACLE,
    BACKEND_POSTGRES,
    BACKEND_SQLITE,
    BACKEND_DUCKDB,
    BACKEND_MONETDB,
	BACKEND_MSSQL
);


// encapsulates option state
typedef struct option_state OptionState;


// declare option fields
// show help only
extern boolean opt_show_help;

// connection options
extern char *connection_host;
extern char *connection_db;
extern char *connection_user;
extern char *connection_passwd;
extern int connection_port;

// logging options
extern int logLevel;
extern boolean logActive;
extern boolean opt_log_operator_colorize;
extern boolean opt_log_operator_verbose;
extern int opt_log_operator_verbose_props;

// input options
extern char *sql;

// database backend
extern char *backend;
extern char *plugin_metadata;
extern char *plugin_parser;
extern char *plugin_sqlcodegen;
extern char *plugin_executor;

// instrumentation options
extern boolean opt_timing;
extern boolean opt_memmeasure;

// rewrite options
extern boolean opt_aggressive_model_checking;
extern boolean opt_update_only_conditions;
extern boolean opt_treeify_opterator_model;
extern boolean opt_only_updated_use_history;
extern boolean opt_pi_cs_composable;
extern boolean opt_pi_cs_rewrite_agg_window;
extern boolean opt_optimize_operator_model;
extern boolean opt_translate_update_with_case;

// cost based optimization option
extern boolean cost_based_optimizer;

// optimization options
extern boolean opt_optimization_push_selections;
extern boolean opt_optimization_merge_ops;
extern boolean opt_optimization_factor_attrs;
extern boolean opt_optimization_materialize_unsafe_proj;
extern boolean opt_optimization_merge_unsafe_proj;
extern boolean opt_optimization_remove_redundant_projections;
extern boolean opt_optimization_remove_redundant_duplicate_operator;
extern boolean opt_optimization_pulling_up_provenance_proj;
extern boolean opt_optimization_push_selections_through_joins;
extern boolean opt_optimization_selection_move_around;
extern boolean opt_optimization_remove_unnecessary_columns;
extern boolean opt_optimization_remove_unnecessary_window_operators;
extern boolean opt_optimization_pull_up_duplicate_remove_operators;
extern boolean cost_based_close_option_removedp_by_set;

// temporal database options
extern boolean temporal_use_coalesce;
extern boolean temporal_use_normalization;
extern boolean temporal_use_normalization_window;

// lateral rewrite for nesting operator
extern boolean opt_lateral_rewrite;
extern boolean opt_unnest_rewrite;
extern boolean opt_agg_reduction_model_rewrite;

// Uncertainty rewriter options
extern boolean range_optimize_join;
extern boolean range_optimize_agg;
extern boolean range_compression_rate;

// optimization options for group by
extern boolean opt_optimization_push_down_group_by_operator_through_join;

// new option interface
extern char *getOptionAsString (char *name);
extern char *getStringOption (char *name);
extern int getIntOption (char *name);
extern boolean getBoolOption (char *name);
extern double getFloatOption (char *name);

extern void setOption (char *name, char *value);
extern void setStringOption (char *name, char *value);
extern void setIntOption(char *name, int value);
extern void setBoolOption(char *name, boolean value);
extern void setFloatOption(char *name, double value);
extern void setOptionsFromMap(HashMap *opts);

extern boolean hasOption(char *name);
extern boolean hasCommandOption(char *name);
extern char *commandOptionGetOption(char *name);
extern OptionType getOptionType(char *name);
extern boolean optionSet(char *name);
extern void printVersion(FILE *stream);

extern BackendType getBackend(void);
extern char *getBackendPlugin(char *be, char *pluginOpt);
extern char *getFrontendPlugin(char *fe, char *pluginOpt);

extern void printOptionsHelp(FILE *stream, char *progName, char *description,
        boolean showValues);
extern void printCurrentOptions(FILE *stream);
extern char *optionsToStringOnePerLine(void);
extern char *internalOptionsToString(boolean showValues);
extern HashMap *optionsToHashMap(void);

extern void mallocOptions();
extern void freeOptions();
extern boolean isRewriteOptionActivated(char *name);

#endif
