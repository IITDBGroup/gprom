/*-------------------------------------------------------------------------
 *
 * translator.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"

#include "mem_manager/mem_mgr.h"

#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

#include "analysis_and_translate/analyze_qb.h"
#include "analysis_and_translate/translator.h"

#include <stdio.h>


static QueryOperator *translateQuery (Node *node);

/* Three branches of translating a Query */
static QueryOperator *translateSetQuery(SetQuery *sq);
static QueryOperator *translateQueryBlock(QueryBlock *qb);
static QueryOperator *translateProvenanceStmt(ProvenanceStmt *prov);

/* Functions of translating from clause in a QueryBlock */
static QueryOperator *translateFromClause(List *fromClause);
static QueryOperator *buildJoinTreeFromOperatorList(List *opList);
static List *translateFromClauseToOperatorList(List *fromClause);
static List *getAttrsOffsets(List *fromClause);
static inline QueryOperator *createTableAccessOpFromFromTableRef(FromTableRef *ftr);
static QueryOperator *translateFromJoinExpr(FromJoinExpr *fje);
static QueryOperator *translateFromSubquery(FromSubquery *fsq);

/* Functions of translating where clause in a QueryBlock */
static QueryOperator *translateWhereClause(Node *whereClause, QueryOperator *joinTreeRoot, List *attrsOffsets);
static boolean visitAttrRefToSetNewAttrPos(Node *n, List *state);

/* Functions of translating simple select clause in a QueryBlock */
static QueryOperator *translateSelectClause(List *selectClause, QueryOperator *select, List *attrsOffsets);

/* Functions of translating aggregations, having and group by */
static QueryOperator *translateHavingClause(Node *havingClause, QueryOperator *input, List *attrsOffsets);
static QueryOperator *translateAggregation(List *selectClause, Node *havingClause, List *groupByClause, QueryOperator *input, List *attrsOffsets);
static QueryOperator *createProjectionOverNonAttrRefExprs(List *selectClause, Node *havingClause, List *groupByClause, QueryOperator *input, List *attrsOffsets);
static List *getListOfNonAttrRefExprs(List *selectClause, Node *havingClause, List *groupByClause);
static List *getListOfAggregFunctionCalls(List *selectClause, Node *havingClause);
static boolean visitAggregFunctionCall(Node *n, void *state);


Node *
translateParse(Node *q)
{
    Node *result;
    NEW_AND_ACQUIRE_MEMCONTEXT("TRANSLATOR_CONTEXT");
    analyzeQueryBlockStmt(q);

    INFO_LOG("translate QB model \n%s", nodeToString(q));

    if (isA(q, List))
    {
        result = (Node *) copyList((List *) q);
        FOREACH(Node,stmt,(List *) result)
            stmt_his_cell->data.ptr_value = (Node *) translateQuery(stmt);
    }
    else
        result = (Node *) translateQuery (q);

    INFO_LOG("result of translation is \n%s", beatify(nodeToString(result)));
    assert(equal(result, copyObject(result)));

    FREE_MEM_CONTEXT_AND_RETURN_COPY(Node,result);
}

static QueryOperator *
translateQuery (Node *node)
{
    DEBUG_LOG("translate query <%s>", node);

    switch(node->type)
    {
        case T_QueryBlock:
            return translateQueryBlock((QueryBlock *) node);
        case T_SetQuery:
            return translateSetQuery((SetQuery *) node);
        case T_ProvenanceStmt:
            return translateProvenanceStmt((ProvenanceStmt *) node);
        default:
            assert(FALSE);
            return NULL;
    }
}

