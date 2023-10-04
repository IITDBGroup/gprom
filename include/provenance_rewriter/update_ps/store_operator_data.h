#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_STORE_OPERATOR_DATA_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_STORE_OPERATOR_DATA_H_

#include "model/query_operator/query_operator.h"

extern boolean checkQueryInfoStored(char *queryName);
extern void storeOperatorData(QueryOperator *op);
extern void setInfoStored(char *queryName);
#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_UPDATE_PS_INCREMENTAL_H_ */