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
#include "analysis_and_translate/translate_update.h"

// data types
typedef struct ReplaceGroupByState
{
    List *expressions;
    List *attrNames;
} ReplaceGroupByState;

// function declarations
static Node *translateGeneral (Node *node);

/* Three branches of translating a Query */
static QueryOperator *translateSetQuery(SetQuery *sq);
static QueryOperator *translateQueryBlock(QueryBlock *qb);
static QueryOperator *translateProvenanceStmt(ProvenanceStmt *prov);

/* Functions of translating from clause in a QueryBlock */
static QueryOperator *translateFromClause(List *fromClause);
static QueryOperator *buildJoinTreeFromOperatorList(List *opList);
static List *translateFromClauseToOperatorList(List *fromClause);
static List *getAttrsOffsets(List *fromClause);
static inline QueryOperator *createTableAccessOpFromFromTableRef(
        FromTableRef *ftr);
static QueryOperator *translateFromJoinExpr(FromJoinExpr *fje);
static QueryOperator *translateFromSubquery(FromSubquery *fsq);

/* Functions of translating where clause in a QueryBlock */
static QueryOperator *translateWhereClause(Node *whereClause,
        QueryOperator *joinTreeRoot, List *attrsOffsets);
static boolean visitAttrRefToSetNewAttrPos(Node *n, List *state);

/* Functions of translating simple select clause in a QueryBlock */
static QueryOperator *translateSelectClause(List *selectClause,
        QueryOperator *select, List *attrsOffsets, boolean hasAgg);

/* Functions of translating aggregations, having and group by */
static QueryOperator *translateHavingClause(Node *havingClause,
        QueryOperator *input, List *attrsOffsets);
static QueryOperator *translateAggregation(QueryBlock *qb, QueryOperator *input,
        List *attrsOffsets);
static Node *replaceAggsAndGroupByMutator (Node *node,
        ReplaceGroupByState *state);
static QueryOperator *createProjectionOverNonAttrRefExprs(List *selectClause,
        Node *havingClause, List *groupByClause, QueryOperator *input,
        List *attrsOffsets);
static List *getListOfNonAttrRefExprs(List *selectClause, Node *havingClause, List *groupByClause);
static List *getListOfAggregFunctionCalls(List *selectClause, Node *havingClause);
static boolean visitAggregFunctionCall(Node *n, List **aggregs);


Node *
translateParse(Node *q)
{
    Node *result;
    NEW_AND_ACQUIRE_MEMCONTEXT("TRANSLATOR_CONTEXT");
    analyzeQueryBlockStmt(q, NULL);

    INFO_LOG("translate QB model \n%s", nodeToString(q));

    result = translateGeneral(q);

    INFO_LOG("result of translation is \n%s", beatify(nodeToString(result)));
    assert(equal(result, copyObject(result)));

    FREE_MEM_CONTEXT_AND_RETURN_COPY(Node,result);
}

static Node *translateGeneral (Node *node)
{
    Node *result;

    if (isA(node, List))
    {
        result = (Node *) copyList((List *) node);
        FOREACH(Node,stmt,(List *) result)
            stmt_his_cell->data.ptr_value = (Node *) translateQuery(stmt);
    }
    else
        result = (Node *) translateQuery(node);
    return result;
}

QueryOperator *
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
        case T_Insert:
        case T_Update:
        case T_Delete:
            return translateUpdate(node);
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
        left = translateQuery(sq->lChild);
