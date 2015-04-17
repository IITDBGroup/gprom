/*-----------------------------------------------------------------------------
 *
 * transformation_prov_main.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"
#include "log/logger.h"
#include "model/query_operator/query_operator.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/list/list.h"
#include "mem_manager/mem_mgr.h"


Operator *
generateOp(int *count, Node *attr1, Node *attr2)
{
    List *argList = NIL;
    Operator *o = NULL;

    if(isA(attr2, AttributeDef))
    {
    	AttributeDef *attrDef= (AttributeDef *) attr2;
    	AttributeReference *attrRef = createAttributeReference(attrDef->attrName);
    	attrRef->fromClauseItem = 0;
    	attrRef->attrType = attrDef->dataType;
    	attrRef->attrPosition = *count;
    	(*count) ++;
    	argList = appendToTailOfList(argList, attr1);
    	argList = appendToTailOfList(argList, attrRef);

    }

    else if(isA(attr1, AttributeDef))
    {
    	AttributeDef *attrDef= (AttributeDef *) attr1;
    	AttributeReference *attrRef = createAttributeReference(attrDef->attrName);
    	attrRef->fromClauseItem = 0;
    	attrRef->attrType = attrDef->dataType;
    	attrRef->attrPosition = *count;
    	(*count) ++;
    	argList = appendToTailOfList(argList, attrRef);
    	argList = appendToTailOfList(argList, attr2);
    }
    else
    {
    	argList = appendToTailOfList(argList, attr1);
    	argList = appendToTailOfList(argList, attr2);
    }

    o = createOpExpr("||", argList);

	return o;
}

QueryOperator *
rewriteTransformationProvenance(QueryOperator *op)
{
	List *normalAttrs = getNormalAttrs(op);

	//First projection: normal attributes
	StringInfo prefixQ = makeStringInfo();
	appendStringInfoChar(prefixQ, '"');
	appendStringInfoString(prefixQ, "rel:tupQ(");
	Constant *cPrefixQ = createConstString(prefixQ->data);

	StringInfo prefixR = makeStringInfo();
	appendStringInfoChar(prefixR, '"');
	appendStringInfoString(prefixR, "rel:tup_R(");
	Constant *cPrefixR = createConstString(prefixR->data);

	StringInfo suffix = makeStringInfo();
	appendStringInfoString(suffix, ")");
	appendStringInfoChar(suffix, '"');
	appendStringInfoString(suffix, " : {");
	appendStringInfoChar(suffix, '"');
	appendStringInfoString(suffix, "prov:type");
	appendStringInfoChar(suffix, '"');
	appendStringInfoString(suffix, ":");
	appendStringInfoChar(suffix, '"');
	appendStringInfoString(suffix, "tuple");
	appendStringInfoChar(suffix, '"');
	appendStringInfoString(suffix, "}");
	Constant *cSuffix = createConstString(suffix->data);

	Constant *cBar = createConstString(" | ");

	List *reverseNormalAttrs = copyObject(normalAttrs);
	reverseList(reverseNormalAttrs);
	int count = 0;
	int lenNormalAttrs = LIST_LENGTH(reverseNormalAttrs);
	count = lenNormalAttrs - 1;
	AttributeDef *firstNormalAttrDef = (AttributeDef *) getHeadOfListP(
			reverseNormalAttrs);
	AttributeReference *firstNormalAttr = createAttributeReference(
			firstNormalAttrDef->attrName);
	firstNormalAttr->fromClauseItem = 0;
	firstNormalAttr->attrType = firstNormalAttrDef->dataType;
	firstNormalAttr->attrPosition = count;
	count--;
	List *NormalExprList1 = NIL;
	NormalExprList1 = appendToTailOfList(NormalExprList1, firstNormalAttr);
	NormalExprList1 = appendToTailOfList(NormalExprList1, cSuffix);
	Operator *NormalO1 = createOpExpr("||", NormalExprList1);

	reverseNormalAttrs = removeFromHead(reverseNormalAttrs);
	FOREACH(AttributeDef, a, reverseNormalAttrs) {
		List *exprList2 = NIL;
		exprList2 = appendToTailOfList(exprList2, cBar);
		exprList2 = appendToTailOfList(exprList2, NormalO1);
		Operator *o2 = createOpExpr("||", exprList2);

		AttributeReference *attr = createAttributeReference(a->attrName);
		attr->fromClauseItem = 0;
		attr->attrType = a->dataType;
		attr->attrPosition = count;

		List *exprList3 = NIL;
		exprList3 = appendToTailOfList(exprList3, attr);
		exprList3 = appendToTailOfList(exprList3, o2);
		Operator *o3 = createOpExpr("||", exprList3);
		NormalO1 = copyObject(o3);
		count--;
	}

	List *NormalExprList4 = NIL;
	NormalExprList4 = appendToTailOfList(NormalExprList4, cPrefixQ);
	NormalExprList4 = appendToTailOfList(NormalExprList4, NormalO1);
	Operator *NormalO4 = createOpExpr("||", NormalExprList4);
	DEBUG_LOG("new operator expression %s", nodeToString(NormalO4));
	List *NormalProjExprs = singleton(NormalO4);
	count = lenNormalAttrs;

	int entityCount = 0;
	StringInfo name = makeStringInfo();
	appendStringInfoString(name, "entity");
	List *attrNames = singleton(name->data);
	entityCount++;
	DEBUG_LOG("new attribute name %s", name->data);

	//Introduce projection operator
	ProjectionOperator *proj = createProjectionOp(NormalProjExprs, op, NIL,
			attrNames);
	op->parents = appendToTailOfList(op->parents, proj);
	DEBUG_LOG("new proj %s", nodeToString(proj));

	//Introduce duplicateRemoval operator
	DuplicateRemoval *dr = createDuplicateRemovalOp(NIL, (QueryOperator *) proj,
			NIL, attrNames);
	proj->op.parents = appendToTailOfList(proj->op.parents, dr);

	//Second projection: provenance attributes
	List *provAttrs = getProvenanceAttrDefs(op);
	List *reverseProvAttrs = copyObject(provAttrs);
	reverseList(reverseProvAttrs);

	int len = LIST_LENGTH(reverseProvAttrs);
	count = count + len - 1;
	AttributeDef *firstAttrDef = (AttributeDef *) getHeadOfListP(
			reverseProvAttrs);
	AttributeReference *firstAttr = createAttributeReference(
			firstAttrDef->attrName);
	firstAttr->fromClauseItem = 0;
	firstAttr->attrType = firstAttrDef->dataType;
	firstAttr->attrPosition = count;
	count--;
	List *exprList1 = NIL;
	exprList1 = appendToTailOfList(exprList1, firstAttr);
	exprList1 = appendToTailOfList(exprList1, cSuffix);
	Operator *o1 = createOpExpr("||", exprList1);

	reverseProvAttrs = removeFromHead(reverseProvAttrs);
	FOREACH(AttributeDef, a, reverseProvAttrs) {
		List *exprList2 = NIL;
		exprList2 = appendToTailOfList(exprList2, cBar);
		exprList2 = appendToTailOfList(exprList2, o1);
		Operator *o2 = createOpExpr("||", exprList2);

		AttributeReference *attr = createAttributeReference(a->attrName);
		attr->fromClauseItem = 0;
		attr->attrType = a->dataType;
		attr->attrPosition = count;

		List *exprList3 = NIL;
		exprList3 = appendToTailOfList(exprList3, attr);
		exprList3 = appendToTailOfList(exprList3, o2);
		Operator *o3 = createOpExpr("||", exprList3);
		o1 = copyObject(o3);
		count--;
	}

	List *exprList4 = NIL;
	exprList4 = appendToTailOfList(exprList4, cPrefixR);
	exprList4 = appendToTailOfList(exprList4, o1);
	Operator *o4 = createOpExpr("||", exprList4);
	DEBUG_LOG("new prov operator expression %s", nodeToString(o4));

	List *provProjExprs = singleton(o4);

	StringInfo provName = makeStringInfo();
	appendStringInfoString(provName, "entity");
	List *provAttrNames = singleton(provName->data);
	entityCount++;
	DEBUG_LOG("new prov attribute name %s", provName->data);

	//Introduce projection operator
	ProjectionOperator *provProj = createProjectionOp(provProjExprs, op, NIL,
			provAttrNames);
	op->parents = appendToTailOfList(op->parents, provProj);

	//Introduce duplicateRemoval operator
	DuplicateRemoval *provDr = createDuplicateRemovalOp(NIL,
			(QueryOperator *) provProj, NIL, provAttrNames);
	provProj->op.parents = appendToTailOfList(provProj->op.parents, provDr);

	//Introduce union operator
	List *unionNames = concatTwoLists(attrNames, provAttrNames);
	SetOperator *so = createSetOperator(SETOP_UNION, LIST_MAKE(dr, provDr), NIL,
			unionNames);
	OP_LCHILD(so)->parents = singleton(so);
	OP_RCHILD(so)->parents = singleton(so);

	//Introduce aggregation operator
	AttributeDef *aggDef = (AttributeDef *) getHeadOfListP(
			so->op.schema->attrDefs);
	AttributeReference *aggRef = createAttributeReference(aggDef->attrName);
	aggRef->fromClauseItem = 0;
	aggRef->attrType = aggDef->dataType;
	aggRef->attrPosition = 0;
	FunctionCall *aggFunc = createFunctionCall(strdup("AGG_STRAGG"),
			singleton(aggRef));

	List *aggrs = singleton(aggFunc);

	// create fake attribute names for aggregation output schema
	List *aggAttrNames = NIL;
	aggAttrNames = singleton("allEntities");

	AggregationOperator *ao = createAggregationOp(aggrs, NIL,
			(QueryOperator *) so, NIL, aggAttrNames);
	so->op.parents = appendToTailOfList(so->op.parents, ao);

	/* Second Part: derivedBy and allDerived */

	//Introduce projection operator
	char *wdb = "_:wdb";

	StringInfo prefixDB = makeStringInfo();
	appendStringInfoChar(prefixDB, '"');
	appendStringInfoString(prefixDB, strdup(wdb));
	Constant *cPrefixDB = createConstString(prefixDB->data);

	int lenNormal = LIST_LENGTH(normalAttrs);
	List *wholeAttrs = concatTwoLists(copyObject(normalAttrs),
			copyObject(provAttrs));

	int countDB = 0;
	AttributeDef *firstNormalAttrDefDB = (AttributeDef *) getHeadOfListP(
			wholeAttrs);
	AttributeReference *firstNormalAttrDB = createAttributeReference(
			firstNormalAttrDefDB->attrName);
	firstNormalAttrDB->fromClauseItem = 0;
	firstNormalAttrDB->attrType = firstNormalAttrDefDB->dataType;
	firstNormalAttrDB->attrPosition = countDB;
	countDB++;
	List *NormalExprListDB1 = NIL;
	NormalExprListDB1 = appendToTailOfList(NormalExprListDB1, cPrefixDB);
	NormalExprListDB1 = appendToTailOfList(NormalExprListDB1,
			firstNormalAttrDB);
	Operator *NormalODB1 = createOpExpr("||", NormalExprListDB1);

	wholeAttrs = removeFromHead(wholeAttrs);
	FOREACH(AttributeDef, a, wholeAttrs) {
		List *exprList2 = NIL;
		exprList2 = appendToTailOfList(exprList2, NormalODB1);
		exprList2 = appendToTailOfList(exprList2, cBar);
		Operator *o2 = createOpExpr("||", exprList2);

		AttributeReference *attr = createAttributeReference(a->attrName);
		attr->fromClauseItem = 0;
		attr->attrType = a->dataType;
		attr->attrPosition = countDB;

		List *exprList3 = NIL;
		exprList3 = appendToTailOfList(exprList3, o2);
		exprList3 = appendToTailOfList(exprList3, attr);
		Operator *o3 = createOpExpr("||", exprList3);
		NormalODB1 = copyObject(o3);
		countDB++;
	}

	List *exprListDB4 = NIL;
	StringInfo midDB = makeStringInfo();
	appendStringInfoString(midDB, ")");
	appendStringInfoChar(midDB, '"');
	appendStringInfoString(midDB, ": { ");
	//appendStringInfoChar(midDB, '"');
	//appendStringInfoString(midDB, "prov:activity");
	//appendStringInfoChar(midDB, '"');
	//appendStringInfoString(midDB, ": ");
	//appendStringInfoChar(midDB, '"');
	//appendStringInfoChar(midDB, 'Q');
	//appendStringInfoChar(midDB, '"');
	//appendStringInfoChar(midDB, ',');
	appendStringInfoChar(midDB, '"');
	appendStringInfoString(midDB, "prov:usedEntity");
	appendStringInfoChar(midDB, '"');
	appendStringInfoString(midDB, ": ");
	appendStringInfoChar(midDB, '"');
	appendStringInfoString(midDB, "rel:tup_R(");
	Constant *cmidDB = createConstString(midDB->data);

	exprListDB4 = appendToTailOfList(exprListDB4, NormalODB1);
	exprListDB4 = appendToTailOfList(exprListDB4, cmidDB);
	Operator *DBo4 = createOpExpr("||", exprListDB4);
	//DEBUG_LOG("new prov operator expression %s", nodeToString(DBo4));

	List *provAttrs1 = copyObject(provAttrs);
	AttributeDef *firstProvAttrDef1 = (AttributeDef *) getHeadOfListP(
			provAttrs1);
	AttributeReference *firstProvAttr1 = createAttributeReference(
			firstProvAttrDef1->attrName);
	firstProvAttr1->fromClauseItem = 0;
	firstProvAttr1->attrType = firstProvAttrDef1->dataType;
	firstProvAttr1->attrPosition = lenNormal;
	lenNormal++;

	List *provExprList1 = NIL;
	provExprList1 = appendToTailOfList(provExprList1, DBo4);
	provExprList1 = appendToTailOfList(provExprList1, firstProvAttr1);
	Operator *provO1 = createOpExpr("||", provExprList1);

	provAttrs1 = removeFromHead(provAttrs1);
	FOREACH(AttributeDef, a, provAttrs1) {
		List *exprList2 = NIL;
		exprList2 = appendToTailOfList(exprList2, provO1);
		exprList2 = appendToTailOfList(exprList2, cBar);
		Operator *o2 = createOpExpr("||", exprList2);

		AttributeReference *attr = createAttributeReference(a->attrName);
		attr->fromClauseItem = 0;
		attr->attrType = a->dataType;
		attr->attrPosition = lenNormal;

		List *exprList3 = NIL;
		exprList3 = appendToTailOfList(exprList3, o2);
		exprList3 = appendToTailOfList(exprList3, attr);
		Operator *o3 = createOpExpr("||", exprList3);
		provO1 = copyObject(o3);
		lenNormal++;
	}

	List *provExprList4 = NIL;
	StringInfo midProv = makeStringInfo();
	appendStringInfoChar(midProv, ')');
	appendStringInfoChar(midProv, '"');
	Constant *cmidProv = createConstString(midProv->data);

	provExprList4 = appendToTailOfList(provExprList4, provO1);
	provExprList4 = appendToTailOfList(provExprList4, cmidProv);
	Operator *midProvO4 = createOpExpr("||", provExprList4);
	//DEBUG_LOG("new prov operator expression %s", nodeToString(midProvO4));

	List *normalExprList = NIL;
	StringInfo midNor = makeStringInfo();
	appendStringInfoString(midNor, ",");
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoString(midNor, "prov:usage");
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoChar(midNor, ':');
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoString(midNor, "u1");
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoChar(midNor, ',');
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoString(midNor, "prov:generation");
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoChar(midNor, ':');
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoString(midNor, "g2");
	//appendStringInfoChar(midNor, '"');
	//appendStringInfoChar(midNor, ',');
	appendStringInfoChar(midNor, '"');
	appendStringInfoString(midNor, "prov:generatedEntity");
	appendStringInfoChar(midNor, '"');
	appendStringInfoChar(midNor, ':');
	appendStringInfoChar(midNor, '"');
	Constant *cmidNor = createConstString(midNor->data);

	normalExprList = appendToTailOfList(normalExprList, midProvO4);
	normalExprList = appendToTailOfList(normalExprList, cmidNor);
	Operator *midNor1 = createOpExpr("||", normalExprList);

	List *normalExprList2 = NIL;
	StringInfo midNor2 = makeStringInfo();
	appendStringInfoString(midNor2, "rel:tupQ(");
	Constant *cmidNor2 = createConstString(midNor2->data);

	normalExprList2 = appendToTailOfList(normalExprList2, midNor1);
	normalExprList2 = appendToTailOfList(normalExprList2, cmidNor2);
	Operator *midNorO2 = createOpExpr("||", normalExprList2);

	List *normalAttrs3 = copyObject(normalAttrs);
	AttributeDef *firstNorAttrDef3 = (AttributeDef *) getHeadOfListP(
			normalAttrs3);
	int gb1Count = 0;
	AttributeReference *firstNorAttr3 = createAttributeReference(
			firstNorAttrDef3->attrName);
	firstNorAttr3->fromClauseItem = 0;
	firstNorAttr3->attrType = firstNorAttrDef3->dataType;
	firstNorAttr3->attrPosition = gb1Count;
    gb1Count++;

	List *normalExprList3 = NIL;
	normalExprList3 = appendToTailOfList(normalExprList3, midNorO2);
	normalExprList3 = appendToTailOfList(normalExprList3, firstNorAttr3);
	Operator *midNor3 = createOpExpr("||", normalExprList3);

	normalAttrs3 = removeFromHead(normalAttrs3);
	FOREACH(AttributeDef, a, normalAttrs3) {
		Operator *o2 = generateOp(&gb1Count, (Node *) midNor3, (Node *) cBar);
		Operator *o3 = generateOp(&gb1Count, (Node *) o2, (Node *) a);
		midNor3 = copyObject(o3);
	}

	List *normalExprList4 = NIL;
	StringInfo midNor4 = makeStringInfo();
	appendStringInfoChar(midNor4, ')');
	Constant *cmidNor4 = createConstString(midNor4->data);

	normalExprList4 = appendToTailOfList(normalExprList4, midNor3);
	normalExprList4 = appendToTailOfList(normalExprList4, cmidNor4);
	Operator *midNorO4 = createOpExpr("||", normalExprList4);

	List *normalExprList5 = NIL;
	StringInfo midNor5 = makeStringInfo();
	appendStringInfoChar(midNor5, '"');
	appendStringInfoChar(midNor5, '}');
	Constant *cmidNor5 = createConstString(midNor5->data);

	normalExprList5 = appendToTailOfList(normalExprList5, midNorO4);
	normalExprList5 = appendToTailOfList(normalExprList5, cmidNor5);
	Operator *midNorO5 = createOpExpr("||", normalExprList5);

	List *derivedProjExprs = singleton(midNorO5);
	List *derivedAttrNames = singleton(strdup(wdb));
	ProjectionOperator *derivedProj = createProjectionOp(derivedProjExprs, op,
			NIL, derivedAttrNames);
	op->parents = appendToTailOfList(op->parents, derivedProj);

	//Introduce aggregation operator
	AttributeDef *aggDef1 = (AttributeDef *) getHeadOfListP(
			derivedProj->op.schema->attrDefs);
	AttributeReference *aggRef1 = createAttributeReference(aggDef1->attrName);
	aggRef1->fromClauseItem = 0;
	aggRef1->attrType = aggDef1->dataType;
	aggRef1->attrPosition = 0;
	FunctionCall *aggFunc1 = createFunctionCall(strdup("AGG_STRAGG"),
			singleton(aggRef1));

	List *aggrs1 = singleton(aggFunc1);

	// create fake attribute names for aggregation output schema
	List *aggAttrNames1 = NIL;
	aggAttrNames1 = singleton("allWdb");

	AggregationOperator *ao1 = createAggregationOp(aggrs1, NIL,
			(QueryOperator *) derivedProj, NIL, aggAttrNames1);
	derivedProj->op.parents = appendToTailOfList(derivedProj->op.parents, ao1);
