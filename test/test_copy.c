/*-----------------------------------------------------------------------------
 *
 * test_copy.c
 *
 *
 *		AUTHOR: Bowen
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include <string.h>

#include "model/list/list.h"
#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

/* internal tests */
//static rc testcopyAttributeReference (void);
static rc testshallowCopyList (void);
static rc testdeepCopyList (void);
static rc testCopyOpGraph (void);

/* check equal model */
rc
testCopy (void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testshallowCopyList(), "test shallow copy for List");
    RUN_TEST(testdeepCopyList(), "test deep copy for List");
    RUN_TEST(testCopyOpGraph(), "test deep copy for operator graphs");

    return PASS;
}



static rc
testshallowCopyList(void)
{
    List *fromList = NIL, *newList = NIL;

    // check shallow integer list copy
    fromList = appendToTailOfListInt(fromList, 1);
    fromList = appendToTailOfListInt(fromList, 2);
    fromList = appendToTailOfListInt(fromList, 3);

    newList = copyList(fromList);
    ASSERT_EQUALS_INT(getListLength(fromList),getListLength(newList),"Lists have the same length");
    FORBOTH_INT(l,r,fromList,newList)
    ASSERT_EQUALS_INT(l,r,"compare list element");
    FORBOTH_LC(l,r,fromList,newList)
    ASSERT_FALSE(l == r, "list cells are actually new");
    FREE(fromList);
    FREE(newList);

    return PASS;
}

static rc
testdeepCopyList(void)
{
    List *fromList = NIL, *toList = NIL;
    char *a = "a";
    char *b = "b";
    AttributeReference *a1, *a2;

    a1 = createAttributeReference(strdup(a));
	a2 = createAttributeReference(strdup(b));

	fromList = appendToTailOfList(fromList, a1);
	fromList = appendToTailOfList(fromList, a2);
	toList = copyList(fromList);
	ASSERT_EQUALS_NODE(fromList,toList,"Lists have the same node type");
	ASSERT_EQUALS_STRING(nodeToString(fromList),nodeToString(toList), "both lists are equal");
	FREE(fromList);
	FREE(toList);

	return PASS;
}

static rc
testCopyOpGraph (void)
{
    QueryOperator *par, *c1, *c2,
                  *copyP, *copyC1, *copyC2;

    c1 = (QueryOperator *) createTableAccessOp("R", NULL, NULL, NIL, LIST_MAKE("a", "b"), NIL);
    c2 = c1;
    par = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(c1,c2), NIL, LIST_MAKE("a", "b"));
    c1->parents = singleton(par);

    copyP = copyObject(par);
    copyC1 = OP_LCHILD(copyP);
    copyC2 = OP_RCHILD(copyP);

    DEBUG_LOG("%s\ncopied to overview\n%s",
            operatorToOverviewString((Node *) par),
            operatorToOverviewString((Node *) copyP));

    ASSERT_EQUALS_P(copyC1, copyC2, "graph structure preserved in result of copy");

    return PASS;
}


