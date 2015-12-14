/*-----------------------------------------------------------------------------
 *
 * rpq_to_datalog.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_RPQ_RPQ_TO_DATALOG_H_
#define INCLUDE_RPQ_RPQ_TO_DATALOG_H_

#include "model/rpq/rpq_model.h"

NEW_ENUM_WITH_TO_STRING(RPQQueryType,
RPQ_QUERY_RESULT,
RPQ_QUERY_SUBGRAPH,
RPQ_QUERY_PROV
);

extern Node *rpqToDatalog(Regex *req); //for now later add other stuff

#endif /* INCLUDE_RPQ_RPQ_TO_DATALOG_H_ */
