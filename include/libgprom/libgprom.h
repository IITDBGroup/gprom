/*-----------------------------------------------------------------------------
 *
 * libgprom.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_LIBGPROM_LIBGPROM_H_
#define INCLUDE_LIBGPROM_LIBGPROM_H_

// define boolean type and ExceptionHandler if not already defined
#ifndef COMMON_H
typedef int boolean;
#endif

#ifndef INCLUDE_EXCEPTION_EXCEPTION_H_
typedef enum ExceptionHandler {
    EXCEPTION_DIE,
    EXCEPTION_ABORT,
    EXCEPTION_WIPE
} ExceptionHandler;
#endif

/* Handle for gprom encapsulating state
 *
 */
typedef struct libgprom_handle GProMHandle;

// initialize system
extern void gprom_init(void);
extern void gprom_readOptions(int argc, char *const args[]);
extern void gprom_readOptionAndInit(int argc, char *const args[]);
extern void gprom_configFromOptions(void);
extern void gprom_reconfPlugins(void);
extern void gprom_shutdown(void);

// process an input query
extern const char *gprom_rewriteQuery(const char *query);

// callback interface for logger (application can process log messages)
// takes message, c-file, line, loglevel
typedef void (*GProMLoggerCallbackFunction) (const char *,const char *,int,int);

// callback interface for exception handling (application can deal with exceptions)
// takes message, c-file, line, severity
// return value indicates what to do
typedef ExceptionHandler (*GProMExceptionCallbackFunction) (const char *, const char *, int, int);

// register handlers and set log level
extern void gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback);
extern void gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback);
extern void gprom_setMaxLogLevel (int maxLevel);

// interface to configuration
extern const char *gprom_getStringOption (const char *name);
extern int gprom_getIntOption (const char *name);
extern boolean gprom_getBoolOption (const char *name);
extern double gprom_getFloatOption (const char *name);
extern const char *gprom_getOptionType(const char *name);
extern boolean gprom_optionExists(const char *name);

extern void gprom_setOption(const char *name, const char *value);
extern void gprom_setStringOption (const char *name, const char *value);
extern void gprom_setIntOption(const char *name, int value);
extern void gprom_setBoolOption(const char *name, boolean value);
extern void gprom_setFloatOption(const char *name, double value);

/* plugin definition */
typedef struct GProMMetadataLookupPlugin
{
    /* init and shutdown plugin and connection */
    boolean (*isInitialized) (void);
    int (*initMetadataLookupPlugin) (void);
    int (*databaseConnectionOpen) (void);
    int (*databaseConnectionClose) (void);
    int (*shutdownMetadataLookupPlugin) (void);

    /* catalog lookup */
    boolean (*catalogTableExists) (char * tableName);
    boolean (*catalogViewExists) (char * viewName);
    char * (*getKeyInformation) (char *tableName);
    char * (*getDataTypes) (char *tableName);
    char * (*getAttributeNames) (char *tableName);
    char * (*getAttributeDefaultVal) (char *schema, char *tableName, char *attrName);

    boolean (*isAgg) (char *functionName);
    boolean (*isWindowFunction) (char *functionName);
    char * (*getFuncReturnType) (char *fName, char **args, int numArgs);
    char * (*getOpReturnType) (char *oName, char **args, int numArgs);

    char * (*getTableDefinition) (char *tableName);
    char * (*getViewDefinition) (char *viewName);

    /* audit log access */
//    void (*getTransactionSQLAndSCNs) (char *xid, List **scns, List **sqls,
//            List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
//    long (*getCommitScn) (char *tableName, long maxScn, char *xid);

    /* execution */
//    Node * (*executeAsTransactionAndGetXID) (List *statements, IsolationLevel isoLevel);
    //TODO define iterator interface for query results
//    char *** (*executeQuery) (char *query);       // returns a list of stringlist (tuples)
} GProMMetadataLookupPlugin;

extern void gprom_registerMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin);

#endif /* INCLUDE_LIBGPROM_LIBGPROM_H_ */
