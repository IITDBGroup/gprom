/*-----------------------------------------------------------------------------
 *
 * pi_cs_main.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "provenance_rewriter/prov_schema.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "model/bitset/bitset.h"
#include "parser/parser_jp.h"
#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"
#include "provenance_rewriter/semiring_combiner/sc_main.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "metadata_lookup/metadata_lookup.h"

typedef struct PICSRewriteState {
    HashMap *opToRewrittenOp; // mapping op address to address of rewritten operator
	HashMap *origOps; // mapping op to address of a copies of the original query graph for reuse (e.g., aggregation with join)
	HashMap *provCounts; // map from tablename / prov prefix -> count
	HashMap *coarseGrainedOpRefCount; // map from op to count
	Node *asOf;
} PICSRewriteState;

#define REWR_NULLARY_SETUP_PI(optype)			\
	REWR_NULLARY_SETUP(PI-CS,optype)

#define REWR_UNARY_SETUP_PI(optype)				\
	REWR_UNARY_SETUP(PI-CS,optype)

#define REWR_BINARY_SETUP_PI(optype)			\
	REWR_BINARY_SETUP(PI-CS,optype)

#define REWR_UNARY_CHILD_PI()					\
	REWR_UNARY_CHILD(rewritePI_CSOperator)

#define REWR_BINARY_CHILDREN_PI()				\
	REWR_BINARY_CHILDREN(rewritePI_CSOperator)

#define REWR_NULLARY_SETUP_COARSE(optype)		\
	REWR_NULLARY_SETUP(COARSE-GRAINED,optype)

#define REWR_UNARY_SETUP_COARSE(optype)			\
	REWR_UNARY_SETUP(COARSE-GRAINED,optype)

static QueryOperator *rewritePI_CSOperator(QueryOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSLimit(LimitOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSSelection(SelectionOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSProjection(ProjectionOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSJoin(JoinOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSAggregation(AggregationOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSAggregationReductionModel(AggregationOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSSet(SetOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSTableAccess(TableAccessOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSConstRel(ConstRelOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSDuplicateRemOp(DuplicateRemoval *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSOrderOp(OrderOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSJsonTableOp(JsonTableOperator *op, PICSRewriteState *state);
static QueryOperator *rewritePI_CSNestingOp(NestingOperator *op, PICSRewriteState *state);

static QueryOperator *rewriteCoarseGrainedTableAccess(TableAccessOperator *op, PICSRewriteState *state);
static QueryOperator *rewriteCoarseGrainedAggregation(AggregationOperator *op, PICSRewriteState *state);
static QueryOperator *rewriteUseCoarseGrainedAggregation(AggregationOperator *op, PICSRewriteState *state);
static QueryOperator *rewriteUseCoarseGrainedTableAccess(TableAccessOperator *op, PICSRewriteState *state);

static QueryOperator *rewritePI_CSReuseRewrittenOp(QueryOperator *op, PICSRewriteState *state);
static QueryOperator *addUserProvenanceAttributes(QueryOperator *op,
        List *userProvAttrs, boolean showIntermediate, char *provRelName,
												   char *provAddRelName, PICSRewriteState *state);
static QueryOperator *addIntermediateProvenance(QueryOperator *op,
												List *userProvAttrs, Set *ignoreProvAttrs, char *provRelName, PICSRewriteState *state);
static QueryOperator *rewritePI_CSAddProvNoRewrite(QueryOperator *op, List *userProvAttrs, PICSRewriteState * state);
static QueryOperator *rewritePI_CSUseProvNoRewrite(QueryOperator *op, List *userProvAttrs, PICSRewriteState * state);


/* provenance sketch */
static List* getCondList(AttributeReference *attr, List *rangeList);
static List* getWhenList(List *condList);
static List* getWhenListOracle(List *condList);
static char* getBinarySearchArryList(List *rangeList);
static Node* bsCaseWhen(AttributeReference *attr, List *l, int low, int high, char *s);
static Node* baCaseWhenBar(AttributeReference *attr, List *l, int low, int high);

/* static Node *asOf; */
/* static RelCount *nameState; */
//static QueryOperator *provComputation;

QueryOperator *
rewritePI_CS(ProvenanceComputation  *op)
{
//    List *provAttrs;
	PICSRewriteState *state = NEW(PICSRewriteState);

    START_TIMER("rewrite - PI-CS rewrite");

    state->opToRewrittenOp = NEW_MAP(Constant,Constant);
    state->origOps = NEW_MAP(Constant,Constant);
	state->provCounts = NEW_MAP(Constant,Constant);
	state->coarseGrainedOpRefCount = NEW_MAP(Constant,Constant);

    // unset relation name counters
    /* nameState = (RelCount *) NULL; */

    DEBUG_NODE_BEATIFY_LOG("*\n************************************\nREWRITE INPUT\n"
            "******************************\n", op);

//    //mark the number of table - used in provenance scratch
//    markNumOfTableAccess((QueryOperator *) op);

    QueryOperator *rewRoot = OP_LCHILD(op);
    DEBUG_NODE_BEATIFY_LOG("rewRoot is:", rewRoot);

    // cache asOf
    state->asOf = op->asOf;

    // get provenance attrs
//    provAttrs = getQueryOperatorAttrNames((QueryOperator *) op);

    //check for SC aggregation optimizer
    if(isSemiringCombinerActivatedOp((QueryOperator *) op))
	{
        SET_STRING_PROP(rewRoot, PROP_PC_SC_AGGR_OPT, getSemiringCombinerAddExpr((QueryOperator *) op));
    }

    // rewrite subquery under provenance computation
    rewRoot = rewritePI_CSOperator(rewRoot, state);

    /* // update root of rewritten subquery */
    /* rewRoot = OP_LCHILD(op); */

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, rewRoot);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root is:", rewRoot);

    // Check if we should export
    if(HAS_STRING_PROP(op, PROP_TRANSLATE_AS))
    {
    	rewRoot = rewriteTransformationProvenance(rewRoot);
    }

    STOP_TIMER("rewrite - PI-CS rewrite");

    return rewRoot;
}

static QueryOperator *
rewritePI_CSOperator (QueryOperator *op, PICSRewriteState *state)
{
	boolean combinerAggrOpt = HAS_STRING_PROP(op,  PROP_PC_SC_AGGR_OPT);
    boolean showIntermediate = HAS_STRING_PROP(op,  PROP_SHOW_INTERMEDIATE_PROV);
    boolean noRewriteUseProv = HAS_STRING_PROP(op, PROP_USE_PROVENANCE);
    boolean noRewriteHasProv = HAS_STRING_PROP(op, PROP_HAS_PROVENANCE);
    boolean isDummyHasProvProj = HAS_STRING_PROP(op, PROP_DUMMY_HAS_PROV_PROJ);
    boolean rewriteAddProv = HAS_STRING_PROP(op, PROP_ADD_PROVENANCE);
    boolean coarseGrainedUseProv = HAS_STRING_PROP(op, USE_PROP_COARSE_GRAINED_AGGREGATION_MARK) ? TRUE : FALSE;

    List *userProvAttrs = (List *) getStringProperty(op, PROP_USER_PROV_ATTRS);
    List *addProvAttrs = NIL;
    Set *ignoreProvAttrs = (Set *) getStringProperty(op, PROP_PROV_IGNORE_ATTRS);
    QueryOperator *rewrittenOp;
    char *provRelName = HAS_STRING_PROP(op, PROP_PROV_REL_NAME) ?
            STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME)) : NULL;
    char *provAddRelName = HAS_STRING_PROP(op, PROP_PROV_ADD_REL_NAME) ?
            STRING_VALUE(getStringProperty(op, PROP_PROV_ADD_REL_NAME)) : NULL;

    if (rewriteAddProv)
        addProvAttrs = (List *)  GET_STRING_PROP(op, PROP_ADD_PROVENANCE);

    DEBUG_LOG("REWRITE OPERATIONS:\n\tshow intermediates: %s\n\tuse prov: %s"
			  "\n\thas prov: %s\n\tadd prov: %s"
			  "\n\tuser prov attrs: %s"
			  "\n\tadd prov attrs: %s"
			  "\n\tignore prov attrs: %s"
			  "\n\talready was rewrriten: %s",
			  showIntermediate ? "T": "F",
			  noRewriteUseProv ? "T": "F",
			  noRewriteHasProv ? "T": "F",
			  rewriteAddProv ? "T": "F",
			  nodeToString(userProvAttrs),
			  nodeToString(addProvAttrs),
			  nodeToString(ignoreProvAttrs),
			  isOpRewritten(state->opToRewrittenOp, op) ? "T" : "F");

	// when operator is already rewritten, then just reuse the rewritten operator, but change provenance attribute names
	if(isOpRewritten(state->opToRewrittenOp, op))
	{
		return rewritePI_CSReuseRewrittenOp(op, state);
	}


    if (noRewriteUseProv)
        return rewritePI_CSAddProvNoRewrite(op, userProvAttrs, state);
    if (isDummyHasProvProj)
    {
        userProvAttrs = (List *) getStringProperty(OP_LCHILD(op), PROP_USER_PROV_ATTRS);
        return rewritePI_CSUseProvNoRewrite(op, userProvAttrs, state);
    }

    if(combinerAggrOpt)
	{
    	INFO_OP_LOG("go SEMIRING COMBINER aggregation optimization:",op);
    }
    switch(op->type)
    {
	    case T_LimitOperator:
			DEBUG_LOG("go limit");
			rewrittenOp = rewritePI_CSLimit((LimitOperator *) op, state);
        break;
        case T_SelectionOperator:
            DEBUG_LOG("go selection");
            rewrittenOp = rewritePI_CSSelection((SelectionOperator *) op, state);
            break;
        case T_ProjectionOperator:
            DEBUG_LOG("go projection");
            rewrittenOp = rewritePI_CSProjection((ProjectionOperator *) op, state);
            break;
        case T_AggregationOperator:
        	if(combinerAggrOpt) {
        		if(coarseGrainedUseProv)
        			rewrittenOp = rewriteUseCoarseGrainedAggregation ((AggregationOperator *) op, state);
        		else
        			rewrittenOp = rewriteCoarseGrainedAggregation ((AggregationOperator *) op, state);
        		INFO_LOG("go SEMIRING COMBINER aggregation optimization!");
        	}
        	else
        	{
				DEBUG_LOG("go aggregation");
				if(isRewriteOptionActivated(OPTION_AGG_REDUCTION_MODEL_REWRITE))
            		rewrittenOp = rewritePI_CSAggregationReductionModel ((AggregationOperator *) op, state);
				else
            		rewrittenOp = rewritePI_CSAggregation ((AggregationOperator *) op, state);
        	}
            break;
        case T_JoinOperator:
            DEBUG_LOG("go join");
            rewrittenOp = rewritePI_CSJoin((JoinOperator *) op, state);
            break;
        case T_SetOperator:
            DEBUG_LOG("go set");
            rewrittenOp = rewritePI_CSSet((SetOperator *) op, state);
            break;
        case T_TableAccessOperator:
            DEBUG_LOG("go table access");
            if(HAS_STRING_PROP(op, USE_PROP_COARSE_GRAINED_TABLEACCESS_MARK))
				rewrittenOp = rewriteUseCoarseGrainedTableAccess((TableAccessOperator *) op, state);
            else if(HAS_STRING_PROP(op, PROP_COARSE_GRAINED_TABLEACCESS_MARK))
				rewrittenOp = rewriteCoarseGrainedTableAccess((TableAccessOperator *) op, state);
            else
				rewrittenOp = rewritePI_CSTableAccess((TableAccessOperator *) op, state);
            break;
        case T_ConstRelOperator:
            DEBUG_LOG("go const rel operator");
            rewrittenOp = rewritePI_CSConstRel((ConstRelOperator *) op, state);
            break;
        case T_DuplicateRemoval:
            DEBUG_LOG("go duplicate removal operator");
            rewrittenOp = rewritePI_CSDuplicateRemOp((DuplicateRemoval *) op, state);
            break;
        case T_OrderOperator:
            DEBUG_LOG("go order operator");
            rewrittenOp = rewritePI_CSOrderOp((OrderOperator *) op, state);
            break;
        case T_JsonTableOperator:
            DEBUG_LOG("go JsonTable operator");
            rewrittenOp = rewritePI_CSJsonTableOp((JsonTableOperator *) op, state);
	     break;
        case T_NestingOperator:
            DEBUG_LOG("go nesting operator");
            rewrittenOp = rewritePI_CSNestingOp((NestingOperator *) op, state);
            break;
        default:
            FATAL_LOG("no rewrite implemented for operator ", nodeToString(op));
            return NULL;
    }

    if (showIntermediate)
        rewrittenOp = addIntermediateProvenance(rewrittenOp, userProvAttrs, ignoreProvAttrs, provRelName, state);

    if (rewriteAddProv)
        rewrittenOp = addUserProvenanceAttributes(rewrittenOp, addProvAttrs, showIntermediate, provRelName, provAddRelName, state);

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel(rewrittenOp));

	// associate rewritten operator with original operator
	setRewrittenOp(state->opToRewrittenOp, op, rewrittenOp);

    return rewrittenOp;
}

