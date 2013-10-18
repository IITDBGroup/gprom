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

#include "test_main.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"

static rc testStringInfo(void);
static rc testQueryBlockToString(void);

rc
testToString(void)
{
    RUN_TEST(testStringInfo(), "Test StringInfo data type");
    RUN_TEST(testQueryBlockToString(), "Test QueryBlock to String");

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
            ":limitClause|<>}", toString, "");

    return PASS;
}
