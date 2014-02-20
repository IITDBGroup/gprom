#ifndef OPTION_H
#define OPTION_H

#include "common.h"
#include <stdlib.h>

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

typedef struct OptionRewrite{
	RewriteMethod** rewriteMethods;
	int size;
} OptionRewrite;

typedef struct Options{
	OptionConnection* optionConnection;
	OptionDebug* optionDebug;
	OptionRewrite* optionRewrite;
} Options;

#define MAKE_OPTIONS()		\
	((Options*)calloc(1,sizeof(Options)))

#define MAKE_OPTION_CONNECTION()		\
	((OptionConnection*)calloc(1, sizeof(OptionConnection)))

#define MAKE_OPTION_DEBUG()		\
	((OptionDebug*)calloc(1, sizeof(OptionDebug)))

#define MAKE_OPTION_REWRITE()			\
	((OptionRewrite*)calloc(1, sizeof(OptionRewrite)))

extern void mallocOptions();
extern void freeOptions();
extern Options* getOptions();
extern boolean isRewriteOptionActivated(char *name);

#endif
