/*
 * xml_prov_main.c
 *
 *  Created on: Mar 9, 2017
 *      Author: Xing
 */

#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "provenance_rewriter/xml_rewrites/xml_prov_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/list/list.h"



static QueryOperator *rewriteXMLOperator (QueryOperator *op);
static QueryOperator *rewriteXMLSelection (SelectionOperator *op);
static QueryOperator *rewriteXMLProjection (ProjectionOperator *op);
static QueryOperator *rewriteXMLJoin (JoinOperator *op);
static QueryOperator *rewriteXMLAggregation (AggregationOperator *op);
static QueryOperator *rewriteXMLSet (SetOperator *op);
static QueryOperator *rewriteXMLTableAccess(TableAccessOperator *op);
static QueryOperator *rewriteXMLConstRel(ConstRelOperator *op);
static QueryOperator *rewriteXMLDuplicateRemOp(DuplicateRemoval *op);
static QueryOperator *rewriteXMLOrderOp(OrderOperator *op);
static QueryOperator *rewriteXMLJsonTableOp(JsonTableOperator *op);


QueryOperator *
rewriteXML (ProvenanceComputation  *op)
{
    START_TIMER("rewrite - XML rewrite");

    // unset relation name counters
    //nameState = (RelCount *) NULL;

    DEBUG_NODE_BEATIFY_LOG("*************************************\nREWRITE INPUT\n"
            "******************************\n", op);

    QueryOperator *rewRoot = OP_LCHILD(op);
    DEBUG_NODE_BEATIFY_LOG("rewRoot is:", rewRoot);

    // cache asOf
    //asOf = op->asOf;

    // get provenance attrs
//    provAttrs = getQueryOperatorAttrNames((QueryOperator *) op);

    // rewrite subquery under provenance computation
    rewriteXMLOperator(rewRoot);

    // update root of rewritten subquery
    rewRoot = OP_LCHILD(op);

    // adapt inputs of parents to remove provenance computation
    //switchSubtrees((QueryOperator *) op, rewRoot);
    //DEBUG_NODE_BEATIFY_LOG("rewritten query root is:", rewRoot);

    STOP_TIMER("rewrite - XML rewrite");

	return rewRoot;
}



QueryOperator *rewriteXMLOperator (QueryOperator *op)
{
	QueryOperator *rewrittenOp;

	switch(op->type)
	{
	case T_SelectionOperator:
		DEBUG_LOG("go selection");
		rewrittenOp = rewriteXMLSelection((SelectionOperator *) op);
		break;
	case T_ProjectionOperator:
		DEBUG_LOG("go projection");
		rewrittenOp = rewriteXMLProjection((ProjectionOperator *) op);
		break;
	case T_AggregationOperator:
		DEBUG_LOG("go aggregation");
		rewrittenOp = rewriteXMLAggregation ((AggregationOperator *) op);
		break;
	case T_JoinOperator:
		DEBUG_LOG("go join");
		rewrittenOp = rewriteXMLJoin((JoinOperator *) op);
		break;
	case T_SetOperator:
		DEBUG_LOG("go set");
		rewrittenOp = rewriteXMLSet((SetOperator *) op);
		break;
	case T_TableAccessOperator:
		DEBUG_LOG("go table access");
		rewrittenOp = rewriteXMLTableAccess((TableAccessOperator *) op);
		break;
	case T_ConstRelOperator:
		DEBUG_LOG("go const rel operator");
		rewrittenOp = rewriteXMLConstRel((ConstRelOperator *) op);
		break;
	case T_DuplicateRemoval:
		DEBUG_LOG("go duplicate removal operator");
		rewrittenOp = rewriteXMLDuplicateRemOp((DuplicateRemoval *) op);
		break;
	case T_OrderOperator:
		DEBUG_LOG("go order operator");
		rewrittenOp = rewriteXMLOrderOp((OrderOperator *) op);
		break;
	case T_JsonTableOperator:
		DEBUG_LOG("go JsonTable operator");
		rewrittenOp = rewriteXMLJsonTableOp((JsonTableOperator *) op);
		break;
	default:
		FATAL_LOG("no rewrite implemented for operator ", nodeToString(op));
		return NULL;
	}

	return rewrittenOp;
}



static QueryOperator
*rewriteXMLSelection (SelectionOperator *op)
{

	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLProjection (ProjectionOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLJoin (JoinOperator *op)
{

	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLAggregation (AggregationOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLSet (SetOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLTableAccess(TableAccessOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLConstRel(ConstRelOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLDuplicateRemOp(DuplicateRemoval *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLOrderOp(OrderOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLJsonTableOp(JsonTableOperator *op)
{
	return (QueryOperator *)op;
}
