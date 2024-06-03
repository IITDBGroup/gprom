/*-----------------------------------------------------------------------------
 *
 * datalog_model_checker.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_MODEL_DATALOG_DATALOG_MODEL_CHECKER_H_
#define INCLUDE_MODEL_DATALOG_DATALOG_MODEL_CHECKER_H_

#include "model/node/nodetype.h"
#include "model/datalog/datalog_model.h"

boolean checkDLModel(Node *dlModel);
boolean checkDLRuleSafety(DLRule *r);
boolean checkFact(DLAtom *f);


#endif /* INCLUDE_MODEL_DATALOG_DATALOG_MODEL_CHECKER_H_ */
