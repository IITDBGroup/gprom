/*-----------------------------------------------------------------------------
 *
 * test_copy.c
 *
 *
 *		AUTHOR: Bowen
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
//static rc testcopyAttributeReference (void);
static rc testshallowCopyList (void);
static rc testdeepCopyList (void);

/* check equal model */
rc
testCopy (void)
{
    // RUN_TEST(testcopyAttributeReference(), "test copy AttibuteReference");
    RUN_TEST(testshallowCopyList(), "test shallow copy for List");
    RUN_TEST(testdeepCopyList(), "test deep copy for List");
    return PASS;
}



static rc
testshallowCopyList(void)
{
    List *fromList = NIL, *newList = NIL;

    // check shallow integer list copy
    fromList = appendToTailOfListInt(fromList, 1);
    fromList = appendToTailOfListInt(fromList, 2);
    fromList = appendToTailOfListInt(fromList, 3);
	newList = copyList(fromList);
    ASSERT_EQUALS_INT(getListLength(fromList),getListLength(newList),"Lists have the same length");
    FORBOTH_INT(l,r,fromList,newList)
    ASSERT_EQUALS_INT(l,r,"compare list element");
    FORBOTH_LC(l,r,fromList,newList)
    ASSERT_FALSE(l == r, "list cells are actually new");
    FREE(fromList);
    FREE(newList);
    return PASS;

}

//static rc
//testshallowCopyList(void)
//{
//    List *fromList =NIL, *newList = NIL;
//    StringInfo str = makeStringInfo();
//    fromList = appendStringInfoString(str,"a");
//    fromList = appendStringInfoString(str, "b");
//    fromList = appendStringInfoString(str, "c");
//    newList  = copyList(fromList);
//
//    ASSERT_EQUALS_INT(getListLength(fromList), getListLength(newList), " Two lists have the same length");
//    ASSERT_EQUALS_STRING(fromList, newList, "Two List have same content");
//    FORBOTH_LC(l,r,fromList, newList)
//    ASSERT_FALSE(l==r, "List cells are actually new");
//    FREE(fromList);
//    FREE(newList);
//    return PASS;
//
//
//
//}

static rc
testdeepCopyList(void)
{
    List *fromList = NIL, *toList = NIL;
    //AttributeReference *a, *b;
    char *a = "a";
    char *b = "b";
    AttributeReference *a1, *a2;
  //  StringInfo str = makeStringInfo();
	a1 = createAttributeReference(strdup(a));
	a2 = createAttributeReference(strdup(b));
  //  appendStringInfoString(str, a1);
//	appendStringInforString(str,a2);
	fromList = appendToTailOfList(fromList, a1);
	fromList = appendToTailOfList(fromList, a2);
	toList = copyList(fromList);
	ASSERT_EQUALS_NODE(fromList,toList,"Lists have the same node type");
	ASSERT_EQUALS_STRING(nodeToString(fromList),nodeToString(toList), "both lists are equal");
	FREE(fromList);
	FREE(toList);
	return PASS;

    // FORBOTH(void,l,r,fromList,newList)
    //compare

    //ASSERT_EQUALS_NODE(fromList, newList);

    //    ASSERT_EQUALS_P(getHeadOfList(fromList),getHeadOfList(newList),"Header pointers piont to the same list");
    //    ASSERT_EQUALS_P(getTailOfList(fromList),getTailOfList(newList),"Tail pointers piont to the same list");
    //ASSERT_EQUALS_P(a,b,message)
}




