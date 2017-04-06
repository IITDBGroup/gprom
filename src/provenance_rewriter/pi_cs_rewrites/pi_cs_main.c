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
#include "parser/parser_jp.h"
#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"

#define LOG_RESULT(mes,op) \
    do { \
        INFO_OP_LOG(mes,op); \
        DEBUG_NODE_BEATIFY_LOG(mes,op); \
    } while(0)

static QueryOperator *rewritePI_CSOperator (QueryOperator *op);
static QueryOperator *rewritePI_CSSelection (SelectionOperator *op);
static QueryOperator *rewritePI_CSProjection (ProjectionOperator *op);
static QueryOperator *rewritePI_CSJoin (JoinOperator *op);
static QueryOperator *rewritePI_CSAggregation (AggregationOperator *op);
static QueryOperator *rewritePI_CSSet (SetOperator *op);
static QueryOperator *rewritePI_CSTableAccess(TableAccessOperator *op);
static QueryOperator *rewritePI_CSConstRel(ConstRelOperator *op);
static QueryOperator *rewritePI_CSDuplicateRemOp(DuplicateRemoval *op);
static QueryOperator *rewritePI_CSOrderOp(OrderOperator *op);
static QueryOperator *rewritePI_CSJsonTableOp(JsonTableOperator *op);

static QueryOperator *addUserProvenanceAttributes (QueryOperator *op, List *userProvAttrs, boolean showIntermediate);
static QueryOperator *addIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs);
static QueryOperator *rewritePI_CSAddProvNoRewrite (QueryOperator *op, List *userProvAttrs);
static QueryOperator *rewritePI_CSUseProvNoRewrite (QueryOperator *op, List *userProvAttrs);


static Node *asOf;
static RelCount *nameState;
//static QueryOperator *provComputation;

QueryOperator *
rewritePI_CS (ProvenanceComputation  *op)
{
//    List *provAttrs;

    START_TIMER("rewrite - PI-CS rewrite");

    // unset relation name counters
    nameState = (RelCount *) NULL;

    DEBUG_NODE_BEATIFY_LOG("*************************************\nREWRITE INPUT\n"
            "******************************\n", op);

    QueryOperator *rewRoot = OP_LCHILD(op);
    DEBUG_NODE_BEATIFY_LOG("rewRoot is:", rewRoot);

    // cache asOf
    asOf = op->asOf;

    // get provenance attrs
//    provAttrs = getQueryOperatorAttrNames((QueryOperator *) op);

    // rewrite subquery under provenance computation
    rewritePI_CSOperator(rewRoot);

    // update root of rewritten subquery
    rewRoot = OP_LCHILD(op);

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
rewritePI_CSOperator (QueryOperator *op)
{
    boolean showIntermediate = HAS_STRING_PROP(op,  PROP_SHOW_INTERMEDIATE_PROV);
    boolean noRewriteUseProv = HAS_STRING_PROP(op, PROP_USE_PROVENANCE);
    boolean noRewriteHasProv = HAS_STRING_PROP(op, PROP_HAS_PROVENANCE);
    boolean isDummyHasProvProj = HAS_STRING_PROP(op, PROP_DUMMY_HAS_PROV_PROJ);
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
        return rewritePI_CSAddProvNoRewrite(op, userProvAttrs);
    if (isDummyHasProvProj)
    {
        userProvAttrs = (List *) getStringProperty(OP_LCHILD(op), PROP_USER_PROV_ATTRS);
        return rewritePI_CSUseProvNoRewrite(op, userProvAttrs);
    }

    switch(op->type)
    {
        case T_SelectionOperator:
            DEBUG_LOG("go selection");
            rewrittenOp = rewritePI_CSSelection((SelectionOperator *) op);
            break;
        case T_ProjectionOperator:
            DEBUG_LOG("go projection");
            rewrittenOp = rewritePI_CSProjection((ProjectionOperator *) op);
            break;
        case T_AggregationOperator:
            DEBUG_LOG("go aggregation");
            rewrittenOp = rewritePI_CSAggregation ((AggregationOperator *) op);
            break;
        case T_JoinOperator:
            DEBUG_LOG("go join");
            rewrittenOp = rewritePI_CSJoin((JoinOperator *) op);
            break;
        case T_SetOperator:
            DEBUG_LOG("go set");
            rewrittenOp = rewritePI_CSSet((SetOperator *) op);
            break;
        case T_TableAccessOperator:
            DEBUG_LOG("go table access");
            rewrittenOp = rewritePI_CSTableAccess((TableAccessOperator *) op);
            break;
        case T_ConstRelOperator:
            DEBUG_LOG("go const rel operator");
            rewrittenOp = rewritePI_CSConstRel((ConstRelOperator *) op);
            break;
        case T_DuplicateRemoval:
            DEBUG_LOG("go duplicate removal operator");
            rewrittenOp = rewritePI_CSDuplicateRemOp((DuplicateRemoval *) op);
            break;
        case T_OrderOperator:
            DEBUG_LOG("go order operator");
            rewrittenOp = rewritePI_CSOrderOp((OrderOperator *) op);
            break;
        case T_JsonTableOperator:
            DEBUG_LOG("go JsonTable operator");
            rewrittenOp = rewritePI_CSJsonTableOp((JsonTableOperator *) op);
	     break;
        default:
            FATAL_LOG("no rewrite implemented for operator ", nodeToString(op));
            return NULL;
    }

    if (showIntermediate)
        rewrittenOp = addIntermediateProvenance(rewrittenOp, userProvAttrs, ignoreProvAttrs);

    if (rewriteAddProv)
        rewrittenOp = addUserProvenanceAttributes(rewrittenOp, addProvAttrs, showIntermediate);

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel(rewrittenOp));

    return rewrittenOp;
}

