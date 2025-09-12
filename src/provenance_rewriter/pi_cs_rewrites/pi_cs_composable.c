/*-----------------------------------------------------------------------------
 *
 * pi_cs_composable.c
 *
 *
 *        AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "log/logger.h"
#include "configuration/option.h"

#include "mem_manager/mem_mgr.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/prov_schema.h"
#include "provenance_rewriter/prov_utility.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "provenance_rewriter/semiring_combiner/sc_main.h"
#include "utility/string_utils.h"

// result tuple-id attribute and provenance duplicate counter attribute
#define RESULT_TID_ATTR backendifyIdentifier("_result_tid")
#define PROV_DUPL_COUNT_ATTR backendifyIdentifier("_setprov_dup_count")

#define REWR_NULLARY_SETUP_PIC(optype)			\
	REWR_NULLARY_SETUP(PICS-Composable,optype)

#define REWR_UNARY_SETUP_PIC(optype)			\
	REWR_UNARY_SETUP(PICS-Composable,optype)

#define REWR_BINARY_SETUP_PIC(optype)			\
	REWR_BINARY_SETUP(PICS-Composable,optype)

#define REWR_UNARY_CHILD_PIC()							\
	REWR_UNARY_CHILD(rewritePI_CSComposableOperator)

#define REWR_BINARY_CHILDREN_PIC()							\
	REWR_BINARY_CHILDREN(rewritePI_CSComposableOperator)

// data structures
/* static Node *asOf; */
//static RelCount *nameState; //TODO replace with PICSRewriteState

typedef struct PICSComposableRewriteState {
    HashMap *opToRewrittenOp; // mapping op address to address of rewritten operator
	HashMap *origOps; // mapping op to address of a copies of the original query graph for reuse (e.g., aggregation with join)
	HashMap *provCounts; // map from tablename / prov prefix -> count
	DataType rowNumDT;
	Node *asOf;
} PICSComposableRewriteState;

// static methods
static boolean isTupleAtATimeSubtree(QueryOperator *op);

static QueryOperator *rewritePI_CSComposableOperator (QueryOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableUseProvNoRewrite (QueryOperator *op, List *userProvAttrs, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableAddProvNoRewrite (QueryOperator *op, List *userProvAttrs, PICSComposableRewriteState *state);
static QueryOperator *composableAddUserProvenanceAttributes (QueryOperator *op, List *userProvAttrs, boolean showIntermediate, PICSComposableRewriteState *state);
static QueryOperator *composableAddIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs, PICSComposableRewriteState *state);

static QueryOperator *rewritePI_CSComposableSelection (SelectionOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableProjection (ProjectionOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableJoin (JoinOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableAggregationWithWindow (AggregationOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableAggregationWithJoin (AggregationOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableSet (SetOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableTableAccess(TableAccessOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableConstRel(ConstRelOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableDuplicateRemOp(DuplicateRemoval *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableOrderOp(OrderOperator *op, PICSComposableRewriteState *state);
static QueryOperator *rewritePI_CSComposableLimitOp(LimitOperator *op, PICSComposableRewriteState *state);

static QueryOperator *rewritePI_CSComposableReuseRewrittenOp(QueryOperator *op, PICSComposableRewriteState *state);

static DataType getRowNumDT();
static Constant *getOneForRowNum();
static void addResultTIDAndProvDupAttrs (QueryOperator *op, boolean addToSchema);
static void addChildResultTIDAndProvDupAttrsToSchema (QueryOperator *op);
static List *getResultTidAndProvDupAttrsProjExprs(QueryOperator * op);
static void addNormalAttrsWithoutSpecialToSchema(QueryOperator *target, QueryOperator *source);
static List *getAttrNamesWithoutSpecial(QueryOperator *op);
//static List *getNormalAttrNamesWithoutSpecial(QueryOperator *op);
static List *getAllAttrWithoutSpecial(QueryOperator *op);
static List *getNormalAttrWithoutSpecial(QueryOperator *op);
static List *removeSpecialAttrsFromNormalProjectionExprs(List *projExpr);
static void aggCreateParitionAndOrderBy(AggregationOperator* op,
        List** partitionBy, List** orderBy);

static Node *replaceAttrWithCaseForProvDupRemoval (FunctionCall *f, Node *provDupAttrRef);


/*
 *
 */
QueryOperator *
rewritePI_CSComposable (ProvenanceComputation *op)
{
    QueryOperator *rewRoot;
    PICSComposableRewriteState *state = NEW(PICSComposableRewriteState);

    state->opToRewrittenOp = NEW_MAP(Constant,Constant);
    state->origOps = NEW_MAP(Constant,Constant);
	state->provCounts = NEW_MAP(Constant,Constant);
	state->rowNumDT = getRowNumDT();

    rewRoot = OP_LCHILD(op);
    rewRoot = rewritePI_CSComposableOperator(rewRoot, state);
	switchSubtreeWithExisting((QueryOperator *) op, rewRoot);

    return (QueryOperator *) rewRoot;
}

/*
 * Figure out whether for a certain operator there will be at most one provenance tuple
 * per original result tuple. This can be used to use ROWNUM instead of window functions.
 * Store result as property to avoid recomputation.
 */
static boolean
isTupleAtATimeSubtree(QueryOperator *op)
{
    if (HAS_STRING_PROP(op, PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME))
        return GET_BOOL_STRING_PROP(op, PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);

    switch(op->type)
    {
        case T_SelectionOperator:
        case T_ProjectionOperator:
        case T_JoinOperator:
        case T_TableAccessOperator:
            break;
        default:
        {
            SET_STRING_PROP(op, PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME, createConstBool(FALSE));
            return FALSE;
        }
    }

    FOREACH(QueryOperator,child,op->inputs)
        if (!isTupleAtATimeSubtree(child))
        {
            SET_STRING_PROP(op, PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME, createConstBool(FALSE));
            return FALSE;
        }


    SET_BOOL_STRING_PROP(op, PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);
    return TRUE;
}

static DataType
getRowNumDT()
{
	return typeOf((Node *) makeNode(RowNumExpr));
}

static Constant *
getOneForRowNum()
{
	switch(getRowNumDT())
	{
	case DT_INT:
		return createConstInt(1);
	case DT_LONG:
		return createConstLong(1L);
	default:
		FATAL_LOG("only integer types are supported for row number attributes.");
		return NULL;
	}
}

static QueryOperator *
rewritePI_CSComposableOperator (QueryOperator *op, PICSComposableRewriteState *state)
{
    boolean showIntermediate = HAS_STRING_PROP(op,  PROP_SHOW_INTERMEDIATE_PROV);
    boolean noRewriteUseProv = HAS_STRING_PROP(op, PROP_USE_PROVENANCE);
    boolean noRewriteHasProv = HAS_STRING_PROP(op, PROP_HAS_PROVENANCE);
    boolean rewriteAddProv = HAS_STRING_PROP(op, PROP_ADD_PROVENANCE);
    List *userProvAttrs = (List *) getStringProperty(op, PROP_USER_PROV_ATTRS);
    List *addProvAttrs = NIL;
    Set *ignoreProvAttrs = (Set *) getStringProperty(op, PROP_PROV_IGNORE_ATTRS);
    QueryOperator *rewrittenOp;

    if (rewriteAddProv)
    {
        addProvAttrs = (List *)  GET_STRING_PROP(op, PROP_ADD_PROVENANCE);
    }

    DEBUG_LOG("REWRITE OPERATIONS [%s@%p]:\n\tshow intermediates: %s\n\tuse prov: %s"
			  "\n\thas prov: %s\n\tadd prov: %s"
			  "\n\tuser prov attrs: %s"
			  "\n\tadd prov attrs: %s"
			  "\n\tignore prov attrs: %s"
			  "\n\talready was rewritten: %s",
			  NodeTagToString(op->type),
			  op,
			  showIntermediate ? "T": "F",
			  noRewriteUseProv ? "T": "F",
			  noRewriteHasProv ? "T": "F",
			  rewriteAddProv ? "T": "F",
			  nodeToString(userProvAttrs),
			  nodeToString(addProvAttrs),
			  nodeToString(ignoreProvAttrs),
			  isOpRewritten(state->opToRewrittenOp, op) ? "T" : "F"
		);

	// when operator is already rewritten, then just reuse the rewritten operator, but change provenance attribute names
	if(isOpRewritten(state->opToRewrittenOp, op))
	{
		return rewritePI_CSComposableReuseRewrittenOp(op, state);
	}

    if (noRewriteUseProv)
    {
        rewrittenOp = rewritePI_CSComposableAddProvNoRewrite(op, userProvAttrs, state);
        setRewrittenOp(state->opToRewrittenOp, op, rewrittenOp);
        return rewrittenOp;
    }
    if (noRewriteHasProv)
    {
        rewrittenOp = rewritePI_CSComposableUseProvNoRewrite(op, userProvAttrs, state);
        setRewrittenOp(state->opToRewrittenOp, op, rewrittenOp);
        return rewrittenOp;
    }

    switch(op->type)
    {
        case T_SelectionOperator:
            rewrittenOp = rewritePI_CSComposableSelection((SelectionOperator *) op, state);
            break;
        case T_ProjectionOperator:
            rewrittenOp = rewritePI_CSComposableProjection((ProjectionOperator *) op, state);
            break;
        case T_JoinOperator:
            rewrittenOp = rewritePI_CSComposableJoin((JoinOperator *) op, state);
            break;
        case T_AggregationOperator:
        {
            if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
            {
                QueryOperator *op1;
                int res;

                res = callback(2);

                if (res == 1)
                {
                    op1 = rewritePI_CSComposableAggregationWithWindow((AggregationOperator *) op, state);
                }
                else
                {
                       op1 = rewritePI_CSComposableAggregationWithJoin((AggregationOperator *) op, state);
                }

                rewrittenOp = op1;
               }
            else
            {
                if(getBoolOption(OPTION_PI_CS_COMPOSABLE_REWRITE_AGG_WINDOW))
                {
                    rewrittenOp = rewritePI_CSComposableAggregationWithWindow((AggregationOperator *) op, state);
                }
                else
                {
                    rewrittenOp = rewritePI_CSComposableAggregationWithJoin((AggregationOperator *) op, state);
                }
            }
            break;
        }
        case T_SetOperator:
            rewrittenOp = rewritePI_CSComposableSet((SetOperator *) op, state);
            break;
        case T_TableAccessOperator:
            rewrittenOp = rewritePI_CSComposableTableAccess((TableAccessOperator *) op, state);
            break;
        case T_ConstRelOperator:
            rewrittenOp = rewritePI_CSComposableConstRel((ConstRelOperator *) op, state);
            break;
        case T_DuplicateRemoval:
            rewrittenOp = rewritePI_CSComposableDuplicateRemOp((DuplicateRemoval *) op, state);
            break;
        case T_OrderOperator:
            rewrittenOp = rewritePI_CSComposableOrderOp((OrderOperator *) op, state);
            break;
        case T_LimitOperator:
            rewrittenOp = rewritePI_CSComposableLimitOp((LimitOperator *) op, state);
        break;
        default:
            FATAL_LOG("rewrite for not implemented for: %s",
                      singleOperatorToOverview(op));
            rewrittenOp = NULL;
            break;
    }

    if (showIntermediate)
    {
        rewrittenOp = composableAddIntermediateProvenance(rewrittenOp, userProvAttrs, ignoreProvAttrs, state);
    }

    if (rewriteAddProv)
    {
        rewrittenOp = composableAddUserProvenanceAttributes(rewrittenOp, addProvAttrs, showIntermediate, state);
    }

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel(rewrittenOp));

	setRewrittenOp(state->opToRewrittenOp, op, rewrittenOp);

    return rewrittenOp;
}

/**
 * @brief      Reuse an already rewritten subquery, but rename provenance attributes.
 *
 * @details
 *
 * @param      op - the operator to rewrite
 * @param      state - bookkeeping datastructure
 *
 * @return     a projection on the rewritten subtree that renames provenance attributes
 */
static QueryOperator *
rewritePI_CSComposableReuseRewrittenOp(QueryOperator *op, PICSComposableRewriteState *state)
{
	QueryOperator *rewrOp = getRewrittenOp(state->opToRewrittenOp, op);
	QueryOperator *rewr;
	List *normalAttrNames = getAttrDefNames(getNormalAttrWithoutSpecial(rewrOp));
	List *oldProjAttrNames = getOpProvenanceAttrNames(rewrOp);
	List *composeSpecialAttributes = LIST_MAKE(strdup(RESULT_TID_ATTR), strdup(PROV_DUPL_COUNT_ATTR));
	List *newProvAttrNames = NIL;
	List *oldAttrs;
	List *newAttrs;

	DEBUG_NODE_BEATIFY_LOG("reuse rewritten subquery: ", rewrOp);

	// create new provenance attribute names
	FOREACH(KeyValue,k,opGetProvAttrInfo(rewrOp))
	{
		char *table = STRING_VALUE(k->key);
		List *attNames = constStringListToStringList((List *) k->value);
		int cnt = increaseRefCount(state->provCounts, table);

		newProvAttrNames = concatTwoLists(newProvAttrNames, getProvenanceAttrNames(table, attNames, cnt));
	}

	// create projection to rename provenance attributes
	oldAttrs = CONCAT_LISTS(deepCopyStringList(normalAttrNames),
							oldProjAttrNames,
							deepCopyStringList(composeSpecialAttributes));
    newAttrs = CONCAT_LISTS(deepCopyStringList(normalAttrNames),
							newProvAttrNames,
							deepCopyStringList(composeSpecialAttributes));

	DEBUG_LOG("rename %s to %s", stringListToString(oldAttrs), stringListToString(newAttrs));

	rewr = createProjOnAttrsByName(rewrOp, oldAttrs, newAttrs);
	addChildOperator(rewr, rewrOp);

	// add result TID and dupl attributes
    addResultTIDAndProvDupAttrs(rewr, FALSE);

	COPY_PROV_INFO(rewr, rewrOp);

	LOG_RESULT_AND_RETURN(PICS-Composable,REUSE);
}

static QueryOperator *
composableAddUserProvenanceAttributes(QueryOperator *op,
									  List *userProvAttrs,
									  boolean showIntermediate,
									  PICSComposableRewriteState *state)
{
    QueryOperator *proj;
    List *attrNames = NIL;
    List *projExpr = NIL;
    List *attrDefs = NIL;
    List *provAttrPos = NIL;
    List *normalAttrExprs = getNormalAttrProjectionExprs(op);
    List *userPAttrExprs = NIL;
    int cnt = 0;
    char *newAttrName;
    int relAccessCount;
    char *tableName; // = "INTERMEDIATE";
    Set *userNames = STRSET();

    // create set for fast access
    FOREACH(Constant,n,userProvAttrs)
        addToSet(userNames,STRING_VALUE(n));

    FOREACH(AttributeReference,a,normalAttrExprs)
    {
        if (hasSetElem(userNames, a->name))
            userPAttrExprs = appendToTailOfList(userPAttrExprs,a);
    }

    if (isA(op,TableAccessOperator))
        tableName = ((TableAccessOperator *) op)->tableName;
    else
    {
        if (HAS_STRING_PROP(op, PROP_PROV_ADD_REL_NAME))
        {
            tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_ADD_REL_NAME));
        }
        else
            tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME));
    }

    if (showIntermediate)
        relAccessCount = getCurRelNameCount(state->provCounts, tableName) - 1;
    else
        relAccessCount = increaseRefCount(state->provCounts, tableName);

    DEBUG_LOG("REWRITE-PICS - Add Intermediate Provenance Attrs <%s> <%u>",  tableName, relAccessCount);

    // copy all attributes except TID and DUP if they were already added
    provAttrPos = copyObject(op->provAttrs);
//    if (showIntermediate)
//    {
        attrNames = getAttrNamesWithoutSpecial(op);
        attrDefs = getAllAttrWithoutSpecial(op);
//    }
//    else
//    {
//        attrNames = getQueryOperatorAttrNames(op);
//        attrDefs = op->schema->attrDefs;
//    }

    // create schema
    FOREACH(AttributeDef, attr, attrDefs)
    {
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    // add projection expression for newly added prov attributes
    FOREACH(AttributeReference, a, userPAttrExprs)
    {
        //TODO naming of intermediate results
        newAttrName = getProvenanceAttrName(tableName, a->name, relAccessCount);
        DEBUG_LOG("new attr name: %s", newAttrName);
        attrNames = appendToTailOfList(attrNames, newAttrName);
        projExpr = appendToTailOfList(projExpr, a);
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, cnt + LIST_LENGTH(userPAttrExprs) - 1, 1);
    provAttrPos = CONCAT_LISTS(provAttrPos, newProvPosList);

//    if (showIntermediate)
//    {
        // result tuple ID attribute
         newAttrName = strdup(RESULT_TID_ATTR);
         attrNames = appendToTailOfList(attrNames, newAttrName);
         projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));

         // provenance duplicate attribute
         newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
         attrNames = appendToTailOfList(attrNames, newAttrName);
         projExpr = appendToTailOfList(projExpr, getOneForRowNum());
//    }

    DEBUG_LOG("add additional provenance\n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(attrNames),
            nodeToString(projExpr),
            nodeToString(provAttrPos));

    // Create a new projection operator with these new attributes
    proj = (QueryOperator *) createProjectionOp(projExpr, op, NIL, attrNames);
    proj->inputs = NIL;
    proj->provAttrs = provAttrPos;


    // Switch the subtree with this newly created projection operator.
    switchSubtreeWithExisting((QueryOperator *) op, (QueryOperator *) proj);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    // add TID and DUP attributes from child
//    if (!showIntermediate)
//        addResultTIDAndProvDupAttrs(proj,TRUE);
//    else // add properties for TID and DUP attributes
//    {
        int numAttrs = getNumAttrs((QueryOperator *) proj) - 1;
        SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(numAttrs - 1));
        SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(numAttrs));
//    }

    DEBUG_LOG("added projection: %s", operatorToOverviewString((Node *) proj));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) proj));

    return proj;
}

