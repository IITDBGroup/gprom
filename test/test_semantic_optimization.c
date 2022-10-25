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

#include "configuration/option.h"
#include "model/expression/expression.h"
#include "model/integrity_constraints/integrity_constraint_inference.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "test_main.h"
#include "log/logger.h"

#include "model/datalog/datalog_model.h"
#include "model/datalog/datalog_model_checker.h"
#include "analysis_and_translate/analyze_dl.h"
#include "model/graph/graph.h"
#include "parser/parser_dl.h"
#include "provenance_rewriter/semantic_optimization/prov_semantic_optimization.h"
#include "provenance_rewriter/datalog_lineage/datalog_lineage.h"
#include "utility/string_utils.h"


static rc testOptimization(void);
static rc testRewriting(void);
static rc testJoinGraph(void);
static rc testAdaptFDsForRule(void);
static rc testFDchecking(void);
static rc testInferFDsForIDB(void);

rc
testSemanticOptimization(void)
{
    RUN_TEST(testJoinGraph(), "testing creation of join graphs for datalog rules.");
	RUN_TEST(testRewriting(), "testing creation of capture rules.");
	RUN_TEST(testFDchecking(), "testing checking validty of FDs over body atoms.");
    RUN_TEST(testOptimization(), "testing optimizing a DL rule for provenance capture using semantic query optimization.");
	RUN_TEST(testAdaptFDsForRule(), "test adapting FDs to a FL rule by replacing attributes with variables.");
	RUN_TEST(testInferFDsForIDB(), "test inferring FDs that hold for IDB atoms.");

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

#define TEST_LINEAGE_REWRITE(_inp,_exp,_description)					\
	do {																\
		DLProgram *_inp_, *_ep_,*_ap_;									\
																		\
		_inp_ = (DLProgram *) parseFromStringdl(_inp);					\
		_ep_ = (DLProgram *) parseFromStringdl(_exp);					\
																		\
		_inp_ = (DLProgram *) analyzeDLModel((Node *) _inp_);			\
		_ep_ = (DLProgram *) analyzeDLModel((Node *) _ep_);				\
																		\
		_ap_ = rewriteDLForLinageCapture(_inp_);						\
		_ap_->rules = CONCAT_LISTS(_ap_->rules, _ap_->facts);			\
		_ap_ =  (DLProgram *) analyzeDLModel((Node *) _ap_);			\
																		\
		delAllProps((DLNode *) _inp_);									\
		delAllProps((DLNode *) _ep_);									\
		delAllProps((DLNode *) _ap_);									\
		_ep_->func = NIL;												\
		_ap_->func = NIL;												\
																		\
		INFO_DL_LOG("Expected program", _ep_);							\
	    INFO_DL_LOG("Actual program", _ap_);							\
		DEBUG_NODE_BEATIFY_LOG("Expected program", _ep_);				\
		DEBUG_NODE_BEATIFY_LOG("Actual program", _ap_);					\
																		\
		ASSERT_EQUALS_NODE(_ep_,_ap_,_description " : " _inp);			\
	} while (0)

#define TEST_LINEAGE_REWRITE_FILE(_inf,_expf)							\
	do {																\
		DLProgram *_inp_, *_ep_,*_ap_;									\
		char *_inq = readStringFromFile(_inf);							\
		char *_expq = readStringFromFile(_expf);						\
		_inp_ = (DLProgram *) parseFromStringdl(_inq);					\
		_ep_ = (DLProgram *) parseFromStringdl(_expq);					\
																		\
		_inp_ = (DLProgram *) analyzeDLModel((Node *) _inp_);			\
		_ep_ = (DLProgram *) analyzeDLModel((Node *) _ep_);				\
																		\
		_ap_ = rewriteDLForLinageCapture(_inp_);						\
		_ap_->rules = CONCAT_LISTS(_ap_->rules, _ap_->facts);			\
		_ap_ =  (DLProgram *) analyzeDLModel((Node *) _ap_);			\
																		\
		delAllProps((DLNode *) _inp_);									\
		delAllProps((DLNode *) _ep_);									\
		delAllProps((DLNode *) _ap_);									\
		_ep_->func = NIL;												\
		_ap_->func = NIL;												\
																		\
		INFO_DL_LOG("Expected program", _ep_);							\
		INFO_DL_LOG("Actual program", _ap_);							\
		DEBUG_NODE_BEATIFY_LOG("Expected program", _ep_);				\
		DEBUG_NODE_BEATIFY_LOG("Actual program", _ap_);					\
																		\
		char *_mes = formatMes("\n================================================================================\n%s rewritten into %s:\n================================================================================\n\n%s\n================================================================================\n%s", \
							   _inf, _expf, _inq, _expq);				\
		ASSERT_EQUALS_NODE(_ep_,_ap_,_mes);								\
	} while(0)

static rc
testRewriting(void)
{
	boolean origSubqueryMerge, origSemanticOpt;

	origSubqueryMerge = getBoolOption(OPTION_DL_MERGE_RULES);
	origSemanticOpt = getBoolOption(OPTION_DL_SEMANTIC_OPT);

	// tests without subquery merging and semantic optimization
	setBoolOption(OPTION_DL_MERGE_RULES, FALSE);
	setBoolOption(OPTION_DL_SEMANTIC_OPT, FALSE);

	TEST_LINEAGE_REWRITE(
		"Q(X) :- R(X,Y). ANS : Q. LINEAGE FOR R.",
		"Q(X) :- R(X,Y). PROV_R(X,Y) :- R(X,Y), Q(X). ANS : PROV_R.",
		"simple query");

	TEST_LINEAGE_REWRITE(
		"Q(X) :- R(X,Y), S(Y,Z). ANS : Q. LINEAGE FOR R.",
		"Q(X) :- R(X,Y), S(Y,Z). PROV_R(X,Y) :- R(X,Y), S(Y,Z), Q(X). ANS : PROV_R.",
		"join query");

	TEST_LINEAGE_REWRITE(
		"Q(X) :- R(X,Y), X < 5. ANS : Q. LINEAGE FOR R.",
		"Q(X) :- R(X,Y), X < 5. PROV_R(X,Y) :- R(X,Y), X < 5, Q(X). ANS : PROV_R.",
		"query with comparison");

	TEST_LINEAGE_REWRITE(
		"Q(X) :- R(X,Y), S(Y,Z). ANS : Q. RQ(20). LINEAGE FOR R FOR RESULTS FROM RQ.",
		"rq(20)."
        "q(x) :- r(x,y),s(y,z)."
        "prov_r(x,y) :- r(x,y),s(y,z),rq(x)."
		"ANS : PROV_R.",
		"join query filter provenance.");

	TEST_LINEAGE_REWRITE("Q(X) :- R(X,Y), R(X,Z). ANS : Q. LINEAGE FOR R.",
						 "q(x) :- r(x,y),r(x,z)."
						 "prov_r(x,y) :- r(x,y),r(x,z),q(x)."
						 "prov_r(x,z) :- r(x,y),r(x,z),q(x)."
						 "ANS: PROV_R.",
						 "computing provenance for self-join");

	TEST_LINEAGE_REWRITE("Q(Y,count(1)) :- R(X,Y). ANS : Q. LINEAGE FOR R.",
						 "q(y,count(1)) :- r(x,y)."
						 "prov_r(x,y) :- r(x,y),q(y,v1)."
						 "ANS: PROV_R.",
						 "single table single rule aggregation.");

	TEST_LINEAGE_REWRITE("Q(Y,count(1)) :- R(X,Y), S(X,Z). ANS : Q. LINEAGE FOR R.",
						 "Q(Y,count(1)) :- R(X,Y),S(X,Z)."
						 "PROV_R(X,Y) :- R(X,Y),S(X,Z),Q(Y,V1)."
						 "ANS: PROV_R.",
						 "join with aggregation.");

	TEST_LINEAGE_REWRITE("Q(X) :- S(X,Y), Q1(X). Q1(X) :- R(X,Y), R(X,Z). ANS : Q. LINEAGE FOR R.",
						 "q(x) :- s(x,y),q1(x)."
						 "q1(x) :- r(x,y),r(x,z)."
						 "prov_q1(x) :- s(x,y),q1(x),q(x)."
						 "prov_r(x,y) :- r(x,y),r(x,z),prov_q1(x)."
						 "prov_r(x,z) :- r(x,y),r(x,z),prov_q1(x)."
						 "ANS : PROV_R.",
						 "capture provenance for table not in top rule.");

	// TPC-H queries
	List *queries = LIST_MAKE(
				"q01_lineitem.dl",
				"q02_nation.dl",
				"q03_lineitem.dl",
				"q04_lineitem.dl",
				"q05_lineitem.dl",
				"q06_lineitem.dl",
				"q07_lineitem.dl",
				"q08_supplier.dl",
				"q09_lineitem.dl",
				"q10_lineitem.dl",
				"q11_supplier.dl",
				"q12_lineitem.dl",
				"q13_orders.dl",
				"q14_lineitem.dl",
				"q15_lineitem.dl",
				/* "q16_part.dl", */
				"q17_lineitem.dl",
				"q18_lineitem.dl",
				"q19_lineitem.dl",
				"q20_part.dl"
		);

	// check rewrites without optimizations
	FOREACH(char,c,queries)
	{
		char *qfile = CONCAT_STRINGS("./datalog/lineage/", c);
		char *resultqueryfile = CONCAT_STRINGS("./datalog/lineage/result_", c);

		TEST_LINEAGE_REWRITE_FILE(qfile, resultqueryfile);
	}

	// check rewrites with optimizations (merging subqueries and semantic
	// optimization)
	setBoolOption(OPTION_DL_MERGE_RULES, TRUE);
	setBoolOption(OPTION_DL_SEMANTIC_OPT, TRUE);

	queries = LIST_MAKE(
				"q01_lineitem.dl",
				"q02_nation.dl",
				"q03_lineitem.dl",
				"q04_lineitem.dl",
				"q05_lineitem.dl",
				"q06_lineitem.dl",
				"q07_lineitem.dl",
				"q08_supplier.dl",
				"q09_lineitem.dl",
				"q10_lineitem.dl",
				"q11_supplier.dl",
				"q12_lineitem.dl",
				"q13_orders.dl",
				"q14_lineitem.dl",
				"q15_lineitem.dl",
				"q17_lineitem.dl",
				"q18_lineitem.dl",
				"q19_lineitem.dl",
				"q20_part.dl"
		);

	FOREACH(char,c,queries)
	{
		char *qfile = CONCAT_STRINGS("./datalog/lineage/", c);
		char *resultqueryfile = CONCAT_STRINGS("./datalog/lineage/optresult_", c);

		TEST_LINEAGE_REWRITE_FILE(qfile, resultqueryfile);
	}

	// merge subqueries but do not use semantic optimization
	setBoolOption(OPTION_DL_MERGE_RULES, TRUE);
	setBoolOption(OPTION_DL_SEMANTIC_OPT, FALSE);

	queries = LIST_MAKE(
				"q01_lineitem.dl",
				"q02_nation.dl",
				"q03_lineitem.dl",
				"q04_lineitem.dl",
				"q05_lineitem.dl",
				"q06_lineitem.dl",
				"q07_lineitem.dl",
				"q08_supplier.dl",
				"q09_lineitem.dl",
				"q10_lineitem.dl",
				"q11_supplier.dl",
				"q12_lineitem.dl",
				"q13_orders.dl",
				"q14_lineitem.dl",
				"q15_lineitem.dl",
				"q17_lineitem.dl",
				"q18_lineitem.dl",
				"q19_lineitem.dl",
				"q20_part.dl"
		);

	FOREACH(char,c,queries)
	{
		char *qfile = CONCAT_STRINGS("./datalog/lineage/", c);
		char *resultqueryfile = CONCAT_STRINGS("./datalog/lineage/flatresult_", c);

		TEST_LINEAGE_REWRITE_FILE(qfile, resultqueryfile);
	}


	// reset optimization options
	setBoolOption(OPTION_DL_MERGE_RULES, origSubqueryMerge);
	setBoolOption(OPTION_DL_SEMANTIC_OPT, origSemanticOpt);

	return PASS;
}

#define LOG_AND_ASSERT_EQUALS(exp,opt) \
 	INFO_DL_LOG("expected and optimized rules: ", exp, opt); \
	DEBUG_NODE_BEATIFY_LOG("expected and optimized rules: ", exp, opt); \
	delAllProps((DLNode *) opt); \
	delAllProps((DLNode *) exp); \
	ASSERT_EQUALS_NODE(exp, opt, "optimized rule")


static rc
testOptimization(void)
{
	DLRule *r, *opt, *exp;
	List *fds;
	DLAtom *headx, *headxy, *headya, *gr,*gs, *gsp, *glr, *gt, *gu, *pr, *plr;
	DLProgram *p;

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
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	exp = createDLRule(pr,LIST_MAKE(headxy));
	opt = optimizeDLRule(p, r, fds, gr, NULL);

	LOG_AND_ASSERT_EQUALS(exp,opt);

	// Q(X) :- R(X,Y), S(Y,Z). R(a,b), S(c,d). X -> Y for R
	// optimized capture rule: R(X,Y) :- Q(X), R(X,Y).
	r = createDLRule(headx, LIST_MAKE(gr,gs));
	fds = LIST_MAKE(createFD("R", MAKE_STR_SET("A"), MAKE_STR_SET("B")));
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	exp = createDLRule(pr,LIST_MAKE(gr,headx));
	opt = optimizeDLRule(p, r, fds, gr, NULL);

	LOG_AND_ASSERT_EQUALS(exp,opt);

	// Q(X) :- PRR(X,Y,A), S(Y,Z), T(A,B), U(B,C). a->b for R
	// optimized capture rule: R(X,Y,A) :- Q(X), R(X,Y,A), T(A,B), U(B,C)
	r = createDLRule(headx, LIST_MAKE(glr,gs,gt,gu));
	fds = LIST_MAKE(createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("B")));
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	exp = createDLRule(plr,LIST_MAKE(glr,gt,gu,headx));
	opt = optimizeDLRule(p, r, fds, glr, NULL);

	LOG_AND_ASSERT_EQUALS(exp,opt);

	// Q(X) :- PRR(X,Y,A), S(Y,Z), T(A,B), U(B,C). x->y for R X -> A
	// optimized capture rule: R(X,Y,A) :- PRR(X,Y,A), Q(X)
	r = createDLRule(headx, LIST_MAKE(glr,gs,gt,gu));
	fds = LIST_MAKE(createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("B")),
					createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("C")));
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	exp = createDLRule(plr,LIST_MAKE(glr,headx));
	opt = optimizeDLRule(p, r, fds, glr, NULL);

	LOG_AND_ASSERT_EQUALS(exp,opt);

	// create comparison predicates
	DLComparison *y3, *z5, *xc;

	y3 = createDLComparison("<",
							(Node *) createDLVar("Y", DT_INT),
							(Node *) createConstInt(3));

	z5 = createDLComparison(">",
							(Node *) createDLVar("Z", DT_INT),
							(Node *) createConstInt(5));

	xc = createDLComparison("<",
							(Node *) createDLVar("X", DT_INT),
							(Node *) createDLVar("C", DT_INT));

	// Q(X) :- PRR(X,Y,A), S(Y,Z), T(A,B), U(B,C), Y < 3, Z > 5, X < C. X->Y,A (A-> B,C)
	// optimized capture rule: PRR(X,Y,A) :- Q(X), Y < 3, PRRR(X,Y,A)
	r = createDLRule(headx, LIST_MAKE(glr,gs,gt,gu,y3,z5,xc));
	fds = LIST_MAKE(createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("B")),
					createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("C")));
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	exp = createDLRule(plr,LIST_MAKE(glr,y3,headx));
	opt = optimizeDLRule(p, r, fds, glr, NULL);

	LOG_AND_ASSERT_EQUALS(exp,opt);

	// Q(Y,A) :- PRR(X,Y,A), S(X,Z), T(A,B), U(B,C), Y < 3, Z > 5, X < C. X->Y,A (A-> B,C)
	// optimized capture rule: PROV_PRR(X,Y,A) :- PRR(X,Y,A), S(X,Z), T(A,B), U(B,C), Y < 3, Z > 5, X < C, Q(X).
	gsp = DLATOM_FROM_STRS("S",FALSE,"X","Z");
	headya = DLATOM_FROM_STRS("Q", FALSE, "Y", "A");

	r = createDLRule(headya, LIST_MAKE(glr,gsp,gt,gu,y3,z5,xc));
	fds = LIST_MAKE(createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("B")),
					createFD("PRR", MAKE_STR_SET("A"), MAKE_STR_SET("C")));
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	exp = createDLRule(plr,LIST_MAKE(glr,gsp,gt,gu,y3,z5,xc,headya));
	opt = optimizeDLRule(p, r, fds, glr, NULL);

	LOG_AND_ASSERT_EQUALS(exp,opt);

	return PASS;
}

