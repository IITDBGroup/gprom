/*-----------------------------------------------------------------------------
 *
 * pi_cs_main.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "instrumentation/timing_instrumentation.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "provenance_rewriter/prov_schema.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

static QueryOperator *rewritePI_CSOperator (QueryOperator *op);
static QueryOperator *rewritePI_CSSelection (SelectionOperator *op);
static QueryOperator *rewritePI_CSProjection (ProjectionOperator *op);
static QueryOperator *rewritePI_CSJoin (JoinOperator *op);
static QueryOperator *rewritePI_CSAggregation (AggregationOperator *op);
static QueryOperator *rewritePI_CSSet (SetOperator *op);
static QueryOperator *rewritePI_CSTableAccess(TableAccessOperator *op);
static QueryOperator *rewritePI_CSConstRel(ConstRelOperator *op);
static QueryOperator *rewritePI_CSDuplicateRemOp(DuplicateRemoval *op);
static QueryOperator *rewritePI_CSOrderOp(OrderOperator *op);

static QueryOperator *addIntermediateProvenance (QueryOperator *op, List *userProvAttrs);
static QueryOperator *rewritePI_CSAddProvNoRewrite (QueryOperator *op, List *userProvAttrs);
static QueryOperator *rewritePI_CSUseProvNoRewrite (QueryOperator *op, List *userProvAttrs);


static Node *asOf;
static RelCount *nameState;

QueryOperator *
rewritePI_CS (ProvenanceComputation  *op)
{
    List *provAttrs = NIL;

    START_TIMER("rewrite - PI-CS rewrite");

    // unset relation name counters
    nameState = (RelCount *) NULL;

    DEBUG_LOG("*************************************\nREWRITE INPUT\n"
            "******************************\n%s",
            beatify(nodeToString((Node *) op)));

    QueryOperator *rewRoot = OP_LCHILD(op);
    DEBUG_LOG("rewRoot is: %s", beatify(nodeToString(rewRoot)));

    // cache asOf
    asOf = op->asOf;

    // get provenance attrs
    provAttrs = getQueryOperatorAttrNames((QueryOperator *) op);

    // rewrite subquery under provenance computation
    rewritePI_CSOperator(rewRoot);

    // update root of rewritten subquery
    rewRoot = OP_LCHILD(op);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, rewRoot);
    DEBUG_LOG("rewritten query root is: %s", beatify(nodeToString(rewRoot)));

    STOP_TIMER("rewrite - PI-CS rewrite");

    return rewRoot;
}

static QueryOperator *
rewritePI_CSOperator (QueryOperator *op)
{
    boolean showIntermediate = HAS_STRING_PROP(op,  PROP_SHOW_INTERMEDIATE_PROV);
    boolean noRewriteUseProv = HAS_STRING_PROP(op, PROP_USE_PROVENANCE);
    boolean noRewriteHasProv = HAS_STRING_PROP(op, PROP_HAS_PROVENANCE);
    List *userProvAttrs = (List *) getStringProperty(op, PROP_USER_PROV_ATTRS);

    QueryOperator *rewrittenOp;

    if (noRewriteUseProv)
        return rewritePI_CSAddProvNoRewrite(op, userProvAttrs);
    if (noRewriteHasProv)
        return rewritePI_CSUseProvNoRewrite(op, userProvAttrs);

    switch(op->type)
    {
        case T_SelectionOperator:
            DEBUG_LOG("go selection");
            rewrittenOp = rewritePI_CSSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            DEBUG_LOG("go projection");
            rewrittenOp = rewritePI_CSProjection((ProjectionOperator *) op);
            break;
        case T_AggregationOperator:
            DEBUG_LOG("go aggregation");
            rewrittenOp = rewritePI_CSAggregation ((AggregationOperator *) op);
            break;
        case T_JoinOperator:
            DEBUG_LOG("go join");
            rewrittenOp = rewritePI_CSJoin((JoinOperator *) op);
            break;
        case T_SetOperator:
            DEBUG_LOG("go set");
            rewrittenOp = rewritePI_CSSet((SetOperator *) op);
            break;
        case T_TableAccessOperator:
            DEBUG_LOG("go table access");
            rewrittenOp = rewritePI_CSTableAccess((TableAccessOperator *) op);
            break;
        case T_ConstRelOperator:
            DEBUG_LOG("go const rel operator");
            rewrittenOp = rewritePI_CSConstRel((ConstRelOperator *) op);
            break;
        case T_DuplicateRemoval:
            DEBUG_LOG("go duplicate removal operator");
            rewrittenOp = rewritePI_CSDuplicateRemOp((DuplicateRemoval *) op);
            break;
        case T_OrderOperator:
            DEBUG_LOG("fo order operator");
            rewrittenOp = rewritePI_CSOrderOp((OrderOperator *) op);
            break;
        default:
            FATAL_LOG("no rewrite implemented for operator ", nodeToString(op));
            return NULL;
    }

    if (showIntermediate)
        rewrittenOp = addIntermediateProvenance(rewrittenOp, userProvAttrs);

    return rewrittenOp;
}

static QueryOperator *
addIntermediateProvenance (QueryOperator *op, List *userProvAttrs)
{
    QueryOperator *proj;
    List *attrNames = NIL;
    List *projExpr = NIL;
    List *provAttrPos = NIL;
    List *normalAttrExpr = getNormalAttrProjectionExprs(op);
    int cnt = 0;
    char *newAttrName;
    int relAccessCount;
    char *tableName = "INTERMEDIATE";

    if (isA(op,TableAccessOperator))
        tableName = ((TableAccessOperator *) op)->tableName;

    relAccessCount = getRelNameCount(&nameState, tableName);

    DEBUG_LOG("REWRITE-PICS - Add Intermediate Provenance Attrs <%s> <%u>",  tableName, relAccessCount);

    attrNames = getQueryOperatorAttrNames(op);
    provAttrPos = copyObject(op->provAttrs);

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, op->schema->attrDefs)
    {
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0));
        cnt++;
    }

    FOREACH(AttributeReference, a, normalAttrExpr)
    {
        //TODO naming of intermediate results
        newAttrName = getProvenanceAttrName(tableName, a->name, relAccessCount);
        DEBUG_LOG("new attr name: %s", newAttrName);
        attrNames = appendToTailOfList(attrNames, newAttrName);
        projExpr = appendToTailOfList(projExpr, a);
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, cnt + LIST_LENGTH(normalAttrExpr) - 1, 1);
    provAttrPos = CONCAT_LISTS(provAttrPos, newProvPosList);
    DEBUG_LOG("add intermediate provenance\n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(attrNames),
            nodeToString(projExpr),
            nodeToString(provAttrPos));

    // Create a new projection operator with these new attributes
    proj = (QueryOperator *) createProjectionOp(projExpr, NULL, NIL, attrNames);
    proj->provAttrs = provAttrPos;

    // Switch the subtree with this newly created projection operator.
    switchSubtreeWithExisting((QueryOperator *) op, (QueryOperator *) proj);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    DEBUG_LOG("added projection: %s", operatorToOverviewString((Node *) proj));

    return proj;
}

static QueryOperator *
rewritePI_CSAddProvNoRewrite (QueryOperator *op, List *userProvAttrs)
{
    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;
    int relAccessCount;
    int numProvAttrs = LIST_LENGTH(userProvAttrs);
    int numNormalAttrs = LIST_LENGTH(op->schema->attrDefs);
    int cnt = 0;
    char *tableName = "INTERMEDIATE";

    if (isA(op,TableAccessOperator))
        tableName = ((TableAccessOperator *) op)->tableName;

    DEBUG_LOG("REWRITE-PICS - Add Provenance Attrs <%s> <%u>",  tableName, relAccessCount);

    relAccessCount = getRelNameCount(&nameState, tableName);


    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0));
        cnt++;
    }

    cnt = 0;
    FOREACH(Constant, attr, userProvAttrs)
    {
        char *name = STRING_VALUE(attr);
        newAttrName = getProvenanceAttrName(tableName, name, relAccessCount);
        provAttr = appendToTailOfList(provAttr, newAttrName);
        cnt = getAttrPos(op,name);
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(name, 0, cnt, 0));
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, numNormalAttrs, numNormalAttrs + numProvAttrs - 1, 1);

    DEBUG_LOG("no rewrite add provenance, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite add provenance attrs:\n%s", operatorToOverviewString((Node *) newpo));
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSUseProvNoRewrite (QueryOperator *op, List *userProvAttrs)
{
    List *provAttrs = op->provAttrs;
    int relAccessCount;
    char *tableName = "USER";
    boolean isTableAccess = isA(op,TableAccessOperator);

    if (isTableAccess)
        tableName = ((TableAccessOperator *) op)->tableName;

    relAccessCount = getRelNameCount(&nameState, tableName);

    // for table access operations we need to add a projection that renames the attributes
    if (isTableAccess)
    {
        QueryOperator *proj;

        proj = createProjOnAllAttrs(op);

        // Switch the subtree with this newly created projection operator
        switchSubtrees(op, proj);

        // Add child to the newly created projection operator
        addChildOperator(proj, op);

        FOREACH(Constant,a,userProvAttrs)
        {
            char *name = STRING_VALUE(a);
            int pos = getAttrPos(proj, name);
            AttributeDef *attr;

            attr = getNthOfListP(proj->schema->attrDefs, pos);
            name = getProvenanceAttrName(tableName, name, relAccessCount);
            attr->attrName = name;
            provAttrs = appendToTailOfListInt(provAttrs, pos);

            // in parent operators adapt attribute references to use new name
            FOREACH(QueryOperator,p,proj->parents)
            {
                List *aRefs = findOperatorAttrRefs(p);
                int childPos = getChildPosInParent(p,proj);

                FOREACH(AttributeReference,a,aRefs)
                {
                    if (a->fromClauseItem == childPos && a->attrPosition == pos)
                        a->name = strdup(name);
                }
            }
        }

        proj->provAttrs = provAttrs;

        return proj;
    }
    // for non-tableaccess operators simply change the attribute names and mark the attributes as provenance attributes
    else
    {
        FOREACH(Constant,a,userProvAttrs)
        {
            char *name = STRING_VALUE(a);
            int pos = getAttrPos(op, name);
//            char *oldName;
            AttributeDef *attr;

            attr = getNthOfListP(op->schema->attrDefs, pos);
            name = getProvenanceAttrName(tableName, name, relAccessCount);
//            oldName = attr->attrName;
            attr->attrName = name;
            provAttrs = appendToTailOfListInt(provAttrs, pos);

            // in parent operators adapt attribute references to use new name
            FOREACH(QueryOperator,p,op->parents)
            {
                List *aRefs = findOperatorAttrRefs(p);
                int childPos = getChildPosInParent(p,op);

                FOREACH(AttributeReference,a,aRefs)
                {
                    if (a->fromClauseItem == childPos && a->attrPosition == pos)
                        a->name = strdup(name);
                }
            }
        }

        op->provAttrs = provAttrs;

        return op;
    }
}

static QueryOperator *
rewritePI_CSSelection (SelectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child first
    rewritePI_CSOperator(OP_LCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(op)));
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSProjection (ProjectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child
    rewritePI_CSOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(op);
    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
         op->projExprs = appendToTailOfList(op->projExprs,
                 createFullAttrReference(att->attrName, 0, a, 0));
    }

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(op)));
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSJoin (JoinOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Join");
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    // rewrite children
    lChild = rewritePI_CSOperator(lChild);
    rChild = rewritePI_CSOperator(rChild);

    // adapt schema for join op
    clearAttrsFromSchema((QueryOperator *) op);
    addNormalAttrsToSchema((QueryOperator *) op, lChild);
    addProvenanceAttrsToSchema((QueryOperator *) op, lChild);
    addNormalAttrsToSchema((QueryOperator *) op, rChild);
    addProvenanceAttrsToSchema((QueryOperator *) op, rChild);

    // add projection to put attributes into order on top of join op
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) op),
            getProvAttrProjectionExprs((QueryOperator *) op));
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(op)));
    return (QueryOperator *) op;
}

/*
 * Rewrite an aggregation operator:
 *      - replace aggregation with projection over join between the aggregation
 *       and the aggregation rewritten input
 */
