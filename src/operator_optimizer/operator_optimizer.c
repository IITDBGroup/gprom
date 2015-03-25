/*-----------------------------------------------------------------------------
 *
 * operator_optimizer.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "log/logger.h"
#include "operator_optimizer/operator_optimizer.h"
#include "operator_optimizer/operator_merge.h"
#include "operator_optimizer/expr_attr_factor.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_operator/schema_utility.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "rewriter.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "model/set/set.h"
#include "operator_optimizer/optimizer_prop_inference.h"

// macros for running and timing an optimization rule and logging the resulting AGM graph.
#define OPTIMIZER_LOG_PREFIX "\n**********************************************" \
		    "**************\n\tOPTIMIZATION STEP: "
#define OPTIMIZER_LOG_POSTFIX "\n*********************************************" \
		    "***************\n\n"
#define LOG_OPT(message,rewrittenAGM) \
    INFO_LOG(OPTIMIZER_LOG_PREFIX message OPTIMIZER_LOG_POSTFIX "%s" \
            OPTIMIZER_LOG_POSTFIX, \
            operatorToOverviewString((Node *) rewrittenAGM))
#define APPLY_AND_TIME_OPT(optName,optMethod,configOption) \
    if(getBoolOption(configOption)) \
    { \
        START_TIMER("OptimizeModel - " optName); \
        rewrittenTree = optMethod((QueryOperator *) rewrittenTree); \
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree)); \
        LOG_OPT(optName, rewrittenTree); \
        STOP_TIMER("OptimizeModel - " optName); \
    }

static QueryOperator *optimizeOneGraph (QueryOperator *root);
static QueryOperator *pullup(QueryOperator *op, List *duplicateattrs, List *normalAttrNames);
static void pushDownSelection(QueryOperator *root, List *opList,
                              QueryOperator *r, QueryOperator *child);
static void renameOpAttrRefs(QueryOperator *op, HashMap *nameMap, QueryOperator *pro);
static boolean renameAttrRefs (Node *node, HashMap *nameMap);

/* Selection move around */
static boolean compareTwoOperators(Operator *o1, Operator *o2);
static boolean checkBothAttrAndFrom(Operator *op, QueryOperator *root, boolean *f1, boolean *f2);
static void changeNamecheckCond(List *n1, List *n2, Operator *o);
static void changeNameRemoveUnnecessaryCond(List *n1, List *n2, Operator *o1);
static List *getMoveAroundOpList(QueryOperator *qo);
static void introduceSelection(Operator *o, QueryOperator *qo1);
static void introduceSelectionInMoveAround(QueryOperator *root);
static void removeUnnecessaryCond(QueryOperator *root, Operator *o);
static void checkCond(QueryOperator *op, Operator *o, boolean *flag);

Node  *
optimizeOperatorModel (Node *root)
{
    if(isA(root, List))
    {
        FOREACH_LC(lc, (List *) root)
        {
            QueryOperator *o = (QueryOperator *) LC_P_VAL(lc);

            o = optimizeOneGraph(o);
            LC_P_VAL(lc) = o;
        }

        return root;
    }
    else
        return (Node *) optimizeOneGraph((QueryOperator *) root);
}

//static void
//applyMerge(QueryOperator *rewrittenTree)
//{
//    APPLY_AND_TIME_OPT("merge adjacent projections and selections",mergeAdjacentOperators,OPTIMIZATION_MERGE_OPERATORS);
//	if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
//	{
//		START_TIMER("OptimizeModel - merge adjacent operator");
//		rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
//		TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//		LOG_OPT("merged adjacent", rewrittenTree);
//		STOP_TIMER("OptimizeModel - merge adjacent operator");
//	}
//}

//TODO may be unsafe if the pushed down selection is the topmost operator, because the rewrittenTree variable is local to the method
//static void
//applySelectionPushdown(QueryOperator *rewrittenTree)
//{
//    APPLY_AND_TIME_OPT("selection pushdown",pushDownSelectionOperatorOnProv,OPTIMIZATION_SELECTION_PUSHING);
//	if(getBoolOption(OPTIMIZATION_SELECTION_PUSHING))
//	{
//		START_TIMER("OptimizeModel - pushdown selections");
//		rewrittenTree = pushDownSelectionOperatorOnProv((QueryOperator *) rewrittenTree);
//		DEBUG_LOG("selections pushed down", operatorToOverviewString((Node *) rewrittenTree));
//		TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//		STOP_TIMER("OptimizeModel - pushdown selections");
//	}
//}

