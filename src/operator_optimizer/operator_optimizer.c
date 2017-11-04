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
#include "model/query_operator/query_operator.h"
#include "model/query_operator/schema_utility.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "rewriter.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "model/set/set.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "metadata_lookup/metadata_lookup.h"

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
    	INFO_LOG("START: %s", optName); \
        START_TIMER("OptimizeModel - " optName); \
        rewrittenTree = optMethod((QueryOperator *) rewrittenTree); \
        TIME_ASSERT(checkModel((QueryOperator *) rewrittenTree)); \
        LOG_OPT(optName, rewrittenTree); \
        DOT_TO_CONSOLE_WITH_MESSAGE(optName,rewrittenTree); \
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
//static void resetAttrPosInCond(QueryOperator *root, Operator *condOp);
static void resetPosInExprs (Node *exprs, List *attrDefs);
static void resetPos(AttributeReference *ar,  List* attrDefs);

/* materialze projection sequences */
static boolean internalMaterializeProjectionSequences (QueryOperator *root, void *context);
static QueryOperator *mergeAdjacentOperatorInternal (QueryOperator *root);
static QueryOperator *removeRedundantProjectionsInternal(QueryOperator *root);
static QueryOperator *removeRedundantDuplicateOperatorByKeyInternal(QueryOperator *root);
static QueryOperator *removeUnnecessaryWindowOperatorInternal(QueryOperator *root);


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

static QueryOperator *
optimizeOneGraph (QueryOperator *root)
{
    QueryOperator *rewrittenTree = root;


    int numHeuOptItens = getIntOption(OPTION_COST_BASED_NUM_HEURISTIC_OPT_ITERATIONS);
    NEW_AND_ACQUIRE_MEMCONTEXT("HEURISTIC OPTIMIZER CONTEXT");

    int res;
    int c = 0;
    if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
    {
    	if(numHeuOptItens == 1 || numHeuOptItens < 1) // <1 used to handle numHeuOptItens = 0, smaller than 0 already be handled which will show the help
    		res = 0;
    	else
    	    res = callback(numHeuOptItens);
    }
    else
        res = 0;

    DEBUG_LOG("callback = %d",res);
    DEBUG_LOG("numHeuOptItens = %d",numHeuOptItens);
    INFO_LOG("callback = %d",res);
    INFO_LOG("numHeuOptItens = %d",numHeuOptItens);
    ERROR_LOG("callback = %d",res);
    ERROR_LOG("numHeuOptItens = %d",numHeuOptItens);
    while(c <= res)
    {
    	APPLY_AND_TIME_OPT("factor attributes in conditions",
    			factorAttrsInExpressions,
				OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR);
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
//    	APPLY_AND_TIME_OPT("selection pushdown",
//    			pushDownSelectionOperatorOnProv,
//				OPTIMIZATION_SELECTION_PUSHING);
//    	APPLY_AND_TIME_OPT("merge adjacent projections and selections",
//    			mergeAdjacentOperators,
//				OPTIMIZATION_MERGE_OPERATORS);
//    	APPLY_AND_TIME_OPT("pushdown selections through joins",
//    			pushDownSelectionThroughJoinsOperatorOnProv,
//				OPTIMIZATION_SELECTION_PUSHING_THROUGH_JOINS);
    	APPLY_AND_TIME_OPT("factor attributes in conditions",
    			factorAttrsInExpressions,
				OPTIMIZATION_FACTOR_ATTR_IN_PROJ_EXPR);
    	APPLY_AND_TIME_OPT("merge adjacent projections and selections",
    			mergeAdjacentOperators,
				OPTIMIZATION_MERGE_OPERATORS);
    	if (getBoolOption(OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR))
    	{
    	    START_TIMER("PropertyInference - Keys");
    		computeKeyProp(rewrittenTree);
    		STOP_TIMER("PropertyInference - Keys");

    		//exit(-1);
    		// Set TRUE for each Operator
    		START_TIMER("PropertyInference - Set");
    		initializeSetProp(rewrittenTree);
    		// Set FALSE for root
    		setStringProperty((QueryOperator *) rewrittenTree, PROP_STORE_BOOL_SET, (Node *) createConstBool(FALSE));
    		computeSetProp(rewrittenTree);
            STOP_TIMER("PropertyInference - Set");

    		List *icols =  getAttrNames(GET_OPSCHEMA(root));
    		//char *a = (char *)getHeadOfListP(icols);
    		Set *seticols = MAKE_STR_SET(strdup((char *)getHeadOfListP(icols)));
    		FOREACH(char, a, icols)
    		addToSet (seticols, a);
    	}
//    	/*    APPLY_AND_TIME_OPT("remove redundant duplicate removal operators by set",
//            removeRedundantDuplicateOperatorBySet,
//            OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR);*/
    	APPLY_AND_TIME_OPT("remove redundant duplicate removal operators by set",
    			removeRedundantDuplicateOperatorBySetWithInit,
				OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR);
    	APPLY_AND_TIME_OPT("remove redundant duplicate removal operators by key",
    			removeRedundantDuplicateOperatorByKey,
				OPTIMIZATION_REMOVE_REDUNDANT_DUPLICATE_OPERATOR);
    	APPLY_AND_TIME_OPT("remove redundant projection operators",
    			removeRedundantProjections,
				OPTIMIZATION_REMOVE_REDUNDANT_PROJECTIONS);
    	APPLY_AND_TIME_OPT("pull up provenance projections",
    			pullingUpProvenanceProjections,
				OPTIMIZATION_PULLING_UP_PROVENANCE_PROJ);
    	APPLY_AND_TIME_OPT("merge adjacent projections and selections",
    			mergeAdjacentOperators,
				OPTIMIZATION_MERGE_OPERATORS);
    	APPLY_AND_TIME_OPT("materialize projections that are unsafe to be merged",
    			materializeProjectionSequences,
				OPTIMIZATION_MATERIALIZE_MERGE_UNSAFE_PROJ);

    	APPLY_AND_TIME_OPT("push down aggregation through join",
    			pushDownAggregationThroughJoin,
				OPTIMIZATION_PUSH_DOWN_AGGREGATION_THROUGH_JOIN);

    	DEBUG_LOG("callback = %d in loop %d",res,c);
    	c++;
        START_TIMER("OptimizeModel - RemoveProperties");
        ERROR_LOG("number of operators in graph: %d", numOpsInGraph(rewrittenTree));
        INFO_LOG("number of operators in tree: %d", numOpsInTree(rewrittenTree));
    	emptyProperty(rewrittenTree);
    	STOP_TIMER("OptimizeModel - RemoveProperties");
    }
    FREE_MEM_CONTEXT_AND_RETURN_COPY(QueryOperator,rewrittenTree);
    return rewrittenTree;
}

QueryOperator *
materializeProjectionSequences (QueryOperator *root)
{
    visitQOGraph(root, TRAVERSAL_PRE, internalMaterializeProjectionSequences, NULL);
    return root;
}

static boolean
internalMaterializeProjectionSequences (QueryOperator *root, void *context)
{
    QueryOperator *lChild = OP_LCHILD(root);

    if (isA(root, ProjectionOperator) && isA(lChild, ProjectionOperator))
        SET_BOOL_STRING_PROP(lChild, PROP_MATERIALIZE);

    return TRUE;
}

QueryOperator *
mergeAdjacentOperators (QueryOperator *root)
{
    QueryOperator *newRoot;

    newRoot = mergeAdjacentOperatorInternal(root);
    removeProp(root, PROP_STORE_MERGE_DONE);

    return newRoot;
}


