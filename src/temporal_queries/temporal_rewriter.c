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
#include "configuration/option.h"
#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/lateral_rewrites/lateral_prov_main.h"
#include "provenance_rewriter/unnest_rewrites/unnest_main.h"
#include "temporal_queries/temporal_rewriter.h"
#include "analysis_and_translate/translator_oracle.h"
#include "utility/string_utils.h"


static QueryOperator *temporalRewriteOperator(QueryOperator *op);
static QueryOperator *tempRewrSelection (SelectionOperator *o);
static QueryOperator *tempRewrProjection (ProjectionOperator *o);
static QueryOperator *tempRewrJoin (JoinOperator *op);
static QueryOperator *tempRewrGeneralAggregation (AggregationOperator *o);
static QueryOperator *tempRewrAggregation (AggregationOperator *o);
static QueryOperator *tempRewrTemporalSource (QueryOperator *o);
static QueryOperator *tempRewrSetOperator (SetOperator *o);
static QueryOperator *tempRewrNestedSubquery(NestingOperator *op);
static QueryOperator *tempRewrNestedSubqueryCorrelated(NestingOperator *op);
static QueryOperator *tempRewrNestedSubqueryLateralPostFilterTime(NestingOperator *op);
static QueryOperator *tempRewrDuplicateRemOp (DuplicateRemoval *o);

static QueryOperator *temporalLateralizeAndUnnestSubqueries(QueryOperator *root);
static QueryOperator *constructJoinIntervalIntersection(QueryOperator *op);
static Node *constructNestingIntervalOverlapCondition(QueryOperator *op);
static Node *constructIntervalOverlapCondition(QueryOperator *lChild, QueryOperator *rChild);
static void rewriteJoinChildren(QueryOperator **lChild, QueryOperator **rChild, QueryOperator *o);
static boolean isSetCoalesceSufficient(QueryOperator *q);
static void setTempAttrProps(QueryOperator *o);
static AttributeReference *getTempAttrRef (QueryOperator *o, boolean begin);
static void coalescingAndNormalizationVisitor (QueryOperator *q, Set *done);
static ProjectionOperator *createProjDoublingAggAttrs(QueryOperator *agg, int numNewAggs, boolean add, boolean isGB);
static List *getAttrRefsByNames (QueryOperator *op, List *attrNames);
static void markTemporalAttrsAsProv (QueryOperator *op);

#define ONE 1
#define ZERO 0
#define INT_MINVAL -2000000000
#define INT_MAXVAL 2000000000
#define ADD_AGG_PREFIX "ADD__"
#define DEC_AGG_PREFIX "DEC__"
#define WIN_PREFIX "W_"
#define TIMEPOINT_ATTR backendifyIdentifier("ts")
#define NEXT_TS_ATTR backendifyIdentifier("_next_ts")
#define OPEN_INTER_COUNT_ATTR backendifyIdentifier("open_inter_c_")
#define COUNT_START_ANAME backendifyIdentifier("count_start")
#define COUNT_END_ANAME backendifyIdentifier("count_end")
#define TS backendifyIdentifier("ts")
#define IS_S_NAME backendifyIdentifier("is_s")
#define IS_E_NAME backendifyIdentifier("is_e")
#define NUMOPEN backendifyIdentifier("numopen")

#define TNTAB_DUMMY_TABLE_NAME "__TNTAB_PLACEHOLDER"

#define FUNCNAME_LEAST backendifyIdentifier("least")
#define FUNCNAME_GREATEST backendifyIdentifier("greatest")

#define AGGNAME_SUM backendifyIdentifier("sum")
#define AGGNAME_COUNT backendifyIdentifier("count")
#define AGGNAME_AVG backendifyIdentifier("avg")
#define AGGNAME_MIN backendifyIdentifier("min")
#define AGGNAME_MAX backendifyIdentifier("max")
#define AGGNAME_LAST_VALUE backendifyIdentifier("last_value")
#define AGGNAME_LAG backendifyIdentifier("lag")
#define AGGNAME_LEAD backendifyIdentifier("lead")


static int T_BEtype = -1;

QueryOperator *
rewriteImplicitTemporal(QueryOperator *q)
{
    ASSERT(LIST_LENGTH(q->inputs) == 1);
    QueryOperator *top = getHeadOfListP(q->inputs);
    List *topSchema;
    boolean setCoalesce = FALSE;
    T_BEtype =  INT_VALUE(GET_STRING_PROP(q, PROP_TEMP_ATTR_DT));

	// rewrite subqueries into lateral and try to unnested them if we are asked to
	top = temporalLateralizeAndUnnestSubqueries(top);

    setCoalesce = isSetCoalesceSufficient(top);
    addCoalescingAndNormalization(top);

    top = temporalRewriteOperator(top);

    // make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    topSchema = copyObject(q->schema->attrDefs);
    disambiguiteAttrNames((Node *) top, done);
    if (!isA(top,ProjectionOperator))
    {
        QueryOperator *proj = createProjOnAllAttrs(top);
        addChildOperator(proj, top);
        switchSubtrees((QueryOperator *) top, proj);
        proj->schema->attrDefs = topSchema;
        top = proj;
        setTempAttrProps(top);
    }

    // add coalescing if requested
    if(getBoolOption(TEMPORAL_USE_COALSECE))
    {
        // check whether set coalesce is sufficient
        if (setCoalesce)
        {
            INFO_LOG("apply set coalesce");
            SET_BOOL_STRING_PROP(top,PROP_TEMP_DO_SET_COALESCE);
        }
        else
        {
            INFO_LOG("apply bag coalesce");
            SET_BOOL_STRING_PROP(top,PROP_TEMP_DO_COALESCE);
        }
        DEBUG_OP_LOG("mark top operator for coalescing", top);
        top = addCoalesceForAllOp(top); // addCoalesce(top);
    }

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) q, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root is:", top);

    return top;
}

static QueryOperator *
temporalLateralizeAndUnnestSubqueries(QueryOperator *root)
{
	Node *result = (Node *) root;

    if(isRewriteOptionActivated(OPTION_LATERAL_REWRITE) && !hasProvComputation(result))
	{
		result = lateralTranslateQBModel(result);
		INFO_AND_DEBUG_OP_LOG("subqueries rewritten into lateral", result);
	}

    if(isRewriteOptionActivated(OPTION_UNNEST_REWRITE) && !hasProvComputation(result))
	{
		result = unnestTranslateQBModel(result);
	    INFO_AND_DEBUG_OP_LOG("unnested subqueries", result);
	}

	return (QueryOperator *) result;
}

static boolean
isSetCoalesceSufficient(QueryOperator *q)
{
    List *keys;

    computeKeyProp(q);
    keys = (List *)getStringProperty(q, PROP_STORE_LIST_KEY);

    if (keys != NIL)
        return TRUE;

    return FALSE;
}

static QueryOperator *
temporalRewriteOperator(QueryOperator *op)
{
    QueryOperator *rewrittenOp = NULL;
    List *attrsConsts = (List *) GET_STRING_PROP(op, PROP_TEMP_NORMALIZE_INPUTS);
    boolean addNormalzation = HAS_STRING_PROP(op, PROP_TEMP_NORMALIZE_INPUTS);

    if (HAS_STRING_PROP(op, PROP_DUMMY_HAS_PROV_PROJ))
        rewrittenOp = tempRewrTemporalSource(op);
    else
    {
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
            {
                DEBUG_LOG("go aggregation");
                rewrittenOp = tempRewrGeneralAggregation((AggregationOperator *) op);
            }
            break;
            case T_JoinOperator:
                DEBUG_LOG("go join");
                rewrittenOp = tempRewrJoin((JoinOperator *) op);
                break;
            case T_SetOperator:
            {
                SetOperator *setOp = (SetOperator *) op;
                DEBUG_LOG("go set");//TODO check whether to use SetDiff + Normalize rewrite

                if (setOp->setOpType == SETOP_DIFFERENCE)
                	rewrittenOp = rewriteTemporalSetDiffWithNormalization((SetOperator *) op); //TODO
                else
                    rewrittenOp = tempRewrSetOperator((SetOperator *) op);
            }
                break;
		case T_NestingOperator:
		{

			NestingOperator *n = (NestingOperator *) op;
			rewrittenOp = tempRewrNestedSubquery(n);
		}
		break;
    //        case T_TableAccessOperator:
    //            DEBUG_LOG("go table access");
    //            rewrittenOp = tempRewrTableAccess((TableAccessOperator *) op);
    //            break;
    //        case T_ConstRelOperator:
    //            DEBUG_LOG("go const rel operator");
    //            rewrittenOp = tempRewrConstRel((ConstRelOperator *) op);
    //            break;
            case T_DuplicateRemoval:
                DEBUG_LOG("go duplicate removal operator");
                rewrittenOp = tempRewrDuplicateRemOp((DuplicateRemoval *) op);
                break;
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
    }

    if (addNormalzation)
    {
        List *attrs = NIL;

        FOREACH(Constant,c,attrsConsts)
        {
            attrs = appendToTailOfList(attrs, strdup(STRING_VALUE(c)));
        }

        if(getBoolOption(TEMPORAL_USE_NORMALIZATION_WINDOW))
        	    rewrittenOp = addTemporalNormalizationUsingWindow(rewrittenOp, rewrittenOp, attrs);
        else if(getBoolOption(TEMPORAL_USE_NORMALIZATION))
        	    rewrittenOp = addTemporalNormalization(rewrittenOp, rewrittenOp, attrs);

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
tempRewrJoin(JoinOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Join");
    QueryOperator *o = (QueryOperator *) op;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
    /* List *rNormAttrs; */
    /* int numLAttrs, numRAttrs; */
	Node *cond;

	rewriteJoinChildren(&lChild, &rChild, o);

    /* numLAttrs = LIST_LENGTH(lChild->schema->attrDefs); */
    /* numRAttrs = LIST_LENGTH(rChild->schema->attrDefs); */

    /* // get attributes from right input */
    /* rNormAttrs = sublist(o->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1); */
    /* o->schema->attrDefs = sublist(copyObject(o->schema->attrDefs), 0, numLAttrs - 1); */

    /* // rewrite children */
    /* lChild = temporalRewriteOperator(lChild); */
    /* rChild = temporalRewriteOperator(rChild); */

    /* // adapt schema for join op use */
    /* addProvenanceAttrsToSchema(o, lChild); */
    /* o->schema->attrDefs = CONCAT_LISTS(o->schema->attrDefs, rNormAttrs); */
    /* addProvenanceAttrsToSchema(o, rChild); */

    // add extra condition to join to check for interval overlap
    // left.begin <= right.begin <= right.end OR right.begin <= left.begin <= right.end
    /* AttributeReference *lBegin, *lEnd, *rBegin, *rEnd; */
    /* Node *cond; */

    /* lBegin = getTempAttrRef(lChild, TRUE); */
    /* lEnd = getTempAttrRef(lChild, FALSE); */
    /* rBegin = getTempAttrRef(rChild, TRUE); */
    /* rBegin->fromClauseItem = 1; */
    /* rEnd = getTempAttrRef(rChild, FALSE); */
    /* rEnd->fromClauseItem = 1; */

	/* // interval overlap join condition */
    /* cond = AND_EXPRS( */
    /*             (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(lBegin), copyObject(rEnd))), */
    /*             (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(rBegin), copyObject(lEnd))) */
    /*         ); */

	cond = constructIntervalOverlapCondition(lChild, rChild);

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


    // construct projection that intersect intervals
	QueryOperator *proj = constructJoinIntervalIntersection(o);
    /* List *temporalAttrProjs = NIL; */
    /* Node *tBegin; */
    /* Node *tEnd; */
    /* List *temporalAttrRefs = getProvAttrProjectionExprs((QueryOperator *) op); */

    /* lBegin = getNthOfListP(temporalAttrRefs, 0); */
    /* lEnd = getNthOfListP(temporalAttrRefs, 1); */
    /* rBegin = getNthOfListP(temporalAttrRefs, 2); */
    /* rEnd = getNthOfListP(temporalAttrRefs, 3); */

    /* tBegin = (Node *) createFunctionCall(FUNCNAME_GREATEST, LIST_MAKE(lBegin, rBegin)); */
    /* tEnd = (Node *) createFunctionCall(FUNCNAME_LEAST, LIST_MAKE(lEnd, rEnd)); */
    /* temporalAttrProjs = LIST_MAKE(tBegin, tEnd); */


    /* // add projection to put attributes into order on top of join op and computes the interval intersection */
    /* List *projExpr = CONCAT_LISTS( */
    /*         getNormalAttrProjectionExprs((QueryOperator *) op), */
    /*         temporalAttrProjs); */
    /* ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL); */

    /* addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op); */
    /* // use left inputs provenance attributes in join output as provenance attributes to be able to reuse code from provenance rewriting */
    /* o->provAttrs = sublist(o->provAttrs, 0, 1); */
    /* addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op); */


    /* // switch join with new projection */
    /* switchSubtrees((QueryOperator *) op, (QueryOperator *) proj); */
    /* addChildOperator((QueryOperator *) proj, (QueryOperator *) op); */

    /* setTempAttrProps((QueryOperator *) proj); */

    LOG_RESULT("Rewritten join", proj);
    return (QueryOperator *) proj;
}

#define JOIN_RIGHT_TEMP_ATTR_SUFFIX "_1"

static void
rewriteJoinChildren(QueryOperator **lChild, QueryOperator **rChild, QueryOperator *o)
{
	QueryOperator *l = *lChild;
	QueryOperator *r = *rChild;
	List *rNormAttrs;
	int numLAttrs, numRAttrs;

    numLAttrs = LIST_LENGTH(l->schema->attrDefs);
    numRAttrs = LIST_LENGTH(r->schema->attrDefs);

    // get attributes from right input
    rNormAttrs = sublist(o->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    o->schema->attrDefs = sublist(copyObject(o->schema->attrDefs), 0, numLAttrs - 1);

    // rewrite children
    *lChild = temporalRewriteOperator(l);
    *rChild = temporalRewriteOperator(r);

    // adapt schema for join op use
    addProvenanceAttrsToSchema(o, l);
    o->schema->attrDefs = CONCAT_LISTS(o->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchemaWithRename(o, r, JOIN_RIGHT_TEMP_ATTR_SUFFIX);
}

static Node *
constructIntervalOverlapCondition(QueryOperator *lChild, QueryOperator *rChild)
{
    AttributeReference *lBegin, *lEnd, *rBegin, *rEnd;
    Node *cond;

    lBegin = getTempAttrRef(lChild, TRUE);
    lEnd = getTempAttrRef(lChild, FALSE);
    rBegin = getTempAttrRef(rChild, TRUE);
    rBegin->fromClauseItem = 1;
    rEnd = getTempAttrRef(rChild, FALSE);
    rEnd->fromClauseItem = 1;

	// interval overlap join condition
    cond = AND_EXPRS(
                (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(lBegin), copyObject(rEnd))),
                (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(rBegin), copyObject(lEnd)))
            );

	return cond;
}

static Node *
constructNestingIntervalOverlapCondition(QueryOperator *op)
{
    AttributeReference *lBegin, *lEnd, *rBegin, *rEnd;
	List *tempAttrDefs = getAttrRefsByNames(op, getOpProvenanceAttrNames(op));
    Node *cond;

    lBegin = getNthOfListP(tempAttrDefs, 0);
    lEnd = getNthOfListP(tempAttrDefs, 1);
    rBegin = getNthOfListP(tempAttrDefs, 2);
    rEnd =  getNthOfListP(tempAttrDefs, 3);

	// interval overlap join condition
    cond = AND_EXPRS(
                (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(lBegin), copyObject(rEnd))),
                (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(rBegin), copyObject(lEnd)))
            );

	return cond;
}


static QueryOperator *
constructJoinIntervalIntersection(QueryOperator *op)
{
    AttributeReference *lBegin, *lEnd, *rBegin, *rEnd;
    List *temporalAttrProjs = NIL;
    Node *tBegin;
    Node *tEnd;
    List *temporalAttrRefs = getProvAttrProjectionExprs((QueryOperator *) op);

	// construct projection expressions that intersect intervals
    lBegin = getNthOfListP(temporalAttrRefs, 0);
    lEnd = getNthOfListP(temporalAttrRefs, 1);
    rBegin = getNthOfListP(temporalAttrRefs, 2);
    rEnd = getNthOfListP(temporalAttrRefs, 3);

    tBegin = (Node *) createFunctionCall(FUNCNAME_GREATEST, LIST_MAKE(lBegin, rBegin));
    tEnd = (Node *) createFunctionCall(FUNCNAME_LEAST, LIST_MAKE(lEnd, rEnd));
    temporalAttrProjs = LIST_MAKE(tBegin, tEnd);

	// add projection to put attributes into order on top of join op and computes the interval intersection
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) op),
            temporalAttrProjs);
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);
	proj->op.schema->attrDefs = NIL;

    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    // use left inputs provenance attributes in join output as provenance attributes to be able to reuse code from provenance rewriting
    op->provAttrs = sublist(op->provAttrs, 0, 1);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);

    // switch join with new projection
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    setTempAttrProps((QueryOperator *) proj);

    return (QueryOperator *) proj;
}


static QueryOperator *
tempRewrGeneralAggregation (AggregationOperator *o)
{
    QueryOperator *child = OP_LCHILD(o);
    QueryOperator *rewrittenOp;
    boolean minmax = GET_BOOL_STRING_PROP(child,PROP_TEMP_NORMALIZE_INPUTS);

    if(getBoolOption(TEMPORAL_AGG_WITH_NORM) && !minmax)
        rewrittenOp = rewriteTemporalAggregationWithNormalization((AggregationOperator *) o);
    else
        rewrittenOp = tempRewrAggregation ((AggregationOperator *) o);

    return rewrittenOp;
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
        {
            addProvenanceAttrsToSchema((QueryOperator *) o, (QueryOperator *) lOp);
        }
            break;
        case SETOP_DIFFERENCE:
        {
            addProvenanceAttrsToSchema((QueryOperator *) o, (QueryOperator *) lOp);
        }
            break;
    }

    setTempAttrProps((QueryOperator *) o);
    return (QueryOperator *) o;
}

static QueryOperator *
tempRewrNestedSubquery(NestingOperator *op)
{
	//TODO add other rewrite methods
    if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
    {
        int res = callback(2);
        if (res == 1) {
            return tempRewrNestedSubqueryLateralPostFilterTime(op);
        }
        return tempRewrNestedSubqueryCorrelated(op);
    }
    return tempRewrNestedSubqueryCorrelated(op);
    // return tempRewrNestedSubqueryLateralPostFilterTime(op);
}

