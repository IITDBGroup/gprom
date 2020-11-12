/*-----------------------------------------------------------------------------
 *
 * pi_cs_composable.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
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
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/list/list.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_composable.h"
#include "provenance_rewriter/prov_schema.h"
#include "provenance_rewriter/prov_utility.h"
#include "operator_optimizer/cost_based_optimizer.h"

// result tuple-id attribute and provenance duplicate counter attribute
#define RESULT_TID_ATTR "_result_tid"
#define PROV_DUPL_COUNT_ATTR "_prov_dup_count"

#define LOG_RESULT(mes,op) \
    do { \
    	INFO_OP_LOG(mes,op); \
    	DEBUG_NODE_BEATIFY_LOG(mes,op); \
    } while(0)

// data structures
static Node *asOf;
static RelCount *nameState;

// static methods
static boolean isTupleAtATimeSubtree(QueryOperator *op);

static QueryOperator *rewritePI_CSComposableOperator (QueryOperator *op);
static QueryOperator *rewritePI_CSComposableUseProvNoRewrite (QueryOperator *op, List *userProvAttrs);
static QueryOperator *rewritePI_CSComposableAddProvNoRewrite (QueryOperator *op, List *userProvAttrs);
static QueryOperator *composableAddUserProvenanceAttributes (QueryOperator *op, List *userProvAttrs, boolean showIntermediate);
static QueryOperator *composableAddIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs);


static QueryOperator *rewritePI_CSComposableSelection (SelectionOperator *op);
static QueryOperator *rewritePI_CSComposableProjection (ProjectionOperator *op);
static QueryOperator *rewritePI_CSComposableJoin (JoinOperator *op);
static QueryOperator *rewritePI_CSComposableAggregationWithWindow (AggregationOperator *op);
static QueryOperator *rewritePI_CSComposableAggregationWithJoin (AggregationOperator *op);
static QueryOperator *rewritePI_CSComposableSet (SetOperator *op);
static QueryOperator *rewritePI_CSComposableTableAccess(TableAccessOperator *op);
static QueryOperator *rewritePI_CSComposableConstRel(ConstRelOperator *op);
static QueryOperator *rewritePI_CSComposableDuplicateRemOp(DuplicateRemoval *op);
static QueryOperator *rewritePI_CSComposableOrderOp(OrderOperator *op);

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

    rewRoot = OP_LCHILD(op);
    rewRoot = rewritePI_CSComposableOperator(rewRoot);

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

static QueryOperator *
rewritePI_CSComposableOperator (QueryOperator *op)
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
        addProvAttrs = (List *)  GET_STRING_PROP(op, PROP_ADD_PROVENANCE);

    DEBUG_LOG("REWRITE OPERATIONS:\n\tshow intermediates: %s\n\tuse prov: %s"
            "\n\thas prov: %s\n\tadd prov: %s"
            "\n\tuser prov attrs: %s"
            "\n\tadd prov attrs: %s"
            "\n\tignore prov attrs: %s",
            showIntermediate ? "T": "F",
            noRewriteUseProv ? "T": "F",
            noRewriteHasProv ? "T": "F",
            rewriteAddProv ? "T": "F",
            nodeToString(userProvAttrs),
            nodeToString(addProvAttrs),
            nodeToString(ignoreProvAttrs));

    if (noRewriteUseProv)
        return rewritePI_CSComposableAddProvNoRewrite(op, userProvAttrs);
    if (noRewriteHasProv)
        return rewritePI_CSComposableUseProvNoRewrite(op, userProvAttrs);


    switch(op->type)
    {
        case T_SelectionOperator:
            rewrittenOp = rewritePI_CSComposableSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            rewrittenOp = rewritePI_CSComposableProjection((ProjectionOperator *) op);
            break;
        case T_JoinOperator:
            rewrittenOp = rewritePI_CSComposableJoin((JoinOperator *) op);
            break;
        case T_AggregationOperator:
        {
        	if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
        	{
            	QueryOperator *op1;
            	int res;

            	res = callback(2);

        		if (res == 1)
        			op1 = rewritePI_CSComposableAggregationWithWindow((AggregationOperator *) op);
        		else
       				op1 = rewritePI_CSComposableAggregationWithJoin((AggregationOperator *) op);

        		rewrittenOp = op1;
       	    }
        	else
        	{
        		if(getBoolOption(OPTION_PI_CS_COMPOSABLE_REWRITE_AGG_WINDOW))
					rewrittenOp = rewritePI_CSComposableAggregationWithWindow((AggregationOperator *) op);
				else
					rewrittenOp = rewritePI_CSComposableAggregationWithJoin((AggregationOperator *) op);
        	}
            break;
        }
        case T_Set:
            rewrittenOp = rewritePI_CSComposableSet((SetOperator *) op);
            break;
        case T_TableAccessOperator:
            rewrittenOp = rewritePI_CSComposableTableAccess((TableAccessOperator *) op);
            break;
        case T_ConstRelOperator:
            rewrittenOp = rewritePI_CSComposableConstRel((ConstRelOperator *) op);
            break;
        case T_DuplicateRemoval:
            rewrittenOp = rewritePI_CSComposableDuplicateRemOp((DuplicateRemoval *) op);
            break;
        case T_OrderOperator:
            rewrittenOp = rewritePI_CSComposableOrderOp((OrderOperator *) op);
            break;
        default:
            FATAL_LOG("rewrite for %u not implemented", op->type);
            rewrittenOp = NULL;
            break;
    }

    if (showIntermediate)
        rewrittenOp = composableAddIntermediateProvenance(rewrittenOp, userProvAttrs, ignoreProvAttrs);

    if (rewriteAddProv)
        rewrittenOp = composableAddUserProvenanceAttributes(rewrittenOp, addProvAttrs, showIntermediate);

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel(rewrittenOp));

    return rewrittenOp;
}

static QueryOperator *
composableAddUserProvenanceAttributes (QueryOperator *op, List *userProvAttrs, boolean showIntermediate)
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
        relAccessCount = getCurRelNameCount(&nameState, tableName) - 1;
    else
        relAccessCount = getRelNameCount(&nameState, tableName);

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
         projExpr = appendToTailOfList(projExpr, createConstInt(1));
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
composableAddIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs)
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

    relAccessCount = getRelNameCount(&nameState, tableName);

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
rewritePI_CSComposableAddProvNoRewrite (QueryOperator *op, List *userProvAttrs)
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

    relAccessCount = getRelNameCount(&nameState, tableName);

    DEBUG_LOG("REWRITE-PICS - Composable - Add Provenance Attrs <%s> <%u>",
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

    // result tuple ID attribute
    newAttrName = strdup(RESULT_TID_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));

    // provenance duplicate attribute
    newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
    provAttr = appendToTailOfList(provAttr, newAttrName);
    projExpr = appendToTailOfList(projExpr, createConstInt(1));

    List *newProvPosList = NIL;
    CREATE_INT_SEQ(newProvPosList, numNormalAttrs, numNormalAttrs + numProvAttrs + 1, 1);

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
rewritePI_CSComposableUseProvNoRewrite (QueryOperator *op, List *userProvAttrs)
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

    DEBUG_LOG("Use existing provenance attributes %s for %s",
            beatify(nodeToString(userProvAttrs)), tableName);

    relAccessCount = getRelNameCount(&nameState, tableName);

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
                        createAttributeDef(newAttrName, DT_INT));
        theProj->projExprs = appendToTailOfList(theProj->projExprs, makeNode(RowNumExpr));

        // provenance duplicate attribute
        newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
        proj->schema->attrDefs = appendToTailOfList(proj->schema->attrDefs,
                createAttributeDef(newAttrName, DT_INT));
        theProj->projExprs = appendToTailOfList(theProj->projExprs, createConstInt(1));

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
                        createAttributeDef(newAttrName, DT_INT));
        proj->projExprs = appendToTailOfList(proj->projExprs, makeNode(RowNumExpr));

        // provenance duplicate attribute
        newAttrName = strdup(PROV_DUPL_COUNT_ATTR);
        projOp->schema->attrDefs = appendToTailOfList(projOp->schema->attrDefs,
                createAttributeDef(newAttrName, DT_INT));
        proj->projExprs = appendToTailOfList(proj->projExprs, createConstInt(1));

        // prov attributes and store TID and DUP attributes as property
        SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(curPos));
        SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(curPos + 1));

        return projOp;
    }
}

static QueryOperator *
rewritePI_CSComposableSelection (SelectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS-Composable - Selection");
    DEBUG_NODE_BEATIFY_LOG("Operator tree", op);

    // rewrite child first
    rewritePI_CSComposableOperator(OP_LCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    // add result TID and prov duplicate attributes
    addResultTIDAndProvDupAttrs((QueryOperator *) op, TRUE);

    if (isTupleAtATimeSubtree(OP_LCHILD(op)))
        SET_BOOL_STRING_PROP(op,PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);

    LOG_RESULT("Selection - Rewritten Operator tree", op);
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSComposableProjection (ProjectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS-Composable - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child
    rewritePI_CSComposableOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(op);
    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
        op->projExprs = appendToTailOfList(op->projExprs,
                createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
    }

    // add projection expressions for result TID and prov dup attrs
    op->projExprs = appendToTailOfList(op->projExprs,
            createFullAttrReference(RESULT_TID_ATTR, 0,
                    INT_VALUE(GET_STRING_PROP(child,PROP_RESULT_TID_ATTR)), 0, DT_INT));

    op->projExprs = appendToTailOfList(op->projExprs,
            createFullAttrReference(PROV_DUPL_COUNT_ATTR, 0,
                    INT_VALUE(GET_STRING_PROP(child,PROP_PROV_DUP_ATTR)), 0, DT_INT));
    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));
    addResultTIDAndProvDupAttrs((QueryOperator *) op, TRUE);

    LOG_RESULT("Projection - Rewritten Operator tree", op);
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSComposableJoin (JoinOperator *op)
{
    DEBUG_LOG("REWRITE-PICS-Composable - Join");
    WindowOperator *wOp = NULL;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
    QueryOperator *prev = NULL;
    QueryOperator *o = (QueryOperator *) op;
    boolean noDupInput = isTupleAtATimeSubtree((QueryOperator *) op);
    boolean lChildNoDup = isTupleAtATimeSubtree(lChild);
    boolean rChildNoDup = isTupleAtATimeSubtree(rChild);
    List *rNormAttrs;
    int numLAttrs, numRAttrs;

    numLAttrs = LIST_LENGTH(lChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rChild->schema->attrDefs);

    // rewrite children
    lChild = rewritePI_CSComposableOperator(lChild);
    rChild = rewritePI_CSComposableOperator(rChild);

    // get attributes from right input
    rNormAttrs = sublist(o->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    o->schema->attrDefs = sublist(copyObject(o->schema->attrDefs), 0, numLAttrs - 1);

    // adapt schema for join op
    addProvenanceAttrsToSchema((QueryOperator *) op, lChild);
    addChildResultTIDAndProvDupAttrsToSchema((QueryOperator *) op);

    o->schema->attrDefs = CONCAT_LISTS(o->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchema((QueryOperator *) op, rChild);
    addChildResultTIDAndProvDupAttrsToSchema((QueryOperator *) op);

    // add window functions for result TID and prov dup columns
    if (!lChildNoDup || !rChildNoDup)
    {
        List *orderBy = NIL;
        List *partitionBy = NIL;
        wOp = NULL;

        if (lChildNoDup)
        {
            AttributeReference *childResultTidAttr = (AttributeReference *)
                    getHeadOfListP(getResultTidAndProvDupAttrsProjExprs(lChild));
            orderBy = appendToTailOfList(orderBy, copyObject(childResultTidAttr));
            partitionBy = appendToTailOfList(partitionBy, copyObject(childResultTidAttr));
        }
        if (rChildNoDup)
        {
            AttributeReference *childResultTidAttr = (AttributeReference *)
                    getHeadOfListP(getResultTidAndProvDupAttrsProjExprs(rChild));
            childResultTidAttr->attrPosition += getNumAttrs(lChild);
            orderBy = appendToTailOfList(orderBy, copyObject(childResultTidAttr));
            partitionBy = appendToTailOfList(partitionBy, copyObject(childResultTidAttr));
        }

        // add window functions for result TID attr
        Node *tidFunc = (Node *) createFunctionCall(strdup("DENSE_RANK"), NIL);

        wOp = createWindowOp(tidFunc,
                NIL,
                orderBy,
                NULL,
                strdup(RESULT_TID_ATTR),
                (QueryOperator *) op,
                NIL
        );
        wOp->op.provAttrs = copyObject(op->op.provAttrs);

        // add window function for prov dup attr
        prev = (QueryOperator *) wOp;
        Node *provDupFunc = (Node *) createFunctionCall(strdup("ROW_NUMBER"), NIL);

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
            (QueryOperator *) op :
            (QueryOperator *) wOp;

    // get special attributes from window op or create projection expression for them
    if (!noDupInput)
        resultTidAndProvCount = getResultTidAndProvDupAttrsProjExprs((QueryOperator *) wOp);
    else
    {
        resultTidAndProvCount = LIST_MAKE(
                makeNode(RowNumExpr),
                createConstInt(1)
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
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    if (noDupInput)
        addParent((QueryOperator *) op, (QueryOperator *) proj);
    else
    {
        addParent((QueryOperator *) wOp, (QueryOperator *) proj);
        addParent((QueryOperator *) op, (QueryOperator *) prev);
    }

    LOG_RESULT("Join - Rewritten Operator tree", proj);
    return (QueryOperator *) proj;
}

static QueryOperator *
rewritePI_CSComposableAggregationWithJoin (AggregationOperator *op)
{
    JoinOperator *joinProv;
    boolean groupBy = (op->groupBy != NIL);
    ProjectionOperator *proj;
    QueryOperator *aggInput;
    QueryOperator *origAgg;
    QueryOperator *pInput;
    int numGroupAttrs = LIST_LENGTH(op->groupBy);
//    boolean noDupInput;
    List *partitionBy = NIL;
    List *orderBy = NIL;

    DEBUG_LOG("REWRITE-PICS-Composable - Aggregation - Join");

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

    // copy aggregation input
    origAgg = (QueryOperator *) op;
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));
    // rewrite aggregation input copy
    aggInput = rewritePI_CSComposableOperator(aggInput);
//    noDupInput =
    isTupleAtATimeSubtree(aggInput);

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

        attrNames = CONCAT_LISTS(gbNames, getOpProvenanceAttrNames(aggInput));
        groupByProjExprs = CONCAT_LISTS(groupByProjExprs, getProvAttrProjectionExprs(aggInput));

        groupByProj = createProjectionOp(groupByProjExprs,
                        aggInput, NIL, attrNames);
        CREATE_INT_SEQ(provAttrs, numGroupAttrs, numGroupAttrs + getNumProvAttrs(aggInput) - 1,1);
        groupByProj->op.provAttrs = provAttrs;
        aggInput->parents = singleton(groupByProj);
        aggInput = (QueryOperator *) groupByProj;

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
        joinCond = (Node *) createOpExpr("=", LIST_MAKE(createConstInt(1), createConstInt(1)));

    // create join operator
    List *joinAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(origAgg), getQueryOperatorAttrNames(aggInput));
    joinProv = createJoinOp(joinT, joinCond, LIST_MAKE(origAgg, aggInput), NIL,
            joinAttrNames);
    joinProv->op.provAttrs = copyObject(aggInput->provAttrs);
    FOREACH_LC(lc,joinProv->op.provAttrs)
        lc->data.int_value += getNumAttrs(origAgg);
    pInput = (QueryOperator *) joinProv;

    // add result TID attr and prov dup attr, if group by then use window function, otherwise use projection
    //TODO use simpler approach where TID is created as projection on ROWNUM of original aggregation
    if (groupBy)
    {
        WindowOperator *curWindow;
        // add window functions for result TID attr
        Node *tidFunc = (Node *) createFunctionCall(strdup("DENSE_RANK"), NIL);

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
        Node *provDupFunc = (Node *) createFunctionCall(strdup("ROW_NUMBER"), NIL);

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
            getOpProvenanceAttrNames(aggInput),
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
                DT_INT
                ));
        projExprs = appendToTailOfList(projExprs, createFullAttrReference(
                PROV_DUPL_COUNT_ATTR,
                0,
                getNumAttrs(pInput) - 1,
                INVALID_ATTR,
                DT_INT
                ));
    }
    // else add 1 as RESULT_TID and ROWNUM AS PROV DUP
    else
    {
        projExprs = appendToTailOfList(projExprs, createConstInt(1));
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
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addParent(origAgg, (QueryOperator *) joinProv);
    addParent(aggInput, (QueryOperator *) joinProv);

    // adapt schema for final projection
    DEBUG_NODE_BEATIFY_LOG("Rewritten Operator tree \n%s", proj);
    return (QueryOperator *) proj;
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
rewritePI_CSComposableAggregationWithWindow (AggregationOperator *op)
{
    boolean groupBy = (op->groupBy != NIL);
    WindowOperator *curWindow = NULL;
    QueryOperator *firstChild;
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

    DEBUG_LOG("REWRITE-PICS-Composable - Aggregation - Window");
    DEBUG_NODE_BEATIFY_LOG("Operator tree", op);

    // rewrite child
    curChild = rewritePI_CSComposableOperator(OP_LCHILD(op));
    firstChild = curChild;
    removeParentFromOps(singleton(firstChild), (QueryOperator *) op);
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
                DT_INT);
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
        Node *tidFunc = (Node *) createFunctionCall(strdup("DENSE_RANK"), NIL);

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
        Node *provDupFunc = (Node *) createFunctionCall(strdup("ROW_NUMBER"), NIL);

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
                LIST_MAKE(createConstInt(1),
                        makeNode(RowNumExpr)));

        finalAttrs = CONCAT_LISTS(aggAttrNames,
                            provAttrNames,
                            LIST_MAKE(strdup(RESULT_TID_ATTR),strdup(PROV_DUPL_COUNT_ATTR)));
    }
    // else move result TID and prov dup attribute to end of list
    else
    {
//        List *tidAndDupAttrs = sublist(copyList(normalAttrs),
//                LIST_LENGTH(normalAttrs) - 3,
//                LIST_LENGTH(normalAttrs));
        normalAttrs = sublist(normalAttrs,
                LIST_LENGTH(normalAttrs) - LIST_LENGTH(op->aggrs) - 2,
                LIST_LENGTH(normalAttrs) - 3);

        projExprs = CONCAT_LISTS(normalAttrs, groupByExprs, provAttrs,
                LIST_MAKE(createFullAttrReference(
                            strdup(RESULT_TID_ATTR),
                            0,
                            getNumAttrs((QueryOperator *) curWindow) - 2,
                            INVALID_ATTR,
                            DT_INT),
                        createFullAttrReference(
                            strdup(PROV_DUPL_COUNT_ATTR),
                            0,
                            getNumAttrs((QueryOperator *) curWindow) - 1,
                            INVALID_ATTR,
                            DT_INT)));

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

    SET_STRING_PROP(proj, PROP_RESULT_TID_ATTR, createConstInt(LIST_LENGTH(finalAttrs) - 2));
    SET_STRING_PROP(proj, PROP_PROV_DUP_ATTR, createConstInt(LIST_LENGTH(finalAttrs) - 1));

    // switch aggregation and rewritten
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    DEBUG_LOG("projection is:\n%s", operatorToOverviewString((Node *) proj));

    // return projection
    LOG_RESULT("Aggregation - Rewritten Operator tree", op);
    return (QueryOperator *) proj;
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
                singleton(createCaseWhen((Node *) createOpExpr("=",
                        LIST_MAKE(createConstInt(1), copyObject(provDupAttrRef))),
                        arg)),
                (Node *) createNullConst(DT_INT)
                );
    }

    DEBUG_NODE_BEATIFY_LOG("modified agg function call:", f);
    return (Node *) f;
}

static QueryOperator *
rewritePI_CSComposableSet (SetOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposableTableAccess(TableAccessOperator *op)
{
//    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;

    int relAccessCount = getRelNameCount(&nameState, op->tableName);
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS-Composable - Table Access <%s> <%u>", op->tableName, relAccessCount);

    // copy any as of clause if there
    if (asOf)
        op->asOf = copyObject(asOf);

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
        provAttr = appendToTailOfList(provAttr, newAttrName);
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
    projExpr = appendToTailOfList(projExpr, createConstInt(1));

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
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);
    SET_BOOL_STRING_PROP(newpo,PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME);

    LOG_RESULT("Table Access - Rewritten Operator tree", newpo);
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSComposableConstRel(ConstRelOperator *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static QueryOperator *
rewritePI_CSComposableDuplicateRemOp(DuplicateRemoval *op)
{
    FATAL_LOG("not implemented yet");
    return NULL;
}

static void
addResultTIDAndProvDupAttrs (QueryOperator *op, boolean addToSchema)
{
    int numAttrs = getNumAttrs(op);
    QueryOperator *child = OP_LCHILD(op);

    if (addToSchema)
    {
        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                createAttributeDef(strdup(RESULT_TID_ATTR), DT_INT));
        op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
                    createAttributeDef(strdup(PROV_DUPL_COUNT_ATTR), DT_INT));

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
            createAttributeDef(strdup(RESULT_TID_ATTR), DT_INT));
    op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs,
            createAttributeDef(strdup(PROV_DUPL_COUNT_ATTR), DT_INT));
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
                    DT_INT),
            createFullAttrReference(PROV_DUPL_COUNT_ATTR,
                    0,
                    INT_VALUE(GET_STRING_PROP(op, PROP_PROV_DUP_ATTR)),
                    INVALID_ATTR,
                    DT_INT)
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
rewritePI_CSComposableOrderOp(OrderOperator *op)
{
    QueryOperator *child = OP_LCHILD(op);

    // rewrite child
    rewritePI_CSComposableOperator(child);

    // adapt provenance attr list and schema
    addProvenanceAttrsToSchema((QueryOperator *) op, child);
    addResultTIDAndProvDupAttrs((QueryOperator *) op, TRUE);

    LOG_RESULT("Order Operator - Rewritten Operator tree", op);
    return (QueryOperator *) op;
}