static rc
testAdaptFDsForRule(void)
{
	List *in, *exp, *out;
	DLRule *r;
	DLAtom *headx, *gr,*gs, *grt;
	DLProgram *p;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	grt = DLATOM_FROM_STRS("R",FALSE,"X","Z");

	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	headx = DLATOM_FROM_STRS("Q", FALSE, "X");

	r = createDLRule(headx, LIST_MAKE(gr,gs,grt));
    in = LIST_MAKE(createFD("R", MAKE_STR_SET("A"), MAKE_STR_SET("B")),
					createFD("S", MAKE_STR_SET("C"), MAKE_STR_SET("D")));
	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);

	out = adaptFDsToRules(p, r, in);
	exp = LIST_MAKE(createFD("R", MAKE_STR_SET("X"), MAKE_STR_SET("Y")),
					createFD("R", MAKE_STR_SET("X"), MAKE_STR_SET("Z")),
					createFD("S", MAKE_STR_SET("Y"), MAKE_STR_SET("Z")));

	ASSERT_EQUALS_NODE(exp,out, "x->y");

	return PASS;
}

static rc
testFDchecking(void)
{
	List *fds;
	Set *in;
	DLAtom *gr, *gs, *gt;
	FD *fd1, *fd2, *fd3, *fd4;

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	gt = DLATOM_FROM_STRS("T",FALSE,"X","A");

	fd1 = createFD("R", MAKE_STR_SET("X"), MAKE_STR_SET("Y"));
	fd2 = createFD("S", MAKE_STR_SET("Y"), MAKE_STR_SET("Z"));
	fd3 = createFD("T", MAKE_STR_SET("X"), MAKE_STR_SET("A"));
	fd4 = createFD(NULL, MAKE_STR_SET("X"), MAKE_STR_SET("Z"));

    fds = LIST_MAKE(fd1,fd2,fd3);

	in = MAKE_NODE_SET(gr);
	ASSERT_TRUE(checkFDonAtoms(in, fds, fd1), "X->Y on R");

	in = MAKE_NODE_SET(gs);
	ASSERT_FALSE(checkFDonAtoms(in, fds, fd1), "not X->Y on S");

	in = MAKE_NODE_SET(gr,gs);
	ASSERT_TRUE(checkFDonAtoms(in, fds, fd2), "Y->Z on R,S");

	in = MAKE_NODE_SET(gt);
	ASSERT_TRUE(checkFDonAtoms(in, fds, fd3), "X->Z on T");

	in = MAKE_NODE_SET(gr,gs);
	ASSERT_TRUE(checkFDonAtoms(in, fds, fd4), "X->Z on R,S");

	return PASS;
}