// static Node *
// addConditionMutator (Node *node, Node *condition)
// {
//     if (node == NULL)
//         return NULL;

//     if(isA(node, SelectionOperator))
//     {
//         SelectionOperator *i = (SelectionOperator *)node;
//         if(i->cond) {
//             i->cond = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(i->cond, condition));
//         } else {
//             i->cond = condition;
//         }
//     }

//     return mutate(node, addConditionMutator, condition);
// }

static QueryOperator *
tempRewrNestedSubqueryCorrelated(NestingOperator *op)
{
    LOG_RESULT("before rewriting:", op);

    QueryOperator *asq = (QueryOperator *)op;

    QueryOperator *outer = OP_LCHILD(op);
	QueryOperator *inner = OP_RCHILD(op);
    int numOuterAttrs = getNumAttrs(OP_LCHILD(op)); // track pre-rewrite
    Schema *innerSchemaPreRewrite = copyObject(inner->schema);

    // List *innerAttrs = copyObject(inner->schema->attrDefs);

    // rewrite to add provenance attributes and include interval in selection

    outer = temporalRewriteOperator(outer);
    inner = temporalRewriteOperator(inner);

    // replace schema of nesting operator with outer attrs + nesting eval
    List *nestingEvals = copyObject(sublist(asq->schema->attrDefs, numOuterAttrs, -1));
    List *schema = concatTwoLists((List*)(copyObject(OP_LCHILD(op)->schema->attrDefs)), nestingEvals);

    addProvenanceAttrsToSchema(asq, outer); // we need the prov attrs for making overlap condition
    // addProvenanceAttrsToSchemaWithRename(asq, inner, JOIN_RIGHT_TEMP_ATTR_SUFFIX); // "

    List *intersection = NIL;
    FOREACH(char, attrR, getAttrNames(innerSchemaPreRewrite)) {
        FOREACH(char, attrL, getAttrNames(outer->schema)) {
            if (streq(attrR, attrL)) {
                intersection = appendToTailOfList(intersection, attrR);
            }
        }
    }

    outer = addTemporalNormalization(outer, copyObject(inner), intersection);

    // !!
    // reuse of overlap condition from lateral join, instead now we push it into the subquery
    // Node *correlation = constructNestingIntervalOverlapCondition(asq);
    // !!

    // gut of constructNestingIntervalOverlapCondition to change outer levels up value on outer attrrefs
    AttributeReference *lBegin, *lEnd, *rBegin, *rEnd;
	List *tempAttrDefs = getAttrRefsByNames(outer, getOpProvenanceAttrNames(outer));
    List *tempAttrDefs2 = getAttrRefsByNames(inner, getOpProvenanceAttrNames(inner));
    Node *correlation;

    lBegin = getNthOfListP(tempAttrDefs, 0);
    lEnd = getNthOfListP(tempAttrDefs, 1);
    rBegin = getNthOfListP(tempAttrDefs2, 0);
    rEnd =  getNthOfListP(tempAttrDefs2, 1);

    lBegin->outerLevelsUp = 1;
    lEnd->outerLevelsUp = 1;

	// interval overlap join condition
    correlation = AND_EXPRS(
                (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(lBegin), copyObject(rEnd))),
                (Node *) createOpExpr(OPNAME_LE, LIST_MAKE(copyObject(rBegin), copyObject(lEnd)))
            );

    // todo: robustness
    // add the correlation into the subquery
    // addConditionMutator((Node *)inner, correlation);

    QueryOperator *selection = (QueryOperator *)createSelectionOp(correlation, inner, NIL, NIL);
    selection->schema = copyObject(selection->schema);
    selection->provAttrs = copyObject(inner->provAttrs);
    inner->parents = appendToTailOfList(inner->parents, selection);
    switchSubtrees(inner, selection);

    // return a projection on top of "asq" to reorder schema

    Schema *nestingSchema = copyObject(asq->schema);

    asq->schema->attrDefs = schema;
    asq->provAttrs = copyObject(outer->provAttrs);

	QueryOperator *pOp = createProjOnAllAttrs(asq);
    ProjectionOperator *projection = (ProjectionOperator *) pOp;
	List *proj = NIL, *provProj = NIL;
	int pos = 0;

	FOREACH(AttributeReference,a,projection->projExprs)
	{
		if(searchListInt(asq->provAttrs, pos++))
		{
			provProj = appendToTailOfList(provProj, a);
		}
		else
		{
			proj = appendToTailOfList(proj, a);
		}
	}

	projection->projExprs = CONCAT_LISTS(proj, provProj);

    pOp->schema = nestingSchema;
    pOp->provAttrs = NIL;
    pOp->provAttrs = appendToTailOfListInt(
		pOp->provAttrs,
		getListLength(pOp->schema->attrDefs)-2);
    pOp->provAttrs = appendToTailOfListInt(
		pOp->provAttrs,
		getListLength(pOp->schema->attrDefs)-1);

    addChildOperator(pOp, asq);
    switchSubtrees(asq, pOp);

    LOG_RESULT("after rewriting:", projection);
    return pOp;
}

static QueryOperator *
tempRewrNestedSubqueryLateralPostFilterTime(NestingOperator *op)
{
	QueryOperator *o = (QueryOperator *) op;
	QueryOperator *outer = OP_LCHILD(op);
	QueryOperator *inner = OP_RCHILD(op);
	QueryOperator *rewritten;
	Node *overlapCond;

	// rewrite children and adapt nesting operator's schema
	rewriteJoinChildren(&outer, &inner, o);

	// add selection for interval overlap condition
	overlapCond = constructNestingIntervalOverlapCondition(o);
	rewritten = (QueryOperator *) createSelectionOp(overlapCond, o, NIL, NIL);
	rewritten->provAttrs = copyList(o->provAttrs);

    // switch nesting operator with new selection
    switchSubtrees((QueryOperator *) op, (QueryOperator *) rewritten);
	addParent(o, rewritten);

    setTempAttrProps((QueryOperator *) rewritten);

	// add projection to intersect intervals
	rewritten = constructJoinIntervalIntersection(rewritten);

	LOG_RESULT("nested subquery", rewritten);
	return rewritten;
}

static QueryOperator *
tempRewrDuplicateRemOp (DuplicateRemoval *o)
{
    QueryOperator *child = OP_LCHILD(o);
    QueryOperator *newAgg;
    QueryOperator *rewrittenOp;
    List *groupBy;
    List *attrNames;

    attrNames = getNormalAttrNames(child);
    groupBy = getNormalAttrProjectionExprs(child);

    newAgg = (QueryOperator *) createAggregationOp(NIL, groupBy, child, NIL, attrNames);
    removeParentFromOps(LIST_MAKE(child), (QueryOperator *) o);
    child->parents = appendToTailOfList(child->parents, newAgg);
    switchSubtreeWithExisting((QueryOperator *) o, newAgg);
    newAgg->inputs = o->op.inputs;

    rewrittenOp = tempRewrGeneralAggregation((AggregationOperator *) newAgg);

    return rewrittenOp;
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
    result = createFullAttrReference(strdup(d->attrName), 0, pos, INVALID_ATTR, T_BEtype); //TODO use actual temporal DT
    return result;
}

/*
 * determine where in a temporal query we have to apply the two temporal normalization
 * operations "coalesce" and "align". This method only marks operators that should
 * be subjected to one of these normalization operators.
 */
void
addCoalescingAndNormalization (QueryOperator *q)
{
    Set *done = PSET();
    // mark children first

    coalescingAndNormalizationVisitor(q, done);
}

static void
coalescingAndNormalizationVisitor (QueryOperator *q, Set *done)
{
    if (hasSetElem(done,q))
        return;

    FOREACH(QueryOperator,child,q->inputs)
        coalescingAndNormalizationVisitor(child, done);

    switch(q->type)
    {
        case T_AggregationOperator:
        {
            boolean minmax = FALSE;
            if(isA(q, AggregationOperator))
            {
                AggregationOperator *aggOp = (AggregationOperator *) q;

                FOREACH(FunctionCall, fc, aggOp->aggrs)
                {
                    char *upcaseName = strToUpper(fc->functionname);
                    if(streq(upcaseName,AGGNAME_MIN) || streq(upcaseName,AGGNAME_MAX))
                    {
                        minmax = TRUE;
                        break;
                    }
                }
            }

            if (!getBoolOption(TEMPORAL_AGG_WITH_NORM) || minmax) //TODO check that not min or max
            {
                QueryOperator* child = OP_LCHILD(q);
                AggregationOperator *a = (AggregationOperator *) q;
                List *attrs = NIL;

                FOREACH(AttributeReference,g,a->groupBy)
                {
                    char *attrName = getAttrNameByPos(child, g->attrPosition);

                    attrs = appendToTailOfList(attrs, createConstString(attrName));
                }

                if(attrs == NIL)
                    attrs = appendToTailOfList(attrs, createConstString("!EMPTY!"));

                SET_STRING_PROP(child,PROP_TEMP_NORMALIZE_INPUTS, attrs);
                DEBUG_OP_LOG("mark aggregation input for normalization", q);
            }
            // otherwise we need to normalize
            else
            {
                DEBUG_LOG("use combined aggregation+normalization, no separate normalization is required");
            }
        }
        break;
        case T_SetOperator:
        {
            SetOperator *s = (SetOperator *) q;//TODO not do this anymore if SET DIFF
            //if (s->setOpType == SETOP_DIFFERENCE || s->setOpType == SETOP_INTERSECTION)
            if (s->setOpType == SETOP_INTERSECTION)
            {
                //add all attributes
                List *attrs = getAttrNames(q->schema);
                SET_STRING_PROP(q,PROP_TEMP_NORMALIZE_INPUTS, attrs);
                DEBUG_OP_LOG("mark setop for normalization of inputs", q);
            }
        }
        break;
        default:
            break;
    }

    addToSet(done, q);
}

/* add coalesce for each operator */
QueryOperator*
addCoalesceForAllOp(QueryOperator *op)
{
	QueryOperator *result = op;

	if(op->inputs != NIL)
	{
		FOREACH(QueryOperator,c,op->inputs)
			addCoalesceForAllOp(c);

		if(GET_STRING_PROP(op, PROP_TEMP_DO_COALESCE))
			result = addCoalesce(op);
		else if (GET_STRING_PROP(op, PROP_TEMP_DO_SET_COALESCE))
		    result = addSetCoalesce(op);
	}

	return result;
}


/*
 * adds algebra expressions to coalesce the output of an operator
 *
 * Set coalescing is only applicable if the output of the operator does not
 * contain any duplicates
 */
QueryOperator *
addSetCoalesce (QueryOperator *input)
{
    QueryOperator *op = input;
    List *parents = op->parents;
    Constant *c0 = createConstInt(ZERO);
    Constant *c1 = createConstInt(ONE);
    List *norAttrnames = getNormalAttrNames(op);

    //****************************************
    // Construct T1: a union of projections on start and end points of intervals
    // result schema: normalattrs is_start is_end timestamp

    //construct projExprs (NORMAL ATTRS 1 0 T_B )
    //construct projExprs (NORMAL ATTRS 0 1 T_E )
    List *t1NorProjExpr1 = getNormalAttrProjectionExprs(op);
    List *t1NorProjExpr2 = getNormalAttrProjectionExprs(op);

    List *t1ProjExpr1 = t1NorProjExpr1;
    t1ProjExpr1 = appendToTailOfList(t1ProjExpr1, copyObject(c1));
    t1ProjExpr1 = appendToTailOfList(t1ProjExpr1, copyObject(c0));

    List *t1ProjExpr2 = t1NorProjExpr2;
    t1ProjExpr2 = appendToTailOfList(t1ProjExpr2, copyObject(c0));
    t1ProjExpr2 = appendToTailOfList(t1ProjExpr2, copyObject(c1));

    AttributeReference *t1BeginRef = getAttrRefByName(op, TBEGIN_NAME);
    AttributeReference *t1EndRef = getAttrRefByName(op, TEND_NAME);

    t1ProjExpr1 = appendToTailOfList(t1ProjExpr1, t1BeginRef);
    t1ProjExpr2 = appendToTailOfList(t1ProjExpr2, t1EndRef);

    //construct schema NORMALATTRS S E TS  for BOTH
    List *t1AttrNames = getNormalAttrNames(op);
    t1AttrNames = appendToTailOfList(t1AttrNames,strdup(IS_S_NAME));
    t1AttrNames = appendToTailOfList(t1AttrNames,strdup(IS_E_NAME));
    t1AttrNames = appendToTailOfList(t1AttrNames,strdup(TS));

    ProjectionOperator *t1Proj1 = createProjectionOp(t1ProjExpr1, op, NIL, deepCopyStringList(t1AttrNames));
    ProjectionOperator *t1Proj2 = createProjectionOp(t1ProjExpr2, op, NIL, deepCopyStringList(t1AttrNames));
    op->parents = LIST_MAKE(t1Proj1,t1Proj2);

    //construct union on top
    QueryOperator *t1 = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(t1Proj1, t1Proj2), NIL,
            deepCopyStringList(t1AttrNames));
    ((QueryOperator *) t1Proj1)->parents = singleton(t1);
    ((QueryOperator *) t1Proj2)->parents = singleton(t1);

    //****************************************
    // Construct T2: add window function to calculate number of intervals started/ended before TS
    /*
     * T2 AS (
     * SELECT NORMAL_ATTRS, IS_S, IS_E, TS
     *      SUM(IS_S) OVER (PARTITION BY A, B ORDER BY TS, IS_E ROWS UNBOUNDED PRECEDING) cS,
     *      SUM(IS_E) OVER (PARTITION BY A, B ORDER BY TS, IS_E ROWS UNBOUNDED PRECEDING) cE,
     *   FROM T1
     * ),
     */
    //WindowDefinition OVER (PARTITION BY normal_attrs ORDER BY TS, IS_E ROWS UNBOUNDED PRECEDING)
    WindowBound *winBoundCountOpen = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
    WindowFrame *winFrameCountOpen = createWindowFrame(WINFRAME_ROWS,winBoundCountOpen,NULL);

    // partition-by
    List *partByCountOpen = getAttrRefsByNames(t1, norAttrnames);

    // order-by
    AttributeReference *countOpenTSRef = getAttrRefByName(t1, TS);
    AttributeReference *countOpenIsERef = getAttrRefByName(t1, IS_E_NAME);
    List *countOpenOrderBy = LIST_MAKE(countOpenTSRef, countOpenIsERef);

    // create window operator that counts open intervals
    AttributeReference *isSRef = getAttrRefByName(t1, IS_S_NAME);
    FunctionCall *sumS = createFunctionCall(AGGNAME_SUM,singleton(isSRef));

    char *countOpenName = strdup(COUNT_START_ANAME);
    WindowOperator *winCountOpen = createWindowOp((Node *) sumS,
            partByCountOpen,
            countOpenOrderBy,
            winFrameCountOpen,
            countOpenName,
            t1,
            NIL);
    t1->parents = singleton(winCountOpen);

    // create window operator that counts closed intervals
    AttributeReference *countClosedIsERef = getAttrRefByName(t1, IS_E_NAME);
    FunctionCall *sumE = createFunctionCall(AGGNAME_SUM,singleton(countClosedIsERef));

    char *countCloseName = strdup(COUNT_END_ANAME);
    WindowOperator *winCountClosed = createWindowOp((Node *) sumE,
            copyObject(partByCountOpen),
            copyObject(countOpenOrderBy),
            copyObject(winFrameCountOpen),
            countCloseName,
            (QueryOperator *) winCountOpen,
            NIL);
    winCountOpen->op.parents = singleton((QueryOperator *) winCountClosed);

    QueryOperator *t2 = (QueryOperator *) winCountClosed;

    //****************************************
    // Construct T3: compute adjacent changepoints
    /*
     * T3 AS (
     *  SELECT normal_attrs, cS, cE, LAG(TS, 1) OVER (ORDER BY normal_attrs, TS) AS TBEGIN, TS AS TEND
     * FROM T2
     * WHERE cS = cE
     *       OR cS - IS_S = cE - IS_E
     *  )
     */
    Node *cond, *startDiff, *endDiff;
    QueryOperator *selCPs;

    startDiff = (Node *) createOpExpr("-", LIST_MAKE(getAttrRefByName(t2, COUNT_START_ANAME), getAttrRefByName(t2, IS_S_NAME)));
    endDiff = (Node *) createOpExpr("-", LIST_MAKE(getAttrRefByName(t2, COUNT_END_ANAME), getAttrRefByName(t2, IS_E_NAME)));
    cond = OR_EXPRS(
            (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(t2, COUNT_START_ANAME),
                              getAttrRefByName(t2, COUNT_END_ANAME))),
            (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(startDiff, endDiff))
            );
    selCPs = (QueryOperator *) createSelectionOp(cond, t2, NIL, NIL);
    t2->parents = singleton(selCPs);

    // construct window operator to compute adjacent changepoint with LAG
    // order-by
    List *orderByCPs = getAttrRefsByNames(t2, norAttrnames);
    AttributeReference *cpsTSRef = getAttrRefByName(t2, TS);
    orderByCPs = appendToTailOfList(orderByCPs, cpsTSRef);

    // create window operator fetches previous change point
    FunctionCall *lagTS = createFunctionCall(AGGNAME_LAG,
            LIST_MAKE(getAttrRefByName(t1, TS), createConstInt(1)));

    char *lagName = strdup(TBEGIN_NAME);
    WindowOperator *winCPs = createWindowOp((Node *) lagTS,
            NIL,
            orderByCPs,
            NULL,
            lagName,
            (QueryOperator *) selCPs,
            NIL);
    selCPs->parents = singleton(winCPs);
    QueryOperator *t3 = (QueryOperator *) winCPs;

    //****************************************
    // Construct T4: project on normal attributes and interval endpoints
    /*
     * T3 AS (
     *  SELECT normal_attrs, TBEGIN, TEND FROM T3 WHERE cS = cE
     *  )
     */
    QueryOperator *finalSel;
    cond = (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(getAttrRefByName(t3, COUNT_START_ANAME),
            getAttrRefByName(t3, COUNT_END_ANAME)));
    finalSel = (QueryOperator *) createSelectionOp(cond, t3, NIL, NIL);
    t3->parents = singleton(finalSel);

    // construct final projection
    QueryOperator *finalProj;
    List *finalNames = deepCopyStringList(norAttrnames);
    List *inAttrs = deepCopyStringList(norAttrnames);

    inAttrs = appendToTailOfList(inAttrs, strdup(TBEGIN_NAME));
    inAttrs = appendToTailOfList(inAttrs, strdup(TS));

    finalNames = appendToTailOfList(finalNames, strdup(TBEGIN_NAME));
    finalNames = appendToTailOfList(finalNames, strdup(TEND_NAME));

    finalProj = createProjOnAttrsByName(finalSel, inAttrs, finalNames);
    finalProj->inputs = singleton(finalSel);
    finalSel->parents = singleton(finalProj);

    // replace with original operator and mark temporal attributes
    substOpInParents(parents, op, finalProj);
    setTempAttrProps(finalProj);
    markTemporalAttrsAsProv(finalProj);

    return finalProj;
}