static QueryOperator *
addUserProvenanceAttributes (QueryOperator *op, List *userProvAttrs, boolean showIntermediate)
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
addIntermediateProvenance (QueryOperator *op, List *userProvAttrs, Set *ignoreProvAttrs)
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
        tableName = STRING_VALUE(getStringProperty(op, PROP_PROV_REL_NAME));

    relAccessCount = getRelNameCount(&nameState, tableName);

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
rewritePI_CSAddProvNoRewrite (QueryOperator *op, List *userProvAttrs)
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
rewritePI_CSUseProvNoRewrite (QueryOperator *op, List *userProvAttrs)
{
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

        proj = createProjOnAllAttrs(op);

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

        proj->provAttrs = provAttrs;

        if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
            ASSERT(checkModel(proj));

        return proj;
    }
    // for non-tableaccess operators simply change the attribute names and mark the attributes as provenance attributes
    else
    {
        FOREACH(Constant,a,userProvAttrs)
        {
            char *name = STRING_VALUE(a);
            int pos = getAttrPos(op, name);
//            char *oldName;
            AttributeDef *attr;

            attr = getNthOfListP(op->schema->attrDefs, pos);
            name = getProvenanceAttrName(tableName, name, relAccessCount);
//            oldName = attr->attrName;
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

        return op;
    }
}