static QueryOperator *
rewritePI_CSAggregation (AggregationOperator *op)
{
    JoinOperator *joinProv;
    ProjectionOperator *proj;
    QueryOperator *aggInput;
    QueryOperator *origAgg;
    int numGroupAttrs = LIST_LENGTH(op->groupBy);

    DEBUG_LOG("REWRITE-PICS - Aggregation");

    // copy aggregation input
    origAgg = (QueryOperator *) op;
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));
    // rewrite aggregation input copy
    aggInput = rewritePI_CSOperator(aggInput);

    // add projection including group by expressions if necessary
    if(op->groupBy != NIL)
    {
        List *groupByProjExprs = (List *) copyObject(op->groupBy);
        List *attrNames = NIL;
        List *provAttrs = NIL;
        ProjectionOperator *groupByProj;
        List *gbNames = aggOpGetGroupByAttrNames(op);
        ListCell *lc;

        // adapt right side group by attr names
        FOREACH_LC(lc,gbNames)
        {
            char *name = (char *) LC_P_VAL(lc);
            LC_P_VAL(lc) = CONCAT_STRINGS("_P_SIDE_", name);
        }

        attrNames = CONCAT_LISTS(gbNames, getOpProvenanceAttrNames(aggInput));
        groupByProjExprs = CONCAT_LISTS(groupByProjExprs, getProvAttrProjectionExprs(aggInput));

        groupByProj = createProjectionOp(groupByProjExprs,
                        aggInput, NIL, attrNames);
        CREATE_INT_SEQ(provAttrs, numGroupAttrs, numGroupAttrs + getNumProvAttrs(aggInput) - 1,1);
        groupByProj->op.provAttrs = provAttrs;
        aggInput->parents = singleton(groupByProj);
        aggInput = (QueryOperator *) groupByProj;
    }

    // create join condition
	Node *joinCond = NULL;
	JoinType joinT = (op->groupBy) ? JOIN_INNER : JOIN_LEFT_OUTER;

	// create join condition for group by
	if(op->groupBy != NIL)
	{
	    int pos = 0;
	    List *groupByNames = aggOpGetGroupByAttrNames(op);

		FOREACH(AttributeReference, a , op->groupBy)
		{
		    char *name = getNthOfListP(groupByNames, pos);
			AttributeReference *lA = createFullAttrReference(name, 0, LIST_LENGTH(op->aggrs) + pos, INVALID_ATTR);
			AttributeReference *rA = createFullAttrReference(
			        CONCAT_STRINGS("_P_SIDE_",name), 1, pos, INVALID_ATTR);
			if(joinCond)
				joinCond = AND_EXPRS((Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA), joinCond);
			else
				joinCond = (Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA);
			pos++;
		}
	}
	// or for without group by
	else
	    joinCond = (Node *) createOpExpr("=", LIST_MAKE(createConstInt(1), createConstInt(1)));

    // create join operator
    List *joinAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getQueryOperatorAttrNames(aggInput));
    joinProv = createJoinOp(joinT, joinCond, LIST_MAKE(origAgg, aggInput), NIL,
            joinAttrNames);
    joinProv->op.provAttrs = copyObject(aggInput->provAttrs);
    FOREACH_LC(lc,joinProv->op.provAttrs)
        lc->data.int_value += getNumAttrs(origAgg);

	// create projection expressions for final projection
    List *projAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getOpProvenanceAttrNames(aggInput));
    List *projExprs = CONCAT_LISTS(getNormalAttrProjectionExprs(origAgg),
                                getProvAttrProjectionExprs((QueryOperator *) joinProv));

    // create final projection and replace aggregation subtree with projection
	proj = createProjectionOp(projExprs, (QueryOperator *) joinProv, NIL, projAttrNames);
	joinProv->op.parents = singleton(proj);
	CREATE_INT_SEQ(proj->op.provAttrs, getNumNormalAttrs((QueryOperator *) origAgg),
	        getNumNormalAttrs((QueryOperator *) origAgg) + getNumProvAttrs((QueryOperator *) joinProv) - 1,1);

	// switch provenance computation with original aggregation
	switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addParent(origAgg, (QueryOperator *) joinProv);
    addParent(aggInput, (QueryOperator *) joinProv);

    // adapt schema for final projection
    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(proj)));
    return (QueryOperator *) proj;
}

