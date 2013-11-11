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

#include "common.h"

#include "mem_manager/mem_mgr.h"

#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

#include "analysis_and_translate/analyze_qb.h"
#include "analysis_and_translate/translator.h"


static QueryOperator *translateQuery (Node *node);

/* Three branches of translating a Query */
static QueryOperator *translateSetQuery(SetQuery *sq);
static QueryOperator *translateQueryBlock(QueryBlock *qb);
static QueryOperator *translateProvenanceStmt(ProvenanceStmt *prov);

/* Functions of translating from clause in a QueryBlock */
static QueryOperator *translateFromClause(List *fromClause);
static QueryOperator *buildJoinTreeFromOperatorList(List *opList);
static List *translateFromClauseToOperatorList(List *fromClause);
static List *getAttrsOffsets(List *fromClause);
static inline QueryOperator *createTableAccessOpFromFromTableRef(FromTableRef *ftr);
static QueryOperator *translateFromJoinExpr(FromJoinExpr *fje);
static QueryOperator *translateFromSubquery(FromSubquery *fsq);

/* Functions of translating where clause in a QueryBlock */
static QueryOperator *translateWhereClause(Node *whereClause, QueryOperator *joinTreeRoot, List *attrsOffsets);
static boolean visitAttrRefToSetNewAttrPos(Node *n, List *state);

/* Functions of translating select clause in a QueryBlock */
static QueryOperator *translateSelectClause(List *selectClause, QueryOperator *select, List *attrsOffsets);
static boolean visitAttrRefToGetAttrNames(Node *n, void *state);


Node *
translateParse(Node *q)
{
    Node *result;
    NEW_AND_ACQUIRE_MEMCONTEXT("TRANSLATOR_CONTEXT");
    analyzeQueryBlockStmt(q);

    INFO_LOG("translate QB model \n%s", nodeToString(q));

    if (isA(q, List))
    {
        result = (Node *) copyList((List *) q);
        FOREACH(Node,stmt,(List *) result)
            stmt_his_cell->data.ptr_value = (Node *) translateQuery(stmt);
    }
    else
        result = (Node *) translateQuery (q);

    INFO_LOG("result of translation is \n%s", beatify(nodeToString(result)));
    assert(equal(result, copyObject(result)));

    FREE_MEM_CONTEXT_AND_RETURN_COPY(Node,result);
}

static QueryOperator *
translateQuery (Node *node)
{
    DEBUG_LOG("translate query <%s>", node);

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
    // set the parent of the operator's children

    return ((QueryOperator *) so);
}

static QueryOperator *
translateQueryBlock(QueryBlock *qb)
{
    List *attrsOffsets = NIL;

    INFO_LOG("translate a QB:\n%s", nodeToString(qb));

    QueryOperator *joinTreeRoot = translateFromClause(qb->fromClause);
    INFO_LOG("translatedFrom is\n%s", nodeToString(joinTreeRoot));

    attrsOffsets = getAttrsOffsets(qb->fromClause);
    QueryOperator *select = translateWhereClause(qb->whereClause, joinTreeRoot, attrsOffsets);
    INFO_LOG("translatedWhere is\n%s", nodeToString(select));

    QueryOperator *project = translateSelectClause(qb->selectClause, select, attrsOffsets);
    INFO_LOG("translatedSelect is\n%s", nodeToString(project));

    // TODO translate aggregation
    return project;
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
    DEBUG_LOG("build join tree from operator list\n%s", nodeToString(opList));

    QueryOperator *root = (QueryOperator *) getHeadOfListP(opList);
    FOREACH(QueryOperator, op, opList)
    {
        if (op == (QueryOperator *) getHeadOfListP(opList))
            continue;

        QueryOperator *oldRoot = (QueryOperator *) root;
        List *inputs = NIL;
        inputs = appendToTailOfList(inputs, oldRoot);
        inputs = appendToTailOfList(inputs, op);
        // set children of the join node

        List *attrNames = concatTwoLists(getAttrNames(oldRoot->schema), getAttrNames(op->schema));
        // contact children's attribute names as the node's attribute names

        root = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
        // create join operator

        OP_LCHILD(root)->parents = OP_RCHILD(root)->parents = singleton(root);
        // set the parent of the operator's children
    }

    DEBUG_LOG("join tree for translated from is\n%s", nodeToString(root));

    return root;
}

static List *
getAttrsOffsets(List *fromClause)
{
    int len = getListLength(fromClause);
    List *offsets = NIL;
    int curOffset = 0;

    FOREACH(FromItem, from, fromClause)
    {
       offsets = appendToTailOfListInt(offsets, curOffset);
       curOffset += getListLength(from->attrNames);
    }

    DEBUG_LOG("attribute offsets for from clause items are %s", offsets);

    return offsets;
}

