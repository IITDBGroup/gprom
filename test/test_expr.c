/*-----------------------------------------------------------------------------
 *
 * test_expr.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"

/* internal tests */
static rc testAttributeReference (void);
static rc testFunctionCall (void);

/* check expression model */
rc
testExpr (void)
{
    RUN_TEST(testAttributeReference(), "test attribute references");
    RUN_TEST(testFunctionCall(), "test function call nodes");

    return PASS;
}

/* */
static rc
testAttributeReference (void)
{
    AttributeReference *a, *b;

    a = createAttributeReference("test");
    b = makeNode(AttributeReference);
    b->name = "test";

    ASSERT_EQUALS_INT(a->type, T_AttributeReference, "type is attribute reference");
    ASSERT_EQUALS_INT(a->type, b->type, "types are the same");
    ASSERT_EQUALS_STRINGP(a->name, b->name, "names are the same");
    ASSERT_EQUALS_NODE(a,b,"both attribute references are same");

    return PASS;
}

/* */
static rc
testFunctionCall(void)
{
    return PASS;
}
