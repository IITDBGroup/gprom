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

rc
testHashMap()
{
    RUN_TEST(testIntKeyHashMap(), "test integer key hashmap");
//    RUN_TEST(testNodeVector(), "test node vectors");
//    RUN_TEST(testVectorIteration(), "test set iteration");
//    RUN_TEST(testVectorEquals(), "test equal function on sets");
//    RUN_TEST(testVectorOperations(), "test set operations");

    return PASS;
}

#define _I(val) ((Node *) createConstInt(val))

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
