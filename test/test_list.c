/*-----------------------------------------------------------------------------
 *
 * test_list.c
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
#include "model/node/nodetype.h"
#include "model/expression/expression.h"

static rc testIntList(void);
static rc testPList(void);
static rc testListConstruction(void);
static rc testListOperations(void);

rc
testList()
{
    RUN_TEST(testIntList(), "test integer lists");
    RUN_TEST(testPList(), "test pointer lists");
    RUN_TEST(testListConstruction(), "test list construction");
    RUN_TEST(testListOperations(), "test list operations");

    return PASS;
}


static rc
testIntList(void)
{
    List *newList = NIL;

    ASSERT_EQUALS_INT(0, getListLength(newList), "NULL list has length 0");
    ASSERT_EQUALS_P(NULL, getHeadOfList(newList), "NULL list's head is NULL");

    FOREACH_INT(i, newList)
    {
        ASSERT_EQUALS_INT(0,1,"should never reach here for empty list");
    }

    newList = appendToTailOfListInt(newList, 1);
    ASSERT_EQUALS_INT(1, getListLength(newList), "(1) list has length 1");
    ASSERT_EQUALS_INT(1, getNthOfList(newList, 0)->data.int_value, "first element is 1");

    FOREACH_INT(i, newList)
    {
        ASSERT_EQUALS_INT(1, i, "first element is 1");
    }



    return PASS;
}

static rc
testPList(void)
{
	List *newList = NIL;

	char *a = "a";
	newList = appendToTailOfList(newList, a);
	ASSERT_EQUALS_P(a, getNthOfListP(newList,0), "Pointer a has added");
	char *b = "b";
	newList = appendToTailOfList(newList, b);
	ASSERT_EQUALS_P(b, getNthOfListP(newList,1), "Pointer b has added");

    return PASS;
}

static rc
testListConstruction(void)
{
    char *a = "a";
    char *b = "b";
    char *c = "c";
    List *newList = NIL;

    newList = LIST_MAKE(a,b,c);
    ASSERT_EQUALS_INT(3, LIST_LENGTH(newList), "list length is 3");
    ASSERT_EQUALS_P(a, getNthOfListP(newList,0), "1) element is a");
    ASSERT_EQUALS_P(b, getNthOfListP(newList,1), "2) element is b");
    ASSERT_EQUALS_P(c, getNthOfListP(newList,2), "3) element is c");

    newList = singleton(a);
    ASSERT_EQUALS_INT(1, LIST_LENGTH(newList), "list length is 1");
    ASSERT_EQUALS_P(a, getNthOfListP(newList,0), "1) element is a");
    return PASS;
}

static rc
testListOperations(void)
{
    char *a = "a";
    char *b = "b";
    List *l1,*l2, *l;
    AttributeReference *a1, *a2;

    // concat
    l1 = singleton(a);
    l2 = singleton(b);
    l = concatTwoLists(l1,l2);
    ASSERT_EQUALS_INT(2, LIST_LENGTH(l), "list length is 2");
    ASSERT_EQUALS_P(a, getNthOfListP(l,0), "1) element is a");
    ASSERT_EQUALS_P(b, getNthOfListP(l,1), "2) element is b");

    // equal
    a1 = createAttributeReference(strdup(a));
    a2 = createAttributeReference(strdup(b));
    l1 = LIST_MAKE(a1, a2);
    l2 = LIST_MAKE(copyObject(a1), copyObject(a2));
    ASSERT_EQUALS_NODE(a1, getNthOfListP(l1,0), "1) is a1");
    ASSERT_EQUALS_NODE(a2, getNthOfListP(l1,1), "2) is a2");
    ASSERT_EQUALS_STRING(nodeToString(l1),nodeToString(l2), "both lists are equal");//TODO core dump here
    ASSERT_EQUALS_NODE(l1,l2, "both lists are equal");

    // shallow free
    l1 = LIST_MAKE(a1,a2);
    freeList(l1);
    ASSERT_TRUE(-1 == a1->fromClauseItem, "access should not be problematic");

    return PASS;
}