/*
	 Top projection based on allTup and allDerived, first introduce cross product
	List *joinNames = concatTwoLists(aggAttrNames, aggAttrNames1);

	//Introduce cross product
	JoinOperator *jo = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(ao, ao1), NULL,
			joinNames);
	OP_LCHILD(jo)->parents = singleton(jo);
	OP_RCHILD(jo)->parents = singleton(jo);

	//Introduce projection operation
	List *topProjExprs = NIL;
	List *topProjNames = NIL;
	int topCount = 0;

	//Op expr step 1
	StringInfo topPrefix1 = makeStringInfo();
	appendStringInfoChar(topPrefix1, '{');
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfoString(topPrefix1, "entities");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfoString(topPrefix1, " : { ");
	Constant *cTopPrefix1 = createConstString(topPrefix1->data);

	AttributeDef *topAttrDef1 = (AttributeDef *) getHeadOfListP(
			jo->op.schema->attrDefs);
	AttributeReference *topAttr1 = createAttributeReference(
			topAttrDef1->attrName);
	topAttr1->fromClauseItem = 0;
	topAttr1->attrType = topAttrDef1->dataType;
	topAttr1->attrPosition = topCount;
	topCount++;

	List *topExpr1 = NIL;
	topExpr1 = appendToTailOfList(topExpr1, cTopPrefix1);
	topExpr1 = appendToTailOfList(topExpr1, topAttr1);
	Operator *topO1 = createOpExpr("||", topExpr1);

	//Op expr step 2
	StringInfo topPrefix2 = makeStringInfo();
	appendStringInfoString(topPrefix2, "}, ");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "activity");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, " : { ");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoChar(topPrefix2, 'q');
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, ": {} }, ");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "wasGeneratedBy");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, " : {");
	Constant *cTopPrefix2 = createConstString(topPrefix2->data);

	List *topExpr2 = NIL;
	topExpr2 = appendToTailOfList(topExpr2, topO1);
	topExpr2 = appendToTailOfList(topExpr2, cTopPrefix2);
	Operator *topO2 = createOpExpr("||", topExpr2);

	//Op expr step 3
	AttributeDef *topAttrDef3 = (AttributeDef *) getTailOfListP(
			jo->op.schema->attrDefs);
	AttributeReference *topAttr3 = createAttributeReference(
			topAttrDef3->attrName);
	topAttr3->fromClauseItem = 0;
	topAttr3->attrType = topAttrDef3->dataType;
	topAttr3->attrPosition = topCount;
	topCount++;

	List *topExpr3 = NIL;
	topExpr3 = appendToTailOfList(topExpr3, topO2);
	topExpr3 = appendToTailOfList(topExpr3, topAttr3);
	Operator *topO3 = createOpExpr("||", topExpr3);

	//Op expr step 4
	StringInfo topSuffix4 = makeStringInfo();
	appendStringInfoString(topSuffix4, "}}");
	//appendStringInfoChar(topSuffix4, '}');
	Constant *cTopSuffix4 = createConstString(topSuffix4->data);

	List *topExpr4 = NIL;
	topExpr4 = appendToTailOfList(topExpr4, topO3);
	topExpr4 = appendToTailOfList(topExpr4, cTopSuffix4);
	Operator *topO4 = createOpExpr("||", topExpr4);

	topProjExprs = singleton(topO4);
	topProjNames = singleton("jExport");

	ProjectionOperator *pj = createProjectionOp(topProjExprs,
			(QueryOperator *) jo, NIL, topProjNames);
	jo->op.parents = appendToTailOfList(jo->op.parents, pj);*/


	//-- was generated by edges output tuple -> Q
	//generatedBy AS (SELECT '"_wgb' || X || ')" : { "prov:activity": "Q", "prov:entity": "' || '"tupQ(' || X || ')"' || '"}' AS wgb
	//FROM PQ)

	char *prov = "prov";
	char *activity = "activity";
	char *entity = "entity";
	char *usedEntity = "entity";
	char *tup_R = "rel:tup_R";
	char *tupQ = "rel:tupQ";
	char *_wgb = "_:wgb";
	char *_wub = "_:wub";
	int gbCount = 0;

	//OP expr 1
	StringInfo gbS1 = makeStringInfo();
	appendStringInfoChar(gbS1, '"');
	appendStringInfoString(gbS1, _wgb);
	Constant *cgbS1 = createConstString(gbS1->data);

	List *gbAttr1 = copyObject(normalAttrs);
	AttributeDef *firstGbAttrDef1= (AttributeDef *) getHeadOfListP(gbAttr1);
    Operator *gbO1 = generateOp(&gbCount, (Node *)cgbS1, (Node *)firstGbAttrDef1);

    gbAttr1 = removeFromHead(gbAttr1);
	FOREACH(AttributeDef, a, gbAttr1) {
		Operator *o2 = generateOp(&gbCount, (Node *) gbO1, (Node *) cBar);
		Operator *o3 = generateOp(&gbCount, (Node *) o2, (Node *) a);
		gbO1 = copyObject(o3);
	}

	//OP expr 2
	StringInfo gbS2 = makeStringInfo();
	appendStringInfoChar(gbS2, ')');
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, " : { ");
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, prov);
	appendStringInfoChar(gbS2, ':');
	appendStringInfoString(gbS2, activity);
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, ": ");
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, "rel:");
	appendStringInfoChar(gbS2, 'Q');
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, ", ");
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, prov);
	appendStringInfoChar(gbS2, ':');
	appendStringInfoString(gbS2, entity);
	appendStringInfoChar(gbS2, '"');
	appendStringInfoString(gbS2, ": ");
	appendStringInfoChar(gbS2, '"');
	Constant *cgbS2 = createConstString(gbS2->data);
    Operator *gbO2 = generateOp(&gbCount, (Node *)gbO1, (Node *)cgbS2);

    //OP expr 3
    gbCount = 0;
	StringInfo gbS3 = makeStringInfo();
	//appendStringInfoChar(gbS3, '"');
	appendStringInfoString(gbS3, tupQ);
	appendStringInfoChar(gbS3, '(');
	Constant *cgbS3 = createConstString(gbS3->data);
    Operator *gbO3 = generateOp(&gbCount, (Node *)gbO2, (Node *)cgbS3);

    //OP expr 4
	List *gbAttr2 = copyObject(normalAttrs);
	AttributeDef *firstGbAttrDef2= (AttributeDef *) getHeadOfListP(gbAttr2);
    Operator *gbO4 = generateOp(&gbCount, (Node *) gbO3, (Node *) firstGbAttrDef2);

    gbAttr2 = removeFromHead(gbAttr2);
	FOREACH(AttributeDef, a, gbAttr2) {
		Operator *o2 = generateOp(&gbCount, (Node *) gbO4, (Node *) cBar);
		Operator *o3 = generateOp(&gbCount, (Node *) o2, (Node *) a);
		gbO4 = copyObject(o3);
	}

	//OP expr 5
	StringInfo gbS5 = makeStringInfo();
	appendStringInfoChar(gbS5, ')');
	//appendStringInfoChar(gbS5, '"');
	Constant *cgbS5 = createConstString(gbS5->data);
    Operator *gbO5 = generateOp(&gbCount, (Node *)gbO4, (Node *)cgbS5);

    //OP expr 6
	StringInfo gbS6 = makeStringInfo();
	appendStringInfoChar(gbS6, '"');
	appendStringInfoChar(gbS6, '}');
	Constant *cgbS6 = createConstString(gbS6->data);
    Operator *gbO6 = generateOp(&gbCount, (Node *)gbO5, (Node *)cgbS6);

	ProjectionOperator *generatedByPj = createProjectionOp(singleton(gbO6), op, NIL,
			singleton("wgb"));
	op->parents = appendToTailOfList(op->parents, generatedByPj);

	//allGenerated AS (SELECT LISTAGG(wgb,',') WITHIN GROUP (ORDER BY wgb) AS allWgb FROM generatedBy)
	AttributeDef *aggALDef1 = (AttributeDef *) getHeadOfListP(
			generatedByPj->op.schema->attrDefs);
	AttributeReference *aggALRef1 = createAttributeReference(aggALDef1->attrName);
	aggALRef1->fromClauseItem = 0;
	aggALRef1->attrType = aggALDef1->dataType;
	aggALRef1->attrPosition = 0;
	FunctionCall *aggALFunc1 = createFunctionCall(strdup("AGG_STRAGG"),
			singleton(aggALRef1));

	List *aggrsAL1 = singleton(aggALFunc1);

	// create fake attribute names for aggregation output schema
	List *aggALAttrNames1 = NIL;
	aggALAttrNames1 = singleton("allWgb");

	AggregationOperator *aoAL1 = createAggregationOp(aggrsAL1, NIL,
			(QueryOperator *) generatedByPj, NIL, aggALAttrNames1);
	generatedByPj->op.parents = appendToTailOfList(generatedByPj->op.parents, aoAL1);

	//usedBy AS (SELECT '"_wub(' || PA || '|' || PB || ')" : { "prov:activity": "Q", "prov:usedEntity": "tup_R(' || PA || '|' || PB || ') "}' AS used
	//FROM PQ)
	//OP expr 1
	int ubCount = LIST_LENGTH(normalAttrs);
	StringInfo ubS1 = makeStringInfo();
	appendStringInfoChar(ubS1, '"');
	appendStringInfoString(ubS1, _wub);
	appendStringInfoChar(ubS1, '(');
	Constant *cubS1 = createConstString(ubS1->data);

	List *ubAttr1 = copyObject(provAttrs);
	AttributeDef *firstUbAttrDef1= (AttributeDef *) getHeadOfListP(ubAttr1);
    Operator *ubO1 = generateOp(&ubCount, (Node *)cubS1, (Node *)firstUbAttrDef1);

    ubAttr1 = removeFromHead(ubAttr1);
	FOREACH(AttributeDef, a, ubAttr1) {
		Operator *o2 = generateOp(&ubCount, (Node *) ubO1, (Node *) cBar);
		Operator *o3 = generateOp(&ubCount, (Node *) o2, (Node *) a);
		ubO1 = copyObject(o3);
	}

	//OP expr 2
	StringInfo ubS2 = makeStringInfo();
	appendStringInfoChar(ubS2, ')');
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, " : { ");
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, prov);
	appendStringInfoChar(ubS2, ':');
	appendStringInfoString(ubS2, activity);
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, ": ");
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, "rel:");
	appendStringInfoChar(ubS2, 'Q');
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, ", ");
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, prov);
	appendStringInfoChar(ubS2, ':');
	appendStringInfoString(ubS2, usedEntity);
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, ": ");
	appendStringInfoChar(ubS2, '"');
	appendStringInfoString(ubS2, tup_R);
	appendStringInfoChar(ubS2, '(');
	Constant *cubS2 = createConstString(ubS2->data);
    Operator *ubO2 = generateOp(&ubCount, (Node *)ubO1, (Node *)cubS2);

    //OP expr 3
    ubCount = LIST_LENGTH(normalAttrs);;
	List *ubAttr2 = copyObject(provAttrs);
	AttributeDef *firstUbAttrDef2= (AttributeDef *) getHeadOfListP(ubAttr2);
    Operator *ubO3 = generateOp(&ubCount, (Node *) ubO2, (Node *) firstUbAttrDef2);

    ubAttr2 = removeFromHead(ubAttr2);
	FOREACH(AttributeDef, a, ubAttr2) {
		Operator *o2 = generateOp(&ubCount, (Node *) ubO3, (Node *) cBar);
		Operator *o3 = generateOp(&ubCount, (Node *) o2, (Node *) a);
		ubO3 = copyObject(o3);
	}

	//OP expr 5
	StringInfo ubS4 = makeStringInfo();
	appendStringInfoString(ubS4, ")");
	appendStringInfoChar(ubS4, '"');
	appendStringInfoChar(ubS4, '}');
	Constant *cubS4 = createConstString(ubS4->data);
    Operator *ubO4 = generateOp(&ubCount, (Node *)ubO3, (Node *)cubS4);

	ProjectionOperator *usedByPj = createProjectionOp(singleton(ubO4), op, NIL,
			singleton("used"));
	op->parents = appendToTailOfList(op->parents, usedByPj);

    //allUsed AS (SELECT LISTAGG(used,',') WITHIN GROUP (ORDER BY used) AS allUsed FROM usedBy)
	AttributeDef *aggAUDef1 = (AttributeDef *) getHeadOfListP(
			usedByPj->op.schema->attrDefs);
	AttributeReference *aggAURef1 = createAttributeReference(aggAUDef1->attrName);
	aggAURef1->fromClauseItem = 0;
	aggAURef1->attrType = aggAUDef1->dataType;
	aggAURef1->attrPosition = 0;
	FunctionCall *aggAUFunc1 = createFunctionCall(strdup("AGG_STRAGG"),
			singleton(aggAURef1));

	List *aggrsAU1 = singleton(aggAUFunc1);

	// create fake attribute names for aggregation output schema
	List * aggAUAttrNames1 = singleton("allUsed");

	AggregationOperator *aoAU1 = createAggregationOp(aggrsAU1, NIL,
			(QueryOperator *) usedByPj, NIL, aggAUAttrNames1);
	usedByPj->op.parents = appendToTailOfList(usedByPj->op.parents, aoAU1);



	/* Top projection based on allTup and allDerived, first introduce cross product */
	//List *joinNames = concatTwoLists(aggAttrNames, aggAttrNames1);
	//LIST_MAKE(ao, ao1, aoAL1, aoAU1)

	//Agg 1
	List *joinNames1 = concatTwoLists(aggAttrNames, aggAttrNames1);

	//Introduce cross product
	JoinOperator *jo1 = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(ao, ao1), NULL,
			joinNames1);
	OP_LCHILD(jo1)->parents = singleton(jo1);
	OP_RCHILD(jo1)->parents = singleton(jo1);

	//Agg 2
	List *joinNames2 = concatTwoLists(joinNames1, aggALAttrNames1);

	//Introduce cross product
	JoinOperator *jo2 = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(jo1, aoAL1), NULL,
			joinNames2);
	OP_LCHILD(jo2)->parents = singleton(jo2);
	OP_RCHILD(jo2)->parents = singleton(jo2);

	//Agg 3
	List *joinNames3 = concatTwoLists(joinNames2, aggAUAttrNames1);

	//Introduce cross product
	JoinOperator *jo3 = createJoinOp(JOIN_CROSS, NULL, LIST_MAKE(jo2, aoAU1), NULL,
			joinNames3);
	OP_LCHILD(jo3)->parents = singleton(jo3);
	OP_RCHILD(jo3)->parents = singleton(jo3);

	//Introduce projection operation
	List *topProjExprs = NIL;
	List *topProjNames = NIL;
	List *joSchema = copyObject(jo3->op.schema->attrDefs);
	int topCount = 0;

	//Op expr step 1
	StringInfo topPrefix1 = makeStringInfo();
	appendStringInfoChar(topPrefix1, '{');
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfo(topPrefix1, "prefix");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfo(topPrefix1, ": {");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfo(topPrefix1, "rel");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfo(topPrefix1, ": ");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfo(topPrefix1, "http://example.org");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfo(topPrefix1, "}, ");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfoString(topPrefix1, "entity");
	appendStringInfoChar(topPrefix1, '"');
	appendStringInfoString(topPrefix1, " : { ");
	Constant *cTopPrefix1 = createConstString(topPrefix1->data);
	AttributeDef *topAttrDef1 = (AttributeDef *) getHeadOfListP(
			joSchema);
	Operator *topO1 = generateOp(&topCount, (Node *) cTopPrefix1, (Node *) topAttrDef1);

