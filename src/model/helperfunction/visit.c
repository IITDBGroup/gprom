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
//#include "model/set/set.h"
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
        /* set nodes */
//        case T_Set:
//        	{
//        		PREP_VISIT(Set);
//        		VISIT(elem);
//        	}
//        	break;
        /* expression nodes */
        case T_Constant:
        	{
        		PREP_VISIT(Constant);
        		VISIT(value);
        	}
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
                VISIT(selectClause);
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
        	{
        		PREP_VISIT(QueryBlock);
        		VISIT(selectClause);
        		VISIT(distinct);
        		VISIT(fromClause);
        		VISIT(whereClause);
        		VISIT(groupByClause);
        		VISIT(havingClause);
        		VISIT(orderByClause);
        		VISIT(limitClause);
        	}
        	break;
        case T_SelectItem:
        	{
        		PREP_VISIT(SelectItem);
        		//VISIT(alias);
        		VISIT(expr);
        	}
        	break;
//        case T_FromItem:
//        	{
//        		PREP_VISIT(FromItem);
//        		//VISIT(name);
//        		VISIT(attrNames);
//        	}
//        	break;
        case T_FromTableRef:
        	{
        		PREP_VISIT(FromTableRef);
        		//VISIT(from.attrNames);
        		//VISIT(tableId);
        	}
        	break;
        case T_FromSubquery:
        	{
        		PREP_VISIT(FromSubquery);
        		//VISIT(from.attrNames);
        		VISIT(subquery);
        	}
        	break;
        case T_FromJoinExpr:
        	{
        		PREP_VISIT(FromJoinExpr);
        		//VISIT(from.attrNames);
        		VISIT(left);
        		VISIT(right);
        		VISIT(cond);
        	}
        	break;
        case T_DistinctClause:
        	{
        		PREP_VISIT(DistinctClause);
        		VISIT(distinctExprs);
        	}
        	break;
        case T_NestedSubquery:
        	{
        		PREP_VISIT(NestedSubquery);
        		VISIT(expr);
        		VISIT(query);
        	}
        	break;
        case T_Insert:
        	{
        		PREP_VISIT(Insert);
        		VISIT(query);
        	}
        	break;
        case T_Delete:
        	{
        		PREP_VISIT(Delete);
        		VISIT(cond);
        	}
        	break;
        case T_Update:
        	{
        		PREP_VISIT(Update);
        		VISIT(selectClause);
        		VISIT(cond);
        	}
        	break;
        /* query operator model nodes */
        case T_Schema:
        	{
        		PREP_VISIT(Schema);
        		VISIT(attrDefs);
        	}
        	break;
        case T_AttributeDef:
        	{
        		PREP_VISIT(AttributeDef);
        		//VISIT();
        	}
        	break;
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
        	{
        		PREP_VISIT(ProvenanceComputation);
        		VISIT_OPERATOR_FIELDS();
        	}
        	break;
        case T_TableAccessOperator:
        	{
        		PREP_VISIT(TableAccessOperator);
        		VISIT_OPERATOR_FIELDS();
        	}
        	break;
        case T_SetOperator:
        	{
        		PREP_VISIT(SetOperator);
        		VISIT_OPERATOR_FIELDS();
        	}
        	break;
        case T_DuplicateRemoval:
        	{
        		PREP_VISIT(DuplicateRemoval);
        		VISIT_OPERATOR_FIELDS();
        		VISIT(attrs);
        	}
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
        	return node;
        case T_IntList:
        	return node;
        /* set nodes */
//        case T_Set:
//        	{
//        		NEWN(Set);
//        		MUTATE(SetElem, elem);
//        	}
//        	break;
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
                MUTATE(List, selectClause);
                MUTATE(Node,lChild);
                MUTATE(Node,rChild);
            }
            break;
        case T_ProvenanceStmt:
        	{
        		NEWN(ProvenanceStmt);
        		MUTATE(Node, query);
        	}
        	break;
        case T_QueryBlock:
        	{
        		NEWN(QueryBlock);
        		MUTATE(List, selectClause);
        		MUTATE(Node, distinct);
        		MUTATE(List, fromClause);
        		MUTATE(Node, whereClause);
        		MUTATE(List, groupByClause);
        		MUTATE(Node, havingClause);
        		MUTATE(List, orderByClause);
        		MUTATE(Node, limitClause);
        	}
        	break;
        case T_SelectItem:
        	{
        		NEWN(SelectItem);
        		MUTATE(Node, expr);
        	}
        	break;
        case T_FromItem:
        	{
        		NEWN(FromItem);
        		MUTATE(List, attrNames);
        	}
        	break;
        case T_FromTableRef:
        	{
        		NEWN(FromTableRef);
        		MUTATE(List, from.attrNames);
        	}
        	break;
        case T_FromSubquery:
        	{
        		NEWN(FromSubquery);
        		MUTATE(List, from.attrNames);
        		MUTATE(Node, subquery);
        	}
        	break;
        case T_FromJoinExpr:
        	{
        		NEWN(FromJoinExpr);
        		MUTATE(List, from.attrNames);
        		MUTATE(FromItem, left);
        		MUTATE(FromItem, right);
        		MUTATE(Node, cond);
        	}
        	break;
        case T_DistinctClause:
        	{
        		NEWN(DistinctClause);
        		MUTATE(List, distinctExprs);
        	}
        	break;
        case T_NestedSubquery:
        	{
        		NEWN(NestedSubquery);
        		MUTATE(Node, expr);
        		MUTATE(Node, query);
        	}
        	break;
        case T_Insert:
        	{
        		NEWN(Insert);
        		MUTATE(Node, query);
        	}
            break;
        case T_Delete:
        	{
        		NEWN(Delete);
        		MUTATE(Node, cond);
        	}
        	break;
        case T_Update:
        	{
        		NEWN(Update);
        		MUTATE(List, selectClause);
        		MUTATE(Node, cond);
        	}
        	break;

        /* query operator model nodes */
        case T_Schema:
        	{
        		NEWN(Schema);
        		MUTATE_OPERATOR();
        		MUTATE(List, attrDefs);
        	}
            break;
        case T_AttributeDef:
        	return node;
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
        	{
        		NEWN(JoinOperator);
        		MUTATE_OPERATOR();
        		MUTATE(Node, cond);
        	}
        	break;
        case T_AggregationOperator:
        	{
        		NEWN(AggregationOperator);
        		MUTATE_OPERATOR();
        		MUTATE(List, aggrs);
        		MUTATE(List, groupBy);
        	}
        	break;
        case T_ProvenanceComputation:
        	return node;
        case T_TableAccessOperator:
        	return node;
        case T_SetOperator:
        	return node;
        case T_DuplicateRemoval:
        	{
        		NEWN(DuplicateRemoval);
        		MUTATE_OPERATOR();
        		MUTATE(List, attrs);
        	}
            break;
        default:
            break;
    }

    return node;
}
