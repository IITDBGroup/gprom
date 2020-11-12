/*-----------------------------------------------------------------------------
 *
 * relation.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_MODEL_RELATION_RELATION_H_
#define INCLUDE_MODEL_RELATION_RELATION_H_

#include "model/list/list.h"

typedef struct Relation {
    NodeTag type;
    List *schema;
    List *tuples;
} Relation;

#endif /* INCLUDE_MODEL_RELATION_RELATION_H_ */
