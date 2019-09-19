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
markTableAccessAndAggregation (QueryOperator *op, Node *coarsePara)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
               DEBUG_LOG("mark tableAccessOperator.");

               /* mark coarsePara info */
               SET_STRING_PROP(o, PROP_COARSE_GRAINED_TABLEACCESS_MARK, coarsePara);

           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }

           markTableAccessAndAggregation(o, coarsePara);
      }
}

void
autoMarkTableAccessAndAggregation (QueryOperator *op, Node *coarsePara, HashMap *map)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
        	   	   TableAccessOperator *tableOp = (TableAccessOperator *) o;

               DEBUG_LOG("mark tableAccessOperator.");

               /* mark coarsePara info */
               SET_STRING_PROP(o, PROP_COARSE_GRAINED_TABLEACCESS_MARK, coarsePara);

               /*  auto-range */
               int histSize = getIntOption(OPTION_BIT_VECTOR_SIZE);
               char *attrName = "";
               if(hasMapStringKey(map, tableOp->tableName))
               {
            	   	   attrName = ((Constant *) getMapString(map, tableOp->tableName))->value;

            	   	   List *hist = getHist(tableOp->tableName, attrName, histSize);
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
            	   	   appendStringInfo(newRangeString, "{%s,%s,%d}", minValue, subRangeString, maxV);
            	   	   DEBUG_LOG("newRangeString : %s", newRangeString->data);

            	   	   /* mark histogram */
            	   	   SET_STRING_PROP(o, AUTO_HISTOGRAM_TABLEACCESS_MARK, createConstString(newRangeString->data));
               }
           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }

           autoMarkTableAccessAndAggregation(o, coarsePara, map);
      }
}


void
markUseTableAccessAndAggregation (QueryOperator *op, Node *coarsePara)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
               DEBUG_LOG("mark use tableAccessOperator.");
               SET_STRING_PROP(o, USE_PROP_COARSE_GRAINED_TABLEACCESS_MARK, coarsePara);
           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               SET_BOOL_STRING_PROP(o, USE_PROP_COARSE_GRAINED_AGGREGATION_MARK);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }
           markUseTableAccessAndAggregation(o,coarsePara);
      }
}


void
markAutoUseTableAccess (QueryOperator *op, HashMap *psMap)
{

      FOREACH(QueryOperator, o, op->inputs)
      {
           if(isA(o,TableAccessOperator))
           {
               DEBUG_NODE_LOG("QueryOperator :", o);
               SET_STRING_PROP(o, AUTO_USE_PROV_COARSE_GRAINED_TABLEACCESS_MARK, psMap);
           }

           markAutoUseTableAccess(o,psMap);
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
