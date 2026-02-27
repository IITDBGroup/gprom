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
#include "exception/exception.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "log/logger.h"
#include "operator_optimizer/operator_optimizer.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "operator_optimizer/operator_merge.h"
#include "operator_optimizer/expr_attr_factor.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/schema_utility.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_schema.h"
#include "provenance_rewriter/prov_utility.h"
#include "rewriter.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "model/set/set.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "metadata_lookup/metadata_lookup.h"
#include <stdint.h>

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

typedef struct PullUpProvProjContext {
    Set *allprov; // all provenance attribute names from our branch
    Set *remainingProvAttr; // provenance attributes that have not been handled yet (they need to be removed from current schema)
    Set *renamedRemainingAttr; // remaining normal attributes (their names in the current schema) whose provenance versions have not been added yet
    Set *handledAttr; // provenance attribute names that have been handled (they are part of the schema and continue to be propagated)
    HashMap *remainingAttrToOriginalAttr; // map from current name of an original attribute and the provenance attribute we want to create from it
    Set *otherBranchProvAttrs; // provenance attributes from other branch that need to be kept
} PullUpProvProjContext;

static QueryOperator *optimizeOneGraph (QueryOperator *root);
static char *pullupProvProjToString(PullUpProvProjContext *context);
static Set *getProvenanceAttrsFromOtherBranches(QueryOperator *op, PullUpProvProjContext *context);
static List *getAndSetOriginalAttrLists(QueryOperator *op);
static QueryOperator *pullup(QueryOperator *op, PullUpProvProjContext *context); //List *duplicateattrs, List *normalAttrNames);
static void
removeRemainingProvenanceAttributes(QueryOperator *op, PullUpProvProjContext *context);
static void sortProjProvenanceAttrs(ProjectionOperator *p);
static void keepTrackOfRemainingAttributeRenaming(QueryOperator *p, PullUpProvProjContext *context);
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

	// remove unncessary attributes from children first
	if(root->inputs != NULL)
    {
        FOREACH(QueryOperator, op, root->inputs)
        {
            if(!HAS_STRING_PROP(op, PROP_OPT_UNNECESSARY_COLS_REMOVED_DONE))
            {
                removeUnnecessaryColumnsFromProjections(op);
            }
        }
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
        root->schema->attrDefs = copyObject (OP_LCHILD(root)->schema->attrDefs);
        resetPosInExprs(((SelectionOperator *)root)->cond, cSchema);
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
	}

	else if(isA(root, AggregationOperator))
	{
		/*
		 * Reset the attributeReference pos in Group By and Aggrs
		 */

		AggregationOperator *agg = (AggregationOperator *)root;
        List *newattrdef = NIL;
        List *newaggfs = NIL;
        List *aggfdefs = aggOpGetAggAttrDefs(agg);
        List *groupbyattrdefs = aggOpGetGroupByAttrDefs(agg);

        // remove unnecessary aggregation functions
        FORBOTH(void,f,d,agg->aggrs,aggfdefs)
        {
            char *aggname = ((AttributeDef *) d)->attrName;
            if(hasSetElem(icols, aggname))
            {
                newattrdef = appendToTailOfList(newattrdef, d);
                newaggfs = appendToTailOfList(newaggfs, f);
            }
        }

        // if no group-by and no aggregation function, we have to keep some agg
        // function
        if(LIST_LENGTH(agg->groupBy) == 0 && LIST_LENGTH(newaggfs) == 0)
        {
            newattrdef = singleton(getHeadOfList(agg->op.schema->attrDefs));
            newaggfs = singleton(getHeadOfList(agg->aggrs));
        }

        // update operator
        newattrdef = CONCAT_LISTS(newattrdef, groupbyattrdefs);
        agg->aggrs = newaggfs;
        agg->op.schema->attrDefs = newattrdef;

		resetPosInExprs((Node *) agg->aggrs, cSchema);
		resetPosInExprs((Node *) agg->groupBy, cSchema);
	}

	else if(isA(root, JoinOperator))
	{
		JoinOperator *j = (JoinOperator *) root;

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

		// we may have un-uniquified attribute names, so uniqify them again if necessary
		if(!checkUniqueAttrNames(root))
        {
            makeAttrNamesUnique(root);
            DEBUG_OP_LOG("join or projection attributes are not unique", root);
        }


		//Set cond attr ref pos
		if(j->cond != NULL)
		{
		    //DONE: TODO fix this only works in a very simplistic case. In general we need to split list of attr refs into left and right input refs
			List *attrRefs = getAttrReferences (j->cond);
			List *rcSchema = OP_RCHILD(root)->schema->attrDefs;

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
		int numicols = setSize(icols);
		int numAttrs = getNumAttrs(root);
		ProjectionOperator *proj = (ProjectionOperator *) root;
		List *newAttrDefs = NIL;
		List *newAttrRefs = NIL;

		if(numicols < numAttrs)
		{
            // projection with no required attributes, remove this projection
            if (numicols == 0)
            {
                QueryOperator *child = OP_LCHILD(root);

                switchSubtreeWithExisting(root, child);

                return child;
            }
            else
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
		}
		else
		{
	     	QueryOperator *child = (QueryOperator *)OP_LCHILD(root);
	        resetPosOfAttrRefBaseOnBelowLayerSchema((QueryOperator *)root,(QueryOperator *)child);
		}
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

                // currently we only support pulling up if the operator has one
                // parent, also if this is the last operator (no parents) then
                // we are done for pulling up in this branch
                if(GET_BOOL_STRING_PROP(o, PROP_PROJ_PROV_ATTR_DUP) && LIST_LENGTH(o->parents) == 1)
                {
                    PullUpProvProjContext context;
                    //Get the attrReference of the provenance attribute
                    List *provAttrRefs = getProvenanceAttrReferences(op, o);
                    List *provAttrNames = getOpProvenanceAttrNames(o);
                    List *origAttrNames = NIL;
                    HashMap *normalToProv = NEW_MAP(Constant, Constant);
                    HashMap *provToNormal = NEW_MAP(Constant, Constant);
                    HashMap *remainingToOriginal = NEW_MAP(Constant, Constant);

                    // create list of original attributes corresponding to provenance attributes
                    FOREACH(AttributeReference,ar,provAttrRefs)
                    {
                        origAttrNames = appendToTailOfList(origAttrNames, strdup(ar->name));
                    }

                    /* context.normalToProvName = normalToProv; */
                    /* context.provAttrNames = provAttrNames; */
                    context.allprov = makeStrSetFromList(provAttrNames);
                    context.renamedRemainingAttr = makeStrSetFromList(origAttrNames);
                    context.remainingProvAttr = makeStrSetFromList(provAttrNames);
                    context.handledAttr = STRSET();
                    context.remainingAttrToOriginalAttr = remainingToOriginal;
                    context.otherBranchProvAttrs = STRSET();

                    // map attr -> provattr and provattr -> attr
                    FORBOTH(char,n,p,origAttrNames,provAttrNames)
                    {
                        MAP_ADD_STRING_KEY_AND_VAL(provToNormal,p,n);
                        MAP_ADD_STRING_KEY_AND_VAL(normalToProv,p,n);
                        MAP_ADD_STRING_KEY_AND_VAL(remainingToOriginal, n, p);
                    }

                    // original attribute list
                    getAndSetOriginalAttrLists((QueryOperator *) op);

                    // remove all provenance projections and then pull them up as far as possible
                    op->projExprs = getNormalAttrProjExprsFromChild(op, (QueryOperator *) op);
                    op->op.schema->attrDefs = getNormalAttrs((QueryOperator *) op);
                    op->op.provAttrs = NIL;

                    DEBUG_OP_LOG("adjusted provenance projection", o);
                    DEBUG_LOG("Context:\n%s", pullupProvProjToString(&context));

                    pullup(OP_LCHILD(o), &context);
                }
            }
            pullingUpProvenanceProjections(o);
        }
    }

    // need to sort provenance attributes to restore original order
    if(root->parents == NIL && root->provAttrs != NIL)
    {
        ProjectionOperator *p;

        // if root is not a projection, then need to add one to reorder
        if(!isA(root,ProjectionOperator))
        {
            p = (ProjectionOperator *) createProjOnAllAttrs(root);
            p->op.properties = copyObject(root->properties);
            root = (QueryOperator *) p;
        }
        p = (ProjectionOperator *) root;
        sortProjProvenanceAttrs(p);
    }

    return root;
}

