/*-----------------------------------------------------------------------------
 *
 * temporal_rewriter.c
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
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "temporal_queries/temporal_rewriter.h"
#include "analysis_and_translate/translator_oracle.h"

static QueryOperator *temporalRewriteOperator(QueryOperator *op);
static QueryOperator *tempRewrSelection (SelectionOperator *o);
static QueryOperator *tempRewrProjection (ProjectionOperator *o);
static QueryOperator *tempRewrJoin (JoinOperator *op);
static QueryOperator *tempRewrAggregation (AggregationOperator *o);
static QueryOperator *tempRewrTemporalSource (QueryOperator *o);
static QueryOperator *tempRewrSetOperator (SetOperator *o);

static void setTempAttrProps(QueryOperator *o);
static AttributeReference *getTempAttrRef (QueryOperator *o, boolean begin);
static void coalescingAndAlignmentVisitor (QueryOperator *q, Set *done);

#define LOG_RESULT(mes,op) DEBUG_OP_LOG(mes,op);



QueryOperator *
rewriteImplicitTemporal (QueryOperator *q)
{
//    ProvenanceComputation *p = (ProvenanceComputation *) q;
    ASSERT(LIST_LENGTH(q->inputs) == 1);
    QueryOperator *top = getHeadOfListP(q->inputs);
    List *topSchema;

    addCoalescingAndAlignment(top);

    top = temporalRewriteOperator (top);

    // make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    topSchema = copyObject(q->schema->attrDefs);
    disambiguiteAttrNames((Node *) top, done);
    if (isA(top,ProjectionOperator))
        top->schema->attrDefs = topSchema;
    else
    {
        QueryOperator *proj = createProjOnAllAttrs(top);
        addChildOperator(proj, top);
        switchSubtrees((QueryOperator *) top, proj);
        proj->schema->attrDefs = topSchema;
    }

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) q, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root is:", top);

    return top;
}

static QueryOperator *
temporalRewriteOperator(QueryOperator *op)
{
    QueryOperator *rewrittenOp = NULL;

    if (HAS_STRING_PROP(op, PROP_DUMMY_HAS_PROV_PROJ))
        return tempRewrTemporalSource(op);

    switch(op->type)
    {
        case T_SelectionOperator:
            DEBUG_LOG("go selection");
            rewrittenOp = tempRewrSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            DEBUG_LOG("go projection");
            rewrittenOp = tempRewrProjection((ProjectionOperator *) op);
            break;
        case T_AggregationOperator:
            DEBUG_LOG("go aggregation");
            rewrittenOp = tempRewrAggregation ((AggregationOperator *) op);
            break;
        case T_JoinOperator:
            DEBUG_LOG("go join");
            rewrittenOp = tempRewrJoin((JoinOperator *) op);
            break;
        case T_SetOperator:
            DEBUG_LOG("go set");
            rewrittenOp = tempRewrSetOperator((SetOperator *) op);
            break;
//        case T_TableAccessOperator:
//            DEBUG_LOG("go table access");
//            rewrittenOp = tempRewrTableAccess((TableAccessOperator *) op);
//            break;
//        case T_ConstRelOperator:
//            DEBUG_LOG("go const rel operator");
//            rewrittenOp = tempRewrConstRel((ConstRelOperator *) op);
//            break;
//        case T_DuplicateRemoval:
//            DEBUG_LOG("go duplicate removal operator");
//            rewrittenOp = tempRewrDuplicateRemOp((DuplicateRemoval *) op);
//            break;
//        case T_OrderOperator:
//            DEBUG_LOG("go order operator");
//            rewrittenOp = tempRewrOrderOp((OrderOperator *) op);
//            break;
//        case T_JsonTableOperator:
//            DEBUG_LOG("go JsonTable operator");
//            rewrittenOp = tempRewrJsonTableOp((JsonTableOperator *) op);
//            break;
        default:
            FATAL_LOG("no rewrite implemented for operator %s", beatify(nodeToString(op)));
            return NULL;
    }

    return rewrittenOp;
}

static QueryOperator *
tempRewrSelection (SelectionOperator *o)
{
    // rewrite child first
    temporalRewriteOperator(OP_LCHILD(o));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) o, OP_LCHILD(o));
    setTempAttrProps((QueryOperator *) o);

    LOG_RESULT("Rewritten Selection:", o);
    return (QueryOperator *) o;
}


static QueryOperator *
tempRewrProjection (ProjectionOperator *o)
{
    // rewrite child
    temporalRewriteOperator(OP_LCHILD(o));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(o);
    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
        o->projExprs = appendToTailOfList(o->projExprs,
                createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
    }

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) o, OP_LCHILD(o));
    setTempAttrProps((QueryOperator *) o);

    return (QueryOperator *) o;
}

static QueryOperator *
tempRewrJoin (JoinOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Join");
    QueryOperator *o = (QueryOperator *) op;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
    List *rNormAttrs;
    int numLAttrs, numRAttrs;

    numLAttrs = LIST_LENGTH(lChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rChild->schema->attrDefs);

    // get attributes from right input
    rNormAttrs = sublist(o->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    o->schema->attrDefs = sublist(copyObject(o->schema->attrDefs), 0, numLAttrs - 1);

    // rewrite children
    lChild = temporalRewriteOperator(lChild);
    rChild = temporalRewriteOperator(rChild);

    // adapt schema for join op use
    addProvenanceAttrsToSchema(o, lChild);
    o->schema->attrDefs = CONCAT_LISTS(o->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchema(o, rChild);

    // add extra condition to join to check for interval overlap
    // left.begin <= right.begin <= right.end OR right.begin <= left.begin <= right.end
    AttributeReference *lBegin, *lEnd, *rBegin, *rEnd;
    Node *cond;

    lBegin = getTempAttrRef(lChild, TRUE);
    lEnd = getTempAttrRef(lChild, FALSE);
    rBegin = getTempAttrRef(rChild, TRUE);
    rBegin->fromClauseItem = 1;
    rEnd = getTempAttrRef(rChild, FALSE);
    rEnd->fromClauseItem = 1;

    cond = OR_EXPRS(
            AND_EXPRS(
                    (Node *) createOpExpr("<=", LIST_MAKE(copyObject(lBegin), copyObject(rBegin))),
                    (Node *) createOpExpr("<=", LIST_MAKE(copyObject(rBegin), copyObject(lEnd)))
            ),
            AND_EXPRS(
                    (Node *) createOpExpr("<=", LIST_MAKE(copyObject(rBegin), copyObject(lBegin))),
                    (Node *) createOpExpr("<=", LIST_MAKE(copyObject(lBegin), copyObject(rEnd)))
            )
    );

    // since we are adding a join condition this turns cross products into inner joins
    if (op->joinType == JOIN_CROSS)
    {
        op->joinType = JOIN_INNER;
        op->cond = cond;
    }
    else
    {
        op->cond = AND_EXPRS(op->cond, cond);
    }
    DEBUG_NODE_BEATIFY_LOG("new join condition", op->cond);


    // construct projection expressions that intersect intervals
    List *temporalAttrProjs = NIL;
    Node *tBegin;
    Node *tEnd;
    List *temporalAttrRefs = getProvAttrProjectionExprs((QueryOperator *) op);

    lBegin = getNthOfListP(temporalAttrRefs, 0);
    lEnd = getNthOfListP(temporalAttrRefs, 1);
    rBegin = getNthOfListP(temporalAttrRefs, 2);
    rEnd = getNthOfListP(temporalAttrRefs, 3);

    tBegin = (Node *) createFunctionCall("GREATEST", LIST_MAKE(lBegin, rBegin));
    tEnd = (Node *) createFunctionCall("LEAST", LIST_MAKE(lEnd, rEnd));
    temporalAttrProjs = LIST_MAKE(tBegin, tEnd);


    // add projection to put attributes into order on top of join op and computes the interval intersection
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) op),
            temporalAttrProjs);
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    // use left inputs provenance attributes in join output as provenance attributes to be able to reuse code from provenance rewriting
    o->provAttrs = sublist(o->provAttrs, 0, 1);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);


    // switch join with new projection
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    setTempAttrProps((QueryOperator *) o);

    LOG_RESULT("Rewritten join", op);
    return (QueryOperator *) proj;
}

static QueryOperator *
tempRewrAggregation (AggregationOperator *o)
{
    AttributeReference *tb, *te;
    QueryOperator *op = (QueryOperator *) o;
    QueryOperator *c = OP_LCHILD(op);
    int bPos = getNumAttrs(op);
    int ePos = bPos + 1;

    // rewrite child
    c = temporalRewriteOperator(c);

    tb = getTempAttrRef(c, TRUE);
    te = getTempAttrRef(c, FALSE);

    o->groupBy = appendToTailOfList(o->groupBy, tb);
    o->groupBy = appendToTailOfList(o->groupBy, te);

    op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
            createAttributeDef(strdup(tb->name), tb->attrType));
    op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                createAttributeDef(strdup(te->name), te->attrType));

    op->provAttrs = CONCAT_LISTS(singletonInt(bPos), singletonInt(ePos));

    setTempAttrProps((QueryOperator *) o);

    LOG_RESULT("Rewritten aggregation", o);
    return (QueryOperator *) o;
}

static QueryOperator *
tempRewrTemporalSource (QueryOperator *op)
{
    ProjectionOperator *p = (ProjectionOperator *) op;
    QueryOperator *c = OP_LCHILD(op);
    List *userProvAttrs = (List *) getStringProperty(c, PROP_USER_PROV_ATTRS);
    int curPos = getNumAttrs(op);
    int tempAttrPos = 0;

    DEBUG_LOG("Use existing temporal attributes %s for %s",
            beatify(nodeToString(userProvAttrs)), c->schema->name);

    FOREACH(Constant,a,userProvAttrs)
    {
        char *name = STRING_VALUE(a);
        char *newName = (tempAttrPos == 0) ? TBEGIN_NAME : TEND_NAME ;
        int pos = getAttrPos(c, name);
        AttributeDef *attr, *origAttr;
        AttributeReference *aRef;

        origAttr = getAttrDefByPos(c, pos);

        op->provAttrs = appendToTailOfListInt(op->provAttrs, curPos++);

        attr = createAttributeDef(newName, origAttr->dataType);
        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs, attr);

        aRef = createFullAttrReference(strdup(name),0,pos,INVALID_ATTR, origAttr->dataType);
        p->projExprs = appendToTailOfList(p->projExprs, aRef);
        tempAttrPos++;
    }

    setTempAttrProps((QueryOperator *) p);

    return op;
}

static QueryOperator *
tempRewrSetOperator (SetOperator *o)
{
    QueryOperator *lOp = OP_LCHILD(o);
    QueryOperator *rOp = OP_RCHILD(o);

    lOp = temporalRewriteOperator(lOp);
    rOp = temporalRewriteOperator(rOp);

    switch(o->setOpType)
    {
        case SETOP_UNION:
        {
            addProvenanceAttrsToSchema((QueryOperator *) o, (QueryOperator *) lOp);
        }
        break;
        case SETOP_INTERSECTION:
            break;
        case SETOP_DIFFERENCE:
            break;
    }


    setTempAttrProps((QueryOperator *) o);

    return (QueryOperator *) o;
}

static void
setTempAttrProps(QueryOperator *o)
{
    AttributeDef *tb = copyObject(getAttrDefByName(o, TBEGIN_NAME));
    AttributeDef *te = copyObject(getAttrDefByName(o, TEND_NAME));

    setStringProperty(o, PROP_TEMP_TBEGIN_ATTR, (Node *) tb);
    setStringProperty(o, PROP_TEMP_TEND_ATTR, (Node *) te);
}

static AttributeReference *
getTempAttrRef (QueryOperator *o, boolean begin)
{
    AttributeReference *result;
    AttributeDef *d;
    int pos;

    if (begin)
    {
        d = (AttributeDef *) getStringProperty(o, PROP_TEMP_TBEGIN_ATTR);
    }
    else
    {
        d = (AttributeDef *) getStringProperty(o, PROP_TEMP_TEND_ATTR);
    }

    pos = getAttrPos(o, d->attrName);
    result = createFullAttrReference(strdup(d->attrName), 0, pos, INVALID_ATTR, TEMPORAL_DT);
    return result;
}

/*
 * determine where in a temporal query we have to apply the two temporal normalization
 * operations "coalesce" and "align". This method only marks operators that should
 * be subjected to one of these normalization operators.
 */
