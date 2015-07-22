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

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "configuration/option.h"

static boolean checkAttributeRefList (List *attrRefs, List *children, QueryOperator *parent);
static boolean checkUniqueAttrNames (QueryOperator *op);


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

#define SHOULD(opt) (getBoolOption(OPTION_AGGRESSIVE_MODEL_CHECKING) || getBoolOption(opt))
#define FREE_CONTEXT_AND_RETURN_BOOL(b) \
		do { \
		    FREE_AND_RELEASE_CUR_MEM_CONTEXT(); \
            return b; \
        } while (0)

boolean
checkModel (QueryOperator *op)
{
    NEW_AND_ACQUIRE_MEMCONTEXT("QO_MODEL_CHECKER");

    if (SHOULD(CHECK_OM_PARENT_CHILD_LINKS) && !checkParentChildLinks(op))
        FREE_CONTEXT_AND_RETURN_BOOL(FALSE);
    if (SHOULD(CHECK_OM_ATTR_REF) && !checkAttributeRefConsistency(op))
        FREE_CONTEXT_AND_RETURN_BOOL(FALSE);
    if (SHOULD(CHECK_OM_SCHEMA_CONSISTENCY) && !checkSchemaConsistency(op))
        FREE_CONTEXT_AND_RETURN_BOOL(FALSE);

    FREE_CONTEXT_AND_RETURN_BOOL(TRUE);
}

boolean
checkAttributeRefConsistency (QueryOperator *op)
{
    List *attrRefs = NIL;

    //TODO correlations in nested subqueries and missing operators
    switch(op->type)
    {
        case T_ProjectionOperator:
        {
            ProjectionOperator *o = (ProjectionOperator *) op;
            attrRefs = getAttrReferences((Node *) o->projExprs);
        }
        break;
        case T_SelectionOperator:
        {
            SelectionOperator *o = (SelectionOperator *) op;
            attrRefs = getAttrReferences(o->cond);
        }
        break;
        case T_JoinOperator:
        {
            JoinOperator *o = (JoinOperator *) op;
            attrRefs = getAttrReferences(o->cond);
        }
        break;
        case T_AggregationOperator:
        {
            AggregationOperator *o = (AggregationOperator *) op;
            attrRefs = CONCAT_LISTS(getAttrReferences((Node *) o->aggrs),
                    getAttrReferences((Node *) o->groupBy));
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *o = (WindowOperator *) op;
            attrRefs = CONCAT_LISTS(getAttrReferences((Node *) o->partitionBy),
                    getAttrReferences((Node *) o->orderBy),
                    getAttrReferences((Node *) o->frameDef),
                    getAttrReferences((Node *) o->f));
        }
        break;
        case T_OrderOperator:
        {
            OrderOperator *o = (OrderOperator *) op;
            attrRefs = getAttrReferences((Node *) o->orderExprs);
        }
        break;
        case T_DuplicateRemoval:
        {

        }
            break;
        // Check Attribute that we use as Json Column should be from/should exist in child
        case T_JsonTableOperator:
        {
	    JsonTableOperator *o = (JsonTableOperator *)op;
	    attrRefs = singleton(o->jsonColumn);
        }
        break;
        default:
            break;
    }
    if(!checkAttributeRefList(attrRefs, op->inputs, op))
        return FALSE;

    FOREACH(QueryOperator,child,op->inputs)
        if (!checkAttributeRefConsistency(child))
            return FALSE;

    return TRUE;
}

