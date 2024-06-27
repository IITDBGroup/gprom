#include "common.h"
#include "log/logger.h"

#include "analysis_and_translate/translator_oracle.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/zonotope_rewrites/zonotope_rewriter.h"
#include "utility/enum_magic.h"
#include "utility/string_utils.h"

static QueryOperator *rewrite_ZonoProvComp(QueryOperator *op);
static QueryOperator *rewrite_ZonoProjection(QueryOperator *op);
static QueryOperator *rewrite_ZonoTableAccess(QueryOperator *op);

static void markZonoAttrsAsProv(QueryOperator *op);
static void addZonoRowToSchema(HashMap *hmp, QueryOperator *target);
static void addZonoAttrToSchema(HashMap *hmp, QueryOperator *target, Node *aRef);


// ------------- Rewriter Functions ------------- 


/// @brief Recursive entry point for Zonotope rewriting
/// @param op operator to begin rewrite on
/// @return The root operator transformed into a proper sql operator
QueryOperator *rewriteZono(QueryOperator * op)
{
    QueryOperator *rewrittenOp;

    switch(op->type)
    {
        // Any statement that is from GProM is considered "Provenance" in this scope
        case T_ProvenanceComputation:
        {
            INFO_OP_LOG("Zono rewrite for Provenance Computation (T_ProvenanceComputation)");
            rewrittenOp = rewrite_ZonoProvComp(op);
            break;
        }
        // Projection here is actually select statements! This is a Relational Algebra Projection!
        case T_ProjectionOperator:
        {
            INFO_OP_LOG("Zono rewrite for Projection Operator (T_ProjectionOperator)");
            rewrittenOp = rewrite_ZonoProjection(op);
            break;
        }
        // Pretty self explanitory, but any time the table is accessed, this is the operator (i.e. FROM table)
        case T_TableAccessOperator:
        {
            INFO_OP_LOG("Zono rewrite for Table Access Operator (T_TableAccessOperator)");
            rewrittenOp = rewrite_ZonoTableAccess(op);
            break;
        }
        default:
        {
            FATAL_LOG("Zono rewrite not implemented (%s)", NodeTagToString(op->type));
            FATAL_LOG("ABORTING!!!");
            rewrittenOp = NULL;
            break;
        }
    }
    return rewrittenOp;
}


/// @brief Recursive entry point for any custom GProM statement
/// @param op operator to begin rewrite on
/// @return The root operator transformed into a proper sql operator
static QueryOperator *rewrite_ZonoProvComp(QueryOperator *op)
{
    INFO_LOG("REWRITE-ZONO - Provenance");
    // INFO_OP_LOG("Operator tree ", op);

    // Zono only supports 1 child statement!
    ASSERT(LIST_LENGTH(op->inputs) == 1);

    QueryOperator *top = getHeadOfListP(op->inputs);
    top = rewriteZono(top);

    // Make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    disambiguiteAttrNames((Node *) top, done);

    // Convert Provenance statement to whatever the child is, but keep the same tree struct!
    switchSubtrees((QueryOperator *) op, top);
    LOG_RESULT("Zono: Rewritten Operator tree [PROVENANCE]", op);
    return top;
}


