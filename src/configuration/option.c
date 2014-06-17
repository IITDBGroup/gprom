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

Options* options;
HashMap *optionPos; // optionname -> position of option in list

typedef enum OptionType {
    OPTION_BOOL,
    OPTION_STRING,
    OPTION_INT,
    OPTION_FLOAT
} OptionType;

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

// connection options
char *connection_host = NULL;
char *connection_db = NULL;
char *connection_user = NULL;
char *connection_passwd = NULL;
int connection_port = 0;

// logging options
int logLevel = 0;
boolean logActive = FALSE;

// instrumentation options
boolean opt_timing = FALSE;

// rewrite options
boolean opt_aggressive_model_checking = FALSE;
boolean opt_update_only_conditions = FALSE;
boolean opt_treeify_opterator_model = FALSE;
boolean opt_only_updated_use_history = FALSE;
boolean opt_pi_cs_composable = FALSE;
boolean opt_optimize_operator_model = FALSE;
boolean opt_translate_update_with_case = FALSE;
//boolean   = FALSE;

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

#define OPT_POS(name) INT_VALUE(MAP_GET_STRING(optionPos,name))

// array storing information for all supported options
OptionInfo opts[] =
{
        // database backend connection options
        {
                "connection.host",
                "host",
                "Host IP address for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_host),
                defOptionString("ligeti.cs.iit.edu")
        },
        {
                "connection.db",
                "db",
                "Database name for backend DB connection (SID for Oracle backends).",
                OPTION_STRING,
                wrapOptionString(&connection_db),
                defOptionString("orcl")
        },
        {
                "connection.user",
                "user",
                "User for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_user),
                defOptionString("fga_user")
        },
        {
                "connection.passwd",
                "passwd",
                "Password for backend DB connection.",
                OPTION_STRING,
                wrapOptionString(&connection_passwd),
                defOptionString("fga_")
        },
        {
                "connection.port",
                "port",
                "TCP/IP port for backend DB connection.",
                OPTION_INT,
                wrapOptionInt(&connection_port),
                defOptionInt(1521)
        },
        // logging options
        {
                "log.level",
                "loglevel",
                "Log level determining log output: TRACE, DEBUG, INFO, WARN, ERROR, FATAL",
                OPTION_INT,
                wrapOptionInt(&connection_port),
                defOptionInt(1521)
        },
        {
                "log.active",
                "log",
                "Activate/Deactivate logging",
                OPTION_BOOL,
                wrapOptionBool(&connection_port),
                defOptionInt(TRUE)
        },
        // boolean instrumentation options
        aRewriteOption(OPTION_TIMING,
                NULL,
                "measure and output execution time of modules",
                opt_timing,
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
                FALSE),
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
        aRewriteOption(OPTION_OPTIMIZE_OPERATOR_MODEL,
                NULL,
                "Apply heuristic and cost based optimizations to operator model",
                opt_optimize_operator_model,
                FALSE),
        aRewriteOption(OPTION_TRANSLATE_UPDATE_WITH_CASE,
                NULL,
                "Create reenactment query for UPDATE statements using CASE instead of UNION.",
                opt_translate_update_with_case,
                FALSE),
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

//static OptionValue
//wrapOptionValue(void *value, OptionType type)
//{
//    OptionValue result;
//
//    switch(type)
//    {
//        case OPTION_STRING:
//            result.string = (char **) value;
//            break;
//        case OPTION_FLOAT:
//            result.f = (double *) value;
//            break;
//        case OPTION_BOOL:
//            result.b = (boolean *) value;
//            break;
//        case OPTION_INT:
//            result.i = (int *) value;
//            break;
//    }
//
//    return result;
//}
//
//static OptionValue
//defOptionInt(int i)
//{
//    OptionValue result;
//    int *iP = malloc(sizeof(int));
//
//    *iP = i;
//    result.i = iP;
//
//    return result;
//}
//
//static OptionValue
//defOptionBool(boolean b)
//{
//    OptionValue result;
//    boolean *p = malloc(sizeof(boolean));
//
//    *p = b;
//    result.b = p;
//
//    return result;
//}
//
//static OptionValue
//defOptionString(char *s)
//{
//    OptionValue result;
//    char **p = malloc(sizeof(char **));
//
//    *p = s;
//
//    result.string = p;
//
//    return result;
//}
//
//static OptionValue
//defOptionFloat(double f)
//{
//    OptionValue result;
//    double *p = malloc(sizeof(double));
//
//    *p = f;
//    result.f = p;
//
//    return result;
//}

static void
initOptions(void)
{
    // create hashmap option -> position in option info array for lookup
    optionPos = NEW_MAP(Constant,Constant);

    // add options to hashmap and set all options to default values
    for(int i = 0; strcmp(opts[i].option,"STOPPER") != 0; i++)
    {
        OptionInfo *o = &(opts[i]);
        setDefault(o);
        ASSERT(!MAP_HAS_STRING_KEY(optionPos, o->option)); // no two option with same identifier
        MAP_ADD_STRING_KEY(optionPos, o->option, createConstInt(i));
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
                char *newS = malloc(sizeof(char) * (strlen(o->def.string) + 1));
                strcpy(newS, o->def.string);
                *(o->value.string) = newS;
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
	options=MAKE_OPTIONS();
	options->optionConnection=MAKE_OPTION_CONNECTION();
	options->optionDebug=MAKE_OPTION_DEBUG();
	options->optionRewrite=NIL;

	initOptions();
}

void
freeOptions()
{
	free(options->optionConnection->host);
	free(options->optionConnection->db);
	free(options->optionConnection->user);
	free(options->optionConnection->passwd);
	free(options->optionConnection->sql);
	free(options->optionConnection);
	free(options->optionDebug);
//	FOREACH(KeyValue,o,options->optionRewrite)
//	{
//	    Constant *c;
//
//	    c = (Constant *) o->key;
//	    if (c->constType == DT_STRING)
//	        free(c->value);
//	    free(c);
//
//	    c = (Constant *) o->value;
//        if (c->constType == DT_STRING)
//            free(c->value);
//        free(c);
//
//        free(o);
//	}
//	freeList(options->optionRewrite);
//	free(options->optionRewrite);
	free(options);
}



Options*
getOptions()
{
	return options;
}

boolean
isRewriteOptionActivated(char *name)
{
    Options *ops = getOptions();

    FOREACH(KeyValue,op,ops->optionRewrite)
    {
        if (strcmp(STRING_VALUE(op->key),name) == 0)
            return BOOL_VALUE(op->value);
    }

    return FALSE;
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
optionSet(char *name)
{
    return TRUE;
}

void
printOptionsHelp(FILE *stream, char *progName)
{
    fprintf(stream, "%s:\nthe following options are supported:\n\n", progName);

    FOREACH_HASH_KEY(Constant,k,optionPos)
    {
        char *name = STRING_VALUE(k);
        OptionInfo *v = getInfo(name);
        fprintf(stream, "-%s%s internal name: %s default value: %s\n\t%s\n",
                v->cmdLine ? v->cmdLine : "activate ",
                v->cmdLine ? "" : v->option,
                v->option,
                defGetString(&v->def, v->valueType),
                v->description);
    }
}

static char *
defGetString(OptionDefault *def, OptionType type)
{
    switch(type)
    {
        case OPTION_INT:
            return itoa(def->i);
        case OPTION_STRING:
            return def->string;
        case OPTION_BOOL:
            return (def->b ? "TRUE" : "FALSE");
        case OPTION_FLOAT:
        {
            char *buf = malloc(sizeof(char) * 50);
            snprintf(buf,50,"%f", def->f);
            return buf;
        }
    }
}
