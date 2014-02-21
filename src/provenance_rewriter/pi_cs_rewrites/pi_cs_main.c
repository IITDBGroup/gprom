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

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "provenance_rewriter/prov_schema.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

static void rewritePI_CSOperator (QueryOperator *op);
static void rewritePI_CSSelection (SelectionOperator *op);
static void rewritePI_CSProjection (ProjectionOperator *op);
static void rewritePI_CSJoin (JoinOperator *op);
static void rewritePI_CSAggregation (AggregationOperator *op);
static void rewritePI_CSSet (SetOperator *op);
static void rewritePI_CSTableAccess(TableAccessOperator *op);
static void rewritePI_CSConstRel(ConstRelOperator * op);

static Node *asOf;
static RelCount *nameState;

QueryOperator *
rewritePI_CS (ProvenanceComputation  *op)
{
    List *provAttrs = NIL;

    // unset relation name counters
    nameState = (RelCount *) NULL;

    INFO_LOG("*************************************\nREWRITE INPUT\n"
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
    INFO_LOG("rewritten query root is: %s", beatify(nodeToString(rewRoot)));

    return rewRoot;
}

static void
rewritePI_CSOperator (QueryOperator *op)
{
    switch(op->type)
    {
        case T_SelectionOperator:
         ERROR_LOG("go selection");
            rewritePI_CSSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
         ERROR_LOG("go projection");
            rewritePI_CSProjection((ProjectionOperator *) op);
            break;
        case T_AggregationOperator:
         ERROR_LOG("go aggregation");
            rewritePI_CSAggregation ((AggregationOperator *) op);
            break;
        case T_JoinOperator:
         ERROR_LOG("go join");
            rewritePI_CSJoin((JoinOperator *) op);
            break;
        case T_SetOperator:
         ERROR_LOG("go set");
         rewritePI_CSSet((SetOperator *) op);
            break;
        case T_TableAccessOperator:
         ERROR_LOG("go table access");
         rewritePI_CSTableAccess((TableAccessOperator *) op);
         break;
        case T_ConstRelOperator:
         ERROR_LOG("go const rel operator");
         rewritePI_CSConstRel((ConstRelOperator *) op);
         break;
        default:
            break;
    }
}

static void
rewritePI_CSSelection (SelectionOperator *op)
{
    assert(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child first
    rewritePI_CSOperator(OP_LCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    DEBUG_LOG("Rewritten Operator tree \n%s", beatify(nodeToString(op)));
}

static void
rewritePI_CSProjection (ProjectionOperator *op)
{
    assert(OP_LCHILD(op));

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
}

static void
rewritePI_CSJoin (JoinOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Join");

    // rewrite children
    rewritePI_CSOperator(OP_LCHILD(op));
    rewritePI_CSOperator(OP_RCHILD(op));

    // adapt schema for join op
    clearAttrsFromSchema((QueryOperator *) op);
    addNormalAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
    addNormalAttrsToSchema((QueryOperator *) op, OP_RCHILD(op));
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_RCHILD(op));

    // add projection to put attributes into order on top of join op
    ProjectionOperator *proj = createProjectionOp(NIL, NULL, NIL, NIL);
    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);
}

/*
 * Rewrite an aggregation operator:
 *      - replace aggregation with projection over join between the aggregation
 *       and the aggregation rewritten input
 */
static void
rewritePI_CSAggregation (AggregationOperator *op)
{
    JoinOperator *joinProv;
    ProjectionOperator *proj;
    QueryOperator *aggInput;
    QueryOperator *origAgg;

    DEBUG_LOG("REWRITE-PICS - Aggregation");

    // copy aggregation input
    origAgg = (QueryOperator *) op;
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));
    // rewrite aggregation input copy
    rewritePI_CSOperator(aggInput);

	// add aggregation to join input