static QueryOperator *
optimizeOneGraph (QueryOperator *root)
{
    QueryOperator *rewrittenTree = root;

    APPLY_AND_TIME_OPT("factor attributes in conditions", factorAttrsInExpressions, OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR);
//    if(getBoolOption(OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR))
//    {
//        START_TIMER("OptimizeModel - factor attributes in conditions");
//        rewrittenTree = factorAttrsInExpressions((QueryOperator *) rewrittenTree);
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        DEBUG_LOG("factor out attribute references in conditions\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - factor attributes in conditions");
//    }

   /* if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
    {
		res = callback(2);
		if (res == 1)
			applyMerge(rewrittenTree);

		res = callback(2);
		if (res == 1)
			applySelectionPushdown(rewrittenTree);
    }
    else
    {
	applyMerge(rewrittenTree);
	applySelectionPushdown(rewrittenTree);
    }*/

//	applyMerge(rewrittenTree);
//	applySelectionPushdown(rewrittenTree);

    APPLY_AND_TIME_OPT("selection move around",
    		selectionMoveAround,
    		OPTIMIZATION_SELECTION_MOVE_AROUND);
    APPLY_AND_TIME_OPT("pull up duplicate remove operators",
    		pullUpDuplicateRemoval,
    		OPTIMIZATION_PULL_UP_DUPLICATE_REMOVE_OPERATORS);
    APPLY_AND_TIME_OPT("remove unnecessary columns",
    		removeUnnecessaryColumns,
    		OPTIMIZATION_REMOVE_UNNECESSARY_COLUMNS);
    APPLY_AND_TIME_OPT("remove unnecessary window operators",
    		removeUnnecessaryWindowOperator,
    		OPTIMIZATION_REMOVE_UNNECESSARY_WINDOW_OPERATORS);
    APPLY_AND_TIME_OPT("merge adjacent projections and selections",
            mergeAdjacentOperators,
            OPTIMIZATION_MERGE_OPERATORS);
    APPLY_AND_TIME_OPT("selection pushdown",
            pushDownSelectionOperatorOnProv,
            OPTIMIZATION_SELECTION_PUSHING);

    APPLY_AND_TIME_OPT("merge adjacent projections and selections",
            mergeAdjacentOperators,
            OPTIMIZATION_MERGE_OPERATORS);

//    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
//    {
//        START_TIMER("OptimizeModel - merge adjacent operator");
//        rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - merge adjacent operator");
//    }
//
//    if(getBoolOption(OPTIMIZATION_SELECTION_PUSHING))
//    {
//        START_TIMER("OptimizeModel - pushdown selections");
//        rewrittenTree = pushDownSelectionOperatorOnProv((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("selections pushed down\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - pushdown selections");
//    }


//    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
//    {
//        START_TIMER("OptimizeModel - merge adjacent operator");
//        rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - merge adjacent operator");
//    }

    APPLY_AND_TIME_OPT("pushdown selections through joins",
            pushDownSelectionThroughJoinsOperatorOnProv,
            OPTIMIZATION_SELECTION_PUSHING_THROUGH_JOINS);
//    if(getBoolOption(OPTIMIZATION_SELECTION_PUSHING_THROUGH_JOINS))
//    {
//        START_TIMER("OptimizeModel - pushdown selections through joins");
//        rewrittenTree = pushDownSelectionThroughJoinsOperatorOnProv((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("selections pushed down through joins\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - pushdown selections through joins");
//    }

    APPLY_AND_TIME_OPT("factor attributes in conditions",
            factorAttrsInExpressions,
            OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR);
//    if(getBoolOption(OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR))
//    {
//        START_TIMER("OptimizeModel - factor attributes in conditions");
//        rewrittenTree = factorAttrsInExpressions((QueryOperator *) rewrittenTree);
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        DEBUG_LOG("factor out attribute references in conditions again\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - factor attributes in conditions");
//    }

    APPLY_AND_TIME_OPT("merge adjacent projections and selections",
            mergeAdjacentOperators,
            OPTIMIZATION_MERGE_OPERATORS);
//    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
//    {
//        START_TIMER("OptimizeModel - merge adjacent operator");
//        rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - merge adjacent operator");
//    }


//    if(getBoolOption(OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ))
//    {
//        START_TIMER("OptimizeModel - set materialization hints");
//        rewrittenTree = materializeProjectionSequences((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("add materialization hints for projection sequences\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - set materialization hints");
//    }




    if (getBoolOption(OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR))
    {
        computeKeyProp(rewrittenTree);

        // Set TRUE for each Operator
        initializeSetProp(rewrittenTree);
        // Set FALSE for root
        setStringProperty((QueryOperator *) rewrittenTree, PROP_STORE_BOOL_SET, (Node *) createConstBool(FALSE));
        computeSetProp(rewrittenTree);

        List *icols =  getAttrNames(GET_OPSCHEMA(root));
    	//char *a = (char *)getHeadOfListP(icols);
    	Set *seticols = MAKE_STR_SET(strdup((char *)getHeadOfListP(icols)));
    	FOREACH(char, a, icols)
    		addToSet (seticols, a);
    }
    APPLY_AND_TIME_OPT("remove redundant duplicate removal operators by set",
            removeRedundantDuplicateOperatorBySet,
            OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR);

    APPLY_AND_TIME_OPT("remove redundant duplicate removal operators by key",
               removeRedundantDuplicateOperatorByKey,
               OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR);
//
//    if(getBoolOption(OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR))
//    {
//        START_TIMER("OptimizeModel - remove redundant duplicate operator");
//        computeKeyProp(root);
//        rewrittenTree = removeRedundantDuplicateOperator((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("remove redundant duplicate operator \n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - remove redundant duplicate operator");
//    }

    APPLY_AND_TIME_OPT("remove redundant projection operators",
            removeRedundantProjections,
            OPTIMIZATION_REMOVE_REDUNDANT_PROJECTIONS);
//    if(getBoolOption(OPTIMIZATION_REMOVE_REDUNDANT_PROJECTIONS))
//    {
//      START_TIMER("OptimizeModel - remove redundant projections");
//      rewrittenTree = removeRedundantProjections((QueryOperator *) rewrittenTree);
//      DEBUG_LOG("remove redundant projections\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//      ASSERT(checkModel((QueryOperator *) rewrittenTree));
//      STOP_TIMER("OptimizeModel - remove redundant projections");
//    }

    /*
    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
    {
        START_TIMER("OptimizeModel - merge adjacent operator");
        rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
        DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
        STOP_TIMER("OptimizeModel - merge adjacent operator");
    }
*/
/*    APPLY_AND_TIME_OPT("selection move around",
            selectionMoveAround,
            OPTIMIZATION_SELECTION_MOVE_AROUND);*/
//    if(getBoolOption(OPTIMIZATION_SELECTION_MOVE_AROUND))
//    {
//        START_TIMER("OptimizeModel - selections move around");
//        rewrittenTree = selectionMoveAround((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("selections move around\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - selections move around");
//    }

    APPLY_AND_TIME_OPT("pull up provenance projections",
            pullingUpProvenanceProjections,
            OPTIMIZATION_PULLING_UP_PROVENANCE_PROJ);

//    if(getBoolOption(OPTIMIZATION_PULLING_UP_PROVENANCE_PROJ))
//    {
//    	START_TIMER("OptimizeModel - pulling up provenance");
//    	rewrittenTree = pullingUpProvenanceProjections((QueryOperator *) rewrittenTree);
//    	DEBUG_LOG("pulling up provenance projections\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//    	ASSERT(checkModel((QueryOperator *) rewrittenTree));
//    	STOP_TIMER("OptimizeModel - pulling up provenance");
//    }


    APPLY_AND_TIME_OPT("merge adjacent projections and selections",
            mergeAdjacentOperators,
            OPTIMIZATION_MERGE_OPERATORS);

    APPLY_AND_TIME_OPT("materialize projections that are unsafe to be merged",
            materializeProjectionSequences,
            OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ);

//    if(getBoolOption(OPTIMIZATION_MERGE_OPERATORS))
//    {
//    	START_TIMER("OptimizeModel - merge adjacent operator");
//    	rewrittenTree = mergeAdjacentOperators((QueryOperator *) rewrittenTree);
//    	DEBUG_LOG("merged adjacent\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//    	TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree));
//    	STOP_TIMER("OptimizeModel - merge adjacent operator");
//    }
//
//    if(getBoolOption(OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ))
//    {
//        START_TIMER("OptimizeModel - set materialization hints");
//        rewrittenTree = materializeProjectionSequences((QueryOperator *) rewrittenTree);
//        DEBUG_LOG("add materialization hints for projection sequences\n\n%s", operatorToOverviewString((Node *) rewrittenTree));
//        ASSERT(checkModel((QueryOperator *) rewrittenTree));
//        STOP_TIMER("OptimizeModel - set materialization hints");
//    }

    return rewrittenTree;
}

QueryOperator *
materializeProjectionSequences (QueryOperator *root)
{
    QueryOperator *lChild = OP_LCHILD(root);

    // if two adjacent projections then materialize the lower one
    if (isA(root, ProjectionOperator) && isA(lChild, ProjectionOperator))
        SET_BOOL_STRING_PROP(lChild, PROP_MATERIALIZE);

    FOREACH(QueryOperator,o,root->inputs)
        materializeProjectionSequences(o);

    return root;
}


QueryOperator *
mergeAdjacentOperators (QueryOperator *root)
{
    if (isA(root, SelectionOperator) && isA(OP_LCHILD(root), SelectionOperator))
        root = (QueryOperator *) mergeSelection((SelectionOperator *) root);
    if (isA(root, ProjectionOperator) && isA(OP_LCHILD(root), ProjectionOperator))
        root = (QueryOperator *) mergeProjection((ProjectionOperator *) root);

    FOREACH(QueryOperator,o,root->inputs)
         mergeAdjacentOperators(o);

    return root;
}


QueryOperator *
pushDownSelectionOperatorOnProv(QueryOperator *root)
{
    QueryOperator *newRoot = root;

	if (isA(root, SelectionOperator) && isA(OP_LCHILD(root), ProjectionOperator))
		newRoot = pushDownSelectionWithProjection((SelectionOperator *) root);

	FOREACH(QueryOperator, o, newRoot->inputs)
		pushDownSelectionOperatorOnProv(o);

	return newRoot;
}

