/*-----------------------------------------------------------------------------
 *
 * analyze_qb.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef ANALYSE_QB_H_
#define ANALYSE_QB_H_

#include "model/node/nodetype.h"
#include "model/list/list.h"

void analyzeQueryBlockStmt (Node *stmt, List *parentFroms);

#endif /* ANALYSE_QB_H_ */