static QueryOperator *
rewritePI_CSReuseRewrittenOp(QueryOperator *op, PICSRewriteState *state)
{
	QueryOperator *rewrOp = getRewrittenOp(state->opToRewrittenOp, op);
	QueryOperator *rewr;
	List *normalAttrNames = getNormalAttrNames(rewrOp);
	List *oldProjAttrNames = getOpProvenanceAttrNames(rewrOp);
	List *newProvAttrNames = NIL;
	List *oldAttrs;
	List *newAttrs;
	boolean isCoarseGrained = HAS_STRING_PROP(op, PROP_LEVEL_AGGREGATION_MARK);

	DEBUG_NODE_BEATIFY_LOG("reuse rewritten subquery: ", rewrOp);

	// create new provenance attribute names
	FOREACH(KeyValue,k,opGetProvAttrInfo(rewrOp))
	{
		char *table = STRING_VALUE(k->key);
		List *attNames = constStringListToStringList((List *) k->value);
		int cnt = increaseRefCount(state->provCounts, table);

		if (isCoarseGrained)
		{
			newProvAttrNames = concatTwoLists(newProvAttrNames, getCoarseGrainedAttrNames(table, attNames, cnt));
		}
		else
		{
			newProvAttrNames = concatTwoLists(newProvAttrNames, getProvenanceAttrNames(table, attNames, cnt));
		}
	}

	// create projection to rename provenance attributes
	oldAttrs = CONCAT_LISTS(deepCopyStringList(normalAttrNames),
							oldProjAttrNames);
	newAttrs = CONCAT_LISTS(deepCopyStringList(normalAttrNames),
							newProvAttrNames);

	DEBUG_LOG("rename %s to %s", stringListToString(oldAttrs), stringListToString(newAttrs));

	rewr = createProjOnAttrsByName(rewrOp, oldAttrs, newAttrs);
	addChildOperator(rewr, rewrOp);

	// add result TID and dupl attributes
	COPY_PROV_INFO(rewr, rewrOp);

	LOG_RESULT_AND_RETURN(REUSE);
}


static QueryOperator *
addUserProvenanceAttributes (QueryOperator *op,
							 List *userProvAttrs,
							 boolean showIntermediate,
							 char *provRelName,
							 char *provAddRelName,
							 PICSRewriteState *state)
{
    QueryOperator *proj;
    List *attrNames = NIL;
    List *projExpr = NIL;
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
        if (provAddRelName != NULL)
            tableName = provAddRelName;
        else if (provRelName != NULL)
            tableName = provRelName;
        else
            FATAL_LOG("should have at least add provenance rel name or provenance rel name given");
    }

    if (showIntermediate)
        relAccessCount = getCurRelNameCount(state->provCounts, tableName) - 1;
    else
        relAccessCount = increaseRefCount(state->provCounts, tableName);

    DEBUG_LOG("REWRITE-PICS - Add Intermediate Provenance Attrs <%s> <%u>",  tableName, relAccessCount);

    attrNames = getQueryOperatorAttrNames(op);
    provAttrPos = copyObject(op->provAttrs);

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, op->schema->attrDefs)
    {
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

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
    DEBUG_LOG("add intermediate provenance\n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
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

    DEBUG_LOG("added projection: %s", operatorToOverviewString((Node *) proj));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) proj));

    return proj;
}

static QueryOperator *
addIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs, char *provRelName, PICSRewriteState *state)
{
    QueryOperator *proj;
    List *attrNames = NIL;
    List *projExpr = NIL;
    List *provAttrPos = NIL;
    List *normalAttrExpr = getNormalAttrProjectionExprs(op);
    List *temp = NIL;
    int cnt = 0;
    char *newAttrName;
    int relAccessCount;
    char *tableName; // = "INTERMEDIATE";

    if (isA(op,TableAccessOperator))
        tableName = ((TableAccessOperator *) op)->tableName;
    else
        tableName = provRelName;

    relAccessCount = increaseRefCount(state->provCounts, tableName);

    DEBUG_LOG("REWRITE-PICS - Add Intermediate Provenance Attrs <%s> <%u>",  tableName, relAccessCount);

    attrNames = getQueryOperatorAttrNames(op);
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
    FOREACH(AttributeDef, attr, op->schema->attrDefs)
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

    DEBUG_LOG("added projection: %s", operatorToOverviewString((Node *) proj));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) proj));

    return proj;
}

static QueryOperator *
rewritePI_CSAddProvNoRewrite (QueryOperator *op, List *userProvAttrs, PICSRewriteState *state)
{
//    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;
    int relAccessCount;
    int numProvAttrs = LIST_LENGTH(userProvAttrs);
    int numNormalAttrs = LIST_LENGTH(op->schema->attrDefs);
    int cnt = 0;
    char *tableName; // = "INTERMEDIATE";

    if (isA(op,TableAccessOperator))
        tableName = ((TableAccessOperator *) op)->tableName;
    else
        tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME));

    relAccessCount = increaseRefCount(state->provCounts, tableName);

    DEBUG_LOG("REWRITE-PICS - Add Provenance Attrs <%s> <%u>",
            tableName, relAccessCount);

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, op->schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
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

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, numNormalAttrs, numNormalAttrs + numProvAttrs - 1, 1);

    DEBUG_LOG("no rewrite add provenance, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;

    // Switch the subtree with this newly created projection operator.
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite add provenance attrs:\n%s", operatorToOverviewString((Node *) newpo));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) newpo));

    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSUseProvNoRewrite (QueryOperator *op, List *userProvAttrs, PICSRewriteState *state)
{
    int relAccessCount;
    int curPos;
    char *tableName; // = "USER";
    QueryOperator *c = OP_LCHILD(op);
//    List *provAttrs = c->provAttrs;
    ProjectionOperator *p = (ProjectionOperator *) op;
    boolean isTableAccess = isA(c,TableAccessOperator);

    if (isTableAccess)
        tableName = ((TableAccessOperator *) c)->tableName;
    else
        tableName = STRING_VALUE(getStringProperty(c, PROP_PROV_REL_NAME));

    DEBUG_LOG("Use existing provenance attributes %s for %s",
            beatify(nodeToString(userProvAttrs)), tableName);

    relAccessCount = increaseRefCount(state->provCounts, tableName);
    curPos = getNumAttrs(op);
    // for table access operations we need to add a projection that renames the attributes
//    if (isTableAccess)
//    {

//        proj = createProjOnAllAttrs(op);

        // Switch the subtree with this newly created projection operator
//        switchSubtrees(op, proj);

        // Add child to the newly created projection operator
//        addChildOperator(proj, op);

        FOREACH(Constant,a,userProvAttrs)
        {
            char *name = STRING_VALUE(a);
            char *newName;
            int pos = getAttrPos(c, name);
            AttributeDef *attr, *origAttr;
            AttributeReference *aRef;

            origAttr = getAttrDefByPos(c, pos);
            newName = getProvenanceAttrName(tableName, name, relAccessCount);

            op->provAttrs = appendToTailOfListInt(op->provAttrs, curPos++);

            attr = createAttributeDef(newName, origAttr->dataType);
            op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs, attr);

            aRef = createFullAttrReference(name,0,pos,INVALID_ATTR, origAttr->dataType);
            p->projExprs = appendToTailOfList(p->projExprs, aRef);

            // in parent operators adapt attribute references to use new name
//            FOREACH(QueryOperator,p,proj->parents)
//            {
//                List *aRefs = findOperatorAttrRefs(p);
//                int childPos = getChildPosInParent(p,proj);
//
//                FOREACH(AttributeReference,a,aRefs)
//                {
//                    if (a->fromClauseItem == childPos && a->attrPosition == pos)
//                        a->name = strdup(name);
//                }
//            }
        }

//        List *provProj = getProvAttrProjectionExprs(op);

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
            ASSERT(checkModel(op));

        return op;
//    }
//    // for non-tableaccess operators simply change the attribute names and mark the attributes as provenance attributes
//    else
//    {
//        FOREACH(Constant,a,userProvAttrs)
//        {
//            char *name = STRING_VALUE(a);
//            int pos = getAttrPos(op, name);
////            char *oldName;
//            AttributeDef *attr;
//
//            attr = getNthOfListP(op->schema->attrDefs, pos);
//            name = getProvenanceAttrName(tableName, name, relAccessCount);
////            oldName = attr->attrName;
//            attr->attrName = name;
//            provAttrs = appendToTailOfListInt(provAttrs, pos);
//
//            // in parent operators adapt attribute references to use new name
//            FOREACH(QueryOperator,p,op->parents)
//            {
//                List *aRefs = findOperatorAttrRefs(p);
//                int childPos = getChildPosInParent(p,op);
//
//                FOREACH(AttributeReference,a,aRefs)
//                {
//                    if (a->fromClauseItem == childPos && a->attrPosition == pos)
//                        a->name = strdup(name);
//                }
//            }
//        }
//
//        op->provAttrs = provAttrs;
//
//        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
//            ASSERT(checkModel(op));
//
//        return op;
//    }
}