static QueryOperator *
composableAddIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs, PICSComposableRewriteState *state)
{
    QueryOperator *proj;
    List *attrNames = NIL;
    List *projExpr = NIL;
    List *provAttrPos = NIL;
    List *normalAttrExpr = removeSpecialAttrsFromNormalProjectionExprs(getNormalAttrProjectionExprs(op));
    List *temp = NIL;
    int cnt = 0;
    char *newAttrName;
    int relAccessCount;
    char *tableName; // = "INTERMEDIATE";

    if (isA(op,TableAccessOperator))
        tableName = ((TableAccessOperator *) op)->tableName;
    else
        tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME));

    relAccessCount = increaseRefCount(state->provCounts, tableName);

    DEBUG_LOG("REWRITE-PICS - Add Intermediate Provenance Attrs <%s> <%u>",  tableName, relAccessCount);

    attrNames = getAttrNamesWithoutSpecial(op);
    provAttrPos = copyObject(op->provAttrs);

    // remove ignore prov attributes
    if (ignoreProvAttrs != NULL)
    {
        FOREACH(AttributeReference, a, normalAttrExpr)
        {
            if (!hasSetElem(ignoreProvAttrs, a->name))
            {
                temp = appendToTailOfList(temp, a);
            }
        }
        normalAttrExpr = temp;
    }

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, getAllAttrWithoutSpecial(op))
    {
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    FOREACH(AttributeReference, a, normalAttrExpr)
    {
        //TODO naming of intermediate results
        newAttrName = getProvenanceAttrName(tableName, a->name, relAccessCount);
        DEBUG_LOG("new attr name: %s", newAttrName);
        attrNames = appendToTailOfList(attrNames, newAttrName);
        projExpr = appendToTailOfList(projExpr, a);
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, cnt + LIST_LENGTH(normalAttrExpr) - 1, 1);
    provAttrPos = CONCAT_LISTS(provAttrPos, newProvPosList);

    // add TID and DUP attributes
    projExpr = CONCAT_LISTS(projExpr, getResultTidAndProvDupAttrsProjExprs(op));
    attrNames = appendToTailOfList(attrNames, strdup(RESULT_TID_ATTR));
    attrNames = appendToTailOfList(attrNames, strdup(PROV_DUPL_COUNT_ATTR));

    DEBUG_LOG("add intermediate provenance\n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(attrNames),
            nodeToString(projExpr),
            nodeToString(provAttrPos));

    // Create a new projection operator with these new attributes
    proj = (QueryOperator *) createProjectionOp(projExpr, NULL, NIL, attrNames);
    proj->provAttrs = provAttrPos;

    // if there is also PROP_PC_ADD_PROV set then copy over the properties to the new proj op
    if(HAS_STRING_PROP(op, PROP_ADD_PROVENANCE))
    {
        SET_STRING_PROP(proj, PROP_ADD_PROVENANCE,
                copyObject(GET_STRING_PROP(op, PROP_ADD_PROVENANCE)));
        SET_STRING_PROP(proj, PROP_PROV_REL_NAME,
                copyObject(GET_STRING_PROP(op, PROP_PROV_REL_NAME)));
        SET_STRING_PROP(proj, PROP_PROV_ADD_REL_NAME,
                copyObject(GET_STRING_PROP(op, PROP_PROV_ADD_REL_NAME)));
    }
    // Switch the subtree with this newly created projection operator.
    switchSubtreeWithExisting((QueryOperator *) op, (QueryOperator *) proj);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    int tidAttrPos = getNumAttrs(proj) -2;
    SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(tidAttrPos));
    SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(tidAttrPos + 1));

    DEBUG_LOG("added projection: %s", operatorToOverviewString((Node *) proj));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) proj));

    return proj;
}