static QueryOperator *
mergeAdjacentOperatorInternal (QueryOperator *root)
{
    QueryOperator *child = OP_LCHILD(root);

    // cannot merge operator with child if the child has more than one parent
    if (isA(root, SelectionOperator) && isA(child, SelectionOperator))
    {
        int numParents = LIST_LENGTH(child->parents);
        if(numParents == 1)
            root = (QueryOperator *) mergeSelection((SelectionOperator *) root);
    }

    if (isA(root, ProjectionOperator) && isA(child, ProjectionOperator))
    {
    	int numParents = LIST_LENGTH(child->parents);
    	if(numParents == 1)
    		root = (QueryOperator *) mergeProjection((ProjectionOperator *) root);
    }

    SET_BOOL_STRING_PROP(root, PROP_STORE_MERGE_DONE);

    FOREACH(QueryOperator,o,root->inputs)
    {
        if(!GET_BOOL_STRING_PROP(o, PROP_STORE_MERGE_DONE))
            mergeAdjacentOperatorInternal(o);
    }

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

/*
 * Used in removeUnnecessaryWindowOperator to
 * reset pos in every operator above this operator
 */
void
upCheckResetPos(QueryOperator *op)
{
	FOREACH(QueryOperator, parent, op->parents)
	{
		resetPosOfAttrRefBaseOnBelowLayerSchema(parent, op);
		upCheckResetPos(parent);
	}
}

QueryOperator *
removeUnnecessaryWindowOperator(QueryOperator *root)
{
    root = removeUnnecessaryWindowOperatorInternal(root);
    removeProp(root, PROP_OPT_REMOVE_RED_WIN_DONW);
    return root;
}

static QueryOperator *
removeUnnecessaryWindowOperatorInternal(QueryOperator *root)
{
	if(isA(root, WindowOperator))
	{
		Set *icols = (Set *) getStringProperty(root, PROP_STORE_SET_ICOLS);
		char *funcName = ((WindowOperator *)root)->attrName;
		if(!hasSetElem(icols, funcName))
		{
			//window operator's attributes should be its child's attributes + function attributes
			//so no need to reset pos
			QueryOperator *lChild = OP_LCHILD(root);

			// Remove root and make lChild as the new root
			//switchSubtree((QueryOperator *) root, (QueryOperator *) lChild);
			switchSubtreeWithExisting((QueryOperator *) root, (QueryOperator *) lChild);
			root = lChild;

			//delete the funcName in its parents' schema attrdefs
			List *newAttrDefs = NIL;
			FOREACH(QueryOperator, op, root->parents)
			{
				FOREACH(AttributeDef, ad, op->schema->attrDefs)
		        {
				     if(!streq(funcName, ad->attrName))
				    	 newAttrDefs = appendToTailOfList(newAttrDefs, ad);
		        }
		        op->schema->attrDefs = newAttrDefs;

		        if(isA(op, ProjectionOperator))
		        {
		        	ProjectionOperator *pj = (ProjectionOperator *) op;
		        	List *newAttrRefs = NIL;
					FOREACH(AttributeReference, ar, pj->projExprs)
			        {
					     if(!streq(funcName, ar->name))
					    	 newAttrRefs = appendToTailOfList(newAttrRefs, ar);
			        }
					pj->projExprs = newAttrRefs;
					resetPosOfAttrRefBaseOnBelowLayerSchema(op, root);
		        }
			}

			//Because remove one attribute in schema, so need to reset pos in every above operators
			upCheckResetPos(root);

        }
    }

	SET_BOOL_STRING_PROP(root,PROP_OPT_REMOVE_RED_WIN_DONW);

	FOREACH(QueryOperator, o, root->inputs)
	    if (!GET_BOOL_STRING_PROP(o,PROP_OPT_REMOVE_RED_WIN_DONW))
	        removeUnnecessaryWindowOperatorInternal(o);

	return root;
}

QueryOperator *
removeUnnecessaryColumns(QueryOperator *root)
{
	START_TIMER("PropertyInference - iCols");
    initializeIColProp(root);
	computeReqColProp(root);
	printIcols(root);
	STOP_TIMER("PropertyInference - iCols");
	removeUnnecessaryColumnsFromProjections(root);

    return root;
}

//void
//resetAttrPosInCond(QueryOperator *root, Operator *condOp){
//
//	if(isA(getHeadOfListP(condOp->args), Operator))
//	{
//		resetAttrPosInCond(root, (Operator *)getHeadOfListP(condOp->args));
//	}
//	else if(isA(getHeadOfListP(condOp->args), AttributeReference))
//	{
//		AttributeReference *a1 = (AttributeReference *)getHeadOfListP(condOp->args);
//		resetPos(a1, ((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//	}
//
//	if(isA(getTailOfListP(condOp->args), Operator))
//	{
//		resetAttrPosInCond(root, (Operator *)getTailOfListP(condOp->args));
//	}
//	else if(isA(getTailOfListP(condOp->args), AttributeReference))
//	{
//		AttributeReference *a2 = (AttributeReference *)getTailOfListP(condOp->args);
//		resetPos(a2, ((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//	}
//}

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

static void
resetPosInExprs (Node *exprs, List *attrDefs)
{
    List *attrRefs = getAttrReferences(exprs);

    FOREACH(AttributeReference,a,attrRefs)
        resetPos(a, attrDefs);
}

static void
resetPos(AttributeReference *ar,  List* attrDefs)
{

	int count1 = 0;
	FOREACH(AttributeDef, ad, attrDefs)
	{
//		if(streq(ar->name, "AGG_GB_ARG1"))
//			DEBUG_LOG("name AGG_GB_ARG1 %s", nodeToString(ar));
		DEBUG_LOG("compare attrDef name %s, count %d", ad->attrName, count1);
		if(streq(ar->name,ad->attrName))
		{
			DEBUG_LOG("map name %s to %s", ar->name, ad->attrName);
			ar->attrPosition = count1;
			DEBUG_LOG("set attr pos of %s to %d", ar->name, count1);
			break;
		}
		count1++;
	}

	//DEBUG_LOG("set attr pos of %s to %d", ar->name, count1);
}


void
printWindowAttrRefs(QueryOperator *op1)
{
	DEBUG_LOG("test");
	if(op1->inputs != NULL)
	{
		QueryOperator *op = (QueryOperator *) getHeadOfListP(op1->inputs);
		if(isA(op, WindowOperator))
		{
			//DEBUG_LOG("WINATTR find window op %p: %s", (void *) op, beatify(nodeToString(op)));
			List *attrRefs = NIL;
			WindowOperator *w = (WindowOperator *) op;
			List *partitionBy = getAttrReferences((Node *) w->partitionBy);
			List *orderBy = getAttrReferences((Node *) w->orderBy);
			List *frameDef = getAttrReferences((Node *) w->frameDef);
			List *f = getAttrReferences((Node *) w->f);
			attrRefs = concatTwoLists(partitionBy, orderBy);
			attrRefs = concatTwoLists(attrRefs, frameDef);
			attrRefs = concatTwoLists(attrRefs, f);
			DEBUG_LOG("WINATTR %p:", (void *) op);
			DEBUG_NODE_BEATIFY_LOG("", attrRefs);
		}

	    printWindowAttrRefs(op);
	}
}

QueryOperator *
removeUnnecessaryColumnsFromProjections(QueryOperator *root)
{
    SET_BOOL_STRING_PROP(root, PROP_OPT_UNNECESSARY_COLS_REMOVED_DONE);
    if(root->inputs != NULL)
    {
        FOREACH(QueryOperator, op, root->inputs)
            if(!HAS_STRING_PROP(op, PROP_OPT_UNNECESSARY_COLS_REMOVED_DONE))
                removeUnnecessaryColumnsFromProjections(op);
    }
    List *cSchema = (root->inputs != NIL) ? OP_LCHILD(root)->schema->attrDefs : NIL;
	Set *icols = (Set*) getStringProperty(root, PROP_STORE_SET_ICOLS);
    List *provAttrNames = getOpProvenanceAttrNames(root);

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
		resetPosInExprs((Node *) ordList, cSchema);
//		FOREACH(OrderExpr, o, ordList)
//		{
//            AttributeReference *ar = (AttributeReference *)(o->expr);
//            resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//		}
	}

	else if(isA(root, DuplicateRemoval))
	{
		//step (1)
		//root = removeUnnecessaryAttrDefInSchema(icols, root);
        root->schema->attrDefs = copyObject(OP_LCHILD(root)->schema->attrDefs);

		//step (2)
		List *attrsList = ((DuplicateRemoval *)root)->attrs;
		resetPosInExprs((Node *) attrsList, cSchema);
	}

	else if(isA(root, SelectionOperator))
	{
        /*
         * (1) Remove unnecessary attributeDef in schema based on icols
         * (2) Reset the pos of attributeRef in cond
         */
		//step (1)
		//Set *eicols = (Set*)getStringProperty(OP_LCHILD(root), PROP_STORE_SET_ICOLS);
		//root = removeUnnecessaryAttrDefInSchema(eicols, root);
		//Set *unicols = unionSets(eicols, icols);
		//root = removeUnnecessaryAttrDefInSchema(unicols, root);
        root->schema->attrDefs = copyObject (OP_LCHILD(root)->schema->attrDefs);

        //step (2)
//        Operator *condOp = (Operator *)((SelectionOperator *)root)->cond;
        resetPosInExprs(((SelectionOperator *)root)->cond, cSchema);
//        resetAttrPosInCond(root, condOp);


	}

	else if(isA(root, WindowOperator))
	{
		//Used for debug
		//printWindowAttrRefs(root);
		//DEBUG_LOG("INFO: WINDOW OPERATOR");
        /*
         * (1) Window operator's attributes should be its child's attributes + function attributes
         * (2) Reset the pos of attributeRef in FunctionalCall, Partition By and Order By
         */

		//step (1)
		Set *eicols = (Set*)getStringProperty(OP_LCHILD(root), PROP_STORE_SET_ICOLS);
        icols = unionSets(icols,eicols);
        WindowOperator *winOp = (WindowOperator *) root;

        //List *newAttrDefs = NIL;
		List *newAttrDefs = copyObject(OP_LCHILD(root)->schema->attrDefs);

		FOREACH(AttributeDef, ad, root->schema->attrDefs)
		{
			if(streq(winOp->attrName, ad->attrName))
			{
				newAttrDefs = appendToTailOfList(newAttrDefs, ad);
			}
		}
		root->schema->attrDefs = newAttrDefs;

		resetPosOfAttrRefBaseOnBelowLayerSchema(root, OP_LCHILD(root));

/*
		//List *cSchema1 = cSchema;
		//step (2)
		//(1)FunctionalCall
		DEBUG_LOG("window function");
		resetPosInExprs(((WindowOperator *)root)->f,cSchema);
//        List *funList = ((FunctionCall *)(((WindowOperator *)root)->f))->args;
//        FOREACH(AttributeReference, ar, funList)
//        {
//        	resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//        }
*/


/*        //(2)PartitionBy
		DEBUG_LOG("window PartitionBy");
//	    List *ccSchema = (root->inputs != NIL) ? OP_LCHILD(root)->schema->attrDefs : NIL;
//	    FOREACH(AttributeDef, ad, ccSchema)
//	    {
//	    	printf("ccdef %s \n", ad->attrName);
//	    }
	    FOREACH(AttributeDef, ad, cSchema)
	    {
	    	printf("cdef %s \n", ad->attrName);
	    }*/

//		List *parList = NIL;
//		parList = ((WindowOperator *)root)->partitionBy;
//		if(LIST_LENGTH(parList) != 0)
//		{
//			resetPosInExprs((Node *) parList ,cSchema1);
//		    FOREACH(AttributeReference, ar, parList)
//		    {
//		    	printf(" %s \n", nodeToString(ar));
//		    }
//		}
//        printf("0000000000000 length %d \n", LIST_LENGTH(parList));
//        FOREACH(AttributeReference, ar, parList)
//        {
//        	printf("0000000000000 arg %s \n", nodeToString(ar));
//        }
//        FOREACH(AttributeDef, ad, cSchema)
//        {
//        	printf("0001111111000 def %s \n", ad->attrName);
//        }
//        if(parList != NIL)
//        {
//        	FOREACH(AttributeReference, ar, parList)
//        	{
//        		resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//        	}
//        }

/*        //(3)OrderBy
		DEBUG_LOG("window order By");
		List *ordList = NIL;
		ordList = ((WindowOperator *)root)->orderBy;
		if(LIST_LENGTH(ordList) != 0)
			resetPosInExprs((Node *) ordList, cSchema);*/


//        if(ordList != NIL)
//        {
//        	FOREACH_LC(o,ordList)
//		    {
//        		/*
//        		 * If-else because sometimes ordList store OrderExpr,
//        		 * sometimes such as q10, it stores AttributeReference  directly
//        		 */
//        	      if(isA(LC_P_VAL(o), AttributeReference))
//        	      {
//        	    	  AttributeReference *ar = (AttributeReference *)LC_P_VAL(o);
//        	    	  resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//        	      }
//        	      else if(isA(LC_P_VAL(o), OrderExpr))
//        	      {
//        	    	  AttributeReference *ar = (AttributeReference *)(((OrderExpr *)LC_P_VAL(o))->expr);
//        	    	  resetPos(ar,((QueryOperator *)OP_LCHILD(root))->schema->attrDefs);
//        	      }
//		    }
//        }
	}

	else if(isA(root, AggregationOperator))
	{
		/*
		 * Reset the attributeReference pos in Group By and Aggrs
		 */

		AggregationOperator *agg = (AggregationOperator *)root;
		resetPosInExprs((Node *) agg->aggrs, cSchema);
		resetPosInExprs((Node *) agg->groupBy, cSchema);

//		QueryOperator *child = (QueryOperator *)getHeadOfListP(root->inputs);
		//e.g. sum(A)
//		FOREACH(FunctionCall, a, agg->aggrs)
//		{
//			//TODO: ar should get from list args, not only the head one
//			AttributeReference *ar = (AttributeReference *)(getHeadOfListP(a->args));
//			resetPos(ar,child->schema->attrDefs);
//		}
//
//		//e.g. Group By
//		FOREACH(AttributeReference, a, agg->groupBy)
//		{
//			resetPos(a,child->schema->attrDefs);
//		}
	}

	else if(isA(root, JoinOperator))
	{
//		Set *elicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
//		Set *ericols = (Set*)getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
//
//		Set *eicols = unionSets(elicols,ericols);
//		FOREACH_SET(char, e, eicols)
//		{
//			DEBUG_LOG("%s ", e);
//		}
		JoinOperator *j = (JoinOperator *) root;

		//List *lChildAttrDefsNames = (List *) getStringProperty(OP_LCHILD(root), PROP_STORE_LIST_SCHEMA_NAMES);
		//List *rChildAttrDefsNames = (List *) getStringProperty(OP_RCHILD(root), PROP_STORE_LIST_SCHEMA_NAMES);

		//int lLength = LIST_LENGTH(lChildAttrDefsNames);
//		HashMap *hm = (HashMap *) getStringProperty(root, PROP_STORE_LIST_SCHEMA_NAMES);

		List *leftSchemaNames = getAttrNames(OP_LCHILD(root)->schema);
		List *rightSchemaNames = getAttrNames(OP_RCHILD(root)->schema);


		//Set schema attr def
		List *newAttrDefs = NIL;
		if(j->cond != NULL)
		{
			if(LIST_LENGTH(root->schema->attrDefs) != (LIST_LENGTH(OP_LCHILD(root)->schema) + LIST_LENGTH(OP_RCHILD(root)->schema)))
			{
				FOREACH(AttributeDef, ad, root->schema->attrDefs)
		    	{
					HashMap *hm = (HashMap *) getStringProperty(root, PROP_STORE_LIST_SCHEMA_NAMES);
					char *name = STRING_VALUE(MAP_GET_STRING(hm, ad->attrName));
					DEBUG_LOG("TEST REMOVE UNNECESSARY COLUMNS MAP %s TO %s .", ad->attrName, name);

					if(searchListString(leftSchemaNames, name) || searchListString(rightSchemaNames, name))
						newAttrDefs = appendToTailOfList(newAttrDefs, ad);
		         }
			}
		}
		else
		{
		     newAttrDefs = concatTwoLists(copyObject(OP_LCHILD(root)->schema->attrDefs), copyObject(OP_RCHILD(root)->schema->attrDefs));
		}
		root->schema->attrDefs = newAttrDefs;

//		List *newAttrDefs = NIL;
//		if(j->cond != NULL)
//		{
//			FOREACH(AttributeDef, ad, root->schema->attrDefs)
//		    {
//				if(searchListString(leftSchemaNames, ad->attrName) || searchListString(rightSchemaNames, ad->attrName))
//					newAttrDefs = appendToTailOfList(newAttrDefs, ad);
//				else
//				{
//					char *tempName = strdup(STRING_VALUE(MAP_GET_STRING(hm, ad->attrName)));
//					if(searchListString(leftSchemaNames, tempName) || searchListString(rightSchemaNames, tempName))
//						newAttrDefs = appendToTailOfList(newAttrDefs, ad);
//				}
//		    }
//			root->schema->attrDefs = newAttrDefs;
//		}

//		//Set schema attr def
//		List *newAttrDefs = NIL;
//		//List *newAttrDefNames = NIL;
//		int count = 1;
//		if(j->cond != NULL)
//		{
//			printf("1111111111 join schema lenght %d \n", LIST_LENGTH(root->schema->attrDefs));
//			printf("=====================\n");
//			FOREACH(AttributeDef, ad, root->schema->attrDefs)
//			{
//				if(count <= lLength)
//				{
//					if(searchListString(leftSchemaNames, ad->attrName))
//						newAttrDefs = appendToTailOfList(newAttrDefs, ad);
//					else
//					{
//						char *tempName = (char *) getNthOfListP(lChildAttrDefsNames, count-1);
//						printf("map tempName %s to %s", tempName, ad->attrName);
//						if(searchListString(leftSchemaNames, tempName))
//							newAttrDefs = appendToTailOfList(newAttrDefs, ad);
//					}
//				}
//				else
//				{
//					if(searchListString(rightSchemaNames, ad->attrName))
//						newAttrDefs = appendToTailOfList(newAttrDefs, ad);
//					else
//					{
//						char *tempName = (char *) getNthOfListP(rChildAttrDefsNames, count-lLength-1);
//						printf("map tempName %s to %s", tempName, ad->attrName);
//						if(searchListString(rightSchemaNames, tempName))
//							newAttrDefs = appendToTailOfList(newAttrDefs, ad);
//					}
//				}
//				count ++;
//			}
//			root->schema->attrDefs = newAttrDefs;
//		}
//		printf("\n=====================\n");
//		else
//		{
//			List *lSchemaAttrDefs = copyObject(OP_LCHILD(root)->schema->attrDefs);
//			List *rSchemaAttrDefs = copyObject(OP_RCHILD(root)->schema->attrDefs);
//			FOREACH(AttributeDef, ad, root->schema->attrDefs)
//			{
//				if((searchListString(leftSchemaNames, ad->attrName) || searchListString(rightSchemaNames, ad->attrName)) && !searchListString(newAttrDefNames, ad->attrName))
//				{
//					newAttrDefs = AppendToTailOfList(newAttrDefs, ad);
//					newAttrDefNames = AppendToTailOfList(newAttrDefNames, ad->attrName);
//				}
//			}
//		}

//		if(j->cond != NULL)
//		{
//			List *condAttrNames = NIL;
//			List *condAttr = getAttrReferences(j->cond);
//			FOREACH(AttributeReference, a, condAttr)
//			    condAttrNames = appendToTailOfList(condAttrNames, a->name);
//			Set *condSet = makeStrSetFromList(condAttrNames);
//			DEBUG_LOG("condAttr: %d, condSet: %d", LIST_LENGTH(condAttrNames), setSize(condSet));
//			if(LIST_LENGTH(condAttrNames) == setSize(condSet))
//				root->schema->attrDefs = newAttrDefs;
//		}
//		else
//		    root->schema->attrDefs = newAttrDefs;

		 //Set cond attr ref pos
		if(j->cond != NULL)
		{
		    //DONE: TODO fix this only works in a very simplistic case. In general we need to split list of attr refs into left and right input refs
			List *attrRefs = getAttrReferences (j->cond);
			List *rcSchema = OP_RCHILD(root)->schema->attrDefs;
//
//			List *leftSchemaNames = getAttrNames(OP_LCHILD(root)->schema);
//			List *rightSchemaNames = getAttrNames(OP_RCHILD(root)->schema);

			List *leftRefs = NIL;
			List *rightRefs = NIL;

			FOREACH(AttributeReference,a,attrRefs)
			{
				if(searchListString(leftSchemaNames, a->name) && a->fromClauseItem == 0)
					leftRefs = appendToTailOfList(leftRefs, a);
				else if(searchListString(rightSchemaNames, a->name) && a->fromClauseItem == 1)
					rightRefs = appendToTailOfList(rightRefs, a);
			}

			DEBUG_LOG("Reset join left");
			FOREACH(AttributeReference,a,leftRefs)
			        resetPos(a,cSchema);

			DEBUG_LOG("Reset join right");
			FOREACH(AttributeReference,a,rightRefs)
			        resetPos(a,rcSchema);

//			FOREACH(AttributeReference,a,attrRefs)
//			{
//			    if (a->fromClauseItem == 0)
//			        resetPos(a,cSchema);
//			    else
//			        resetPos(a,rcSchema);
//			}

//
//			if(streq(condOp->name,"="))
//			{
//				AttributeReference *a1 = (AttributeReference *)getHeadOfListP(condOp->args);
//				AttributeReference *a2 = (AttributeReference *)getTailOfListP(condOp->args);
//
//			}
		}
	}

	else if(isA(root, TableAccessOperator))
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
        	 resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)newpo,(QueryOperator *)root);
        	 //resetPosOfAttrRefBaseOnBelowLayerSchema((ProjectionOperator *)parentOp,(QueryOperator *)newpo);

        	 //set new operator's icols property
 		 	setStringProperty((QueryOperator *) newpo, PROP_STORE_SET_ICOLS, (Node *)icols);
         }
	}

	else if(isA(root, ProjectionOperator))
	{
		//Used for debug
		//printWindowAttrRefs(root);
		//DEBUG_LOG("INFO: PROJECTION OPERATOR");
		int numicols = setSize(icols);
		int numAttrs = getNumAttrs(root);
		ProjectionOperator *proj = (ProjectionOperator *) root;
		List *newAttrDefs = NIL;
		List *newAttrRefs = NIL;
		if(numicols < numAttrs)
		{
            FORBOTH(Node, ad, ar, root->schema->attrDefs,proj->projExprs)
		    {
                AttributeDef *attrDef = (AttributeDef *) ad;

                if(hasSetElem(icols, (char *)attrDef->attrName))
                {
                     newAttrDefs = appendToTailOfList(newAttrDefs, attrDef);
                     newAttrRefs = appendToTailOfList(newAttrRefs, ar);
                }
		    }
            root->schema->attrDefs = newAttrDefs;
            proj->projExprs = newAttrRefs;

         	QueryOperator *child = OP_LCHILD(root);
            resetPosOfAttrRefBaseOnBelowLayerSchema(root,child);

            //if up layer is projection, reset the pos of up layer's reference
            if(root->parents != NIL)
            {
            	QueryOperator *p = (QueryOperator *)getHeadOfListP(root->parents);
            	if(isA(p, ProjectionOperator))
            	{
            		QueryOperator *r = root;
            		resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)p,(QueryOperator *)r);
            	}

            }
		}
		else
		{
	     	QueryOperator *child = (QueryOperator *)OP_LCHILD(root);
	        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)root,(QueryOperator *)child);
		}

		//////////new add for test
