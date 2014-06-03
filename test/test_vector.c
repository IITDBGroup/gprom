/*-----------------------------------------------------------------------------
 *
 * test_vector.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */


#include "test_main.h"

#include "model/node/nodetype.h"
#include "model/set/vector.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

static rc testIntVector(void);
static rc testNodeVector(void);
//static rc testStringVector(void);
//static rc testVectorIteration(void);
//static rc testVectorEquals(void);
//static rc testVectorOperations(void);

rc
testVector()
{
    RUN_TEST(testIntVector(), "test integer vectors");
    RUN_TEST(testNodeVector(), "test node vectors");
////    RUN_TEST(testStringVector(), "test node vectors");
//    RUN_TEST(testVectorIteration(), "test set iteration");
//    RUN_TEST(testVectorEquals(), "test equal function on sets");
//    RUN_TEST(testVectorOperations(), "test set operations");

    return PASS;
}

static rc
testIntVector(void)
{
    Vector *a = MAKE_VEC_INT(1,2,3);
    Vector *b = MAKE_VEC_INT(1,2,3,1,2,3);

    ASSERT_EQUALS_INT(3, VEC_LENGTH(a), "vector size = 3");
    ASSERT_EQUALS_INT(6, VEC_LENGTH(b), "vector size = 6");

    ASSERT_EQUALS_INT(1, getVecInt(a,0), "vector a[0] = 1");
    ASSERT_EQUALS_INT(2, getVecInt(a,1), "vector a[1] = 2");
    ASSERT_EQUALS_INT(3, getVecInt(a,2), "vector a[2] = 3");

    ASSERT_EQUALS_INT(1, getVecInt(b,0), "vector b[0] = 1");
    ASSERT_EQUALS_INT(2, getVecInt(b,1), "vector b[1] = 2");
    ASSERT_EQUALS_INT(3, getVecInt(b,2), "vector b[2] = 3");
    ASSERT_EQUALS_INT(1, getVecInt(b,3), "vector b[3] = 1");
    ASSERT_EQUALS_INT(2, getVecInt(b,4), "vector b[4] = 2");
    ASSERT_EQUALS_INT(3, getVecInt(b,5), "vector b[5] = 3");

    vecAppendInt(a,1);
    vecAppendInt(a,2);
    vecAppendInt(a,3);

    ASSERT_EQUALS_INT(6, VEC_LENGTH(a), "6 element vector");

    ASSERT_EQUALS_INT(1, getVecInt(a,3), "vector a[3] = 1");
    ASSERT_EQUALS_INT(2, getVecInt(a,4), "vector a[4] = 2");
    ASSERT_EQUALS_INT(3, getVecInt(a,5), "vector a[5] = 3");

    // search elements
    ASSERT_TRUE(findVecInt(a,1), "a contains 1");
    ASSERT_TRUE(findVecInt(a,3), "a contains 2");
    ASSERT_TRUE(findVecInt(b,1), "b contains 1");
    ASSERT_TRUE(findVecInt(b,3), "b contains 2");

    ASSERT_FALSE(findVecInt(b,4), "b does not contains 4");

    // pop element
    ASSERT_EQUALS_INT(3, popVecInt(a), "pop a = 3");
    ASSERT_EQUALS_INT(2, popVecInt(a), "pop a = 2");
    ASSERT_EQUALS_INT(1, popVecInt(a), "pop a = 1");

    ASSERT_EQUALS_INT(3, VEC_LENGTH(a), "3 element vector");

    return PASS;
}


