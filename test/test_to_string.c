/*-----------------------------------------------------------------------------
 *
 * test_to_string.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/set/vector.h"
#include "test_main.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"

static rc testStringInfo(void);
static rc testQueryBlockToString(void);
static rc testCollectionsToString(void);

rc
testToString(void)
{
    RUN_TEST(testStringInfo(), "Test StringInfo data type");
    RUN_TEST(testQueryBlockToString(), "Test QueryBlock to String");
	RUN_TEST(testCollectionsToString(), "Test collections to string");

    return PASS;
}

static rc
testStringInfo(void)
{
    StringInfo str = makeStringInfo();

    appendStringInfoChar(str,'a');
    ASSERT_EQUALS_STRING("a", str->data, "data is a");

    appendStringInfoString(str, "hello");
    ASSERT_EQUALS_STRING("ahello", str->data, "data is ahello");
    ASSERT_EQUALS_INT(6, str->len, "length is 6");

    for(int i = 0; i < 256; i++)
        appendStringInfoChar(str, 'b');
    ASSERT_EQUALS_INT(6 + 256, str->len, "length is 6 + 256");
    for(int i = 255; i < 256 + 6; i++)
       ASSERT_EQUALS_INT('b', str->data[i], "chars are all b");

    resetStringInfo(str);
    ASSERT_EQUALS_INT(0, str->len, "after reset length is 0");

    appendStringInfo(str, "%s", "test");
    ASSERT_EQUALS_STRING("test", str->data, "data is test");

    return PASS;
}

static rc
testQueryBlockToString(void)
{
    char *toString;
    QueryBlock *q = createQueryBlock();

    q->distinct = NULL;
    q->fromClause = NIL;
    q->havingClause = NULL;
    q->selectClause = NIL;
    q->whereClause = NULL;

    toString = nodeToString(q);
    ASSERT_EQUALS_STRING("{QUERYBLOCK:distinct|<>:selectClause|<>:fromClause|<>"
            ":whereClause|<>:groupByClause|<>:havingClause|<>:orderByClause|<>"
            ":limitClause|<>:offsetClause|<>}", toString, "");

    return PASS;
}

static rc
testCollectionsToString(void)
{
	Vector *v = MAKE_VEC_STRING("a", "bc", "d");
	HashMap *m = NEW_MAP(Constant, Constant);
	Set *s = MAKE_STR_SET("a", "b", "c");

    MAP_ADD_STRING_KEY_AND_VAL(m, "a", "1");
    MAP_ADD_STRING_KEY_AND_VAL(m, "b", "2");

	ASSERT_EQUALS_STRING("{[a, bc, d]}", nodeToString(v), "vector to string");
	ASSERT_EQUALS_STRING("{{{CONSTANT:constType|DT_STRING - 2:value 'a':isNull|false} => {CONSTANT:constType|DT_STRING - 2:value '1':isNull|false}, {CONSTANT:constType|DT_STRING - 2:value 'b':isNull|false} => {CONSTANT:constType|DT_STRING - 2:value '2':isNull|false}}}", nodeToString(m), "hashmap to string");
	ASSERT_EQUALS_STRING("{a, b, c}", nodeToString(s), "set to string");

	return PASS;
}