//		printf("\n 1111111111 ATTR list length %d \n", LIST_LENGTH(root->schema->attrDefs));
//		FOREACH(AttributeDef, ad, root->schema->attrDefs)
//		{
//			printf("%s ", ad->attrName);
//		}
//		printf("\n");
//		FOREACH(AttributeReference, ar, ((ProjectionOperator *)root)->projExprs)
//		{
//			printf("%s ", ar->name);
//		}
//
//		printf("\n 2222222222 ICOLS list length %d \n", numicols);
//		FOREACH_SET(char, c, icols)
//		{
//			printf("%s ", c);
//		}
	}

	else if(isA(root, JsonTableOperator))
	{
         List *newAttrDef = NIL;
         FOREACH(AttributeDef, a, root->schema->attrDefs)
        	 if(hasSetElem(icols, a->attrName))
        		 newAttrDef = appendToTailOfList(newAttrDef, copyObject(a));

         root->schema->attrDefs = newAttrDef;

         /* reset pos in jsonColumn */
         JsonTableOperator *jt = (JsonTableOperator *)root;
         char *name = jt->jsonColumn->name;
         int cnt = 0;
         FOREACH(AttributeDef, a, cSchema)
         {
        	 if(streq(name, a->attrName))
        	 {
        		 jt->jsonColumn->attrPosition = cnt;
        		 break;
        	 }
        	 cnt++;
         }
	}

	//reset provenance attribute position : provAttrs   provAttrNames
	List *newProvAttrs = NIL;
	int count = 0;
	FOREACH(AttributeDef, a, root->schema->attrDefs)
	{
		if(searchListString(provAttrNames, a->attrName))
			newProvAttrs = appendToTailOfListInt(newProvAttrs, count);

		count ++;
	}
	root->provAttrs = newProvAttrs;

	return root;
}

