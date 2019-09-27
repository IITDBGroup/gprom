/*-----------------------------------------------------------------------------
 *
 * coarse_grained_rewrite.c
 *
 *
 *      AUTHOR: xing_niu
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "configuration/option.h"
#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "metadata_lookup/metadata_lookup.h"

static void loopMarkNumOfTableAccess(QueryOperator *op, HashMap *map);

QueryOperator *
addTopAggForCoarse (QueryOperator *op)
{
    List *provAttr = getOpProvenanceAttrNames(op);
    List *projExpr = NIL;
    int cnt = 0;
    List *provPosList = NIL;
//    List *opParents = op->parents;

    FOREACH(char, c, provAttr)
    {
        provPosList = appendToTailOfListInt(provPosList, cnt);
        AttributeReference *a = createAttrsRefByName(op, c);
        FunctionCall *f = NULL;
        if(getBackend() == BACKEND_ORACLE)
        		f = createFunctionCall ("BITORAGG", singleton(a));
        else if(getBackend() == BACKEND_POSTGRES)
        		f = createFunctionCall ("bit_or", singleton(a));
        //FunctionCall *f = createFunctionCall ("BITORAGG", singleton(a));
        projExpr = appendToTailOfList(projExpr, f);
        cnt ++;
    }

    ProjectionOperator *newOp = createProjectionOp(projExpr, op, NIL, provAttr);
    newOp->op.provAttrs = provPosList;

    switchSubtrees((QueryOperator *) op, (QueryOperator *) newOp);
    op->parents = singleton(newOp);
//    newOp->op.parents = opParents;

    return (QueryOperator *) newOp;
}


void
markTableAccessAndAggregation (QueryOperator *op, Node *psPara)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
               DEBUG_LOG("mark tableAccessOperator.");

               /* mark coarsePara info */
               SET_STRING_PROP(o, PROP_COARSE_GRAINED_TABLEACCESS_MARK, psPara);

           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }

           markTableAccessAndAggregation(o, psPara);
      }
}

void
autoMarkTableAccessAndAggregation (QueryOperator *op, Node *psPara)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
        	   	  // TableAccessOperator *tableOp = (TableAccessOperator *) o;
               DEBUG_LOG("mark tableAccessOperator.");

               /* mark coarsePara info */
               SET_STRING_PROP(o, PROP_COARSE_GRAINED_TABLEACCESS_MARK, psPara);
               //AUTO_HISTOGRAM_TABLEACCESS_MARK
           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }

           autoMarkTableAccessAndAggregation(o, psPara);
      }
}


void
markUseTableAccessAndAggregation (QueryOperator *op, Node *psPara)
{
      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
               DEBUG_LOG("mark use tableAccessOperator.");
               SET_STRING_PROP(o, USE_PROP_COARSE_GRAINED_TABLEACCESS_MARK, psPara);
           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               SET_BOOL_STRING_PROP(o, USE_PROP_COARSE_GRAINED_AGGREGATION_MARK);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }
           markUseTableAccessAndAggregation(o,psPara);
      }
}


void
markAutoUseTableAccess (QueryOperator *op, HashMap *psMap)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
        	   	   DEBUG_NODE_BEATIFY_LOG("QueryOperator :", (Node *) o);
               SET_STRING_PROP(o, AUTO_USE_PROV_COARSE_GRAINED_TABLEACCESS_MARK, psMap);
           }

           markAutoUseTableAccess(o, psMap);
      }
}



void
markNumOfTableAccess(QueryOperator *op)
{
	//R -> 1, S-> 1....
	HashMap *map = NEW_MAP(Constant,Constant);
	loopMarkNumOfTableAccess(op, map);
}

static void
loopMarkNumOfTableAccess(QueryOperator *op, HashMap *map)
{
	FOREACH(QueryOperator, o, op->inputs)
	{
		if(isA(o, TableAccessOperator))
		{
			TableAccessOperator *tOp = (TableAccessOperator *) o;
			Constant *ctableName = createConstString(strdup(tOp->tableName));
			int num = 0;
			if(MAP_HAS_STRING_KEY(map, tOp->tableName))
			{
				num = INT_VALUE((Constant *) MAP_GET_STRING(map, tOp->tableName));
				num++;
				MAP_ADD_STRING_KEY(map, tOp->tableName, createConstInt(num));
			}
			else
				MAP_ADD_STRING_KEY(map, tOp->tableName, createConstInt(1));


			int pnum = INT_VALUE((Constant *) MAP_GET_STRING(map, tOp->tableName));
			DEBUG_LOG("table: %s, num: %d", ctableName->value, pnum);

			SET_STRING_PROP(o, PROP_NUM_TABLEACCESS_MARK, createConstInt(pnum));
		}


		loopMarkNumOfTableAccess(o, map);
	}
}