//TODO this may fail for regular PI-CS if there are duplicates, we should throw an exception
static QueryOperator *
rewritePI_CSLimit(LimitOperator *op, PICSRewriteState *state)
{
	REWR_UNARY_SETUP_PI(Limit);

    //add semiring options
    addSCOptionToChild((QueryOperator *) op,OP_LCHILD(op));

	// rewrite child first
	REWR_UNARY_CHILD_PI();

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) rewr, OP_LCHILD(rewr));

	LOG_RESULT_AND_RETURN(Limit);
}


static QueryOperator *
rewritePI_CSSelection(SelectionOperator *op, PICSRewriteState *state)
{
	REWR_UNARY_SETUP_PI(Selection);

    //add semiring options
    addSCOptionToChild((QueryOperator *) op,OP_LCHILD(op));

    // rewrite child first
	REWR_UNARY_CHILD_PI();

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) rewr, OP_LCHILD(rewr));

	LOG_RESULT_AND_RETURN(Selection);
}

static QueryOperator *
rewritePI_CSProjection (ProjectionOperator *op, PICSRewriteState *state)
{
	REWR_UNARY_SETUP_PI(Projection);
	ProjectionOperator *rewrP;

	// rewrite child
	REWR_UNARY_CHILD_PI();
	rewrP = (ProjectionOperator *) rewr;

    //add semiring options
    addSCOptionToChild((QueryOperator *) rewr,OP_LCHILD(rewr));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(rewr);
    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        rewrP->projExprs = appendToTailOfList(rewrP->projExprs,
                 createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
    }

    // adapt schema
    addProvenanceAttrsToSchema(rewr, OP_LCHILD(rewr));

	// copy prov info
	COPY_PROV_INFO(rewr, rewrInput);

	LOG_RESULT_AND_RETURN(Projection);
}

static QueryOperator *
rewritePI_CSJoin (JoinOperator *op, PICSRewriteState *state)
{
	REWR_BINARY_SETUP_PI(Join);
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
    List *rNormAttrs;
    int numLAttrs, numRAttrs;
	List *provInfo;

    numLAttrs = LIST_LENGTH(lChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rChild->schema->attrDefs);


    // rewrite children
    //add semiring options
    addSCOptionToChild((QueryOperator *) op,lChild);
    addSCOptionToChild((QueryOperator *) op,rChild);

	REWR_BINARY_CHILDREN_PI();

    // adapt schema for join op
    // get attributes from right input
    rNormAttrs = sublist(rewr->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    rewr->schema->attrDefs = sublist(copyObject(rewr->schema->attrDefs), 0, numLAttrs - 1);

    addProvenanceAttrsToSchema(rewr, rewrLeftInput);
    rewr->schema->attrDefs = CONCAT_LISTS(rewr->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchema(rewr, rewrRightInput);

    // add projection to put attributes into order on top of join op
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) rewr),
            getProvAttrProjectionExprs((QueryOperator *) rewr));
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

	// recreate schema and set provenance attribute positions
	clearAttrsFromSchema((QueryOperator *) proj);
    addNormalAttrsToSchema((QueryOperator *) proj, rewr);
    addProvenanceAttrsToSchema((QueryOperator *) proj, rewr);
    addChildOperator((QueryOperator *) proj, rewr);

	// copy prov info
	rewr = (QueryOperator *) proj;

	// provenance info is concatenation of child prov infos
	provInfo = CONCAT_LISTS(
		(List *) GET_STRING_PROP(rewrLeftInput, PROP_PROVENANCE_TABLE_ATTRS),
		(List *) GET_STRING_PROP(rewrRightInput, PROP_PROVENANCE_TABLE_ATTRS));

	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

	LOG_RESULT_AND_RETURN(Join);
}


static QueryOperator *
rewritePI_CSNestingOp (NestingOperator *op, PICSRewriteState *state)
{
	REWR_BINARY_SETUP_PI(NestingOperator);
    QueryOperator *llChild = OP_LCHILD(op);
    QueryOperator *rrChild = OP_RCHILD(op);
    List *rNormAttrs;
    int numLAttrs, numRAttrs;

    numLAttrs = LIST_LENGTH(llChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rrChild->schema->attrDefs);


    // rewrite children

    //add semiring options
    addSCOptionToChild((QueryOperator *) op,llChild);
    addSCOptionToChild((QueryOperator *) op,rrChild);

	REWR_BINARY_CHILDREN_PI();

    // get attributes from right input
    rNormAttrs = sublist(op->op.schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    rewr->schema->attrDefs = sublist(copyObject(rewr->schema->attrDefs), 0, numLAttrs - 1);

    // adapt schema for join op
//    clearAttrsFromSchema((QueryOperator *) op);
//    addNormalAttrsToSchema(o, lChild);
    addProvenanceAttrsToSchema(rewr, rewrLeftInput);
    rewr->schema->attrDefs = CONCAT_LISTS(rewr->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchema(rewr, rewrRightInput);

    // add projection to put attributes into order on top of join op
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) rewr),
            getProvAttrProjectionExprs((QueryOperator *) rewr));
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) rewr);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) rewr);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) rewr);

	// set rewritten op
	rewr = (QueryOperator *) proj;
	LOG_RESULT_AND_RETURN(NestingOperator);
}


/*
 * Rewrite an aggregation operator:
 *      - replace aggregation with projection over join between the aggregation
 *       and the aggregation rewritten input
 */
static QueryOperator *
rewritePI_CSAggregation (AggregationOperator *op, PICSRewriteState *state)
{
	REWR_UNARY_SETUP_PI(Aggregation);
    JoinOperator *joinProv;
    ProjectionOperator *proj;
    QueryOperator *origAgg;
	QueryOperator *curOp;
    int numGroupAttrs = LIST_LENGTH(op->groupBy);

    DEBUG_LOG("REWRITE-PICS - Aggregation");

    // copy aggregation
    origAgg = (QueryOperator *) getOrSetOpCopy(state->origOps, (QueryOperator *) op);;

    // rewrite aggregation input copy
	rewrInput = rewritePI_CSOperator(OP_LCHILD(op), state);
	curOp = rewrInput;

    // add projection including group by expressions if necessary
    if(op->groupBy != NIL)
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

        attrNames = CONCAT_LISTS(gbNames, getOpProvenanceAttrNames(curOp));
        groupByProjExprs = CONCAT_LISTS(groupByProjExprs, getProvAttrProjectionExprs(curOp));

        groupByProj = createProjectionOp(groupByProjExprs,
                        curOp, NIL, attrNames);
        CREATE_INT_SEQ(provAttrs, numGroupAttrs, numGroupAttrs + getNumProvAttrs(curOp) - 1,1);
        groupByProj->op.provAttrs = provAttrs;
        curOp->parents = singleton(groupByProj);
        curOp = (QueryOperator *) groupByProj;
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
	    joinCond = (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(createConstInt(1), createConstInt(1)));

    // create join operator
    List *joinAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getQueryOperatorAttrNames(curOp));
    joinProv = createJoinOp(joinT, joinCond, LIST_MAKE(origAgg, curOp), NIL,
            joinAttrNames);
    joinProv->op.provAttrs = copyObject(curOp->provAttrs);
    FOREACH_LC(lc,joinProv->op.provAttrs)
        lc->data.int_value += getNumAttrs(origAgg);

	// create projection expressions for final projection
    List *projAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getOpProvenanceAttrNames(curOp));
    List *projExprs = CONCAT_LISTS(getNormalAttrProjectionExprs(origAgg),
                                getProvAttrProjectionExprs((QueryOperator *) joinProv));

    // create final projection and replace aggregation subtree with projection
	proj = createProjectionOp(projExprs, (QueryOperator *) joinProv, NIL, projAttrNames);
	joinProv->op.parents = singleton(proj);
	CREATE_INT_SEQ(proj->op.provAttrs, getNumNormalAttrs((QueryOperator *) origAgg),
	        getNumNormalAttrs((QueryOperator *) origAgg) + getNumProvAttrs((QueryOperator *) joinProv) - 1,1);

	// switch provenance computation with original aggregation
	/* switchSubtrees((QueryOperator *) op, (QueryOperator *) proj); */
    addParent(origAgg, (QueryOperator *) joinProv);
    addParent(curOp, (QueryOperator *) joinProv);

    // adapt schema for final projection
	rewr = (QueryOperator *) proj;

	// copy provenance table and attr info
	COPY_PROV_INFO(rewr,rewrInput);

	LOG_RESULT_AND_RETURN(Aggregation);
}


/*
 * Rewrite an aggregation operator using reduction model:
 *      - the difference with above is: handle MIN/MAX - only treat the necessary tuple (min/max tuple) as the provenance
 *      - not all tuple in the same group
 */