static QueryOperator *
translateSetQuery(SetQuery *sq)
{
    QueryOperator *left = NULL;
    QueryOperator *right = NULL;
    if (sq->lChild)
    {
        if (sq->lChild->type == T_SetQuery)
            left = translateSetQuery((SetQuery *) sq->lChild);
        else if (sq->lChild->type == T_QueryBlock)
            left = translateQueryBlock(((QueryBlock *) sq->lChild));
    }
    if (sq->rChild)
    {
        if (sq->rChild->type == T_SetQuery)
            right = translateSetQuery((SetQuery *) sq->rChild);
        else if (sq->rChild->type == T_QueryBlock)
            right = translateQueryBlock(((QueryBlock *) sq->rChild));
    }
    assert(left && right);

    // set children of the set operator node
    List *inputs = appendToTailOfList(inputs, left);
    inputs = appendToTailOfList(inputs, right);

    // create set operator node
    SetOperator *so = createSetOperator(sq->setOp, inputs, NIL, sq->selectClause);

    // set the parent of the operator's children
    OP_LCHILD(so)->parents = OP_RCHILD(so)->parents = singleton(so);

    return ((QueryOperator *) so);
}

static QueryOperator *
translateQueryBlock(QueryBlock *qb)
{
	List *attrsOffsets = NIL;

	INFO_LOG("translate a QB:\n%s", nodeToString(qb));

	QueryOperator *joinTreeRoot = translateFromClause(qb->fromClause);
	INFO_LOG("translatedFrom is\n%s", nodeToString(joinTreeRoot));

	attrsOffsets = getAttrsOffsets(qb->fromClause);
	QueryOperator *select = translateWhereClause(qb->whereClause, joinTreeRoot,
			attrsOffsets);
	INFO_LOG("translatedWhere is\n%s", nodeToString(select));

	QueryOperator *aggr = translateAggregation(qb->selectClause,
			qb->havingClause, qb->groupByClause, select, attrsOffsets);
	if (aggr != select)
		INFO_LOG("translatedAggregation is\n%s", nodeToString(aggr));

	QueryOperator *having = translateHavingClause(qb->havingClause, aggr,
			attrsOffsets);
	if (having != aggr)
		INFO_LOG("translatedHaving is\n%s", nodeToString(having));

	QueryOperator *project = translateSelectClause(qb->selectClause, having,
			attrsOffsets);
	INFO_LOG("translatedSelect is\n%s", nodeToString(project));

	// TODO translate aggregation
	return project;
}

static QueryOperator *
translateProvenanceStmt(ProvenanceStmt *prov)
{
    QueryOperator *child;
    ProvenanceComputation *result;
    List *attrs = NIL;
    Schema *schema = NULL;
    //TODO create attribute list by analyzing subquery under child
    child = translateQuery(prov->query);

    result = createProvenanceComputOp(PI_CS, singleton(child), NIL, NIL, attrs); //TODO adapt function parameters

    child->parents = singleton(result);

    return (QueryOperator *) result;
}

static QueryOperator *
translateFromClause(List *fromClause)
{
    List *opList = translateFromClauseToOperatorList(fromClause);
    return buildJoinTreeFromOperatorList(opList);
}

static QueryOperator *
buildJoinTreeFromOperatorList(List *opList)
{
    DEBUG_LOG("build join tree from operator list\n%s", nodeToString(opList));

    QueryOperator *root = (QueryOperator *) getHeadOfListP(opList);
    FOREACH(QueryOperator, op, opList)
    {
        if (op == (QueryOperator *) getHeadOfListP(opList))
            continue;

        QueryOperator *oldRoot = (QueryOperator *) root;
        List *inputs = NIL;
        // set children of the join node
        inputs = appendToTailOfList(inputs, oldRoot);
        inputs = appendToTailOfList(inputs, op);

        // contact children's attribute names as the node's attribute names
        List *attrNames = concatTwoLists(getAttrNames(oldRoot->schema), getAttrNames(op->schema));

        // create join operator
        root = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);

        // set the parent of the operator's children
        OP_LCHILD(root)->parents = OP_RCHILD(root)->parents = singleton(root);
    }

    DEBUG_LOG("join tree for translated from is\n%s", nodeToString(root));

    return root;
}

