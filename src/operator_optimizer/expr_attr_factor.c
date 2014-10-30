/*-----------------------------------------------------------------------------
 *
 * expr_attr_factor.c
 *
 *			- Factor out attribute references in expression to simplify expressions and
 *			open up new opportunities for projection merging.
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "operator_optimizer/expr_attr_factor.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"

//static boolean getCaseExprs (Node *node, List **state);

static Node *factorAttrRefs (Node *node);
static Node *removeNonOpCase (Node *node, void *state);
static Node *factorAdd (Node *node, void *state);
static Node *factorMult (Node *node, void *state);
static Node *mergeSameResultCase (Node *node, void *state);
//TODO integrate better with projection merging. More directed application of the transformations
//based on which attribute reference counts should be reduced. Because some
//transformations may decreate some attribute counts at the cost of increasing the count
//for another attribute(s)

/*
 * If a child attribute is mentioned more than once in an expression try to match
 * the expression to a known pattern where we know how to factor out the attribute reference
 * to reduce the number of child references.
 *
 *  PATTERNS:
 *      1) CASE WHEN C THEN f(a) + X ELSE a END
 *          - Conditions: a is not mentioned in C
 *              - X does not contain a
 *          - Rewrite: CASE WHEN C THEN f(a) ELSE a END + CASE WHEN C THEN X ELSE 0 END
 *          - Method: factorAdd
 *      2) CASE WHEN C THEN f(a) * X ELSE a END
 *          - Conditions: a is not mentioned in C,
 *              - X does not contain a
 *          - Rewrite: CASE WHEN C THEN f(a) ELSE a END * CASE WHEN C THEN X ELSE 1 END
 *          - Method: factorMult
 *      3) CASE WHEN C THEN a ELSE a END
 *          - Conditions: none
 *          - Rewrite: a
 *          - Method: removeNonOpCase
 *          - Note: can appear after repeated application of 1) or 2)
 *      4) CASE WHEN C1 THEN f(CASE WHEN C1 THEN X ELSE Y END, Z) ELSE U END
 *          - Conditions:
 *          - Rewrite: CASE WHEN C1 THEN f(X,Z) ELSE U END
 *      5) CASE WHEN C1 THEN X ELSE (CASE WHEN C2 THEN X ELSE Y END) END
 *          - Conditions: X does not mention an attribute that C1 or C2 mentions
 *          - Rewrite: CASE WHEN (C1 OR C2) THEN X ELSE Y END
 *          - Method: mergeSameResultCase
 */
QueryOperator *
projectionFactorAttrReferences(ProjectionOperator *op)
{

    // try to simplify each projection expression individually
    FOREACH(Node,p,op->projExprs)
    {
//        List *caseExprs = NIL;
//        getCaseExprs(p,&caseExprs);
        p_his_cell->data.ptr_value = factorAttrRefs(p);
    }

    return (QueryOperator *) op;
}

static Node *
factorAttrRefs (Node *node)
{
    Node *previous;
    DEBUG_LOG("simplify:\n%s", exprToSQL(node));

    // compute fix-point of simplification rules
    //TODO we may have to explore alternative options
    do {
        previous = node;

        TRACE_LOG("before remove non op case",nodeToString(node));
        node = removeNonOpCase(previous, NULL);
        TRACE_LOG("before factor add",nodeToString(node));
        node = factorAdd(node, NULL);
        TRACE_LOG("before factor mult",nodeToString(node));
        node = factorMult(node, NULL);
        TRACE_LOG("merge same result case",nodeToString(node));
        node = mergeSameResultCase(node, NULL);
    } while(!equal(node,previous));

    DEBUG_LOG("simplified expression is:\n%s", exprToSQL(node));

    return node;
}

static Node *
removeNonOpCase (Node *node, void *state)
{
    if (node == NULL)
        return NULL;

    if (isA(node,CaseExpr))
    {
        CaseExpr *c = (CaseExpr *) node;

        if (LIST_LENGTH(c->whenClauses) == 1)
        {
            CaseWhen *w = (CaseWhen *) getHeadOfListP(c->whenClauses);

            if (equal(w->then,c->elseRes))
                 return c->elseRes;
        }
    }

    return mutate(node, removeNonOpCase, state);
}

