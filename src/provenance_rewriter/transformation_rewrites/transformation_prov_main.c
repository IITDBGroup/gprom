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
#include "model/set/hashmap.h"

#define REL_TUPQ "\"rel:tupQ("
//#define REL_TUPR "\"rel:tup_R("
#define REL_TUP_ "\"rel:tup_"
#define PROV_TYPE_TUPLE ")\" :{\"prov:type\":\"tuple\"}"
#define ENTITY "entity"
#define ALL_ENTITIES "allEntities"
#define WDB "_:wdb"
#define PROV_USEDENTITY_REL_TUP_R ")\": { \"prov:usedEntity\": \"rel:tup_"
#define BRACKETS_DOUBLE_QUOTATION ")\""
#define PROV_GENERATEDENTITY ",\"prov:generatedEntity\":\""
#define PROV_ACTIVITY_REL_Q_PROV_ENTITY_REL_TUP_R ")\" : { \"prov:activity\": \"rel:Q\", \"prov:entity\": \"rel:tup_"
#define PREFIX_REL_URL_ENTITY "{\"prefix\": {\"rel\": \"http://example.org\"}, \"entity\" : { "
#define ACTIVIT_REL_Q_PROV_TYPE_QUERY_WASDERIVEDFROM "}, \"activity\" : { \"rel:Q\": {\"prov:type\":\"query\"} }, \"wasDerivedFrom\" : {"
#define WAS_GENREATED_BY "}, \"wasGeneratedBy\" : {"
#define USED "}, \"used\" : {"
#define PROV_ACTIVITY_RELQ_PROV_ENTITY ")\" : { \"prov:activity\": \"rel:Q\", \"prov:entity\": \""
#define tupQ "rel:tupQ"
#define _wgb "_:wgb"
#define _wub "_:wub_"
//	char *tupQ = "rel:tupQ";
//	char *_wgb = "_:wgb";
//	char *_wub = "_:wub_";

Operator *
generateOpP(int *count, Node *attr1, Node *attr2)
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