static List *
translateFromClauseToOperatorList(List *fromClause)
{
    List *opList = NIL;

    DEBUG_LOG("translate from clause");

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
            default:
                FATAL_LOG("did not expect node <%s> in from list", nodeToString(from));
                break;
        }

        assert(op);
        opList = appendToTailOfList(opList, op);
    }

    assert(opList);
    DEBUG_LOG("translated from clause into list of operator trees is \n%s", nodeToString(opList));
    return opList;
}

static inline QueryOperator *
createTableAccessOpFromFromTableRef(FromTableRef *ftr)
{
    TableAccessOperator *ta = createTableAccessOp(ftr->tableId, ftr->from.name,
                NIL, ftr->from.attrNames, NIL); // TODO  get data types
    DEBUG_LOG("translated table access:\n%s\nINTO\n%s", nodeToString(ftr), nodeToString(ta));
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
        default:
            FATAL_LOG("did not expect node <%s> in from list", nodeToString(input1));
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
        default:
            FATAL_LOG("did not expect node <%s> in from list", nodeToString(input2));
            break;
    }

    assert(input1 && input2);
    List *inputs = appendToTailOfList(inputs, input1);
    inputs = appendToTailOfList(inputs, input2);
    // set children of the join operator node

    JoinOperator *jo = createJoinOp(fje->joinType, fje->cond, inputs, NIL,
            fje->from.attrNames); // TODO merge schema?
    // create join operator node

    OP_LCHILD(jo)->parents = OP_RCHILD(jo)->parents = singleton(jo);
    // set the parent of the operator's children

    if (fje->joinCond == JOIN_COND_NATURAL)
    {
        // TODO create projection?
    }

    return ((QueryOperator *) jo);
}

static QueryOperator *
translateFromSubquery(FromSubquery *fsq)
{
    return translateQuery(fsq->subquery);
    //TODO set attr names from FromItem
}

static QueryOperator *
translateWhereClause(Node *whereClause, QueryOperator *joinTreeRoot, List *attrsOffsets)
{
    if (whereClause == NULL)
        return joinTreeRoot;

    SelectionOperator *so = createSelectionOp (whereClause, joinTreeRoot, NIL, getAttrNames(joinTreeRoot->schema));
    // create selection operator node upon the root of the join tree

    visitAttrRefToSetNewAttrPos(so->cond, attrsOffsets);
    // change attributes positions in selection condition

    OP_LCHILD(so)->parents = singleton(so);
    // set the parent of the operator's children

    return ((QueryOperator *) so);
}

static boolean
visitAttrRefToSetNewAttrPos(Node *n, List *state)
{
    if (n == NULL)
        return TRUE;

    int *offsets = (int *) state;
    if (n->type == T_AttributeReference)
    {
        AttributeReference *attrRef = (AttributeReference *) n;
        attrRef->attrPosition += getNthOfListInt(state, attrRef->fromClauseItem);
        attrRef->fromClauseItem = 0;
    }

    return visit(n, visitAttrRefToSetNewAttrPos, offsets);
}

static QueryOperator *
translateSelectClause(List *selectClause, QueryOperator *select,
        List *attrsOffsets)
{
    List *attrNames = NIL;
    List *projExprs = NIL;

    // determine projection expressions and attribute names
    // visit each expression in select clause to get attribute names
    FOREACH(SelectItem, s, selectClause)
    {
        Node *projExpr = copyObject(s->expr);
        visitAttrRefToGetAttrNames(projExpr, attrNames);
        projExprs = appendToTailOfList(projExprs, projExpr);
    }

    ProjectionOperator *po = createProjectionOp(selectClause, select, NIL,
            attrNames);
    // create projection operator upon selection operator from select clause

    FOREACH(Node, expr, po->projExprs)
    {
        visitAttrRefToSetNewAttrPos(expr, attrsOffsets);
        // change attribute position in attribute reference in each projection expression
    }

    OP_LCHILD(po)->parents = singleton(po);
    // set the parent of the operator's children

    return ((QueryOperator *) po);
}

static boolean
visitAttrRefToGetAttrNames(Node *n, void *state)
{
    if (n == NULL)
        return TRUE;

    List *attrNames = (List *) state;
    if (n->type == T_AttributeReference)
    {
        AttributeReference *attrRef = (AttributeReference *) n;
        attrNames = appendToTailOfList(attrNames, attrRef->name);
    }

    return visit(n, visitAttrRefToGetAttrNames, attrNames);
}
