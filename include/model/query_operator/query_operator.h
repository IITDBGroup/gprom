/*-----------------------------------------------------------------------------
 *
 * query_operator.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef QUERY_OPERATOR_H_
#define QUERY_OPERATOR_H_

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"

typedef struct AttributeDef
{
    NodeTag type;
    DataType dataType;
    char *attrName;
} AttributeDef;

typedef struct Schema
{
    NodeTag type;
    char *name;
    List *attrDefs; // AttributeDef type
} Schema;

typedef struct QueryOperator
{
    NodeTag type;
    List *inputs; // children of the operator node, QueryOperator type
    Schema *schema; // attributes and their data types of result tables, Schema type
    List *parents; // direct parents of the operator node, QueryOperator type
    List *provAttrs; // positions of provenance attributes in the operator's schema
    Node *properties; // generic node to store flexible list or map of properties (KeyValue) for query operators
} QueryOperator; // common fields that all operators have

typedef struct TableAccessOperator
{
    QueryOperator op;
    char *tableName;
    Node *asOf;
//    Node *sampClause;
} TableAccessOperator;

typedef struct SelectionOperator
{
    QueryOperator op;
    Node *cond; // condition expression
} SelectionOperator;

typedef struct ProjectionOperator
{
    QueryOperator op;
    List *projExprs; // projection expressions
} ProjectionOperator;

typedef struct JoinOperator
{
    QueryOperator op;
    JoinType joinType;
    Node *cond; // join condition expression
} JoinOperator;

typedef struct AggregationOperator
{
    QueryOperator op;
    List *aggrs; // aggregation expressions, FunctionCall type
    List *groupBy; // group by expressions
} AggregationOperator;

typedef struct SetOperator
{
    QueryOperator op;
    SetOpType setOpType;
} SetOperator;

typedef struct DuplicateRemoval
{
    QueryOperator op;
    List *attrs; // attributes that need duplicate removal, AttributeReference type
} DuplicateRemoval;

typedef struct ProvenanceComputation
{
    QueryOperator op;
    ProvenanceType provType;
    ProvenanceInputType inputType;
    ProvenanceTransactionInfo *transactionInfo;
    Node *asOf;
} ProvenanceComputation;

NEW_ENUM_WITH_TO_STRING(ReenactUpdateType,
        UPDATE_TYPE_DELETE,
        UPDATE_TYPE_UPDATE,
        UPDATE_TYPE_INSERT_VALUES,
        UPDATE_TYPE_INSERT_QUERY,
        UPDATE_TYPE_DDL,
        UPDATE_TYPE_QUERY
);

typedef struct UpdateOperator
{
    QueryOperator op;
    char *tableName;
} UpdateOperator;

typedef struct ConstRelOperator
{
    QueryOperator op;
    List *values;
} ConstRelOperator;

typedef struct NestingOperator
{
	QueryOperator op;
	NestingExprType nestingType;
	Node *cond;
} NestingOperator;

typedef struct WindowOperator
{
    QueryOperator op;
    List *partitionBy;
    List *orderBy;
    WindowFrame *frameDef;
    char *attrName;
    Node *f;
} WindowOperator;

typedef struct OrderOperator
{
    QueryOperator op;
    List *orderExprs;
} OrderOperator;

typedef struct JsonTableOperator
{
    QueryOperator op;
    //List *pathExprs;
    List *columns;
    char *documentcontext;
    AttributeReference *jsonColumn;
    char *jsonTableIdentifier;
    char *forOrdinality;
} JsonTableOperator;

typedef struct SampleClauseOperator
{
	QueryOperator op;
	Node *sampPerc;
} SampleClauseOperator;

typedef struct LimitOperator
{
	QueryOperator op;
	Node *limitExpr;
	Node *offsetExpr;
} LimitOperator;

typedef struct ExecPreparedOperator
{
	QueryOperator op;
	char *name;
	List *params;
} ExecPreparedOperator;

/* type of operator macros */
#define IS_NULLARY_OP(op) (isA(op, TableAccessOperator) \
                        || isA(op, ConstRelOperator) \
						|| isA(op, SampleClauseOperator))