static List *
getAttrsOffsets(List *fromClause)
{
    int len = getListLength(fromClause);
    List *offsets = NIL;
    int curOffset = 0;

    FOREACH(FromItem, from, fromClause)
    {
       offsets = appendToTailOfListInt(offsets, curOffset);
       curOffset += getListLength(from->attrNames);
    }

    DEBUG_LOG("attribute offsets for from clause items are %s", offsets);

    return offsets;
}

static List *
translateFromClauseToOperatorList(List *fromClause)
{
    List *opList = NIL;

    DEBUG_LOG("translate from clause");

    FOREACH(FromItem, from, fromClause)
    {
        QueryOperator *op = NULL;
        switch (from->type)
        {
            case T_FromTableRef:
                op = createTableAccessOpFromFromTableRef((FromTableRef *) from);
                break;
            case T_FromJoinExpr:
                op = translateFromJoinExpr((FromJoinExpr *) from);
                break;
            case T_FromSubquery:
                op = translateFromSubquery((FromSubquery *) from);
                break;
            default:
                FATAL_LOG("did not expect node <%s> in from list", nodeToString(from));
                break;
        }

        assert(op);
        opList = appendToTailOfList(opList, op);
    }

    assert(opList);
    DEBUG_LOG("translated from clause into list of operator trees is \n%s", nodeToString(opList));
    return opList;
}

static inline QueryOperator *
createTableAccessOpFromFromTableRef(FromTableRef *ftr)
{
    TableAccessOperator *ta = createTableAccessOp(ftr->tableId, ftr->from.name,
                NIL, ftr->from.attrNames, NIL); // TODO  get data types
    DEBUG_LOG("translated table access:\n%s\nINTO\n%s", nodeToString(ftr), nodeToString(ta));
    return ((QueryOperator *) ta);
}

static QueryOperator *
translateFromJoinExpr(FromJoinExpr *fje)
{
    QueryOperator *input1 = NULL;
    QueryOperator *input2 = NULL;
    switch (fje->left->type)
    {
        case T_FromTableRef:
            input1 = createTableAccessOpFromFromTableRef(
                    (FromTableRef *) fje->left);
            break;
        case T_FromJoinExpr:
            input1 = translateFromJoinExpr((FromJoinExpr *) fje->left);
            break;
        case T_FromSubquery:
            input1 = translateFromSubquery((FromSubquery *) fje->left);
            break;
        default:
            FATAL_LOG("did not expect node <%s> in from list", nodeToString(input1));
            break;
    }
    switch (fje->right->type)
    {
        case T_FromTableRef:
            input2 = createTableAccessOpFromFromTableRef(
                    (FromTableRef *) fje->right);
            break;
        case T_FromJoinExpr:
            input2 = translateFromJoinExpr((FromJoinExpr *) fje->right);
            break;
        case T_FromSubquery:
            input2 = translateFromSubquery((FromSubquery *) fje->right);
            break;
        default:
            FATAL_LOG("did not expect node <%s> in from list", nodeToString(input2));
            break;
    }

    assert(input1 && input2);

    // set children of the join operator node
    List *inputs = appendToTailOfList(inputs, input1);
    inputs = appendToTailOfList(inputs, input2);

    // create join operator node
    JoinOperator *jo = createJoinOp(fje->joinType, fje->cond, inputs, NIL,
            fje->from.attrNames); // TODO merge schema?

    // set the parent of the operator's children
    OP_LCHILD(jo)->parents = OP_RCHILD(jo)->parents = singleton(jo);

    if (fje->joinCond == JOIN_COND_NATURAL)
    {
        // TODO create projection?
    }

    return ((QueryOperator *) jo);
}

static QueryOperator *
translateFromSubquery(FromSubquery *fsq)
{
    return translateQuery(fsq->subquery);
    //TODO set attr names from FromItem
}

static QueryOperator *
translateWhereClause(Node *whereClause, QueryOperator *joinTreeRoot,
		List *attrsOffsets) {
	if (whereClause == NULL)
		return joinTreeRoot;

	// create selection operator node upon the root of the join tree
	SelectionOperator *so = createSelectionOp(whereClause, joinTreeRoot, NIL,
			getAttrNames(joinTreeRoot->schema));

	// change attributes positions in selection condition
	visitAttrRefToSetNewAttrPos(so->cond, attrsOffsets);

	// set the parent of the operator's children
	OP_LCHILD(so)->parents = singleton(so);

	return ((QueryOperator *) so);
}

