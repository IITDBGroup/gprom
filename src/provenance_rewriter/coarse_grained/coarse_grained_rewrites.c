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
#define MAX_NUM_RANGE 10000

QueryOperator *
addTopAggForCoarse (QueryOperator *op)
{
    List *provAttr = getOpProvenanceAttrNames(op);
    List *projExpr = NIL;
    int cnt = 0;
    List *provPosList = NIL;
//    List *opParents = op->parents;

    HashMap *map	 = (HashMap *) GET_STRING_PROP(op, PROP_LEVEL_AGGREGATION_MARK);
    FOREACH(char, c, provAttr)
    {
        List *levelandNumFrags =  (List *) getMapString(map, c);
        int level =  INT_VALUE((Constant *) getNthOfListP(levelandNumFrags, 0));
        int numFrags =  INT_VALUE((Constant *) getNthOfListP(levelandNumFrags, 1));
        provPosList = appendToTailOfListInt(provPosList, cnt);
        AttributeReference *a = createAttrsRefByName(op, c);
        FunctionCall *f = NULL;
        if(getBackend() == BACKEND_ORACLE)
        {
        		f = createFunctionCall ("BITORAGG", singleton(a));
        		projExpr = appendToTailOfList(projExpr, f);
        }
        else if(getBackend() == BACKEND_POSTGRES)
        {
        		//if(getBoolOption(OPTION_PS_SET_BITS))
        		if(level == 0)
        		{
        			f = createFunctionCall ("set_bits", singleton(a));
        			CastExpr *c = createCastExprOtherDT((Node *) f, "bit", numFrags);
        			projExpr = appendToTailOfList(projExpr, c);
        		}
        		else
        		{
        			f = createFunctionCall ("fast_bit_or", singleton(a));
        			projExpr = appendToTailOfList(projExpr, f);
        		}
        }

        //FunctionCall *f = createFunctionCall ("BITORAGG", singleton(a));
        //projExpr = appendToTailOfList(projExpr, f);
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


//void
//propagateLevelAggregation(QueryOperator *op)
//{
//	//R -> 1, S-> 1....
//	HashMap *map = NEW_MAP(Constant,Node);
//	bottomUpPropagateNumAggregation(op);
//}

void
bottomUpPropagateLevelAggregation(QueryOperator *op, psInfo *psPara)
{
	FOREACH(QueryOperator, o, op->inputs)
	{
		bottomUpPropagateLevelAggregation(o, psPara);

		if(isA(o, TableAccessOperator))
		{
			HashMap *map = NEW_MAP(Constant,Node);
			TableAccessOperator *tbOp = (TableAccessOperator *) o;
			DEBUG_LOG("table-> %s", tbOp->tableName);
			/*propagate map: prov_R_A1 -> (level of agg, num of fragments)*/

			/*get num of table for ps attr*/
			int numTable = 0;
		    if(HAS_STRING_PROP(o, PROP_NUM_TABLEACCESS_MARK))
		    		numTable = INT_VALUE(GET_STRING_PROP(o, PROP_NUM_TABLEACCESS_MARK));


		    /*get psInfo -> attrName and num of fragments*/
			//psInfo* psPara = (psInfo*) GET_STRING_PROP(op, PROP_COARSE_GRAINED_TABLEACCESS_MARK);
			if(hasMapStringKey((HashMap *) psPara->tablePSAttrInfos, tbOp->tableName))
			{
				List *psAttrList = (List *) getMapString(psPara->tablePSAttrInfos, tbOp->tableName);

				for(int j=0; j<LIST_LENGTH(psAttrList); j++)
				{
					psAttrInfo *curPSAI = (psAttrInfo *) getNthOfListP(psAttrList, j);
					int numFragments = LIST_LENGTH(curPSAI->rangeList);
					char *newAttrName = CONCAT_STRINGS(  "prov_",
														strdup(tbOp->tableName),
														"_",
														strdup(curPSAI->attrName),
														gprom_itoa(numTable));

					List *vl = LIST_MAKE(createConstInt(0), createConstInt(numFragments-1));
					MAP_ADD_STRING_KEY(map, newAttrName, (Node *) vl);
					DEBUG_LOG("tableAccess mark map: %s -> count: %d, numFragments: %d", newAttrName, 0, numFragments-1);

				}
			}
			SET_STRING_PROP(o, PROP_LEVEL_AGGREGATION_MARK, map);
		}
		else if(LIST_LENGTH(o->inputs) > 1)
		{
			DEBUG_LOG("table-> more childrens");
			HashMap *newMap = NEW_MAP(Constant,Node);
			FOREACH(Operator, child, o->inputs)
			{
				HashMap *map = (HashMap *) GET_STRING_PROP(child, PROP_LEVEL_AGGREGATION_MARK);
				FOREACH_HASH_ENTRY(kv, map)
				{
					Constant *k = (Constant *) kv->key;
					List *v = (List *) kv->value;
					Constant *firstV = (Constant *) getNthOfListP(v, 0);
					Constant *secondV = (Constant *) getNthOfListP(v, 1);
					List* newV = LIST_MAKE(createConstInt(INT_VALUE(firstV)), createConstInt(INT_VALUE(secondV)));
					MAP_ADD_STRING_KEY(newMap, strdup(STRING_VALUE(k)), (Node *) newV);
					DEBUG_LOG("join mark map: %s -> count: %d, numFragments: %d", STRING_VALUE(k), INT_VALUE(firstV), INT_VALUE(secondV));
				}
			}
			SET_STRING_PROP(o, PROP_LEVEL_AGGREGATION_MARK, newMap);
		}
		else
		{
			DEBUG_LOG("table-> one child");
			QueryOperator *child = (QueryOperator *) getNthOfListP(o->inputs, 0);
			HashMap *map = (HashMap *) GET_STRING_PROP(child, PROP_LEVEL_AGGREGATION_MARK);
			HashMap *newMap = NEW_MAP(Constant,Node);
			if(isA(o, AggregationOperator))
			{
				DEBUG_LOG("table-> aggregation");
				FOREACH_HASH_ENTRY(kv, map)
				{
					Constant *k = (Constant *) kv->key;
					List *v = (List *) kv->value;
					Constant *firstV = (Constant *) getNthOfListP(v, 0);
					Constant *secondV = (Constant *) getNthOfListP(v, 1);
					List* newV = LIST_MAKE(createConstInt(INT_VALUE(firstV) + 1), createConstInt(INT_VALUE(secondV)));
					MAP_ADD_STRING_KEY(newMap, strdup(STRING_VALUE(k)), (Node *) newV);
					DEBUG_LOG("agg mark map: %s -> count: %d, numFragments: %d", STRING_VALUE(k), INT_VALUE(firstV)+1, INT_VALUE(secondV));
				}
				SET_STRING_PROP(o, PROP_LEVEL_AGGREGATION_MARK, newMap);
			}
			else
			{
				DEBUG_LOG("table-> not aggregation");
				SET_STRING_PROP(o, PROP_LEVEL_AGGREGATION_MARK, copyObject(map));
			}
		}

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
	 psAttrInfo *result = makeNode(psAttrInfo);

	 Constant *attrName = (Constant *) getNthOfListP(l, 0);
	 Constant *rangeList = (Constant *) getNthOfListP(l, 1);

	 result->attrName = STRING_VALUE(attrName);
	 result->rangeList = (List *) rangeList;
	 if(LIST_LENGTH(result->rangeList) == 1)
	 {
		 int numRanges = INT_VALUE((Constant *) getNthOfListP(result->rangeList, 0));
		 DEBUG_LOG("test numRanges: %d", numRanges);
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
	/* e.g., 1,2,3,4,5,...,9999,10000 */
	char *firstComma = strchr(rangeString, ',');  //,2,3,4,5,...,10000
	char *lastComma = strrchr(rangeString, ',');	 // ,10000

	char *subRangeString =(char *)calloc(1, lastComma - firstComma + 1);
	strncpy(subRangeString, firstComma+1, lastComma-(firstComma+1));
	DEBUG_LOG("subRangeString : %s", subRangeString); //2,3,4,5,...,9999

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

	/* revise:  if only 10000 tuples (9999 ranges) but request 10000 range */
	/* e.g., 1,2,...,9999,10001 ->  1,2,...,9999,10000,10001*/
//	if(LIST_LENGTH(result) < numRanges+1)
//	{
//		Node *curLast = getTailOfListP(result);
//		result = removeFromTail(result);
//		result = appendToTailOfList(result, createConstString(maxValue));
//		result = appendToTailOfList(result, curLast);
//	}

	DEBUG_LOG("numRanges %d, MAX_NUM_RANGE: %d", numRanges,LIST_LENGTH(result));
    if(numRanges > MAX_NUM_RANGE && LIST_LENGTH(result) == MAX_NUM_RANGE+1)
    {
    	    int numPerRange = numRanges/MAX_NUM_RANGE;
    		List *largeResult = NIL;
    		for(int i=0; i<MAX_NUM_RANGE; i++)
    		{
    			int l = atoi(STRING_VALUE((Constant *) getNthOfListP(result, i)));
    			int r = atoi(STRING_VALUE((Constant *) getNthOfListP(result, i+1)));

    			if(l == r)
    			{
    				largeResult = appendToTailOfList(largeResult, createConstInt(l));
    				continue;
    			}

    			int intervalLen = (r-l)/numPerRange;
    			if(r-l < numPerRange)
    			{
    				numPerRange = r-l;
    				intervalLen = 1;
    			}

    			largeResult = appendToTailOfList(largeResult, createConstInt(l));
    			int next = l;
    			DEBUG_LOG("range begin: %d",l);
    			for(int j=1; j<numPerRange; j++)
    			{
    				next = next + intervalLen;
    				largeResult = appendToTailOfList(largeResult, createConstInt(next));
    				DEBUG_LOG("range middle: %d",next);
    			}
    			numPerRange = numRanges/MAX_NUM_RANGE;
    		}

    		int last = atoi(STRING_VALUE((Constant *) getTailOfListP(result)));
		largeResult = appendToTailOfList(largeResult, createConstInt(last));
		DEBUG_LOG("range end: %d",last);
    		return largeResult;
    }

	return result;
}