static List *
getAttrRefsByNames (QueryOperator *op, List *attrNames)
{
    List *result = NIL;

    FOREACH(char, c, attrNames)
    {
        AttributeReference *normARef = getAttrRefByName(op, c);
        result = appendToTailOfList(result,normARef);
    }

    return result;
}


/*
 * add algebra expressions to bag-coalesce the output of an operator
 *
 * T1 - union of count of starting and ending tuples with same values for each time point
 *      this computes all "change points", i.e,. interval start or end points
 *
 * WITH T0 (tstart, tend, A, B) AS (
      SELECT T_BEGIN AS TSTART, T_END AS TEND, A, B
      FROM TEMP_TEST WHERE A = 1 AND B = 1
    ),
    T1 (Start_ts, End_ts, ts, A,B) AS (
      SELECT 1, 0, TSTART, A, B
      FROM T0
      UNION ALL
      SELECT 0, 1, TEND, A,B
      FROM T0
    )
 *
 */
QueryOperator *
addCoalesce (QueryOperator *input)
{

	QueryOperator *op = input;
	List *parents = op->parents;

	//---------------------------------------------------------------------------------------
    // Construct T1: a union on two projections
    Constant *c0 = createConstInt(ZERO);
    Constant *c1 = createConstInt(ONE);
    List *norAttrnames = getNormalAttrNames(op);

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
    int attrPos = 0;

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
    //Construct T2: SELECT DISTINCT A, B, SUM(T_B) - SUM(T_E), TS

    //Window 1 (SUM(S_B), PARTITION BY salary, GROUP_BY TS, RANGE UNBOUNDED PRECEDING
    WindowBound *wbT2W1 = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
    WindowFrame *wfT2W1 = createWindowFrame(WINFRAME_RANGE,wbT2W1,NULL);

    QueryOperator *t1Op = (QueryOperator *) t1;

    // partion-by
    List *t2PartitionBy1 = NIL;
    attrPos = 0;
    FOREACH(char, c, norAttrnames)
    {
        AttributeDef *t2PBDef1 = getAttrDefByName(t1Op, c);
        AttributeReference *t2PBRef1 = createFullAttrReference(strdup(t2PBDef1->attrName), 0, attrPos, INVALID_ATTR, t2PBDef1->dataType);
        t2PartitionBy1 = appendToTailOfList(t2PartitionBy1,t2PBRef1);
        attrPos++;
    }

    // order-by
    AttributeDef *t2GBDef1 = (AttributeDef *) getTailOfListP(t1Op->schema->attrDefs);
    AttributeReference *t2GBRef1 = createFullAttrReference(strdup(t2GBDef1->attrName), 0, LIST_LENGTH(t1Op->schema->attrDefs) - 1, INVALID_ATTR, t2GBDef1->dataType);
    List *t2GroupBy1 = singleton(t2GBRef1);

    WindowDef *t2WD1 = createWindowDef(t2PartitionBy1,t2GroupBy1,wfT2W1);

    AttributeDef *t1TBDef = copyObject(getAttrDefByName(t1Op, TBEGIN_NAME));
    int t1TBDefPos = getAttrPos(op, t1TBDef->attrName);
    AttributeReference *t2FCRef = createFullAttrReference(strdup(t1TBDef->attrName), 0, t1TBDefPos, INVALID_ATTR, t1TBDef->dataType);
    FunctionCall *t2FC = createFunctionCall(AGGNAME_SUM,singleton(t2FCRef));

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
    List *t2PartitionBy2 = getAttrRefsByNames(t2W1Op, norAttrnames);

    //groupBy
    AttributeDef *t2GBDef2 = (AttributeDef *) getAttrDefByName(t2W1Op, TS);
    AttributeReference *t2GBRef2 = createFullAttrReference(strdup(t2GBDef2->attrName), 0, LIST_LENGTH(t1Op->schema->attrDefs) - 1, INVALID_ATTR, t2GBDef2->dataType);
    List *t2GroupBy2 = singleton(t2GBRef2);

    WindowDef *t2WD2 = createWindowDef(t2PartitionBy2,t2GroupBy2,wfT2W2);

    AttributeReference *t2W2FCRef = getAttrRefByName(t2W1Op, TEND_NAME);
    FunctionCall *t2W2FC = createFunctionCall(AGGNAME_SUM,singleton(t2W2FCRef));

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
    List *t2ProjExpr = getAttrRefsByNames(t2W1Op, norAttrnames);

    //sum(w_0) - sum(w_1)
    AttributeReference *t2Winf0 = getAttrRefByName(t2W2Op, "winf_0");
    AttributeReference *t2Winf1 = getAttrRefByName(t2W2Op, "winf_1");
    List *t2MinusExpr = LIST_MAKE(t2Winf0,t2Winf1);
    Operator *t2Minus = createOpExpr("-",t2MinusExpr);
    t2ProjExpr = appendToTailOfList(t2ProjExpr, t2Minus);

    //TS
    AttributeReference *t2TS = getAttrRefByName(t2W2Op, TS);
    t2ProjExpr = appendToTailOfList(t2ProjExpr, t2TS);
    List *T2AttrNames = LIST_MAKE(NUMOPEN,strdup(TS));
    T2AttrNames = concatTwoLists(deepCopyStringList(norAttrnames),T2AttrNames);

    ProjectionOperator *t2topP = createProjectionOp(t2ProjExpr, t2W2Op, NIL, T2AttrNames);
    t2W2Op->parents = singleton(t2topP);

    DuplicateRemoval *t2;
    t2 = createDuplicateRemovalOp(NIL, (QueryOperator *) t2topP, NIL, NIL);
    addParent((QueryOperator *) t2topP, (QueryOperator *) t2);

	//---------------------------------------------------------------------------------------
    //Construct T3: (diffFollowing, diffPrevious, numOpen, ts, salary)
    QueryOperator *t2Op = (QueryOperator *) t2;

    //window 1 : LEAD(numOpen) OVER (PARTITION BY salary ORDER BY ts ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING

    //partationBy
    List *t3PartitionBy1 = getAttrRefsByNames(t2Op, norAttrnames);

    //groupBy
    AttributeDef *t3gbDef1 = (AttributeDef *) getTailOfListP(t2Op->schema->attrDefs);
    AttributeReference *t3gbRef1 = createFullAttrReference(strdup(t3gbDef1->attrName), 0, LIST_LENGTH(t2Op->schema->attrDefs) - 1, INVALID_ATTR, t3gbDef1->dataType);
    List *t3GroupBy1 = singleton(t3gbRef1);

    WindowDef *t3wd1 = createWindowDef(t3PartitionBy1,t3GroupBy1,NULL);

    AttributeReference *t3fcRef = getAttrRefByName(t2Op, NUMOPEN);
    FunctionCall *t3fc = createFunctionCall(AGGNAME_LEAD,singleton(t3fcRef));

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

    //partationBy
    List *t3PartitionBy2 = getAttrRefsByNames(t3w1Op, norAttrnames);

    //groupBy
    AttributeReference *t3gpRef2 = getAttrRefByName(t3w1Op, TS);
    List *t3GroupBy2 = singleton(t3gpRef2);

    WindowDef *t3wd2 = createWindowDef(t3PartitionBy2,t3GroupBy2,NULL);

    AttributeReference *t3fcRef2 = getAttrRefByName(t3w1Op, NUMOPEN);
    FunctionCall *t3fc2 = createFunctionCall(AGGNAME_LAG,singleton(t3fcRef2));

    WindowFunction *t3wff = createWindowFunction(t3fc2,t3wd2);

    char *t3ANamef = "winf_1";
    WindowOperator *t3w2 = createWindowOp(copyObject(t3wff->f),
    		copyObject(t3wff->win->partitionBy),
			copyObject(t3wff->win->orderBy),
			copyObject(t3wff->win->frame),
			t3ANamef, t3w1Op, NIL);
    t3w1Op->parents = singleton(t3w2);

    //projection :  Projection[ATTRS COALESCE((NUMOPEN - W0),small_value) COALESCE((NUMOPEN - W1),small_value) NUMOPEN TS ]
    //(DIFFFOLLOWING DIFFPREVIOUS NUMOPEN TS SALARY)
    QueryOperator *t3w2Op = (QueryOperator *)t3w2;
    Constant *c6 = createConstInt(INT_MINVAL);
    List *t3ProjExpr = getAttrRefsByNames(t3w2Op, norAttrnames);

    //COALESCE((NUMOPEN - winf_0),666)
    AttributeReference *t3ProjNOpen1 = getAttrRefByName(t3w2Op, NUMOPEN);

    //COALESCE((NUMOPEN - winf_1),666)
    AttributeReference *t3ProjNOpen2 = copyObject(t3ProjNOpen1);
    AttributeReference *t3ProjW02 = getAttrRefByName(t3w2Op, "winf_1");
    Operator *t3O2 = createOpExpr("-", LIST_MAKE(t3ProjNOpen2,t3ProjW02));
    FunctionCall *t3Projfc2 = createFunctionCall(COALESCE_FUNC_NAME,LIST_MAKE(t3O2,copyObject(c6)));
    t3ProjExpr = appendToTailOfList(t3ProjExpr, t3Projfc2);

    //numOpen, TS
    t3ProjExpr = appendToTailOfList(t3ProjExpr, copyObject(t3ProjNOpen1));
    AttributeReference *t3ProjTS = getAttrRefByName(t3w2Op, TS);
    t3ProjExpr = appendToTailOfList(t3ProjExpr, t3ProjTS);

    //Proj names
    List *t3ProjAttrNames = LIST_MAKE("DIFFPREVIOUS", NUMOPEN, TIMEPOINT_ATTR);
    t3ProjAttrNames = concatTwoLists(deepCopyStringList(norAttrnames),t3ProjAttrNames);

    ProjectionOperator *t3Proj = createProjectionOp(t3ProjExpr, t3w2Op, NIL, t3ProjAttrNames);
    t3w2Op->parents = singleton(t3Proj);

	//---------------------------------------------------------------------------------------
    //Construct T4:   SELECT * FROM T3 SELECTION: WHERE diffFollowing != 0 OR diffPrevious != 0
	QueryOperator *t3Op = (QueryOperator *) t3Proj;

	AttributeReference *t4dpRef = getAttrRefByName(t3Op, "DIFFPREVIOUS");

    Operator *t4O2 = createOpExpr("!=", LIST_MAKE(t4dpRef,copyObject(c0)));

    SelectionOperator *t4 = createSelectionOp((Node *)t4O2, t3Op, NIL, deepCopyStringList(t3ProjAttrNames));
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
    List *t5PartitionBy = getAttrRefsByNames(t4Op, norAttrnames);

    //groupBy
    AttributeReference *t5gbRef = getAttrRefByName(t4Op, TS);
    List *t5GroupBy = singleton(t5gbRef);

    WindowDef *t5wd = createWindowDef(t5PartitionBy,t5GroupBy,t5wf);

    FunctionCall *t5fc = createFunctionCall(AGGNAME_LAST_VALUE,singleton(copyObject(t5gbRef)));

    WindowFunction *t5wff = createWindowFunction(t5fc,t5wd);

    char *t5AName = "winf_0";
    WindowOperator *t5w = createWindowOp(copyObject(t5wff->f),
            copyObject(t5wff->win->partitionBy),
            copyObject(t5wff->win->orderBy),
            copyObject(t5wff->win->frame),
			t5AName, t4Op, NIL);
    t4Op->parents = singleton(t5w);

    QueryOperator *t5wOp = (QueryOperator *) t5w;
    AttributeDef *winf0Def = getAttrDefByName(t5wOp, t5AName);
    winf0Def->dataType = t5gbRef->attrType; // DT_INT;

    //Projection: SELECT  salary, diffFollowing, diffPrevious, numOpen, ts as t_b, winf_0 as t_e

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

	AttributeReference *t5CondRef2 = getAttrRefByName(t5ProjOp, TEND_NAME);

    IsNullExpr *t5O2 = createIsNullExpr((Node *) singleton(t5CondRef2));
    Operator *t5O3 = createOpExpr(OPNAME_NOT, singleton(t5O2));

    SelectionOperator *t5 = createSelectionOp((Node *) t5O3, t5ProjOp, NIL, deepCopyStringList(t5ProjNames));
    t5ProjOp->parents = singleton(t5);

    QueryOperator *t5Op = (QueryOperator *)t5;

    //-----------------------------------------------------------------------------------------------------------------
    //TNTAB AS (SELECT rownum n from dual connect by level <= (SELECT max(numOpen) FROM T5))
	TableAccessOperator *TNTAB = createTableAccessOp(TNTAB_DUMMY_TABLE_NAME, NULL, "TNTAB", NIL, singleton("N"), singletonInt(DT_INT));

	//set boolean prop (when translate to SQL, translate to above SQL not this table)
	//SET_STRING_PROP(TNTAB, PROP_TEMP_TNTAB, createConstLong((gprom_long_t) top1));
	SET_STRING_PROP(TNTAB, PROP_TEMP_TNTAB, createConstLong((gprom_long_t) t5));
	// ensure that t5 is treated as a common table expression
	SET_BOOL_STRING_PROP(t5, PROP_MATERIALIZE);
	//---------------------------------------------------------------------------------------
    //Construct T6:    SELECT TSTART, TEND, SALARY FROM T5, TNTAB WHERE T5.numOpen <= n

	//join cond = numOpen <= N (SALARY DIFFFOLLOWING DIFFPREVIOUS NUMOPEN T_B T_E N)
    QueryOperator *TNTABOp = (QueryOperator *)TNTAB;
    //QueryOperator *top1Op = (QueryOperator *)t5Op;

    //cond
	AttributeReference *t6CondRef1 = getAttrRefByName(t5Op, NUMOPEN);
	AttributeReference *t6CondRef2 = getAttrRefByName(TNTABOp, "N");
	t6CondRef2->fromClauseItem = 1;
	Operator *t6JoinCond = createOpExpr(OPNAME_GE, LIST_MAKE(t6CondRef1,t6CondRef2));

    List *t6JoinNames = deepCopyStringList(t5ProjNames);
    t6JoinNames = appendToTailOfList(t6JoinNames, "N");

    JoinOperator *t6Join = createJoinOp(JOIN_INNER,(Node *) t6JoinCond, LIST_MAKE(t5, TNTAB), NIL, t6JoinNames);
    t5Op->parents = singleton(t6Join);
    TNTABOp->parents = singleton(t6Join);

    //Top projection SELECT SALARY,T_B, T_E
    QueryOperator *t6JoinOp = (QueryOperator *) t6Join;

    List *t6ProjExpr = getAttrRefsByNames(t6JoinOp, norAttrnames);

	AttributeReference *t6Ref2 = getAttrRefByName(t6JoinOp, TBEGIN_NAME);
	AttributeReference *t6Ref3 = getAttrRefByName(t6JoinOp, TEND_NAME);
    t6ProjExpr = appendToTailOfList(t6ProjExpr, t6Ref2);
    t6ProjExpr = appendToTailOfList(t6ProjExpr, t6Ref3);

    List *t6ProjNames = LIST_MAKE(TBEGIN_NAME,TEND_NAME);
    t6ProjNames = concatTwoLists(deepCopyStringList(norAttrnames),t6ProjNames);

    ProjectionOperator *top = createProjectionOp(t6ProjExpr, t6JoinOp, NIL, t6ProjNames);
    t6JoinOp->parents = singleton(top);

    QueryOperator *topOp = (QueryOperator *) top;

    // replace topOp for op in  original parents of op and mark temporal attrs
    substOpInParents(parents, op, topOp);
    setTempAttrProps(topOp);
    markTemporalAttrsAsProv(topOp);

    return (QueryOperator *) top;
}

static void
markTemporalAttrsAsProv (QueryOperator *op)
{
    op->provAttrs = appendToTailOfListInt(op->provAttrs, getAttrPos(op, TBEGIN_NAME));
    op->provAttrs = appendToTailOfListInt(op->provAttrs, getAttrPos(op, TEND_NAME));
}

//TODO add normalization specialized for min/max with only two-way union and preaggregation computing min/max group by G, TSTART, TEND as input for normalization

/*
 *
 */

/*
 * normalize "input" with respect to "reference" and a set of attribute G.
 * This operation split the interval assigned to a tuple t based on the all the interval end points
 * that exists within the same group as t (grouping on G). In the result all intervals assigned to
 * tuples from one group are either disjunct or equal.
 *
 * This normalization has to be applied to the inputs of set difference
 * for R - S we rewrite it into A(R,S) - A(S,R). Also this can be used to
 * prepare the input of a temporal aggregation such that the aggregation
 * can then be evaluated by using standard aggregation grouping on the original
 * group by attributes plus the interval start and end point attributes.
 *
 *
 * -- normalize(R,S) ON X means split intervals of R such that all intervals in the result that have the same value for X are either equal or disjoint.
 * -- this is achieved by splitting intervals in R at change points (starting or end points of intervals in R or S that have the same value for X)
 * ----------------------------------------
 * -- join based implementation using window functions
 * ----------------------------------------
 */