QueryOperator *
factorAttrsInExpressions(QueryOperator *root)
{
    QueryOperator *newRoot = root;

    if (isA(root, ProjectionOperator))
        newRoot = projectionFactorAttrReferences((ProjectionOperator *) root);

    FOREACH(QueryOperator, o, newRoot->inputs)
        factorAttrsInExpressions(o);

    return root;
}

QueryOperator *
removeUnnecessaryWindowOperator(QueryOperator *root)
{
	if(isA(root, WindowOperator))
	{
		Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		char *funcName = ((WindowOperator *)root)->attrName;
		if(!hasSetElem(icols, funcName))
		{
			//window operator's attributes should be its child's attributes + function attributes
			//so no need to reset pos
			QueryOperator *lChild = OP_LCHILD(root);

			// Remove Parent and make lChild as the new parent
			switchSubtrees((QueryOperator *) root, (QueryOperator *) lChild);
			root = lChild;

			//Reset pos, but seems no need
			QueryOperator *parent = getHeadOfListP(root->parents);
			resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)parent,(QueryOperator *)root);

        }
    }

	FOREACH(QueryOperator, o, root->inputs)
	      removeUnnecessaryWindowOperator(o);

	return root;
}

QueryOperator *
removeUnnecessaryColumns(QueryOperator *root)
{
	initializeIColProp(root);
	computeReqColProp(root);
	printIcols(root);
	removeUnnecessaryColumnsFromProjections(root);


    return root;
}

void
resetAttrPosInCond(QueryOperator *root, Operator *condOp){

	if(isA(getHeadOfListP(condOp->args), Operator))
	{
		resetAttrPosInCond(root, (Operator *)getHeadOfListP(condOp->args));
	}
	else if(isA(getHeadOfListP(condOp->args), AttributeReference))
	{
		AttributeReference *a1 = (AttributeReference *)getHeadOfListP(condOp->args);
		resetPos(a1, ((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
	}

	if(isA(getTailOfListP(condOp->args), Operator))
	{
		resetAttrPosInCond(root, (Operator *)getTailOfListP(condOp->args));
	}
	else if(isA(getTailOfListP(condOp->args), AttributeReference))
	{
		AttributeReference *a2 = (AttributeReference *)getTailOfListP(condOp->args);
		resetPos(a2, ((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
	}
}

QueryOperator *
removeUnnecessaryAttrDefInSchema(Set *icols, QueryOperator *op)
{
	List *newAttrDefs = NIL;
	FOREACH(AttributeDef, ad, op->schema->attrDefs)
	{
		if(hasSetElem(icols, ad->attrName))
		{
			newAttrDefs = appendToTailOfList(newAttrDefs, ad);
		}
	}
	op->schema->attrDefs = newAttrDefs;

	return op;
}

void
resetPos(AttributeReference *ar,  List* attrDefs)
{

	int count = 0;
	FOREACH(AttributeDef, ad, attrDefs)
	{
		if(streq(ar->name,ad->attrName))
		{
			ar->attrPosition = count;
			break;
		}
		count++;
	}
}

QueryOperator *
removeUnnecessaryColumnsFromProjections(QueryOperator *root)
{

	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
				removeUnnecessaryColumnsFromProjections(op);
	}

	Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));

	if(isA(root, OrderOperator))
	{
		/*
		 * (1) Remove unnecessary attributeDef in schema based on icols
		 * (2) Reset the pos of attributeRef in ORDER_EXPR
		 */

		//step (1)
		root = removeUnnecessaryAttrDefInSchema(icols, root);

		//step (2)
		List *ordList = ((OrderOperator *)root)->orderExprs;
		FOREACH(OrderExpr, o, ordList)
		{
            AttributeReference *ar = (AttributeReference *)(o->expr);
            resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
		}

	}

	if(isA(root, SelectionOperator))
	{
        /*
         * (1) Remove unnecessary attributeDef in schema based on icols
         * (2) Reset the pos of attributeRef in cond
         */
		//step (1)
		Set *eicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
        root = removeUnnecessaryAttrDefInSchema(eicols, root);

        //step (2)
        Operator *condOp = (Operator *)((SelectionOperator *)root)->cond;
        resetAttrPosInCond(root, condOp);
	}

	if(isA(root, WindowOperator))
	{
        /*
         * (1) Window operator's attributes should be its child's attributes + function attributes
         * (2) Reset the pos of attributeRef in FunctionalCall, Partition By and Order By
         */

		//step (1)
		Set *eicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
        icols = unionSets(icols,eicols);
        QueryOperator *winOp = &(((WindowOperator *)root)->op);

        //List *newAttrDefs = NIL;
		List *newAttrDefs = copyObject(OP_LCHILD(root)->schema->attrDefs);

		FOREACH(AttributeDef, ad, winOp->schema->attrDefs)
		{
			if(streq(((WindowOperator *)root)->attrName, ad->attrName))
			{
				newAttrDefs = appendToTailOfList(newAttrDefs, ad);
			}
		}
		winOp->schema->attrDefs = newAttrDefs;

		//step (2)
		//(1)FunctionalCall
        List *funList = ((FunctionCall *)(((WindowOperator *)root)->f))->args;
        FOREACH(AttributeReference, ar, funList)
        {
        	resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
        }

        //(2)PartitionBy
        List *parList = ((WindowOperator *)root)->partitionBy;
        if(parList != NIL)
        {
        	FOREACH(AttributeReference, ar, parList)
        	{
        		resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
        	}
        }

        //(3)OrderBy
        List *ordList = ((WindowOperator *)root)->orderBy;
        if(ordList != NIL)
        {
        	FOREACH_LC(o,ordList)
		    {
        		/*
        		 * If-else because sometimes ordList store OrderExpr,
        		 * sometimes such as q10, it stores AttributeReference  directly
        		 */
        	      if(isA(LC_P_VAL(o), AttributeReference))
        	      {
        	    	  AttributeReference *ar = (AttributeReference *)LC_P_VAL(o);
        	    	  resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
        	      }
        	      else if(isA(LC_P_VAL(o), OrderExpr))
        	      {
        	    	  AttributeReference *ar = (AttributeReference *)(((OrderExpr *)LC_P_VAL(o))->expr);
        	    	  resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
        	      }
		    }
        }
	}

	if(isA(root, AggregationOperator))
	{
		/*
		 * Reset the attributeReference pos in Group By and Aggrs
		 */

		AggregationOperator *agg = (AggregationOperator *)root;
		QueryOperator *child = (QueryOperator *)getHeadOfListP(root->inputs);
		//e.g. sum(A)
		FOREACH(FunctionCall, a, agg->aggrs)
		{
			//TODO: ar should get from list args, not only the head one
			AttributeReference *ar = (AttributeReference *)(getHeadOfListP(a->args));
			resetPos(ar,child->schema->attrDefs);
		}

		//e.g. Group By
		FOREACH(AttributeReference, a, agg->groupBy)
		{
			resetPos(a,child->schema->attrDefs);
		}
	}

	if(isA(root, JoinOperator))
	{
		Set *elicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
		Set *ericols = (Set*)getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
		Set *eicols = unionSets(elicols,ericols);

		QueryOperator *joinOp = &(((JoinOperator *)root)->op);
		List *newAttrDefs = NIL;
		FOREACH(AttributeDef, ad, joinOp->schema->attrDefs)
		{
			if(hasSetElem(eicols, ad->attrName))
			{
				newAttrDefs = appendToTailOfList(newAttrDefs, ad);
			}
		}
		joinOp->schema->attrDefs = newAttrDefs;

		if(((JoinOperator*)root)->joinType == JOIN_INNER)
		{
			Operator *condOp = (Operator *)((JoinOperator *)root)->cond;
			if(streq(condOp->name,"="))
			{
				AttributeReference *a1 = (AttributeReference *)getHeadOfListP(condOp->args);
				AttributeReference *a2 = (AttributeReference *)getTailOfListP(condOp->args);

				resetPos(a1,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
				resetPos(a2,((QueryOperator *)OP_RCHILD(root))->schema->attrDefs);

			}
		}
	}

	if(isA(root, TableAccessOperator))
	{
		 /*
		  * if size(icols)<size(schema) and its parent is not projection
		  * then need to introduce one projection operator
		  */
		 QueryOperator *tabOp = &((TableAccessOperator *)root)->op;
		 int sizeSchema = getNumAttrs(tabOp);
         if(!isA(getHeadOfListP(root->parents), ProjectionOperator) && setSize(icols)<sizeSchema)
         {
        	 List* projExpr = NIL;
        	 List *projAttrN = NIL;

        	 //Create attrDef name list(projAttrN) and attrRef list(projExpr)
        	 int cnt = 0;
        	 FOREACH(AttributeDef, ad, tabOp->schema->attrDefs)
        	 {
        		 if(hasSetElem(icols, ad->attrName))
        		 {
        			 projAttrN = appendToTailOfList(projAttrN, ad->attrName);
            		 projExpr = appendToTailOfList(projExpr,
            				 createFullAttrReference(
            						 ad->attrName, 0,
            						 cnt, 0,
            						 ad->dataType));
            		 cnt++;
        		 }
        	 }

        	 //Generate projection
        	 ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, projAttrN);

        	 // Switch the subtree with this newly created projection operator.
        	 switchSubtrees((QueryOperator *) root, (QueryOperator *) newpo);

        	 // Add child to the newly created projections operator,
        	 addChildOperator((QueryOperator *) newpo, (QueryOperator *) root);

        	 //Reset the pos of the schema
        	 resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)newpo,(QueryOperator *)root);
        	 //resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)parentOp,(QueryOperator *)newpo);

        	 //set new operator's icols property
 		 	setStringProperty((QueryOperator *) newpo, PROP_STORE_SET_ICOLS, (Node *)icols);
         }
	}

	if(isA(root, ProjectionOperator))
	{
		int numicols = setSize(icols);
		int numAttrs = getNumAttrs(root);

		List *newAttrDefs = NIL;
		List *newAttrRefs = NIL;
		if(numicols < numAttrs)
		{
            FORBOTH_LC(ad, ar, ((ProjectionOperator *)root)->op.schema->attrDefs ,((ProjectionOperator *)root)->projExprs)
		    {
                AttributeReference *attrRef = (AttributeReference *)LC_P_VAL(ar);
                AttributeDef *attrDef = (AttributeDef *)LC_P_VAL(ad);

                if(hasSetElem(icols, (char *)attrDef->attrName))
                {
                     newAttrDefs = appendToTailOfList(newAttrDefs, attrDef);
                     newAttrRefs = appendToTailOfList(newAttrRefs, attrRef);
                }
		    }
            ((ProjectionOperator *)root)->op.schema->attrDefs = newAttrDefs;
            ((ProjectionOperator *)root)->projExprs = newAttrRefs;

         	QueryOperator *child = (QueryOperator *)OP_LCHILD(root);
            resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)root,(QueryOperator *)child);

            //if up layer is projection, reset the pos of up layer's reference
            if(root->parents != NIL)
            {
            	QueryOperator *p = (QueryOperator *)getHeadOfListP(root->parents);
            	if(isA(p, ProjectionOperator))
            	{
            		QueryOperator *r = &((ProjectionOperator *)root)->op;
            		resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)p,(QueryOperator *)r);

            	}

            }
		}
		else
		{
	     	QueryOperator *child = (QueryOperator *)OP_LCHILD(root);
	        resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)root,(QueryOperator *)child);
		}

	}

	return root;
}

