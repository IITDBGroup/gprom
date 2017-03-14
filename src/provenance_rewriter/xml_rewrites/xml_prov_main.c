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

static QueryOperator *renameProvName(QueryOperator *op, char *suffix);


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

    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
         op->projExprs = appendToTailOfList(op->projExprs,
                 createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
    }

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    LOG_RESULT("Rewritten Operator tree", op);

	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLJoin (JoinOperator *op)
{

    DEBUG_LOG("REWRITE-XML - Join");
    QueryOperator *o = (QueryOperator *) op;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    // rewrite children
    lChild = rewriteXMLOperator(lChild);
    rChild = rewriteXMLOperator(rChild);

    // adapt schema for join op
    renameProvName(lChild, "L");
    renameProvName(rChild, "R");
    o->schema->attrDefs = concatTwoLists(copyList(lChild->schema->attrDefs), copyList(rChild->schema->attrDefs));

    //adapt join prov position
    int numLAttrs, numRAttrs;
    List *provList = NIL;
    numLAttrs = LIST_LENGTH(lChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rChild->schema->attrDefs);
    provList = appendToTailOfListInt(provList,numLAttrs-1);
    provList = appendToTailOfListInt(provList,numLAttrs+numRAttrs-1);
    o->provAttrs = provList;

    // add projection to put attributes into order on top of join op
    //create function call
    Constant *mult = createConstString("mult");
    Constant *quo = createConstString(",");
    List *funArgs = NIL;
    funArgs = appendToTailOfList(funArgs, mult);

    int schemaLen = LIST_LENGTH(o->schema->attrDefs);
    FOREACH_INT(i,o->provAttrs)
    {
    	AttributeDef *ad = getNthOfListP(o->schema->attrDefs, i);
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType));
    	if(schemaLen != i + 1)
    		funArgs = appendToTailOfList(funArgs, copyObject(quo));
    }

    FunctionCall *funXML = createFunctionCall("XMLELEMENT", funArgs);

    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) o),
            singleton(funXML));
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) o);
    proj->op.schema->attrDefs = appendToTailOfList(proj->op.schema->attrDefs,createAttributeDef("PROV", DT_STRING));
    proj->op.provAttrs = singletonInt(numLAttrs+numRAttrs-2);

    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    // SET PROP IS_REWRITTEN

    LOG_RESULT("Rewritten Operator tree", op);
    return (QueryOperator *) op;

}
static QueryOperator
*rewriteXMLAggregation (AggregationOperator *op)
{

	QueryOperator *agg = (QueryOperator *)op;
	int aggrLen = LIST_LENGTH(op->aggrs);
	int aggDefLen = LIST_LENGTH(agg->schema->attrDefs);
	List *newProjDefs = copyList(agg->schema->attrDefs);
	List *newProjExprs = NIL;
	int attrPosProj = 0;
    FOREACH(AttributeDef, ad, newProjDefs)
	{
    	if(attrPosProj == aggrLen)
    		attrPosProj ++;
        newProjExprs = appendToTailOfList(newProjExprs, createFullAttrReference(ad->attrName, 0, attrPosProj, 0, ad->dataType));
        attrPosProj ++;
	}
    newProjDefs = appendToTailOfList(newProjDefs, createAttributeDef("PROV", DT_STRING));



	rewriteXMLOperator(OP_LCHILD(op));
	QueryOperator *lChild = OP_LCHILD(op);


    //create function call
    List *funArgs = NIL;
    //int schemaLen = LIST_LENGTH(op->op.schema->attrDefs);
    FOREACH_INT(i, lChild->provAttrs)
    {
    	AttributeDef *ad = getNthOfListP(lChild->schema->attrDefs, i);
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType));
    }
    FunctionCall *funXMLAGG = createFunctionCall("XMLAGG", funArgs);
    op->aggrs = appendToTailOfList(op->aggrs, funXMLAGG);


    AttributeDef *provDef = createAttributeDef(CONCAT_STRINGS("AGGR_", itoa(aggrLen)), DT_STRING);
    List *newAggDefs = NIL;
    List *proAggDefs = NIL;
    List *postAggDefs = NIL;
    int cnt =0;
    FOREACH(AttributeDef, ad, op->op.schema->attrDefs)
    {
    	if(cnt < aggrLen)
    		proAggDefs = appendToTailOfList(proAggDefs, ad);
    	else
    		postAggDefs = appendToTailOfList(postAggDefs, ad);
    	cnt ++;
    }
    newAggDefs = CONCAT_LISTS(proAggDefs, singleton(provDef), postAggDefs);
    op->op.schema->attrDefs = newAggDefs;


    //new proj
    List *funArgsProj = NIL;
    AttributeReference *provAttrProj = createFullAttrReference(strdup(provDef->attrName), 0, aggrLen, 0, provDef->dataType);
    Constant *adds = createConstString("adds");
    funArgsProj = appendToTailOfList(funArgsProj, adds);
    funArgsProj = appendToTailOfList(funArgsProj, provAttrProj);
    FunctionCall *funXML = createFunctionCall("XMLELEMENT", funArgsProj);
    newProjExprs = appendToTailOfList(newProjExprs,funXML);


    ProjectionOperator *proj = createProjectionOp(newProjExprs, NULL, NIL, NIL);
    proj->op.schema->attrDefs = newProjDefs;
    proj->op.provAttrs = singletonInt(aggDefLen);

    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

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
    List *newProvPosList = NIL;

    int cnt = 0;

    DEBUG_LOG("REWRITE-XML - Table Access <%s>", op->tableName);

    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }
    newProvPosList = appendToTailOfListInt(newProvPosList, cnt);

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

    FunctionCall *funXML = createFunctionCall("XMLELEMENT", funArgs);
    provAttr = appendToTailOfList(provAttr, "PROV");
    projExpr = appendToTailOfList(projExpr, funXML);

//    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
//            stringListToString(provAttr),
//            nodeToString(projExpr)
//            );

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;
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




static QueryOperator
*renameProvName(QueryOperator *op, char *suffix)
{
      int i = getHeadOfListInt(op->provAttrs);
      AttributeDef *ad = getNthOfListP(op->schema->attrDefs, i);
//      StringInfo str = makeStringInfo();
//      appendStringInfoString(str, ad->attrName);
//      appendStringInfoString(str, "_");
//      appendStringInfoString(str, suffix);
//      ad->attrName = str->data;

      ad->attrName = CONCAT_STRINGS(ad->attrName, "_", suffix);


	return (QueryOperator *)op;
}
