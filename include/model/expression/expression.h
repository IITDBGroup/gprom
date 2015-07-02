#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "utility/enum_magic.h"

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


NEW_ENUM_WITH_TO_STRING(DataType,
    DT_INT,
    DT_LONG,
    DT_STRING,
    DT_FLOAT,
    DT_BOOL,
    DT_VARCHAR2
);

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
    DataType attrType;
} AttributeReference;

typedef struct SQLParameter {
    NodeTag type;
    char *name;
    int position;
    DataType parType;
} SQLParameter;

typedef struct RowNumExpr {
    NodeTag type;
} RowNumExpr;

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

NEW_ENUM_WITH_TO_STRING(WindowBoundType,
    WINBOUND_UNBOUND_PREC,
    WINBOUND_CURRENT_ROW,
    WINBOUND_EXPR_PREC,
    WINBOUND_EXPR_FOLLOW
);

typedef struct WindowBound {
    NodeTag type;
    WindowBoundType bType;
    Node *expr;
} WindowBound;

NEW_ENUM_WITH_TO_STRING(WinFrameType,
    WINFRAME_ROWS,
    WINFRAME_RANGE
);

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

typedef struct CastExpr {
    NodeTag type;
    DataType resultDT;
    Node *expr;
} CastExpr;

NEW_ENUM_WITH_TO_STRING(SortOrder,
    SORT_ASC,
    SORT_DESC
);

NEW_ENUM_WITH_TO_STRING(SortNullOrder,
    SORT_NULLS_FIRST,
    SORT_NULLS_LAST
);

typedef struct OrderExpr {
    NodeTag type;
    Node *expr;
    SortOrder order;
    SortNullOrder nullOrder;
} OrderExpr;

#define IS_EXPR(_n) (isA(_n,FunctionCall) || \
    isA(_n,Operator) || \
	isA(_n,Constant) || \
	isA(_n,AttributeReference) || \
	isA(_n,SQLParameter) || \
	isA(_n,RowNumExpr) || \
	isA(_n,CaseExpr) || \
	isA(_n,CaseWhen) || \
	isA(_n,IsNullExpr) || \
	isA(_n,WindowBound) || \
	isA(_n,WindowFrame) || \
	isA(_n,WindowDef) || \
	isA(_n,WindowFunction) || \
	isA(_n,CastExpr) || \
	isA(_n,OrderExpr)  \
    )

/* functions to create expression nodes */
extern FunctionCall *createFunctionCall (char *fName, List *args);
extern Operator *createOpExpr (char *name, List *args);
extern AttributeReference *createAttributeReference (char *name);
extern AttributeReference *createFullAttrReference (char *name, int fromClause, int attrPos,
        int outerLevelsUp, DataType attrType);
extern Node *andExprList (List *exprs);
extern Node *andExprs (Node *expr, ...);
extern Node *orExprs (Node *expr, ...);
#define AND_EXPRS(...) andExprs(__VA_ARGS__, NULL)
#define OR_EXPRS(...) orExprs(__VA_ARGS__, NULL)
extern SQLParameter *createSQLParameter (char *name);
extern CaseExpr *createCaseExpr (Node *expr, List *whenClauses, Node *elseRes);
extern CaseWhen *createCaseWhen (Node *when, Node *then);
extern IsNullExpr *createIsNullExpr (Node *expr);
extern Node *createIsNotDistinctExpr (Node *lArg, Node *rArg);

extern WindowBound *createWindowBound (WindowBoundType bType, Node *expr);
extern WindowFrame *createWindowFrame (WinFrameType winType, WindowBound *lower, WindowBound *upper);
extern WindowDef *createWindowDef (List *partitionBy, List *orderBy, WindowFrame *frame);
extern WindowFunction *createWindowFunction (FunctionCall *f, WindowDef *win);

extern OrderExpr *createOrderExpr (Node *expr, SortOrder order, SortNullOrder nullOrder);

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
#define CONST_TO_STRING(_c) (exprToSQL(_c))

/* functions for determining the type of an expression */
extern DataType typeOf (Node *expr);
extern DataType typeOfInOpModel (Node *expr, List *inputOperators);

/* create an SQL expression from an expression tree */
extern char *exprToSQL (Node *expr);

/* functions for searching inside expressions */
extern List *getAttrReferences (Node *node);

/* for the condition of selection operator, separate the AND operator to a
 * list of operators, these relation among these operators is AND */
extern void getSelectionCondOperatorList(Node *expr, List **opList);

/* combine a list operator to an AND operator */
extern Node *changeListOpToAnOpNode(List *l1);

#endif /* EXPRESSION_H */