static char *
pullupProvProjToString(PullUpProvProjContext *context)
{
    StringInfo s = makeStringInfo();
    StringInfo remainorig = makeStringInfo();

    appendStringInfoString(remainorig, "{");
    FOREACH_HASH_ENTRY(kv, context->remainingAttrToOriginalAttr)
    {
        appendStringInfo(remainorig, "%s -> %s%s",
                         STRING_VALUE(kv->key),
                         STRING_VALUE(kv->value),
                         FOREACH_HASH_HAS_MORE(kv) ? ",": "");
    }
    appendStringInfoString(remainorig, "}");

    appendStringInfo(s,
                     "remainingAttr: %s\nremainingProvAttr: %s\nremainingToOriginal: %s\nhandledProvAttrs: %s\nallprov: %s\nprov attrs from different branch: %s\n",
                     nodeToString(context->renamedRemainingAttr),
                     nodeToString(context->remainingProvAttr),
                     remainorig->data,
                     nodeToString(context->handledAttr),
                     nodeToString(context->allprov),
                     nodeToString(context->otherBranchProvAttrs));

    return s->data;
}

static Set *
getProvenanceAttrsFromOtherBranches(QueryOperator *op, PullUpProvProjContext *context)
{
    List *provAttrNames = getOpProvenanceAttrNames(op);
    Set *result = STRSET();

    FOREACH(char,a,provAttrNames)
    {
        if(!hasSetElem(context->allprov, a))
        {
            addToSet(result, a);
        }
    }

    return result;
}