static QueryOperator *
rewritePI_CSComposableAddProvNoRewrite(QueryOperator *op, List *userProvAttrs, PICSComposableRewriteState *state)
{
    List *provAttr = NIL;
    List *provAttrsOnly = NIL;
    List *projExpr = NIL;
    char *newAttrName;
    int relAccessCount;
    int numProvAttrs = LIST_LENGTH(userProvAttrs);
    int numNormalAttrs = LIST_LENGTH(op->schema->attrDefs);
    int cnt = 0;
    char *tableName; // = "INTERMEDIATE";
    QueryOperator *rewr = NULL;

    if (isA(op,TableAccessOperator))
    {
        tableName = ((TableAccessOperator *) op)->tableName;
    }
    else
    {
        tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME));
    }

    relAccessCount = increaseRefCount(state->provCounts, tableName);

    DEBUG_LOG("REWRITE-PICS-Composable - add existing attrs as provenance attributes <%s> accress count: <%u> instead of rewriting",
            tableName, relAccessCount);

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, op->schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        provAttrsOnly = appendToTailOfList(provAttrsOnly, createConstString(strdup(attr->attrName)));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(
                attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    cnt = 0;
    FOREACH(Constant, attr, userProvAttrs)
    {
        char *name = STRING_VALUE(attr);
        AttributeDef *a;

        newAttrName = getProvenanceAttrName(tableName, name, relAccessCount);
        provAttr = appendToTailOfList(provAttr, newAttrName);
        cnt = getAttrPos(op,name);
        a = getAttrDefByPos(op,cnt);
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(name, 0,
                cnt, 0, a->dataType));
    }

    // result tuple ID attribute
    newAttrName = strdup(RESULT_TID_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));

    // provenance duplicate attribute
    newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, getOneForRowNum());

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, numNormalAttrs, numNormalAttrs + numProvAttrs - 1, 1);

    DEBUG_LOG("no rewrite add provenance, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    int numAttrs = getNumAttrs((QueryOperator *) newpo) - 1;
    SET_STRING_PROP(newpo, PROP_RESULT_TID_ATTR, createConstInt(numAttrs - 1));
    SET_STRING_PROP(newpo, PROP_PROV_DUP_ATTR, createConstInt(numAttrs));

//    addResultTIDAndProvDupAttrs((QueryOperator *) newpo, FALSE);

    // create copy of subtree
    QueryOperator *rewrInput = copyUnrootedSubtree(op);

    // Switch the subtree with this newly created projection operator.
    /* switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo); */

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) rewrInput);

    SET_BOOL_STRING_PROP(newpo,PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);

    DEBUG_LOG("rewrite add provenance attrs:\n%s", operatorToOverviewString((Node *) newpo));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) newpo));

    // prov info (key: TABLE_NAME, value: (ATTRIBUTES))
	List *provInfo = singleton(createNodeKeyValue((Node *) createConstString(tableName),
										    (Node *) provAttrsOnly));
	SET_STRING_PROP(newpo, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

    rewr = (QueryOperator *) newpo;
    LOG_RESULT_AND_RETURN(PICS-Composable, AddProvToSubqueryNoRewrite);
}

static QueryOperator *
rewritePI_CSComposableUseProvNoRewrite(QueryOperator *op, List *userProvAttrs, PICSComposableRewriteState *state)
{
    char *newAttrName;
    List *provAttrs = op->provAttrs;
    int relAccessCount;
    char *tableName; // = "USER";
    boolean isTableAccess = isA(op,TableAccessOperator);

    if (isTableAccess)
        tableName = ((TableAccessOperator *) op)->tableName;
    else
        tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME));

    DEBUG_LOG("REWRITE PICS: Use existing provenance attributes %s for %s instead of rewriting",
            beatify(nodeToString(userProvAttrs)), tableName);

    relAccessCount = increaseRefCount(state->provCounts, tableName);

    // for table access operations we need to add a projection that renames the attributes
    if (isTableAccess)
    {
        QueryOperator *proj;
        ProjectionOperator *theProj;

        proj = createProjOnAllAttrs(op);
        theProj = (ProjectionOperator *) proj;

        // Switch the subtree with this newly created projection operator
        switchSubtrees(op, proj);

        // Add child to the newly created projection operator
        addChildOperator(proj, op);

        FOREACH(Constant,a,userProvAttrs)
        {
            char *name = STRING_VALUE(a);
            int pos = getAttrPos(proj, name);
            AttributeDef *attr;

            attr = getNthOfListP(proj->schema->attrDefs, pos);
            name = getProvenanceAttrName(tableName, name, relAccessCount);
            attr->attrName = name;
            provAttrs = appendToTailOfListInt(provAttrs, pos);

            // in parent operators adapt attribute references to use new name
            FOREACH(QueryOperator,p,proj->parents)
            {
                List *aRefs = findOperatorAttrRefs(p);
                int childPos = getChildPosInParent(p,proj);

                FOREACH(AttributeReference,a,aRefs)
                {
                    if (a->fromClauseItem == childPos && a->attrPosition == pos)
                        a->name = strdup(name);
                }
            }
        }

        // result tuple ID attribute
        int curPos = getNumAttrs(proj);
        newAttrName = strdup(RESULT_TID_ATTR);
        proj->schema->attrDefs = appendToTailOfList(proj->schema->attrDefs,
													createAttributeDef(newAttrName, state->rowNumDT));
        theProj->projExprs = appendToTailOfList(theProj->projExprs, makeNode(RowNumExpr));

        // provenance duplicate attribute
        newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
        proj->schema->attrDefs = appendToTailOfList(proj->schema->attrDefs,
                createAttributeDef(newAttrName, state->rowNumDT));
        theProj->projExprs = appendToTailOfList(theProj->projExprs, getOneForRowNum());

        // prov attributes and store TID and DUP attributes as property
        proj->provAttrs = provAttrs;
        SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(curPos));
        SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(curPos + 1));

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
            ASSERT(checkModel(proj));

        return proj;
    }
    // for non-tableaccess operators simply change the attribute names and mark the attributes as provenance attributes
    else
    {
        ProjectionOperator *proj;
        QueryOperator *projOp;

        FOREACH(Constant,a,userProvAttrs)
        {
            char *name = STRING_VALUE(a);
            int pos = getAttrPos(op, name);
            AttributeDef *attr;

            attr = getNthOfListP(op->schema->attrDefs, pos);
            name = getProvenanceAttrName(tableName, name, relAccessCount);
            attr->attrName = name;
            provAttrs = appendToTailOfListInt(provAttrs, pos);

            // in parent operators adapt attribute references to use new name
            FOREACH(QueryOperator,p,op->parents)
            {
                List *aRefs = findOperatorAttrRefs(p);
                int childPos = getChildPosInParent(p,op);

                FOREACH(AttributeReference,a,aRefs)
                {
                    if (a->fromClauseItem == childPos && a->attrPosition == pos)
                        a->name = strdup(name);
                }
            }
        }

        op->provAttrs = provAttrs;

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
            ASSERT(checkModel(op));

        // create projection so we can add TID and DUP attrs
        projOp = createProjOnAllAttrs(op);
        proj = (ProjectionOperator *) projOp;

        // Switch the subtree with this newly created projection operator.
        switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);

        // Add child to the newly created projections operator,
        addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

        // result tuple ID attribute
        int curPos = getNumAttrs(projOp);
        newAttrName = strdup(RESULT_TID_ATTR);
        projOp->schema->attrDefs = appendToTailOfList(projOp->schema->attrDefs,
                        createAttributeDef(newAttrName, state->rowNumDT));
        proj->projExprs = appendToTailOfList(proj->projExprs, makeNode(RowNumExpr));

        // provenance duplicate attribute
        newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
        projOp->schema->attrDefs = appendToTailOfList(projOp->schema->attrDefs,
                createAttributeDef(newAttrName, state->rowNumDT));
        proj->projExprs = appendToTailOfList(proj->projExprs, getOneForRowNum());

        // prov attributes and store TID and DUP attributes as property
        SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(curPos));
        SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(curPos + 1));

        return projOp;
    }
}

static QueryOperator *
rewritePI_CSComposableSelection (SelectionOperator *op, PICSComposableRewriteState *state)
{
	REWR_UNARY_SETUP_PIC(Selection);
	REWR_UNARY_CHILD_PIC();

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) rewr, OP_LCHILD(rewr));

    // add result TID and prov duplicate attributes
    addResultTIDAndProvDupAttrs((QueryOperator *) rewr, TRUE);

    if (isTupleAtATimeSubtree(OP_LCHILD(op))) {
        SET_BOOL_STRING_PROP(op,PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);
    }

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

    LOG_RESULT_AND_RETURN(PICS-Composable,Selection);
}