static QueryOperator *
rewritePI_CSSet(SetOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Set");

    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    switch(op->setOpType)
    {
    case SETOP_UNION:
    {
        List *projExprs = NIL;
        List *attNames;
        List *provAttrs = NIL;
        int lProvs;
        int i;

        // rewrite children
        lChild = rewritePI_CSOperator(lChild);
        rChild = rewritePI_CSOperator(rChild);
        lProvs = LIST_LENGTH(lChild->provAttrs);

        // create projection over left rewritten input
        attNames = concatTwoLists(getQueryOperatorAttrNames(lChild), getOpProvenanceAttrNames(rChild));

        // createAttrRefs for attributes of left input
        i = 0;
        FOREACH(AttributeDef,a,lChild->schema->attrDefs)
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i++, INVALID_ATTR);
            projExprs = appendToTailOfList(projExprs, att);
        }
        provAttrs = copyObject(lChild->provAttrs);

        // create NULL expressions for provenance attrs of right input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rChild))
        {
            Constant *expr;

            expr = createNullConst(a->dataType);
            projExprs = appendToTailOfList(projExprs, expr);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }
        DEBUG_LOG("have created projection expression: %s\nattribute names: "
                "%s\n provAttrs: %s\n for left UNION input",
                nodeToString(projExprs), stringListToString(attNames),
                nodeToString(provAttrs));

        ProjectionOperator *projLeftChild = createProjectionOp(projExprs,
                lChild, NIL, attNames);
        ((QueryOperator *) projLeftChild)->provAttrs = provAttrs;

        // create projection over right rewritten input
        provAttrs = NIL;
        projExprs = NIL;
        attNames = CONCAT_LISTS(getNormalAttrNames(rChild),
                getOpProvenanceAttrNames(lChild),
                getOpProvenanceAttrNames(rChild));

        // create AttrRefs for normal attributes of right input
        i = 0;
        FOREACH(AttributeDef,a,getNormalAttrs(rChild))
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i++, INVALID_ATTR);
            projExprs = appendToTailOfList(projExprs, att);
        }

        // create NULL expressions for provenance attrs of left input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(lChild))
        {
            Constant *expr;

            expr = createNullConst(a->dataType);
            projExprs = appendToTailOfList(projExprs, expr);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }

        // create AttrRefs for provenance attrs of right input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rChild))
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i - lProvs, INVALID_ATTR);
            projExprs = appendToTailOfList(projExprs, att);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }

        DEBUG_LOG("have created projection expressions: %s\nattribute names: "
                "%s\n provAttrs: %s\n for right UNION input",
                nodeToString(projExprs), stringListToString(attNames),
                nodeToString(provAttrs));
        ProjectionOperator *projRightChild = createProjectionOp(projExprs,
                rChild, NIL, attNames);
        ((QueryOperator *) projRightChild)->provAttrs = provAttrs;

    	// make projections of rewritten inputs the direct children of the union operation
        switchSubtrees(lChild, (QueryOperator *) projLeftChild);
        switchSubtrees(rChild, (QueryOperator *) projRightChild);
        lChild->parents = singleton(projLeftChild);
        rChild->parents = singleton(projRightChild);

    	// adapt schema of union itself, we can get full provenance attributes from left input
    	addProvenanceAttrsToSchema((QueryOperator *) op, (QueryOperator *) projLeftChild);
        return (QueryOperator *) op;
    }
    case SETOP_INTERSECTION:
    {
    	JoinOperator *joinOp = createJoinOp(JOIN_CROSS, NULL, NIL, NIL, NIL);

    	//restrcuture the tree
    	addChildOperator((QueryOperator *)joinOp, lChild);
    	addChildOperator((QueryOperator *)joinOp, rChild);
    	switchSubtrees((QueryOperator *) op, (QueryOperator *) joinOp);

    	//create join condition
    	Node *joinCond;//TODO

    	return (QueryOperator *) joinOp;
    }
    case SETOP_DIFFERENCE:
    {
    	JoinOperator *joinOp;
    	ProjectionOperator *projOp;
    	QueryOperator *rewrLeftChild = rewritePI_CSOperator(
    	        copyUnrootedSubtree(lChild));

    	// join provenance with rewritten right input
    	// create join condition
        Node *joinCond;
        List *joinAttrs = CONCAT_LISTS(getQueryOperatorAttrNames((QueryOperator *) op),
                getQueryOperatorAttrNames(rewrLeftChild));
    	joinCond = NULL;

        FORBOTH(AttributeReference, aL , aR, getNormalAttrProjectionExprs(lChild),
                getNormalAttrProjectionExprs(rewrLeftChild))
        {
            aL->fromClauseItem = 0;
            aR->fromClauseItem = 1;
            if(joinCond)
                joinCond = AND_EXPRS((Node *) createIsNotDistinctExpr((Node *) aL, (Node *) aR), joinCond);
            else
                joinCond = (Node *) createIsNotDistinctExpr((Node *) aL, (Node *) aR);
        }

        joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(op, rewrLeftChild),
                NIL, joinAttrs);
        joinOp->op.provAttrs = copyObject(rewrLeftChild->provAttrs);
        SHIFT_INT_LIST(joinOp->op.provAttrs, getNumAttrs((QueryOperator *) op));

    	// adapt schema using projection
        List *projExpr = CONCAT_LISTS(getNormalAttrProjectionExprs((QueryOperator *)op),
                getProvAttrProjectionExprs((QueryOperator *) joinOp));
        List *projAttrs = CONCAT_LISTS(getQueryOperatorAttrNames((QueryOperator *) op),
                getOpProvenanceAttrNames((QueryOperator *) joinOp));
        projOp = createProjectionOp(projExpr, (QueryOperator *) joinOp, NIL, projAttrs);
        projOp->op.provAttrs = copyObject(rewrLeftChild->provAttrs);
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, OP_LCHILD(projOp));

    	// switch original set diff with projection
    	switchSubtrees((QueryOperator *) op, (QueryOperator *) projOp);

    	return (QueryOperator *) projOp;
    }
    default:
    	break;
    }
    return NULL;
}

