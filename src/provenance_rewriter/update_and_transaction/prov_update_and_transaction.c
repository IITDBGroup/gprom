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


static QueryOperator *getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos);
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
    List *updates = op->op.inputs;
    int i = 0;

    // reverse list
    reverseList(updates);

    DEBUG_LOG("Updates to merge are: %s", beatify(nodeToString(updates)));

    /*
     * Merge the individual queries for all updates into one
     */
    FOREACH(QueryOperator, u, op->op.inputs)
    {
         List *children = NULL;

         // find all table access operators
         findTableAccessVisitor((Node *) u, &children);
         DEBUG_LOG("Replace table access operators in %s", operatorToOverviewString((Node *) u));

         FOREACH(TableAccessOperator, t, children)
         {
             DEBUG_LOG("\tTable Access %s", operatorToOverviewString((Node *) t));
             QueryOperator *up = getUpdateForPreviousTableVersion(op,
                     t->tableName, i);

             DEBUG_LOG("\tUpdate is %s", operatorToOverviewString((Node *) up));
             // previous table version was created by transaction
             if (up != NULL)
                 switchSubtrees((QueryOperator *) t, up);
             // previous table version is the one at transaction begin
             else
                 t->asOf = (Node *) getHeadOfListP(op->transactionInfo->scns);//TODO get SCN
         }
         i++;
    }
    DEBUG_LOG("Merged updates are: %s", beatify(nodeToString(updates)));

    // replace updates sequence with root of the whole merged update query
    removeParentFromOps(op->op.inputs, (QueryOperator *) op);
    op->op.inputs = singleton(getHeadOfListP(updates));
    DEBUG_LOG("Provenance computation for updates that will be passed "
            "to rewriter: %s", beatify(nodeToString(op)));
}

static void
mergeReadCommittedTransaction(ProvenanceComputation *op)
{

   /* List *updates = op->op.inputs;
    int v = 0;
    int k = 0;
    List *children = NULL;

    FOREACH(TableAccessOperator, t, children)
    {
       t->asOf = (Node *) getHeadOfListP(op->transactionInfo->scns);

    }

	if(v< = 1 && )
	{

	}

	else(v>1 && )
		{

		}
 */
//we need to deal with R(c-1) version of read committed
}



static QueryOperator *
getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos)
{
    ProvenanceTransactionInfo *tInfo = p->transactionInfo;
    int pos = startPos + 1;
    if (startPos + 1 >= LIST_LENGTH(tInfo->updateTableNames))
        return NULL;

    for(ListCell *lc = getNthOfList(tInfo->updateTableNames, startPos + 1); lc != NULL; lc = lc->next)
    {
        char *curTable = LC_STRING_VAL(lc);
        if (!strcmp(curTable, tableName))
            return (QueryOperator *) getNthOfListP(p->op.inputs, pos);
        pos++;
    }

    return NULL;
}
