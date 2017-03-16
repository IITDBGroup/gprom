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


#define TUPLE "tuple"
#define COMMA ","
#define PROV "PROV"
#define XMLELEMENT "XMLELEMENT"
#define XMLAGG "XMLAGG"
#define MULT "MULT"
#define ADDS "ADDS"

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
static void replaceOperatorInOpTree(QueryOperator *orginal, QueryOperator *new);
static void addOperatorOnTopOfOp(QueryOperator *op, QueryOperator *top);
static FunctionCall *createProvXMLFunctionCall(QueryOperator *op, char *type1, char *type2);


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
	ASSERT(OP_LCHILD(op));

	DEBUG_LOG("REWRITE-XML - Selection");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	// rewrite child first
	rewriteXMLOperator(OP_LCHILD(op));

	// adapt schema
	addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

	LOG_RESULT("Rewritten Operator tree", op);
	return (QueryOperator *) op;
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
    QueryOperator *jOp = (QueryOperator *) op;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    // rewrite children
    lChild = rewriteXMLOperator(lChild);
    rChild = rewriteXMLOperator(rChild);

    // adapt schema for join op
    // PROV in left child -> PROV_L, PROV in right child -> PROV_R
    renameProvName(lChild, "L");
    renameProvName(rChild, "R");
    // new join op schema: A, B, PROV_L, C, D, PROV_R
    jOp->schema->attrDefs = concatTwoLists(copyList(lChild->schema->attrDefs), copyList(rChild->schema->attrDefs));

    //adapt join prov position
    int numLAttrs, numRAttrs;
    List *provList = NIL;
    numLAttrs = LIST_LENGTH(lChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rChild->schema->attrDefs);
    provList = appendToTailOfListInt(provList,numLAttrs-1);
    provList = appendToTailOfListInt(provList,numLAttrs+numRAttrs-1);
    jOp->provAttrs = provList; //in this example, 2, 5  2->PROV_L  5->PROV_R

    // add projection to put attributes into order on top of join op
    //create function call
    Constant *mult = createConstString(MULT);
    Constant *cma = createConstString(COMMA);
    int schemaLen = LIST_LENGTH(jOp->schema->attrDefs);

    List *funArgs = NIL;
    funArgs = appendToTailOfList(funArgs, mult);

    FOREACH_INT(i,jOp->provAttrs)
    {
    	AttributeDef *ad = getNthOfListP(jOp->schema->attrDefs, i);
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType));
    	if(schemaLen != i + 1)
    		funArgs = appendToTailOfList(funArgs, copyObject(cma));
    }

    // XMLELEMENT('MULT', PROV_L, ',', PROV_R)
    FunctionCall *funXML = createFunctionCall(XMLELEMENT, funArgs);

    // A, B, C, D, XMLELEMENT('MULT', PROV_L, ',', PROV_R)
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) jOp),
            singleton(funXML));
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

    //A, B, C, D
    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) jOp);
    //PROV
    proj->op.schema->attrDefs = appendToTailOfList(proj->op.schema->attrDefs,createAttributeDef(PROV, DT_STRING));
    proj->op.provAttrs = singletonInt(numLAttrs+numRAttrs-2);

    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    LOG_RESULT("Rewritten Operator tree", op);

    return (QueryOperator *) op;
}
static QueryOperator
*rewriteXMLAggregation (AggregationOperator *op)
{

	QueryOperator *agg = (QueryOperator *)op;
	int aggrLen = LIST_LENGTH(op->aggrs);
	int aggDefLen = LIST_LENGTH(agg->schema->attrDefs);
	List *projDefs = copyList(agg->schema->attrDefs);
	List *projExprs = NIL;
	int attrPosProj = 0;

    FOREACH(AttributeDef, ad, projDefs)
	{
    	if(attrPosProj == aggrLen)
    		attrPosProj ++;
    	projExprs = appendToTailOfList(projExprs, createFullAttrReference(ad->attrName, 0, attrPosProj, 0, ad->dataType));
        attrPosProj ++;
	}
    projDefs = appendToTailOfList(projDefs, createAttributeDef(PROV, DT_STRING));

	rewriteXMLOperator(OP_LCHILD(op));

	QueryOperator *lChild = OP_LCHILD(op);

	//modify aggregation op
    //create function call in agg op
    List *funArgs = NIL;
    FOREACH_INT(i, lChild->provAttrs)
    {
    	AttributeDef *ad = getNthOfListP(lChild->schema->attrDefs, i);
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(ad->attrName, 0, i, 0, ad->dataType));
    }
    FunctionCall *funXMLAGG = createFunctionCall(XMLAGG, funArgs);
    op->aggrs = appendToTailOfList(op->aggrs, funXMLAGG);

    AttributeDef *provDef = createAttributeDef(CONCAT_STRINGS("AGGR_", itoa(aggrLen)), DT_STRING);
    List *newAggDefs = NIL;
    List *preAggDefs = NIL;
    List *aftAggDefs = NIL;
    int cnt =0;
    FOREACH(AttributeDef, ad, op->op.schema->attrDefs)
    {
    	if(cnt < aggrLen)
    		preAggDefs = appendToTailOfList(preAggDefs, ad);
    	else
    		aftAggDefs = appendToTailOfList(aftAggDefs, ad);
    	cnt ++;
    }
    //AGGR_0 AGGR_1 GROUP_0
    newAggDefs = CONCAT_LISTS(preAggDefs, singleton(provDef), aftAggDefs);
    op->op.schema->attrDefs = newAggDefs;

    //new proj
    List *funArgsProj = NIL;
    AttributeReference *provAttrProj = createFullAttrReference(strdup(provDef->attrName), 0, aggrLen, 0, provDef->dataType);
    Constant *adds = createConstString(ADDS);
    funArgsProj = appendToTailOfList(funArgsProj, adds);
    funArgsProj = appendToTailOfList(funArgsProj, provAttrProj);
    FunctionCall *funXML = createFunctionCall(XMLELEMENT, funArgsProj);

    //AGGR_0 GROUP_0 XMLELEMENT('ADDS', AGGR_1)
    projExprs = appendToTailOfList(projExprs,funXML);

    ProjectionOperator *proj = createProjectionOp(projExprs, NULL, NIL, NIL);
    proj->op.schema->attrDefs = projDefs;
    proj->op.provAttrs = singletonInt(aggDefLen);

    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLSet (SetOperator *op)
{
	DEBUG_LOG("REWRITE-XML - Set");

	QueryOperator *setOp = (QueryOperator *) op;
	QueryOperator *lChild = OP_LCHILD(op);
	QueryOperator *rChild = OP_RCHILD(op);

	switch(op->setOpType)
	{
	case SETOP_UNION:
	{

		// rewrite children
		lChild = rewriteXMLOperator(lChild);
		rChild = rewriteXMLOperator(rChild);

		addProvenanceAttrsToSchema(setOp, lChild);


		return (QueryOperator *) op;
	}
	case SETOP_INTERSECTION:
	{
		// rewrite children
		lChild = rewriteXMLOperator(lChild);
		rChild = rewriteXMLOperator(rChild);

		return (QueryOperator *) op;
	}
	case SETOP_DIFFERENCE:
	{
		// rewrite children
		lChild = rewriteXMLOperator(lChild);
		rChild = rewriteXMLOperator(rChild);

		return (QueryOperator *) op;
	}
	default:
		break;
	}
	return NULL;
}
static QueryOperator
*rewriteXMLTableAccess(TableAccessOperator *op)
{
	List *provAttr = NIL;
    List *projExpr = NIL;
    List *provPos = NIL;
    int cnt = 0;

    DEBUG_LOG("REWRITE-XML - Table Access <%s>", op->tableName);

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }
    provPos = appendToTailOfListInt(provPos, cnt);

    //create function call
    //Constant *tup = createConstString(TUPLE);  // 'tuple'
    Constant *tup = createConstString(strdup(op->tableName));  // 'tuple'
    Constant *cma = createConstString(COMMA);  // ','
    List *funArgs = NIL;

    List *attrDefs = op->op.schema->attrDefs;
    int schemaLen = LIST_LENGTH(attrDefs);
    funArgs = appendToTailOfList(funArgs, copyObject(tup));
    cnt = 0;

    //construct functionCall XMLELEMENT(tuple, A , ',' , B) AS PROV
    FOREACH(AttributeDef, attr, attrDefs)
    {
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
    	if(schemaLen != cnt + 1)
    		funArgs = appendToTailOfList(funArgs, copyObject(cma));
        cnt++;
    }

    FunctionCall *funXML = createFunctionCall(XMLELEMENT, funArgs);
    provAttr = appendToTailOfList(provAttr, PROV);  //add 'PROV'
    projExpr = appendToTailOfList(projExpr, funXML); //add XMLELEMENT(tuple, A , ',' , B)

    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(funXML));

    // Create a new projection operator with these new attributes
    // A, B, XMLELEMENT(tuple, A , ',' , B) -> A, B, PROV
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, provAttr);
    proj->op.provAttrs = provPos;
    SET_BOOL_STRING_PROP((QueryOperator *)proj, PROP_PROJ_PROV_ATTR_DUP);

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    DEBUG_LOG("rewrite table access: %s", operatorToOverviewString((Node *) proj));

    return (QueryOperator *) proj;
}
static QueryOperator
*rewriteXMLConstRel(ConstRelOperator *op)
{
	return (QueryOperator *)op;
}
static QueryOperator
*rewriteXMLDuplicateRemOp(DuplicateRemoval *op)
{

    QueryOperator *child = OP_LCHILD(op);
    QueryOperator *theOp = (QueryOperator *) op;

    //rewrite child op
    rewriteXMLOperator(child);

    //create aggregation
    List *aggrs = NIL;
    List *groupBy = NIL;
    List *attrNames = NIL;

    //create functional call
    FunctionCall *funXMLAGG = createProvXMLFunctionCall(child, XMLAGG, NULL);
    aggrs = appendToTailOfList(aggrs, funXMLAGG);

    groupBy = getNormalAttrProjectionExprs(child);
    attrNames = concatTwoLists(getOpProvenanceAttrNames(child) ,getNormalAttrNames(child));

    AggregationOperator *agg = createAggregationOp(aggrs, groupBy, NULL, NIL, attrNames);
    ((QueryOperator *) agg)->provAttrs = singletonInt(0);

    //replace theOp with agg if no additional proj on the top of agg
    replaceOperatorInOpTree(theOp, (QueryOperator *) agg);

    //create new proj on top of agg to get A, B, xmlelement(adds, prov)
    QueryOperator *aggOp = (QueryOperator *) agg;
    List *projExprs = getNormalAttrProjectionExprs(aggOp);
    List *projAttrNames = concatTwoLists(getNormalAttrNames(aggOp),getOpProvenanceAttrNames(aggOp));

    //create function call
    FunctionCall *funXMLELEMENT = createProvXMLFunctionCall(aggOp, XMLELEMENT, ADDS);
    projExprs = appendToTailOfList(projExprs, funXMLELEMENT);

    ProjectionOperator *proj = createProjectionOp(projExprs, NULL, NIL, projAttrNames);
    ((QueryOperator *) proj)->provAttrs = singletonInt(LIST_LENGTH(projAttrNames) - 1);
    addOperatorOnTopOfOp(aggOp, (QueryOperator *) proj);


	return (QueryOperator *) proj;
}
static QueryOperator
*rewriteXMLOrderOp(OrderOperator *op)
{
	QueryOperator *child = OP_LCHILD(op);

	// rewrite child
	rewriteXMLOperator(child);

	// adapt provenance attr list and schema
	addProvenanceAttrsToSchema((QueryOperator *) op, child);

	return (QueryOperator *) op;
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
	ad->attrName = CONCAT_STRINGS(ad->attrName, "_", suffix);

	return (QueryOperator *)op;
}

