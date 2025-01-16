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
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/graph/graph.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/integrity_constraints/integrity_constraints.h"
#include "model/datalog/datalog_model.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"

/*
 * Traverses a node tree and calls a user provided function for each node in
 * the tree. The user function takes a node and state parameter as input. The
 * state can, e.g., be used to store search results. The function should return
 * TRUE unless it wants to indicate that the traversal should be finished. A
 * user function should follow this pattern:
 *
 * boolean myVisitor (Node *n, void *state)
 * {
 *      if (n == NULL)
 *          return TRUE;
 *
 *      if (isA(n,NodeTypeICareAbout))
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
    if (!checkNode((Node *) n->field, state)) \
        return FALSE;
#define VISIT_NODE(_node) \
    if (!checkNode((Node *) _node, state)) \
        return FALSE;

#define PREP_VISIT(_type) \
        _type *n = (_type *) node; \
        TRACE_LOG("visit node <%s>", nodeToString(node));
#define LOG_VISIT_ONLY(_type) \
		TRACE_LOG("visit node <%s>", nodeToString(node));

#define VISIT_OPERATOR_FIELDS() \
        do { \
            VISIT(op.inputs); \
            VISIT(op.provAttrs); \
            VISIT(op.schema); \
        } while(0)

#define VISIT_FROM_ITEM() \
        VISIT_NODE(((FromItem *) n)->provInfo)

boolean
visit (Node *node, boolean (*checkNode) (Node *n, void *state), void *state)
{
    switch(node->type)
    {
        case T_List:
        {
            if (node == NULL)
            {
            	break;
            }
            PREP_VISIT(List);
            FOREACH(Node,el,n)
            VISIT_NODE(el);
        }
        break;
        case T_IntList:
        break;
    	case T_KeyValue:
	    {
			PREP_VISIT(KeyValue);
			VISIT(key);
			VISIT(value);
        	break;
		}
      	case T_Set:
		{
			Set *s = (Set *) node;
			FOREACH_SET(Node,el,s)
			{
				VISIT_NODE(el);
			}
		}
		break;
    	case T_HashMap:
    	{
    		HashMap *h = (HashMap *) node;
    		FOREACH_HASH_ENTRY(kv,h)
    		{
    			VISIT_NODE(kv);
    		}
    	}
    	case T_Graph:
    	{
    		Graph *g = (Graph *) node;
    		VISIT_NODE(g->nodes);
    		VISIT_NODE(g->edges);
    	}

        /* expression nodes */
        case T_Constant:
        {
        	LOG_VISIT_ONLY(Constant);
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
        case T_CaseExpr:
        {
            PREP_VISIT(CaseExpr);
            VISIT(expr);
            VISIT(whenClauses);
            VISIT(elseRes);
        }
        break;
        case T_CaseWhen:
        {
            PREP_VISIT(CaseWhen);
            VISIT(when);
            VISIT(then);
        }
        break;
        case T_IsNullExpr:
        {
            PREP_VISIT(IsNullExpr);
            VISIT(expr);
        }
        break;
        case T_CastExpr:
        {
            PREP_VISIT(CastExpr);
            VISIT(expr);
        }
        break;
        case T_WindowBound:
        {
            PREP_VISIT(WindowBound);
            VISIT(expr);
        }
        break;
        case T_WindowFrame:
        {
            PREP_VISIT(WindowFrame);
            VISIT(lower);
            VISIT(higher);
        }
        break;
        case T_WindowDef:
        {
            PREP_VISIT(WindowDef);
            VISIT(partitionBy);
            VISIT(orderBy);
            VISIT(frame);
        }
        break;
        case T_WindowFunction:
        {
            PREP_VISIT(WindowFunction);
            VISIT(f);
            VISIT(win);
        }
        break;
        case T_RowNumExpr:
        break;
        case T_OrderExpr:
        {
            PREP_VISIT(OrderExpr);
            VISIT(expr);
        }
        break;
        case T_QuantifiedComparison:
        {
            PREP_VISIT(QuantifiedComparison);
            VISIT(checkExpr);
            VISIT(exprList);
        }
        break;
        /* integrity constraint nodes */
    	case T_FD:
    	break;
    	case T_FOdep:
    	{
    		PREP_VISIT(FOdep);
    		VISIT(lhs);
    		VISIT(rhs);
		}
		break;
        /* query block model nodes */
        case T_SetQuery:
        {
            PREP_VISIT(SetQuery);
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
			VISIT(offsetClause);
        }
        break;
        case T_SelectItem:
        {
        	PREP_VISIT(SelectItem);
        	VISIT(expr);
        }
        break;
        case T_FromTableRef:
        {
            PREP_VISIT(FromTableRef);
            VISIT_FROM_ITEM();
        	LOG_VISIT_ONLY(FromTableRef);
        }
        break;
        case T_FromSubquery:
        {
        	PREP_VISIT(FromSubquery);
            VISIT_FROM_ITEM();
        	VISIT(subquery);
        }
        break;
        case T_FromLateralSubquery:
        {
        	PREP_VISIT(FromLateralSubquery);
            VISIT_FROM_ITEM();
        	VISIT(subquery);
        }
        break;
	    case T_FromJoinExpr:
        {
        	PREP_VISIT(FromJoinExpr);
            VISIT_FROM_ITEM();
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
	    case T_PreparedQuery:
		{
			PREP_VISIT(PreparedQuery);
			VISIT(q);
			VISIT(dts);
		}
		break;
	    case T_ExecQuery:
		{
			PREP_VISIT(ExecQuery);
			VISIT(params);
		}
		break;
	    case T_WithStmt:
		{
			PREP_VISIT(WithStmt);
			VISIT(withViews);
			VISIT(query);
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
        	LOG_VISIT_ONLY(AttributeDef);
        	//VISIT();
        }
        break;
	    case T_ParameterizedQuery:
		{
			PREP_VISIT(ParameterizedQuery);
			VISIT(q);
			VISIT(parameters);
		}
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
        case T_SampleClauseOperator:
        {
        	PREP_VISIT(SampleClauseOperator);
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
        case T_NestingOperator:
        {
            PREP_VISIT(NestingOperator);
            VISIT_OPERATOR_FIELDS();
            VISIT(cond);
        }
        break;
        case T_WindowOperator:
        {
            PREP_VISIT(WindowOperator);
            VISIT_OPERATOR_FIELDS();
            VISIT(partitionBy);
            VISIT(orderBy);
            VISIT(frameDef);
            VISIT(f);
        }
        break;
        case T_OrderOperator:
        {
            PREP_VISIT(OrderOperator);
            VISIT_OPERATOR_FIELDS();
            VISIT(orderExprs);
        }
        break;
	    case T_LimitOperator:
        {
            PREP_VISIT(LimitOperator);
            VISIT_OPERATOR_FIELDS();
            VISIT(limitExpr);
            VISIT(offsetExpr);
        }
        break;
     	case T_ExecPreparedOperator:
		{
			PREP_VISIT(ExecPreparedOperator);
			VISIT_OPERATOR_FIELDS();
			VISIT(params);
		}
		break;
        // DLNodes
        case T_DLAtom:
        {
            PREP_VISIT(DLAtom);
            VISIT(args);
        }
        break;
        case T_DLComparison:
        {
            PREP_VISIT(DLComparison);
            VISIT(opExpr);
        }
        break;
        case T_DLRule:
        {
            PREP_VISIT(DLRule);
            VISIT(head);
            VISIT(body);
        }
        break;
        case T_DLProgram:
        {
            PREP_VISIT(DLProgram);
            VISIT(rules);
            VISIT(facts);
			VISIT(func);
        }
        break;

        /* provenance sketch */
        case T_psInfo:
        {
            PREP_VISIT(psInfo);
            VISIT(psType);
            VISIT(tablePSAttrInfos);
        }
        break;
        case T_psAttrInfo:
        {
            PREP_VISIT(psAttrInfo);
            VISIT(attrName);
            VISIT(rangeList);
            VISIT(BitVector);
            VISIT(psIndexList);
        }
        break;
        case T_psInfoCell:
        {
            //            PREP_VISIT(psInfoCell);
            //            VISIT(storeTable);
            //            VISIT(pqSql);
            //            VISIT(paraValues);
            //            VISIT(tableName);
            //            VISIT(attrName);
            //            VISIT(provTableAttr);
            //            VISIT(numRanges);
            //            VISIT(psSize);
            //            VISIT(ps);
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

#define MUTATE_DLNODE() \
    do { \
    	DLNode *newN = (DLNode *) node; \
    	MUTATE(HashMap,properties); \
    } while (0)

Node *
mutate (Node *node, Node *(*modifyNode) (Node *n, void *state), void *state)
{
    if (node == NULL)
        return NULL;

    /* if the user has not modifed the node, then traverse further */
    switch(node->type)
    {
        case T_List:
        {
        	NEWN(List);
        	FOREACH(Node, el, newN)
        	el_his_cell->data.ptr_value = (void *) modifyNode((Node *) el, state);
        }
        break;
        case T_IntList:
        	return node;
	    case T_Set:
		{
			Set *s = (Set *) node;
			if (s->setType == SET_TYPE_NODE)
			{
				Set *out = NODESET();

				FOREACH_SET(Node,n,s)
				{
					addToSet(out,
							 modifyNode(n, state));
				}

				return (Node *) out;
			}

			return node;
		}

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
        case T_CaseExpr:
        {
            NEWN(CaseExpr);
            MUTATE(Node, expr);
            MUTATE(List, whenClauses);
            MUTATE(Node, elseRes);
        }
        break;
        case T_CaseWhen:
        {
            NEWN(CaseWhen);
            MUTATE(Node, when);
            MUTATE(Node, then);
        }
        break;
        case T_CastExpr:
        {
            NEWN(CastExpr);
            MUTATE(Node, expr);
        }
        break;
        case T_IsNullExpr:
        {
            NEWN(IsNullExpr);
            MUTATE(Node, expr);
        }
        break;
        case T_WindowBound:
        {
            NEWN(WindowBound);
            MUTATE(Node, expr);
        }
        break;
        case T_WindowFrame:
        {
            NEWN(WindowFrame);
            MUTATE(WindowBound, lower);
            MUTATE(WindowBound, higher);
        }
        break;
        case T_WindowDef:
        {
            NEWN(WindowDef);
            MUTATE(List, partitionBy);
            MUTATE(List, orderBy);
            MUTATE(WindowFrame, frame);
        }
        break;
        case T_WindowFunction:
        {
            NEWN(WindowFunction);
            MUTATE(FunctionCall, f);
            MUTATE(WindowDef, win);
        }
        break;
        case T_RowNumExpr:
            return node;
        case T_OrderExpr:
        {
            NEWN(OrderExpr);
            MUTATE(Node,expr);
        }
        break;
        case T_QuantifiedComparison:
        {
            NEWN(QuantifiedComparison);
            MUTATE(Node,checkExpr);
            MUTATE(List,exprList);
        }
        break;
		/* integrity constraint nodes */
    	case T_FD:
			return node;
    	case T_FOdep:
    	{
    		NEWN(FOdep);
    		MUTATE(List,lhs);
    		MUTATE(List,rhs);
    	}
		break;
        /* query block model nodes */
        case T_SetQuery:
        {
            NEWN(SetQuery);
            MUTATE(List,selectClause);
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
			MUTATE(Node, offsetClause);
        }
        break;
	    case T_ExecQuery:
		{
			NEWN(ExecQuery);
			MUTATE(List, params);
		}
	    case T_PreparedQuery:
		{
			NEWN(PreparedQuery);
			MUTATE(Node, q);
			MUTATE(List, dts);
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
        	//NEWN(FromItem);
        	//MUTATE(List, attrNames);
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
        case T_FromLateralSubquery:
        {
        	NEWN(FromLateralSubquery);
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
        	//MUTATE_OPERATOR();
        	MUTATE(List, attrDefs);
        }
        break;
        case T_AttributeDef:
        	return node;
	    case T_ParameterizedQuery:
		{
			NEWN(ParameterizedQuery);
			MUTATE(Node, q);
			MUTATE(List, parameters);
		}
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
        {
        	NEWN(ProvenanceComputation);
    		MUTATE_OPERATOR();
    		MUTATE(ProvenanceTransactionInfo, transactionInfo);
        }
        break;
        case T_TableAccessOperator:
    	{
            //    		    NEWN(TableAccessOperator);
    		MUTATE_OPERATOR();
    	}
        break;
        case T_SampleClauseOperator:
		{
			MUTATE_OPERATOR();
		}
		break;
        case T_SetOperator:
    	{
            //    		    NEWN(SetOperator);
    		MUTATE_OPERATOR();
    	}
        break;
        case T_DuplicateRemoval:
        {
        	NEWN(DuplicateRemoval);
        	MUTATE_OPERATOR();
        	MUTATE(List, attrs);
        }
        break;
        case T_NestingOperator:
        {
            NEWN(NestingOperator);
            MUTATE_OPERATOR();
            MUTATE(Node, cond);
        }
        break;
        case T_WindowOperator:
        {
            NEWN(WindowOperator);
            MUTATE_OPERATOR();
            MUTATE(List,partitionBy);
            MUTATE(List,orderBy);
            MUTATE(WindowFrame,frameDef);
            MUTATE(Node,f);
        }
        break;
        case T_OrderOperator:
        {
            NEWN(OrderOperator);
            MUTATE(List,orderExprs);
        }
        break;
        case T_LimitOperator:
        {
            NEWN(LimitOperator);
            MUTATE(Node,limitExpr);
            MUTATE(Node,offsetExpr);
        }
        break;
	    case T_ExecPreparedOperator:
		{
			NEWN(ExecPreparedOperator);
			MUTATE_OPERATOR();
			MUTATE(List,params);
		}
		// DL nodes
        case T_DLAtom:
        {
            NEWN(DLAtom);
            MUTATE_DLNODE();
            MUTATE(List,args);
        }
        break;
        case T_DLComparison:
        {
            NEWN(DLComparison);
            MUTATE_DLNODE();
            MUTATE(Operator,opExpr);
        }
        break;
        case T_DLRule:
        {
            NEWN(DLRule);
            MUTATE_DLNODE();
            MUTATE(DLAtom,head);
            MUTATE(List,body);
        }
        break;
        case T_DLProgram:
        {
            NEWN(DLProgram);
            MUTATE_DLNODE();
            MUTATE(List,rules);
            MUTATE(List,facts);
        }
        break;
        /* provenance sketch */
        case T_psInfo:
        {
            NEWN(psInfo);

            MUTATE(HashMap,tablePSAttrInfos);
        }
        break;
        case T_psAttrInfo:
        {
            NEWN(psAttrInfo);

            MUTATE(List,rangeList);
            MUTATE(BitSet,BitVector);
            MUTATE(List,psIndexList);
        }
        break;
        case T_psInfoCell:
        {
            NEWN(psInfoCell);

            MUTATE(BitSet,ps);
        }
        break;
        default:
        break;
    }

    return node;
}

/**
 * Variant of visit function that propagates the pointer the parent node is used to point to the current node.
 * This can be used to modify parts of a tree without the need to copy the whole tree (as mutate does).
 */

#define VISIT_P(field) \
    if (!userVisitor((Node *) n->field, state, &(n->field))) \
        return FALSE;
#define VISIT_LC_P(_el) \
    if (!userVisitor((Node *) _el->data.ptr_value, state, &(_el->data.ptr_value))) \
        return FALSE;

#define PREP_VISIT_P(_type) \
        _type *n = (_type *) node; \
        TRACE_LOG("visit node <%s>", nodeToString(node));

#define VISIT_OPERATOR_FIELDS_P() \
        do { \
            VISIT_P(op.inputs); \
            VISIT_P(op.provAttrs); \
            VISIT_P(op.schema); \
        } while(0)

boolean
visitWithPointers (Node *node, boolean (*userVisitor) (Node *n, void *state, void *ptr), void **parentLink, void *state)
{
    switch(node->type)
    {
        case T_List:
            {
                if (node == NULL)
                    break;
                PREP_VISIT_P(List);
                FOREACH_LC(el,n)
                    VISIT_LC_P(el);
            }
            break;
        case T_IntList:
            break;
			// sets and hashmaps do not work
    	case T_Graph:
    	{
    		PREP_VISIT_P(Graph);
    		VISIT_P(nodes);
    		VISIT_P(edges);
    	}
		break;

        /* expression nodes */
        case T_Constant:
            {
                LOG_VISIT_ONLY(Constant);
            }
            break;
        case T_AttributeReference:
            break;
        case T_FunctionCall:
            {
                PREP_VISIT_P(FunctionCall);
                VISIT_P(args);
            }
            break;
        case T_Operator:
            {
                PREP_VISIT_P(Operator);
                VISIT_P(args);
            }
            break;
        case T_CaseExpr:
            {
                PREP_VISIT_P(CaseExpr);
                VISIT_P(expr);
                VISIT_P(whenClauses);
                VISIT_P(elseRes);
            }
            break;
        case T_CaseWhen:
            {
                PREP_VISIT_P(CaseWhen);
                VISIT_P(when);
                VISIT_P(then);
            }
            break;
        case T_CastExpr:
            {
                PREP_VISIT_P(CastExpr);
                VISIT_P(expr);
            }
            break;
        case T_IsNullExpr:
            {
                PREP_VISIT_P(IsNullExpr);
                VISIT_P(expr);
            }
            break;
        case T_WindowBound:
            {
                PREP_VISIT_P(WindowBound);
                VISIT_P(expr);
            }
        break;
        case T_WindowFrame:
            {
                PREP_VISIT_P(WindowFrame);
                VISIT_P(lower);
                VISIT_P(higher);
            }
        break;
        case T_WindowDef:
            {
                PREP_VISIT_P(WindowDef);
                VISIT_P(partitionBy);
                VISIT_P(orderBy);
                VISIT_P(frame);
            }
        break;
        case T_WindowFunction:
            {
                PREP_VISIT_P(WindowFunction);
                VISIT_P(f);
                VISIT_P(win);
            }
        break;
        case T_RowNumExpr:
            break;
        case T_OrderExpr:
            {
                PREP_VISIT_P(OrderExpr);
                VISIT_P(expr);
            }
        break;
        case T_QuantifiedComparison:
            {
                PREP_VISIT_P(QuantifiedComparison);
                VISIT_P(checkExpr);
                VISIT_P(exprList);
            }
        break;
		/* integrity constraints */
    	case T_FD:
    		break;
    	case T_FOdep:
    	{
    		PREP_VISIT_P(FOdep);
    		VISIT_P(lhs);
    		VISIT_P(rhs);
    	}
        /* query block model nodes */
        case T_SetQuery:
            {
                PREP_VISIT_P(SetQuery);
                VISIT_P(setOp);
                VISIT_P(selectClause);
                VISIT_P(lChild);
                VISIT_P(rChild);
            }
            break;
        case T_ProvenanceStmt:
            {
                PREP_VISIT_P(ProvenanceStmt);
                VISIT_P(query);
            }
            break;
        case T_QueryBlock:
            {
                PREP_VISIT_P(QueryBlock);
                VISIT_P(selectClause);
                VISIT_P(distinct);
                VISIT_P(fromClause);
                VISIT_P(whereClause);
                VISIT_P(groupByClause);
                VISIT_P(havingClause);
                VISIT_P(orderByClause);
                VISIT_P(limitClause);
                VISIT_P(offsetClause);
            }
            break;
        case T_SelectItem:
            {
                PREP_VISIT_P(SelectItem);
                VISIT_P(expr);
            }
            break;
        case T_FromTableRef:
            {
                LOG_VISIT_ONLY(FromTableRef);
            }
            break;
        case T_FromSubquery:
            {
                PREP_VISIT_P(FromSubquery);
                VISIT_P(subquery);
            }
            break;
        case T_FromLateralSubquery:
            {
                PREP_VISIT_P(FromLateralSubquery);
                VISIT_P(subquery);
            }
            break;
        case T_FromJoinExpr:
            {
                PREP_VISIT_P(FromJoinExpr);
                VISIT_P(left);
                VISIT_P(right);
                VISIT_P(cond);
            }
            break;
        case T_DistinctClause:
            {
                PREP_VISIT_P(DistinctClause);
                VISIT_P(distinctExprs);
            }
            break;
        case T_NestedSubquery:
            {
                PREP_VISIT_P(NestedSubquery);
                VISIT_P(expr);
                VISIT_P(query);
            }
            break;
        case T_Insert:
            {
                PREP_VISIT_P(Insert);
                VISIT_P(query);
            }
            break;
        case T_Delete:
            {
                PREP_VISIT_P(Delete);
                VISIT_P(cond);
            }
            break;
        case T_Update:
            {
                PREP_VISIT_P(Update);
                VISIT_P(selectClause);
                VISIT_P(cond);
            }
            break;
        /* query operator model nodes */
        case T_Schema:
            {
                PREP_VISIT_P(Schema);
                VISIT_P(attrDefs);
            }
            break;
        case T_AttributeDef:
            {
                LOG_VISIT_ONLY(AttributeDef);
                //VISIT_P();
            }
            break;
	    case T_ParameterizedQuery:
		{
			PREP_VISIT_P(ParameterizedQuery);
			VISIT_P(q);
			VISIT_P(parameters);
		}
		break;
        case T_SelectionOperator:
            {
                PREP_VISIT_P(SelectionOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(cond);
            }
            break;
        case T_ProjectionOperator:
            {
                PREP_VISIT_P(ProjectionOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(projExprs);
            }
            break;
        case T_JoinOperator:
            {
                PREP_VISIT_P(JoinOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(cond);
            }
            break;
        case T_AggregationOperator:
            {
                PREP_VISIT_P(AggregationOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(aggrs);
                VISIT_P(groupBy);
            }
            break;
        case T_ProvenanceComputation:
            {
                PREP_VISIT_P(ProvenanceComputation);
                VISIT_OPERATOR_FIELDS_P();
            }
            break;
        case T_TableAccessOperator:
            {
                PREP_VISIT_P(TableAccessOperator);
                VISIT_OPERATOR_FIELDS_P();
            }
            break;
        case T_SampleClauseOperator:
			{
				PREP_VISIT_P(SampleClauseOperator);
				VISIT_OPERATOR_FIELDS_P();
			}
			break;
        case T_SetOperator:
            {
                PREP_VISIT_P(SetOperator);
                VISIT_OPERATOR_FIELDS_P();
            }
            break;
        case T_DuplicateRemoval:
            {
                PREP_VISIT_P(DuplicateRemoval);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(attrs);
            }
            break;
        case T_NestingOperator:
            {
                PREP_VISIT_P(NestingOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(cond);
            }
            break;
        case T_WindowOperator:
            {
                PREP_VISIT_P(WindowOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(partitionBy);
                VISIT_P(orderBy);
                VISIT_P(frameDef);
                VISIT_P(f);
            }
            break;
        case T_OrderOperator:
            {
                PREP_VISIT_P(OrderOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(orderExprs);
            }
            break;
        case T_ConstRelOperator:
            {
                PREP_VISIT_P(ConstRelOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(values);
            }
            break;
	    case T_LimitOperator:
            {
                PREP_VISIT_P(LimitOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(limitExpr);
			    VISIT_P(offsetExpr);
            }
            break;
        case T_JsonTableOperator:
            {
                PREP_VISIT_P(JsonTableOperator);
                VISIT_OPERATOR_FIELDS_P();
                VISIT_P(columns);
            }
            break;

        case T_psInfo:
            {
                PREP_VISIT_P(psInfo);

                VISIT_P(tablePSAttrInfos);
            }
            break;
        case T_psAttrInfo:
            {
                PREP_VISIT_P(psAttrInfo);
                VISIT_P(rangeList);
                VISIT_P(BitVector);
                VISIT_P(psIndexList);
            }
            break;
        case T_psInfoCell:
            {
                PREP_VISIT_P(psInfoCell);

                VISIT_P(ps);
            }
            break;
        default:
            break;
    }

    return TRUE;
}
