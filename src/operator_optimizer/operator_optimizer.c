/*-----------------------------------------------------------------------------
 *
 * operator_optimizer.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "log/logger.h"
#include "operator_optimizer/operator_optimizer.h"
#include "operator_optimizer/operator_merge.h"
#include "operator_optimizer/expr_attr_factor.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/query_operator/schema_utility.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"

static QueryOperator *optimizeOneGraph (QueryOperator *root);

Node  *
optimizeOperatorModel (Node *root)
{
    if(isA(root, List))
    {
        FOREACH_LC(lc, (List *) root)
        {
            QueryOperator *o = (QueryOperator *) LC_P_VAL(lc);

            o = optimizeOneGraph(o);
            LC_P_VAL(lc) = o;
        }

        return root;
    }
    else
        return (Node *) optimizeOneGraph((QueryOperator *) root);
}

static QueryOperator *
optimizeOneGraph (QueryOperator *root)
{
    QueryOperator *rewrittenTree = root;

    if(getBoolOption(OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR))
    {
        START_TIMER("OptimizeModel - factor attributes in conditions");
        rewrittenTree = factorAttrsInExpressions((QueryOperator *) rewrittenTree);
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
        DEBUG_LOG("factor out attribute references in conditions\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        STOP_TIMER("OptimizeModel - factor attributes in conditions");
    }

    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
    {
        START_TIMER("OptimizeModel - merge adjacent operator");
        rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
        DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        STOP_TIMER("OptimizeModel - merge adjacent operator");
    }

    if(getBoolOption(OPTIMIZATION_SELECTION_PUSHING))
    {
        START_TIMER("OptimizeModel - pushdown selections");
        rewrittenTree = pushDownSelectionOperatorOnProv((QueryOperator *) rewrittenTree);
        DEBUG_LOG("selections pushed down\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
        STOP_TIMER("OptimizeModel - pushdown selections");
    }

    if(getBoolOption(OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR))
    {
        START_TIMER("OptimizeModel - factor attributes in conditions");
        rewrittenTree = factorAttrsInExpressions((QueryOperator *) rewrittenTree);
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
        DEBUG_LOG("factor out attribute references in conditions again\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        STOP_TIMER("OptimizeModel - factor attributes in conditions");
    }

    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
    {
        START_TIMER("OptimizeModel - merge adjacent operator");
        rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
        DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
        STOP_TIMER("OptimizeModel - merge adjacent operator");
    }

    if(getBoolOption(OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ))
    {
        START_TIMER("OptimizeModel - set materialization hints");
        rewrittenTree = materializeProjectionSequences((QueryOperator *) rewrittenTree);
        DEBUG_LOG("add materialization hints for projection sequences\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        ASSERT(checkModel((QueryOperator *) rewrittenTree));
        STOP_TIMER("OptimizeModel - set materialization hints");
    }
    return rewrittenTree;
}

QueryOperator *
materializeProjectionSequences (QueryOperator *root)
{
    QueryOperator *lChild = OP_LCHILD(root);

    // if two adjacent projections then materialize the lower one
    if (isA(root, ProjectionOperator) && isA(lChild, ProjectionOperator))
        SET_BOOL_STRING_PROP(lChild, PROP_MATERIALIZE);

    FOREACH(QueryOperator,o,root->inputs)
        materializeProjectionSequences(o);

    return root;
}


QueryOperator *
mergeAdjacentOperators (QueryOperator *root)
{
    if (isA(root, SelectionOperator) && isA(OP_LCHILD(root), SelectionOperator))
        root = (QueryOperator *) mergeSelection((SelectionOperator *) root);
    if (isA(root, ProjectionOperator) && isA(OP_LCHILD(root), ProjectionOperator))
        root = (QueryOperator *) mergeProjection((ProjectionOperator *) root);

    FOREACH(QueryOperator,o,root->inputs)
         mergeAdjacentOperators(o);

    return root;
}


QueryOperator *
pushDownSelectionOperatorOnProv(QueryOperator *root)
{
    QueryOperator *newRoot = root;

	if (isA(root, SelectionOperator) && isA(OP_LCHILD(root), ProjectionOperator))
		newRoot = pushDownSelectionWithProjection((SelectionOperator *) root);

	FOREACH(QueryOperator, o, newRoot->inputs)
		pushDownSelectionOperatorOnProv(o);

	return newRoot;
}

QueryOperator *
factorAttrsInExpressions(QueryOperator *root)
{
    QueryOperator *newRoot = root;

    if (isA(root, ProjectionOperator))
        newRoot = projectionFactorAttrReferences((ProjectionOperator *) root);

    FOREACH(QueryOperator, o, newRoot->inputs)
        factorAttrsInExpressions(o);

    return root;
}
