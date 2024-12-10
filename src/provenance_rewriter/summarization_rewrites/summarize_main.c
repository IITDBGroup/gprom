/*-----------------------------------------------------------------------------
 *
 * summarize_main.c
 *
 *
 *		AUTHOR: seokki
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "instrumentation/timing_instrumentation.h"

#include "model/node/nodetype.h"
#include "utility/string_utils.h"
#include "model/datalog/datalog_model.h"
#include "model/expression/expression.h"
#include "sql_serializer/sql_serializer.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "model/query_operator/query_operator.h"
#include "operator_optimizer/operator_optimizer.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/query_operator_dt_inference.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "provenance_rewriter/summarization_rewrites/summarize_main.h"
#include "provenance_rewriter/prov_schema.h"

#define NUM_PROV_ATTR "NumInProv"
#define NUM_NONPROV_ATTR "NumInNonProv"
#define HAS_PROV_ATTR "HAS_PROV"
#define TOTAL_PROV_ATTR "TotalProv"
#define TOTAL_PROV_SAMP_ATTR "TotalProvInSamp"
#define TOTAL_NONPROV_SAMP_ATTR "TotalNonProvInSamp"
#define ACCURACY_ATTR "Precision"
#define COVERAGE_ATTR "Recall"
#define INFORMATIVE_ATTR "Informativeness"
#define FMEASURE_ATTR "Fmeasure"
#define COVERED_ATTR "Covered"
#define SAMP_NUM_PREFIX "SampNum"
#define NON_PROV_SIZE "nonProvSize"
#define SAMP_NUM_L_ATTR SAMP_NUM_PREFIX "Left"
#define SAMP_NUM_R_ATTR SAMP_NUM_PREFIX "Right"

static List *domAttrsOutput (Node *sampleInput, int sampleSize, ProvQuestion qType, HashMap *vrPair, List *domRels, List *fPattern);
static List *joinOnSeqOutput (List *doms);
//static int *computeSampleSize (int *samplePerc, Node *prov);

static Node *rewriteUserQuestion (List *userQ, Node *rewrittenTree);
static Node *rewriteProvJoinOutput (Node *rewrittenTree, boolean nonProvOpt);
static Node *rewriteRandomProvTuples (Node *provExpl, int sampleSize, ProvQuestion qType, List *fPattern, boolean nonProvOpt);
static Node *rewriteRandomNonProvTuples (Node *provExpl, ProvQuestion qType, List *fPattern);
static Node *rewriteSampleOutput (Node *randProv, Node *randNonProv, int sampleSize, ProvQuestion qType);
static Node *rewritePatternOutput (char *summaryType, Node *unionSample, Node *randProv);
static Node *rewriteScanSampleOutput (Node *sampleInput, Node *patternInput);
static Node *rewriteCandidateOutput (Node *scanSampleInput, ProvQuestion qType, List *fPattern, boolean nonProvOpt);
static Node *scaleUpOutput (List *doms, Node *candInput, Node *provJoin, Node *randProv, Node *randNonProv);
static Node *replaceDomWithSampleDom (List *sampleDoms, List *domRels, Node *input);
static Node *rewriteComputeFracOutput (Node *scaledCandInput, Node *sampleInput, ProvQuestion qType);
static Node *rewritefMeasureOutput (Node *computeFracInput, float sPrec, float sRec, float sInfo, float thPrec, float thRec, float thInfo);
static Node *rewriteTopkExplOutput (Node *fMeasureInput, int topK);
static Node *integrateWithEdgeRel (Node *topkInput, Node *moveRels);

static List *provAttrs = NIL;
static List *normAttrs = NIL;
static List *userQuestion = NIL;
static List *origDataTypes = NIL;
static List *givenConsts = NIL;
static boolean isDL = FALSE;
//static int givenConsts = 0;


Node *
rewriteSummaryOutput (Node *rewrittenTree, HashMap *summOpts, ProvQuestion qType)
{
	/*
	 * collect options for summarization
	 * e.g., top-k, type computing patterns, sample size, and so on
	 */
	List *fPattern = NIL;
	char *summaryType = NULL;
	int sampleSize = 0;
	int topK = 0;

//	Node *score = NULL;
	float sPrecision = 0;
	float sRecall = 0;
	float sInfo = 0;

	float thPrecision = 0;
	float thRecall = 0;
	float thInfo = 0;

	HashMap *varRelPair = NULL;

	// reset global variables
	provAttrs = NIL;
	provAttrs = NIL;
	normAttrs = NIL;
	userQuestion = NIL;
	origDataTypes = NIL;
	givenConsts = NIL;
	isDL = FALSE;


	if (summOpts != NULL)
	{
	    INFO_LOG(" * do summarization rewrite");
	    FOREACH_HASH_ENTRY(n,summOpts)
		{
	        KeyValue *kv = (KeyValue *) n;
	        char *key = STRING_VALUE(kv->key);

	        if(streq(key,PROP_SUMMARIZATION_TOPK))
	            topK = INT_VALUE(kv->value);

	        // whynot only for PUG (not implemented for SQL yet)
	        if(qType == PROV_Q_WHYNOT && streq(key,PROP_SUMMARIZATION_FPATTERN))
	            fPattern = copyObject((List *) kv->value);

	        if(streq(key,PROP_SUMMARIZATION_TYPE))
	            summaryType = STRING_VALUE(kv->value);

	        if(streq(key,PROP_SUMMARIZATION_SAMPLE))
	            sampleSize = INT_VALUE(kv->value);

	        //				if(isPrefix(key,"score_"))
	        //					score = (Node *) kv->value;

            if(streq(key,PROP_SUMMARIZATION_SC_PRECISION))
                sPrecision = FLOAT_VALUE(kv->value);

            if(streq(key,PROP_SUMMARIZATION_SC_RECALL))
                sRecall = FLOAT_VALUE(kv->value);

            if(streq(key,PROP_SUMMARIZATION_SC_INFORMATIVENESS))
                sInfo = FLOAT_VALUE(kv->value);

            if(streq(key,PROP_SUMMARIZATION_TH_PRECISION))
                thPrecision = FLOAT_VALUE(kv->value);

            if(streq(key,PROP_SUMMARIZATION_TH_RECALL))
                thRecall = FLOAT_VALUE(kv->value);

            if(streq(key,PROP_SUMMARIZATION_TH_INFORMATIVENESS))
                thInfo = FLOAT_VALUE(kv->value);

			if(streq(key,PROP_SUMMARIZATION_VARREL))
				varRelPair = (HashMap *) n->value;

			if(streq(key,PROP_SUMMARIZATION_SAMPLE_PROPS))
			{
				List *explSamp = (List *) n->value;

				FOREACH(KeyValue,kv,explSamp)
				{
					char *key = STRING_VALUE(kv->key);

					if(streq(key,PROP_SUMMARIZATION_TYPE))
						summaryType = STRING_VALUE(kv->value);

					if(streq(key,PROP_SUMMARIZATION_TO_EXPLAIN))
						userQuestion = (List *) kv->value;

					if(streq(key,PROP_SUMMARIZATION_SAMPLE))
						sampleSize = INT_VALUE(kv->value);

//					if(isPrefix(key,"score_"))
//						score = (Node *) kv->value;

					if(streq(key,PROP_SUMMARIZATION_SC_PRECISION))
						sPrecision = FLOAT_VALUE(kv->value);

					if(streq(key,PROP_SUMMARIZATION_SC_RECALL))
						sRecall = FLOAT_VALUE(kv->value);

					if(streq(key,PROP_SUMMARIZATION_SC_INFORMATIVENESS))
						sInfo = FLOAT_VALUE(kv->value);

					if(streq(key,PROP_SUMMARIZATION_TH_PRECISION))
						thPrecision = FLOAT_VALUE(kv->value);

					if(streq(key,PROP_SUMMARIZATION_TH_RECALL))
						thRecall = FLOAT_VALUE(kv->value);

					if(streq(key,PROP_SUMMARIZATION_TH_INFORMATIVENESS))
						thInfo = FLOAT_VALUE(kv->value);
				}
			}
		}
	}

	DEBUG_LOG("summarization options are: qType: %s, fPattern: %s, summaryType: %s, topK: %u, sPrecision: %f, sRecall: %f, sInfo: %f, thPrecision: %f, thRecall: %f, thInfo: %f",
			  ProvQuestionToString(qType),
			  nodeToString(fPattern),
			  summaryType ? summaryType :"",
			  topK,
			  sPrecision,
			  sRecall,
			  sInfo,
			  thPrecision,
			  thRecall,
			  thInfo);

	/*
	 * store edge relation in separate
	 * TODO: not safe to check whether input comes from dl or SQL
	 */
	Node *moveRels = NULL;

	if(isA(rewrittenTree,List))
	{
		if(MAP_HAS_STRING_KEY(summOpts, PROP_SUMMARIZATION_IS_DL))
		/* if(isA(getHeadOfListP((List *) rewrittenTree), DuplicateRemoval)) */
		{
			isDL = TRUE;
			moveRels = (Node *) getTailOfListP((List *) rewrittenTree);

			// remove move rels from the input
			List *withTail = (List *) copyObject(rewrittenTree);
			List *noTail = removeFromTail(withTail);
			rewrittenTree = (Node *) noTail;
		}
	}

	/*
	 * summarization steps begin
	 */
	List *sampleDoms = NIL,
//		 *doms = NIL,
//		 *domRels = NIL,
		 *eachResults = NIL;
//	int *sampleSize = computeSampleSize(samplePerc,rewrittenTree);

	Node *result=NULL,
        *provJoin=NULL,
        *randomProv=NULL,
        *randomNonProv=NULL,
        *samples=NULL,
        *patterns=NULL,
//        *sampleDom=NULL,
        *whynotExpl=NULL,
        *scanSamples=NULL,
        *candidates=NULL,
        *scaleUp=NULL,
        *computeFrac=NULL,
        *fMeasure = NULL;

//	// if domain exists, then replace, e.g., even for why question which contains negated IDB atoms
//	if(isDL)
//	{
//	//	boolean isDomSampNecessary = FALSE;
//		List *inputs = (List *) rewrittenTree;
//
//		if(LIST_LENGTH(inputs) > 2)
//			for(int i = 0; i < LIST_LENGTH(inputs)-1; i++)
//				findTableAccessVisitor((Node *) getNthOfListP(inputs,i),&doms);
//		else
//			findTableAccessVisitor((Node *) getHeadOfListP(inputs),&doms);
//
//		FOREACH(TableAccessOperator,t,doms)
//			if(HAS_STRING_PROP(t,DL_IS_DOMAIN_REL) && !searchListString(domRels,t->tableName))
//				domRels = appendToTailOfList(domRels, t->tableName);
//	//			isDomSampNecessary = TRUE;
//	}

	/* make the non-prov set involve as optional
	 * TODO: connect to the parser if needed
	 */
	boolean nonProvOpt = FALSE;

	/*
	 *  compute summary for each rewritten input
	 */
	int rewrittenSize = 0;

	// check the size of the rewritten inputs
	if(isA(rewrittenTree,List))
		rewrittenSize = LIST_LENGTH((List *) rewrittenTree);
	else
		rewrittenSize = 1;

	/*
	 * compute summary for each input
	 * for example, Q(X) :- R(X,A,B), Q1(X). Q1(X) :- S(X,C,D). WHY(Q(1)).
	 * 1) first compute the summary of Q(X) (e.g., top 1 pattern is R(1,A,'a'))
	 * 2) then compute the summary of Q1(X) based on the top-k pattern(s) from Q(X)
	 * 	  i.e., propagate the condition B = 'a' while computing the summary of Q1(X)
	 * 	  e.g., top 1 pattern might look like S(1,C,2010), R(1,A,'a')
	 */
	for(int i = 0; i < rewrittenSize; i++)
	{
		Node *eachRewrittenTree = NULL;

		if(!isA(rewrittenTree,List))
			eachRewrittenTree = copyObject(rewrittenTree);
		else
			eachRewrittenTree = (Node *) getNthOfListP((List *) rewrittenTree,i);


		// check domain exists in each rewritten RA
		List *domRels = NIL, *doms = NIL;
		findTableAccessVisitor(eachRewrittenTree,&doms);

		FOREACH(TableAccessOperator,t,doms)
			if(HAS_STRING_PROP(t,DL_IS_DOMAIN_REL) && !searchListString(domRels,t->tableName))
				domRels = appendToTailOfList(domRels, t->tableName);

		/*
		 * Note that regardless of type of questions (why or why-not) if the domain is needed,
		 * then generate sample domain queries at the very first stage,
		 * e.g., idb negated atoms in the why question or positive atoms in the why-not
		 */
		if(!MY_LIST_EMPTY(domRels))
		{
			doms = domAttrsOutput(eachRewrittenTree, sampleSize, qType, varRelPair, domRels, fPattern);

			if(MY_LIST_EMPTY(sampleDoms))
				sampleDoms = joinOnSeqOutput(doms);
			else
				FOREACH(QueryOperator,qo,joinOnSeqOutput(doms))
					if(!searchListNode(sampleDoms,(Node *) qo))
						sampleDoms = appendToTailOfList(sampleDoms, qo);
		}

		/*
		 * for why questions,
		 * 1) take the prov result as an input
		 * 2) sample prov from the result
		 * 3) compute patterns over sample
		 * 4) scale up to compute approximate real recall
		 * 5) rank the patterns based on the computed fraction (e.g., recall and informativeness)
		 * 6) return the explanation based on the top-k patterns
		 */
		if(qType == PROV_Q_WHY)
		{
			if(!MY_LIST_EMPTY(domRels))
				eachRewrittenTree = replaceDomWithSampleDom(sampleDoms, domRels, eachRewrittenTree);

			if (!MY_LIST_EMPTY(userQuestion) && !isDL)
				eachRewrittenTree = rewriteUserQuestion(userQuestion, eachRewrittenTree);

			provJoin = rewriteProvJoinOutput(eachRewrittenTree, nonProvOpt);
			randomProv = rewriteRandomProvTuples(provJoin, sampleSize, qType, fPattern, nonProvOpt);

			if(nonProvOpt)
			{
				randomNonProv = rewriteRandomNonProvTuples(provJoin, qType, fPattern);
				samples = rewriteSampleOutput(randomProv, randomNonProv, sampleSize, qType);
			}
			else
				samples = randomProv;

			patterns = rewritePatternOutput(summaryType, samples, randomProv); //TODO: different types of pattern generation
			scanSamples = rewriteScanSampleOutput(samples, patterns);
			candidates = rewriteCandidateOutput(scanSamples, qType, fPattern, nonProvOpt);

			if(nonProvOpt)
			{
	//			isDomSampNecessary = FALSE;
				doms = domAttrsOutput(eachRewrittenTree, sampleSize, qType, varRelPair, domRels, fPattern);
			}

			scaleUp = scaleUpOutput(doms, candidates, provJoin, randomProv, randomNonProv);
			computeFrac = rewriteComputeFracOutput(scaleUp, randomProv, qType);
			fMeasure = rewritefMeasureOutput(computeFrac, sPrecision, sRecall, sInfo, thPrecision, thRecall, thInfo);
			result = rewriteTopkExplOutput(fMeasure, topK);
		}
		/*
		 * for why-not questions,
		 * 1) replace the domain part with the domain created over sampling
		 * 2) compute patterns over sample
		 * 3) rank the patterns based on the computed fraction (e.g., recall and informativeness)
		 * 4) return the explanation based on the top-k patterns
		 */
		else if(qType == PROV_Q_WHYNOT)
		{
	//		doms = domAttrsOutput(rewrittenTree, sampleSize, qType, varRelPair);
	//		sampleDom = joinOnSeqOutput(doms);
			whynotExpl = replaceDomWithSampleDom(sampleDoms, domRels, eachRewrittenTree);
			randomProv = rewriteRandomProvTuples(whynotExpl, sampleSize, qType, fPattern, nonProvOpt);

			if(nonProvOpt)
			{
				randomNonProv = rewriteRandomNonProvTuples(whynotExpl, qType, fPattern);
				samples = rewriteSampleOutput(randomProv, randomNonProv, sampleSize, qType);
			}
			else
				samples = randomProv;

			patterns = rewritePatternOutput(summaryType, samples, randomProv); //TODO: different types of pattern generation
			scanSamples = rewriteScanSampleOutput(samples, patterns);
			candidates = rewriteCandidateOutput(scanSamples, qType, fPattern, nonProvOpt);
			computeFrac = rewriteComputeFracOutput(candidates, randomProv, qType);
			fMeasure = rewritefMeasureOutput(computeFrac, sPrecision, sRecall, sInfo, thPrecision, thRecall, thInfo);
			result = rewriteTopkExplOutput(fMeasure, topK);
		}

		eachResults = appendToTailOfList(eachResults, result);

		// swith the question type by the existence of domain
		if(qType == PROV_Q_WHY && !MY_LIST_EMPTY(domRels))
			qType = PROV_Q_WHYNOT;
//		else if(streq(qType,"WHYNOT") && !MY_LIST_EMPTY(domRels))
//			qType = "WHY";
	}

	// make the collected each result as a whole final result
	result = (Node *) eachResults;

	/*
	 * integrate with the edge relation
	 */
	if (moveRels != NULL)
		result = integrateWithEdgeRel(result, moveRels);

    // apply casts where necessary
    if (isA(result, List))
    {
        List *translation = (List *) result;
        FOREACH(QueryOperator,c,translation)
        {
            introduceCastsWhereNecessary(c);
//            ASSERT(checkModel(c));
        }
    }
    else if (IS_OP(result))
    {
        introduceCastsWhereNecessary((QueryOperator *) result);
//        ASSERT(checkModel((QueryOperator *) result));
    }

	if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
		ASSERT(checkModel((QueryOperator *) result));

	return result;
}



