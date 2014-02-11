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
#include "log/logger.h"

void
clearAttrsFromSchema(QueryOperator *target)
{
	target->schema->attrDefs = NIL;
}

void
addNormalAttrsToSchema(QueryOperator *target, QueryOperator *source)
{
	List *newAttrs = (List *) copyObject(getNormalAttrs(source));
	target->schema->attrDefs = concatTwoLists(target->schema->attrDefs, newAttrs);
}

void
addProvenanceAttrsToSchema(QueryOperator *target, QueryOperator *source)
{
    List *newProvAttrs = (List *) copyObject(getProvenanceAttrDefs(source));
    int curAttrLen = LIST_LENGTH(target->schema->attrDefs);
    int numProvAttrs = LIST_LENGTH(newProvAttrs);
    List *newProvPos;

    DEBUG_LOG("add provenance attributes\n%s", nodeToString(newProvAttrs));

    CREATE_INT_SEQ(newProvPos, curAttrLen, curAttrLen + numProvAttrs - 1, 1);
    target->schema->attrDefs = concatTwoLists(target->schema->attrDefs, newProvAttrs);
    target->provAttrs = concatTwoLists(target->provAttrs, newProvPos);

    DEBUG_LOG("new prov attr list is \n%s\n\nprov attr pos %s", nodeToString(target->schema->attrDefs), nodeToString(target->provAttrs));
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
    FOREACH(QueryOperator,parent,new->parents)
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

/*
 *
 */

void
removeParentFromOps (List *operators, QueryOperator *parent)
{
    FOREACH(QueryOperator,op,operators)
        op->parents = REMOVE_FROM_LIST_PTR(op->parents, parent);
}
