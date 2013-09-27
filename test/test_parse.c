/*-----------------------------------------------------------------------------
 *
 * test_parse.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "parser/parser.h"

static rc testParseSimpleSQL(void);

rc
testParse(void)
{
    RUN_TEST(testParseSimpleSQL(), "Test simple SQL queries");

    return PASS;
}


static rc
testParseSimpleSQL(void)
{
    return PASS;
}