/*
 * integrate with edge rel for dl
 */
static Node *
integrateWithEdgeRel(Node * topkInput, Node *moveRels)
{
	Node *result;
	QueryOperator *edgeRels = (QueryOperator *) moveRels;

	// store table access operator for later use of dom attrs
	List *rels = NIL;
	findTableAccessVisitor((Node *) edgeRels,&rels);

	List *distinctRels = NIL;
	FOREACH(TableAccessOperator,t,rels)
		if(!searchListNode(distinctRels,(Node *) createConstString(t->tableName)))
			distinctRels = appendToTailOfList(distinctRels,createConstString(t->tableName));

	// capture the attribute original attr refs
	TableAccessOperator *qo = (TableAccessOperator *) getHeadOfListP(rels);
	QueryOperator *parent = (QueryOperator *) getHeadOfListP(qo->op.parents);
	QueryOperator *grandParent = (QueryOperator *) getHeadOfListP(parent->parents);
	List *origAttrDefs = grandParent->schema->attrDefs;

	// only prov attrs are needed (create projection operator)
	int newBasePos = 0;
	HashMap *relToNewbase = NEW_MAP(Constant,Node);
	List *projExpr = NIL;
	List *mAttrDefs = NIL;

	FOREACH(QueryOperator,newEdgeBase,(List *) topkInput)
	{
		int pos = 0;
		projExpr = NIL;
		mAttrDefs = NIL;

		List *attrNames = NIL;
		List *measures = NIL;
		List *measureAttrs = NIL;

		FOREACH(AttributeDef,a,newEdgeBase->schema->attrDefs)
		{
			if(isPrefix(a->attrName,PROV_ATTR_PREFIX) || a->dataType == DT_BOOL)
			{
				projExpr = appendToTailOfList(projExpr,
						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

				attrNames = appendToTailOfList(attrNames, a->attrName);
			}

			if(streq(a->attrName,"NumInProv") || streq(a->attrName,"Recall"))
			{
				measures = appendToTailOfList(measures,
						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

				measureAttrs = appendToTailOfList(measureAttrs, a->attrName);
				mAttrDefs = appendToTailOfList(mAttrDefs,a);
			}

			pos++;
		}

		projExpr = CONCAT_LISTS(projExpr,measures);
		attrNames = CONCAT_LISTS(attrNames,measureAttrs);

	//	// bring the failure pattern back before merge with edge rel for WHYNOT
	//	if(!MY_LIST_EMPTY(fPattern))
	//	{
	//		FOREACH(Node,n,fPattern)
	//		{
	//			projExpr = appendToTailOfList(projExpr, n);
	//			attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("A",gprom_itoa(pos)));
	//			pos++;
	//		}
	//
	//		List *userQdt = NIL;
	//		FOREACH(AttributeDef,a,userQuestion)
	//			userQdt = appendToTailOfListInt(userQdt, a->dataType);
	//
	//		origDataTypes = CONCAT_LISTS(userQdt, origDataTypes);
	//	}

		// case when attr is null then * else attr value
		List *caseExprs = NIL;
		pos = 0;

		FOREACH(Node,n,projExpr)
		{
			if(pos < LIST_LENGTH(origAttrDefs))
			{
				// if the attr is number or constant, then make it char
				FunctionCall *toChar;

				// use attr names for placeholders
				AttributeDef *a = (AttributeDef *) getNthOfListP(origAttrDefs,pos);
				char *attrAs = strdup(a->attrName);

		//		if(isA(n,Constant) || ((DataType *) getNthOfListP(origDataTypes,pos)) == DT_INT)
		//		{
					if(isA(n,Constant))
						attrAs = "*";

					toChar = createFunctionCall("TO_CHAR", singleton(n));
					n = (Node *) toChar;
		//		}

				Node *cond = (Node *) createIsNullExpr((Node *) n);
				Node *then = (Node *) createConstString(attrAs);

				CaseWhen *caseWhen = createCaseWhen(cond, then);
				CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), n);
				caseExprs = appendToTailOfList(caseExprs, (List *) caseExpr);
			}
			else
				caseExprs = appendToTailOfList(caseExprs, n);

			pos++;
		}

		ProjectionOperator *op = createProjectionOp(caseExprs, newEdgeBase, NIL, attrNames);
		newEdgeBase->parents = singleton(op);
		newEdgeBase = (QueryOperator *) op;

		MAP_ADD_STRING_KEY(relToNewbase,STRING_VALUE(getNthOfListP(distinctRels,newBasePos)),newEdgeBase);
		newBasePos++;
	}

	FOREACH(TableAccessOperator,t,rels)
	{
		if(MAP_HAS_STRING_KEY(relToNewbase,t->tableName))
		{
			// replace attr defs of parent and grand parent
			QueryOperator *par = (QueryOperator *) getHeadOfListP(t->op.parents);
			par->schema->attrDefs = CONCAT_LISTS(par->schema->attrDefs, mAttrDefs);

			projExpr = NIL;
			int pos = 0;

			FOREACH(AttributeDef,a,par->schema->attrDefs)
			{
				projExpr = appendToTailOfList(projExpr,
						createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
				pos++;
			}

			QueryOperator *gp = (QueryOperator *) getHeadOfListP(par->parents);
			((ProjectionOperator *) gp)->projExprs = projExpr;
			gp->schema->attrDefs = CONCAT_LISTS(gp->schema->attrDefs, mAttrDefs);

			Node *value = MAP_GET_STRING(relToNewbase,t->tableName);
			switchSubtreeWithExisting((QueryOperator *) t,(QueryOperator *) value);
			DEBUG_LOG("replaced table %s with\n:%s", t->tableName, operatorToOverviewString(value));
		}
	}

	/*
	 *  create new node for measures (numInProv,recall)
	 *  TODO: find the better algorithm
	 *  	currently, copy the projection operator from the grand child of rule -> goal node
	 *  	to create another rule -> measure node
	 */
	QueryOperator *child = (QueryOperator *) getHeadOfListP(edgeRels->inputs);
	QueryOperator *grandChild = (QueryOperator *) getHeadOfListP(child->inputs);

	QueryOperator *dup = (QueryOperator *) getTailOfListP(grandChild->inputs);
	ProjectionOperator *po = (ProjectionOperator *) getHeadOfListP(dup->inputs);

	QueryOperator *newDup = copyObject(dup);
	ProjectionOperator *newNode = copyObject(po);

	newNode->op.parents = singleton(newDup);
	newNode->op.inputs = po->op.inputs;

	// create edge relation between rule and measure node
	Operator *o = (Operator *) getTailOfListP(newNode->projExprs);

	Node *bracketLeft = (Node *) createConstString("NUMPROVRECALL_WON(");
	Node *middle = (Node *) createConstString(",");
	Node *bracketRight = (Node *) createConstString(")");

	Node *numInProv = (Node *) getNthOfListP(projExpr,LIST_LENGTH(projExpr)-2);
	Node *recall = (Node *) getNthOfListP(projExpr,LIST_LENGTH(projExpr)-1);

	o->args = CONCAT_LISTS(singleton(bracketLeft),singleton(numInProv), singleton(middle),
			singleton(recall), singleton(bracketRight));
	newNode->projExprs = CONCAT_LISTS(singleton(getHeadOfListP(newNode->projExprs)), singleton(o));
//	newNode->projExprs = CONCAT_LISTS(singleton(o), singleton(o));
	newDup->inputs = singleton(newNode);

	//create union operator
	List *allInput = LIST_MAKE(getHeadOfListP(edgeRels->inputs),newDup);
	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION,allInput,NIL,getAttrNames(newDup->schema));
	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);
	edgeRels = unionOp;

	result = (Node *) edgeRels;

	DEBUG_NODE_BEATIFY_LOG("integrate top-k summaries with edge relation:", result);
	INFO_OP_LOG("integrate top-k summaries with edge relation as overview:", result);

	return result;
}


/*
 * return top-k explanations
 */
static Node *
rewriteTopkExplOutput (Node *fMeasureInput, int topK)
{
	Node *result;
	QueryOperator *topkOp = (QueryOperator *) fMeasureInput;

	// create selection for returning top most general explanation
	Node *selCond = NULL;

	if (topK != 0)
		selCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(makeNode(RowNumExpr),createConstInt(topK)));
	else
		selCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(makeNode(RowNumExpr),createConstInt(1))); // TODO: top1 or more?

	SelectionOperator *so = createSelectionOp(selCond, topkOp, NIL, getAttrNames(topkOp->schema));

	topkOp->parents = singleton(so);
	topkOp = (QueryOperator *) so;

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,a,topkOp->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	op = createProjectionOp(projExpr, topkOp, NIL, getAttrNames(topkOp->schema));
	topkOp->parents = singleton(op);
	topkOp = (QueryOperator *) op;

	result = (Node *) topkOp;

	DEBUG_NODE_BEATIFY_LOG("top-k summarized explanation from summarization:", result);
	INFO_OP_LOG("top-k summarized explanation from summarization as overview:", result);

	return result;
}



/*
 * compute f-measure based on precision and recall
 */
