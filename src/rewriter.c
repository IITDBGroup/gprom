/*-----------------------------------------------------------------------------
 *
 * rewriter.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "rewriter.h"

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "exception/exception.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/datalog/datalog_model.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "sql_serializer/sql_serializer.h"
#include "operator_optimizer/operator_optimizer.h"
#include "parser/parser.h"
#include "metadata_lookup/metadata_lookup.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "execution/executor.h"
#include "provenance_rewriter/prov_utility.h"
#include "utility/string_utils.h"
#include "model/set/hashmap.h"

#include "instrumentation/timing_instrumentation.h"
#include "instrumentation/memory_instrumentation.h"

#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"
//#include "provenance_rewriter/summarization_rewrites/summarize_main.h"

static char *rewriteParserOutput (Node *parse, boolean applyOptimizations);
static char *rewriteQueryInternal (char *input, boolean rethrowExceptions);
static void setupPlugin(const char *pluginType);
//static void summarizationPlan(Node *parse);
//static List *summOpts = NIL;
//static char *qType = NULL;

int
initBasicModules (void)
{
    initMemManager();
    mallocOptions();
    initLogger();

    return EXIT_SUCCESS;
}


int
initBasicModulesAndReadOptions (char *appName, char *appHelpText, int argc, char* argv[])
{
    initBasicModules();
    return readOptions(appName, appHelpText, argc, argv);
}

int
readOptions (char *appName, char *appHelpText, int argc, char* argv[])
{
    int parserReturn = parseOption(argc, argv);

    if(parserReturn == OPTION_PARSER_RETURN_ERROR)
    {
        printOptionParseError(stdout);
        printOptionsHelp(stdout, appName, appHelpText, TRUE);
        return EXIT_FAILURE;
    }

    if (parserReturn == OPTION_PARSER_RETURN_HELP || getBoolOption("help"))
    {
        printOptionsHelp(stdout, appName, appHelpText, FALSE);
        return EXIT_FAILURE;
    }

    if (parserReturn == OPTION_PARSER_RETURN_VERSION)
    {
        printVersion(stdout);
        return EXIT_FAILURE;
    }


    // set log level from options
    setMaxLevel(getIntOption("log.level"));

    if (opt_memmeasure)
        setupMemInstrumentation();

    return EXIT_SUCCESS;
}

void
reactToOptionsChange (const char *optName)
{
    // need to reinitialize logger
    if (strStartsWith(optName, "log"))
    {
        reinitLogger();
        return;
    }
    // need to reestablish connection
    //TODO right now this would try to reconnect after one connection parameter has been changed
    if (strStartsWith(optName, "connection"))
    {
        databaseConnectionClose();
        databaseConnectionOpen();
        return;
    }
    // need to switch a plugin
    if (strStartsWith(optName, "plugin"))
    {
        setupPlugin(optName);
    }
}

//int
//initBasicModules (void)
//{
//    initMemManager();
//    mallocOptions();
//    initLogger();
//    if (opt_memmeasure)
//        setupMemInstrumentation();
//
//    return EXIT_SUCCESS;
//}

#define CHOOSE_PLUGIN(_plugin,_method) \
    do { \
    	if ((pluginName = getStringOption(_plugin)) != NULL) \
            _method(pluginName); \
        else if (fe != NULL) \
            _method(getFrontendPlugin(fe,_plugin)); \
    	else if (be != NULL) \
		    _method(getBackendPlugin(be, _plugin)); \
        else \
            FATAL_LOG("no backend is set and no " _plugin " provided either, e.g., use -backend BACKEND to set a backend"); \
    } while (0);

#define CHOOSE_BE_PLUGIN(_plugin,_method) \
    do { \
        if ((pluginName = getStringOption(_plugin)) != NULL) \
            _method(pluginName); \
        else if (be != NULL) \
            _method(getBackendPlugin(be, _plugin)); \
        else \
            FATAL_LOG("no backend is set and no " _plugin " provided either, e.g., use -backend BACKEND to set a backend"); \
    } while (0);


void
setupPluginsFromOptions(void)
{
    char *be = getStringOption("backend");
    char *fe = getStringOption("frontend");
    char *pluginName = be;

    // setup parser - individual option overrides backend option
    CHOOSE_PLUGIN(OPTION_PLUGIN_PARSER, chooseParserPluginFromString);

    // setup metadata lookup - individual option overrides backend option
    pluginName = getStringOption("plugin.metadata");
    if (strpeq(pluginName,"external"))
    {
        //printf("\nPLUGIN******************************************\n\n");
    }
    else
    {
        initMetadataLookupPlugins();
        CHOOSE_BE_PLUGIN(OPTION_PLUGIN_METADATA, chooseMetadataLookupPluginFromString);
        initMetadataLookupPlugin();
    }

    // setup analyzer - individual option overrides backend option
    CHOOSE_PLUGIN(OPTION_PLUGIN_ANALYZER, chooseAnalyzerPluginFromString);

    // setup analyzer - individual option overrides backend option
    CHOOSE_PLUGIN(OPTION_PLUGIN_TRANSLATOR, chooseTranslatorPluginFromString);

    // setup analyzer - individual option overrides backend option
    CHOOSE_BE_PLUGIN(OPTION_PLUGIN_SQLSERIALIZER, chooseSqlserializerPluginFromString);
    CHOOSE_BE_PLUGIN(OPTION_PLUGIN_SQLCODEGEN, chooseSqlserializerPluginFromString);

    // setup analyzer - individual option overrides backend option
    pluginName = getStringOption(OPTION_PLUGIN_EXECUTOR);
    chooseExecutorPluginFromString(pluginName);

    // setup cost-based optimizer
    if ((pluginName = getStringOption(OPTION_PLUGIN_CBO)) != NULL)
        chooseOptimizerPluginFromString(pluginName);
    else
        chooseOptimizerPluginFromString("exhaustive");
}

static void
setupPlugin(const char *pluginType)
{
    char *pluginName;
    char *be = getStringOption("backend");
    char *fe = getStringOption("frontend");

    if (streq(pluginType,OPTION_PLUGIN_PARSER))
    {
        CHOOSE_PLUGIN(OPTION_PLUGIN_PARSER, chooseParserPluginFromString);
    }

    // setup metadata lookup - individual option overrides backend option
    if (streq(pluginType,OPTION_PLUGIN_METADATA))
    {
        pluginName = getStringOption(OPTION_PLUGIN_METADATA);
        if (strpeq(pluginName,"external"))
        {
            printf("\nPLUGIN******************************************\n\n");
        }
        else
        {
            initMetadataLookupPlugins();//TODO not necessary
            CHOOSE_BE_PLUGIN(OPTION_PLUGIN_METADATA, chooseMetadataLookupPluginFromString);
            initMetadataLookupPlugin();
        }
    }

    // setup analyzer - individual option overrides backend option
    if (streq(pluginType,OPTION_PLUGIN_ANALYZER))
    {
        CHOOSE_PLUGIN(OPTION_PLUGIN_ANALYZER, chooseAnalyzerPluginFromString);
    }

    // setup analyzer - individual option overrides backend option
    if (streq(pluginType,OPTION_PLUGIN_TRANSLATOR))
    {
        CHOOSE_PLUGIN(OPTION_PLUGIN_TRANSLATOR, chooseTranslatorPluginFromString);
    }

    // setup  sql serializer - individual option overrides backend option
    if (streq(pluginType,OPTION_PLUGIN_SQLSERIALIZER))
    {
        CHOOSE_BE_PLUGIN(OPTION_PLUGIN_SQLSERIALIZER, chooseSqlserializerPluginFromString);
    }

    // setup executor
    if (streq(pluginType,OPTION_PLUGIN_EXECUTOR))
    {
        pluginName = getStringOption(OPTION_PLUGIN_EXECUTOR);
        chooseExecutorPluginFromString(pluginName);
    }

    // setup cost-based optimizer
    if (streq(pluginType,OPTION_PLUGIN_CBO))
    {
        if ((pluginName = getStringOption(OPTION_PLUGIN_CBO)) != NULL)
            chooseOptimizerPluginFromString(pluginName);
        else
            chooseOptimizerPluginFromString("exhaustive");
    }
}

void
resetupPluginsFromOptions (void)
{
    char *be = getStringOption("backend");
    char *pluginName = be;

    // setup parser - individual option overrides backend option
    if ((pluginName = getStringOption("plugin.parser")) != NULL)
        chooseParserPluginFromString(pluginName);
    else
        chooseParserPluginFromString(be);

    // setup analyzer - individual option overrides backend option
    if ((pluginName = getStringOption("plugin.analyzer")) != NULL)
        chooseAnalyzerPluginFromString(pluginName);
    else
        chooseAnalyzerPluginFromString(be);

    // setup translator - individual option overrides backend option
    if ((pluginName = getStringOption("plugin.translator")) != NULL)
        chooseTranslatorPluginFromString(pluginName);
    else
        chooseTranslatorPluginFromString(be);

    // setup serializer - individual option overrides backend option
    if ((pluginName = getStringOption("plugin.sqlserializer")) != NULL)
        chooseSqlserializerPluginFromString(pluginName);
    else
        chooseSqlserializerPluginFromString(be);

    // setup executor- individual option overrides backend option
    if ((pluginName = getStringOption("plugin.executor")) != NULL)
        chooseExecutorPluginFromString(pluginName);
    else
        chooseExecutorPluginFromString("sql");
}

int
readOptionsAndIntialize(char *appName, char *appHelpText, int argc, char* argv[])
{
    int retVal;
    retVal = initBasicModulesAndReadOptions(appName, appHelpText, argc, argv);
    if (retVal != EXIT_SUCCESS)
        return retVal;

    setupPluginsFromOptions();

    return EXIT_SUCCESS;
}

int
shutdownApplication(void)
{
    INFO_LOG("shutdown plugins, logger, and memory manager");

    if (opt_memmeasure)
    {
        outputMemstats(FALSE);
        shutdownMemInstrumentation();
    }
    shutdownMetadataLookupPlugins();

    freeOptions();
    destroyMemManager();

    return EXIT_SUCCESS;
}

void
processInput(char *input)
{
    char *q = NULL;
    Node *parse;

    TRY
    {
        NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
        parse = parseFromString(input);
        q = rewriteParserOutput(parse, isRewriteOptionActivated(OPTION_OPTIMIZE_OPERATOR_MODEL));
        execute(q);
        FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    }
    ON_EXCEPTION
    {
        DEBUG_LOG("allocated in memory context: %s", getCurMemContext()->contextName);
        RETHROW();
    }
    END_ON_EXCEPTION
}

static char *
rewriteQueryInternal (char *input, boolean rethrowExceptions)
{
    Node *parse;
    char *result = "";

    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);

    TRY
    {
        parse = parseFromString(input);

        DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

        result = rewriteParserOutput(parse, isRewriteOptionActivated(OPTION_OPTIMIZE_OPERATOR_MODEL));
        INFO_LOG("Rewritten SQL text from <%s>\n\n is <%s>", input, result);
        FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
    }
    ON_EXCEPTION
    {
        if (rethrowExceptions)
            RETHROW();
        // if an exception is thrown then the query memory context has been
        // destroyed and we can directly create an empty string in the callers
        // context
        DEBUG_LOG("curContext is: %s", getCurMemContext()->contextName);
    }
    END_ON_EXCEPTION
    return strdup("");
}

char *
rewriteQuery(char *input)
{
    return rewriteQueryInternal(input, FALSE);
}

char *
rewriteQueryWithRethrow(char *input)
{
    return rewriteQueryInternal(input, TRUE);
}

int
costQuery (char *input)
{
    return getCostEstimation(input);
}

char *
rewriteQueryFromStream (FILE *stream) {
    Node *parse;
    char *result;

    parse = parseStream(stream);
    DEBUG_LOG("parser returned:\n\n%s", nodeToString(parse));

    result = rewriteParserOutput(parse, isRewriteOptionActivated(OPTION_OPTIMIZE_OPERATOR_MODEL));
    INFO_LOG("Rewritten SQL text is <%s>", result);

    return result;
}

char *
rewriteQueryWithOptimization(char *input)
{
    Node *parse;
    char *result;

    parse = parseFromString(input);
    DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

    result = rewriteParserOutput(parse, TRUE);
    INFO_LOG("Rewritten SQL text from <%s>\n\n is <%s>", input, result);

    return result;
}

char *
generatePlan(Node *oModel, boolean applyOptimizations)
{
	StringInfo result = makeStringInfo();
	Node *rewrittenTree;
	char *rewrittenSQL = NULL;
	START_TIMER("rewrite");

	rewrittenTree = provRewriteQBModel(oModel);

	if (IS_QB(rewrittenTree))
	{
	    DOT_TO_CONSOLE_WITH_MESSAGE("BEFORE REWRITE", rewrittenTree);

	    /*******************new Add for test json import jimp table***************************/
	    if(isA(rewrittenTree,List) && isA(getHeadOfListP((List *)rewrittenTree), ProjectionOperator))
	    {
	        QueryOperator *q = (QueryOperator *)getHeadOfListP((List *)rewrittenTree);
	        List *taOp = NIL;
	        findTableAccessOperator(&taOp, (QueryOperator *) q);
	        int l = LIST_LENGTH(taOp);
	        DEBUG_LOG("len %d", l);
	        if(isA(getHeadOfListP(taOp),TableAccessOperator))
	        {
	            TableAccessOperator *ta = (TableAccessOperator *) getHeadOfListP(taOp);
	            DEBUG_LOG("test %s", ta->tableName);
	            if(streq(ta->tableName,"JIMP2") || streq(ta->tableName, "JSDOC1"))
	            {
	                q = rewriteTransformationProvenanceImport(q);
	                DEBUG_LOG("Table: %s", nodeToString(ta));
	                rewrittenTree = (Node *) singleton(q);
	            }
	        }
	    }
	    /*******************new Add for test json import jimp table***************************/

	    DEBUG_NODE_BEATIFY_LOG("provenance rewriter returned:", rewrittenTree);
	    INFO_OP_LOG("provenance rewritten query as overview:", rewrittenTree);
	    STOP_TIMER("rewrite");

	    ASSERT_BARRIER(
	            if (isA(rewrittenTree, List))
	                FOREACH(QueryOperator,o,(List *) rewrittenTree)
	                TIME_ASSERT(checkModel(o));
	            else
	                TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
	    )

        // rewrite for summarization
