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
#include "libgprom/libgprom.h"
#include "model/set/hashmap.h"
#include "rewriter.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
//#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_external.h"

#define LIBARY_REWRITE_CONTEXT "LIBGRPROM_QUERY_CONTEXT"

#define LOCK_NAME gprom_lib_globallock

#ifdef HAVE_LIBPTHREAD
#define LOCK_MUTEX() pthread_mutex_lock(&LOCK_NAME); printf("\nMUTEX\n%s:%u\n", __FILE__, __LINE__); fflush(stdout);
#define UNLOCK_MUTEX() printf("\nUNLOCK\n%s:%u\n", __FILE__, __LINE__); fflush(stdout); pthread_mutex_unlock(&LOCK_NAME);
#define CREATE_MUTEX static pthread_mutex_t LOCK_NAME = PTHREAD_MUTEX_INITIALIZER;
#define DESTROY_MUTEX()
#else
#define LOCK_MUTEX() do {} while(0)
#define UNLOCK_MUTEX() do {} while(0)
#define CREATE_MUTEX
#define DESTROY_MUTEX() do {} while(0)
#endif

CREATE_MUTEX
#define PRINT_TRY(counter) //printf("\ntry: line: %u counter: %u", __LINE__, counter)
#define PRINT_EXCEPT(counter) printf("\nexcept: [libgprom.c] line: %u counter: %u\n", __LINE__, counter)

void
rebootFunc(void)
{
	gprom_shutdown();
	gprom_init();
}

void
gprom_init(void)
{
    LOCK_MUTEX();
    initMemManager();
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
    LOCK_MUTEX();
    setupPluginsFromOptions();
    UNLOCK_MUTEX();
}

void
gprom_reconfPlugins(void)
{
    LOCK_MUTEX();
    resetupPluginsFromOptions();
    UNLOCK_MUTEX();
}

void gprom_shutdown(void)
{
    LOCK_MUTEX();
    deregisterSignalHandler();
    shutdownApplication();
    UNLOCK_MUTEX();
//    DESTROY_MUTEX();
}

const char *
gprom_rewriteQuery(const char *query)
{
    LOCK_MUTEX();
    NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
    char *result = "";
    char *returnResult = NULL;
    TRY
    {
        result = rewriteQueryWithRethrow((char *) query);
        RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(result,returnResult);
    }
    ON_EXCEPTION
    {
        ERROR_LOG("\nLIBGPROM Error occured\n%s", currentExceptionToString());
        UNLOCK_MUTEX();
        return NULL;
    }
    END_ON_EXCEPTION
    UNLOCK_MUTEX();
    return returnResult;
}


void
gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback)
{
    LOCK_MUTEX();
    registerLogCallback(callback);
    UNLOCK_MUTEX();
}

void
gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback)
{
LOCK_MUTEX();
    registerExceptionCallback((GProMExceptionCallbackFunctionInternal) callback);
    UNLOCK_MUTEX();
}

void
gprom_setMaxLogLevel (int maxLevel)
{
LOCK_MUTEX();
    setMaxLevel((LogLevel) maxLevel);
    UNLOCK_MUTEX();
}