static Node *
rewritefMeasureOutput (Node *computeFracInput, float sPrec, float sRec, float sInfo, float thPrec, float thRec, float thInfo)
{
	Node *result;
	QueryOperator *fMeasure = (QueryOperator *) computeFracInput;

//	// where clause to filter out the pattern that only contains user question info
//	int aPos = 0;
//	int count = 1;
//	AttributeReference *lA, *rA = NULL;
//	Node *whereCond = NULL;
//
//	FOREACH(AttributeDef,a,fMeasure->schema->attrDefs)
//	{
//		if(isPrefix(a->attrName, "use"))
//		{
//			if(count % 2 != 0)
//				lA = createFullAttrReference(strdup(a->attrName), 0, aPos, 0, a->dataType);
//			else
//				rA = createFullAttrReference(strdup(a->attrName), 0, aPos, 0, a->dataType);
//
//			if(lA != NULL && rA != NULL)
//			{
//				Node *pairCond = (Node *) createOpExpr("+",LIST_MAKE(lA,rA));
//
//				if(whereCond != NULL)
//					whereCond = (Node *) createOpExpr("+",LIST_MAKE(whereCond,pairCond));
//				else
//					whereCond = copyObject(pairCond);
//
//				lA = NULL;
//				rA = NULL;
//			}
//			count++;
//		}
//		aPos++;
//	}
//
//	// add the last attr
//	if(lA != NULL && rA == NULL)
//		whereCond = (Node *) createOpExpr("+",LIST_MAKE(whereCond,lA));
//
//	int maxNum = count - LIST_LENGTH(userQuestion) - 1;
//	Node *filterCond = (Node *) createOpExpr(OPNAME_LT,LIST_MAKE(whereCond,createConstInt(maxNum)));
//
//	SelectionOperator *so = createSelectionOp(filterCond, fMeasure, NIL, getAttrNames(fMeasure->schema));
//	fMeasure->parents = appendToTailOfList(fMeasure->parents,so);
//	fMeasure = (QueryOperator *) so;

	// projection operator with a f-measure computation
	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
	AttributeReference *prec = NULL, *rec = NULL, *info = NULL;

	FOREACH(AttributeDef,a,fMeasure->schema->attrDefs)
	{
		if(streq(a->attrName, ACCURACY_ATTR))
			prec = createFullAttrReference(strdup(ACCURACY_ATTR), 0, pos, 0, a->dataType);

		if(streq(a->attrName, COVERAGE_ATTR))
			rec = createFullAttrReference(strdup(COVERAGE_ATTR), 0, pos, 0, a->dataType);

		if(streq(a->attrName, INFORMATIVE_ATTR))
			info = createFullAttrReference(strdup(INFORMATIVE_ATTR), 0, pos, 0, a->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	// use given score function, otherwise f-measure is default
	Node *fmeasure = NULL;
	List *attrNames = NIL;

	if(sPrec == 0 && sRec == 0 && sInfo == 0)
	{
		// add 2 measures into the computation
		Node *times = (Node *) createOpExpr("*",LIST_MAKE(prec,rec));
		Node *plus = (Node *) createOpExpr("+",LIST_MAKE(prec,rec));

		// add third measure into the computation
		Node *newtimes = (Node *) createOpExpr("*",LIST_MAKE(times,info));
		Node *newplus = (Node *) createOpExpr("+",LIST_MAKE(plus,info));

		// compute f-measure
		Node *cal = (Node *) createOpExpr("/",LIST_MAKE(newtimes,newplus));
		fmeasure = (Node *) createOpExpr("*",LIST_MAKE(createConstInt(3),cal));
	}
	else
	{
		Node *precCond = NULL;
		Node *recCond = NULL;
		Node *infoCond = NULL;

		if(sPrec != 0)
		{
			precCond = (Node *) createOpExpr("*",LIST_MAKE(prec,createConstFloat(sPrec)));
			if(fmeasure != NULL)
				fmeasure = (Node *) createOpExpr("+",LIST_MAKE(fmeasure,precCond));
			else
				fmeasure = precCond;
		}

		if(sRec != 0)
		{
			recCond = (Node *) createOpExpr("*",LIST_MAKE(rec,createConstFloat(sRec)));
			if(fmeasure != NULL)
				fmeasure = (Node *) createOpExpr("+",LIST_MAKE(fmeasure,recCond));
			else
				fmeasure = recCond;
		}

		if(sInfo != 0)
		{
			infoCond = (Node *) createOpExpr("*",LIST_MAKE(info,createConstFloat(sInfo)));
			if(fmeasure != NULL)
				fmeasure = (Node *) createOpExpr("+",LIST_MAKE(fmeasure,infoCond));
			else
				fmeasure = infoCond;
		}
	}

	projExpr = appendToTailOfList(projExpr, fmeasure);
	attrNames = CONCAT_LISTS(getAttrNames(fMeasure->schema), singleton(FMEASURE_ATTR));
	op = createProjectionOp(projExpr, fMeasure, NIL, attrNames);

	fMeasure->parents = singleton(op);
	fMeasure = (QueryOperator *) op;

	// add selection if thresholds are given
	if(thPrec != 0 || thRec != 0 || thInfo != 0)
	{
		Node *precCond = NULL;
		Node *recCond = NULL;
		Node *infoCond = NULL;
		Node *cond = NULL;

		if(thPrec != 0)
		{
			precCond = (Node *) createOpExpr(OPNAME_GE,LIST_MAKE(prec,createConstFloat(thPrec)));
			if(cond != NULL)
				cond = AND_EXPRS(cond,precCond);
			else
				cond = precCond;
		}

		if(thRec != 0)
		{
			recCond = (Node *) createOpExpr(OPNAME_GE,LIST_MAKE(rec,createConstFloat(thRec)));
			if(cond != NULL)
				cond = AND_EXPRS(cond,recCond);
			else
				cond = recCond;
		}

		if(thInfo!= 0)
		{
			infoCond = (Node *) createOpExpr(OPNAME_GE,LIST_MAKE(info,createConstFloat(thInfo)));
			if(cond != NULL)
				cond = AND_EXPRS(cond,infoCond);
			else
				cond = infoCond;
		}

		SelectionOperator *so = createSelectionOp(cond, fMeasure, NIL, getAttrNames(fMeasure->schema));
		fMeasure->parents = singleton(so);
		fMeasure = (QueryOperator *) so;
	}

	// add projection for ORDER BY
	pos = 0;
	projExpr = NIL;

	FOREACH(AttributeDef,a,fMeasure->schema->attrDefs)
	{
		AttributeReference *ar = createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType);

		if(streq(a->attrName,FMEASURE_ATTR))
			fmeasure = (Node *) ar;

		projExpr = appendToTailOfList(projExpr, ar);
		pos++;
	}

	op = createProjectionOp(projExpr, fMeasure, NIL, attrNames);
	fMeasure->parents = singleton(op);
	fMeasure = (QueryOperator *) op;

	// create ORDER BY
	OrderExpr *fmeasureExpr = createOrderExpr(fmeasure, SORT_DESC, SORT_NULLS_LAST);
	OrderOperator *ord = createOrderOp(singleton(fmeasureExpr), fMeasure, NIL);

	fMeasure->parents = singleton(ord);
	fMeasure = (QueryOperator *) ord;

	result = (Node *) fMeasure;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("compute f-measure for summarization:", result);
	INFO_OP_LOG("compute f-measure for summarization as overview:", result);

	return result;
}


/*
 * compute precision and recall
 */
static Node *
rewriteComputeFracOutput (Node *scaledCandInput, Node *sampleInput, ProvQuestion qType)
{
	Node *result;
	QueryOperator *computeFrac = (QueryOperator *) scaledCandInput;

	if(qType == PROV_Q_WHYNOT)
	{
		QueryOperator *provSample = (QueryOperator *) sampleInput;

		// count hasProv
		Constant *countProv = createConstInt(1);
		FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
		fcCount->isAgg = TRUE;

		ProjectionOperator *projOp = createProjectionOp(singleton(fcCount), provSample, NIL, singleton(strdup(TOTAL_PROV_ATTR)));
		provSample->parents = singleton(projOp);
		provSample = (QueryOperator *) projOp;

		// create cross product
		List *inputs = LIST_MAKE(provSample, computeFrac);
		List *attrNames = CONCAT_LISTS(getAttrNames(provSample->schema), getAttrNames(computeFrac->schema));

		QueryOperator *cp = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
		OP_LCHILD(cp)->parents = OP_RCHILD(cp)->parents = singleton(cp);
		computeFrac = cp;
	}

//	// get total count for prov from samples
//	int aPos = LIST_LENGTH(samples->schema->attrDefs) - 1;
//	AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);
//
//	Node *whereClause = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lC,createConstInt(1)));
//	SelectionOperator *so = createSelectionOp(whereClause, samples, NIL, getAttrNames(samples->schema));
//
//	samples->parents = appendToTailOfList(samples->parents,so);
//	samples = (QueryOperator *) so;
//
//	// create projection operator
//	Constant *countProv = createConstInt(1);
//	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countProv));
//	fcCount->isAgg = TRUE;
//	//countProv->name = strdup(TOTAL_PROV_ATTR);
//
//	ProjectionOperator *op = createProjectionOp(singleton(fcCount), samples, NIL, singleton(strdup(TOTAL_PROV_ATTR)));
//	samples->parents = singleton(op);
//	samples = (QueryOperator *) op;
//
//	// cross product with candidates to compute
//	List *crossInput = LIST_MAKE(samples,candidates);
//	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(candidates->schema));
//	QueryOperator *computeFrac = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, crossInput, NIL, attrNames);
//
//	// set the parent of the operator's children
////	OP_LCHILD(computeFrac)->parents = OP_RCHILD(computeFrac)->parents = singleton(computeFrac);
//	samples->parents = appendToTailOfList(samples->parents,computeFrac);
//	candidates->parents = singleton(computeFrac);

	// create projection operator
	int pos = 0;
	List *projExpr = NIL;
	AttributeReference *totProv = NULL, *covProv = NULL, *numProv = NULL;

	FOREACH(AttributeDef,a,computeFrac->schema->attrDefs)
	{
		if (pos == 0)
			totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, pos, 0, a->dataType);

		if (pos == 1)
			covProv = createFullAttrReference(strdup(COVERED_ATTR), 0, pos, 0, a->dataType);

		if (pos == 2)
			numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, pos, 0, a->dataType);

		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
		pos++;
	}

	// round up after second decimal number
	Node *rdup = (Node *) createConstInt(10); // ???? why was that used: atoi("5"));

	// add attribute for accuracy
//	AttributeReference *numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, 2, 0, DT_INT);
//	AttributeReference *covProv = createFullAttrReference(strdup(COVERED_ATTR), 0, 1, 0, DT_INT);
	Node *precRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,covProv));
	FunctionCall *rdupAr = createFunctionCall("ROUND", LIST_MAKE(precRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupAr);

	// add attribute for coverage
//	AttributeReference *totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, 0, 0, DT_INT);
	Node* recRate = (Node *) createOpExpr("/",LIST_MAKE(numProv,totProv));
	FunctionCall *rdupCr = createFunctionCall("ROUND", LIST_MAKE(recRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupCr);

	// add attribute for informativeness
	int summArity = 0;
	int userQLen = LIST_LENGTH(userQuestion);
	int givenConstLen = LIST_LENGTH(givenConsts);
	List *attrPos = NIL;

	// store the position of attrs given
	FOREACH(AttributeReference,aOfg,givenConsts)
	{
		FOREACH(AttributeReference,aOfu,userQuestion)
		{
			if(aOfg->attrPosition < aOfu->attrPosition)
				aOfg->attrPosition = aOfg->attrPosition + 1;

			attrPos = appendToTailOfListInt(attrPos,aOfg->attrPosition);
		}
	}

	Node *newInfoOfSumm = NULL;
	AttributeReference *lA = NULL;
	AttributeReference *rA = NULL;

	FOREACH(AttributeReference,a,projExpr)
	{
		// after user question attributes, sum of the values of attributes to compute informativeness
		if (isPrefix(a->name,"use"))
		{
			if (summArity >= userQLen && !searchListInt(attrPos,summArity))
			{
				if(lA == NULL)
					lA = copyObject(a);
				else
					rA = copyObject(a);

				if(lA != NULL && rA != NULL)
				{
					Node *pairCond = (Node *) createOpExpr("+",LIST_MAKE(lA,rA));

//					if(infoSum != NULL)
//						infoSum = (Node *) createOpExpr("+",LIST_MAKE(infoSum,pairCond));
//					else
					newInfoOfSumm = copyObject(pairCond);

					lA = (AttributeReference *) newInfoOfSumm;
					rA = NULL;
				}
			}

			summArity++;
		}
	}

	if(lA != NULL)
		newInfoOfSumm = (Node *) lA;

	// compute the fraction of informativeness
//	Node *noGivenConsts = (Node *) createOpExpr("-",LIST_MAKE(infoSum,createConstInt(givenConsts)));
	Node *infoRate = (Node *) createOpExpr("/",LIST_MAKE(newInfoOfSumm,createConstInt(summArity-userQLen-givenConstLen)));
	FunctionCall *rdupInfo = createFunctionCall("ROUND", LIST_MAKE(infoRate, rdup));
	projExpr = appendToTailOfList(projExpr, rdupInfo);

	List *attrNames = NIL;
	attrNames = CONCAT_LISTS(getAttrNames(computeFrac->schema), singleton(ACCURACY_ATTR),
			singleton(COVERAGE_ATTR), singleton(INFORMATIVE_ATTR));
	ProjectionOperator *op = createProjectionOp(projExpr, computeFrac, NIL, attrNames);
	computeFrac->parents = singleton(op);
	computeFrac = (QueryOperator *) op;

//	// create ORDER BY
//	// TODO: probably put another projection for order by operation
////	AttributeReference *accuR = createFullAttrReference(strdup(ACCURACY_ATTR), 0,
////							LIST_LENGTH(computeFrac->schema->attrDefs) - 2, 0, DT_INT);
//
//	OrderExpr *accExpr = createOrderExpr(precRate, SORT_DESC, SORT_NULLS_LAST);
//	OrderExpr *covExpr = createOrderExpr(recRate, SORT_DESC, SORT_NULLS_LAST);
//
//	OrderOperator *ord = createOrderOp(LIST_MAKE(accExpr, covExpr), computeFrac, NIL);
//	computeFrac->parents = singleton(ord);
//	computeFrac = (QueryOperator *) ord;

	result = (Node *) computeFrac;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("compute fraction for summarization:", result);
	INFO_OP_LOG("compute fraction for summarization as overview:", result);

	return result;
}



/*
 * for WHY, scale up the measure values to real one
 */
static Node *
scaleUpOutput (List *doms, Node *candInput, Node *provJoin, Node *randSamp, Node *randNonSamp)
{
	Node *result;
	List *inputs = NIL;
	List *attrNames = NIL;

	// inputs for computing scale up
	QueryOperator *candidates = (QueryOperator *) candInput;
	QueryOperator *provQuery = (QueryOperator *) provJoin;
	QueryOperator *sampProv = (QueryOperator *) randSamp;
	QueryOperator *sampNonProv = NULL;

	if(randNonSamp != NULL)
		sampNonProv = (QueryOperator *) randNonSamp;

	// store candidates and doms as inputs for cross product later
	SET_BOOL_STRING_PROP((Node *) candidates, PROP_MATERIALIZE);
	inputs = appendToTailOfList(inputs, (Node *) candidates);
	attrNames = getAttrNames(candidates->schema);

	// generate sub-queries for 1) totalProv
	int aPos = LIST_LENGTH(provQuery->schema->attrDefs) - 1;
	AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

	Node *cond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lC,createConstInt(1)));
	SelectionOperator *so = createSelectionOp(cond, provQuery, NIL, getAttrNames(provQuery->schema));

	provQuery->parents = singleton(so);
	provQuery = (QueryOperator *) so;

	Constant *countTProv = createConstInt(1);
	FunctionCall *fcTp = createFunctionCall("COUNT", singleton(countTProv));
	fcTp->isAgg = TRUE;

	AggregationOperator *totalProv = createAggregationOp(singleton(fcTp), NIL, provQuery, NIL, singleton(strdup(TOTAL_PROV_ATTR)));
	SET_BOOL_STRING_PROP((Node *) totalProv, PROP_MATERIALIZE);
	inputs = appendToTailOfList(inputs, (Node *) totalProv);
	attrNames = appendToTailOfList(attrNames, strdup(TOTAL_PROV_ATTR));

	// create cross product for provQuery and totalProv
	QueryOperator *provQtotalProv = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
	candidates->parents = singleton(provQtotalProv);
	((QueryOperator *) totalProv)->parents = singleton(provQtotalProv);

	// 2) totalProvInSamp
	int gPos = LIST_LENGTH(sampProv->schema->attrDefs) - 1;
	AttributeReference *TProvInSamp = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
	FunctionCall *fcTps = createFunctionCall("COUNT", singleton(TProvInSamp));
	fcTps->isAgg = TRUE;

	AggregationOperator *totalProvInSamp = createAggregationOp(singleton(fcTps), NIL, sampProv, NIL, singleton(strdup(TOTAL_PROV_SAMP_ATTR)));
	SET_BOOL_STRING_PROP((Node *) totalProvInSamp, PROP_MATERIALIZE);
	inputs = LIST_MAKE(provQtotalProv, totalProvInSamp);
	attrNames = appendToTailOfList(attrNames, strdup(TOTAL_PROV_SAMP_ATTR));

	// create cross product for provQtotalProv and totalProvInSamp
	QueryOperator *crossPtotalProvInSamp = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
	provQtotalProv->parents = singleton(crossPtotalProvInSamp);
	((QueryOperator *) totalProvInSamp)->parents = singleton(crossPtotalProvInSamp);

	QueryOperator *crossPdom = NULL;

	if(sampNonProv != NULL)
	{
		// 3) totalNonProvInSamp
		gPos = LIST_LENGTH(sampNonProv->schema->attrDefs) - 1;
		AttributeReference *TNonProvInSamp = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
		FunctionCall *fcTnps = createFunctionCall("COUNT", singleton(TNonProvInSamp));
		fcTnps->isAgg = TRUE;

		AggregationOperator *totalNonProvInSamp = createAggregationOp(singleton(fcTnps), NIL, sampNonProv, NIL, singleton(strdup(TOTAL_NONPROV_SAMP_ATTR)));
		SET_BOOL_STRING_PROP((Node *) totalNonProvInSamp, PROP_MATERIALIZE);
		inputs = LIST_MAKE(crossPtotalProvInSamp, totalNonProvInSamp);
		attrNames = appendToTailOfList(attrNames, strdup(TOTAL_NONPROV_SAMP_ATTR));

		// create cross product for provQtotalProv and totalNonProvInSamp
		QueryOperator *crossPtotalNonProvInSamp = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);
		crossPtotalProvInSamp->parents = singleton(crossPtotalNonProvInSamp);
		((QueryOperator *) totalNonProvInSamp)->parents = singleton(crossPtotalNonProvInSamp);

		// add cross product for doms
		for(int i = 0; i < LIST_LENGTH(doms); i++)
		{
			Node *n = (Node *) getNthOfListP(doms,i);
			SET_BOOL_STRING_PROP(n, PROP_MATERIALIZE);

			if(i == 0)
				inputs = LIST_MAKE(crossPtotalNonProvInSamp, n);
			else
				inputs = LIST_MAKE(crossPdom, n);

			QueryOperator *oDom = (QueryOperator *) n;
			attrNames = concatTwoLists(attrNames, getAttrNames(oDom->schema));

			// create cross product for provQuery and doms
			crossPdom = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);

			if(i == 0)
				crossPtotalNonProvInSamp->parents = singleton(crossPdom);
			else
				OP_LCHILD(crossPdom)->parents = singleton(crossPdom);

			oDom->parents = singleton(crossPdom);
		}
	}
	else
		crossPdom = crossPtotalProvInSamp;

	/*
	 * create projection operator for computing
	 * p = numInProv * totalProv / totalProvInSamp
	 * np = numInNonProv * (domA * domB ... * domN - totalProv) / totalNonProvInSamp
	 * p + np = covered in real dataset
	 * p = numInProv in real dataset
	 */

	int pos = 0;
	int counter = 0;
	List *projExpr = NIL;
	List *attrs = NIL;
	Node *crossDoms = NULL;
	AttributeReference *totProv = NULL,
	                    *numProv = NULL,
	                    *numNonProv = NULL,
	                    *totProvInSamp = NULL,
	                    *totNonProvInSamp = NULL,
	                    *domL = NULL,
	                    *domR = NULL;

	FOREACH(AttributeDef,a,crossPdom->schema->attrDefs)
	{
		if(streq(a->attrName,TOTAL_PROV_ATTR))
			totProv = createFullAttrReference(strdup(TOTAL_PROV_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,NUM_PROV_ATTR))
			numProv = createFullAttrReference(strdup(NUM_PROV_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,NUM_NONPROV_ATTR))
			numNonProv = createFullAttrReference(strdup(NUM_NONPROV_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,TOTAL_PROV_SAMP_ATTR))
			totProvInSamp = createFullAttrReference(strdup(TOTAL_PROV_SAMP_ATTR), 0, pos, 0, a->dataType);
		else if(streq(a->attrName,TOTAL_NONPROV_SAMP_ATTR))
			totNonProvInSamp = createFullAttrReference(strdup(TOTAL_NONPROV_SAMP_ATTR), 0, pos, 0, a->dataType);
		else if(isPrefix(a->attrName,"cnt"))
		{
			if(counter == 0)
			{
				domL = createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType);
				counter++;
			}
			else
				domR = createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType);

			if(domL != NULL && domR != NULL)
			{
				crossDoms = (Node *) createOpExpr("*",LIST_MAKE(domL,domR));
				domL = (AttributeReference *) crossDoms;
				domR = NULL;
			}
		}
		else
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

			attrs = appendToTailOfList(attrs, a);
		}

		pos++;
	}

	// create numInProv, covered, and totalProv, and add to the head of the projExpr
	Node *subNumProv = (Node *) createOpExpr("*",LIST_MAKE(numProv,totProv));
	Node *numInProv = (Node *) createOpExpr("/",LIST_MAKE(subNumProv,totProvInSamp));
	projExpr = appendToHeadOfList(projExpr, numInProv);

	Node *numInNonProv = NULL;

	if(crossDoms != NULL)
	{
		Node *nonProv = (Node *) createOpExpr("-",LIST_MAKE(crossDoms,totProv));
		Node *scaleNonProv = (Node *) createOpExpr("*",LIST_MAKE(numNonProv,nonProv));
		numInNonProv = (Node *) createOpExpr("/",LIST_MAKE(scaleNonProv,totNonProvInSamp));
	}
	else
		numInNonProv = (Node *) numNonProv;

	Node *numCov = (Node *) createOpExpr("+",LIST_MAKE(numInProv,numInNonProv));
	projExpr = appendToHeadOfList(projExpr, numCov);
	projExpr = appendToHeadOfList(projExpr, totProv);

	// create projection for candidates with real measure values
	attrNames = CONCAT_LISTS(singleton(TOTAL_PROV_ATTR), singleton (COVERED_ATTR), singleton(NUM_PROV_ATTR), getAttrDefNames(attrs));
	ProjectionOperator *op = createProjectionOp(projExpr, crossPdom, NIL, attrNames);
	crossPdom->parents = singleton(op);

	result = (Node *) op;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("scale up numInProv and covered for summarization:", result);
	INFO_OP_LOG("scale up numInProv and covered for summarization as overview:", result);

	return result;
}