static FunctionCall
*createProvXMLFunctionCall(QueryOperator *op, char *type1, char *type2)
{
    List *funArgs = NIL;

    if(type2 != NULL)
    {
    	Constant *func = createConstString(type2);
    	funArgs = appendToTailOfList(funArgs, func);
    }
    FOREACH_INT(i, op->provAttrs)
    {
    	AttributeDef *ad1 = getNthOfListP(op->schema->attrDefs, i);
    	funArgs = appendToTailOfList(funArgs, createFullAttrReference(ad1->attrName, 0, i, 0, ad1->dataType));
    }
    FunctionCall *funXMLELEMENT = createFunctionCall(type1, funArgs);

    return funXMLELEMENT;
}


static void
addOperatorOnTopOfOp(QueryOperator *op, QueryOperator *top)
{
	switchSubtreeWithExisting(op, top);
	op->parents = singleton(top);
	top->inputs = singleton(op);
}

static void
replaceOperatorInOpTree(QueryOperator *orginal, QueryOperator *new)
{
	QueryOperator *child = OP_LCHILD(orginal);
    new->inputs = orginal->inputs;
    orginal->inputs = NIL;
    switchSubtreeWithExisting(orginal, new);
    replaceNode(child->parents, (Node *)orginal, (Node *)new);
}
