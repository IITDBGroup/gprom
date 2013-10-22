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
    List *provAttrs; // provenance attributes, AttributeReference type
} QueryOperator; // common fields that all operators have

typedef struct TableAccessOperator
{
    QueryOperator op;
    char *tableName;
} TableAccessOperator;

typedef struct SelectionOperator
{
    QueryOperator op;
    Node *cond; // condition expression, Expr type
} SelectionOperator;

typedef struct ProjectionOperator
{
    QueryOperator op;
    List *projExprs; // projection expressions, Expression type
} ProjectionOperator;

typedef struct JoinOperator
{
    QueryOperator op;
    JoinType joinType;
    Node *cond; // join condition expression, Operator type
} JoinOperator;

typedef struct AggregationOperator
{
    QueryOperator op;
    List *aggrs; // aggregation expressions, FunctionCall type
    List *groupBy; // group by expressions, AttributeReference type
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

typedef enum ProvenanceType
{
    PI_CS,
    TRANSFORMATION
} ProvenanceType;

typedef struct ProvenanceComputation
{
    QueryOperator op;
    ProvenanceType provType;
} ProvenanceComputation;

/* schema helper functions */
extern Schema *createSchema(char *name, List *attrDefs);
extern Schema *createSchemaFromLists (char *name, List *attrNames, List *dataTypes);
extern List *getDataTypes (Schema *schema);
extern List *getAttrNames(Schema *schema);

/* create functions */
extern TableAccessOperator *createTableAccessOp (char *tableName, char *alias, List *parents, List *attrNames, List *dataTypes);
extern SelectionOperator *createSelectionOp (Node *cond, QueryOperator *input, List *parents, List *attrNames);
extern ProjectionOperator *createProjectionOp (List *projExprs, QueryOperator *input, List *parents, List *attrNames);
extern JoinOperator *createJoinOp (JoinType joinType, Node *cond, List *inputs, List *parents, List *attrNames);
extern AggregationOperator *createAggregationOp (List *aggrs, List *groupBy, QueryOperator *input, List *parents, List *attrNames);
extern SetOperator *createSetOperator (SetOpType setOpType, List *inputs, List *parents, List *attrNames);
extern DuplicateRemoval *createDuplicateRemovalOp (List *attrs, QueryOperator *input, List *parents, List *attrNames);
extern ProvenanceComputation *createProvenanceComputOp(ProvenanceType provType, List *inputs, List *schema, List *parents, List *attrNames);

/* navigation functions */
#define OP_LCHILD(op) \
    ((QueryOperator *) ((QueryOperator*) op)->inputs->head->data.ptr_value)

#define OP_RCHILD(op) \
    ((QueryOperator *) ((QueryOperator*) op)->inputs->head->next->data.ptr_value)

#define _OP_LCHILD(op) \
	((QueryOperator*) op)->inputs->head->data.ptr_value

#define _OP_RCHILD(op) \
	((QueryOperator*) op)->inputs->head->next->data.ptr_value

/* access functions */
extern List *getProvenanceAttrs(QueryOperator *op);
extern List *getNormalAttrs(QueryOperator *op);

#endif /* QUERY_OPERATOR_H_ */
