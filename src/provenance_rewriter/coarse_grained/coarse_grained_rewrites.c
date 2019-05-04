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
               SET_STRING_PROP(o, PROP_COARSE_GRAINED_TABLEACCESS_MARK, coarsePara);
           }
           if(isA(o,AggregationOperator))
           {
               DEBUG_LOG("mark aggregationOperator.");
               SET_BOOL_STRING_PROP(o, PROP_PC_SC_AGGR_OPT);
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }

           markTableAccessAndAggregation(o,coarsePara);
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
