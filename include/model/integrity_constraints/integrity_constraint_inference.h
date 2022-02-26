/*
 *------------------------------------------------------------------------------
 *
 * integrity_constraint_inference.h - Inferring integrity constraints for DL queries
 *
 *     Methods that infer integrity constraints for Datalog rules based on base table ICs.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2022-02-18
 *        SUBDIR: include/model/integrity_constraints/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _INTEGRITY_CONSTRAINT_INFERENCE_H_
#define _INTEGRITY_CONSTRAINT_INFERENCE_H_

#include "model/datalog/datalog_model.h"
#include "model/list/list.h"

extern List *inferFDsForProgram(DLProgram *program);
extern List *inferFDsForRule(DLProgram * p, DLRule *r, List *fds);

#endif /* _INTEGRITY_CONSTRAINT_INFERENCE_H_ */
