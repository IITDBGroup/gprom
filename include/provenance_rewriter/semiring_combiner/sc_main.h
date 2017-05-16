#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"

extern boolean isSemiringCombinerActivatedOp(QueryOperator *op);
extern boolean isSemiringCombinerActivatedPs(ProvenanceStmt *stmt);
extern char *getSemiringCombinerFuncName(QueryOperator *op);
extern Node *getSemiringCombinerExpr(QueryOperator *op);
extern DataType getSemiringCombinerDatatype(ProvenanceStmt *stmt, List *dts);
extern QueryOperator *addSemiringCombiner (QueryOperator *result, char * funcN, Node * expr);
extern void addSCOptionToChild(QueryOperator *op, QueryOperator *to);
extern Node *getUncertaintyExpr(Node *expr, HashMap *hmp, Set* st); //hashmap of attrref->attrref(attribute->attribute uncertainty)
