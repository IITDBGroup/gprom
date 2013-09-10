#ifndef OPTION_H
#define OPTION_H

#include "Common.h"

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

#define MakeOptions()		\
	((Options*)malloc(sizeof(Options)))

#define MakeOptionConnection()		\
	((OptionConnection*)malloc(sizeof(OptionConnection)))

#define MakeOptionDebug()		\
	((OptionDebug*)malloc(sizeof(OptionDebug)))

#define MakeOptionRewrite()			\
	((OptionRewrite*)malloc(sizeof(OptionRewrite)))

extern void mallocOptions();
extern void freeOptions();
extern Options* getOptions();

#endif