static QueryOperator *
rewritePI_CSComposableProjection (ProjectionOperator *op, PICSComposableRewriteState *state)
{
	ProjectionOperator *p;
	REWR_UNARY_SETUP_PIC(Projection);
	REWR_UNARY_CHILD_PIC();

	p = (ProjectionOperator *) rewr;

    /* // rewrite child */
    /* rewritePI_CSComposableOperator(OP_LCHILD(op), state); */

    // add projection expressions for provenance attrs
    FOREACH_INT(a, rewrInput->provAttrs)
    {
        AttributeDef *att = getAttrDef(rewrInput,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
        p->projExprs = appendToTailOfList(p->projExprs,
                createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
    }

    // add projection expressions for result TID and prov dup attrs
    p->projExprs = appendToTailOfList(p->projExprs,
            createFullAttrReference(RESULT_TID_ATTR, 0,
                    INT_VALUE(GET_STRING_PROP(rewrInput,PROP_RESULT_TID_ATTR)), 0, state->rowNumDT));

    p->projExprs = appendToTailOfList(p->projExprs,
            createFullAttrReference(PROV_DUPL_COUNT_ATTR, 0,
                    INT_VALUE(GET_STRING_PROP(rewrInput,PROP_PROV_DUP_ATTR)), 0, state->rowNumDT));

    // adapt schema
    addProvenanceAttrsToSchema(rewr, OP_LCHILD(rewr));
    addResultTIDAndProvDupAttrs(rewr, TRUE);

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

    LOG_RESULT_AND_RETURN(PICS-Composable,Projection);
}

static QueryOperator *
rewritePI_CSComposableJoin (JoinOperator *op, PICSComposableRewriteState *state)
{
	REWR_BINARY_SETUP_PIC(Join);

    WindowOperator *wOp = NULL;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
    QueryOperator *prev = NULL;
    boolean noDupInput = isTupleAtATimeSubtree((QueryOperator *) op);
    boolean lChildNoDup = isTupleAtATimeSubtree(lChild);
    boolean rChildNoDup = isTupleAtATimeSubtree(rChild);
    List *rNormAttrs;
    int numLAttrs, numRAttrs;
	List *provInfo;

    numLAttrs = getNumAttrs(lChild);
    numRAttrs = getNumAttrs(rChild);

	// rewrite children
	REWR_BINARY_CHILDREN_PIC();

    // get attributes from right input
    rNormAttrs = sublist(rewr->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    rewr->schema->attrDefs = sublist(copyObject(rewr->schema->attrDefs), 0, numLAttrs - 1);

    // adapt schema for join op
    addProvenanceAttrsToSchema((QueryOperator *) rewr, rewrLeftInput);
    addChildResultTIDAndProvDupAttrsToSchema((QueryOperator *) rewr);

    rewr->schema->attrDefs = CONCAT_LISTS(rewr->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchema((QueryOperator *) rewr, rewrRightInput);
    addChildResultTIDAndProvDupAttrsToSchema((QueryOperator *) rewr);

    // add window functions for result TID and prov dup columns
    if (!lChildNoDup || !rChildNoDup)
    {
        List *orderBy = NIL;
        List *partitionBy = NIL;
        wOp = NULL;

        if (lChildNoDup)
        {
            AttributeReference *childResultTidAttr = (AttributeReference *)
                    getHeadOfListP(getResultTidAndProvDupAttrsProjExprs(rewrLeftInput));
            orderBy = appendToTailOfList(orderBy, copyObject(childResultTidAttr));
            partitionBy = appendToTailOfList(partitionBy, copyObject(childResultTidAttr));
        }
        if (rChildNoDup)
        {
            AttributeReference *childResultTidAttr = (AttributeReference *)
                    getHeadOfListP(getResultTidAndProvDupAttrsProjExprs(rewrRightInput));
            childResultTidAttr->attrPosition += getNumAttrs(lChild);
            orderBy = appendToTailOfList(orderBy, copyObject(childResultTidAttr));
            partitionBy = appendToTailOfList(partitionBy, copyObject(childResultTidAttr));
        }

        // add window functions for result TID attr
        Node *tidFunc = (Node *) createFunctionCall(DENSE_RANK_FUNC_NAME, NIL);

        wOp = createWindowOp(tidFunc,
                NIL,
                orderBy,
                NULL,
                strdup(RESULT_TID_ATTR),
                (QueryOperator *) rewr,
                NIL
        );
        wOp->op.provAttrs = copyObject(rewr->provAttrs);

        // add window function for prov dup attr
        prev = (QueryOperator *) wOp;
        Node *provDupFunc = (Node *) createFunctionCall(ROW_NUMBER_FUNC_NAME, NIL);

        wOp = createWindowOp(provDupFunc,
                partitionBy,
                orderBy,
                NULL,
                strdup(PROV_DUPL_COUNT_ATTR),
                prev,
                NIL
        );
        wOp->op.provAttrs = copyObject(prev->provAttrs);
        addParent(prev, (QueryOperator *) wOp);
        SET_STRING_PROP(wOp, PROP_RESULT_TID_ATTR, createConstInt(LIST_LENGTH(wOp->op.schema->attrDefs) - 2));
        SET_STRING_PROP(wOp, PROP_PROV_DUP_ATTR, createConstInt(LIST_LENGTH(wOp->op.schema->attrDefs) - 1));

        LOG_RESULT("Added result TID and prov duplicate window ops:", wOp);
    }

    // add projection to put attributes into order on top of join op
    List *resultTidAndProvCount = NIL;
    List *projExpr;
    ProjectionOperator *proj;
    QueryOperator *projInput = (noDupInput) ?
            (QueryOperator *) rewr :
            (QueryOperator *) wOp;

    // get special attributes from window op or create projection expression for them
    if (!noDupInput)
        resultTidAndProvCount = getResultTidAndProvDupAttrsProjExprs((QueryOperator *) wOp);
    else
    {
        resultTidAndProvCount = LIST_MAKE(
                makeNode(RowNumExpr),
                getOneForRowNum()
        );
    }

    projExpr = CONCAT_LISTS(
            removeSpecialAttrsFromNormalProjectionExprs(
                    getNormalAttrProjectionExprs((QueryOperator *) projInput)),
            getProvAttrProjectionExprs((QueryOperator *) projInput),
            resultTidAndProvCount);
    proj = createProjectionOp(projExpr, projInput, NIL, NIL);

    addNormalAttrsWithoutSpecialToSchema((QueryOperator *) proj, (QueryOperator *) projInput);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) projInput);
    addChildResultTIDAndProvDupAttrsToSchema((QueryOperator *) proj);
    SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(LIST_LENGTH(projExpr) - 2));
    SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(LIST_LENGTH(projExpr) - 1));

    // switch projection with join in tree
    //TODO check, but should not be necessary anymore, switchSubtrees((QueryOperator *) rewr, (QueryOperator *) proj);
    if (noDupInput)
        addParent((QueryOperator *) rewr, (QueryOperator *) proj);
    else
    {
        addParent((QueryOperator *) wOp, (QueryOperator *) proj);
        addParent((QueryOperator *) rewr, (QueryOperator *) prev);
    }

	// make sure join result attributes are unique
	makeAttrNamesUnique(rewr);

	// final result is the projection
	rewr = (QueryOperator *) proj;

	// provenance info is concatenation of child prov infos
	provInfo = CONCAT_LISTS(
		(List *) GET_STRING_PROP(rewrLeftInput, PROP_PROVENANCE_TABLE_ATTRS),
		(List *) GET_STRING_PROP(rewrRightInput, PROP_PROVENANCE_TABLE_ATTRS));

	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

    LOG_RESULT_AND_RETURN(PICS-Composable,Join);
}

