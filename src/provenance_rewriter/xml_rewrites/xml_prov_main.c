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

#define LOG_RESULT(mes,op) \
    do { \
        INFO_OP_LOG(mes,op); \
        DEBUG_NODE_BEATIFY_LOG(mes,op); \
    } while(0)


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
	 ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-XML - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child
    rewriteXMLOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(op);

    int ocnt = 0;
    FOREACH(AttributeDef, ad, child->schema->attrDefs)
    {
    	if(streq(ad->attrName, "PROV"))
    	{
    		op->op.schema->attrDefs = appendToTailOfList(op->op.schema->attrDefs, copyObject(ad));
    		op->projExprs = appendToTailOfList(op->projExprs,
    				createFullAttrReference(ad->attrName, 0, ocnt, 0, ad->dataType));
    	}
    	ocnt ++;
    }

//    FOREACH_INT(a, child->provAttrs)
//    {
//        AttributeDef *att = getAttrDef(child,a);
//        DEBUG_LOG("attr: %s", nodeToString(att));
//         op->projExprs = appendToTailOfList(op->projExprs,
//                 createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
//    }
//
//    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
//
    LOG_RESULT("Rewritten Operator tree", op);

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
	  List *provAttr = NIL;
    List *projExpr = NIL;

    int cnt = 0;

    DEBUG_LOG("REWRITE-XML - Table Access <%s>", op->tableName);

    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    //create function call
    Constant *tup = createConstString("tuple");
    Constant *quo = createConstString(",");
    List *funArgs = NIL;
    funArgs = appendToTailOfList(funArgs, tup);

    cnt = 0;
    int schemaLen = LIST_LENGTH(op->op.schema->attrDefs);
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
    	if(schemaLen != cnt + 1)
    		funArgs = appendToTailOfList(funArgs, copyObject(quo));
        cnt++;
    }

//    List *newProvPosList = NIL;
//    CREATE_INT_SEQ(newProvPosList, 1, (1 * 2) - 1, 1);
//    FOREACH(Node, n, funArgs)
//    {
//    	DEBUG_LOG("functioncall: %s", nodeToString(n));
//    }

    FunctionCall *funXML = createFunctionCall("XMLELEMENT", funArgs);
    provAttr = appendToTailOfList(provAttr, "PROV");
    projExpr = appendToTailOfList(projExpr, funXML);

//    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
//            stringListToString(provAttr),
//            nodeToString(projExpr)
//            );

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    //newpo->op.provAttrs = newProvPosList;
    SET_BOOL_STRING_PROP((QueryOperator *)newpo, PROP_PROJ_PROV_ATTR_DUP);

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite table access: %s", operatorToOverviewString((Node *) newpo));



    return (QueryOperator *) newpo;
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
