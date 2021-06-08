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

#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "mem_manager/mem_mgr.h"
#include "provenance_rewriter/prov_utility.h"
#include "log/logger.h"

static boolean hasProvVisitor(Node *q, boolean *found);


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

void
addProvenanceAttrsToSchemabasedOnList(QueryOperator *target, List *provList)
{
    List *newProvAttrs = (List *) copyObject(provList);
    int curAttrLen = LIST_LENGTH(target->schema->attrDefs);
    int numProvAttrs = LIST_LENGTH(newProvAttrs);
    List *newProvPos;

    DEBUG_LOG("add provenance attributes\n%s", nodeToString(newProvAttrs));

    CREATE_INT_SEQ(newProvPos, curAttrLen, curAttrLen + numProvAttrs - 1, 1);
    target->schema->attrDefs = concatTwoLists(target->schema->attrDefs, newProvAttrs);
    target->provAttrs = concatTwoLists(target->provAttrs, newProvPos);

    DEBUG_LOG("new prov attr list is \n%s\n\nprov attr pos %s", nodeToString(target->schema->attrDefs), nodeToString(target->provAttrs));
}

List *
getProvAttrProjectionExprs(QueryOperator *op)
{
    List *result = NIL;
    List *pDefs = getProvenanceAttrDefs(op);
    int pos = 0;

    FOREACH_INT(i,op->provAttrs)
    {
        AttributeReference *a;
        AttributeDef *d = getNthOfListP(pDefs, pos++);
        char *name = strdup(d->attrName);

        a = createFullAttrReference(name, 0, i, INVALID_ATTR, d->dataType);
        result = appendToTailOfList(result, a);
    }

    return result;
}

List *
getNormalAttrProjectionExprs(QueryOperator *op)
{
    List *result = NIL;
//    List *names = getQueryOperatorAttrNames(op);
    List *attrDefs = op->schema->attrDefs;

    for(int i = 0; i < getNumAttrs(op); i++)
    {
        if (!searchListInt(op->provAttrs, i))
        {
            AttributeReference *a;
            AttributeDef *d = getNthOfListP(attrDefs, i);
            char *name = strdup(d->attrName);
            a = createFullAttrReference(name, 0, i, INVALID_ATTR, d->dataType);
            result = appendToTailOfList(result, a);
        }
    }

    return result;
}

List *
getAllAttrProjectionExprs(QueryOperator *op)
{
	List *result = NIL;
	int i = 0;
	
	FOREACH(AttributeDef,a,op->schema->attrDefs)
	{
		AttributeReference *at;

		at = createFullAttrReference(a->attrName, 0, i++, INVALID_ATTR, a->dataType);
		result = appendToTailOfList(result, at);
	}

	return result;
}

QueryOperator *
createProjOnAllAttrs(QueryOperator *op)
{
    ProjectionOperator *p;
    List *projExprs = NIL;
    List *attrNames = NIL;
    int i = 0;

    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        AttributeReference *att;

        att = createFullAttrReference(a->attrName, 0, i++, INVALID_ATTR, a->dataType);
        projExprs = appendToTailOfList(projExprs, att);
        attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
    }

    p = createProjectionOp (projExprs, NULL, NIL, attrNames);
    p->op.provAttrs = copyObject(op->provAttrs);

    return (QueryOperator *) p;
}

QueryOperator *
createProjOnAttrs(QueryOperator *op, List *attrPos)
{
    ProjectionOperator *p;
    List *projExprs = NIL;
    List *attrNames = NIL;

    FOREACH_INT(i,attrPos)
    {
        AttributeDef *a = getAttrDefByPos(op, i);
        AttributeReference *att;

        att = createFullAttrReference(strdup(a->attrName), 0, i, INVALID_ATTR, a->dataType);
        projExprs = appendToTailOfList(projExprs, att);
        attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
    }

    DEBUG_LOG("projection expressions: %s", nodeToString((Node*) projExprs));

    p = createProjectionOp (projExprs, NULL, NIL, attrNames);
    p->op.provAttrs = copyObject(op->provAttrs); //TODO create real prov attrs list

    return (QueryOperator *) p;
}

QueryOperator *
createProjOnAttrsByName(QueryOperator *op, List *attrNames, List *newAttrNames)
{
    List *attrPos = NIL;
    QueryOperator *p;

    FOREACH(char,c,attrNames)
        attrPos = appendToTailOfListInt(attrPos, getAttrPos(op,c));
    DEBUG_LOG("child attr pos: %s", nodeToString(attrPos));

    p = createProjOnAttrs(op, attrPos);

    if (newAttrNames != NULL)
    {
        FORBOTH(void,def,name,p->schema->attrDefs, newAttrNames)
        {
            AttributeDef *a = (AttributeDef *) def;
            char *n = (char *) name;
            a->attrName = n;
        }
    }

    return p;
}

AttributeReference *
createAttrsRefByName(QueryOperator *op, char *attrNames)
{
	AttributeDef *ad = copyObject(getAttrDefByName(op, attrNames));
	int pos = getAttrPos(op, ad->attrName);
	AttributeReference *ar = createFullAttrReference(strdup(ad->attrName), 0, pos, INVALID_ATTR, ad->dataType);

	return ar;
}


//AttributeReference *
//createAttrsRefByName(QueryOperator *op, char *attrNames)
//{
//	AttributeDef *ad = copyObject(getAttrDefByName(op, attrNames));
//
//	ASSERT(ad != NULL);
//
//	int defPos = getAttrPos(op, ad->attrName);
//	AttributeReference *ar = createFullAttrReference(strdup(ad->attrName), 0, defPos, INVALID_ATTR, ad->dataType);
//
//	return ar;
//}
//
//AttributeReference *
//createAttrRefByPos(QueryOperator *op, int pos)
//{
//    AttributeDef *ad = copyObject(getAttrDefByPos(op, pos));
//    ASSERT(ad != NULL);
//
//    AttributeReference *ar = createFullAttrReference(strdup(ad->attrName), 0, pos, INVALID_ATTR, ad->dataType);
//
//    return ar;
//}