static List *
getAndSetOriginalAttrLists(QueryOperator *op)
{
    List *result = NIL;

    if(!HAS_STRING_PROP(op, PROP_ORIGINAL_ATTR_LIST))
    {
        result = stringListToConstList(getQueryOperatorAttrNames(op));
        SET_STRING_PROP(op,
                        PROP_ORIGINAL_ATTR_LIST,
                        result);
        SET_STRING_PROP(op,
                        PROP_ORIGINAL_PROV_SET,
                        makeStrSetFromList(getOpProvenanceAttrNames(op)));
    }
    else
    {
        result = constStringListToStringList((List *) GET_STRING_PROP(op,
                                                                      PROP_ORIGINAL_ATTR_LIST));
    }

    return result;
}

/*
 * duplicateattrs stores attrDef name of provenance attribute, normalAttrnames
 * store attrRef name of provenance attribute
 */
QueryOperator *
pullup(QueryOperator *op, PullUpProvProjContext *context) // List *duplicateattrs, List *normalAttrNames)
{
	boolean isLost= FALSE;
    Set *lostAttrs = STRSET();
    QueryOperator *p = OP_FIRST_PARENT(op);
    List *parentAttrNames = NIL;

    ASSERT(checkModel(op));

    INFO_LOG("pullup prov projection:\n   op: %s\n   parent: %s",
             singleOperatorToOverview(op),
             singleOperatorToOverview(p));

    // keep original list of provenance attributes for ordering
    getAndSetOriginalAttrLists(p);

    // keep track of provenance attributes coming from a separate branch
    unionIntoSet(context->otherBranchProvAttrs,
                 getProvenanceAttrsFromOtherBranches(p, context));

    //TODO iterate over all parents or do not try to push if more than one parent

    // is parent a root operator need to treat all attribute as lost to ensure
    // we get the provenance attributes in the result schema
    if(p->parents == NIL)
    {
        isLost = TRUE;
        lostAttrs = copyObject(context->renamedRemainingAttr);
    }
    else
    {
        // get attribute names of parent
        if(isA(p, ProjectionOperator))
        {
            FOREACH(Node,n, ((ProjectionOperator *) p)->projExprs)
            {
			    if (isA(n,AttributeReference))
			    {
				    AttributeReference *a = (AttributeReference *) n;
                    parentAttrNames = appendToTailOfList(parentAttrNames, a->name);
			    }
            }
        }
        // nonprojection
        else //TODO handle join renaming
        {
            parentAttrNames = getQueryOperatorAttrNames(p);
        }

        // find attributes that are lost
        FOREACH_SET(char, a, context->renamedRemainingAttr)
        {
            if(!searchListString(parentAttrNames, a))
            {
                addToSet(lostAttrs, a);
                isLost = TRUE;
            }
        }
    }
    // handle lost attributes, add projection (or adjust projection), adjust context, and continue to parent
    if(isLost)
    {
        DEBUG_LOG("lost attributes: %s\n\ncontext:\n%s",
                  nodeToString(lostAttrs),
                  pullupProvProjToString(context));
        DEBUG_SINGLE_OP_LOG("operator with lost attributes is", p);
        Set *handledOrOther = unionSets(context->handledAttr, context->otherBranchProvAttrs);

        // for projections do not introduce a new projection operator, but add new provenance projections
        if(isA(p,ProjectionOperator))
        {
            ProjectionOperator *po = (ProjectionOperator *) p;
            List *newproj = getNormalAttrProjExprs(po);
            List *newAttrDefs = getNormalAttrs(p);
            List *provAttrs = NIL;
            int pos = LIST_LENGTH(newproj);

            DEBUG_LOG("Adjust projection for lost provenance attributes:\n%s\n\nlost attrs: %s",
                      singleOperatorToOverview(p),
                      nodeToString(handledOrOther));

            // handled provenance attributes should exist in the input
            FOREACH_SET(char,a,context->handledAttr)
            {
                char *provAttr = a;
                AttributeReference *ar = (AttributeReference *) getAttrRefByName(op, strdup(provAttr));
                AttributeDef *ad = createAttributeDef(strdup(provAttr), typeOf((Node *) ar));
                provAttrs = appendToTailOfListInt(provAttrs, pos);
                newproj = appendToTailOfList(newproj, ar);
                newAttrDefs = appendToTailOfList(newAttrDefs, ad);
                pos++;
            }

            // add other provenance attributes which may be renamed input
            // attributes, so we find them from their positions in the
            // projections's schema
            FOREACH_SET(char,a,context->otherBranchProvAttrs)
            {
                int ppos = getAttrPos(p, a);
                AttributeDef *ad;
                Node *pexpr;

                ASSERT(pos != -1);
                ad = copyObject(getAttrDefByPos(p, ppos));
                pexpr = copyObject(getNthOfListP(po->projExprs, ppos));
                provAttrs = appendToTailOfListInt(provAttrs, pos);
                newproj = appendToTailOfList(newproj, pexpr);
                newAttrDefs = appendToTailOfList(newAttrDefs, ad);
                pos++;
            }

            // add projections for lost attributes
            FOREACH_SET(char,l,lostAttrs)
            {
                char *aName = strdup(l);
                char *provName = MAP_GET_STRING_VAL_FOR_STRING_KEY(context->remainingAttrToOriginalAttr, aName);
                Node *projExpr = (Node *) getAttrRefByName(op, aName);
                AttributeDef *ad = createAttributeDef(strdup(provName), typeOf(projExpr));

                newproj = appendToTailOfList(newproj, projExpr);
                newAttrDefs = appendToTailOfList(newAttrDefs, ad);
                provAttrs = appendToTailOfListInt(provAttrs, pos);
                pos++;

                // adjust context
                addToSet(context->handledAttr, strdup(provName));
                removeSetElem(context->remainingProvAttr, provName);
                removeSetElem(context->renamedRemainingAttr, aName);
                removeMapStringElem(context->remainingAttrToOriginalAttr, aName);
            }

            DEBUG_LOG("Adjust projection:\nprojExprs: %s\nprovAttrs: %s\nschema:%s",
                      exprToSQL((Node *) newproj, NULL, FALSE),
                      nodeToString(provAttrs),
                      nodeToString(newAttrDefs));

            po->projExprs = newproj;
            p->provAttrs = provAttrs;
            p->schema->attrDefs = newAttrDefs;

            DEBUG_SINGLE_OP_LOG("After add projection expressions for lost attributes:\n", p);
            DEBUG_LOG("new context:\n%s", pullupProvProjToString(context));
        }
        // otherwise we have to add a projection below to add the lost attribute's provenance version
        else
        {
            // add projection below and process this operator again by calling ourselves on projection operator
            ProjectionOperator *pr = NULL;
            List *inattrs = NIL;
            List *outattrs = NIL;

            inattrs = getNormalAttrNames(op);

            FOREACH_SET(char,c,handledOrOther)
            {
                inattrs = appendToTailOfList(inattrs, strdup(c));
            }
            outattrs = deepCopyStringList(inattrs);

            FOREACH_SET(char,l,lostAttrs)
            {
                char *aName = l;
                char *provName = MAP_GET_STRING_VAL_FOR_STRING_KEY(context->remainingAttrToOriginalAttr, aName);

                inattrs = appendToTailOfList(inattrs, aName);
                outattrs = appendToTailOfList(outattrs, provName);

                // adjust context
                addToSet(context->handledAttr, provName);
                removeSetElem(context->remainingProvAttr, provName);
                removeSetElem(context->renamedRemainingAttr, aName);
                removeMapStringElem(context->remainingAttrToOriginalAttr, aName);
            }

            pr = (ProjectionOperator *) createProjOnAttrsByName(op, inattrs, outattrs);

            pr->op.inputs = singleton(op);
            pr->op.parents = singleton(p);

            substOpInParentList(op, p, (QueryOperator *) pr);
            substOpInInputs(p, op, (QueryOperator *) pr);

            // need to copy original provenance attr list
            pr->op.properties = copyObject(op->properties);

            DEBUG_LOG("lost attributes at operator, add projection and process it:\nop: %s\nproj: %s\nparent: %s",
                      singleOperatorToOverview(op),
                      singleOperatorToOverview(pr),
                      singleOperatorToOverview(p));

            resetPosOfAttrRefBaseOnBelowLayerSchema(p, (QueryOperator *) pr);
            return pullup((QueryOperator *) pr, context);
        }

        // if all attributes handled, then we are done and should return
        //TODO
    }
    // no attributes lost handle renaming, remove remaining provenance attritbutes and adjust prov attr list of operator, and continue to parent
    else
    {
        DEBUG_SINGLE_OP_LOG("no provenance attributes are lost at operator", p);

        // remove provenance attributes that are not handled yet from schema
        removeRemainingProvenanceAttributes(p, context);

        // keep track of renaming of normal attributes whose provenance version remains unhandled (only for projection and join)
        keepTrackOfRemainingAttributeRenaming(p, context);
    }

    // continue to parent
    resetPosOfAttrRefBaseOnBelowLayerSchema(p, op);

    ASSERT(checkModel(op));

    if(p->parents != NIL)
    {
        return pullup(p, context);
    }
    else
    {
        return p;
    }
}