//        if (sq->lChild->type == T_SetQuery)
//            left = translateSetQuery((SetQuery *) sq->lChild);
//        else if (sq->lChild->type == T_QueryBlock)
//            left = translateQueryBlock(((QueryBlock *) sq->lChild));
    }
    if (sq->rChild)
    {
        left = translateQuery(sq->rChild);
//        if (sq->rChild->type == T_SetQuery)
//            right = translateSetQuery((SetQuery *) sq->rChild);
//        else if (sq->rChild->type == T_QueryBlock)
//            right = translateQueryBlock(((QueryBlock *) sq->rChild));
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
	boolean hasAggOrGroupBy= FALSE;

	INFO_LOG("translate a QB:\n%s", nodeToString(qb));

	QueryOperator *joinTreeRoot = translateFromClause(qb->fromClause);
	INFO_LOG("translatedFrom is\n%s", nodeToString(joinTreeRoot));

	attrsOffsets = getAttrsOffsets(qb->fromClause);
	QueryOperator *select = translateWhereClause(qb->whereClause, joinTreeRoot,
			attrsOffsets);
	if (select != joinTreeRoot)
	    INFO_LOG("translatedWhere is\n%s", nodeToString(select));

	QueryOperator *aggr = translateAggregation(qb, select, attrsOffsets);
	hasAggOrGroupBy = (aggr != select);
	if (hasAggOrGroupBy)
		INFO_LOG("translatedAggregation is\n%s", nodeToString(aggr));

	QueryOperator *having = translateHavingClause(qb->havingClause, aggr,
			attrsOffsets);
	if (having != aggr)
		INFO_LOG("translatedHaving is\n%s", nodeToString(having));

	QueryOperator *project = translateSelectClause(qb->selectClause, having,
			attrsOffsets, hasAggOrGroupBy);
	INFO_LOG("translatedSelect is\n%s", nodeToString(project));

	return project;
}

static QueryOperator *
translateProvenanceStmt(ProvenanceStmt *prov)
{
    QueryOperator *child;
    List *children = NIL;
    ProvenanceComputation *result;
    Schema *schema = NULL;

    result = createProvenanceComputOp(prov->provType, NIL, NIL, prov->selectClause, NULL);
    result->inputType = prov->inputType;
    result->asOf = copyObject(prov->asOf);

    switch(prov->inputType)
    {
        case PROV_INPUT_TRANSACTION:
            break;
        case PROV_INPUT_UPDATE_SEQUENCE:
        {
            ProvenanceTransactionInfo *tInfo = makeNode(ProvenanceTransactionInfo);

            result->transactionInfo = tInfo;
            tInfo->originalUpdates = copyObject(prov->query);
            tInfo->updateTableNames = NIL;

            FOREACH(Node,n,(List *) prov->query)
            {
                char *tableName;

                /* get table name */
                switch(n->type)
                {
                    case T_Insert:
                        tableName = ((Insert *) n)->tableName;
                        break;
                    case T_Update:
                        tableName = ((Update *) n)->nodeName;
                        break;
                    case T_Delete:
                        tableName = ((Delete *) n)->nodeName;
                        break;
                    case T_QueryBlock:
                    case T_SetQuery:
                        tableName = strdup("_NONE");
                        break;
                    default:
                        FATAL_LOG("Unexpected node type %u as input to provenance computation", n->type);
                        break;
                }

                tInfo->updateTableNames = appendToTailOfList(
                        tInfo->updateTableNames, strdup(tableName));
                tInfo->scns = appendToTailOfListInt(tInfo->scns, -1); //TODO get SCN

                // translate and add update as child to provenance computation
                child = translateQuery(n);
                addChildOperator ((QueryOperator *) result, child);
            }
        }
            break;
        case PROV_INPUT_UPDATE:
        case PROV_INPUT_QUERY:
            child = translateQuery(prov->query);
            addChildOperator((QueryOperator *) result, child);
            break;
        case PROV_INPUT_TIME_INTERVAL:
            break;
    }

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
        OP_LCHILD(root)->parents = singleton(root);
        OP_RCHILD(root)->parents = singleton(root);
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

    DEBUG_LOG("attribute offsets for from clause items are %s", nodeToString(offsets));

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
    TableAccessOperator *ta = createTableAccessOp(ftr->tableId, NULL,ftr->from.name,
                NIL, ftr->from.attrNames, NIL); // TODO  get data types
    DEBUG_LOG("translated table access:\n%s\nINTO\n%s", nodeToString(ftr), nodeToString(ta));
    return ((QueryOperator *) ta);
}

