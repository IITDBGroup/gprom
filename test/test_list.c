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

static rc testIntList(void);
static rc testPList(void);

rc
testList()
{
    RUN_TEST(testIntList(), "test integer lists");
    RUN_TEST(testPList(), "test pointer lists");

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
    return PASS;
}