static void
sortProjProvenanceAttrs(ProjectionOperator *p)
{
    QueryOperator *op = (QueryOperator *) p;
    List *tempproj = NIL;
    List *tempProvAttrs = NIL;
    List *tempAttrDefs = NIL;
    Set *targetProvAttrs;
    List *targetAttrList;
    List *currentAttrNames = getQueryOperatorAttrNames(op);
    int pos = 0;

    DEBUG_OP_LOG("sort provenance attributes in final projection operator", op);

    ASSERT(HAS_STRING_PROP(p, PROP_ORIGINAL_ATTR_LIST));
    targetProvAttrs = (Set *) GET_STRING_PROP(p, PROP_ORIGINAL_PROV_SET);
    targetAttrList = constStringListToStringList((List *) GET_STRING_PROP(p, PROP_ORIGINAL_ATTR_LIST));

    // sort projection expression
    FOREACH(char,attr,targetAttrList)
    {
        int curPos = listPosString(currentAttrNames, attr);

        // have this attribute
        if(curPos != -1)
        {
            tempproj = appendToTailOfList(tempproj, getNthOfListP(p->projExprs, curPos));
            tempAttrDefs = appendToTailOfList(tempAttrDefs, getNthOfListP(op->schema->attrDefs, curPos));

            if(hasSetElem(targetProvAttrs, attr))
            {
                tempProvAttrs = appendToTailOfListInt(tempProvAttrs, pos);
            }
            pos++;
        }
    }

    p->projExprs = tempproj;
    op->schema->attrDefs = tempAttrDefs;
    op->provAttrs = tempProvAttrs;

    DEBUG_SINGLE_OP_LOG("this is a root operator, have sorted attributes correctly",
                        p);
}

