/*-----------------------------------------------------------------------------
 *
 * operator_merge.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "model/query_operator/query_operator.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "operator_optimizer/operator_merge.h"

SelectionOperator *
mergeSelection(SelectionOperator *op)
{
    while(isA(OP_LCHILD(op),SelectionOperator))
    {
        SelectionOperator *child = (SelectionOperator *) OP_LCHILD(op);

        // and condition and link child's children to root
        op->cond = (Node *) createOpExpr("AND", LIST_MAKE(op->cond, child->cond));
        op->op.inputs = child->op.inputs;

        // clean up child
        child->cond = NULL;
        child->op.inputs = NULL;
        deepFree(child);
    }

    return op;
}

ProjectionOperator *
mergeProjection(ProjectionOperator *op)
{
//    while(isA(OP_LCHILD(op),ProjectionOperator))
//    {
//        ProjectionOperator *child = (ProjectionOperator *) OP_LCHILD(op);
//
//        // and condition and link child's children to root
//        op->cond = (Node *) createOpExpr("AND", LIST_MAKE(op->cond, child->cond));
//        op->op.inputs = child->op.inputs;
//
//        // clean up child
//        child->cond = NULL;
//        child->op.inputs = NULL;
//        deepFree(child);
//    }

    return op;
}