QueryOperator *
removeRedundantDuplicateOperatorBySetWithInit(QueryOperator *root)
{
    root = removeRedundantDuplicateOperatorBySet(root);
	removeProp(root, PROP_OPT_REMOVE_RED_DUP_BY_SET_DONE);

	return root;
}

QueryOperator *
removeRedundantDuplicateOperatorBySet(QueryOperator *root)
{
    // only remove duprev
    if (isA(root, DuplicateRemoval) && (GET_BOOL_STRING_PROP(root, PROP_STORE_BOOL_SET) == TRUE))
    {
        // make an optimization choice
        if (getBoolOption(OPTION_COST_BASED_OPTIMIZER) && !getBoolOption(OPTION_COST_BASED_CLOSE_OPTION_REMOVEDP_BY_SET))
        {
            int res = callback(2);

            INFO_LOG("res is %d", res);

            // only remove if optimizer decides so
            if (res == 0)
            {
                QueryOperator *lChild = OP_LCHILD(root);
                switchSubtreeWithExisting((QueryOperator *) root, lChild);
                root = lChild;
                return removeRedundantDuplicateOperatorBySet(root);
            }
        }
        else
        {
            QueryOperator *lChild = OP_LCHILD(root);

            switchSubtreeWithExisting((QueryOperator *) root, lChild);
            root = lChild;
            return removeRedundantDuplicateOperatorBySet(root);
        }
    }

    SET_BOOL_STRING_PROP(root, PROP_OPT_REMOVE_RED_DUP_BY_SET_DONE);

    FOREACH(QueryOperator, o, root->inputs)
        if (!GET_BOOL_STRING_PROP(o, PROP_OPT_REMOVE_RED_DUP_BY_SET_DONE))
                removeRedundantDuplicateOperatorBySet(o);

    return root;
}