//	addChildOperator((QueryOperator *) joinProv, (QueryOperator *) op);

    // add projection including group by expressions if necessary
    if(op->groupBy != NIL)
    {
        List *groupByProjExprs = (List *) copyObject(op->groupBy);
        ProjectionOperator *groupByProj = createProjectionOp(groupByProjExprs,
                NULL, NIL, NIL);

        // adapt schema for groupByProjection
        clearAttrsFromSchema((QueryOperator *) op);
        FOREACH(QueryOperator, q, op->op.inputs)
        {
        	addNormalAttrsToSchema((QueryOperator *) op, q);
        	addProvenanceAttrsToSchema((QueryOperator *) op, q);
        }
        //TODO how to adapt project exprs to schema?

        addChildOperator((QueryOperator *) groupByProj, (QueryOperator *) aggInput);
        aggInput = (QueryOperator *) groupByProj;
    }
//    else
//        addChildOperator((QueryOperator *) joinProv, (QueryOperator *) aggInput);

    // create join operator
    List *joinAttrNames = NIL;
    joinProv = createJoinOp(JOIN_LEFT_OUTER, NULL, LIST_MAKE(origAgg, aggInput), NIL,
            joinAttrNames);

    // create join condition
	Node *joinCond = NULL;
	if(op->groupBy != NIL)
	{
		FOREACH(AttributeReference, a , op->groupBy)
		{
			AttributeReference *lA = createAttributeReference(a->name);
			lA->fromClauseItem = 0;
			lA->attrPosition = a->attrPosition;
			AttributeReference *rA = createAttributeReference(a->name);
			rA->fromClauseItem = 1;
			rA->attrPosition = a->attrPosition; //TODO change
			if(joinCond)
			{
				joinCond = AND_EXPRS((Node *) createOpExpr("=", LIST_MAKE(lA,rA)), joinCond);
			}
			else
				joinCond = (Node *) createOpExpr("=", LIST_MAKE(lA, rA));
		}
	}
	joinProv->cond = joinCond;

	// create projection expressions for final projection

    // create final projection and replace aggregation subtree with projection
	proj = createProjectionOp(NIL, NULL, NIL, NIL);
	switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);

    addChildOperator((QueryOperator *) proj, (QueryOperator *) joinProv);

    // adapt schema for final projection

}

static void
rewritePI_CSSet(SetOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Set");

    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    // rewrite children
    rewritePI_CSOperator(lChild);
    rewritePI_CSOperator(rChild);

    switch(op->setOpType)
    {
    case SETOP_UNION:
    {
        List *projExprs = NIL;
        List *attNames;
        List *provAttrs = NIL;
        int lProvs = LIST_LENGTH(lChild->provAttrs);
        int i;

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

    	// adapt schema of union itself, we can get full provenance attributes from left input
    	addProvenanceAttrsToSchema((QueryOperator *) op, (QueryOperator *) projLeftChild);
    }
    	break;
    case SETOP_INTERSECTION:
    {
    	JoinOperator *joinOp = createJoinOp(JOIN_CROSS, NULL, NIL, NIL, NIL);

    	//restrcuture the tree
    	addChildOperator((QueryOperator *)joinOp, lChild);
    	addChildOperator((QueryOperator *)joinOp, rChild);
    	switchSubtrees((QueryOperator *) op, (QueryOperator *) joinOp);

    	//create join condition
    	Node *joinCond;//TODO
    }
    	break;
    case SETOP_DIFFERENCE:
    {
    	SelectionOperator *selOp = createSelectionOp(NULL, NULL, NIL, NIL);
    	JoinOperator *joinOp = createJoinOp(JOIN_CROSS, NULL, NIL, NIL, NIL);
    	ProjectionOperator *projOp = createProjectionOp(NIL, NULL, NIL, NIL);

    	//restructure the tree
    	switchSubtrees((QueryOperator *) op, (QueryOperator *) projOp);
    	addChildOperator((QueryOperator *) projOp, (QueryOperator *) joinOp);
    	addChildOperator((QueryOperator *) joinOp, (QueryOperator *) selOp);
    	addChildOperator((QueryOperator *) joinOp, (QueryOperator *) lChild);
    	addChildOperator((QueryOperator *) selOp, (QueryOperator *) op);

    	// adapt schema for projection
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, OP_LCHILD(projOp));

    	// create join condition
    	Node *joinCond;//TODO

    	// create selection condition
    	Node *selCond;//TODO
    }
    	break;
    default:
    	break;
    }
    // adapt schema
//    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
}

static void
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

    DEBUG_LOG("rewrite table acces: %s", operatorToOverviewString((Node *) newpo));
}

static void
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
}
