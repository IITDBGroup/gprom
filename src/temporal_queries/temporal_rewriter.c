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

#define ONE 1
#define ZERO 0


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

    top = addCoalesce(top);

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

	QueryOperator *op = input;

	//---------------------------------------------------------------------------------------
    //Construct T1: a union on two projections
    Constant *c0 = createConstInt(ZERO);
    Constant *c1 = createConstInt(ONE);

    //construct projExprs (SALARY 0 1 T_B )
    //construct projExprs (SALARY 1 0 T_E )
    List *t1NorProjExpr1 = getNormalAttrProjectionExprs(op);
    List *t1NorProjExpr2 = getNormalAttrProjectionExprs(op);

    List *t1ProjExpr1 = t1NorProjExpr1;
    List *t1ProjExpr2 = t1NorProjExpr2;
    t1ProjExpr1 = appendToTailOfList(t1ProjExpr1, copyObject(c0));
    t1ProjExpr1 = appendToTailOfList(t1ProjExpr1, copyObject(c1));
    t1ProjExpr2 = appendToTailOfList(t1ProjExpr2, copyObject(c1));
    t1ProjExpr2 = appendToTailOfList(t1ProjExpr2, copyObject(c0));

    AttributeDef *t1BeginDef  = (AttributeDef *) getStringProperty(op, PROP_TEMP_TBEGIN_ATTR);
    int t1BeginPos = getAttrPos(op, t1BeginDef->attrName);
    AttributeReference *t1BeginRef = createFullAttrReference(strdup(t1BeginDef->attrName), 0, t1BeginPos, INVALID_ATTR, t1BeginDef->dataType);

    AttributeDef *t1EndDef  = (AttributeDef *) getStringProperty(op, PROP_TEMP_TEND_ATTR);
    int t1EndPos = getAttrPos(op, t1EndDef->attrName);
    AttributeReference *t1EndRef = createFullAttrReference(strdup(t1EndDef->attrName), 0, t1EndPos, INVALID_ATTR, t1EndDef->dataType);

    t1ProjExpr1 = appendToTailOfList(t1ProjExpr1, t1EndRef);
    t1ProjExpr2 = appendToTailOfList(t1ProjExpr2, t1BeginRef);

    //construct schema SALARY T_B T_E TS  for BOTH
    List *t1AttrNames = getAttrNames(op->schema);
    t1AttrNames = appendToTailOfList(t1AttrNames,TS);

    ProjectionOperator *t1Proj1 = createProjectionOp(t1ProjExpr1, op, NIL, deepCopyStringList(t1AttrNames));
    ProjectionOperator *t1Proj2 = createProjectionOp(t1ProjExpr2, op, NIL, deepCopyStringList(t1AttrNames));
    op->parents = LIST_MAKE(t1Proj1,t1Proj2);

    //construct union on top
    SetOperator *t1 = createSetOperator(SETOP_UNION, LIST_MAKE(t1Proj1, t1Proj2), NIL,
    		deepCopyStringList(t1AttrNames));
    ((QueryOperator *) t1Proj1)->parents = singleton(t1);
    ((QueryOperator *) t1Proj2)->parents = singleton(t1);

	//---------------------------------------------------------------------------------------
    //Construct T2: SELECT salary, SUM(T_B) - SUM(T_E), TS

    //Window 1 (SUM(S_B), PARTITION BY salary, GROUP_BY TS, RANGE UNBOUNDED PRECEDING
    WindowBound *wbT2W1 = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
    WindowFrame *wfT2W1 = createWindowFrame(WINFRAME_RANGE,wbT2W1,NULL);

    QueryOperator *t1Op = (QueryOperator *) t1;
    //partationBy
    AttributeDef *t2PBDef1 = (AttributeDef *) getHeadOfListP(t1Op->schema->attrDefs);
    AttributeReference *t2PBRef1 = createFullAttrReference(strdup(t2PBDef1->attrName), 0, 0, INVALID_ATTR, t2PBDef1->dataType);
    List *t2PartitionBy1 = singleton(t2PBRef1);

    //groupBy
    AttributeDef *t2GBDef1 = (AttributeDef *) getTailOfListP(t1Op->schema->attrDefs);
    AttributeReference *t2GBRef1 = createFullAttrReference(strdup(t2GBDef1->attrName), 0, LIST_LENGTH(t1Op->schema->attrDefs) - 1, INVALID_ATTR, t2GBDef1->dataType);
    List *t2GroupBy1 = singleton(t2GBRef1);

    WindowDef *t2WD1 = createWindowDef(t2PartitionBy1,t2GroupBy1,wfT2W1);

    AttributeDef *t1TBDef = copyObject(getAttrDefByName(t1Op, TBEGIN_NAME));
    int t1TBDefPos = getAttrPos(op, t1TBDef->attrName);
    AttributeReference *t2FCRef = createFullAttrReference(strdup(t1TBDef->attrName), 0, t1TBDefPos, INVALID_ATTR, t1TBDef->dataType);
    FunctionCall *t2FC = createFunctionCall("SUM",singleton(t2FCRef));

    WindowFunction *t2WF = createWindowFunction(t2FC,t2WD1);

    char *t2AName = "winf_0";
    WindowOperator *t2W1 = createWindowOp(copyObject(t2WF->f),
            copyObject(t2WF->win->partitionBy),
            copyObject(t2WF->win->orderBy),
            copyObject(t2WF->win->frame),
			t2AName, t1Op, NIL);
    t1Op->parents = singleton(t2W1);

    //Window 2
    WindowFrame *wfT2W2 = copyObject(wfT2W1);

    QueryOperator *t2W1Op = (QueryOperator *) t2W1;
    //partationBy
    AttributeDef *t2PBDef2 = (AttributeDef *) getHeadOfListP(t2W1Op->schema->attrDefs);
    AttributeReference *t2PBRef2 = createFullAttrReference(strdup(t2PBDef2->attrName), 0, 0, INVALID_ATTR, t2PBDef2->dataType);
    List *t2PartitionBy2 = singleton(t2PBRef2);

    //groupBy
    AttributeDef *t2GBDef2 = (AttributeDef *) getAttrDefByName(t2W1Op, TS);
    AttributeReference *t2GBRef2 = createFullAttrReference(strdup(t2GBDef2->attrName), 0, LIST_LENGTH(t1Op->schema->attrDefs) - 1, INVALID_ATTR, t2GBDef2->dataType);
    List *t2GroupBy2 = singleton(t2GBRef2);

    WindowDef *t2WD2 = createWindowDef(t2PartitionBy2,t2GroupBy2,wfT2W2);

    AttributeReference *t2W2FCRef = createAttrsRefByName(t2W1Op, TEND_NAME);
    FunctionCall *t2W2FC = createFunctionCall("SUM",singleton(t2W2FCRef));

    WindowFunction *t2W2WF = createWindowFunction(t2W2FC,t2WD2);

    char *t2W2AName = "winf_1";
    WindowOperator *t2W2 = createWindowOp(copyObject(t2W2WF->f),
            copyObject(t2W2WF->win->partitionBy),
            copyObject(t2W2WF->win->orderBy),
            copyObject(t2W2WF->win->frame),
			t2W2AName, t2W1Op, NIL);

    t2W1Op->parents = singleton(t2W2);

    //Top projection
    QueryOperator *t2W2Op = (QueryOperator *) t2W2;
    List *t2ProjExpr = NIL;

    //salary
    AttributeDef *t2NorDef= (AttributeDef *) getHeadOfListP(t2W2Op->schema->attrDefs);
    AttributeReference *t2NorRef = createFullAttrReference(strdup(strdup(t2NorDef->attrName)), 0, 0, INVALID_ATTR, t2NorDef->dataType);
    t2ProjExpr = appendToTailOfList(t2ProjExpr, t2NorRef);

    //sum(w_0) - sum(w_1)
    AttributeReference *t2Winf0 = createAttrsRefByName(t2W2Op, "winf_0");
    AttributeReference *t2Winf1 = createAttrsRefByName(t2W2Op, "winf_1");
    List *t2MinusExpr = LIST_MAKE(t2Winf0,t2Winf1);
    Operator *t2Minus = createOpExpr("-",t2MinusExpr);
    t2ProjExpr = appendToTailOfList(t2ProjExpr, t2Minus);

    //TS
    AttributeReference *t2TS = createAttrsRefByName(t2W2Op, TS);
    t2ProjExpr = appendToTailOfList(t2ProjExpr, t2TS);

    List *T2AttrNames = LIST_MAKE(strdup(t2NorDef->attrName),"NUMOPEN",strdup(TS));

    ProjectionOperator *t2 = createProjectionOp(t2ProjExpr, t2W2Op, NIL, T2AttrNames);
    t2W2Op->parents = singleton(t2);

	//---------------------------------------------------------------------------------------
    //Construct T3: (diffFollowing, diffPrevious, numOpen, ts, salary)
    QueryOperator *t2Op = (QueryOperator *) t2;

    //window 1 : FIRST_VALUE(numOpen) OVER (PARTITION BY salary ORDER BY ts ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING
    WindowBound *t3wb1 = createWindowBound(WINBOUND_EXPR_FOLLOW,(Node *)singleton(copyObject(c1)));
    WindowFrame *t3wf1 = createWindowFrame(WINFRAME_ROWS,t3wb1,copyObject(t3wb1));

    //partationBy
    AttributeDef *t3pbDef1 = (AttributeDef *) getHeadOfListP(t2Op->schema->attrDefs);
    AttributeReference *t3pbRef1 = createFullAttrReference(strdup(t3pbDef1->attrName), 0, 0, INVALID_ATTR, t3pbDef1->dataType);
    List *t3PartitionBy1 = singleton(t3pbRef1);

    //groupBy
    AttributeDef *t3gbDef1 = (AttributeDef *) getTailOfListP(t2Op->schema->attrDefs);
    AttributeReference *t3gbRef1 = createFullAttrReference(strdup(t3gbDef1->attrName), 0, LIST_LENGTH(t2Op->schema->attrDefs) - 1, INVALID_ATTR, t3gbDef1->dataType);
    List *t3GroupBy1 = singleton(t3gbRef1);

    WindowDef *t3wd1 = createWindowDef(t3PartitionBy1,t3GroupBy1,t3wf1);

    AttributeReference *t3fcRef = createAttrsRefByName(t2Op, NUMOPEN);
    FunctionCall *t3fc = createFunctionCall("FIRST_VALUE",singleton(t3fcRef));

    WindowFunction *t3wf = createWindowFunction(t3fc,t3wd1);

    char *t3AName = "winf_0";
    WindowOperator *t3w1 = createWindowOp(copyObject(t3wf->f),
            copyObject(t3wf->win->partitionBy),
            copyObject(t3wf->win->orderBy),
            copyObject(t3wf->win->frame),
			t3AName, t2Op, NIL);
    t2Op->parents = singleton(t3w1);

    //window 2 : FIRST_VALUE(numOpen) OVER (PARTITION BY salary ORDER BY ts ROWS BETWEEN 1 PRECEDING AND 1 PRECEDING
    QueryOperator *t3w1Op = (QueryOperator *) t3w1;

    WindowBound *t3wb2 = createWindowBound(WINBOUND_EXPR_PREC,(Node *)singleton(copyObject(c1)));
    WindowFrame *t3wf2 = createWindowFrame(WINFRAME_ROWS,t3wb2,copyObject(t3wb2));

    //partationBy
    AttributeDef *t3pbDef2 = (AttributeDef *) getHeadOfListP(t3w1Op->schema->attrDefs);
    AttributeReference *t3pbRef2 = createFullAttrReference(strdup(t3pbDef2->attrName), 0, 0, INVALID_ATTR, t3pbDef2->dataType);
    List *t3PartitionBy2 = singleton(t3pbRef2);

    //groupBy
    AttributeReference *t3gpRef2 = createAttrsRefByName(t3w1Op, TS);
    List *t3GroupBy2 = singleton(t3gpRef2);

    WindowDef *t3wd2 = createWindowDef(t3PartitionBy2,t3GroupBy2,t3wf2);

    AttributeReference *t3fcRef2 = createAttrsRefByName(t3w1Op, NUMOPEN);
    FunctionCall *t3fc2 = createFunctionCall("FIRST_VALUE",singleton(t3fcRef2));

    WindowFunction *t3wff = createWindowFunction(t3fc2,t3wd2);

    char *t3ANamef = "winf_1";
    WindowOperator *t3w2 = createWindowOp(copyObject(t3wff->f),
    		copyObject(t3wff->win->partitionBy),
			copyObject(t3wff->win->orderBy),
			copyObject(t3wff->win->frame),
			t3ANamef, t3w1Op, NIL);
    t3w1Op->parents = singleton(t3w2);

    //projection :  Projection[SALARY COALESCE((NUMOPEN - W0),666) COALESCE((NUMOPEN - W1),666) NUMOPEN TS ]
    //(DIFFFOLLOWING DIFFPREVIOUS NUMOPEN TS SALARY)
    QueryOperator *t3w2Op = (QueryOperator *)t3w2;
    Constant *c6 = createConstInt(666);
    List *t3ProjExpr = NIL;

    //salary
    AttributeDef *t3ProjDef1 = (AttributeDef *) getHeadOfListP(t3w2Op->schema->attrDefs);
    AttributeReference *t3ProjRef1 = createFullAttrReference(strdup(t3ProjDef1->attrName), 0, 0, INVALID_ATTR, t3ProjDef1->dataType);
    t3ProjExpr = appendToTailOfList(t3ProjExpr, t3ProjRef1);

    //COALESCE((NUMOPEN - winf_0),666)
    AttributeReference *t3ProjNOpen1 = createAttrsRefByName(t3w2Op, NUMOPEN);
    AttributeReference *t3ProjW01 = createAttrsRefByName(t3w2Op, "winf_0");
    Operator *t3O1 = createOpExpr("-", LIST_MAKE(t3ProjNOpen1,t3ProjW01));
    FunctionCall *t3Projfc1 = createFunctionCall("COALESCE",LIST_MAKE(t3O1,copyObject(c6)));
    t3ProjExpr = appendToTailOfList(t3ProjExpr, t3Projfc1);

    //COALESCE((NUMOPEN - winf_1),666)
    AttributeReference *t3ProjNOpen2 = copyObject(t3ProjNOpen1);
    AttributeReference *t3ProjW02 = createAttrsRefByName(t3w2Op, "winf_1");
    Operator *t3O2 = createOpExpr("-", LIST_MAKE(t3ProjNOpen2,t3ProjW02));
    FunctionCall *t3Projfc2 = createFunctionCall("COALESCE",LIST_MAKE(t3O2,copyObject(c6)));
    t3ProjExpr = appendToTailOfList(t3ProjExpr, t3Projfc2);

    //numOpen, TS
    t3ProjExpr = appendToTailOfList(t3ProjExpr, copyObject(t3ProjNOpen1));
    AttributeReference *t3ProjTS = createAttrsRefByName(t3w2Op, TS);
    t3ProjExpr = appendToTailOfList(t3ProjExpr, t3ProjTS);

    //Proj names
    List *t3ProjAttrNames = LIST_MAKE(strdup(t3ProjDef1->attrName), "DIFFFOLLOWING", "DIFFPREVIOUS", "NUMOPEN", "TS");

    ProjectionOperator *t3Proj = createProjectionOp(t3ProjExpr, t3w2Op, NIL, t3ProjAttrNames);
    t3w2Op->parents = singleton(t3Proj);

	DuplicateRemoval *t3Dr = createDuplicateRemovalOp(NIL, (QueryOperator *) t3Proj,
			NIL, deepCopyStringList(t3ProjAttrNames));

	QueryOperator *t3ProjOp = (QueryOperator *)t3Proj;
	t3ProjOp->parents = singleton(t3Dr);

	//---------------------------------------------------------------------------------------
    //Construct T4:   SELECT * FROM T3 SELECTION: WHERE diffFollowing != 0 OR diffPrevious != 0
	QueryOperator *t3Op = (QueryOperator *) t3Dr;

	AttributeReference *t4dfRef = createAttrsRefByName(t3Op, "DIFFFOLLOWING");
	AttributeReference *t4dpRef = createAttrsRefByName(t3Op, "DIFFPREVIOUS");

    Operator *t4O1 = createOpExpr("!=", LIST_MAKE(t4dfRef,copyObject(c0)));
    Operator *t4O2 = createOpExpr("!=", LIST_MAKE(t4dpRef,copyObject(c0)));
    Node *t4Cond = OR_EXPRS((Node *)t4O1,(Node *)t4O2);

    SelectionOperator *t4 = createSelectionOp(t4Cond, t3Op, NIL, deepCopyStringList(t3ProjAttrNames));
    t3Op->parents = singleton(t4);

	//---------------------------------------------------------------------------------------
    //Construct T5:  SELECT  diffFollowing, diffPrevious, numOpen, ts AS t_b,LAST_VALUE(ts) OVER
    //(PARTITION BY salary ORDER BY ts ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING) AS t_e
    //salary FROM t4) WHERE diffPrevious != 0 AND tend IS NOT NULL
    QueryOperator *t4Op = (QueryOperator *) t4;

    //window: LAST_VALUE(ts) OVER (PARTITION BY salary ORDER BY ts ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING) AS t_e
    WindowBound *t5wb = createWindowBound(WINBOUND_EXPR_FOLLOW,(Node *)singleton(copyObject(c1)));
    WindowFrame *t5wf = createWindowFrame(WINFRAME_ROWS,t5wb,copyObject(t5wb));

    //partationBy
    AttributeDef *t5pbDef = (AttributeDef *) getHeadOfListP(t4Op->schema->attrDefs);
    AttributeReference *t5pbRef = createFullAttrReference(strdup(t5pbDef->attrName), 0, 0, INVALID_ATTR, t5pbDef->dataType);
    List *t5PartitionBy = singleton(t5pbRef);

    //groupBy
    AttributeReference *t5gbRef = createAttrsRefByName(t4Op, TS);
    List *t5GroupBy = singleton(t5gbRef);

    WindowDef *t5wd = createWindowDef(t5PartitionBy,t5GroupBy,t5wf);

    FunctionCall *t5fc = createFunctionCall("LAST_VALUE",singleton(copyObject(t5gbRef)));

    WindowFunction *t5wff = createWindowFunction(t5fc,t5wd);

    char *t5AName = "winf_0";
    WindowOperator *t5w = createWindowOp(copyObject(t5wff->f),
            copyObject(t5wff->win->partitionBy),
            copyObject(t5wff->win->orderBy),
            copyObject(t5wff->win->frame),
			t5AName, t4Op, NIL);
    t4Op->parents = singleton(t5w);

    //Projection: SELECT  salary, diffFollowing, diffPrevious, numOpen, ts as t_b, winf_0 as t_e
    QueryOperator *t5wOp = (QueryOperator *) t5w;

    List *t5ProjExpr = NIL;
    List *t5ProjNames = NIL;
    int i = 0;
    int t5DefLen = LIST_LENGTH(t5wOp->schema->attrDefs);
    FOREACH(AttributeDef,a,t5wOp->schema->attrDefs)
    {
        AttributeReference *att;
        att = createFullAttrReference(a->attrName, 0, i++, INVALID_ATTR, a->dataType);
        t5ProjExpr = appendToTailOfList(t5ProjExpr, att);
        if(i < t5DefLen-1)
        	t5ProjNames = appendToTailOfList(t5ProjNames, strdup(a->attrName));
        else if(i == t5DefLen-1)
        	t5ProjNames = appendToTailOfList(t5ProjNames, TBEGIN_NAME);
        else if(i == t5DefLen)
        	t5ProjNames = appendToTailOfList(t5ProjNames, TEND_NAME);
    }

    ProjectionOperator *t5Proj = createProjectionOp(t5ProjExpr, t5wOp, NIL, t5ProjNames);
    t5wOp->parents = singleton(t5Proj);

    //SELECTION : WHERE diffPrevious != 0 AND tend IS NOT NULL
	QueryOperator *t5ProjOp = (QueryOperator *) t5Proj;

	AttributeReference *t5CondRef1 = createAttrsRefByName(t5ProjOp, "DIFFPREVIOUS");
	AttributeReference *t5CondRef2 = createAttrsRefByName(t5ProjOp, TEND_NAME);

    Operator *t5O1 = createOpExpr("!=", LIST_MAKE(t5CondRef1,copyObject(c0)));
    IsNullExpr *t5O2 = createIsNullExpr((Node *) singleton(t5CondRef2));
    Operator *t5O3 = createOpExpr("NOT", singleton(t5O2));
    Node *t5Cond = AND_EXPRS((Node *)t5O1,(Node *)t5O3);

    SelectionOperator *t5 = createSelectionOp(t5Cond, t5ProjOp, NIL, deepCopyStringList(t5ProjNames));
    t5ProjOp->parents = singleton(t5);

    //-----------------------------------------------------------------------------------------------------------------
    //TNTAB AS (SELECT rownum n from dual connect by level <= (SELECT max(numOpen) FROM T5))
	TableAccessOperator *TNTAB = createTableAccessOp("TNTAB_EMPHIST_100K", NULL, "TNTAB", NIL, singleton("N"), singletonInt(DT_INT));

	//---------------------------------------------------------------------------------------
    //Construct T6:    SELECT TSTART, TEND, SALARY FROM T5, TNTAB WHERE T5.numOpen <= n

	//join cond = numOpen <= N (SALARY DIFFFOLLOWING DIFFPREVIOUS NUMOPEN T_B T_E N)
    QueryOperator *t5Op = (QueryOperator *)t5;
    QueryOperator *TNTABOp = (QueryOperator *)TNTAB;

    //cond
	AttributeReference *t6CondRef1 = createAttrsRefByName(t5Op, "NUMOPEN");
	AttributeReference *t6CondRef2 = createAttrsRefByName(TNTABOp, "N");
	t6CondRef2->fromClauseItem = 1;
	Operator *t6JoinCond = createOpExpr("<=", LIST_MAKE(t6CondRef1,t6CondRef2));

    List *t6JoinNames = deepCopyStringList(t5ProjNames);
    t6JoinNames = appendToTailOfList(t6JoinNames, "N");

    JoinOperator *t6Join = createJoinOp(JOIN_INNER,(Node *) t6JoinCond, LIST_MAKE(t5, TNTAB), NIL, t6JoinNames);
    t5Op->parents = singleton(t6Join);
    TNTABOp->parents = singleton(t6Join);

    //Top projection SELECT SALARY,T_B, T_E
    QueryOperator *t6JoinOp = (QueryOperator *) t6Join;

    AttributeDef *t6Def1 = (AttributeDef *) getHeadOfListP(t6JoinOp->schema->attrDefs);
    AttributeReference *t6Ref1 = createFullAttrReference(strdup(t6Def1->attrName), 0, 0, INVALID_ATTR, t6Def1->dataType);
	AttributeReference *t6Ref2 = createAttrsRefByName(t6JoinOp, TBEGIN_NAME);
	AttributeReference *t6Ref3 = createAttrsRefByName(t6JoinOp, TEND_NAME);
    List *t6ProjExpr = LIST_MAKE(t6Ref1,t6Ref2,t6Ref3);

    List *t6ProjNames = LIST_MAKE(t6Def1->attrName,TBEGIN_NAME,TEND_NAME);

    ProjectionOperator *t6 = createProjectionOp(t6ProjExpr, t6JoinOp, NIL, t6ProjNames);
    t6JoinOp->parents = singleton(t6);

	//---------------------------------------------------------------------------------------
    //Construct Top order by:    order by salary, TSTART
    QueryOperator *t6Op = (QueryOperator *) t6;

    AttributeDef *topDef1 = (AttributeDef *) getHeadOfListP(t6Op->schema->attrDefs);
    AttributeReference *topRef1 = createFullAttrReference(strdup(topDef1->attrName), 0, 0, INVALID_ATTR, topDef1->dataType);
	AttributeReference *topRef2 = createAttrsRefByName(t6Op, TBEGIN_NAME);

    OrderOperator *top = createOrderOp(LIST_MAKE(topRef1,topRef2),t6Op, NIL);
    t6Op->parents = singleton(top);

    setTempAttrProps((QueryOperator *) top);


    return (QueryOperator *) top;
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