// should take 2 lists of attributes, outer & inner
QueryOperator *
addTemporalNormalization (QueryOperator *input, QueryOperator *reference, List *attrNames)
{
	if(LIST_LENGTH(attrNames) == 1 && streq(getHeadOfListP(attrNames),"!EMPTY!"))
		attrNames = NIL;

	QueryOperator *left = input;
	QueryOperator *right = reference;
	List *parents = left->parents;
	List *newParents;

    DEBUG_OP_LOG("add join-based temporal normalization for operator ", input, reference);

	//---------------------------------------------------------------------------------------
    //Construct CP: a union on four projections
    Constant *c1 = createConstInt(ONE);

    //construct first union (CP1)
    //SELECT TSTART AS T, salary FROM LEFTY
    //UNION
    //SELECT TEND AS T, salary FROM LEFTY
    List *leftProjExpr1 = NIL;
    List *leftProjExpr2 = NIL;

    AttributeDef *leftBeginDef  = (AttributeDef *) getStringProperty(left, PROP_TEMP_TBEGIN_ATTR);
    int leftBeginPos = getAttrPos(left, leftBeginDef->attrName);
    AttributeReference *leftBeginRef = createFullAttrReference(strdup(leftBeginDef->attrName), 0, leftBeginPos, INVALID_ATTR, leftBeginDef->dataType);
    leftProjExpr1 = appendToTailOfList(leftProjExpr1, leftBeginRef);

    AttributeDef *leftEndDef  = (AttributeDef *) getStringProperty(left, PROP_TEMP_TEND_ATTR);
    int leftEndPos = getAttrPos(left, leftEndDef->attrName);
    AttributeReference *leftEndRef = createFullAttrReference(strdup(leftEndDef->attrName), 0, leftEndPos, INVALID_ATTR, leftEndDef->dataType);
    leftProjExpr2 = appendToTailOfList(leftProjExpr2, leftEndRef);

    FOREACH(char, c, attrNames)
    {
    	AttributeReference *leftRef = getAttrRefByName(left, c);
    	leftProjExpr1 = appendToTailOfList(leftProjExpr1, leftRef);
    	leftProjExpr2 = appendToTailOfList(leftProjExpr2, copyObject(leftRef));
    }

    //construct schema T SALARY  for BOTH
    List *leftAttrNames = singleton("T");
    leftAttrNames = concatTwoLists(leftAttrNames,deepCopyStringList(attrNames));

    ProjectionOperator *leftProj1 = createProjectionOp(leftProjExpr1, left, NIL, deepCopyStringList(leftAttrNames));
    ProjectionOperator *leftProj2 = createProjectionOp(leftProjExpr2, left, NIL, deepCopyStringList(leftAttrNames));
    left->parents = LIST_MAKE(leftProj1,leftProj2);
//    addParent(left, (QueryOperator *) leftProj1);
//    addParent(left, (QueryOperator *) leftProj2);

    //construct union on top (u1)
    SetOperator *u1 = createSetOperator(SETOP_UNION, LIST_MAKE(leftProj1, leftProj2), NIL,
    		deepCopyStringList(leftAttrNames));
    ((QueryOperator *) leftProj1)->parents = singleton(u1);
    ((QueryOperator *) leftProj2)->parents = singleton(u1);

    //duplicate removal on top (CP1)
	DuplicateRemoval *d1 = createDuplicateRemovalOp(NIL, (QueryOperator *) u1,
			NIL, deepCopyStringList(leftAttrNames));

	QueryOperator *u1Op = (QueryOperator *) u1;
	u1Op->parents = singleton(d1);

    //construct second union (CP2)
    //CP1
    //UNION
    //SELECT TEND AS T, salary FROM LEFTY
    List *rightProjExpr1 = NIL;
    List *rightProjExpr2 = NIL;

    AttributeReference *rightBeginRef  = getAttrRefByName(right, TBEGIN_NAME);
    rightProjExpr1 = appendToTailOfList(rightProjExpr1, rightBeginRef);

    AttributeReference *rightEndRef  = getAttrRefByName(right, TEND_NAME);
    rightProjExpr2 = appendToTailOfList(rightProjExpr2, rightEndRef);

    FOREACH(char, c, attrNames)
    {
    	AttributeReference *rightRef = getAttrRefByName(right, c);
    	rightProjExpr1 = appendToTailOfList(rightProjExpr1, rightRef);
    	rightProjExpr2 = appendToTailOfList(rightProjExpr2, copyObject(rightRef));
    }

    List *rightAttrNames  = leftAttrNames;
    ProjectionOperator *rightProj1 = createProjectionOp(rightProjExpr1, right, NIL, deepCopyStringList(rightAttrNames));
    ProjectionOperator *rightProj2 = createProjectionOp(rightProjExpr2, right, NIL, deepCopyStringList(rightAttrNames));
    right->parents = concatTwoLists(right->parents,LIST_MAKE(rightProj1,rightProj2));

    //UNION u2
    SetOperator *u2 = createSetOperator(SETOP_UNION, LIST_MAKE(d1, rightProj1), NIL,
    		deepCopyStringList(rightAttrNames));
    ((QueryOperator *) d1)->parents = singleton(u2);
    ((QueryOperator *) rightProj1)->parents = singleton(u2);

    //duplicate removal on top (CP1)
	DuplicateRemoval *d2 = createDuplicateRemovalOp(NIL, (QueryOperator *) u2,
			NIL, deepCopyStringList(rightAttrNames));

	QueryOperator *u2Op = (QueryOperator *) u2;
	u2Op->parents = singleton(d2);

	//construct third union (u3)
    SetOperator *u3 = createSetOperator(SETOP_UNION, LIST_MAKE(d2, rightProj2), NIL,
    		deepCopyStringList(rightAttrNames));
    ((QueryOperator *) d2)->parents = singleton(u3);
    ((QueryOperator *) rightProj2)->parents = singleton(u3);

    //duplicate removal on top (CP3)
	DuplicateRemoval *d3 = createDuplicateRemovalOp(NIL, (QueryOperator *) u3,
			NIL, deepCopyStringList(rightAttrNames));

	QueryOperator *u3Op = (QueryOperator *) u3;
	u3Op->parents = singleton(d3);

	//additional proj rename SALARY -> SALARY_1
	QueryOperator *d3Op = (QueryOperator *) d3;
    List *projCPTempNames = getAttrNames(d3Op->schema);
    List *projCPNames = NIL;
    List *projCPExprs = NIL;

    //keep two list used in later for condition e.g., salary, job  and salary_1, job_1  (salary=salary_1, job=job_1)
    List *leftList = NIL;
    List *rightList = NIL;

    FOREACH(char, c, projCPTempNames)
    {
    	projCPExprs = appendToTailOfList(projCPExprs,getAttrRefByName(d3Op,c));
    	if(!streq(c, "T"))
    	{
        DEBUG_LOG("Unequal to T");
    		char *cc = CONCAT_STRINGS(c,"_1");
    		projCPNames = appendToTailOfList(projCPNames,cc);
    		leftList = appendToTailOfList(leftList, strdup(c));
    		rightList = appendToTailOfList(rightList, strdup(cc));
    	}
    	else
    		projCPNames = appendToTailOfList(projCPNames,strdup(c));
    }

    ProjectionOperator *projCP = createProjectionOp(projCPExprs, d3Op, NIL, projCPNames);
    d3Op->parents = singleton(projCP);
    QueryOperator *projCPOp = (QueryOperator *) projCP;

    //---------------------------------------------------------------------------------------
	//INTERVALS
    FunctionCall *intervalFunc = createFunctionCall("ROW_NUMBER",NIL);
    List *intervalOrderBy = singleton(copyObject(c1));

    char *intervalFuncName = "winf_0";
    WindowOperator *intervalW = createWindowOp((Node *) intervalFunc,
            NULL,
			intervalOrderBy,
            NULL,
			intervalFuncName, left, NIL);

    left->parents = appendToTailOfList(left->parents, intervalW);

    //projection T_B T_E SALARY ID
    QueryOperator *intervalWOp = (QueryOperator *) intervalW;
    List *intervalTempNames = getAttrNames(intervalWOp->schema);
    List *intervalNames = NIL;
    List *intervalExprs = NIL;
    FOREACH(char, c, intervalTempNames)
    {
    	intervalExprs = appendToTailOfList(intervalExprs,getAttrRefByName(intervalWOp,c));
    	if(streq(c, intervalFuncName))
    		intervalNames = appendToTailOfList(intervalNames,"IDD");
    	else
    		intervalNames = appendToTailOfList(intervalNames,strdup(c));
    }

    ProjectionOperator *interval = createProjectionOp(intervalExprs, intervalWOp, NIL, intervalNames);
    intervalWOp->parents = singleton(interval);
    QueryOperator *intervalOp = (QueryOperator *) interval;

    //JOINCP
    List *joinCPNames = concatTwoLists(deepCopyStringList(getAttrNames(intervalOp->schema)), deepCopyStringList(getAttrNames(projCPOp->schema)));
    JoinOperator *joinCP = createJoinOp(JOIN_CROSS,NULL, LIST_MAKE(interval,projCP), NIL, joinCPNames);
    intervalOp->parents = singleton(joinCP);
    projCPOp->parents = singleton(joinCP);

    // TODO: join on multiple attributes or cross product
    QueryOperator *joinCPOp = (QueryOperator *)joinCP;
    //selection cond
    List *joinCPcondList = NIL;
    FORBOTH(char, cl, cr, leftList, rightList)
    {
        AttributeReference *al = getAttrRefByName(joinCPOp, cl);
        AttributeReference *ar = getAttrRefByName(joinCPOp, cr);
        Operator *oJoinCP = createOpExpr(OPNAME_EQ, LIST_MAKE(al,ar));
        joinCPcondList = appendToTailOfList(joinCPcondList,oJoinCP);
    }

    //c.T >= l.TSTART, c.T < l.TEND
    AttributeReference *oJoinCPT = getAttrRefByName(joinCPOp, "T");
    AttributeReference *oJoinCPB = getAttrRefByName(joinCPOp, TBEGIN_NAME);
    AttributeReference *oJoinCPE = getAttrRefByName(joinCPOp, TEND_NAME);

    Operator *oJoinCP1 = createOpExpr(OPNAME_GE, LIST_MAKE(oJoinCPT,oJoinCPB));
    Operator *oJoinCP2 = createOpExpr(OPNAME_LT , LIST_MAKE(copyObject(oJoinCPT),oJoinCPE));
    joinCPcondList = appendToTailOfList(joinCPcondList,oJoinCP1);
    joinCPcondList = appendToTailOfList(joinCPcondList,oJoinCP2);

    Node *joinJOINCPcond = andExprList(joinCPcondList);
    SelectionOperator *selJOINCP = createSelectionOp(joinJOINCPcond, joinCPOp, NIL, deepCopyStringList(joinCPNames));
    joinCPOp->parents = singleton(selJOINCP);

    QueryOperator *selJOINCPOp = (QueryOperator *) selJOINCP;
    //proj on top
    List *projJOINCPNames = deepCopyStringList(getAttrNames(intervalOp->schema));
    projJOINCPNames = appendToTailOfList(projJOINCPNames, "T");
    List *projJOINCPExprs = NIL;
    FOREACH(char, c, projJOINCPNames)
    	projJOINCPExprs = appendToTailOfList(projJOINCPExprs,getAttrRefByName(selJOINCPOp,c));

    ProjectionOperator *projJOINCP = createProjectionOp(projJOINCPExprs, selJOINCPOp, NIL, projJOINCPNames);
    selJOINCPOp->parents = singleton(projJOINCP);

    QueryOperator *projJOINCPOp = (QueryOperator *) projJOINCP;
    //---------------------------------------------------------------------------------------
    //top
    //top window
    AttributeReference *topT = getAttrRefByName(projJOINCPOp,"T");
    AttributeReference *topID = getAttrRefByName(projJOINCPOp,"IDD");

    FunctionCall *topFunc = createFunctionCall(AGGNAME_LEAD,singleton(copyObject(topT)));
    List *topOrderBy = singleton(copyObject(topT));
    List *topPartBy = singleton(copyObject(topID));

    char *topFuncName = "winf_1";
    WindowOperator *topW = createWindowOp((Node *) topFunc,
    		topPartBy,
			topOrderBy,
            NULL,
			topFuncName, projJOINCPOp, NIL);

    projJOINCPOp->parents = appendToTailOfList(projJOINCPOp->parents, topW);

    QueryOperator *topWOp = (QueryOperator *) topW;

    //topProj
    AttributeReference *topProjE = getAttrRefByName(topWOp,TEND_NAME);
    AttributeReference *topProjwin = getAttrRefByName(topWOp,topFuncName);
    FunctionCall *topProjFunc = createFunctionCall(COALESCE_FUNC_NAME,LIST_MAKE(topProjwin,topProjE));
    List *topProjExprs = NIL;
    List *topProjNames = NIL;

    FOREACH(char, c, getAttrNames(left->schema))
    {
    	if(!streq(c,leftBeginDef->attrName) && !streq(c,leftEndDef->attrName))
    	{
    		topProjExprs = appendToTailOfList(topProjExprs, getAttrRefByName(topWOp,c));
    		topProjNames = appendToTailOfList(topProjNames, strdup(c));
    	}
    }

    topProjExprs = CONCAT_LISTS(topProjExprs, LIST_MAKE(copyObject(topT), topProjFunc));
    topProjNames = CONCAT_LISTS(topProjNames, LIST_MAKE(TBEGIN_NAME,TEND_NAME));

    ProjectionOperator *topProj = createProjectionOp(topProjExprs, topWOp, NIL, topProjNames);
    topWOp->parents = singleton(topProj);
    setTempAttrProps((QueryOperator *) topProj);

    QueryOperator *topProjOp = (QueryOperator *) topProj;
    int pCount = 0;
    FOREACH(AttributeDef, a, topProjOp->schema->attrDefs)
    {
    	if(streq(a->attrName, TBEGIN_NAME) || streq(a->attrName, TEND_NAME))
    		topProjOp->provAttrs = appendToTailOfListInt(topProjOp->provAttrs, pCount);
    	pCount ++;
    }

    // switch subtrees
    newParents = input->parents;
    input->parents = parents;
    switchSubtrees(input, (QueryOperator *) topProj);
    input->parents = newParents;

    return (QueryOperator *) topProj;
}

/*
* --------------------------------------------------------------------------------
* -- new version using window functions
* --------------------------------------------------------------------------------
*
* -- This form of normalization only works if we do not need attributes other
* -- than TSART, TEND, and the attributes we normalized on.
* -- normalize(LEFTY, RIGHTY) ON salary (Output has schema (TSTART, TEND, salary))
* -- assumes half-open intervals [TSTART, TEND) otherwise we need +/-1 on
* -- timestamps
*/

QueryOperator *
addTemporalNormalizationUsingWindow (QueryOperator *input, QueryOperator *reference, List *attrNames)
{
	QueryOperator *left = input;
	QueryOperator *right = reference;

	DEBUG_OP_LOG("add window-based temporal normalization for operator ", input, reference);

	//---------------------------------------------------------------------------------------
    //Construct CP:
    Constant *c1 = createConstInt(ONE);
    Constant *c0 = createConstInt(ZERO);

    //construct first union all (Q1 UNION ALL Q2)
    //SELECT COUNT(*) AS S, 0 AS E, TSTART AS T, SALARY
    //FROM LEFTY
    //GROUP BY TSTART, SALARY
    //UNION ALL
    //SELECT 0 AS S, COUNT(*) AS E, TEND AS T, SALARY
    //FROM LEFTY
    //GROUP BY TEND, SALARY

    //base projections used in Q1, then two aggr on Q1, each agg with 1 proj on top, at last union all
    List *leftProjExpr1 = NIL;
    //List *leftProjExpr2 = NIL;
    List *leftProjAttrNames1 = NIL;

    leftProjAttrNames1 = getAttrNames(left->schema);
    FOREACH(AttributeDef, ad, left->schema->attrDefs)
    {
    	int leftBeginPos = getAttrPos(left, ad->attrName);
    	AttributeReference *leftBeginRef = createFullAttrReference(strdup(ad->attrName), 0, leftBeginPos, INVALID_ATTR, ad->dataType);
    	leftProjExpr1 = appendToTailOfList(leftProjExpr1, leftBeginRef);
    }

    leftProjAttrNames1 = appendToHeadOfList(leftProjAttrNames1, "AGG_GB_ARG0");
    leftProjExpr1 = appendToHeadOfList(leftProjExpr1, copyObject(c1));

    ProjectionOperator *leftProj1 = createProjectionOp(leftProjExpr1, left, NIL, deepCopyStringList(leftProjAttrNames1));

    left->parents = singleton(leftProj1);

    //agg 1
    QueryOperator *leftProj1Op = (QueryOperator *) leftProj1;
    List *agg1GroupBy = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(leftProj1Op, c);
//    	agg1GroupBy = appendToTailOfList(agg1GroupBy, a);
//    }
    FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(leftProj1Op, d->attrName);
        	agg1GroupBy = appendToTailOfList(agg1GroupBy, a);
    	}
    }

    agg1GroupBy = appendToHeadOfList(agg1GroupBy, getAttrRefByName(leftProj1Op, TBEGIN_NAME));

    AttributeReference *agg1Attr = getAttrRefByName(leftProj1Op, "AGG_GB_ARG0");
	FunctionCall *agg1Func = createFunctionCall(strdup(AGGNAME_COUNT),
			singleton(agg1Attr));
	List *aggrs1 = singleton(agg1Func);
	List *agg1Names = NIL;

	FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
	{
		if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName))
			agg1Names = appendToTailOfList(agg1Names, strdup(d->attrName));
	}
	agg1Names = appendToHeadOfList(agg1Names, "AGGR_0");

	AggregationOperator *agg1CP = createAggregationOp(aggrs1, agg1GroupBy, leftProj1Op, NIL, agg1Names);
	leftProj1Op->parents = singleton(agg1CP);

	//proj on agg 1 COUNT(*) AS S, 0 AS E, T_B AS T, SALARY
	QueryOperator *agg1CPOp = (QueryOperator *) agg1CP;
	QueryOperator *proj1CPOp = createProjOnAllAttrs(agg1CPOp);
	proj1CPOp->inputs = singleton(agg1CPOp);
	agg1CPOp->parents = singleton(proj1CPOp);

	//add 0 AS E
    ProjectionOperator *proj1CP = (ProjectionOperator *) proj1CPOp;
    proj1CP->projExprs = appendToTailOfList(proj1CP->projExprs, copyObject(c0));
    proj1CPOp->schema->attrDefs = appendToTailOfList(proj1CPOp->schema->attrDefs,createAttributeDef("E", DT_INT));

    //rename count(*) and T_B
    FOREACH(AttributeDef, d, proj1CPOp->schema->attrDefs)
    {
    	if(streq(d->attrName,"AGGR_0"))
    		d->attrName = "S";
    	else if(streq(d->attrName, TBEGIN_NAME))
    		d->attrName = "T";
    }

    //agg2
    //agg 1
    List *agg2GroupBy = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(leftProj1Op, c);
