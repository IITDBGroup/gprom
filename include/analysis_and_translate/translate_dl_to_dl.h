/*
 *------------------------------------------------------------------------------
 *
 * translate_dl_to_dl.h - translate datalog to datalog
 *
 *     This is a no-op except for that provenance computations are rewritten.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2023-01-04
 *        SUBDIR: include/analysis_and_translate/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _TRANSLATE_DL_TO_DL_H_
#define _TRANSLATE_DL_TO_DL_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern Node *translateParseDLToDL(Node *q);
extern QueryOperator *translateQueryDLToDL(Node *node);

#endif /* _TRANSLATE_DL_TO_DL_H_ */
