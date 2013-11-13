/*-------------------------------------------------------------------------
 *
 * query_operator.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

static Schema *mergeSchemas (List *inputs);
static Schema *schemaFromExpressions (char *name, List *attributeNames, List *exprs, List *inputs);

Schema *
createSchema(char *name, List *attrDefs)
{
    Schema *s = NEW(Schema);
    s->name = name;
    s->attrDefs = attrDefs;
    return s;
}

Schema *
createSchemaFromLists (char *name, List *attrNames, List *dataTypes)
{
    Schema *result = makeNode(Schema);

    result->name = strdup(name);
    result->attrDefs = NIL;

    int i = 0;
    if (dataTypes == NULL)
    {
        FOREACH(char,n,attrNames)
        {
            AttributeDef *a = makeNode(AttributeDef);
            a->attrName = strdup(n);
            a->dataType = DT_STRING;

            result->attrDefs = appendToTailOfList(result->attrDefs, a);
        }
    }
    else
    {
        FORBOTH_LC(n,dt,attrNames,dataTypes)
        {
            AttributeDef *a = makeNode(AttributeDef);
            a->attrName = strdup(LC_P_VAL(n));
            a->dataType = LC_INT_VAL(dt);

            result->attrDefs = appendToTailOfList(result->attrDefs, a);
        }
    }
    return result;
}

static Schema *
schemaFromExpressions (char *name, List *attributeNames, List *exprs, List *inputs)
{
    List *dataTypes = NIL;

    FOREACH(Node,n,exprs)
        dataTypes = appendToHeadOfListInt(dataTypes, typeOfInOpModel(n, inputs));

    return createSchemaFromLists(name, attributeNames, dataTypes);
}

List *
getDataTypes (Schema *schema)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,schema->attrDefs)
    {
        result = appendToTailOfListInt(result, a->dataType);
    }

    return result;
}

List *
getAttrNames(Schema *schema)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,schema->attrDefs)
    {
        result = appendToTailOfList(result, a->attrName);
    }

    return result;
}

TableAccessOperator *
createTableAccessOp(char *tableName, char *alias, List *parents,
        List *attrNames, List *dataTypes)
{
    TableAccessOperator *ta = NEW(TableAccessOperator);

    ta->tableName = tableName;
    ta->op.type = T_TableAccessOperator;
    ta->op.inputs = NULL;
    ta->op.schema = createSchemaFromLists(alias, attrNames, dataTypes);
    ta->op.parents = parents;
    ta->op.provAttrs = NIL;

    return ta;
}

SelectionOperator *
createSelectionOp(Node *cond, QueryOperator *input, List *parents,
        List *attrNames)
{
    SelectionOperator *sel = NEW(SelectionOperator);

    sel->cond = copyObject(cond);
    sel->op.type = T_SelectionOperator;
    sel->op.inputs = singleton(input);
    sel->op.schema = createSchemaFromLists("SELECT", attrNames, getDataTypes(input->schema));
    sel->op.parents = parents;
    sel->op.provAttrs = NIL;

    return sel;
}

ProjectionOperator *
createProjectionOp(List *projExprs, QueryOperator *input, List *parents,
        List *attrNames)
{
    ProjectionOperator *prj = NEW(ProjectionOperator);

    FOREACH(Node, expr, projExprs)
    {
        prj->projExprs = appendToTailOfList(prj->projExprs, (Node *) copyObject(expr));
    }

    prj->op.type = T_ProjectionOperator;
    prj->op.inputs = singleton(input);
    prj->op.schema = schemaFromExpressions("PROJECTION", attrNames, projExprs,
            singleton(input));

    prj->op.parents = parents;
    prj->op.provAttrs = NIL;

    return prj;
}

JoinOperator *
createJoinOp(JoinType joinType, Node *cond, List *inputs, List *parents,
        List *attrNames)
{
    JoinOperator *join = NEW(JoinOperator);

    join->cond = copyObject(cond);
    join->joinType = joinType;
    join->op.type = T_JoinOperator;
    join->op.inputs = inputs;
    /* get data types from inputs and attribute names from parameter to create
     * schema */
    List *lDT, *rDT;
    lDT = getDataTypes(OP_LCHILD(join)->schema);
    rDT = getDataTypes(OP_RCHILD(join)->schema);
    join->op.schema = createSchemaFromLists("JOIN", attrNames, concatTwoLists(lDT, rDT));

    join->op.parents = parents;
    join->op.provAttrs = NULL;

    return join;
}

AggregationOperator *
createAggregationOp(List *aggrs, List *groupBy, QueryOperator *input,
        List *parents, List *attrNames)
{
    AggregationOperator *aggr = NEW(AggregationOperator);

    FOREACH(Node, func, aggrs)
    {
    	aggr->aggrs = appendToTailOfList(aggr->aggrs, copyObject(func));
    }
    FOREACH(Node, expr, groupBy)
    {
    	aggr->groupBy = appendToTailOfList(aggr->groupBy, copyObject(expr));
    }
    aggr->op.type = T_AggregationOperator;
    aggr->op.inputs = singleton(input);
    aggr->op.schema = schemaFromExpressions("AGG", attrNames,
            concatTwoLists(copyList(aggrs),copyList(groupBy)), singleton(input));
    aggr->op.parents = parents;
    aggr->op.provAttrs = NULL;

    return aggr;
}

SetOperator *
createSetOperator(SetOpType setOpType, List *inputs, List *parents,
        List *attrNames)
{
    SetOperator *set = NEW(SetOperator);

    set->setOpType = setOpType;
    set->op.type = T_SetOperator;
    set->op.inputs = inputs;
    set->op.schema = createSchemaFromLists("SET", attrNames, getDataTypes(OP_LCHILD(set)->schema));
    set->op.parents = parents;
    set->op.provAttrs = NULL;

    return set;
}

DuplicateRemoval *
createDuplicateRemovalOp(List *attrs, QueryOperator *input, List *parents,
        List *attrNames)
{
    DuplicateRemoval *dr = NEW(DuplicateRemoval);

    dr->attrs = attrs;
    dr->op.type = T_DuplicateRemoval;
    dr->op.inputs = singleton(input);
    dr->op.schema = createSchemaFromLists("DUPREM", attrNames, getDataTypes(input->schema));
    dr->op.parents = parents;
    dr->op.provAttrs = NULL;

    return dr;
}

ProvenanceComputation *
createProvenanceComputOp(ProvenanceType provType, List *inputs, List *schema, List *parents, List *attrNames)
{
    return NULL; //TODO
}

List *
getProvenanceAttrs(QueryOperator *op)
{
    return op ? op->provAttrs : NIL;
}

List *
getNormalAttrs(QueryOperator *op)
{
    return op ? op->schema ? op->schema->attrDefs : NIL : NIL;
}

static Schema *
mergeSchemas (List *inputs)
{
    Schema *result = NULL;

    FOREACH(QueryOperator,O,inputs)
    {
        if (result == NULL)
            result = (Schema *) copyObject(O->schema);
        else
            result->attrDefs = concatTwoLists(result->attrDefs, copyObject(O->schema->attrDefs));
    }

    return result;
}
