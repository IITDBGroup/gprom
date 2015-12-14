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

extern Node *rpqToDatalog(Regex *rpq, RPQQueryType type, char *edgeRel, char *outRel);

#endif /* INCLUDE_RPQ_RPQ_TO_DATALOG_H_ */
