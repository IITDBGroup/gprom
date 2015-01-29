/*-----------------------------------------------------------------------------
 *
 * test_dl.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test/test_main.h"
#include "model/list/list.h"
#include "model/datalog/datalog_model.h"

static rc testMakeVarsUnique (void);

rc
testDatalogModel(void)
{
    RUN_TEST(testMakeVarsUnique(), "test replacing vars with unique vars");

    return PASS;
}

#define VAR(x) createDLVar(strdup(x),DT_STRING)

static rc
testMakeVarsUnique (void)
{
    DLAtom *a1, *a2, *e1, *e2;
    DLRule *r1, *er1;

    a1 = createDLAtom("R", LIST_MAKE(VAR("A"), VAR("B")), FALSE);
    e1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("V1")), FALSE);
    makeVarNamesUnique(LIST_MAKE(a1));
    ASSERT_EQUALS_NODE(e1,a1,"R(V0,V1)");


    a1 = createDLAtom("R", LIST_MAKE(VAR("A"), VAR("B")), FALSE);
    a2 = createDLAtom("S", LIST_MAKE(VAR("B"), VAR("C")), FALSE);
    e1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("V1")), FALSE);
    e2 = createDLAtom("S", LIST_MAKE(VAR("V2"), VAR("V3")), FALSE);
    makeVarNamesUnique(LIST_MAKE(a1,a2));
    ASSERT_EQUALS_NODE(LIST_MAKE(e1,e2), LIST_MAKE(a1,a2),"R(V0,V1)");

    a1 = createDLAtom("R", LIST_MAKE(VAR("A"), VAR("B")), FALSE);
    a2 = createDLAtom("S", LIST_MAKE(VAR("B"), VAR("C")), FALSE);
    r1 = createDLRule(a1, singleton(a2));
    e1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("V1")), FALSE);
    e2 = createDLAtom("S", LIST_MAKE(VAR("V1"), VAR("V2")), FALSE);
    er1 = createDLRule(e1, singleton(e2));
    makeVarNamesUnique(LIST_MAKE(r1));
    ASSERT_EQUALS_NODE(er1, r1, "R(A,B) :- S(B,C);");

    return PASS;
}