void
addCoalescingAndAlignment (QueryOperator *q)
{
    Set *done = PSET();
    // mark children first
    SET_BOOL_STRING_PROP(q,PROP_TEMP_DO_COALESCE);
    DEBUG_OP_LOG("mark top operator for coalescing", q);

    coalescingAndAlignmentVisitor(q, done);
}

static void
coalescingAndAlignmentVisitor (QueryOperator *q, Set *done)
{
    if (hasSetElem(done,q))
        return;

    FOREACH(QueryOperator,child,q->inputs)
        coalescingAndAlignmentVisitor(child, done);

    switch(q->type)
    {
        case T_AggregationOperator:
        {
            QueryOperator* child = OP_LCHILD(q);
            SET_BOOL_STRING_PROP(child,PROP_TEMP_DO_COALESCE);
            DEBUG_OP_LOG("mark aggregation input for coalescing", q);
        }
        break;
        case T_SetOperator:
        {
            SetOperator *s = (SetOperator *) q;
            if (s->setOpType == SETOP_DIFFERENCE || s->setOpType == SETOP_INTERSECTION)
            {
                SET_BOOL_STRING_PROP(q,PROP_TEMP_NORMALIZE_INPUTS);
                DEBUG_OP_LOG("mark setop for normalization of inputs", q);
            }
        }
        break;
        default:
            break;
    }

    addToSet(done, q);
}

/* add algebra expressions to coalesce the output of an operator */
QueryOperator *
addCoalesce (QueryOperator *input)
{
    return input;
}

/*
 * aligns the output of "input" to temporally align with "reference"
 * this normalization has to be applied to the inputs of set difference
 * for R - S we rewrite it into A(R,S) - A(S,R)
 */
QueryOperator *
addTemporalAlignment (QueryOperator *input, QueryOperator *reference)
{
    return input;
}

