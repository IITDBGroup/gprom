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


#include "log/logger.h"
#include "test_main.h"

#include "model/node/nodetype.h"
#include "model/set/vector.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

static rc testIntVector(void);
static rc testNodeVector(void);
static rc testCopyVector(void);
static rc testStringVector(void);
static rc testVectorIteration(void);

rc
testVector()
{
    RUN_TEST(testIntVector(), "test integer vectors");
    RUN_TEST(testNodeVector(), "test node vectors");
	RUN_TEST(testVectorIteration(), "test iterate vector");
	RUN_TEST(testCopyVector(), "test copying vectors");
    RUN_TEST(testStringVector(), "test string vectors");

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
	DEBUG_NODE_BEATIFY_LOG("vector", a);
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

static rc
testVectorIteration(void)
{
	Vector *i = MAKE_VEC_INT(1,2,3,4,5);
	int n = 1;

	FOREACH_VEC_INT(c,i)
	{
		ASSERT_EQUALS_INT(n++, c, "int vector iteration");
	}

	AttributeReference *a1 = createAttributeReference ("a1");
    Vector *a = MAKE_VEC_NODE(AttributeReference, a1, a1, a1);

	FOREACH_VEC(AttributeReference,attr,a)
	{
		ASSERT_EQUALS_NODE(a1, attr, "iterate node vector");
	}

	return PASS;
}

static rc
testCopyVector(void)
{
    Vector *a = MAKE_VEC_INT(1,2,3);

	ASSERT_EQUALS_NODE(a, copyObject(a), "Copy int vector");

	AttributeReference *a1 = createAttributeReference ("a1");
    AttributeReference *a2 = createAttributeReference ("a2");
    a = MAKE_VEC_NODE(AttributeReference, a1, a2, a2);

	ASSERT_EQUALS_NODE(a, copyObject(a), "copy node vector");

	return PASS;
}

static rc
testStringVector(void)
{
   Vector *a = MAKE_VEC_STRING(strdup("abc"), strdup("a"));
   Vector *b = MAKE_VEC_STRING(strdup("ab"), strdup("bc"));

   ASSERT_EQUALS_INT(2, a->length, "vector a is of size 2");
   ASSERT_EQUALS_INT(2, b->length, "vector b is of size 2");

   ASSERT_TRUE(findVecString(a, "abc"), "vector a has abc");
   ASSERT_TRUE(findVecString(a, "a"), "vector a has a");
   ASSERT_FALSE(findVecString(a, "ab"), "vector a has not ab");

   ASSERT_TRUE(findVecString(b, "ab"), "vector a has ab");
   ASSERT_TRUE(findVecString(b, "bc"), "vector a has bc");
   ASSERT_FALSE(findVecString(b, "abc"), "vector b has not abc");

   popVecString(b);
   ASSERT_EQUALS_INT(1, b->length, "vector b is of size 1");
   ASSERT_FALSE(findVecString(b, "bc"), "vector b has not ab");

   return PASS;
}