//    	agg2GroupBy = appendToTailOfList(agg2GroupBy, a);
//    }
    FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(leftProj1Op, d->attrName);
        	agg2GroupBy = appendToTailOfList(agg2GroupBy, a);
    	}
    }

    agg2GroupBy = appendToHeadOfList(agg2GroupBy, getAttrRefByName(leftProj1Op, TEND_NAME));

    AttributeReference *agg2Attr = getAttrRefByName(leftProj1Op, "AGG_GB_ARG0");
	FunctionCall *agg2Func = createFunctionCall(strdup(AGGNAME_COUNT),
			singleton(agg2Attr));
	List *aggrs2 = singleton(agg2Func);
	List *agg2Names = NIL;

	FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
	{
		if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TBEGIN_NAME, d->attrName))
			agg2Names = appendToTailOfList(agg2Names, strdup(d->attrName));
	}
	agg2Names = appendToHeadOfList(agg2Names, "AGGR_0");

	AggregationOperator *agg2CP = createAggregationOp(aggrs2,agg2GroupBy, leftProj1Op, NIL, agg2Names);
	leftProj1Op->parents = appendToTailOfList(leftProj1Op->parents, agg2CP);

	//proj on agg 2 COUNT(*) AS S, 0 AS E, T_E AS T, SALARY
	QueryOperator *agg2CPOp = (QueryOperator *) agg2CP;
	QueryOperator *proj2CPOp = createProjOnAllAttrs(agg2CPOp);
	proj2CPOp->inputs = singleton(agg2CPOp);
	agg2CPOp->parents = singleton(proj2CPOp);

	//add 0 AS E
    ProjectionOperator *proj2CP = (ProjectionOperator *) proj2CPOp;
    proj2CP->projExprs = appendToTailOfList(proj2CP->projExprs, copyObject(c0));
    proj2CPOp->schema->attrDefs = appendToTailOfList(proj2CPOp->schema->attrDefs,createAttributeDef("E", DT_INT));

    //rename count(*) and T_E
    FOREACH(AttributeDef, d, proj2CPOp->schema->attrDefs)
    {
            if(streq(d->attrName,"AGGR_0"))
                d->attrName = "S";
            else if(streq(d->attrName, TEND_NAME))
                d->attrName = "T";
    }

    //construct union on top (u1)
    SetOperator *u1 = createSetOperator(SETOP_UNION, LIST_MAKE(proj1CP, proj2CP), NIL,
    		deepCopyStringList(getAttrNames(proj2CPOp->schema)));
    ((QueryOperator *) proj1CP)->parents = singleton(u1);
    ((QueryOperator *) proj2CP)->parents = singleton(u1);


    //  part 2 of CP
    //  SELECT *
    //  FROM (
    //  SELECT 0 AS S, SALARY, TSTART AS T, 0 AS E
    //  FROM RIGHTY
    //  UNION
    //  SELECT 0 AS S, SALARY, TEND AS T, 0 AS E
    //  FROM RIGHTY) RIGHT_CP

    //proj 1
    List *rightProj1Expr = NIL;
    List *rightProj1Names = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(right, c);
//    	rightProj1Expr = appendToTailOfList(rightProj1Expr, a);
//    	rightProj1Names = appendToTailOfList(rightProj1Names, strdup(c));
//    }

    FOREACH(AttributeDef, d, right->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(right, d->attrName);
        	rightProj1Expr = appendToTailOfList(rightProj1Expr, a);
        	rightProj1Names = appendToTailOfList(rightProj1Names, strdup(d->attrName));
    	}
    }


    rightProj1Expr = appendToTailOfList(rightProj1Expr, getAttrRefByName(right, TBEGIN_NAME));
    rightProj1Names = appendToTailOfList(rightProj1Names, "T");

    rightProj1Expr = appendToTailOfList(rightProj1Expr,copyObject(c0));
    rightProj1Expr = appendToHeadOfList(rightProj1Expr,copyObject(c0));
    rightProj1Names = appendToTailOfList(rightProj1Names, "E");
    rightProj1Names = appendToHeadOfList(rightProj1Names, "S");

    ProjectionOperator *rightProj1 = createProjectionOp(rightProj1Expr, right, NIL, deepCopyStringList(rightProj1Names));
    //QueryOperator *rightProj1Op = (QueryOperator *) rightProj1;
    /*
     * if change (R,R,..) the second R to S, then using following command
     * right->parents = singleton(rightProj1);
     */
    right->parents = appendToTailOfList(right->parents,rightProj1);

    //proj 2
    List *rightProj2Expr = NIL;
    List *rightProj2Names = deepCopyStringList(rightProj1Names);
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(right, c);
//    	rightProj2Expr = appendToTailOfList(rightProj2Expr, a);
//    	//rightProj2Names = appendToTailOfList(rightProj2Names, strdup(c));
//    }
    FOREACH(AttributeDef, d, right->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(right, d->attrName);
        	rightProj2Expr = appendToTailOfList(rightProj2Expr, a);
    	}
    }

    rightProj2Expr = appendToTailOfList(rightProj2Expr, getAttrRefByName(right, TEND_NAME));
    //rightProj2Names = appendToTailOfList(rightProj2Names, "T");

    rightProj2Expr = appendToTailOfList(rightProj2Expr,copyObject(c0));
    rightProj2Expr = appendToHeadOfList(rightProj2Expr,copyObject(c0));
    //rightProj2Names = appendToTailOfList(rightProj2Names, "E");
    //rightProj2Names = appendToHeadOfList(rightProj2Names, "S");

    ProjectionOperator *rightProj2 = createProjectionOp(rightProj2Expr, right, NIL, rightProj2Names);
    //QueryOperator *rightProj1Op = (QueryOperator *) rightProj1;
    right->parents = appendToTailOfList(right->parents, rightProj2);

    //construct union on top (u2)
    SetOperator *u2 = createSetOperator(SETOP_UNION, LIST_MAKE(rightProj1, rightProj2), NIL,
    		deepCopyStringList(rightProj1Names));
    ((QueryOperator *) rightProj1)->parents = singleton(u2);
    ((QueryOperator *) rightProj2)->parents = singleton(u2);

    //duplicate removal on top of u2
	DuplicateRemoval *du2 = createDuplicateRemovalOp(NIL, (QueryOperator *) u2,
			NIL, deepCopyStringList(rightProj1Names));

	QueryOperator *u2Op = (QueryOperator *) u2;
	u2Op->parents = singleton(du2);

	//CP
    //construct union on top (u3)  union u1 and du2
    SetOperator *u3 = createSetOperator(SETOP_UNION, LIST_MAKE(u1, du2), NIL,
    		deepCopyStringList(rightProj1Names));
    ((QueryOperator *) u1)->parents = singleton(u3);
    ((QueryOperator *) du2)->parents = singleton(u3);


    QueryOperator *u3Op = (QueryOperator *) u3;
    //CP_merge agg + proj
    List *groupByCPMerge = NIL;
    List *aggS = NIL;
    List *aggE = NIL;
    List *attrNamesCPMerge = NIL;

    attrNamesCPMerge = appendToTailOfList(attrNamesCPMerge, "S");
    attrNamesCPMerge = appendToTailOfList(attrNamesCPMerge, "E");
	FOREACH(AttributeDef, d, u3Op->schema->attrDefs)
	{
		AttributeReference *a = getAttrRefByName(u3Op, d->attrName);
		if(!streq("S", d->attrName) && !streq("E", d->attrName))
		{
			groupByCPMerge = appendToTailOfList(groupByCPMerge, a);
			attrNamesCPMerge = appendToTailOfList(attrNamesCPMerge, strdup(d->attrName));
		}
		else if(streq("S", d->attrName))
			aggS = appendToTailOfList(aggS,a);
		else if(streq("E", d->attrName))
			aggE = appendToTailOfList(aggE,a);
	}

	FunctionCall *sumS = createFunctionCall(AGGNAME_SUM,aggS);
	FunctionCall *sumE = createFunctionCall(AGGNAME_SUM,aggE);
	List *functionCallList = LIST_MAKE(sumS,sumE);

	AggregationOperator *aggCPMerge = createAggregationOp(functionCallList,groupByCPMerge, u3Op, NIL, attrNamesCPMerge);
    u3Op->parents = singleton(aggCPMerge);

    QueryOperator *aggCPMergeOp = (QueryOperator *) aggCPMerge;

    //internals

    //w1
    WindowBound *internalsWB1 = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
    WindowFrame *internalsWF1 = createWindowFrame(WINFRAME_RANGE,internalsWB1,NULL);

    //OrderBy
    AttributeReference *attrTW1 = getAttrRefByName(aggCPMergeOp, "T");
    List *internalsOrderBy1 = singleton(copyObject(attrTW1));

    //partationBy
    List *internalPartitionBy1 = NIL;
    FOREACH(char, c, attrNames)
    	 internalPartitionBy1 = appendToTailOfList(internalPartitionBy1,getAttrRefByName(aggCPMergeOp, c));

    WindowDef *internalWDef1 = createWindowDef(internalPartitionBy1,internalsOrderBy1,internalsWF1);

    FunctionCall *internalFC1 = createFunctionCall(AGGNAME_SUM,singleton(getAttrRefByName(aggCPMergeOp, "S")));
    WindowFunction *winternalF1 = createWindowFunction(internalFC1,internalWDef1);

    char *internalWNames1 = "winf_0";
    WindowOperator *internalW1 = createWindowOp(copyObject(winternalF1->f),
    		copyObject(winternalF1->win->partitionBy),
			copyObject(winternalF1->win->orderBy),
			copyObject(winternalF1->win->frame),
			internalWNames1, aggCPMergeOp, NIL);

    aggCPMergeOp->parents = singleton(internalW1);
    QueryOperator *internalW1Op = (QueryOperator *) internalW1;


    //w2
    WindowFrame *internalsWF2 = createWindowFrame(WINFRAME_RANGE,copyObject(internalsWB1),NULL);
    List *internalsOrderBy2 = singleton(copyObject(attrTW1));
    //partationBy
    List *internalPartitionBy2 = NIL;
    FOREACH(char, c, attrNames)
    	 internalPartitionBy2 = appendToTailOfList(internalPartitionBy2,getAttrRefByName(internalW1Op, c));

    WindowDef *internalWDef2 = createWindowDef(internalPartitionBy2,internalsOrderBy2,internalsWF2);

    FunctionCall *internalFC2 = createFunctionCall(AGGNAME_SUM,singleton(getAttrRefByName(internalW1Op, "E")));
    WindowFunction *winternalF2 = createWindowFunction(internalFC2,internalWDef2);

    char *internalWNames2 = "winf_1";
    WindowOperator *internalW2 = createWindowOp(copyObject(winternalF2->f),
    		copyObject(winternalF2->win->partitionBy),
			copyObject(winternalF2->win->orderBy),
			copyObject(winternalF2->win->frame),
			internalWNames2, internalW1Op, NIL);

    internalW1Op->parents = singleton(internalW2);

    QueryOperator *internalW2Op = (QueryOperator *) internalW2;

    //w3
    //WindowFrame *internalsWF2 = createWindowFrame(WINFRAME_RANGE,copyObject(internalsWB1),NULL);
    List *internalsOrderBy3 = singleton(copyObject(attrTW1));
    //partationBy
    List *internalPartitionBy3 = NIL;
    FOREACH(char, c, attrNames)
    	 internalPartitionBy3 = appendToTailOfList(internalPartitionBy3,getAttrRefByName(internalW2Op, c));

    WindowDef *internalWDef3 = createWindowDef(internalPartitionBy3,internalsOrderBy3,NULL);

    FunctionCall *internalFC3 = createFunctionCall(AGGNAME_LEAD,singleton(getAttrRefByName(internalW1Op, "T")));
    WindowFunction *winternalF3 = createWindowFunction(internalFC3,internalWDef3);

    char *internalWNames3 = "winf_3";
    WindowOperator *internalW3 = createWindowOp(copyObject(winternalF3->f),
    		copyObject(winternalF3->win->partitionBy),
			copyObject(winternalF3->win->orderBy),
			copyObject(winternalF3->win->frame),
			internalWNames3, internalW2Op, NIL);

    internalW2Op->parents = singleton(internalW3);

    QueryOperator *internalW3Op = (QueryOperator *) internalW3;

    //top proj for intervals winf_0  - winf_1, T, winf_3, salary
    List *intervalsProjExpr = NIL;
    List *intervalsProjNames = NIL;
    Operator *multiplicity = createOpExpr("-",
    		LIST_MAKE(getAttrRefByName(internalW3Op, strdup(internalWNames1)),getAttrRefByName(internalW3Op, strdup(internalWNames2))));
    intervalsProjExpr = appendToTailOfList(intervalsProjExpr, multiplicity);
    intervalsProjExpr = appendToTailOfList(intervalsProjExpr, getAttrRefByName(internalW3Op, "T"));
    intervalsProjExpr = appendToTailOfList(intervalsProjExpr, getAttrRefByName(internalW3Op, strdup(internalWNames3)));

    intervalsProjNames = appendToTailOfList(intervalsProjNames, NUMOPEN);
    intervalsProjNames = appendToTailOfList(intervalsProjNames, TBEGIN_NAME);
    intervalsProjNames = appendToTailOfList(intervalsProjNames, TEND_NAME);

    FOREACH(AttributeDef, d, internalW3Op->schema->attrDefs)
    {
    	if(!streq(d->attrName, "T") && !streq(d->attrName, "S") && !streq(d->attrName, "E")
    			&& !streq(d->attrName,internalWNames1) && !streq(d->attrName,internalWNames2) && !streq(d->attrName,internalWNames3))
    	{
    		intervalsProjExpr = appendToTailOfList(intervalsProjExpr, getAttrRefByName(internalW3Op, strdup(d->attrName)));
    		intervalsProjNames = appendToTailOfList(intervalsProjNames, strdup(d->attrName));
    	}
    }
    ProjectionOperator *intervalsProj = createProjectionOp(intervalsProjExpr, internalW3Op, NIL, intervalsProjNames);
    internalW3Op->parents = singleton(intervalsProj);

    QueryOperator *intervalsProjOp = (QueryOperator *) intervalsProj;
    //-----------------------------------------------------------------------------------------------------------------
    //TNTAB AS (SELECT rownum n FROM dual connect by level <= (SELECT MAX(MULTIPLICITY) FROM INTERVALS))
	TableAccessOperator *TNTAB = createTableAccessOp(TNTAB_DUMMY_TABLE_NAME, NULL, "TNTAB", NIL, singleton("N"), singletonInt(DT_INT));

	//set boolean prop (when translate to SQL, translate to above SQL not this table)
	SET_STRING_PROP(TNTAB, PROP_TEMP_TNTAB, createConstLong((gprom_long_t) intervalsProj));

	//---------------------------------------------------------------------------------------
    //Construct Top

    QueryOperator *TNTABOp = (QueryOperator *)TNTAB;

    //join
	AttributeReference *topAttrNum = getAttrRefByName(intervalsProjOp, NUMOPEN);
	AttributeReference *topAttrN = getAttrRefByName(TNTABOp, "N");
	topAttrN->fromClauseItem = 1;
	Operator *topCond1 = createOpExpr(OPNAME_GT, LIST_MAKE(topAttrNum,copyObject(c0)));
	Operator *topCond2 = createOpExpr(OPNAME_GE, LIST_MAKE(topAttrNum,topAttrN));
	Node *topCond = andExprList(LIST_MAKE(topCond1, topCond2));


    List *topNames = deepCopyStringList(getAttrNames(intervalsProjOp->schema));
    topNames = appendToTailOfList(topNames, "N");

    JoinOperator *topJoin = createJoinOp(JOIN_INNER, topCond, LIST_MAKE(intervalsProj, TNTAB), NIL, topNames);
    intervalsProjOp->parents = singleton(topJoin);
    TNTABOp->parents = singleton(topJoin);

    QueryOperator *topJoinOp = (QueryOperator *) topJoin;
    //projection on top
    List *topProjNames = NIL;
    FOREACH(AttributeDef, d, topJoinOp->schema->attrDefs)
    {
    	if(!streq(d->attrName, "N"))
    		topProjNames = appendToTailOfList(topProjNames, strdup(d->attrName));
    }

    QueryOperator *topProjOp = createProjOnAttrsByName(topJoinOp , topProjNames, NIL);
    topProjOp->inputs = singleton(topJoin);
    topJoinOp->parents = singleton(topProjOp);

    setTempAttrProps(topProjOp);
    int pCount = 0;
    FOREACH(AttributeDef, a, topProjOp->schema->attrDefs)
    {
    	if(streq(a->attrName, TBEGIN_NAME) || streq(a->attrName, TEND_NAME))
    		topProjOp->provAttrs = appendToTailOfListInt(topProjOp->provAttrs, pCount);
    	pCount ++;
    }

    switchSubtreeWithExisting(input, (QueryOperator *) topProjOp);

	return (QueryOperator *) topProjOp;
}

