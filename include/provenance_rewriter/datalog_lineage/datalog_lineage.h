/*
 *------------------------------------------------------------------------------
 *
 * datalog_lineage.h - Rewrites for computing Lineage for Datalog programs.
 *
 *
 *     Given a relation of interest and Datalog program, these rewrites
 *     instrument the program to capture lineage for this query wrt. this
 *     relation.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-09
 *        SUBDIR: include/provenance_rewriter/datalog_lineage/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _DATALOG_LINEAGE_H_
#define _DATALOG_LINEAGE_H_

#include "model/datalog/datalog_model.h"
#include "model/graph/graph.h"
#include "model/set/hashmap.h"

extern DLProgram *rewriteDLForLinageCapture(DLProgram *p);
extern List *createCaptureRule(DLRule *r, DLAtom *targetAtom, char *filterAnswerPred, Graph *goalToHeadPred, HashMap *ruleids);
extern List *createCaptureRuleForTable(DLRule *r, char *table, char *filterAnswerPred, DLAtom *goal, Graph *goalToHeadPred, HashMap *ruleids);

#endif /* _DATALOG_LINEAGE_H_ */