const char *
gprom_getStringOption (const char *name)
{
    LOCK_MUTEX();
    const char *result = getStringOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

int
gprom_getIntOption (const char *name)
{
    LOCK_MUTEX();
    int result = getIntOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

boolean
gprom_getBoolOption (const char *name)
{
    LOCK_MUTEX();
    boolean result = getBoolOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

double
gprom_getFloatOption (const char *name)
{
    LOCK_MUTEX();
    float result = getFloatOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

const char *
gprom_getOptionType(const char *name)
{
    LOCK_MUTEX();
    ASSERT(hasOption((char *) name));
    char *result = OptionTypeToString(getOptionType((char *) name));
    UNLOCK_MUTEX();
    return result;
}

boolean
gprom_optionExists(const char *name)
{
    LOCK_MUTEX();
    boolean result = hasOption((char *) name);
    UNLOCK_MUTEX();
    return result;
}

void
gprom_setOptionsFromMap()
{
LOCK_MUTEX();
UNLOCK_MUTEX();
}

void
gprom_setOption(const char *name, const char *value)
{
    LOCK_MUTEX();
    setOption((char *) name, strdup((char *) value));
    UNLOCK_MUTEX();
}

void
gprom_setStringOption (const char *name, const char *value)
{
    LOCK_MUTEX();
    setStringOption((char *) name, strdup((char *) value));
    UNLOCK_MUTEX();
}

void
gprom_setIntOption(const char *name, int value)
{
    LOCK_MUTEX();
    setIntOption((char *) name, value);
    UNLOCK_MUTEX();
}

void
gprom_setBoolOption(const char *name, boolean value)
{
    LOCK_MUTEX();
    setBoolOption((char *) name,value);
    UNLOCK_MUTEX();
}

void
gprom_setFloatOption(const char *name, double value)
{
    LOCK_MUTEX();
    setFloatOption((char *) name,value);
    UNLOCK_MUTEX();
}

void
gprom_registerMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin)
{
	LOCK_MUTEX();
    setMetadataLookupPlugin(assembleExternalMetadataLookupPlugin(plugin));
	UNLOCK_MUTEX();
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
    int counter = 0;
    TRY
    {
    TRY

    {
    	PRINT_TRY(counter);
    	if(counter++ == 0){
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
    }
    ON_EXCEPTION
	{
    	PRINT_EXCEPT(counter);
    	NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
	}
	END_ON_EXCEPTION
    }
    END_TRY

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
	    int counter = 0;
	    TRY
	        {TRY
		{
	    	PRINT_TRY(counter);
	    	if(counter++ == 0){
				copiedTree = copyObject(nodeFromMimir);
				rewrittenTree = provRewriteQBModel(copiedTree);


				UNLOCK_MUTEX();
				returnResult = (GProMNode*)rewrittenTree;
				//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
				return returnResult;
	        }
	    }
		ON_EXCEPTION
		{
			PRINT_EXCEPT(counter);
			NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
		}
		END_ON_EXCEPTION
	        }
	        		    END_TRY
	    UNLOCK_MUTEX();
	    //RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
	    return NULL;
}

GProMNode *
gprom_taintRewriteOperator(GProMNode* nodeFromMimir)
{
	LOCK_MUTEX();
	    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	    //char *result = "";
	    Node *rewrittenTree;
	    Node *copiedTree;
	    GProMNode* returnResult;
	    int counter = 0;
	    TRY
	        {TRY
		{
	    	PRINT_TRY(counter);
	    	if(counter++ == 0){
				copiedTree = copyObject(nodeFromMimir);
				rewrittenTree = (Node *)rewriteUncert((QueryOperator *)copiedTree);


				UNLOCK_MUTEX();
				returnResult = (GProMNode*)rewrittenTree;
				//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
				return returnResult;
	        }
	    }
		ON_EXCEPTION
		{
			PRINT_EXCEPT(counter);
			NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
		}
		END_ON_EXCEPTION
	        }
	        		    END_TRY
	    UNLOCK_MUTEX();
	    //RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
	    return NULL;
}

GProMNode *
gprom_optimizeOperatorModel(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	//NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
	    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	    //char *result = "";
	    Node *rewrittenTree;
	    Node *copiedTree;
	    GProMNode* returnResult;
	    int counter = 0;
	    TRY
	        {TRY
	    {
	    	PRINT_TRY(counter);
	    	if(counter++ == 0){
	        	copiedTree = copyObject(nodeFromMimir);

				rewrittenTree = optimizeOperatorModelRW(copiedTree);

				UNLOCK_MUTEX();
				returnResult = (GProMNode*)rewrittenTree;
				//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
				return returnResult;
	        }
		}
	    ON_EXCEPTION
		{
	    	PRINT_EXCEPT(counter);
	    	NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
		}
		END_ON_EXCEPTION
	        }
	        		    END_TRY
		UNLOCK_MUTEX();
		//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
		return NULL;
}

char *
gprom_operatorModelToSql(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	//NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
	    //NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
	    //char *result = "";
	    char *rewrittenSQL;
	    Node *copiedTree;
	    int counter = 0;
	    TRY
	        {TRY
	    {
	    	PRINT_TRY(counter);
	    	if(counter++ == 0){
				copiedTree = copyObject(nodeFromMimir);

				rewrittenSQL = serializeOperatorModelRW(copiedTree);

				UNLOCK_MUTEX();
				//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
				return rewrittenSQL;
	        }
		}
	    ON_EXCEPTION
		{
	    	PRINT_EXCEPT(counter);
	    	NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
		}
		END_ON_EXCEPTION
	        }
	        		    END_TRY
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
	int counter = 0;
	TRY
	    {TRY
	{
		PRINT_TRY(counter);
		if(counter++ == 0){
			//copiedTree = copyObject(nodeFromMimir);
			returnResult = jsonify(nodeToString(nodeFromMimir));

			UNLOCK_MUTEX();
			//RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(result,returnResult);
			//RELEASE_MEM_CONTEXT_AND_CREATE_STRING_COPY(beatify(nodeToString(nodeFromMimir)),returnResult);
			return returnResult;
	    }
	}
	ON_EXCEPTION
	{
		PRINT_EXCEPT(counter);
		NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
	}
	END_ON_EXCEPTION
	    }
	    		    END_TRY
	UNLOCK_MUTEX();
	return returnResult;
}

char *
gprom_OperatorModelToQuery(GProMNode * nodeFromMimir)
{
	LOCK_MUTEX();
	Node * copiedTree;
    char *result = "";
    //NEW_AND_ACQUIRE_MEMCONTEXT(QUERY_MEM_CONTEXT);
    int counter = 0;
    TRY
        {TRY
	{
    	PRINT_TRY(counter);
    	if(counter++ == 0){
			copiedTree = copyObject(nodeFromMimir);
			result = generatePlan(copiedTree, isRewriteOptionActivated(OPTION_OPTIMIZE_OPERATOR_MODEL));
			UNLOCK_MUTEX();
			//RELEASE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
			return result;
		}
    }
	ON_EXCEPTION
	{
		PRINT_EXCEPT(counter);
		NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
	}
	END_ON_EXCEPTION
        }
        		    END_TRY
	UNLOCK_MUTEX();
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


GProMHashMap* gprom_addToMap(GProMHashMap* map, GProMNode * key, GProMNode * value)
{

	LOCK_MUTEX();
		   ///NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
		    HashMap* newMap;
			Node *newKey;
		    Node *newValue;
		    GProMHashMap* returnResult;
		    int counter = 0;
		    TRY
		        {TRY
			{
		    	PRINT_TRY(counter);
		    	if(counter++ == 0){
					newKey = (Node*)copyObject(key);
					newValue = (Node*)copyObject(value);

					if(map == NULL){
						newMap = newHashMap(newKey->type, newValue->type, NULL, NULL);
					}
					else
						newMap = (HashMap*)copyObject(map);

					addToMap(newMap, newKey, newValue);

					UNLOCK_MUTEX();

					returnResult = (GProMHashMap*)copyObject(newMap);
					//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
					return returnResult;
				}
			}
			ON_EXCEPTION
			{
				PRINT_EXCEPT(counter);
				NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
			}
			END_ON_EXCEPTION
		        }
		    END_TRY

			UNLOCK_MUTEX();
			//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
			return NULL;
}

GProMNode * gprom_getMap(GProMHashMap* map, GProMNode* key)
{

	LOCK_MUTEX();
		   ///NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
		    HashMap* newMap;
			Node *newKey;
			Node *gotValue;
			GProMNode* returnResult;
		    int counter = 0;
		    TRY
		        {TRY
			{
		    	PRINT_TRY(counter);
		    	if(counter++ == 0){
		    		newMap = (HashMap*)copyObject(map);
		    		newKey = (Node*)copyObject(key);

					gotValue =  getMap(newMap, newKey);

					UNLOCK_MUTEX();

					returnResult = (GProMNode*)copyObject(gotValue);
					//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
					return returnResult;
				}
			}
			ON_EXCEPTION
			{
				PRINT_EXCEPT(counter);
				NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
			}
			END_ON_EXCEPTION
		        }
		    END_TRY

			UNLOCK_MUTEX();
			//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
			return NULL;
}

GProMNode * gprom_getMapString(GProMHashMap* map, char* key)
{

	LOCK_MUTEX();
		   ///NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT);
		    HashMap* newMap;
			Node *gotValue;
			GProMNode* returnResult;
		    int counter = 0;
		    TRY
		        {TRY
			{
		    	PRINT_TRY(counter);
		    	if(counter++ == 0){
		    		newMap = (HashMap*)copyObject(map);
					gotValue =  getMapString(newMap, key);

					UNLOCK_MUTEX();

					returnResult = (GProMNode*)copyObject(gotValue);
					//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, rewrittenTree);
					return returnResult;
				}
			}
			ON_EXCEPTION
			{
				PRINT_EXCEPT(counter);
				NEW_AND_ACQUIRE_MEMCONTEXT(LIBARY_REWRITE_CONTEXT)
			}
			END_ON_EXCEPTION
		        }
		    END_TRY

			UNLOCK_MUTEX();
			//RELEASE_MEM_CONTEXT_AND_RETURN_COPY(GProMNode, NULL);
			return NULL;
}
