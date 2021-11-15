/*
 *------------------------------------------------------------------------------
 *
 * test_semantic_optimization.c - Testing semantic query optimization for
 * provenance capture.
 *
 *
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-08
 *        SUBDIR: test/
 *
 *-----------------------------------------------------------------------------
 */

#include "model/node/nodetype.h"
#include "test_main.h"
#include "log/logger.h"

#include "model/datalog/datalog_model.h"
#include "model/graph/graph.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"

static rc testOptimization(void);
static rc testRewriting(void);
static rc testJoinGraph(void);
static rc testAdaptFDsForRule(void);


rc
testSemanticOptimization(void)
{
    RUN_TEST(testJoinGraph(), "testing creation of join graphs for datalog rules.");
	RUN_TEST(testRewriting(), "testing creation of capture rules.");
    RUN_TEST(testOptimization(), "testing optimizing a DL rule for provenance capture using semantic query optimization.");
	RUN_TEST(testAdaptFDsForRule(), "test adapting FDs to a FL rule by replacing attributes with variables.");

	return PASS;
}

#define ASSERT_NODE(_g,_a) ASSERT_TRUE(hasNode(_g,(Node *) _a), CONCAT_STRINGS("graph contains node ", nodeToString(_a)))
#define ASSERT_EDGE(_g,_f,_t) ASSERT_TRUE(hasEdge(_g,(Node *) _f, (Node *) _t), CONCAT_STRINGS("graph contains edge ", nodeToString(_f),  " - ",  nodeToString(_t)))
#define ASSERT_NO_EDGE(_g,_f,_t) ASSERT_TRUE(!hasEdge(_g,(Node *) _f, (Node *) _t), CONCAT_STRINGS("graph does not contain edge ", nodeToString(_f)," - ",nodeToString(_t)))

static rc
testJoinGraph(void)
{
	Graph *jg;
	DLRule *r;
	DLAtom *gr, *gs, *gt;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	gt = DLATOM_FROM_STRS("T",FALSE,"Z","A");

	// Q(X) :- R(X,Y), S(Y,Z). => R -> S, S -> R
	r = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Y"),
					 LIST_MAKE(gr,gs));
	DEBUG_NODE_BEATIFY_LOG("DLrule", r);
	jg = createJoinGraph(r);
	DEBUG_NODE_BEATIFY_LOG("generated join graph", jg);

	ASSERT_NODE(jg,gr);
	ASSERT_NODE(jg,gs);

	ASSERT_EDGE(jg,gr,gs);
	ASSERT_EDGE(jg,gs,gr);

	// Q(X) :- R(X,Y), S(Y,Z), T(Z,A) => R -> S, S -> R, S -> T, T -> S
	r = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Y"),
					 LIST_MAKE(gr,gs,gt));
	DEBUG_NODE_BEATIFY_LOG("DLrule", r);
	jg = createJoinGraph(r);
	DEBUG_NODE_BEATIFY_LOG("generated join graph", jg);

	ASSERT_NODE(jg,gr);
	ASSERT_NODE(jg,gs);
	ASSERT_NODE(jg,gt);

	ASSERT_EDGE(jg,gr,gs);
	ASSERT_EDGE(jg,gs,gr);

	ASSERT_EDGE(jg,gs,gt);
	ASSERT_EDGE(jg,gt,gs);

	// Q(X) :- R(X,Y), S(Y,Z), T(Z,A), Y < Z, Z < A. => R -> S, S -> R, S -> T, T -> S
	r = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Y"),
					 LIST_MAKE(gr,gs,gt,
							   createDLComparison("<",
												  (Node *) createDLVar("Y", DT_INT),
												  (Node *) createDLVar("Z", DT_INT)),
							   createDLComparison("<",
												  (Node *) createDLVar("Z", DT_INT),
												  (Node *) createDLVar("A", DT_INT))
						 ));
	DEBUG_NODE_BEATIFY_LOG("DLrule", r);
	jg = createJoinGraph(r);
	DEBUG_NODE_BEATIFY_LOG("generated join graph", jg);

	ASSERT_NODE(jg,gr);
	ASSERT_NODE(jg,gs);
	ASSERT_NODE(jg,gt);

	ASSERT_EDGE(jg,gr,gs);
	ASSERT_EDGE(jg,gs,gr);

	ASSERT_EDGE(jg,gs,gt);
	ASSERT_EDGE(jg,gt,gs);


	return PASS;
}

static rc
testRewriting(void)
{
	DLRule *r, *rewr, *exp;
	DLAtom *gr, *gs, *head, *phead;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	head = DLATOM_FROM_STRS("Q", FALSE, "X");
	phead = DLATOM_FROM_STRS("PROV_R",FALSE,"X","Y");

	r = createDLRule(head,
					 LIST_MAKE(gr,gs));

	rewr = createCaptureRule(r, gr, NULL);
	exp = createDLRule(phead,
					   LIST_MAKE(gr,gs,head));

	ASSERT_EQUALS_NODE(exp,rewr,"capture rule is");

	return PASS;
}