QueryOperator *
removeRedundantDuplicateOperatorByKey(QueryOperator *root)
{
    root = removeRedundantDuplicateOperatorByKeyInternal(root);
    removeProp(root, PROP_STORE_REMOVE_RED_DUP_BY_KEY_DONE);
    return root;
}


static QueryOperator *
removeRedundantDuplicateOperatorByKeyInternal(QueryOperator *root)
{
    QueryOperator *lChild = OP_LCHILD(root);

    if (isA(root, DuplicateRemoval))
    {
        List *l1 = (List *)getStringProperty(lChild, PROP_STORE_LIST_KEY);

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

    SET_BOOL_STRING_PROP(root, PROP_STORE_REMOVE_RED_DUP_BY_KEY_DONE);

    FOREACH(QueryOperator, o, root->inputs)
        if (!GET_BOOL_STRING_PROP(root, PROP_STORE_REMOVE_RED_DUP_BY_KEY_DONE))
            removeRedundantDuplicateOperatorByKeyInternal(o);

    return root;
}

QueryOperator *
pullUpDuplicateRemoval(QueryOperator *root)
{
//    if (!HAS_STRING_PROP(root, PROP_STORE_LIST_KEY))
        computeKeyProp(root); //TODO Boris: this repeatively computes the key prop

    List *drOp = NULL;
    findDuplicateRemoval(&drOp, root);

    FOREACH(DuplicateRemoval, op, drOp)
    {
    	int numParent = LIST_LENGTH(op->op.parents);
    	if(op->op.parents != NIL && numParent == 1)
    		doPullUpDuplicateRemoval(op);
    }

    return root;
}

void
findDuplicateRemoval(List **drOp, QueryOperator *root)
{
    SET_BOOL_STRING_PROP(root, PROP_STORE_DUP_MARK);
	if(isA(root, DuplicateRemoval))
	{
		*drOp = appendToTailOfList(*drOp, (DuplicateRemoval *)root);
	}

	FOREACH(QueryOperator, op, root->inputs)
	    if (!HAS_STRING_PROP(op, PROP_STORE_DUP_MARK))
	        findDuplicateRemoval(drOp, op);
}

void
doPullUpDuplicateRemoval(DuplicateRemoval *root)
{

	//Calculate the num of op(has key) above the DR op
	int count = 0;
	List *keyList = NIL;
	QueryOperator *tempRoot = (QueryOperator *)root;
    while(tempRoot->parents != NIL && LIST_LENGTH(tempRoot->parents) == 1)
    {
    	keyList = (List *) getStringProperty(tempRoot, PROP_STORE_LIST_KEY);
    	if(keyList != NIL)
            count++;
    	else
    		break;
    	tempRoot = ((QueryOperator *) getHeadOfListP(tempRoot->parents));
    }
    DEBUG_LOG("count = %d\n",count);
    if(count != 0)
    {
        int countrolNum = count;
        if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
            callback(count);
        if(countrolNum == -1)
            countrolNum = 0;
        /*
         * if count = 3, countrolNum = callback(3)
         * (1) countrolNum = 0, DR pull up 1 layer
         * (2) countrolNum = 1, DR pull up 2 layer
         * (3) countrolNum = 2, DR pull up 3 layer
         * Will skip the layer which don't has key
         */
        DEBUG_LOG("countrolNum %d", countrolNum);
        QueryOperator *newOp = (QueryOperator *)root;
        QueryOperator *child = OP_LCHILD(root);
        for(int i=0; i<=countrolNum; i++)
        {
            /*    	//Make sure the new parent has key
    	while(TRUE)
    	{
    		keyList = NIL;
    		if(newOp->parents == NIL)
    			break;
    		newOp = ((QueryOperator *) getHeadOfListP(newOp->parents));

    		//TODO: After set key in the table R, retrieve below line and comment out another line
    		//keyList = (List *) getStringProperty(newOp, PROP_STORE_LIST_KEY);
    		keyList = appendToTailOfList(keyList,"A");
    		DEBUG_LOG("keyList length %d", LIST_LENGTH(keyList));
    		if(keyList != NIL)
    			break;

    	}*/
            if(LIST_LENGTH(newOp->parents) == 1)
                newOp = ((QueryOperator *) getHeadOfListP(newOp->parents));
            else
                break;
        }
        switchSubtrees((QueryOperator *) root, (QueryOperator *) child);
        switchSubtrees((QueryOperator *) newOp, (QueryOperator *) root);

        root->op.inputs = NIL;
        newOp->parents = NIL;
        addChildOperator((QueryOperator *) root, (QueryOperator *) newOp);

        root->op.schema->attrDefs = OP_LCHILD(root)->schema->attrDefs;
    }
}

QueryOperator *
removeRedundantProjections(QueryOperator *root)
{
    QueryOperator *result = removeRedundantProjectionsInternal(root);

    removeProp(root, PROP_STORE_REMOVE_RED_PROJ_DONE);

    return result;
}

static QueryOperator *
removeRedundantProjectionsInternal(QueryOperator *root)
{
    QueryOperator *lChild = OP_LCHILD(root);

    if (isA(root, ProjectionOperator))
    {
        //if rename, It can be removed if it's child only has one parent
        int numParents = LIST_LENGTH(lChild->parents);

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

        if (compare && isA(lChild, ProjectionOperator) && numParents == 1)
        {
            List *projAttrs = getQueryOperatorAttrNames(root);
            List *childAttrs = getQueryOperatorAttrNames(lChild);
            HashMap *nameMap = NEW_MAP(Node,Node);

            // Before remove the projection, let child's schema equal to its schema
            lChild->schema = copyObject(root->schema);

            // Remove Parent and make lChild as the new parent
            switchSubtreeWithExisting((QueryOperator *) root, (QueryOperator *) lChild);

            // adapt any attribute references in the parent of the redundant
            // projection
            FORBOTH(char,pA,cA,projAttrs, childAttrs)
                MAP_ADD_STRING_KEY(nameMap, pA, createConstString(cA));

            FOREACH(QueryOperator,parent,root->parents)
                renameOpAttrRefs(parent, nameMap, root);

            root = lChild;
        }

        else if(compare)
        {
        	boolean compare2 = TRUE;
        	List *l3 = ((ProjectionOperator *)root)->op.schema->attrDefs;
        	FORBOTH_LC(lc1,lc3,l1,l3)
        	{
                Node *n1 = LC_P_VAL(lc1);
                if (isA(n1,AttributeReference))
                {
                	AttributeReference *x = (AttributeReference *) n1;
                    AttributeDef *y = (AttributeDef *)LC_P_VAL(lc3);
                    if (!streq(x->name,y->attrName))
                    {
                    	compare2 = FALSE;
                        break;
                    }
                }
                else
                {
                  compare2 = FALSE;
                  break;
                }
        	}

        	if(compare2)
        	{
        		List *projAttrs = getQueryOperatorAttrNames(root);
        		List *childAttrs = getQueryOperatorAttrNames(lChild);
        		HashMap *nameMap = NEW_MAP(Node,Node);

        		// Remove Parent and make lChild as the new parent
        		switchSubtreeWithExisting((QueryOperator *) root, (QueryOperator *) lChild);
        		//switchSubtreeWithExisting((QueryOperator *) root, (QueryOperator *) lChild);
        		// adapt any attribute references in the parent of the redundant
        		// projection
        		FORBOTH(char,pA,cA,	projAttrs, childAttrs)
        		     MAP_ADD_STRING_KEY(nameMap, pA, createConstString(cA));

        		FOREACH(QueryOperator,parent,root->parents)
        		       renameOpAttrRefs(parent, nameMap, root);

        		root = lChild;
        	}

        }
    }

    SET_BOOL_STRING_PROP(root, PROP_STORE_REMOVE_RED_PROJ_DONE);

    FOREACH(QueryOperator, o, root->inputs)
        if (!GET_BOOL_STRING_PROP(root, PROP_STORE_REMOVE_RED_PROJ_DONE))
            removeRedundantProjectionsInternal(o);

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
        if(!GET_BOOL_STRING_PROP(o, PROP_PROJ_PROV_ATTR_DUP_PULLUP))
        {
            SET_BOOL_STRING_PROP((QueryOperator *)o, PROP_PROJ_PROV_ATTR_DUP_PULLUP);
            if(isA(o, ProjectionOperator))
            {
                ProjectionOperator *op = (ProjectionOperator *)o;
                if(GET_BOOL_STRING_PROP(o, PROP_PROJ_PROV_ATTR_DUP))
                {
                    //Get the attrReference of the provenance attribute
                    List *l1 = getProvenanceAttrReferences(op, o);

                    //Get the attrDef name of the provenance attribute
                    List *l2 = getNormalAttrNames(o);

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

                    if(LIST_LENGTH(o->parents) == 1)
                        pullup(o, duplicateattrs, normalAttrNames);

                }
            }
            pullingUpProvenanceProjections(o);
        }
    }

    return root;
}