static QueryOperator *
rewritePI_CSTableAccess(TableAccessOperator *op)
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

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite table access: %s", operatorToOverviewString((Node *) newpo));
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSConstRel(ConstRelOperator *op)
{
    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;

    int relAccessCount = getRelNameCount(&nameState, "query");
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS - Const Rel Operator <%s> <%u>", nodeToString(op->values), relAccessCount);

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
        newAttrName = getProvenanceAttrName("query", attr->attrName, relAccessCount);
        provAttr = appendToTailOfList(provAttr, newAttrName);
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0));
        cnt++;
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

    DEBUG_LOG("rewrite const rel operator, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite const rel operator: %s", operatorToOverviewString((Node *) newpo));
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSDuplicateRemOp(DuplicateRemoval *op)
{
    QueryOperator *child = OP_LCHILD(op);
    QueryOperator *theOp = (QueryOperator *) op;

    // remove duplicate removal op
    removeParentFromOps(singleton(child), theOp);
    switchSubtreeWithExisting(theOp, child);

    return rewritePI_CSOperator(child);
}

static QueryOperator *
rewritePI_CSOrderOp(OrderOperator *op)
{
    QueryOperator *child = OP_LCHILD(op);

    // rewrite child
    rewritePI_CSOperator(child);

    // adapt provenance attr list and schema
    addProvenanceAttrsToSchema((QueryOperator *) op, child);

    return (QueryOperator *) op;
}
