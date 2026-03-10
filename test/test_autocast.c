/*-----------------------------------------------------------------------------
 *
 * test_autocast.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "model/list/list.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_dt_inference.h"
#include "provenance_rewriter/prov_utility.h"

/* internal tests */
static rc testLcaType(void);
static rc testCastExprs(void);
static rc testCastSetOp(void);

#define SIN(_dt) singletonInt(_dt)

/* check equal model */
rc
testAutocast(void)
{
	RUN_TEST(testLcaType(), "test inferring the lowest comman ancestor type of two types");
    RUN_TEST(testCastExprs(), "test casting in expressions of algebra operators");
    RUN_TEST(testCastSetOp(), "test making inputs of set operators union compatible");

    return PASS;
}


static rc
testLcaType(void)
{
	DataType types[] = { DT_INT, DT_FLOAT, DT_BOOL, DT_LONG, DT_STRING };

	for(int i = 0; i < 5; i++)
	{
		ASSERT_EQUALS_INT(lcaType(types[i], types[i]), types[i], "reflexivity");
	}

	// string is the most general
	for(int i = 0; i < 5; i++)
	{
		ASSERT_EQUALS_INT(lcaType(types[i], DT_STRING), DT_STRING, "string is top type");
	}

	ASSERT_EQUALS_INT(lcaType(DT_FLOAT, DT_INT), DT_FLOAT, "int,float -> float");
	ASSERT_EQUALS_INT(lcaType(DT_INT, DT_FLOAT), DT_FLOAT, "float,int -> float");

	return PASS;
}

static rc
testCastExprs(void)
{
    QueryOperator *p, *eP;
    QueryOperator *t;
    QueryOperator *s, *eS;
    QueryOperator *result;
    List *projExpr;
    ListCell *lc;

    // input tree
    t = (QueryOperator *) createTableAccessOp(strdup("R"), NULL, strdup("R"), NIL,
             LIST_MAKE(strdup("A"), strdup("B")), CONCAT_LISTS(SIN(DT_INT), SIN(DT_FLOAT)));

    p = (QueryOperator *) createProjOnAllAttrs(t);
    addChildOperator(p, t);
    projExpr = ((ProjectionOperator *) p)->projExprs;
    lc = projExpr->head;
    LC_P_VAL(lc) = createOpExpr(strdup("+"),
            LIST_MAKE(createFullAttrReference(strdup("A"), 0, 0, 0, DT_INT),
                    createFullAttrReference(strdup("B"), 0, 1, 0, DT_FLOAT)
            ));

    s = (QueryOperator *) createSelectionOp((Node *) createConstBool(TRUE), p, NIL, LIST_MAKE(strdup("A"), strdup("B")));

    // cast
    result = copyObject(s);
    introduceCastsWhereNecessary((QueryOperator *) result);

    // expected
    eS = copyObject(s);
    eP = OP_LCHILD(eS);
    projExpr = ((ProjectionOperator *) eP)->projExprs;
    lc = projExpr->head;
    LC_P_VAL(lc) = createOpExpr(strdup("+"),
            LIST_MAKE(createCastExpr((Node *) createFullAttrReference(strdup("A"), 0, 0, 0, DT_INT), DT_FLOAT),
                    createFullAttrReference(strdup("B"), 0, 1, 0, DT_FLOAT)
            ));
    AttributeDef *a1 = (AttributeDef *) LC_P_VAL(eP->schema->attrDefs->head);
    a1->dataType = DT_FLOAT;
    a1 = (AttributeDef *) LC_P_VAL(eS->schema->attrDefs->head);
    a1->dataType = DT_FLOAT;

    DEBUG_NODE_BEATIFY_LOG("expected: %s", eS);
    DEBUG_NODE_BEATIFY_LOG("result: %s", result);
    DEBUG_NODE_BEATIFY_LOG("after casting", result);

    ASSERT_EQUALS_NODE(eS, result, "casting in algebra operator expressions");

    return PASS;
}



static rc
testCastSetOp (void)
{
    TableAccessOperator *t1, *t2;
    QueryOperator *p1, *p2;
    SetOperator *u;
    QueryOperator *result, *expected;
	ListCell *lc;

    // input tree
    t1 = createTableAccessOp("R", NULL, "R", NIL,
            LIST_MAKE(strdup("A"), strdup("B")), CONCAT_LISTS(SIN(DT_INT), SIN(DT_FLOAT)));

    t2 = createTableAccessOp("R", NULL, "R", NIL,
                LIST_MAKE(strdup("C"), strdup("D")), CONCAT_LISTS(SIN(DT_STRING), SIN(DT_INT)));

    u = createSetOperator(SETOP_UNION, LIST_MAKE(t1,t2), NIL, LIST_MAKE(strdup("A"), strdup("B")));

    // cast
    result = copyObject(u);
    introduceCastsWhereNecessary((QueryOperator *) result);

    // expected result
    p1 = createProjOnAllAttrs((QueryOperator *) t1);
    lc = ((ProjectionOperator *) p1)->projExprs->head;
    LC_P_VAL(lc) = createCastExpr((Node *) createFullAttrReference(strdup("A"), 0, 0, 0, DT_INT), DT_STRING);
	((AttributeDef *) p1->schema->attrDefs->head->data.ptr_value)->dataType = DT_STRING;
    addChildOperator(p1,copyObject(t1));

    p2 = createProjOnAllAttrs((QueryOperator *) t2);
    lc = ((ProjectionOperator *) p2)->projExprs->head->next;
    LC_P_VAL(lc) = createCastExpr((Node *) createFullAttrReference(strdup("D"), 0, 1, 0, DT_INT), DT_FLOAT);
	((AttributeDef *) p2->schema->attrDefs->head->next->data.ptr_value)->dataType = DT_FLOAT;
    addChildOperator(p2,copyObject(t2));

    expected = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(p1,p2), NIL, LIST_MAKE(strdup("A"), strdup("B")));
	addParent(p1, expected);
	addParent(p2, expected);

    INFO_OP_LOG("casting result: ", result);
    INFO_OP_LOG("expected result: ", expected);
    ASSERT_EQUALS_NODE(expected, result, "making inputs union compatible");

    return PASS;
}
