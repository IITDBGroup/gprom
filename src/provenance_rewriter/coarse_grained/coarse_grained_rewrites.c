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
#include "log/logger.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "model/list/list.h"

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
        FunctionCall *f = createFunctionCall ("BITORAGG", singleton(a));
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
               //SET_BOOL_STRING_PROP(o, PROP_COARSE_GRAINED_AGGREGATION_MARK);
           }

           markUseTableAccessAndAggregation(o,coarsePara);
      }
}
