/*-----------------------------------------------------------------------------
 *
 * visit.c
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
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

/*
 * Traverses a node tree and calls a user provided function for each node in
 * the tree. The user function takes a node and state parameter as input. The
 * state can, e.g., be used to store search results. The function should return
 * TRUE unless it wants to indicate that the travesal should be finished. A
 * user function should follow this pattern:
 *
 * boolean myVisitor (Node *n, void *state)
 * {
 *      if (n == NULL)
 *          return TRUE;
 *
 *      if (isA(node,NodeTypeICareAbout))
 *      {
 *          NodeTypeICareAbout *x = (NodeTypeICareAbout *) n;
 *      }
 *
 *      ...
 *
 *      return visit(n, myVisitor, state);
 * }
 */

#define VISIT(field) \
    if (!visit((Node *) n->field, checkNode, state)) \
        return FALSE;
#define VISIT_NODE(_node) \
    if (!visit((Node *) _node, checkNode, state)) \
        return FALSE;

#define PREP_VISIT(_type) \
        _type *n = (_type *) node;
#define VISIT_OPERATOR_FIELDS() \
        do { \
            VISIT(op.inputs); \
            VISIT(op.provAttrs); \
            VISIT(op.schema); \
        } while(0)

boolean
visit (Node *node, boolean (*checkNode) (), void *state)
{
    switch(node->type)
    {
        case T_List:
            {
                PREP_VISIT(List);
                FOREACH(Node,el,n)
                    VISIT_NODE(el);
            }
            break;
        case T_IntList:
            break;
        /* expression nodes */
        case T_Constant:
            break;
        case T_AttributeReference:
            break;
        case T_FunctionCall:
            {
                PREP_VISIT(FunctionCall);
                VISIT(args);
            }
            break;
        case T_Operator:
            {
                PREP_VISIT(Operator);
                VISIT(args);
            }
            break;
        /* query block model nodes */
/*        case T_SetOp:
            {
                PREP_VISIT(SetOp);
                VISIT(lChild);
                VISIT(rChild);
            }
            break;
        case T_SetQuery:
            {
                PREP_VISIT(SetQuery);
                VISIT(selectClause);
                VISIT(rootSetOp);
            }
            break;*/
        case T_SetQuery:
            {
                PREP_VISIT(SetQuery);
                VISIT(setOp);
                VISIT(lChild);
                VISIT(rChild);
            }
            break;
        case T_ProvenanceStmt:
            {
                PREP_VISIT(ProvenanceStmt);
                VISIT(query);
            }
            break;
        case T_QueryBlock:
        case T_SelectItem:
        case T_FromItem:
        case T_FromTableRef:
        case T_FromSubquery:
        case T_FromJoinExpr:
        case T_DistinctClause:

        /* query operator model nodes */
        case T_Schema:
        case T_AttributeDef:
        case T_SelectionOperator:
            {
                PREP_VISIT(SelectionOperator);
                VISIT_OPERATOR_FIELDS();
                VISIT(cond);
            }
            break;
        case T_ProjectionOperator:
            {
                PREP_VISIT(ProjectionOperator);
                VISIT_OPERATOR_FIELDS();
                VISIT(projExprs);
            }
            break;
        case T_JoinOperator:
            {
                PREP_VISIT(JoinOperator);
                VISIT_OPERATOR_FIELDS();
                VISIT(cond);
            }
            break;
        case T_AggregationOperator:
            {
                PREP_VISIT(AggregationOperator);
                VISIT_OPERATOR_FIELDS();
                VISIT(aggrs);
                VISIT(groupBy);
            }
            break;
        case T_ProvenanceComputation:
        case T_TableAccessOperator:
        case T_SetOperator:
        case T_DuplicateRemoval:
            break;
        default:
            break;
    }

    return TRUE;
}

/*
 * Function to traverse and modify a node tree. The user provides a function
 * modifyNode that takes a node and returns the a potentially modified version
 * of this node.
 */
#define NEWN(_type) \
    _type *newN = (_type *) node
#define MUTATE(_type,_n) \
    newN->_n =  (_type *) modifyNode((Node *) newN->_n, state)
#define MUTATE_OPERATOR() \
    do { \
    	QueryOperator *newN = (QueryOperator *) node; \
    	MUTATE(List,inputs); \
    	MUTATE(Schema,schema); \
    	MUTATE(List,provAttrs); \
    } while (0)

Node *
mutate (Node *node, Node *(*modifyNode) (), void *state)
{
    /* if the user has not modifed the node, then traverse further */
    switch(node->type)
    {
        case T_List:

        case T_IntList:

        /* expression nodes */
        case T_Constant:
            return node;
        case T_AttributeReference:
            return node;
        case T_FunctionCall:
            {
                NEWN(FunctionCall);
                MUTATE(List,args);
            }
            break;
        case T_Operator:
            {
                NEWN(Operator);
                MUTATE(List,args);
            }
            break;
        /* query block model nodes */
/*        case T_SetOp:
            {
                NEWN(SetOp);
                MUTATE(Node,lChild);
                MUTATE(Node,rChild);
            }
            break;
        case T_SetQuery:
            {
                NEWN(SetQuery);
                MUTATE(List,selectClause);
                MUTATE(SetOp,rootSetOp);
            }
            break;*/
        case T_SetQuery:
            {
                NEWN(SetQuery);
                MUTATE(char, setOp);
                MUTATE(Node,lChild);
                MUTATE(Node,rChild);
            }
            break;
        case T_ProvenanceStmt:
        case T_QueryBlock:
        case T_SelectItem:
        case T_FromItem:
        case T_FromTableRef:
        case T_FromSubquery:
        case T_FromJoinExpr:
        case T_DistinctClause:

        /* query operator model nodes */
        case T_Schema:
            break;
        case T_AttributeDef:
            break;
        case T_SelectionOperator:
            {
                NEWN(SelectionOperator);
                MUTATE_OPERATOR();
                MUTATE(Node,cond);
            }
            break;
        case T_ProjectionOperator:
            {
                NEWN(ProjectionOperator);
                MUTATE_OPERATOR();
                MUTATE(List,projExprs);
            }
            break;
        case T_JoinOperator:
        case T_AggregationOperator:
        case T_ProvenanceComputation:
        case T_TableAccessOperator:
        case T_SetOperator:
        case T_DuplicateRemoval:
            break;
        default:
            break;
    }

    return node;
}
