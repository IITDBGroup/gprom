/*-----------------------------------------------------------------------------
 *
 * query_operator_dt_inference.c
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
#include "model/list/list.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_dt_inference.h"

static void addCastsToExpressions(QueryOperator *q);
static void adaptOpSchema (QueryOperator *q);
static void adaptAttributeRefList (QueryOperator *parent, List *children);
static List *queryOperatorGetAttrRefs(QueryOperator *q);
static void castOpSchema (QueryOperator *op, List *dts);


void
introduceCastsWhereNecessary(QueryOperator *q)
{
    // process children first
    FOREACH(QueryOperator,c,q->inputs)
    {
		introduceCastsWhereNecessary(c);
    }

    // introduce cast for expressions that are used by the operator
    addCastsToExpressions(q);

    // for set operators we need to ensure that the inputs are union compatible
    switch(q->type)
    {
        case T_SetOperator:
        {
            QueryOperator *lChild = OP_LCHILD(q);
            QueryOperator *rChild = OP_RCHILD(q);
            List *lDts = getDataTypes(GET_OPSCHEMA(lChild));
            List *rDts = getDataTypes(GET_OPSCHEMA(rChild));

            // if input schemas have different datatypes then casting is required
            if (!equal(lDts,rDts))
            {
                List *lcaDTs = NIL;

                FORBOTH_INT(lDt,rDt,lDts, rDts)
                {
                    lcaDTs = appendToTailOfListInt(lcaDTs, lcaType(lDt,rDt));
                }
                castOpSchema(lChild, lcaDTs);
                castOpSchema(rChild, lcaDTs);
                adaptOpSchema(q);

                DEBUG_NODE_BEATIFY_LOG("introduced casts for inputs of set operator", q);
            }
        }
            break;
        // other operators?
        default:
            break;
    }
}

static void
castOpSchema (QueryOperator *op, List *dts)
{
    List *origDts = getDataTypes(GET_OPSCHEMA(op));
    int pos = 0;
    ProjectionOperator *p;

    if (equal(dts, origDts))
        return;

    // if the operator is a projection then add casts to projection expressions
    if (isA(op,ProjectionOperator))
    {
        p = (ProjectionOperator *) op;
    }
    else
    {
        p = (ProjectionOperator *) createProjOnAllAttrs(op);
        addChildOperator((QueryOperator *) p, op);
        switchSubtrees(op,(QueryOperator *) p);
    }
    FOREACH(Node,e,p->projExprs)
    {
        DataType targetDt = getNthOfListInt(dts, pos++);
        if (typeOf(e) != targetDt)
        {
            LC_P_VAL(e_his_cell) = createCastExpr(e, targetDt);
        }
    }

    adaptOpSchema((QueryOperator *) p);
}

static void
addCastsToExpressions(QueryOperator *q)
{
    switch(q->type)
    {
        case T_TableAccessOperator:
            break;
        case T_SampleClauseOperator:
        	break;
        case T_ProjectionOperator:
        {
            ProjectionOperator *p = (ProjectionOperator *) q;
            p->projExprs = (List *) addCastsToExpr((Node *) p->projExprs, TRUE);
        }
        break;
        case T_JoinOperator:
        {
            JoinOperator *j = (JoinOperator *) q;
            j->cond = addCastsToExpr(j->cond, TRUE);
        }
            break;
        case T_SelectionOperator:
        {
            SelectionOperator *s = (SelectionOperator *) q;
            s->cond = addCastsToExpr(s->cond, TRUE);
        }
        break;
        case T_AggregationOperator:
        {
            AggregationOperator *a = (AggregationOperator *) q;
            a->aggrs = (List *) addCastsToExpr((Node *) a->aggrs, TRUE);
            a->groupBy = (List *) addCastsToExpr((Node *) a->groupBy, TRUE);
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *w = (WindowOperator *) q;
            w->partitionBy = (List *) addCastsToExpr((Node *) w->partitionBy, TRUE);
            w->orderBy= (List *) addCastsToExpr((Node *) w->orderBy, TRUE);
            w->frameDef= (WindowFrame *) addCastsToExpr((Node *) w->frameDef, TRUE);
            w->f= addCastsToExpr(w->f, TRUE);
        }
        break;
        case T_NestingOperator:
        {
            NestingOperator *n = (NestingOperator *) q;
            n->cond = addCastsToExpr(n->cond, TRUE);
        }
        break;
        case T_SetOperator:
        case T_OrderOperator:
        case T_JsonTableOperator:
        case T_DuplicateRemoval:
        case T_ConstRelOperator:
        { //TODO

        }
        break;
        default:
            FATAL_LOG("not a query operator %s", beatify(nodeToString(q)));
    }

    adaptAttributeRefList(q, q->inputs);
    adaptOpSchema(q);

    DEBUG_OP_LOG("after casting op is:", q);
}

static void
adaptOpSchema (QueryOperator *q)
{
    List *dts = inferOpResultDTs(q);
    ListCell *dtCell = dts->head;
    FOREACH(AttributeDef, a, GET_OPSCHEMA(q)->attrDefs)
    {
        a->dataType = LC_INT_VAL(dtCell);
        LC_ADVANCE(dtCell);
    }
}

static void
adaptAttributeRefList (QueryOperator *parent, List *children)
{
    List *attrRefs = queryOperatorGetAttrRefs(parent);

    FOREACH(AttributeReference,a,attrRefs)
    {
        int input = a->fromClauseItem;
        int attrPos = a->attrPosition;
        QueryOperator *child;
        AttributeDef *childA;

        child = (QueryOperator *) getNthOfListP(children, input);
        childA = getAttrDefByPos(child, attrPos);

        // different datatype adapt parent
        if (childA->dataType != a->attrType)
        {
            DEBUG_NODE_BEATIFY_LOG("adapt parent attr ref:", a, childA);
            a->attrType = childA->dataType;
        }
    }
}

static List *
queryOperatorGetAttrRefs(QueryOperator *op)
{
    List *attrRefs = NIL;

    switch(op->type)
    {
        case T_ProjectionOperator:
        {
            ProjectionOperator *o = (ProjectionOperator *) op;
            attrRefs = getAttrReferences((Node *) o->projExprs);
        }
        break;
        case T_SelectionOperator:
        {
            SelectionOperator *o = (SelectionOperator *) op;
            attrRefs = getAttrReferences(o->cond);
        }
        break;
        case T_JoinOperator:
        {
            JoinOperator *o = (JoinOperator *) op;
            attrRefs = getAttrReferences(o->cond);
        }
        break;
        case T_AggregationOperator:
        {
            AggregationOperator *o = (AggregationOperator *) op;
            attrRefs = CONCAT_LISTS(getAttrReferences((Node *) o->aggrs),
                    getAttrReferences((Node *) o->groupBy));
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *o = (WindowOperator *) op;
            attrRefs = CONCAT_LISTS(getAttrReferences((Node *) o->partitionBy),
                    getAttrReferences((Node *) o->orderBy),
                    getAttrReferences((Node *) o->frameDef),
                    getAttrReferences((Node *) o->f));
        }
        break;
        case T_OrderOperator:
        {
            OrderOperator *o = (OrderOperator *) op;
            attrRefs = getAttrReferences((Node *) o->orderExprs);
        }
        break;
        case T_DuplicateRemoval:
        {

        }
            break;
        // Check Attribute that we use as Json Column should be from/should exist in child
        case T_JsonTableOperator:
        {
            JsonTableOperator *o = (JsonTableOperator *)op;
            attrRefs = singleton(o->jsonColumn);
        }
        break;
        default:
            break;
    }

    return attrRefs;
}
