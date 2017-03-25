/*-----------------------------------------------------------------------------
 *
 * whatif_algo.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

// only include cplex headers if the library is available
//TODO why this was included?
//#ifdef HAVE_LIBCPLEX
//#include <ilcplex/cplex.h>
//#endif
#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "parser/parser.h"
#include "symbolic_eval/expr_to_constraint.h"
#include "symbolic_eval/whatif_algo.h"
#include "model/set/hashmap.h"
#include "model/relation/relation.h"
#include "model/query_operator/query_operator.h"

static List *cond = NIL; // global pointer to the list of conditions
static List *tables = NIL; // global pointer to the list of tables

static void initWhatif(Node *update, Node *wUpdate);
static List *addTBToList(List *list, Node *n);
static char *getTBName(Node *n);

//initialize whatif algo and set the original update and the whatifquery
static void initWhatif(Node *update, Node *wUpdate) {

	cond = appendToTailOfList(cond, update);
	cond = appendToTailOfList(cond, wUpdate);

	tables = addTBToList(tables, update);
	tables = addTBToList(tables, wUpdate);
}

static List *addTBToList(List *list, Node *n) {
	char *tbName = getTBName(n);
	if (tbName != NULL && !searchListString(tables, tbName)) {
		list = appendToTailOfList(list, tbName);
	}
	return list;
}

static char *getTBName(Node *n) {
	char *tbName = NULL;
	switch (n->type) {
	case T_Update:
		tbName = ((Update *) n)->updateTableName;
		break;
	case T_Delete:
		tbName = ((Delete *) n)->deleteTableName;
		break;
	case T_Insert:
		tbName = ((Insert *) n)->insertTableName;
		break;
	default:
		break;
	}
	return tbName;
}

boolean checkCplex(List *exprs) {
	Node *u = popHeadOfListP(exprs);
	Node *e = popHeadOfListP(exprs);
	return exprToSat(u, TRUE, e, FALSE);
}

List *dependAlgo(List *exprs) {

	Node *up = popHeadOfListP(exprs);
	Node *wUp = popHeadOfListP(exprs);
	initWhatif(up, wUp);

	char *tbName = NULL;
	FOREACH(Node,e,exprs)
	{
		tbName = getTBName(e);
		if (searchListString(tables, tbName)) {
			FOREACH(Node,u,cond)
			{
				if (exprToSat(u, TRUE, e, FALSE)) {
					cond = appendToTailOfList(cond, e);
					tables = addTBToList(tables, e);
					break;
				}
			}
		}
	}
	return cond;
}