QueryOperator *
removeRedundantDuplicateOperatorBySet(QueryOperator *root)
{
	if(isA(root, DuplicateRemoval) && (GET_BOOL_STRING_PROP(root, PROP_STORE_BOOL_SET) == TRUE))
	{
		QueryOperator *lChild = OP_LCHILD(root);

		// Remove Parent and make lChild as the new parent
		switchSubtrees((QueryOperator *) root, lChild);
		root = lChild;
	}

    FOREACH(QueryOperator, o, root->inputs)
        removeRedundantDuplicateOperatorBySet(o);

    return root;
}


QueryOperator *
removeRedundantDuplicateOperatorByKey(QueryOperator *root)
{
    QueryOperator *lChild = OP_LCHILD(root);

    if (isA(root, DuplicateRemoval) && isA(lChild, ProjectionOperator))
    {
        Node *n1 = getProperty(lChild, (Node *) createConstString(PROP_STORE_LIST_KEY));
        List *l1 = (List *)n1;

        /* Projection is sensitive to Duplicates, If there is no key, we can't
         * remove Duplicate Operator
         */
        if (l1 != NULL)
        {
            // Remove Parent and make lChild as the new parent
        	switchSubtrees((QueryOperator *) root, (QueryOperator *) lChild);
	    	root = lChild;
        }
    }

    FOREACH(QueryOperator, o, root->inputs)
        removeRedundantDuplicateOperatorByKey(o);

    return root;
}

QueryOperator *
pullUpDuplicateRemoval(QueryOperator *root)
{
    computeKeyProp(root);

    List *drOp = NULL;
    findDuplicateRemoval(&drOp, root);

    FOREACH(DuplicateRemoval, op, drOp)
    {
    	if(op->op.parents != NIL)
    		doPullUpDuplicateRemoval(op);
    }

    return root;
}

void
findDuplicateRemoval(List **drOp, QueryOperator *root)
{
	if(isA(root, DuplicateRemoval))
		*drOp = appendToTailOfList(*drOp, (DuplicateRemoval *)root);

	FOREACH(QueryOperator, op, root->inputs)
	      findDuplicateRemoval(drOp, op);

}

void
doPullUpDuplicateRemoval(DuplicateRemoval *root)
{

	//Calculate the num of op(has key) above the DR op
	int count = 0;
	List *keyList = NIL;
	QueryOperator *tempRoot = (QueryOperator *)root;
    while(tempRoot->parents != NIL)
    {
    	keyList = (List *) getStringProperty(tempRoot, PROP_STORE_LIST_KEY);
    	if(keyList != NIL)
            count++;
    	tempRoot = ((QueryOperator *) getHeadOfListP(tempRoot->parents));
    }

    int countrolNum = callback(count);
    if(countrolNum == -1)
    	countrolNum = 0;
    /*
     * if count = 3, countrolNum = callback(3)
     * (1) countrolNum = 0, DR pull up 1 layer
     * (2) countrolNum = 1, DR pull up 2 layer
     * (3) countrolNum = 2, DR pull up 3 layer
     * Will skip the layer which don't has key
     */
    QueryOperator *newOp = (QueryOperator *)root;
    QueryOperator *child = OP_LCHILD(root);
    for(int i=0; i<=countrolNum; i++)
    {
    	//Make sure the new parent has key
    	while(TRUE)
    	{
    		keyList = NIL;
    		if(newOp->parents == NIL)
    			break;
    		newOp = ((QueryOperator *) getHeadOfListP(newOp->parents));

    		//TODO: After set key in the table R, retrieve below line and comment out another line
    		//sd
    		keyList = appendToTailOfList(keyList,"A");
    		if(keyList != NIL)
    			break;

    	}
    }

    switchSubtrees((QueryOperator *) root, (QueryOperator *) child);
    switchSubtrees((QueryOperator *) newOp, (QueryOperator *) root);

    root->op.inputs = NIL;
    newOp->parents = NIL;
	addChildOperator((QueryOperator *) root, (QueryOperator *) newOp);

	root->op.schema->attrDefs = OP_LCHILD(root)->schema->attrDefs;
}

