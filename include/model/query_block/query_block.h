/*************************************************************************
	> File Name: query_block.h
	> Author: Shukun Xie
    > Descriptions: Head file for query_block
 ************************************************************************/

#ifndef QUERY_BLOCK_H
#define QUERY_BLOCK_H

#include "model/list/list.h"
#include "model/node/nodetype.h"

typedef enum SetOpType
{
    UNIONOP,
    INTERSECTION,
    DIFFERENCE
} SetOpType;

typedef struct SetOp
{
    NodeTag type;
    SetOpType setOp;
    boolean all;
    Node *lChild; // either SetOp or QueryBlock
    Node *rChild; // either SetOp or QueryBlock
} SetOp;

typedef struct SetQuery
{
    NodeTag type;
    List *selectClause;
    SetOp *rootSetOp;
} SetQuery;


typedef struct QueryBlock
{
    NodeTag type;
    List *selectClause;
    Node *distinct;
    List *fromClause;
    Node *whereClause;
    Node *havingClause;
} QueryBlock;

typedef struct ProvenanceStmt
{
    NodeTag type;
    Node *query;
} ProvenanceStmt;

typedef struct SelectItem
{
    NodeTag type;
    char *alias;
    Node *expr;
} SelectItem;

typedef struct FromItem
{
    NodeTag type;
    char *name;
    List *attrNames;
} FromItem;

typedef struct FromTableRef
{
    FromItem from;
    char *tableId;
} FromTableRef;

typedef struct FromSubquery
{
    FromItem from;
    Node *subquery;
} FromSubquery;

typedef enum JoinType
{
    JOIN_INNER,
    JOIN_CROSS,
    JOIN_LEFT_OUTER,
    JOIN_RIGHT_OUTER,
    JOIN_FULL_OUTER
} JoinType;

typedef enum JoinConditionType
{
    JOIN_COND_ON,
    JOIN_COND_USING,
    JOIN_COND_NATURAL
} JoinConditionType;

typedef struct FromJoinExpr
{
    FromItem from;
    FromItem *left;
    FromItem *right;
    JoinType joinType;
    JoinConditionType joinCond;
    Node *cond;
} FromJoinExpr;

typedef struct DistinctClause
{
    List *distinctExprs;
} DistinctClause;


/* functions for creating query block nodes */
extern SetQuery *createSetQuery(List *selectClause, SetOp *root);
extern SetOp *createSetOp(SetOpType opType, boolean all, Node *lChild,
        Node *rChild);
extern QueryBlock *createQueryBlock(void);
extern ProvenanceStmt *createProvenanceStmt(Node *query);
extern SelectItem *createSelectItem(char *alias, Node *expr);
extern FromItem *createFromTableRef(char *alias, List *attrNames,
        char *tableId);
extern FromItem *createFromSubquery(char *alias, List *attrNames, Node *query);
extern FromItem *createFromJoin(char *alias, List *attrNames, FromItem *left,
        FromItem *right, JoinType joinType, JoinConditionType condType,
        Node *cond);
extern DistinctClause *createDistinctClause (List *distinctExprs);

#endif /* QUERY_BLOCK_H */