static ProjectionOperator *
createProjDoublingAggAttrs(QueryOperator *agg, int numNewAggs, boolean add, boolean isGB)
{
    ProjectionOperator *dummyProj = (ProjectionOperator *) createProjOnAllAttrs(agg);
    ProjectionOperator *result;
    List *aNames = getQueryOperatorAttrNames((QueryOperator *) dummyProj);
    List *gbNames = NIL;
    List *gbRefs = NIL;
    List *aggNames = sublist(deepCopyStringList(aNames), 0, numNewAggs - 1);
    List *aggRefs = sublist(copyObject(dummyProj->projExprs), 0, numNewAggs - 1);

    if (isGB)
    {
        gbNames = sublist(deepCopyStringList(aNames), numNewAggs, LIST_LENGTH(aNames) - 3);
        gbRefs = sublist(copyObject(dummyProj->projExprs), numNewAggs, LIST_LENGTH(dummyProj->projExprs) - 3);
    }

    //first projection add
    List *resultAttrNames = NIL;
    List *projExprs = NIL;

    // add aggregation results as add_AGGNAME
    for(int i = 0; i < LIST_LENGTH(aggNames); i++)
    {
        AttributeReference *a =  (AttributeReference *) copyObject(getNthOfListP(aggRefs, i));
        char *aName = CONCAT_STRINGS(ADD_AGG_PREFIX, strdup(getNthOfListP(aggNames, i)));

        resultAttrNames = appendToTailOfList(resultAttrNames, aName);
        if (add)
        {
            projExprs = appendToTailOfList(projExprs, a);
        }
        else
        {
            projExprs = appendToTailOfList(projExprs, createConstInt(0));
        }
    }

    // add aggregation results as dec_AGGNAME
    for(int i = 0; i < LIST_LENGTH(aggNames); i++)
    {
        AttributeReference *a =  (AttributeReference *) copyObject(getNthOfListP(aggRefs, i));
        char *aName = CONCAT_STRINGS(DEC_AGG_PREFIX, strdup(getNthOfListP(aggNames, i)));

        resultAttrNames = appendToTailOfList(resultAttrNames, aName);
        if (add)
        {
            projExprs = appendToTailOfList(projExprs, createConstInt(0));
        }
        else
        {
            projExprs = appendToTailOfList(projExprs, a);
        }
    }

    // add regularGB attributes
    resultAttrNames = CONCAT_LISTS(resultAttrNames, gbNames);
    projExprs = CONCAT_LISTS(projExprs, gbRefs);

    // add time begin or end attribute
    if (add)
    {
        AttributeReference *timeA = getAttrRefByName(agg,TBEGIN_NAME);    //T_B AS TS
        projExprs = appendToTailOfList(projExprs, timeA);
        resultAttrNames = appendToTailOfList(resultAttrNames, strdup(TIMEPOINT_ATTR));
    }
    else
    {
        AttributeReference *timeA = getAttrRefByName(agg,TEND_NAME);    //T_E AS TS
        projExprs = appendToTailOfList(projExprs, timeA);
        resultAttrNames = appendToTailOfList(resultAttrNames, strdup(TIMEPOINT_ATTR));
    }

    result = createProjectionOp(projExprs, agg, NIL, resultAttrNames);
    DEBUG_OP_LOG("created projection", result);
    return result;
}


//--------------------------------------------------------------------------------
//-- NEW VERSION OF NORMALIZATION+AGG THAT WORKS FOR ALL DECOMPOSABLE AGGREGATION
//-- FUNCTIONS, I.E., AGGREGATION FUNCTIONS THAT CAN BE SPLIT INTO A
//-- PREAGGREGATION AND POSTAGGREGATION, e.g., SUM(A) = SUM(SUM(sub_A)) AND
//-- COUNT(A) = SUM(COUNT(sub_A))
//--------------------------------------------------------------------------------
//-- EXAMPLE QUERY IS
//-- TEMPORAL(SELECT dept_no, avg(salary) as avg_salary
//-- FROM TDB_DEPT_EMP WITH TIME (from_date, to_date) a,
//--      TDB_SALARIES WITH TIME (from_date, to_date) b
//-- WHERE a.emp_no = b.emp_no
//-- GROUP BY dept_no);
//--
//-- T0 is the sequenced rewriting of the aggregation input


QueryOperator *
rewriteTemporalAggregationWithNormalization(AggregationOperator *agg)
{
    AttributeReference *tb, *te;
    QueryOperator *op = (QueryOperator *) agg;
    QueryOperator *c = OP_LCHILD(op);
    List *origAggs;
    boolean isGB = (agg->groupBy != NIL);
    int numAggs;
    int numGB;
//    int numNewGB = numGB + 2;
    int numNewAggs;
    List *gbNames = NIL;
    List *aggNames = NIL;
    QueryOperator *top = NULL;
    Constant *c1 = createConstInt(ONE);
    Constant *c0 = createConstInt(ZERO);
    List *origAggReturnTypes = NIL;
    // make aggregation functions upper case
//    FOREACH(FunctionCall,f,agg->aggrs)
//    {
//        f->functionname = strToUpper(f->functionname);
//    }

    FOREACH(FunctionCall,f,agg->aggrs)
    {
        origAggReturnTypes = appendToTailOfListInt(origAggReturnTypes, typeOf((Node *) f));
    }
    DEBUG_LOG("original aggregation function return types: %s", nodeToString(origAggReturnTypes));

    origAggs = copyObject(agg->aggrs);
    numAggs = LIST_LENGTH(origAggs);
    numGB = LIST_LENGTH(agg->groupBy);
    numNewAggs = numAggs;

    if (isGB)
        gbNames = aggOpGetGroupByAttrNames(agg);
    aggNames = aggOpGetAggAttrNames(agg);

    // ****************************************
    // rewrite child
    c = temporalRewriteOperator(c);

    // ****************************************
    // adapt aggregation operator
    List *newAgg = NIL;
    List *newSchema = NIL;
//    List *newGB = NIL;
    int bPos = INVALID_ATTR;
    int ePos = INVALID_ATTR;
    tb = getTempAttrRef(c, TRUE);
    te = getTempAttrRef(c, FALSE);

    // translate aggregation into pre-aggregation
    FORBOTH(Node,agg,def,origAggs,op->schema->attrDefs)
    {
        FunctionCall *a  = (FunctionCall*) agg;
        AttributeDef *d = (AttributeDef *) def;
        char *fName = a->functionname;
        if (streq(fName,AGGNAME_SUM) || streq(fName,AGGNAME_COUNT))
        {
            newAgg = appendToTailOfList(newAgg, a);
            newSchema = appendToTailOfList(newSchema, d);
        }
        else if (streq(fName, AGGNAME_AVG))
        {
            FunctionCall *cnt, *sum;
            AttributeDef *sumDef, *cntDef;

            numNewAggs++;

            sum = copyObject(a);
            sum->functionname = strdup(AGGNAME_SUM);
            sumDef = copyObject(d);
            sumDef->attrName = CONCAT_STRINGS(sumDef->attrName, "_avg_sum");
            newAgg = appendToTailOfList(newAgg, sum);
            newSchema = appendToTailOfList(newSchema, sumDef);

            cnt = createFunctionCall (strdup(AGGNAME_COUNT), singleton(createConstInt(1)));
            cntDef = copyObject(d);
            cntDef->attrName = CONCAT_STRINGS(cntDef->attrName, "_avg_cnt");
            newAgg = appendToTailOfList(newAgg, cnt);
            newSchema = appendToTailOfList(newSchema, cntDef);
        }
        else if (streq(fName,AGGNAME_MIN))
        {
            FATAL_LOG("aggregation + normalization does not work for MIN currently");
        }
        else if (streq(fName,AGGNAME_MAX))
        {
            FATAL_LOG("aggregation + normalization does not work for MAX currently");
        }
        else
            FATAL_LOG("do not know how to handle aggregation function %s", fName);
    }

    // add count aggregation to be able to filter out gaps between intervals later on
    FunctionCall *openInterCount = createFunctionCall (strdup(AGGNAME_COUNT), singleton(createConstInt(1)));
    AttributeDef *cntDef = createAttributeDef(strdup(OPEN_INTER_COUNT_ATTR), DT_LONG);
    newAgg = appendToTailOfList(newAgg, openInterCount);
    newSchema = appendToTailOfList(newSchema, cntDef);
    numNewAggs++;

    agg->aggrs = newAgg;

    // add group by attributes to schema
    if (isGB)
    {
        newSchema = CONCAT_LISTS(newSchema,
                sublist(op->schema->attrDefs,
                        LIST_LENGTH(origAggs),
                        LIST_LENGTH(op->schema->attrDefs) - 1));
    }

    // add tb,te to group by
    agg->groupBy = appendToTailOfList(agg->groupBy, tb);
    agg->groupBy = appendToTailOfList(agg->groupBy, te);

    newSchema = appendToTailOfList(newSchema,
            createAttributeDef(strdup(tb->name), tb->attrType));
    newSchema = appendToTailOfList(newSchema,
                createAttributeDef(strdup(te->name), te->attrType));

    op->schema->attrDefs = newSchema;

    bPos = LIST_LENGTH(newSchema) - 2;
    ePos = bPos + 1;
    op->provAttrs = CONCAT_LISTS(singletonInt(bPos), singletonInt(ePos));
    SetOperator *unionDummy;

    // aggregation has group by clause?
    if (isGB)
    {
        top = (QueryOperator *) agg;
    }
    else
    {
        List *aNames = getQueryOperatorAttrNames((QueryOperator *) agg);
        List *constVals = NIL;
        ConstRelOperator *neutralCRel;

        // add empty relation result values for each aggregation function
        FOREACH(FunctionCall,f,origAggs)
        {
            char *fName = f->functionname;
            if(streq(fName,AGGNAME_COUNT))
            {
                constVals = appendToTailOfList(constVals, createConstInt(0));
            }
            else if(streq(fName, AGGNAME_AVG))
            {
                constVals = appendToTailOfList(constVals, createNullConst(DT_INT));
                constVals = appendToTailOfList(constVals, createConstInt(0));
            }
            else
            {
                constVals = appendToTailOfList(constVals, createNullConst(typeOf((Node *) f)));
            }
        }

        // add dummy value for open interval counter attributes
        constVals = appendToTailOfList(constVals, createConstInt(0));

        if(T_BEtype == DT_INT)
        {
            // add minimal and maximal value for the domain of the time attributes
            constVals = appendToTailOfList(constVals, createConstInt(INT_MINVAL));
            constVals = appendToTailOfList(constVals, createConstInt(INT_MAXVAL));
        }
        else // is date
        {
            // use date format 1-JAN-92 (1992)
            //FunctionCall *dateBegin = createFunctionCall("TO_DATE", LIST_MAKE(createConstInt(1),createConstString("J")));
            FunctionCall *dateBegin = createFunctionCall("TO_DATE", LIST_MAKE(createConstString("1992-01-01"),createConstString("YYYY-MM-DD")));
            FunctionCall *dateEnd = createFunctionCall("TO_DATE", LIST_MAKE(createConstString("9999-01-01"),createConstString("YYYY-MM-DD")));
            constVals = appendToTailOfList(constVals, dateBegin);
            constVals = appendToTailOfList(constVals, dateEnd);
        }

        neutralCRel = createConstRelOp(constVals, NIL, aNames, NIL);
        unionDummy = createSetOperator(SETOP_UNION, LIST_MAKE(agg,neutralCRel), NIL, deepCopyStringList(aNames));

        //TODO Add union with single tuple with neutral elements
        switchSubtrees((QueryOperator *) agg, (QueryOperator *) unionDummy);
        addParent((QueryOperator *) agg, (QueryOperator *) unionDummy);
        addParent((QueryOperator *) neutralCRel, (QueryOperator *) unionDummy);
        top = (QueryOperator *) unionDummy;
    }

    // add
    QueryOperator *oldTop = top;
    top = createProjOnAllAttrs(oldTop);
    top->inputs = LIST_MAKE(oldTop);
    switchSubtrees((QueryOperator *) oldTop, (QueryOperator *) top);
    addParent((QueryOperator *) oldTop, (QueryOperator *) top);

    //    addChildOperator(top, oldTop);

    // ****************************************
	//1 Operator

	//-- create partial results at each time point
	/*T2 AS (
	 *SELECT c AS add_c, s AS add_s, 0 AS dec_c, 0 AS dec_s, dept_no, T_B AS TS
	 *FROM T1
	 *UNION ALL
	 *SELECT 0 AS add_c, 0 AS add_s, c AS dec_c, s AS dec_s,  dept_no, T_E AS TS
	 *FROM T1
	 *)
     */

	//3 Operator
	//union

    ProjectionOperator *proj1T2 = createProjDoublingAggAttrs(top, numNewAggs, TRUE, isGB);
    ProjectionOperator *proj2T2 = createProjDoublingAggAttrs(top, numNewAggs, FALSE, isGB);

	//union
    QueryOperator *proj1T2Op = (QueryOperator *) proj1T2;
    QueryOperator *proj2T2Op = (QueryOperator *) proj2T2;
    SetOperator *t2 = createSetOperator(SETOP_UNION, LIST_MAKE(proj1T2, proj2T2), NIL,
    		deepCopyStringList(getAttrNames(proj1T2Op->schema)));

    proj1T2Op->parents = singleton(t2);
    proj2T2Op->parents = singleton(t2);

    QueryOperator *t2Op = (QueryOperator *) t2;

    // ****************************************
    // add intermediate aggregation on timepoints over union output
    AggregationOperator *iAgg;
    List *attrNames = deepCopyStringList(getQueryOperatorAttrNames(t2Op));
    List *attrRefs = getNormalAttrProjectionExprs(t2Op);
    List *groupBy = sublist(copyList(attrRefs),numNewAggs * 2, LIST_LENGTH(attrRefs) - 1);
    List *aggs = NIL;

    attrRefs = sublist(attrRefs, 0, (numNewAggs * 2) - 1);

    // create aggregation expressions
    FOREACH(AttributeReference,a,attrRefs)
    {
//        char *name = a->name;
        FunctionCall *f;

        //TODO deal with non SUM combiner (what would that be)
        f = createFunctionCall(AGGNAME_SUM, singleton(a));
        aggs = appendToTailOfList(aggs, f);
    }

    iAgg = createAggregationOp(aggs, groupBy, (QueryOperator *) t2Op, NIL, attrNames);
    addParent(t2Op, (QueryOperator *) iAgg);
    t2Op = (QueryOperator *) iAgg;

    DEBUG_OP_LOG("created intermediate aggregation on timepoints over union output", iAgg);

    // ****************************************
    //-- computing the sliding window adding new values and deducting the values of "closing" intervals
    /*  T3 AS (
     *  SELECT
     *  sum(add_c) OVER (PARTITION BY dept_no ORDER BY ts RANGE UNBOUNDED PRECEDING)
     *  - sum(dec_c) OVER (PARTITION BY dept_no ORDER BY ts RANGE UNBOUNDED PRECEDING) AS c,
     *  sum(add_s) OVER (PARTITION BY dept_no ORDER BY ts RANGE UNBOUNDED PRECEDING)
     *  - sum(dec_s) OVER (PARTITION BY dept_no ORDER BY ts RANGE UNBOUNDED PRECEDING) AS s,
     *  ts,
     *  dept_no
     *  FROM T2
     *  )
     */
    // create window definition shared by all windows
    WindowBound *b1T3 = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
    WindowFrame *f1T3 = createWindowFrame(WINFRAME_RANGE,b1T3,NULL);
    AttributeReference *attr1T3 = getAttrRefByName(t2Op, TIMEPOINT_ATTR);
    List *orderBy1T3 = singleton(attr1T3);
    QueryOperator *curChild = t2Op;
//    QueryOperator *curWindow = NULL;

    attrRefs = getNormalAttrProjectionExprs(t2Op);

    //partationBy
    List *partatiionBy1T3 = getAttrRefsByNames(t2Op, gbNames);
//    FOREACH(char, c, gbNames)
//    		partatiionBy1T3 = appendToTailOfList(partatiionBy1T3,getAttrRefByName(t2Op, c));

    WindowDef *wd1T3 = createWindowDef(partatiionBy1T3,orderBy1T3,f1T3);

    // create one window operator for each aggregation function
    for(int i = 0; i < numNewAggs * 2; i++)
    {
        char *aName = strdup(getAttrDefByPos(curChild, i)->attrName);//TODO other than SUM?
        FunctionCall *fc1T3 = createFunctionCall(AGGNAME_SUM,singleton(getAttrRefByName(t2Op, aName)));
        WindowFunction *wf1T3 = createWindowFunction(fc1T3,wd1T3);

        char *wname1T3 = CONCAT_STRINGS(WIN_PREFIX, strdup(aName));
        WindowOperator *winOp = createWindowOp(copyObject(wf1T3->f),
                copyObject(wf1T3->win->partitionBy),
                copyObject(wf1T3->win->orderBy),
                copyObject(wf1T3->win->frame),
                wname1T3,
                curChild,
                NIL);
        curChild->parents = singleton(winOp);
        curChild = (QueryOperator *) winOp;
    }

    QueryOperator *window4T3Op = (QueryOperator *) curChild;

    DEBUG_OP_LOG("created each window operator on intermediate aggregation", window4T3Op);

    //projection winf_0 - winf_1 as C, winf_2 - winf_3 as S, TS, DEPT_NO
    //    Operator *t4O2 = createOpExpr("!=", LIST_MAKE(t4dpRef,copyObject(c0)));
    // the schema is now add_agg_1, ...., add_agg_n, dec_add_1, ..., dec_agg_n, GB, TS, w_add_agg_1, ...
    attrNames = deepCopyStringList(getQueryOperatorAttrNames(t2Op));
    attrNames = sublist(deepCopyStringList(attrNames),numNewAggs * 2, LIST_LENGTH(attrNames) - 1);
    attrRefs = getNormalAttrProjectionExprs(window4T3Op);
    groupBy = sublist(copyObject(attrRefs),numNewAggs * 2, numNewAggs * 2 +  LIST_LENGTH(gbNames));
    List *attrExprProjT3 = NIL;
    List *namesProjT3 = NIL;
    int offset = (numNewAggs * 2) + numGB + 1;
    // add subtractions
    for(int i = 0 ; i < numNewAggs; i++)
    {
        AttributeReference *addWinf = getAttrRefByPos(window4T3Op, offset + i);
        AttributeReference *decWinf = getAttrRefByPos(window4T3Op, offset + i + numNewAggs);
        Operator *diffWinf = createOpExpr("-", LIST_MAKE(addWinf,decWinf)); //winf_add - winf_dec
        char *attrName = strdup(addWinf->name);

        attrExprProjT3 = appendToTailOfList(attrExprProjT3, diffWinf);
        namesProjT3 = appendToTailOfList(namesProjT3, attrName);
    }
    // add remaining attributes
    attrExprProjT3 = CONCAT_LISTS(attrExprProjT3, groupBy);

    // schema is the same as for aggregation
    namesProjT3 = CONCAT_LISTS(namesProjT3, attrNames);

    ProjectionOperator *projT3 = createProjectionOp(attrExprProjT3, window4T3Op, NIL, namesProjT3);
    window4T3Op->parents = singleton(projT3);
    QueryOperator *projT3Op = (QueryOperator *) projT3;

    QueryOperator *dupT3Op = (QueryOperator *) projT3Op;

    DEBUG_OP_LOG("combine each window operator", dupT3Op);

    // ****************************************
    //-- create intervals based on adjacent time points and compute final aggregation results (for avg and other aggs)
    /*
     *T4 AS (
     *  SELECT average, sum, dept_no, tstart, tend
     *  FROM (
     *   SELECT
     *     CASE WHEN (c = 0) THEN NULL ELSE s / c END AS average,
     *     CASE WHEN (c = 0) THEN NULL ELSE s END AS sum,
     *     dept_no,
     *     ts AS tstart,
     *     LAST_VALUE(ts) OVER (PARTITION BY dept_no ORDER BY ts ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING) AS tend
     *   FROM T3)
     *  WHERE tend IS NOT NULL
     *)
     */

    //window
    //LAST_VALUE(ts) OVER (PARTITION BY dept_no ORDER BY ts ROWS BETWEEN 1 FOLLOWING AND 1 FOLLOWING) AS tend
    WindowBound *bT4 = createWindowBound(WINBOUND_EXPR_FOLLOW,(Node *)singleton(copyObject(c1)));
    WindowFrame *fT4 = createWindowFrame(WINFRAME_ROWS,bT4,copyObject(bT4));

    // partition by
    DEBUG_LOG("group-by names: %s", stringListToString(gbNames));
    List *partitionByT4 = getAttrRefsByNames(dupT3Op, gbNames);
    DEBUG_NODE_BEATIFY_LOG("partition by refs", partitionByT4);

    // order by
    AttributeReference *tsT4 = getAttrRefByName(dupT3Op, TS);
    List *groupByT4 = singleton(tsT4);

    WindowDef *wdT4 = createWindowDef(partitionByT4,groupByT4,fT4);
    FunctionCall *fcT4 = createFunctionCall(AGGNAME_LAST_VALUE,singleton(copyObject(tsT4)));
    WindowFunction *wfT4 = createWindowFunction(fcT4,wdT4);

    char *wnameT4 = NEXT_TS_ATTR;
    WindowOperator *windowT4 = createWindowOp(copyObject(wfT4->f),
            copyObject(wfT4->win->partitionBy),
            copyObject(wfT4->win->orderBy),
            copyObject(wfT4->win->frame),
			wnameT4, dupT3Op, NIL);
    dupT3Op->parents = singleton(windowT4);
    QueryOperator *windowT4Op = (QueryOperator *) windowT4;

    DEBUG_OP_LOG("final window operator", windowT4Op);

    // projection that computes final agg
    List *projExprT4 = NIL;
    List *projNamesT4 = NIL;
    int attrPos = 0;
    int attrPosRef = 0;
    attrRefs = getNormalAttrProjectionExprs(windowT4Op);
    if (isGB)
        groupBy = sublist(copyList(attrRefs),numNewAggs, LIST_LENGTH(attrRefs) - 3);
    else
        groupBy = NIL;

    for(int i = 0; i < numAggs; i++, attrPos++, attrPosRef++)
    {
        FunctionCall *agg = (FunctionCall *) getNthOfListP(origAggs, attrPos);
        char *fName = agg->functionname;
        Node *projExpr = NULL;

        if (streq(fName,AGGNAME_SUM))
        {
            AttributeReference *countRef, *sumRef;
            char *openCname = CONCAT_STRINGS(WIN_PREFIX, ADD_AGG_PREFIX, OPEN_INTER_COUNT_ATTR);

            sumRef = (AttributeReference *) getNthOfListP(attrRefs, attrPos);
            countRef = getAttrRefByName(windowT4Op, openCname);

            Operator *whenOperator = createOpExpr(OPNAME_EQ, LIST_MAKE(copyObject(countRef), copyObject(c0)));
            CaseWhen *whenT4 = createCaseWhen((Node *) whenOperator, (Node *) createNullConst(sumRef->attrType));
            Node *elseT4 = (Node *) sumRef;
            CaseExpr *caseExprT4 = createCaseExpr(NULL, singleton(whenT4), elseT4);
            projExpr = (Node *) caseExprT4;
        }
        else if (streq(fName,AGGNAME_COUNT))
        {
            projExpr = (Node *) getNthOfListP(attrRefs, attrPos);
        }
        else if (streq(fName, AGGNAME_AVG))
        {
            AttributeReference *countRef, *sumRef;

            sumRef = (AttributeReference *) getNthOfListP(attrRefs, attrPosRef);
            attrPosRef++;
            countRef = (AttributeReference *) getNthOfListP(attrRefs, attrPosRef);

            Operator *whenOperator = createOpExpr(OPNAME_EQ, LIST_MAKE(copyObject(countRef), copyObject(c0)));
            CaseWhen *whenT4 = createCaseWhen((Node *) whenOperator, (Node *) createNullConst(DT_FLOAT)); // (Node *) createConstFloat(0.0));
            Operator *elseT4 = createOpExpr("/", LIST_MAKE(copyObject(sumRef), copyObject(countRef)));
            CaseExpr *caseExprT4 = createCaseExpr(NULL, singleton(whenT4), (Node *) elseT4);
            projExpr = (Node *) caseExprT4;
        }

        projExprT4 = appendToTailOfList(projExprT4, projExpr);
    }

    // add open interval counter
    projExprT4 = appendToTailOfList(projExprT4, getNthOfListP(attrRefs, attrPos));

    projNamesT4 = CONCAT_LISTS(deepCopyStringList(aggNames), singleton(strdup(OPEN_INTER_COUNT_ATTR)), deepCopyStringList(gbNames));
    projExprT4 = CONCAT_LISTS(projExprT4, groupBy);

    // add interval bounds to projection
    projExprT4 = appendToTailOfList(projExprT4, getAttrRefByName(windowT4Op,TIMEPOINT_ATTR));
    projNamesT4 = appendToTailOfList(projNamesT4, TBEGIN_NAME);

    projExprT4 = appendToTailOfList(projExprT4, getAttrRefByName(windowT4Op,NEXT_TS_ATTR));
    projNamesT4 = appendToTailOfList(projNamesT4, TEND_NAME);

    ProjectionOperator *projT4 = createProjectionOp(projExprT4, windowT4Op, NIL, projNamesT4);
    windowT4Op->parents = singleton(projT4);
    QueryOperator *projT4Op = (QueryOperator *) projT4;

    // final selection that removes intervals without an upper bound (largest change point) and intervals that are gaps in the input
    AttributeReference *tendSel = getAttrRefByName(projT4Op,TEND_NAME);
    IsNullExpr *isnullExprSel =  createIsNullExpr((Node *) singleton(tendSel));
    Operator *notnullOperator = createOpExpr(OPNAME_NOT, singleton(isnullExprSel));
    Node *selectionCond = NULL;
    if (isGB)
    {
        // count should be larger than 0
        Operator *nonZeroCount = createOpExpr(OPNAME_GT, LIST_MAKE(getAttrRefByName(projT4Op, OPEN_INTER_COUNT_ATTR), copyObject(c0)));
        selectionCond = AND_EXPRS((Node *) notnullOperator, (Node *) nonZeroCount);
    }
    else
    {
        selectionCond = (Node *) notnullOperator;
    }

    SelectionOperator *selT4 = createSelectionOp((Node *) selectionCond, projT4Op, NIL, deepCopyStringList(projNamesT4));
    projT4Op->parents = singleton(selT4);

    // project out attribute that counts open intervals
    QueryOperator *finalOp;
    List *aggAttrPos, *gbAttrPos, *finalAttrPos;

    CREATE_INT_SEQ(aggAttrPos,0,numAggs - 1,1);
    CREATE_INT_SEQ(gbAttrPos,numAggs + 1,numAggs + numGB + 2,1);
    finalAttrPos = CONCAT_LISTS(aggAttrPos, gbAttrPos);

    finalOp = createProjOnAttrs((QueryOperator *) selT4, finalAttrPos);

    // add casts to final projection if preaggregating has changed aggregation function result type
    int aggPos = 0;
    FOREACH_INT(d,origAggReturnTypes)
    {
        DataType dt = (DataType) d;
        Node *projExpr = getNthOfListP(((ProjectionOperator *) finalOp)->projExprs, aggPos);
        if (dt != typeOf(projExpr))
        {
            ListCell *pCell = getNthOfList(((ProjectionOperator *) finalOp)->projExprs, aggPos);
            pCell->data.ptr_value = createCastExpr(projExpr, dt);
            AttributeDef *a = getNthOfListP(finalOp->schema->attrDefs, aggPos);
            a->dataType = dt;
        }

        aggPos++;
    }

    addChildOperator(finalOp, (QueryOperator *) selT4);

    // set temporal attributes for rewrites of parents to access and switch top operator with aggregation
    bPos = LIST_LENGTH(finalOp->schema->attrDefs) - 2;
    ePos = bPos + 1;
    finalOp->provAttrs = CONCAT_LISTS(singletonInt(bPos), singletonInt(ePos));
    setTempAttrProps((QueryOperator *) finalOp);
    switchSubtrees(top, (QueryOperator *) finalOp);
    top->parents = LIST_MAKE(proj1T2, proj2T2);

    LOG_RESULT("Rewritten aggregation+normalization", finalOp);
	return (QueryOperator *) finalOp;
}


