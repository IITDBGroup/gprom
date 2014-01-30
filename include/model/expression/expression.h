#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "model/node/nodetype.h"
#include "model/list/list.h"

typedef struct FunctionCall {
    NodeTag type;
    char *functionname;
    List *args;
    boolean isAgg;
} FunctionCall;

typedef struct Operator {
    NodeTag type;
    char *name;
    List *args;
} Operator;


typedef enum DataType
{
    DT_INT,
    DT_LONG,
    DT_STRING,
    DT_FLOAT,
    DT_BOOL
} DataType;

typedef struct Constant {
    NodeTag type;
    DataType constType;
    void *value;
} Constant;

#define INVALID_ATTR -1
#define INVALID_FROM_ITEM -1

typedef struct AttributeReference {
    NodeTag type;
    char *name;
    int fromClauseItem;
    int attrPosition;
    int outerLevelsUp;
} AttributeReference;

/* functions to create expression nodes */
extern FunctionCall *createFunctionCall (char *fName, List *args);
extern Operator *createOpExpr (char *name, List *args);
extern AttributeReference *createAttributeReference (char *name);
extern AttributeReference *createFullAttrReference (char *name, int fromClause,
        int attrPos, int outerLevelsUp);
extern Node *andExprs (Node *expr, ...);
#define AND_EXPRS(...) andExprs(__VA_ARGS__, NULL);

/* functions for creating constants */
extern Constant *createConstInt (int value);
extern Constant *createConstLong (long value);
extern Constant *createConstString (char *value);
extern Constant *createConstFloat (double value);
extern Constant *createConstBool (boolean value);
#define INT_VALUE(_c) *((int *) ((Constant *) _c)->value)
#define FLOAT_VALUE(_c) *((double *) ((Constant *) _c)->value)
#define BOOL_VALUE(_c) *((boolean *) ((Constant *) _c)->value)
#define STRING_VALUE(_c) ((char *) ((Constant *) _c)->value)

/* functions for determining the type of an expression */
extern DataType typeOf (Node *expr);
extern DataType typeOfInOpModel (Node *expr, List *inputOperators);

extern char *exprToSQL (Node *expr);

#endif /* EXPRESSION_H */