static rc
testNodeVector(void)
{
    AttributeReference *a1 = createAttributeReference ("a1");
    AttributeReference *a2 = createAttributeReference ("a2");
    AttributeReference *a2a = createAttributeReference ("a2");
    AttributeReference *a3 = createAttributeReference ("a3");
    Vector *a = MAKE_VEC_NODE(AttributeReference, a1, a2, a2);
    Vector *b = MAKE_VEC_NODE(AttributeReference, a2a, a3);

    ASSERT_EQUALS_INT(3, VEC_LENGTH(a), "vector a is of size 3");
    ASSERT_EQUALS_INT(2, VEC_LENGTH(b), "vector b is of size 2");

    ASSERT_EQUALS_NODE(a1, getVecNode(a,0), "vector a[0] = a1");
    ASSERT_EQUALS_NODE(a2, getVecNode(a,1), "vector a[1] = a2");
    ASSERT_EQUALS_NODE(a2, getVecNode(a,2), "vector a[0] = a2");

    VEC_ADD_NODE(b,(Node *) a1);
    ASSERT_EQUALS_INT(3, VEC_LENGTH(b), "vector b is of size 3");

    // find elements
    ASSERT_TRUE(findVecNode(a,(Node *) a1), "a contains a1");
    ASSERT_TRUE(findVecNode(a,(Node *) a2), "a contains a2");
    ASSERT_TRUE(findVecNode(b,(Node *) a2), "b contains a2a aka a2");
    ASSERT_TRUE(findVecNode(b,(Node *) a3), "b contains a3");

    ASSERT_FALSE(findVecNode(a,(Node *) a3), "a does not contains a3");

    // pop element
    ASSERT_EQUALS_NODE(a2, popVecNode(a), "pop a = a2");
    ASSERT_EQUALS_NODE(a2, popVecNode(a), "pop a = a2");
    ASSERT_EQUALS_NODE(a1, popVecNode(a), "pop a = a1");

    ASSERT_EQUALS_INT(0, VEC_LENGTH(a), "0 element vector");

    return PASS;
}
//
//static rc
//testStringVector(void)
//{
//    Vector *a = MAKE_STR_SET(strdup("abc"), strdup("a"));
//    Vector *b = MAKE_STR_SET(strdup("ab"), strdup("bc"));
//
////    ASSERT_EQUALS_INT(2, setSize(a), "set a is of size 2");
////    ASSERT_EQUALS_INT(2, setSize(b), "set b is of size 2");
////
////    ASSERT_TRUE(hasVectorElem(a, "abc"), "set a has abc");
////    ASSERT_TRUE(hasVectorElem(a, "a"), "set a has a");
////    ASSERT_FALSE(hasVectorElem(a, "ab"), "set a has not ab");
////
////    ASSERT_TRUE(hasVectorElem(b, "ab"), "set a has ab");
////    ASSERT_TRUE(hasVectorElem(b, "bc"), "set a has bc");
////    ASSERT_FALSE(hasVectorElem(b, "abc"), "set b has not abc");
////
////    removeVectorElem(b, "ab");
////    ASSERT_EQUALS_INT(1, setSize(b), "set b is of size 1");
////    ASSERT_FALSE(hasVectorElem(b, "ab"), "set b has not ab");
//
//    return PASS;
//}
//
//
//static rc
//testVectorIteration(void)
//{
//    Vector *a, *b;
//    int exp[3] = {1,2,3};
//    int pos;
//
//    a = MAKE_INT_SET(1,2,3);
//    pos = 0;
////    FOREACH_SET(int,i,a)
////    {
////        boolean found = FALSE;
////        for(int j = 0; j < 3; j++)
////            if (exp[j] == *i)
////                found = TRUE;
////        ASSERT_TRUE(found, "found element");
////    }
//
//    return PASS;
//}
//
//static rc
//testVectorEquals(void)
//{
//    Vector *a, *b;
//    char *c1, *c2, *c3;
//
////    a = MAKE_INT_SET(1,2,3);
////    b = MAKE_INT_SET(1,2,3);
////    ASSERT_TRUE(equal(a,b), "same int sets");
////    addIntToVector(a,4);
////    ASSERT_FALSE(equal(a,b), "not same int sets");
////
////    a = MAKE_STR_SET(strdup("a"), strdup("bc"));
////    b = MAKE_STR_SET(strdup("a"), strdup("bc"));
////    ASSERT_TRUE(equal(a,b), "same string sets");
////    addToVector(a, "123");
////    ASSERT_FALSE(equal(a,b), "not same string sets");
////
////    c1 = NEW(char);
////    *c1 = 'a';
////    c2 = NEW(char);
////    *c2 = 'b';
////    c3 = NEW(char);
////    *c3 = 'b';
////    a = MAKE_SET_PTR(c1,c2,c3);
////    b = MAKE_SET_PTR(c1,c2,c3);
////    ASSERT_TRUE(equal(a,b), "same pointer sets");
////    removeVectorElem(a, c3);
////    ASSERT_FALSE(equal(a,b), "not same pointer sets");
//
//    return PASS;
//}
//
//static rc
//testVectorOperations(void)
//{
//    Vector *a;
//    Vector *b;
//    Vector *un;
//    Vector *in;
//    Vector *result;
//
//    // string sets
////    a = MAKE_STR_SET(strdup("a"), strdup("b"));
////    b = MAKE_STR_SET(strdup("b"), strdup("c"));
////    un = MAKE_STR_SET(strdup("a"), strdup("b"), strdup("c"));
////    in = MAKE_STR_SET(strdup("b"));
////
////    result = unionVectors(a,b);
////    ASSERT_EQUALS_NODE(un,result, "union set if {a,b,c}");
////
////    result = intersectVectors(a,b);
////    ASSERT_EQUALS_NODE(in, result, "intersected set if {b}");
////
////    // int sets
////    a = MAKE_INT_SET(1,2,3,4);
////    b = MAKE_INT_SET(3,4,5,6);
////    un = MAKE_INT_SET(1,2,3,4,5,6);
////    in = MAKE_INT_SET(3,4);
////
////    INFO_LOG("created sets");
////
////    result = unionVectors(a,b);
////    ASSERT_EQUALS_NODE(un, result, "union set if {1,2,3,4,5,6}");
////
////    result = intersectVectors(a,b);
////    ASSERT_EQUALS_NODE(in, result, "intersected set if {3,4}");
//
//    return PASS;
//}
//
