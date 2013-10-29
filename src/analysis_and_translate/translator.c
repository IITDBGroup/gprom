/*-------------------------------------------------------------------------
 *
 * translator.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "analysis_and_translate/translator.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include <assert.h>

static QueryOperator *translateQuery (Node *node);

/* Three branches of translating a Query */
static QueryOperator *translateSetQuery(SetQuery *sq);
static QueryOperator *translateQueryBlock(QueryBlock *qb);
static QueryOperator *translateProvenanceStmt(ProvenanceStmt *prov);

/* Functions of translating from clause in a QueryBlock */
static QueryOperator *translateFromClause(List *fromClause);
static QueryOperator *buildJoinTreeFromOperatorList(List *opList);
static List *translateFromClauseToOperatorList(List *fromClause);
static int *getAttrsOffsets(List *fromClause);
static inline QueryOperator *createTableAccessOpFromFromTableRef(FromTableRef *ftr);
static QueryOperator *translateFromJoinExpr(FromJoinExpr *fje);
static QueryOperator *translateFromSubquery(FromSubquery *fsq);

/* Functions of translating where clause in a QueryBlock */
static QueryOperator *translateWhereClause(Node *whereClause, QueryOperator *joinTreeRoot, int *attrsOffsets);
static boolean visitAttrRefToSetNewAttrPos(Node *n, void *state);


QueryOperator *
translateParse(Node *q)
{
    //TODO create and destroy memory context - don't forget to copy result to callers context
    //TODO call analysis function first
    return translateQuery(q);
}

static QueryOperator *
translateQuery (Node *node)
{
    switch(node->type)
    {
        case T_QueryBlock:
            return translateQueryBlock((QueryBlock *) node);
        case T_SetQuery:
            return translateSetQuery((SetQuery *) node);
        case T_ProvenanceStmt:
            return translateProvenanceStmt((ProvenanceStmt *) node);
        default:
            assert(FALSE);
            return NULL;
    }
}

static QueryOperator *
translateSetQuery(SetQuery *sq)
{
    QueryOperator *left = NULL;
    QueryOperator *right = NULL;
    if (sq->lChild)
    {
        if (sq->lChild->type == T_SetQuery)
            left = translateSetQuery((SetQuery *) sq->lChild);
        else if (sq->lChild->type == T_QueryBlock)
            left = translateQueryBlock(((QueryBlock *) sq->lChild));
    }
    if (sq->rChild)
    {
        if (sq->rChild->type == T_SetQuery)
            right = translateSetQuery((SetQuery *) sq->rChild);
        else if (sq->rChild->type == T_QueryBlock)
            right = translateQueryBlock(((QueryBlock *) sq->rChild));
    }
    assert(left && right);
    List *inputs = appendToTailOfList(inputs, left);
    inputs = appendToTailOfList(inputs, right);
    // set children of the set operator node

    SetOperator *so = createSetOperator(sq->setOp, inputs, NIL, sq->selectClause);
    // create set operator node

    OP_LCHILD(so)->parents = OP_RCHILD(so)->parents = singleton(so);
    // set the parent of the node's children

    return ((QueryOperator *) so);
}

static QueryOperator *
translateQueryBlock(QueryBlock *qb)
{
    QueryOperator *joinTreeRoot = translateFromClause(qb->fromClause);
    int *attrsOffsets = getAttrsOffsets(qb->fromClause);
    QueryOperator *select = translateWhereClause(qb->whereClause, joinTreeRoot, attrsOffsets);
    // TODO translate selection clause
    return NULL;
}

static QueryOperator *
translateProvenanceStmt(ProvenanceStmt *prov)
{
    QueryOperator *child;
    ProvenanceComputation *result;
    List *attrs = NIL;
    Schema *schema = NULL;
    //TODO create attribute list by analyzing subquery under child
    child = translateQuery(prov->query);

    result = createProvenanceComputOp(PI_CS, singleton(child), NIL, NIL, attrs); //TODO adapt function parameters

    child->parents = singleton(result);

    return (QueryOperator *) result;
}

static QueryOperator *
translateFromClause(List *fromClause)
{
    List *opList = translateFromClauseToOperatorList(fromClause);
    return buildJoinTreeFromOperatorList(opList);
}

static QueryOperator *
buildJoinTreeFromOperatorList(List *opList)
{
    QueryOperator *root = (QueryOperator *) LC_P_VAL(getHeadOfList(opList));
    FOREACH(QueryOperator, op, opList)
    {
        if (op == (QueryOperator *) LC_P_VAL(getHeadOfList(opList)))
            continue;

        QueryOperator *oldRoot = (QueryOperator *) root;
        List *inputs = appendToTailOfList(inputs, oldRoot);
        inputs = appendToTailOfList(inputs, op);
        // set children of the join node

        List *attrNames = concatTwoLists(getAttrNames(oldRoot->schema), getAttrNames(op->schema));
        // contact children's attribute names as the node's attribute names

        root = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
        // create join operator

        OP_LCHILD(root)->parents = OP_RCHILD(root)->parents = singleton(root);
        // set the parent of the node's children
    }
    return root;
}

