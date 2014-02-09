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

static void rewritePI_CSOperator (QueryOperator *op);
static void rewritePI_CSSelection (SelectionOperator *op);
static void rewritePI_CSProjection (ProjectionOperator *op);
static void rewritePI_CSJoin (JoinOperator *op);
static void rewritePI_CSAggregation (AggregationOperator *op);
static void rewritePI_CSSet (SetOperator *op);
static void rewritePI_CSTableAccess(TableAccessOperator *op);

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
            rewritePI_CSSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            rewritePI_CSProjection((ProjectionOperator *) op);
            break;
        case T_AggregationOperator:
            rewritePI_CSAggregation ((AggregationOperator *) op);
            break;
        case T_JoinOperator:
            rewritePI_CSJoin((JoinOperator *) op);
            break;
        case T_SetOperator:
         	rewritePI_CSSet((SetOperator *) op);
            break;
        case T_TableAccessOperator:
        	rewritePI_CSTableAccess((TableAccessOperator *) op);
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

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_RCHILD(op));

    // add projection to put attributes into order
    //it is working, seems not to be necessary to add the projection any more...
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

    DEBUG_LOG("REWRITE-PICS - Aggregation");

    // create final projection and replace aggregation subtree with projection
    proj = createProjectionOp(NIL, NULL, NIL, NIL);
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);

    // create join operator
    joinProv = createJoinOp(JOIN_LEFT_OUTER, NULL, NIL, NIL,
            NIL);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) joinProv);

    // copy aggregation input
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));

    // add projection including group by expressions if necessary
    if(op->groupBy != NIL)
    {
    	List *groupByProjExprs = (List *) copyObject(op->groupBy);
    	ProjectionOperator *groupByProj = createProjectionOp(groupByProjExprs,
    			NULL, NIL, NIL);
    	addChildOperator((QueryOperator *) joinProv, (QueryOperator *) groupByProj);
    	addChildOperator((QueryOperator *) groupByProj, (QueryOperator *) aggInput);
    }
    else
		addChildOperator((QueryOperator *) joinProv, (QueryOperator *) aggInput);

    // add aggregation to join input
    addChildOperator((QueryOperator *) joinProv, (QueryOperator *) op);

    // rewrite aggregation input copy
    rewritePI_CSOperator(joinProv);

    // create join condition
    Node *joinCond;
    //TODO how to create join condition?
    // create projection expressions for final projection
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
    	// create necessary projections
    	ProjectionOperator *projLeftChild = createProjectionOp(NIL, NULL, NIL, NIL);
    	ProjectionOperator *projRwLeftChild = createProjectionOp(NIL, NULL, NIL, NIL);
    	ProjectionOperator *projRightChild = createProjectionOp(NIL, NULL, NIL, NIL);

    	// restructure the tree
    	addChildOperator((QueryOperator *) projLeftChild, (QueryOperator *) lChild);
    	addChildOperator((QueryOperator *) projRwLeftChild, (QueryOperator *) projLeftChild);
    	addChildOperator((QueryOperator *) projRightChild, (QueryOperator *) rChild);
    	switchSubtrees((QueryOperator *) lChild, (QueryOperator *) projRwLeftChild);
    	switchSubtrees((QueryOperator *) rChild, (QueryOperator *) projRightChild);

    	// adapt schema for every projections
    	addProvenanceAttrsToSchema((QueryOperator *) projRwLeftChild, OP_LCHILD(projRwLeftChild));
    	addProvenanceAttrsToSchema((QueryOperator *) projLeftChild, OP_LCHILD(projLeftChild));
    	addProvenanceAttrsToSchema((QueryOperator *) projRightChild, OP_LCHILD(projRightChild));
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
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
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
