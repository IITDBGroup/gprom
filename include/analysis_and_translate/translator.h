/*-----------------------------------------------------------------------------
 *
 * translator.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern Node *translateParse(Node *q);

#endif /* TRANSLATOR_H_ */
