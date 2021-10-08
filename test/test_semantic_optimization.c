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


//static rc testOptimization(void);
static rc testJoinGraph(void);

rc
testSemanticOptimization(void)
{
    RUN_TEST(testJoinGraph(), "testing creation of join graphs for datalog rules.");
//    RUN_TEST(testOptimization(), "testing optimizing a DL rule for provenance capture using semantic query optimization.");

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
	DLAtom *gr, *gs;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");

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

	return PASS;
}

/* static rc */
/* testOptimization(void) */
/* { */
/* 	DLRule *r, *opt, *exp; */
/* 	List *fds; */

/* 	// Q(X) :- R(X,Y), S(Y,Z). R(a,b), S(c,d). a -> b for R */
/* 	// optimized capture rule: Q(X) :- R(X,Y). */
/* 	r = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Y"), */
/* 					 LIST_MAKE(DLATOM_FROM_STRS("R",FALSE,"X","Y"), */
/* 							   DLATOM_FROM_STRS("S",FALSE,"Y","Z"))); */
/* 	fds = LIST_MAKE(createFD("R", MAKE_STR_SET("X"), MAKE_STR_SET("Y"))); */

/* 	exp = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Y"), */
/* 					 LIST_MAKE(DLATOM_FROM_STRS("R",FALSE,"X","Y"), */
/* 							   DLATOM_FROM_STRS("S",FALSE,"Y","Z"))); */
/* 	opt = optimizeDLRule(r, fds, "R"); */


/* 	ASSERT_EQUALS_NODE(opt, exp, "optimized rule"); */

/* 	return PASS; */
/* } */