//	AttributeReference *topAttr1 = createAttributeReference(
//			topAttrDef1->attrName);
//	topAttr1->fromClauseItem = 0;
//	topAttr1->attrType = topAttrDef1->dataType;
//	topAttr1->attrPosition = topCount;
//	topCount++;
//
//	List *topExpr1 = NIL;
//	topExpr1 = appendToTailOfList(topExpr1, cTopPrefix1);
//	topExpr1 = appendToTailOfList(topExpr1, topAttr1);
//	Operator *topO1 = createOpExpr("||", topExpr1);

	//Op expr step 2
	StringInfo topPrefix2 = makeStringInfo();
	appendStringInfoString(topPrefix2, "}, ");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "activity");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, " : { ");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "rel:");
	appendStringInfoChar(topPrefix2, 'Q');
	appendStringInfoChar(topPrefix2, '"');
	//appendStringInfoString(topPrefix2, ": {} }, ");
	appendStringInfoString(topPrefix2, ": {");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "prov:type");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoChar(topPrefix2, ':');
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "query");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "} }, ");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, "wasDerivedFrom");
	appendStringInfoChar(topPrefix2, '"');
	appendStringInfoString(topPrefix2, " : {");
	Constant *cTopPrefix2 = createConstString(topPrefix2->data);
	Operator *topO2 = generateOp(&topCount, (Node *) topO1, (Node *) cTopPrefix2);