static QueryOperator *
rewritePI_CSComposableAggregationWithJoin (AggregationOperator *op, PICSComposableRewriteState *state)
{
	REWR_UNARY_SETUP_PIC(Aggregation-Join);
    JoinOperator *joinProv;
    boolean groupBy = (op->groupBy != NIL);
    ProjectionOperator *proj;
    QueryOperator *origAgg;
	QueryOperator *provInput;
    QueryOperator *pInput;
    int numGroupAttrs = LIST_LENGTH(op->groupBy);
//    boolean noDupInput;
    List *partitionBy = NIL;
    List *orderBy = NIL;

    if (groupBy)
    {
        List *subnames = sublist(getQueryOperatorAttrNames((QueryOperator *) op),
                LIST_LENGTH(op->aggrs),
                LIST_LENGTH(op->op.schema->attrDefs) - 1);
        int pos = LIST_LENGTH(op->aggrs);

        FORBOTH(void,gBy,res,op->groupBy,subnames)
        {
            AttributeReference *g = (AttributeReference *) copyObject(gBy);
            g->attrPosition = pos++;
            g->name = strdup((char *) res);
            partitionBy = appendToTailOfList(partitionBy,
                    g);
        }
        orderBy = (List *) copyObject(partitionBy);
    }

    // copy aggregation
    origAgg = (QueryOperator *) getOrSetOpCopy(state->origOps, (QueryOperator *) op);;

    // rewrite aggregation input copy
	rewrInput = rewritePI_CSComposableOperator(OP_LCHILD(op), state);
//    noDupInput =
    isTupleAtATimeSubtree(rewrInput);
	provInput = rewrInput;

    // add projection including group by expressions if necessary
    if(groupBy)
    {
        List *groupByProjExprs = (List *) copyObject(op->groupBy);
        List *attrNames = NIL;
        List *provAttrs = NIL;
        ProjectionOperator *groupByProj;
        List *gbNames = aggOpGetGroupByAttrNames(op);

        // adapt right side group by attr names
        FOREACH_LC(lc,gbNames)
        {
            char *name = (char *) LC_P_VAL(lc);
            LC_P_VAL(lc) = CONCAT_STRINGS("_P_SIDE_", name);
        }

        attrNames = CONCAT_LISTS(gbNames, getOpProvenanceAttrNames(provInput));
        groupByProjExprs = CONCAT_LISTS(groupByProjExprs, getProvAttrProjectionExprs(provInput));

        groupByProj = createProjectionOp(groupByProjExprs,
                        provInput, NIL, attrNames);
        CREATE_INT_SEQ(provAttrs, numGroupAttrs, numGroupAttrs + getNumProvAttrs(provInput) - 1,1);
        groupByProj->op.provAttrs = provAttrs;
        provInput->parents = singleton(groupByProj);
        provInput = (QueryOperator *) groupByProj;

        // no need to add prov duplicate and tid attributes to projection
        //TODO make sure the prov dup and tid attributes don't mess things up
    }

    // create join condition
    Node *joinCond = NULL;
    JoinType joinT = (op->groupBy) ? JOIN_INNER : JOIN_LEFT_OUTER;

    // create join condition for group by
    if(op->groupBy != NIL)
    {
        int pos = 0;
        List *groupByNames = aggOpGetGroupByAttrNames(op);

        FOREACH(AttributeReference, a , op->groupBy)
        {
            char *name = getNthOfListP(groupByNames, pos);
            AttributeReference *lA = createFullAttrReference(name, 0, LIST_LENGTH(op->aggrs) + pos, INVALID_ATTR, a->attrType);
            AttributeReference *rA = createFullAttrReference(
                    CONCAT_STRINGS("_P_SIDE_",name), 1, pos, INVALID_ATTR, a->attrType);
            if(joinCond)
                joinCond = AND_EXPRS((Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA), joinCond);
            else
                joinCond = (Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA);
            pos++;
        }
    }
    // or for without group by
    else
        joinCond = (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(getOneForRowNum(), getOneForRowNum()));

    // create join operator
    List *joinAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getQueryOperatorAttrNames(provInput));
    joinProv = createJoinOp(joinT, joinCond, LIST_MAKE(origAgg, provInput), NIL,
            joinAttrNames);
    joinProv->op.provAttrs = copyObject(provInput->provAttrs);
    FOREACH_LC(lc,joinProv->op.provAttrs)
        lc->data.int_value += getNumAttrs(origAgg);
    pInput = (QueryOperator *) joinProv;

    // add result TID attr and prov dup attr, if group by then use window function, otherwise use projection
    //TODO use simpler approach where TID is created as projection on ROWNUM of original aggregation
    if (groupBy)
    {
        WindowOperator *curWindow;
        // add window functions for result TID attr
        Node *tidFunc = (Node *) createFunctionCall(DENSE_RANK_FUNC_NAME, NIL);

        curWindow = createWindowOp(tidFunc,
                NIL,
                orderBy,
                NULL,
                strdup(RESULT_TID_ATTR),
                pInput,
                NIL
        );
        curWindow->op.provAttrs = copyObject(pInput->provAttrs);
        addParent(pInput, (QueryOperator *) curWindow);
        pInput = (QueryOperator *) curWindow;

        // add window function for prov dup attr
        Node *provDupFunc = (Node *) createFunctionCall(ROW_NUMBER_FUNC_NAME, NIL);

        curWindow = createWindowOp(provDupFunc,
                partitionBy,
                orderBy,
                NULL,
                strdup(PROV_DUPL_COUNT_ATTR),
                pInput,
                NIL
        );
        curWindow->op.provAttrs = copyObject(pInput->provAttrs);
        addParent(pInput, (QueryOperator *) curWindow);
        pInput = (QueryOperator *) curWindow;

        DEBUG_LOG("Added result TID and prov duplicate window ops:\n%s",
                       operatorToOverviewString((Node *) curWindow));
    }

    // create projection expressions for final projection
    List *projAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg),
            getOpProvenanceAttrNames(provInput),
            LIST_MAKE(RESULT_TID_ATTR, PROV_DUPL_COUNT_ATTR));
    List *projExprs = CONCAT_LISTS(getNormalAttrProjectionExprs(origAgg),
                                getProvAttrProjectionExprs((QueryOperator *) joinProv));

    // add DUP and TID attrs projection expresions
    // if group by then take these attributes from the child
    if (groupBy)
    {
        projExprs = appendToTailOfList(projExprs, createFullAttrReference(
                RESULT_TID_ATTR,
                0,
                getNumAttrs(pInput) - 2,
                INVALID_ATTR,
                state->rowNumDT
                ));
        projExprs = appendToTailOfList(projExprs, createFullAttrReference(
                PROV_DUPL_COUNT_ATTR,
                0,
                getNumAttrs(pInput) - 1,
                INVALID_ATTR,
                state->rowNumDT
                ));
    }
    // else add 1 as RESULT_TID and ROWNUM AS PROV DUP
    else
    {
        projExprs = appendToTailOfList(projExprs, getOneForRowNum());
        projExprs = appendToTailOfList(projExprs, makeNode(RowNumExpr));
    }

    // create final projection and replace aggregation subtree with projection
    int numProj = LIST_LENGTH(projAttrNames);
    proj = createProjectionOp(projExprs, (QueryOperator *) pInput, NIL, projAttrNames);
    pInput->parents = singleton(proj);
    CREATE_INT_SEQ(proj->op.provAttrs, getNumNormalAttrs((QueryOperator *) origAgg),
            getNumNormalAttrs((QueryOperator *) origAgg)
            + getNumProvAttrs((QueryOperator *) joinProv) - 1,
            1);

    // set DUP and TID attributes properties
    SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR,
            createConstInt(numProj - 2));
    SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR,
            createConstInt(numProj - 1));

    // switch provenance computation with original aggregation
    //switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addParent(origAgg, (QueryOperator *) joinProv);
    addParent(provInput, (QueryOperator *) joinProv);

    // adapt schema for final projection
	rewr = (QueryOperator *) proj;

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

	LOG_RESULT_AND_RETURN(PICS-Composable,Aggregation-Join);
}

static void
aggCreateParitionAndOrderBy(AggregationOperator* op, List** partitionBy,
        List** orderBy)
{
    FOREACH(AttributeReference, a, op->groupBy)
        *partitionBy = appendToTailOfList(*partitionBy, copyObject(a));
    *orderBy = copyObject(*partitionBy);
}

static QueryOperator *
rewritePI_CSComposableAggregationWithWindow (AggregationOperator *op, PICSComposableRewriteState *state)
{
	REWR_UNARY_SETUP_PIC(Aggregation-RewriteWithWindow);
    boolean groupBy = LIST_LENGTH(op->groupBy) > 0;
	boolean hasAgg = LIST_LENGTH(op->aggrs) > 0;
    WindowOperator *curWindow = NULL;
    //QueryOperator *firstChild;
    QueryOperator *curChild;
    ProjectionOperator *proj;
    Node *provDupAttrRef;
    boolean noDupInput;
    List *projExprs = NIL;
    List *finalAttrs = NIL;
    List *orderBy = NIL;
    List *partitionBy = NIL;
    List *groupByExprs = copyObject(op->groupBy);
    List *aggNames = aggOpGetAggAttrNames(op);
    int pos;

    // rewrite child
	rewrInput = rewritePI_CSComposableOperator(OP_LCHILD(op), state);
//    curChild = rewritePI_CSComposableOperator(OP_LCHILD(op), state);
	curChild = rewrInput;
    //firstChild = curChild;
    //removeParentFromOps(singleton(firstChild), (QueryOperator *) op);
    noDupInput = isTupleAtATimeSubtree(curChild);

    // create partition clause and order by clauses
    if (groupBy)
        aggCreateParitionAndOrderBy(op, &partitionBy, &orderBy);

    // get input prov dup attribute
    if (!noDupInput)
    {
        provDupAttrRef = (Node *) createFullAttrReference(PROV_DUPL_COUNT_ATTR,
                0,
                INT_VALUE(GET_STRING_PROP(curChild, PROP_PROV_DUP_ATTR)),
                INVALID_ATTR,
                state->rowNumDT);
    }
    else
        provDupAttrRef = NULL;

    // create window op for each aggregation
    pos = 0;
    FOREACH(Node,agg,op->aggrs)
    {
        Node *aggForWindow = replaceAttrWithCaseForProvDupRemoval(
                copyObject(agg), provDupAttrRef);
        char *attrName = getNthOfListP(aggNames, pos);

        curWindow = createWindowOp(aggForWindow,
                partitionBy,
                NIL,
                NULL,
                attrName,
                curChild,
                NIL
                );
        curWindow->op.provAttrs = copyObject(curChild->provAttrs);
        addParent(curChild, (QueryOperator *) curWindow);

        curChild = (QueryOperator *) curWindow;
        pos++;
        DEBUG_NODE_BEATIFY_LOG("Translated aggregation function",agg);
        DEBUG_OP_LOG("into window op", curWindow);
    }

    // add result TID attr and prov dup attr, if group by then use window function, otherwise use projection
    if (groupBy)
    {
        // agg projection to get rid of input TID and DUP attrs
        QueryOperator *gProj;
        List *attrPosPref = NIL;
        List *attrPosPost = NIL;
        int numAs = getNumAttrs(curChild);
        int numAggs = LIST_LENGTH(aggNames);
        // remove TID and DUP FROM attrs, TID, DUP, aggrs
        CREATE_INT_SEQ(attrPosPref, 0, numAs - numAggs - 2 - 1, 1);
        CREATE_INT_SEQ(attrPosPost, numAs - numAggs, numAs - 1, 1);
        gProj = createProjOnAttrs(curChild, CONCAT_LISTS(attrPosPref, attrPosPost));
        addChildOperator(gProj,curChild);
        curChild = gProj;

        // add window functions for result TID attr
        Node *tidFunc = (Node *) createFunctionCall(DENSE_RANK_FUNC_NAME, NIL);

        curWindow = createWindowOp(tidFunc,
                NIL,
                orderBy,
                NULL,
                strdup(RESULT_TID_ATTR),
                curChild,
                NIL
        );
        curWindow->op.provAttrs = copyObject(curChild->provAttrs);
        addParent(curChild, (QueryOperator *) curWindow);
        curChild = (QueryOperator *) curWindow;

        // add window function for prov dup attr
        Node *provDupFunc = (Node *) createFunctionCall(ROW_NUMBER_FUNC_NAME, NIL);

        curWindow = createWindowOp(provDupFunc,
                copyObject(partitionBy),
                copyObject(orderBy),
                NULL,
                strdup(PROV_DUPL_COUNT_ATTR),
                curChild,
                NIL
        );
        curWindow->op.provAttrs = copyObject(curChild->provAttrs);
        addParent(curChild, (QueryOperator *) curWindow);
        curChild = (QueryOperator *) curWindow;

        DEBUG_LOG("Added result TID and prov duplicate window ops:\n%s",
                       operatorToOverviewString((Node *) curWindow));
    }


    // create final projection: normal attributes + provenance attribute + result TID and prov dup attr
	// need to do this unless we have group-by without aggregation
	List *normalAttrs = getNormalAttrProjectionExprs((QueryOperator *) curWindow);
	List *provAttrs = getProvAttrProjectionExprs((QueryOperator *) curWindow);
	List *aggAttrNames = aggOpGetAggAttrNames(op);
	List *groupByAttrNames = groupBy ? aggOpGetGroupByAttrNames(op) : NIL;
	List *provAttrNames = getOpProvenanceAttrNames((QueryOperator *) curWindow);

	// no group by, add result TID and prov dup attributes to projection
	if (!groupBy)
	{
		normalAttrs = sublist(normalAttrs,
							  LIST_LENGTH(normalAttrs) - LIST_LENGTH(op->aggrs),
							  LIST_LENGTH(normalAttrs) - 1);
		projExprs = CONCAT_LISTS(normalAttrs, provAttrs,
								 LIST_MAKE(getOneForRowNum(),
										   makeNode(RowNumExpr)));

		finalAttrs = CONCAT_LISTS(aggAttrNames,
								  provAttrNames,
								  LIST_MAKE(strdup(RESULT_TID_ATTR),strdup(PROV_DUPL_COUNT_ATTR)));
	}
	// else move result TID and prov dup attribute to end of list
	else
	{
		if(hasAgg)
		{
			normalAttrs = sublist(normalAttrs,
								  LIST_LENGTH(normalAttrs) - LIST_LENGTH(op->aggrs) - 2,
								  LIST_LENGTH(normalAttrs) - 3);
		}
		else
		{
			normalAttrs = NIL;
		}

		projExprs = CONCAT_LISTS(normalAttrs, groupByExprs, provAttrs,
								 LIST_MAKE(createFullAttrReference(
											   strdup(RESULT_TID_ATTR),
											   0,
											   getNumAttrs((QueryOperator *) curWindow) - 2,
											   INVALID_ATTR,
											   state->rowNumDT),
										   createFullAttrReference(
											   strdup(PROV_DUPL_COUNT_ATTR),
											   0,
											   getNumAttrs((QueryOperator *) curWindow) - 1,
											   INVALID_ATTR,
											   state->rowNumDT)));

		finalAttrs = CONCAT_LISTS(aggAttrNames,
								  groupByAttrNames,
								  provAttrNames,
								  LIST_MAKE(strdup(RESULT_TID_ATTR),strdup(PROV_DUPL_COUNT_ATTR)));
	}

	proj = createProjectionOp(projExprs, curChild, NIL, finalAttrs);
	CREATE_INT_SEQ(proj->op.provAttrs,
				   LIST_LENGTH(op->aggrs) + LIST_LENGTH(op->groupBy),
				   getNumAttrs((QueryOperator *) proj) - 3, 1);
	addParent((QueryOperator *) curWindow, (QueryOperator *) proj);
	rewr = (QueryOperator *) proj;

	DEBUG_LOG("projection is:\n%s", operatorToOverviewString((Node *) rewr));

    SET_STRING_PROP(rewr, PROP_RESULT_TID_ATTR, createConstInt(LIST_LENGTH(finalAttrs) - 2));
    SET_STRING_PROP(rewr, PROP_PROV_DUP_ATTR, createConstInt(LIST_LENGTH(finalAttrs) - 1));

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

    // return projection
	LOG_RESULT_AND_RETURN(PICS-Composable,Aggregation-RewriteWithWindow);
}