/*
 * generate domain attrs for later use of scale up of the measure values to the real values
 */
static List *
domAttrsOutput (Node *input, int sampleSize, ProvQuestion qType, HashMap *vrPair, List *domRels, List *fPattern)
{
	List *result = NIL;

	// translated input algebra to use the table acess operators
	QueryOperator *prov = NULL;

//	if(isA(input, List))
//		prov = (QueryOperator *) getHeadOfListP((List *) input);
//	else
		prov = (QueryOperator *) input;

	QueryOperator *transInput = (QueryOperator *) prov->properties;

	if(isDL)
	{
//		QueryOperator *dup = (QueryOperator *) transInput;
		ProjectionOperator *fromInputQ = (ProjectionOperator *) getHeadOfListP(transInput->inputs);
		userQuestion = fromInputQ->projExprs;
	}
//	else
//	{
//
//		// replace the attr defs in userQuestion to attr Refs
//		int attrPos = 0;
//		FOREACH(AttributeDef, a, userQuestion)
//		{
//			AttributeDef *aDef = (AttributeDef *) getNthOfListP(normAttrs, attrPos);
//			ar->name = strdup(aDef->attrName);
//			attrPos++;
//		}
//	}

	// store table access operator for later use of dom attrs
	List *removeDupTa = NIL;
	List *distinctRels = NIL;
	List *rels = NIL;
	findTableAccessVisitor((Node *) transInput,&rels);

	// remove duplicate tableaccessOp
	FOREACH(TableAccessOperator,t,rels)
	{
		if(!searchListString(distinctRels,t->tableName))
		{
			distinctRels = appendToTailOfList(distinctRels,t->tableName);
			removeDupTa = appendToTailOfList(removeDupTa,t);
		}

		if(isDL)
		{
			QueryOperator *tBase = (QueryOperator *) t;
			QueryOperator *parent = NULL;

			if(!MY_LIST_EMPTY(tBase->parents))
				parent = (QueryOperator *) getHeadOfListP(tBase->parents);

			if(isA((Node *) parent,SelectionOperator) && parent != NULL)
			{
				SelectionOperator *so = (SelectionOperator *) parent;
				Operator *op = (Operator *) so->cond;

				if(streq(op->name,OPNAME_EQ))
				{
					FOREACH(Node,n,op->args)
					{
						if(isA(n,AttributeReference))
						{
							AttributeReference *ar = (AttributeReference *) n;
							ar->outerLevelsUp = 0;

							if(!searchListNode(givenConsts,n))
								givenConsts = appendToTailOfList(givenConsts,ar);
						}
					}
				}
				else
				{
					FOREACH(Operator,o,op->args)
					{
						FOREACH(Node,n,o->args)
						{
							if(isA(n,AttributeReference))
							{
								AttributeReference *ar = (AttributeReference *) n;
								ar->outerLevelsUp = 0;

								if(!searchListNode(givenConsts,n))
									givenConsts = appendToTailOfList(givenConsts,ar);
							}
						}
					}
				}
			}
		}
	}

	if(isDL)
	{
		// replace attr names in user question with full names
		FOREACH(AttributeReference,ar,userQuestion)
		{
			int relCnt = 0;
			int prevAttrCnt = 0;
			int pos = ar->attrPosition;
			char *rel = "dump";

			if(MAP_HAS_STRING_KEY(vrPair,ar->name))
				rel = STRING_VALUE(MAP_GET_STRING(vrPair,ar->name));

			if(LIST_LENGTH(removeDupTa) == 1)
			{
				FOREACH(TableAccessOperator,t,removeDupTa)
				{
					if(streq(rel,t->tableName))
					{
						pos = pos - prevAttrCnt;
						ar->outerLevelsUp = 0; // TODO: force to be '0' or keep it
//						ar->attrPosition = pos;
						ar->name = STRING_VALUE(getNthOfListP(t->op.schema->attrDefs,pos));
					}

					prevAttrCnt += LIST_LENGTH(t->op.schema->attrDefs);
				}
			}
			else if(LIST_LENGTH(removeDupTa) > 1)
			{
				FOREACH(TableAccessOperator,t,removeDupTa)
				{
					if(streq(rel,t->tableName))
					{
						if(relCnt > 0)
							pos = pos - prevAttrCnt;

						ar->outerLevelsUp = 0; // TODO: force to be '0' or keep it
						ar->attrPosition = pos;
						ar->name = STRING_VALUE(getNthOfListP(t->op.schema->attrDefs,pos));
					}

					prevAttrCnt += LIST_LENGTH(t->op.schema->attrDefs);
					relCnt++;
				}
			}
		}
	}

	int attrCount = 0;
	/* int relCount = 0; */
	char *relName = NULL;
//	HashMap *existAttr = NEW_MAP(Constant,Constant);

	FOREACH(TableAccessOperator,t,removeDupTa)
	{
		// if the input query is not self-joined, then reset everything
		if(relName != NULL)
		{
			if(!streq(relName,t->tableName))
//				relCount++;
//			else
			{
				attrCount = 0;
//				relCount = 0;
				relName = NULL;
			}
		}

		// collect the attrs not in the prov question and create dom for those
		// TODO: condition is temporary (e.g., to filter out the case that for self-join, dom attrs are generated based on only left)
		if(relName == NULL && MAP_HAS_STRING_KEY(vrPair,t->tableName))
		{
			relName = strdup(t->tableName);

			FOREACH(AttributeDef, a, t->op.schema->attrDefs)
			{
				AttributeReference *ar = createFullAttrReference(strdup(a->attrName), 0, attrCount, 0, a->dataType);

	//			if(relCount > 0)
	//				ar->attrPosition = ar->attrPosition + attrCount;

				if(!searchListNode(userQuestion, (Node *) ar) &&
						!searchListNode(givenConsts, (Node *) ar))
				{
//					// create attr domains only once
//					int existAttrCnt = MAP_INCR_STRING_KEY(existAttr,a->attrName);
//
//					if(existAttrCnt == 0)
//					{
						// count for why
						if(qType == PROV_Q_WHY && MY_LIST_EMPTY(domRels))
						{
							// create count attr
							AttributeReference *countAr = createFullAttrReference(strdup(ar->name), 0, ar->attrPosition - attrCount, 0, DT_INT);
							FunctionCall *fcCount = createFunctionCall("COUNT", singleton(countAr));
							fcCount->isAgg = TRUE;

							// create agg operator
							char *domAttr = CONCAT_STRINGS("cnt",ar->name);
							AggregationOperator *aggCount = createAggregationOp(singleton(fcCount), NIL, (QueryOperator *) t,
																NIL, singleton(strdup(domAttr)));
							SET_BOOL_STRING_PROP((Node *) aggCount, PROP_MATERIALIZE);

							result = appendToTailOfList(result, (Node *) aggCount);
						}
						// random sample from attr domain for whynot
						else if(qType == PROV_Q_WHYNOT || (qType == PROV_Q_WHY && !MY_LIST_EMPTY(domRels)))
						{
							/*
							 *  TODO: compute percentile for random sample based on the sample size
							 *  e.g., how to capture table size. Currently, size captured by the table name
							 */

							// capture the very first position of the number in a string
							char *numInChar = NULL;
							char string[strlen(t->tableName)];
							strcpy(string,t->tableName);

							for(int i = 0; string[i]!='\0'; i++)
							{
								if(isdigit(string[i]))
								{
									numInChar = &string[i];
									break;
								}
							}

							// capture the size of data from the string
							char *numInTableName = NULL;

							if(isSubstr(t->tableName,numInChar))
								numInTableName = strstr(t->tableName,numInChar);

							// replace the string[i] that represent the number (e.g., M/m = million)
							if(isSubstr(numInTableName,"K"))
								numInTableName = replaceSubstr(numInTableName,"K","000");

							if(isSubstr(numInTableName,"k"))
								numInTableName = replaceSubstr(numInTableName,"k","000");

							if(isSubstr(numInTableName,"M"))
								numInTableName = replaceSubstr(numInTableName,"M","000000");

							if(isSubstr(numInTableName,"m"))
								numInTableName = replaceSubstr(numInTableName,"m","000000");

							// calculate percentile for sampling
							float dataSize = 0;

							if(numInTableName != NULL)
								dataSize = atof(numInTableName);
							else // for tpch
								dataSize = 15000000;

							float perc = 0;

							if(sampleSize < dataSize)
							{
								float s = ((float) sampleSize) / dataSize;

								/*
								 *  re-compute percentile for sampling to guarantee that over 99% of over sampling
								 *  which guarantees 99% of having minimum number of failure pattern (from user) in the sample
								 *  TODO: exact percentile can be calculated over the binomial computation
								 */
								if(sampleSize >= 100 && sampleSize < 1000)
								{
									perc = (s + (s / 10 * 5)) * 100;
								}
								else if(sampleSize >= 1000)
								{
									// more sample needed for the case where a particular pattern is given
									if(!MY_LIST_EMPTY(fPattern))
										perc = (s + (s / 10 * 5)) * 100;
									else
										perc = (s + (s / 10) + (s / 100 * 5)) * 100;
								}
							}
							else
								perc = 99.99999;

							if(perc == 0)
								perc = 0.000002;

							// sample perc for adding SAMPLE clause in the serializer
//							t->sampClause = (Node *) createConstInt(perc);
							SampleClauseOperator *scOp = createSampleClauseOp((QueryOperator *) t,
															(Node *) createConstFloat(perc),
															getAttrNames(t->op.schema),
															getAttrDataTypes(t->op.schema->attrDefs));

							// rownum as a sequence
							Node *rowNum = (Node *) makeNode(RowNumExpr);
							List *projExpr = LIST_MAKE(rowNum, ar);
							ProjectionOperator *projDom = createProjectionOp(projExpr, (QueryOperator *) scOp, NIL,
														LIST_MAKE(strdup("SEQ"),strdup(ar->name)));
							SET_BOOL_STRING_PROP((Node *) projDom, PROP_MATERIALIZE);
							result = appendToTailOfList(result, (Node *) projDom);
						}
//					}
				}
				attrCount++;
			}
		}
		/* relCount++; */
	}

	DEBUG_NODE_BEATIFY_LOG("dom attrs for summarization:", (Node *) result);
	INFO_OP_LOG("dom attrs for summarization as overview:", (Node *) result);

	return result;
}


