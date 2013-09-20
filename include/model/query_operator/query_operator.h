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

typedef struct QueryOperator
{
    NodeTag type;
    List *inputs;
} QueryOperator;

typedef struct SelectionOperator
{
    QueryOperator op;
    Node *cond;
} SelectionOperator;

typedef struct ProjectionOperator
{
    QueryOperator op;
    Node *cond;
} ProjectionOperator;

typedef struct JoinOperator
{
    QueryOperator op;
    Node *cond;
} JoinOperator;

typedef struct AggregationOperator
{
    QueryOperator op;
    List *aggs;
    List *groupBy;
} AggregationOperator;


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

/* create functions */

/* navigation functions */
#define OP_LCHILD(op) \
    ((QueryOperator *) ((QueryOperator*) op)->inputs->head->data.ptr_value)

#define OP_RCHILD(op) \
    ((QueryOperator *) ((QueryOperator*) op)->inputs->head->next->data.ptr_value)

/* access functions */
extern List *getProvenanceAttrs(QueryOperator *op);
extern List *getNormalAttrs(QueryOperator *op);

#endif /* QUERY_OPERATOR_H_ */