/// @brief Recursive entry point for any Projection statement (i.e SELECT)
/// @param op operator to begin rewrite on
/// @return The root operator transformed into a proper sql operator
static QueryOperator *rewrite_ZonoProjection(QueryOperator *op)
{
    (void) removeZonoOpFromExpr(NULL);

    ASSERT(OP_LCHILD(op));

    //rewrite child first
    rewriteZono(OP_LCHILD(op));

    INFO_LOG("REWRITE-ZONO - Projection");
    INFO_OP_LOG("Operator tree ", op);

    HashMap *hmp = NEW_MAP(Node, Node);
    HashMap *hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), ZONO_MAPPING_PROP);

    // Create list of child expressions
    int ict = 0;
    List *uncertlist = NIL;
    List *attrExpr = getProjExprsForAllAttrs(op);
    FOREACH(Node, nd, attrExpr)
    {
        addZonoAttrToSchema(hmp, op, nd);
        Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs, ict);
        Node *ubExpr = getUBExpr(projexpr, hmpIn);
        Node *lbExpr = getLBExpr(projexpr, hmpIn);

        ict++;
        uncertlist = appendToTailOfList(uncertlist, ubExpr);
        uncertlist = appendToTailOfList(uncertlist, lbExpr);
        replaceNode(((ProjectionOperator *)op)->projExprs, projexpr, removeZonoOpFromExpr(projexpr));
    }
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);

    addZonoRowToSchema(hmp, op);
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ZONO_ROW_CERTAIN)));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ZONO_ROW_BESTGUESS)));
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ZONO_ROW_POSSIBLE)));
    setStringProperty(op, ZONO_MAPPING_PROP, (Node *)hmp);

    LOG_RESULT("Zono: Rewritten Operator tree [PROJECTION]", op);
    return op;
}


/// @brief Recursive entry point for any Table Access statement (i.e FROM)
/// @param op operator to begin rewrite on
/// @return The root operator transformed into a proper sql operator
static QueryOperator *rewrite_ZonoTableAccess(QueryOperator *op)
{
    INFO_LOG("REWRITE-ZONO - TableAccess");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    HashMap *hmp = NEW_MAP(Node, Node);

    // Create new Projection Operator to assign stuff to instead of access op
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getQueryOperatorAttrNames(op));
    switchSubtrees(op, proj);
    op->parents = singleton(proj);

    List *attrExpr = getNormalAttrProjectionExprs(op);
    FOREACH(Node, nd, attrExpr)
    {
        addZonoAttrToSchema(hmp, proj, nd);
        appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
        appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
    }

    addZonoRowToSchema(hmp, proj);
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
    setStringProperty(proj, ZONO_MAPPING_PROP, (Node *)hmp);
    markZonoAttrsAsProv(proj);

    LOG_RESULT("ZONO: Rewritten Operator tree [TABLE ACCESS]", op);
    return proj;
}


// ------------- Helper Functions -------------


/// @brief Adds CET_R, BST_R, POS_R to the result schema
/// @param hmp Hashmap to store data to
/// @param target The operator which is being modified
static void addZonoRowToSchema(HashMap *hmp, QueryOperator *target)
{
    addAttrToSchema(target, ZONO_ROW_CERTAIN, DT_INT);
    ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ZONO_ROW_CERTAIN), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
    addAttrToSchema(target, ZONO_ROW_BESTGUESS, DT_INT);
    ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ZONO_ROW_BESTGUESS), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
    addAttrToSchema(target, ZONO_ROW_POSSIBLE, DT_INT);
    ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ZONO_ROW_POSSIBLE), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
}


/// @brief Add Z_UB_x and Z_LB_x to the schema for provided attribute
/// @param hmp Hashmap to store data to
/// @param target The operator which is being modified
/// @param aRef ???
static void addZonoAttrToSchema(HashMap *hmp, QueryOperator *target, Node *aRef)
{
    ((AttributeReference *)aRef)->outerLevelsUp = 0;
    addAttrToSchema(target, getZonoUBString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
    List *refs = singleton((Node *)getTailOfListP(getProjExprsForAllAttrs(target)));
    addAttrToSchema(target, getZonoLBString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
    appendToTailOfList(refs, (Node *)getTailOfListP(getProjExprsForAllAttrs(target)));

    // Map each attribute to their upper&lower bounds list
    ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)refs));
}