/*
 * compute measure values, e.g., numInProv and coverage
 * numInProv: how many prov within whole prov
 * coverage: how many prov or non-prov are covered by the pattern
 */
static Node *
rewriteCandidateOutput (Node *scanSampleInput, ProvQuestion qType, List *fPattern, boolean nonProvOpt)
{
	Node *result;

	QueryOperator *scanSamples = (QueryOperator *) scanSampleInput;

	// create group by operator
	List *groupBy = NIL;
	int gPos = 0;
	int boolAttrPos = LIST_LENGTH(scanSamples->schema->attrDefs) - LIST_LENGTH(fPattern) - 1;

	FOREACH(AttributeDef,a,scanSamples->schema->attrDefs)
	{
		if (nonProvOpt && gPos == boolAttrPos)
		{
			gPos = gPos + LIST_LENGTH(fPattern);
			break;
		}

		if (!streq(a->attrName,HAS_PROV_ATTR))
		{
			groupBy = appendToTailOfList(groupBy,
					createFullAttrReference(strdup(a->attrName), 0, gPos, 0, a->dataType));

			gPos++;
		}
	}

	List *aggrs = NIL;
	FunctionCall *fcShnp = NULL; // keep compiler quiet
	List *attrNames = NIL;

	if(qType == PROV_Q_WHY)
	{
		Constant *sumHasNonProv = createConstInt(0);
		fcShnp = createFunctionCall("SUM", singleton(sumHasNonProv));
		attrNames = concatTwoLists(singleton(NUM_NONPROV_ATTR), singleton(NUM_PROV_ATTR));
	}
	else if(qType == PROV_Q_WHYNOT)
	{
		Constant *countProv = createConstInt(1);
		fcShnp = createFunctionCall("COUNT", singleton(countProv));
		attrNames = concatTwoLists(singleton(COVERED_ATTR), singleton(NUM_PROV_ATTR));
	}
	fcShnp->isAgg = TRUE;

	AttributeReference *sumHasProv = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, gPos, 0, DT_INT);
	FunctionCall *fc = createFunctionCall("SUM", singleton(sumHasProv));
	fc->isAgg = TRUE;

	aggrs = appendToTailOfList(aggrs,fcShnp);
	aggrs = appendToTailOfList(aggrs,fc);

//	List *attrs = getAttrDefNames(removeFromTail(scanSamples->schema->attrDefs));
	List *attrs = getAttrDefNames(scanSamples->schema->attrDefs);
	attrNames = concatTwoLists(attrNames, attrs);

	AggregationOperator *gb = createAggregationOp(aggrs, groupBy, scanSamples, NIL, attrNames);
	scanSamples->parents = singleton(gb);
	scanSamples = (QueryOperator *) gb;
//	scanSamples->provAttrs = copyObject(provAttrs);

	// create projection operator
	List *projExpr = NIL;
//	List *origExprs = NIL;
	List *caseExprs = NIL;
	int pos = 0;
	attrNames = NIL;

	FOREACH(AttributeDef,a,scanSamples->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

		attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
		pos++;
	}

	if (nonProvOpt && !MY_LIST_EMPTY(fPattern))
	{
		int attrNum = LIST_LENGTH(scanSamples->schema->attrDefs) - 2;

		FOREACH(Constant,c,fPattern)
		{
//			c->constType = DT_BOOL;
			projExpr = appendToTailOfList(projExpr, (Node *) c);

			char *lastAttr = CONCAT_STRINGS("A",gprom_itoa(attrNum++));
			attrNames = appendToTailOfList(attrNames, strdup(lastAttr));

			pos++;
		}
	}

//	pos = 2;
//	FOREACH(AttributeDef,a,normAttrs)
//	{
//		origExprs = appendToTailOfList(origExprs,
//				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//
//		pos++;
//	}

	attrs = NIL;

	FOREACH(AttributeDef,n,projExpr)
	{
//		AttributeDef *a = (AttributeDef *) n;

		if (isPrefix(n->attrName,PROV_ATTR_PREFIX))
		{
			Node *cond = (Node *) createIsNullExpr((Node *) n);
			Node *then = (Node *) createConstInt(0);
			Node *els = (Node *) createConstInt(1);

			CaseWhen *caseWhen = createCaseWhen(cond, then);
			CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

			caseExprs = appendToTailOfList(caseExprs, (List *) caseExpr);
			attrs = appendToTailOfList(attrs, CONCAT_STRINGS("use",n->attrName));
		}
	}

//	attrNames = concatTwoLists(getAttrNames(scanSamples->schema), attrs);
	attrNames = concatTwoLists(attrNames, attrs);
	ProjectionOperator *op = createProjectionOp(concatTwoLists(projExpr,caseExprs), scanSamples, NIL, attrNames);
	scanSamples->parents = singleton(op);
	scanSamples = (QueryOperator *) op;

	result = (Node *) scanSamples;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("candidate patterns for summarization:", result);
	INFO_OP_LOG("candidate patterns for summarization as overview:", result);

	return result;
}


/*
 * match patterns generated with the full sample
 */
static Node *
rewriteScanSampleOutput (Node *sampleInput, Node *patternInput)
{
	Node *result;

	QueryOperator *samples = (QueryOperator *) sampleInput;
	QueryOperator *patterns = (QueryOperator *) patternInput;

	// create join condition
	int aPos = 0;
	Node *joinCond = NULL;
	Node *isNullCond = NULL;
	Node *attrCond = NULL;
	Node *curCond = NULL;
//	int sLen = LIST_LENGTH(samples->schema->attrDefs) - 1;

	FOREACH(AttributeDef,ar,patterns->schema->attrDefs)
	{
		AttributeReference *lA, *rA = NULL;
		rA = createFullAttrReference(strdup(ar->attrName), 1, aPos, 0, ar->dataType);

		FOREACH(AttributeDef,al,samples->schema->attrDefs)
		{
			if(isSubstr(ar->attrName,al->attrName) && isPrefix(al->attrName,PROV_ATTR_PREFIX))
			{
				int alPos = LIST_LENGTH(normAttrs) + aPos;
				lA = createFullAttrReference(strdup(al->attrName), 0, alPos, 0, al->dataType);

				// create equality condition and update global condition
				joinCond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lA,rA));
				isNullCond = (Node *) createIsNullExpr((Node *) rA);
				attrCond = OR_EXPRS(joinCond,isNullCond);
				curCond = AND_EXPRS(attrCond,curCond);
			}
		}
		aPos++;
	}

	// create join operator
	List *inputs = LIST_MAKE(samples,patterns);
	List *attrNames = concatTwoLists(getAttrNames(samples->schema),getAttrNames(patterns->schema));
	QueryOperator *scanSample = (QueryOperator *) createJoinOp(JOIN_INNER, curCond, inputs, NIL, attrNames);
	makeAttrNamesUnique(scanSample);

	// set the parent of the operator's children
//	OP_LCHILD(scanSample)->parents = OP_RCHILD(scanSample)->parents = singleton(scanSample);
	samples->parents = appendToTailOfList(samples->parents,scanSample);
	patterns->parents = singleton(scanSample);
//	scanSample->provAttrs = provAttrs;

	// create projection for adding HAS_PROV_ATTR attribute
	int pos = 0;
	int hasPos = 0;
	List *projExpr = NIL;
	List *hasExpr = NIL;
	List *failExpr = NIL;
	attrNames = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,scanSample->schema->attrDefs)
	{
		if (streq(p->attrName,HAS_PROV_ATTR))
		{
			hasPos = pos;
			hasExpr = appendToTailOfList(hasExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		}
		else if (pos > hasPos && hasPos != 0)
		{
			if (isPrefix(p->attrName,PROV_ATTR_PREFIX))
				projExpr = appendToTailOfList(projExpr,
						createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
		}
		else if (p->dataType == DT_BOOL)
		{
			failExpr = appendToTailOfList(failExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));

			attrNames = appendToTailOfList(attrNames,p->attrName);
		}
		pos++;
	}

	List *subAttrs = NIL;
	FOREACH(char,a,getAttrNames(patterns->schema))
		if (isPrefix(a,PROV_ATTR_PREFIX))
			subAttrs = appendToTailOfList(subAttrs,a);

	if(MY_LIST_EMPTY(failExpr))
	{
		projExpr = CONCAT_LISTS(projExpr, hasExpr);
		attrNames = CONCAT_LISTS(subAttrs, singleton(HAS_PROV_ATTR));
	}
	else
	{
		projExpr = CONCAT_LISTS(projExpr, failExpr, hasExpr);
		attrNames = CONCAT_LISTS(subAttrs, attrNames, singleton(HAS_PROV_ATTR));
	}

	op = createProjectionOp(projExpr, scanSample, NIL, attrNames);

	scanSample->parents = singleton(op);
	scanSample = (QueryOperator *) op;
//	scanSample->provAttrs = copyObject(provAttrs);

	result = (Node *) scanSample;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("join patterns with samples for summarization:", result);
	INFO_OP_LOG("join patterns with samples for summarization as overview:", result);

	return result;
}


/*
 * compute patterns (currently LCA is implemented)
 * TODO: more techniques to generate patterns
 */
static Node *
rewritePatternOutput (char *summaryType, Node *unionSample, Node *randProv)
{
	Node *result;

	// compute Lowest Common Ancestors (LCA)
	if (streq(summaryType,"LCA"))
	{
		QueryOperator *allSample = (QueryOperator *) unionSample;

		// return only sample tuples having provenance
		QueryOperator *provSample = (QueryOperator *) randProv;

//		int aPos = LIST_LENGTH(provSample->schema->attrDefs) - 1;
//		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);
//
//		Node *whereClause = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lC,createConstInt(1)));
//		SelectionOperator *so = createSelectionOp(whereClause, provSample, NIL, getAttrNames(provSample->schema));
//
//		provSample->parents = singleton(so);
//		provSample = (QueryOperator *) so;
//		provSample->provAttrs = copyObject(provAttrs);
//
//		// create projection operator
//		List *projExpr = NIL;
//		int pos = LIST_LENGTH(allSample->schema->attrDefs);
//
//		FOREACH(AttributeDef,a,provSample->schema->attrDefs)
//		{
//			projExpr = appendToTailOfList(projExpr,
//					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//			pos++;
//		}
//
//		ProjectionOperator *op = createProjectionOp(projExpr, provSample, NIL, getAttrNames(provSample->schema));
//		provSample->parents = singleton(op);
//		provSample->provAttrs = copyObject(provAttrs);

		// create CROSS_JOIN operator
		List *crossInput = LIST_MAKE(allSample, provSample);
		List *attrNames = concatTwoLists(getAttrNames(allSample->schema),getAttrNames(provSample->schema));
		QueryOperator *patternJoin = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, crossInput, NIL, attrNames);
		makeAttrNamesUnique(patternJoin);

		// set the parent of the operator's children
//		OP_LCHILD(patternJoin)->parents = OP_RCHILD(patternJoin)->parents = singleton(patternJoin);
		allSample->parents = appendToTailOfList(allSample->parents,patternJoin);
		provSample->parents = singleton(patternJoin);
//		patternJoin->provAttrs = copyObject(provAttrs);

		// create projection operator
		List *projExpr = NIL;
		List *lProjExpr = NIL;
		List *rProjExpr = NIL;
		int pos = 0;
		int numAttr = getNumAttrs(allSample);

		FOREACH(AttributeDef,a,allSample->schema->attrDefs)
		{
//			if(searchListNode(normAttrs,(Node *) a))
			if(strcmp(a->attrName,strdup(HAS_PROV_ATTR)) != 0)
			{
				lProjExpr = appendToTailOfList(lProjExpr,
						createFullAttrReference(strdup(getAttrNameByPos(patternJoin, pos)),
						        0, pos, 0, a->dataType));
                rProjExpr = appendToTailOfList(rProjExpr,
                        createFullAttrReference(strdup(getAttrNameByPos(patternJoin, pos + numAttr)),
                                0, pos + numAttr, 0, a->dataType));
				pos++;
			}
		}

		List *provAttrNames = NIL;

		FORBOTH(Node,l,r,lProjExpr,rProjExpr)
		{
			AttributeDef *a = (AttributeDef *) r;

			if(isPrefix(a->attrName,PROV_ATTR_PREFIX))
			{
				provAttrNames = appendToTailOfList(provAttrNames,a->attrName);

				DataType d = a->dataType;
				Node *cond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(l,r));

				Node *then = l;
				Node *els = (Node *) createNullConst(d);

				CaseWhen *caseWhen = createCaseWhen(cond, then);
				CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

				projExpr = appendToTailOfList(projExpr, (List *) caseExpr);
			}
		}

		ProjectionOperator *op = createProjectionOp(projExpr, patternJoin, NIL, provAttrNames);
		patternJoin->parents = singleton(op);
		patternJoin = (QueryOperator *) op;
//		patternJoin->provAttrs = copyObject(provAttrs);

		// create duplicate removal
		projExpr = NIL;
		pos = 0;

		FOREACH(AttributeDef,a,patternJoin->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

			pos++;
		}

		QueryOperator *dr = (QueryOperator *) createDuplicateRemovalOp(projExpr, patternJoin, NIL, getAttrNames(patternJoin->schema));
		patternJoin->parents = singleton(dr);
		patternJoin = (QueryOperator *) dr;
//		patternJoin->provAttrs = copyObject(provAttrs);

		result = (Node *) patternJoin;
		SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

		DEBUG_NODE_BEATIFY_LOG("pattern generation for summarization:", result);
		INFO_OP_LOG("pattern generation for summarization as overview:", result);
	}
	else
	{
		result = NULL;
		INFO_OP_LOG("Other pattern generation techniques have not implemented yet!!");
	}

	return result;
}




/*
 * Full sample of prov and non-prov
 */
