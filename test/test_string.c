/*-----------------------------------------------------------------------------
 *
 * test_string.c
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

static rc testStringConstruction(void);

rc
testString(void)
{
    RUN_TEST(testStringConstruction(), "Test simple SQL queries");

    return PASS;
}

static rc
testStringConstruction (void)
{
    char *concat;
    char *a = "a";
    char *b = "b";
    char *c = "c";
    StringInfo str;

    str = makeStringInfo();
    appendStringInfoStrings(str,a,b,c,NULL);
    ASSERT_EQUALS_STRING(str->data,"abc",
            "string concatenation result is abc");

    concat = CONCAT_STRINGS(a,b,c);
    ASSERT_EQUALS_STRING(concat,"abc", "string concatenation result is abc");

    return PASS;
}