QueryOperator *
removeRedundantProjections(QueryOperator *root)
{
    QueryOperator *lChild = OP_LCHILD(root);

    if (isA(root, ProjectionOperator))
    {
        boolean compare = TRUE;
        List *l1 = ((ProjectionOperator *)root)->projExprs;
        List *l2 = lChild->schema->attrDefs;
        int i = 0;

        if (LIST_LENGTH(l1) != LIST_LENGTH(l2))
            compare = FALSE;
        else
        {
            FORBOTH_LC(lc1,lc2,l1,l2)
            {
                Node *n1 = LC_P_VAL(lc1);
                if (isA(n1,AttributeReference))
                {
                    AttributeReference *x = (AttributeReference *) n1;
                    AttributeDef *y = (AttributeDef *)LC_P_VAL(lc2);

                    if (!streq(x->name,y->attrName) || i++ != x->attrPosition)
                    {
                        compare = FALSE;
                        break;
                    }
                }
                else
                {
                    compare = FALSE;
                    break;
                }
            }
        }

        if (compare)
        {
            List *projAttrs = getQueryOperatorAttrNames(root);
            List *childAttrs = getQueryOperatorAttrNames(lChild);
            HashMap *nameMap = NEW_MAP(Node,Node);

            // adapt any attribute references in the parent of the redundant
            // projection
            FORBOTH(char,pA,cA,projAttrs, childAttrs)
                MAP_ADD_STRING_KEY(nameMap, pA, createConstString(cA));

            FOREACH(QueryOperator,parent,root->parents)
                renameOpAttrRefs(parent, nameMap, root);

            // Remove Parent and make lChild as the new parent
            switchSubtrees((QueryOperator *) root, (QueryOperator *) lChild);
            root = lChild;
        }
    }

    FOREACH(QueryOperator, o, root->inputs)
        removeRedundantProjections(o);

    return root;
}

#define CHILDPOS_KEY "__CHILDPOS__"

static void
renameOpAttrRefs(QueryOperator *op, HashMap *nameMap, QueryOperator *pro)
{
    int childPos = 0;

    // find position in parents input list and store in hashmap
    FOREACH(QueryOperator,child,op->inputs)
    {
        if (child == pro)
            break;
        childPos++;
    }
    MAP_ADD_STRING_KEY(nameMap, CHILDPOS_KEY, createConstInt(childPos));

    if (isA(op,SelectionOperator))
        renameAttrRefs((Node *) ((SelectionOperator *) op)->cond, nameMap);
    if (isA(op,JoinOperator))
        renameAttrRefs((Node *) ((JoinOperator *) op)->cond, nameMap);
    if (isA(op,AggregationOperator))
    {
        AggregationOperator *agg = (AggregationOperator *) op;
        renameAttrRefs((Node *) agg->aggrs, nameMap);
        renameAttrRefs((Node *) agg->groupBy, nameMap);
    }
    if (isA(op,ProjectionOperator))
        renameAttrRefs((Node *) ((ProjectionOperator *) op)->projExprs, nameMap);
    if (isA(op,DuplicateRemoval))
        renameAttrRefs((Node *) ((DuplicateRemoval *) op)->attrs, nameMap);
    if (isA(op,NestingOperator))
        renameAttrRefs((Node *) ((NestingOperator *) op)->cond, nameMap);
    if (isA(op,WindowOperator))
    {
        WindowOperator *w = (WindowOperator *) op;
        renameAttrRefs((Node *) w->partitionBy, nameMap);
        renameAttrRefs((Node *) w->orderBy, nameMap);
        renameAttrRefs((Node *) w->frameDef, nameMap);
        renameAttrRefs((Node *) w->f, nameMap);
    }
    if (isA(op,OrderOperator))
        renameAttrRefs((Node *) ((OrderOperator *) op)->orderExprs, nameMap);
}

