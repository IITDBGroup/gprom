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
#include "common.h"

#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "utility/string_utils.h"

static rc testIntList(void);
static rc testPList(void);
static rc testPopList(void);
static rc testListConstruction(void);
static rc testListOperations(void);
static rc testListGenericOps(void);
static rc testSort(void);
static rc testUnique(void);
static rc testRemove(void);

static boolean eqConstFirst (void *a, void *b);
static int cmpConstFirst (const void **a, const void **b);

rc
testList()
{
    RUN_TEST(testIntList(), "test integer lists");
    RUN_TEST(testPList(), "test pointer lists");
    RUN_TEST(testListConstruction(), "test list construction");
    RUN_TEST(testListOperations(), "test list operations");
	RUN_TEST(testPopList(), "test pop from list");
    RUN_TEST(testListGenericOps(), "test generic list operations");
	RUN_TEST(testSort(), "test generic list sorting");
	RUN_TEST(testUnique(), "test duplicate elimination");
	RUN_TEST(testRemove(), "test removing list elements");

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
    	DEBUG_LOG("reached %u", i);
        ASSERT_TRUE(FALSE,"should never reach here for empty list");
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
    List *l1,*l2, *l, *lCon;
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

    // concat multiple lists
    l1 = LIST_MAKE(a1,a2);
    l2 = LIST_MAKE(a1,a2);
    l = LIST_MAKE(a1,a2);

    lCon = CONCAT_LISTS(l1,l2,l);
    ASSERT_EQUALS_NODE(LIST_MAKE(a1,a2,a1,a2,a1,a2), lCon, "concat list is a1,a2,a1,a2,a1,a2");

    // sort list
    l = LIST_MAKE("c","b","a");
    l = sortList(l, strCompare);

    ASSERT_EQUALS_STRING(stringListToString(LIST_MAKE("a","b","c")), stringListToString(l), "sort lists");

    // sublist
    l = LIST_MAKE("a","b","c");
    l2 = sublist(l, 0,0);
    ASSERT_EQUALS_STRING(stringListToString(LIST_MAKE("a")), stringListToString(l2), "sublist 0 to 0");
    ASSERT_EQUALS_INT(1, LIST_LENGTH(l2), "... of length 1");

    l = LIST_MAKE("a","b","c");
    l2 = sublist(l, 2,2);
    ASSERT_EQUALS_STRING(stringListToString(LIST_MAKE("c")), stringListToString(l2), "sublist 2 to 2");
    ASSERT_EQUALS_INT(1, LIST_LENGTH(l2), "... of length 1");

    // reverse list
	l = LIST_MAKE("a","b","c");
	reverseList(l);
	l2 = LIST_MAKE("c","b","a");

	ASSERT_EQUALS_STRING(stringListToString(l2),
						 stringListToString(l),
						 "reverse list");

    // search list
    CREATE_INT_SEQ(l,0,5,1);
    ASSERT_TRUE(searchListInt(l,0), "0 in (0,1,2,3,4,5)");
    ASSERT_TRUE(searchListInt(l,5), "5 in (0,1,2,3,4,5)");
    ASSERT_TRUE(searchListInt(l,2), "2 in (0,1,2,3,4,5)");
    ASSERT_FALSE(searchListInt(l,6), "6 not in (0,1,2,3,4,5)");

    l = LIST_MAKE("a","b","c");
    ASSERT_TRUE(searchListString(l,"a"), "a in (a,b,c)");
    ASSERT_TRUE(searchListString(l,"c"), "c in (a,b,c)");
    ASSERT_FALSE(searchListString(l,"d"), "d not in (a,b,c)");

    return PASS;
}

static rc
testPopList(void)
{
	List  *l = LIST_MAKE("a","b","c");
	List *l2 = LIST_MAKE("c");

	ASSERT_EQUALS_STRING("a", popHeadOfListP(l), "pop a");
	ASSERT_EQUALS_STRING("b", popHeadOfListP(l), "pop b");

	ASSERT_EQUALS_STRING(stringListToString(l2), stringListToString(l), "after poping two elements = (c)");

	ASSERT_EQUALS_STRING("c", popHeadOfListP(l), "pop c");
	ASSERT_TRUE(MY_LIST_EMPTY(l), "is empty");

	return PASS;
}

static rc
testListGenericOps(void)
{
    Constant *c1 = createConstString("abc");
    Constant *c2 = createConstString("aaa");
    Constant *c3 = createConstString("bcd");

    List *l = LIST_MAKE(c1,c2,c3);

    // search
    ASSERT_TRUE(genericSearchList(l, eqConstFirst, createConstString("bbb")), "has string starting with b");
    ASSERT_FALSE(genericSearchList(l, eqConstFirst, createConstString("cbc")), "doe not have string starting with c");

    // remove
    l = genericRemoveFromList(l, eqConstFirst, createConstString("axxxx"));
    ASSERT_EQUALS_NODE(l, LIST_MAKE(c3), "after removal list is: (\"bcd\")");
    return PASS;
}

static rc
testSort(void)
{
	Constant *c1 = createConstString("abc");
	Constant *c2 = createConstString("caa");
	Constant *c3 = createConstString("bbb");

	List *l = LIST_MAKE(c1,c2,c3);
	List *exp = LIST_MAKE(c1,c3,c2);
	List *actual = sortList(l, cmpConstFirst);

	ASSERT_EQUALS_NODE(exp, actual, "sorted is [abc, bbb, caa]");

	return PASS;
}

static rc
testUnique(void)
{
	Constant *c1 = createConstString("abc");
	Constant *c2 = createConstString("aaa");
	Constant *c3 = createConstString("bbb");

	List *l = LIST_MAKE(c1,c2,c3);

	// remove dups based on first character equality
	List *exp = LIST_MAKE(c1,c3);
	List *actual = unique(l, cmpConstFirst);

	ASSERT_EQUALS_NODE(exp, actual, "unique removes one element starting with a.");

	return PASS;
}

#define CS(s) createConstString(s)

static rc
testRemove(void)
{
	List *l = LIST_MAKE_INT(1,2,3,5,6);

	l = removeFromListInt(l, 3);
	l = removeFromListInt(l, 7);
	l = removeFromListInt(l, 4);

	ASSERT_EQUALS_NODE(LIST_MAKE_INT(1,2,5,6), l, "(1,2,5,6)");

	l = LIST_MAKE(CS("a"), CS("b"), CS("c"));

	l = REMOVE_FROM_LIST_NODE(l, CS("b"));
	l = REMOVE_FROM_LIST_NODE(l, CS("d"));

	ASSERT_EQUALS_NODE(LIST_MAKE(CS("a"), CS("c")), l, "(a,c)");

	return PASS;
}


static boolean
eqConstFirst (void *a, void *b)
{
    char *c1 = STRING_VALUE(a);
    char *c2 = STRING_VALUE(b);

	return c2[0] == c1[0];
}

static int
cmpConstFirst (const void **a, const void **b)
{
    char *c1;
	char *c2;

	if(*a == *b)
		return 0;
	if(*a == NULL || *b == NULL)
		return -1;

    c1 = STRING_VALUE(*a);
    c2 = STRING_VALUE(*b);

	return c1[0] - c2[0];
}