Operator *
generateOp(Node *attr1, Node *attr2, HashMap *namePosMap)
{
    List *argList = NIL;
    Operator *o = NULL;

    if(isA(attr2, AttributeDef))
    {
    	AttributeDef *attrDef= (AttributeDef *) attr2;
    	AttributeReference *attrRef = createAttributeReference(attrDef->attrName);
    	attrRef->fromClauseItem = 0;
    	attrRef->attrType = attrDef->dataType;
    	attrRef->attrPosition = INT_VALUE(getMapString(namePosMap, attrRef->name));
    	argList = appendToTailOfList(argList, attr1);
    	argList = appendToTailOfList(argList, attrRef);

    }

    else if(isA(attr1, AttributeDef))
    {
    	AttributeDef *attrDef= (AttributeDef *) attr1;
    	AttributeReference *attrRef = createAttributeReference(attrDef->attrName);
    	attrRef->fromClauseItem = 0;
    	attrRef->attrType = attrDef->dataType;
    	attrRef->attrPosition = INT_VALUE(getMapString(namePosMap, attrRef->name));
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

void
findTableAccessOperator(List **drOp, QueryOperator *root)
{
	if(isA(root, TableAccessOperator) && isA(getHeadOfListP(root->parents), ProjectionOperator))
		*drOp = appendToTailOfList(*drOp, (DuplicateRemoval *)root);

	FOREACH(QueryOperator, op, root->inputs)
	      findTableAccessOperator(drOp, op);

}

QueryOperator *
rewriteTransformationProvenance(QueryOperator *op)
{
	HashMap *namePosMap = NEW_MAP(Constant,Constant);
	int pos = 0;
    FOREACH(AttributeDef,ad,op->schema->attrDefs)
	{
    	MAP_ADD_STRING_KEY(namePosMap, ad->attrName, createConstInt(pos));
    	DEBUG_LOG("pos: %d", INT_VALUE(getMapString(namePosMap, ad->attrName)));
        pos++;
	}
//	int ss = INT_VALUE(getMapString(namePosMap, "A"));
//	DEBUG_LOG("ss: pos = %d", ss);

	//traverse the tree and store table access operator into list taOp
	List *taOp = NULL;
    findTableAccessOperator(&taOp, op);
    FOREACH(TableAccessOperator, op1, taOp)
         DEBUG_LOG("Table Access Operator name: %s", op1->tableName);
    DEBUG_LOG("taOp List length: %d", LIST_LENGTH(taOp));
	List *normalAttrs = getNormalAttrs(op);

	//First projection: normal attributes
	StringInfo prefixQ = makeStringInfo();
	//appendStringInfoString(prefixQ, "\"rel:tupQ(");
	appendStringInfoString(prefixQ,strdup(REL_TUPQ));
	Constant *cPrefixQ = createConstString(prefixQ->data);

	StringInfo suffix = makeStringInfo();
	//appendStringInfoString(suffix, ")\" :{\"prov:type\":\"tuple\"}");
	appendStringInfoString(suffix, strdup(PROV_TYPE_TUPLE));
	Constant *cSuffix = createConstString(suffix->data);

	Constant *cBar = createConstString(" | ");

	List *reverseNormalAttrs = copyObject(normalAttrs);
	reverseList(reverseNormalAttrs);

	AttributeDef *firstNormalAttrDef = (AttributeDef *) getHeadOfListP(
			reverseNormalAttrs);
	Operator *NormalO1 = generateOp((Node *)firstNormalAttrDef, (Node *)cSuffix , namePosMap);

	reverseNormalAttrs = removeFromHead(reverseNormalAttrs);
	FOREACH(AttributeDef, a, reverseNormalAttrs) {
		Operator *o2 = generateOp((Node *)cBar, (Node *)NormalO1 , namePosMap);
		Operator *o3 = generateOp((Node *)a, (Node *)o2 , namePosMap);
		NormalO1 = copyObject(o3);
	}

	Operator *NormalO4 = generateOp((Node *)cPrefixQ, (Node *)NormalO1, namePosMap );
	DEBUG_LOG("new operator expression %s", nodeToString(NormalO4));

	List *NormalProjExprs = singleton(NormalO4);
	List *attrNames = singleton("entity");
	//List *attrNames = singleton(strdup(ENTITY));

	//Introduce first normal attr projection operator
	ProjectionOperator *proj = createProjectionOp(NormalProjExprs, op, NIL,
			attrNames);
	op->parents = appendToTailOfList(op->parents, proj);
	//DEBUG_LOG("new proj %s", nodeToString(proj));

	//Introduce duplicateRemoval operator for the first proj
	DuplicateRemoval *dr = createDuplicateRemovalOp(NIL, (QueryOperator *) proj,
			NIL, attrNames);
	proj->op.parents = appendToTailOfList(proj->op.parents, dr);
    //End of introducint first normal attr proj op

/***BEGIN Loop**************************************************************************/
    List *drOp = NIL;
   	List *provAttrNames = singleton(ENTITY);
   	List *provAttrs = NIL; //need to remove
    FOREACH(TableAccessOperator, ta, taOp)
    {
    	StringInfo prefixTable = makeStringInfo();
    	//appendStringInfoString(prefixR, "\"rel:tup_R(");
    	appendStringInfoString(prefixTable,strdup(REL_TUP_));
    	appendStringInfoString(prefixTable, strdup(ta->tableName));
    	appendStringInfoString(prefixTable, "(");
    	Constant *cPrefixR = createConstString(prefixTable->data);

    	//Second projection: provenance attributes
    	//List *provAttrs = getProvenanceAttrDefs(op);
    	//List *provAttrs = NIL;
    	provAttrs = NIL;
    	if(isA(getHeadOfListP(ta->op.parents), ProjectionOperator))
    	{
    		ProjectionOperator *provPj = (ProjectionOperator *) getHeadOfListP(ta->op.parents);
    		provAttrs = getProvenanceAttrDefs((QueryOperator *)provPj);
    		DEBUG_LOG("Length %d", LIST_LENGTH(provAttrs));
    		FOREACH(AttributeDef, ad, provAttrs)
    			DEBUG_LOG("attr name %s", ad->attrName);
    	}
    	List *reverseProvAttrs = copyObject(provAttrs);
    	reverseList(reverseProvAttrs);

    	AttributeDef *firstAttrDef = (AttributeDef *) getHeadOfListP(
    			reverseProvAttrs);
    	Operator *o1 = generateOp((Node *)firstAttrDef, (Node *)cSuffix , namePosMap);
    	reverseProvAttrs = removeFromHead(reverseProvAttrs);
    	FOREACH(AttributeDef, a, reverseProvAttrs) {
    		Operator *o2 = generateOp((Node *)cBar, (Node *)o1 , namePosMap);
    		Operator *o3 = generateOp((Node *)a, (Node *)o2 , namePosMap);
    		o1 = copyObject(o3);
    	}

    	Operator *o4 = generateOp((Node *)cPrefixR, (Node *)o1, namePosMap);
    	//DEBUG_LOG("new prov operator expression %s", nodeToString(o4));

    	List *provProjExprs = singleton(o4);
    	DEBUG_LOG("new prov proj exprs operator expression %s", nodeToString(o4));
    	//List *provAttrNames = singleton("entity");
    	//List *provAttrNames = singleton(ENTITY);

    	//Introduce second prov projection operator
    	ProjectionOperator *provProj = createProjectionOp(provProjExprs, op, NIL,
    			provAttrNames);
    	op->parents = appendToTailOfList(op->parents, provProj);


    	//Introduce duplicateRemoval operator
    	DuplicateRemoval *provDr = createDuplicateRemovalOp(NIL,
    			(QueryOperator *) provProj, NIL, provAttrNames);
    	provProj->op.parents = appendToTailOfList(provProj->op.parents, provDr);
    	//End of introducing second prov attr proj op

    	drOp = appendToTailOfList(drOp, provDr);
    }

	/***END Loop**************************************************************************/

    //Introduce union operators for provs
    List *copyDrOp = copyObject(drOp);
    List *unionNames = concatTwoLists(attrNames, provAttrNames);

    SetOperator *so1 = NULL;
    if(LIST_LENGTH(copyDrOp) > 1)
    {
    	DuplicateRemoval *dr2 = (DuplicateRemoval *)getTailOfListP(copyDrOp);
    	copyDrOp = removeFromTail(copyDrOp);
    	DuplicateRemoval *dr1 = (DuplicateRemoval *)getTailOfListP(copyDrOp);
    	so1 = createSetOperator(SETOP_UNION, LIST_MAKE(dr1, dr2), NIL,
    			unionNames);
    	OP_LCHILD(so1)->parents = singleton(so1);
    	OP_RCHILD(so1)->parents = singleton(so1);

        while(LIST_LENGTH(copyDrOp) != 1)
        {
        	copyDrOp = removeFromTail(copyDrOp);
        	DuplicateRemoval *dr3 = (DuplicateRemoval *)getTailOfListP(copyDrOp);
        	SetOperator *so2 = createSetOperator(SETOP_UNION, LIST_MAKE(dr3, so1), NIL,
        			unionNames);
        	OP_LCHILD(so2)->parents = singleton(so2);
        	OP_RCHILD(so2)->parents = singleton(so2);
        	so1 = so2;
        }
    }

	//Introduce union operator
    SetOperator *so = NULL;
    if(LIST_LENGTH(drOp) == 1)
    	so = createSetOperator(SETOP_UNION, LIST_MAKE(dr, getHeadOfListP(drOp)), NIL,
    			unionNames);
    else
    	so = createSetOperator(SETOP_UNION, LIST_MAKE(dr, so1), NIL,
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
	//aggAttrNames = singleton("allEntities");
	aggAttrNames = singleton(strdup(ALL_ENTITIES));

	AggregationOperator *ao = createAggregationOp(aggrs, NIL,
			(QueryOperator *) so, NIL, aggAttrNames);
	so->op.parents = appendToTailOfList(so->op.parents, ao);

	/* Second Part: derivedBy and allDerived */

	//Introduce projection operator
	//char *wdb = "_:wdb";

	/*Loop BEGIN************************************************************************/
	List *dbProjList = NIL;
	ProjectionOperator *derivedProj = NULL;
	FOREACH(TableAccessOperator, ta, taOp)
	{
		StringInfo prefixDB = makeStringInfo();
		//appendStringInfoChar(prefixDB, '"');
		//appendStringInfoString(prefixDB, strdup(WDB));
		appendStringInfoString(prefixDB, "\"_:wdb_");
		appendStringInfoString(prefixDB, strdup(ta->tableName));
		Constant *cPrefixDB = createConstString(prefixDB->data);

    	provAttrs = NIL;
    	if(isA(getHeadOfListP(ta->op.parents), ProjectionOperator))
    	{
    		ProjectionOperator *provPj = (ProjectionOperator *) getHeadOfListP(ta->op.parents);
    		provAttrs = getProvenanceAttrDefs((QueryOperator *)provPj);
    	}

		//int lenNormal = LIST_LENGTH(normalAttrs);
		List *wholeAttrs = concatTwoLists(copyObject(normalAttrs),
				copyObject(provAttrs));

		AttributeDef *firstNormalAttrDefDB = (AttributeDef *) getHeadOfListP(
				wholeAttrs);
		Operator *NormalODB1 = generateOp((Node *) cPrefixDB, (Node *) firstNormalAttrDefDB,namePosMap);

		wholeAttrs = removeFromHead(wholeAttrs);
		FOREACH(AttributeDef, a, wholeAttrs) {
			Operator *o2 = generateOp((Node *) NormalODB1, (Node *) cBar, namePosMap);
			Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
			NormalODB1 = copyObject(o3);
		}

		StringInfo midDB = makeStringInfo();
		//appendStringInfoString(midDB, ")\": { \"prov:usedEntity\": \"rel:tup_R(");
		appendStringInfoString(midDB, strdup(PROV_USEDENTITY_REL_TUP_R));
		appendStringInfoString(midDB, strdup(ta->tableName));
		appendStringInfoString(midDB, "(");
		Constant *cmidDB = createConstString(midDB->data);
		Operator *DBo4 = generateOp((Node *) NormalODB1, (Node *) cmidDB, namePosMap);
		//DEBUG_LOG("new prov operator expression %s", nodeToString(DBo4));

		List *provAttrs1 = copyObject(provAttrs);
		AttributeDef *firstProvAttrDef1 = (AttributeDef *) getHeadOfListP(
				provAttrs1);
		Operator *provO1 = generateOp((Node *) DBo4, (Node *) firstProvAttrDef1, namePosMap);

		provAttrs1 = removeFromHead(provAttrs1);
		FOREACH(AttributeDef, a, provAttrs1) {
			Operator *o2 = generateOp((Node *) provO1, (Node *) cBar, namePosMap);
			Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
			provO1 = copyObject(o3);
		}

		StringInfo midProv = makeStringInfo();
		//appendStringInfoString(midProv,")\"");
		appendStringInfoString(midProv, strdup(BRACKETS_DOUBLE_QUOTATION));
		Constant *cmidProv = createConstString(midProv->data);
		Operator *midProvO4 = generateOp((Node *) provO1, (Node *) cmidProv, namePosMap);
		//DEBUG_LOG("new prov operator expression %s", nodeToString(midProvO4));

		StringInfo midNor = makeStringInfo();
		//appendStringInfoString(midNor, ",\"prov:generatedEntity\":\"");
		appendStringInfoString(midNor, strdup(PROV_GENERATEDENTITY));
		Constant *cmidNor = createConstString(midNor->data);
		Operator *midNor1 = generateOp((Node *) midProvO4, (Node *) cmidNor, namePosMap);

		StringInfo midNor2 = makeStringInfo();
		appendStringInfoString(midNor2, "rel:tupQ(");
		Constant *cmidNor2 = createConstString(midNor2->data);
		Operator *midNorO2 = generateOp((Node *) midNor1, (Node *) cmidNor2, namePosMap);

		List *normalAttrs3 = copyObject(normalAttrs);
		AttributeDef *firstNorAttrDef3 = (AttributeDef *) getHeadOfListP(
				normalAttrs3);

		Operator *midNor3 = generateOp((Node *) midNorO2, (Node *) firstNorAttrDef3, namePosMap);

		normalAttrs3 = removeFromHead(normalAttrs3);
		FOREACH(AttributeDef, a, normalAttrs3) {
			Operator *o2 = generateOp((Node *) midNor3, (Node *) cBar, namePosMap);
			Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
			midNor3 = copyObject(o3);
		}

		List *normalExprList4 = NIL;
		StringInfo midNor4 = makeStringInfo();
		appendStringInfoString(midNor4, ")\"}");
		Constant *cmidNor4 = createConstString(midNor4->data);

		normalExprList4 = appendToTailOfList(normalExprList4, midNor3);
		normalExprList4 = appendToTailOfList(normalExprList4, cmidNor4);
		Operator *midNorO5 = createOpExpr("||", normalExprList4);

		List *derivedProjExprs = singleton(midNorO5);
		List *derivedAttrNames = singleton(strdup(WDB));
		derivedProj = createProjectionOp(derivedProjExprs, op,
				NIL, derivedAttrNames);
		op->parents = appendToTailOfList(op->parents, derivedProj);

		dbProjList = appendToTailOfList(dbProjList, derivedProj);

	}
	/*Loop END************************************************************************/

	List *dbUnionNames = concatTwoLists(singleton(strdup(WDB)), singleton(strdup(WDB)));
	List *copyDbProjList = copyObject(dbProjList);
	SetOperator *u1 = NULL;
	if(LIST_LENGTH(copyDbProjList) > 1)
	{
    	ProjectionOperator *pj2 = (ProjectionOperator *)getTailOfListP(copyDbProjList);
    	copyDbProjList = removeFromTail(copyDbProjList);
    	ProjectionOperator *pj1 = (ProjectionOperator *)getTailOfListP(copyDbProjList);
    	u1 = createSetOperator(SETOP_UNION, LIST_MAKE(pj1, pj2), NIL,
    			dbUnionNames);
    	OP_LCHILD(u1)->parents = singleton(u1);
    	OP_RCHILD(u1)->parents = singleton(u1);

        while(LIST_LENGTH(copyDbProjList) != 1)
        {
        	copyDbProjList = removeFromTail(copyDbProjList);
        	ProjectionOperator *pj3 = (ProjectionOperator *)getTailOfListP(copyDbProjList);
        	SetOperator *u2 = createSetOperator(SETOP_UNION, LIST_MAKE(pj3, u1), NIL,
        			dbUnionNames);
        	OP_LCHILD(u2)->parents = singleton(u2);
        	OP_RCHILD(u2)->parents = singleton(u2);
        	u1 = u2;
        }
	}

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

	AggregationOperator *ao1 = NULL;
	if(LIST_LENGTH(dbProjList) == 1)
	{
		ao1 = createAggregationOp(aggrs1, NIL,
				(QueryOperator *) derivedProj, NIL, aggAttrNames1);
		derivedProj->op.parents = appendToTailOfList(derivedProj->op.parents, ao1);
	}
	else
	{
		ao1 = createAggregationOp(aggrs1, NIL,
				(QueryOperator *) u1, NIL, aggAttrNames1);
		u1->op.parents = appendToTailOfList(u1->op.parents, ao1);
	}

	//-- was generated by edges output tuple -> Q
	//generatedBy AS (SELECT '"_wgb' || X || ')" : { "prov:activity": "Q", "prov:entity": "' || '"tupQ(' || X || ')"' || '"}' AS wgb
	//FROM PQ)

	//OP expr 1
	StringInfo gbS1 = makeStringInfo();
	appendStringInfoChar(gbS1, '"');
	appendStringInfoString(gbS1, _wgb);
	Constant *cgbS1 = createConstString(gbS1->data);

	List *gbAttr1 = copyObject(normalAttrs);
	AttributeDef *firstGbAttrDef1= (AttributeDef *) getHeadOfListP(gbAttr1);
    Operator *gbO1 = generateOp((Node *)cgbS1, (Node *)firstGbAttrDef1, namePosMap);

    gbAttr1 = removeFromHead(gbAttr1);
	FOREACH(AttributeDef, a, gbAttr1) {
		Operator *o2 = generateOp((Node *) gbO1, (Node *) cBar, namePosMap);
		Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
		gbO1 = copyObject(o3);
	}

	//OP expr 2
	StringInfo gbS2 = makeStringInfo();
	//appendStringInfoString(gbS2, ")\" : { \"prov:activity\": \"rel:Q\", \"prov:entity\": \"");
	appendStringInfoString(gbS2, strdup(PROV_ACTIVITY_RELQ_PROV_ENTITY));
	Constant *cgbS2 = createConstString(gbS2->data);
    Operator *gbO2 = generateOp((Node *)gbO1, (Node *)cgbS2, namePosMap);

    //OP expr 3
	StringInfo gbS3 = makeStringInfo();
	//appendStringInfoChar(gbS3, '"');
	appendStringInfoString(gbS3, tupQ);
	appendStringInfoChar(gbS3, '(');
	Constant *cgbS3 = createConstString(gbS3->data);
    Operator *gbO3 = generateOp((Node *)gbO2, (Node *)cgbS3, namePosMap);

    //OP expr 4
	List *gbAttr2 = copyObject(normalAttrs);
	AttributeDef *firstGbAttrDef2= (AttributeDef *) getHeadOfListP(gbAttr2);
    Operator *gbO4 = generateOp((Node *) gbO3, (Node *) firstGbAttrDef2, namePosMap);

    gbAttr2 = removeFromHead(gbAttr2);
	FOREACH(AttributeDef, a, gbAttr2) {
		Operator *o2 = generateOp((Node *) gbO4, (Node *) cBar, namePosMap);
		Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
		gbO4 = copyObject(o3);
	}

	//OP expr 5
	StringInfo gbS5 = makeStringInfo();
	appendStringInfoChar(gbS5, ')');
	Constant *cgbS5 = createConstString(gbS5->data);
    Operator *gbO5 = generateOp((Node *)gbO4, (Node *)cgbS5, namePosMap);

    //OP expr 6
	StringInfo gbS6 = makeStringInfo();
	appendStringInfoString(gbS6, "\"}");
	Constant *cgbS6 = createConstString(gbS6->data);
    Operator *gbO6 = generateOp((Node *)gbO5, (Node *)cgbS6, namePosMap);

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
	/*Loop BEGIN***************************************************************/
	ProjectionOperator *usedByPj = NULL;

	List *ubProjList = NIL;
	FOREACH(TableAccessOperator, ta, taOp)
	{
    	provAttrs = NIL;
    	if(isA(getHeadOfListP(ta->op.parents), ProjectionOperator))
    	{
    		ProjectionOperator *provPj = (ProjectionOperator *) getHeadOfListP(ta->op.parents);
    		provAttrs = getProvenanceAttrDefs((QueryOperator *)provPj);
    	}

		StringInfo ubS1 = makeStringInfo();
		appendStringInfoChar(ubS1, '"');
		appendStringInfoString(ubS1, _wub);
		appendStringInfoString(ubS1, strdup(ta->tableName));
		appendStringInfoChar(ubS1, '(');
		Constant *cubS1 = createConstString(ubS1->data);

		List *ubAttr1 = copyObject(provAttrs);
		AttributeDef *firstUbAttrDef1= (AttributeDef *) getHeadOfListP(ubAttr1);
		Operator *ubO1 = generateOp((Node *)cubS1, (Node *)firstUbAttrDef1, namePosMap);

		ubAttr1 = removeFromHead(ubAttr1);
		FOREACH(AttributeDef, a, ubAttr1) {
			Operator *o2 = generateOp((Node *) ubO1, (Node *) cBar, namePosMap);
			Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
			ubO1 = copyObject(o3);
		}

		//OP expr 2
		StringInfo ubS2 = makeStringInfo();
		//appendStringInfoString(ubS2, ")\" : { \"prov:activity\": \"rel:Q\", \"prov:entity\": \"rel:tup_R(");
		appendStringInfoString(ubS2, strdup(PROV_ACTIVITY_REL_Q_PROV_ENTITY_REL_TUP_R));
		appendStringInfoString(ubS2, strdup(ta->tableName));
		appendStringInfoString(ubS2, "(");
		Constant *cubS2 = createConstString(ubS2->data);
		Operator *ubO2 = generateOp((Node *)ubO1, (Node *)cubS2, namePosMap);

		//OP expr 3
		//ubCount = LIST_LENGTH(normalAttrs);
		//ubCount = ubCount1;
		List *ubAttr2 = copyObject(provAttrs);
		AttributeDef *firstUbAttrDef2= (AttributeDef *) getHeadOfListP(ubAttr2);
		Operator *ubO3 = generateOp((Node *) ubO2, (Node *) firstUbAttrDef2, namePosMap);

		ubAttr2 = removeFromHead(ubAttr2);
		FOREACH(AttributeDef, a, ubAttr2) {
			Operator *o2 = generateOp((Node *) ubO3, (Node *) cBar, namePosMap);
			Operator *o3 = generateOp((Node *) o2, (Node *) a, namePosMap);
			ubO3 = copyObject(o3);
		}

		//OP expr 5
		StringInfo ubS4 = makeStringInfo();
		appendStringInfoString(ubS4, ")\"}");
		Constant *cubS4 = createConstString(ubS4->data);
		Operator *ubO4 = generateOp((Node *)ubO3, (Node *)cubS4, namePosMap);

		usedByPj = createProjectionOp(singleton(ubO4), op, NIL,
				singleton("used"));
		op->parents = appendToTailOfList(op->parents, usedByPj);

		ubProjList = appendToTailOfList(ubProjList, usedByPj);
	}
	/*Loop END***************************************************************/

	List *ubUnionNames = concatTwoLists(singleton("used"), singleton("used"));
	List *copyUbProjList = copyObject(ubProjList);
	SetOperator *u2 = NULL;
	if(LIST_LENGTH(copyUbProjList) > 1)
	{
    	ProjectionOperator *pj2 = (ProjectionOperator *)getTailOfListP(copyUbProjList);
    	copyUbProjList = removeFromTail(copyUbProjList);
    	ProjectionOperator *pj1 = (ProjectionOperator *)getTailOfListP(copyUbProjList);
    	u2 = createSetOperator(SETOP_UNION, LIST_MAKE(pj1, pj2), NIL,
    			ubUnionNames);
    	OP_LCHILD(u2)->parents = singleton(u2);
    	OP_RCHILD(u2)->parents = singleton(u2);

        while(LIST_LENGTH(copyUbProjList) != 1)
        {
        	copyUbProjList = removeFromTail(copyUbProjList);
        	ProjectionOperator *pj3 = (ProjectionOperator *)getTailOfListP(copyUbProjList);
        	SetOperator *u3 = createSetOperator(SETOP_UNION, LIST_MAKE(pj3, u2), NIL,
        			dbUnionNames);
        	OP_LCHILD(u3)->parents = singleton(u3);
        	OP_RCHILD(u3)->parents = singleton(u3);
        	u2 = u3;
        }
	}

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

	AggregationOperator *aoAU1 = NULL;
	if(LIST_LENGTH(ubProjList) == 1)
	{
		aoAU1 = createAggregationOp(aggrsAU1, NIL,
				(QueryOperator *) usedByPj, NIL, aggAUAttrNames1);
		usedByPj->op.parents = appendToTailOfList(usedByPj->op.parents, aoAU1);
	}
	else
	{
		aoAU1 = createAggregationOp(aggrsAU1, NIL,
				(QueryOperator *) u2, NIL, aggAUAttrNames1);
		u2->op.parents = appendToTailOfList(u2->op.parents, aoAU1);
	}

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
	//appendStringInfoString(topPrefix1, "{\"prefix\": {\"rel\": \"http://example.org\"}, \"entity\" : { ");
	appendStringInfoString(topPrefix1, strdup(PREFIX_REL_URL_ENTITY));
	Constant *cTopPrefix1 = createConstString(topPrefix1->data);
	AttributeDef *topAttrDef1 = (AttributeDef *) getHeadOfListP(joSchema);
	Operator *topO1 = generateOpP(&topCount, (Node *) cTopPrefix1, (Node *) topAttrDef1);

	//Op expr step 2
	StringInfo topPrefix2 = makeStringInfo();
	//appendStringInfoString(topPrefix2, "}, \"activity\" : { \"rel:Q\": {\"prov:type\":\"query\"} }, \"wasDerivedFrom\" : {");
	appendStringInfoString(topPrefix2, strdup(ACTIVIT_REL_Q_PROV_TYPE_QUERY_WASDERIVEDFROM));
	Constant *cTopPrefix2 = createConstString(topPrefix2->data);
	Operator *topO2 = generateOpP(&topCount, (Node *) topO1, (Node *) cTopPrefix2);

	//Op expr step 3
	joSchema = removeFromHead(joSchema);
	AttributeDef *topAttrDef3 = (AttributeDef *) getHeadOfListP(
			joSchema);
	Operator *topO3 = generateOpP(&topCount, (Node *) topO2, (Node *) topAttrDef3);

	//Op expr step 4
	StringInfo topSuffix4 = makeStringInfo();
	//appendStringInfoString(topSuffix4, "}, \"wasGeneratedBy\" : {");
	appendStringInfoString(topSuffix4, strdup(WAS_GENREATED_BY));
	Constant *cTopSuffix4 = createConstString(topSuffix4->data);
	Operator *topO4 = generateOpP(&topCount, (Node *) topO3, (Node *) cTopSuffix4);

	//Op expr step 5
	joSchema = removeFromHead(joSchema);
	AttributeDef *topAttrDef5 = (AttributeDef *) getHeadOfListP(
			joSchema);
	Operator *topO5 = generateOpP(&topCount, (Node *) topO4, (Node *) topAttrDef5);

	//Op expr step 6
	StringInfo topSuffix6 = makeStringInfo();
	//appendStringInfoString(topSuffix6, "}, \"used\" : {");
	appendStringInfoString(topSuffix6, strdup(USED));
	Constant *cTopSuffix6 = createConstString(topSuffix6->data);
	Operator *topO6 = generateOpP(&topCount, (Node *) topO5, (Node *) cTopSuffix6);

	//Op expr step 7
	AttributeDef *topAttrDef7 = (AttributeDef *) getTailOfListP(
			joSchema);
	Operator *topO7 = generateOpP(&topCount, (Node *) topO6, (Node *) topAttrDef7);

	//Op expr step 8
	StringInfo topSuffix8 = makeStringInfo();
	appendStringInfoString(topSuffix8, "}}");
	Constant *cTopSuffix8 = createConstString(topSuffix8->data);
	Operator *topO8 = generateOpP(&topCount, (Node *) topO7, (Node *) cTopSuffix8);

	topProjExprs = singleton(topO8);
	topProjNames = singleton("jExport");

	ProjectionOperator *pj = createProjectionOp(topProjExprs,
			(QueryOperator *) jo3, NIL, topProjNames);
	jo3->op.parents = appendToTailOfList(jo3->op.parents, pj);

	return (QueryOperator *) pj;
}

