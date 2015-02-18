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

/*try to remove unnecessary columns */
extern QueryOperator *removeUnnecessaryColumns(QueryOperator *root);
extern QueryOperator *removeUnnecessaryColumnsFromProjections(QueryOperator *root);
extern QueryOperator *removeUnnecessaryAttrDefInSchema(Set *icols, QueryOperator *op);
extern void resetAttrPosInCond(QueryOperator *root, Operator *condOp);
extern void resetPos(AttributeReference *ar,  List* attrDefs);

/*try to remove redundant duplicate operator */
extern QueryOperator *removeRedundantDuplicateOperatorBySet(QueryOperator *root);

/*try to remove redundant duplicate operator */
extern QueryOperator *removeRedundantDuplicateOperatorByKey(QueryOperator *root);

/* try to pull up provenance projections. */
extern QueryOperator *pullingUpProvenanceProjections(QueryOperator *root);

/* try to push down the selection operator through joins for provenence. */
extern QueryOperator *pushDownSelectionThroughJoinsOperatorOnProv(QueryOperator *root);

/* try to implement selection move around */
extern QueryOperator *selectionMoveAround(QueryOperator *root);

/* used in selection move around,  step 1  */
extern void setMoveAroundListSetProperityForWholeTree(QueryOperator *root);

/* used in selection move around,  step 2  */
extern void reSetMoveAroundListSetProperityForWholeTree(QueryOperator *root);

/* used in selection move around,  step 3  */
extern void introduceSelection(QueryOperator *root);

/* used in selection move around  */
extern List *getMoveAroundOpList(QueryOperator *qo);
extern List *addNonEqOpToOplistInMoveAround(QueryOperator *root, QueryOperator *opl, List *opListr);
extern void introduceSelectionOrChangeSelectionCond(List *opList, QueryOperator *qo1);
extern List *removeRedundantSelectionCondOfOpList(List *opList);

#endif /* OPERATOR_OPTIMIZER_H_ */