static rc
testInferFDsForIDB(void)
{
	DLRule *r, *r2;
	DLProgram *p;
	DLAtom *gr, *gs, *gq;
	List *fds, *actualFds, *expectedFds;
	FD *f1, *f2, *f3, *f4;

	f1 = createFD("R", MAKE_STR_SET("A"), MAKE_STR_SET("B"));
	f2 = createFD("S", MAKE_STR_SET("C"), MAKE_STR_SET("D"));
	f3 = createFD("Q", MAKE_STR_SET("A0"), MAKE_STR_SET("A1"));
	fds = LIST_MAKE(f1,f2);

	expectedFds = LIST_MAKE(f1,f2,f3);

	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");

	// Q(X,Z) :- R(X,Y), S(Y,Z). => X->Y, Y->Z implies X->Z for Q
	r = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Z"),
					 LIST_MAKE(gr,gs));

	p = createDLProgram(LIST_MAKE(r), NIL, "Q", NIL, NIL, NIL);
	setDLProp((DLNode *) p, DL_PROG_FDS, (Node *) fds);

    actualFds = inferFDsForProgram(p);

	ASSERT_EQUALS_NODE(expectedFds, actualFds, "FDs on Q: A0 -> A1");

	// add second rule
	gr = DLATOM_FROM_STRS("R",FALSE,"X","Y");
	gs = DLATOM_FROM_STRS("S",FALSE,"Y","Z");
	gq = DLATOM_FROM_STRS("Q",FALSE,"X","Y");

	r = createDLRule(DLATOM_FROM_STRS("Q", FALSE, "X", "Z"),
					 LIST_MAKE(gr,gs));
	r2 = createDLRule(DLATOM_FROM_STRS("Q2", FALSE, "Y", "X"),
					  LIST_MAKE(gq));
	p = createDLProgram(LIST_MAKE(r,r2), NIL, "Q2", NIL, NIL, NIL);
	setDLProp((DLNode *) p, DL_PROG_FDS, (Node *) fds);

	f4 = createFD("Q2", MAKE_STR_SET("A1"), MAKE_STR_SET("A0"));

	expectedFds = LIST_MAKE(f1,f2,f3,f4);
    actualFds = inferFDsForProgram(p);

	ASSERT_EQUALS_NODE(expectedFds, actualFds, "FDs on Q2: A1 -> A0");

	return PASS;
}