psInfo*
createPSInfo(Node *coarsePara)
{
	psInfo* result = makeNode(psInfo);
	List *coarseParaList = (List *) coarsePara;

	Constant *psType = (Constant *) getNthOfListP(coarseParaList, 0);
	result->psType = STRING_VALUE(psType);

	List *tablePSList = (List *) getNthOfListP(coarseParaList, 1);
	HashMap *map =  NEW_MAP(Constant,Node);

    FOREACH(KeyValue, tablesKV, tablePSList) //R-> [A 1,2,3,4 '101'] [B 1,2,3,4 '110'], S->......
    {
    		List *attrPSInfoList = NIL;
    		char *tableName = STRING_VALUE(tablesKV->key); //R
    		FOREACH(List, l, (List *) tablesKV->value) //[A 1,2,3,4 '101'] [B 1,2,3,4 '110']
    		{
    			psAttrInfo* curPSAI = createPSAttrInfo(l, tableName);
    			attrPSInfoList = appendToTailOfList(attrPSInfoList, curPSAI);
    		}
    		MAP_ADD_STRING_KEY(map, tableName, (Node *) attrPSInfoList);
    }
    result->tablePSAttrInfos = map;

    DEBUG_LOG("psInfo psType: %s", result->psType);
    HashMap *testMap = result->tablePSAttrInfos;
    List* listPSInfos = (List *) getMapString(testMap, "customer");
    FOREACH(psAttrInfo, psif, listPSInfos)
    {
     	DEBUG_LOG("psInfo attrName: %s", psif->attrName);
     	FOREACH(Constant, c, psif->rangeList)
     	{
     		if(c->constType == DT_INT)
    				DEBUG_LOG("psInfo rangeList: %d", INT_VALUE(c));
     		else if(c->constType == DT_STRING)
     			DEBUG_LOG("psInfo rangeList: %s", STRING_VALUE(c));
     	}
     }


	return result;
}

psAttrInfo*
createPSAttrInfo(List *l, char *tableName)
{
	 psAttrInfo *result = NEW(psAttrInfo);

	 Constant *attrName = (Constant *) getNthOfListP(l, 0);
	 Constant *rangeList = (Constant *) getNthOfListP(l, 1);

	 result->attrName = STRING_VALUE(attrName);
	 result->rangeList = (List *) rangeList;
	 if(LIST_LENGTH(result->rangeList) == 1)
	 {
		 int numRanges = INT_VALUE((Constant *) getNthOfListP(result->rangeList, 0));
		 result->rangeList = getRangeList(numRanges, result->attrName, tableName);
	 }

	 if(LIST_LENGTH(l) == 3)
	 {
		 Constant *bitVector = (Constant *) getNthOfListP(l, 2);
		 char *bitVectorStr = STRING_VALUE(bitVector);
		 result->BitVector = stringToBitset(bitVectorStr);
	 }
	 else
		 result->BitVector = NULL;

	 return result;
}

List *
getRangeList(int numRanges, char* attrName, char *tableName)
{
	List *result = NIL;
	/*  auto-range */
	List *hist = getHist(tableName, attrName, numRanges);
	char *rangeString = getHeadOfListP(hist);
	char *minValue = getNthOfListP(hist, 1);
	char *maxValue = getNthOfListP(hist, 2);
	int maxV = atoi(maxValue) + 1;

	/* remove the first and the last and add the min and max (fix first and last) */
	char *firstComma = strchr(rangeString, ',');
	char *lastComma = strrchr(rangeString, ',');
	char *subRangeString = (char *)calloc(1, lastComma - firstComma + 1);
	strncpy(subRangeString, firstComma+1, lastComma-(firstComma+1));
	DEBUG_LOG("subRangeString : %s", subRangeString);

	StringInfo newRangeString = makeStringInfo();
	appendStringInfo(newRangeString, "%s,%s,%d", minValue, subRangeString, maxV);
	DEBUG_LOG("newRangeString : %s", newRangeString->data);


	char *p =  strtok(strdup(newRangeString->data),",");
	while (p!= NULL)
	{
	    result = appendToTailOfList(result, createConstString(p));
	    DEBUG_LOG("p: %s", p);
	    p = strtok (NULL, ",");
	}

	return result;
}
