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
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"

typedef struct AttributeDef
{
    NodeTag type;
    DataType dataType;
    char *attrName;
    int pos;
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

/* type of operator macros */
#define IS_NULLARY_OP(op) (isA(op, TableAccessOperator) \
                        || isA(op, ConstRelOperator))

#define IS_UNARY_OP(op) (isA(op,ProjectionOperator)     \
        || isA(op,SelectionOperator)                    \
        || isA(op,AggregationOperator)                  \
        || isA(op,DuplicateRemoval)                     \
        || isA(op,WindowOperator))

#define IS_BINARY_OP(op) (isA(op,JoinOperator)          \
        || isA(op,SetOperator)                          \
        || isA(op,NestingOperator))

/* schema helper functions */
extern AttributeDef *createAttributeDef (char *name, DataType dt);
extern Schema *createSchema(char *name, List *attrDefs);
extern Schema *createSchemaFromLists (char *name, List *attrNames,
        List *dataTypes);
extern List *getDataTypes (Schema *schema);
extern List *getAttrNames(Schema *schema);
#define GET_OPSCHEMA(o) ((QueryOperator *) o)->schema

/* create functions */
extern TableAccessOperator *createTableAccessOp(char *tableName, Node *asOf,
        char *alias, List *parents, List *attrNames, List *dataTypes);
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
        List *inputs, List *parents, List *attrNames, Node *asOf);
extern ConstRelOperator *createConstRelOp(List *values,List *parents,
        List *attrNames, List *dataTypes);
extern NestingOperator *createNestingOp(NestingExprType nestingType, Node *cond,
        List *inputs, List *parents, List *attrNames);
extern WindowOperator *createWindowOp(FunctionCall *fCall, List *partitionBy,
        List *orderBy, WindowFrame *frameDef, char *attrName,
        QueryOperator *input, List *parents);

/* navigation functions */
#define OP_LCHILD(op) \
    ((QueryOperator *) getHeadOfListP(((QueryOperator*) op)->inputs))

#define OP_RCHILD(op) \
    ((QueryOperator *) getNthOfListP(((QueryOperator*) op)->inputs,1))

#define getAttrDef(op,aPos) \
    ((AttributeDef *) getNthOfListP(((QueryOperator *) op)->schema->attrDefs, aPos))

/* deal with properties */
extern void setProperty (QueryOperator *op, Node *key, Node *value);
extern Node *getProperty (QueryOperator *op, Node *key);
extern void setStringProperty (QueryOperator *op, char *key, Node *value);
extern Node *getStringProperty (QueryOperator *op, char *key);
#define HAS_PROP(op,key) (getProperty(((QueryOperator *) op),key) != NULL)
#define HAS_STRING_PROP(op,key) (getStringProperty((QueryOperator *) op, key) != NULL)
#define SET_BOOL_STRING_PROP(op,key) (setStringProperty((QueryOperator *) op, \
        key, (Node *) createConstBool(TRUE)))
#define GET_STRING_PROP(op,key) (getStringProperty((QueryOperator *) op, key))

/* add children and parents */
extern void addChildOperator (QueryOperator *parent, QueryOperator *child);
extern void addParent (QueryOperator *child, QueryOperator *parent);

/* attribute functions */
extern List *getProvenanceAttrs(QueryOperator *op);
extern List *getProvenanceAttrDefs(QueryOperator *op);
extern List *getOpProvenanceAttrNames(QueryOperator *op);
extern int getNumProvAttrs(QueryOperator *op);

extern List *getNormalAttrs(QueryOperator *op);
extern List *getNormalAttrNames(QueryOperator *op);
extern int getNumNormalAttrs(QueryOperator *op);

extern List *getQueryOperatorAttrNames (QueryOperator *op);

extern int getNumAttrs(QueryOperator *op);

extern int getAttrPos(QueryOperator *op, char *attr);

/* operator specific functions */
extern List *aggOpGetGroupByAttrNames(AggregationOperator *op);
extern List *aggOpGetAggAttrNames(AggregationOperator *op);

/* transforms a graph query model into a tree */
extern void treeify(QueryOperator *op);

#endif /* QUERY_OPERATOR_H_ */
