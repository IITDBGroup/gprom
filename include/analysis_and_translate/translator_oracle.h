/*-----------------------------------------------------------------------------
 *
 * translator_oracle.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_ORACLE_H_
#define INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_ORACLE_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern Node *translateParseOracle(Node *q);
extern QueryOperator *translateQueryOracle(Node *node);


#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_ORACLE_H_ */
