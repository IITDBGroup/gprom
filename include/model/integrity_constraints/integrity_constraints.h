/*
 *------------------------------------------------------------------------------
 *
 * integrity_constraints.h - Data structures for inclusion dependencies
 *
 *     Data structures for inclusion dependencies such as functional dependencies or inclusion dependencies.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-01
 *        SUBDIR: include/model/integrity_constraints/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _INTEGRITY_CONSTRAINTS_H_
#define _INTEGRITY_CONSTRAINTS_H_

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"

// functional dependencies
typedef struct FD {
	NodeTag type;
	char *table;
	Set *lhs;
	Set *rhs;
} FD;

/*
 * A dependency that is a universially quantified implication of two conjunction of atoms:
 *
 * FORALL X: R1(X1) AND ... AND Rn(Xn) -> EXISTS Y: S1(Z1), ..., Sm(Zm)
 *
 * where Z = X u Y
 *
 * This is used to represent, e.g., inclusion dependencies
 */
typedef struct FOdep {
	NodeTag type;
	List *lhs;
	List *rhs;
} FOdep;

extern FD *createFD(char *table, Set *lhs, Set *rhs);
extern Set *attrListToSet(List *attrs);
extern Set *attributeClosure(List *fds, Set *attrs, char *table);
extern FOdep *createFODep(List *lhs, List *rhs);
extern List *getFDsForAttributes(List *fds, Set *attrs);
extern List *normalizeFDs(List *fds);
extern char *icToString(Node *ics);

#endif /* _INTEGRITY_CONSTRAINTS_H_ */