static void
keepTrackOfRemainingAttributeRenaming(QueryOperator *op, PullUpProvProjContext *context)
{
    if(isA(op, ProjectionOperator))
    {
        ProjectionOperator *pr = (ProjectionOperator *) op;
        HashMap *newNames = NEW_MAP(Constant,Constant);
        Set *remainingAttr = STRSET();

        FORBOTH(Node, pexpr, a, pr->projExprs, op->schema->attrDefs)
        {
            AttributeDef *ad = (AttributeDef *) a;
            if(isA(pexpr,AttributeReference))
            {
                AttributeReference *ar = (AttributeReference *) pexpr;

                if(hasSetElem(context->renamedRemainingAttr, ar->name))
                {
                    MAP_ADD_STRING_KEY_AND_VAL(newNames,
                                               ad->attrName,
                                               MAP_GET_STRING_VAL_FOR_STRING_KEY(context->remainingAttrToOriginalAttr,
                                                                                 ar->name));
                    addToSet(remainingAttr, ad->attrName);
                }
            }
        }

        context->renamedRemainingAttr = remainingAttr;
        context->remainingAttrToOriginalAttr = newNames;

        DEBUG_LOG("projection renamed provenance attributes:\n\n%s",
                  pullupProvProjToString(context));
    }
    if (isA(op, JoinOperator))
    {
        HashMap *newNames = NEW_MAP(Constant,Constant);
        Set *remainingAttr = STRSET();
        List *inputAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(OP_LCHILD(op)),
                                            getQueryOperatorAttrNames(OP_RCHILD(op)));
        List *outputAttrNames = getQueryOperatorAttrNames(op);

        FORBOTH(char,inname,outname,inputAttrNames, outputAttrNames)
        {
            if(hasSetElem(context->renamedRemainingAttr, inname))
            {
                MAP_ADD_STRING_KEY_AND_VAL(newNames,
                                           outname,
                                           MAP_GET_STRING_VAL_FOR_STRING_KEY(context->remainingAttrToOriginalAttr,
                                                                             inname));
                addToSet(remainingAttr, outname);
            }
        }

        context->renamedRemainingAttr = remainingAttr;
        context->remainingAttrToOriginalAttr = newNames;

        DEBUG_LOG("join renamed provenance attributes:\n\n%s",
                  pullupProvProjToString(context));
    }
}