static QueryOperator *
rewritePI_CSAggregationReductionModel (AggregationOperator *op, PICSRewriteState *state)
{
    JoinOperator *joinProv;
    ProjectionOperator *proj;
    QueryOperator *aggInput;
    QueryOperator *origAgg;
    int numGroupAttrs = LIST_LENGTH(op->groupBy);

    DEBUG_LOG("REWRITE-PICS - Aggregation - Reduction Model");

    // copy aggregation input
    origAgg = (QueryOperator *) op;
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));
    // rewrite aggregation input copy

    aggInput = rewritePI_CSOperator(aggInput, state);

	FunctionCall *fc = getNthOfListP(op->aggrs, 0);
	List *aggAttrs = getAttrReferences((Node *) op->aggrs);
	int numAggAttrs = LIST_LENGTH(aggAttrs);
	int aggAttrsLen = 0;
    if(streq(fc->functionname, "MAX") || streq(fc->functionname, "MIN"))
    		aggAttrsLen = numAggAttrs;

    // add projection including group by expressions if necessary
    if(op->groupBy != NIL)
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

        attrNames = CONCAT_LISTS(gbNames, getOpProvenanceAttrNames(aggInput));
        groupByProjExprs = CONCAT_LISTS(groupByProjExprs, getProvAttrProjectionExprs(aggInput));

        if(streq(fc->functionname, "MAX") || streq(fc->functionname, "MIN"))
        {
            List *aggProjExprs = (List *) copyObject(aggAttrs);
            List *aggNames = NIL;
            FOREACH(AttributeReference, a, aggAttrs)
            		aggNames = appendToTailOfList(aggNames, strdup(a->name));
            attrNames = CONCAT_LISTS(aggNames, attrNames);
            groupByProjExprs = CONCAT_LISTS(aggProjExprs, groupByProjExprs);
        }

        groupByProj = createProjectionOp(groupByProjExprs,
                        aggInput, NIL, attrNames);
        CREATE_INT_SEQ(provAttrs, aggAttrsLen + numGroupAttrs, aggAttrsLen + numGroupAttrs + getNumProvAttrs(aggInput) - 1,1);
        groupByProj->op.provAttrs = provAttrs;
        aggInput->parents = singleton(groupByProj);
        aggInput = (QueryOperator *) groupByProj;
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
			        CONCAT_STRINGS("_P_SIDE_",name), 1, pos + aggAttrsLen, INVALID_ATTR, a->attrType);
			if(joinCond)
				joinCond = AND_EXPRS((Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA), joinCond);
			else
				joinCond = (Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA);
			pos++;
		}
	}
	// or for without group by
	else
	    joinCond = (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(createConstInt(1), createConstInt(1)));


	if(streq(fc->functionname, "MAX") || streq(fc->functionname, "MIN"))
	{
		//handle nested subquery "ALL" case - add isNull expr - so using left outer join instead of inner join
		if(HAS_STRING_PROP(op, PROP_OPT_SELECTION_MOVE_AROUND_DONE))
			joinT = JOIN_LEFT_OUTER;

		AttributeDef *ad = getNthOfListP(((QueryOperator *) op)->schema->attrDefs, 0);
		AttributeReference *lA = createFullAttrReference(strdup(ad->attrName), 0, 0, INVALID_ATTR, ad->dataType);

		List *attrList = getAttrReferences((Node *) fc);
		AttributeReference *ar = getNthOfListP(attrList, 0);

		AttributeReference *rA = createFullAttrReference(
				ar->name, 1, ar->attrPosition, INVALID_ATTR, ar->attrType);

		Node *subJoinCond = (Node *) createIsNotDistinctExpr((Node *) lA, (Node *) rA);

		if(joinCond)
			joinCond = AND_EXPRS(subJoinCond, joinCond);

		else
			joinCond = subJoinCond;
	}

    // create join operator
    List *joinAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getQueryOperatorAttrNames(aggInput));
    joinProv = createJoinOp(joinT, joinCond, LIST_MAKE(origAgg, aggInput), NIL,
            joinAttrNames);
    joinProv->op.provAttrs = copyObject(aggInput->provAttrs);
    FOREACH_LC(lc,joinProv->op.provAttrs)
        lc->data.int_value += getNumAttrs(origAgg);

	// create projection expressions for final projection
    List *projAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getOpProvenanceAttrNames(aggInput));
    List *projExprs = CONCAT_LISTS(getNormalAttrProjectionExprs(origAgg),
                                getProvAttrProjectionExprs((QueryOperator *) joinProv));

    // create final projection and replace aggregation subtree with projection
	proj = createProjectionOp(projExprs, (QueryOperator *) joinProv, NIL, projAttrNames);
	joinProv->op.parents = singleton(proj);
	CREATE_INT_SEQ(proj->op.provAttrs, getNumNormalAttrs((QueryOperator *) origAgg),
	        getNumNormalAttrs((QueryOperator *) origAgg) + getNumProvAttrs((QueryOperator *) joinProv) - 1,1);

	// switch provenance computation with original aggregation
	switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addParent(origAgg, (QueryOperator *) joinProv);
    addParent(aggInput, (QueryOperator *) joinProv);

    // adapt schema for final projection
    DEBUG_NODE_BEATIFY_LOG("Rewritten Operator tree", proj);
    return (QueryOperator *) proj;
}



static QueryOperator *
rewritePI_CSSet(SetOperator *op, PICSRewriteState *state)
{
	REWR_BINARY_SETUP_PI(Set-Operation);
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
	List *provInfo;

    //add semiring options
    addSCOptionToChild((QueryOperator *) op,lChild);
    addSCOptionToChild((QueryOperator *) op,rChild);

	// rewrite children
	REWR_BINARY_CHILDREN_PI();

    switch(op->setOpType)
    {
    case SETOP_UNION:
    {
        List *projExprs = NIL;
        List *attNames;
        List *provAttrs = NIL;
        int lProvs;
        int i;

        // left provenance attributes
        lProvs = LIST_LENGTH(rewrLeftInput->provAttrs);

        // create projection over left rewritten input
        attNames = concatTwoLists(getQueryOperatorAttrNames(rewrLeftInput),
								  getOpProvenanceAttrNames(rewrRightInput));

        // createAttrRefs for attributes of left input
        i = 0;
        provAttrs = copyObject(rewrLeftInput->provAttrs);
		projExprs = getAllAttrProjectionExprs(rewrLeftInput);

        // create NULL expressions for provenance attrs of right input
		i = LIST_LENGTH(projExprs);
		FOREACH(AttributeDef,a, getProvenanceAttrDefs(rewrRightInput))
        {
            Constant *expr;

            expr = createNullConst(a->dataType);
            projExprs = appendToTailOfList(projExprs, expr);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }
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
        attNames = CONCAT_LISTS(getNormalAttrNames(rewrRightInput),
                getOpProvenanceAttrNames(rewrLeftInput),
                getOpProvenanceAttrNames(rewrRightInput));

        // create AttrRefs for normal attributes of right input
        i = 0;
        FOREACH(AttributeDef,a,getNormalAttrs(rewrRightInput))
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i++, INVALID_ATTR, a->dataType);
            projExprs = appendToTailOfList(projExprs, att);
        }

        // create NULL expressions for provenance attrs of left input
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

        DEBUG_LOG("have created projection expressions: %s\nattribute names: "
                "%s\n provAttrs: %s\n for right UNION input",
                nodeToString(projExprs), stringListToString(attNames),
                nodeToString(provAttrs));
        ProjectionOperator *projRightChild = createProjectionOp(projExprs,
                rewrRightInput, NIL, attNames);
        ((QueryOperator *) projRightChild)->provAttrs = provAttrs;

    	// make projections of rewritten inputs the direct children of the union operation
        /* switchSubtrees(rewrLeftInput, (QueryOperator *) projLeftChild); */
        /* switchSubtrees(rewrRightInput, (QueryOperator *) projRightChild); */
        rewrLeftInput->parents = singleton(projLeftChild);
        rewrRightInput->parents = singleton(projRightChild);
		rewr->inputs = NIL;
		addChildOperator(rewr, (QueryOperator *) projLeftChild);
		addChildOperator(rewr, (QueryOperator *) projRightChild);

    	// adapt schema of union itself, we can get full provenance attributes from left input
    	addProvenanceAttrsToSchema((QueryOperator *) rewr, (QueryOperator *) projLeftChild);
    }
	break;
    case SETOP_INTERSECTION:
    {
        //create join condition
        Node *joinCond;
        List *comparisons = NIL;
        int schemaSize = LIST_LENGTH(lChild->schema->attrDefs);

        for(int i = 0 ; i < schemaSize; i++)
        {
            AttributeDef *lDef, *rDef;
            lDef = getAttrDefByPos(lChild, i);
            rDef = getAttrDefByPos(rChild, i);
            comparisons = appendToTailOfList(comparisons, createOpExpr(OPNAME_EQ,
                    LIST_MAKE(createFullAttrReference (strdup(lDef->attrName),0,i,INVALID_ATTR, lDef->dataType),
                            createFullAttrReference (strdup(rDef->attrName),1,i,INVALID_ATTR, rDef->dataType))
                    ));
        }
        joinCond = andExprList(comparisons);
        DEBUG_LOG("join cond: %s", beatify(nodeToString(joinCond)));

        //restrcuture the tree
    	JoinOperator *joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(rewrLeftInput,rewrRightInput), NIL, NIL);
    	addParent(rewrLeftInput,(QueryOperator *)  joinOp);
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

        rewr =  (QueryOperator *) joinOp;
    }
	break;
    case SETOP_DIFFERENCE:
    {
    	JoinOperator *joinOp;
    	ProjectionOperator *projOp;
		QueryOperator *diffCopy = getOrSetOpCopy(state->origOps, (QueryOperator *) op);

		// remove copy of set operator from rewritten left inputs parents list
		rewrLeftInput->parents = NIL;

    	// join provenance with rewritten right input
    	// create join condition
        Node *joinCond;
        List *joinAttrs = CONCAT_LISTS(getQueryOperatorAttrNames((QueryOperator *) diffCopy),
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

        joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(diffCopy, rewrLeftInput),
                NIL, joinAttrs);
        joinOp->op.provAttrs = copyObject(rewrLeftInput->provAttrs);
        SHIFT_INT_LIST(joinOp->op.provAttrs, getNumAttrs((QueryOperator *) diffCopy));

    	// adapt schema using projection
        List *rightProvAttrs = getProvenanceAttrDefs(rewrRightInput);
//        List *rightProvNames = getOpProvenanceAttrNames(rewrRightInput);

        List *projExpr = CONCAT_LISTS(getNormalAttrProjectionExprs((QueryOperator *) diffCopy),
                getProvAttrProjectionExprs((QueryOperator *) joinOp));
        FOREACH(AttributeDef,a,rightProvAttrs)
            projExpr = appendToTailOfList(projExpr, createNullConst(a->dataType));

        List *projAttrs = getQueryOperatorAttrNames((QueryOperator *) diffCopy);

        projOp = createProjectionOp(projExpr, (QueryOperator *) joinOp, NIL, projAttrs);
        /* projOp->op.provAttrs = copyObject(rewrLeftInput->provAttrs); */
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, OP_LCHILD(projOp));
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, rewrRightInput);
    	addParent((QueryOperator *) joinOp, (QueryOperator *) projOp);

    	// switch original set diff with projection
    	addParent((QueryOperator *) diffCopy, (QueryOperator *) joinOp);
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

	LOG_RESULT_AND_RETURN(SetOperation);
}