/*
 * duplicateattrs stores attrDef name of provenance attribute, normalAttrnames
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

	QueryOperator *o = (QueryOperator *) getHeadOfListP(op->parents);

	FORBOTH_LC(d, nms, duplicateattrs, normalAttrNames)
	{
		// find the lost attribute, if we do not find it, we need to add
		// projection op; or continue upward check.
		fd = FALSE;
		if(isA(o, ProjectionOperator))
		{
			FOREACH(Node,n ,((ProjectionOperator *)o)->projExprs)
            {
				if (isA(n,AttributeReference))
				{
					AttributeReference *a = (AttributeReference *) n;

					if (streq(a->name, nms->data.ptr_value))
					{
						fd = TRUE;
						break;
					}
				}
             }
		}
		else
		{
			FOREACH(AttributeDef,a ,o->schema->attrDefs)
            {
				if (streq(a->attrName, nms->data.ptr_value))
				{
					fd = TRUE;
					break;
				}
             }
		}

		//if not find this attrRef(searched by name), means lost, need add
		if(!fd)
		{
			isLost = TRUE;

			//add d to the list which stores the name of lost attributes
			LostList = appendToTailOfList(LostList, strdup(d->data.ptr_value));
			LostNormalList = appendToTailOfList(LostNormalList, strdup(nms->data.ptr_value));

			//get rid of the attribute from the duplicate list and
			//normalAttrnames
			duplicateattrsCopy = REMOVE_FROM_LIST_PTR(duplicateattrsCopy, d->data.ptr_value);
			normalAttrNamesCopy = REMOVE_FROM_LIST_PTR(normalAttrNamesCopy, nms->data.ptr_value);
		}
		//if find this attrRef(searched by name), means have this attrRef, not need to add, just remove it
		else
		{
			//If not projection op, just get rid of the attrDef from
			//schema. If projection op get rid of the attrDef from schema
			//and attrRef from projExprs

			if(isA(o, ProjectionOperator))
			{
				if(o->parents != NIL && LIST_LENGTH(o->parents) == 1)
				{
					//Get rid of the attrDef from schema and attrRef from projExprs
					int pos = getAttrPos((QueryOperator *)o, LC_P_VAL(d));

					if(pos != -1)
					{
						deleteAttrFromSchemaByName((QueryOperator *)o, LC_P_VAL(d));
						deleteAttrRefFromProjExprs((ProjectionOperator *)o, pos);
					}
				}
				else
				{
					FORBOTH(Node,attrDef, attrRef, o->schema->attrDefs, ((ProjectionOperator *)o)->projExprs)
		            {
						if (isA(attrRef, AttributeReference))
						{
							if(streq(LC_P_VAL(d),((AttributeDef *)attrDef)->attrName))
							{
								((AttributeReference *)(attrRef))->name = LC_P_VAL(nms);
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

		FOREACH(QueryOperator, opChild, o->inputs)
		    		{
			List *projAttrNames = NIL;
			List *projExpr = NIL;

			//e.g. projection
			// A B | PA  -> name   (1.1) copy child's old attr name (1.2) add new prov name from lost list
			// A B | A   -> ref    (2.1) copy child's old attr ref (2.2) add new normal prov name from normal lost list

			// (1.1)
			if(isA(opChild, ProjectionOperator))
				projAttrNames = getAttrRefNames((ProjectionOperator *) opChild);
			else
				projAttrNames = getQueryOperatorAttrNames((QueryOperator *) opChild);

			// (2.1)
			int cnt = 0;
			if(isA(opChild, ProjectionOperator))
			{
				projExpr = copyObject(((ProjectionOperator *)opChild)->projExprs);
				cnt = getNumAttrs(opChild);
			}
			else
			{
				FOREACH(AttributeDef,attrDef,opChild->schema->attrDefs)
				{
					projExpr = appendToTailOfList(projExpr,
							createFullAttrReference(
									attrDef->attrName, 0,
									cnt, 0,
									attrDef->dataType));
					cnt++;
				}
			}

			// (1.2)
                		FOREACH(char, attrName, LostList)
                		projAttrNames = appendToTailOfList(projAttrNames, attrName);

                		// (2.2)
                		List *childType = getDataTypes(opChild->schema);
                		List *childName = getAttrNames(opChild->schema);

                		FOREACH(char, attrName, LostNormalList)
                		{
                			DataType type = DT_INT;
                			char *name = NULL;
                			FORBOTH(Node, t, n, childType, childName)
                			{
                				name = (char *) n;
                				if(streq(name, attrName))
                				{
                					type = (DataType) t;
                					break;
                				}
                			}
                			if(name != NULL)
                			{
                				projExpr = appendToTailOfList(projExpr,
                						createFullAttrReference(
                								name, 0,
                								cnt, 0,
                								type));
                				cnt++;
                			}
                		}

                		List *newProvPosList = NIL;
                		CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

                		//Add projection
                		ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, projAttrNames);
                		newpo->op.provAttrs = newProvPosList;

                		// Switch the subtree with this newly created projection operator.
                		switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

                		// Add child to the newly created projections operator,
                		addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

                		//Reset the pos of the schema
                		resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)newpo,(QueryOperator *)op);
                		resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)o,(QueryOperator *)newpo);

                		if(LIST_LENGTH(o->parents) == 1)
                			pullup(o, duplicateattrsCopy, normalAttrNamesCopy);
		    		}
	}
	else
	{
		resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)o,(QueryOperator *)op);

		if(LIST_LENGTH(o->parents) == 1)
			pullup(o, duplicateattrsCopy, normalAttrNamesCopy);
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
    	resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)newSo1,(QueryOperator *)o1);
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
    	resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)newSo2,(QueryOperator *)o2);
    }
}

QueryOperator *
selectionMoveAround(QueryOperator *root)
{
/*
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
*/

	computeECProp(root);