static boolean
visitAttrRefToSetNewAttrPos(Node *n, List *state)
{
    if (n == NULL)
        return TRUE;

    int *offsets = (int *) state;
    if (isA(n, AttributeReference))
    {
        AttributeReference *attrRef = (AttributeReference *) n;
        attrRef->attrPosition += getNthOfListInt(state, attrRef->fromClauseItem);
        attrRef->fromClauseItem = 0;
    }

    return visit(n, visitAttrRefToSetNewAttrPos, offsets);
}

static QueryOperator *
translateSelectClause(List *selectClause, QueryOperator *select,
        List *attrsOffsets)
{
    List *attrNames = NIL;
    List *projExprs = NIL;

    // determine projection expressions
    // visit each expression in select clause to get attribute names
    FOREACH(SelectItem, s, selectClause)
    {
        Node *projExpr = copyObject(s->expr);
        projExprs = appendToTailOfList(projExprs, projExpr);

        // change attribute position in attribute reference in each projection expression
        visitAttrRefToSetNewAttrPos(projExpr, attrsOffsets);

        // add attribute names
        attrNames = appendToTailOfList(attrNames, strdup(s->alias));
    }

    // create projection operator upon selection operator from select clause
    ProjectionOperator *po = createProjectionOp(projExprs, select, NIL,
            attrNames);

    // set the parent of the operator's children
    OP_LCHILD(po)->parents = singleton(po);

    return ((QueryOperator *) po);
}

static QueryOperator *
translateHavingClause(Node *havingClause, QueryOperator *input, List *attrsOffsets)
{
	QueryOperator *output = input;

	if (havingClause)
	{
		List *attrNames = getAttrNames(input->schema);

		// create selection operator over having clause
		SelectionOperator *so = createSelectionOp (havingClause, input, NIL, attrNames);

		// set the parent of the selection's child
		OP_LCHILD(so)->parents = singleton(so);

		// change attributes positions in having condition
		visitAttrRefToSetNewAttrPos(so->cond, attrsOffsets);

		output = (QueryOperator *) so;
	}

	return output;
}

static QueryOperator *
translateAggregation(List *selectClause, Node *havingClause,
		List *groupByClause, QueryOperator *input, List *attrsOffsets)
{
	QueryOperator *in = createProjectionOverNonAttrRefExprs(selectClause,
			havingClause, groupByClause, input, attrsOffsets);
	QueryOperator *output = in;
	List *aggrs = getListOfAggregFunctionCalls(selectClause, havingClause);
	if (getListLength(aggrs) > 0)
	{
		List *attrNames = getAttrNames(in->schema); // TODO what should attrNames be?

		// copy aggregation function calls and groupBy expressions
		// and create aggregation operator
		AggregationOperator *ao = createAggregationOp(aggrs, groupByClause,
				in, NIL, attrNames);

		// set the parent of the aggregation's child
		OP_LCHILD(ao)->parents = singleton(ao);

		// change attributes positions in each expression of groupBy
		FOREACH(Node, exp, ao->groupBy)
		{
			visitAttrRefToSetNewAttrPos(exp, attrsOffsets);
		}

		output = ((QueryOperator *) ao);
	}

	freeList(aggrs);
	return output;
}