//		if(!LIST_EMPTY(summOpts) && qType != NULL && relToDoms != NULL)
//			rewrittenTree = rewriteSummaryOutput(rewrittenTree, summOpts, qType, relToDoms);

	    if(applyOptimizations)
	    {
	        START_TIMER("OptimizeModel");
	        rewrittenTree = optimizeOperatorModel(rewrittenTree);
	        INFO_OP_LOG("after optimizing AGM graph:", rewrittenTree);
	        STOP_TIMER("OptimizeModel");
	    }
	    else
	    {
	        if (isA(rewrittenTree, List))
	        {
	            FOREACH(QueryOperator,o,(List *) rewrittenTree)
                {
                    LC_P_VAL(o_his_cell) = materializeProjectionSequences (o);
                }
	        }
	        else
	        {
	            rewrittenTree = (Node *) materializeProjectionSequences((QueryOperator *) rewrittenTree);
	        }
	    }

	    DOT_TO_CONSOLE_WITH_MESSAGE("AFTER OPTIMIZATIONS", rewrittenTree);
	}

	START_TIMER("SQLcodeGen");
	appendStringInfo(result, "%s\n", serializeOperatorModel(rewrittenTree));
	STOP_TIMER("SQLcodeGen");

	rewrittenSQL = result->data;
	FREE(result);

	return rewrittenSQL;

}

