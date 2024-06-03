/*
 *------------------------------------------------------------------------------
 *
 * prov_semantic_optimization.h - Semantic query optimization with constraints for provenance capture.
 *
 *     Implements faster alternatives to chase-based methods for semantic query
 *     optimization for the special case of provenance capture queries.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-01
 *        SUBDIR: include/provenance_rewriter/semantic_optimization/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _PROV_SEMANTIC_OPTIMIZATION_H_
#define _PROV_SEMANTIC_OPTIMIZATION_H_

#include "model/integrity_constraints/integrity_constraints.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/graph/graph.h"
#include "model/datalog/datalog_model.h"

extern List *optimizeDLRule(DLProgram *p, DLRule *r, List *fds, DLAtom *target, char *filterPred);
extern List *adaptFDsToRules(DLProgram *p, DLRule *r, List *fds);
extern boolean checkFDonAtoms(Set *atoms, List *fds, FD *fd);
extern Set *attributeClosureOnAtoms(Set *atoms, Set *attrs, List *fds);
extern Graph *createJoinGraph(DLRule *r);
extern DLRule *minimizeDLRule(DLRule *r);

#endif /* _PROV_SEMANTIC_OPTIMIZATION_H_ */