static QueryOperator *
rewritePI_CSTableAccess(TableAccessOperator *op, PICSRewriteState *state)
{
	REWR_NULLARY_SETUP_PI(TableAccess);
	QueryOperator *tableAcess;
    List *provAttr = NIL;
    List *projExpr = NIL;
	List *provAttrsOnly = NIL;
    char *newAttrName;
	List *provInfo = NIL;
    int relAccessCount = increaseRefCount(state->provCounts, op->tableName);
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS - Table Access <%s> <%u>", op->tableName, relAccessCount);

	tableAcess = shallowCopyQueryOperator((QueryOperator *) op);

    // copy any as of clause if there
    if (state->asOf)
        op->asOf = copyObject(state->asOf);

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
        newAttrName = getProvenanceAttrName(op->tableName, attr->attrName, relAccessCount);
		provAttrsOnly = appendToTailOfList(provAttrsOnly, createConstString(strdup(attr->attrName)));
        provAttr = appendToTailOfList(provAttr, newAttrName);
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, cnt, (cnt * 2) - 1, 1);

    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
            stringListToString(provAttr),
            nodeToString(projExpr),
            nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;
    SET_BOOL_STRING_PROP((QueryOperator *)newpo, PROP_PROJ_PROV_ATTR_DUP);

    // Switch the subtree with this newly created projection operator.
    /* switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo); */

    // Add child to the newly created projections operator,
	rewr = (QueryOperator *) newpo;
    addChildOperator(rewr, tableAcess);

	// prov info (key: TABLE_NAME, value: (ATTRIBUTES))
	provInfo = singleton(createNodeKeyValue((Node *) createConstString(op->tableName),
										    (Node *) provAttrsOnly));
	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

	LOG_RESULT_AND_RETURN(TableAccess);
}

static QueryOperator *
rewritePI_CSConstRel(ConstRelOperator *op, PICSRewriteState *state)
{
    List *provAttr = NIL;
    List *projExpr = NIL;
	List *provAttrsOnly = NIL;
	List *provInfo;
    char *newAttrName;
	ConstRelOperator *inCopy = (ConstRelOperator *) shallowCopyQueryOperator((QueryOperator *) op);
	QueryOperator *rewr;
    int relAccessCount = increaseRefCount(state->provCounts, "query");
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS - Const Rel Operator <%s> <%u>", nodeToString(op->values), relAccessCount);

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

	LOG_RESULT_AND_RETURN(ConstRel);
}

static QueryOperator *
rewritePI_CSDuplicateRemOp(DuplicateRemoval *op, PICSRewriteState *state)
{
    QueryOperator *child = OP_LCHILD(op);
	QueryOperator *rewr;
	DEBUG_LOG("REWRITE-PICS - DuplicateRemoval");

	rewr = rewritePI_CSOperator(child, state);

	LOG_RESULT_AND_RETURN(DuplicateRemoval);
}

static QueryOperator *
rewritePI_CSOrderOp(OrderOperator *op, PICSRewriteState *state)
{
	REWR_UNARY_SETUP_PI(OrderBy);

    // rewrite child
	REWR_UNARY_CHILD_PI();

    // adapt provenance attr list and schema
    addProvenanceAttrsToSchema((QueryOperator *) rewr, rewrInput);

	LOG_RESULT_AND_RETURN(OrderBy);
}

void
recursiveAppendNestedAttr(JsonColInfoItem *attr, List **provAttr, List **newDef, List **provList, int *cnt, JsonTableOperator *op)
{
	char *newAttrName = NULL; //new provenance name
	if (attr->nested)
	{
//    	StringInfo forOrdinality = makeStringInfo ();
//        char *prefixFOD = "prov_for_ord_";
//        appendStringInfoString(forOrdinality, prefixFOD);
//        appendStringInfoString(forOrdinality, itoa(*countNest));
//        attr->forOrdinality = forOrdinality->data;
//        (*countNest)++;

		FOREACH(JsonColInfoItem, attr1, attr->nested)
        {
			if(attr1->nested)
			{
				recursiveAppendNestedAttr(attr1, provAttr, newDef, provList, cnt, op);
			}
			else
			{
				*provAttr = appendToTailOfList(*provAttr, strdup(attr1->attrName));
				*newDef = appendToTailOfList(*newDef, createAttributeDef(attr1->attrName, DT_VARCHAR2));

				newAttrName = getProvenanceAttrName(op->jsonTableIdentifier, attr1->attrName, 0);
				*provList = appendToTailOfList(*provList, createAttributeDef(newAttrName, DT_VARCHAR2));
				(*cnt)++;
			}
        }
	}
	else
	{
		*provAttr = appendToTailOfList(*provAttr, strdup(attr->attrName));
		*newDef = appendToTailOfList(*newDef, createAttributeDef(attr->attrName, DT_VARCHAR2));

		newAttrName = getProvenanceAttrName(op->jsonTableIdentifier, attr->attrName, 0);
		*provList = appendToTailOfList(*provList, createAttributeDef(newAttrName, DT_VARCHAR2));
		(*cnt)++;
	}
}

void
addForOrdinality(JsonTableOperator **op, JsonColInfoItem **attr, int *countFOD, int *count, ProjectionOperator **proj)
{
	List *path = NIL;
	if(!streq((*attr)->path, "$"))
		path = (List *) parseFromStringjp(strdup((*attr)->path));
	DEBUG_LOG("path element: %s", nodeToString(path));
	//List *cpath = copyObject(path);
	if(LIST_LENGTH(path) == 2 && (*attr)->nested)
	{
		StringInfo forOrdinality = makeStringInfo ();
		char *prefixFOD = "prov_for_ord_";
		appendStringInfoString(forOrdinality, prefixFOD);
		appendStringInfoString(forOrdinality, gprom_itoa(*countFOD));
        (*attr)->forOrdinality = forOrdinality->data;

		StringInfo renameFOD = makeStringInfo ();
		char *prefixRFOD = "prov_path_";
		appendStringInfoString(renameFOD, prefixRFOD);
		appendStringInfoString(renameFOD, gprom_itoa(*countFOD));
		(*countFOD) ++;

		AttributeDef *projAttr = createAttributeDef(renameFOD->data, DT_STRING);
		AttributeDef *forOrdAttr = createAttributeDef(forOrdinality->data, DT_STRING);
		AttributeReference *forOrdRef = createAttributeReference(forOrdinality->data);
		forOrdRef->fromClauseItem = 0;
		forOrdRef->attrType = DT_STRING;
		forOrdRef->attrPosition = *count;
		(*count) ++;

		char *fPathEl = ((JsonPath *)getHeadOfListP(path))->path;
		StringInfo expr1 = makeStringInfo ();
		appendStringInfoString(expr1, fPathEl);
		appendStringInfoString(expr1, "[");
		Constant *c1 = createConstString(expr1->data);

		StringInfo nameEl = makeStringInfo ();
		appendStringInfoString(nameEl, "]");

		if(LIST_LENGTH((*attr)->nested) == 1)
		{
			char *colPath = ((JsonColInfoItem *) getHeadOfListP((*attr)->nested))->path;
			//StringInfo testPath = makeStringInfo ();
			//appendStringInfoString(testPath, colPath);
			if(!streq(colPath, "$"))
			{
				appendStringInfoString(nameEl, ".");
				//List *path1 = (List *) parseFromStringjp(strdup(op->documentcontext));
				List *path1 = (List *) parseFromStringjp(strdup(colPath));
				DEBUG_LOG("new nest path element: %s", nodeToString(path1));

				FOREACH(JsonPath, j, path1)
				{
					if(streq(j->path, "*"))
						appendStringInfoString(nameEl, "[*]");
					else
						appendStringInfoString(nameEl, j->path);
				}
			}
		}
		Constant *c2 = createConstString(nameEl->data);
		DEBUG_LOG("Const c1: %s", nodeToString(c1));
		DEBUG_LOG("Const c2: %s", nodeToString(c2));

		List *exprList1 = NIL;
		exprList1 = appendToTailOfList(exprList1, forOrdRef);
		exprList1 = appendToTailOfList(exprList1, c2);
		Operator *o1 = createOpExpr("||", exprList1);

		List *exprList2 = NIL;
		exprList2 = appendToTailOfList(exprList2, c1);
		exprList2 = appendToTailOfList(exprList2, o1);
		Operator *o2 = createOpExpr("||", exprList2);

		addProvenanceAttrsToSchemabasedOnList((QueryOperator *) (*proj), singleton(projAttr));
		(*proj)->projExprs = appendToTailOfList((*proj)->projExprs, o2);
		(*op)->op.schema->attrDefs = appendToTailOfList((*op)->op.schema->attrDefs, forOrdAttr);

	}

	if((*attr)->nested)
	{
		FOREACH(JsonColInfoItem, attr1, (*attr)->nested)
    	{
			if(attr1->nested)
			{
				addForOrdinality(op, &attr1, countFOD, count, proj);
			}
    	}
	}
}