static QueryOperator *
rewritePI_CSSelection (SelectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Selection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child first
    rewritePI_CSOperator(OP_LCHILD(op));

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    LOG_RESULT("Rewritten Operator tree", op);
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSProjection (ProjectionOperator *op)
{
    ASSERT(OP_LCHILD(op));

    DEBUG_LOG("REWRITE-PICS - Projection");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // rewrite child
    rewritePI_CSOperator(OP_LCHILD(op));

    // add projection expressions for provenance attrs
    QueryOperator *child = OP_LCHILD(op);
    FOREACH_INT(a, child->provAttrs)
    {
        AttributeDef *att = getAttrDef(child,a);
        DEBUG_LOG("attr: %s", nodeToString(att));
         op->projExprs = appendToTailOfList(op->projExprs,
                 createFullAttrReference(att->attrName, 0, a, 0, att->dataType));
    }

    // adapt schema
    addProvenanceAttrsToSchema((QueryOperator *) op, OP_LCHILD(op));

    LOG_RESULT("Rewritten Operator tree", op);
    return (QueryOperator *) op;
}

static QueryOperator *
rewritePI_CSJoin (JoinOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Join");
    QueryOperator *o = (QueryOperator *) op;
    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);
    List *rNormAttrs;
    int numLAttrs, numRAttrs;

    numLAttrs = LIST_LENGTH(lChild->schema->attrDefs);
    numRAttrs = LIST_LENGTH(rChild->schema->attrDefs);

    // get attributes from right input
    rNormAttrs = sublist(o->schema->attrDefs, numLAttrs, numLAttrs + numRAttrs - 1);
    o->schema->attrDefs = sublist(copyObject(o->schema->attrDefs), 0, numLAttrs - 1);

    // rewrite children
    // if (!HAS_STRING_PROP(PROP...))
    lChild = rewritePI_CSOperator(lChild);
    // if (!HAS_STRING_PROP(PROP...))
    rChild = rewritePI_CSOperator(rChild);

    // adapt schema for join op
//    clearAttrsFromSchema((QueryOperator *) op);
//    addNormalAttrsToSchema(o, lChild);
    addProvenanceAttrsToSchema(o, lChild);
    o->schema->attrDefs = CONCAT_LISTS(o->schema->attrDefs, rNormAttrs);
    addProvenanceAttrsToSchema(o, rChild);

    // add projection to put attributes into order on top of join op
    List *projExpr = CONCAT_LISTS(
            getNormalAttrProjectionExprs((QueryOperator *) op),
            getProvAttrProjectionExprs((QueryOperator *) op));
    ProjectionOperator *proj = createProjectionOp(projExpr, NULL, NIL, NIL);

    addNormalAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    addProvenanceAttrsToSchema((QueryOperator *) proj, (QueryOperator *) op);
    switchSubtrees((QueryOperator *) op, (QueryOperator *) proj);
    addChildOperator((QueryOperator *) proj, (QueryOperator *) op);

    // SET PROP IS_REWRITTEN

    LOG_RESULT("Rewritten Operator tree", op);
    return (QueryOperator *) proj;
}

/*
 * Rewrite an aggregation operator:
 *      - replace aggregation with projection over join between the aggregation
 *       and the aggregation rewritten input
 */
