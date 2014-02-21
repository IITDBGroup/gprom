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

#include "operator_optimizer/operator_optimizer.h"
#include "operator_optimizer/operator_merge.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/query_operator/schema_utility.h"

QueryOperator *
mergeAdjacentOperators (QueryOperator *root)
{
    if (isA(root, SelectionOperator) && isA(OP_LCHILD(root), SelectionOperator))
        mergeSelection((SelectionOperator *) root);
    if (isA(root, ProjectionOperator) && isA(OP_LCHILD(root), ProjectionOperator))
        mergeProjection((ProjectionOperator *) root);

    FOREACH(QueryOperator,o,root->inputs)
         mergeAdjacentOperators(o);

    return root;
}


QueryOperator *
pushDownSelectionOperatorOnProv(QueryOperator *root)
{
    QueryOperator *newRoot = root;

	if (isA(root, SelectionOperator) && isA(OP_LCHILD(root), ProjectionOperator)) {
		newRoot = (QueryOperator *) pushDownSelectionWithProjection((SelectionOperator *) root);
	}

	FOREACH(QueryOperator, o, newRoot->inputs)
		pushDownSelectionOperatorOnProv(o);

	return newRoot;
}