QueryOperator *
rewriteTemporalSetDiffWithNormalization(SetOperator *diff)
{
    //TODO
	QueryOperator *o = (QueryOperator *) diff;
	QueryOperator *left = OP_LCHILD(o);
	QueryOperator *right = OP_RCHILD(o);

	left = temporalRewriteOperator(left);
	right = temporalRewriteOperator(right);

    List *attrNames = getNormalAttrNames(o);

	//---------------------------------------------------------------------------------------
    //Construct CP:
    Constant *c1 = createConstInt(ONE);
    Constant *c0 = createConstInt(ZERO);

    //construct first union all (Q1 UNION ALL Q2)
    //SELECT COUNT(*) AS S, 0 AS E, TSTART AS T, SALARY
    //FROM LEFTY
    //GROUP BY TSTART, SALARY
    //UNION ALL
    //SELECT 0 AS S, COUNT(*) AS E, TEND AS T, SALARY
    //FROM LEFTY
    //GROUP BY TEND, SALARY

    //base projections used in Q1, then two aggr on Q1, each agg with 1 proj on top, at last union all
    List *leftProjExpr1 = NIL;
    //List *leftProjExpr2 = NIL;
    List *leftProjAttrNames1 = NIL;

    leftProjAttrNames1 = getAttrNames(left->schema);
    FOREACH(AttributeDef, ad, left->schema->attrDefs)
    {
    	int leftBeginPos = getAttrPos(left, ad->attrName);
    	AttributeReference *leftBeginRef = createFullAttrReference(strdup(ad->attrName), 0, leftBeginPos, INVALID_ATTR, ad->dataType);
    	leftProjExpr1 = appendToTailOfList(leftProjExpr1, leftBeginRef);
    }

    leftProjAttrNames1 = appendToHeadOfList(leftProjAttrNames1, "AGG_GB_ARG0");
    leftProjExpr1 = appendToHeadOfList(leftProjExpr1, copyObject(c1));

    ProjectionOperator *leftProj1 = createProjectionOp(leftProjExpr1, left, NIL, deepCopyStringList(leftProjAttrNames1));

    left->parents = singleton(leftProj1);

    //agg 1
    QueryOperator *leftProj1Op = (QueryOperator *) leftProj1;
    List *agg1GroupBy = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(leftProj1Op, c);
//    	agg1GroupBy = appendToTailOfList(agg1GroupBy, a);
//    }
    FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(leftProj1Op, d->attrName);
        	agg1GroupBy = appendToTailOfList(agg1GroupBy, a);
    	}
    }

    agg1GroupBy = appendToTailOfList(agg1GroupBy, getAttrRefByName(leftProj1Op, TBEGIN_NAME));

    AttributeReference *agg1Attr = getAttrRefByName(leftProj1Op, "AGG_GB_ARG0");
	FunctionCall *agg1Func = createFunctionCall(strdup(AGGNAME_COUNT),
			singleton(agg1Attr));
	List *aggrs1 = singleton(agg1Func);
	List *agg1Names = NIL;

	FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
	{
		if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName))
			agg1Names = appendToTailOfList(agg1Names, strdup(d->attrName));
	}
	agg1Names = appendToHeadOfList(agg1Names, "AGGR_0");

	AggregationOperator *agg1CP = createAggregationOp(aggrs1, agg1GroupBy, leftProj1Op, NIL, agg1Names);
	leftProj1Op->parents = singleton(agg1CP);

	//proj on agg 1 COUNT(*) AS S, 0 AS E, T_B AS T, SALARY
	QueryOperator *agg1CPOp = (QueryOperator *) agg1CP;
	QueryOperator *proj1CPOp = createProjOnAllAttrs(agg1CPOp);
	proj1CPOp->inputs = singleton(agg1CPOp);
	agg1CPOp->parents = singleton(proj1CPOp);

	//add 0 AS E
    ProjectionOperator *proj1CP = (ProjectionOperator *) proj1CPOp;
    proj1CP->projExprs = appendToTailOfList(proj1CP->projExprs, copyObject(c0));
    proj1CPOp->schema->attrDefs = appendToTailOfList(proj1CPOp->schema->attrDefs,createAttributeDef("E", DT_INT));

    //rename count(*) and T_B
    FOREACH(AttributeDef, d, proj1CPOp->schema->attrDefs)
    {
    	if(streq(d->attrName,"AGGR_0"))
    		d->attrName = "S";
    	else if(streq(d->attrName, TBEGIN_NAME))
    		d->attrName = "T";
    }

    //agg2
    List *agg2GroupBy = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(leftProj1Op, c);
//    	agg2GroupBy = appendToTailOfList(agg2GroupBy, a);
//    }
    FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(leftProj1Op, d->attrName);
        	agg2GroupBy = appendToTailOfList(agg2GroupBy, a);
    	}
    }

    agg2GroupBy = appendToTailOfList(agg2GroupBy, getAttrRefByName(leftProj1Op, TEND_NAME));

    AttributeReference *agg2Attr = getAttrRefByName(leftProj1Op, "AGG_GB_ARG0");
	FunctionCall *agg2Func = createFunctionCall(strdup(AGGNAME_COUNT),
			singleton(agg2Attr));
	List *aggrs2 = singleton(agg2Func);
	List *agg2Names = NIL;

	FOREACH(AttributeDef, d, leftProj1Op->schema->attrDefs)
	{
		if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TBEGIN_NAME, d->attrName))
			agg2Names = appendToTailOfList(agg2Names, strdup(d->attrName));
	}
	agg2Names = appendToHeadOfList(agg2Names, "AGGR_0");

	AggregationOperator *agg2CP = createAggregationOp(aggrs2,agg2GroupBy, leftProj1Op, NIL, agg2Names);
	leftProj1Op->parents = appendToTailOfList(leftProj1Op->parents, agg2CP);

	//proj on agg 2  0 AS S, COUNT(*) AS E, T_E AS T, SALARY
	QueryOperator *agg2CPOp = (QueryOperator *) agg2CP;
	AttributeReference *aggr_0_ref = getAttrRefByName(agg2CPOp, "AGGR_0");


	List *agg2ProjExpr = NIL;
	List *agg2NameList = NIL;
	agg2ProjExpr = appendToTailOfList(agg2ProjExpr, copyObject(c0));
	agg2NameList = appendToTailOfList(agg2NameList, "S");
	FOREACH(AttributeDef, ad, agg2CPOp->schema->attrDefs)
	{
		if(!streq(ad->attrName, "AGGR_0"))
		{
			AttributeReference *a = getAttrRefByName(agg2CPOp, ad->attrName);
			agg2ProjExpr = appendToTailOfList(agg2ProjExpr, a);

			if(streq(ad->attrName, TEND_NAME))
				agg2NameList = appendToTailOfList(agg2NameList, "T");
			else
				agg2NameList = appendToTailOfList(agg2NameList, ad->attrName);
		}
	}
	agg2ProjExpr = appendToTailOfList(agg2ProjExpr, aggr_0_ref);
	agg2NameList = appendToTailOfList(agg2NameList, "E");

	ProjectionOperator *proj2CP = createProjectionOp (agg2ProjExpr, agg2CPOp, NIL, agg2NameList);
	agg2CPOp->parents = singleton(proj2CP);

	QueryOperator *proj2CPOp = (QueryOperator *) proj2CP;

    //construct union on top (u1)
    SetOperator *u1 = createSetOperator(SETOP_UNION, LIST_MAKE(proj1CP, proj2CP), NIL,
    		deepCopyStringList(getAttrNames(proj2CPOp->schema)));
    ((QueryOperator *) proj1CP)->parents = singleton(u1);
    ((QueryOperator *) proj2CP)->parents = singleton(u1);



    //second union
//    UNION ALL
//     SELECT - COUNT(*) AS S, SALARY, TSTART AS T,  0 AS E
//     FROM RIGHTY
//     GROUP BY TSTART, SALARY

    //COUNT(*) FROM RIGHTY GROUP BY TSTART, SALARY

    //base projections used in Q1, then two aggr on Q1, each agg with 1 proj on top, at last union all
    List *rightProjExpr1 = NIL;
    List *rightProjAttrNames1 = NIL;

    rightProjAttrNames1 = getAttrNames(right->schema);
    FOREACH(AttributeDef, ad, right->schema->attrDefs)
    {
    	int rightBeginPos = getAttrPos(right, ad->attrName);
    	AttributeReference *rightBeginRef = createFullAttrReference(strdup(ad->attrName), 0, rightBeginPos, INVALID_ATTR, ad->dataType);
    	rightProjExpr1 = appendToTailOfList(rightProjExpr1, rightBeginRef);
    }

    rightProjAttrNames1 = appendToHeadOfList(rightProjAttrNames1, "AGG_GB_ARG0");
    rightProjExpr1 = appendToHeadOfList(rightProjExpr1, copyObject(c1));

    ProjectionOperator *rightProj = createProjectionOp(rightProjExpr1, right, NIL, deepCopyStringList(rightProjAttrNames1));
    right->parents = singleton(rightProj);

    //agg 1
    QueryOperator *rightProjOp = (QueryOperator *) rightProj;
    List *agg1GroupByRight = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(leftProj1Op, c);
