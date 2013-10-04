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

#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"

extern Schema *
createSchema(char *name, List *attrDefs)
{
    Schema *s = NEW(Schema);
    s->name = name;
    s->attrDefs = attrDefs;
    return s;
}

extern TableAccessOperator *
createTableAccessOp(char *tableName, Schema *schema, List *parents,
        List *attrNames)
{
    TableAccessOperator *ta = NEW(TableAccessOperator);
    ta->tableName = tableName;
    ta->op.type = T_TableAccessOperator;
    ta->op.inputs = NULL;
    ta->op.schema = newList(T_Invalid);
    appendToTailOfList(ta->op.schema, schema);
    ta->op.parents = parents;
    ta->op.provAttrs = attrNames;
    return ta;
}

extern SelectionOperator *
createSelectionOp(Node *cond, QueryOperator *input, List *parents,
        List *attrNames)
{
    SelectionOperator *sel = NEW(SelectionOperator);
    sel->cond = cond;
    sel->op.type = T_SelectionOperator;
    sel->op.inputs = newList(T_QueryOperator);
    appendToTailOfList(sel->op.inputs, input);
    sel->op.schema = newList(T_Invalid);
    appendToTailOfList(sel->op.schema, getHeadOfListP(input->schema));
    sel->op.parents = parents;
    sel->op.provAttrs = attrNames;
    return sel;
}

extern ProjectionOperator *
createProjectionOp(List *projExprs, QueryOperator *input, List *parents,
        List *attrNames)
{
    ProjectionOperator *prj = NEW(ProjectionOperator);
    prj->projExprs = projExprs;
    prj->op.type = T_ProjectionOperator;
    prj->op.inputs = newList(T_QueryOperator);
    appendToTailOfList(prj->op.inputs, input);
    prj->op.schema = newList(T_Invalid);
    appendToTailOfList(prj->op.schema, getHeadOfListP(input->schema));
    prj->op.parents = parents;
    prj->op.provAttrs = attrNames;
    return prj;
}

extern JoinOperator *
createJoinOp(JoinType joinType, Node *cond, List *inputs, List *parents,
        List *attrNames)
{
    JoinOperator *join = NEW(JoinOperator);
    join->cond = cond;
    join->joinType = joinType;
    join->op.type = T_JoinOperator;
    join->op.inputs = inputs;
    join->op.schema = newList(T_Invalid);
    appendToTailOfList(join->op.schema,
            getHeadOfListP(((QueryOperator *) getHeadOfListP(inputs))->schema));
    appendToTailOfList(join->op.schema,
            getHeadOfListP(
                    ((QueryOperator *) getNthOfList(inputs, 2)->data.ptr_value)->schema));
    join->op.parents = parents;
    join->op.provAttrs = attrNames;
    return join;
}

extern AggregationOperator *
createAggregationOp(List *aggrs, List *groupBy, QueryOperator *input,
        List *parents, List *attrNames)
{
    AggregationOperator *aggr = NEW(AggregationOperator);
    aggr->aggrs = aggrs;
    aggr->groupBy = groupBy;
    aggr->op.type = T_AggregationOperator;
    aggr->op.inputs = newList(T_QueryOperator);
    appendToTailOfList(aggr->op.inputs, input);
    aggr->op.schema = newList(T_Invalid);
    appendToTailOfList(aggr->op.schema, getHeadOfListP(input->schema));
    aggr->op.parents = parents;
    aggr->op.provAttrs = attrNames;
    return aggr;
}

extern SetOperator *
createSetOperator(SetOpType setOpType, List *inputs, List *parents,
        List *attrNames)
{
    SetOperator *set = NEW(SetOperator);
    set->setOpType = setOpType;
    set->op.type = T_SetOperator;
    set->op.inputs = inputs;
    set->op.schema = newList(T_Invalid);
    appendToTailOfList(set->op.schema,
            getHeadOfListP(((QueryOperator *) getHeadOfListP(inputs))->schema));
    set->op.parents = parents;
    set->op.provAttrs = attrNames;
    return set;
}

extern DuplicateRemoval *
createDuplicateRemovalOp(List *attrs, QueryOperator *input, List *parents,
        List *attrNames)
{
    DuplicateRemoval *dr = NEW(DuplicateRemoval);
    dr->attrs = attrs;
    dr->op.type = T_DuplicateRemoval;
    dr->op.inputs = newList(T_QueryOperator);
    appendToTailOfList(dr->op.inputs, input);
    dr->op.schema = newList(T_Invalid);
    appendToTailOfList(dr->op.schema, getHeadOfListP(input->schema));
    dr->op.parents = parents;
    dr->op.provAttrs = attrNames;
    return dr;
}
