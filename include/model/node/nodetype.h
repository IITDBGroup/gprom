#ifndef NODETYPE_H
#define NODETYPE_H

#include "common.h"
#include "utility/enum_magic.h"

NEW_ENUM_WITH_TO_STRING(NodeTag,
    T_Invalid,
    T_Node,

    /* collection types */
    T_List,
    T_IntList,
    T_Set,
    T_HashMap,
    T_Vector,

    /* options */
    T_KeyValue,

    /* expression nodes */
    T_Constant,
    T_AttributeReference,
    T_SQLParameter,
    T_FunctionCall,
    T_Operator,
    T_CaseExpr,
    T_CaseWhen,
    T_IsNullExpr,
    T_WindowBound,
    T_WindowFrame,
    T_WindowDef,
    T_WindowFunction,
    T_RowNumExpr,
    T_OrderExpr,
    T_CastExpr,

    /* query block model nodes */
    T_SetQuery,
    T_ProvenanceStmt,
    T_ProvenanceTransactionInfo,
    T_QueryBlock,
    T_SelectItem,
    T_FromItem,
    T_FromProvInfo,
    T_FromTableRef,
    T_FromSubquery,
    T_FromJoinExpr,
    T_DistinctClause,
    T_NestedSubquery,
    T_Insert,
    T_Delete,
    T_Update,
    T_TransactionStmt,
    T_WithStmt,
    T_DDLStatement,
    T_UtilityStatement,

    /* query operator model nodes */
    T_Schema,
    T_AttributeDef,
    T_QueryOperator,
    T_SelectionOperator,
    T_ProjectionOperator,
    T_JoinOperator,
    T_AggregationOperator,
    T_ProvenanceComputation,
    T_TableAccessOperator,
    T_SetOperator,
    T_DuplicateRemoval,
    T_ConstRelOperator,
    T_NestingOperator,
    T_WindowOperator,
    T_OrderOperator,

    /* datalog model nodes */
    T_DLNode,
    T_DLAtom,
    T_DLVar,
    T_DLRule,
    T_DLProgram,
    T_DLComparison,

    /* Json Table Node */
    T_FromJsonTable,
    T_JsonTableOperator,
    T_JsonColInfoItem,
    T_JsonPath
);

typedef struct Node{
    NodeTag type;
} Node;

NEW_ENUM_WITH_TO_STRING(ProvenanceType,
    PROV_PI_CS,
    PROV_TRANSFORMATION
);

/* what type of database operation(s) a provenance computation is for */
NEW_ENUM_WITH_TO_STRING(ProvenanceInputType,
    PROV_INPUT_QUERY,
    PROV_INPUT_UPDATE,
    PROV_INPUT_UPDATE_SEQUENCE,
    PROV_INPUT_TRANSACTION,
    PROV_INPUT_TIME_INTERVAL
);

/* stringinfo provides the string data type*/

typedef struct StringInfoData
{
    char *data;
    int  len;
    int maxlen;
    int cursor;
} StringInfoData;

typedef StringInfoData *StringInfo;

/* Key-Value pair */

typedef struct KeyValue
{
    NodeTag type;
    Node *key;
    Node *value;
} KeyValue;

/*------------------------------------------------------------------
*makeStringInfo
*The function is create an empty StringInfoData and return a pointer.
*-------------------------------------------------------------------*/
extern StringInfo makeStringInfo(void);
//extern StringInfo makeStringInfoString(char *string);

/*------------------------------------------------------------------
*initStringInfo
*The function is init a StringInfoData.
*-------------------------------------------------------------------*/
extern void initStringInfo(StringInfo str);

/*------------------------------------------------------------------
*resetStringInfo
*The function is clear the current content of StringInfo.
*-------------------------------------------------------------------*/
extern void resetStringInfo(StringInfo str);

/*------------------------------------------------------------------
*appendStringInfoString
*The function is append a string to str.
*-------------------------------------------------------------------*/
extern void appendStringInfoString(StringInfo str, const char *s);
extern void appendStringInfoStrings(StringInfo str, ...);
extern char *concatStrings(const char *s, ...);

#define CONCAT_STRINGINFO(str, other) appendStringInfoString(str, other->data)
#define CONCAT_STRINGS(...) concatStrings(__VA_ARGS__, NULL)

/*------------------------------------------------------------------
* The function is append to a StringInfo using a format string and a variable
* length parameter list.
*-------------------------------------------------------------------*/
extern void appendStringInfo(StringInfo str, const char *format, ...);
extern boolean vAppendStringInfo(StringInfo str, const char *format, va_list args);
extern void appendStringInfoChar(StringInfo str, char ch);
extern void appendBinaryStringInfo(StringInfo str, const char *data, int datalen);
extern void prependStringInfo (StringInfo str, const char *format, ...);
extern void enlargeStringInfo(StringInfo str, int needed);
#define STRINGLEN(_str) _str->len

// node helpers
#define nodeTag(nodeptr) (((Node*)(nodeptr))->type)
#define makeNode(type)  ((type*)newNode(sizeof(type),T_##type))
#define nodeSetTag(nodeptr,t) (((Node*)(nodeptr))->type = (t))
#define isA(nodeptr, type)  (nodeptr != NULL && (nodeTag(nodeptr) == T_##type))

/*extern declaration */
extern Node *newNode(size_t size, NodeTag type);
extern KeyValue *createStringKeyValue(char *key, char *value);
extern KeyValue *createNodeKeyValue(Node *key, Node *value);

/* get a string representation of a node */
extern char *nodeToString(void *obj);
extern char *beatify(char *input);
char *operatorToOverviewString(Node *op);
char *datalogToOverviewString(Node *n);
char *itoa(int value);

/* get a dot script for a query operator graph or query block tree */
extern char *nodeToDot(void *obj);

#define DOT_TO_CONSOLE(obj) \
    do { \
        if (getBoolOption(OPTION_GRAPHVIZ)) \
            printf("%s",nodeToDot(obj)); \
    } while (0)

/* create a node tree from a string */
extern void *stringToNode(char *str);

/* deep copy a node */
//#define COPY_OBJECT_TO_CONTEXT(obj, result, context)

//    (AQUIRE_MEM_CONTEXT(context,))
extern void *copyObject(void *obj);

/* deep equals for nodes */
extern boolean equal(void *a, void *b);
extern boolean ptrEqual(void *a, void *b);

/* deep free a node structure */
extern void deepFree(void *a);
extern void freeStringInfo (StringInfo node);

/* compute a hash for a node structure */
extern uint64_t hashValue(void *a);

/* compute a hash value for a node tree */
//extern int hashObject(void *a);

/*
 * Visit all nodes in a tree using a user-provided function that decides
 * whether to continue the traversal. The user function should return
 * true if the traversal should be continued and false otherwise. The
 * user function has to take a state parameter as second input that can, e.g.,
 * be used to store search results.
 */
extern boolean visit (Node *node, boolean (*checkNode) (), void *state);
extern Node *mutate (Node *node, Node *(*modifyNode) (), void *state);
extern boolean visitWithPointers (Node *node,
        boolean (*userF) (),
        void **parentLink, void *state);


#endif /*NODETYPE_H*/
