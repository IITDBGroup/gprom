/*-----------------------------------------------------------------------------
 *
 * test_hashmap.c
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
#include "model/set/hashmap.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

static rc testIntKeyHashMap(void);
static rc testHashMapToStringSortOrder(void);
static rc testHashListValue(void);

rc
testHashMap()
{
    RUN_TEST(testIntKeyHashMap(), "test integer key hashmap");
    RUN_TEST(testHashMapToStringSortOrder(),
            "test determinisim of toString for hashmap");
	RUN_TEST(testHashListValue(), "test list appending in hashmaps");

    return PASS;
}

#define _I(val) ((Node *) createConstInt(val))
#define _S(val) ((Node *) createConstString(val))

static rc
testIntKeyHashMap(void)
{
    HashMap *a = NEW_MAP(Constant, Constant);

    MAP_ADD_INT_KEY(a,1,_I(1));
    MAP_ADD_INT_KEY(a,2,_I(3));
    MAP_ADD_INT_KEY(a,3,_I(15));

    ASSERT_EQUALS_INT(3, mapSize(a), "hashmap size 3");

    // has keys
    ASSERT_TRUE(MAP_HAS_INT_KEY(a,1), "map has entry with key 1");
    ASSERT_TRUE(MAP_HAS_INT_KEY(a,2), "map has entry with key 2");
    ASSERT_TRUE(MAP_HAS_INT_KEY(a,3), "map has entry with key 3");
    ASSERT_FALSE(MAP_HAS_INT_KEY(a,4), "map does not have entry with key 4");

    // get entry
    ASSERT_EQUALS_NODE(_I(15), MAP_GET_INT(a,3), "hashmap 3 => 15");

    // remove element
    removeAndFreeMapElem(a,_I(2));
    ASSERT_FALSE(MAP_HAS_INT_KEY(a,2), "map does not have entry with key 2");
    ASSERT_EQUALS_INT(2, mapSize(a), "hashmap size 2");

    return PASS;
}

static rc
testHashMapToStringSortOrder(void)
{
    HashMap *a = NEW_MAP(Constant, Constant);
    HashMap *b = NEW_MAP(Constant, Constant);

    MAP_ADD_STRING_KEY(a, strdup("b"), createConstString("VALUE2"));
    MAP_ADD_STRING_KEY(a, strdup("a"), createConstString("VALUE1"));

    MAP_ADD_STRING_KEY(b, strdup("a"), createConstString("VALUE1"));
    MAP_ADD_STRING_KEY(b, strdup("b"), createConstString("VALUE2"));

    ASSERT_EQUALS_STRING(nodeToString(a), nodeToString(b),
            "insertion order does not effect toString result for hashmaps");

    return PASS;
}

static rc
testHashListValue(void)
{
	HashMap *a = NEW_MAP(Constant, List);

    addToMapValueList(a, _S("A"), _S("A"), FALSE);
    addToMapValueList(a, _S("A"), _S("A"), FALSE);
    addToMapValueList(a, _S("A"), _S("B"), FALSE);
	addToMapValueList(a, _S("A"), _S("C"), FALSE);

	addToMapValueList(a, _S("B"), _S("A"), FALSE);

	ASSERT_EQUALS_NODE(LIST_MAKE(_S("A"), _S("A"), _S("B"), _S("C")), getMapString(a, "A"), "a -> (a,a,b,c)");
	ASSERT_EQUALS_NODE(LIST_MAKE(_S("A")), getMapString(a, "B"), "b -> (a)");

	a = NEW_MAP(Constant, List);

    addToMapValueList(a, _S("A"), _S("A"), TRUE);
    addToMapValueList(a, _S("A"), _S("A"), TRUE);
    addToMapValueList(a, _S("A"), _S("B"), TRUE);
	addToMapValueList(a, _S("A"), _S("C"), TRUE);
    addToMapValueList(a, _S("A"), _S("A"), TRUE);
    addToMapValueList(a, _S("A"), _S("B"), TRUE);

	addToMapValueList(a, _S("B"), _S("A"), TRUE);

	ASSERT_EQUALS_NODE(LIST_MAKE(_S("A"), _S("B"), _S("C")), getMapString(a, "A"), "a -> (a,b,c)");
	ASSERT_EQUALS_NODE(LIST_MAKE(_S("A")), getMapString(a, "B"), "b -> (a)");

	return PASS;
}
