/*-----------------------------------------------------------------------------
 *
 * rpq_model.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_MODEL_RPQ_RPQ_MODEL_H_
#define INCLUDE_MODEL_RPQ_RPQ_MODEL_H_

#include "model/node/nodetype.h"
#include ""
NEW_ENUM(RegexOpType, )

typedef struct RegexNode {
    List *children;
    RegexOpType opType;
} RegexNode;

#endif /* INCLUDE_MODEL_RPQ_RPQ_MODEL_H_ */