static QueryOperator *
rewritePI_CSJsonTableOp(JsonTableOperator *op, PICSRewriteState *state)
{
	QueryOperator *child = OP_LCHILD(op);

	// rewrite child
	child = rewritePI_CSOperator(child, state);

	// adapt provenance attr list and schema
	clearAttrsFromSchema((QueryOperator *) op);
	addNormalAttrsToSchema((QueryOperator *) op, child);
	addProvenanceAttrsToSchema((QueryOperator *) op, child);

	// Add the correct JsonColumn things by duplicating them and adapt the
	// schema accordingly and set the provenance attribute list adding it to
	// schema of JsonTable

	List *provAttr = NIL;  //new json attribute name list
	List *newDef = NIL;    //new json attribute Def list
	List *provList = NIL;  //new provenance attribute Def list

	int cnt = 0;

	DEBUG_LOG("REWRITE-PICS - JsonTable <%s>", op->jsonTableIdentifier);

	FOREACH(JsonColInfoItem, attr, op->columns)
	      recursiveAppendNestedAttr(attr, &provAttr, &newDef, &provList, &cnt, op);

	//new json table schema
	op->op.schema->attrDefs = concatTwoLists(op->op.schema->attrDefs, newDef);

	//Add projection operator
	//projExprName:  the new projection operator projExpr name list
	List *normalName = getNormalAttrNames((QueryOperator *) op);
	List *provName = getOpProvenanceAttrNames((QueryOperator *) op);
	List *projExprName = concatTwoLists(normalName, provName);
	projExprName = concatTwoLists(projExprName, provAttr);
	List *projExpr = NIL;

	HashMap *namePosMap = NEW_MAP(Constant,Constant);
	HashMap *nameTypeMap = NEW_MAP(Constant, Node);

	// adapt name to position and name to Defs
	int count = 0;
	FOREACH(AttributeDef, d, op->op.schema->attrDefs)
	{
		char *a = d->attrName;
		MAP_ADD_STRING_KEY(namePosMap, a, createConstInt(count));
		MAP_ADD_STRING_KEY(nameTypeMap, a, (Node *) d);
		count ++;
	}

	int count1 = 0;
	FOREACH(char, c, projExprName)
	{
		if(MAP_HAS_STRING_KEY(namePosMap, c) && MAP_HAS_STRING_KEY(nameTypeMap, c))
		{
			count1 = INT_VALUE(getMapString(namePosMap, c));
			AttributeDef *d = (AttributeDef *)getMapString(nameTypeMap, c);
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(c, 0, count1, 0, d->dataType));
		}
	}

	ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

	//get new projection operator's schema
	addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
	addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
	addProvenanceAttrsToSchemabasedOnList((QueryOperator *) proj, provList);

	//begin to split path
	List *path = NIL;
	if(!streq(op->documentcontext, "$"))
	    path = (List *) parseFromStringjp(strdup(op->documentcontext));
	DEBUG_LOG("path element: %s", nodeToString(path));

	//introduce new nest
	List *cpath = copyObject(path);
	while (LIST_LENGTH(cpath) > 2)
	{
		//E.g. $.Addresses[*].City, pathEl is City
		//It used to generate $.City and '].City
		StringInfo pathEl = makeStringInfo ();
		char *a = ((JsonPath *)getTailOfListP(cpath))->path;
		if(streq(a,"*"))
		{
			cpath = removeFromTail(cpath);
			a = ((JsonPath *)getTailOfListP(cpath))->path;
			appendStringInfoString(pathEl, a);
			appendStringInfoString(pathEl, "[*]");
		}
		else
			appendStringInfoString(pathEl, a);

		cpath = removeFromTail(cpath);

		//Set the new documentcontext
		StringInfo newdoc = makeStringInfo ();
		appendStringInfoString(newdoc, "$");
		FOREACH(JsonPath, j, cpath)
		{
			if(streq(j->path, "*"))
				appendStringInfoString(newdoc, "[*]");
			else
			{
				appendStringInfoString(newdoc, ".");
				appendStringInfoString(newdoc, j->path);
			}
		}
		op->documentcontext = newdoc->data;
		DEBUG_LOG("new doc %s", op->documentcontext);

		//Introduce new nested JsonColInfoItem
		StringInfo nestedpath = makeStringInfo ();
		appendStringInfoString(nestedpath, "$.");
		appendStringInfoString(nestedpath, pathEl->data);

		JsonColInfoItem *c = createJsonColInfoItem (NULL, NULL, nestedpath->data, NULL, NULL, op->columns, NULL);
		op->columns = singleton(c);
	}

	//for ordinality
	List *newPath = NIL;
	if(!streq(op->documentcontext, "$"))
		newPath = (List *) parseFromStringjp(strdup(op->documentcontext));
	DEBUG_LOG("new path element: %s", nodeToString(newPath));
	List *cnewPath = copyObject(newPath);
    int countFOD = 0;

	if(LIST_LENGTH(cnewPath) == 2)
	{
		StringInfo forOrdinality = makeStringInfo ();
		char *prefixFOD = "prov_for_ord_";
		appendStringInfoString(forOrdinality, prefixFOD);
		appendStringInfoString(forOrdinality, gprom_itoa(countFOD));
        op->forOrdinality = forOrdinality->data;

		StringInfo renameFOD = makeStringInfo ();
		char *prefixRFOD = "prov_path_";
		appendStringInfoString(renameFOD, prefixRFOD);
		appendStringInfoString(renameFOD, gprom_itoa(countFOD));
		countFOD ++;

		AttributeDef *projAttr = createAttributeDef(renameFOD->data, DT_STRING);
		AttributeDef *forOrdAttr = createAttributeDef(forOrdinality->data, DT_STRING);
		AttributeReference *forOrdRef = createAttributeReference(forOrdinality->data);
		forOrdRef->fromClauseItem = 0;
		forOrdRef->attrType = DT_STRING;
		forOrdRef->attrPosition = count;
		count++;

		char *fPathEl = ((JsonPath *)getHeadOfListP(cnewPath))->path;
		StringInfo expr1 = makeStringInfo ();
		appendStringInfoString(expr1, fPathEl);
		appendStringInfoString(expr1, "[");
		Constant *c1 = createConstString(expr1->data);

		StringInfo nameEl = makeStringInfo ();
		appendStringInfoString(nameEl, "]");

		if(LIST_LENGTH(op->columns) == 1)
		{
			char *colPath = ((JsonColInfoItem *) getHeadOfListP(op->columns))->path;
			if(!streq(colPath, "$"))
			{
				appendStringInfoString(nameEl, ".");
				List *path1 = (List *) parseFromStringjp(strdup(colPath));
				DEBUG_LOG("new path element: %s", nodeToString(path1));

				FOREACH(JsonPath, j, path1)
				{
					if(streq(j->path, "*"))
						appendStringInfoString(nameEl, "[*]");
					else
						appendStringInfoString(nameEl, j->path);
				}
			}
		}
		Constant *c2 = createConstString(nameEl->data);
		DEBUG_LOG("Const c1: %s", nodeToString(c1));
		DEBUG_LOG("Const c2: %s", nodeToString(c2));

		List *exprList1 = NIL;
		exprList1 = appendToTailOfList(exprList1, forOrdRef);
		exprList1 = appendToTailOfList(exprList1, c2);
		Operator *o1 = createOpExpr("||", exprList1);

		List *exprList2 = NIL;
		exprList2 = appendToTailOfList(exprList2, c1);
		exprList2 = appendToTailOfList(exprList2, o1);
		Operator *o2 = createOpExpr("||", exprList2);

		addProvenanceAttrsToSchemabasedOnList((QueryOperator *) proj, singleton(projAttr));
		proj->projExprs = appendToTailOfList(proj->projExprs, o2);
		op->op.schema->attrDefs = appendToTailOfList(op->op.schema->attrDefs, forOrdAttr);

	}

	FOREACH(JsonColInfoItem, attr, op->columns)
	    addForOrdinality(&op, &attr,&countFOD, &count, &proj);

	switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
	addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

	return (QueryOperator *) proj;
}

static char*
getBinarySearchArryList(List *rangeList)
{
	//case -> binary search
	StringInfo binary_element = makeStringInfo();
	appendStringInfoString(binary_element,"{");

	for(int i=0; i<=LIST_LENGTH(rangeList)-1; i++)
	{
		Constant *rangePoint = (Constant *)getNthOfListP(rangeList, i);
		if(typeOf((Node *) rangePoint) == DT_INT)
			appendStringInfo(binary_element, "%d", INT_VALUE(rangePoint));
		else
			appendStringInfo(binary_element, "%s", STRING_VALUE(rangePoint));

		if(i != LIST_LENGTH(rangeList)-1)
			appendStringInfoString(binary_element,",");
	}
	appendStringInfoString(binary_element,"}");
	DEBUG_LOG("binary search array element: %s", binary_element->data);

	return binary_element->data;
}

static List*
getCondList(AttributeReference *attr, List *rangeList)
{
	List *condList = NIL;
	for(int i=0; i<LIST_LENGTH(rangeList)-1; i++)
	{
		Operator *leftOperator = createOpExpr(">=", LIST_MAKE(copyObject(attr), copyObject(getNthOfListP(rangeList, i)) ));
	 	Operator *rightOperator = createOpExpr("<", LIST_MAKE(copyObject(attr), copyObject(getNthOfListP(rangeList, i+1)) ));
	 	Node *cond = AND_EXPRS((Node *) leftOperator, (Node *) rightOperator);
	 	condList = appendToTailOfList(condList, cond);
	}

	return condList;
}

static List*
getWhenListOracle(List *condList)
{
	List *whenList = NIL;
	unsigned long long int power = 0;
	for(int i=0; i<LIST_LENGTH(condList); i++)
	{
		Node *cond = (Node *) getNthOfListP(condList, i);
		power = 1L << i;
		DEBUG_LOG("power: %llu", power);
		CaseWhen *when = createCaseWhen(cond, (Node *) createConstLong(power));
		whenList = appendToTailOfList(whenList, when);
	}

	return whenList;
}


static List*
getWhenList(List *condList)
{
	List *whenList = NIL;
	for(int i=0; i<LIST_LENGTH(condList); i++)
	{
		Node *cond = (Node *) getNthOfListP(condList, i);
		BitSet *bset = newBitSet(LIST_LENGTH(condList));
		setBit(bset, i, TRUE);
		char *cbset = bitSetToString(bset);

		CaseWhen *when = createCaseWhen(cond, (Node *) createConstString(cbset));
		whenList = appendToTailOfList(whenList, when);
	}

	return whenList;
}

static Node*
baCaseWhenBar(AttributeReference *attr, List *l, int low, int high)
{
	StringInfo s1 = makeStringInfo();
	int len = (high+8-1)/8; //ceil

	for(int i=0; i<len; i++)
	{
		appendStringInfo(s1, "-");
	}
	return bsCaseWhen(attr,l,low,high, s1->data);
}


static Node*
bsCaseWhen(AttributeReference *attr, List *l, int low, int high, char *s)
{
	if(low < high)
	{
		int mid = (low + high)/2;
		Operator *oper = createOpExpr("<", LIST_MAKE(copyObject(attr), copyObject(getNthOfListP(l, mid)) ));
		CaseWhen *when = createCaseWhen((Node *) oper, bsCaseWhen(attr,l,low,mid,s));
		return (Node *) createCaseExpr(NULL, singleton(when), bsCaseWhen(attr,l,mid+1,high,s));
	}
	else
	{
		return (Node *) createConstString(s);
	}
}

