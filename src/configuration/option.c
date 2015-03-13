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
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"

// we have to use actual free here
#undef free
#undef malloc

//Options* options;
HashMap *optionPos; // optionname -> position of option in list
HashMap *cmdOptionPos;

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

// show help only
boolean opt_show_help = FALSE;

// connection options
char *connection_host = NULL;
char *connection_db = NULL;
char *connection_user = NULL;
char *connection_passwd = NULL;
int connection_port = 0;

// backend specific options
char *oracle_audit_log_table = NULL;

// logging options
int logLevel = 0;
boolean logActive = FALSE;

// input options
char *sql = NULL;

// database backend
char *backend = NULL;
char *plugin_metadata = NULL;
char *plugin_parser = NULL;
char *plugin_sqlcodegen = NULL;
char *plugin_analyzer = NULL;
char *plugin_translator = NULL;
char *plugin_sql_serializer = NULL;
char *plugin_executor = NULL;

// instrumentation options
boolean opt_timing = FALSE;
boolean opt_memmeasure = FALSE;
boolean opt_graphviz_output = FALSE;

// rewrite options
boolean opt_aggressive_model_checking = FALSE;
boolean opt_update_only_conditions = FALSE;
boolean opt_treeify_opterator_model = FALSE;
boolean opt_only_updated_use_history = FALSE;
boolean opt_pi_cs_composable = FALSE;
boolean opt_pi_cs_rewrite_agg_window = FALSE;
boolean opt_optimize_operator_model = FALSE;
boolean opt_translate_update_with_case = FALSE;
//boolean   = FALSE;

// cost based optimization option
boolean cost_based_optimizer = FALSE;

// optimization options
boolean opt_optimization_push_selections = FALSE;
boolean opt_optimization_merge_ops = FALSE;
boolean opt_optimization_factor_attrs = FALSE;
boolean opt_materialize_unsafe_proj = FALSE;
boolean opt_remove_redundant_projections = TRUE;
boolean opt_remove_redundant_duplicate_operator = TRUE;
boolean opt_optimization_pulling_up_provenance_proj = FALSE;
boolean opt_optimization_push_selections_through_joins = FALSE;
boolean opt_optimization_selection_move_around = FALSE;
boolean opt_optimization_remove_unnecessary_columns = FALSE;
boolean opt_optimization_remove_unnecessary_window_operators = FALSE;
boolean opt_optimization_pull_up_duplicate_remove_operators = FALSE;

// sanity check options
boolean opt_operator_model_unique_schema_attribues = FALSE;
boolean opt_operator_model_parent_child_links = FALSE;
boolean opt_operator_model_schema_consistency = FALSE;
boolean opt_operator_model_attr_reference_consistency = FALSE;

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


#define OPT_POS(name) INT_VALUE(MAP_GET_STRING(optionPos,name))

