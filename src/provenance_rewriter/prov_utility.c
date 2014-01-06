/*-----------------------------------------------------------------------------
 *
 * prov_utility.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "provenance_rewriter/prov_utility.h"

void
addProvenanceAttrsToSchema(QueryOperator *target, QueryOperator *source)
{
//    target->schema = concatTwoLists(target->schema, copyObject(getProvenanceAttrs(source)));
}

/*
 * Given a subtree rooted a "orig" replace this subtree with the tree rooted
 * at "new". This method adapts all input lists of all parents of "orig" to point
 * to "new" instead.
 */
void
switchSubtrees(QueryOperator *orig, QueryOperator *new)
{
    // copy list of parents to new subtree
    new->parents = orig->parents;
    orig->parents = NIL;

    // adapt original parent's inputs
    FOREACH(QueryOperator,parent,orig->parents)
    {
        FOREACH(QueryOperator,pChild,parent->inputs)
        {
            if (equal(pChild,orig))
                pChild_his_cell->data.ptr_value = new;
        }
    }
}

/*
 * Create a copy of an operator subtree without copying its parents.
 */
QueryOperator *
copyUnrootedSubtree(QueryOperator *op)
{
    List *parents;
    QueryOperator *result;

    // cache parents
    parents = op->parents;
    op->parents = NULL;

    // copy
    result = copyObject(op);

    // restore parents
    op->parents = parents;

    return result;
}

/*
 * Find table access operators
 */

boolean
findTableAccessVisitor (Node *node, List **result)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, TableAccessOperator))
        *result = appendToTailOfList(*result, node);

    return visit(node, findTableAccessVisitor, (void *) result);
}