static Node *
rewriteSampleOutput (Node *randProv, Node *randNonProv, int sampleSize, ProvQuestion qType)
{
	Node *result;
	List *allInput = NIL;
	QueryOperator *randomProv = (QueryOperator *) randProv;

	if(qType == PROV_Q_WHY)
	{
		Node *selCond = NULL;
		SelectionOperator *so;

//		int provSize = sampleSize;
//
//		if(nonProvOpt)
//			provSize = sampleSize / 2;
//
//		/* sampling from prov */
//		// create selection clause
//		selCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(makeNode(RowNumExpr),createConstInt(provSize)));
//		so = createSelectionOp(selCond, randomProv, NIL, getAttrNames(randomProv->schema));
//
//		randomProv->parents = singleton(so);
//		randomProv = (QueryOperator *) so;

		// compute non-prov size
		int hasProvPos = LIST_LENGTH(getAttrNames(randomProv->schema)) - 1;
		AttributeReference *hasProv = createFullAttrReference(strdup(HAS_PROV_ATTR),0,hasProvPos,0,DT_INT);
		FunctionCall *countProv = createFunctionCall("COUNT", singleton(hasProv));
		countProv->isAgg = TRUE;

		Node *nonProvSize = (Node *) createOpExpr("-", LIST_MAKE(createConstInt(sampleSize),countProv));
		ProjectionOperator *nonProvOp = createProjectionOp(singleton(nonProvSize),randomProv,NIL,singleton(strdup(NON_PROV_SIZE)));

		/* sampling from non-prov */
		QueryOperator *randomNonProv = (QueryOperator *) randNonProv;

		// create CROSS_JOIN operator
		List *crossInput = LIST_MAKE(randomNonProv, (QueryOperator *) nonProvOp);
		List *attrNames = concatTwoLists(getAttrNames(randomNonProv->schema),singleton(strdup(NON_PROV_SIZE)));
		QueryOperator *randomNonProvJoin= (QueryOperator *) createJoinOp(JOIN_CROSS,NULL,crossInput,NIL,attrNames);
		OP_LCHILD(randomNonProvJoin)->parents = OP_RCHILD(randomNonProvJoin)->parents = singleton(randomNonProvJoin);

		int pos = 0;
		List *projExpr = NIL;
		attrNames = NIL;

		FOREACH(AttributeDef,a,randomNonProvJoin->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
							createFullAttrReference(strdup(a->attrName),0,pos,0,a->dataType));

			attrNames = appendToTailOfList(attrNames,strdup(a->attrName));
			pos++;
		}

		ProjectionOperator *intermedProj = createProjectionOp(projExpr,randomNonProvJoin,NIL,attrNames);
		randomNonProvJoin->parents = singleton(intermedProj);
		randomNonProvJoin = (QueryOperator *) intermedProj;

		// generate non-prov for the size of (sampleSize - sampleProvSize/2)
		AttributeReference *npSize = (AttributeReference *) getNthOfListP(intermedProj->projExprs,pos-1);
		selCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(makeNode(RowNumExpr), npSize));
		so = createSelectionOp(selCond,randomNonProvJoin,NIL,getAttrNames(randomNonProvJoin->schema));

		randomNonProvJoin->parents = singleton(so);
		randomNonProvJoin = (QueryOperator *) so;

		// remove the attribute of nonProvSize
		pos = 0;
		projExpr = NIL;
		attrNames = NIL;

		FOREACH(AttributeDef,a,randomNonProvJoin->schema->attrDefs)
		{
			if(!streq(a->attrName,NON_PROV_SIZE))
			{
				projExpr = appendToTailOfList(projExpr,
								createFullAttrReference(strdup(a->attrName),0,pos,0,a->dataType));

				attrNames = appendToTailOfList(attrNames,strdup(a->attrName));
			}
			pos++;
		}

		intermedProj = createProjectionOp(projExpr,randomNonProvJoin,NIL,attrNames);
		randomNonProvJoin->parents = singleton(intermedProj);
		randomNonProvJoin = (QueryOperator *) intermedProj;

	//
	//	/*
	//	 * create sample based on the previous method
	//	 */
	//	List *attrNames = NIL;
	//	QueryOperator *randomProvL = (QueryOperator *) randProv;
	//	QueryOperator *randomProvR = (QueryOperator *) randProv;
	//
	//	attrNames = getAttrNames(randomProvL->schema);
	//
	//	// create projection for adding "ROWNUM" for the left input
	//	int pos = 0;
	//	List *projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randomProvL->schema->attrDefs)
	//	{
	//		projExpr = appendToTailOfList(projExpr,
	//				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		pos++;
	//	}
	//	projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));
	//	attrNames = appendToTailOfList(attrNames, strdup(SAMP_NUM_L_ATTR));
	//
	//	ProjectionOperator *leftOp = createProjectionOp(projExpr, randomProvL, NIL, attrNames);
	//	randomProvL->parents = singleton(leftOp);
	//	randomProvL = (QueryOperator *) leftOp;
	//	randomProvL->provAttrs = copyObject(provAttrs);
	//
	//	// create projection for computing sample size for the right input
	////	AttributeReference *countProv = createFullAttrReference(strdup("1"), 0, 0, 0, DT_INT);
	//	FunctionCall *fcCount = createFunctionCall("COUNT", singleton(createConstInt(1)));
	//	fcCount->isAgg = TRUE;
	//
	//	float spSize = 0.0;
	//
	//	if (samplePerc != 0)
	//		spSize = (float) samplePerc / 100;
	//	else
	//		spSize = 0.1; // TODO: Whole or still do sampling?
	//
	//	Node* sampSize = (Node *) createOpExpr("*",LIST_MAKE(fcCount,createConstFloat(spSize)));
	//	FunctionCall *fcCeil = createFunctionCall("CEIL", singleton(sampSize));
	//
	//	ProjectionOperator *rightOp = createProjectionOp(singleton(fcCeil), randomProvR, NIL, singleton(strdup(SAMP_NUM_R_ATTR)));
	//	randomProvR->parents = appendToTailOfList(randomProvR->parents, rightOp);
	//	randomProvR = (QueryOperator *) rightOp;
	//
	//	// create JOIN operator
	//	QueryOperator *left = (QueryOperator *) leftOp;
	//	QueryOperator *right = (QueryOperator *) rightOp;
	//	Node *joinCond = NULL;
	//
	//	FOREACH(AttributeDef,l,left->schema->attrDefs)
	//	{
	//		FOREACH(AttributeDef,r,right->schema->attrDefs)
	//		{
	//			if (streq(l->attrName,SAMP_NUM_L_ATTR) && streq(r->attrName,SAMP_NUM_R_ATTR))
	//			{
	//				AttributeReference *lA, *rA = NULL;
	//				lA = createFullAttrReference(strdup(l->attrName), 0, LIST_LENGTH(left->schema->attrDefs)-1, 0, l->dataType);
	//				rA = createFullAttrReference(strdup(r->attrName), 1, 0, 0, r->dataType);
	//				joinCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(lA,rA));
	//			}
	//		}
	//	}
	//
	//	List *crossInput = LIST_MAKE(left, right);
	//	attrNames = concatTwoLists(getAttrNames(left->schema),getAttrNames(right->schema));
	//	QueryOperator *randProvJoin = (QueryOperator *) createJoinOp(JOIN_INNER, joinCond, crossInput, NIL, attrNames);
	//
	//	// set the parent of the operator's children
	//	left->parents = appendToTailOfList(left->parents,randProvJoin);
	//	right->parents = appendToTailOfList(right->parents,randProvJoin);
	//
	//	// create projection to remove sampNum attrs
	//	pos = 0;
	//	projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randProvJoin->schema->attrDefs)
	//	{
	//		if (!isPrefix(p->attrName,SAMP_NUM_PREFIX))
	//		{
	//			projExpr = appendToTailOfList(projExpr,
	//					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		}
	//		pos++;
	//	}
	//
	//	attrNames = getAttrNames(((QueryOperator *) randProv)->schema);
	//	ProjectionOperator *op = createProjectionOp(projExpr, randProvJoin, NIL, attrNames);
	//	randProvJoin->parents = singleton(op);
	//	randProvJoin = (QueryOperator *) op;
	//	randProvJoin->provAttrs = copyObject(provAttrs);
	//
	//
	//	// sampling from random ordered non-provenance tuples
	//	QueryOperator *randomNonProvL = (QueryOperator *) randNonProv;
	//	QueryOperator *randomNonProvR = (QueryOperator *) randNonProv;
	//
	//	attrNames = getAttrNames(randomNonProvL->schema);
	//
	//	// create projection for adding "ROWNUM" for the left input
	//	pos = 0;
	//	projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randomNonProvL->schema->attrDefs)
	//	{
	//		projExpr = appendToTailOfList(projExpr,
	//				createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		pos++;
	//	}
	//	projExpr = appendToTailOfList(projExpr, makeNode(RowNumExpr));
	//	attrNames = appendToTailOfList(attrNames, strdup(SAMP_NUM_L_ATTR));
	//
	//	leftOp = createProjectionOp(projExpr, randomNonProvL, NIL, attrNames);
	//	randomNonProvL->parents = singleton(leftOp);
	//	randomNonProvL = (QueryOperator *) leftOp;
	//	randomNonProvL->provAttrs = copyObject(provAttrs);
	//
	//	// create projection for computing sample size for the right input
	//	rightOp = createProjectionOp(singleton(fcCeil), randomNonProvR, NIL, singleton(strdup(SAMP_NUM_R_ATTR)));
	//	randomNonProvR->parents = appendToTailOfList(randomNonProvR->parents, rightOp);
	//	randomNonProvR = (QueryOperator *) rightOp;
	//
	//	// create JOIN operator
	//	left = (QueryOperator *) leftOp;
	//	right = (QueryOperator *) rightOp;
	//	joinCond = NULL;
	//
	//	FOREACH(AttributeDef,l,left->schema->attrDefs)
	//	{
	//		FOREACH(AttributeDef,r,right->schema->attrDefs)
	//		{
	//		    if (streq(l->attrName,SAMP_NUM_L_ATTR) && streq(r->attrName,SAMP_NUM_R_ATTR))
	//			{
	//				AttributeReference *lA, *rA = NULL;
	//				lA = createFullAttrReference(strdup(l->attrName), 0, LIST_LENGTH(left->schema->attrDefs)-1, 0, l->dataType);
	//				rA = createFullAttrReference(strdup(r->attrName), 1, 0, 0, r->dataType);
	//				joinCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(lA,rA));
	//			}
	//		}
	//	}
	//
	//	crossInput = LIST_MAKE(left, right);
	//	attrNames = concatTwoLists(getAttrNames(left->schema),getAttrNames(right->schema));
	//	QueryOperator *randNonProvJoin = (QueryOperator *) createJoinOp(JOIN_INNER, joinCond, crossInput, NIL, attrNames);
	//
	//	// set the parent of the operator's children
	//	left->parents = appendToTailOfList(left->parents,randNonProvJoin);
	//	right->parents = appendToTailOfList(right->parents,randNonProvJoin);
	//
	//	// create projection to remove sampNum attrs
	//	pos = 0;
	//	projExpr = NIL;
	//
	//	FOREACH(AttributeDef,p,randNonProvJoin->schema->attrDefs)
	//	{
	//		if (!isPrefix(p->attrName,SAMP_NUM_PREFIX))
	//		{
	//			projExpr = appendToTailOfList(projExpr,
	//					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
	//		}
	//		pos++;
	//	}
	//
	//	attrNames = getAttrNames(((QueryOperator *) randNonProv)->schema);
	//	op = createProjectionOp(projExpr, randNonProvJoin, NIL, attrNames);
	//	randNonProvJoin->parents = singleton(op);
	//	randNonProvJoin = (QueryOperator *) op;
	//	randNonProvJoin->provAttrs = copyObject(provAttrs);

		allInput = LIST_MAKE(randomProv,randomNonProvJoin);
	}
	else if(qType == PROV_Q_WHYNOT)
	{
//		// make attr name with PROV_ATTR_PREFIX
//		FOREACH(AttributeDef,a,randomProv->schema->attrDefs)
//			if(!streq(a->attrName,HAS_PROV_ATTR))
//				a->attrName = CONCAT_STRINGS(PROV_ATTR_PREFIX,a->attrName);

		FOREACH(AttributeDef,a,((QueryOperator *) randNonProv)->schema->attrDefs)
			if(!streq(a->attrName,HAS_PROV_ATTR))
				a->attrName = CONCAT_STRINGS(PROV_ATTR_PREFIX,a->attrName);

		allInput = LIST_MAKE(randomProv,randNonProv);
	}

	// create UNION operator to get all sample
	QueryOperator *unionOp = (QueryOperator *) createSetOperator(SETOP_UNION,allInput,NIL,getAttrNames(randomProv->schema));
	OP_LCHILD(unionOp)->parents = OP_RCHILD(unionOp)->parents = singleton(unionOp);
//	unionOp->provAttrs = copyObject(provAttrs);

	result = (Node *) unionOp;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sampling for summarization:", result);
	INFO_OP_LOG("sampling for summarization as overview:", result);

	return result;
}


//
///*
// * compute sample size based on the sample percentile requested
// * TODO: how to use sample percentile for sample size
// * (e.g., what would the sample size be based on?)
// */
//
//static int *computeSampleSize (int *samplePerc, Node *prov)
//{
//	QueryOperator *provOp = (QueryOperator *) prov;
//
//	// create count operator with sample percentile
//}


/*
 * random sampling for non-prov tuples
 */
static Node *
rewriteRandomNonProvTuples (Node *provExpl, ProvQuestion qType, List *fPattern)
{
	Node *result;
	QueryOperator *randomNonProv = (QueryOperator *) provExpl;

	if(qType == PROV_Q_WHY)
	{
		List *attrNames = NIL;
		// random sampling from hasProv = 0
		attrNames = getAttrNames(randomNonProv->schema);

		// create selection for the prov instance
		int aPos = LIST_LENGTH(randomNonProv->schema->attrDefs) - 1;
		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

		Node *whereClause = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lC,createConstInt(0)));
		SelectionOperator *so = createSelectionOp(whereClause, randomNonProv, NIL, attrNames);

		randomNonProv->parents = appendToTailOfList(randomNonProv->parents, so);
		randomNonProv = (QueryOperator *) so;
		randomNonProv->provAttrs = copyObject(provAttrs);

		// create projection for adding HAS_PROV_ATTR attribute
		int pos = 0;
		List *projExpr = NIL;
		ProjectionOperator *op;

		FOREACH(AttributeDef,p,randomNonProv->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
			pos++;
		}

		op = createProjectionOp(projExpr, randomNonProv, NIL, attrNames);
		randomNonProv->parents = singleton(op);
		randomNonProv = (QueryOperator *) op;
		randomNonProv->provAttrs = copyObject(provAttrs);

		// create order by operator
		Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
		OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);

		OrderOperator *ord = createOrderOp(singleton(ordExpr), randomNonProv, NIL);
		randomNonProv->parents = singleton(ord);
		randomNonProv = (QueryOperator *) ord;
		randomNonProv->provAttrs = copyObject(provAttrs);
	}
	else if(qType == PROV_Q_WHYNOT)
	{
		int numAttrInput = LIST_LENGTH(randomNonProv->schema->attrDefs);
		int lenOfPattern = LIST_LENGTH(fPattern);
		int patternPos = numAttrInput - lenOfPattern;
		int pos = 0;

		// create selection condition
		Node *curCond = NULL;

		while(patternPos < numAttrInput)
		{
			Node *attrCond = NULL;
			AttributeDef *a = (AttributeDef *) getNthOfListP(randomNonProv->schema->attrDefs, patternPos);

			AttributeReference *lA = createFullAttrReference(strdup(a->attrName),0,patternPos,0,a->dataType);
			Node *rA = (Node *) createConstInt(INT_VALUE(getNthOfListP(fPattern,pos)));

			attrCond = (Node *) createOpExpr("<>",LIST_MAKE(lA,rA));

			if(pos == 0)
				curCond = attrCond;
			else if(pos > 0)
				curCond = OR_EXPRS(curCond,attrCond);

			patternPos++;
			pos++;
		}

//		// add NOT in where clause
//		Node *notOp = (Node *) createOpExpr("NOT ", singleton(curCond));
		SelectionOperator *selCond = createSelectionOp(curCond, randomNonProv, NIL, NIL);
		randomNonProv->parents = singleton(selCond);
		randomNonProv = (QueryOperator *) selCond;

		// create projection operator
		List *projExpr = NIL;
		List *attrNames = NIL;
		patternPos = numAttrInput - lenOfPattern;
		pos = 0;

		FOREACH(AttributeDef,a,randomNonProv->schema->attrDefs)
		{
//			if(pos < patternPos)
//			{
				attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
				projExpr = appendToTailOfList(projExpr,
								createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//			}
			pos++;
		}

		// add 0 as a hasProv
		Node *hasProv = (Node *) createConstInt(0);
		projExpr = appendToTailOfList(projExpr, hasProv);
		attrNames = appendToTailOfList(attrNames, strdup(HAS_PROV_ATTR));

		ProjectionOperator *projOp = createProjectionOp(projExpr, randomNonProv, NIL, attrNames);
		randomNonProv->parents = singleton(projOp);
		randomNonProv = (QueryOperator *) projOp;
	}

	result = (Node *) randomNonProv;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("random order of non-provenance for summarization:", result);
	INFO_OP_LOG("random order of non-provenance for summarization as overview:", result);

	return result;
}