static QueryOperator *
rewriteCoarseGrainedTableAccess(TableAccessOperator *op, PICSRewriteState *state)
{
	REWR_NULLARY_SETUP_COARSE(TableAccess);
    List *provAttr = NIL;
    List *projExpr = NIL;
	List *provAttrsOnly = NIL;
    char *newAttrName;
    int cnt = 0;
    int numTable = 0;
	List *provInfo;
	char *tableName;

	mapIncrPointer(state->coarseGrainedOpRefCount, op);
	tableName = strdup(op->tableName);

	// increase twice because naming is prov_r_a1, [prov_r_a2], prov_r_a3 ...
	increaseRefCount(state->provCounts, tableName);
	increaseRefCount(state->provCounts, tableName);
	REWR_NULLARY();

    if (state->asOf)
        ((TableAccessOperator *) rewr)->asOf = copyObject(state->asOf);

	// if table access operator occurs in multiple paths, then there will be one number per path
	// since we are only rewriting once and afterwards reuse, pick first one
    if(HAS_STRING_PROP(op, PROP_NUM_TABLEACCESS_MARK))
		numTable = INT_VALUE(getNthOfListP(
								 (List *) GET_STRING_PROP(op, PROP_NUM_TABLEACCESS_MARK), 0));

    psInfo* psPara = (psInfo*) GET_STRING_PROP(op, PROP_COARSE_GRAINED_TABLEACCESS_MARK);
    HashMap *map = psPara->tablePSAttrInfos;
    DEBUG_LOG("provenance sketch type: %s", psPara->psType);

    if(!hasMapStringKey(map, op->tableName))
		return rewr;

    // Get the provenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    //three cases: fragment or range or page
    List *newProvPosList = NIL;
    FunctionCall *bsfc = NULL;
    List *psAttrList = (List *) getMapString(map, op->tableName);

    if(streq(psPara->psType, COARSE_GRAINED_RANGEB))
    {
		DEBUG_LOG("Using range B partition method");

		for(int j = 0; j < LIST_LENGTH(psAttrList); j++)
		{
			newProvPosList = appendToTailOfListInt(newProvPosList, cnt++);

			psAttrInfo *curPSAI = (psAttrInfo *) getNthOfListP(psAttrList, j);
			AttributeReference *pAttr = createAttrsRefByName((QueryOperator *)op, strdup(curPSAI->attrName));

			newAttrName = getCoarseGrainedAttrName(op->tableName, curPSAI->attrName, numTable);
			provAttr = appendToTailOfList(provAttr, newAttrName);
			provAttrsOnly = singleton(createConstString(strdup(curPSAI->attrName)));
			if(getBoolOption(OPTION_PS_BINARY_SEARCH))
			{
				//case -> binary search
				char *bsArray = getBinarySearchArryList(curPSAI->rangeList);
				DEBUG_LOG("bsArray: %s", bsArray);
				bsfc = createFunctionCall ("binary_search_array_pos", LIST_MAKE(createConstString(bsArray),copyObject(pAttr)));
//    				if(getBoolOption(OPTION_PS_SET_BITS))
//    				{
				projExpr = appendToTailOfList(projExpr, bsfc);
//    				}
//    				else
//    				{
//    					CastExpr *c = createCastExprOtherDT((Node *) createConstString("0"), "bit", LIST_LENGTH(curPSAI->rangeList)-1);
//    					FunctionCall *setBit = createFunctionCall("set_bit", LIST_MAKE(c, bsfc));
//    					projExpr = appendToTailOfList(projExpr, setBit);
				//   				}
			}
			else if(getBoolOption(OPTION_PS_BINARY_SEARCH_CASE_WHEN))
			{
				CaseExpr *caseExpr = (CaseExpr *) baCaseWhenBar(pAttr,curPSAI->rangeList, 0, LIST_LENGTH(curPSAI->rangeList)-1);
				projExpr = appendToTailOfList(projExpr, caseExpr);
			}
			else
			{
				List *condList = getCondList(pAttr, curPSAI->rangeList);
				List *whenList = NIL;

				if(getBackend() == BACKEND_ORACLE)
					whenList = getWhenListOracle(condList);
				else if(getBackend() == BACKEND_POSTGRES)
					whenList = getWhenList(condList);

				CaseExpr *caseExpr = createCaseExpr(NULL, whenList, NULL);

				projExpr = appendToTailOfList(projExpr, caseExpr);
			}
		}
    }

    DEBUG_LOG("rewrite table access, \n\nattrs <%s> and \n\nprojExprs <%s> and \n\nprovAttrs <%s>",
			  stringListToString(provAttr),
			  nodeToString(projExpr),
			  nodeToString(newProvPosList));

    // Create a new projection operator with these new attributes
    ProjectionOperator *newpo = createProjectionOp(projExpr, NULL, NIL, provAttr);
    newpo->op.provAttrs = newProvPosList;
    SET_BOOL_STRING_PROP((QueryOperator *) newpo, PROP_PROJ_PROV_ATTR_DUP);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, rewr);

	// the projection is the rewritten result
	rewr = (QueryOperator *) newpo;

	// store table name and sketch attribute name
	provInfo = singleton(createNodeKeyValue((Node *) createConstString(tableName),
										    (Node *) provAttrsOnly));
	SET_STRING_PROP(rewr, PROP_PROVENANCE_TABLE_ATTRS, provInfo);

	LOG_RESULT_AND_RETURN(TableAccess);
}



static QueryOperator *
rewriteCoarseGrainedAggregation(AggregationOperator *op, PICSRewriteState *state)
{
	REWR_UNARY_SETUP_COARSE(Aggregation);
	AggregationOperator *a;
	mapIncrPointer(state->coarseGrainedOpRefCount, op);

    //add semiring options
    addSCOptionToChild((QueryOperator *) op,OP_LCHILD(op));

    // rewrite child first
	REWR_UNARY_CHILD_PI();
	a = (AggregationOperator *) rewr;

    //prepare add new aggattr
    List *agg = a->aggrs;
    List *provList = getOpProvenanceAttrNames(OP_LCHILD(rewr));

    ///addProvenanceAttrsToSchema
    List *aggDefs = aggOpGetAggAttrDefs(a);
    int aggDefsLen = LIST_LENGTH(aggDefs);

    List *groupbyDefs = NIL;
    if(a->groupBy != NIL)
		groupbyDefs = aggOpGetGroupByAttrDefs(a);

    List *newProvAttrDefs = (List *) copyObject(getProvenanceAttrDefs(OP_LCHILD(rewr)));
    int newProvAttrDefsLen = LIST_LENGTH(newProvAttrDefs);

    List *newAttrDefs = CONCAT_LISTS(aggDefs, newProvAttrDefs, groupbyDefs);
    rewr->schema->attrDefs = newAttrDefs;

    List *newProvAttrs = NIL;
    int provAttrDefsPos = aggDefsLen;
    for(int i=0; i< newProvAttrDefsLen; i++)
    {
    	newProvAttrs = appendToTailOfListInt(newProvAttrs,provAttrDefsPos);
    	provAttrDefsPos ++;
    }

    // finish add new aggattr (for the cnt-th reference to operator)
    HashMap *map = (HashMap *) getNthOfListP((List *) GET_STRING_PROP(rewr, PROP_LEVEL_AGGREGATION_MARK), 0);

    FOREACH(char, c, provList)
    {
		List *levelandNumFrags =  (List *) getMapString(map, c);
		int level =  INT_VALUE((Constant *) getNthOfListP(levelandNumFrags, 0));
		int numFrags =  INT_VALUE((Constant *) getNthOfListP(levelandNumFrags, 1));
        AttributeReference *a = createAttrsRefByName(OP_LCHILD(rewr), c);
        FunctionCall *f = NULL;

        if(getBackend() == BACKEND_ORACLE)
		{
			f = createFunctionCall(ORACLE_SKETCH_AGG_FUN, singleton(a));
		}
        else if(getBackend() == BACKEND_POSTGRES)
        {
			//if(getBoolOption(OPTION_PS_SET_BITS))
			if(level == 1)
			{
				f = createFunctionCall(POSTGRES_SET_BITS_FUN, singleton(a));
				CastExpr *c = createCastExprOtherDT((Node *) f, POSTGRES_BIT_DT, numFrags, DT_STRING);
				agg = appendToTailOfList(agg, c);
			}
			else
			{
				f = createFunctionCall(POSTGRES_FAST_BITOR_FUN, singleton(a));
				agg = appendToTailOfList(agg, f);
			}
        }
    }
    //finish adapt schema (adapt provattrs)
    rewr->provAttrs = newProvAttrs;

    //proj on top
    List *provAttrDefs = getProvenanceAttrDefs(rewr);
    List *norAttrDefs = getNormalAttrs(rewr);

    List *projExprs = NIL;
    List *projNames = NIL;
    List *projProvAttrs = NIL;
    int count = 0;
    int pos = LIST_LENGTH(provAttrDefs);
    FOREACH(AttributeDef, ad, norAttrDefs)
    {
		projNames = appendToTailOfList(projNames, strdup(ad->attrName));
		AttributeReference *ar = NULL;
		if(count >= LIST_LENGTH(a->aggrs) - LIST_LENGTH(provAttrDefs))
			ar = createFullAttrReference (strdup(ad->attrName), 0, pos, 0, ad->dataType);
		else
			ar = createFullAttrReference (strdup(ad->attrName), 0, count, 0, ad->dataType);
		projExprs = appendToTailOfList(projExprs, ar);
		count++;
		pos++;
    }

    pos = LIST_LENGTH(a->aggrs) - LIST_LENGTH(provAttrDefs);
    FOREACH(AttributeDef, ad, provAttrDefs)
    {
		projNames = appendToTailOfList(projNames, strdup(ad->attrName));
		AttributeReference *ar = createFullAttrReference (strdup(ad->attrName), 0, pos,
														  0, ad->dataType);
		projExprs = appendToTailOfList(projExprs, ar);
		projProvAttrs = appendToTailOfListInt(projProvAttrs, count);
		count++;
		pos++;
    }

    ProjectionOperator *projOp = createProjectionOp(projExprs,
            rewr, rewr->parents, projNames);

    rewr->parents = singleton(projOp);

    ((QueryOperator *) projOp)->provAttrs = projProvAttrs;

	// return projection operator
	rewr = (QueryOperator *) projOp;

	// copy prov info
 	COPY_PROV_INFO(rewr, rewrInput);

	LOG_RESULT_AND_RETURN(Aggregation-CoarseGrained);
}


static QueryOperator *
rewriteUseCoarseGrainedAggregation (AggregationOperator *op, PICSRewriteState *state)
{
	ASSERT(OP_LCHILD(op));

	DEBUG_LOG("REWRITE - Use Coarse grained - Aggregation");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	//add semiring options
	addSCOptionToChild((QueryOperator *) op,OP_LCHILD(op));

	// rewrite child first
	rewritePI_CSOperator(((QueryOperator *)getHeadOfListP(
							  ((QueryOperator *)op)->inputs)),
						 state);

	///addProvenanceAttrsToSchema
	List *aggDefs = aggOpGetAggAttrDefs(op);
	int aggDefsLen = LIST_LENGTH(aggDefs);

	List *newProvAttrDefs = (List *) copyObject(getProvenanceAttrDefs(OP_LCHILD(op)));
	int newProvAttrDefsLen = LIST_LENGTH(newProvAttrDefs);

	List *newProvAttrs = NIL;
	int provAttrDefsPos = aggDefsLen;
	for(int i=0; i< newProvAttrDefsLen; i++)
	{
		newProvAttrs = appendToTailOfListInt(newProvAttrs,provAttrDefsPos);
		provAttrDefsPos ++;
	}

	//proj on top
	List *provAttrDefs = getProvenanceAttrDefs((QueryOperator *) op);
	List *norAttrDefs = getNormalAttrs((QueryOperator *) op);

	List *projExprs = NIL;
	List *projNames = NIL;
	List *projProvAttrs = NIL;
	int count = 0;
	int pos = LIST_LENGTH(provAttrDefs);
	FOREACH(AttributeDef, ad, norAttrDefs)
	{
		projNames = appendToTailOfList(projNames, strdup(ad->attrName));
		AttributeReference *ar = NULL;
		if(count >= LIST_LENGTH(op->aggrs) - LIST_LENGTH(provAttrDefs))
			ar = createFullAttrReference (strdup(ad->attrName), 0, pos, 0, ad->dataType);
		else
			ar = createFullAttrReference (strdup(ad->attrName), 0, count, 0, ad->dataType);
		projExprs = appendToTailOfList(projExprs, ar);
		count ++;
		pos ++;
	}

	pos = LIST_LENGTH(op->aggrs) - LIST_LENGTH(provAttrDefs);
	FOREACH(AttributeDef, ad, provAttrDefs)
	{
		projNames = appendToTailOfList(projNames, strdup(ad->attrName));
		AttributeReference *ar = createFullAttrReference (strdup(ad->attrName), 0, pos,
														  0, ad->dataType);
		projExprs = appendToTailOfList(projExprs, ar);
		projProvAttrs = appendToTailOfListInt(projProvAttrs, count);
		count ++;
		pos ++;
	}

	ProjectionOperator *projOp = createProjectionOp(projExprs,
													(QueryOperator *) op, ((QueryOperator *) op)->parents, projNames);

	FOREACH(QueryOperator, o, ((QueryOperator *) op)->parents)
		o->inputs = singleton(projOp);

	((QueryOperator *) op)->parents = singleton(projOp);

	((QueryOperator *) projOp)->provAttrs = projProvAttrs;


	LOG_RESULT("Rewritten Operator tree", projOp);

	return (QueryOperator *) projOp;
}



