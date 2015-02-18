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


    APPLY_AND_TIME_OPT("merge adjacent projections and selections",
            mergeAdjacentOperators,
            OPTIMIZATION_MERGE_OPERATORS);
    APPLY_AND_TIME_OPT("selection pushdown",
            pushDownSelectionOperatorOnProv,
            OPTIMIZATION_SELECTION_PUSHING);
    APPLY_AND_TIME_OPT("remove unnecessary columns",
    		removeUnnecessaryColumns,
    		OPTIMIZATION_REMOVE_UNNECESSARY_COLUMNS);

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
    APPLY_AND_TIME_OPT("selection move around",
            selectionMoveAround,
            OPTIMIZATION_SELECTION_MOVE_AROUND);
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
        root = removeUnnecessaryAttrDefInSchema(icols, root);

        //step (2)
        Operator *condOp = (Operator *)((SelectionOperator *)root)->cond;
        resetAttrPosInCond(root, condOp);
	}

	if(isA(root, WindowOperator))
	{
        /*
         * (1) Remove unnecessary attributeDef in schema based on icols and e.icols
         * (2) Reset the pos of attributeRef in FunctionalCall, Partition By and Order By
         */

		//step (1)
		Set *eicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
        icols = unionSets(icols,eicols);
        QueryOperator *winOp = &(((WindowOperator *)root)->op);

		List *newAttrDefs = NIL;
		FOREACH(AttributeDef, ad, winOp->schema->attrDefs)
		{
			if(hasSetElem(icols, ad->attrName))
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
        	FOREACH(AttributeReference, ar, ordList)
        	{
        		resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
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
		QueryOperator *joinOp = &(((JoinOperator *)root)->op);
		List *newAttrDefs = NIL;
		FOREACH(AttributeDef, ad, joinOp->schema->attrDefs)
		{
			if(hasSetElem(icols, ad->attrName))
			{
				newAttrDefs = appendToTailOfList(newAttrDefs, ad);
			}
		}
		joinOp->schema->attrDefs = newAttrDefs;

		if((((JoinOperator*)root)->joinType == JOIN_INNER))
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
                AttributeReference *x = (AttributeReference *)LC_P_VAL(lc1);
                AttributeDef *y = (AttributeDef *)LC_P_VAL(lc2);

                if (!streq(x->name,y->attrName) || i++ != x->attrPosition)
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
		Operator *c = (Operator *)((SelectionOperator *)newRoot)->cond;
		opList = getSelectionCondOperatorList(opList, c);

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
    //loop 1, bottom to top trace the tree and set the property of each
    //operation(tree node)
    setMoveAroundListSetProperityForWholeTree(root);

    //loop 2, top to bottom trace the tree and reset the property of each
    //operation(tree node)
    reSetMoveAroundListSetProperityForWholeTree(root);

    //loop 3, bottom to top trace the tree and introduce the new selection or
    //change the condition of original selection op
    introduceSelection(root);

    //DEBUG_LOG("after the beauty is: \n%s",beatify(nodeToString(root)));

    return root;
}

void
setMoveAroundListSetProperityForWholeTree(QueryOperator *root)
{
	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
            		setMoveAroundListSetProperityForWholeTree(op);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			List *setList = NIL;
			FOREACH(AttributeDef,a, root->schema->attrDefs)
			{
				Set *s = MAKE_SET_PTR(a->attrName);
				setList = appendToTailOfList(setList, s);
			}

			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), (Node *)setList);
		}

		else if(isA(root, SelectionOperator))
		{
			List *opList = NIL;

			Operator *c = (Operator *)(((SelectionOperator *)root)->cond);

			opList = getSelectionCondOperatorList(opList, c);


			QueryOperator *child = OP_LCHILD(root);
			Node *n1 = getProperty((QueryOperator *)child, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));

			List *l1 = (List *)n1;
			List *l11 = NIL;
			List *setList = NIL;

			FOREACH(Set, s1, l1)
			{
				//Set *s = unionSets(s1,s1);
				Set *s = copyObject(s1);
				l11 = appendToTailOfList(l11, s);
			}

			setList = UnionEqualElemOfTwoSetList(opList, l11);
			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), (Node *)setList);
		}

		else if(isA(root, JoinOperator))
		{
			QueryOperator *newRoot = root;
			QueryOperator *parent = getHeadOfListP(root->parents);
			List *opList = NIL;
			List *setList = NIL;
			List *helpList = NIL;

			Operator *c = NULL;
			if(((JoinOperator *)newRoot)->joinType == JOIN_INNER)
				c = (Operator *)(((JoinOperator *)newRoot)->cond);
			else if(((JoinOperator *)newRoot)->joinType == JOIN_CROSS)
                               if(parent != NULL)
				  c = (Operator *)(((SelectionOperator *)parent)->cond);

                        if(c != NULL)
			   opList = getSelectionCondOperatorList(opList, c);


			FOREACH(QueryOperator, op, root->inputs)
			{
				Node *n1 = getProperty(op, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
				List *l1 = (List *)n1;

				List *templ1 = NIL;
				FOREACH(Set, s1, l1)
				{
					//Set *s = unionSets(s1,s1);
					Set *s = copyObject(s1);
					templ1 = appendToTailOfList(templ1, s);
				}

				helpList = concatTwoLists(helpList,templ1);
			}

			setList = UnionEqualElemOfTwoSetList(opList, helpList);
			setProperty((QueryOperator *)newRoot, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), (Node *)setList);
		}

		//else if(isA(root, ProjectionOperator))
		else
		{
			QueryOperator *child = OP_LCHILD(root);
			Node *n1 = getProperty((QueryOperator *)child, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), n1);
		}
	}
}