/*
 * Given a subtree rooted a "orig" replace this subtree with the tree rooted
 * at "new". This method adapts all input lists of all parents of "orig" to point
 * to "new" instead. WARNING: "new" is assumed to be unrooted (no parents and not a
 * child of any other node)
 */
void
switchSubtrees(QueryOperator *orig, QueryOperator *new)
{
    // copy list of parents to new subtree
    new->parents = orig->parents;
    orig->parents = NIL;
    boolean newParentOfOrig = FALSE;

    // adapt original parent's inputs
    FOREACH(QueryOperator,parent,new->parents)
    {
        // handle case when orig was a child of new
        if (parent == new)
        {
            orig->parents = singleton(new);
            newParentOfOrig  = TRUE;
        }
        else
        {
            FOREACH(QueryOperator,pChild,parent->inputs)
            {//TODO check whether this change is correct: we want to check for pointer equality here
                if (pChild == orig)
                    pChild_his_cell->data.ptr_value = new;
            }
        }
    }

    if (newParentOfOrig)
        removeParentFromOps(singleton(new), new);
}

/*
 * Same as switchSubtrees, but keep parents of "new" intact. This is used
 * to replace "orig" with "new" if "new" is part of an existing tree
 */
void
switchSubtreeWithExisting (QueryOperator *orig, QueryOperator *new)
{
	new->parents = CONCAT_LISTS(new->parents, orig->parents);
	orig->parents = NIL;
    boolean newParentOfOrig = FALSE;
    boolean origParentOfNew = FALSE;

	// adapt inputs of "orig"'s parents
    // adapt original parent's inputs
    FOREACH(QueryOperator,parent,new->parents)
    {
        // handle case when orig was a child of new
        if (parent == new)
        {
            orig->parents = singleton(new);
            newParentOfOrig  = TRUE;
        }
        // handle case when new was a child of orig
        else if (parent == orig)
        {
            origParentOfNew = TRUE;
        }
        else
        {
            FOREACH(QueryOperator,pChild,parent->inputs)
            {
                if (pChild == orig)
                    pChild_his_cell->data.ptr_value = new;
            }
        }
    }

    if (newParentOfOrig)
        removeParentFromOps(singleton(new), new);
    if (origParentOfNew)
        removeParentFromOps(singleton(new), orig);
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

List *
findOperatorAttrRefs (QueryOperator *op)
{
    List *result = NIL;

    switch(op->type)
    {
        case T_ProjectionOperator:
            result = getAttrReferences((Node *)((ProjectionOperator *) op)->projExprs);
            break;
        case T_SelectionOperator:
            result = getAttrReferences((Node *)((SelectionOperator *) op)->cond);
            break;
        case T_JoinOperator:
            result = getAttrReferences((Node *)((JoinOperator *) op)->cond);
            break;
        case T_AggregationOperator:
        {
            AggregationOperator *a = (AggregationOperator *) op;
            result = CONCAT_LISTS(getAttrReferences((Node *)a->aggrs), getAttrReferences((Node *)a->groupBy));
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *w = (WindowOperator *) op;
            result = CONCAT_LISTS(getAttrReferences((Node *)w->f), getAttrReferences((Node *)w->partitionBy),
                    getAttrReferences((Node *)w->orderBy), getAttrReferences((Node *)w->frameDef));
        }
        break;
        case T_OrderOperator:
        {
            OrderOperator *o = (OrderOperator *) op;
            result = getAttrReferences((Node *)o->orderExprs);
        }
        break;
        case T_NestingOperator:
        {
            NestingOperator *n = (NestingOperator *) op;
            result = getAttrReferences((Node *)n->cond);
        }
        break;
        case T_DuplicateRemoval:
        {
            DuplicateRemoval *d = (DuplicateRemoval *) op;
            result = getAttrReferences((Node *)d->attrs);
        }
        break;
        //TODO
        default:
            break;
    }

    return result;
}

void
makeNamesUnique (List *names, Set *allNames)
{
    HashMap *nCount = NEW_MAP(Constant,Constant);
    if (allNames == NULL)
        allNames = STRSET();

    FOREACH_LC(lc,names)
    {
        char *oldName = LC_P_VAL(lc);
        char *newName = oldName;
        int count = MAP_INCR_STRING_KEY(nCount,oldName);

        if (count != 0)
        {
            // create a new unique name
            newName = CONCAT_STRINGS(oldName, gprom_itoa(count));
            while(hasSetElem(allNames, newName))
                newName = CONCAT_STRINGS(oldName, gprom_itoa(++count));

            while(searchListString(names, newName))
            	newName = CONCAT_STRINGS(oldName, gprom_itoa(++count));
        }

        LC_P_VAL(lc) = newName;
    }
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

void
substOpInParents (List *parents, QueryOperator *orig, QueryOperator *newOp)
{
    FOREACH(QueryOperator,p,parents)
    {
        FOREACH(QueryOperator,pChild,p->inputs)
        {
            if (equal(pChild,orig))
                pChild_his_cell->data.ptr_value = newOp;
        }
    }
}

boolean
hasProvComputation(Node *op)
{
	boolean found = FALSE;

	hasProvVisitor(op, &found);
	return found;
}

static boolean
hasProvVisitor(Node *q, boolean *found)
{
	if(q == NULL)
		return TRUE;

	if(isA(q, ProvenanceComputation))
	{
		*found = TRUE;
		return FALSE;
	}

	return visit(q, hasProvVisitor, found);
}