static void markZonoAttrsAsProv(QueryOperator *op)
{
    HashMap *hmp = (HashMap *)getStringProperty(op, ZONO_MAPPING_PROP);
    Set *uncertAttrs = STRSET();
    int pos = 0;

    FOREACH_HASH(Node, n, hmp)
    {
        if (isA(n, List))
        {
            List *l = (List *)n;
            AttributeReference *la = (AttributeReference *)(getHeadOfListP(l));
            AttributeReference *ra = (AttributeReference *)getNthOfListP(l, 1);
            addToSet(uncertAttrs, la->name);
            addToSet(uncertAttrs, ra->name);
        }
    }

    FOREACH(AttributeDef, a, op->schema->attrDefs)
    {
        if (streq(a->attrName, ROW_ZONO) || hasSetElem(uncertAttrs, a->attrName))
        {
            op->provAttrs = appendToTailOfListInt(op->provAttrs, pos);
        }
        pos++;
    }

    DEBUG_LOG("after marking attributes as uncertain: \n%s", singleOperatorToOverview(op));
}


Node *removeZonoOpFromExpr(Node *expr)
{
    if (!expr)
    {
        return NULL;
    }

    switch (expr->type)
    {
        case T_Operator:
        {
            if (streq(((Operator *)expr)->name, ZUNCERT_FUNC_NAME))
            {
                return (Node *)getHeadOfListP(((Operator *)expr)->args);
            }
            FOREACH(Node, nd, ((Operator *)expr)->args)
            {
                replaceNode(((Operator *)expr)->args, nd, removeZonoOpFromExpr(nd));
            }
            return expr;
            break;
        }
        case T_CaseExpr:
        {
            ((CaseExpr *)expr)->elseRes = removeZonoOpFromExpr(((CaseExpr *)expr)->elseRes);
            ((CaseExpr *)expr)->expr = removeZonoOpFromExpr(((CaseExpr *)expr)->expr);
            FOREACH(Node, nd, ((CaseExpr *)expr)->whenClauses)
            {
                CaseWhen *tmp = (CaseWhen *)nd;
                tmp->when = removeZonoOpFromExpr(tmp->when);
                tmp->then = removeZonoOpFromExpr(tmp->then);
            }
            return expr;
            break;
        }
        case T_FunctionCall:
        {
            if (streq(((FunctionCall *)expr)->functionname, ZUNCERT_FUNC_NAME))
            {
                return (Node *)getHeadOfListP(((FunctionCall *)expr)->args);
            }
            FOREACH(Node, nd, ((FunctionCall *)expr)->args)
            {
                replaceNode(((Operator *)expr)->args, nd, removeZonoOpFromExpr(nd));
            }
            return expr;
            break;
        }
        default:
        {
            return expr;
            break;
        }
    }

    return expr;
}


char *getZonoUBString(char *in)
{
    StringInfo str = makeStringInfo();
    appendStringInfo(str, "%s", ZONO_ATTR_HIGH_BOUND);
    appendStringInfo(str, "%s", in);
    return backendifyIdentifier(str->data);
}


char *getZonoLBString(char *in)
{
    StringInfo str = makeStringInfo();
    appendStringInfo(str, "%s", ZONO_ATTR_LOW_BOUND);
    appendStringInfo(str, "%s", in);
    return backendifyIdentifier(str->data);
}


Node *getUBExpr(Node *expr, HashMap *hmp)
{
    switch (expr->type)
    {
        case T_AttributeReference:
        {
            if (((AttributeReference *)expr)->outerLevelsUp == -1)
            {
                ((AttributeReference *)expr)->outerLevelsUp = 0;
            }

            Node *ret = getNthOfListP((List *)getMap(hmp, expr), 0);
            ((AttributeReference *)ret)->outerLevelsUp = 0;
            return ret;
        }
        default:
        {
            FATAL_LOG("[getUBExpr] Unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
            break;
        }
    }
    return NULL;
}


Node *getLBExpr(Node *expr, HashMap *hmp)
{
    switch (expr->type)
    {
        case T_AttributeReference:
        {
            if (((AttributeReference *)expr)->outerLevelsUp == -1)
            {
                ((AttributeReference *)expr)->outerLevelsUp = 0;
            }
            
            Node *ret = getNthOfListP((List *)getMap(hmp, expr), 0);
            ((AttributeReference *)ret)->outerLevelsUp = 0;
            return ret;
        }
        default:
        {
            FATAL_LOG("[getLBExpr] Unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
            break;
        }
    }
    return NULL;
}