static char *
rewriteParserOutput (Node *parse, boolean applyOptimizations)
{
    char *rewrittenSQL = NULL;
    Node *oModel;

//    if(!getBoolOption(OPTION_INPUTDB))
//    {
//    	relToDoms = NEW_MAP(Constant,Constant);
//    	summarizationPlan(parse, relToDoms);
//    }

    START_TIMER("translation");
    oModel = translateParse(parse);
    DEBUG_NODE_BEATIFY_LOG("translator returned:", oModel);
    if (IS_OP(oModel))
    {
        INFO_OP_LOG("translator result as overview:", oModel);
        DOT_TO_CONSOLE_WITH_MESSAGE("TRANSLATOR OUTPUT", oModel);
    }
    else if (IS_DL_NODE(oModel))
    {
        INFO_DL_LOG("translator result as overview:", oModel);
    }
    STOP_TIMER("translation");

    ASSERT_BARRIER(
        if (IS_OP(oModel))
        {
            if (isA(oModel, List))
                FOREACH(QueryOperator,o,(List *) oModel)
                TIME_ASSERT(checkModel(o));
            else
                TIME_ASSERT(checkModel((QueryOperator *) oModel));
        }
    )

    if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
        rewrittenSQL = doCostBasedOptimization(oModel, applyOptimizations);
    else
    	rewrittenSQL = generatePlan(oModel, applyOptimizations);

    return rewrittenSQL;
}

