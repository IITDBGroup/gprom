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
	DLVar *va, *ve;
	
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

	a1 = createDLAtom("R", LIST_MAKE(VAR("B"), VAR("A")), FALSE);
	ve = createDLVar("V0", DT_INT);
	va = createUniqueVar((Node *) a1, DT_INT);
	ASSERT_EQUALS_NODE(ve,va,"R(B,A) -> V0");
	
	a1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("A")), FALSE);
	ve = createDLVar("V1", DT_INT);
	va = createUniqueVar((Node *) a1, DT_INT);
	ASSERT_EQUALS_NODE(ve,va,"R(V0,A) -> V1");
	
	a1 = createDLAtom("R", LIST_MAKE(VAR("V0"), VAR("V1")), FALSE);
	ve = createDLVar("V2", DT_INT);
	va = createUniqueVar((Node *) a1, DT_INT);
	ASSERT_EQUALS_NODE(ve,va,"R(V0,V1) -> V2");
	
    return PASS;
}

#define TEST_MERGED(_inp,_exp) \
	do {																\
		DLProgram *_inparse, *_expparse, *_merged;						\
		char *_mes;														\
																		\
	    _inparse = (DLProgram *) parseFromStringdl(_inp);				\
		_expparse = (DLProgram *) parseFromStringdl(_exp);				\
		_inparse = (DLProgram *) analyzeDLModel((Node *) _inparse);		\
		_expparse = (DLProgram *) analyzeDLModel((Node *) _expparse);	\
																		\
		_merged = mergeSubqueries(_inparse, TRUE);						\
																		\
		INFO_DL_LOG("================================================================================\ninput program", _inparse); \
		INFO_DL_LOG("================================================================================\nmerged program", _merged); \
		INFO_DL_LOG("================================================================================\nexpected program", _inparse); \
																		\
		_mes = formatMes("================================================================================" \
						"\nEXPECTED:\n%s\n"								\
						"================================================================================\n" \
						 "ACTUAL\n%s", \
						 datalogToOverviewString(_expparse),			\
						 datalogToOverviewString(_merged));				\
		ASSERT_EQUALS_NODE(_expparse->rules, _merged->rules, _mes);		\
	} while(0)	
		  
static rc
testRuleMerging(void)
{
	TEST_MERGED(
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- Q2(X,Y), Q3(Y,Z).\n"
		"Q2(Y,Z) :- RP(Y,Z), SP(Z,X).\n"
		"Q3(Z,X) :- TP(X), UP(Z,Z).",
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- RP(X,Y),SP(Y,V2),TP(Z),UP(Y,Y).");

    TEST_MERGED(
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- Q2(X,Y), Q2(Y,Z).\n"
		"Q2(Z,X) :- RP(X,X), RP(Z,Z).",
		"RP(1,1).\nSP(1,1).\n"
		"TP(1).\nUP(1,1).\n"
		"Q(X) :- RP(Y,Y),RP(X,X),RP(Z,Z),RP(Y,Y).");

	// aggregation with subquery that cannot be merged (R.1 is a key)
	TEST_MERGED(
		"Q(count(1)) :- Q1(X)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.",
		"Q(count(1)) :- Q1(X)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.");

	// aggregation with subquery that can be merged (R.1 is a key)
    TEST_MERGED(
		"Q(count(1)) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"ANS : Q."
        "FD R: A -> B.",
		"Q(count(1)) :- R(X,V1)."
		"ANS : Q."
		"FD R: A -> B.");

	// allowed to merge union
    TEST_MERGED(
		"Q(X) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.",
		"Q(X) :- R(X,V1)."
		"Q(X) :- R(V1,X)."
		"ANS : Q.");

	// can't merge union in agg rule
    TEST_MERGED(
		"Q(sum(X)) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.",
		"Q(sum(X)) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"Q1(Y) :- R(X,Y)."
		"ANS : Q.");
	
	// cannot merge generalized projections
	TEST_MERGED(
		"Q(X) :- Q1(X)."
		"Q1(X+Y) :- R(X,Y)."
		"Q1(X*Y) :- R(X,Y)."
		"ANS : Q.",
		"Q(X) :- Q1(X)."
		"Q1(X+Y) :- R(X,Y)."
		"Q1(X*Y) :- R(X,Y)."
		"ANS : Q.");
	
	// merging with idb being used at different levels
	TEST_MERGED(
		"Q(X) :- Q1(X),Q2(X)."
		"Q2(min(X)) :- Q1(X)."
		"Q1(X) :- R(X,Y)."
		"ANS : Q."
		"FD R: A -> B.",
		"Q(X) :- R(X,V1),Q2(X)."
		"Q2(min(X)) :- R(X,V1)."
		"ANS : Q."
		"FD R: A -> B.");

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
