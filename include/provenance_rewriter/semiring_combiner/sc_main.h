#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"

extern boolean isSemiringCombinerActivatedOp(QueryOperator *op);
extern boolean isSemiringCombinerActivatedPs(ProvenanceStmt *stmt);
extern Node *getSemiringCombinerAddExpr(QueryOperator *op);
extern Node *getSemiringCombinerMultExpr(QueryOperator *op);
extern DataType getSemiringCombinerDatatype(ProvenanceStmt *stmt, List *dts);
extern QueryOperator *addSemiringCombiner (QueryOperator *result, Node *addExpr, Node *multExpr);
extern void addSCOptionToChild(QueryOperator *op, QueryOperator *to);
