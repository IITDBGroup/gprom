/*-----------------------------------------------------------------------------
 *
 * pi_cs_composable.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"

#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/prov_schema.h"
#include "provenance_rewriter/prov_utility.h"

// result tuple-id attribute and provenance duplicate counter attribute
#define RESULT_TID_ATTR "_result_tid"
#define PROV_DUPL_COUNT_ATTR "_prov_dup_count"

// data structures
static Node *asOf;
static RelCount *nameState;

// static methods
static boolean isTupleAtATimeSubtree(QueryOperator *op);

static QueryOperator *rewritePI_CSComposableOperator (QueryOperator *op);
static QueryOperator *rewritePI_CSComposableSelection (SelectionOperator *op);
static QueryOperator *rewritePI_CSComposableProjection (ProjectionOperator *op);
static QueryOperator *rewritePI_CSComposableJoin (JoinOperator *op);
static QueryOperator *rewritePI_CSComposableAggregation (AggregationOperator *op);
static QueryOperator *rewritePI_CSComposableSet (SetOperator *op);
static QueryOperator *rewritePI_CSComposableTableAccess(TableAccessOperator *op);
static QueryOperator *rewritePI_CSComposableConstRel(ConstRelOperator *op);
static QueryOperator *rewritePI_CSComposableDuplicateRemOp(DuplicateRemoval *op);

static void addResultTIDAndProvDupAttrs (QueryOperator *op, boolean addToSchema);
static Node *replaceAttrWithCaseForProvDupRemoval (FunctionCall *f, Node *provDupAttrRef);


/*
 *
 */
QueryOperator *
rewritePI_CSComposable (ProvenanceComputation *op)
{
    QueryOperator *rewRoot;

    rewRoot = OP_LCHILD(op);
    rewRoot = rewritePI_CSComposableOperator(rewRoot);

    return (QueryOperator *) rewRoot;
}

/*
 * Figure out whether for a certain operator there will be at most one provenance tuple
 * per original result tuple. This can be used to use ROWNUM instead of window functions.
 * Store result as property to avoid recomputation.
 */
static boolean
isTupleAtATimeSubtree(QueryOperator *op)
{
    if (HAS_STRING_PROP(op, "PROVENANCE_OPERATOR_TUPLE_AT_A_TIME"))
        return TRUE;

    switch(op->type)
    {
        case T_SelectionOperator:
        case T_ProjectionOperator:
        case T_JoinOperator:
            break;
        default:
            return FALSE;
    }

    FOREACH(QueryOperator,child,op->inputs)
        if (!isTupleAtATimeSubtree(child))
            return FALSE;

    SET_BOOL_STRING_PROP(op,"PROVENANCE_OPERATOR_TUPLE_AT_A_TIME");
    return TRUE;
}

static QueryOperator *
rewritePI_CSComposableOperator (QueryOperator *op)
{
    switch(op->type)
    {
        case T_SelectionOperator:
            return rewritePI_CSComposableSelection((SelectionOperator *) op);
        case T_ProjectionOperator:
            return rewritePI_CSComposableProjection((ProjectionOperator *) op);
        case T_JoinOperator:
            return rewritePI_CSComposableJoin((JoinOperator *) op);
        case T_AggregationOperator:
            return rewritePI_CSComposableAggregation((AggregationOperator *) op);
        case T_Set:
            return rewritePI_CSComposableSet((SetOperator *) op);
        case T_TableAccessOperator:
            return rewritePI_CSComposableTableAccess((TableAccessOperator *) op);
        case T_ConstRelOperator:
            return rewritePI_CSComposableConstRel((ConstRelOperator *) op);
        case T_DuplicateRemoval:
            return rewritePI_CSComposableDuplicateRemOp((DuplicateRemoval *) op);
        default:
            FATAL_LOG("rewrite for %u not implemented", op->type);
            return NULL;
    }
}