static int *
getAttrsOffsets(List *fromClause)
{
    int len = getListLength(fromClause);
    int *offsets = CNEW(int, len);
    offsets[0] = 0;
    int i = 0;
    FOREACH(FromItem, from, fromClause)
    {
        i++;
        if (i < len)
            offsets[i] = offsets[i - 1] + getListLength(from->attrNames);
    }
    return offsets;
}

static List *
translateFromClauseToOperatorList(List *fromClause)
{
    List *opList = NIL;

    FOREACH(FromItem, from, fromClause)
    {
        QueryOperator *op = NULL;
        switch (from->type)
        {
            case T_FromTableRef:
                op = createTableAccessOpFromFromTableRef((FromTableRef *) from);
                break;
            case T_FromJoinExpr:
                op = translateFromJoinExpr((FromJoinExpr *) from);
                break;
            case T_FromSubquery:
                op = translateFromSubquery((FromSubquery *) from);
                break;
        }

        assert(op);
        opList = appendToTailOfList(opList, op);
    }

    assert(opList);
    return opList;
}

static inline QueryOperator *
createTableAccessOpFromFromTableRef(FromTableRef *ftr)
{
    TableAccessOperator *ta = createTableAccessOp(ftr->tableId, ftr->from.name,
    NIL, ftr->from.attrNames, NIL); // TODO  get data types
    return ((QueryOperator *) ta);
}

static QueryOperator *
translateFromJoinExpr(FromJoinExpr *fje)
{
    QueryOperator *input1 = NULL;
    QueryOperator *input2 = NULL;
    switch (fje->left->type)
    {
        case T_FromTableRef:
            input1 = createTableAccessOpFromFromTableRef(
                    (FromTableRef *) fje->left);
            break;
        case T_FromJoinExpr:
            input1 = translateFromJoinExpr((FromJoinExpr *) fje->left);
            break;
        case T_FromSubquery:
            input1 = translateFromSubquery((FromSubquery *) fje->left);
            break;
    }
    switch (fje->right->type)
    {
        case T_FromTableRef:
            input2 = createTableAccessOpFromFromTableRef(
                    (FromTableRef *) fje->right);
            break;
        case T_FromJoinExpr:
            input2 = translateFromJoinExpr((FromJoinExpr *) fje->right);
            break;
        case T_FromSubquery:
            input2 = translateFromSubquery((FromSubquery *) fje->right);
            break;
    }

    assert(input1 && input2);
    List *inputs = appendToTailOfList(inputs, input1);
    inputs = appendToTailOfList(inputs, input2);
    // set children of the join operator node

    JoinOperator *jo = createJoinOp(fje->joinType, fje->cond, inputs, NIL,
            fje->from.attrNames); // TODO compute attrNames?
    // create join operator node

    // TODO create selection/projection upon join according to join condition

    OP_LCHILD(jo)->parents = OP_RCHILD(jo)->parents = singleton(jo);
    // set the parent of the node's children

    return ((QueryOperator *) jo);
}

static QueryOperator *
translateFromSubquery(FromSubquery *fsq)
{
    return translateQuery(fsq->subquery);
    //TODO set attr names from FromItem
}

static QueryOperator *
translateWhereClause(Node *whereClause, QueryOperator *joinTreeRoot, int *attrsOffsets)
{
    if (whereClause == NULL)
        return NULL;

    QueryOperator *input = joinTreeRoot;
    SelectionOperator *so = createSelectionOp (whereClause, input, NIL, getAttrNames(input->schema));
    // create selection operator node upon the root of the join tree

    visitAttrRefToSetNewAttrPos(so->cond, attrsOffsets);
    // change attributes positions in selection condition

    OP_LCHILD(so)->parents = singleton(so);
    // set the parent of the node's children

    return so;
}

static boolean
visitAttrRefToSetNewAttrPos(Node *n, void *state)
{
    if (n == NULL)
        return TRUE;

    if (n->type == T_AttributeReference)
    {
        AttributeReference *attrRef = (AttributeReference *) n;
        int *offsets = (int *) state;
        attrRef->attrPosition += offsets[attrRef->fromClauseItem];
        attrRef->fromClauseItem = 0;
    }

    return visit(n, visitAttrRefToSetNewAttrPos, state);
}