static boolean
renameAttrRefs (Node *node, HashMap *nameMap)
{
    if (node == NULL)
        return TRUE;

    if(isA(node,AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        int childPos = INT_VALUE(MAP_GET_STRING(nameMap,CHILDPOS_KEY));

        if (a->fromClauseItem == childPos)
            a->name = strdup(STRING_VALUE(MAP_GET_STRING(nameMap, a->name)));
    }

    if (!isA(node,QueryOperator))
        return visit(node, renameAttrRefs, nameMap);
    return TRUE;
}


QueryOperator *
pullingUpProvenanceProjections(QueryOperator *root)
{
	//QueryOperator *newRoot = root;
	FOREACH(QueryOperator, o, root->inputs)
	{
		if(isA(o, ProjectionOperator))
		{
			if(HAS_STRING_PROP(o, PROP_PROJ_PROV_ATTR_DUP))
			{
				if(GET_BOOL_STRING_PROP(o, PROP_PROJ_PROV_ATTR_DUP) == TRUE)
				{
					ProjectionOperator *op = (ProjectionOperator *)o;

					//Get the attrReference of the provenance attribute
					List *l1 = getProvenanceAttrReferences(op, o);

					//Get the attrDef name of the provenance attribute
					List *l2 = getOpProvenanceAttrNames(o);

					//Get the attrReference of non provenance attribute
					List *l3 = getNormalAttrReferences(op, o);

					//Get the attrDef name in the schema of non provenance
					//attribute
					List *l4 = getNormalAttrNames(o);

					List *l_prov_attr = NIL;

					FOREACH(AttributeReference, a, l1)
					l_prov_attr = appendToTailOfList(l_prov_attr, a->name);

					List *l_normal_attr = NIL;

					FOREACH(AttributeReference, a, l3)
					l_normal_attr =  appendToTailOfList(l_normal_attr, a->name);

					List *normalAttrNames = NIL;
					List *duplicateattrs = NIL;

					FORBOTH_LC(lc1, lc2, l_prov_attr, l2)
					{
						FORBOTH_LC(lc3 ,lc4, l_normal_attr, l4)
                        {
							if(streq(lc1->data.ptr_value, lc3->data.ptr_value))
							{
								duplicateattrs = appendToTailOfList(duplicateattrs,lc2->data.ptr_value);
								normalAttrNames = appendToTailOfList(normalAttrNames, lc4->data.ptr_value);
								break;
							}
                        }
					}


					//Delete the duplicateattrs from the provenance projection
					FOREACH_LC(d,duplicateattrs)
					{
						//Delete the duplicate attr_ref from the projExprs
						int pos = getAttrPos(o, LC_P_VAL(d));
						deleteAttrRefFromProjExprs((ProjectionOperator *)op, pos);

						//Delete the duplicate attr_def from the schema
						deleteAttrFromSchemaByName((QueryOperator *)op, LC_P_VAL(d));
					}

					pullup(o, duplicateattrs, normalAttrNames);
				}
			}
		}

		pullingUpProvenanceProjections(o);
	}

    return root;
}


/*
 * duplicateattrs store attrDef name of provenance attribute, normalAttrnames
 * store attrRef name of provenance attribute
 */
QueryOperator *
pullup(QueryOperator *op, List *duplicateattrs, List *normalAttrNames)
{
	boolean fd = FALSE;
	boolean isLost= FALSE;

	// used to store the name of lost attributes, LostList-duplicateattrs,
	// LostNormalList-normalAttrnames
	List* LostList = NIL;
	List* LostNormalList = NIL;

	List* duplicateattrsCopy = copyList(duplicateattrs);
	List* normalAttrNamesCopy = copyList(normalAttrNames);

	FOREACH(QueryOperator, o, op->parents)
	{
		FORBOTH_LC(d, nms, duplicateattrs, normalAttrNames)
        {
			// find the lost attribute, if we do not find it, we need to add
			// projection op; or continue upward check.
			fd = FALSE;
			if(isA(o, ProjectionOperator))
			{
				FOREACH(AttributeReference,a ,((ProjectionOperator *)o)->projExprs)
                {
					if (streq(a->name, nms->data.ptr_value))
					{
						fd = TRUE;
						break;
					}
                }
			}
			else
			{
				FOREACH_LC(a ,o->schema->attrDefs)
                {
					if (streq(((AttributeDef *) a->data.ptr_value)->attrName, nms->data.ptr_value))
					{
						fd = TRUE;
						break;
					}
                }
			}

			if(!fd)
			{
				isLost = TRUE;

				//add d to the list which stores the name of lost attributes
				LostList = appendToTailOfList(LostList, d->data.ptr_value);
				LostNormalList = appendToTailOfList(LostNormalList, nms->data.ptr_value);

				//get rid of the attribute from the duplicate list and
				//normalAttrnames./test/testrewriter.sh 4 "PROVENANCE OF (SELECT sum(A),B FROM R GROUP BY B);" -activate optimize_operator_model
				duplicateattrsCopy = REMOVE_FROM_LIST_PTR(duplicateattrsCopy, d->data.ptr_value);
				normalAttrNamesCopy = REMOVE_FROM_LIST_PTR(normalAttrNamesCopy, nms->data.ptr_value);
			}
			else
			{
				//If not projection op, just get rid of the attrDef from
				//schema. If projection op get rid of the attrDef from schema
				//and attrRef from projExprs


				if(isA(o, ProjectionOperator))
				{
					if(o->parents != NIL)
					{
						//Get rid of the attrDef from schema and attrRef from projExprs
						int pos = getAttrPos((QueryOperator *)o, LC_P_VAL(d));
						deleteAttrRefFromProjExprs((ProjectionOperator *)o, pos);

						List *normalAttrNamesCopyTempList = NIL;
						boolean nacpFlag;
						char *name;

						FOREACH_LC(n,normalAttrNamesCopy)
						{
							nacpFlag = FALSE;
							FORBOTH_LC(lc1, lc2,((ProjectionOperator *)o)->projExprs,o->schema->attrDefs)
							{
								if(streq(((AttributeReference *)LC_P_VAL(lc1))->name,LC_P_VAL(n)))
								{
									name = ((AttributeDef *)LC_P_VAL(lc2))->attrName;
									nacpFlag = TRUE;
									break;
								}
							}
							if(nacpFlag == TRUE)
								normalAttrNamesCopyTempList = appendToTailOfList(normalAttrNamesCopyTempList, name);
							else
								normalAttrNamesCopyTempList = appendToTailOfList(normalAttrNamesCopyTempList, n);
						}
						normalAttrNamesCopy = normalAttrNamesCopyTempList;

						deleteAttrFromSchemaByName((QueryOperator *)o, LC_P_VAL(d));
					}
					else
					{
						FORBOTH(char, dup, nor, duplicateattrsCopy, normalAttrNamesCopy)
						{
							FORBOTH_LC(attrDef, attrRef, o->schema->attrDefs, ((ProjectionOperator *)o)->projExprs)
		                    {
								if(streq(dup,((AttributeDef *)(attrDef->data.ptr_value))->attrName))
								{
									((AttributeReference *)(attrRef->data.ptr_value))->name = nor;
									break;
								}
	                       	}
						}

					}
			}
				else
				{
					//Just get rid of the attrDef from schema
					deleteAttrFromSchemaByName((QueryOperator *)o, LC_P_VAL(d));
				}
			}

        }


		if(isLost)
		{
			List* projExpr = NIL;
			List *provAttr = NIL;

			//Create the new schema attrDef names in provAttr list
			FOREACH(AttributeReference, attrProv, ((ProjectionOperator *)o)->projExprs)
			{
				provAttr = appendToTailOfList(provAttr, attrProv->name);
			}

			//Create the attr reference from upper op projExprs
			int cnt = 0;
			FOREACH_LC(lc, ((ProjectionOperator *)o)->projExprs)
			{
				projExpr = appendToTailOfList(projExpr,
						createFullAttrReference(
								((AttributeReference *) LC_P_VAL(lc))->name, 0,
								cnt, 0,
								((AttributeReference *) LC_P_VAL(lc))->attrType));
				cnt++;
			}

			//Change the attr reference
			FORBOTH_LC(attrProvName, attrNorName, LostList, LostNormalList)
			{
				FOREACH(AttributeReference, p, projExpr)
                {
					if(streq(p->name,LC_P_VAL(attrProvName)))
					{
						p->name = LC_P_VAL(attrNorName);
						break;
					}
                }
			}

			List *newProvPosList = NIL;
			CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

			//Add projection
			ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
			newpo->op.provAttrs = newProvPosList;

			// Switch the subtree with this newly created projection operator.
			switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

			// Add child to the newly created projections operator,
			addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

			//Reset the pos of the schema
			resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)newpo,(QueryOperator *)op);
			resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)o,(QueryOperator *)newpo);

			pullup(o, duplicateattrsCopy, normalAttrNamesCopy);
		}
		else
		{
			if(isA(o, ProjectionOperator))
			{
				resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)o,(QueryOperator *)op);
			}
			pullup(o, duplicateattrsCopy, normalAttrNamesCopy);
		}

	}

	return op;
}

QueryOperator *
pushDownSelectionThroughJoinsOperatorOnProv(QueryOperator *root)
{
	QueryOperator *newRoot = root;
	QueryOperator *child = OP_LCHILD(root);
	List *opList = NIL;

	if(isA(root, SelectionOperator) && isA(child, JoinOperator))
	{
//		Operator *c = (Operator *)((SelectionOperator *)newRoot)->cond;
		opList = NIL;
		getSelectionCondOperatorList(((SelectionOperator *)newRoot)->cond, &opList);

		if(opList != NIL)
			pushDownSelection(child, opList, newRoot, child);
	}

	FOREACH(QueryOperator, o, newRoot->inputs)
     	pushDownSelectionThroughJoinsOperatorOnProv(o);

	return newRoot;
}

