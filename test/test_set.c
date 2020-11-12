/*-----------------------------------------------------------------------------
 *
 * test_set.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"

#include "model/set/set.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"

static rc testIntSet(void);
static rc testPSet(void);
static rc testStringSet(void);
static rc testNodeSet(void);
static rc testSetIteration(void);
static rc testSetEquals(void);
static rc testSetOperations(void);

rc
testSet()
{
    RUN_TEST(testIntSet(), "test integer sets");
    RUN_TEST(testPSet(), "test pointer sets");
    RUN_TEST(testStringSet(), "test string sets");
    RUN_TEST(testNodeSet(), "test node set");
    RUN_TEST(testSetIteration(), "test set iteration");
    RUN_TEST(testSetEquals(), "test equal function on sets");
    RUN_TEST(testSetOperations(), "test set operations");

    return PASS;
}

static rc
testIntSet(void)
{
    Set *a = INTSET();
    Set *b = MAKE_INT_SET(1,2,3,4,6);

    ASSERT_EQUALS_INT(0, setSize(a), "empty set");

    ASSERT_TRUE(addIntToSet(a,1), "element 1 is new");
    ASSERT_TRUE(addIntToSet(a,2), "element 2 is new");
    ASSERT_TRUE(hasSetIntElem(a,1), "set contains 1");
    ASSERT_TRUE(hasSetIntElem(a,2), "set contains 2");

    ASSERT_FALSE(addIntToSet(a,2), "element 2 is already in set");
    ASSERT_EQUALS_INT(2, setSize(a), "2 element set");

    ASSERT_FALSE(hasSetIntElem(a,3), "set does not contain 3");

    removeSetIntElem(a,2);
    ASSERT_FALSE(hasSetIntElem(a,2), "set does not contain 2 anymore");

    ASSERT_TRUE(hasSetIntElem(b,1), "set contains 1");
    ASSERT_TRUE(hasSetIntElem(b,2), "set contains 2");
    ASSERT_TRUE(hasSetIntElem(b,3), "set contains 3");
    ASSERT_TRUE(hasSetIntElem(b,4), "set contains 4");
    ASSERT_TRUE(hasSetIntElem(b,6), "set contains 6");
    ASSERT_EQUALS_INT(5, setSize(b), "5 element set");

    return PASS;
}

static rc
testPSet(void)
{
    char *a1 = NEW(char);
    *a1 = 'a';
    char *a2 = NEW(char);
    *a2 = 'b';
    char *a3 = NEW(char);
    *a3 = 'b';
    char *t1 = NEW(char);
    *t1 = 'a';
    char *t2 = NEW(char);
    *t1 = 'b';

    Set *a = MAKE_SET_PTR(a1, a2);
    Set *b = MAKE_SET_PTR(a2, a3);

    ASSERT_EQUALS_INT(2, setSize(a), "set a is of size 2");
    ASSERT_EQUALS_INT(2, setSize(b), "set b is of size 2");

    ASSERT_TRUE(hasSetElem(a,a1), "same pointer a1");
    ASSERT_TRUE(hasSetElem(a,a2), "same pointer a2");
    ASSERT_FALSE(hasSetElem(a,t1), "same content, but different pointer");

    ASSERT_TRUE(hasSetElem(b,a2), "same pointer a2");
    ASSERT_TRUE(hasSetElem(b,a3), "same pointer a3");
    ASSERT_FALSE(hasSetElem(b,t2), "same content, but different pointer");

    removeSetElem(b, a2);
    ASSERT_EQUALS_INT(1, setSize(b), "set b is of size 1");
    ASSERT_FALSE(hasSetElem(b,a2), "a2 no longer in set");
    ASSERT_TRUE(hasSetElem(a,a2), "a2 still has valid a2");

    return PASS;
}

static rc
testStringSet(void)
{
    Set *a = MAKE_STR_SET(strdup("abc"), strdup("a"));
    Set *b = MAKE_STR_SET(strdup("ab"), strdup("bc"));

    ASSERT_EQUALS_INT(2, setSize(a), "set a is of size 2");
    ASSERT_EQUALS_INT(2, setSize(b), "set b is of size 2");

    ASSERT_TRUE(hasSetElem(a, "abc"), "set a has abc");
    ASSERT_TRUE(hasSetElem(a, "a"), "set a has a");
    ASSERT_FALSE(hasSetElem(a, "ab"), "set a has not ab");

    ASSERT_TRUE(hasSetElem(b, "ab"), "set a has ab");
    ASSERT_TRUE(hasSetElem(b, "bc"), "set a has bc");
    ASSERT_FALSE(hasSetElem(b, "abc"), "set b has not abc");

    removeSetElem(b, "ab");
    ASSERT_EQUALS_INT(1, setSize(b), "set b is of size 1");
    ASSERT_FALSE(hasSetElem(b, "ab"), "set b has not ab");

    return PASS;
}

static rc
testNodeSet(void)
{
    AttributeReference *a1 = createAttributeReference ("a1");
    AttributeReference *a2 = createAttributeReference ("a2");
    AttributeReference *a2a = createAttributeReference ("a2");
    AttributeReference *a3 = createAttributeReference ("a3");
    Set *a = MAKE_NODE_SET(a1, a2);
    Set *b = MAKE_NODE_SET(a2a, a3);

    ASSERT_EQUALS_INT(2, setSize(a), "set a is of size 2");
    ASSERT_EQUALS_INT(2, setSize(b), "set b is of size 2");

    ASSERT_TRUE(hasSetElem(a, a1), "set a has a1");
    ASSERT_TRUE(hasSetElem(a, a2), "set a has a2");
    ASSERT_TRUE(hasSetElem(a, a2a), "set a has a2a");
    ASSERT_FALSE(hasSetElem(a, a3), "set a not has a3");

    removeSetElem(a, a2);
    ASSERT_EQUALS_INT(1, setSize(a), "set a is of size 1");
    ASSERT_FALSE(hasSetElem(a, a2), "set a has not a2");

    ASSERT_FALSE(hasSetElem(b, a1), "set b has not a1");
    ASSERT_TRUE(hasSetElem(b, a2), "set b has a2");
    ASSERT_TRUE(hasSetElem(b, a2a), "set b has a2a");
    ASSERT_TRUE(hasSetElem(b, a3), "set b not has a3");

    return PASS;
}

static rc
testSetIteration(void)
{
    Set *a; //, *b;
    int exp[3] = {1,2,3};
//    int pos;

    a = MAKE_INT_SET(1,2,3);
//    pos = 0;
    FOREACH_SET(int,i,a)
    {
        boolean found = FALSE;
        for(int j = 0; j < 3; j++)
            if (exp[j] == *i)
                found = TRUE;
        ASSERT_TRUE(found, "found element");
    }

    return PASS;
}

static rc
testSetEquals(void)
{
    Set *a, *b;
    char *c1, *c2, *c3;

    a = MAKE_INT_SET(1,2,3);
    b = MAKE_INT_SET(1,2,3);
    ASSERT_TRUE(equal(a,b), "same int sets");
    addIntToSet(a,4);
    ASSERT_FALSE(equal(a,b), "not same int sets");

    a = MAKE_STR_SET(strdup("a"), strdup("bc"));
    b = MAKE_STR_SET(strdup("a"), strdup("bc"));
    ASSERT_TRUE(equal(a,b), "same string sets");
    addToSet(a, "123");
    ASSERT_FALSE(equal(a,b), "not same string sets");

    c1 = NEW(char);
    *c1 = 'a';
    c2 = NEW(char);
    *c2 = 'b';
    c3 = NEW(char);
    *c3 = 'b';
    a = MAKE_SET_PTR(c1,c2,c3);
    b = MAKE_SET_PTR(c1,c2,c3);
    ASSERT_TRUE(equal(a,b), "same pointer sets");
    removeSetElem(a, c3);
    ASSERT_FALSE(equal(a,b), "not same pointer sets");

    return PASS;
}

static rc
testSetOperations(void)
{
    Set *a;
    Set *b;
    Set *un;
    Set *in;
    Set *result;

    // string sets
    a = MAKE_STR_SET(strdup("a"), strdup("b"));
    b = MAKE_STR_SET(strdup("b"), strdup("c"));
    un = MAKE_STR_SET(strdup("a"), strdup("b"), strdup("c"));
    in = MAKE_STR_SET(strdup("b"));

    result = unionSets(a,b);
    ASSERT_EQUALS_NODE(un,result, "union set if {a,b,c}");

    result = intersectSets(a,b);
    ASSERT_EQUALS_NODE(in, result, "intersected set if {b}");

    // int sets
    a = MAKE_INT_SET(1,2,3,4);
    b = MAKE_INT_SET(3,4,5,6);
    un = MAKE_INT_SET(1,2,3,4,5,6);
    in = MAKE_INT_SET(3,4);

    INFO_LOG("created sets");

    result = unionSets(a,b);
    ASSERT_EQUALS_NODE(un, result, "union set if {1,2,3,4,5,6}");

    result = intersectSets(a,b);
    ASSERT_EQUALS_NODE(in, result, "intersected set if {3,4}");

    return PASS;
}