static QueryOperator *
translateFromJoinExpr(FromJoinExpr *fje)
{
    QueryOperator *input1 = NULL;
    QueryOperator *input2 = NULL;
    Node *joinCond = NULL;
    List *commonAttrs = NIL;
    List *uniqueRightAttrs = NIL;
    List *attrNames = fje->from.attrNames;

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
    List *inputs = LIST_MAKE(input1, input2);

    // NATURAL join condition, create equality condition for all common attributes
    if (fje->joinCond == JOIN_COND_NATURAL)
    {
        List *leftAttrs = getQueryOperatorAttrNames(input1);
        List *rightAttrs = getQueryOperatorAttrNames(input2);
        List *commonAttRefs = NIL;
        int lPos = 0;

        // search for common attributes and create condition for equality comparisons
        FOREACH(char,rA,rightAttrs)
        {
            int rPos = listPosString(leftAttrs, rA);
            if(rPos != -1)
            {
                AttributeReference *lRef = createFullAttrReference(strdup(rA), 0, lPos, 0);
                AttributeReference *rRef = createFullAttrReference(strdup(rA), 1, rPos, 0);

                commonAttrs = appendToTailOfList(commonAttrs, rA);
                joinCond = AND_EXPRS((Node *) createOpExpr("=", LIST_MAKE(lRef,rRef)), joinCond);
            }
            else
                uniqueRightAttrs = appendToTailOfList(uniqueRightAttrs, rA);
            lPos++;
        }

        DEBUG_LOG("common attributes for natural join <%s>, unique right "
                "attrs <%s>, with left <%s> and right <%s>",
                stringListToString(commonAttrs),
                stringListToString(uniqueRightAttrs),
                stringListToString(leftAttrs),
                stringListToString(rightAttrs));

        // need to update attribute names for join result
        attrNames = concatTwoLists(leftAttrs, rightAttrs);
    }
    // USING (a1, an) join create condition as l.a1 = r.a1 AND ... AND l.an = r.an
    else if (fje->joinCond == JOIN_COND_USING)
    {
        Node *curCond = NULL;

        FOREACH(char,a,(List *)fje->cond)
        {
            Node *attrCond;
            AttributeReference *lA, *rA;
            int aPos = 0;
            boolean found = FALSE;

            lA = createAttributeReference(a);
            lA->fromClauseItem = 0;
            rA = createAttributeReference(a);
            rA->fromClauseItem = 1;

            FOREACH(AttributeDef,a1,input1->schema->attrDefs)
            {
                if (strcmp(a,a1->attrName) == 0)
                {
                    if (found)
                        FATAL_LOG("USING join is using ambiguous attribute"
                                " references <%s>", a);
                    else
                        lA->attrPosition = aPos;
                }
                aPos++;
            }

            aPos = 0;
            FOREACH(AttributeDef,a2,input2->schema->attrDefs)
            {
                if (strcmp(a,a2->attrName) == 0)
                {
                    if (found)
                        FATAL_LOG("USING join is using ambiguous attribute"
                                " references <%s>", a);
                    else
                        rA->attrPosition = aPos;
                }
                aPos++;
            }

            // create equality condition and update global condition
            attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
            curCond = AND_EXPRS(attrCond,curCond);
        }

        joinCond = curCond;
    }
    // inner join
    else
        joinCond = fje->cond;

    // create join operator node
    JoinOperator *jo = createJoinOp(fje->joinType, joinCond, inputs, NIL,
            attrNames);

    // set the parent of the operator's children
    OP_LCHILD(jo)->parents = OP_RCHILD(jo)->parents = singleton(jo);

    // create projection for natural join
    if (fje->joinCond == JOIN_COND_NATURAL)
    {
        ProjectionOperator *op;
        List *projExpr = NIL;
        int pos = 0;

        FOREACH(AttributeDef,a,input1->schema->attrDefs)
        {
            projExpr = appendToTailOfList(projExpr,
                    createFullAttrReference(strdup(a->attrName), 0, pos, 0));
            pos++;
        }
        FOREACH(AttributeDef,a,input2->schema->attrDefs)
        {
            if (!searchListString(commonAttrs, a->attrName))
                projExpr = appendToTailOfList(projExpr,
                        createFullAttrReference(strdup(a->attrName), 1, pos, 0));
        }

        DEBUG_LOG("projection expressions for natural join: %s", projExpr);

        op = createProjectionOp(projExpr, (QueryOperator *) jo, NIL, fje->from.attrNames);
        jo->op.parents = singleton(op);

        return ((QueryOperator *) op);
    }
    else
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
        List *attrsOffsets, boolean hasAgg)
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
        // this is not necessary if an aggregation operator has been added
        if (!hasAgg)
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
		//visitAttrRefToSetNewAttrPos(so->cond, attrsOffsets);

		output = (QueryOperator *) so;
	}

	return output;
}

