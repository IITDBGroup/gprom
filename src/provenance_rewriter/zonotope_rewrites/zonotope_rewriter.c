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
static void addZonoWorldRowToSchema(HashMap *hmp, QueryOperator *target);
static void addZonoBoundAttrToSchema(HashMap *hmp, QueryOperator *target, Node *aRef);


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
    List *tempList = NIL;   // TODO: REMOVE ME

    FOREACH(Node, nd, attrExpr)
    {
        addZonoBoundAttrToSchema(hmp, op, nd);
        Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs, ict);
        // INFO_LOG("Expr: %s", nodeToString(projexpr));
        Node *ubExpr = getUBExpr(projexpr, hmpIn);
        // INFO_LOG("Ub: %s", nodeToString(ubExpr));
        Node *lbExpr = getLBExpr(projexpr, hmpIn);
        // INFO_LOG("Lb: %s", nodeToString(lbExpr));

        ict++;
        uncertlist = appendToTailOfList(uncertlist, ubExpr);
        uncertlist = appendToTailOfList(uncertlist, lbExpr);
        replaceNode(((ProjectionOperator *)op)->projExprs, projexpr, removeZonoOpFromExpr(projexpr));

        // TODO: THIS IS A TEMP FIX
        // IDEALLY THIS WILL SHOW THE VALUE OF THE PARENT ATTRIBUTE IF IT IS A VALID OPERATION, IF NOT
        // THEN THE ATTRIBUTE SHOULD BE OMMITED ENTIRELY, THIS IS A PROBLEM FOR L;ATER...
        if (((AttributeReference *) ((Node*)nd))->attrType == DT_STRING) {
            tempList = appendToTailOfList(tempList, copyObject(nd));
        }
        else {
            tempList = appendToTailOfList(tempList, (Node *)createConstInt(0));
        }
        // ^^
    }
    // ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(tempList, uncertlist);

    addZonoWorldRowToSchema(hmp, op);
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
        addZonoBoundAttrToSchema(hmp, proj, nd);
        appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
        appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
        // appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstFloat(1.0));
        // appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstFloat(1.0));
    }

    addZonoWorldRowToSchema(hmp, proj);
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
static void addZonoWorldRowToSchema(HashMap *hmp, QueryOperator *target)
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
static void addZonoBoundAttrToSchema(HashMap *hmp, QueryOperator *target, Node *aRef)
{
    ((AttributeReference *)aRef)->outerLevelsUp = 0;
    List* refs = NIL;

    char* zonoUBString = getZonoUBString(((AttributeReference *)aRef)->name);
    addAttrToSchema(target, zonoUBString, DT_FLOAT);
    Node* ubNode = (Node*) createFunctionCall("get_ub", singleton((Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
    refs = appendToTailOfList(refs, ubNode);

    char* zonoLBString = getZonoLBString(((AttributeReference *)aRef)->name);
    addAttrToSchema(target, zonoLBString, DT_FLOAT);
    Node* lbNode = (Node*) createFunctionCall("get_lb", singleton((Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
    refs = appendToTailOfList(refs, lbNode);

    ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node*)refs));
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


static Node *ZonoUBOp(Operator *expr, HashMap *hmp)
{
    if (!expr)
    {
        return NULL;
    }
    if (strcmp(expr->name, ZUNCERT_FUNC_NAME) == 0)
    {
        return (Node *)createConstInt(0);
    }
    if (strcmp(expr->name, "+") == 0)
    {
        Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
        Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
        // Upper bound of addition is the sum of upper bounds
        Node *ret = (Node *)createFunctionCall("z_add", appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
        // INFO_LOG("REWRITE_RANGE_EXPR_PLUS: %s", nodeToString(ret));
        return ret;
    }
    if (strcmp(expr->name, "-") == 0)
    {
        Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
        Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
        // Upper bound of subtraction is the ub-lb
        Node *ret = (Node *)createFunctionCall("z_sub", appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
        return ret;
    }
    // if (strcmp(expr->name, OPNAME_EQ) == 0)
    // {
    //     // INFO_LOG("rewrite = ");
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *c1 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     Node *c2 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     // Node *c1 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
    //     // Node *c2 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
    //     // Node *c3 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
    //     // Node *c4 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
    //     // Node *c5 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
    //     // Node *c6 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
    //     // Node *c12 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c1),c2));
    //     // Node *c34 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c3),c4));
    //     // Node *c56 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c5),c6));
    //     // Node *c1234 = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(c12),c34));
    //     // Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(c1234),c56));
    //     Node *ret = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(c1, c2));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_GT) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_GT, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_GE) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_LT) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_LT, appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_LE) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, "*") == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *c1 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     Node *c2 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     Node *c3 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     Node *c4 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     Node *c12 = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c1), c2));
    //     Node *c34 = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c3), c4));
    //     Node *ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c12), c34));
    //     return ret;
    // }
    // else if (strcmp(strToUpper(expr->name), OPNAME_OR) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // else if (strcmp(strToUpper(expr->name), OPNAME_AND) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // else
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     return (Node *)createOpExpr(expr->name, LIST_MAKE(getUBExpr(e1, hmp), getUBExpr(e2, hmp)));
    // }
    return NULL;
}


static Node *ZonoLBOp(Operator *expr, HashMap *hmp)
{
    if (!expr)
    {
        return NULL;
    }
    if (strcmp(expr->name, ZUNCERT_FUNC_NAME) == 0)
    {
        return (Node *)createConstInt(0);
    }
    if (strcmp(expr->name, "+") == 0)
    {
        Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
        Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
        // Upper bound of addition is the sum of upper bounds
        Node *ret = (Node *)createFunctionCall("z_add", appendToTailOfList(singleton(getLBExpr(e1, hmp)), getLBExpr(e2, hmp)));
        // INFO_LOG("REWRITE_RANGE_EXPR_PLUS: %s", nodeToString(ret));
        return ret;
    }
    if (strcmp(expr->name, "-") == 0)
    {
        Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
        Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
        // Upper bound of subtraction is the ub-lb
        Node *ret = (Node *)createFunctionCall("z_sub", appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
        return ret;
    }
    // if (strcmp(expr->name, OPNAME_EQ) == 0)
    // {
    //     // INFO_LOG("rewrite = ");
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *c1 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     Node *c2 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     // Node *c1 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
    //     // Node *c2 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getLBExpr(e2, hmp)));
    //     // Node *c3 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getUBExpr(e2, hmp)));
    //     // Node *c4 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
    //     // Node *c5 = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)),getUBExpr(e2, hmp)));
    //     // Node *c6 = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)),getLBExpr(e2, hmp)));
    //     // Node *c12 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c1),c2));
    //     // Node *c34 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c3),c4));
    //     // Node *c56 = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(c5),c6));
    //     // Node *c1234 = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(c12),c34));
    //     // Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(c1234),c56));
    //     Node *ret = (Node *)createOpExpr(OPNAME_AND, LIST_MAKE(c1, c2));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_GT) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_GT, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_GE) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_GE, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_LT) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_LT, appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, OPNAME_LE) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_LE, appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // if (strcmp(expr->name, "*") == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *c1 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     Node *c2 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getUBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     Node *c3 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     Node *c4 = (Node *)createOpExpr("*", appendToTailOfList(singleton(getLBExpr(e1, hmp)), getLBExpr(e2, hmp)));
    //     Node *c12 = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c1), c2));
    //     Node *c34 = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c3), c4));
    //     Node *ret = (Node *)createFunctionCall(GREATEST_FUNC_NAME, appendToTailOfList(singleton(c12), c34));
    //     return ret;
    // }
    // else if (strcmp(strToUpper(expr->name), OPNAME_OR) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_OR, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // else if (strcmp(strToUpper(expr->name), OPNAME_AND) == 0)
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     Node *ret = (Node *)createOpExpr(OPNAME_AND, appendToTailOfList(singleton(getUBExpr(e1, hmp)), getUBExpr(e2, hmp)));
    //     return ret;
    // }
    // else
    // {
    //     Node *e1 = (Node *)(getNthOfListP(expr->args, 0));
    //     Node *e2 = (Node *)(getNthOfListP(expr->args, 1));
    //     return (Node *)createOpExpr(expr->name, LIST_MAKE(getUBExpr(e1, hmp), getUBExpr(e2, hmp)));
    // }
    return NULL;
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
        case T_Operator:
        {
            return ZonoUBOp((Operator *)expr, hmp);
        }
        case T_FunctionCall:
        {
            FATAL_LOG("[getUBExpr] Called a function");
            break;
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
            
            Node *ret = getNthOfListP((List *)getMap(hmp, expr), 1);
            ((AttributeReference *)ret)->outerLevelsUp = 0;
            return ret;
        }
        case T_Operator:
        {
            return ZonoLBOp((Operator *)expr, hmp);
        }
        case T_FunctionCall:
        {
            FATAL_LOG("[getLBExpr] Called a function");
            break;
        }
        default:
        {
            FATAL_LOG("[getLBExpr] Unknown expression type for uncertainty:(%d) %s", expr->type, nodeToString(expr));
            break;
        }
    }
    return NULL;
}