static QueryOperator *
createProjectionOverNonAttrRefExprs(List *selectClause, Node *havingClause,
		List *groupByClause, QueryOperator *input, List *attrsOffsets)
{
	// each entry of the list directly points to the original expression, not copy
	List *projExprs = getListOfNonAttrRefExprs(selectClause, havingClause,
			groupByClause);

	QueryOperator *output = input;

	if (getListLength(projExprs) > 0)
	{
		// create alias for each non-AttributeReference expression
		List *attrNames = NIL;
		int i = 0;
		FOREACH(Node, expr, projExprs)
		{
			char alias[20] = "agg_gb_arg";
			char postfix[10];
			sprintf(postfix, "%d", i);
			attrNames = appendToTailOfList(attrNames, strdup(strcat(alias, postfix)));
			i++;
		}

		// copy expressions and create projection operator over the copies
		ProjectionOperator *po = createProjectionOp(projExprs, input, NIL,
				attrNames);

		// set the parent of the projection's child
		OP_LCHILD(po)->parents = singleton(po);

		// change attributes positions in each expression copy
		FOREACH(Node, exp, po->projExprs)
		{
			visitAttrRefToSetNewAttrPos(exp, attrsOffsets);
		}

		// replace non-AttributeReference arguments in aggregation with alias
		// each entry of the list directly points to the original aggregation, not copy
		List *aggregs = getListOfAggregFunctionCalls(selectClause,
				havingClause);
		i = 0;
		FOREACH(FunctionCall, agg, aggregs)
		{
			FOREACH(void, arg, agg->args)
			{
				if (!isA(arg, AttributeReference))
				{
					deepFree(arg);
					arg = (char *) getNthOfListP(attrNames, i); // TODO ?
					i++;
				}
			}
		}
		freeList(aggregs);

		// replace non-AttributeReference expressions in groupBy with alias
		int len = getListLength(attrNames);
		if (groupByClause)
		{
			FOREACH(void, expr, groupByClause)
			{
				if (!isA(expr, AttributeReference) && i < len)
				{
					deepFree(expr);
					expr = (char *) getNthOfListP(attrNames, i); // TODO ?
					i++;
				}
			}
		}
		output = ((QueryOperator *) po);
	}

	freeList(projExprs);
	return output;
}

static List *
getListOfNonAttrRefExprs(List *selectClause, Node *havingClause, List *groupByClause)
{
	List *nonAttrRefExprs = newList(T_List);
	List *aggregs = getListOfAggregFunctionCalls(selectClause, havingClause);

	// get non-AttributeReference expressions from arguments of aggregations
	FOREACH(FunctionCall, agg, aggregs)
	{
		FOREACH(Node, arg, agg->args)
		{
			if (!isA(arg, AttributeReference))
				nonAttrRefExprs = appendToTailOfList(nonAttrRefExprs, arg);
		}
	}

	if (groupByClause)
	{
		// get non-AttributeReference expressions from group by clause
		FOREACH(Node, expr, groupByClause)
		{
			if (!isA(expr, AttributeReference))
				nonAttrRefExprs = appendToTailOfList(nonAttrRefExprs, expr);
		}
	}

	freeList(aggregs);
	return nonAttrRefExprs;
}

/*
static boolean
visitNonAttrRefExpr(Node *n, void *state)
{
	if (n == NULL)
		return TRUE;

	List *exprs = (List *) state;
	if (!isA(n, AttributeReference))
	{
		exprs = appendToTailOfList(exprs, n);
	}

	return visit(n, visitNonAttrRefExpr, exprs);
}
*/

static List *
getListOfAggregFunctionCalls(List *selectClause, Node *havingClause)
{
	List *aggregs = newList(T_List);
	// get aggregations from select clause
	FOREACH(Node, sel, selectClause)
	{
		visitAggregFunctionCall(sel, aggregs);
	}
	// get aggregations from having clause
	visitAggregFunctionCall(havingClause, aggregs);

	return aggregs;
}

static boolean
visitAggregFunctionCall(Node *n, void *state)
{
	if (n == NULL)
		return TRUE;

	List *aggregs = (List *) state;
	if (isA(n, FunctionCall)) // TODO how to prevent from going into nested sub-query?
	{
		FunctionCall *fc = (FunctionCall *) n;
		if (fc->isAgg)
		{
			DEBUG_LOG("Found aggregation '%s'.", fc->functionname);
			aggregs = appendToTailOfList(aggregs, fc);
		}
	}

	return visit(n, visitAggregFunctionCall, aggregs);
}