//    	agg1GroupBy = appendToTailOfList(agg1GroupBy, a);
//    }
    FOREACH(AttributeDef, d, rightProjOp->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(rightProjOp, d->attrName);
        	agg1GroupByRight = appendToTailOfList(agg1GroupByRight, a);
    	}
    }

    agg1GroupByRight = appendToTailOfList(agg1GroupByRight, getAttrRefByName(rightProjOp, TBEGIN_NAME));

    AttributeReference *agg1AttrRight = getAttrRefByName(rightProjOp, "AGG_GB_ARG0");
	FunctionCall *agg1FuncRight = createFunctionCall(strdup(AGGNAME_COUNT),
			singleton(agg1AttrRight));
	List *aggrs1Right = singleton(agg1FuncRight);
	List *agg1NamesRight = NIL;

	FOREACH(AttributeDef, d, rightProjOp->schema->attrDefs)
	{
		if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName))
			agg1NamesRight = appendToTailOfList(agg1NamesRight, strdup(d->attrName));
	}
	agg1NamesRight = appendToHeadOfList(agg1NamesRight, "AGGR_0");

	AggregationOperator *agg1CPRight = createAggregationOp(aggrs1Right, agg1GroupByRight, rightProjOp, NIL, agg1NamesRight);
	rightProjOp->parents = singleton(agg1CPRight);

	QueryOperator *agg1CPRightOp = (QueryOperator *)agg1CPRight;

    // SELECT - COUNT(*) AS S, SALARY, TSTART AS T,  0 AS E
    AttributeReference *aggr_0_ref_u2 = getAttrRefByName(agg1CPRightOp, "AGGR_0");
    Operator *minusCount_u2 = createOpExpr("-", LIST_MAKE(copyObject(c0),aggr_0_ref_u2));

    //SELECT - COUNT(*) AS S, SALARY, TSTART AS T,  0 AS E

    List *projExprU2 = NIL;
    List *nameListU2 = NIL;

    projExprU2 = appendToTailOfList(projExprU2, minusCount_u2);
    nameListU2 = appendToTailOfList(nameListU2, "S");

	FOREACH(AttributeDef, ad, agg1CPRightOp->schema->attrDefs)
	{
		if(!streq(ad->attrName, "AGGR_0"))
		{
			AttributeReference *a = getAttrRefByName(agg1CPRightOp, ad->attrName);
			projExprU2 = appendToTailOfList(projExprU2, a);

			if(streq(ad->attrName, "T_B"))
				nameListU2 = appendToTailOfList(nameListU2, "T");
			else
				nameListU2 = appendToTailOfList(nameListU2, ad->attrName);
		}
	}

    projExprU2 = appendToTailOfList(projExprU2, copyObject(c0));
    nameListU2 = appendToTailOfList(nameListU2, "E");

	ProjectionOperator *projU2 = createProjectionOp (projExprU2, agg1CPRightOp, NIL, nameListU2);
	agg1CPRightOp->parents = appendToTailOfList(agg1CPRightOp->parents,projU2);
	QueryOperator *projU2Op = (QueryOperator *) projU2;

    //construct union on top (u2)
    SetOperator *u2 = createSetOperator(SETOP_UNION, LIST_MAKE(u1, projU2), NIL,
    		deepCopyStringList(getAttrNames(projU2Op->schema)));
    ((QueryOperator *) u1)->parents = singleton(u2);
    ((QueryOperator *) projU2Op)->parents = singleton(u2);


    //third union
//    UNION ALL
//    SELECT 0 AS S, SALARY, TEND AS T,  - COUNT(*) AS E
//    FROM RIGHTY
//    GROUP BY TEND, SALARY

    //  COUNT(*)  FROM RIGHTY GROUP BY TEND, SALARY
    //agg 2
//    QueryOperator *rightProj2Op = (QueryOperator *) rightProj;
    List *agg2GroupByRight = NIL;
//    FOREACH(char, c, attrNames)
//    {
//    	AttributeReference *a = getAttrRefByName(leftProj1Op, c);
//    	agg1GroupBy = appendToTailOfList(agg1GroupBy, a);
//    }
    FOREACH(AttributeDef, d, rightProjOp->schema->attrDefs)
    {
    	if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TEND_NAME, d->attrName) && !streq(TBEGIN_NAME, d->attrName))
    	{
        	AttributeReference *a = getAttrRefByName(rightProjOp, d->attrName);
        	agg2GroupByRight = appendToTailOfList(agg2GroupByRight, a);
    	}
    }

    agg2GroupByRight = appendToTailOfList(agg2GroupByRight, getAttrRefByName(rightProjOp, TEND_NAME));

    AttributeReference *agg2AttrRight = getAttrRefByName(rightProjOp, "AGG_GB_ARG0");
	FunctionCall *agg2FuncRight = createFunctionCall(strdup(AGGNAME_COUNT),
			singleton(agg2AttrRight));
	List *aggrs2Right = singleton(agg2FuncRight);
	List *agg2NamesRight = NIL;

	FOREACH(AttributeDef, d, rightProjOp->schema->attrDefs)
	{
		if(!streq("AGG_GB_ARG0", d->attrName) && !streq(TBEGIN_NAME, d->attrName))
			agg2NamesRight = appendToTailOfList(agg2NamesRight, strdup(d->attrName));
	}
	agg2NamesRight = appendToHeadOfList(agg2NamesRight, "AGGR_0");

	AggregationOperator *agg2CPRight = createAggregationOp(aggrs2Right, agg2GroupByRight, rightProjOp, NIL, agg2NamesRight);
	rightProjOp->parents = appendToTailOfList(rightProjOp->parents, agg2CPRight);

	QueryOperator *agg2CPRightOp = (QueryOperator *)agg2CPRight;

	//SELECT 0 AS S, SALARY, TEND AS T,  - COUNT(*) AS E

    AttributeReference *aggr_0_ref_u3 = getAttrRefByName(agg2CPRightOp, "AGGR_0");
    Operator *minusCount_u3 = createOpExpr("-", LIST_MAKE(copyObject(c0),aggr_0_ref_u3));

    List *projExprU3 = NIL;
    List *nameListU3 = NIL;

    projExprU3 = appendToTailOfList(projExprU3, copyObject(c0));
    nameListU3 = appendToTailOfList(nameListU3, "S");

	FOREACH(AttributeDef, ad, agg2CPRightOp->schema->attrDefs)
	{
		if(!streq(ad->attrName, "AGGR_0"))
		{
			AttributeReference *a = getAttrRefByName(agg2CPRightOp, ad->attrName);
			projExprU3 = appendToTailOfList(projExprU3, a);

			if(streq(ad->attrName, TEND_NAME))
				nameListU3 = appendToTailOfList(nameListU3, "T");
			else
				nameListU3 = appendToTailOfList(nameListU3, ad->attrName);
		}
	}

    projExprU3 = appendToTailOfList(projExprU3, minusCount_u3);
    nameListU3 = appendToTailOfList(nameListU3, "E");

	ProjectionOperator *projU3 = createProjectionOp (projExprU3, agg2CPRightOp, NIL, nameListU3);
	agg2CPRightOp->parents = appendToTailOfList(agg2CPRightOp->parents,projU3);
	QueryOperator *projU3Op = (QueryOperator *) projU3;

    //construct union on top (u3)
    SetOperator *u3 = createSetOperator(SETOP_UNION, LIST_MAKE(u2, projU3), NIL,
    		deepCopyStringList(getAttrNames(projU3Op->schema)));
    ((QueryOperator *) u2)->parents = singleton(u3);
    ((QueryOperator *) projU3Op)->parents = singleton(u3);

    QueryOperator *u3Op = (QueryOperator *) u3;

//    -- merge change points
//    , CP_MERGED AS (
//      SELECT SUM(S) AS S, SUM(E) AS E, T, SALARY
//      FROM CP
//      GROUP BY T, SALARY
//    )
    //CP_merge agg + proj
    List *groupByCPMerge = NIL;
    List *aggS = NIL;
    List *aggE = NIL;
    List *attrNamesCPMerge = NIL;

    attrNamesCPMerge = appendToTailOfList(attrNamesCPMerge, "S");
    attrNamesCPMerge = appendToTailOfList(attrNamesCPMerge, "E");
	FOREACH(AttributeDef, d, u3Op->schema->attrDefs)
	{
		AttributeReference *a = getAttrRefByName(u3Op, d->attrName);
		if(!streq("S", d->attrName) && !streq("E", d->attrName))
		{
			groupByCPMerge = appendToTailOfList(groupByCPMerge, a);
			attrNamesCPMerge = appendToTailOfList(attrNamesCPMerge, strdup(d->attrName));
		}
		else if(streq("S", d->attrName))
			aggS = appendToTailOfList(aggS,a);
		else if(streq("E", d->attrName))
			aggE = appendToTailOfList(aggE,a);
	}

	FunctionCall *sumS = createFunctionCall(AGGNAME_SUM,aggS);
	FunctionCall *sumE = createFunctionCall(AGGNAME_SUM,aggE);
	List *functionCallList = LIST_MAKE(sumS,sumE);

	AggregationOperator *aggCPMerge = createAggregationOp(functionCallList,groupByCPMerge, u3Op, NIL, attrNamesCPMerge);
    u3Op->parents = singleton(aggCPMerge);

    QueryOperator *aggCPMergeOp = (QueryOperator *) aggCPMerge;



    //internals

     //w1
     WindowBound *internalsWB1 = createWindowBound(WINBOUND_UNBOUND_PREC,NULL);
     WindowFrame *internalsWF1 = createWindowFrame(WINFRAME_RANGE,internalsWB1,NULL);

     //OrderBy
     AttributeReference *attrTW1 = getAttrRefByName(aggCPMergeOp, "T");
     List *internalsOrderBy1 = singleton(copyObject(attrTW1));

     //partationBy
     List *internalPartitionBy1 = NIL;
     FOREACH(char, c, attrNames)
     	 internalPartitionBy1 = appendToTailOfList(internalPartitionBy1,getAttrRefByName(aggCPMergeOp, c));

     WindowDef *internalWDef1 = createWindowDef(internalPartitionBy1,internalsOrderBy1,internalsWF1);

     FunctionCall *internalFC1 = createFunctionCall(AGGNAME_SUM,singleton(getAttrRefByName(aggCPMergeOp, "S")));
     WindowFunction *winternalF1 = createWindowFunction(internalFC1,internalWDef1);

     char *internalWNames1 = "winf_0";
     WindowOperator *internalW1 = createWindowOp(copyObject(winternalF1->f),
     		copyObject(winternalF1->win->partitionBy),
 			copyObject(winternalF1->win->orderBy),
 			copyObject(winternalF1->win->frame),
 			internalWNames1, aggCPMergeOp, NIL);

     aggCPMergeOp->parents = singleton(internalW1);
     QueryOperator *internalW1Op = (QueryOperator *) internalW1;


     //w2
     WindowFrame *internalsWF2 = createWindowFrame(WINFRAME_RANGE,copyObject(internalsWB1),NULL);
     List *internalsOrderBy2 = singleton(copyObject(attrTW1));
     //partationBy
     List *internalPartitionBy2 = NIL;
     FOREACH(char, c, attrNames)
     	 internalPartitionBy2 = appendToTailOfList(internalPartitionBy2,getAttrRefByName(internalW1Op, c));

     WindowDef *internalWDef2 = createWindowDef(internalPartitionBy2,internalsOrderBy2,internalsWF2);

     FunctionCall *internalFC2 = createFunctionCall(AGGNAME_SUM,singleton(getAttrRefByName(internalW1Op, "E")));
     WindowFunction *winternalF2 = createWindowFunction(internalFC2,internalWDef2);

     char *internalWNames2 = "winf_1";
     WindowOperator *internalW2 = createWindowOp(copyObject(winternalF2->f),
     		copyObject(winternalF2->win->partitionBy),
 			copyObject(winternalF2->win->orderBy),
 			copyObject(winternalF2->win->frame),
 			internalWNames2, internalW1Op, NIL);

     internalW1Op->parents = singleton(internalW2);

     QueryOperator *internalW2Op = (QueryOperator *) internalW2;

     //w3
     //WindowFrame *internalsWF2 = createWindowFrame(WINFRAME_RANGE,copyObject(internalsWB1),NULL);
     List *internalsOrderBy3 = singleton(copyObject(attrTW1));
     //partationBy
     List *internalPartitionBy3 = NIL;
     FOREACH(char, c, attrNames)
     	 internalPartitionBy3 = appendToTailOfList(internalPartitionBy3,getAttrRefByName(internalW2Op, c));

     WindowDef *internalWDef3 = createWindowDef(internalPartitionBy3,internalsOrderBy3,NULL);

     FunctionCall *internalFC3 = createFunctionCall(AGGNAME_LEAD,singleton(getAttrRefByName(internalW1Op, "T")));
     WindowFunction *winternalF3 = createWindowFunction(internalFC3,internalWDef3);

     char *internalWNames3 = "winf_3";
     WindowOperator *internalW3 = createWindowOp(copyObject(winternalF3->f),
     		copyObject(winternalF3->win->partitionBy),
 			copyObject(winternalF3->win->orderBy),
 			copyObject(winternalF3->win->frame),
 			internalWNames3, internalW2Op, NIL);

     internalW2Op->parents = singleton(internalW3);

     QueryOperator *internalW3Op = (QueryOperator *) internalW3;

     //top proj for intervals winf_0  - winf_1, T, winf_3, salary
     List *intervalsProjExpr = NIL;
     List *intervalsProjNames = NIL;

     Operator *multiplicity = createOpExpr("-",
    	 LIST_MAKE(getAttrRefByName(internalW3Op, strdup(internalWNames1)),getAttrRefByName(internalW3Op, strdup(internalWNames2))));
     intervalsProjExpr = appendToTailOfList(intervalsProjExpr, multiplicity);
     intervalsProjNames = appendToTailOfList(intervalsProjNames, NUMOPEN);


     FOREACH(AttributeDef, d, internalW3Op->schema->attrDefs)
     {
    	 if(!streq(d->attrName, "T") && !streq(d->attrName, "S") && !streq(d->attrName, "E")
    			 && !streq(d->attrName,internalWNames1) && !streq(d->attrName,internalWNames2) && !streq(d->attrName,internalWNames3))
    	 {
    		 intervalsProjExpr = appendToTailOfList(intervalsProjExpr, getAttrRefByName(internalW3Op, strdup(d->attrName)));
    		 intervalsProjNames = appendToTailOfList(intervalsProjNames, strdup(d->attrName));
    	 }
     }

     intervalsProjExpr = appendToTailOfList(intervalsProjExpr, getAttrRefByName(internalW3Op, "T"));
     intervalsProjExpr = appendToTailOfList(intervalsProjExpr, getAttrRefByName(internalW3Op, strdup(internalWNames3)));

     intervalsProjNames = appendToTailOfList(intervalsProjNames, TBEGIN_NAME);
     intervalsProjNames = appendToTailOfList(intervalsProjNames, TEND_NAME);



     DEBUG_LOG("ProjExpr size: %d, ProjNames size: %d", LIST_LENGTH(intervalsProjExpr), LIST_LENGTH(intervalsProjNames));


     ProjectionOperator *intervalsProj = createProjectionOp(intervalsProjExpr, internalW3Op, NIL, intervalsProjNames);
     internalW3Op->parents = singleton(intervalsProj);

     DEBUG_LOG("ProjExpr size: %d, ProjSchemaNames size: %d", LIST_LENGTH(intervalsProj->projExprs), LIST_LENGTH(((QueryOperator *) intervalsProj)->schema->attrDefs));


     QueryOperator *intervalsProjOp = (QueryOperator *) intervalsProj;
     //-----------------------------------------------------------------------------------------------------------------
     //TNTAB AS (SELECT rownum n FROM dual connect by level <= (SELECT MAX(MULTIPLICITY) FROM INTERVALS))
     TableAccessOperator *TNTAB = createTableAccessOp(TNTAB_DUMMY_TABLE_NAME, NULL, "TNTAB", NIL, singleton("N"), singletonInt(DT_INT));

     //set boolean prop (when translate to SQL, translate to above SQL not this table)
     SET_STRING_PROP(TNTAB, PROP_TEMP_TNTAB, createConstLong((gprom_long_t) intervalsProj));

     //---------------------------------------------------------------------------------------
     //Construct Top

     QueryOperator *TNTABOp = (QueryOperator *)TNTAB;

     //join
     AttributeReference *topAttrNum = getAttrRefByName(intervalsProjOp, NUMOPEN);
     AttributeReference *topAttrN = getAttrRefByName(TNTABOp, "N");
     topAttrN->fromClauseItem = 1;
     Operator *topCond1 = createOpExpr(OPNAME_GT, LIST_MAKE(topAttrNum,copyObject(c0)));
     Operator *topCond2 = createOpExpr(OPNAME_GE, LIST_MAKE(topAttrNum,topAttrN));
     Node *topCond = andExprList(LIST_MAKE(topCond1, topCond2));


     List *topNames = deepCopyStringList(getAttrNames(intervalsProjOp->schema));
     topNames = appendToTailOfList(topNames, "N");

     JoinOperator *topJoin = createJoinOp(JOIN_INNER, topCond, LIST_MAKE(intervalsProj, TNTAB), NIL, topNames);
     intervalsProjOp->parents = singleton(topJoin);
     TNTABOp->parents = singleton(topJoin);

     QueryOperator *topJoinOp = (QueryOperator *) topJoin;
     //projection on top
     List *topProjNames = NIL;
     FOREACH(AttributeDef, d, topJoinOp->schema->attrDefs)
     {
     	if(!streq(d->attrName, "N"))
     		topProjNames = appendToTailOfList(topProjNames, strdup(d->attrName));
     }

     QueryOperator *topProjOp = createProjOnAttrsByName(topJoinOp , topProjNames, NIL);
     topProjOp->inputs = singleton(topJoin);
     topJoinOp->parents = singleton(topProjOp);


     //additionl proj to remove attribute (multiplicity or numopen)
     List *addProjNames = NIL;
     FOREACH(char, c, getAttrNames(topProjOp->schema))
     {
    	 if(!streq(c, NUMOPEN))
    		 addProjNames = appendToTailOfList(addProjNames, strdup(c));
     }

     QueryOperator *addProj = createProjOnAttrsByName(topProjOp , addProjNames, NIL);
     QueryOperator *addProjOp = (QueryOperator *) addProj;
     addProjOp->inputs = singleton(topProjOp);
     topProjOp->parents = singleton(addProjOp);



     setTempAttrProps(addProjOp);
     int pCount = 0;
     FOREACH(AttributeDef, a, addProjOp->schema->attrDefs)
     {
     	if(streq(a->attrName, TBEGIN_NAME) || streq(a->attrName, TEND_NAME))
     		addProjOp->provAttrs = appendToTailOfListInt(addProjOp->provAttrs, pCount);
     	pCount ++;
     }



    switchSubtrees(o, (QueryOperator *) addProjOp);

//  addProvenanceAttrsToSchema((QueryOperator *) o, (QueryOperator *) lOp);
    LOG_RESULT("Rewritten set difference + normalization", addProjOp);

    return (QueryOperator *) addProjOp;
}