static boolean
checkAttributeRefList (List *attrRefs, List *children, QueryOperator *parent)
{
    FOREACH(AttributeReference,a,attrRefs)
    {
        int input = a->fromClauseItem;
        int attrPos = a->attrPosition;
        QueryOperator *child;
        AttributeDef *childA;

        if (a->name == NULL)
        {
            ERROR_LOG("attribute NULL name: %s\n\nin%s",
                    beatify(nodeToString(a)),
                    operatorToOverviewString((Node *) parent));
            return FALSE;
        }

        if (input < 0 || input >= LIST_LENGTH(children))
        {
            ERROR_LOG("attribute %s references input operator that does not "
                    "exist:\n\n%s",
                    beatify(nodeToString(a)),
                    operatorToOverviewString((Node *) parent));
            return FALSE;
        }

        child = (QueryOperator *) getNthOfListP(children, input);
        if (attrPos < 0 || attrPos >= getNumAttrs(child))
        {
            ERROR_LOG("attribute %s references attribute position that does not "
                                "exist in child:\n\n%s",
                                beatify(nodeToString(a)),
                                operatorToOverviewString((Node *) parent));
            return FALSE;
        }

        childA = getAttrDefByPos(child, attrPos);
        if (strcmp(childA->attrName, a->name) != 0)
        {
            ERROR_LOG("attribute ref name and child attrdef names are not the "
                    "same: <%s> and <%s> in\n\n%s", childA->attrName, a->name,
                    operatorToOverviewString((Node *) parent));
            DEBUG_LOG("details are: \n%s\n\n%s\n\n%s", nodeToString(a),
                    nodeToString(childA),
                    beatify(nodeToString(parent)));
            return FALSE;
        }
        if (childA->dataType != a->attrType)
        {
            ERROR_LOG("attribute datatype and child attrdef datatypes are not the "
                    "same: <%s> and <%s> in\n\n%s",
                    DataTypeToString(childA->dataType),
                    DataTypeToString(a->attrType),
                    operatorToOverviewString((Node *) parent));
            DEBUG_LOG("details are: \n%s\n\n%s\n\n%s", nodeToString(a),
                    nodeToString(childA),
                    beatify(nodeToString(parent)));
            return FALSE;
        }
       /* else
        {
            DEBUG_LOG("attribute datatype and child attrdef datatypes are not the "
                    "same: <%s> and <%s> in\n\n%s",
                    DataTypeToString(childA->dataType),
                    DataTypeToString(a->attrType),
                    operatorToOverviewString((Node *) parent));;
            DEBUG_LOG("details are: \n%s\n\n%s\n\n", nodeToString(a),
                    nodeToString(childA));
        }*/
    }

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

            FORBOTH(Node,p,a,o->projExprs,o->op.schema->attrDefs)
            {
                AttributeDef *def = (AttributeDef *) a;

                if (typeOf(p) != def->dataType)
                {
                    ERROR_LOG("schema and projection expression data types should"
                            " be the same: %s = %s",
                            DataTypeToString(typeOf(p)),
                            DataTypeToString(def->dataType));
                    DEBUG_LOG("details: %s", beatify(nodeToString(o)));
                    return FALSE;
                }
            }
        }
        break;
        case T_DuplicateRemoval:
        {
            if (!equal(OP_LCHILD(op)->schema->attrDefs, op->schema->attrDefs))
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

            if (LIST_LENGTH(op->schema->attrDefs) != LIST_LENGTH(child->schema->attrDefs))
            {
                ERROR_LOG("Number of attributes of a selection operator should match the "
                        "number of attributes of its child:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }

//            if (!equal(op->schema->attrDefs,child->schema->attrDefs))
//            {
//                ERROR_LOG("Attributes of a selection operator should match the "
//                        "attributes of its child:\n%s",
//                        operatorToOverviewString((Node *) op));
//                return FALSE;
//            }
        }
        break;
        case T_JoinOperator:
        {
            JoinOperator *o = (JoinOperator *) op;
            QueryOperator *lChild = OP_LCHILD(o);
            QueryOperator *rChild = OP_RCHILD(o);
            //TODO only check names
//            List *expectedSchema = CONCAT_LISTS(
//                    copyObject(lChild->schema->attrDefs),
//                    copyObject(rChild->schema->attrDefs));
//
            if (LIST_LENGTH(o->op.schema->attrDefs) !=
                    LIST_LENGTH(lChild->schema->attrDefs) +
                    LIST_LENGTH(rChild->schema->attrDefs))
            {
                ERROR_LOG("Number of attributes of a join operator should be the "
                        "addition of the number of attributes of its children:\n%s\n"
                        "expected:\n%u\nbut was\n%u",
                        operatorToOverviewString((Node *) op),
                        LIST_LENGTH(lChild->schema->attrDefs) +
                                LIST_LENGTH(rChild->schema->attrDefs),
                        LIST_LENGTH(o->op.schema->attrDefs));
                return FALSE;
            }
        }
        break;
        case T_SetOperator:
        {
            SetOperator *o = (SetOperator *) op;
            QueryOperator *lChild = OP_LCHILD(o);
            QueryOperator *rChild = OP_RCHILD(o);

            if (!equal(o->op.schema->attrDefs, lChild->schema->attrDefs))
            {
                ERROR_LOG("Attributes of a set operator should be the "
                        "attributes of its left child:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
            // left and right child should have the same number of attributes
            if (LIST_LENGTH(lChild->schema->attrDefs) != LIST_LENGTH(rChild->schema->attrDefs))
            {
                ERROR_LOG("Both children of a set operator should have the same"
                        " number of attributes:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }

        }
        break;
        case T_WindowOperator:
        {
//            WindowOperator *o = (WindowOperator *) op;
            QueryOperator *lChild = OP_LCHILD(op);
            List *expected = sublist(copyObject(op->schema->attrDefs), 0,
                    LIST_LENGTH(op->schema->attrDefs) - 2);

            if (!equal(expected, lChild->schema->attrDefs))
            {
                ERROR_LOG("Attributes of a window operator should be the "
                        "attributes of its left child + window function:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        case T_OrderOperator:
        {
//            OrderOperator *o = (OrderOperator *) op;
            QueryOperator *lChild = OP_LCHILD(op);
            List *expected = op->schema->attrDefs;

            if (!equal(expected, lChild->schema->attrDefs))
            {
                ERROR_LOG("Attributes of a order operator should be the "
                        "attributes of its left child:\n%s",
                        operatorToOverviewString((Node *) op));
                return FALSE;
            }
        }
        break;
        // We should Check that the schema of JsonTable has the attributes of the Child plus new JsonTable Attributes
        case T_JsonTableOperator:
        {
	    QueryOperator *lChild = OP_LCHILD(op);
	    FOREACH(Node, n, lChild->schema->attrDefs)
	    {
		if(!searchListNode(op->schema->attrDefs, n))
                {
                    ERROR_LOG("Attributes of a Json Table operator should be the "
                              "attributes of its left child plus new JsonTable Attributes:\n%s",
                              operatorToOverviewString((Node *) op));
                    return FALSE;
                }
            }
        }
        break;
        default:
            break;
    }

    FOREACH(QueryOperator,o,op->inputs)
        if (!checkSchemaConsistency(o))
            return FALSE;

    return !SHOULD(CHECK_OM_UNIQUE_ATTR_NAMES)
            || checkUniqueAttrNames(op);
}

static boolean
checkUniqueAttrNames (QueryOperator *op)
{
    Set *names = STRSET();

    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        if (hasSetElem(names,a->attrName))
        {
            ERROR_LOG("Attribute <%s> appears more than once in\n\n%s",
                    a->attrName, operatorToOverviewString((Node *) op));
            return FALSE;
        }
        addToSet(names,strdup(a->attrName));
    }

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


