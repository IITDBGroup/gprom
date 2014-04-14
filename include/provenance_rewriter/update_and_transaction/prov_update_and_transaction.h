/*-----------------------------------------------------------------------------
 *
 * prov_update_and_transaction.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PROV_UPDATE_AND_TRANSACTION_H_
#define PROV_UPDATE_AND_TRANSACTION_H_

#include "model/query_operator/query_operator.h"

extern void mergeUpdateSequence(ProvenanceComputation *op);
extern void restrictToUpdatedRows (ProvenanceComputation *op);
extern boolean isSimpleUpdate(Node *update);

#endif /* PROV_UPDATE_AND_TRANSACTION_H_ */
