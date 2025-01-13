/*-----------------------------------------------------------------------------
 *
 * query_operator_model_checker.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef QUERY_OPERATOR_MODEL_CHECKER_H_
#define QUERY_OPERATOR_MODEL_CHECKER_H_

#include "query_operator.h"

extern boolean isTree(QueryOperator *op);

extern boolean checkModel(QueryOperator *op);
extern boolean checkSingleOperator(QueryOperator *op);
extern boolean checkUniqueAttrNames(QueryOperator *op);
extern void makeAttrNamesUnique(QueryOperator *op);


#endif /* QUERY_OPERATOR_MODEL_CHECKER_H_ */