// array storing information for all supported options
OptionInfo opts[] =
{
        // show help only and quit
        {
                "help",
                "-help",
                "Show this help text.",
                OPTION_BOOL,
                wrapOptionString(&opt_show_help),
                defOptionBool(FALSE)
        },
        // database backend connection options
        {
                "connection.host",
                "-host",
                "Host IP address for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_host),
                defOptionString("ligeti.cs.iit.edu")
        },
        {
                "connection.db",
                "-db",
                "Database name for backend DB connection (SID for Oracle backends).",
                OPTION_STRING,
                wrapOptionString(&connection_db),
                defOptionString("orcl")
        },
        {
                "connection.user",
                "-user",
                "User for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_user),
                defOptionString("fga_user")
        },
        {
                "connection.passwd",
                "-passwd",
                "Password for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_passwd),
                defOptionString("fga")
        },
        {
                "connection.port",
                "-port",
                "TCP/IP port for backend DB connection.",
                OPTION_INT,
                wrapOptionInt(&connection_port),
                defOptionInt(1521)
        },
        {
                "backendOpts.oracle.logtable",
                "-Boracle.audittable",
                "Table storing the audit log (usually fga_log$ or unified_audit_trail)",
                OPTION_STRING,
                wrapOptionString(&oracle_audit_log_table),
                defOptionString("UNIFIED_AUDIT_TRAIL")
        },
        // logging options
        {
                "log.level",
                "-loglevel",
                "Log level determining log output: TRACE=5, DEBUG=4, INFO=3, WARN=2, ERROR=1, FATAL=0",
                OPTION_INT,
                wrapOptionInt(&logLevel),
                defOptionInt(1)
        },
        {
                "log.active",
                "-log",
                "Activate/Deactivate logging",
                OPTION_BOOL,
                wrapOptionBool(&logActive),
                defOptionBool(TRUE)
        },
        // input options
        {
                "input.sql",
                "-sql",
                "input SQL text",
                OPTION_STRING,
                wrapOptionString(&sql),
                defOptionString(NULL)
        },
        // backend and plugin selectionselection
        {
                "backend",
                "-backend",
                "select backend database type: postgres, oracle - this determines parser, metadata-lookup, and sql-code generator",
                OPTION_STRING,
                wrapOptionString(&backend),
                defOptionString("oracle")
        },
        {
                "plugin.metadata",
                "-Pmetadata",
                "select metadatalookup plugin: postgres, oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_metadata),
                defOptionString(NULL)
        },
        {
                "plugin.parser",
                "-Pparser",
                "select parser plugin: oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_parser),
                defOptionString(NULL)
        },
        {
                "plugin.sqlcodegen",
                "-Psqlcodegen",
                "select SQL code generator plugin: oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_sqlcodegen),
                defOptionString(NULL)
        },
        {
                "plugin.analyzer",
                "-Panalyzer",
                "select parser result model analyzer: oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_analyzer),
                defOptionString(NULL)
        },
        {
                "plugin.translator",
                "-Ptranslator",
                "select parser result to relational algebra translator: oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_translator),
                defOptionString(NULL)
        },
        {
                "plugin.sqlserializer",
                "-Psqlserializer",
                "select SQL code generator plugin: oracle",
                OPTION_STRING,
                wrapOptionString(&plugin_sql_serializer),
                defOptionString(NULL)
        },
        {
                "plugin.executor",
                "-Pexecutor",
                "select Executor plugin: sql (output rewritten SQL code), gp (output Game provenance)",
                OPTION_STRING,
                wrapOptionString(&plugin_executor),
                defOptionString(NULL)
        },
        // boolean instrumentation options
        aRewriteOption(OPTION_TIMING,
                NULL,
                "measure and output execution time of modules.",
                opt_timing,
                FALSE),
        aRewriteOption(OPTION_MEMMEASURE,
                NULL,
                "measure and output memory allocation stats.",
                opt_memmeasure,
                FALSE),
        aRewriteOption(OPTION_GRAPHVIZ,
                NULL,
                "output created query operator models as graphviz scripts.",
                opt_graphviz_output,
                FALSE),
        // boolean rewrite options
        aRewriteOption(OPTION_AGGRESSIVE_MODEL_CHECKING,
                NULL,
                "do aggressive validity checking of AGM models.",
                opt_aggressive_model_checking,
                FALSE),
        aRewriteOption(OPTION_UPDATE_ONLY_USE_CONDS,
                NULL,
                "Use disjunctions of update conditions to filter out tuples from "
                "transaction provenance that are not updated by the transaction.",
                opt_update_only_conditions,
                TRUE),
        aRewriteOption(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN,
                NULL,
                "Use a join between the version at commit time with the table version"
                " at transaction start to prefilter rows that were not updated by the transaction.",
                opt_only_updated_use_history,
                FALSE),
        aRewriteOption(OPTION_TREEIFY_OPERATOR_MODEL,
                NULL,
                "Turn AGM graph into a tree before passing it off to the provenance rewriter.",
                opt_treeify_opterator_model,
                TRUE),
        aRewriteOption(OPTION_PI_CS_USE_COMPOSABLE,
                NULL,
                "Use composable version of PI-CS provenance that adds additional columns which"
                " enumerate duplicates introduced by provenance.",
                opt_pi_cs_composable,
                FALSE),
        aRewriteOption(OPTION_PI_CS_COMPOSABLE_REWRITE_AGG_WINDOW,
                NULL,
                "When composable version of PI-CS provenance is use then rewrite aggregations using window functions.",
                opt_pi_cs_rewrite_agg_window,
                TRUE),
        aRewriteOption(OPTION_OPTIMIZE_OPERATOR_MODEL,
                NULL,
                "Apply heuristic and cost based optimizations to operator model",
                opt_optimize_operator_model,
                FALSE),
        aRewriteOption(OPTION_TRANSLATE_UPDATE_WITH_CASE,
                NULL,
                "Create reenactment query for UPDATE statements using CASE instead of UNION.",
                opt_translate_update_with_case,
                TRUE),
        // Cost Based Optimization Option
         {
                 OPTION_COST_BASED_OPTIMIZER,
                "-cost_based_optimizer",
                "Activate/Deactivate cost based optimizer",
                OPTION_BOOL,
                wrapOptionBool(&cost_based_optimizer),
                defOptionBool(FALSE)
         },
        // AGM (Query operator model) individual optimizations
        anOptimizationOption(OPTIMIZATION_SELECTION_PUSHING,
                "-Opush_selections",
                "Optimization: Activate selection move-around",
                opt_optimization_push_selections,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_MERGE_OPERATORS,
                "-Omerge_ops",
                "Optimization: try to merge adjacent selection and projection operators",
                opt_optimization_merge_ops,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR,
                "-Ofactor_attrs",
                "Optimization: try to factor out attribute references in projection"
                " expressions to open up new operator merging opportunities",
                opt_optimization_factor_attrs,
                FALSE
        ),
        anOptimizationOption(OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ,
                "-Omaterialize_unsafe",
                "Optimization: add materialization hint for projections that "
                "if merged with adjacent projection would cause expontential "
                "expression size blowup",
                opt_materialize_unsafe_proj,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_REDUNDANT_PROJECTIONS,
                "-Oremove_redundant_projections",
                "Optimization: try to remove redundant projections",
                opt_remove_redundant_projections,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR,
                "-Oremove_redundant_duplicate_operator",
                "Optimization: try to remove redundant duplicate operator",
                opt_remove_redundant_duplicate_operator,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_UNNECESSARY_WINDOW_OPERATORS,
                "-Oremove_unnecessary_window_operators",
                "Optimization: try to remove unnecessary window operators",
                opt_optimization_remove_unnecessary_window_operators,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_REMOVE_UNNECESSARY_COLUMNS,
                "-Oremove_unnecessary_columns",
                "Optimization: try to remove unnecessary columns",
                opt_optimization_remove_unnecessary_columns,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_PULL_UP_DUPLICATE_REMOVE_OPERATORS,
        		"-Opullup_duplicate_remove_operators",
        		"Optimization: try to pull up duplicate remove operators",
        		opt_optimization_pull_up_duplicate_remove_operators,
        		TRUE
        ),
        anOptimizationOption(OPTIMIZATION_PULLING_UP_PROVENANCE_PROJ,
                "-Opulling_up_provenance_proj",
                "Optimization: try to pull up provenance projection",
                opt_optimization_pulling_up_provenance_proj,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_SELECTION_PUSHING_THROUGH_JOINS,
                "-Opush_selections_through_joins",
                "Optimization: try to push selections through joins",
                opt_optimization_push_selections_through_joins,
                TRUE
        ),
        anOptimizationOption(OPTIMIZATION_SELECTION_MOVE_AROUND,
                "-Oselections_move_around",
                "Optimization: try to move selection around",
                opt_optimization_selection_move_around,
                TRUE
                ),
        // sanity model checking options
        anSanityCheckOption(CHECK_OM_UNIQUE_ATTR_NAMES,
                "-Cunique_attr_names",
                "Model Check: check that attribute names are unique for each operator's schema.",
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
        // stopper to indicate end of array
        {
                "STOPPER",
                NULL,
                NULL,
                OPTION_STRING,
                wrapOptionString(NULL),
                defOptionString("")
        }
};

static void
initOptions(void)
{
    // create hashmap option -> position in option info array for lookup
    optionPos = NEW_MAP(Constant,Constant);
    cmdOptionPos = NEW_MAP(Constant,Constant);

    // add options to hashmap and set all options to default values
    for(int i = 0; strcmp(opts[i].option,"STOPPER") != 0; i++)
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
            fprintf(stream, "%s%s\n\tDEFAULT VALUE: %s\n\tACTUAL VALUE: %s\n\t%s\n",
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

static char *
valGetString(OptionValue *def, OptionType type)
{
    switch(type)
    {
        case OPTION_INT:
            return itoa(*(def->i));
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

    return NULL; //keep compiler quit
}


static char *
defGetString(OptionDefault *def, OptionType type)
{
    switch(type)
    {
        case OPTION_INT:
            return itoa(def->i);
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
