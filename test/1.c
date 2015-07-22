
#include "common.h"

#include "configuration/option.h"
#include "configuration/option_parser.h"


#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "analysis_and_translate/analyze_oracle.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "metadata_lookup/metadata_lookup.h"
#include "configuration/option.h"

#include "mem_manager/mem_mgr.h"

#include "execution/executor.h"
#include "rewriter.h"


#define PROP_PC_TRANS_XID "TRANSACTION_XID"

int main(int argc, char* argv[]){

     List *scns = NIL;
     List *sqls = NIL;
     List *sqlBinds = NIL;
//     ProvenanceComputation *result;
     IsolationLevel isoLevel;
     Constant *commitSCN;
//     ProvenanceComputation *op;
     char *xid = "3002170034500000";

     READ_OPTIONS_AND_INIT("testrewriter", "Run all stages on input and output rewritten SQL.");

     DEBUG_LOG("got this far");

     // call metadata lookup -> SCNS + SQLS
     commitSCN = createConstLong(-1L);
     getTransactionSQLAndSCNs(xid, &scns, &sqls, &sqlBinds, &isoLevel, commitSCN);

     // set provenance transaction info
//     ProvenanceTransactionInfo *tInfo = makeNode(ProvenanceTransactionInfo);
//     result->transactionInfo = tInfo;
//     tInfo->updateTableNames = NIL;
//     tInfo->scns = scns;
//     tInfo->transIsolation = isoLevel;

     DEBUG_LOG("we got commit SCN %u", LONG_VALUE(commitSCN));
     DEBUG_LOG("we got commit SCN %u", isoLevel);

     FOREACH(Constant,c,scns)
     {
         DEBUG_LOG("scns is %u", LONG_VALUE(c));
     }

     //for the transaction info found, printf the next transactions info by using the ListCell *next
//     ListCell *aux = result;
//
//     while (aux != NIL){
//         printf("/n%u", aux->data);        //printf the cell value
//         aux = aux->next;
//     }

     shutdownApplication();
     return EXIT_SUCCESS;
}