void
pushDownSelection(QueryOperator *root, List *opList, QueryOperator *r, QueryOperator *child)
{

    JoinOperator *newRoot = (JoinOperator *)root;

    List *l1 = NIL;
    List *l2 = NIL;
    List *l3 = NIL;

    QueryOperator *o1 = getHeadOfListP(newRoot->op.inputs);
    QueryOperator *o2 = getTailOfListP(newRoot->op.inputs);

    l1 = getCondOpList(o1->schema->attrDefs, opList);
    l2 = getCondOpList(o2->schema->attrDefs, opList);

    l3 = removeListElementsFromAnotherList(l1, opList);
    l3 = removeListElementsFromAnotherList(l2, l3);

    if(l3 != NIL)
    {
    		Node *opNode3 = changeListOpToAnOpNode(l3);
    		((SelectionOperator *)r)->cond = (Node *)opNode3;
    }
    else
    {
        switchSubtrees((QueryOperator *) r, (QueryOperator *) child);
    }

    if(l1 != NIL)
    {
    	Node *opNode1 = changeListOpToAnOpNode(l1);
    	SelectionOperator *newSo1 = createSelectionOp(opNode1, NULL, NIL,
    			getAttrNames(o1->schema));

    	// Switch the subtree with this newly created Selection operator.
    	switchSubtrees((QueryOperator *) o1, (QueryOperator *) newSo1);

    	// Add child to the newly created Selection operator.
    	addChildOperator((QueryOperator *) newSo1, (QueryOperator *) o1);

    	//set the data type
    	setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)newSo1, (QueryOperator *)o1);

    	//reset the attr_ref position
    	resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)newSo1,(QueryOperator *)o1);
    }

    if(l2 != NIL)
    {
    	Node *opNode2 = changeListOpToAnOpNode(l2);
    	SelectionOperator *newSo2 = createSelectionOp(opNode2, NULL, NIL,
    			getAttrNames(o2->schema));

    	// Switch the subtree with this newly created Selection operator.
    	switchSubtrees((QueryOperator *) o2, (QueryOperator *) newSo2);

    	// Add child to the newly created Selection operator.
    	addChildOperator((QueryOperator *) newSo2, (QueryOperator *) o2);

    	//set the data type
    	setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)newSo2, (QueryOperator *)o2);

    	//reset the attr_ref position
    	resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)newSo2,(QueryOperator *)o2);
    }
}

QueryOperator *
selectionMoveAround(QueryOperator *root)
{
	computeECProp(root);
	introduceSelectionInMoveAround(root);

    return root;
}

boolean
compareTwoOperators(Operator *o1, Operator *o2)
{
	boolean flag = FALSE;

	FOREACH(Node, n1, o1->args)
    {
		if(isA(n1, AttributeReference))
		{
			char *name1 = ((AttributeReference *)n1)->name;
			FOREACH(Node, n2, o2->args)
			{
				if(isA(n2, AttributeReference))
				{
					char *name2 = ((AttributeReference *)n2)->name;
					if(streq(name1, name2))
					{
						flag = TRUE;
						break;
					}
				}
			}
		}
		if(flag == TRUE)
			break;
      }

	return flag;
}

boolean
checkBothAttrAndFrom(Operator *op, QueryOperator *root, boolean *f1, boolean *f2)
{
	boolean flag = FALSE;

	Node *node1 = getHeadOfListP(op->args);
	Node *node2 = getTailOfListP(op->args);
	if(isA(node1, AttributeReference) && isA(node2, AttributeReference))
	{
		flag = TRUE;

		char *a1 = ((AttributeReference *)node1)->name;
		char *a2 = ((AttributeReference *)node2)->name;
		List *names = getQueryOperatorAttrNames(OP_LCHILD(root));

		FOREACH(char, a, names)
		{
			if(streq(a1, a))
				*f1 = TRUE;
			if(streq(a2, a))
				*f2 = TRUE;
		}
	}

	return flag;
}

void
removeUnnecessaryCond(QueryOperator *root, Operator *o)
{
	if(isA(root, JoinOperator))
	{
		List *pCond = NIL;
		List *newCondList = NIL;
		getSelectionCondOperatorList(((JoinOperator *)root)->cond,&pCond);

		FOREACH(Operator, op, pCond)
		{
			boolean flag = FALSE;
			if(!streq(op->name, "="))
				newCondList = appendToTailOfList(newCondList, copyObject(op));
			if(streq(op->name, "="))
			{
				boolean f1 = FALSE;
				boolean f2 = FALSE;
				if(checkBothAttrAndFrom(op, root, &f1, &f2))
				{
					if((f1 == TRUE && f2 == FALSE) || (f1 == FALSE && f2 == TRUE))
						newCondList = appendToTailOfList(newCondList, copyObject(op));
				}
				else
				{
					flag = compareTwoOperators(op, o);
					if(flag == FALSE)
						newCondList = appendToTailOfList(newCondList, copyObject(op));
				}
			}

			if(newCondList != NIL)
			{
				Node *newCond = changeListOpToAnOpNode(copyObject(newCondList));
				((JoinOperator *)root)->cond = newCond;
			}
		}
	}

	if(isA(root, SelectionOperator))
	{
		List *rootCond = NIL;
		List *newCondList = NIL;
		getSelectionCondOperatorList(((SelectionOperator *)root)->cond,&rootCond);

		FOREACH(Operator, op, rootCond)
		{
			boolean flag = FALSE;
			if(!streq(op->name, "="))
				newCondList = appendToTailOfList(newCondList, copyObject(op));
			if(streq(op->name, "="))
			{
				flag = compareTwoOperators(op, o);
				if(flag == FALSE)
					newCondList = appendToTailOfList(newCondList, copyObject(op));
			}
		}

		if(newCondList != NIL)
		{
			Node *newCond = changeListOpToAnOpNode(copyObject(newCondList));
			((SelectionOperator *)root)->cond = newCond;
		}
		else
		{
			if(root->parents == NIL)
				OP_LCHILD(root)->parents = NIL;
			else
			{
				switchSubtrees((QueryOperator *) root, OP_LCHILD(root));
			}
		}
	}

	if(root->parents != NIL)
	{
		if(isA(root, ProjectionOperator))
		{
			ProjectionOperator *p = (ProjectionOperator *)root;
			Operator *o1 = copyObject(o);
			changeNameRemoveUnnecessaryCond(p->projExprs, p->op.schema->attrDefs, o1);

			FOREACH(QueryOperator, p1, root->parents)
			       removeUnnecessaryCond(p1, o1);
		}
		else if(isA(root, AggregationOperator))
		{
			Operator *o1 = copyObject(o);
			AggregationOperator *agg = (AggregationOperator *)root;
			List *l = concatTwoLists(copyObject(agg->aggrs), copyObject(agg->groupBy));
			changeNameRemoveUnnecessaryCond(l, agg->op.schema->attrDefs, o1);

			FOREACH(QueryOperator, p1, root->parents)
			         removeUnnecessaryCond(p1, o1);
		}
		else
		{
			FOREACH(QueryOperator, p1, root->parents)
	        		 removeUnnecessaryCond(p1, o);
		}
	}
}