/*
 * random sampling for prov tuples
 */
static Node *
rewriteRandomProvTuples (Node *provExpl, int sampleSize, ProvQuestion qType, List *fPattern, boolean nonProvOpt)
{
	Node *result;
	SelectionOperator *so;
	QueryOperator *randomProv = (QueryOperator *) provExpl;

	if(qType == PROV_Q_WHY)
	{
		List *attrNames = NIL;
		// random sampling from hasProv = 1
//		randomProv = (QueryOperator *) provExpl;
		attrNames = getAttrNames(randomProv->schema);

		// create selection for the prov instance
		int aPos = LIST_LENGTH(randomProv->schema->attrDefs) - 1;
		AttributeReference *lC = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, aPos, 0, DT_INT);

		Node *whereClause = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lC,createConstInt(1)));
		so = createSelectionOp(whereClause, randomProv, NIL, attrNames);

		randomProv->parents = singleton(so);
		randomProv = (QueryOperator *) so;
		randomProv->provAttrs = copyObject(provAttrs);

		// create projection for adding HAS_PROV_ATTR attribute
		int pos = 0;
		attrNames = NIL;
		List *projExpr = NIL;
		ProjectionOperator *op;

		FOREACH(AttributeDef,p,randomProv->schema->attrDefs)
		{
//			// make the boolean type attribute not a provenance attr if exists
//			if(p->dataType == DT_BOOL)
//				p->attrName = replaceSubstr(p->attrName,PROV_ATTR_PREFIX,"");

			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));

			attrNames = appendToTailOfList(attrNames,p->attrName);
			pos++;
		}

		op = createProjectionOp(projExpr, randomProv, NIL, attrNames);
		randomProv->parents = singleton(op);
		randomProv = (QueryOperator *) op;
		randomProv->provAttrs = copyObject(provAttrs);
	}
	else if (qType == PROV_Q_WHYNOT)
	{
		int numAttrInput = LIST_LENGTH(randomProv->schema->attrDefs);
		int lenOfPattern, patternPos, pos = 0;

		if(!MY_LIST_EMPTY(fPattern))
		{
			lenOfPattern = LIST_LENGTH(fPattern);
			patternPos = numAttrInput - lenOfPattern;

			// create selection condition
			Node *curCond = NULL;

			while(patternPos < numAttrInput)
			{
				Node *attrCond = NULL;
				AttributeDef *a = (AttributeDef *) getNthOfListP(randomProv->schema->attrDefs, patternPos);

				AttributeReference *lA = createFullAttrReference(strdup(a->attrName),0,patternPos,0,a->dataType);
				Node *rA = (Node *) createConstInt(INT_VALUE(getNthOfListP(fPattern,pos)));

				attrCond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lA,rA));

				if(pos == 0)
					curCond = attrCond;
				else if(pos > 0)
					curCond = AND_EXPRS(curCond,attrCond);

				patternPos++;
				pos++;
			}

			SelectionOperator *selCond = createSelectionOp(curCond, randomProv, NIL, NIL);
			randomProv->parents = singleton(selCond);
			randomProv = (QueryOperator *) selCond;
		}

		// create projection operator
		List *projExpr = NIL;
		List *attrNames = NIL;
		pos = 0;

		FOREACH(AttributeDef,a,randomProv->schema->attrDefs)
		{
//			if(a->dataType != DT_BOOL)
//			{
				attrNames = appendToTailOfList(attrNames, strdup(a->attrName));
				projExpr = appendToTailOfList(projExpr,
								createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
//			}
			pos++;
		}

		// add 1 as a hasProv
		Node *hasProv = (Node *) createConstInt(1);
		projExpr = appendToTailOfList(projExpr, hasProv);
		attrNames = appendToTailOfList(attrNames, strdup(HAS_PROV_ATTR));

		ProjectionOperator *projOp = createProjectionOp(projExpr, randomProv, NIL, attrNames);
		randomProv->parents = singleton(projOp);
		randomProv = (QueryOperator *) projOp;

		// make attr name with PROV_ATTR_PREFIX
		FOREACH(AttributeDef,a,randomProv->schema->attrDefs)
			if(!streq(a->attrName,HAS_PROV_ATTR) && a->dataType != DT_BOOL)
				a->attrName = CONCAT_STRINGS(PROV_ATTR_PREFIX,a->attrName);
	}

	// create order by operator
	Node *ordCond = (Node *) createConstString("DBMS_RANDOM.RANDOM");
	OrderExpr *ordExpr = createOrderExpr(ordCond, SORT_ASC, SORT_NULLS_LAST);

	OrderOperator *ord = createOrderOp(singleton(ordExpr), randomProv, NIL);
	randomProv->parents = singleton(ord);
	randomProv = (QueryOperator *) ord;

	//
	int provSize = sampleSize;

	if(nonProvOpt)
		provSize = sampleSize / 2;

	/* sampling from prov */
	// create selection clause
	Node *selCond = NULL;
	selCond = (Node *) createOpExpr(OPNAME_LE,LIST_MAKE(makeNode(RowNumExpr),createConstInt(provSize)));
	so = createSelectionOp(selCond, randomProv, NIL, getAttrNames(randomProv->schema));

	randomProv->parents = singleton(so);
	randomProv = (QueryOperator *) so;
	randomProv->provAttrs = copyObject(provAttrs);

	result = (Node *) randomProv;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("random sampling from provenance for the given size:", result);
	INFO_OP_LOG("random sampling from provenance for the given size as overview:", result);

	return result;
}


/*
 * replace cross product with sample domain generated
 */
static Node *
replaceDomWithSampleDom (List *sampleDoms, List *domRels, Node *input)
{
	Node *result;
//	Node *whyNotRuleFire = (Node *) getHeadOfListP((List *) input);
	Node *whyNotRuleFire = copyObject(input);

//	// TODO: function for capturing sub-rooted RA for domain
//	// count number of attrs in sample domain
//	QueryOperator *sDom = (QueryOperator *) sampleDom;
//	int numOfAttrs = LIST_LENGTH(sDom->schema->attrDefs);

	List *rels = NIL;
	findTableAccessVisitor((Node *) whyNotRuleFire,&rels);

	FOREACH(TableAccessOperator,t,rels)
	{
		if(HAS_STRING_PROP(t,DL_IS_DOMAIN_REL))
		{
			QueryOperator *tBase = (QueryOperator *) t;

			// find the corresponding domain RA by the position
			int qoPos = listPosString(domRels,t->tableName);
			QueryOperator *sampleDom = (QueryOperator *) getNthOfListP(sampleDoms,qoPos);

			/*
			 *  replace the query block with joined sampled domain
			 *  1) for cross products:
			 *     the grand parent is a binary operator (e.g., cross product) and the grand parent has more
			 *     than one parent (indicator for another binary operator), replace the grand parent query block
			 *  2) no cross products:
			 *     the direct parent of the domain table has more than one parent, then find the direct grand parent
			 *     if it is unary operator with only one direct parent, then replace the direct
			 *  TODO: the query block is checked by the type of operator and the size of the direct parent
			 */
			QueryOperator *parent = (QueryOperator *) getHeadOfListP(tBase->parents);
			QueryOperator *grandparent = (QueryOperator *) getHeadOfListP(parent->parents);

			if(IS_BINARY_OP(grandparent) && LIST_LENGTH(grandparent->parents) > 1)
				switchSubtreeWithExisting(grandparent, sampleDom);

			if(LIST_LENGTH(parent->parents) > 1)
			{
				grandparent = (QueryOperator *) getTailOfListP(parent->parents);

				if(IS_UNARY_OP(grandparent) && LIST_LENGTH(grandparent->parents) == 1)
					switchSubtreeWithExisting(parent, sampleDom);
			}

			DEBUG_LOG("replaced domain %s with\n:%s", operatorToOverviewString((Node *) tBase),
					operatorToOverviewString((Node *) sampleDom));
		}
	}

	result = (Node *) whyNotRuleFire;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("sample based why-not explanation:", result);
	INFO_OP_LOG("sample based why-not explanation:", result);

	return result;
}


/*
 * for WHYNOT, join on seq
 */
static List *
joinOnSeqOutput (List *doms)
{
	List *result = NIL;
	List *inputs = NIL;
	List *attrNames = NIL;
	List * outputs = NIL;

	QueryOperator *sampDom = NULL;
	AttributeReference *lA = NULL,
					   *rA = NULL;

	int lApos = 0;

	inputs = appendToTailOfList(inputs, (Node *) getNthOfListP(doms,0));
	QueryOperator *firstOp = (QueryOperator *) getNthOfListP(doms,0);

	List *rels = NIL;
	findTableAccessVisitor((Node *) firstOp,&rels);
	TableAccessOperator *firstOpTo = (TableAccessOperator *) getHeadOfListP(rels);
	char *strRel = firstOpTo->tableName;

	FOREACH(AttributeDef,a,firstOp->schema->attrDefs)
	{
		if(streq(a->attrName,"SEQ"))
			lA = createFullAttrReference(strdup(a->attrName),0,lApos,0,a->dataType);

		attrNames = appendToTailOfList(attrNames, a->attrName);
		lApos++;
	}

	for(int i = 1; i < LIST_LENGTH(doms); i++)
	{
		Node *n = (Node *) getNthOfListP(doms,i);

		if(i == 1 || sampDom == NULL)
			inputs = appendToTailOfList(inputs,n);
		else
			inputs = LIST_MAKE(sampDom, n);

		// capture table name
		rels = NIL;
		findTableAccessVisitor(n,&rels);
		TableAccessOperator *followedTo = (TableAccessOperator *) getHeadOfListP(rels);
		char *followedRel = followedTo->tableName;

		QueryOperator *oDom = (QueryOperator *) n;

		if(streq(strRel,followedRel))
		{
			rA = NULL;
			int rApos = 0;

			FOREACH(AttributeDef,oa,oDom->schema->attrDefs)
			{
				if(streq(oa->attrName,"SEQ"))
					rA = createFullAttrReference(strdup(oa->attrName),1,rApos,0,oa->dataType);

				attrNames = appendToTailOfList(attrNames, oa->attrName);
				rApos++;
			}

			// create join on "seq"
			if(lA != NULL && rA != NULL)
			{
				Node *cond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lA,rA));
				sampDom = (QueryOperator *) createJoinOp(JOIN_INNER, cond, inputs, NIL, attrNames);
			}

			if(i == 1 || sampDom == NULL)
				firstOp->parents = singleton(sampDom);
			else
				OP_LCHILD(sampDom)->parents = singleton(sampDom);

			oDom->parents = singleton(sampDom);
		}
		else
		{
			// store the previous sample domain generated
			if(rA == NULL)
				sampDom = firstOp;

			outputs = appendToTailOfList(outputs, sampDom);
			sampDom = NULL;
			firstOp = (QueryOperator *) n;
			inputs = singleton(n);

			rels = NIL;
			findTableAccessVisitor((Node *) firstOp,&rels);
			firstOpTo = (TableAccessOperator *) getHeadOfListP(rels);
			strRel = firstOpTo->tableName;

			AttributeDef *a = (AttributeDef *) getHeadOfListP(firstOp->schema->attrDefs);
			lA = createFullAttrReference(strdup(a->attrName),0,0,0,a->dataType);
			attrNames = getAttrNames(firstOp->schema);
			rA = NULL;
		}
	}
	// add the last domain generated based on join
	if(rA == NULL)
		sampDom = firstOp;

	outputs = appendToTailOfList(outputs, sampDom);

	// add projection operator to remove seq attr
	FOREACH(QueryOperator,o,outputs)
	{
		List *projExpr = NIL;
		attrNames = NIL;
		int pos = 0;

		FOREACH(AttributeDef,a,o->schema->attrDefs)
		{
			if(!streq(a->attrName,"SEQ"))
			{
				projExpr = appendToTailOfList(projExpr,createFullAttrReference(strdup(a->attrName),0,pos,0,a->dataType));
				attrNames = appendToTailOfList(attrNames, a->attrName);
			}
			pos++;
		}

		ProjectionOperator *p = createProjectionOp(projExpr,o,NIL,attrNames);
		o->parents = singleton(p);
		o = (QueryOperator *) p;

		// add the projection operator to the result
		SET_BOOL_STRING_PROP(o, PROP_MATERIALIZE);
		result = appendToTailOfList(result,o);
	}

	DEBUG_NODE_BEATIFY_LOG("sample domain based on the seq:", result);
	INFO_OP_LOG("sample domain based on the seq:", result);

	return result;
}


/*
 * For SQL, create base input for the summarization by
 * joining whole provenance and a user specific provenance
 * and mark user specific as 1 and 0 otherwise
 *
 * TODO: for Datalog, we need to have whole prov
 */
