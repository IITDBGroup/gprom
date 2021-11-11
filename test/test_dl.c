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

#include "model/graph/graph.h"
#include "parser/parser_dl.h"
#include "test_main.h"
#include "model/list/list.h"
#include "analysis_and_translate/analyze_dl.h"
#include "model/datalog/datalog_model.h"
#include "model/graph/graph.h"

static rc testMakeVarsUnique(void);
static rc testRuleMerging(void);
static rc testRuleGraph(void);

rc
testDatalogModel(void)
{
    RUN_TEST(testMakeVarsUnique(), "test replacing vars with unique vars");
	RUN_TEST(testRuleMerging(), "test merging of subqueries");
	RUN_TEST(testRuleGraph(), "test creation of rule graph");

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

static rc
testRuleMerging(void)
{
	DLProgram *p = (DLProgram *) parseFromStringdl(
		"R(1,1).\nS(1,1).\n"
		"T(1).\nU(1,1).\n"
		"Q(X) :- G(X,Y), H(Y,Z).\n"
		"G(Y,Z) :- R(Y,Z), S(Z,X).\n"
		"H(Z,X) :- T(X), U(Z,Z).");
	DLProgram *expected = (DLProgram *) parseFromStringdl(
		"R(1,1).\nS(1,1).\n"
		"T(1).\nU(1,1).\n"
		"Q(V3) :- R(V3,V4),S(V4,V6),T(V7),U(V4,V4).");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, TRUE);
	ASSERT_EQUALS_NODE(expected->rules, p->rules, "after merging subqueries.");

	p = (DLProgram *) parseFromStringdl(
		"R(1,1).\nS(1,1).\n"
		"T(1).\nU(1,1).\n"
		"Q(X) :- G(X,Y), G(Y,Z).\n"
		"G(Z,X) :- R(X,X), R(Z,Z).");
	expected = (DLProgram *) parseFromStringdl(
		"R(1,1).\nS(1,1).\n"
		"T(1).\nU(1,1).\n"
		"Q(V0) :- R(V1,V1),R(V0,V0),R(V2,V2),R(V1,V1).");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, TRUE);
	ASSERT_EQUALS_NODE(expected->rules, p->rules, "after merging subqueries.");

	return PASS;
}

#define ADD_STR_EDGE(_g,_s,_e) ADD_EDGE(_g, createConstString(_s), createConstString(_e))

static rc
testRuleGraph(void)
{
	DLProgram *p = (DLProgram *) parseFromStringdl("Q(X) :- R(X,Y), S(Y,Z).");
	Graph *g, *exp;

	analyzeDLModel((Node *) p);
	g = createRelToRelGraph((Node *) p);
	exp = createEmptyGraph();
	ADD_STR_EDGE(exp,"Q","R");
	ADD_STR_EDGE(exp,"Q","S");

	ASSERT_EQUALS_NODE(exp, g, "rel to rel graph");

	p = (DLProgram *) parseFromStringdl("Q(X) :- R(X,Y), S(Y,Z). Q2(X) :- Q(X), Q(X). Q3(X) :- R(X,Z), Q2(X).");
	analyzeDLModel((Node *) p);
	g = createRelToRelGraph((Node *) p);
	exp = createEmptyGraph();
	ADD_STR_EDGE(exp,"Q","R");
	ADD_STR_EDGE(exp,"Q","S");
	ADD_STR_EDGE(exp,"Q2","Q");
	ADD_STR_EDGE(exp,"Q3","R");
	ADD_STR_EDGE(exp,"Q3","Q2");

	ASSERT_EQUALS_NODE(exp, g, "rel to rel graph");

	return PASS;
}