#define IS_UNARY_OP(op) (isA(op,ProjectionOperator)     \
        || isA(op,SelectionOperator)                    \
        || isA(op,AggregationOperator)                  \
        || isA(op,DuplicateRemoval)                     \
        || isA(op,WindowOperator)                       \
		|| isA(op,OrderOperator)                        \
		|| isA(op,LimitOperator)                        \
		|| isA(op,JsonTableOperator)                    \
		)

#define IS_BINARY_OP(op) (isA(op,JoinOperator)          \
        || isA(op,SetOperator)                          \
        || isA(op,NestingOperator))

#define IS_SPECIAL_OP(op) (isA(op,ProvenanceComputation) \
    || isA(op,/UpdateOperator) \
    || isA(op,/ExecPreparedOperator) \
    )

#define IS_OP(op) (IS_NULLARY_OP(op) || IS_UNARY_OP(op) || IS_BINARY_OP(op))

#define IS_QB(op) (IS_OP(op) || (isA(op,List) && (op == NULL || IS_OP(getNthOfListP((List *) op, 0)))))

/* schema helper functions */
extern AttributeDef *createAttributeDef (char *name, DataType dt);
extern Schema *createSchema(char *name, List *attrDefs);
extern Schema *createSchemaFromLists (char *name, List *attrNames,
        List *dataTypes);
extern void addAttrToSchema(QueryOperator *op, char *name, DataType dt);
extern void deleteAttrFromSchemaByName(QueryOperator *op, char *name, boolean adaptProvAttrs);
extern void deleteAttrRefFromProjExprs(ProjectionOperator *op, int pos);
extern void setAttrDefDataTypeBasedOnBelowOp(QueryOperator *op1, QueryOperator *op2);
extern void reSetPosOfOpAttrRefBaseOnBelowLayerSchema(QueryOperator *op2, List *attrRefs);
extern void resetPosOfAttrRefBaseOnBelowLayerSchema(QueryOperator *parent, QueryOperator *child);

/* union equal element between two set list */
extern List *unionEqualElemOfTwoSetList(List *l1, List *l2);
extern List *addOneEqlOpAttrToListSet(Node *n1,Node *n2,List *listSet);
extern List *getCondOpList(List *l1, List *l2);
extern List *getDataTypes (Schema *schema);
extern List *getAttrNames(Schema *schema);
extern List *getAttrDefNames (List *defs);
extern List *getAttrDataTypes (List *defs);
extern List *inferOpResultDTs (QueryOperator *op);
#define GET_OPSCHEMA(o) ((QueryOperator *) o)->schema

/* shallow copy of a query operator (do not copy children or parents) */
extern QueryOperator *shallowCopyQueryOperator(QueryOperator *op);

/* create functions */
extern TableAccessOperator *createTableAccessOp(char *tableName, Node *asOf,
        char *alias, List *parents, List *attrNames, List *dataTypes);
extern SampleClauseOperator *createSampleClauseOp(QueryOperator *input, Node *sampPerc, List *attrNames, List *dataTypes);
extern JsonTableOperator *createJsonTableOperator(FromJsonTable *fjt);
extern SelectionOperator *createSelectionOp (Node *cond, QueryOperator *input,
        List *parents, List *attrNames);
extern ProjectionOperator *createProjectionOp (List *projExprs,
        QueryOperator *input, List *parents, List *attrNames);
extern JoinOperator *createJoinOp (JoinType joinType, Node *cond, List *inputs,
        List *parents, List *attrNames);
extern AggregationOperator *createAggregationOp (List *aggrs, List *groupBy,
        QueryOperator *input, List *parents, List *attrNames);
extern SetOperator *createSetOperator (SetOpType setOpType, List *inputs,
        List *parents, List *attrNames);
