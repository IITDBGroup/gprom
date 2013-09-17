#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "model/node/nodetype.h"
#include "model/list/list.h"

typedef struct FunctionCall{
	NodeTag type;
	char *functionname;
	List *argument;
} FunctionCall;

typedef struct Operator{
	NodeTag type;
	char *name;
	List *argument;
} Operator;


typedef struct Constant{
	NodeTag type;
	int constType;
	void *value;
} Constant;

typedef struct AttributeReference{
	NodeTag type;
	char *name;
} AttributeReference;

#endif /* EXPRESSION_H */