static QueryOperator *
rewritePI_CSComposableSelection (SelectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child first
    rewritePI_CSComposableOperator(OP_LCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    // add result TID and prov duplicate attributes
    addResultTIDAndProvDupAttrs((QueryOperator *) op, TRUE);

    if (isTupleAtATimeSubtree(OP_LCHILD(op)))
        SET_BOOL_STRING_PROP(op,"PROVENANCE_OPERATOR_TUPLE_AT_A_TIME");

    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(op)));
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSComposableProjection (ProjectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child
    rewritePI_CSComposableOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(op);
    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
        op->projExprs = appendToTailOfList(op->projExprs,
                createFullAttrReference(att->attrName, 0, a, 0));
    }

    // add projection expressions for result TID and prov dup attrs
    op->projExprs = appendToTailOfList(op->projExprs,
            createFullAttrReference(RESULT_TID_ATTR, 0,
                    INT_VALUE(GET_STRING_PROP(child,"RESULT_TID_ATTR")), 0));

    op->projExprs = appendToTailOfList(op->projExprs,
            createFullAttrReference(PROV_DUPL_COUNT_ATTR, 0,
                    INT_VALUE(GET_STRING_PROP(child,"PROV_DUP_ATTR")), 0));
    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
    addResultTIDAndProvDupAttrs((QueryOperator *) op, TRUE);

    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(op)));
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSComposableJoin (JoinOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposableAggregation (AggregationOperator *op)
{
    boolean groupBy = (op->groupBy != NIL);
    WindowOperator *curWindow;
    QueryOperator *firstChild;
    QueryOperator *curChild;
    ProjectionOperator *proj;
    Node *provDupAttrRef;
    boolean noDupInput;
    List *projExprs = NIL;
    List *finalAttrs = NIL;
    List *orderBy = NIL;
    List *partitionBy = NIL;
    List *groupByExprs = copyObject(op->groupBy);
    List *aggNames = aggOpGetAggAttrNames(op);
    int pos;

    DEBUG_LOG("REWRITE-PICS - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child
    curChild = rewritePI_CSComposableOperator(OP_LCHILD(op));
    firstChild = curChild;
    removeParentFromOps(singleton(firstChild), (QueryOperator *) op);
    noDupInput = isTupleAtATimeSubtree(curChild);

    // create partition clause and order by clauses
    if (groupBy)
    {
        FOREACH(AttributeReference, a, op->groupBy)
            partitionBy = appendToTailOfList(partitionBy,
                                copyObject(a));

        orderBy = copyObject(partitionBy);
    }

    // get input prov dup attribute
    if (!noDupInput)
    {
        provDupAttrRef = (Node *) createFullAttrReference(PROV_DUPL_COUNT_ATTR,
                0,
                INT_VALUE(GET_STRING_PROP(curChild, "PROV_DUP_ATTR")),
                INVALID_ATTR);
    }
    else
        provDupAttrRef = NULL;

    // create window op for each aggregation
    pos = 0;
    FOREACH(Node,agg,op->aggrs)
    {
        Node *aggForWindow = replaceAttrWithCaseForProvDupRemoval(
                copyObject(agg), provDupAttrRef);
        char *attrName = getNthOfListP(aggNames, pos);

        curWindow = createWindowOp(aggForWindow,
                partitionBy,
                NIL,
                NULL,
                attrName,
                curChild,
                NIL
                );
        curWindow->op.provAttrs = copyObject(curChild->provAttrs);
        addParent(curChild, (QueryOperator *) curWindow);

        curChild = (QueryOperator *) curWindow;
        pos++;
        DEBUG_LOG("Translated aggregation function <%s> into window op:\n%s",
                beatify(nodeToString(agg)), operatorToOverviewString((Node *) curWindow));
    }

    // add result TID attr and prov dup attr, if group by then use window function, otherwise use projection
    if (groupBy)
    {
        // add window functions for result TID attr
        Node *tidFunc = (Node *) createFunctionCall(strdup("DENSE_RANK"), NIL);

        curWindow = createWindowOp(tidFunc,
                NIL,
                orderBy,
                NULL,
                strdup(RESULT_TID_ATTR),
                curChild,
                NIL
        );
        curWindow->op.provAttrs = copyObject(curChild->provAttrs);
        addParent(curChild, (QueryOperator *) curWindow);
        curChild = (QueryOperator *) curWindow;

        // add window function for prov dup attr
        Node *provDupFunc = (Node *) createFunctionCall(strdup("ROW_NUMBER"), NIL);

        curWindow = createWindowOp(provDupFunc,
                partitionBy,
                orderBy,
                NULL,
                strdup(PROV_DUPL_COUNT_ATTR),
                curChild,
                NIL
        );
        curWindow->op.provAttrs = copyObject(curChild->provAttrs);
        addParent(curChild, (QueryOperator *) curWindow);
        curChild = (QueryOperator *) curWindow;

        DEBUG_LOG("Added result TID and prov duplicate window ops:\n%s",
                       operatorToOverviewString((Node *) curWindow));
    }

    // create final projection: normal attributes + provenance attribute + result TID and prov dup attr
    List *normalAttrs = getNormalAttrProjectionExprs((QueryOperator *) curWindow);
    List *provAttrs = getProvAttrProjectionExprs((QueryOperator *) curWindow);
    List *aggAttrNames = aggOpGetAggAttrNames(op);
    List *groupByAttrNames = groupBy ? aggOpGetGroupByAttrNames(op) : NIL;
    List *provAttrNames = getOpProvenanceAttrNames((QueryOperator *) curWindow);

    // no group by, add result TID and prov dup attributes to projection
    if (!groupBy)
    {
        normalAttrs = sublist(normalAttrs,
                LIST_LENGTH(normalAttrs) - LIST_LENGTH(op->aggrs),
                LIST_LENGTH(normalAttrs));
        projExprs = CONCAT_LISTS(normalAttrs, provAttrs,
                LIST_MAKE(createConstInt(1),
                        makeNode(RowNumExpr)));

        finalAttrs = CONCAT_LISTS(aggAttrNames,
                            provAttrNames,
                            LIST_MAKE(strdup(RESULT_TID_ATTR),strdup(PROV_DUPL_COUNT_ATTR)));
    }
    // else move result TID and prov dup attribute to end of list
    else
    {
        List *tidAndDupAttrs = sublist(copyList(normalAttrs),
                LIST_LENGTH(normalAttrs) - 3,
                LIST_LENGTH(normalAttrs));
        normalAttrs = sublist(normalAttrs,
                LIST_LENGTH(normalAttrs) - LIST_LENGTH(op->aggrs) - 2,
                LIST_LENGTH(normalAttrs) - 2);

        projExprs = CONCAT_LISTS(normalAttrs, groupByExprs, provAttrs,
                LIST_MAKE(createFullAttrReference(
                            strdup(RESULT_TID_ATTR),
                            0,
                            getNumAttrs((QueryOperator *) curWindow) - 2,
                            INVALID_ATTR),
                        createFullAttrReference(
                            strdup(PROV_DUPL_COUNT_ATTR),
                            0,
                            getNumAttrs((QueryOperator *) curWindow) - 1,
                            INVALID_ATTR)));

        finalAttrs = CONCAT_LISTS(aggAttrNames,
                            groupByAttrNames,
                            provAttrNames,
                            LIST_MAKE(strdup(RESULT_TID_ATTR),strdup(PROV_DUPL_COUNT_ATTR)));
    }

    proj = createProjectionOp(projExprs, curChild, NIL, finalAttrs);
    CREATE_INT_SEQ(proj->op.provAttrs,
            LIST_LENGTH(op->aggrs) + LIST_LENGTH(op->groupBy),
            getNumAttrs((QueryOperator *) proj) - 3, 1);
    addParent((QueryOperator *) curWindow, (QueryOperator *) proj);

    SET_STRING_PROP(proj, "RESULT_TID_ATTR", createConstInt(LIST_LENGTH(finalAttrs) - 2));
    SET_STRING_PROP(proj, "PROV_DUP_ATTR", createConstInt(LIST_LENGTH(finalAttrs) - 1));

    // switch aggregation and rewritten
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    DEBUG_LOG("projection is:\n%s", operatorToOverviewString((Node *) proj));

    // return projection
    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(proj)));
    return (QueryOperator *) proj;
}