static void
removeRemainingProvenanceAttributes(QueryOperator *op, PullUpProvProjContext *context)
{
    // if operator is projection also remove remaining attributes from projection expressions
    if(isA(op, ProjectionOperator))
    {
        ProjectionOperator *p = (ProjectionOperator *) op;
        List *newprojExprs = NIL;

        FOREACH(Node,n,p->projExprs)
        {
            if(isA(n,AttributeReference))
            {
                AttributeReference *a = (AttributeReference *) n;
                if(!hasSetElem(context->remainingProvAttr, a->name)
                   || hasSetElem(context->otherBranchProvAttrs, a->name))
                {
                    newprojExprs = appendToTailOfList(newprojExprs, n);
                }
            }
            else
            {
                newprojExprs = appendToTailOfList(newprojExprs, n);
            }
        }

        p->projExprs = newprojExprs;
    }
    // for join we may get renaming, but we never rename provenance attributes

    List *newschema = NIL;
    List *newprov = NIL;
    int pos = 0;

    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        if(!hasSetElem(context->remainingProvAttr, a->attrName))
        {
            newschema = appendToTailOfList(newschema, a);
            if(hasSetElem(context->handledAttr, a->attrName)
               || hasSetElem(context->otherBranchProvAttrs, a->attrName))
            {
                newprov = appendToTailOfListInt(newprov, pos);
            }
            pos++;
        }
    }

    op->schema->attrDefs = newschema;
    op->provAttrs = newprov;

    DEBUG_SINGLE_OP_LOG("After removing unhandled provenance attributes: ", op);
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
			if(!streq(op->name, OPNAME_EQ))
				newCondList = appendToTailOfList(newCondList, copyObject(op));
			if(streq(op->name, OPNAME_EQ))
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
			if(!streq(op->name, OPNAME_EQ))
				newCondList = appendToTailOfList(newCondList, copyObject(op));
			if(streq(op->name, OPNAME_EQ))
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
	            Operator *o = createOpExpr(OPNAME_EQ, LIST_MAKE(aRef, copyObject(c)));
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
    	            Operator *o = createOpExpr(OPNAME_EQ, LIST_MAKE(aRef1, aRef2));

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

    selectionOp->op.provAttrs = copyList(root->provAttrs);

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
