#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "model/node/nodetype.h"
#include "model/list/list.h"

typedef struct FunctionCall {
    NodeTag type;
    char *functionname;
    List *args;
} FunctionCall;

typedef struct Operator {
    NodeTag type;
    char *name;
    List *args;
} Operator;


typedef enum DataType
{
    DT_INT,
    DT_STRING,
    DT_FLOAT,
    DT_BOOL
} DataType;

typedef struct Constant {
    NodeTag type;
    DataType constType;
    void *value;
} Constant;

typedef struct AttributeReference {
    NodeTag type;
    char *name;
} AttributeReference;

/* functions to create expression nodes */
extern FunctionCall *createFunctionCall (char *fName, List *args);
extern Operator *createOpExpr (char *name, List *args);
extern AttributeReference *createAttributeReference (char *name);

/* functions for creating constants */
extern Constant *createConstInt (int value);
extern Constant *createConstString (char *value);
extern Constant *createConstFloat (float value);

#endif /* EXPRESSION_H */