extern DuplicateRemoval *createDuplicateRemovalOp (List *attrs,
        QueryOperator *input, List *parents, List *attrNames);
extern ProvenanceComputation *createProvenanceComputOp(ProvenanceType provType,
        List *inputs, List *parents, List *attrNames, List *dts, Node *asOf);
extern ConstRelOperator *createConstRelOp(List *values,List *parents,
        List *attrNames, List *dataTypes);
extern NestingOperator *createNestingOp(NestingExprType nestingType, Node *cond,
        List *inputs, List *parents, List *attrNames, List *dts);
extern WindowOperator *createWindowOp(Node *fCall, List *partitionBy,
        List *orderBy, WindowFrame *frameDef, char *attrName,
        QueryOperator *input, List *parents);
extern OrderOperator *createOrderOp(List *orderExprs, QueryOperator *input,
        List *parents);
extern LimitOperator *createLimitOp(Node *limitExpr, Node *offsetExpr, QueryOperator *input, List *parents);

/* navigation functions */
#define OP_LCHILD(op) \
    ((QueryOperator *) getHeadOfListP(((QueryOperator*) op)->inputs))

#define OP_RCHILD(op) \
    ((QueryOperator *) getNthOfListP(((QueryOperator*) op)->inputs,1))

#define OP_FIRST_PARENT(op) \
    ((QueryOperator *) getHeadOfListP(((QueryOperator*) op)->parents))

#define getAttrDef(op,aPos) \
    ((AttributeDef *) getNthOfListP(((QueryOperator *) op)->schema->attrDefs, aPos))

extern QueryOperator *getFirstRoot(QueryOperator *op);

/* deal with properties */
extern void setProperty(QueryOperator *op, Node *key, Node *value);
extern Node *getProperty(QueryOperator *op, Node *key);
extern void setStringProperty(QueryOperator *op, char *key, Node *value);
extern Node *getStringProperty(QueryOperator *op, char *key);
extern void removeStringProperty(QueryOperator *op, char *key);
extern List *appendToListProperty(QueryOperator *op, Node *key, Node *newTail);
extern List *appendToListStringProperty(QueryOperator *op, char *key, Node *newTail);
extern char *format_prop_value_for_user(char *prop, Node *val);
extern char *format_op_prop_value_for_user(QueryOperator *op, char *prop);



#define SET_KEYVAL_PROPERTY(op,kv) (setProperty(((QueryOperator *) op), kv->key, kv->value))
#define HAS_PROP(op,key) (getProperty(((QueryOperator *) op),key) != NULL)
#define HAS_STRING_PROP(op,key) (getStringProperty((QueryOperator *) op, key) != NULL)
#define SET_STRING_PROP(op,key,value) (setStringProperty((QueryOperator *) op, \
        key, (Node *) value))
#define SET_BOOL_STRING_PROP(op,key) (setStringProperty((QueryOperator *) op, \
        key, (Node *) createConstBool(TRUE)))
#define APPEND_PROP(op,key,value) (appendToListProperty((QueryOperator *) op, \
														key,			\
														(Node *) value))
#define APPEND_STRING_PROP(op,key,value) (appendToListStringProperty((QueryOperator *) op, \
																	 key, \
																	 (Node *) value))
#define GET_STRING_PROP(op,key) (getStringProperty((QueryOperator *) op, key))
#define GET_STRING_PROP_STRING_VAL(op,key) (HAS_STRING_PROP(op,key) ? STRING_VALUE(getStringProperty((QueryOperator *) op, key)) : NULL)
#define GET_BOOL_STRING_PROP(op,key) ((getStringProperty((QueryOperator *) op, key) != NULL) \
    && (BOOL_VALUE(getStringProperty((QueryOperator *) op, key))))

/* children and parents */
extern void addChildOperator (QueryOperator *parent, QueryOperator *child);
extern void addParent (QueryOperator *child, QueryOperator *parent);
extern void removeParent (QueryOperator *child, QueryOperator *parent);
extern int getChildPosInParent(QueryOperator *parent, QueryOperator *child);