static rc
testOptimization(void)
{
	DLRule *r, *opt, *exp;
	List *fds;
	DLAtom *headx, *headxy, *gr,*gs, *glr, *gt, *gu, *pr, *plr;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	glr = DLATOM_FROM_STRS("PRR",FALSE,"X","Y","A");

	pr = DLATOM_FROM_STRS("PROV_R",FALSE,"X","Y");
	plr = DLATOM_FROM_STRS("PROV_PRR",FALSE,"X","Y","A");

	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	gt = DLATOM_FROM_STRS("T",FALSE,"A","B");
	gu = DLATOM_FROM_STRS("U",FALSE,"B","C");
	headx = DLATOM_FROM_STRS("Q", FALSE, "X");
	headxy = DLATOM_FROM_STRS("Q", FALSE, "X", "Y");

	// Q(X,Y) :- R(X,Y), S(Y,Z). R(a,b), S(c,d). a -> b for R
	// optimized capture rule: R(X,Y) :- Q(X,Y).
	r = createDLRule(headxy, LIST_MAKE(gr,gs));
	fds = LIST_MAKE(createFD("R", MAKE_STR_SET("A"), MAKE_STR_SET("B")));

	exp = createDLRule(pr,LIST_MAKE(headxy));
	opt = optimizeDLRule(r, fds, "R", NULL);

	DEBUG_NODE_BEATIFY_LOG("expected and optimized rules: ", exp, opt);
	ASSERT_EQUALS_NODE(exp, opt, "optimized rule");

	// Q(X) :- R(X,Y), S(Y,Z). R(a,b), S(c,d). X -> Y for R
	// optimized capture rule: R(X,Y) :- Q(X), R(X,Y).
	r = createDLRule(headx, LIST_MAKE(gr,gs));
	fds = LIST_MAKE(createFD("R", MAKE_STR_SET("A"), MAKE_STR_SET("B")));

	exp = createDLRule(pr,LIST_MAKE(gr,headx));
	opt = optimizeDLRule(r, fds, "R", NULL);

	DEBUG_NODE_BEATIFY_LOG("expected and optimized rules: ", exp, opt);
	ASSERT_EQUALS_NODE(exp, opt, "optimized rule");

	// Q(X) :- PRR(X,Y,A), S(Y,Z), T(A,B), U(B,C). a->b for R
	// optimized capture rule: R(X,Y,A) :- Q(X), R(X,Y,A), T(A,B), U(B,C)
	r = createDLRule(headx, LIST_MAKE(glr,gs,gt,gu));
	fds = LIST_MAKE(createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("B")));

	exp = createDLRule(plr,LIST_MAKE(glr,gt,gu,headx));
	opt = optimizeDLRule(r, fds, "PRR", NULL);

	DEBUG_NODE_BEATIFY_LOG("expected and optimized rules: ", exp, opt);
	ASSERT_EQUALS_NODE(exp, opt, "optimized rule");

	// Q(X) :- PRR(X,Y,A), S(Y,Z), T(A,B), U(B,C). x->y for R X -> A
	// optimized capture rule: R(X,Y,A) :- Q(X), R(X,Y,A)
	r = createDLRule(headx, LIST_MAKE(glr,gs,gt,gu));
	fds = LIST_MAKE(createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("B")),
					createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("C")));

	exp = createDLRule(plr,LIST_MAKE(glr,headx));
	opt = optimizeDLRule(r, fds, "PRR", NULL);

	DEBUG_NODE_BEATIFY_LOG("expected and optimized rules: ", exp, opt);
	ASSERT_EQUALS_NODE(exp, opt, "optimized rule");

	return PASS;
}

static rc
testAdaptFDsForRule(void)
{
	List *in, *exp, *out;
	DLRule *r;
	DLAtom *headx, *gr,*gs, *grt;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	grt = DLATOM_FROM_STRS("R",FALSE,"X","Z");

	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	headx = DLATOM_FROM_STRS("Q", FALSE, "X");

	r = createDLRule(headx, LIST_MAKE(gr,gs,grt));
    in = LIST_MAKE(createFD("R", MAKE_STR_SET("A"), MAKE_STR_SET("B")),
					createFD("S", MAKE_STR_SET("C"), MAKE_STR_SET("D")));

	out = adaptFDsToRules(r, in);
	exp = LIST_MAKE(createFD("R", MAKE_STR_SET("X"), MAKE_STR_SET("Y")),
					createFD("R", MAKE_STR_SET("X"), MAKE_STR_SET("Z")),
					createFD("S", MAKE_STR_SET("Y"), MAKE_STR_SET("Z")));

	ASSERT_EQUALS_NODE(exp,out, "x->y");

	return PASS;
}