void
reSetMoveAroundListSetProperityForWholeTree(QueryOperator *root)
{
    if(isA(root, JoinOperator))
    {
    	Node *n1 = NULL;
    	List *l1 = NIL;
    	QueryOperator *parent = getHeadOfListP(root->parents);
    	if(((JoinOperator *)root)->joinType == JOIN_INNER && isA(parent, SelectionOperator))
    	{
    		n1 = getProperty(parent, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
    		l1 = (List *)n1;
    	}
    	else
    	{
    		n1 = getProperty(root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
    		l1 = (List *)n1;
    	}

        FOREACH(QueryOperator, op, root->inputs)
        {
            while(!(isA(op,SelectionOperator) || isA(op,JoinOperator) || isA(op,TableAccessOperator)))
            {
                op = OP_LCHILD(op);
            }

            List *attrName = getAttrNames((Schema *)(op->schema));

            List *newl1 = NIL;
            FOREACH(Set,s,l1)
            {
                Set *tempSet = unionSets(s,s);
            	//Set *tempSet = copyObject(s);
                boolean flag;
                FOREACH_SET(Node,n,s)
                {
                    flag = FALSE;
                    if(!isA(n,Constant))
                    {
                        FOREACH(char,nme,attrName)
                        {
                            if(streq((char *)n,nme))
                            {
                                flag = TRUE;
                            }
                        }
                    }
                    else
                        flag = TRUE;

                    if(flag == FALSE)
                    {
                        removeSetElem(tempSet,n);
                    }
                }
                if(setSize(tempSet) != 0)
                {
                    newl1 = appendToTailOfList(newl1,tempSet);
                }
            }

            setProperty(op, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND),(Node *)newl1);
        }
    }

    FOREACH(QueryOperator, o, root->inputs)
        reSetMoveAroundListSetProperityForWholeTree(o);
}

void
introduceSelection(QueryOperator *root)
{
    if(root->inputs != NULL)
    {
        FOREACH(QueryOperator, op, root->inputs)
        {
            introduceSelection(op);
        }
    }

	if(isA(root,JoinOperator))
	{
		QueryOperator *opl = (QueryOperator *)(OP_LCHILD(root));
		QueryOperator *opr = (QueryOperator *)(OP_RCHILD(root));

		if(isA(opl, ProjectionOperator) && isA(OP_LCHILD(opl), SelectionOperator))
			opl = OP_LCHILD(opl);

		if(isA(opr, ProjectionOperator) && isA(OP_LCHILD(opr), SelectionOperator))
			opr = OP_LCHILD(opr);

		List *opListl = NIL;
		List *opListr = NIL;

		List *originalOpList1 = NIL;
		if(isA(opl, SelectionOperator))
		{
			Operator *originalCondOp1 = (Operator *)(((SelectionOperator *)opl)->cond);
			originalOpList1 = getSelectionCondOperatorList(originalOpList1, originalCondOp1);
		}

		List *originalOpList2 = NIL;
		if(isA(opr, SelectionOperator))
		{
			Operator *originalCondOp2 = (Operator *)(((SelectionOperator *)opr)->cond);
			originalOpList2 = getSelectionCondOperatorList(originalOpList2, originalCondOp2);
		}

		opListl = getMoveAroundOpList(opl);
		opListr = getMoveAroundOpList(opr);

		if(opListl != NIL)
		DEBUG_LOG("opList r1 length = %d", opListl->length);

		if(opListr != NIL)
		DEBUG_LOG("opList l1 length = %d", opListr->length);

		opListr = addNonEqOpToOplistInMoveAround(root, opl, opListr);
		opListl = addNonEqOpToOplistInMoveAround(root, opr, opListl);

		if(opListl != NIL && originalOpList1 != NIL)
		{
			FOREACH(Operator,condOp,originalOpList1)
		    {
				if(!streq(condOp->name,"="))
					opListl = appendToTailOfList(opListl,condOp);
		    }
		}

		if(opListr != NIL && originalOpList2 != NIL)
		{
			FOREACH(Operator,condOp,originalOpList2)
		    {
				if(!streq(condOp->name,"="))
					opListr = appendToTailOfList(opListr,condOp);
		    }
		}

		if(opListl != NIL)
		DEBUG_LOG("opList r2 length = %d", opListl->length);

		if(opListr != NIL)
		DEBUG_LOG("opList l2 length = %d", opListr->length);

		introduceSelectionOrChangeSelectionCond(opListl, opl);
		introduceSelectionOrChangeSelectionCond(opListr, opr);

	}
}

List *
getMoveAroundOpList(QueryOperator *qo)
{
	List *opList = NIL;
	QueryOperator *qo1 = qo;

	//while(isA(qo1, ProjectionOperator))
	//	qo1 = (QueryOperator *)(OP_LCHILD(qo1));

	Node *n1 = getProperty(qo1, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
	List *l1 = (List *)n1;
	opList = NIL;

	FOREACH(Set, s1, l1)
	{

		if(setSize(s1) == 2)
		{
			List *argList = NIL;
			AttributeReference *a;
			FOREACH_SET(Node,selem,s1)
			{
				if(!isA(selem,Constant))
				{
					FOREACH(AttributeDef,attrDef,qo1->schema->attrDefs)
                    {
						if(streq((char *)selem,attrDef->attrName))
						{
							a = createFullAttrReference((char *)selem , 0, 0, 0, attrDef->dataType);
							argList = appendToHeadOfList(argList,a);
							break;
						}
                     }
					//argList = appendToHeadOfList(argList,a);
				}
				else
					argList = appendToTailOfList(argList,selem);
			}

			if(argList->length == 2)
			{
				Operator *o = createOpExpr("=", argList);
				opList = appendToTailOfList(opList, copyObject(o));
			}
		}

		if(setSize(s1) > 2)
		{

			List *argList = NIL;
			AttributeReference *a;
			AttributeReference *b;
			int flagFst = FALSE;

			FOREACH_SET(Node,selem,s1)
			{
				if(flagFst == FALSE)
				{
					flagFst = TRUE;
					if(!isA(selem,Constant))
					{
						FOREACH(AttributeDef,attrDef,qo1->schema->attrDefs)
                        {
							if(streq((char *)selem,attrDef->attrName))
							{
								a = createFullAttrReference((char *)selem , 0, 0, 0, attrDef->dataType);
								argList = appendToHeadOfList(argList,a);
								break;
							}
                         }
						//argList = appendToHeadOfList(argList,a);
					}
					else
					{
						argList = appendToTailOfList(argList,selem);
					}
				}
				else
				{
					if(!isA(selem,Constant))
					{
						FOREACH(AttributeDef,attrDef,qo1->schema->attrDefs)
                        {
							if(streq((char *)selem,attrDef->attrName))
							{
								b = createFullAttrReference((char *)selem , 0, 0, 0, attrDef->dataType);
								argList = appendToHeadOfList(argList,b);
								break;
							}
                        }
						if(argList->length == 2)
						{
							Operator *o1 = createOpExpr("=", argList);
							opList = appendToTailOfList(opList,  copyObject(o1));
							argList = REMOVE_FROM_LIST_PTR(argList,b);
						}

					}
					else
					{
						if(argList->length == 1){
							argList = appendToTailOfList(argList,selem);
							Operator *o2 = createOpExpr("=", argList);
							opList = appendToTailOfList(opList, copyObject(o2));
							argList = REMOVE_FROM_LIST_PTR(argList,selem);
						}
					}
				}
			}
		}

		if(opList != NIL)
		{
			if(isA(qo1, SelectionOperator))
			{
				List *originalOpList = NIL;
				Operator *originalCondOp = (Operator *)(((SelectionOperator *)qo1)->cond);

				originalOpList = getSelectionCondOperatorList(originalOpList, originalCondOp);


				FOREACH(Operator,condOp,originalOpList)
				{
					if(!streq(condOp->name,"="))
					{
						opList = appendToHeadOfList(opList,condOp);
					}
				}
			}
		}
	}
	return opList;
}

List *
addNonEqOpToOplistInMoveAround(QueryOperator *root, QueryOperator *opl, List *opListr)
{
	if(isA(opl, SelectionOperator))
	{

        Node *n = getProperty(root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
        List *l1 = (List *)n;

		List *originalOpList = NIL;
		Operator *originalCondOp = (Operator *)(((SelectionOperator *)opl)->cond);

		originalOpList = getSelectionCondOperatorList(originalOpList, originalCondOp);

		FOREACH(Operator,condOp,originalOpList)
		{

			if(!streq(condOp->name,"="))
			{

				if(isA(getHeadOfListP(condOp->args),AttributeReference) && isA(getTailOfListP(condOp->args),Constant))
				{
					List *tempList = NIL;
					AttributeReference *attrRef = getHeadOfListP(condOp->args);

					FOREACH(Set, s1, l1)
					{
						if(setSize(s1) >= 2)
						{

							boolean flag2 = FALSE;
							char *temp;
							boolean flag1 = FALSE;

							FOREACH_SET(Node,n1,s1)
							{
								if(!isA(n1, Constant))
								{
									if(streq((char *)n1, attrRef->name))
									{
										temp = (char *)n1;
										flag2 = TRUE;
									}
								}
							}

							if(flag2 == TRUE)
							{
								FOREACH_SET(Node,n1,s1)
								{
									if(!isA(n1, Constant))
									{
								         if(!streq(temp, (char *)n1))
								        {
								             AttributeReference *tempAttrRef = createFullAttrReference((char *)n1 , 0, 0, 0, ((AttributeReference *)(getHeadOfListP(condOp->args)))->attrType);
								        	 tempList = appendToHeadOfList(tempList, tempAttrRef);
								        	 flag1 = TRUE;
								        	 break;
								        }
									}
								}
							}

							if(flag1 == TRUE)
							{
								tempList = appendToTailOfList(tempList, copyObject(getTailOfListP(condOp->args)));
								Operator *tempO1 = createOpExpr((char *)(condOp->name), tempList);
								opListr = appendToTailOfList(opListr, tempO1);
								break;
							}

						}
					}
				}

			}
		}
	}

	return opListr;
}

List *
removeRedundantSelectionCondOfOpList(List *opList)
{

   List *eqOpList = NIL;
   List *resultList = NIL;

   FOREACH(Operator, op, opList)
   {
	   if(streq(op->name,"="))
	   {
		   if(isA(getHeadOfListP(op->args),Constant) || isA(getTailOfListP(op->args),Constant))
		   {
			   eqOpList = appendToTailOfList(eqOpList,op);
		   }
	   }
   }

   boolean flag = FALSE;

   FOREACH(Operator, op, opList)
   {
	   flag = FALSE;
       if(streq(op->name,"="))
       {
    	   resultList = appendToTailOfList(resultList,op);
       }
       else if(!isA(getHeadOfListP(op->args),Constant) && !isA(getTailOfListP(op->args),Constant))
       {
    	   resultList = appendToTailOfList(resultList,op);
       }
       else
       {
    	   Operator *opl = (Operator *)(getHeadOfListP(op->args));
    	   Operator *opr = (Operator *)(getTailOfListP(op->args));

    	   if(!isA(opl,Constant))
    	   {
    		   FOREACH(Operator, eqOp, eqOpList)
		       {
    	    	   Operator *eqOpl = (Operator *)(getHeadOfListP(eqOp->args));
    	    	   Operator *eqOpr = (Operator *)(getTailOfListP(eqOp->args));

    	    	   if(!isA(eqOpl,Constant))
    	    	   {
                       if(streq(opl->name, eqOpl->name))
                       {
                    	   flag = TRUE;
                    	   break;
                       }
    	    	   }
    	    	   else if(!isA(eqOpr,Constant))
		           {
                       if(streq(opl->name, eqOpr->name))
                       {
                    	   flag = TRUE;
                    	   break;
                       }
		           }
		       }
    	   }
    	   else if(!isA(opr,Constant))
    	   {
    		   FOREACH(Operator, eqOp, eqOpList)
		       {
    	    	   Operator *eqOpl = (Operator *)(getHeadOfListP(eqOp->args));
    	    	   Operator *eqOpr = (Operator *)(getTailOfListP(eqOp->args));

    	    	   if(!isA(eqOpl,Constant))
    	    	   {
                       if(streq(opr->name, eqOpl->name))
                       {
                    	   flag = TRUE;
                    	   break;
                       }
    	    	   }
    	    	   else if(!isA(eqOpr,Constant))
		           {
                       if(streq(opr->name, eqOpr->name))
                       {
                    	   flag = TRUE;
                    	   break;
                       }
		           }

		       }
    	   }

    	   if(flag == FALSE)
    		   resultList = appendToTailOfList(resultList,op);
       }
   }

	return resultList;
}

void
introduceSelectionOrChangeSelectionCond(List *opList, QueryOperator *qo1)
{
    if(opList != NIL)
     {
         if(isA(qo1, SelectionOperator))
         {
             //e.g. if c=5 and c<9, remove c<9
             //opList = removeRedundantSelectionCondOfOpList(opList);

             Node *opCond = changeListOpToAnOpNode(opList);
             ((SelectionOperator *)qo1)->cond = copyObject(opCond);

             QueryOperator *child = getHeadOfListP(qo1->inputs);
             resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)qo1,(QueryOperator *)child);

         }
         else
         {
         	 //opList = removeRedundantSelectionCondOfOpList(opList);

             Node *opCond = changeListOpToAnOpNode(opList);
             SelectionOperator *newSo1 = createSelectionOp(opCond, NULL, NIL,getAttrNames(qo1->schema));

             // Switch the subtree with this newly created projection
             // operator.
             switchSubtrees((QueryOperator *) qo1, (QueryOperator *) newSo1);

             // Add child to the newly created projections operator,
             addChildOperator((QueryOperator *) newSo1, (QueryOperator *) qo1);

             //set the data type
             setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)newSo1, (QueryOperator *)qo1);

             DEBUG_LOG("111111111111111111111111111111111111111");

             //reset the attr_ref position
             resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)newSo1,(QueryOperator *)qo1);
             DEBUG_LOG("22222222222222222222222222222222222222222");
         }
     }
}