/* attribute functions */
extern List *getProvenanceAttrs(QueryOperator *op);
extern List *getProvenanceAttrDefs(QueryOperator *op);
extern List *getProvenanceAttrReferences(ProjectionOperator *op, QueryOperator *op1);
extern List *getOpProvenanceAttrNames(QueryOperator *op);
extern int getNumProvAttrs(QueryOperator *op);

extern List *getNormalAttrs(QueryOperator *op);
extern List *getNormalAttrProjExprs(ProjectionOperator *op);
extern List *getNormalAttrProjExprsFromChild(ProjectionOperator *parent, QueryOperator *child);
extern List *getNormalAttrNames(QueryOperator *op);
extern List *getAttrRefNames(ProjectionOperator *op);
extern List *getAttrNameFromOpExpList(List *aNameOpList, Operator *opExpList);
extern List *getAttrRefNamesContainOps(ProjectionOperator *op);
extern int getNumNormalAttrs(QueryOperator *op);

extern List *getQueryOperatorAttrNames (QueryOperator *op);

extern int getNumAttrs(QueryOperator *op);

extern int getAttrPos(QueryOperator *op, char *attr);
extern AttributeDef *getAttrDefByName(QueryOperator *op, char *attr);
extern AttributeDef *getAttrDefByPos(QueryOperator *op, int pos);
extern AttributeReference *getAttrRefByPos (QueryOperator *op, int pos);
extern AttributeReference *getAttrRefByName(QueryOperator *op, char *attr);
extern char *getAttrNameByPos(QueryOperator *op, int pos);

extern List *getAttrRefsInOperator(QueryOperator *op);
extern boolean opReferencesAttr(QueryOperator *op, char *a);

/* operator specific functions */
extern List *aggOpGetGroupByAttrNames(AggregationOperator *op);
extern List *aggOpGetAggAttrNames(AggregationOperator *op);
extern List *aggOpGetGroupByAttrDefs(AggregationOperator *op);
extern List *aggOpGetAggAttrDefs(AggregationOperator *op);

extern WindowFunction *winOpGetFunc(WindowOperator *op);

extern HashMap *joinGetChildAttrToResultAttr(JoinOperator *op, boolean left);

extern List *getProjExprsForAttrNames(QueryOperator *op, List *names);
extern List *getProjExprsForAllAttrs(QueryOperator *op);
extern List *getProjResultAttrNamesForProjExpr(ProjectionOperator *op, Node *expr);

extern List *getNestingResultAttributeNames(NestingOperator *op);
extern char *getSingleNestingResultAttribute(NestingOperator *op);
extern Set *getNestingCorrelatedAttributes(NestingOperator *op, boolean corrInSubquery);
extern Set *getCorrelatedAttributes(Node *op, boolean corrInSubquery);

/* change operator schema and adapt references in parents */
extern void removeAttrFromOp(QueryOperator *op, char *attr);

/* transforms a graph query model into a tree */
extern void treeify(QueryOperator *op);

/* visit a query operator graph in a specified order */
NEW_ENUM_WITH_TO_STRING(TraversalOrder,
        TRAVERSAL_PRE,
        TRAVERSAL_POST);
extern boolean visitQOGraph (QueryOperator *q, TraversalOrder tOrder,
        boolean (*visitF) (QueryOperator *op, void *context),
        void *context);
extern unsigned int numOpsInGraph (QueryOperator *root);
extern unsigned int numOpsInTree (QueryOperator *root);

//find NestingOperator based on levelsUp
extern QueryOperator*findNestingOperator(QueryOperator *op, int levelsUp);
extern char *getNestingAttrPrefix();
extern char *getNestingResultAttribute(int number);
extern boolean isNestingAttribute(char *attr);

#endif /* QUERY_OPERATOR_H_ */