//	if (TRUE)//TODO remove this once EC is fixed
//	    return root;
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
    SET_BOOL_STRING_PROP(root, PROP_OPT_SELECTION_MOVE_AROUND_DONE);
	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		    if (!HAS_STRING_PROP(op, PROP_OPT_SELECTION_MOVE_AROUND_DONE))
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

					resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)parent,(QueryOperator *)root);
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
            	//deal with case A = PROV_A
            	List *provs = getOpProvenanceAttrNames(op);

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

    	            //deal with case A = PROV_A
    	            if((!searchListString(provs, aRef1->name) && !searchListString(provs, aRef2->name) && !streq(aRef1->name, aRef2->name)) || (searchListString(provs, aRef1->name) && searchListString(provs, aRef2->name) && !streq(aRef1->name, aRef2->name)))
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

	SET_BOOL_STRING_PROP(selectionOp, PROP_OPT_SELECTION_MOVE_AROUND_DONE);

	// Switch the subtree with this newly created projection
	switchSubtrees((QueryOperator *) root, (QueryOperator *) selectionOp);

	// Add child to the newly created projections operator,
	addChildOperator((QueryOperator *) selectionOp, (QueryOperator *) root);

	//set the data type
	setAttrDefDataTypeBasedOnBelowOp((QueryOperator *)selectionOp, (QueryOperator *)root);

	//reset the attr_ref position
	resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)selectionOp,(QueryOperator *)root);
}


QueryOperator *
pushDownAggregationThroughJoin(QueryOperator *root)
{
	//QueryOperator *newRoot = root;
	QueryOperator *child = OP_LCHILD(root);
	QueryOperator *parent = OP_FIRST_PARENT(root);
	//List *opList = NIL;

	if(isA(root, AggregationOperator) && isA(child, JoinOperator))
	{
		boolean cond1 = FALSE;
		boolean cond2 = FALSE;
		boolean cond3 = FALSE;

		AggregationOperator *agg = (AggregationOperator *) root;
		JoinOperator *jOp = (JoinOperator *) child;
		QueryOperator *lChild = OP_LCHILD(jOp);

    	List *lSchemaList = getAttrNames(lChild->schema);
    	Set *lSchemaSet = makeStrSetFromList(lSchemaList);

		/* Condition 1 */
		List *aggList = getAttrReferences((Node *) agg->aggrs);
		Set *aggSet = STRSET();

		FOREACH(AttributeReference, a, aggList)
		    addToSet(aggSet,strdup(a->name));

		List *groupByList = getAttrReferences((Node *) agg->groupBy);
		Set *groupBySet = STRSET();
		FOREACH(AttributeReference, a, groupByList)
		    addToSet(groupBySet,strdup(a->name));

		List *joinCondList = getAttrReferences((Node *) jOp->cond);
		Set *joinCondSet = STRSET();
		FOREACH(AttributeReference, a, joinCondList)
		    addToSet(joinCondSet,strdup(a->name));

		Set *joinUgrouBySet = unionSets(joinCondSet, groupBySet);
		Set *cond1Set = intersectSets(joinUgrouBySet,aggSet);

		//if(EMPTY_SET(cond1Set))
		cond1 = EMPTY_SET(cond1Set) && containsSet(aggSet, lSchemaSet);

		/* Condition 2 */
        Set *joinIgroupBySet = intersectSets(joinCondSet, groupBySet);
        if(setSize(joinIgroupBySet) == setSize(groupBySet) && containsSet(joinIgroupBySet, lSchemaSet))
        	cond2 = TRUE;

        /* Condition 3 */
       // QueryOperator *lChild = OP_LCHILD(jOp);
        QueryOperator *rChild = OP_RCHILD(jOp);

//        if(isA(lChild, TableAccessOperator))
//        {
//        	TableAccessOperator *lRel = (TableAccessOperator *) lChild;
//        	List *lKeyList = getKeyInformation(lRel->tableName);
//        	DEBUG_LOG("left keyList length: %d", LIST_LENGTH(lKeyList));
//        	DEBUG_NODE_BEATIFY_LOG("left keys are:", lKeyList);
//        	DEBUG_LOG("Table operator %s", lChild->schema->name);
//        }


        if(isA(rChild, TableAccessOperator))
        {
        	TableAccessOperator *rRel = (TableAccessOperator *) rChild;
        	List *rKeyList = getKeyInformation(rRel->tableName);

//        	DEBUG_LOG("Right keyList length: %d", LIST_LENGTH(rKeyList));
//        	DEBUG_NODE_BEATIFY_LOG("right keys are:", rKeyList);
//        	DEBUG_LOG("Table operator %s", rChild->schema->name);
//        	DEBUG_LOG("join set length : %d", setSize(joinCondSet));

        	List *rSchemaList = getAttrNames(rRel->op.schema);
        	Set *rSchemaSet = makeStrSetFromList(rSchemaList);
            Set *rJoinCond = intersectSets(joinCondSet,rSchemaSet);

//            FOREACH_SET(char, c, rJoinCond)
//               DEBUG_LOG("right join cond attr : %s", c);

            //check if attribute in join condition is equal to the primary key in right child
            FOREACH(Set, s, rKeyList)
            {
//              DEBUG_LOG("join set size %d, key set size %d", setSize(rJoinCond), setSize(s));
//              DEBUG_LOG("after difference the set size %d", setSize(setDifference(rJoinCond,s)));
                if(setSize(rJoinCond) == setSize(s) && setSize(setDifference(rJoinCond,s)) == 0)
                {
                      cond3 = TRUE;
                      break;
                }
            }

            /* output the value of these 3 condition */
    		DEBUG_LOG("Condition 1 is : %d", cond1);
            DEBUG_LOG("Condition 2 is : %d", cond2);
            DEBUG_LOG("Condition 3 is : %d", cond3);
        }

        List *newGroupByAttrRefs = NIL;
        FOREACH_SET(char, c, joinIgroupBySet)
        {
        	FOREACH(AttributeReference, ar, joinCondList)
		   {
        		if(streq(c,ar->name) && !searchList(newGroupByAttrRefs, ar) && cond2)
        			newGroupByAttrRefs = appendToTailOfList(newGroupByAttrRefs, copyObject(ar));
		   }
        }

        if(cond1 && cond2 && cond3) //CASE 1
        	switchAggregationWithJoinToLeftChild(agg, jOp);
        else if(cond1 && cond3)  //CASE 2
        	addAdditionalAggregationBelowJoin(agg, jOp);
        else if(cond2 && cond3 && isA(parent, ProjectionOperator))   //CASE 3
        	addCountAggregationBelowJoin(agg, jOp, newGroupByAttrRefs);

	}

	FOREACH(QueryOperator, o, root->inputs)
	          pushDownAggregationThroughJoin(o);

	return root;
}

