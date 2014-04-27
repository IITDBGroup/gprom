/*-----------------------------------------------------------------------------
 *
 * query_operator_model_checker.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "log/logger.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"

boolean
isTree(QueryOperator *op)
{
    if (LIST_LENGTH(op->parents) > 1)
        return FALSE;

    FOREACH(QueryOperator,o,op->inputs)
    {
        if(!isTree(o))
            return FALSE;
    }

    return TRUE;
}

boolean
checkModel (QueryOperator *op)
{
    if (!checkParentChildLinks(op))
        return FALSE;

    if (!checkAttributeRefConsistency(op))
        return FALSE;

    if (!checkSchemaConsistency(op))
        return FALSE;

    return TRUE;
}

boolean
checkAttributeRefConsistency (QueryOperator *op)
{
    return TRUE;
}

boolean
checkSchemaConsistency (QueryOperator *op)
{
    switch(op->type)
    {
        case T_ProjectionOperator:
        {
            ProjectionOperator *o = (ProjectionOperator *) op;

            if (LIST_LENGTH(o->projExprs) != LIST_LENGTH(op->schema->attrDefs))
            {
                ERROR_LOG("Number of attributes should be the same as number of"
                        " projection expressions:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        case T_DuplicateRemoval:
        {
            if (equal(OP_LCHILD(op)->schema->attrDefs, op->schema->attrDefs))
            {
                ERROR_LOG("Attributes of DuplicateRemoval should match attributes"
                        " of its child:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        case T_SelectionOperator:
        {
            SelectionOperator *o = (SelectionOperator *) op;
            QueryOperator *child = OP_LCHILD(o);

            if (!equal(op->schema->attrDefs,child->schema->attrDefs))
            {
                ERROR_LOG("Attributes of a selection operator should match the "
                        "attributes of its child:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        case T_JoinOperator:
        {
            JoinOperator *o = (JoinOperator *) op;
            QueryOperator *lChild = OP_LCHILD(o);
            QueryOperator *rChild = OP_RCHILD(o);
            List *expectedSchema = CONCAT_LISTS(
                    copyObject(lChild->schema->attrDefs),
                    copyObject(rChild->schema->attrDefs));

            if (!equal(o->op.schema->attrDefs, expectedSchema))
            {
                ERROR_LOG("Attributes of a join operator should be the "
                        "concatenation of attributes of its children:\n%s\n"
                        "expected:\n%s\nbut was\n%s",
                        operatorToOverviewString((Node *) op),
                        nodeToString(o->op.schema),
                        nodeToString(expectedSchema));
                return FALSE;
            }
        }
        break;
        case T_SetOperator:
        {
            SetOperator *o = (SetOperator *) op;
            QueryOperator *lChild = OP_LCHILD(o);

            if (!equal(o->op.schema->attrDefs, lChild->schema->attrDefs))
            {
                ERROR_LOG("Attributes of a set operator should be the "
                        "attributes of its left child:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *o = (WindowOperator *) op;
            QueryOperator *lChild = OP_LCHILD(op);
            List *expected = sublist(copyObject(op->schema->attrDefs), 0,
                    LIST_LENGTH(op->schema->attrDefs) - 1);

            if (!equal(expected, lChild->schema->attrDefs))
            {
                ERROR_LOG("Attributes of a window operator should be the "
                        "attributes of its left child + window function:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        default:
            break;
    }

    FOREACH(QueryOperator,o,op->inputs)
        if (!checkSchemaConsistency(o))
            return FALSE;

    return TRUE;
}

boolean
checkParentChildLinks (QueryOperator *op)
{
    // check that children have this node as their parent
    FOREACH(QueryOperator,o,op->inputs)
    {
        if (!searchList(o->parents, op))
        {
            ERROR_LOG("operator \n%s\n\n is child of \n\n%s\n, but does not have"
                    " this operator in its parent list",
                    operatorToOverviewString((Node *) o),
                    operatorToOverviewString((Node *) op));
            return FALSE;
        }
        if (!checkParentChildLinks(o))
            return FALSE;
    }

    // check that this node's parents have this node as a child
    FOREACH(QueryOperator,o,op->parents)
    {
        if (!searchList(o->inputs, op))
        {
            ERROR_LOG("operator \n%s\n\n is parent of \n\n%s\n, but does not have"
                    " this operator in its list of children",
                    operatorToOverviewString((Node *) o),
                    operatorToOverviewString((Node *) op));

            return FALSE;
        }
    }

    // check number of children based on operator type
    if (IS_NULLARY_OP(op) && LIST_LENGTH(op->inputs) != 0)
    {
        ERROR_LOG("nullary operator should have no inputs:\n%s",
                operatorToOverviewString((Node *) op));
        return FALSE;
    }
    if (IS_UNARY_OP(op) && LIST_LENGTH(op->inputs) != 1)
    {
        ERROR_LOG("nullary operator should have one input:\n%s",
                operatorToOverviewString((Node *) op));
        return FALSE;
    }
    if (IS_BINARY_OP(op) && LIST_LENGTH(op->inputs) != 2)
    {
        ERROR_LOG("nullary operator should have two inputs:\n%s",
                operatorToOverviewString((Node *) op));
        return FALSE;
    }



    return TRUE;
}


