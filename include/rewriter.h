/*-----------------------------------------------------------------------------
 *
 * rewriter.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef REWRITER_H_
#define REWRITER_H_

#include "common.h"
#include "model/list/list.h"

#define QUERY_MEM_CONTEXT "QUERY_CONTEXT"

extern int initBasicModules (void);
extern int initBasicModulesAndReadOptions (char *appName, char *appHelpText, int argc, char* argv[]);
extern int readOptions (char *appName, char *appHelpText, int argc, char* argv[]);
extern void reactToOptionsChange (const char *optName);

extern void setupPluginsFromOptions(void);
extern void resetupPluginsFromOptions (void);
extern int readOptionsAndIntialize(char *appName, char *appHelpText, int argc, char* argv[]);
extern int shutdownApplication(void);

#define READ_OPTIONS_AND_INIT(app,helpText) \
    do { \
        int _retVal = readOptionsAndIntialize(app, helpText, argc, argv); \
        if (_retVal != EXIT_SUCCESS) \
        return EXIT_FAILURE;    \
    } while(0)

#define READ_OPTIONS_AND_BASIC_INIT(app,helpText) \
    do { \
        int _retVal = initBasicModulesAndReadOptions(app, helpText, argc, argv); \
        if (_retVal != EXIT_SUCCESS) \
        return EXIT_FAILURE;    \
    } while(0)

extern void processInput(char *input);
extern char *rewriteQuery(char *input);
extern char *rewriteQueryWithRethrow(char *input);
extern char *rewriteQueryFromStream (FILE *stream);
extern char *rewriteQueryWithOptimization(char *input);
extern char *generatePlan(Node *oModel, boolean applyOptimizations);
extern Node *optimizeOperatorModelRW(Node *oModel);
extern char *serializeOperatorModelRW(Node *oModel);

#endif /* REWRITER_H_ */
