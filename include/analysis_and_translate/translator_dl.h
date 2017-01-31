/*-----------------------------------------------------------------------------
 *
 * translator_dl.h
 *		translate DL into relational algebra (AGM model)
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_DL_H_
#define INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_DL_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"

extern Node *translateParseDL(Node *q);
extern QueryOperator *translateQueryDL(Node *node);
extern HashMap *taRel;

#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_DL_H_ */
