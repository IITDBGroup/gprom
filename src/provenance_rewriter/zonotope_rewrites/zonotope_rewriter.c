#include "common.h"
#include "log/logger.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "analysis_and_translate/translator_oracle.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "utility/enum_magic.h"
#include "utility/string_utils.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "configuration/option.h"

#define UNCERT_FUNC_NAME backendifyIdentifier("uncert")

/* xtables attributes */
#define MAX_PROB_ATTR_NAME "MAX_PROB"
#define SUM_PROB_ATTR_NAME "SUM_PROB"
#define COUNT_ATTR_NAME "COUNT_"
#define ROW_NUM_BY_ID_ATTR_NAME "ROW_NUM_BY_ID"

/* bound based uncertainty */
#define ATTR_LOW_BOUND backendifyIdentifier("LB_")
#define ATTR_HIGH_BOUND backendifyIdentifier("UB_")
#define ATTR_UNCERT_PFX backendifyIdentifier("U_")
#define SELFJOIN_AFFIX backendifyIdentifier("1")//differentiate attr names when selfjoin

/* rank and window attributes */
#define ATTR_ISEND backendifyIdentifier("ISEND")
#define ATTR_NOTEND backendifyIdentifier("NOTEND")
#define ATTR_POINT backendifyIdentifier("P")
#define ATTR_RANK backendifyIdentifier("RANK")
#define AGG_ATTR_RENAME backendifyIdentifier("AGG_ATTR")
#define RANK_ATTR_RENAME backendifyIdentifier("RANKR")

#define UNCERT_MAPPING_PROP "UNCERT_MAPPING"



static QueryOperator *rewrite_ZonoProvComp(QueryOperator *op);
static QueryOperator *rewrite_ZonoProjection(QueryOperator *op);
static QueryOperator *rewrite_ZonoTableAccess(QueryOperator *op);


QueryOperator *rewriteZono(QueryOperator * op)
{
    // FATAL_LOG("RANGE_REWRITE");
    QueryOperator *rewrittenOp;

    switch(op->type)
    {
        case T_ProvenanceComputation:
        {
            INFO_OP_LOG("Zono rewrite for Provenance Computation (T_ProvenanceComputation)");
            rewrittenOp = rewrite_ZonoProvComp(op);
            break;
        }
        case T_ProjectionOperator:
        {
            INFO_OP_LOG("Zono rewrite for Projection Operator (T_ProjectionOperator)");
            rewrittenOp = rewrite_ZonoProjection(op);
            break;
        }
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
    // INFO_OP_LOG("Zono rewrite for %s completed", NodeTagToString(op->type));
    return rewrittenOp;
}


static QueryOperator *rewrite_ZonoProvComp(QueryOperator *op)
{
    ASSERT(LIST_LENGTH(op->inputs) == 1);

    QueryOperator *top = getHeadOfListP(op->inputs);
    top = rewriteZono(top);

    // Make sure we do not introduce name clashes, but keep the top operator's schema intact
    Set *done = PSET();
    disambiguiteAttrNames((Node *) top, done);

    // adapt inputs of parents to remove provenance computation
    switchSubtrees((QueryOperator *) op, top);
    DEBUG_NODE_BEATIFY_LOG("rewritten query root for range is:\n", top);

    return top;
}


static Node *removeZonoOpFromExpr(Node *expr)
{
    if(!expr){
        return NULL;
    }

    switch(expr->type){
        case T_Operator: 
        {
            if(streq(((Operator *)expr)->name,UNCERT_FUNC_NAME)) 
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
            if(streq(((FunctionCall *)expr)->functionname,UNCERT_FUNC_NAME))
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


static QueryOperator *rewrite_ZonoProjection(QueryOperator *op)
{
    ASSERT(OP_LCHILD(op));

    //rewrite child first
    rewriteZono(OP_LCHILD(op));

    INFO_LOG("REWRITE-ZONO - Projection");
    INFO_OP_LOG("Operator tree ", op);

    HashMap * hmp = NEW_MAP(Node, Node);

    //get child hashmap
    HashMap * hmpIn = (HashMap *)getStringProperty(OP_LCHILD(op), UNCERT_MAPPING_PROP);
    List *attrExpr = getProjExprsForAllAttrs(op);
    List *uncertlist = NIL;
    int ict = 0;
    FOREACH(Node, nd, attrExpr){
        (void)nd;
        Node *projexpr = (Node *)getNthOfListP(((ProjectionOperator *)op)->projExprs,ict);
        ict ++;
        replaceNode(((ProjectionOperator *)op)->projExprs, projexpr, removeZonoOpFromExpr(projexpr));
    }
    ((ProjectionOperator *)op)->projExprs = concatTwoLists(((ProjectionOperator *)op)->projExprs, uncertlist);
    appendToTailOfList(((ProjectionOperator *)op)->projExprs, (List *)getMap(hmpIn, (Node *)createAttributeReference(ROW_CERTAIN)));
    setStringProperty(op, UNCERT_MAPPING_PROP, (Node *)hmp);

    LOG_RESULT("Zono: Rewritten Operator tree [PROJECTION]", op);

    return op;
}


static QueryOperator *rewrite_ZonoTableAccess(QueryOperator *op)
{
    INFO_LOG("REWRITE-ZONO - TableAccess");
    DEBUG_LOG("Operator tree \n%s", nodeToString(op));

    // HashMap * hmp = NEW_MAP(Node, Node);
    QueryOperator *proj = (QueryOperator *)createProjectionOp(getNormalAttrProjectionExprs(op), op, NIL, getQueryOperatorAttrNames(op));
    switchSubtrees(op, proj);
    op->parents = singleton(proj);
    // List *attrExpr = getNormalAttrProjectionExprs(op);

    // appendToTailOfList(((ProjectionOperator *)proj)->projExprs, createConstInt(1));
    // setStringProperty(proj, UNCERT_MAPPING_PROP, (Node *)hmp);

    return proj;
}