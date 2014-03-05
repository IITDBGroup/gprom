/*-----------------------------------------------------------------------------
 *
 * prov_update_and_transaction.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/query_operator/query_operator.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"


static QueryOperator *getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates);
static void mergeSerializebleTransaction(ProvenanceComputation *op);
static void mergeReadCommittedTransaction(ProvenanceComputation *op);

void
mergeUpdateSequence(ProvenanceComputation *op)
{
    ProvenanceTransactionInfo *tInfo = op->transactionInfo;

    switch(tInfo->transIsolation)
    {
        case ISOLATION_SERIALIZABLE:
            mergeSerializebleTransaction(op);
            break;
        case ISOLATION_READ_COMMITTED:
            mergeReadCommittedTransaction(op);
            break;
        default:
            FATAL_LOG("isolation level %u not supported:", tInfo->transIsolation);
            break;
    }

    INFO_LOG("updates after merge:\n%s", operatorToOverviewString((Node *) op));
}

static void
mergeSerializebleTransaction(ProvenanceComputation *op)
{
    List *updates = copyList(op->op.inputs);
    int i = 0;

    // cut links to parent
    removeParentFromOps(op->op.inputs, (QueryOperator *) op);
    op->op.inputs = NIL;

    // reverse list
    reverseList(updates);
    reverseList(op->transactionInfo->updateTableNames);

    DEBUG_LOG("Updates to merge are: \n\n%s", beatify(nodeToString(updates)));
    INFO_LOG("Updates to merge overview are: \n\n%s", operatorToOverviewString((Node *) updates));
    /*
     * Merge the individual queries for all updates into one
     */
    FOREACH(QueryOperator, u, updates)
    {
         List *children = NULL;

         // find all table access operators
         findTableAccessVisitor((Node *) u, &children);
         INFO_LOG("Replace table access operators in %s", operatorToOverviewString((Node *) u));

         FOREACH(TableAccessOperator, t, children)
         {
             INFO_LOG("\tTable Access %s", operatorToOverviewString((Node *) t));
             QueryOperator *up = getUpdateForPreviousTableVersion(op,
                     t->tableName, i, updates);

             INFO_LOG("\tUpdate is %s", operatorToOverviewString((Node *) up));
             // previous table version was created by transaction
             if (up != NULL)
                 switchSubtreeWithExisting((QueryOperator *) t, up);
             // previous table version is the one at transaction begin
             else
                 t->asOf = (Node *) getHeadOfListP(op->transactionInfo->scns);

             INFO_LOG("\tafter merge %s", operatorToOverviewString((Node *) u));
         }
         i++;
    }
    DEBUG_LOG("Merged updates are: %s", beatify(nodeToString(updates)));

    // replace updates sequence with root of the whole merged update query
    addChildOperator((QueryOperator *) op, (QueryOperator *) getHeadOfListP(updates));
    //op->op.inputs = singleton();
    DEBUG_LOG("Provenance computation for updates that will be passed "
            "to rewriter: %s", beatify(nodeToString(op)));
}

static void
mergeReadCommittedTransaction(ProvenanceComputation *op)
{
/*
	 List *updates = op->op.inputs;
	 int i = 0;
	 int v = -i;
	 removeParentFromOps(op->op.inputs, (QueryOperator *) op);
	 op->op.inputs = NIL;

	 FOREACH(TableAccessOperator, u, updates)
	 {
	    List *children = NULL;
	    findTableAccessVisitor((Node *) u, &children);

	    FOREACH(TableAccessOperator, t, children)

	    {
		  QueryOperator *up = getUpdateForPreviousTableVersion(op,
		                     t->tableName, i, v);
		  if (up != NULL)
		  {
			  switchSubtreeWithExisting((QueryOperator *) t, up);
		  }
		  else
		  {
			  t->asOf = (Node *) getHeadOfListP(op->transactionInfo->scns);
		  }
		  i++;
	    }

	 SetOperator *seto;
	 seto = createSetOperator(SETOP_UNION, updates, NIL, deepCopyStringList(updates));
 //	 addChildOperator((QueryOperator *) , (QueryOperator *) );
   //  addChildOperator((QueryOperator *) , (QueryOperator *) );
*/

//we need to deal with R(c-1) version of read committed
}



static QueryOperator *
getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates)
{
    ProvenanceTransactionInfo *tInfo = p->transactionInfo;
    int pos = startPos + 1;
    if (startPos + 1 >= LIST_LENGTH(tInfo->updateTableNames))
        return NULL;

    for(ListCell *lc = getNthOfList(tInfo->updateTableNames, startPos + 1); lc != NULL; lc = lc->next)
    {
        char *curTable = LC_STRING_VAL(lc);
        if (!strcmp(curTable, tableName))
            return (QueryOperator *) getNthOfListP(updates, pos);
        pos++;
    }

    return NULL;
}