static Node *
rewriteProvJoinOutput (Node *rewrittenTree, boolean nonProvOpt)
{
	Node *result;
	QueryOperator *prov;

//	if(userQuestion == NIL)
//	if(isA(rewrittenTree, List))
//		prov = (QueryOperator *) getHeadOfListP((List *) rewrittenTree);
//	else
		prov = (QueryOperator *) rewrittenTree;

		// take the input query out for use with join operator later
		QueryOperator *transInput = (QueryOperator *) prov->properties;


	// For dl, make attr names starting with "PROV"
	if(isDL)
	{
		// replace the attr names starting with PROV_ATTR_PREFIX
		FOREACH(QueryOperator,q,prov->inputs)
			FOREACH(AttributeDef,a,q->schema->attrDefs)
				a->attrName = CONCAT_STRINGS(PROV_ATTR_PREFIX,a->attrName);

		FOREACH(AttributeDef,a,prov->schema->attrDefs)
//			if (a->dataType != DT_BOOL)
				a->attrName = CONCAT_STRINGS(PROV_ATTR_PREFIX,a->attrName);

		// store orig data types
		origDataTypes = getDataTypes(prov->schema);
	}

	// store normal and provenance attributes for later use
	if(provAttrs == NIL || normAttrs == NIL)
	{
		if(isDL)
		{
//			FOREACH(AttributeDef,a,prov->schema->attrDefs)
//				provAttrs = appendToTailOfList(provAttrs, CONCAT_STRINGS("i",gprom_itoa(getAttrPos(prov, a->attrName))));
			ProjectionOperator *p = (ProjectionOperator *) getHeadOfListP((List *) transInput->inputs);
			normAttrs = copyObject(p->projExprs);

			// store user question attrs
			userQuestion = normAttrs;

			List *rels = NIL;
			findTableAccessVisitor((Node *) transInput,&rels);

			// collect the constants given from the user
			FOREACH(TableAccessOperator,t,rels)
			{
				QueryOperator *tBase = (QueryOperator *) t;
				QueryOperator *parent = NULL;

				if(!MY_LIST_EMPTY(tBase->parents))
					parent = (QueryOperator *) getHeadOfListP(tBase->parents);

				if(isA((Node *) parent,SelectionOperator) && parent != NULL)
				{
					SelectionOperator *so = (SelectionOperator *) parent;
					Operator *op = (Operator *) so->cond;

					if(streq(op->name,OPNAME_EQ))
					{
						FOREACH(Node,n,op->args)
						{
							if(isA(n,AttributeReference))
							{
								AttributeReference *ar = (AttributeReference *) n;
								ar->outerLevelsUp = 0;

								if(!searchListNode(givenConsts,n))
									givenConsts = appendToTailOfList(givenConsts,ar);
							}
						}
					}
					else
					{
						FOREACH(Operator,o,op->args)
						{
							FOREACH(Node,n,o->args)
							{
								if(isA(n,AttributeReference))
								{
									AttributeReference *ar = (AttributeReference *) n;
									ar->outerLevelsUp = 0;

									if(!searchListNode(givenConsts,n))
										givenConsts = appendToTailOfList(givenConsts,ar);
								}
							}
						}
					}
				}
			}

		}
		else
		{
			provAttrs = getProvenanceAttrs(prov);
			normAttrs = getNormalAttrs(prov);
		}
	}

	int pos = 0;
	List *projExpr = NIL;
	ProjectionOperator *op;
	List *provAttrNames = NIL;
//	QueryOperator *origProv = prov;

	// create projection for adding HAS_PROV_ATTR attribute
	FOREACH(AttributeDef,p,prov->schema->attrDefs)
	{
		if(isPrefix(p->attrName,PROV_ATTR_PREFIX))
		{
			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));

			provAttrNames = appendToTailOfList(provAttrNames, strdup(p->attrName));
		}
		pos++;
	}
	projExpr = appendToTailOfList(projExpr,createConstInt(1));

	// add an attribute for prov
//	int attrPos = LIST_LENGTH(transInput->schema->attrDefs) + LIST_LENGTH(prov->schema->attrDefs);
//	int attrPos = LIST_LENGTH(prov->schema->attrDefs);
//	AttributeDef *hasProv = (AttributeDef *) createFullAttrReference(strdup(HAS_PROV_ATTR), 0, attrPos, 0, DT_INT);

//	List *newAttrs = concatTwoLists(getAttrNames(prov->schema),singleton(hasProv->attrName));
	List *newAttrs = concatTwoLists(provAttrNames,singleton(HAS_PROV_ATTR));
	op = createProjectionOp(projExpr, prov, NIL, newAttrs);

	prov->parents = singleton(op);
	prov = (QueryOperator *) op;
//	prov->provAttrs = copyObject(provAttrs);

	QueryOperator *provJoin = NULL;

	if(!nonProvOpt)
	{
		// add user question attributes
		pos = 0;
		projExpr = NIL;
		List *attrNames = NIL;

		// add user question attrs
		if(isDL)
		{
			for(int i = 0; i < LIST_LENGTH(transInput->schema->attrDefs); i++)
			{
				AttributeDef *a = (AttributeDef *) getNthOfListP(transInput->schema->attrDefs,i);
				char *renameA = CONCAT_STRINGS("A",gprom_itoa(i));

				projExpr = appendToTailOfList(projExpr, createFullAttrReference(strdup(renameA), 0, i, 0, a->dataType));
				attrNames = appendToTailOfList(attrNames, renameA);
			}
		}
		else
		{
			ProjectionOperator* pIn = (ProjectionOperator *) transInput;
			projExpr = pIn->projExprs;
			attrNames = getAttrNames(transInput->schema);
		}

		pos = 0;
		FOREACH(AttributeDef,a,prov->schema->attrDefs)
		{
			projExpr = appendToTailOfList(projExpr, createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
			pos++;
		}

		// create projection operator
		attrNames = CONCAT_LISTS(attrNames, getAttrNames(prov->schema));
		op = createProjectionOp(projExpr, prov, NIL, attrNames);
		prov->parents = singleton(op);
	}
	else
	{
		// create join condition
		// TODO: find the corresponding provenance attribute to join if the name of attrs repeats
	//	boolean suffix = FALSE;
		Node *curCond = NULL;
		int aPos = 0;
		int chkPos = 0;
		List *inputs = NIL;
		List *attrNames = NIL;
		int orgAttr = LIST_LENGTH(transInput->schema->attrDefs);

		FOREACH(AttributeDef,ia,transInput->schema->attrDefs)
		{
			Node *attrCond = NULL;
			AttributeReference *lA, *rA = NULL;
			lA = createFullAttrReference(strdup(ia->attrName), 0, aPos, 0, ia->dataType);

	//		// check suffix upfront to recognize if attributes are renamed
			for(int provAttr = orgAttr; provAttr < LIST_LENGTH(prov->schema->attrDefs); provAttr++)
			{
				AttributeDef *pa = getAttrDefByPos(prov,provAttr);

				if(isSuffix(pa->attrName,ia->attrName))
				{
					rA = createFullAttrReference(strdup(pa->attrName), 1, provAttr, 0, pa->dataType);
					attrCond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lA,rA));
					chkPos++;

					if(chkPos == 1)
						curCond = attrCond;
					else if (chkPos > 1)
						curCond = AND_EXPRS(curCond,attrCond);
				}
				else if(streq(ia->attrName,pa->attrName))
					FATAL_LOG("USING join is using ambiguous attribute references <%s>", ia->attrName);
			}
			aPos++;
		}

		// no matches exist on name, then match by position
		if(curCond == NULL || chkPos > orgAttr) // then there exist repeating attrs
		{
			List *orgRef = ((ProjectionOperator *) transInput)->projExprs;
			chkPos = 0;
			int attrPos = 0;
			curCond = NULL;

			FOREACH(AttributeReference,a,orgRef)
			{
				Node *attrCond;
				AttributeReference *lA, *rA = NULL;

				int matPos = a->attrPosition + LIST_LENGTH(orgRef);
				lA = createFullAttrReference(strdup(a->name), 0, attrPos, 0, a->attrType);

	//			for(int rPos = 0; rPos < LIST_LENGTH(prov->schema->attrDefs); rPos++)
				List *provRef = ((ProjectionOperator *) prov)->projExprs;

				FOREACH(AttributeReference,rPos,provRef)
				{
					if(rPos->attrPosition == matPos)
					{
	//					AttributeDef *r = getAttrDefByPos(prov,rPos);
						rA = createFullAttrReference(strdup(rPos->name), 1, rPos->attrPosition, 0, rPos->attrType);
						attrCond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lA,rA));
						chkPos++;

						if(chkPos == 1)
							curCond = attrCond;
						else if(chkPos > 1)
							curCond = AND_EXPRS(curCond,attrCond);
					}
				}
				attrPos++;
			}

			if(isDL)
			{
				// attrs from input query
				QueryOperator *dup = (QueryOperator *) transInput;
				int pos = 0;

				// create join condition based on the attr names
				FOREACH(AttributeDef,ia,dup->schema->attrDefs)
				{
					Node *attrCond;
					AttributeReference *lA = createFullAttrReference(strdup(ia->attrName), 0, pos, 0, ia->dataType);

					FOREACH(AttributeDef,ra,prov->schema->attrDefs)
					{
						if(isSuffix(ra->attrName,lA->name))
						{
							AttributeReference *rA = createFullAttrReference(strdup(ra->attrName), 1, pos, 0, ra->dataType);
							attrCond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(lA,rA));

							if(curCond == NULL)
								curCond = attrCond;
							else
								curCond = AND_EXPRS(curCond,attrCond);
						}
					}
					pos++;
				}
			}
		}

		inputs = LIST_MAKE(transInput,prov);

		// create join operator
		attrNames = concatTwoLists(getAttrNames(transInput->schema), getAttrNames(prov->schema));
		provJoin = (QueryOperator *) createJoinOp(JOIN_LEFT_OUTER, curCond, inputs, NIL, attrNames);
		makeAttrNamesUnique(provJoin);

		// set the parent of the operator's children
		OP_LCHILD(provJoin)->parents = OP_RCHILD(provJoin)->parents = singleton(provJoin);
	//	provJoin->provAttrs = copyObject(provAttrs);

		// create projection for join
		projExpr = NIL;
		pos = 0;

		FOREACH(AttributeDef,a,provJoin->schema->attrDefs)
		{
			if(!streq(a->attrName,strdup(HAS_PROV_ATTR)))
			{
				projExpr = appendToTailOfList(projExpr, createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
				pos++;
			}
		}

		AttributeReference *hasProv = createFullAttrReference(strdup(HAS_PROV_ATTR), 0, pos, 0, DT_INT);
		Node *cond = (Node *) createIsNullExpr((Node *) hasProv);
		Node *then = (Node *) createConstInt(0);
		Node *els = (Node *) createConstInt(1);

		CaseWhen *caseWhen = createCaseWhen(cond, then);
		CaseExpr *caseExpr = createCaseExpr(NULL, singleton(caseWhen), els);

		projExpr = appendToTailOfList(projExpr, (List *) caseExpr);
		DEBUG_LOG("projection expressions for join: %s", projExpr);

	//	attrNames = concatTwoLists(getAttrNames(transInput->schema), getAttrNames(prov->schema));

	//	Set *allNames = STRSET();
	//	List *uniqueAttrNames = CONCAT_LISTS(getQueryOperatorAttrNames(provJoin),singleton(hasProv->attrName));
	//	makeNamesUnique(uniqueAttrNames, allNames);

		op = createProjectionOp(projExpr, provJoin, NIL, getAttrNames(provJoin->schema));
		provJoin->parents = singleton(op);
	}

	provJoin = (QueryOperator *) op;
	provJoin->provAttrs = copyObject(provAttrs);

	// create duplicate removal
	projExpr = NIL;
	pos = 0;

	FOREACH(AttributeDef,a,provJoin->schema->attrDefs)
	{
		projExpr = appendToTailOfList(projExpr,
				createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));

		pos++;
	}

	QueryOperator *dr = (QueryOperator *) createDuplicateRemovalOp(projExpr, provJoin, NIL, getAttrNames(provJoin->schema));
	provJoin->parents = singleton(dr);
	provJoin = (QueryOperator *) dr;

	result = (Node *) provJoin;
	SET_BOOL_STRING_PROP(result, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("provenance for the question as an input to the summarization returned:", result);
	INFO_OP_LOG("provenance for the question as an input to the summarization as overview:", result);
//
//	DEBUG_NODE_BEATIFY_LOG("join input with provenance question for summarization returned:", result);
//	INFO_OP_LOG("join input with provenance question for summarization as overview:", result);

	return result;
}


/*
 * For SQL input, integrate a particular user's interest into provenance computation
 * for Datalog, this step should be skipped since it is already part of the output of PUG
 */
static Node *
rewriteUserQuestion (List *userQ, Node *rewrittenTree)
{
	Node *result;
	QueryOperator *input = NULL;

//	if(isA(rewrittenTree,List))
//		input = (QueryOperator *) getHeadOfListP((List *) rewrittenTree);
//	else
		input = (QueryOperator *) rewrittenTree;

	Node *prop = input->properties;

	if (provAttrs == NIL || normAttrs == NIL)
	{
		provAttrs = getProvenanceAttrs(input);
		normAttrs = getNormalAttrs(input);
	}

	// get attrRefs for the input
//	List *inputAttrRefs = ((ProjectionOperator *) input)->projExprs;
	List *inputAttrRefs = NIL;
	QueryOperator *inOp = (QueryOperator *) input;
	int pos = 0;

	FOREACH(AttributeDef,a,inOp->schema->attrDefs)
	{
		inputAttrRefs = appendToTailOfList(inputAttrRefs,
				createFullAttrReference(strdup(a->attrName), 0, pos, -1, a->dataType));
	}

	// check the list for constant value to create sel condition
	int chkPos = 0;
	int attrPos = 0;
	List *origUserQattrs = NIL;
	Node *curCond = NULL;
	SelectionOperator *so;

	FOREACH(Constant,c,userQ)
	{
		if (!streq(strdup(c->value),"*"))
		{
			char *attr = getAttrNameByPos(input,attrPos);
			AttributeDef *aDef = getAttrDefByName(input,attr);

			AttributeReference *quest = createFullAttrReference(strdup(attr), 0, attrPos, 0, aDef->dataType);
			Node *selCond = (Node *) createOpExpr(OPNAME_EQ,LIST_MAKE(quest,c));

			if(chkPos == 0)
				curCond = selCond;
			else
				curCond = AND_EXPRS(curCond,selCond);

			chkPos++;

			// store user question attrRefs for later use of attrDom computation
			origUserQattrs = appendToTailOfList(origUserQattrs,
						(AttributeReference *) getNthOfListP(inputAttrRefs,attrPos));
		}

		attrPos++;
	}
	so = createSelectionOp(curCond, input, NIL, NIL);
	userQuestion = origUserQattrs;

	input->parents = singleton(so);
	input = (QueryOperator *) so;

	// create projection operator
	pos = 0;
//	List *attrs = NIL;
	List *projExpr = NIL;
	ProjectionOperator *op;

	FOREACH(AttributeDef,p,input->schema->attrDefs)
	{
//		if(isPrefix(p->attrName,PROV_ATTR_PREFIX))
//		{
//			attrs = appendToTailOfList(attrs,p->attrName);

			projExpr = appendToTailOfList(projExpr,
					createFullAttrReference(strdup(p->attrName), 0, pos, 0, p->dataType));
//		}
		pos++;
	}

	op = createProjectionOp(projExpr, input, NIL, getAttrNames(input->schema));
	input->parents = singleton(op);
	input = (QueryOperator *) op;
	input->provAttrs = copyObject(provAttrs);

	input->properties = prop;
	result = (Node *) input;
//	SET_BOOL_STRING_PROP(rewrittenTree, PROP_MATERIALIZE);

	DEBUG_NODE_BEATIFY_LOG("provenance question for summarization:", result);
	INFO_OP_LOG("provenance question for summarization as overview:", result);

	return result;
}