static QueryOperator *
rewritePI_CSAggregation (AggregationOperator *op)
{
    JoinOperator *joinProv;
    ProjectionOperator *proj;
    QueryOperator *aggInput;
    QueryOperator *origAgg;
    int numGroupAttrs = LIST_LENGTH(op->groupBy);

    DEBUG_LOG("REWRITE-PICS - Aggregation");

    // copy aggregation input
    origAgg = (QueryOperator *) op;
    aggInput = copyUnrootedSubtree(OP_LCHILD(op));
    // rewrite aggregation input copy
    aggInput = rewritePI_CSOperator(aggInput);

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

        groupByProj = createProjectionOp(groupByProjExprs,
                        aggInput, NIL, attrNames);
        CREATE_INT_SEQ(provAttrs, numGroupAttrs, numGroupAttrs + getNumProvAttrs(aggInput) - 1,1);
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
rewritePI_CSSet(SetOperator *op)
{
    DEBUG_LOG("REWRITE-PICS - Set");

    QueryOperator *lChild = OP_LCHILD(op);
    QueryOperator *rChild = OP_RCHILD(op);

    switch(op->setOpType)
    {
    case SETOP_UNION:
    {
        List *projExprs = NIL;
        List *attNames;
        List *provAttrs = NIL;
        int lProvs;
        int i;

        // rewrite children
        lChild = rewritePI_CSOperator(lChild);
        rChild = rewritePI_CSOperator(rChild);
        lProvs = LIST_LENGTH(lChild->provAttrs);

        // create projection over left rewritten input
        attNames = concatTwoLists(getQueryOperatorAttrNames(lChild), getOpProvenanceAttrNames(rChild));

        // createAttrRefs for attributes of left input
        i = 0;
        FOREACH(AttributeDef,a,lChild->schema->attrDefs)
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i++, INVALID_ATTR, a->dataType);
            projExprs = appendToTailOfList(projExprs, att);
        }
        provAttrs = copyObject(lChild->provAttrs);

        // create NULL expressions for provenance attrs of right input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rChild))
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
                lChild, NIL, attNames);
        ((QueryOperator *) projLeftChild)->provAttrs = provAttrs;

        // create projection over right rewritten input
        provAttrs = NIL;
        projExprs = NIL;
        attNames = CONCAT_LISTS(getNormalAttrNames(rChild),
                getOpProvenanceAttrNames(lChild),
                getOpProvenanceAttrNames(rChild));

        // create AttrRefs for normal attributes of right input
        i = 0;
        FOREACH(AttributeDef,a,getNormalAttrs(rChild))
        {
            AttributeReference *att;
            att = createFullAttrReference(strdup(a->attrName), 0, i++, INVALID_ATTR, a->dataType);
            projExprs = appendToTailOfList(projExprs, att);
        }

        // create NULL expressions for provenance attrs of left input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(lChild))
        {
            Constant *expr;

            expr = createNullConst(a->dataType);
            projExprs = appendToTailOfList(projExprs, expr);
            provAttrs = appendToTailOfListInt(provAttrs, i++);
        }

        // create AttrRefs for provenance attrs of right input
        FOREACH(AttributeDef,a, getProvenanceAttrDefs(rChild))
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
                rChild, NIL, attNames);
        ((QueryOperator *) projRightChild)->provAttrs = provAttrs;

    	// make projections of rewritten inputs the direct children of the union operation
        switchSubtrees(lChild, (QueryOperator *) projLeftChild);
        switchSubtrees(rChild, (QueryOperator *) projRightChild);
        lChild->parents = singleton(projLeftChild);
        rChild->parents = singleton(projRightChild);

    	// adapt schema of union itself, we can get full provenance attributes from left input
    	addProvenanceAttrsToSchema((QueryOperator *) op, (QueryOperator *) projLeftChild);
        return (QueryOperator *) op;
    }
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
            comparisons = appendToTailOfList(comparisons, createOpExpr("=",
                    LIST_MAKE(createFullAttrReference (strdup(lDef->attrName),0,i,INVALID_ATTR, lDef->dataType),
                            createFullAttrReference (strdup(rDef->attrName),1,i,INVALID_ATTR, rDef->dataType))
                    ));
        }
        joinCond = andExprList(comparisons);
        DEBUG_LOG("join cond: %s", beatify(nodeToString(joinCond)));

        // rewrite children
        lChild = rewritePI_CSOperator(lChild);
        rChild = rewritePI_CSOperator(rChild);

        //restrcuture the tree
    	JoinOperator *joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(lChild,rChild), NIL, NIL);
    	removeParent(lChild, (QueryOperator *) op);
    	addParent(lChild,(QueryOperator *)  joinOp);
    	removeParent(rChild, (QueryOperator *) op);
    	addParent(rChild, (QueryOperator *) joinOp);

        // adapt schema for join op
        clearAttrsFromSchema((QueryOperator *) joinOp);
        addNormalAttrsToSchema((QueryOperator *) joinOp, lChild);
        addProvenanceAttrsToSchema((QueryOperator *) joinOp, lChild);
        addNormalAttrsToSchema((QueryOperator *) joinOp, rChild);
        addProvenanceAttrsToSchema((QueryOperator *) joinOp, rChild);

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
    case SETOP_DIFFERENCE:
    {
    	JoinOperator *joinOp;
    	ProjectionOperator *projOp;
    	QueryOperator *rewrLeftChild = rewritePI_CSOperator(
    	        copyUnrootedSubtree(lChild));
    	QueryOperator *rewrRightChild = rewritePI_CSOperator(
                copyUnrootedSubtree(rChild)); //TODO do not rewrite just get prov attrs
    	// join provenance with rewritten right input
    	// create join condition
        Node *joinCond;
        List *joinAttrs = CONCAT_LISTS(getQueryOperatorAttrNames((QueryOperator *) op),
                getQueryOperatorAttrNames(rewrLeftChild));
        makeNamesUnique(joinAttrs, NULL);
    	joinCond = NULL;

        FORBOTH(AttributeReference, aL , aR, getNormalAttrProjectionExprs(lChild),
                getNormalAttrProjectionExprs(rewrLeftChild))
        {
            aL->fromClauseItem = 0;
            aR->fromClauseItem = 1;
            if(joinCond)
                joinCond = AND_EXPRS((Node *) createIsNotDistinctExpr((Node *) aL, (Node *) aR), joinCond);
            else
                joinCond = (Node *) createIsNotDistinctExpr((Node *) aL, (Node *) aR);
        }

        joinOp = createJoinOp(JOIN_INNER, joinCond, LIST_MAKE(op, rewrLeftChild),
                NIL, joinAttrs);
        joinOp->op.provAttrs = copyObject(rewrLeftChild->provAttrs);
        SHIFT_INT_LIST(joinOp->op.provAttrs, getNumAttrs((QueryOperator *) op));

    	// adapt schema using projection
        List *rightProvAttrs = getProvenanceAttrDefs(rewrRightChild);
//        List *rightProvNames = getOpProvenanceAttrNames(rewrRightChild);

        List *projExpr = CONCAT_LISTS(getNormalAttrProjectionExprs((QueryOperator *)op),
                getProvAttrProjectionExprs((QueryOperator *) joinOp));
        FOREACH(AttributeDef,a,rightProvAttrs)
            projExpr = appendToTailOfList(projExpr, createNullConst(a->dataType));

        List *projAttrs = getQueryOperatorAttrNames((QueryOperator *) op);

        projOp = createProjectionOp(projExpr, (QueryOperator *) joinOp, NIL, projAttrs);
        projOp->op.provAttrs = copyObject(rewrLeftChild->provAttrs);
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, OP_LCHILD(projOp));
    	addProvenanceAttrsToSchema((QueryOperator *) projOp, rewrRightChild);
    	addParent((QueryOperator *) joinOp, (QueryOperator *) projOp);

    	// switch original set diff with projection
    	switchSubtreeWithExisting((QueryOperator *) op, (QueryOperator *) projOp);
    	addParent((QueryOperator *) op, (QueryOperator *) joinOp);
    	addParent((QueryOperator *) rewrLeftChild, (QueryOperator *) joinOp);

    	return (QueryOperator *) projOp;
    }
    default:
    	break;
    }
    return NULL;
}