static QueryOperator *
translateAggregation(QueryBlock *qb, QueryOperator *input, List *attrsOffsets)
{
	QueryOperator *in;
	AggregationOperator *ao;
	List *selectClause = qb->selectClause;
	Node *havingClause = qb->havingClause;
	List *groupByClause = qb->groupByClause;
	List *attrNames = NIL;
    int i;
	List *aggrs = getListOfAggregFunctionCalls(selectClause, havingClause);
	List *aggPlusGroup;
	int numAgg = LIST_LENGTH(aggrs);
	int numGroupBy = LIST_LENGTH(groupByClause);
	List *newGroupBy;
	ReplaceGroupByState *state;

	aggPlusGroup = concatTwoLists(copyList(aggrs), copyList(groupByClause));

	// does query use aggregation or group by at all?
	if (numAgg == 0 && numGroupBy == 0)
	    return input;

	// if necessary create projection for aggregation inputs that are not simple
	// attribute references
	in = createProjectionOverNonAttrRefExprs(selectClause,
	            havingClause, groupByClause, input, attrsOffsets);

    // if no projection was added change attributes positions in each
	// expression of groupBy and aggregation input to refer to the FROM clause
	// translation
	if (in == input)
	{
	    FOREACH(Node, exp, groupByClause)
	        visitAttrRefToSetNewAttrPos(exp, attrsOffsets);
	    FOREACH(FunctionCall, agg, aggrs)
	        FOREACH(Node, aggIn, agg->args)
	            visitAttrRefToSetNewAttrPos(aggIn, attrsOffsets);
	}

	// create fake attribute names for aggregation output schema
	for(i = 0; i < LIST_LENGTH(aggrs); i++)
	    attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("aggr_", itoa(i)));
	for(i = 0; i < LIST_LENGTH(groupByClause); i++)
	    attrNames = appendToTailOfList(attrNames, CONCAT_STRINGS("group_", itoa(i)));

	// copy aggregation function calls and groupBy expressions
	// and create aggregation operator
	ao = createAggregationOp(aggrs, groupByClause, in, NIL, attrNames);

	// set the parent of the aggregation's child
	OP_LCHILD(ao)->parents = singleton(ao);

	//TODO replace aggregation function calls and group by expressions in select and having with references to aggrgeation output attributes
	state = NEW(ReplaceGroupByState);
	state->expressions = aggPlusGroup;
	state->attrNames = attrNames;

	qb->selectClause = (List *) replaceAggsAndGroupByMutator((Node *) selectClause,
	        state);
    qb->havingClause = replaceAggsAndGroupByMutator((Node *) havingClause,
            state);
//    qb->groupByClause = NIL;
//
//    for(i = 0; i < numGroupBy; i++)
//    {
//        AttributeReference *newA = createFullAttrReference(
//                CONCAT_STRINGS("group_", itoa(i)), 0, i, 0);
//        qb->groupByClause = appendToTailOfList(qb->groupByClause, newA);
//    }

	freeList(aggrs);
	FREE(state);

	return (QueryOperator *) ao;
}