static Node *
replaceAttrWithCaseForProvDupRemoval (FunctionCall *f, Node *provDupAttrRef)
{
    if (provDupAttrRef == NULL)
        return (Node *) f;

    FOREACH_LC(lc,f->args)
    {
        Node *arg = LC_P_VAL(lc);
        LC_P_VAL(lc) = createCaseExpr(
                NULL,
                singleton(createCaseWhen((Node *) createOpExpr(OPNAME_EQ,
                        LIST_MAKE(getOneForRowNum(), copyObject(provDupAttrRef))),
                        arg)),
                (Node *) createNullConst(getRowNumDT()));
    }

    DEBUG_NODE_BEATIFY_LOG("modified agg function call:", f);
    return (Node *) f;
}

static QueryOperator *
rewritePI_CSComposableSet (SetOperator *op, PICSComposableRewriteState *state)
{
	REWR_BINARY_SETUP_PIC(Set-Operation);
	List *provInfo;
	QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    //add semiring options
    addSCOptionToChild((QueryOperator *) op,lChild);
    addSCOptionToChild((QueryOperator *) op,rChild);

	// rewrite children
	REWR_BINARY_CHILDREN_PIC();

    switch(op->setOpType)
    {
    case SETOP_UNION:
    {
        List *projExprs = NIL;
        List *attNames;
        List *provAttrs = NIL;
		ProjectionOperator *resultTidProj;
        int lProvs;
        int i;

        lProvs = LIST_LENGTH(rewrLeftInput->provAttrs);

        // create projection over left rewritten input
        attNames = CONCAT_LISTS(getAttrNamesWithoutSpecial(rewrLeftInput),
								  getOpProvenanceAttrNames(rewrRightInput),
								  LIST_MAKE(strdup(RESULT_TID_ATTR), strdup(PROV_DUPL_COUNT_ATTR)));

		projExprs = removeSpecialAttrsFromNormalProjectionExprs(
			getAllAttrProjectionExprs(rewrLeftInput));
        provAttrs = copyObject(rewrLeftInput->provAttrs);

        // create NULL expressions for provenance attrs of right input
		i = LIST_LENGTH(projExprs);
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rewrRightInput))
        {
            Constant *expr;

            expr = createNullConst(a->dataType);
            projExprs = appendToTailOfList(projExprs, expr);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }

		projExprs = CONCAT_LISTS(projExprs,
								 getProjExprsForAttrNames(
									 rewrLeftInput,
									 LIST_MAKE(strdup(RESULT_TID_ATTR),
											   strdup(PROV_DUPL_COUNT_ATTR))));

        DEBUG_LOG("have created projection expression: %s\nattribute names: "
				  "%s\n provAttrs: %s\n for left UNION input",
				  nodeToString(projExprs), stringListToString(attNames),
				  nodeToString(provAttrs));

        ProjectionOperator *projLeftChild = createProjectionOp(projExprs,
															   rewrLeftInput, NIL, attNames);
        ((QueryOperator *) projLeftChild)->provAttrs = provAttrs;

        // create projection over right rewritten input
        provAttrs = NIL;
        projExprs = NIL;
        attNames = CONCAT_LISTS(getAttrDefNames(getNormalAttrWithoutSpecial(rewrRightInput)),
								getOpProvenanceAttrNames(rewrLeftInput),
								getOpProvenanceAttrNames(rewrRightInput),
								LIST_MAKE(strdup(RESULT_TID_ATTR), strdup(PROV_DUPL_COUNT_ATTR)));

        // create AttrRefs for normal attributes of right input
		projExprs = removeSpecialAttrsFromNormalProjectionExprs(getNormalAttrProjectionExprs(rewrRightInput));

        // create NULL expressions for provenance attrs of left input
		i = LIST_LENGTH(projExprs);
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rewrLeftInput))
        {
            Constant *expr;

            expr = createNullConst(a->dataType);
            projExprs = appendToTailOfList(projExprs, expr);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }

        // create AttrRefs for provenance attrs of right input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rewrRightInput))
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i - lProvs, INVALID_ATTR, a->dataType);
            projExprs = appendToTailOfList(projExprs, att);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }

		projExprs = CONCAT_LISTS(projExprs,
								 getProjExprsForAttrNames(
									 rewrRightInput,
									 LIST_MAKE(strdup(RESULT_TID_ATTR),
											   strdup(PROV_DUPL_COUNT_ATTR))));

        DEBUG_LOG("have created projection expressions: %s\nattribute names: "
				  "%s\n provAttrs: %s\n for right UNION input",
				  nodeToString(projExprs), stringListToString(attNames),
				  nodeToString(provAttrs));

        ProjectionOperator *projRightChild = createProjectionOp(projExprs,
																rewrRightInput, NIL, attNames);
        ((QueryOperator *) projRightChild)->provAttrs = provAttrs;

    	// make projections of rewritten inputs the direct children of the union operation
        rewrLeftInput->parents = singleton(projLeftChild);
        rewrRightInput->parents = singleton(projRightChild);

		addParent((QueryOperator *) projLeftChild, rewr);
		addParent((QueryOperator *) projRightChild, rewr);
		rewr->inputs = LIST_MAKE(projLeftChild, projRightChild);

		// adapt schema of union itself, we can get full provenance attributes from left input
		addProvenanceAttrsToSchema((QueryOperator *) rewr, (QueryOperator *) projLeftChild);
		addResultTIDAndProvDupAttrs(rewr, TRUE);

		// add final projection to create TID and DUP attributes
		resultTidProj = (ProjectionOperator *) createProjOnAttrsByName(
			rewr,
			getAttrNamesWithoutSpecial(rewr), NIL);
		resultTidProj->projExprs = CONCAT_LISTS(resultTidProj->projExprs,
												LIST_MAKE(makeNode(RowNumExpr),
														  getOneForRowNum()));
		addChildOperator((QueryOperator *) resultTidProj, rewr);

		rewr = (QueryOperator *) resultTidProj;
    	/* addProvenanceAttrsToSchema((QueryOperator *) rewr, (QueryOperator *) projLeftChild); */
		addResultTIDAndProvDupAttrs(rewr, TRUE);
    }
	break;
    case SETOP_INTERSECTION:
    {//TODO write version for composable
        //create join condition
        Node *joinCond;
        List *comparisons = NIL;
        int schemaSize = LIST_LENGTH(rewrLeftInput->schema->attrDefs);

        for(int i = 0 ; i < schemaSize; i++)
        {
            AttributeDef *lDef, *rDef;
            lDef = getAttrDefByPos(rewrLeftInput, i);
            rDef = getAttrDefByPos(rewrRightInput, i);
            comparisons = appendToTailOfList(comparisons,
											 createOpExpr(OPNAME_EQ,
														  LIST_MAKE(
															  createFullAttrReference(strdup(lDef->attrName),0,i,INVALID_ATTR, lDef->dataType),
															  createFullAttrReference (strdup(rDef->attrName),1,i,INVALID_ATTR, rDef->dataType))));
        }
        joinCond = andExprList(comparisons);
        DEBUG_LOG("join cond: %s", beatify(nodeToString(joinCond)));

        //restrcuture the tree
    	JoinOperator *joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(rewrLeftInput,rewrRightInput), NIL, NIL);
    	/* removeParent(rewrLeftInput, (QueryOperator *) op); */
    	addParent(rewrLeftInput,(QueryOperator *)  joinOp);
    	/* removeParent(rewrRightInput, (QueryOperator *) op); */
    	addParent(rewrRightInput, (QueryOperator *) joinOp);

        // adapt schema for join op
        clearAttrsFromSchema((QueryOperator *) joinOp);
        addNormalAttrsToSchema((QueryOperator *) joinOp, rewrLeftInput);
        addProvenanceAttrsToSchema((QueryOperator *) joinOp, rewrLeftInput);
        addNormalAttrsToSchema((QueryOperator *) joinOp, rewrRightInput);
        addProvenanceAttrsToSchema((QueryOperator *) joinOp, rewrRightInput);

    	// add projection
        List *projExpr = CONCAT_LISTS(
			getNormalAttrProjectionExprs((QueryOperator *) joinOp),
			getProvAttrProjectionExprs((QueryOperator *) joinOp));
        ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

        addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) joinOp);
        addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) joinOp);
        addChildOperator((QueryOperator *) proj, (QueryOperator *) joinOp);

    	switchSubtreeWithExisting((QueryOperator *) op, (QueryOperator *) proj);

    	return (QueryOperator *) joinOp;
    }
	break;
    case SETOP_DIFFERENCE:
    {//TODO rewrite for composable
    	JoinOperator *joinOp;
    	ProjectionOperator *projOp;
    	// join provenance with rewritten right input
    	// create join condition
        Node *joinCond;
        List *joinAttrs = CONCAT_LISTS(getQueryOperatorAttrNames((QueryOperator *) op),
									   getQueryOperatorAttrNames(rewrLeftInput));
        makeNamesUnique(joinAttrs, NULL);
    	joinCond = NULL;

        FORBOTH(AttributeReference, aL , aR, getNormalAttrProjectionExprs(rewrLeftInput),
                getNormalAttrProjectionExprs(rewrLeftInput))
        {
            aL->fromClauseItem = 0;
            aR->fromClauseItem = 1;
            if(joinCond)
                joinCond = AND_EXPRS((Node *) createIsNotDistinctExpr((Node *) aL, (Node *) aR), joinCond);
            else
                joinCond = (Node *) createIsNotDistinctExpr((Node *) aL, (Node *) aR);
        }

        joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(op, rewrLeftInput),
							  NIL, joinAttrs);
        joinOp->op.provAttrs = copyObject(rewrLeftInput->provAttrs);
        SHIFT_INT_LIST(joinOp->op.provAttrs, getNumAttrs((QueryOperator *) op));

    	// adapt schema using projection
        List *rightProvAttrs = getProvenanceAttrDefs(rewrRightInput);
