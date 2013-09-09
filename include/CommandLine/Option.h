#ifndef OPTION_H
#define OPTION_H

#include "CommandLine/Common.h"

#define SIZE 20

typedef struct OptionConnection{
	char host[SIZE];
	char db[SIZE];
	int port;
	char user[SIZE];
	char passwd[SIZE];
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

#endif
