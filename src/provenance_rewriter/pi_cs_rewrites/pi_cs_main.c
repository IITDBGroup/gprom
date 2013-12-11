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
#include "provenance_rewriter/prov_schema.h"

static void rewritePI_CSOperator (QueryOperator *op);
static void rewritePI_CSSelection (SelectionOperator *op);
static void rewritePI_CSProjection (ProjectionOperator *op);
static void rewritePI_CSJoin (JoinOperator *op);
static void rewritePI_CSAggregation (AggregationOperator *op);
static void rewritePI_CSSet (SetOperator *op);
static void rewritePI_CSTableAccess(TableAccessOperator *op);

QueryOperator *
rewritePI_CS (ProvenanceComputation  *op)
{
    QueryOperator *rewRoot = OP_LCHILD(op);

    // rewrite subquery under provenance computation
    rewritePI_CSOperator(rewRoot);

    // update root of rewritten subquery
    rewRoot = OP_LCHILD(op);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, rewRoot);
    return rewRoot;
}

static void
rewritePI_CSOperator (QueryOperator *op)
{
    switch(op->type)
    {
        case T_TableAccessOperator:
            rewritePI_CSTableAccess((TableAccessOperator *) op);
            break;
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
        default:
            break;
    }
}

static void
rewritePI_CSSelection (SelectionOperator *op)
{
    // rewrite child first
    rewritePI_CSOperator(OP_LCHILD(op));

//     adapt schema addProvenanceAttrsToSchema(op, OP_LCHILD(op));
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
}

static void
rewritePI_CSProjection (ProjectionOperator *op)
{
    // rewrite child
    rewritePI_CSOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs
    TableAccessOperator *t = OP_LCHILD(op->op.inputs);
    FOREACH(AttributeDef, a, t->op.schema->attrDefs)
    	t->op.provAttrs = appendToTailOfList(t->op.provAttrs, createAttributeReference(a->attrName));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
}

static void
rewritePI_CSJoin (JoinOperator *op)
{
    // rewrite children
    rewritePI_CSOperator(OP_LCHILD(op));
    rewritePI_CSOperator(OP_RCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
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

    // add aggregation to join input
    addChildOperator((QueryOperator *) joinProv, (QueryOperator *) op);
    addChildOperator((QueryOperator *) joinProv, (QueryOperator *) aggInput);

    // rewrite aggregation input copy
    rewritePI_CSOperator(aggInput);

    // create join condition

    // create projection expressions for final projection
}

static void
rewritePI_CSSet(SetOperator *op)
{
    // rewrite children
    rewritePI_CSOperator(OP_LCHILD(op));
    rewritePI_CSOperator(OP_RCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
}

static void 
rewritePI_CSTableAccess(TableAccessOperator *op)
{
    List *tableAttr;
    List *provAttr;
    List *projExpr;
    char *newAttrName;
    int state = 1;
    int cnt=1;

    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        newAttrName = getProvenanceAttrName(op->tableName, attr->attrName, state);
        provAttr = appendToTailOfList(provAttr, newAttrName);
        projExpr = appendToTailOfList(projExpr, attr->attrName);
        cnt++;
    }

    projExpr = appendToTailOfList(projExpr, provAttr);
    
    int i = cnt;
    List *newProvPosList;
    newProvPosList = singleton((int *) cnt);
    for(i=cnt+1; i <= 2*cnt-1; cnt++) {
        newProvPosList = appendToTailOfList(newProvPosList, (int *) i);
    }
    
    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, (QueryOperator *) op, NIL, provAttr);

    newpo->op.provAttrs = newProvPosList;

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add childs to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);
}