static Node *
replaceAttrWithCaseForProvDupRemoval (FunctionCall *f, Node *provDupAttrRef)
{
    if (provDupAttrRef == NULL)
        return (Node *) f;

    FOREACH_LC(lc,f->args)
    {
        Node *arg = LC_P_VAL(lc);
        LC_P_VAL(lc) = createCaseExpr(
                NULL,
                singleton(createCaseWhen((Node *) createOpExpr("=",
                        LIST_MAKE(createConstInt(1), copyObject(provDupAttrRef))),
                        arg)),
                (Node *) createNullConst(DT_INT)
                );
    }

    DEBUG_LOG("modified agg function call: <%s>", beatify(nodeToString(f)));

    return (Node *) f;
}

static QueryOperator *
rewritePI_CSComposableSet (SetOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposableTableAccess(TableAccessOperator *op)
{
    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;

    int relAccessCount = getRelNameCount(&nameState, op->tableName);
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS - Table Access <%s> <%u>", op->tableName, relAccessCount);

    // copy any as of clause if there
    if (asOf)
        op->asOf = copyObject(asOf);

    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0));
        cnt++;
    }

    cnt = 0;
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        newAttrName = getProvenanceAttrName(op->tableName, attr->attrName, relAccessCount);
        provAttr = appendToTailOfList(provAttr, newAttrName);
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0));
        cnt++;
    }

    // result tuple ID attribute
    newAttrName = strdup(RESULT_TID_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));

    // provenance duplicate attribute
    newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, createConstInt(1));

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // set properties to mark result TID and prov duplicate attrs
    SET_STRING_PROP(newpo, "RESULT_TID_ATTR", createConstInt(cnt * 2));
    SET_STRING_PROP(newpo, "PROV_DUP_ATTR", createConstInt((cnt * 2) + 1));

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);
    SET_BOOL_STRING_PROP(newpo,"PROVENANCE_OPERATOR_TUPLE_AT_A_TIME");

    DEBUG_LOG("rewrite table access: %s", operatorToOverviewString((Node *) newpo));
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSComposableConstRel(ConstRelOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposableDuplicateRemOp(DuplicateRemoval *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static void
addResultTIDAndProvDupAttrs (QueryOperator *op, boolean addToSchema)
{
    int numAttrs = getNumAttrs(op);
    QueryOperator *child = OP_LCHILD(op);

    if (addToSchema)
    {
        char *attrName;

        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                createAttributeDef(strdup(RESULT_TID_ATTR), DT_INT));
        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                    createAttributeDef(strdup(PROV_DUPL_COUNT_ATTR), DT_INT));

        // set properties to mark result TID and prov duplicate attrs
        SET_STRING_PROP(op, "RESULT_TID_ATTR", createConstInt(numAttrs));
        SET_STRING_PROP(op, "PROV_DUP_ATTR", createConstInt(numAttrs + 1));
    }
    else
    {
        SET_STRING_PROP(op, "RESULT_TID_ATTR",
                copyObject(GET_STRING_PROP(child, "RESULT_TID_ATTR")));
        SET_STRING_PROP(op, "PROV_DUP_ATTR",
                copyObject(GET_STRING_PROP(child, "PROV_DUP_ATTR")));
    }
}
