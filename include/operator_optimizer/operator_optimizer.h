/*-----------------------------------------------------------------------------
 *
 * operator_optimizer.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef OPERATOR_OPTIMIZER_H_
#define OPERATOR_OPTIMIZER_H_

#include "model/query_operator/query_operator.h"
#include "include/model/set/set.h"

/* apply all optimizations */
extern Node *optimizeOperatorModel (Node *root);

/* activate materialization for adjcent projections */
extern QueryOperator *materializeProjectionSequences (QueryOperator *root);

/* try to merge adajacent operators of the same type into one operator. */
extern QueryOperator *mergeAdjacentOperators (QueryOperator *root);

/* try to push down the selection operator for provenence. */
extern QueryOperator *pushDownSelectionOperatorOnProv(QueryOperator *root);

extern QueryOperator *factorAttrsInExpressions(QueryOperator *root);

extern QueryOperator *removeRedundantProjections(QueryOperator *root);

/* try to remove unnecessary window operator */
extern QueryOperator *removeUnnecessaryWindowOperator(QueryOperator *root);

/* try to remove unnecessary columns */
extern QueryOperator *removeUnnecessaryColumns(QueryOperator *root);
extern QueryOperator *removeUnnecessaryColumnsFromProjections(QueryOperator *root);
extern QueryOperator *removeUnnecessaryAttrDefInSchema(Set *icols, QueryOperator *op);
extern void resetAttrPosInCond(QueryOperator *root, Operator *condOp);
extern void resetPos(AttributeReference *ar,  List* attrDefs);

/* try to remove redundant duplicate operator */
extern QueryOperator *removeRedundantDuplicateOperatorBySet(QueryOperator *root);

/* try to remove redundant duplicate operator */
extern QueryOperator *removeRedundantDuplicateOperatorByKey(QueryOperator *root);

/* try to pull up duplicate operator */
extern QueryOperator *pullUpDuplicateRemoval(QueryOperator *root);
extern void findDuplicateRemoval(List **drOp, QueryOperator *root);
extern void doPullUpDuplicateRemoval(DuplicateRemoval *root);

/* try to pull up provenance projections. */
extern QueryOperator *pullingUpProvenanceProjections(QueryOperator *root);

/* try to push down the selection operator through joins for provenence. */
extern QueryOperator *pushDownSelectionThroughJoinsOperatorOnProv(QueryOperator *root);

/* try to implement selection move around */
extern QueryOperator *selectionMoveAround(QueryOperator *root);

#endif /* OPERATOR_OPTIMIZER_H_ */