static Node *
factorAdd (Node *node, void *state)
{
    if (node == NULL)
        return NULL;

    if (isA(node,CaseExpr))
    {
        CaseExpr *c = (CaseExpr *) node;

        if (LIST_LENGTH(c->whenClauses) == 1)
        {
            CaseWhen *w = (CaseWhen *) getHeadOfListP(c->whenClauses);
            Operator *add;
            AttributeReference *a;
//            Node *opChild;

            // else is attribute reference
            if (isA(c->elseRes,AttributeReference) && isA(w->then,Operator))
            {
                a = (AttributeReference *) c->elseRes;
                add = (Operator *) w->then;

                // operator is + and attribute a is one argument
                if (streq(add->name,"+") && searchListNode(add->args, (Node *) a))
                {
                    int aPos = genericListPos(add->args, equal, a);
                    ASSERT(aPos == 0 || aPos == 1);

                    // copy data structures
                    add = copyObject(add);
                    c = copyObject(c);
                    w = (CaseWhen *) getHeadOfListP(c->whenClauses);

                    // change CASE to CASE WHEN C THEN X ELSE 0 END
                    w->then = (Node *) getNthOfListP(add->args, aPos == 0 ? 1 : 0);
                    c->elseRes = (Node *) createConstInt(0);

                    // new arguments to + are a, CASE WHEN C THEN X ELSE 0 END
                    add->args = LIST_MAKE(copyObject(a), c);

                    DEBUG_LOG("factored add: ", add);

                    return (Node *)  add;
                }
            }
        }
    }

    return mutate(node, factorAdd, state);
}

static Node *
factorMult (Node *node, void *state)
{
    if (node == NULL)
        return NULL;

    if (isA(node,CaseExpr))
    {
        CaseExpr *c = (CaseExpr *) node;

        if (LIST_LENGTH(c->whenClauses) == 1)
        {
            CaseWhen *w = (CaseWhen *) getHeadOfListP(c->whenClauses);
            Operator *mult;
            AttributeReference *a;
//            Node *opChild;

            // else is attribute reference
            if (isA(c->elseRes,AttributeReference) && isA(w->then,Operator))
            {
                a = (AttributeReference *) c->elseRes;
                mult = (Operator *) w->then;

                // operator is + and attribute a is one argument
                if (streq(mult->name,"*") && searchListNode(mult->args, (Node *) a))
                {
                    int aPos = genericListPos(mult->args, equal, a);
                    ASSERT(aPos == 0 || aPos == 1);

                    // copy data structures
                    mult = copyObject(mult);
                    c = copyObject(c);
                    w = (CaseWhen *) getHeadOfListP(c->whenClauses);

                    // change CASE to CASE WHEN C THEN X ELSE 0 END
                    w->then = (Node *) getNthOfListP(mult->args, aPos == 0 ? 1 : 0);
                    c->elseRes = (Node *) createConstInt(0);

                    // new arguments to + are a, CASE WHEN C THEN X ELSE 0 END
                    mult->args = LIST_MAKE(copyObject(a), c);

                    DEBUG_LOG("factored add: ", mult);

                    return (Node *)  mult;
                }
            }
        }
    }

    return mutate(node, factorMult, state);
}

static Node *
mergeSameResultCase (Node *node, void *state)
{
    if (node == NULL)
        return NULL;

    if (isA(node,CaseExpr))
    {
        CaseExpr *c = (CaseExpr *) node;
        CaseExpr *child;

        if (LIST_LENGTH(c->whenClauses) == 1)
        {
            CaseWhen *w = (CaseWhen *) getHeadOfListP(c->whenClauses);

            if (isA(c->elseRes,CaseExpr))
            {
                child = (CaseExpr *) c->elseRes;
                CaseWhen *childW = (CaseWhen *) getHeadOfListP(child->whenClauses);

                if (LIST_LENGTH(child->whenClauses) == 1 && equal(childW->then, w->then))
                {
                    c = copyObject(c);
                    w = (CaseWhen *) getHeadOfListP(c->whenClauses);

                    w->when = OR_EXPRS(w->when,copyObject(childW->when));
                    c->elseRes = copyObject(child->elseRes);

                    return (Node *) c;
                }
            }
        }
    }

    return mutate(node, mergeSameResultCase, state);
}

// need to capture pointers for replacement too?
//static boolean
//getCaseExprs (Node *node, List **state)
//{
//    if (node == NULL)
//        return TRUE;
//
//    if (isA(node, CaseExpr))
//        *state = appendToTailOfList(*state, node);
//
//    return visit(node, getCaseExprs, state);
//}
