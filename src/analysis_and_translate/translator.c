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

#include "analysis_and_translate/translator.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"

static QueryOperator *
translateSetQuery(SetQuery *sq, List *parents);

static QueryOperator *
translateQueryBlock(QueryBlock *qb, List *parents);

QueryOperator *
translateParse(Node *q) {
	return NULL;
}

static QueryOperator *
translateSetQuery(SetQuery *sq, List *parents) {
	List *inputs = appendToTailOfList(inputs, NULL);
	inputs = appendToTailOfList(inputs, NULL);
	// set children of the set operator node

	List *attrNames = NULL;
	FOREACH(char, attr, sq->selectClause)
		attrNames = appendToTailOfList(attrNames, attr);
	// set result table's attribute names of the set operator node

	SetOperator *so = createSetOperator(sq->setOp, inputs, parents, attrNames);
	List *pa = singleton(so);
	// create set operator node

	if (sq->lChild->type == T_SetQuery)
		_OP_LCHILD(so) = translateSetQuery((SetQuery *) sq->lChild, pa);
	else if (sq->lChild->type == T_QueryBlock)
		_OP_LCHILD(so) = translateQueryBlock(((QueryBlock *) sq->lChild), pa);

	if (sq->rChild->type == T_SetQuery)
		_OP_RCHILD(so) = translateSetQuery((SetQuery *) sq->rChild, pa);
	else if (sq->rChild->type == T_QueryBlock)
		_OP_RCHILD(so) = translateQueryBlock(((QueryBlock *) sq->rChild), pa);

	return ((QueryOperator *) so);
}

static QueryOperator *
translateQueryBlock(QueryBlock *qb, List *parents) {
	return NULL; //TODO
}
