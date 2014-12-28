/*-----------------------------------------------------------------------------
 *
 * test_hash.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test_main.h"
#include "common.h"

#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"


static rc testHashImpliesEquals(void);

rc
testHash()
{
    RUN_TEST(testHashImpliesEquals(), "test that equal nodes have same hash");

    return PASS;
}

#define CS(_s) createConstString(strdup(_s))

static rc
testHashImpliesEquals(void)
{
    List *l1;
    List *l2;

    l1 = LIST_MAKE(CS("A"), CS("B"));
    l2 = LIST_MAKE(CS("A"), CS("B"));

    ASSERT_EQUALS_LONG(hashValue(l1), hashValue(l2), "same hash for l1 and l2");

    return PASS;
}
