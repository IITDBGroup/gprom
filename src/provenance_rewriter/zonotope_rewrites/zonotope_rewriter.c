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

static Node *removeZonoOpFromExpr(Node *expr);
static void markZonoAttrsAsProv(QueryOperator *op);
static void addZonoRowToSchema(HashMap *hmp, QueryOperator *target);
static void markZonoAttrsAsProv(QueryOperator *op);
// static void addZonoAttrToSchema(HashMap *hmp, QueryOperator *target, Node *aRef);

char *getZonoString(char *in);


// ------------- Rewriter Functions ------------- 


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

    addZonoRowToSchema(hmp, op);
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ROW_ZONO)));
    setStringProperty(op, ZONO_MAPPING_PROP, (Node *)hmp);

    LOG_RESULT("Zono: Rewritten Operator tree [PROJECTION]", op);
    return op;
}


static QueryOperator *rewrite_ZonoTableAccess(QueryOperator *op)
{
    INFO_LOG("REWRITE-ZONO - TableAccess");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    HashMap *hmp = NEW_MAP(Node, Node);
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getQueryOperatorAttrNames(op));
    switchSubtrees(op, proj);
    op->parents = singleton(proj);
    // List *attrExpr = getNormalAttrProjectionExprs(op);
    // FOREACH(Node, nd, attrExpr)
    // {
    //     addZonoAttrToSchema(hmp, proj, nd);
    //     appendToTailOfList(((ProjectionOperator *)proj)->projExprs, copyObject(nd));
    // }
    addZonoRowToSchema(hmp, proj);
    appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
    setStringProperty(proj, ZONO_MAPPING_PROP, (Node *)hmp);
    markZonoAttrsAsProv(proj);

    LOG_RESULT("ZONO: Rewritten Operator tree [TABLE ACCESS]", op);
    return proj;
}


// ------------- Helper Functions -------------


// Not sure, but it is needed, ripped from uncert_rewriter.c
static Node *removeZonoOpFromExpr(Node *expr)
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


static void addZonoRowToSchema(HashMap *hmp, QueryOperator *target)
{
    addAttrToSchema(target, ROW_ZONO, DT_INT);
    ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference(ROW_ZONO), (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
}


static void markZonoAttrsAsProv(QueryOperator *op)
{
    HashMap *hmp = (HashMap *)getStringProperty(op, ZONO_MAPPING_PROP);
    Set *uncertAttrs = STRSET();
    int pos = 0;

    // INFO_LOG("MARK ATTRIBUTES AS UNCERTAIN FOR: \n%s",singleOperatorToOverview(op));

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

    // INFO_LOG("Uncertainty Attributes: %s",nodeToString(uncertAttrs));

    FOREACH(AttributeDef, a, op->schema->attrDefs)
    {
        // INFO_LOG("check attribute %s", a->attrName);
        if (streq(a->attrName, ROW_ZONO) ||
            hasSetElem(uncertAttrs, a->attrName))
        {
            op->provAttrs = appendToTailOfListInt(op->provAttrs, pos);
        }
        pos++;
    }

    DEBUG_LOG("after marking attributes as uncertain: \n%s",
              singleOperatorToOverview(op));
}


char *getZonoString(char *in)
{
    StringInfo str = makeStringInfo();
    appendStringInfo(str, "%s", backendifyIdentifier("ZONO_"));
    appendStringInfo(str, "%s", in);
    return backendifyIdentifier(str->data);
}


// static void addZonoAttrToSchema(HashMap *hmp, QueryOperator *target, Node *aRef)
// {
//     ((AttributeReference *)aRef)->outerLevelsUp = 0;
//     addAttrToSchema(target, getZonoString(((AttributeReference *)aRef)->name), ((AttributeReference *)aRef)->attrType);
//     List *refs = singleton((Node *)getTailOfListP(getProjExprsForAllAttrs(target)));
//     // Map each attribute to their upper&lower bounds list
//     ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)refs));
// }

