/*-----------------------------------------------------------------------------
 *
 * test_equal.c
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
static rc testequalFunctionCall (void);
static rc testequalAttributeReference (void);
static rc testequalList (void);


/* check equal model */
rc
testequal (void)
{
    RUN_TEST(testequalFunctionCall (), "test equal FunctionCall");
    RUN_TEST(testequalAttributeReference(), "test equal AttibuteReference");
    RUN_TEST(testequalList(), "test equal List");

    return PASS;
}

static rc testequalFunctionCall (void);
{
  FunctionCall *a, *b;
  a = createFunctionCall("test");
  b = makenode(FunctionCall);
  b->name = "test";
  
  ASSERT_EQUALS_INT(a->type, T_FunctionCall, "type is a FunctionCall");
  ASSERT_EQUALS_INT(a->type, b->type, "types are the same");
  ASSERT_EQUALS_STRINGP(a->fName, b->fName, "names are the same");
  ASSERT_EQUALS_NODE(a->args,b->args,"both args are same");

  return PASS;
}

static rc testequalAttributeReference (void);
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

static rc testequalList (void);
{
  return PASS;‭
}







