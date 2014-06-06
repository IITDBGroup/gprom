#ifndef OPTION_H
#define OPTION_H

#include "common.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

typedef struct OptionConnection{
	char* host;
	char* db;
	int port;
	char* user;
	char* passwd;
	char *sql;
} OptionConnection;

typedef struct OptionDebug{
	boolean log;
	int loglevel;
	boolean debugMemory;
} OptionDebug;

typedef struct RewriteMethod{
	char* name;
	boolean isActive;
} RewriteMethod;

//typedef struct OptionRewrite{
//	RewriteMethod** rewriteMethods;
//	int size;
//} OptionRewrite;

typedef struct Options{
	OptionConnection* optionConnection;
	OptionDebug* optionDebug;
	List* optionRewrite;
} Options;

#define MAKE_OPTIONS()		\
	((Options*)calloc(1,sizeof(Options)))

#define MAKE_OPTION_CONNECTION()		\
	((OptionConnection*)calloc(1, sizeof(OptionConnection)))

#define MAKE_OPTION_DEBUG()		\
	((OptionDebug*)calloc(1, sizeof(OptionDebug)))

/* define rewrite methods */

#define OPTION_TIMING "timing"
#define OPTION_AGGRESSIVE_MODEL_CHECKING "aggressive_model_checking"
#define OPTION_UPDATE_ONLY_USE_CONDS "only_updated_use_conditions"
#define OPTION_UPDATE_ONLY_USE_HISTORY_JOIN "only_updated_use_history"
#define OPTION_TREEIFY_OPERATOR_MODEL "treefiy_prov_rewrite_input"
#define OPTION_PI_CS_USE_COMPOSABLE "pi_cs_use_composable"
#define OPTION_OPTIMIZE_OPERATOR_MODEL "optimize_operator_model"
#define OPTION_TRANSLATE_UPDATE_WITH_CASE "translate_update_with_case"
#define OPTION_

//#define MAKE_OPTION_REWRITE()			\
//	((OptionRewrite*)calloc(1, sizeof(OptionRewrite)))

extern void mallocOptions();
extern void freeOptions();
extern Options *getOptions();
extern boolean isRewriteOptionActivated(char *name);

#endif