void
addCountAggregationBelowJoin(AggregationOperator *aggOp, JoinOperator *jOp, List *groupByAttrRefs)
{
	/* got the error when introduce new projection
	QueryOperator *lChild = OP_LCHILD(jOp);
	List *lChildAttrDefsNames = getAttrNames(lChild->schema);
	List *newlChildAttrDefsNames = NIL;
	FOREACH(char, c, lChildAttrDefsNames)
		newlChildAttrDefsNames = appendToTailOfList(newlChildAttrDefsNames, strdup(c));
	int pos = 0;
	List *newAttrRef = NIL;
    FOREACH(AttributeDef, ad, lChild->schema->attrDefs)
	{
    	AttributeReference *ar = createFullAttrReference(ad->attrName, 0, pos, 0, ad->dataType);
    	newAttrRef = appendToTailOfList(newAttrRef, ar);
    	pos ++;
	}
    ProjectionOperator *newPrj = createProjectionOp(newAttrRef, lChild, singleton(jOp), newlChildAttrDefsNames);
    jOp->op.inputs = replaceNode(jOp->op.inputs, lChild, newPrj);
    lChild->parents = singleton(newPrj);
    */

	QueryOperator *lChild = OP_LCHILD(jOp);
	QueryOperator *rChild = OP_RCHILD(jOp);

	/* new projection 1,B, 1->*  */
	//projExprs
	List *newProjExprs = NIL;
	List *newProjDefs = NIL;
	List *onlyGroupByAttrDefs = NIL;
	Constant *star =  createConstInt(1);
	newProjExprs = appendToTailOfList(newProjExprs, star);

	AttributeDef *starDef = createAttributeDef("STAR", DT_INT);
	newProjDefs = appendToTailOfList(newProjDefs,starDef);

	FOREACH(AttributeReference, a, groupByAttrRefs)
	{
		newProjExprs = appendToTailOfList(newProjExprs, copyObject(a));
		newProjDefs = appendToTailOfList(newProjDefs, createAttributeDef(strdup(a->name), a->attrType));
		onlyGroupByAttrDefs = appendToTailOfList(onlyGroupByAttrDefs, createAttributeDef(strdup(a->name), a->attrType));
	}

	//projSchema
	ProjectionOperator *newProj = (ProjectionOperator *) copyObject(getHeadOfListP(aggOp->op.parents));

	newProj->projExprs = newProjExprs;
	newProj->op.schema->attrDefs = newProjDefs;

	newProj->op.inputs = singleton(lChild);
	lChild->parents = singleton(newProj);

	/* new count aggregation op */
	//aggrs
	AttributeReference *starAttRef = createAttributeReference("STAR");
	starAttRef->attrType = DT_INT;

	//groupBy

	AggregationOperator *newAgg =  (AggregationOperator *) copyObject(aggOp);
	jOp->op.inputs = replaceNode(jOp->op.inputs, lChild, newAgg);
	newAgg->op.parents = singleton(jOp);
	newAgg->op.inputs = singleton(newProj);
	newProj->op.parents = singleton(newAgg);

	FunctionCall *f = getHeadOfListP(newAgg->aggrs);
	f->functionname = "COUNT";
	AttributeReference *ar = getHeadOfListP(f->args);
	ar->name = "STAR";

	List *newAggDefs = NIL;
	AttributeDef *ad1 = createAttributeDef("OCNT",DT_INT);
	newAggDefs = appendToTailOfList(newAggDefs, ad1);

	FOREACH(AttributeReference, ar, groupByAttrRefs)
		newAggDefs = appendToTailOfList(newAggDefs, createAttributeDef(strdup(ar->name),ar->attrType));
	newAgg->op.schema->attrDefs = newAggDefs;

	List *newJOpDefs = concatTwoLists(copyList(newAggDefs), copyList(rChild->schema->attrDefs));
	jOp->op.schema->attrDefs = newJOpDefs;

	resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) jOp, (QueryOperator *) newAgg);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) newAgg, (QueryOperator *) newProj);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) newProj, (QueryOperator *) lChild);


    //proj
	ProjectionOperator *topProj = (ProjectionOperator *) OP_FIRST_PARENT(aggOp);//(ProjectionOperator *) copyObject(getHeadOfListP(aggOp->op.parents));
    topProj->op.inputs = singleton(jOp);
    jOp->op.parents  = singleton(topProj);

    //the operator ocnt * D
    List *topProjNewAttrRefs = NIL;
    List *operatorList = NIL;
    AttributeReference *ocntAttrRef = createFullAttrReference(ad1->attrName, 0, 0, 0, ad1->dataType);
    List *oldAggAttrRefList = getAttrReferences((Node *) aggOp->aggrs);
    AttributeReference *oldAggAttrRef = getHeadOfListP(oldAggAttrRefList);
    operatorList = appendToTailOfList(operatorList, ocntAttrRef);
    operatorList = appendToTailOfList(operatorList, copyObject(oldAggAttrRef));
    Operator *st = createOpExpr("*",operatorList);
    topProjNewAttrRefs =  appendToTailOfList(topProjNewAttrRefs, st);
    topProjNewAttrRefs = concatTwoLists(topProjNewAttrRefs, copyList(groupByAttrRefs));
    topProj->projExprs = topProjNewAttrRefs;

    FORBOTH(Node, a1, a2, topProj->projExprs, topProj->op.schema->attrDefs)
    {
    	if(isA(a1, AttributeReference) && isA(a2, AttributeDef))
    		((AttributeDef *) a2)->dataType = ((AttributeReference *) a1)->attrType;
    	else if(isA(a1, Operator) && isA(a2, AttributeDef))
    	{
    		FOREACH(Node, n, getAttrReferences(a1))
		    {
    			if(isA(n, AttributeReference))
    			{
    				((AttributeDef *) a2)->dataType = ((AttributeReference *) n)->attrType;
    				break;
    			}
		    }
    	}
    }

    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) topProj, (QueryOperator *) jOp);
}

void
addAdditionalAggregationBelowJoin(AggregationOperator *aggOp, JoinOperator *jOp)
{
	QueryOperator *lChild = OP_LCHILD(jOp);

	//copy agg
	AggregationOperator *newAgg = (AggregationOperator*) copyObject(aggOp);

	//reset parents and children
    newAgg->op.parents = singleton(jOp);
    newAgg->op.inputs = singleton(lChild);

	jOp->op.inputs = replaceNode(jOp->op.inputs, lChild, newAgg);
	lChild->parents = singleton(newAgg);

    //group by
	Node *cond = ((JoinOperator *)jOp)->cond;
	List *condAttrRefs = getAttrReferences(cond);
	List *childSchemaList = getAttrNames(lChild->schema);

    newAgg->groupBy = NIL;
    FOREACH(AttributeReference, a, condAttrRefs)
    {
    	if(searchListString(childSchemaList, a->name))
    		newAgg->groupBy = appendToTailOfList(newAgg->groupBy, copyObject(a));
    }

    //agg
    List *newAggSchemaDefs = copyObject(lChild->schema->attrDefs);
    newAgg->op.schema->attrDefs = newAggSchemaDefs;

    //reset position
	resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) newAgg, lChild);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) jOp, (QueryOperator *) newAgg);

}

void
switchAggregationWithJoinToLeftChild(AggregationOperator *aggOp, JoinOperator *jOp)
{
	QueryOperator *pAgg = OP_FIRST_PARENT(aggOp);
	QueryOperator *lChild = OP_LCHILD(jOp);

    // set join
	jOp->op.parents = aggOp->op.parents;
	pAgg->inputs = singleton(jOp);
	jOp->op.inputs = replaceNode(jOp->op.inputs, lChild, aggOp);

    // Set aggregation
	aggOp->op.parents = singleton(jOp);
	aggOp->op.inputs = singleton(lChild);
	lChild->parents = singleton(aggOp);

    /* adapt attribute name*/
	List *aggrs = getAttrReferences((Node *) aggOp->aggrs);
	List *groupBy = getAttrReferences((Node *) aggOp->groupBy);
	List *aggOpAttrRefs = concatTwoLists(aggrs, groupBy);
	List *aggOpAttrDefs = aggOp->op.schema->attrDefs;


	Node *cond = ((JoinOperator *)jOp)->cond;
	List *condAttrRefs = getAttrReferences(cond);
	List *joinAttrDefs = jOp->op.schema->attrDefs;

    FOREACH(AttributeReference, c, condAttrRefs)
	{
          FORBOTH(Node, a, s, aggOpAttrRefs, aggOpAttrDefs)
		  {
        	   AttributeReference *ar = (AttributeReference *) a;
        	   AttributeDef *ad = (AttributeDef *) s;
        	   DEBUG_LOG("\n %s, %s, %s  \n", c->name, ar->name, ad->attrName);
               if(streq(c->name,ar->name))
               {
            	   c->attrType = ad->dataType;
            	   c->name = ad->attrName;
               }
		  }
	}

    FOREACH(AttributeDef, d, joinAttrDefs)
    {
    	FORBOTH(Node, a, s, aggOpAttrRefs, aggOpAttrDefs)
    	{
    		AttributeReference *ar = (AttributeReference *) a;
    		AttributeDef *ad = (AttributeDef *) s;
    		DEBUG_LOG("\n %s, %s, %s  \n", d->attrName, ar->name, ad->attrName);
    		if(streq(d->attrName,ar->name))
    		{
    			d->dataType = ad->dataType;
    			d->attrName = ad->attrName;
    		}
    	}
    }

    //adapt attribute position
	resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) aggOp, lChild);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) jOp, (QueryOperator *) aggOp);
    resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *) pAgg, (QueryOperator *) jOp);

}
