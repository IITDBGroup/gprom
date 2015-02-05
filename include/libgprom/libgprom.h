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

// initialize system
extern void gprom_init(void);
extern void gprom_readOptions(int argc, char *const args[]);
extern void gprom_readOptionAndInit(int argc, char *const args[]);
extern void gprom_configFromOptions(void);
extern void gprom_shutdown(void);

// process an input query
extern const char *gprom_rewriteQuery(const char *query);

// callback interface for logger (application can process log message)
// takes message, c-file, line, loglevel
typedef void (*GProMLoggerCallbackFunction) (const char *,const char *,int,int);

extern void gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback);
extern void gprom_setMaxLogLevel (int maxLevel);

// provide the same for exception handling
//void registerExecptionHandler ()

// interface to configuration
extern char *gprom_getStringOption (char *name);
extern int gprom_getIntOption (char *name);
extern boolean gprom_getBoolOption (char *name);
extern double gprom_getFloatOption (char *name);
extern char *gprom_getOptionType(char *name);
extern boolean gprom_optionExists(char *name);

extern void gprom_setStringOption (char *name, char *value);
extern void gprom_setIntOption(char *name, int value);
extern void gprom_setBoolOption(char *name, boolean value);
extern void gprom_setFloatOption(char *name, double value);

#endif /* INCLUDE_LIBGPROM_LIBGPROM_H_ */