static Node *
replaceAggsAndGroupByMutator (Node *node, ReplaceGroupByState *state)
{
    int i = 0;

    if (node == NULL)
        return NULL;

    // if node is an expression replace it
    FOREACH(Node,e,state->expressions)
    {
        char *attrName;

        if (equal(node, e))
        {
            attrName = (char *) getNthOfListP(state->attrNames, i);
            return (Node *) createFullAttrReference(strdup(attrName), 0, i, 0);
        }
        i++;
    }

    return mutate(node, replaceAggsAndGroupByMutator, state);
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
        INFO_LOG("create new projection for aggregation function inputs and "
                "group by expressions: %s", nodeToString(projExprs));

        // create alias for each non-AttributeReference expression
        List *attrNames = NIL;
        int i = 0;
        FOREACH(Node, expr, projExprs)
        {
            attrNames = appendToTailOfList(attrNames,
                    CONCAT_STRINGS("agg_gb_arg", itoa(i)));
            i++;
        }

        // copy expressions and create projection operator over the copies
        ProjectionOperator *po = createProjectionOp(projExprs, input, NIL,
                attrNames);

        // set the parent of the projection's child
        OP_LCHILD(po)->parents = singleton(po);

        // change attributes positions in each expression copy to refer to from outputs
        FOREACH(Node, exp, po->projExprs)
            visitAttrRefToSetNewAttrPos(exp, attrsOffsets);

        // replace non-AttributeReference arguments in aggregation with alias
        // each entry of the list directly points to the original aggregation, not copy
        List *aggregs = getListOfAggregFunctionCalls(selectClause,
                havingClause);
        i = 0;
        FOREACH(FunctionCall, agg, aggregs)
        {
            FOREACH(Node, arg, agg->args)
            {
                AttributeReference *new;
                char *aName = strdup((char *) getNthOfListP(attrNames, i));

                if (!isA(arg, AttributeReference))
                {
                    new = createFullAttrReference(aName, 0, i, 0);
//                    deepFree(arg);
                    arg_his_cell->data.ptr_value = new;
                }
                else
                {
                    new =  (AttributeReference *) arg;
                    new->name = aName;
                    new->fromClauseItem = 0;
                    new->attrPosition = i;
                }
                i++;
            }
        }
        freeList(aggregs);

        // replace non-AttributeReference expressions in groupBy with alias
        int len = getListLength(attrNames);
        if (groupByClause)
        {
            FOREACH(Node, expr, groupByClause)
            {
                AttributeReference *new;
                char *aName = strdup((char *) getNthOfListP(attrNames, i));

                if (!isA(expr, AttributeReference) && i < len)
                {
//                    deepFree(expr);
                    new = createFullAttrReference(aName, 0, i, 0);
                    expr_his_cell->data.ptr_value = new;
                }
                else
                {
                    new =  (AttributeReference *) expr;
                    new->name = aName;
                    new->fromClauseItem = 0;
                    new->attrPosition = i;
                }
                i++;
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
    List *nonAttrRefExprs = NIL;
    List *aggregs = getListOfAggregFunctionCalls(selectClause, havingClause);
    boolean needProjection = FALSE;

    // get non-AttributeReference expressions from arguments of aggregations
    FOREACH(FunctionCall, agg, aggregs)
    {
        FOREACH(Node, arg, agg->args)
        {
            nonAttrRefExprs = appendToTailOfList(nonAttrRefExprs, arg);
            if (!isA(arg, AttributeReference))
                needProjection = TRUE;
        }
    }

    if (groupByClause)
    {
        // get non-AttributeReference expressions from group by clause
        FOREACH(Node, expr, groupByClause)
        {
            nonAttrRefExprs = appendToTailOfList(nonAttrRefExprs, expr);
            if (!isA(expr, AttributeReference))
                needProjection = TRUE;
        }
    }

    INFO_LOG("aggregation function inputs and group by expressions are %s, we "
            "do %s need to create projection before aggregation" ,
            nodeToString(nonAttrRefExprs), (needProjection) ? "": " not ");

    freeList(aggregs);
    if  (needProjection)
        return nonAttrRefExprs;
    return NULL;
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
	List *aggregs = NIL;
	// get aggregations from select clause
	FOREACH(Node, sel, selectClause)
		visitAggregFunctionCall(sel, &aggregs);

	// get aggregations from having clause
	visitAggregFunctionCall(havingClause, &aggregs);

	DEBUG_LOG("aggregation functions are\n%s", nodeToString(aggregs));

	return aggregs;
}

static boolean
visitAggregFunctionCall(Node *n, List **aggregs)
{
	if (n == NULL)
		return TRUE;

	if (isA(n, FunctionCall)) // TODO how to prevent from going into nested sub-query?
	{
		FunctionCall *fc = (FunctionCall *) n;
		if (fc->isAgg)
		{
			DEBUG_LOG("Found aggregation '%s'.", exprToSQL((Node *) fc));
			*aggregs = appendToTailOfList(*aggregs, fc);

			// do not recurse into aggregation function calls
			return TRUE;
		}
	}

	return visit(n, visitAggregFunctionCall, aggregs);
}