static QueryOperator *
rewriteUseCoarseGrainedTableAccess(TableAccessOperator *op, PICSRewriteState *state)
{
	REWR_NULLARY_SETUP_COARSE(TableAcces);
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;
    int cnt = 0;
	List *attrCnts = NIL;

	REWR_NULLARY();

    // copy any as of clause if there
    if (state->asOf)
	{
        op->asOf = copyObject(state->asOf);
	}

    // Get the povenance name for each attribute
    FOREACH(AttributeDef, attr, op->op.schema->attrDefs)
    {
        provAttr = appendToTailOfList(provAttr, strdup(attr->attrName));
        projExpr = appendToTailOfList(projExpr, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
        cnt++;
    }

    int numTable = 0;
	//TODO merge sketches if there is more than one (if this operator or one of its ancestors have more than one parent)
    if(HAS_STRING_PROP(op, PROP_NUM_TABLEACCESS_MARK))
	{
		// get all provenance attributes (if this table access is in a CTE we have to merge bitsets)
		attrCnts = (List *) GET_STRING_PROP(op, PROP_NUM_TABLEACCESS_MARK);
		// choose one as the provenance attribute name
		numTable = INT_VALUE(getHeadOfListP(attrCnts));
	}

    psInfo *psPara = (psInfo *) GET_STRING_PROP(op, USE_PROP_COARSE_GRAINED_TABLEACCESS_MARK);
    HashMap *map = psPara->tablePSAttrInfos;
    List *psAttrList = (List *) getMapString(map, op->tableName);

    SelectionOperator *sel = NULL;
	if(streq(psPara->psType, COARSE_GRAINED_RANGEB))
	{
		DEBUG_LOG("Deal with range B partition method");

		Node *newCond = NULL;
		for(int j=0; j<LIST_LENGTH(psAttrList); j++)
		{
			psAttrInfo *curPSAI = (psAttrInfo *) getNthOfListP(psAttrList, j);
			//Bitset *bitvec = NULL;

			newAttrName = getCoarseGrainedAttrName(op->tableName, curPSAI->attrName, numTable);
			provAttr = appendToTailOfList(provAttr, newAttrName);

			AttributeReference *pAttr = createAttrsRefByName((QueryOperator *) rewr, curPSAI->attrName);

			// Oracle backend
			if(getBackend() == BACKEND_ORACLE)
			{
				if(HAS_STRING_PROP(op, AUTO_USE_PROV_COARSE_GRAINED_TABLEACCESS_MARK)) //or curPSAI->BitVector == NULL
				{
					DEBUG_LOG("Auto use - BACKEND_ORACLE");
					HashMap *psMap  = (HashMap *) GET_STRING_PROP(op, AUTO_USE_PROV_COARSE_GRAINED_TABLEACCESS_MARK);
					//FIXME need to loop over attrCnts and bitor the longs
					if(hasMapStringKey(psMap, newAttrName))
					{
						int psValue = INT_VALUE((Constant *) getMapString(psMap, newAttrName));
						unsigned long long int k;
						unsigned long long int n = psValue; //FIXME this is still limited to an int, why not use at least a long constant?
						DEBUG_LOG("psValue is %llu", psValue);
						int numPoints = LIST_LENGTH(curPSAI->rangeList);

						for (int c = numPoints - 1,cntOnePos=0; c >= 0; c--,cntOnePos++)
						{
							k = n >> c;
							DEBUG_LOG("n is %llu, c is %d, k is: %llu, cntOnePos is: %d", n, c, k, cntOnePos);
							if (k & 1)
							{
								curPSAI->psIndexList = appendToTailOfList(curPSAI->psIndexList, createConstInt(numPoints - 1  - cntOnePos));
								DEBUG_LOG("cnt is: %d", numPoints - cntOnePos);
							}
						}
					}
				}

			    List *condRightValueList = curPSAI->psIndexList;
			    List *elList = NIL;
			    Constant *ll = (Constant *) popHeadOfListP(condRightValueList);
			    int hh = INT_VALUE(ll) + 1;
			    FOREACH(Constant, c, condRightValueList)
			    {
					if(INT_VALUE(c) == INT_VALUE(ll) - 1)
						ll = c;
					else
					{
						Operator *lOpr =  createOpExpr(">=", LIST_MAKE(copyObject(pAttr), copyObject(getNthOfListP(curPSAI->rangeList, INT_VALUE(ll)))));
						Operator *rOpr =  createOpExpr("<", LIST_MAKE(copyObject(pAttr), copyObject(getNthOfListP(curPSAI->rangeList, hh))));
						Node *elOp = andExprList(LIST_MAKE(lOpr, rOpr));
						elList = appendToTailOfList(elList, elOp);
						ll = c;
						hh = INT_VALUE(c) + 1;
					}
			    }

			    Operator *lOpr =  createOpExpr(">=", LIST_MAKE(copyObject(pAttr), copyObject(getNthOfListP(curPSAI->rangeList, INT_VALUE(ll)))));
			    Operator *rOpr =  createOpExpr("<", LIST_MAKE(copyObject(pAttr), copyObject(getNthOfListP(curPSAI->rangeList, hh))));
			    Node *elOp = andExprList(LIST_MAKE(lOpr, rOpr));
			    elList = appendToTailOfList(elList, elOp);
			    newCond = orExprList(elList);
			    DEBUG_NODE_BEATIFY_LOG("newCond ", newCond);
			}
			// Postgres backend
			else if(getBackend() == BACKEND_POSTGRES)
			{
				if(HAS_STRING_PROP(op, AUTO_USE_PROV_COARSE_GRAINED_TABLEACCESS_MARK)) //or curPSAI->BitVector == NULL
				{
					HashMap *psMap  = (HashMap *) GET_STRING_PROP(op, AUTO_USE_PROV_COARSE_GRAINED_TABLEACCESS_MARK);

					curPSAI->BitVector = NULL;

					FOREACH(Constant,i,attrCnts)
					{
						int cnt = INT_VALUE(i);
						char *newAttr = getCoarseGrainedAttrName(op->tableName, curPSAI->attrName, cnt);
						if(hasMapStringKey(psMap, newAttr))
						{
							char *BitVectorStr = STRING_VALUE((Constant *) getMapString(psMap, newAttr));
							if(curPSAI->BitVector)
							{
								curPSAI->BitVector = bitOr(curPSAI->BitVector, stringToBitset(BitVectorStr));
							}
							else
							{
								curPSAI->BitVector = stringToBitset(BitVectorStr);
							}
						}
					}
				}

				// generate and combine each condition, e.g.,  33, 32, 31, 28, 27, 25 ->  31<=x<34 or 27<=x<29 or 25<=x<26
				List *operatorList = NIL;
				int ll=0, hh=1;
				StringInfo brins = makeStringInfo();
				appendStringInfoString(brins,"{");

				int psSize = 0;
				for(int i=0; i<curPSAI->BitVector->length; i++)
				{
					if(isBitSet(curPSAI->BitVector, i))
					{
						psSize ++;
						if(isBitSet(curPSAI->BitVector, i+1))
						{
							hh++;
							continue;
						}

						if(getBoolOption(OPTION_PS_USE_BRIN_OP))
						{
							Constant *cur_brinl = (Constant *)getNthOfListP(curPSAI->rangeList, ll);
							Constant *cur_brinh = (Constant *)getNthOfListP(curPSAI->rangeList, hh);
							if(cur_brinl->constType == DT_INT)
							{
								appendStringInfo(brins, "%d,", INT_VALUE(cur_brinl));
								appendStringInfo(brins, "%d,", INT_VALUE(cur_brinh));
							}
							else if(cur_brinl->constType == DT_STRING)
							{
								appendStringInfo(brins, "%s,", STRING_VALUE(cur_brinl));
								appendStringInfo(brins, "%s,", STRING_VALUE(cur_brinh));
							}
						}
						else
						{
							Operator *lOpr =  createOpExpr(">=", LIST_MAKE(copyObject(pAttr), copyObject(getNthOfListP(curPSAI->rangeList, ll))));
							Operator *rOpr =  createOpExpr("<", LIST_MAKE(copyObject(pAttr), copyObject(getNthOfListP(curPSAI->rangeList, hh))));
							Node *elOp = andExprList(LIST_MAKE(lOpr, rOpr));
							operatorList = appendToTailOfList(operatorList, elOp);
						}
					}
					ll = i+1;
					hh = i+2;
				}

				WARN_LOG("psSize %s: %d", newAttrName, psSize);

				Node *curCond = NULL;
				if(getBoolOption(OPTION_PS_USE_BRIN_OP))
				{
					if(!streq(brins->data,"{")) //used for the case if the query result is empty
					{
						removeTailingStringInfo(brins,1);
						appendStringInfoString(brins,"}");
						Constant *bsArray = createConstString(brins->data);
						curCond = (Node *) createOpExpr("<@", LIST_MAKE(copyObject(pAttr), bsArray));
						DEBUG_LOG("brins cond: %s", brins->data);
					}
				}
				else
					curCond = orExprList(operatorList);

				if(newCond == NULL)
					newCond = curCond;
				else
					newCond = AND_EXPRS(newCond, curCond);
			}
		}
		if(newCond != NULL)
		{
			sel = createSelectionOp ((Node *) newCond,rewr, NIL, getQueryOperatorAttrNames((QueryOperator *) op));
		}
	}

	//TODO deal with other backends
	if (sel == NULL)
		return rewr;

	// sel is the result
	rewr->parents = singleton(sel);
	rewr = (QueryOperator *) sel;

    LOG_RESULT_AND_RETURN(TableAccess);
}