//        List *rightProvNames = getOpProvenanceAttrNames(rewrRightInput);

        List *projExpr = CONCAT_LISTS(getNormalAttrProjectionExprs((QueryOperator *)op),
									  getProvAttrProjectionExprs((QueryOperator *) joinOp));
        FOREACH(AttributeDef,a,rightProvAttrs)
            projExpr = appendToTailOfList(projExpr, createNullConst(a->dataType));

        List *projAttrs = getQueryOperatorAttrNames((QueryOperator *) op);

        projOp = createProjectionOp(projExpr, (QueryOperator *) joinOp, NIL, projAttrs);
        projOp->op.provAttrs = copyObject(rewrLeftInput->provAttrs);
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, OP_LCHILD(projOp));
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, rewrRightInput);
    	addParent((QueryOperator *) joinOp, (QueryOperator *) projOp);

    	// switch original set diff with projection
    	switchSubtreeWithExisting((QueryOperator *) op, (QueryOperator *) projOp);
    	addParent((QueryOperator *) op, (QueryOperator *) joinOp);
    	addParent((QueryOperator *) rewrLeftInput, (QueryOperator *) joinOp);

    	rewr = (QueryOperator *) projOp;
    }
	break;
    default:
    	break;
    }

	// provenance info is concatenation of child prov infos
	provInfo = CONCAT_LISTS(
		(List *) GET_STRING_PROP(rewrLeftInput, PROP_PROVENANCE_TABLE_ATTRS),
		(List *) GET_STRING_PROP(rewrRightInput, PROP_PROVENANCE_TABLE_ATTRS));

	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

	LOG_RESULT_AND_RETURN(PICS-Composable,SetOperation);
}

static QueryOperator *
rewritePI_CSComposableTableAccess(TableAccessOperator *op, PICSComposableRewriteState *state)
{
    List *provAttr = NIL;
	List *provAttrsOnly = NIL;
    List *projExpr = NIL;
    char *newAttrName;
	char *tableName;
    TableAccessOperator *t = (TableAccessOperator *) shallowCopyQueryOperator((QueryOperator *) op);
	QueryOperator *rewr;
	List *provInfo;
    int relAccessCount = increaseRefCount(state->provCounts, op->tableName);
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS-Composable - Table Access <%s> <%u>", op->tableName, relAccessCount);

	tableName = strdup(op->tableName);

    // copy any as of clause if there
    if (state->asOf)
        t->asOf = copyObject(state->asOf);

    // Add normal attributes
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

	// create provenance attributes by duplicating each of the table's attributes
    cnt = 0;
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        newAttrName = getProvenanceAttrName(op->tableName, attr->attrName, relAccessCount);
        provAttr = appendToTailOfList(provAttr, newAttrName);
		provAttrsOnly = appendToTailOfList(provAttrsOnly, createConstString(strdup(attr->attrName)));
		projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    // result tuple ID attribute
    newAttrName = strdup(RESULT_TID_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));

    // provenance duplicate attribute
    newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, getOneForRowNum());

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // set properties to mark result TID and prov duplicate attrs
    SET_STRING_PROP(newpo, PROP_RESULT_TID_ATTR, createConstInt(cnt * 2));
    SET_STRING_PROP(newpo, PROP_PROV_DUP_ATTR, createConstInt((cnt * 2) + 1));

    // Switch the subtree with this newly created projection operator.
    //TODO no longer necessary switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) t);
    SET_BOOL_STRING_PROP(newpo,PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);
	rewr = (QueryOperator *) newpo;

	// prov info (key: TABLE_NAME, value: (ATTRIBUTES))
	provInfo = singleton(createNodeKeyValue((Node *) createConstString(tableName),
										    (Node *) provAttrsOnly));
	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

    LOG_RESULT_AND_RETURN(PICS-Composable,TableAccess);
}

static QueryOperator *
rewritePI_CSComposableConstRel(ConstRelOperator *op, PICSComposableRewriteState *state)
{
//    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
	List *provAttrsOnly = NIL;
    char *newAttrName;
	List *provInfo;
	QueryOperator *rewr;
	ConstRelOperator *inCopy = (ConstRelOperator *) shallowCopyQueryOperator((QueryOperator *) op);
    int relAccessCount = increaseRefCount(state->provCounts, "query");
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS-Composable - Const Rel Operator <%s> <%u>", nodeToString(op->values), relAccessCount);

    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    cnt = 0;
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        newAttrName = getProvenanceAttrName("query", attr->attrName, relAccessCount);
        provAttr = appendToTailOfList(provAttr, newAttrName);
		provAttrsOnly = appendToTailOfList(provAttrsOnly, createConstString(strdup(newAttrName)));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

    DEBUG_LOG("rewrite const rel operator, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // Switch the subtree with this newly created projection operator.
    //switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) inCopy);
	rewr = (QueryOperator *) newpo;

	// prov info (key: TABLE_NAME, value: (ATTRIBUTES))
	provInfo = singleton(createNodeKeyValue((Node *) createConstString("query"),
										    (Node *) provAttrsOnly));
	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

	LOG_RESULT_AND_RETURN(PICS-Composable,ConstRel);
}

static QueryOperator *
rewritePI_CSComposableDuplicateRemOp(DuplicateRemoval *op, PICSComposableRewriteState *state)
{
	REWR_UNARY_SETUP_PIC(DuplicateRemoval);
	QueryOperator *curOp;
	QueryOperator *curChild;
	List *normalAttrRefs;
	List *allAttrRefs;
	List *orderBy = NIL;
	List *partitionBy = NIL;

	rewrInput = rewritePI_CSComposableOperator(OP_LCHILD(op), state);
	normalAttrRefs = removeSpecialAttrsFromNormalProjectionExprs(
		getNormalAttrProjectionExprs((QueryOperator *) rewrInput));
	allAttrRefs = removeSpecialAttrsFromNormalProjectionExprs(
		getAllAttrProjectionExprs((QueryOperator *) rewrInput));
	curChild = rewrInput;

	// create order-by and group-by attribute lists for
	/* FOREACH(AttributeReference, a, normalAttrRefs) */
    /*     partitionBy = appendToTailOfList(partitionBy, copyObject(a)); */
	partitionBy = copyObject(normalAttrRefs);
    orderBy = copyObject(partitionBy);

	// remove TID and DUP FROM attrs, TID, DUP, aggrs
	curOp = (QueryOperator *) createProjectionOp(allAttrRefs, curChild, NIL, NIL);
	addParent(curChild, curOp);
	curChild = curOp;

	// add window functions for result TID attr
	Node *tidFunc = (Node *) createFunctionCall(DENSE_RANK_FUNC_NAME, NIL);

	curOp = (QueryOperator * ) createWindowOp(tidFunc,
							   NIL,
							   orderBy,
							   NULL,
							   strdup(RESULT_TID_ATTR),
							   curChild,
							   NIL);
	curOp->provAttrs = copyObject(curChild->provAttrs);
	addParent(curChild, (QueryOperator *) curOp);
	curChild = (QueryOperator *) curOp;

	// add window function for prov dup attr
	Node *provDupFunc = (Node *) createFunctionCall(ROW_NUMBER_FUNC_NAME, NIL);

	curOp = (QueryOperator * ) createWindowOp(provDupFunc,
							   copyObject(partitionBy),
							   copyObject(orderBy),
							   NULL,
							   strdup(PROV_DUPL_COUNT_ATTR),
							   curChild,
							   NIL);
	curOp->provAttrs = copyObject(curChild->provAttrs);
	addParent(curChild, (QueryOperator *) curOp);
	curChild = (QueryOperator *) curOp;

	// result is the window function
	rewr = curOp;

	// copy prov info and set special attributes
	int numAttrs = LIST_LENGTH(rewr->schema->attrDefs);

	COPY_PROV_INFO(rewr, rewrInput);
    SET_STRING_PROP(rewr, PROP_RESULT_TID_ATTR, createConstInt(numAttrs - 2));
    SET_STRING_PROP(rewr, PROP_PROV_DUP_ATTR, createConstInt(numAttrs - 1));

    LOG_RESULT_AND_RETURN(PICS-Composable,DuplicateRemoval);
}

static void
addResultTIDAndProvDupAttrs (QueryOperator *op, boolean addToSchema)
{
    int numAttrs = getNumAttrs(op);
    QueryOperator *child = OP_LCHILD(op);

    if (addToSchema)
    {
        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                createAttributeDef(strdup(RESULT_TID_ATTR), getRowNumDT()));
        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                    createAttributeDef(strdup(PROV_DUPL_COUNT_ATTR), getRowNumDT()));

        // set properties to mark result TID and prov duplicate attrs
        SET_STRING_PROP(op, PROP_RESULT_TID_ATTR, createConstInt(numAttrs));
        SET_STRING_PROP(op, PROP_PROV_DUP_ATTR, createConstInt(numAttrs + 1));
    }
    else
    {
        SET_STRING_PROP(op, PROP_RESULT_TID_ATTR,
                copyObject(GET_STRING_PROP(child, PROP_RESULT_TID_ATTR)));
        SET_STRING_PROP(op, PROP_PROV_DUP_ATTR,
                copyObject(GET_STRING_PROP(child, PROP_PROV_DUP_ATTR)));
    }
}