//	List *topExpr2 = NIL;
//	topExpr2 = appendToTailOfList(topExpr2, topO1);
//	topExpr2 = appendToTailOfList(topExpr2, cTopPrefix2);
//	Operator *topO2 = createOpExpr("||", topExpr2);

	//Op expr step 3
	joSchema = removeFromHead(joSchema);
	AttributeDef *topAttrDef3 = (AttributeDef *) getHeadOfListP(
			joSchema);
	Operator *topO3 = generateOp(&topCount, (Node *) topO2, (Node *) topAttrDef3);
//	AttributeReference *topAttr3 = createAttributeReference(
//			topAttrDef3->attrName);
//	topAttr3->fromClauseItem = 0;
//	topAttr3->attrType = topAttrDef3->dataType;
//	topAttr3->attrPosition = topCount;
//	topCount++;
//
//	List *topExpr3 = NIL;
//	topExpr3 = appendToTailOfList(topExpr3, topO2);
//	topExpr3 = appendToTailOfList(topExpr3, topAttr3);
//	Operator *topO3 = createOpExpr("||", topExpr3);

	//Op expr step 4
	StringInfo topSuffix4 = makeStringInfo();
	appendStringInfoString(topSuffix4, "}, ");
	appendStringInfoChar(topSuffix4, '"');
	appendStringInfoString(topSuffix4, "wasGeneratedBy");
	appendStringInfoChar(topSuffix4, '"');
	appendStringInfoString(topSuffix4, " : {");
	Constant *cTopSuffix4 = createConstString(topSuffix4->data);
	Operator *topO4 = generateOp(&topCount, (Node *) topO3, (Node *) cTopSuffix4);

	//Op expr step 5
	joSchema = removeFromHead(joSchema);
	AttributeDef *topAttrDef5 = (AttributeDef *) getHeadOfListP(
			joSchema);
	Operator *topO5 = generateOp(&topCount, (Node *) topO4, (Node *) topAttrDef5);

	//Op expr step 6
	StringInfo topSuffix6 = makeStringInfo();
	appendStringInfoString(topSuffix6, "}, ");
	appendStringInfoChar(topSuffix6, '"');
	appendStringInfoString(topSuffix6, "used");
	appendStringInfoChar(topSuffix6, '"');
	appendStringInfoString(topSuffix6, " : {");
	Constant *cTopSuffix6 = createConstString(topSuffix6->data);
	Operator *topO6 = generateOp(&topCount, (Node *) topO5, (Node *) cTopSuffix6);

	//Op expr step 7
	AttributeDef *topAttrDef7 = (AttributeDef *) getTailOfListP(
			joSchema);
	Operator *topO7 = generateOp(&topCount, (Node *) topO6, (Node *) topAttrDef7);

	//Op expr step 8
	StringInfo topSuffix8 = makeStringInfo();
	appendStringInfoString(topSuffix8, "}}");
	//appendStringInfoChar(topSuffix4, '}');
	Constant *cTopSuffix8 = createConstString(topSuffix8->data);
	Operator *topO8 = generateOp(&topCount, (Node *) topO7, (Node *) cTopSuffix8);
//	List *topExpr4 = NIL;
//	topExpr4 = appendToTailOfList(topExpr4, topO3);
//	topExpr4 = appendToTailOfList(topExpr4, cTopSuffix4);
//	Operator *topO4 = createOpExpr("||", topExpr4);

	topProjExprs = singleton(topO8);
	topProjNames = singleton("jExport");

	ProjectionOperator *pj = createProjectionOp(topProjExprs,
			(QueryOperator *) jo3, NIL, topProjNames);
	jo3->op.parents = appendToTailOfList(jo3->op.parents, pj);




	return (QueryOperator *) pj;
}

