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
	QueryOperator output = NEW(QueryOperator);
	List *attr = getAttributes(insert.tableName);

	TableAccessOperator *to = NEW(TableAccessOperator);
	to = createTableAccessOp(insert->tableName, NULL, NIL, attr, NIL);

	TableAccessOperator *ro = NEW(TableAccessOperator);
	ro = createTableAccessOp(insert->tableName, NULL, NIL, insert->attrList,NIL);


	enum SetOpType unionType = SETOP_UNION;
	SetOperator *seto = NEW(SetOperator);
	seto = createSetOperator(unionType, to, NIL, attr);

	addChildOperator(seto, to);
	addChildOperator(seto, ro);

	OP_LCHILD(seto)->parents = singleton(seto);
	output = (QueryOperator *) seto;

	return (QueryOperator *) output;
}

static QueryOperator *
translateDelete(Delete *delete) {

	QueryOperator output = NEW(QueryOperator);
	List *attr = getAttributes(delete.nodeName);

	TableAccessOperator *to = NEW(TableAccessOperator);
	to = createTableAccessOp(delete.nodeName, NULL, NIL, NIL, attr, NIL);

	SelectionOperator *so = NEW(SelectionOperator);
	so = createSelectionOp(delete.cond, to, NIL, attr);
	// so->op.schema->name= "Not"; //How to set not for selection OP?

	addChildOperator(so, to);

	OP_LCHILD(so)->parents = singleton(so);
	output = (QueryOperator *) so;

	return (QueryOperator *) output;
}

static QueryOperator *
translateUpdate(Update *update) {
	QueryOperator output = NEW(QueryOperator);

	TableAccessOperator *to = NEW(TableAccessOperator);
	to = createTableAccessOp(update.nodeName, NULL, NIL, NIL,update.selectClause, NIL);

	SelectionOperator *so = NEW(SelectionOperator);
	so = createSelectionOp(update.cond, to, NIL, update.selectClause);

	addChildOperator(so, to);

	ProjectionOperator *po = NEW(ProjectionOperator);
	po = createProjectionOp(NIL, so, NIL, update.selectClause);

	addChildOperator(po, so);

	SelectionOperator *nso = NEW(SelectionOperator);
	nso = so;
	// nso->op.schema->name= "Not"; //How to set not for selection OP?
	addChildOperator(nso, to);

	enum SetOpType unionType = SETOP_UNION;
	SetOperator *seto = NEW(SetOperator);
	seto = createSetOperator(unionType, po, NIL, update->selectClause);

	addChildOperator(seto, po);
	addChildOperator(seto, nso);

	OP_LCHILD(seto)->parents = singleton(seto);
	output = (QueryOperator *) seto;

	return (QueryOperator *) output;
}




