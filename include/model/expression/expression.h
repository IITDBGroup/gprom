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
    boolean isNull;
} Constant;

#define INVALID_ATTR -1
#define INVALID_FROM_ITEM -1
#define INVALID_PARAM -1;

typedef struct AttributeReference {
    NodeTag type;
    char *name;
    int fromClauseItem;
    int attrPosition;
    int outerLevelsUp;
} AttributeReference;

typedef struct SQLParameter {
    NodeTag type;
    char *name;
    int position;
    DataType parType;
} SQLParameter;

typedef struct CaseExpr {
    NodeTag type;
    Node *expr;
    List *whenClauses;
    Node *elseRes;
} CaseExpr;

typedef struct CaseWhen {
    NodeTag type;
    Node *when;
    Node *then;
} CaseWhen;

typedef struct IsNullExpr {
    NodeTag type;
    Node *expr;
} IsNullExpr;

typedef enum WindowBoundType {
    WINBOUND_UNBOUND_PREC,
    WINBOUND_CURRENT_ROW,
    WINBOUND_EXPR_PREC,
    WINBOUND_EXPR_FOLLOW
} WindowBoundType;

typedef struct WindowBound {
    NodeTag type;
    WindowBoundType bType;
    Node *expr;
} WindowBound;

typedef enum WinFrameType {
    WINFRAME_ROWS,
    WINFRAME_RANGE
} WinFrameType;

typedef struct WindowFrame {
    NodeTag type;
    WinFrameType frameType;
    WindowBound *lower;
    WindowBound *higher;
} WindowFrame;

typedef struct WindowDef {
    NodeTag type;
    List *partitionBy;
    List *orderBy;
    WindowFrame *frame;
} WindowDef;

typedef struct WindowFunction {
    NodeTag type;
    FunctionCall *f;
    WindowDef *win;
} WindowFunction;

/* functions to create expression nodes */
extern FunctionCall *createFunctionCall (char *fName, List *args);
extern Operator *createOpExpr (char *name, List *args);
extern AttributeReference *createAttributeReference (char *name);
extern AttributeReference *createFullAttrReference (char *name, int fromClause,
        int attrPos, int outerLevelsUp);
extern Node *andExprs (Node *expr, ...);
#define AND_EXPRS(...) andExprs(__VA_ARGS__, NULL)
extern SQLParameter *createSQLParameter (char *name);
extern CaseExpr *createCaseExpr (Node *expr, List *whenClauses, Node *elseRes);
extern CaseWhen *createCaseWhen (Node *when, Node *then);
extern IsNullExpr *createIsNullExpr (Node *expr);
extern Node *createIsNotDistinctExpr (Node *lArg, Node *rArg);

extern WindowBound *createWindowBound (WindowBoundType bType, Node *expr);
extern WindowFrame *createWindowFrame (WinFrameType winType, WindowBound *lower, WindowBound *upper);
extern WindowDef *createWindowDef (List *partitionBy, List *orderBy, WindowFrame *frame);
extern WindowFunction *createWindowFunction (FunctionCall *f, WindowDef *win);

/* functions for creating constants */
extern Constant *createConstInt (int value);
extern Constant *createConstLong (long value);
extern Constant *createConstString (char *value);
extern Constant *createConstFloat (double value);
extern Constant *createConstBool (boolean value);
extern Constant *createNullConst (DataType dt);
#define INT_VALUE(_c) *((int *) ((Constant *) _c)->value)
#define FLOAT_VALUE(_c) *((double *) ((Constant *) _c)->value)
#define LONG_VALUE(_c) *((long *) ((Constant *) _c)->value)
#define BOOL_VALUE(_c) *((boolean *) ((Constant *) _c)->value)
#define STRING_VALUE(_c) ((char *) ((Constant *) _c)->value)
#define CONST_IS_NULL(_c) (((Constant *) _c)->isNull)

/* functions for determining the type of an expression */
extern DataType typeOf (Node *expr);
extern DataType typeOfInOpModel (Node *expr, List *inputOperators);

extern char *exprToSQL (Node *expr);

#endif /* EXPRESSION_H */
