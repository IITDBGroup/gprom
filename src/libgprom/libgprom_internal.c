/*-----------------------------------------------------------------------------
 *
 * libgprom.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "log/logger.h"
#include "libgprom/libgprom-internal.h"
#include "rewriter.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_rewriter.h"
//#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_external.h"

#define LIBARY_REWRITE_CONTEXT "LIBGRPROM_QUERY_CONTEXT"
//#define printf(...) 0
#define LOCK_MUTEX() 0//printf("\nMUTEX\n%s:%u", __FILE__, __LINE__)
#define UNLOCK_MUTEX() 0//printf("\nUNLOCK\n%s:%u", __FILE__, __LINE__)
#define CREATE_MUTEX()
#define DESTROY_MUTEX()

void
gprom_init(void)
{
    CREATE_MUTEX();
    initMemManager();
    LOCK_MUTEX();
    mallocOptions();
    initLogger();
    registerSignalHandler();
    setWipeContext(LIBARY_REWRITE_CONTEXT);
    UNLOCK_MUTEX();
}

void
gprom_readOptions(int argc, char * const args[])
{
    LOCK_MUTEX();
    if(parseOption(argc, args) != 0)
    {
        printOptionParseError(stdout);
    }
    UNLOCK_MUTEX();
}

void
gprom_readOptionAndInit(int argc, char *const args[])
{
    LOCK_MUTEX();
    readOptionsAndIntialize("gprom-libary","",argc,(char **) args);
    UNLOCK_MUTEX();
}

void
gprom_configFromOptions(void)
{
    setupPluginsFromOptions();
}

void
gprom_reconfPlugins(void)
{
    resetupPluginsFromOptions();
}

void gprom_shutdown(void)
{
    LOCK_MUTEX();
    deregisterSignalHandler();
    shutdownApplication();
    UNLOCK_MUTEX();
    DESTROY_MUTEX();
}

const char *
gprom_rewriteQuery(const char *query)
{
    LOCK_MUTEX();
    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
    char *result = "";
    //char *returnResult = NULL;
    TRY
    {
        result = rewriteQueryWithRethrow((char *) query);
        return result;
        UNLOCK_MUTEX();
        //RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(result,returnResult);
    }
    ON_EXCEPTION
    {
        ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
    }
    END_ON_EXCEPTION
    UNLOCK_MUTEX();
    return result;//returnResult;
}


void
gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback)
{
    registerLogCallback(callback);
}

void
gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback)
{
    registerExceptionCallback((GProMExceptionCallbackFunctionInternal) callback);
}

void
gprom_setMaxLogLevel (int maxLevel)
{
    setMaxLevel((LogLevel) maxLevel);
}


const char *
gprom_getStringOption (const char *name)
{
    return getStringOption((char *) name);
}

int
gprom_getIntOption (const char *name)
{
    return getIntOption((char *) name);
}

boolean
gprom_getBoolOption (const char *name)
{
    return getBoolOption((char *) name);
}

double
gprom_getFloatOption (const char *name)
{
    return getFloatOption((char *) name);
}

const char *
gprom_getOptionType(const char *name)
{
    ASSERT(hasOption((char *) name));
    return OptionTypeToString(getOptionType((char *) name));
}

boolean
gprom_optionExists(const char *name)
{
    return hasOption((char *) name);
}

void
gprom_setOptionsFromMap()
{

}

void
gprom_setOption(const char *name, const char *value)
{
    return setOption((char *) name, strdup((char *) value));
}

void
gprom_setStringOption (const char *name, const char *value)
{
    return setStringOption((char *) name, strdup((char *) value));
}

void
gprom_setIntOption(const char *name, int value)
{
    return setIntOption((char *) name, value);
}

void
gprom_setBoolOption(const char *name, boolean value)
{
    return setBoolOption((char *) name,value);
}

void
gprom_setFloatOption(const char *name, double value)
{
    return setFloatOption((char *) name,value);
}

void
gprom_registerMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin)
{
    setMetadataLookupPlugin(assembleExternalMetadataLookupPlugin(plugin));
}

GProMNode *
gprom_rewriteQueryToOperatorModel(const char *query)
{
    LOCK_MUTEX();
    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
    //char *result = "";
    Node *parse;
    Node *oModel;
    Node *rewrittenTree;
    GProMNode* returnResult;
    TRY
    {
    	parse = parseFromString((char *)query);
		//DEBUG_LOG("parser returned:\n\n<%s>", nodeToString(parse));

		oModel = translateParse(parse);
		//DEBUG_NODE_BEATIFY_LOG("translator returned:", oModel);

		rewrittenTree = provRewriteQBModel(oModel);

    	UNLOCK_MUTEX();
    	returnResult = (GProMNode*)rewrittenTree;
    	//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
    	return returnResult;
    }
    ON_EXCEPTION
    {
        ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
    }
    END_ON_EXCEPTION

    UNLOCK_MUTEX();
    //RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
    return NULL;
}

GProMNode *
gprom_provRewriteOperator(GProMNode* nodeFromMimir)
{
	LOCK_MUTEX();
	    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	    //char *result = "";
	    Node *rewrittenTree;
	    Node *copiedTree;
	    GProMNode* returnResult;
	    TRY
	    {

	    	copiedTree = copyObject(nodeFromMimir);
			rewrittenTree = provRewriteQBModel(copiedTree);


	    	UNLOCK_MUTEX();
	    	returnResult = (GProMNode*)rewrittenTree;
	    	//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
	    	return returnResult;
	    }
	    ON_EXCEPTION
	    {
	        ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
	    }
	    END_ON_EXCEPTION

	    UNLOCK_MUTEX();
	    //RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
	    return NULL;
}

GProMNode *
gprom_optimizeOperatorModel(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
	    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	    //char *result = "";
	    Node *rewrittenTree;
	    Node *copiedTree;
	    GProMNode* returnResult;
	    TRY
	    {

	    	copiedTree = copyObject(nodeFromMimir);

	    	rewrittenTree = optimizeOperatorModelRW(copiedTree);

			UNLOCK_MUTEX();
			returnResult = (GProMNode*)rewrittenTree;
			//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
			return returnResult;
		}
		ON_EXCEPTION
		{
			ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
		}
		END_ON_EXCEPTION

		UNLOCK_MUTEX();
		//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
		return NULL;
}

char *
gprom_operatorModelToSql(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
	    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	    //char *result = "";
	    char *rewrittenSQL;
	    Node *copiedTree;
	    TRY
	    {

	    	copiedTree = copyObject(nodeFromMimir);

	    	rewrittenSQL = serializeOperatorModelRW(copiedTree);

			UNLOCK_MUTEX();
			//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
			return rewrittenSQL;
		}
		ON_EXCEPTION
		{
			ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
		}
		END_ON_EXCEPTION

		UNLOCK_MUTEX();
		//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
		return NULL;
}

char *
gprom_nodeToString(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	//NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	char *returnResult = NULL;
	//char *result = NULL;
	//Node *copiedTree;
	TRY
	{
		//copiedTree = copyObject(nodeFromMimir);
		returnResult = jsonify(nodeToString(nodeFromMimir));

		UNLOCK_MUTEX();
		//RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(result,returnResult);
		//RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(beatify(nodeToString(nodeFromMimir)),returnResult);
		return returnResult;
	}
	ON_EXCEPTION
	{
		ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
	}
	END_ON_EXCEPTION

	UNLOCK_MUTEX();
	return returnResult;
}

char *
gprom_OperatorModelToQuery(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	Node * copiedTree;
    char *result ;
    NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
    TRY
    {
    	copiedTree = copyObject(nodeFromMimir);
        result = generatePlan(copiedTree, isRewriteOptionActivated(OPTION_OPTIMIZE_OPERATOR_MODEL));
        RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
    }
    ON_EXCEPTION
    {
    	ERROR_LOG("allocated in memory context: %s", currentExceptionToString());
    }
    END_ON_EXCEPTION
    return result;
}

void *
gprom_createMemContext(void)
{
	LOCK_MUTEX();
	MemContext *_newcontext_ = NEW_MEM_CONTEXT(LIBARY_REWRITE_CONTEXT);
	ACQUIRE_MEM_CONTEXT(_newcontext_);
	UNLOCK_MUTEX();
	return _newcontext_;
}

void *
gprom_createMemContextName(const char * ctxName)
{
	LOCK_MUTEX();
	MemContext *_newcontext_ = NEW_MEM_CONTEXT((char *)ctxName);
	ACQUIRE_MEM_CONTEXT(_newcontext_);
	UNLOCK_MUTEX();
	return _newcontext_;
}

void
gprom_freeMemContext(void * memContext)
{
	LOCK_MUTEX();
	//UNLOCK_MUTEX();
	MemContext *_newcontext_ = (MemContext *) memContext;
	ACQUIRE_MEM_CONTEXT(_newcontext_);
	FREE_CUR_MEM_CONTEXT();
	RELEASE_MEM_CONTEXT();
	UNLOCK_MUTEX();
}
