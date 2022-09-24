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
    makeVarNamesUnique(LIST_MAKE(a1), TRUE);
    ASSERT_EQUALS_NODE(e1,a1,"R(V0,V1)");


    a1 = createDLAtom("R", LIST_MAKE(VAR("A"), VAR("B")), FALSE);
    a2 = createDLAtom("S", LIST_MAKE(VAR("B"), VAR("C")), FALSE);
    e1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("V1")), FALSE);
    e2 = createDLAtom("S", LIST_MAKE(VAR("V2"), VAR("V3")), FALSE);
    makeVarNamesUnique(LIST_MAKE(a1,a2), TRUE);
    ASSERT_EQUALS_NODE(LIST_MAKE(e1,e2), LIST_MAKE(a1,a2),"R(V0,V1)");

    a1 = createDLAtom("R", LIST_MAKE(VAR("A"), VAR("B")), FALSE);
    a2 = createDLAtom("S", LIST_MAKE(VAR("B"), VAR("C")), FALSE);
    r1 = createDLRule(a1, singleton(a2));
    e1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("V1")), FALSE);
    e2 = createDLAtom("S", LIST_MAKE(VAR("V1"), VAR("V2")), FALSE);
    er1 = createDLRule(e1, singleton(e2));
    makeVarNamesUnique(LIST_MAKE(r1), TRUE);
    ASSERT_EQUALS_NODE(er1, r1, "R(A,B) :- S(B,C);");

    return PASS;
}

static rc
testRuleMerging(void)
{
	DLProgram *p = (DLProgram *) parseFromStringdl(
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- Q2(X,Y), Q3(Y,Z).\n"
		"Q2(Y,Z) :- RP(Y,Z), SP(Z,X).\n"
		"Q3(Z,X) :- TP(X), UP(Z,Z).");
	DLProgram *expected = (DLProgram *) parseFromStringdl(
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- RP(X,Y),SP(Y,V2),TP(Z),UP(Y,Y).");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, TRUE);
	ASSERT_EQUALS_NODE(expected->rules, p->rules, "after merging subqueries.");

	p = (DLProgram *) parseFromStringdl(
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- Q2(X,Y), Q2(Y,Z).\n"
		"Q2(Z,X) :- RP(X,X), RP(Z,Z).");
	expected = (DLProgram *) parseFromStringdl(
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- RP(Y,Y),RP(X,X),RP(Z,Z),RP(Y,Y).");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, TRUE);
	ASSERT_EQUALS_NODE(expected->rules, p->rules, "after merging subqueries.");

	// aggregation with subquery that cannot be merged (R.1 is a key)
	p = (DLProgram *) parseFromStringdl(
		"Q(count(1)) :- Q1(X)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.");
	expected = (DLProgram *) parseFromStringdl(
		"Q(count(1)) :- Q1(X)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, TRUE);
	ASSERT_EQUALS_NODE(expected->rules, p->rules, "after merging subqueries.");

	// aggregation with subquery that can be merged (R.1 is a key)
	p = (DLProgram *) parseFromStringdl(
		"Q(count(1)) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"ANS : Q."
        "FD R: A -> B.");
	expected = (DLProgram *) parseFromStringdl(
		"Q(count(1)) :- R(X,V1)."
		"ANS : Q."
		"FD R: A -> B.");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, TRUE);
	ASSERT_EQUALS_NODE(expected->rules, p->rules, "after merging subqueries.");

	// not allowed to merge union
	p = (DLProgram *) parseFromStringdl(
		"Q(X) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.");
	expected = (DLProgram *) parseFromStringdl(
		"Q(X) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.");

	analyzeDLModel((Node *) p);
	analyzeDLModel((Node *) expected);

	p = mergeSubqueries(p, FALSE);
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