static QueryOperator *
rewritePI_CSTableAccess(TableAccessOperator *op)
{
//    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;

    int relAccessCount = getRelNameCount(&nameState, op->tableName);
    int cnt = 0;

    DEBUG_LOG("REWRITE-PICS - Table Access <%s> <%u>", op->tableName, relAccessCount);

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
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite table access: %s", operatorToOverviewString((Node *) newpo));
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSConstRel(ConstRelOperator *op)
{
//    List *tableAttr;
    List *provAttr = NIL;
    List *projExpr = NIL;
    char *newAttrName;

    int relAccessCount = getRelNameCount(&nameState, "query");
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
    switchSubtrees((QueryOperator *) op, (QueryOperator *) newpo);

    // Add child to the newly created projections operator,
    addChildOperator((QueryOperator *) newpo, (QueryOperator *) op);

    DEBUG_LOG("rewrite const rel operator: %s", operatorToOverviewString((Node *) newpo));
    return (QueryOperator *) newpo;
}

static QueryOperator *
rewritePI_CSDuplicateRemOp(DuplicateRemoval *op)
{
    QueryOperator *child = OP_LCHILD(op);
    QueryOperator *theOp = (QueryOperator *) op;

    // remove duplicate removal op
    removeParentFromOps(singleton(child), theOp);
    switchSubtreeWithExisting(theOp, child);

    return rewritePI_CSOperator(child);
}

static QueryOperator *
rewritePI_CSOrderOp(OrderOperator *op)
{
    QueryOperator *child = OP_LCHILD(op);

    // rewrite child
    rewritePI_CSOperator(child);

    // adapt provenance attr list and schema
    addProvenanceAttrsToSchema((QueryOperator *) op, child);

    return (QueryOperator *) op;
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
		appendStringInfoString(forOrdinality, itoa(*countFOD));
        (*attr)->forOrdinality = forOrdinality->data;

		StringInfo renameFOD = makeStringInfo ();
		char *prefixRFOD = "prov_path_";
		appendStringInfoString(renameFOD, prefixRFOD);
		appendStringInfoString(renameFOD, itoa(*countFOD));
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
rewritePI_CSJsonTableOp(JsonTableOperator *op)
{
	QueryOperator *child = OP_LCHILD(op);

	// rewrite child
	child = rewritePI_CSOperator(child);

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
		appendStringInfoString(forOrdinality, itoa(countFOD));
        op->forOrdinality = forOrdinality->data;

		StringInfo renameFOD = makeStringInfo ();
		char *prefixRFOD = "prov_path_";
		appendStringInfoString(renameFOD, prefixRFOD);
		appendStringInfoString(renameFOD, itoa(countFOD));
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
