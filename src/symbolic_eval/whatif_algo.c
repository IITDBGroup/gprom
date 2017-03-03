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
static void addTBToList(List *list, Node *n);

//initialize whatif algo and set the original update and the whatifquery
static void initWhatif(Node *update, Node *wUpdate) {

	cond = appendToTailOfList(cond, update);
	cond = appendToTailOfList(cond, wUpdate);

	addTBToList(tables, update);
	addTBToList(tables, wUpdate);
}

static void addTBToList(List *list, Node *n) {

	switch (n->type) {
	case T_Update:
		list = appendToTailOfList(list, ((Update *) n)->updateTableName);
		break;
	case T_Delete:
		list = appendToTailOfList(list, ((Delete *) n)->deleteTableName);
		break;
	case T_Insert:
		list = appendToTailOfList(list, ((Insert *) n)->insertTableName);
		break;
	default:
		break;
	}
}

List *dependAlgo(List *exprs) {
	Node *up = popHeadOfListP(exprs);
	Node *wUp = popHeadOfListP(exprs);
	initWhatif(up, wUp);

	char *tbName;

	FOREACH(Node,e,exprs)
	{

		switch (e->type) {
		case T_Update:
			tbName = ((Update *) e)->updateTableName;
			break;
		case T_Delete:
			tbName = ((Delete *) e)->deleteTableName;
			break;
		case T_Insert:
			tbName = ((Insert *) e)->insertTableName;
			break;
		default:
			break;
		}
		if (searchListString(tables, tbName)) {
			FOREACH(Node,u,cond)
			{
				if (exprToSat(u, TRUE, e, FALSE)) {
					cond = appendToTailOfList(cond, e);
					//if (!searchListString(tables, tbName))
					//addTBToList(tables, e);
				}
			}
		}
	}

	return cond;
}