static void
addChildResultTIDAndProvDupAttrsToSchema (QueryOperator *op)
{
    op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
            createAttributeDef(strdup(RESULT_TID_ATTR), getRowNumDT()));
    op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
            createAttributeDef(strdup(PROV_DUPL_COUNT_ATTR), getRowNumDT()));
}

static void
addNormalAttrsWithoutSpecialToSchema(QueryOperator *target, QueryOperator *source)
{
    List *newAttrs = (List *) copyObject(getNormalAttrWithoutSpecial(source));
    target->schema->attrDefs = concatTwoLists(target->schema->attrDefs, newAttrs);
}

static List *
getAttrNamesWithoutSpecial(QueryOperator *op)
{
    List *attr;
    List *attrNames = NIL;

    attr = getAllAttrWithoutSpecial(op);
    FOREACH(AttributeDef,a,attr)
        attrNames = appendToTailOfList(attrNames, strdup(a->attrName));

    return attrNames;
}


//static List *
//getNormalAttrNamesWithoutSpecial(QueryOperator *op)
//{
//    List *attr;
//    List *attrNames = NIL;
//
//    attr = getNormalAttrWithoutSpecial(op);
//    FOREACH(AttributeDef,a,attr)
//        attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
//
//    return attrNames;
//}

static List *
getAllAttrWithoutSpecial(QueryOperator *op)
{
    List *norm = copyObject(op->schema->attrDefs);
    List *result = NIL;

    FOREACH(AttributeDef,a,norm)
    {
        if (strcmp(a->attrName, RESULT_TID_ATTR) != 0
                && strcmp(a->attrName, PROV_DUPL_COUNT_ATTR) != 0)
            result = appendToTailOfList(result, a);
    }

    return result;
}

static List *
getNormalAttrWithoutSpecial(QueryOperator *op)
{
    List *norm = getNormalAttrs(op);
    List *result = NIL;

    FOREACH(AttributeDef,a,norm)
    {
        if (strcmp(a->attrName, RESULT_TID_ATTR) != 0
                && strcmp(a->attrName, PROV_DUPL_COUNT_ATTR) != 0)
            result = appendToTailOfList(result, a);
    }

    return result;
}


static List *
getResultTidAndProvDupAttrsProjExprs(QueryOperator *op)
{
    List *result = NIL;

    result = LIST_MAKE(
            createFullAttrReference(RESULT_TID_ATTR,
                    0,
                    INT_VALUE(GET_STRING_PROP(op, PROP_RESULT_TID_ATTR)),
                    INVALID_ATTR,
                    getRowNumDT()),
            createFullAttrReference(PROV_DUPL_COUNT_ATTR,
                    0,
                    INT_VALUE(GET_STRING_PROP(op, PROP_PROV_DUP_ATTR)),
                    INVALID_ATTR,
                    getRowNumDT())
    );

    return result;
}

static List *
removeSpecialAttrsFromNormalProjectionExprs(List *projExpr)
{
    List *result = NIL;

    FOREACH(AttributeReference,a,projExpr)
    {
        if (strcmp(a->name, RESULT_TID_ATTR) != 0
            && strcmp(a->name, PROV_DUPL_COUNT_ATTR) != 0)
            result = appendToTailOfList(result, a);
    }

    return result;
}


static QueryOperator *
rewritePI_CSComposableOrderOp(OrderOperator *op, PICSComposableRewriteState *state)
{
	REWR_UNARY_SETUP_PIC(OrderBy);
	REWR_UNARY_CHILD_PIC();

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) rewr, OP_LCHILD(rewr));

    // add result TID and prov duplicate attributes
    addResultTIDAndProvDupAttrs((QueryOperator *) rewr, TRUE);

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

    LOG_RESULT_AND_RETURN(PICS-Composable,OrderBy);
}

#define NEW_RESULT_TID_ATTR "_result_tid_new"

static QueryOperator *
rewritePI_CSComposableLimitOp(LimitOperator *op, PICSComposableRewriteState *state)
{
	REWR_UNARY_SETUP_PIC(Limit);
    rewrInput = rewritePI_CSComposableOperator(OP_LCHILD(op), state);
    SelectionOperator *s;
    Node *limit = copyObject(op->limitExpr);
    Node *offset = copyObject(op->offsetExpr);
    Node *condition;
    List *orderBy = NIL;
    WindowOperator *w;
    ProjectionOperator *p;
    /* List *projExprs = NIL; */
    Node *tidAttr = (Node *) createFullAttrReference(RESULT_TID_ATTR,
                                                     0,
                                                     INT_VALUE(GET_STRING_PROP(rewrInput,PROP_RESULT_TID_ATTR)),
                                                     0,
                                                     state->rowNumDT);

    // compute new result tids which follow the input order but preserve which tuples have the same TID
    // dense_rank() OVER (ORDER BY _result_tid) AS _result_tid_new
    // if there is an order by operator below we need to take its order O into account
    // dense_rank() OVER (ORDER BY O, _result_tid) AS _result_tid_new
    if(isA((Node *) rewrInput,OrderOperator))
    {
        OrderOperator *o = (OrderOperator *) rewrInput;
        orderBy = copyObject(o->orderExprs); //TODO expect attribute references
        orderBy = appendToTailOfList(orderBy, copyObject(tidAttr));
    }
    else
    {
        orderBy = singleton(copyObject(tidAttr));
    }
    Node *tidFunc = (Node *) createFunctionCall(DENSE_RANK_FUNC_NAME, NIL);
    w = createWindowOp(tidFunc,
                       NIL,
                       orderBy,
                       NULL,
                       strdup(NEW_RESULT_TID_ATTR),
                       rewrInput,
                       NIL);

    w->op.provAttrs = copyObject(rewrInput->provAttrs);
    addParent(rewrInput, (QueryOperator *) w);

    // add projection to use new _result_tid_new attribute as _result_tid attribute
    List *projAttrNames = deepCopyStringList(getAttrNames(rewrInput->schema));
    projAttrNames = sublist(projAttrNames, 0, -3);
    projAttrNames = appendToTailOfList(projAttrNames, NEW_RESULT_TID_ATTR);
    projAttrNames = appendToTailOfList(projAttrNames, PROV_DUPL_COUNT_ATTR);

    p = (ProjectionOperator *) createProjOnAttrsByName((QueryOperator *) w, projAttrNames, getAttrNames(rewrInput->schema));
    p->op.provAttrs = copyObject(rewrInput->provAttrs);
    addChildOperator((QueryOperator *) p, (QueryOperator *) w);

    // create selection with _result_tid > offset AND _result_tid <= offset + limit
    if(limit && offset)
    {
        condition = AND_EXPRS((Node *) createOpExpr(OPNAME_GT,
                                           LIST_MAKE(copyObject(tidAttr),
                                                     offset)),
                              (Node *) createOpExpr(OPNAME_LE,
                                           LIST_MAKE(copyObject(tidAttr),
                                                     (Node *) createOpExpr(OPNAME_ADD,
                                                                           LIST_MAKE(offset,
                                                                                     limit)))));
    }
    // create selection with _result_tid <= limit
    else if (limit)
    {
        condition = (Node *) createOpExpr(OPNAME_LE,
                                          LIST_MAKE(copyObject(tidAttr),
                                                    limit));
    }
    // create selection with _result_tid > offset
    else
    {
        condition = (Node *) createOpExpr(OPNAME_GT,
                                          LIST_MAKE(copyObject(tidAttr),
                                                    offset));
    }

    // replace limit with selection
    s = createSelectionOp(condition, NULL, NIL, getAttrNames(rewrInput->schema));
    s->op.schema = copyObject(p->op.schema);
    rewr = (QueryOperator *) s;

    // add result TID and prov duplicate attributes from rewrInput (use it as child temporarily)
    rewr->inputs = singleton(rewrInput);
    addResultTIDAndProvDupAttrs((QueryOperator *) rewr, FALSE);
    rewr->inputs = NIL;
    rewr->provAttrs = copyObject(rewrInput->provAttrs);

    // actual child is the projection on top of the window operator
    addChildOperator(rewr, (QueryOperator *) p);

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

    LOG_RESULT_AND_RETURN(PICS-Composable,OrderBy);
}