void
introduceSelectionInMoveAround(QueryOperator *root)
{
	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		        		 introduceSelectionInMoveAround(op);
	}

	List *ECcond = getMoveAroundOpList(root);
	boolean checkFlag = FALSE;
	FOREACH(Operator, o, ECcond)
	{
		if(root->parents != NIL)
			removeUnnecessaryCond((QueryOperator *)getHeadOfListP(root->parents), o);

		//Check if this cond op already have in its subtree
		checkFlag = FALSE;
		checkCond(root, copyObject(o), &checkFlag);

		if(checkFlag == FALSE)
		{
			//(1) check subtree, if no this cond, can introduce selection.
			//(2) check parent tree, if has this cond, remove.
			//(3) if can introduce, introduce selection.
			if(root->parents != NIL)
			{
				QueryOperator *parent = (QueryOperator *)getHeadOfListP(root->parents);

				if(isA(parent, SelectionOperator))
				{
					List *pCond = NIL;
					getSelectionCondOperatorList(((SelectionOperator *)parent)->cond,&pCond);

					pCond = appendToTailOfList(pCond, copyObject(o));
					Node *opCond = changeListOpToAnOpNode(copyObject(pCond));
					((SelectionOperator *)parent)->cond = (Node *) opCond;

					resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)parent,(QueryOperator *)root);
				}
				else
				{
					introduceSelection(o, root);
//					Node *newOp = (Node *)copyObject(o);
//					SelectionOperator *selectionOp = createSelectionOp(newOp, NULL, NIL, getAttrNames(root->schema));
//
//					// Switch the subtree with this newly created projection
//					switchSubtrees((QueryOperator *) root, (QueryOperator *) selectionOp);
//
//					// Add child to the newly created projections operator,
//					addChildOperator((QueryOperator *) selectionOp, (QueryOperator *) root);
//
//					//set the data type
//					setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)selectionOp, (QueryOperator *)root);
//
//					//reset the attr_ref position
//					resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)selectionOp,(QueryOperator *)root);
				}
			}
			else
			{
				introduceSelection(o, root);
//
//				Node *newOp = (Node *)copyObject(o);
//				SelectionOperator *selectionOp = createSelectionOp(newOp, NULL, NIL, getAttrNames(root->schema));
//
//				// Switch the subtree with this newly created projection
//				switchSubtrees((QueryOperator *) root, (QueryOperator *) selectionOp);
//
//				// Add child to the newly created projections operator,
//				addChildOperator((QueryOperator *) selectionOp, (QueryOperator *) root);
//
//				//set the data type
//				setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)selectionOp, (QueryOperator *)root);
//
//				//reset the attr_ref position
//				resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)selectionOp,(QueryOperator *)root);
			}
		}
	}
}

void
changeNameRemoveUnnecessaryCond(List *n1, List *n2, Operator *o1)
{
	FORBOTH(Node, ar, ad, n1, n2)
	{
		if(isA(ar, AttributeReference))
		{
			FOREACH(Node, a, o1->args)
            {
				if(isA(a, AttributeReference))
				{
					if(streq(((AttributeReference *)ar)->name, ((AttributeReference *)a)->name))
					{
						((AttributeReference *)a)->name = ((AttributeDef *)ad)->attrName;
					}
				}
             }
		}
	}
}

void
changeNamecheckCond(List *n1, List *n2, Operator *o)
{
	FORBOTH(Node, ar, ad, n1, n2)
	{
		if(isA(ar, AttributeReference))
		{
			FOREACH(Node, a, o->args)
            {
				if(isA(a, AttributeReference))
				{
					if(streq(((AttributeDef *)ad)->attrName, ((AttributeReference *)a)->name))
					{
						((AttributeReference *)a)->name = ((AttributeReference *)ar)->name;
					}
				}
            }
		}
	}
}

void
checkCond(QueryOperator *root, Operator *o, boolean *flag)
{
	*flag = FALSE;
	if(isA(root, SelectionOperator))
	{
		List *condList = NIL;
		Node *condNode = ((SelectionOperator *)root)->cond;
		getSelectionCondOperatorList(condNode,&condList);

		FOREACH(Operator,p1,condList)
		{
			*flag = compareTwoOperators(p1, o);
			if(*flag == TRUE)
				break;
		}
	}

	if(isA(root, JoinOperator))
	{
		if (((JoinOperator*)root)->joinType != JOIN_CROSS)
		{
			boolean f1 = FALSE;
			boolean f2 = FALSE;
			if(checkBothAttrAndFrom(o, root, &f1, &f2))
			{
				if((f1 == TRUE && f2 == FALSE) || (f1 == FALSE && f2 == TRUE))
					*flag = TRUE;
			}
		}
	}

	if(isA(root, ProjectionOperator))
	{
		ProjectionOperator *p = (ProjectionOperator *)root;
		changeNamecheckCond(p->projExprs, p->op.schema->attrDefs, o);
	}

	if(isA(root, AggregationOperator))
	{
		AggregationOperator *agg = (AggregationOperator *)root;
		List *l = concatTwoLists(copyObject(agg->aggrs), copyObject(agg->groupBy));
		changeNamecheckCond(l, agg->op.schema->attrDefs, o);
	}

	if(*flag == FALSE && !isA(root, TableAccessOperator))
	{
		if(root->inputs != NIL)
		{
			FOREACH(QueryOperator, c, root->inputs)
	    	{
				checkCond(c,o,flag);
				if(*flag == TRUE)
					break;
	    	}
		}
	}
}

List *
getMoveAroundOpList(QueryOperator *op)
{
	List *opList = NIL;
	List *l1 = (List *) getStringProperty(op, PROP_STORE_SET_EC);;

	HashMap *nameToAttrDef = NEW_MAP(Constant,Node);
	FOREACH(AttributeDef, a, op->schema->attrDefs)
		MAP_ADD_STRING_KEY(nameToAttrDef, strdup(a->attrName), (Node *)copyObject(a));

	FOREACH(KeyValue, kv, l1)
	{
	    Set *s = (Set *) kv->key;
	    Constant *c = (Constant *) kv->value;

	    if(c != NULL)
	    {
	    	AttributeReference *aRef = NULL;
	    	FOREACH_SET(char,a,s)
		    {
	    		if(MAP_HAS_STRING_KEY(nameToAttrDef, a))
	    		{
	    			AttributeDef *attr = (AttributeDef *)getMapString(nameToAttrDef, a);
    				aRef = createFullAttrReference(strdup(a), 0, 0, 0, attr->dataType);
	    		}
	            Operator *o = createOpExpr("=", LIST_MAKE(aRef, copyObject(c)));
	            opList = appendToTailOfList(opList, copyObject(o));
		    }
	    }
	    else
	    {
            if(setSize(s) > 1)
            {
            	char *n1 = NULL;
            	FOREACH_SET(char, e, s)
	         	{
            		n1 = e;
            		break;
	        	}

            	AttributeReference *aRef1 = NULL;
	    		if(MAP_HAS_STRING_KEY(nameToAttrDef, n1))
	    		{
	    			AttributeDef *attr = (AttributeDef *)getMapString(nameToAttrDef, n1);
    				aRef1 = createFullAttrReference(strdup(n1), 0, 0, 0, attr->dataType);
	    		}
            	Set *s1 = copyObject(s);
            	removeSetElem(s1, n1);
            	FOREACH_SET(char, e, s1)
            	{
                	AttributeReference *aRef2 = NULL;
    	    		if(MAP_HAS_STRING_KEY(nameToAttrDef, e))
    	    		{
    	    			AttributeDef *attr = (AttributeDef *)getMapString(nameToAttrDef, e);
        				aRef2 = createFullAttrReference(strdup(e), 0, 0, 0, attr->dataType);
    	    		}
    	            Operator *o = createOpExpr("=", LIST_MAKE(aRef1, aRef2));
    	            opList = appendToTailOfList(opList, copyObject(o));
            	}
            }
	    }
	}

	return opList;
}

void
introduceSelection(Operator *o, QueryOperator *root)
{
	Node *newOp = (Node *)copyObject(o);
	SelectionOperator *selectionOp = createSelectionOp(newOp, NULL, NIL, getAttrNames(root->schema));

	// Switch the subtree with this newly created projection
	switchSubtrees((QueryOperator *) root, (QueryOperator *) selectionOp);

	// Add child to the newly created projections operator,
	addChildOperator((QueryOperator *) selectionOp, (QueryOperator *) root);

	//set the data type
	setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)selectionOp, (QueryOperator *)root);

	//reset the attr_ref position
	resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)selectionOp,(QueryOperator *)root);
}
