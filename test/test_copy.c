/*-----------------------------------------------------------------------------
 *
 * test_copy.c
 *			  
 *		
 *		AUTHOR: Hao
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */
#include <string.h>
#include "model/list/list.h"
#include "test_main.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"



/* internal tests */
static rc testcopyAttributeReference (void);
static rc testdeepCopyList (void);


/* check equal model */
rc
testcopy (void)
{
    RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testdeepCopyList(), "test deep copy List");

    return PASS;
}



static rc testcopyAttibuteReference (void);
{
    AttributeReference *from, *new;
    from = createAttributeReference("test");
    new = makeNode(AttributeReference);
    new->name = "test";

    ASSERT_EQUALS_INT(from->type, T_AttributeReference, "type is attribute reference");
    ASSERT_EQUALS_INT(from->type, new->type, "types are the same");
    ASSERT_EQUALS_STRINGP(from->name, new->name, "names are the same");
    ASSERT_EQUALS_NODE(from,new,"both copy are same");

    return PASS;‭
}


static rc testdeepCopyList(void);
{
    return PASS;
}






