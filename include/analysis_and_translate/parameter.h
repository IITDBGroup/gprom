/*-----------------------------------------------------------------------------
 *
 * parameter.h
 *		Functions to deal with parameters in SQL statements
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_

#include "model/node/nodetype.h"
#include "model/list/list.h"

// set parameters in an SQL statement to the list of constants provided (values).
extern void setParameterValues (Node *qbModel, List *values);

// find all parameters in a Query Block model statement
extern List *findParameters (Node *qbModel);

#endif /* PARAMETER_H_ */
