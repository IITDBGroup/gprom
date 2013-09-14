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
} OptionConnection;

typedef struct OptionDebug{
	boolean log;
	int loglevel;
	boolean debugMemory;
} OptionDebug;

typedef struct OptionRewrite{
	boolean alternative;

} OptionRewrite;

typedef struct Options{
	OptionConnection* optionConnection;
	OptionDebug* optionDebug;
	OptionRewrite* optionRewrite;
} Options;

#define MAKE_OPTIONS()		\
	((Options*)malloc(sizeof(Options)))

#define MAKE_OPTION_CONNECTION()		\
	((OptionConnection*)malloc(sizeof(OptionConnection)))

#define MAKE_OPTION_DEBUG()		\
	((OptionDebug*)malloc(sizeof(OptionDebug)))

#define MAKE_OPTION_REWRITE()			\
	((OptionRewrite*)malloc(sizeof(OptionRewrite)))

extern void mallocOptions();
extern void freeOptions();
extern Options* getOptions();

#endif
