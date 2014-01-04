/*-----------------------------------------------------------------------------
 *
 * translate_update.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "analysis_and_translate/translate_update.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/translator.h"
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

static QueryOperator *translateUpdate(Update *f);
static QueryOperator *translateInsert(Insert *f);
static QueryOperator *translateDelete(Delete *f);

QueryOperator *
translateUpdate(Node *update) {

	switch (update->type) {
	case T_Insert:
		return translateInsert((Insert *) update);

	case T_Delete:
		return translateDelete((Delete *) update);

	case T_Update:
		return translateUpdate((Update *) update);
	default:
		return NULL;
	}
}


static QueryOperator *
translateInsert(Insert *insert) {
	List *attr = getAttributes(insert.tableName);
	QueryOperator *insertQuery;

	TableAccessOperator *to;
	to = createTableAccessOp(insert->tableName, NULL, NIL, deepCopyStringList(attr), NIL);

	if (isA(insert->query,  List))
	{
		ConstRelOperator *co;
		co = createConstRelOp(insert->query,NIL, deepCopyStringList(attr),NIL);
		insertQuery= (QueryOperator *) co;

	}
	else
	{
	    insertQuery =  translateQuery(insert->query);
	}

	enum SetOpType unionType = SETOP_UNION;
	SetOperator *seto;
	seto = createSetOperator(unionType, NIL, NIL, deepCopyStringList(attr));

	addChildOperator(seto, to);
	addChildOperator(seto, insertQuery);

	return (QueryOperator *) seto;
}

static QueryOperator *
translateDelete(Delete *delete)
{
	List *attr = getAttributes(delete.nodeName);

	TableAccessOperator *to;
	to = createTableAccessOp(strdup(delete.nodeName), NULL, NIL, NIL, deepCopyStringList(attr), NIL);

	SelectionOperator *so;
	Node *negatedCond;
	negatedCond = createOpExpr("NOT", singleton(copyObject(delete.cond)));
	so = createSelectionOp(negatedCond, NIL, NIL, deepCopyStringList(attr));


	addChildOperator(so, to);

	return (QueryOperator *) so;
}

static QueryOperator *
translateUpdate(Update *update)
{
    List *attrs = getAttributes(update.nodeName);

	TableAccessOperator *to;
	to = createTableAccessOp(strdup(update.nodeName), NULL, NIL, NIL,deepCopyStringList(attrs), NIL);

	SelectionOperator *so;
	so = createSelectionOp(copyObject(update.cond), NIL, NIL, deepCopyStringList(attrs));

	addChildOperator(so, to);

	// CREATE PROJECTION EXPRESSIONS
	List *projExprs = NIL;
	for(int i = 0; i < LIST_LENGTH(attrs); i++)
	{
        Node *projExpr= NULL;
	    FOREACH(Operator,o,update.selectClause)
        {
	        AttributeReference *a = (AttributeReference *) getNthOfList(o->args, 0);
	        if (a->attrPosition == i)
	            projExpr = (Node *) copyObject(getNthOfList(o->args, 1));
        }

	    if (projExpr == NULL)
	        projExpr = createFullAttrReference(getNthOfList(attrs,i), 0, i, 0);
	    projExprs = appendToTailOfList(projExprs, projExpr);
	}

	ProjectionOperator *po;
	po = createProjectionOp(projExprs, NIL, NIL, deepCopyStringList(attrs));

	addChildOperator(po, so);

	SelectionOperator *nso;
	Node *negatedCond;
	negatedCond = createOpExpr("NOT", singleton(copyObject(update.cond)));
	nso = createSelectionOp(negatedCond, NIL, NIL, deepCopyStringList(attrs));
	addChildOperator(nso, to);

	enum SetOpType unionType = SETOP_UNION;
	SetOperator *seto;
	seto = createSetOperator(unionType, NIL, NIL, deepCopyStringList(attrs));

	addChildOperator(seto, po);
	addChildOperator(seto, nso);

	return (QueryOperator *) seto;
}