//
//static void
//summarizationPlan (Node *parse, HashMap *relToDoms)
//{
//    // store options for the summarization and question type
//    if (isA(parse, List) && isA(getHeadOfListP((List *) parse), ProvenanceStmt))
//    {
//        ProvenanceStmt *ps = (ProvenanceStmt *) getHeadOfListP((List *) parse);
//
//    	if (!LIST_EMPTY(ps->sumOpts))
//    		FOREACH(Node,n,ps->sumOpts)
//    			summOpts = appendToTailOfList(summOpts,n);
//
//    	qType = "WHY";
//    }
//    else // summarization options for DL input
//    {
//    	DLProgram *p = (DLProgram *) parse;
//
//    	// either why or why-not
//    	FOREACH(Node,n,p->rules)
//    	{
//    		if(isA(n,KeyValue))
//    		{
//    			KeyValue *kv = (KeyValue *) n;
//    			qType = STRING_VALUE(kv->key);
//
//    			if(isPrefix(qType,"WHYNOT_"))
//    				qType = "WHYNOT";
//    			else
//    				qType = "WHY";
//    		}
//
//    		// store info which rel+attr uses which user domain
//    		if(isA(n,DLDomain))
//    		{
//    			DLDomain *dd = (DLDomain *) n;
//    			char *key = CONCAT_STRINGS(dd->rel,dd->attr);
//    			char *value = dd->name;
//
//    			if(!MAP_HAS_STRING_KEY(relToDoms,key))
//    				MAP_ADD_STRING_KEY_AND_VAL(relToDoms,key,value);
//    		}
//    	}
//
//    	if (p->sumOpts != NIL)
//    	{
//    		FOREACH(Node,n,p->sumOpts)
//				summOpts = appendToTailOfList(summOpts,n);
//
//    		// keep track of (var,rel) and (negidb,edb)
//    		HashMap *varRelPair = NEW_MAP(Constant,Constant);
//    		HashMap *headEdbPair = NEW_MAP(Constant,List);
//    		List *negAtoms = NIL;
//
//    		FOREACH(Node,n,p->rules)
//    		{
//    			if(isA(n,DLRule))
//    			{
//    				DLRule *r = (DLRule *) n;
//            		List *edbList = NIL;
//
//    				FOREACH(Node,b,r->body)
//    				{
//    					if(isA(b,DLAtom))
//    					{
//    						DLAtom *a = (DLAtom *) b;
//
//    						// keep track of which negated atom needs domains from which edb atom
//    						if(a->negated)
//    							negAtoms = appendToTailOfList(negAtoms,a->rel);
//    						else
//           						edbList = appendToTailOfList(edbList,a->rel);
//
//    						// keep track of which variable belongs to which edb
//    						FOREACH(Node,n,a->args)
//    						{
//    							if(isA(n,DLVar))
//    							{
//    								DLVar *v = (DLVar *) n;
//    								MAP_ADD_STRING_KEY_AND_VAL(varRelPair,v->name,a->rel);
//    							}
//    						}
//    					}
//    				}
//
//        			char *headPred = getHeadPredName(r);
//    				MAP_ADD_STRING_KEY(headEdbPair,headPred,edbList);
//    			}
//    		}
//
//    		// store edb information for negated atoms and why-not questions
//    		if(!LIST_EMPTY(negAtoms))
//    		{
//        		FOREACH(char,c,negAtoms)
//        		{
//        			if(!MAP_HAS_STRING_KEY(headEdbPair,c))
//        				MAP_ADD_STRING_KEY_AND_VAL(varRelPair,c,c);
//        			else
//        			{
//        				List *edbs = (List *) MAP_GET_STRING(headEdbPair,c);
//
//        				FOREACH(char,e,edbs)
//        					MAP_ADD_STRING_KEY_AND_VAL(varRelPair,e,e);
//        			}
//        		}
//    		}
//
//    		if(LIST_EMPTY(negAtoms) || streq(qType,"WHYNOT"))
//    		{
//				FOREACH_HASH(List,edbs,headEdbPair)
//					FOREACH(char,e,edbs)
//						MAP_ADD_STRING_KEY_AND_VAL(varRelPair,e,e);
//    		}
//
//    		// store into the list of the summarization options
//    		summOpts = appendToTailOfList(summOpts, (Node *) varRelPair);
//    	}
//    }
//}

