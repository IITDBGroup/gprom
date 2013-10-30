/*-----------------------------------------------------------------------------
 *
 * translate_update.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef TRANSLATE_UPDATE_H_
#define TRANSLATE_UPDATE_H_

#include "model/query_operator/query_operator.h"
#include "model/node/nodetype.h"

extern QueryOperator *translateUpdate (Node *update);

#endif /* TRANSLATE_UPDATE_H_ */
