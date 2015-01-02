/*-----------------------------------------------------------------------------
 *
 * test_string_utils.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "utility/string_utils.h"

static rc testStringSubstring(void);
static rc testEndTok(void);
static rc testMatchingSubstring(void);

rc
testStringUtils(void)
{
    RUN_TEST(testStringSubstring(), "Test substring function");
    RUN_TEST(testEndTok(), "Test strEndTok function");
    RUN_TEST(testMatchingSubstring(), "Test getMatchingSubstring function");
    return PASS;
}

static rc
testStringSubstring(void)
{
     char *a = "abcabcabcabc";

     ASSERT_EQUALS_STRING(replaceSubstr(a, "ab",""), "cccc",
             "remove ab from abcabcabcabc");
     ASSERT_EQUALS_STRING(replaceSubstr(a, "ab","bb"), "bbcbbcbbcbbc",
             "replace ab with bb from abcabcabcabc");
     ASSERT_EQUALS_STRING(replaceSubstr(a, "ab","bb"), "bbcbbcbbcbbc",
             "replace ab with bb from abcabcabcabc");

    return PASS;
}

static rc
testEndTok(void)
{
    ASSERT_EQUALS_STRING(strEndTok("AB_CD", "_"), "CD",
                 "get last token from AB_CD with delim _");
    ASSERT_EQUALS_STRING(strEndTok("_CD", "_"), "CD",
                 "get last token from _CD with delim _");
    ASSERT_EQUALS_STRING(strEndTok("AB_AB_AB_CD", "_"), "CD",
                 "get last token from AB_AB_AB_CD with delim _");
    ASSERT_EQUALS_STRING(strEndTok("AB__A_CD", "__A_"), "CD",
                 "get last token from AB__A_CD with delim __A_");

    return PASS;
}

static rc
testMatchingSubstring(void)
{
    ASSERT_EQUALS_STRING(getMatchingSubstring("bcdeaaabcde", "[^a]*([a]+[b]).*"),
            "aaab",
            "get matching substring from bcdeaaabcde according to"
                    " pattern .*([a]+[b]).*");

    return PASS;
}
